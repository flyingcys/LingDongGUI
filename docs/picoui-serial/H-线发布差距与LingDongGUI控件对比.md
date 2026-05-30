# PicoUI H线发布差距与 LingDongGUI 控件对比

## 目的

本文是 H 线的专门对比文档，用于回答：

- PicoUI 离第一版本发布还有多少距离。
- PicoUI 在控件数量上与 LingDongGUI 的差距。
- PicoUI 在每个已覆盖控件的能力上与 LingDongGUI 的差距。
- 哪些缺口是第一版 blocker，哪些可以作为 known limitations。

本文不使用 G 线文件名，因为它不是 G 线剩余缺口继续实现文档。G 线已经完成当前控件 capability gap 收口；H 线面向发布判断。

当前结论：**还没有到发布第一版本的时候**。本文不是 release closeout，而是发布前串行任务的差距基线。

从 H2 起，`tests/picoui/contract/picoui_release_capability_matrix.json` 是 release matrix 的机器可读真相源；本文负责解释口径、差距和发布判断，不再承担机器解析职责。该 matrix 只记录自动可维护事实，不负责表达“人工是否已验收通过”。

## 统计口径

### 计入 LingDongGUI 可封装控件

计入标准：

- `src/gui/ld*.h` / `src/gui/ld*.c` 中有独立 widget struct。
- 有 `ldXxx_init()` / `ldXxxInit()` 或等价初始化入口。
- 有可被 PicoUI 抽象成 public widget 的用户语义。

按此口径，LingDongGUI 可封装原生控件为 `26` 个。

### 不计入可封装控件的基础/辅助

以下文件族不计入控件数量：

- `ldBase`
- `ldGui`
- `ldWindowLayoutInternal`
- `ldMem`
- `ldScene0`
- `ldScene1`
- `ldAnimation`
- `ldSwitchInternal`

说明：

- `ldBase` 是通用基类和状态/布局/事件底座。
- `ldGui` 是全局渲染与消息循环入口。
- `ldWindowLayoutInternal` 是布局内部实现。
- `ldScene0/1` 是示例/场景层。
- `ldAnimation` 有 widget 生命周期，但更接近动画辅助，不作为 H 线第一版控件覆盖目标。
- `ldSwitchInternal` 是 `ldSwitch` 内部实现。

### PicoUI 覆盖口径

PicoUI “已覆盖”不等于“镜像 LingDongGUI 全能力”。本文把覆盖拆成三层：

1. `控件数量覆盖`：是否存在对应 PicoUI public 控件。
2. `真实 backend 覆盖`：是否落到真实 `ld*` 对象或真实 `ldBase` 能力。
3. `能力合同覆盖`：某个具体能力是否有 public API、真实 backend 行为和证据层。

### release matrix schema 口径

H2 新增的机器可读 release matrix 使用以下固定字段：

- 控件状态：`wrapped / not_wrapped / deferred`
- 能力状态：`support / reject / incomplete_contract / deferred`
- 发布判断：`internal_v0_1_blocker / internal_v0_1_known_limitation / post_h_candidate`
- JSON 路径：
  - `status_enums.widget_status`
  - `status_enums.capability_status`
  - `status_enums.release_judgement`
  - `evidence_enums`
  - `widgets[].widget_release_judgement`
  - `widgets[].capabilities[].capability_release_judgement`
- manual artifact：`artifact_entry_exists` 与 `manual_review_required` 分开记录，且 scope 固定为 `widget_level_only`

边界约束：

- release matrix 要表达的是“当前机器可维护的发布事实”，不是“人工验收已经通过”的布尔 gate。
- `widget_release_judgement` 按控件级发布桶理解，不等于该控件每一条 capability 都是同一状态；当前 `internal v0.1` 讨论范围内的 wrapped 控件，若不属于 blocker，就归到 `internal_v0_1_known_limitation`，表示“在当前发布面内，但仍需配套 known limitations 文案”。
- `capability_release_judgement` 只用于当前 H2 聚合 capability 条目对应的发布判断；它和 widget 级字段共享同一套枚举值，但粒度不同。当前属于 `support` 的 capability 也统一归到 `internal_v0_1_known_limitation` 这一发布桶，表示“属于当前 internal v0.1 发布面，需要随 release notes / known limitations 一起发布”，不是说该 capability 本身是 limitation。
- `evidence_enums` 先定义 `unit / contract / mapping / visible / manual_artifact` 五层；当前 H2 仅要求 wrapped 控件提供最小 `evidence_layers`，不代表 44 条 capability 已逐项绑到证据层。
- 当前 capability 名称仍保留聚合粒度，这是 H2 最小初稿的有意选择；若 H3/H4 需要更细 gate，可再继续拆分，不影响当前真相源口径。
- release matrix 的 `manual_artifact` 只表达 widget-level 粒度；demo-level artifact entry 是否存在，统一以 `docs/picoui-serial/C-线人工窗口验收记录.md` 为真相源。
- `artifact_entry_exists = true` 只代表已经登记对应粒度的 artifact 条目，不代表人工 reviewer 已确认通过。
- `manual_review_required = true` 代表该项仍需人工复核；即使已有自动 visible/mapping 证据，也不能把它自动折叠成人工验收结论。

