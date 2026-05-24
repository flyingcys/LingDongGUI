# LingDongGUI 原生 Switch 控件设计规格说明

**日期**：2026-05-24  
**状态**：已确认设计，待实现  
**范围类型**：新增控件 spec  
**关联文档**：

- `LingDongGUI_vs_LVGL_技术对比.md`
- `third_party/lvgl/src/widgets/switch/lv_switch.c`
- `src/gui/ldCheckBox.c`
- `src/gui/ldButton.c`
- `src/gui/ldSlider.c`

---

## 1. 文档目标

本文件用于定义 **LingDongGUI 原生 `ldSwitch` 控件** 的第一轮正式实现范围。

本轮目标不是做一个“能切换状态的临时按钮”，而是做一个：

- 视觉效果接近 LVGL `switch`
- 交互语义独立于 `ldButton` / `ldCheckBox`
- 可作为长期维护控件存在于 `src/gui/`
- 能接入现有 SDL widget demo、文档和测试链

本轮设计已经按用户确认收敛到：

> **视觉 + 交互 + 常用能力对齐 LVGL switch**

也就是：

- track + knob 视觉结构
- 开 / 关切换
- knob 滑动动画
- 按下反馈
- 横向 / 纵向方向
- 禁用态
- 程序设值
- `SIGNAL_VALUE_CHANGED` 值变化通知

---

## 2. 本轮目标

### 2.1 用户可见目标

本轮交付后，LingDongGUI 需要具备一个新的原生控件：

- 控件名：`ldSwitch`
- 默认样式：圆角轨道 + 圆形滑钮
- 默认方向：横向
- 默认行为：点击后在开 / 关之间切换，并带滑动动画

用户从 SDL demo 侧看到的效果应满足：

1. 关闭态时 knob 位于轨道起点
2. 打开态时 knob 位于轨道终点
3. 切换过程中 knob 连续滑动，而不是瞬时跳变
4. 按下时有可见反馈
5. 纵向模式时 knob 改为沿纵轴滑动
6. 禁用态不可响应用户输入，并且视觉上明显弱化

### 2.2 编程接口目标

本轮要提供最小但完整的公开 API：

- 初始化控件
- 读 / 写 checked 状态
- 设置方向
- 设置禁用态
- 设置颜色模式
- 设置图片模式

推荐公开接口如下：

```c
ldSwitch_t *ldSwitch_init(ld_scene_t *ptScene,
                          ldSwitch_t *ptWidget,
                          uint16_t nameId,
                          uint16_t parentNameId,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height);

void ldSwitchSetChecked(ldSwitch_t *ptWidget, bool isChecked);
bool ldSwitchIsChecked(ldSwitch_t *ptWidget);

void ldSwitchSetHorizontal(ldSwitch_t *ptWidget, bool isHorizontal);
bool ldSwitchIsHorizontal(ldSwitch_t *ptWidget);

void ldSwitchSetDisabled(ldSwitch_t *ptWidget, bool isDisabled);
bool ldSwitchIsDisabled(ldSwitch_t *ptWidget);

void ldSwitchSetColor(ldSwitch_t *ptWidget,
                      ldColor offTrackColor,
                      ldColor onTrackColor,
                      ldColor knobColor,
                      ldColor borderColor);

void ldSwitchSetImage(ldSwitch_t *ptWidget,
                      arm_2d_tile_t *ptOffImgTile,
                      arm_2d_tile_t *ptOffMaskTile,
                      arm_2d_tile_t *ptOnImgTile,
                      arm_2d_tile_t *ptOnMaskTile,
                      arm_2d_tile_t *ptKnobImgTile,
                      arm_2d_tile_t *ptKnobMaskTile);
```

说明：

- 本轮不暴露完整样式系统
- 本轮不做类似 LVGL `MAIN / INDICATOR / KNOB` 的外部 part 级 API
- 但内部实现必须按这三层语义组织，方便后续扩展

---

## 3. 非目标

下面这些内容在本轮明确不做：

