# LingDongGUI Switch 对齐 LVGL 差距分析与补足步骤

> 日期：2026-05-24  
> 范围：`src/gui/ldSwitch.*`、`src/gui/ldSwitchInternal.*`、相关 demo/test，与 `third_party/lvgl` 的 `lv_switch` 对比  
> 当前阶段约束：**只补 switch 自身能力，不做上层抽象，不展开通用样式系统、通用动画引擎、统一 part/state/property 框架**

## 1. 结论先说

当前 LingDongGUI 的 `switch` 不是空白，而是已经有了一个可运行的独立控件：

- 已有 checked / disabled / pressed / horizontal 的基础状态
- 已有颜色模式与图片模式
- 已有 knob 滑动动画
- 已有 `SIGNAL_VALUE_CHANGED`
- 已有 demo 与基础 host-side 逻辑测试

但它与 LVGL 的差距，不在“有没有 switch”，而在“用户摸起来像不像 LVGL switch”。

当前最明显的缺口有 4 类：

1. **方向语义不完整**：LingDongGUI 只有布尔横竖，没有 LVGL 的 `AUTO` 方向。
2. **渲染语义不完整**：LingDongGUI 现在更像“整条轨道换色 + knob 滑动”，还不是真正的 `MAIN / INDICATOR / KNOB` 三段结构。
3. **首帧与程序设值体验不完整**：当前程序初始化设值时，容易出现首帧动画；LVGL 会避免这个问题。
4. **控件级行为验证不完整**：当前测试主要锁住内部几何/动画 helper，没锁住完整控件行为。

所以这轮不该做“switch 抽象体系”，而应该做：

- 把 `switch` 的用户感知补到接近 LVGL
- 把 `switch` 的内部语义补完整
- 把 `switch` 的测试与文档补到控件级

---

## 2. 当前 LingDongGUI 的代码真值

### 2.1 已有实现

当前仓库已经存在正式 `ldSwitch` 控件：

- 数据结构：`src/gui/ldSwitch.h`
- 主实现：`src/gui/ldSwitch.c`
- 几何/动画 helper：`src/gui/ldSwitchInternal.h`、`src/gui/ldSwitchInternal.c`
- 纯逻辑测试：`examples/sdl/tests/switch/test_ldswitch_internal.c`
- demo 接入：
  - `examples/common/demo/widget/uiWidgetLegacy.c`
  - `examples/common/demo/widget/uiWidgetSwipePage01.c`

### 2.2 当前已有能力

当前 `ldSwitch_t` 已包含这些字段：

- 视觉资源：`offTrackColor`、`onTrackColor`、`knobColor`、`borderColor`
- 图片资源：off/on track 图片与 knob 图片
- 动画字段：`animProgress`、`animStartProgress`、`animTargetProgress`、`animElapsedMs`
- 状态字段：`isChecked`、`isHorizontal`、`isDisabled`、`isPressed`、`isAnimating`
- 绘制模式字段：`useImageStyle`

当前公开 API 也已具备基本轮廓：

- 初始化：`ldSwitch_init`
- 程序设值：`ldSwitchSetChecked`（当前是宏包装）
- 查询：`ldSwitchIsChecked` / `ldSwitchIsHorizontal` / `ldSwitchIsDisabled`
- 方向：`ldSwitchSetHorizontal`
- 禁用：`ldSwitchSetDisabled`
- 视觉：`ldSwitchSetColor` / `ldSwitchSetImage`

### 2.3 当前交互与动画语义

当前实现的交互路径很简单：

- `SIGNAL_PRESS`：仅设置 `isPressed = true`
- `SIGNAL_RELEASE`：若此前处于按下态，则翻转 checked 并触发动画
- 值变化后，发 `SIGNAL_VALUE_CHANGED`

动画语义也是控件私有线性动画：

- 动画总时长固定 `150ms`
- 每帧推进固定 `10ms`
- `animProgress` 范围为 `0..1000`
- `knob` 位置按 `animProgress` 线性换算

这套结构说明：LingDongGUI 已经具备“独立控件可维护”的基础，不需要回退去复用 `button` 或 `checkbox` 假装成 switch。

---

## 3. LVGL 侧基线是什么