## 总体距离

| 指标 | 当前数量 | 说明 |
| --- | ---: | --- |
| LingDongGUI 可封装原生控件 | 26 | H 线统计口径 |
| PicoUI public 控件 | 9 | `window/label/button/checkbox/switch/slider/text/image/list` |
| 未覆盖原生控件 | 17 | 见后文分组 |
| 控件数量覆盖率 | 34.6% | `9 / 26` |
| PicoUI 当前能力矩阵行 | 44 | 当前 9 控件合同项 |
| 能力项 `support` | 35 | 有 public API / backend / 证据共同支撑 |
| 能力项 `reject` | 4 | 明确不支持 |
| 能力项 `incomplete_contract` | 4 | 有入口或存储，但合同/真实语义不完整 |
| 能力项 `deferred` | 1 | 方向合理，但当前不承诺 |

### 距离判断

如果第一版本是 `internal v0.1`：

- 当前仍不建议马上发布。
- 主要缺口不是某个单点功能，而是发布口径、对比文档、release matrix gate、known limitations、人工验收边界和发布文档包尚未完整闭环。
- 当前 9 个控件可作为发布前基础集合，但还不能直接作为第一版发布集合。

如果第一版本是 `public v1.0`：

- 距离较远。
- 当前控件数量覆盖只有 `34.6%`。
- 输入类、数据展示类、进度仪表类、复合控件类缺口明显。
- 当前 style/theme 只能表达颜色/metric 子集，不足以表达 LingDongGUI 大量 image/mask/transparent skin 能力。
- 至少应先完成 `progress_bar / line_edit / combo_box` 三个高价值控件，再讨论对外第一版。
- 但这已经超出当前 `H线` 基于现有 `9` 控件的发布准备范围，应在 `H线` 收口后另开新线。

## LingDongGUI 原生控件全集

| 分组 | 原生控件 | PicoUI 状态 | 第一版建议 |
| --- | --- | --- | --- |
| 容器 / 基础显示 | `ldWindow` | 已覆盖为 `window` | 第一版保留 |
| 文本显示 | `ldLabel` | 已覆盖为 `label` | 第一版保留 |
| 文本显示 | `ldText` | 已覆盖为 `text` | 第一版保留，但说明 font 是最小映射/fallback |
| 基础交互 | `ldButton` | 已覆盖为 `button` | 第一版保留 |
| 基础交互 | `ldCheckBox` | 已覆盖为 `checkbox` | 第一版保留 |
| 基础交互 | `ldSwitch` | 已覆盖为 `switch` | 第一版保留 |
| 基础交互 | `ldSlider` | 已覆盖为 `slider` | 第一版保留 |
| 图片 | `ldImage` | 已覆盖为 `image` | 第一版保留，但限制 style/enabled/theme/padding |
| 列表 | `ldList` | 已覆盖为 `list` | 第一版保留，但限制 item marker/style/user_data |
| 输入 | `ldLineEdit` | 未覆盖 | `public v1.0` 高优先级 |
| 输入 | `ldKeyboard` | 未覆盖 | 依赖焦点/输入合同，后置于 line_edit contract |
| 输入/选择 | `ldComboBox` | 未覆盖 | `public v1.0` 高优先级 |
| 输入/选择 | `ldScrollSelecter` | 未覆盖 | 第二批 |
| 进度 | `ldProgressBar` | 未覆盖 | `public v1.0` 高优先级 |
| 进度 | `ldProgressWheel` | 未覆盖 | 第二批 |
| 仪表 | `ldArc` | 未覆盖 | 第二批或仪表线 |
| 仪表 | `ldGauge` | 未覆盖 | 第二批或仪表线 |
| 数据展示 | `ldGraph` | 未覆盖 | 后置，数据模型复杂 |
| 数据展示 | `ldTable` | 未覆盖 | 后置，单元格/编辑/键盘合同复杂 |
| 日期时间 | `ldCalendar` | 未覆盖 | 后置 |
| 日期时间 | `ldDateTime` | 未覆盖 | 第二批或轻量显示线 |
| 日期时间 | `ldClock` | 未覆盖 | 后置 |
| 二维码 | `ldQRCode` | 未覆盖 | 可独立低耦合，但不是第一优先级 |
| 复合菜单 | `ldIconSlider` | 未覆盖 | 后置 |
| 复合菜单 | `ldRadialMenu` | 未覆盖 | 后置 |
| 对话框 | `ldMessageBox` | 未覆盖 | 第二批；依赖 modal / callback 口径 |