- 通用动画引擎抽象
- 通用三段样式系统（main / indicator / knob API）
- RTL 行为对齐
- 焦点组 / 键盘 / 编码器专用切换逻辑
- 拖拽 knob 的手势细节还原
- 主题系统接入
- 资源编辑器 / 可视化配置器接入
- 所有旧 demo 页面同步加入 switch
- 把 `ldCheckBox` / `ldButton` 重构成 switch 基类

本轮的原则是：

> **先把 `ldSwitch` 作为独立控件做对，再考虑把通用能力向外抽。**

---

## 4. 对齐基线：以 LVGL switch 为行为参考，而不是 API 逐字复制

本轮参考对象是：

- `third_party/lvgl/src/widgets/switch/lv_switch.c`

需要明确：

- **对齐的是视觉和行为特征**
- **不是照搬 LVGL 内部架构**

LingDongGUI 当前没有：

- `LV_STATE_CHECKED` 这种统一对象状态系统
- `LV_PART_INDICATOR` / `LV_PART_KNOB` 这种通用样式分部
- `lv_anim_t` 这种通用属性动画引擎

因此本轮采用的策略是：

1. 复用 LingDongGUI 现有消息系统和 dirty region 刷新链
2. 在 `ldSwitch` 内部自己维护 checked / disabled / pressed / animProgress
3. 在 `ldSwitch_show()` 中自己完成 track / knob 的几何计算与绘制
4. 在 `ldSwitch_on_frame_start()` 中推进动画

也就是说：

- **外部效果尽量像 LVGL**
- **内部实现遵循 LingDongGUI 现有控件范式**

---

## 5. 控件数据模型

### 5.1 结构体设计

推荐的 `ldSwitch_t` 至少包含以下字段：

```c
typedef struct ldSwitch_t ldSwitch_t;

struct ldSwitch_t {
    implement(ldBase_t);

    ldColor offTrackColor;
    ldColor onTrackColor;
    ldColor knobColor;
    ldColor borderColor;

    arm_2d_tile_t *ptOffImgTile;
    arm_2d_tile_t *ptOffMaskTile;
    arm_2d_tile_t *ptOnImgTile;
    arm_2d_tile_t *ptOnMaskTile;
    arm_2d_tile_t *ptKnobImgTile;
    arm_2d_tile_t *ptKnobMaskTile;

    uint16_t animProgress;      // 0..1000
    uint16_t animStartProgress; // 0..1000
    uint16_t animTargetProgress;// 0..1000
    uint16_t animElapsedMs;
    uint16_t knobPadding;

    bool isChecked : 1;
    bool isHorizontal : 1;
    bool isDisabled : 1;
    bool isPressed : 1;
    bool isAnimating : 1;
    bool useImageStyle : 1;
};
```

### 5.2 设计原则

- `isChecked` 是唯一业务真值
- `animProgress` 只描述当前过渡位置，不是额外业务状态
- `isDisabled` 只屏蔽用户输入，不屏蔽程序设值
- `useImageStyle` 用于在颜色绘制和图片绘制之间切换

---

## 6. 状态与事件模型

### 6.1 事件来源

本轮只要求接入现有触摸 / 点击消息：

- `SIGNAL_PRESS`
- `SIGNAL_RELEASE`

### 6.2 状态流转

#### 用户点击切换

当控件未禁用时：

1. 收到 `SIGNAL_PRESS`
2. 设置 `isPressed = true`
3. 标记 dirty
4. 收到 `SIGNAL_RELEASE` 后
5. 清掉 `isPressed`
6. 翻转 `isChecked`
7. 初始化动画目标
8. 发出 `SIGNAL_VALUE_CHANGED`
9. 标记 dirty

说明：

- 本轮不在 `PRESS` 时立即翻转状态
- 统一在 `RELEASE` 时提交切换，避免拖动和误触时视觉不稳定

#### 程序设值

调用 `ldSwitchSetChecked()` 时：

- 如果值没有变化：不发消息，不重启动画
- 如果值发生变化：更新 `isChecked`，推进动画，并发出 `SIGNAL_VALUE_CHANGED`

### 6.3 禁用态规则

当 `isDisabled == true` 时：

- 忽略用户 `PRESS` / `RELEASE`
- 不进入 pressed 态
- 不改变 checked 值
- 保持可被程序设值