对齐参考主要来自：

- `third_party/lvgl/src/widgets/switch/lv_switch.c`
- `third_party/lvgl/include/lvgl/widgets/lv_switch.h`
- `third_party/lvgl/docs/src/widgets/switch.mdx`
- `third_party/lvgl/tests/src/test_cases/widgets/test_switch.c`
- `third_party/lvgl/examples/widgets/switch/*`

从用户视角，LVGL switch 的关键特征不是 API 名字，而是下面这些体验：

1. 默认方向是 `AUTO`
   - 宽 >= 高时默认横向
   - 宽 < 高时默认纵向

2. 渲染分为三段
   - `MAIN`：底轨
   - `INDICATOR`：打开态高亮区域
   - `KNOB`：滑动圆钮

3. 程序设初值不会出现“首帧自己滑过去”的突兀动画

4. 事件语义稳定
   - 值变化时只触发一次 `LV_EVENT_VALUE_CHANGED`

5. 测试和截图基线完整
   - 默认态
   - 动画
   - ext draw size
   - 事件触发次数
   - 水平/竖直
   - disabled 截图回归

---

## 4. 当前 LingDongGUI 与 LVGL 的差距

## 4.1 应该这轮补的差距

### 4.1.1 缺 `AUTO` 方向

LingDongGUI 现在只有 `isHorizontal` 布尔值，默认永远横向。  
LVGL 默认方向是 `AUTO`，并按宽高自动推导。

这会直接导致一个使用体验差异：

- 在 LVGL，用户只要改 `width/height`，就能自然得到横向或纵向 switch
- 在 LingDongGUI，用户必须额外显式改方向，否则尺寸变化和视觉方向可能不一致

这属于 **必须补的控件能力**，不是抽象层能力。

### 4.1.2 渲染还没有真正的三段语义

当前 `ldSwitch_show()` 的真实语义更接近：

- 先画一整条轨道
- 再画 knob
- checked 与 unchecked 主要靠整条轨道整体变色/换图表达

这和 LVGL 的差别在于：

- LVGL 的 `MAIN` 和 `INDICATOR` 是分开的
- checked 时不是“整个轨道换皮”，而是“底轨仍在，indicator 在里面形成高亮填充区”

这会影响后续所有用户可见效果：

- 轨道边缘保留量
- 开态的视觉层次
- knob 与 indicator 的相对关系
- 图片模式时的切换自然度

所以这轮要补的不是外部 part API，而是 **内部先按 part 语义组织**。

### 4.1.3 程序设值的首帧体验不够像 LVGL

LVGL 在对象尚未稳定渲染前，会避免播出不必要的切换动画。  
LingDongGUI 当前只要发现 `animProgress != target` 就开始播动画。

这会影响两类场景：

- 初始化后立即 `ldSwitchSetChecked(true)`
- 页面切换新建控件时就带初值

结果是：用户刚进页面可能看到 switch 自己滑过去，而不是一开始就稳态呈现。

这类问题不是“功能缺失”，但属于 **用户感知明显不对**，优先级应很高。

### 4.1.4 on/off 轨道切换时机过于生硬

当前实现按 `animProgress >= 500` 作为颜色/图片切换阈值。  
这意味着 knob 正在滑动的过程中，轨道会在中点瞬时切换。

这种观感和 LVGL 不同：

- LVGL 更接近“状态一确立，indicator 语义成立，knob 再滑过去”
- LingDongGUI 当前像“滑到一半再突然变成另一套轨道”

如果不重构内部 `indicator` 语义，这个问题会一直存在。

### 4.1.5 API 真实形态还不够稳

当前 `ldSwitchSetChecked` 依赖宏和外部 `ptScene` 上下文，不是一个纯公开函数。  
这会造成几个实际问题：

- 文档不容易写准确
- 脱离页面模板或局部 helper 使用时不直观
- 与其它普通 setter 的风格不一致

这仍然是 `switch` 自己的接口问题，不属于“上层抽象”，可以在本轮内修正。

### 4.1.6 测试太偏 helper，不够控件级

当前 `examples/sdl/tests/switch/test_ldswitch_internal.c` 主要锁住：

- 横竖几何
- knob offset
- 动画推进