上表对应到 release matrix 时：

- 当前 `9` 个 PicoUI 控件全部标为 `wrapped`。
- 其余 `17` 个 LingDongGUI 可封装控件在 H2 初稿里全部标为 `not_wrapped`。
- `17` 个未覆盖控件在 H2 初稿里全部标为 `not_wrapped`。
- `line_edit` / `combo_box` / `progress_bar` 与其余未覆盖控件一样，当前统一记为 `post_h_candidate`：它们是 `H线` closeout 之后的新线候选，不并入当前 H 线，也不表达“已经支持”。

## 已覆盖控件差距摘要

逐控件 release contract 已收口到 `docs/picoui-serial/H-线当前9控件发布合同.md`。本文只保留 H1/H4 需要的摘要级差距基线，避免对比文档与合同文档双写完整合同。

### 基础可发布子集

以下 `7` 个控件可作为当前 `internal v0.1` 的基础发布面，但都只承诺 PicoUI 子集，不承诺完整镜像对应 `ld*` 控件：

- `window`：容器、flex/grid、padding/gap/align、背景色；不承诺 background image 与更底层布局选项。
- `label`：文本、最小 font 映射、背景色、文字色；不承诺 transparent、align、background image、完整 getters。
- `text`：文本、最小 font fallback、背景色、文字色；不承诺完整字体系统、scroll seek/move、background image、transparent。
- `button`：文本、clicked/pressed/released、基础样式子集；不承诺 press/release image、checkable、完整 button skin。
- `checkbox`：文本、checked state、toggle callback、基础样式子集；不承诺 image mode、radio group、string left space。
- `switch`：checked state、toggle callback、enabled、基础样式子集；不承诺方向、导航、image mode、完整 track/knob skin。
- `slider`：value、range、value changed callback、基础样式子集；不承诺 horizontal、image skin、indicator width/slim size。

### 高风险已覆盖控件

`image` 与 `list` 虽已覆盖，但它们的 first-release 风险显著高于其余 `7` 个控件，必须按显式限制发布，不能弱化成“部分支持”。

#### `image` 摘要差距

- 当前发布面只承认基础 source 绑定与 `ldBase` 布局/可见性子集。
- 明确限制：
  - `theme`: `reject`
  - `style_class / user_data`: `incomplete_contract`
  - `bg_color / text_color / border_color / radius`: `reject`
  - `padding`: `deferred`
  - `enabled`: `reject`
- 仍不承诺完整资源加载系统与 mask color。
- 在 H2 release matrix 初稿中，上述非 `support` 项统一记为 `internal_v0_1_known_limitation`，不得误写成 `support`，也不得把 manual artifact 写成自动通过。

#### `list` 摘要差距

- 当前发布面只承认基础 add item、selected index、真实 `ldList` 文本/选择映射与 callback cookie。
- 明确限制：
  - `item marker`: `reject`
  - `style_class`: `incomplete_contract`
  - `widget-level user_data`: `incomplete_contract`
- 必须明确：
  - item id / marker 不是独立 backend widget support
  - `on_selected(..., user_data)` callback cookie 与 widget-level `user_data` 分开记录
- 仍不承诺 item child widget、item height、text align、padding、margin。
- 在 H2 release matrix 初稿中，list 的非 `support` 项统一记为 `internal_v0_1_known_limitation`。

### 合同文档入口

若要看 `9` 个控件的 `supported APIs / backed by ld* entries / evidence layers / known limitations / first-release wording`，统一以 `docs/picoui-serial/H-线当前9控件发布合同.md` 为准。

## H线外后续候选控件分组

本节只用于说明 `H线` 收口后，若要继续朝更强的对外版本推进，哪些未覆盖控件更值得优先考虑。它不是当前 `H线` 的串行开发清单。

