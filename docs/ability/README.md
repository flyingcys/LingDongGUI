# LingDongGUI 原生控件能力与 PicoUI 覆盖索引

本文是 LingDongGUI 原生控件/API 能力与 PicoUI 覆盖审计入口。
覆盖结论以 `ldgui_public_api_inventory.json` 和 `picoui_release_capability_matrix.json` 为准，不以截图、demo 存在或人工摘要为准。

## 覆盖口径

- inventory：`tests/picoui/contract/ldgui_public_api_inventory.json`
- matrix：`tests/picoui/contract/picoui_release_capability_matrix.json`
- inventory schema：`a-0.8-ldgui-public-api-inventory-v1`，已追加 a-0.9 `group_kind` / `policy_category` 行级字段。
- matrix schema：`a-0.9-allowlist-policy-v1`
- 控件类型口径：`src/gui/ldBase.h` 的 `ldWidgetType_t`，共 `28/28` 个控件类型，包含 `background`。
- LingDongGUI 原生 API 能力口径：`src/gui/ld*.h` public API inventory，共 `611` 条 API。
- PicoUI 覆盖统计：`allowlisted`: 207, `covered`: 404
- `covered` 才表示有 PicoUI API/backend/unit/gate 证据；`allowlisted` 表示已纳入 ledger 但不是 PicoUI user-facing direct wrapper 覆盖。
- 因此当前不能笼统写“PicoUI 100% direct 覆盖 LingDongGUI 全部原生 API”；应逐 group 看 covered/allowlisted。

## Direct Public API 100% 结论

当前未达到 PicoUI 对外 public API direct 100%。

- `covered=404`：有真实 PicoUI public API/backend/unit/gate 证据。
- `allowlisted=207`：policy ledger 已闭环，但不是 direct public wrapper。
- `missing_gap_total=0` 只表示没有未建账 native API，不表示 direct 100%。
- `direct_100_category` 统计：`policy_never_public`: 191, `optional_public_extension`: 16, `direct_100_required_if_user_demands`: 0
- 当前 a-0.10 不新增 PicoUI public API；`optional_public_extension` 只表示未来可单独开线设计的 tree/focus/nameId 等增强。

## a-0.9 policy schema

`a-0.9` 已把 `611` 行 native API 全部纳入机器可校验 policy schema：

- `group_kind` 统计：`widget`: 513, `shared_base`: 63, `runtime_host`: 16, `internal_helper`: 19
- `policy_category` 统计：`direct_covered`: 404, `lifecycle_internal`: 108, `render_pipeline_internal`: 27, `runtime_host_internal`: 16, `layout_solver_internal`: 14, `memory_internal`: 5, `base_tree_policy`: 20, `resource_time_helper_policy`: 5, `drawing_helper_policy`: 5, `backend_private_hook`: 3, `native_action_private`: 2, `enum_only_semantics`: 2
- `covered` 行必须是 `policy_category=direct_covered`。
- `allowlisted` 行必须是 `required=false`，且必须有非空 `allowlist_reason` 与非 `direct_covered` 的 `policy_category`。
- group judgement：`policy_complete_not_direct_100`: 28, `non_widget_policy_complete`: 4；当前没有仍处于 `parity_incomplete` 的 matrix group。

## LingDongGUI API 分类统计

| 分类 | 数量 |
| --- | --- |
| `callback_hook` | 3 |
| `capability` | 13 |
| `getter` | 82 |
| `helper` | 27 |
| `init` | 30 |
| `lifecycle` | 108 |
| `macro_alias` | 129 |
| `setter` | 179 |
| `show` | 27 |
| `update_action` | 13 |

## 控件类型覆盖

