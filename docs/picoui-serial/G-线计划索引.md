# PicoUI G线计划索引

- `D线` 收口索引：`docs/picoui-serial/D-线计划索引.md`
- `F线` 收口索引：`docs/picoui-serial/F-线计划索引.md`
- `PicoUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `D线` 当前控件合同矩阵：`docs/superpowers/specs/2026-05-29-picoui-d-line-widget-contract-matrix.md`
- `F线` list vertical slice 设计：`docs/superpowers/specs/2026-05-29-picoui-f-line-new-widget-vertical-slice-design.md`
- `F线` list vertical slice 实施记录：`docs/superpowers/plans/2026-05-29-picoui-f-line-new-widget-vertical-slice-implementation.md`
- `G线` 总设计真相源：`docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-design.md`
- `G线` 总实施口径：`docs/superpowers/plans/2026-05-29-picoui-g-line-current-widget-capability-gap-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## G线定位

`G线` 不负责“再加一个新控件”，也不负责“顺手补几个 API”。`G线` 的目标是把 `D/F` 完成后的真实代码状态、文档口径、能力边界和下一阶段任务统一收口，避免把“真实 backend 已接通”误写成“对应 `LingDongGUI` 控件能力已 100% 暴露”。

一句话口径：

- 当前已完成控件都已接到真实 `LingDongGUI` backend。
- 当前只保证 PicoUI 已定义、已测试、已记录的合同子集。
- 当前不能宣称 PicoUI 已 100% 支持对应 `ld*` 控件的全部能力。

## 当前完成态

当前已完成并走真实 backend 的 PicoUI 控件：

- `window`
- `label`
- `button`
- `checkbox`
- `switch`
- `slider`
- `text`
- `image`
- `list`

当前代码状态应理解为：

- `window/label/button/checkbox/switch/slider/text/image` 已完成当前阶段的合同子集、真实 backend 映射和既有 gate 接线。
- `list` 已完成最小 vertical slice：public API、unit test、真实 `ldList` backend create/set items/set selected、demo、runtime/mapping/visible gate 已接入。
- `list` 当前仍然只是 vertical slice，不是 `ldList` 全能力封装。

## Review 结论

本轮针对 `D/F` 完成态的独立 review，主结论如下：

### 1. 当前代码和“100% 支持”说法不一致

不能写：

- “只是封装，不是重写，所以应该已经 100% 支持。”
- “已完成控件已经完整暴露对应 `LingDongGUI` 控件能力。”

可以写：

- “已完成控件都有真实 `LingDongGUI` backend 对象和可测的 PicoUI 合同子集。”
- “PicoUI 当前是稳定上层封装层，不是 `ld*` 底层 API 的逐项 1:1 镜像。”

原因：

- PicoUI public API 明显比对应 `ld*` header 更窄。
- 当前通用 style/state/event/theme 也只覆盖一个明确子集。
- 若按 100% 支持标准，`focus/dirty/selectable/corner`、大量 widget-specific setter/getter、以及若干 native 行为都还没有 public contract。

### 2. `list` 当前有一个真实代码缺口

- `picoui_list_set_on_selected()` 已作为 public API 暴露。
- 但当前实现只保存 callback/user_data，未接入 `ldList` native selection event bridge。
- 因此现在不能写“list selection callback 已真实闭环”。
- 更严格地说，这个公开 API 当前合同未兑现，至少应在文档中明确标为 `deferred / incomplete contract`，而不是写成已支持。

### 3. list mapping marker 当前语义偏强

- 当前 `PICOUI_BACKEND_REAL_WIDGET_IDS` 里同时出现 `list` 和 `item_wifi/item_bluetooth/item_display`。
- 这些 `item_*` 是写进 `ldListSetText()` 的数据项 marker，不是独立 `LingDongGUI widget`。
- 因此当前 mapping gate 能证明：
  - `list` 走了真实 `ldList` backend
  - item payload 已写入 `ldList`
- 当前 mapping gate 不能证明：
  - 每个 item 都有独立 backend widget
  - list item 已拥有和普通 widget 一样的 backend tree 语义

### 4. `slider` 还存在一处能力边界风险

- `picoui_slider_set_range()` 当前只更新 PicoUI 侧 `min/max/value` shadow state。
- 当 range clamp 改变 value 时，文档不应把它写成底层 `ldSlider` percent 已同步收口，除非后续补实现和测试。
- 这项目前更适合在 `G线` 任务里明确列为待补项。

## 当前不能写成 100% 支持的证据

### 通用层缺口

- 没有 public `focus` API。
- 没有 public `dirty` API。
- 没有 public `selectable` / `corner` API。
- direct style setter 当前主要是 PicoUI 字段合同，不是“立即下沉到底层 `ld*` style API”的全量承诺。

### 各控件典型缺口