### 第一优先级：发布价值高、实现风险可控

#### `ldProgressBar`

建议 PicoUI 控件名：`progress_bar`

原因：

- 进度反馈是通用 UI 第一版常见能力。
- 原生能力相对清晰：percent、horizontal/inverted、颜色、图片。
- 可以先定义最小合同：percent + direction + color，不做 image skin。

#### `ldLineEdit`

建议 PicoUI 控件名：`line_edit`

原因：

- 表单输入是第一版对外可用性的关键能力。
- 但必须先定义 keyboard binding、focus、edit type、text changed event。

#### `ldComboBox`

建议 PicoUI 控件名：`combo_box`

原因：

- 选择输入比 table/graph 更常见。
- 可先做 static items + selected index + on_selected。
- dropdown image / hold behavior 可后置。

### 第二优先级：常用但依赖额外合同

- `ldScrollSelecter`
- `ldMessageBox`
- `ldDateTime`
- `ldQRCode`
- `ldProgressWheel`
- `ldArc`

### 后置：数据模型或复合交互复杂

- `ldTable`
- `ldGraph`
- `ldKeyboard`
- `ldIconSlider`
- `ldRadialMenu`
- `ldGauge`
- `ldCalendar`
- `ldClock`

这些控件不是不重要，而是第一版直接做会引入较多未定义合同：

- 数据模型。
- per-cell / per-item 子对象。
- focus / navigation。
- keyboard / edit mode。
- image skin / mask。
- animation / timer。

## 能力类型差距

### 数量差距

PicoUI 当前只覆盖 9 个控件；未覆盖 17 个控件。若目标是“LingDongGUI 主流控件覆盖”，当前还缺明显。

### layout 差距

PicoUI 已经建立 flex/grid 上层 API，并真实落到 `ldWindow` / `ldBase` 相关入口。layout 是当前较强部分。

仍需注意：

- PicoUI layout 是上层稳定合同。
- 不应承诺所有 `ldWindow` 内部 layout API 全部暴露。

### event 差距

当前 PicoUI 已覆盖：

- button clicked / pressed / released
- checkbox toggled
- switch toggled
- slider value_changed
- list on_selected

但 LingDongGUI 原生有更广泛的 native signals、弱回调、navigation、hold/release、clicked item 等模式。第一版如要做输入类控件，必须先建立统一 event/focus 合同。

### style/theme 差距

PicoUI 当前 theme 偏：

- color token
- metric token
- part/state subset

LingDongGUI 原生控件普遍有：

- 多组 `ldColor`
- background / foreground image
- mask tile
- transparent
- per item / per cell style
- pointer / indicator / knob / frame 细节

第一版不能宣称 PicoUI theme 已覆盖 LingDongGUI skin 系统。

### image/resource 差距

PicoUI 当前 image source 是 tile/source 绑定边界，不是完整资源加载系统。

缺口：

- 动态资源加载。
- mask color。
- image skin。
- ownership / lifetime 的更完整 public 约束。

### getter/readback 差距

LingDongGUI 很多控件提供 getters：

- text
- color
- align
- selected item
- date
- table cell
- button state

PicoUI 当前 getter 较少，主要集中在 value/checked/selected index。第一版可以不补全 getter，但要承认 readback 能力是子集。

### manual artifact 差距

当前 automatic visible gate 不是 manual acceptance。

第一版若对外发布，应补：

- 人工窗口验收记录。
- 当前最小 manual artifact 范围内每个 demo 的 artifact 路径和观察结论。
- 明确 artifact existence 不等于验收通过。

当前 `H8` 与现实对齐后的最小范围固定为两个 demo：

- `picoui_basic_widgets_demo`
- `picoui_settings_panel_demo`

说明：

- 这是因为当前 `tests/picoui/runtime/check_picoui_manual_window_artifact.py` 只支持这两个 demo。
- `docs/picoui-serial/C-线人工窗口验收记录.md` 是当前唯一人工记录真相源。
- 这两个 demo 的 artifact existence 只能表达“条目已存在、仍需人工复核”，不能表达“人工已验收通过”。
- 若后续要扩张到 `hello_world / layout_flex / layout_grid / theme_showcase / list_basic` 等更多 demo，只能作为 `post-H candidate`，不并入当前 `H8` 完成条件。

## 若要宣称“完整支持当前控件”，以下是 blocker

前提：这里讨论的是更强表述，即对外宣称 “PicoUI 已完整支持当前已覆盖控件” 或接近 “完整镜像对应 LingDongGUI 控件”。在这个前提下，以下问题不处理就不能这样宣称：

