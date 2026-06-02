# LingDongGUI 原生控件能力与 PicoUI 覆盖索引

本文是 LingDongGUI 原生控件/API 能力与 PicoUI 覆盖审计入口。
覆盖结论以 `ldgui_public_api_inventory.json` 和 `picoui_release_capability_matrix.json` 为准，不以截图、demo 存在或人工摘要为准。

## 覆盖口径

- inventory：`tests/picoui/contract/ldgui_public_api_inventory.json`
- matrix：`tests/picoui/contract/picoui_release_capability_matrix.json`
- inventory schema：`a-0.8-ldgui-public-api-inventory-v1`，已追加 a-0.9 `group_kind` / `policy_category` 行级字段。
- matrix schema：`a-0.9-allowlist-policy-v1`
- 控件类型口径：`src/gui/ldBase.h` 的 `ldWidgetType_t`，共 `29/29` 个控件类型，包含 `background` 与 `canvas`。
- LingDongGUI 原生 API 能力口径：`src/gui/ld*.h` public API inventory，共 `619` 条 API。
- PicoUI 覆盖统计：`allowlisted`: 178, `covered`: 441
- `covered` 才表示有 PicoUI API/backend/unit/gate 证据；`allowlisted` 表示已纳入 ledger 但不是 PicoUI user-facing direct wrapper 覆盖。
- 因此当前不能笼统写“PicoUI 100% direct 覆盖 LingDongGUI 全部原生 API”；应逐 group 看 covered/allowlisted。

## a-0.12 strict direct-wrapper 候选清零结论

当前只达到 a-0.12 定义的 strict direct-wrapper 候选清零；这不是 LingDongGUI native/user-facing 100% 对外能力闭环。

- `covered=441`：有真实 PicoUI public API/backend/unit/gate 证据。
- `allowlisted=178`：policy ledger 已闭环，且均为 `policy_never_public`。
- `missing_gap_total=0` 只表示没有未建账 native API，不表示 direct 100%。
- `direct_100_category` 统计：`policy_never_public`: 178
- 严格按“100% direct public API parity”目标看，a-0.12 已把 `base` 的 `optional_public_extension=16` 候选收敛到 `0`。
- `direct_public_100_complete=true` 只表示 strict direct public API parity 当前没有剩余 public wrapper 候选；不能外推为截图、交互、性能或人工验收完成。

## native/user-facing 100% 结论

当前未达到 LingDongGUI native/user-facing 100% 对外能力闭环。

- `178` 行 `allowlisted` 不是 PicoUI user-facing direct wrapper；它们只能证明 policy ledger 已处置，不能证明原生能力都对外公开。
- 每个 widget group 仍有 lifecycle/show 等 `policy_never_public` 行，因此逐控件页的结论仍是 `policy_complete_not_direct_100`。
- `background` 当前已补独立 PicoUI public widget，但 contract 三件套若仍保留旧的 enum-only/policy 口径，需要继续同步更新。
- `backend_proof` 当前是 ledger 证据标签；checker 会反查 `picoui_api` 是否在 public header 中存在，但尚未反查每个 `backend_proof` token 是否是真实 backend 符号或完整 backend 行为。

## 按控件能力等价仍需补齐

以下缺口按新规则判断：目标不是把 `ld*` API 名字逐个翻译成 `picoui_*`，而是以前用户能用 LingDongGUI 完成的控件能力，现在必须能用 PicoUI 完成。只要用户态能力不可达，就不能算 native/user-facing 100%。

| 缺口 | LingDongGUI 来源 | 当前 PicoUI 状态 | 需要补齐的能力 |
| --- | --- | --- | --- |


## 按控件能力等价已补齐

