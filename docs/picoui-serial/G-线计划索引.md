# TINYUI G线计划索引

- `A线` 收口索引：`docs/tinyui-serial/A-线计划索引.md`
- `B线` 收口索引：`docs/tinyui-serial/B-线计划索引.md`
- `C线` 收口索引：`docs/tinyui-serial/C-线计划索引.md`
- `D线` 收口索引：`docs/tinyui-serial/D-线计划索引.md`
- `F线` 收口索引：`docs/tinyui-serial/F-线计划索引.md`
- `TINYUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-tinyui-abstraction-layer-design.md`
- `TINYUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-tinyui-lingdonggui-test-architecture-design.md`
- `G线` 详细 capability gap 真相源：`docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- `G线` 剩余缺口后续入口：`docs/tinyui-serial/G-线剩余缺口合同决策入口.md`
- `Text Font 合同线` design truth：`docs/superpowers/specs/2026-05-30-tinyui-text-font-contract-line-design.md`
- `Text Font 合同线` implementation plan：`docs/superpowers/plans/2026-05-30-tinyui-text-font-contract-line-implementation.md`
- `Image 语义线` design truth：`docs/superpowers/specs/2026-05-30-tinyui-image-semantics-line-design.md`
- `Image 语义线` implementation plan：`docs/superpowers/plans/2026-05-30-tinyui-image-semantics-line-implementation.md`
- `List Metadata 合同线` design truth：`docs/superpowers/specs/2026-05-30-tinyui-list-metadata-contract-line-design.md`
- `List Metadata 合同线` implementation plan：`docs/superpowers/plans/2026-05-30-tinyui-list-metadata-contract-line-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## G线定位

- `G线` 不是新控件扩张线，也不是补视觉线；它只处理既有控件 capability gap 的冻结、排序和后续串行收口。
- `G1` 只冻结真相源：把 `window/label/button/checkbox/switch/slider/text/image/list` 当前 public API、真实 `ld*` backend 入口、状态和证据层放进统一矩阵。
- `G2+` 才进入合同补齐、backend 补齐、visible/manual artifact 增强；`G1` 不顺手修实现，不修改 demo 用户意图。

## 当前摘要

- 当前 8 个 D 线既有控件仍以 `docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md` 记录旧合同；该文档不覆盖 `list`，也不适合继续承担 G 线 capability gap 真相源。
- `G1` 新矩阵补入 `list`，并把旧“支持/拒绝”口径重写为更适合现状冻结的四态：`support`、`reject`、`deferred`、`incomplete_contract`。
- `list on_selected`、`list item marker`、`slider range` 三处边界必须以矩阵为准，不允许在索引里写成模糊“部分支持”。
- `G1` 不改 `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：该脚本仍是 D 线旧矩阵 gate，只覆盖 8 控件 / 3 态；本阶段只冻结 G 线真相源，不做 gate 迁移，避免越界到 `G2+`。

## 当前阶段结论

- `window/label/button/checkbox/switch/slider/text/image` 仍存在大量 D 线已收口合同，但 G 线要把“public API 存在”与“真实 LingDongGUI 语义闭环”分开写。
- `list` 已有 `tinyui_list_*` public API 和真实 `ldList` backend 创建/选择入口；其中 `on_selected` 已在 `G2` 通过 native selection bridge 收口到 `support`，`G4` 只把 `theme/style` 的 `PICOUI_PART_MAIN` 收口到 `ldList` 的真实颜色入口，并把 `visible` 的既有真实闭环补成单测证据；`G8` 已把 `enabled` 收口为真实 selectable / interactive 合同，但不外推到 disabled 视觉或 theme 系统。
- `list` 的 `add_item()` 已在 `G7` 收口为 “append -> full text snapshot -> real ldList render/select by index” 合同：`id` 继续只作为 TINYUI / wrapper 侧稳定 key 存储，不对 runtime marker、真实 backend object id、per-item native object 提供任何结论。
- `G3` 的收口价值是把 `item_*` 从 `PICOUI_BACKEND_REAL_WIDGET_IDS` 这类强 marker contract 中剥离，避免把 list item id 误读成真实 widget/object id。
- `slider` 的 `range` 已在 `G6` 收口为 TINYUI 自身的 `min/max <-> value <-> percent` 归一化合同：`tinyui_slider_set_range()` 现在会在更新 `min_value/max_value`、钳制当前值后，通过统一 backend helper 同步真实 `ldSlider` percent；`create_with_props(min=max=value)` 与 runtime `set_range()` 路径的退化区间语义也已统一。
- 当前 runtime mapping gate 只对 `list` 本体给出强结论；不再对 `item_wifi/item_bluetooth/item_display` 提供任何 runtime marker 级强/弱结论。`list item marker` 状态继续固定为 `reject`。
- `text` 的 `font` 已在 `Text Font 合同线` 收口为“描述值 font -> backend 最小内置映射/回退 -> 真实 `ldText/text_box` 更新”合同：`NULL` fallback、runtime rebind、ownership/cleanup 与 approx failure atomicity 均已补齐，当前可按 `support` 对外表达；但不承诺完整 family/size 解析、动态字体资源加载或 public 指针身份保持。
- 证据层继续沿用 `unit / contract / mapping / visible / manual artifact` 五层，不允许把 `mapping` 或 `capture` 外推成“UI 已完成”。
- 现有 `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 仍可通过，但这只证明未破坏旧 D 线 gate，不代表 G 线矩阵已经被 gate 接管。

