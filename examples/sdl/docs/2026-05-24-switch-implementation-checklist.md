# Switch 补齐任务清单

> 日期：2026-05-24  
> 目标：把 `ldSwitch` 补成用户感知接近 LVGL 的独立控件  
> 范围：只补 `switch` 自身能力；**不**做通用样式系统、通用动画引擎、统一 part/state/property 抽象

## 1. 本轮完成标准

完成后，`switch` 需要同时满足：

1. 默认方向支持 `AUTO`，尺寸变化时能自动判定横/竖
2. 程序初始化设值时首帧不播放滑动动画
3. 渲染内部具备 `track / indicator / knob` 三段语义
4. `checked / unchecked / disabled / pressed` 视觉和交互稳定
5. 稳定发出且只发一次 `SIGNAL_VALUE_CHANGED`
6. 具备控件级行为测试和最小截图回归

---

## 2. 涉及文件

### 核心实现

- `src/gui/ldSwitch.h`
- `src/gui/ldSwitch.c`
- `src/gui/ldSwitchInternal.h`
- `src/gui/ldSwitchInternal.c`

### 现有 demo / 接入点

- `examples/common/demo/widget/uiWidgetLegacy.c`
- `examples/common/demo/widget/uiWidgetSwipePage01.c`

### 测试与文档

- `examples/sdl/tests/switch/test_ldswitch_internal.c`
- `examples/sdl/tests/check_use_demo_0_legacy_widget.py`
- `README.md`
- `docs/tutorial/04 api.md`
- `docs/superpowers/specs/2026-05-24-ldswitch-widget-design.md`

### 对照基线

- `third_party/lvgl/include/lvgl/widgets/lv_switch.h`
- `third_party/lvgl/src/widgets/switch/lv_switch.c`
- `third_party/lvgl/docs/src/widgets/switch.mdx`
- `third_party/lvgl/tests/src/test_cases/widgets/test_switch.c`
- `third_party/lvgl/examples/widgets/switch/switch_orientation/lv_example_switch_orientation.c`
- `third_party/lvgl/examples/widgets/switch/lv_example_switch_event.c`
- `third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c`

---

## 3. A 组：基础能力补足

## A1. 收口状态模型与方向模型

### 目标

先冻结 `switch` 的内部真值，避免后面边改渲染边改状态定义。

### 步骤

- [ ] 在 `src/gui/ldSwitch.h` 中把方向从布尔值升级为 switch 私有三态枚举
  - 建议新增：`ldSwitchOrientationAuto`、`ldSwitchOrientationHorizontal`、`ldSwitchOrientationVertical`
  - `ldSwitch_t` 内部保存 `orientation`，不再只保留 `isHorizontal`
- [ ] 兼容现有查询函数
  - `ldSwitchIsHorizontal()` 内部改为读取最终解析后的方向
- [ ] 保留现有 `ldSwitchSetHorizontal()` 作为兼容入口
  - 它内部只做语义转换：`true -> Horizontal`，`false -> Vertical`
- [ ] 新增更明确的方向接口
  - 建议：`ldSwitchSetOrientation(ldSwitch_t *ptWidget, ldSwitchOrientation_t orientation)`
- [ ] 明确并写死方向解析规则
  - `AUTO`：`width >= height` 时横向，否则纵向
  - `HORIZONTAL`：强制横向
  - `VERTICAL`：强制纵向
- [ ] 明确并写死纵向语义
  - 底部为 off
  - 顶部为 on
- [ ] 明确 checked 真值规则
  - 只有 checked 变化才发 `SIGNAL_VALUE_CHANGED`
  - 重复设同值不能重复发事件

### 验证

- [ ] 补 host-side 用例：`AUTO` 在 `60x30` 时为横向
- [ ] 补 host-side 用例：`AUTO` 在 `30x60` 时为纵向
- [ ] 补 host-side 用例：强制纵向时不受尺寸影响
- [ ] 补 host-side 用例：强制横向时不受尺寸影响

## A2. 重构内部几何语义

### 目标

让 `switch` 内部真正具备 `track / indicator / knob` 三段结构。

### 步骤

- [ ] 保持 `src/gui/ldSwitchInternal.*` 作为纯计算边界
- [ ] 在 `src/gui/ldSwitchInternal.h` 增加新几何结构
  - 建议增加最终方向、track 区域、indicator 区域、knob 区域的描述结构
- [ ] 在 `src/gui/ldSwitchInternal.c` 增加方向解析 helper
- [ ] 在 `src/gui/ldSwitchInternal.c` 增加三段区域计算 helper
  - 横向
  - 纵向
- [ ] 让 indicator 的几何随 `animProgress` 连续变化
  - 不再用 `animProgress >= 500` 作为整条轨道切换阈值
- [ ] 保留 knob 线性滑动模型
  - 继续使用 `animProgress 0..1000`
- [ ] 保持 `knobPadding` 为 switch 私有能力
  - 不抽成通用样式系统

### 验证

- [ ] 补 host-side 用例：横向几何能同时给出 track / indicator / knob 合法区域
- [ ] 补 host-side 用例：纵向几何能同时给出 track / indicator / knob 合法区域
- [ ] 补 host-side 用例：`animProgress=0/500/1000` 时 knob 和 indicator 区域连续变化

## A3. 首帧 no-animation 语义补足

### 目标

避免初始化后程序设值导致首帧自己滑过去。

### 步骤

- [ ] 在 `src/gui/ldSwitch.c` 明确区分两类状态变更
  - 初始化期 / 首帧前程序设值
  - 稳定渲染后的运行期设值
- [ ] 在 `ldSwitchApplyValue()` 中增加首帧直达终态规则
  - 首帧前设值：直接 `animProgress = target`
  - 运行期设值：允许动画推进