| 能力 | LingDongGUI 来源 | PicoUI 补齐状态 | 边界 |
| --- | --- | --- | --- |
| 动态移除/销毁控件 | `ldBaseNodeRemove`、各控件 `*_depose` | a-0.13 已新增 `picoui_widget_remove_from_parent()` 与 `picoui_widget_destroy()`，同步更新 PicoUI backend tree 与真实 `ldBase` tree，并覆盖 focus 清理、nameId 查找移除、child count 更新测试 | 当前 `destroy` 定义为用户态销毁绑定和 tree 脱离，不在本线释放所有 widget 外层内存；完整 allocator/free 所有权另线处理 |
| 按钮全局 action/nameId 状态 | `ldButtonActionInit`、`ldButtonActionIsPressById` | a-0.13 已新增 `picoui_button_get_pressed_by_name_id()` 与 `picoui_button_get_action_state_by_name_id()`，通过 PicoUI root/nameId 查询真实 button pressed/action 状态 | 提供按 `nameId` 查询 pressed/action 的用户态等价能力；不暴露 LingDongGUI `ld_scene_t` |
| keyboard 单键自定义绘制 | `ldKeyboardBtnUserDraw` | a-0.14 已新增 `picoui_keyboard_set_draw_callback()`，通过 keyboard custom button list/backend prepare 路径向用户暴露逐键 draw callback | 当前是 portable key draw callback，不直接暴露 Arm-2D tile/raw draw hook |
| background 独立 public widget | `widgetTypeBackground` | a-0.14 已新增 `picoui_background_create()` 与 `picoui_background_set_source/set_color/get_color/set_offset/get_offset`，并提供 app root 运行/切换入口 | native 仍复用真实 `ldWindow` root/background 语义，不新增 fake renderer |

## a-0.9 policy schema

当前 contract truth 已把 `619` 行 native API 全部纳入机器可校验 policy schema：

- `group_kind` 统计：`widget`: 513, `shared_base`: 63, `runtime_host`: 16, `internal_helper`: 19
- `policy_category` 统计：`direct_covered`: 441, `lifecycle_internal`: 112, `render_pipeline_internal`: 28, `runtime_host_internal`: 11, `layout_solver_internal`: 14, `memory_internal`: 5, `base_tree_policy`: 3, `backend_private_hook`: 1, `native_action_private`: 2, `enum_only_semantics`: 2
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
| `background` | [background](./background.md) | `无` | a-0.14 已新增独立 `picoui_background_*` public widget；native 仍复用真实 root/background `ldWindow` 语义。 |
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
| `keyboard` | [keyboard](./keyboard.md) | `keyboard` | `allowlisted`: 6, `covered`: 12 |
| `canvas` | [canvas](./canvas.md) | `canvas` | `allowlisted`: 5, `covered`: 3 |
| `animation` | [animation](./animation.md) | `animation` | `allowlisted`: 5, `covered`: 8 |
| `list` | [list](./list.md) | `list` | `allowlisted`: 5, `covered`: 19 |
| `message_box` | [message_box](./message_box.md) | `message_box` | `allowlisted`: 5, `covered`: 13 |
| `calendar` | [calendar](./calendar.md) | `calendar` | `allowlisted`: 5, `covered`: 7 |
| `progress_wheel` | [progress_wheel](./progress_wheel.md) | `progress_wheel` | `allowlisted`: 5, `covered`: 5 |
| `clock` | [clock](./clock.md) | `clock` | `allowlisted`: 5, `covered`: 7 |

## 其他原生 API 分组

| 分组 | API 条目数 | 能力文档 | PicoUI 覆盖摘要 |
| --- | --- | --- | --- |
| `gui` | 16 | [gui](./gui.md) | `allowlisted`: 11, `covered`: 5 |
| `mem` | 5 | [mem](./mem.md) | `allowlisted`: 5 |
| `switch_internal` | 6 | [switch_internal](./switch_internal.md) | `allowlisted`: 6 |
| `window_layout_internal` | 8 | [window_layout_internal](./window_layout_internal.md) | `allowlisted`: 8 |

## 共享能力

| 分组 | API 条目数 | 能力文档 | PicoUI 覆盖摘要 |
| --- | --- | --- | --- |
| `base` | 63 | [base](./base.md) | `allowlisted`: 3, `covered`: 60；严格 100% direct public API 缺口候选：`0` |

## 可靠性说明

- 本目录不使用人工摘要判断 PicoUI 100%。
- 每个 group 页按 LingDongGUI symbol 逐行列出 PicoUI 状态、PicoUI API、backend proof、unit/gate 和 allowlist 原因；其中 `picoui_api` 会被 checker 反查 public header，`backend_proof` 仍是 ledger 证据标签，尚未被 checker 逐项反查为真实 backend 符号或完整行为。
- `manual_artifact` 只表示 artifact/catalog/frame 证据；未人工复核时不能当人工验收通过。
- 若 inventory 或 matrix 更新，本目录必须同步更新，并重新跑 native API exhaustiveness checker。