1. `image` 的 style/enabled/theme/padding 仍不是 support。
2. `list` 的 item marker/style_class/widget user_data 仍不是 support。
3. G 线矩阵还没有独立 release gate 接管。
4. manual artifact 仍不是人工验收通过。
5. 未覆盖 17 个原生控件，不能宣传成 LingDongGUI 全控件覆盖。
6. style/theme 不是 LingDongGUI image/mask skin 系统。
7. font 仅最小内置映射/fallback，不是完整字体解析系统。
8. 尚未完成 release matrix gate。
9. 尚未完成 demo catalog 与 release notes。
10. 尚未完成独立发布 review。

## 若目标是保守的 `internal v0.1`，以下可作为 known limitations / non-blocker

前提：这里讨论的是更保守的 first-release 口径，即 “当前 `9` 控件可内部试用，但只承诺明确子集并附带 known limitations”。在这个前提下，以下事项可以不阻塞 `internal v0.1`，但前提仍是 H 线 release matrix、known limitations 和发布文档包已经完成：

1. 未覆盖 17 个原生控件。
2. image style/theme/enabled/padding。
3. list per-item widget / marker / advanced style。
4. button / checkbox / switch / slider 的 image skin。
5. label/text transparent / background image。
6. getter/readback 不完整。
7. manual artifact 未全部人工验收。

条件是发布说明必须明确写出这些限制。

H2 release matrix 初稿把上述 known limitations 机器可读化后，文档与 JSON 必须保持同口径：

- `image` / `list` 的非 `support` 项写为 `internal_v0_1_known_limitation`。
- `17` 个未覆盖控件写为 `not_wrapped`；`line_edit` / `combo_box` / `progress_bar` 当前与其余未覆盖控件一样记为 `post_h_candidate`，表示它们属于 `H线` 之后的新线候选，而不是当前 H 线 blocker。
- manual artifact 只表达条目是否存在、是否仍需人工复核，不能被文档或后续 gate 解释成“人工已验收通过”。
- 当前 `H8` 最小 manual artifact 范围只有 `picoui_basic_widgets_demo` 与 `picoui_settings_panel_demo`；任何 7-demo 扩张都属于 `post-H candidate`，不是当前内部 `v0.1` 的完成条件。

## 发布前串行路线

发布前建议按 H 线索引中的任务顺序严格串行推进。任务数量不做硬性规定，应服从发布前实际依赖；本文只记录对比基线，具体任务顺序以 `docs/picoui-serial/H-线计划索引.md` 为准。

### 内部第一版 `v0.1`

目标：

- 9 个 PicoUI 控件作为首批上层 API。
- 明确能力子集。
- 自动 gate 全通过。
- 对比文档和 release matrix gate 明确限制。

需要完成：

1. H 线索引和本文档入库。
2. release matrix schema 和 gate。
3. 当前 9 控件 release contract。
4. `image` / `list` 发布限制硬化。
5. demo catalog。
6. 两个 demo 的最小 manual artifact 记录。
7. release notes / known limitations。
8. 独立发布 review。
9. 顺序跑 `ctest -L picoui`、`ctest -L visible`、`ctest -L mapping`。

### H线后的对外第一版候选 `v1.0`

目标：

- 除 9 个当前控件外，至少补 3 个高价值控件：
  - `progress_bar`
  - `line_edit`
  - `combo_box`
- 建立输入/focus/event 合同。
- 建立更完整 release evidence。

需要完成：

1. H 线索引中定义的全部发布前串行任务。
2. `ProgressBar` vertical slice。
3. `LineEdit` vertical slice。
4. `ComboBox` vertical slice。
5. 输入事件与焦点合同。
6. manual artifact / 人工验收记录。

## 当前推荐结论

建议当前先不发布第一版本，先按 H 线串行任务推进。

理由：

- 当前 9 个控件已有真实 backend mapping 和自动 gate，适合内部试用。
- 当前数量覆盖率只有 34.6%，不适合宣传成 LingDongGUI 主流控件完整覆盖。
- 当前能力差距已经可控地集中在 image/list 的非 support 项，以及发布口径/证据层文档尚未冻结。
- H 线最有价值的下一步不是马上发布，也不是立刻扩新控件，而是先把发布矩阵、known limitations、manual artifact、发布文档和 closeout review 做完，防止发布口径失真。
- `progress_bar / line_edit / combo_box` 应作为 `H线` 之后的新线候选，而不是当前 H 线 blocker。