## 当前剩余缺口

- `text font` 已不再是 `incomplete_contract`；当前剩余项的 broad gap 已全部被拆进 `Image 语义线` 与 `List Metadata 合同线` 并完成合同冻结。
- `image` 不应再被笼统写成“一整组通用 setter 都未闭环”：`pos/size/visible/flex/grid/ignore_layout` 已真实落到 `ldBase`；剩余项里，`style_class / user_data` 仍是 `incomplete_contract`，`bg_color / text_color / border_color / radius / enabled` 更接近 `reject`，`padding` 更接近 `deferred`。
- `Image 语义线` 已完成 design、implementation、review 与 verification 闭环：`image style_class / user_data` 的 metadata-only 合同测试已补齐，`image enabled` 现已在 `tinyui_widget_set_enabled()` 上稳定维持 `reject` 且不会改写 `widget.enabled`，`image` theme/style 的 reject 测试也已补到 `PICOUI_PART_TEXT`。
- `list style_class` 与 `list user_data` 不应再绑成同一缺口：两者当前都仍是 `incomplete_contract`，但 `style_class` 是 wrapper-only metadata，`user_data` 则要和已收口的 `on_selected(..., user_data)` callback cookie 语义明确区分。
- `G9` 已完成剩余 gap 的再拆分取证，并推动 `Text Font 合同线`、`Image 语义线`、`List Metadata 合同线` 依次收口；后续若继续推进，需要新的 line-level 合同决策，而不是把现有 `incomplete_contract / reject / deferred` 项硬做成新实现线。
- `Text Font 合同线` 已完成设计、计划、实现、review 与验证闭环；当前 support 口径仅覆盖最小内置映射/fallback，不覆盖完整字体解析系统。
- `Image 语义线` 已完成 design、implementation、review 与 verification 闭环。
- `List Metadata 合同线` 已完成 design、implementation、review 与 verification 闭环：`list style_class` 与 widget-level `list user_data` 当前都固定为 metadata-only + `incomplete_contract`，`on_selected(..., user_data)` callback cookie 继续保持独立 support 合同；现有 metadata 写入路径已满足该线最小合同，不需要再为 list metadata 增补额外实现。
- 当前 authoritative 结论是：G 线下已不存在“自然、最小、可直接验证”的下一条串行实现线；若后续继续推进，只能基于新的 line-level 合同决策重开，而不是把现有 `incomplete_contract / reject / deferred` 项直接硬做成新的实现线。

## 使用方式

- 看入口摘要：先读本索引。
- 看逐控件细节：跳转 `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`。
- 要做后续阶段实现时，必须以该矩阵里每个能力项的状态和证据层为准重新拆分后续阶段，不得回退到索引层重复抄写全部细节。
