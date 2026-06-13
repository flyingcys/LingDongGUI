# TINYUI H线发布差距与 LingDongGUI 控件对比

## 目的

本文是 H 线的专门对比文档，用于回答：

- TINYUI 离第一版本发布还有多少距离。
- TINYUI 在控件数量上与 LingDongGUI 的差距。
- TINYUI 在每个已覆盖控件的能力上与 LingDongGUI 的差距。
- 哪些缺口是第一版 blocker，哪些可以作为 known limitations。

本文不使用 G 线文件名，因为它不是 G 线剩余缺口继续实现文档。G 线已经完成当前控件 capability gap 收口；H 线面向发布判断。

当前结论已经收紧为：

- `v0.1` 现在可以宣称 `window / label / button / slider` 已完成首版对齐。
- `checkbox / switch / text / image / list` 已 wrapped，但仍在 `v0.2 parity backlog`。

本文不是 release closeout，而是 `H线` 现状与 `J线` 分流口径的对比真相源。

从 `J0/J1` 起，`tests/tinyui/contract/tinyui_release_capability_matrix.json` 是 release matrix 的机器可读真相源；本文负责解释口径、差距和发布判断，不再承担机器解析职责。该 matrix 只记录自动可维护事实，不负责表达“人工是否已验收通过”。

## 2026-05-30 当前状态补记

