# PicoUI H 线 Demo Catalog

本文整理当前 7 个 visible demo 的发布口径，用于回答三个问题：

- 这个 demo 覆盖哪些控件。
- 这个 demo 证明哪些能力。
- 这个 demo 不证明哪些能力。

本文只描述 demo catalog 和 gate 对应关系，不替代运行指南，也不替代人工窗口验收记录。

## 一、证据层边界

本 catalog 沿用 [picoui/docs/demo_guide.md](../../picoui/docs/demo_guide.md) 的证据层定义，保持同一口径：

1. `runtime smoke`：证明 demo 可 build、可启动、可 capture、可回归。
2. `backend mapping gate`：证明 demo 中声明的真实 backend 映射或布局/事件/theme 映射已进入正式 gate。
3. `automatic visible gate`：证明 demo 在 `SDL_VIDEODRIVER=dummy + PPM readback` 下可显示、可读、可判定。
4. `manual artifact gate`：证明人工 OS 窗口验收通过，必须单独执行并单独记录。

必须保持以下边界：

- demo 可运行，不等于 visible 正确。
- automatic visible gate 通过，不等于人工窗口验收通过。
- manual artifact 属于单独证据层，不能由 runtime smoke、backend mapping 或 automatic visible gate 代替。

## 二、Demo Matrix

### 1. `hello_world`

证明的控件：

- `window`
- `label`
- `button`

证明的能力：

- 最小 `app -> window -> widget` 生命周期可 build、可启动、可 capture。
- `label` / `button` 的基础文本与最小真实 backend 映射路径存在。
- 作为最小发布样本，可用于确认 PicoUI demo 宿主和基础窗口链路可运行。

不证明的能力：

- 不证明复杂布局能力；不覆盖 `flex` 或 `grid` 语义。
- 不证明交互控件事件闭环，例如 `switch`、`checkbox`、`slider`。
- 不证明 theme apply、图片 source、list 行为。
- 不证明人工窗口验收。

对应 gate：

- `runtime smoke`
- `automatic visible gate`

### 2. `basic_widgets`

证明的控件：

- `button`
- `text`
- `checkbox`
- `switch`
- `slider`
- `image`

证明的能力：

- 基础交互控件和展示控件可在同一页面中创建并进入真实 backend 映射路径。
- `checkbox` / `switch` / `slider` 的真实对象映射和 native event 路径已纳入当前发布口径。
- `button` / `text` / `image` 具备第一版发布允许宣称的最小样本。
- `image` 只证明基础 `source/layout/visible` 路径：
  - 可创建真实 `ldImage` 对象。
  - 可绑定调用方提供的 tile 指针。
  - 无 source 时仍可作为真实对象路径进入 visible 观察样本。

不证明的能力：

- 不证明 `image` 资源加载器、占位资源绑定、异步解码或更高层媒体能力。
- 不证明 `image` 的 theme/style apply；这类能力不在当前支持合同内。
- 不证明 `checkbox` / `switch` / `slider` 的所有视觉细节都已做人眼验收。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

### 3. `layout_flex`

证明的控件：

- `window`
- 若干基础子控件，作为 `flex` 容器内样本

证明的能力：

- `flex` 的 `flow / align / gap` 等基础布局语义能从 PicoUI 传到真实 backend。
- 布局结果来自真实 layout 映射，而不是 demo 里手工坐标补丁。
- 可作为第一版发布时 `flex` 布局能力的最小可见样本。

不证明的能力：

- 不证明所有 flex 属性、复杂嵌套或极端布局组合都已闭环。
- 不证明业务级页面组织能力。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

### 4. `layout_grid`

证明的控件：

- `window`
- 若干基础子控件，作为 `grid` 单元样本

证明的能力：

- `grid` 的列定义、行定义、间距和 cell 放置路径已进入真实 backend 语义。
- 固定轨道与基础自动轨道写法具备可见样本。
- 可作为第一版发布时 `grid` 布局能力的最小证明面。

不证明的能力：

- 不证明全部复杂 grid 语义，例如更高阶响应式排布或发布外能力。
- 不证明每个 cell 中控件的完整交互合同。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

### 5. `theme_showcase`

证明的控件：

- `window`
- `button`
- `checkbox`
- `switch`
- `slider`
- `label`
- `text`

证明的能力：

- `theme` 可挂载到 `app`，并对当前支持的真实控件执行 style apply。
- `window/button/checkbox/switch/slider/label/text` 的 theme 应用路径已纳入当前发布口径。
- 可作为“统一主题会影响多控件外观”的自动可见样本。

不证明的能力：

- 不证明 `image` 的 theme/style apply；当前明确不支持，不能由该 demo 外推为支持。
- 不证明任意自定义 part、任意主题组合或所有视觉细节都已人工验收。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

### 6. `settings_panel`

证明的控件：

- `theme`
- `label`
- `switch`
- `slider`
- `button`
- `window`

证明的能力：

- 综合型面板可使用真实 `flex(column)`、主题、标题文本和交互控件组合成页面。
- `title / wifi / brightness / apply` 这类面板级元素都走真实 backend 映射。
- 可作为“真实页面组织方式”样本，证明第一版发布不是只支持孤立控件，还支持基本组合页面。
- `button_props` 风格的声明方式已被该 demo 覆盖到发布样本中。

不证明的能力：

- 不证明更复杂设置页流程、分页、弹窗链路或更大应用结构。
- 不证明所有 props 变体或所有交互状态都已覆盖。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

### 7. `list_basic`

证明的控件：

- `list`
- `label`

证明的能力：

- 该 demo 为 `list` 提供了真实 widget 样本。
- 该 demo 已纳入 `runtime smoke`、`backend mapping gate`、`automatic visible gate`。
- 可作为当前 `list` demo 级接入状态的发布样本。

不证明的能力：

- 不证明更细粒度 item 行为或更高阶交互模式。
- 不证明 manual artifact。

对应 gate：

- `runtime smoke`
- `backend mapping gate`
- `automatic visible gate`

## 三、发布使用方式

面向第一版发布时，这 7 个 demo 的职责是“说明当前支持合同的样本覆盖面”，不是“替代所有测试或人工验收”。因此：

- 要说明某个 demo 可运行，只能引用 `runtime smoke` 语义。
- 要说明某个 demo 在 dummy SDL 下可显示、可读、可判定，只能引用 `automatic visible gate`。
- 要说明人工窗口验收通过，必须引用单独的 `manual artifact gate` 记录，而不是引用本 catalog。

如果后续新增 demo、扩展控件能力或调整 gate 范围，必须同时更新本文和 `picoui/docs/demo_guide.md`，继续保持证据层边界一致。
