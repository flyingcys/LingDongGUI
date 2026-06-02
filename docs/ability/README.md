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
- PicoUI 覆盖统计：`allowlisted`: 191, `covered`: 420
- `covered` 才表示有 PicoUI API/backend/unit/gate 证据；`allowlisted` 表示已纳入 ledger 但不是 PicoUI user-facing direct wrapper 覆盖。
- 因此当前不能笼统写“PicoUI 100% direct 覆盖 LingDongGUI 全部原生 API”；应逐 group 看 covered/allowlisted。

## a-0.12 strict direct-wrapper 候选清零结论

当前只达到 a-0.12 定义的 strict direct-wrapper 候选清零；这不是 LingDongGUI native/user-facing 100% 对外能力闭环。

- `covered=420`：有真实 PicoUI public API/backend/unit/gate 证据。
- `allowlisted=191`：policy ledger 已闭环，且均为 `policy_never_public`。
- `missing_gap_total=0` 只表示没有未建账 native API，不表示 direct 100%。
- `direct_100_category` 统计：`policy_never_public`: 191
- 严格按“100% direct public API parity”目标看，a-0.12 已把 `base` 的 `optional_public_extension=16` 候选收敛到 `0`。
- `direct_public_100_complete=true` 只表示 strict direct public API parity 当前没有剩余 public wrapper 候选；不能外推为截图、交互、性能或人工验收完成。

## native/user-facing 100% 结论

当前未达到 LingDongGUI native/user-facing 100% 对外能力闭环。

- `191` 行 `allowlisted` 不是 PicoUI user-facing direct wrapper；它们只能证明 policy ledger 已处置，不能证明原生能力都对外公开。
- 每个 widget group 仍有 lifecycle/show 等 `policy_never_public` 行，因此逐控件页的结论仍是 `policy_complete_not_direct_100`。
- `background` 没有独立 PicoUI public widget group；当前由 window/root/background source/color 语义承载。如果目标是每个 `ldWidgetType_t` 都有独立 PicoUI public widget，这仍是缺口。
- `backend_proof` 当前是 ledger 证据标签；checker 会反查 `picoui_api` 是否在 public header 中存在，但尚未反查每个 `backend_proof` token 是否是真实 backend 符号或完整 backend 行为。

## 按控件能力等价仍需补齐

以下缺口按新规则判断：目标不是把 `ld*` API 名字逐个翻译成 `picoui_*`，而是以前用户能用 LingDongGUI 完成的控件能力，现在必须能用 PicoUI 完成。只要用户态能力不可达，就不能算 native/user-facing 100%。

| 缺口 | LingDongGUI 来源 | 当前 PicoUI 状态 | 需要补齐的能力 |
| --- | --- | --- | --- |
| 动态移除/销毁控件 | `ldBaseNodeRemove`、各控件 `*_depose` | 当前作为 lifecycle/tree policy allowlist，缺少用户态销毁/移除入口 | `picoui_widget_destroy()` 或 `picoui_widget_remove_from_parent()` 等可移除真实 backend tree 节点的能力，并覆盖资源释放与测试 |
| 键盘自定义布局/按钮表 | `ldKeyboardGetTargetBtnList` | `keyboard` 仅覆盖部分属性和输入态，缺少便携 layout/button list | `picoui_keyboard_set_layout()` 等可表达按键集合、行列、显示文本和值的能力 |
| 键盘按键事件回调 | `ldKeyboardCallback` | 缺少 PicoUI 对外按键事件钩子 | `picoui_keyboard_set_on_key_event()` 或等价事件能力 |
| 按钮全局 action/nameId 状态 | `ldButtonActionInit`、`ldButtonActionIsPressById` | 仅有单 widget 状态与事件能力，缺少按 `nameId` 查询动作状态 | 按 id 查询 pressed/action，或通用 typed lookup 后查询状态 |
| VRES 图片/字体资源 | `ldBaseGetVresImage`、`ldBaseGetVresFont` | 当前作为 resource helper policy allowlist，缺少用户态资源源描述 | `picoui_image_source_from_vres()`、`picoui_font_from_vres()` 或等价资源 provider |
| 系统时间/日期/星期 | `ldBaseGetTime`、`ldBaseGetDate`、`ldBaseGetWeek` | 当前作为 time helper policy allowlist，缺少 PicoUI app/time provider | `picoui_time_now()`、`picoui_date_now()`、`picoui_weekday()` 或等价 host provider |
| 页面/场景切换 | `ldGuiJumpPage*`、`__ldGuiJumpPage` | runtime host policy allowlist，缺少用户态 page/window switch | `picoui_app_set_window()`、`picoui_app_switch_window(mode, ms)` 或等价页面切换能力 |
| background 独立控件与背景移动 | `widgetTypeBackground`、`ldBaseBgMove` | 无独立 `picoui_background_*`；背景移动只按 backend helper allowlist | 明确是否新增独立 background 控件；至少补齐 window/background pan/move 能力 |
| 自定义绘制/基础绘图 | `ldBaseColor`、`ldBaseDrawLine`、`ldBaseImage`、`ldBaseImageScale`、`ldBaseLabel`、`ldKeyboardBtnUserDraw` | 当前 raw drawing helper/render hook allowlist，缺少用户态 custom draw/canvas | `picoui_canvas` 或 custom widget draw callback，能画线、填色、图像、文字并参与真实 backend 渲染 |

## a-0.9 policy schema

`a-0.9` 已把 `611` 行 native API 全部纳入机器可校验 policy schema：

- `group_kind` 统计：`widget`: 513, `shared_base`: 63, `runtime_host`: 16, `internal_helper`: 19
- `policy_category` 统计：`direct_covered`: 420, `lifecycle_internal`: 108, `render_pipeline_internal`: 27, `runtime_host_internal`: 16, `layout_solver_internal`: 14, `memory_internal`: 5, `base_tree_policy`: 4, `resource_time_helper_policy`: 5, `drawing_helper_policy`: 5, `backend_private_hook`: 3, `native_action_private`: 2, `enum_only_semantics`: 2
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
| `background` | [background](./background.md) | `无` | 无独立 matrix group；由 window/root/background source/color 语义承载。若要求每个 `ldWidgetType_t` 都有独立 PicoUI public widget，则仍是缺口。 |
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
| `base` | 63 | [base](./base.md) | `allowlisted`: 14, `covered`: 49；严格 100% direct public API 缺口候选：`0` |

## 可靠性说明

- 本目录不使用人工摘要判断 PicoUI 100%。
- 每个 group 页按 LingDongGUI symbol 逐行列出 PicoUI 状态、PicoUI API、backend proof、unit/gate 和 allowlist 原因；其中 `picoui_api` 会被 checker 反查 public header，`backend_proof` 仍是 ledger 证据标签，尚未被 checker 逐项反查为真实 backend 符号或完整行为。
- `manual_artifact` 只表示 artifact/catalog/frame 证据；未人工复核时不能当人工验收通过。
- 若 inventory 或 matrix 更新，本目录必须同步更新，并重新跑 native API exhaustiveness checker。