但没有真正锁住这些控件级行为：

- 默认态是否正确
- disabled 是否真的不切换
- 重复设同值是否不发事件
- 程序设初值是否首帧不播动画
- auto orientation 是否按尺寸解析
- 值变化事件是否只发一次
- 水平/竖直/disabled 是否截图稳定

这意味着 switch 现在“内部 helper 不容易坏”，但“整控件行为还没有被钉死”。

## 4.2 这轮不做、只记账的差距

下面这些也是 LVGL 有的，但不该在当前阶段展开：

- 对外暴露完整 `MAIN / INDICATOR / KNOB` 样式接口
- 通用动画引擎
- 通用状态系统
- RTL 适配
- property / theme / 数据绑定体系
- 键盘 / 编码器专用逻辑

这些都应该等 switch / flex / grid 能力补齐后，再考虑抽象。

---

## 5. 本轮建议的验收口径

为了避免 scope 膨胀，本轮建议把 switch 的目标收敛成下面 6 条：

1. 尺寸驱动的默认方向正确：宽 >= 高默认横向，宽 < 高默认纵向
2. 程序设初值时首帧不播动画
3. 内部有真实 `track / indicator / knob` 三段渲染语义
4. checked / unchecked / disabled / pressed 的视觉和交互稳定
5. `ldSwitch` 的设值/方向 API 与文档口径一致
6. 补齐控件级行为测试和最小截图回归

只要这 6 条达成，用户感知就会明显更接近 LVGL switch。

---

## 6. 实施步骤拆解

## A 组：基础能力补足

### A1. 收口状态真值与方向模型

目标：先把 switch 自己的状态模型收紧，避免后面渲染和测试反复返工。

具体动作：

1. 冻结本轮核心状态：
   - `checked`
   - `pressed`
   - `disabled`
   - `orientation`
   - `animProgress`

2. 把方向从“布尔横/竖”升级为“内部三态”：
   - `AUTO`
   - `HORIZONTAL`
   - `VERTICAL`

3. 定义最终方向解析规则：
   - `AUTO`：按 `width >= height` 判断横向，否则纵向
   - `HORIZONTAL`：强制横向
   - `VERTICAL`：强制纵向

4. 明确值变化规则：
   - checked 真值变化才发 `SIGNAL_VALUE_CHANGED`
   - pressed 改变、重复设同值、动画推进都不能重复发事件

5. 明确禁用规则：
   - disabled 屏蔽用户输入
   - disabled 不屏蔽程序设值

6. 明确纵向语义：
   - 底部 = off
   - 顶部 = on

### A2. 补内部三段几何与首帧语义

目标：让内部实现真正拥有 LVGL 风格的 `track / indicator / knob` 语义，但先只在 `switch` 自身内部完成。

具体动作：

1. 保留 `ldSwitchInternal.*` 作为纯计算边界，不把几何逻辑重新塞回 `ldSwitch_show()`

2. 在 helper 层新增/补足：
   - 最终方向解析函数
   - track 区域解析
   - indicator 区域解析
   - knob 区域解析

3. 重构 `ldSwitch_show()`：
   - 先画 `MAIN/track`
   - 再画 `INDICATOR`
   - 最后画 `KNOB`

4. indicator 语义改为“开态填充区域”，不再依赖“整条轨道在 500 阈值硬切”

5. 增加首帧 no-animation 规则：
   - 初始化后第一次稳定渲染前
   - 若程序设值改变 checked
   - 直接落终态，不播放切换动画

6. 保留当前控件私有动画模型：
   - 仍用 `animProgress 0..1000`
   - 仍用线性推进
   - 暂不引入通用动画系统

### A3. 收口 switch 专属 API 与文档契约

目标：让 `switch` 的公开接口与真实使用模型一致。

具体动作：

1. 把 `ldSwitchSetChecked` 从依赖 `ptScene` 的宏，收口成更稳定的公开函数形态

2. 方向接口从“只有 `ldSwitchSetHorizontal(bool)`”升级为更清晰的 switch 专属方向接口
   - 即使暂时保留兼容接口，也要让主文档说明真正推荐接口

3. 更新教程/API 文档，补齐 switch 真实接口