- `window`：未暴露 `image`、`grid padding group` 等能力。
- `label`：未暴露 `transparent`、`align`、`background image`、getter。
- `button`：未暴露 `image`、`checkable`、`keyValue`、press state getter/setter。
- `checkbox`：未暴露 radio group / image mode / selectable 相关能力。
- `switch`：未暴露 direction / orientation / image skin / navigation。
- `slider`：未暴露 orientation / image skin / indicator width / slim size。
- `text`：未暴露 static text / scroll seek / scroll move / background image。
- `image`：不做资源加载系统，只做 tile source 绑定；`theme` 明确 reject。
- `list`：未暴露 `itemHeight/textColor/align/selectColor/itemWidget/padding/margin/selectable/corner`，且 `on_selected` 还没接 native event bridge。

## G线目标

`G线` 目标不是追求“控件数量增加”，而是回答并落地这三个问题：

1. 当前已完成控件，到底支持了对应 `LingDongGUI` 控件的哪些能力，哪些没有？
2. 哪些缺口属于应该补齐的上层封装能力，哪些应该明确 reject/deferred？
3. 哪些 gate/marker/documentation 需要改口径，避免把真实 backend 子集误写成全能力完成？

## 阶段导航

- `G0`：worktree 准备和 baseline
- `G1`：冻结 capability gap 真相源
- `G2`：list selection contract 收口
- `G3`：list marker 语义拆分
- `G4`：slider range contract 收口
- `G5`：shared base 语义决议
- `G6`：交互控件高价值缺口
- `G7`：展示控件高价值缺口
- `G8`：G线 closeout

## 串行规则

- `G1` 未完成前，不进入 `G2`
- `G2` 未完成前，不进入 `G3`
- `G3` 未完成前，不进入 `G4`
- `G4` 未完成前，不进入 `G5`
- `G5` 未完成前，不进入 `G6`
- `G6` 未完成前，不进入 `G7`
- `G7` 未完成前，不进入 `G8`
- 每个阶段完成后必须记录：
  - 改了哪些文件
  - 跑了哪些命令
  - 哪些是 public contract 变化
  - 哪些是 backend / mapping / visible 证据
  - 哪些能力仍然是 `reject/deferred/incomplete_contract`

## G线任务顺序

### G0 真相源冻结

- 以当前源码、现有 public header、`ld*` header、unit/runtime gate 和本索引为准。
- 所有后续汇报统一引用 `G线` 口径，不再把 `D/F` 中的阶段性表述当最新真相源。

### G1 逐控件 capability gap matrix

- 对 `window/label/button/checkbox/switch/slider/text/image/list`，逐项对照对应 `ld*` header。
- 每个能力明确标记：
  - `support`
  - `reject`
  - `deferred`
  - `incomplete_contract`
- 这张矩阵是 G 线第一真相源，优先级高于“demo 能跑”“visible 通过”这类阶段性证据。

### G2 先收口现有口径错误

- 把 `list on_selected` 改成明确的 `incomplete_contract / deferred bridge` 口径。
- 把 `REAL_WIDGET_IDS` 中 list item 的表述改成 “list item marker / payload marker”，不要继续写成独立 real widget 证据。
- 把 `slider set_range` 的同步边界明确写进矩阵或 G 线任务，不继续含糊表述。

### G3 优先补高价值缺口

建议顺序：

1. `list`
   - native selection event bridge
   - item marker 语义拆分
   - `itemHeight`
   - `textColor/align/selectColor`
   - `padding/margin`
2. 通用基座
   - direct style setter 是否实时下沉
   - `enabled` 是否所有交互控件都同步到底层
   - 是否公开 `selectable/corner/focus/dirty`
3. 交互控件
   - `switch` direction/orientation/image
   - `slider` orientation/image/indicatorWidth/slimSize
   - `checkbox` radio/image mode
   - `button` checkable/image
4. 展示控件
   - `text` scroll/static/background image
   - `label` align/transparent/background image
   - `window` image/grid padding/padding group
   - `image` mask color

### G4 gate 口径收紧

- `mapping` 只证明真实 backend 对象或 payload 进入真实 backend 路径。
- `visible` 只证明 dummy SDL + readback 下的 automatic visible correctness。
- `manual artifact` 仍独立，不可被 `smoke/mapping/visible` 替代。
- 任何“控件能力已完成”的结论，必须明确对应的是哪一层证据。

## 当前建议

如果下一阶段目标是“做完整当前控件”，应优先进入 `G1 -> G2 -> G3`，不要先继续铺新控件数量。

原因：

- 现在最缺的不是“再多一个控件 demo”，而是“现有控件对底层能力差距到底有多大”的统一真相源。
- 没有 capability gap matrix，就会不断重复把 vertical slice 或 gate pass 误读成全能力完成。
- `list` 现在已经暴露了最典型问题：public API 可以先做出来，但不代表合同真的闭环。