- 当前文档包、release matrix JSON、release matrix gate、当前 9 控件 release contract、demo catalog、发布说明与测试矩阵文档都已入库。
- 当前已实测通过：
  - `ctest --test-dir build --output-on-failure -L tinyui`
  - `ctest --test-dir build --output-on-failure -L visible`
  - `ctest --test-dir build --output-on-failure -L mapping`
  - `python3 tests/tinyui/contract/check_tinyui_public_api.py`
  - `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
  - `python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets`
  - `python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo settings_panel`
- 当前两条 manual artifact 脚本路径都能生成 artifact，且 `C-线人工窗口验收记录.md` 已补入基于 `frame.ppm` 的人眼观察结论。
- 因此当前可以认定最小范围的 `artifact-based visual observation` 已补齐，但不能认定 `live OS window acceptance passed`，也不能据此认定 `release ready`。
- 本轮独立 review 抓出的 blocker 已修复：
  - `label`：`tinyui_label_props` / `tinyui_label_create_with_props()` 已补齐 `transparent / align / background_source` 路径。
  - `slider`：release matrix 已回调到真实 public API 边界，不再把不存在的 `tinyui_slider_get_value()` 对应能力写成 `support`。

## 统计口径

### 计入 LingDongGUI 可封装控件

计入标准：

- `src/gui/ld*.h` / `src/gui/ld*.c` 中有独立 widget struct。
- 有 `ldXxx_init()` / `ldXxxInit()` 或等价初始化入口。
- 有可被 TINYUI 抽象成 public widget 的用户语义。

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

### TINYUI 覆盖口径

TINYUI “已覆盖”不等于“镜像 LingDongGUI 全能力”。本文把覆盖拆成三层：

1. `控件数量覆盖`：是否存在对应 TINYUI public 控件。
2. `真实 backend 覆盖`：是否落到真实 `ld*` 对象或真实 `ldBase` 能力。
3. `能力合同覆盖`：某个具体能力是否有 public API、真实 backend 行为和证据层。

### release matrix schema 口径

当前 `J0/J1` 机器可读 release matrix 使用以下固定字段：

- 控件包裹状态：`wrapped / not_wrapped / deferred`
- parity 分层：`parity_complete / parity_incomplete / not_applicable`
- parity 桶：`v0_1_parity_target / v0_2_parity_backlog / post_v0_2_candidate`
- 能力状态：`support / reject / incomplete_contract / deferred`
- 发布判断：`internal_v0_1_blocker / internal_v0_1_known_limitation / post_h_candidate`
- JSON 路径：
  - `status_enums.widget_status`
  - `status_enums.parity_status`
  - `status_enums.parity_bucket`
  - `status_enums.capability_status`
  - `status_enums.release_judgement`
  - `summary.v0_1_parity_target_total`
  - `summary.v0_2_parity_backlog_total`
  - `summary.parity_complete_total`
  - `summary.parity_bucket_counts`
  - `widgets[].parity_bucket`
  - `widgets[].parity_status`
  - `widgets[].capabilities[]`
- manual artifact：`artifact_entry_exists` 与 `manual_review_required` 分开记录，且 scope 固定为 `widget_level_only`

边界约束：

- release matrix 要表达的是“当前机器可维护的发布事实”，不是“人工验收已经通过”的布尔 gate。
- `wrapped` 只回答“这个控件是否已经接入 TINYUI public widget + 真实 backend mapping”；它不单独等于 parity 完成。
- `parity_complete` 只用于 `window / label / button / slider` 这四个控件，前提是对应 capability rows 已显式覆盖其 `J2-J5` 收口合同，而且 public API / props 路径 / tests / matrix 真正一致，而不是靠纯标签硬编码。
- 当前 capability rows 的已实现口径是：
  - `window`：`background_image_and_mask`、`padding_group_contract`、`honest_minimal_readback`
  - `label`：`transparent`、`align`、`background_image_and_mask`、`text_color_bg_color_align_transparent_readback`
  - `button`：`release_and_press_image`、`transparent`、`font`、`checkable`、`key_value`、`pressed_state`
  - `slider`：`value`、`horizontal`、`background_and_indicator_image_mask`、`indicator_width`、`slim_size`、`percent_and_orientation_readback_consistency`
- `slider` 当前 public API 仍然不包含 `get_value`；这不再是 blocker，而是已被 matrix/合同诚实表达的边界。
- `widget_release_judgement` 按控件级发布桶理解，不等于该控件每一条 capability 都是同一状态；当前讨论范围内的 wrapped 控件，若不属于 blocker，就归到 `internal_v0_1_known_limitation`，表示“在当前发布面内，但仍需配套 known limitations 文案”。
- `capability_release_judgement` 只用于当前 capability 条目对应的发布判断；它和 widget 级字段共享同一套枚举值，但粒度不同。当前属于 `support` 的 capability 也统一归到 `internal_v0_1_known_limitation` 这一发布桶，表示“属于当前 `v0.1 parity` 发布面，需要随 release notes / known limitations 一起发布”，不是说该 capability 本身是 limitation。
- `evidence_enums` 仍保持 `unit / contract / mapping / visible / manual_artifact` 五层；当前 matrix 记录的是 capability 与 parity 的机器可读完成事实，不把 artifact-based visual observation 折叠成 live OS window acceptance。
- release matrix 的 `manual_artifact` 只表达 widget-level 粒度；demo-level artifact entry 是否存在，统一以 `docs/tinyui-serial/C-线人工窗口验收记录.md` 为真相源。
- `artifact_entry_exists = true` 只代表已经登记对应粒度的 artifact 条目，不代表人工 reviewer 已确认通过。
- `manual_review_required = true` 代表该项仍需人工复核；即使已有自动 visible/mapping 证据，也不能把它自动折叠成人工验收结论。

## 总体距离

| 指标 | 当前数量 | 说明 |
| --- | ---: | --- |
| LingDongGUI 可封装原生控件 | 26 | H 线统计口径 |
| TINYUI public 控件 | 9 | `window/label/button/checkbox/switch/slider/text/image/list` |
| 未覆盖原生控件 | 17 | 见后文分组 |
| 控件数量覆盖率 | 34.6% | `9 / 26` |
| TINYUI 当前能力矩阵行 | 62 | 当前 J1 matrix capability rows |
| 能力项 `support` | 53 | 有 public API / backend / 证据共同支撑 |
| 能力项 `reject` | 4 | 明确不支持 |
| 能力项 `incomplete_contract` | 4 | 有入口或存储，但合同/真实语义不完整 |
| 能力项 `deferred` | 1 | 方向合理，但当前不承诺 |

### 距离判断

如果当前讨论的是 `v0.1 parity`：

- `window / label / button / slider` 的 review blockers 已修复，release matrix gate 已补强，关键 capability rows 已能承接 `parity_complete` 结论。
- 当前仍不能写成 `release ready`，因为 manual artifact 只到 artifact-based visual observation，不等于 live OS window acceptance passed。
- 因此当前正确结论是：`v0.1 parity evidence complete`，但不是发布完成。

如果第一版本是 `public v1.0`：

- 距离较远。
- 当前控件数量覆盖只有 `34.6%`。
- 输入类、数据展示类、进度仪表类、复合控件类缺口明显。
- 当前 style/theme 只能表达颜色/metric 子集，不足以表达 LingDongGUI 大量 image/mask/transparent skin 能力。
- 至少应先完成 `progress_bar / line_edit / combo_box` 三个高价值控件，再讨论对外第一版。
- 但这已经超出当前 `H线` 基于现有 `9` 控件的发布准备范围，应在 `H线` 收口后另开新线。

## LingDongGUI 原生控件全集

| 分组 | 原生控件 | TINYUI 状态 | 第一版建议 |
| --- | --- | --- | --- |
| 容器 / 基础显示 | `ldWindow` | 已覆盖为 `window` | `v0.1 parity` |
| 文本显示 | `ldLabel` | 已覆盖为 `label` | `v0.1 parity` |
| 文本显示 | `ldText` | 已覆盖为 `text` | `v0.2 parity backlog` |
| 基础交互 | `ldButton` | 已覆盖为 `button` | `v0.1 parity` |
| 基础交互 | `ldCheckBox` | 已覆盖为 `checkbox` | `v0.2 parity backlog` |
| 基础交互 | `ldSwitch` | 已覆盖为 `switch` | `v0.2 parity backlog` |
| 基础交互 | `ldSlider` | 已覆盖为 `slider` | `v0.1 parity` |
| 图片 | `ldImage` | 已覆盖为 `image` | `v0.2 parity backlog` |
| 列表 | `ldList` | 已覆盖为 `list` | `v0.2 parity backlog` |
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

- 当前 `9` 个 TINYUI 控件全部标为 `wrapped`。
- 其余 `17` 个 LingDongGUI 可封装控件当前全部标为 `not_wrapped`。
- `17` 个未覆盖控件当前全部归到 `post_v0_2_candidate`。
- `line_edit` / `combo_box` / `progress_bar` 与其余未覆盖控件一样，当前都属于 `H线` 之后或 `public v1.0` 讨论范围内的新线候选，不并入当前 H 线，也不表达“已经支持”。

## 已覆盖控件差距摘要

逐控件 release contract 已收口到 `docs/tinyui-serial/H-线当前9控件发布合同.md`。本文只保留 H1/H4 需要的摘要级差距基线，避免对比文档与合同文档双写完整合同。

### `v0.1` 已对齐控件

以下 `4` 个控件已经达到当前首版对齐口径：

- `window`：容器、flex/grid、padding/gap/align、背景色、background image/mask、`padding_group` 高层合同。
- `label`：文本、font 映射、背景色、文字色、transparent、align、background image/mask、诚实 readback；`create_with_props` 也已覆盖 `transparent / align / background_source`。
- `button`：文本、clicked/pressed/released、release/press image、transparent、font、checkable、key_value、pressed state。
- `slider`：`set_value/range` 归一化合同、value changed callback、horizontal、background/indicator image+mask、indicator/slim size、诚实 `percent/orientation` readback；同时诚实保留“无 `get_value` public API”边界。

它们仍不等价于“完整镜像对应 `ld*` 控件”，但已经不再属于当前 `v0.1` 的功能缺口控件。

### 已 wrapped、转入 `v0.2` backlog 的控件

以下 `5` 个控件当前仍是 `wrapped but not yet parity-complete`：

- `checkbox`
- `switch`
- `text`
- `image`
- `list`

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
- 在当前 `J0/J1` release matrix 中，上述非 `support` 项统一记为 `internal_v0_1_known_limitation`，不得误写成 `support`，也不得把 manual artifact 写成自动通过。

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
- 在当前 `J0/J1` release matrix 中，list 的非 `support` 项统一记为 `internal_v0_1_known_limitation`。

### 合同文档入口

若要看 `9` 个控件的 `supported APIs / backed by ld* entries / evidence layers / known limitations / first-release wording`，统一以 `docs/tinyui-serial/H-线当前9控件发布合同.md` 为准。

## 历史说明

早期 H 线文档里曾混入“若第一版本改定义为当前 9 控件完全对齐版”的路线分支。`J0/J1` 收口后，这个分支已经不再承担当前执行路线，只保留一条历史说明：

- 当前真相源只承认 `v0.1 parity = window / label / button / slider`
- `checkbox / switch / text / image / list` 明确停留在 `v0.2 parity backlog`
- 若未来要重新讨论“9 控件全部 parity-complete”或更大的 `public v1.0`，应另开新线文档，不继续在本 H 线对比文档里展开

## 能力类型差距

### 数量差距

TINYUI 当前只覆盖 9 个控件；未覆盖 17 个控件。若目标是“LingDongGUI 主流控件覆盖”，当前还缺明显。

### layout 差距

TINYUI 已经建立 flex/grid 上层 API，并真实落到 `ldWindow` / `ldBase` 相关入口。layout 是当前较强部分。

仍需注意：

- TINYUI layout 是上层稳定合同。
- 不应承诺所有 `ldWindow` 内部 layout API 全部暴露。

### event 差距

当前 TINYUI 已覆盖：

- button clicked / pressed / released
- checkbox toggled
- switch toggled
- slider value_changed
- list on_selected

但 LingDongGUI 原生有更广泛的 native signals、弱回调、navigation、hold/release、clicked item 等模式。第一版如要做输入类控件，必须先建立统一 event/focus 合同。

### style/theme 差距

TINYUI 当前 theme 偏：

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

第一版不能宣称 TINYUI theme 已覆盖 LingDongGUI skin 系统。

### image/resource 差距

TINYUI 当前 image source 是 tile/source 绑定边界，不是完整资源加载系统。

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

TINYUI 当前 getter 较少，主要集中在 value/checked/selected index。第一版可以不补全 getter，但要承认 readback 能力是子集。

### manual artifact 差距

当前 automatic visible gate 不是 manual acceptance。

第一版若对外发布，应补：

- 人工窗口验收记录。
- 当前最小 manual artifact 范围内每个 demo 的 artifact 路径和观察结论。
- 明确 artifact existence 不等于验收通过。

当前 `H8` 与现实对齐后的最小范围固定为两个 demo：

- `tinyui_basic_widgets_demo`
- `tinyui_settings_panel_demo`

说明：

- 这是因为当前 `tests/tinyui/runtime/check_tinyui_manual_window_artifact.py` 只支持这两个 demo。
- `docs/tinyui-serial/C-线人工窗口验收记录.md` 是当前唯一人工记录真相源。
- 这两个 demo 当前已经能表达“artifact 条目存在，且已有 artifact-based visual observation 结论”，但仍不能表达“live OS 窗口验收通过”。
- 若后续要扩张到 `hello_world / layout_flex / layout_grid / theme_showcase / list_basic` 等更多 demo，只能作为 `post-H candidate`，不并入当前 `H8` 完成条件。

## 若要宣称“完整支持当前控件”，以下是 blocker

前提：这里讨论的是更强表述，即对外宣称 “TINYUI 已完整支持当前已覆盖控件” 或接近 “完整镜像对应 LingDongGUI 控件”。在这个前提下，以下问题不处理就不能这样宣称：

1. `image` 的 style/enabled/theme/padding 仍不是 support。
2. `list` 的 item marker/style_class/widget user_data 仍不是 support。
3. release matrix gate 已接入并完成 `wrapped` 状态与 `summary` 统计一致性校验，但它仍不能替代人工观察结论。
4. manual artifact 仍不是 live OS 窗口人工验收通过。
5. 未覆盖 17 个原生控件，不能宣传成 LingDongGUI 全控件覆盖。
6. style/theme 不是 LingDongGUI image/mask skin 系统。
7. font 仅最小内置映射/fallback，不是完整字体解析系统。
8. release matrix gate 已达到当前 H 线自动合同校验所需强度，但不替代 manual artifact / 人工观察。
9. 尚未完成 demo catalog 与 release notes。
10. 独立发布 review 的当前 blocking 已修复，但人工观察与 closeout 仍未完成。

## 若目标是保守的 `v0.1 parity`，以下可作为 known limitations / non-blocker

前提：这里讨论的是更保守的 first-release 口径，即 “当前 `9` 控件可内部试用，但只承诺明确子集并附带 known limitations”。在这个前提下，以下事项可以不阻塞当前 `v0.1 parity` 口径，但前提仍是 H 线 release matrix、known limitations 和发布文档包已经完成：

1. 未覆盖 17 个原生控件。
2. image style/theme/enabled/padding。
3. list per-item widget / marker / advanced style。
4. button / checkbox / switch / slider 的 image skin。
5. label/text transparent / background image。
6. getter/readback 不完整。
7. manual artifact 已补齐最小范围的 artifact-based visual observation，但这仍不等于 live OS 窗口人工验收通过。

条件是发布说明必须明确写出这些限制。

当前 `J0/J1` release matrix 把上述 known limitations 机器可读化后，文档与 JSON 必须保持同口径：

- `image` / `list` 的非 `support` 项写为 `internal_v0_1_known_limitation`。
- `17` 个未覆盖控件写为 `not_wrapped`，并归到 `post_v0_2_candidate`；`line_edit` / `combo_box` / `progress_bar` 当前与其余未覆盖控件一样属于后续新线候选，而不是当前 H 线 blocker。
- manual artifact 只表达条目是否存在、是否仍需人工复核，不能被文档或后续 gate 解释成“人工已验收通过”。
- 当前 `H8` 最小 manual artifact 范围只有 `tinyui_basic_widgets_demo` 与 `tinyui_settings_panel_demo`；任何 7-demo 扩张都属于 `post-H candidate`，不是当前 `v0.1 parity` 的完成条件。

## 当前推荐结论

当前应保持单一结论，不再混写路线：

- `window / label / button / slider` 已有 capability rows 支撑其 `v0.1 parity complete`
- `checkbox / switch / text / image / list` 仍是 `wrapped but not yet parity-complete`
- artifact-based visual observation 已补齐，但这仍不等于 `live OS window acceptance passed` 或 `release ready`
