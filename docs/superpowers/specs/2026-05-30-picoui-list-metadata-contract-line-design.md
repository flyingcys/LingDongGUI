# TINYUI List Metadata 合同线设计

## 目标

- 为 `list style_class` 与 widget-level `list user_data` 建立稳定、诚实、可长期维护的 public 合同边界。
- 把 `list` 当前已收口的真实交互能力，与仍然只是 metadata 存储的附加字段彻底拆开。
- 为后续是否需要把 `list metadata` 升级成更窄的真实行为子线提供唯一 design truth，而不是继续把它们和 `on_selected(..., user_data)`、theme/style、marker 等能力混写。

## 背景

- 当前 `list` 的主能力已经有独立真实合同：
  - `tinyui_list_create()` / `tinyui_list_create_with_props()` -> `ldList_init`
  - `tinyui_list_add_item()` -> full text snapshot -> `ldListSetText`
  - `tinyui_list_set_selected_index()` / `get_selected_index()` -> `ldListSetSelectItem` / `ldListGetSelectItem`
  - `tinyui_list_set_on_selected()` -> native `SIGNAL_CLICKED_ITEM` bridge
  - `tinyui_theme_apply_to_widget(..., PICOUI_PART_MAIN, ...)` -> `ldListSetBackgroundColor()` / `ldListSetSelectColor()`
  - `tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()` 也已分别收口到真实 hidden / selectable 语义
- 当前剩余的 `list style_class` 与 widget-level `list user_data` 不属于上述真实交互链。
- 它们目前更接近 TINYUI / backend wrapper 侧附加 metadata，而不是 `ldList` 已消费的真实 style/state 语义。

## 非目标

- 不在本设计里为 `list style_class` 新增真实 `ldList` style backend。
- 不把 widget-level `list user_data` 偷换成 `on_selected(..., user_data)` callback cookie。
- 不顺手改 `list` 的 theme/style、enabled、marker、selection 或 add-item 合同。
- 不在本线里重开 `image` 或 `text font` 的任何剩余议题。

## 当前事实冻结

### 已真实闭环的 list 能力

- `tinyui_list_create()`、`tinyui_list_create_with_props()`：真实落到 `ldList_init`
- `tinyui_list_add_item()`：真实落到 full text snapshot + `ldListSetText`
- `tinyui_list_set_selected_index()`、`tinyui_list_get_selected_index()`：真实落到 `ldListSetSelectItem`、`ldListGetSelectItem`
- `tinyui_list_set_on_selected()`：真实走 native selected bridge
- `tinyui_theme_apply_to_widget(..., PICOUI_PART_MAIN, ...)`：真实落到 `ldList` 颜色入口
- `tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`：真实落到 hidden / selectable 子合同

### 当前未闭环项的真实现状

- `tinyui_widget_set_style_class()` on list
  - 当前会更新 TINYUI `widget.style_class`
  - 当前也会更新 backend wrapper `style_class`
  - 没有真实 `ldList` 消费链
- `tinyui_widget_set_user_data()` on list
  - 当前会更新 TINYUI `widget.user_data`
  - 当前也会更新 backend wrapper `user_data`
  - 没有真实 `ldList` 消费链
- `tinyui_list_set_on_selected(..., user_data)`
  - 使用的是 `list` callback 私有 `user_data`
  - 它是已收口的 callback cookie 语义
  - 它不等同于 widget-level `tinyui_widget_set_user_data()`

## 合同结论

### list style_class

- 当前只承认 metadata-only 存储语义。
- 它可以继续作为 TINYUI / backend wrapper 侧的附加信息存在，但在没有真实 `ldList` 消费链之前，不承诺任何视觉、主题、样式或交互行为。
- 因此这项在当前矩阵中继续保持 `incomplete_contract`，而不是 `support`。

### list user_data

- 当前只承认 widget-level metadata-only 存储语义。
- 它可以继续作为 TINYUI / backend wrapper 侧的通用附加指针存在，但不承诺任何真实 `ldList` 行为。
- 这项必须与 `tinyui_list_set_on_selected(..., user_data)` 的 callback cookie 语义明确分离。
- 因此这项在当前矩阵中继续保持 `incomplete_contract`，而不是 `support`。

