# `keyboard`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`keyboard`
- group_kind：`widget`
- LingDongGUI API 条目数：`18`
- LingDongGUI 分类统计：`callback_hook`: 2, `capability`: 2, `getter`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 4, `show`: 1, `update_action`: 3
- PicoUI 覆盖统计：`allowlisted`: 8, `covered`: 10
- policy_category 统计：`backend_private_hook`: 3, `direct_covered`: 10, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`10`
- policy_allowlisted：`8`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 10, `policy_never_public`: 8
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖；`covered` 行有 PicoUI API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldKeyboard.h`

## 控件能力等价缺口

本节按用户能否用 PicoUI 完成同等键盘能力判断，不要求一比一公开 LingDongGUI API 名称。

| 缺口 | LingDongGUI 来源 | 当前 PicoUI 状态 | 需要补齐的能力 |
| --- | --- | --- | --- |
| 自定义键盘布局/按钮表 | `ldKeyboardGetTargetBtnList` | 当前按 render/lifecycle/helper policy allowlist，PicoUI 缺少便携按键表 API | `picoui_keyboard_set_layout()` 或等价 layout API，能表达按键行列、显示文本、键值、禁用/特殊键 |
| 按键事件回调 | `ldKeyboardCallback` | 缺少 PicoUI user-facing key event callback | `picoui_keyboard_set_on_key_event()` 或统一事件系统中的 keyboard key event |
| 单键自定义绘制 | `ldKeyboardBtnUserDraw` | 当前归入 raw/custom drawing 缺口 | 若保留用户自绘能力，需要接入 `picoui_canvas` 或 keyboard key draw callback；若不保留，必须证明同等视觉能力可由主题/样式完成 |

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldKeyboardBtnUpdate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `picoui_keyboard_button_update` | `picoui_backend_keyboard_btn_update` | test_keyboard_update_and_button_update_touch_native_state, ctest:test_picoui_keyboard | R4 line_edit/keyboard first batch closed with concrete PicoUI API, backend proof, and focused unit coverage. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldKeyboardBtnUpdate(ldKeyboard_t *ptWidget,uint8_t keyCode);` |
| 2 | `ldKeyboardBtnUserDraw` | `callback_hook` | `widget` | `backend_private_hook` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  | test_keyboard_weak_hooks_remain_backend_private_not_public_api, ctest:test_picoui_keyboard | R4 policy decision: raw Arm-2D keyboard button draw callback is non-portable; PicoUI exposes style/theme customization rather than native tile draw hooks. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `extern bool ldKeyboardBtnUserDraw(arm_2d_tile_t *ptTile, ldKeyboard_t *ptWidget, kbBtnInfo_t *pBtnInfo);` |
| 3 | `ldKeyboardCallback` | `callback_hook` | `widget` | `backend_private_hook` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  | test_keyboard_weak_hooks_remain_backend_private_not_public_api, ctest:test_picoui_keyboard | R4 policy decision: native weak callback is backend-private; PicoUI user intent is covered by keyboard input/navigation APIs and widget event abstraction. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `extern void ldKeyboardCallback(ldKeyboard_t *ptWidget, uint8_t signal);` |
| 4 | `ldKeyboardClick` | `capability` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `picoui_keyboard_click` | `picoui_backend_keyboard_click` | test_keyboard_click_respects_focus_owner, ctest:test_picoui_keyboard | R4 line_edit/keyboard first batch closed with concrete PicoUI API, backend proof, and focused unit coverage. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldKeyboardClick(ld_scene_t *ptScene,ldKeyboard_t *ptWidget,uint8_t signal);` |
| 5 | `ldKeyboardExit` | `capability` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `picoui_keyboard_exit` | `picoui_backend_keyboard_exit` | test_keyboard_exit_clears_focus_or_edit_session, ctest:test_picoui_keyboard | R4 line_edit/keyboard first batch closed with concrete PicoUI API, backend proof, and focused unit coverage. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldKeyboardExit(ldKeyboard_t *ptWidget);` |
| 6 | `ldKeyboardGetTargetBtnList` | `getter` | `widget` | `backend_private_hook` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  | test_keyboard_weak_hooks_remain_backend_private_not_public_api, ctest:test_picoui_keyboard | R4 policy decision: target button list contains native kbBtnInfo_t layout/tile details; PicoUI keeps layout backend-owned and exposes portable keyboard input/navigation APIs instead. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `extern const kbBtnInfo_t *ldKeyboardGetTargetBtnList(ldKeyboard_t *ptWidget);` |
| 7 | `ldKeyboardInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_keyboard_create` | `picoui_backend_create_keyboard` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_picoui_line_edit|test_picoui_keyboard|test_picoui_combo_box|test_picoui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldKeyboardInit(nameId,parentNameId,ptFont) ldKeyboard_init(ptScene,NULL,nameId,parentNameId,ptFont)` |
| 8 | `ldKeyboardMove` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_widget_set_pos` | `picoui_widget_set_pos` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_picoui_line_edit|test_picoui_keyboard|test_picoui_combo_box|test_picoui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldKeyboardMove ldBaseMove` |
| 9 | `ldKeyboardNavigate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `picoui_keyboard_navigate` | `picoui_backend_keyboard_navigate` | test_keyboard_navigation_signal_respects_focus_owner, ctest:test_picoui_keyboard | R4 line_edit/keyboard first batch closed with concrete PicoUI API, backend proof, and focused unit coverage. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldKeyboardNavigate(ldKeyboard_t *ptWidget, ldNavDir_t dir);` |
| 10 | `ldKeyboardSetHidden` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_widget_set_visible` | `picoui_widget_set_visible` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_picoui_line_edit|test_picoui_keyboard|test_picoui_combo_box|test_picoui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldKeyboardSetHidden ldBaseSetHidden` |
| 11 | `ldKeyboardSetOpacity` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_widget_set_opacity` | `picoui_widget_set_opacity` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_picoui_line_edit|test_picoui_keyboard|test_picoui_combo_box|test_picoui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldKeyboardSetOpacity ldBaseSetOpacity` |
| 12 | `ldKeyboardUpdate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `picoui_keyboard_update` | `picoui_backend_keyboard_update` | test_keyboard_update_and_button_update_touch_native_state, ctest:test_picoui_keyboard | R4 line_edit/keyboard first batch closed with concrete PicoUI API, backend proof, and focused unit coverage. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldKeyboardUpdate(ldKeyboard_t *ptWidget);` |
| 13 | `ldKeyboard_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldKeyboard_depose(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 14 | `ldKeyboard_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_keyboard_create` | `picoui_backend_create_keyboard` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_picoui_line_edit|test_picoui_keyboard|test_picoui_combo_box|test_picoui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `ldKeyboard_t* ldKeyboard_init(ld_scene_t *ptScene, ldKeyboard_t *ptWidget, uint16_t nameId, uint16_t parentNameId, arm_2d_font_t *ptFont);` |
| 15 | `ldKeyboard_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldKeyboard_on_frame_complete(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 16 | `ldKeyboard_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldKeyboard_on_frame_start(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 17 | `ldKeyboard_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldKeyboard_on_load(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 18 | `ldKeyboard_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldKeyboard_show(ld_scene_t *pScene, ldKeyboard_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 PicoUI public API/backend/unit/gate 证据；`picoui_api` 会被 checker 反查 `picoui/include` public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作 PicoUI direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为 PicoUI public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
