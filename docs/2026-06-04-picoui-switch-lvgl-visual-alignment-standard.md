# PicoUI switch 对齐 LVGL 视觉验收标准

## 目标

PicoUI `switch` 必须通过真实 LingDongGUI backend 输出，达到 LVGL default switch 的可见效果。验收不以“能显示蓝色开关”作为完成标准，而以真实截图中的画布、几何、颜色、状态和交互视觉共同判断。

## 分层验收

### 1. 宿主画布

- PicoUI SDL runtime 的窗口、capture、root window、background 必须使用同一画布尺寸。
- 当前目标尺寸为 `480x320`。
- 截图中不得出现右侧或底部黑块；黑块说明 root/background 仍按较小画布绘制，不能进入 switch 本体对齐结论。
- 背景必须连续覆盖完整 capture 区域；不能只靠控件局部 crop 证明 UI 已完成。

### 2. Switch 本体几何

- 默认横向尺寸目标为 `48x24`。
- track 是完整圆角胶囊，覆盖整个 switch bbox。
- knob 必须在 track 内部，不能外凸到 track 外。
- 默认内边距目标为 `4px`，即 `48x24` 下 knob 几何为 `16x16`。
- off 状态 knob 位于左侧内嵌位置，约 `(x=4, y=4, w=16, h=16)`。
- on 状态 knob 位于右侧内嵌位置，约 `(x=28, y=4, w=16, h=16)`。
- 中间动画态必须呈现左侧 on 色、右侧 off 色、knob 居中的连续过渡，不允许跳变或错位。
- 纵向 switch 按同一规则旋转：track 覆盖完整 bbox，knob 在内部上下滑动。

### 3. Switch 本体颜色

- off track 目标接近 LVGL default grey：`#E0E0E0`。
- on track 目标接近 LVGL default primary blue：`#2196F3`。
- knob 目标为白色或 RGB565 转换后的近白色。
- 不应出现深色外框、方形外框、黑色边线或额外描边。
- pressed 状态默认不改变 knob 颜色；不能变成黄色高亮。
- disabled 状态可由整体 opacity 变暗，但仍必须保留 off/on track 与 knob 的关系。

### 4. PicoUI backend 边界

- `picoui/demo/*` 只能表达用户意图，不能硬编码 switch 坐标、尺寸或假视觉来掩盖 backend 问题。
- PicoUI switch 必须落到真实 `ldSwitch_t`，不得通过 `backend_app.c` 或 SDL 专用路径画 fake switch。
- PicoUI backend 默认 switch 色值必须与 LingDongGUI 默认 switch 色值保持一致，避免同一控件在直接 LingDongGUI 与 PicoUI 路径下视觉分裂。
- `basic_widgets` 的 `wifi` switch 必须由真实 grid layout 和 `picoui_window_set_padding_group()` 定位；当前 demo 标准位置为 `(x=16, y=24, w=48, h=24)`。
- 若截图中 `wifi` switch 出现在 `(0,0)`，判定为 window padding 未进入真实 grid layout，不能通过修改 demo 固定坐标规避。

## 截图验收方法

### 必须通过的证据

- SDL switch matrix 截图采样：覆盖 horizontal off/on、vertical off/on、disabled off/on、pressed、mid animation。
- PicoUI basic widgets 截图采样：确认 `wifi` switch bbox 是 `(16,24)-(63,47)`，尺寸为 `48x24`，track/knob 颜色和位置符合上述标准。
- root/background 完整性采样：确认 `480x320` 全画布无右侧/底部黑块。
- runtime marker 必须显示 `PICOUI_SMOKE_LAYOUT_USED=0`，说明没有走临时 smoke cursor layout。

### 不能作为完成证据

- 仅单元测试通过。
- 仅 demo 能启动。
- 仅看到一个蓝色胶囊。
- 只看局部 switch crop 而忽略全画布黑边。
- broad visible gate 因页面内容边界红/绿，直接推出 switch 本体是否完成。

## 当前对齐缺口判定

若截图出现右侧或底部黑块，优先判定为宿主/root/background 尺寸不一致，先修画布，再评估 switch 本体。

若画布完整但 switch 与 LVGL reference 仍有差异，再按以下顺序收敛：

1. track bbox 与圆角胶囊。
2. knob 是否内嵌。
3. off/on/pressed/disabled 颜色。
4. 动画中间态。
5. PicoUI 与直接 LingDongGUI 路径是否一致。