### 6.4 值变化通知规则

本轮统一使用：

- `SIGNAL_VALUE_CHANGED`

约定：

- `value = 0` 表示关闭
- `value = 1` 表示打开

不新增新的消息常量。

---

## 7. 渲染设计

### 7.1 内部三段语义

虽然本轮不对外暴露 part API，但内部必须明确以下三段：

1. `track`：轨道整体边界
2. `indicator`：当前开 / 关背景表现
3. `knob`：滑钮

### 7.2 默认颜色模式

颜色模式下建议：

- 关闭态轨道：浅灰 / 冷灰
- 打开态轨道：绿色系或蓝绿色系
- knob：白色或浅色实心圆
- border：比轨道略深一层

绘制要求：

- track 用圆角矩形
- knob 用圆形或强圆角矩形
- 关闭态也必须保留完整轨道，不允许只画 knob

### 7.3 图片模式

图片模式下：

- off/on 两种轨道图独立
- knob 图单独存在
- 如果图片模式资源不完整，必须回退颜色模式，而不是崩溃或空白

### 7.4 几何计算规则

#### 横向

- knob 直径由控件高度和 `knobPadding` 推导
- `animProgress = 0` 时 knob 在左端
- `animProgress = 1000` 时 knob 在右端

#### 纵向

- knob 直径由控件宽度和 `knobPadding` 推导
- `animProgress = 0` 时 knob 在下端或起点端
- `animProgress = 1000` 时 knob 在上端或终点端

本轮必须在 spec 中明确：

> **纵向模式使用“底部关闭，顶部打开”的视觉语义。**

这样能避免实现时再次讨论方向歧义。

### 7.5 按下反馈

`isPressed` 为真时：

- knob 或 track 需要有轻度高亮 / 压暗 / 扩边中的一种
- 本轮不要求做复杂弹性反馈
- 目标是让用户能明显看出“按下了这个 switch”

### 7.6 禁用态视觉

禁用态下至少满足其一：

- 降低整体 opacity
- 调低饱和度 / 亮度
- knob 与 track 都做灰化

要求：

- 用户一眼可区分“开关状态”和“是否可交互”
- 禁止只靠事件层禁用而无视觉反馈

---

## 8. 动画设计

### 8.1 本轮动画目标

本轮只做一个动画：

- knob 在轨道上的开 / 关切换滑动动画

### 8.2 实现策略

由于仓库当前没有通用动画引擎，本轮动画放在 `ldSwitch` 私有实现中：

- 在 `ldSwitch_on_frame_start()` 推进 `animElapsedMs`
- 把 `animElapsedMs` 映射成 `animProgress`
- 到达目标后清掉 `isAnimating`

### 8.3 动画时间

本轮固定使用控件内部默认动画时长：

- `150ms`

本轮不提供公开 API 让用户自定义动画时长。

### 8.4 无动画回退条件

以下场景允许直接跳到终态：

- 控件首次初始化，尚未进入稳定渲染帧
- 动画起点和终点相同
- 控件尺寸非法，无法计算 knob 行程

---

## 9. 代码组织决策

### 9.1 新增文件

本轮新增：

- `src/gui/ldSwitch.h`
- `src/gui/ldSwitch.c`

建议同时新增一个内部辅助边界：

- `src/gui/ldSwitchInternal.h`
- `src/gui/ldSwitchInternal.c`

内部辅助只承载：

- knob 几何计算
- 动画进度推进
- 方向相关坐标解析

这样做的原因：

- 把可单测逻辑从渲染 / 场景依赖中拆出来
- 避免 `ldSwitch.c` 很快演变成一个几百行混合文件

### 9.2 现有文件修改范围

本轮预期修改的现有文件包括：

- `src/gui/ldBase.h`：新增 `widgetTypeSwitch`
- `src/gui/ldGui.h`：注册 `ldSwitch.h`
- `examples/common/demo/widget/uiWidgetPage2.c`：加入 switch 示例
- `README.md`
- `README.en.md`
- `docs/tutorial/04 api.md`
- `LingDongGUI_vs_LVGL_技术对比.md`