### on_selected(..., user_data) 的边界

- `on_selected(..., user_data)` 已由独立 support 合同覆盖。
- 它的 `user_data` 是 callback 私有 cookie，不代表 widget-level `user_data` 已闭环。
- 后续文档与测试不得再把这两条语义混成一条 broad `user_data` 结论。

## 设计选择

### 方案 A：metadata-only 冻结（推荐）

- `list style_class` 继续只承认 metadata-only
- widget-level `list user_data` 继续只承认 metadata-only
- `on_selected(..., user_data)` 保持独立 support 合同，不回流到 broad metadata 线
- 优点：
  - 完全贴合当前真实实现
  - 不会把不存在的 `ldList` style/backend 消费链误写成“差一点 support”
  - 能把 widget-level metadata 与 callback cookie 语义彻底拆开
- 缺点：
  - 当前不会新增任何 user-facing list metadata 行为

### 方案 B：把 style_class / user_data 都升级成 support

- 只因为 TINYUI 与 backend wrapper 两侧都能存到值，就把这两项直接写成 `support`
- 缺点：
  - 会把 metadata 存储错误外推成真实 `ldList` 行为支持
  - 会模糊 widget-level `user_data` 与 callback cookie 的边界
- 不采用

### 方案 C：直接挑一条 list metadata 实现子线开工

- 例如先做真实 `style_class -> ldList` 消费链，或让 widget-level `user_data` 进入更多 runtime 路径
- 缺点：
  - 当前还没先把 broad metadata 语义冻结清楚
  - 很容易再次把 metadata-only 与 callback/visual 行为混回一条 broad gap
- 不采用

### 选择结果

- 本线采用方案 A：先做 metadata-only 语义冻结。

## 架构边界

### TINYUI 层

- 继续保留现有 `tinyui_widget_set_style_class()` 与 `tinyui_widget_set_user_data()` API 形状。
- 但文档与测试口径必须清楚区分：
  - 哪些只是 widget / backend wrapper metadata
  - 哪些已是独立真实行为合同

### backend 层

- 当前 backend 只负责保存 `style_class` / `user_data` 到 wrapper。
- 当前不负责把这些值解释成 `ldList` 的真实 style/state 行为。

### LingDongGUI 层

- 当前 `ldList` 没有由这两条 TINYUI metadata 自动消费的稳定 public 合同。
- 因此在没有新 backend 合同前，不应把它外推成“天然能吃 style_class / widget-level user_data”的控件。

## 测试与证据要求

- `list style_class` 的测试只能证明：
  - TINYUI widget 存到值
  - backend wrapper 也存到值
- `list user_data` 的测试只能证明：
  - TINYUI widget 存到值
  - backend wrapper 也存到值
- 这些测试都不能外推成：
  - `ldList` 已消费
  - 视觉行为已改变
  - callback 语义已改变
- `on_selected(..., user_data)` 的既有 support 证据必须继续单独保留，不得被 broad metadata 测试替代。

## 验收标准

- `list style_class` 与 `list user_data` 不再被混写成一条 broad 缺口。
- widget-level `list user_data` 与 `on_selected(..., user_data)` callback cookie 语义被明确拆开。
- 真相源能直接指导下一步：
  - 若继续推进 `list metadata`，必须先挑一个更窄的真实行为子线
  - 而不是继续把 broad metadata gap 当实现任务硬做

## 对 G 线索引的影响

- `G-线剩余缺口合同决策入口.md` 应为 `List Metadata 合同线` 增加 design truth 链接。
- `G-线计划索引.md` 应把当前下一条串行线记为 `List Metadata 合同线`，直到该线完成实现或被更精细的子线取代。
- 当前阶段不直接改变矩阵里 `list style_class` / `list user_data` 的状态值；本线先冻结语义，后续若进入实现，再更新矩阵。