| 控件类型 | 能力文档 | API group | PicoUI 覆盖摘要 |
| --- | --- | --- | --- |
| `window` | [window](./window.md) | `window` | `allowlisted`: 7, `covered`: 16 |
| `background` | [background](./background.md) | `无` | 无独立 matrix group；需看 root/window/tree 语义，不能虚构 direct 100%。 |
| `button` | [button](./button.md) | `button` | `allowlisted`: 7, `covered`: 20 |
| `image` | [image](./image.md) | `image` | `allowlisted`: 5, `covered`: 4 |
| `text` | [text](./text.md) | `text` | `allowlisted`: 5, `covered`: 18 |
| `line_edit` | [line_edit](./line_edit.md) | `line_edit` | `allowlisted`: 5, `covered`: 14 |
| `graph` | [graph](./graph.md) | `graph` | `allowlisted`: 5, `covered`: 16 |
| `checkbox` | [checkbox](./checkbox.md) | `checkbox` | `allowlisted`: 5, `covered`: 18 |
| `slider` | [slider](./slider.md) | `slider` | `allowlisted`: 5, `covered`: 14 |
| `switch` | [switch](./switch.md) | `switch` | `allowlisted`: 5, `covered`: 20 |
| `progress_bar` | [progress_bar](./progress_bar.md) | `progress_bar` | `allowlisted`: 5, `covered`: 15 |
| `gauge` | [gauge](./gauge.md) | `gauge` | `allowlisted`: 5, `covered`: 14 |
| `qrcode` | [qrcode](./qrcode.md) | `qrcode` | `allowlisted`: 5, `covered`: 9 |
| `date_time` | [date_time](./date_time.md) | `date_time` | `allowlisted`: 5, `covered`: 15 |
| `icon_slider` | [icon_slider](./icon_slider.md) | `icon_slider` | `allowlisted`: 5, `covered`: 11 |
| `combo_box` | [combo_box](./combo_box.md) | `combo_box` | `allowlisted`: 5, `covered`: 19 |
| `arc` | [arc](./arc.md) | `arc` | `allowlisted`: 5, `covered`: 12 |
| `radial_menu` | [radial_menu](./radial_menu.md) | `radial_menu` | `allowlisted`: 5, `covered`: 6 |
| `scroll_selecter` | [scroll_selecter](./scroll_selecter.md) | `scroll_selecter` | `allowlisted`: 5, `covered`: 15 |
| `label` | [label](./label.md) | `label` | `allowlisted`: 5, `covered`: 15 |
| `table` | [table](./table.md) | `table` | `allowlisted`: 5, `covered`: 31 |
| `keyboard` | [keyboard](./keyboard.md) | `keyboard` | `allowlisted`: 8, `covered`: 10 |
| `animation` | [animation](./animation.md) | `animation` | `allowlisted`: 5, `covered`: 8 |
| `list` | [list](./list.md) | `list` | `allowlisted`: 5, `covered`: 19 |
| `message_box` | [message_box](./message_box.md) | `message_box` | `allowlisted`: 5, `covered`: 13 |
| `calendar` | [calendar](./calendar.md) | `calendar` | `allowlisted`: 5, `covered`: 7 |
| `progress_wheel` | [progress_wheel](./progress_wheel.md) | `progress_wheel` | `allowlisted`: 5, `covered`: 5 |
| `clock` | [clock](./clock.md) | `clock` | `allowlisted`: 5, `covered`: 7 |

## 其他原生 API 分组

| 分组 | API 条目数 | 能力文档 | PicoUI 覆盖摘要 |
| --- | --- | --- | --- |
| `gui` | 16 | [gui](./gui.md) | `allowlisted`: 16 |
| `mem` | 5 | [mem](./mem.md) | `allowlisted`: 5 |
| `switch_internal` | 6 | [switch_internal](./switch_internal.md) | `allowlisted`: 6 |
| `window_layout_internal` | 8 | [window_layout_internal](./window_layout_internal.md) | `allowlisted`: 8 |

## 共享能力

| 分组 | API 条目数 | 能力文档 | PicoUI 覆盖摘要 |
| --- | --- | --- | --- |
| `base` | 63 | [base](./base.md) | `allowlisted`: 30, `covered`: 33 |

## 可靠性说明

- 本目录不使用人工摘要判断 PicoUI 100%。
- 每个 group 页按 LingDongGUI symbol 逐行列出 PicoUI 状态、PicoUI API、backend proof、unit/gate 和 allowlist 原因。
- `manual_artifact` 只表示 artifact/catalog/frame 证据；未人工复核时不能当人工验收通过。
- 若 inventory 或 matrix 更新，本目录必须同步更新，并重新跑 native API exhaustiveness checker。