### 9.3 不修改的边界

本轮不做：

- `ldButton` 语义重构
- `ldCheckBox` 语义重构
- `ldSlider` 语义扩展

---

## 10. Demo 与文档更新策略

### 10.1 SDL widget demo

本轮把 switch 示例接入 `USE_DEMO == 2` 的 widget demo。

推荐放在 `uiWidgetPage2.c`，原因：

- 该页已经承载 progress、text、slider 等输入/状态控件
- switch 放在这一页更符合用户认知
- 不需要额外新增新的 demo id

建议至少展示三种状态：

1. 横向普通 switch
2. 纵向 switch
3. 禁用态 switch

### 10.2 README / API 文档

本轮需要同步：

- `README.md` 控件列表新增 switch
- `README.en.md` 控件列表新增 switch
- `docs/tutorial/04 api.md` 新增 `ldSwitch` 章节

### 10.3 对比文档

`LingDongGUI_vs_LVGL_技术对比.md` 需要更新口径：

- 从“LingDongGUI 当前无原生 switch”
- 改成“已新增原生 switch，但样式分部系统和通用动画引擎仍不如 LVGL 完整”

---

## 11. 测试与验收

### 11.1 单元 / 宿主测试

本轮至少要有一组 host-side 断言测试，覆盖：

- 横向 knob 位置计算
- 纵向 knob 位置计算
- 动画从 0 -> 1000 的推进
- 程序设值时的状态切换
- 禁用态屏蔽用户交互

### 11.2 SDL smoke 验收

本轮至少做以下验证：

1. `USE_DEMO=2` 构建成功
2. SDL demo 能打开并进入 widget 页面
3. switch 在页面上可见
4. 点击能开 / 关切换
5. 纵向 switch 运动方向正确
6. 禁用态不响应点击

### 11.3 文档验收

以下文档必须同步：

- README 中出现 switch
- API 文档出现 `ldSwitch` 章节
- 对比文档的 switch 结论更新

---

## 12. 风险与约束

### 12.1 缺少通用 disabled 基类语义

当前 `ldBase_t` 没有统一 disabled 字段。

因此本轮 `isDisabled` 只能先放在 `ldSwitch_t` 私有字段中。

这意味着：

- 本轮能把 switch 做对
- 但 disabled 仍不是全控件通用能力

这是可接受的范围内技术债，不在本轮展开。

### 12.2 缺少通用动画框架

本轮动画如果直接写在 `ldSwitch.c`，后续其他控件若复制这套逻辑，可能出现重复代码。

本轮接受这个折中，但要求：

- 把纯计算逻辑拆到 `ldSwitchInternal.c`
- 不把动画推进逻辑散落到 demo 或业务层

### 12.3 图片模式资源质量风险

如果后续想把 switch 做得更像 LVGL 展示图，可能会希望加更精细的资源图。

本轮不把资源美术当交付前置条件：

- 默认颜色模式先达标
- 图片模式作为增强能力接上接口

---

## 13. 最终验收标准

只有同时满足下面条件，本轮 `ldSwitch` 才算完成：

1. 存在独立的 `ldSwitch.h/.c`，不是把 switch 塞进旧控件里
2. `ldBase.h` 出现独立 `widgetTypeSwitch`
3. `ldGui.h` 已注册 switch 头文件
4. switch 支持横向 / 纵向
5. switch 支持禁用态
6. switch 支持程序设值和取值
7. switch 值变化通过 `SIGNAL_VALUE_CHANGED` 发出
8. knob 切换带动画，不是瞬时跳变
9. SDL widget demo 可见且可操作
10. README、API 文档、技术对比文档都同步更新

---

## 14. 推荐实施顺序

1. 先拆 `ldSwitchInternal`，把几何和动画推进做成可测纯逻辑
2. 再做 `ldSwitch.h/.c` 公共控件骨架和消息接线
3. 再做颜色模式渲染 + 动画推进
4. 再补图片模式
5. 最后接 SDL demo 和文档

这样可以保证：

- 先把状态 / 动画 / 坐标真值锁住
- 再把视觉和集成层叠上去
- 减少“看起来有了，但逻辑不稳”的返工
