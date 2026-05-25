# 2026-05-25 switch 对齐 LVGL 边角审计

## 1. 审计目的

这份文档只回答一个问题：

- `ldSwitch` 在主干能力已经落地后，距离 LVGL 的 `lv_switch` 还剩哪些边角差距？

本轮不扩 scope 去做通用 style/theme/property 系统，也不把“没证据”直接写成“已对齐”。重点是把当前真相拆成：

- 已证实差距
- 审计空洞
- 下一步优先级

## 2. 总结

当前 `switch` 可以升级为“第一轮主干能力已到位，且行为/几何主路径已经闭环”的判断。

原因不是它退回 stub，而是还存在 3 个明确证据缺口：

1. pressed 视觉还没有 screenshot/像素级证据
2. 动画中间帧还没有 capture matrix
3. checked 态 ring 还没有单独锁图

## 3. 已证实差距

### 3.1 已完成：knob overhang 语义

#### 代码依据

- LingDongGUI：`src/gui/ldSwitchInternal.c`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:159`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:269`
- LVGL 示例：`third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c:19`

#### 当前真相

LingDongGUI 现在已经把 knob 主尺寸改成跟随短轴，并让 knob 在起止位置相对 track 产生外扩：

- 横向 `44x24 pad=2` 时，起点 knob `x == 0`，但相对 `track.x == 2` 仍向左 overhang 2px
- 纵向 `24x44 pad=2` 时，起点 knob `y == 20`，但相对 `track.y == 2` 仍向下 overhang 2px
- 对应 internal test 已锁住这组几何

这已经从“轨道内收缩 knob”前进到更接近 LVGL 常见示例的 overhang 关系。

#### 审计结论

这一项本轮已闭环，不再是当前主差距。

### 3.2 已完成：indicator ring 语义

#### 代码依据

- LingDongGUI：`src/gui/ldSwitchInternal.c`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:195`
- LVGL 示例：`third_party/lvgl/examples/widgets/switch/switch_styling/lv_example_switch_styling.c:19`

#### 当前真相

LingDongGUI 的 indicator 现在已经改到 track 内容区内增长，而不是覆盖整个控件：

- 横向时：从 `x=2 y=2 h=20` 开始增长，checked 终点宽度为 `40`
- 纵向时：内容区宽度为 `20`，从底部内容区向上增长，checked 终点 `y=2 h=40`

这与 LVGL 把 indicator 画在 `content coords` 上的关系更接近，checked 后仍能保留外圈。

#### 审计结论

这一项主几何已闭环，但还缺截图矩阵来提升证据强度。

### 3.3 已完成：通用焦点路由接线

#### 代码依据

- LingDongGUI：`src/gui/ldSwitch.h:99`
- LingDongGUI：`src/gui/ldSwitch.c:482`
- LingDongGUI：`src/gui/ldBase.c`
- LingDongGUI：`examples/sdl/tests/layout/test_layout_window.c`
- LingDongGUI：`examples/sdl/tests/check_switch_focus_routing.py`
- LVGL：`third_party/lvgl/src/widgets/switch/lv_switch.c:131`
- LVGL 文档：`third_party/lvgl/docs/src/widgets/switch.mdx:80`

#### 当前真相

LingDongGUI 现在不仅有 `ldSwitchNavigate()`，还补了 `ldSwitchCanNavigate()` 来参与通用焦点层决策：

- `NAV_ENTER` => toggle
- `NAV_UP / NAV_RIGHT` => on
- `NAV_DOWN / NAV_LEFT` => off

并且 `ldBaseFocusNavigate()` 在当前焦点是 selected switch 时，会先判断这次导航是否真的会改值：

- 会改值：路由给 `ldSwitchNavigate()`
- disabled 或重复同值方向：回退到通用 peer focus 导航

因此它不再只存在于 legacy demo 私有接线里，同时也避免了 disabled / no-op 方向把焦点卡死。

现在这条结论也不再只靠源码 grep：`layout_window_test` 已补运行态断言，直接覆盖

- `off + NAV_RIGHT`：留在 switch 并切到 on
- `on + NAV_RIGHT`：放行给右侧 peer focus
- `disabled + NAV_RIGHT`：放行给右侧 peer focus

#### 审计结论

这一项本轮已闭环，不再是主差距。

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

这不是已证实 bug，但仍是当前最值得补的证据空洞。

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

- 回写总状态文档，把 `switch` 状态从“主路径仍有缺口”更新为“主路径已闭环、证据待补”

### P1

- 补 screenshot matrix：
  - pressed
  - animation mid-frame
  - checked edge ring

### P2

- 若继续提高可信度，补像素级断言或截图比对

### P3

- 若后续主题系统继续演进，再复查这套几何和 style 叠加关系

## 6. 本轮判断

本轮最终判断如下：

- `switch` 仍然可以保留“第一轮主干能力已完成”的结论
- 而且主路径已经不再卡在交互/几何逻辑本身
- 更准确的口径应是：
  - 主干能力已对齐
  - 通用输入路由、indicator ring、knob overhang 已落地
  - 现有截图/测试对视觉主张的覆盖还不够完整