4. 修正文档漂移：
   - `docs/superpowers/specs/2026-05-24-ldswitch-widget-design.md` 不能再写“待实现”
   - `README.md` 的支持说明需要与真实边界同步

## B 组：交互 / 渲染 / 验证补足

### B1. 补交互行为测试

目标：把当前 switch 的行为语义锁死，避免每次改渲染/动画都把交互改坏。

建议至少新增这些测试点：

1. 默认创建后：
   - unchecked
   - enabled
   - 方向为 auto/horizontal 的默认解算结果正确

2. 程序设值：
   - 设 `true` 后 checked 生效
   - 重复设 `true` 不重复发事件
   - 初始设值不触发首帧动画

3. 用户输入：
   - `PRESS` 只进入 pressed 态
   - `RELEASE` 才提交值变化
   - disabled 下 press/release 不切换

4. 事件：
   - 一次有效切换只发一次 `SIGNAL_VALUE_CHANGED`

5. 方向：
   - auto orientation 按尺寸决定横竖
   - 强制 vertical / horizontal 时不被尺寸覆盖

### B2. 补渲染语义和视觉回归

目标：不追求通用样式系统，但把控件自身视觉补到足够接近 LVGL。

具体动作：

1. 颜色模式：
   - 底轨始终是底轨
   - checked 时 indicator 单独出现
   - knob 独立绘制

2. 图片模式：
   - off/on track 不再在中点硬切整条背景
   - 若资源不完整，则沿用当前 fallback 思路

3. pressed 反馈增强：
   - 不只把 knob 色改成 border 色
   - 可以让 knob / indicator / border 的反馈更明确

4. disabled 视觉：
   - 继续保持控件私有灰化/半透明策略
   - 不引入统一 disabled style 系统

### B3. 补最小验证闭环

目标：让 switch 从“能跑”升级成“行为已定型”。

建议补齐 3 层验证：

1. host-side 行为测试
2. 最小截图矩阵
3. demo 真实接入回归

截图矩阵至少覆盖：

- horizontal off
- horizontal on
- vertical off
- vertical on
- disabled off
- disabled on

demo 回归至少覆盖：

- `examples/common/demo/widget/uiWidgetLegacy.c`
- `examples/common/demo/widget/uiWidgetSwipePage01.c`

---

## 7. 实施优先级建议

### 第一优先级

这 4 项应最先做：

1. `AUTO` 方向
2. 首帧 no-animation
3. 三段渲染语义落地
4. 控件级行为测试补齐

### 第二优先级

这 3 项建议第一轮顺带完成：

1. `ldSwitchSetChecked` 接口收口
2. 图片模式的 on/off 切换语义变自然
3. pressed 反馈增强

### 第三优先级

这轮只记账：

1. 通用 style/part 抽象
2. 通用动画系统
3. RTL
4. property/theme/data binding
5. 键盘/编码器专用支持

---

## 8. 建议重点盯住的文件

### LingDongGUI 侧

- `src/gui/ldSwitch.h`
- `src/gui/ldSwitch.c`
- `src/gui/ldSwitchInternal.h`
- `src/gui/ldSwitchInternal.c`
- `examples/sdl/tests/switch/test_ldswitch_internal.c`
- `examples/common/demo/widget/uiWidgetLegacy.c`
- `examples/common/demo/widget/uiWidgetSwipePage01.c`
- `docs/superpowers/specs/2026-05-24-ldswitch-widget-design.md`

### LVGL 侧

- `third_party/lvgl/include/lvgl/widgets/lv_switch.h`
- `third_party/lvgl/src/widgets/switch/lv_switch.c`
- `third_party/lvgl/docs/src/widgets/switch.mdx`
- `third_party/lvgl/tests/src/test_cases/widgets/test_switch.c`
- `third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c`
- `third_party/lvgl/examples/widgets/switch/switch_orientation/lv_example_switch_orientation.c`
- `third_party/lvgl/examples/widgets/switch/lv_example_switch_event.c`

---

## 9. 一句话收口

这轮 switch 的目标，不是“再造一个可切换按钮”，而是：

**先不做上层抽象，直接把 `ldSwitch` 补成一个用户感知上已经接近 LVGL switch 的独立控件。**
