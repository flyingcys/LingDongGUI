# 2026-05-25 switch 对齐 LVGL 边角审计

## 1. 审计目的

这份文档只回答一个问题：

- `ldSwitch` 在主干能力已经落地后，距离 LVGL 的 `lv_switch` 还剩哪些边角差距？

本轮不扩 scope 去做通用 style/theme/property 系统，也不把“没证据”直接写成“已对齐”。重点是把当前真相拆成：

- 已证实差距
- 审计空洞
- 下一步优先级

## 2. 总结

当前 `switch` 可以继续维持“第一轮主干能力已到位”的判断，但不能再笼统写成“行为/视觉基本齐”。

原因不是它退回 stub，而是还存在 4 个明确边角项：

1. knob 还没有 LVGL 那种向轨道外侧 overhang 的视觉语义
2. indicator 仍然是整块线性填充，没有保留 LVGL 常见的底轨外圈
3. 没有发现与 LVGL 对齐的键盘 / 导航切换输入路径
4. 视觉回归只锁了静态 off/on/disabled，pressed 与动画中间帧仍是审计空洞

## 3. 已证实差距

### 3.1 knob 没有 LVGL 的外扩语义

#### 代码依据

- LingDongGUI：`src/gui/ldSwitchInternal.c:37`
- LingDongGUI：`src/gui/ldSwitchInternal.c:46`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:159`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:269`
- LVGL 示例：`third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c:19`

#### 当前真相

LingDongGUI 里 `knobPadding` 的语义是把 knob 往轨道内部缩：

- `knobSize = minor - 2 * padding`
- knob 始终被限制在控件主区域内

LVGL 则把 `LV_PART_KNOB` 的 `pad_*` 当成“让 knob 往外长”，并额外参与 ext draw size 计算，因此常见视觉会更接近“浮起来的 knob”。

#### 审计结论

这不是实现崩坏，而是一个明确的视觉语义差距。当前 LingDongGUI 已有 `track / indicator / knob` 三段几何，但还没到 LVGL 常见示例那种 knob overhang 关系。

### 3.2 indicator 没有保留底轨外圈

#### 代码依据

- LingDongGUI：`src/gui/ldSwitchInternal.c:109`
- LingDongGUI：`src/gui/ldSwitchInternal.c:127`
- LingDongGUI：`src/gui/ldSwitchInternal.c:146`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:195`
- LVGL 示例：`third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c:19`

#### 当前真相

LingDongGUI 的 indicator 几何直接按整块宽高线性增长：

- 横向时：从 `x=0` 长到 `indicatorLength`
- 纵向时：从底部长到顶部

LVGL 的 indicator 画在 `lv_obj_get_content_coords()` 上，也就是 `MAIN` 去掉 padding 之后的内容区。常见样式下，checked 后依然能看到一圈底轨。

#### 审计结论

LingDongGUI 现在属于“三段结构已存在，但三段相对关系还没完全像 LVGL”。这也是为什么现有截图矩阵只能证明 off/on/disabled 颜色态成立，不能证明 LVGL 风格的 ring 语义已经成立。

### 3.3 缺少键盘 / 导航切换语义

#### 代码依据

- LingDongGUI：`src/gui/ldSwitch.c:213`
- LingDongGUI：`src/gui/ldSwitch.c:214`
- LingDongGUI：`src/gui/ldBase.h:374`
- LingDongGUI demo 导航：`examples/common/demo/widget/uiWidgetLegacy.c:365`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:131`
- LVGL 文档：`third_party/lvgl/docs/src/widgets/switch.mdx:80`

#### 当前真相

LingDongGUI 的 switch 当前只接了：

- `SIGNAL_PRESS`
- `SIGNAL_RELEASE`

没有看到：

- `NAV_ENTER` / 等价确认键切换
- `UP/RIGHT=ON`
- `DOWN/LEFT=OFF`

LVGL 文档则明确把这些 key 语义列为 switch 能力的一部分。

#### 审计结论

这是一个真实行为差距，不是单纯“测试没写到”。即使 LingDongGUI 全局已有 focus navigate，switch 自身当前也没有实现 LVGL 那条输入模态。

## 4. 审计空洞

### 4.1 pressed 视觉没有截图证据

#### 代码依据

- pressed 颜色入口：`src/gui/ldSwitch.c:319`
- pressed knob 高亮：`src/gui/ldSwitch.c:320`
- widget 级断言：`examples/sdl/tests/switch/test_ldswitch_widget.c:353`

#### 当前真相

现在只通过 unit test 证明：

- pressed 时 knob 不再回退成 `borderColor`
- pressed 时 knob 也不再保持原 `knobColor`

但没有 screenshot/像素级证据证明 pressed 在真实渲染里“足够明显”。

#### 审计结论

这不是已证实 bug，但也是不能继续默认“视觉已基本对齐”的原因之一。

### 4.2 动画中间帧没有视觉回归

#### 代码依据

- 现有截图矩阵：`examples/sdl/tests/check_switch_capture_matrix.py:21`
- 现有截图采样：`examples/sdl/tests/check_switch_capture_matrix.py:155`

#### 当前真相

当前截图矩阵只覆盖：

- horizontal off / on
- vertical off / on
- disabled off / on

没有覆盖：

- pressed
- 动画中间帧
- checked 时边缘 ring 是否仍可见

#### 审计结论

当前视觉回归更像“静态颜色 smoke”，还不是“LVGL 风格细节锁定”。

## 5. 建议的下一步

### P0

- 先回写总状态文档，明确 `switch` 仍有视觉/输入边角差距，避免状态写得过满

### P1

- 若要继续对齐视觉，优先做：
  - indicator ring 语义
  - knob overhang 语义

### P2

- 若要继续对齐交互，补：
  - `ENTER` toggle
  - 方向键显式 on/off

### P3

- 补 screenshot matrix：
  - pressed
  - animation mid-frame
  - checked edge ring

## 6. 本轮判断

本轮最终判断如下：

- `switch` 仍然可以保留“第一轮主干能力已完成”的结论
- 但不应再把它表述成“和 LVGL 已经差不多齐了”
- 更准确的口径应是：
  - 主干能力已对齐
  - 输入模态与视觉细节仍有明确边角差距
  - 现有截图/测试对视觉主张的覆盖还不够完整