- [ ] 若需要，给 `ldSwitch_t` 增加一个极小状态位
  - 例如 `hasRenderedOnce` 或等价语义
- [ ] 在 `ldSwitch_on_frame_complete()` 或合适时机确认首帧状态已经稳定

### 验证

- [ ] 补 host-side 用例：初始化后立刻 `SetChecked(true)` 时不进入动画中间态
- [ ] 回归检查 `examples/common/demo/widget/uiWidgetSwipePage01.c`
  - 页面初始 checked 时首帧视觉应直接稳定

---

## 4. B 组：交互与渲染补足

## B1. 收口交互状态机

### 目标

让 press / release / disabled / 重复设值语义都稳定下来。

### 步骤

- [ ] 审视 `slotSwitchProcess()` 当前逻辑
  - `PRESS` 只负责进入 pressed
  - `RELEASE` 才提交 checked 切换
- [ ] 保留当前“release 提交切换”的主路径
- [ ] 明确 disabled 行为
  - `disabled=true` 时忽略 `PRESS/RELEASE`
- [ ] 明确重复设值行为
  - `SetChecked(current)` 不重启动画，不重复发事件
- [ ] 若消息系统支持，检查是否有取消/越界 release 情况
  - 若当前框架没有，不在这轮强做

### 验证

- [ ] 补测试：一次有效点击只触发一次 `SIGNAL_VALUE_CHANGED`
- [ ] 补测试：`disabled=true` 时点击不切换
- [ ] 补测试：`SetChecked(true)` 后再次 `SetChecked(true)` 不重复发事件

## B2. 重构 `ldSwitch_show()` 渲染顺序

### 目标

让视觉从“整条轨道切换”升级为“三段语义清晰的 switch”。

### 步骤

- [ ] 先画 `track/main`
- [ ] 再画 `indicator`
- [ ] 最后画 `knob`
- [ ] 颜色模式下
  - `track` 始终作为底轨
  - `indicator` 只负责开态填充区域
  - `knob` 独立着色
- [ ] 图片模式下
  - 优先复用现有 on/off track 与 knob 资源
  - 若图片或 mask 缺失，继续 fallback 到颜色模式
- [ ] 调整 `pressed` 反馈
  - 不只让 knob 借用 border 色
  - 至少保证用户能稳定感知“我按下了”
- [ ] 保持 disabled 半透明策略
  - 不引入统一 disabled style 抽象

### 验证

- [ ] 目视检查 horizontal off / on
- [ ] 目视检查 vertical off / on
- [ ] 目视检查 disabled off / on
- [ ] 目视检查 pressed feedback 是否明显

## B3. 稳定 demo 接入点

### 目标

防止 switch 改造后把现有 demo 打坏。

### 步骤

- [ ] 回归 `examples/common/demo/widget/uiWidgetLegacy.c`
  - `OFF/ON` 文案切换仍正确
- [ ] 回归 `examples/common/demo/widget/uiWidgetSwipePage01.c`
  - 初始 checked 状态正确
  - 首帧不滑动
- [ ] 若需要，在 demo 中额外补一个 vertical switch 展示点
  - 只在 demo 层增加最小展示，不扩散到全仓库

---

## 5. C 组：验证与文档补足

## C1. 测试矩阵补足

### 目标

从 helper 级测试升级到控件级测试。

### 最少补齐的用例

- [ ] 默认创建态
- [ ] 程序设值态
- [ ] 重复设值不重复发事件
- [ ] disabled 阻止交互切换
- [ ] auto orientation
- [ ] forced horizontal / vertical
- [ ] 首帧 no-animation
- [ ] 反向动画（on -> off）
- [ ] 图片模式 fallback

### 建议文件

- 继续扩 `examples/sdl/tests/switch/test_ldswitch_internal.c`
- 或新增一个控件级测试文件，例如 `examples/sdl/tests/switch/test_ldswitch_widget.c`

## C2. 最小截图回归

### 目标

让视觉改动可回归，不只靠肉眼描述。

### 建议截图矩阵

- [ ] horizontal off
- [ ] horizontal on
- [ ] vertical off
- [ ] vertical on
- [ ] disabled off
- [ ] disabled on

### 对照参考

- `third_party/lvgl/tests/src/test_cases/widgets/test_switch.c`

## C3. 文档口径同步

### 目标

把“代码真值”和“文档口径”重新对齐。

### 步骤

- [ ] 更新 `README.md`
  - 写清当前 switch 支持边界
- [ ] 更新 `docs/tutorial/04 api.md`
  - 补齐 `ldSwitchSetChecked`
  - 补齐方向接口说明
- [ ] 更新 `docs/superpowers/specs/2026-05-24-ldswitch-widget-design.md`
  - 从“待实现”改为“已实现基础版，待补足能力”
- [ ] 如有必要，在 `examples/sdl/docs/2026-05-24-switch-vs-lvgl-gap-analysis.md` 补充完成状态回写

---

## 6. 推荐顺序

### 第一阶段

- A1 状态 / 方向模型
- A2 三段几何语义
- A3 首帧 no-animation

### 第二阶段

- B1 交互状态机收口
- B2 `ldSwitch_show()` 渲染重构
- B3 demo 回归

### 第三阶段

- C1 控件级测试
- C2 截图回归
- C3 文档同步

---

## 7. 完成判定

满足下面条件才算 `switch` 第一轮补齐完成：

- `AUTO` / 强制方向语义都成立
- 首帧设值不滑动
- 渲染内部有 `track / indicator / knob`
- 值变化事件稳定且只发一次
- legacy / swipe demo 都正常
- 至少有一层控件级行为测试
- 文档口径不再漂移
