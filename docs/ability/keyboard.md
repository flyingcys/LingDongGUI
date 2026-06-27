# `keyboard`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`keyboard`
- group_kind：`widget`
- LingDongGUI API 条目数：`18`
- LingDongGUI 分类统计：`callback_hook`: 2, `capability`: 2, `getter`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 4, `show`: 1, `update_action`: 3
- TinyUI 当前覆盖统计：`allowlisted`: 6, `covered`: 12
- policy_category 统计：`runtime_private_hook`: 1, `direct_covered`: 12, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`12`
- policy_allowlisted：`6`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 12, `policy_never_public`: 6
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/unit/runtime/visible gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldKeyboard.h`

## 控件能力等价已补齐

本节按用户能否用 TinyUI 完成同等键盘能力判断，不要求一比一公开 LingDongGUI API 名称。

| 能力 | LingDongGUI 来源 | 当前补齐状态 | 边界 |
| --- | --- | --- | --- |
| 自定义按钮表/布局 | `ldKeyboardGetTargetBtnList` | a-0.13 已通过 `tinyui_keyboard_set_layout()` 暴露 portable custom button list/layout 能力 | 不直接暴露 native `kbBtnInfo_t` |
| 按键事件回调 | `ldKeyboardCallback` | a-0.13 已通过 `tinyui_keyboard_set_on_key_event()` 暴露 portable key event callback | 不直接暴露 native weak callback 签名 |
| 单键自定义绘制 | `ldKeyboardBtnUserDraw` | a-0.14 已通过 `tinyui_keyboard_set_draw_callback()` 暴露逐键 draw callback，并经 runtime custom button list prepare 路径触发 | 当前是 portable key draw callback，不直接暴露 Arm-2D tile/raw draw hook |

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | TinyUI 状态 | 覆盖类型 | TinyUI API | 历史证据字段 | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldKeyboardBtnUpdate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_keyboard_button_update` | `tinyui_keyboard_button_update widget-local native path` | test_keyboard_update_and_button_update_touch_native_state, ctest:test_tinyui_keyboard | R4 line_edit/keyboard first batch已继续收口到 widget-local/native 路径；当前 focused unit test 直接验证 TinyUI public API 对 native `keyCode/isKeySelect` 的写入，不再依赖独立 runtime 符号存在。 | `void ldKeyboardBtnUpdate(ldKeyboard_t *ptWidget,uint8_t keyCode);` |
| 2 | `ldKeyboardBtnUserDraw` | `callback_hook` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_keyboard_set_draw_callback` | `tinyui_runtime_evidence_keyboard_get_custom_button_list` | test_keyboard_draw_callback_round_trip, ctest:test_tinyui_keyboard | a-0.14 已通过 portable key draw callback 覆盖用户逐键自绘能力；native Arm-2D tile/raw draw hook 签名本身仍是 runtime 实现细节。 | `extern bool ldKeyboardBtnUserDraw(arm_2d_tile_t *ptTile, ldKeyboard_t *ptWidget, kbBtnInfo_t *pBtnInfo);` |
| 3 | `ldKeyboardCallback` | `callback_hook` | `widget` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `tinyui_keyboard_set_on_key_event` | `tinyui_runtime_evidence_keyboard_dispatch_event_callback` | test_keyboard_key_event_callback_receives_native_press_release, ctest:test_tinyui_keyboard | a-0.13 item 2 已通过 portable TinyUI key event callback 覆盖用户态按键事件能力；native weak callback 签名本身仍是 runtime 实现细节。 | `extern void ldKeyboardCallback(ldKeyboard_t *ptWidget, uint8_t signal);` |
| 4 | `ldKeyboardClick` | `capability` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_keyboard_click` | `tinyui_keyboard_click widget-local native path` | test_keyboard_click_respects_focus_owner, ctest:test_tinyui_keyboard | 当前 keyboard click 真相已落到 `tinyui/src/widgets/keyboard.c` 的 widget-local/native 路径；focused unit test 直接验证 focus gate 和 native click 行为，不再依赖独立 runtime 符号存在。 | `void ldKeyboardClick(ld_scene_t *ptScene,ldKeyboard_t *ptWidget,uint8_t signal);` |
| 5 | `ldKeyboardExit` | `capability` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_keyboard_exit` | `tinyui_keyboard_exit widget-local native path` | test_keyboard_exit_clears_focus_or_edit_session, ctest:test_tinyui_keyboard | 当前 keyboard exit 真相已落到 `tinyui/src/widgets/keyboard.c` 的 widget-local/native 路径；focused unit test 直接验证编辑态/焦点释放合同，不再依赖独立 runtime 符号存在。 | `void ldKeyboardExit(ldKeyboard_t *ptWidget);` |
| 6 | `ldKeyboardGetTargetBtnList` | `getter` | `widget` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `tinyui_keyboard_set_layout` | `tinyui_keyboard_set_layout + widget-local custom layout prepare` | test_keyboard_custom_layout_button_table_round_trip, ctest:test_tinyui_keyboard | a-0.13 item 1 已通过 portable keyboard layout API 覆盖自定义按钮表/布局用户意图；当前 native button table prepare 真相已留在 widget-local 路径，不再依赖独立 runtime 符号。 | `extern const kbBtnInfo_t *ldKeyboardGetTargetBtnList(ldKeyboard_t *ptWidget);` |
| 7 | `ldKeyboardInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_keyboard_create` | `tinyui_runtime_evidence_create_keyboard` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldKeyboardInit(nameId,parentNameId,ptFont) ldKeyboard_init(ptScene,NULL,nameId,parentNameId,ptFont)` |
| 8 | `ldKeyboardMove` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_widget_set_pos` | `tinyui_widget_set_pos` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldKeyboardMove ldBaseMove` |
| 9 | `ldKeyboardNavigate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_keyboard_navigate` | `tinyui_keyboard_navigate widget-local native path` | test_keyboard_navigation_signal_respects_focus_owner, ctest:test_tinyui_keyboard | 当前 keyboard navigate 真相已落到 `tinyui/src/widgets/keyboard.c` 的 widget-local/native 路径；focused unit test 直接验证 focus gate 和 native key selection 变化，不再依赖独立 runtime 符号存在。 | `void ldKeyboardNavigate(ldKeyboard_t *ptWidget, ldNavDir_t dir);` |
| 10 | `ldKeyboardSetHidden` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_widget_set_visible` | `tinyui_widget_set_visible` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldKeyboardSetHidden ldBaseSetHidden` |
| 11 | `ldKeyboardSetOpacity` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_widget_set_opacity` | `tinyui_widget_set_opacity` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldKeyboardSetOpacity ldBaseSetOpacity` |
| 12 | `ldKeyboardUpdate` | `update_action` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_keyboard_update` | `tinyui_keyboard_update widget-local native path` | test_keyboard_update_and_button_update_touch_native_state, ctest:test_tinyui_keyboard | 当前 keyboard update 真相已落到 `tinyui/src/widgets/keyboard.c` 的 widget-local/native 路径；focused unit test 直接验证 TinyUI public API 会刷新 native button list / wait-init 状态，不再依赖独立 runtime 符号存在。 | `void ldKeyboardUpdate(ldKeyboard_t *ptWidget);` |
| 13 | `ldKeyboard_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldKeyboard_depose(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 14 | `ldKeyboard_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_keyboard_create` | `tinyui_runtime_evidence_create_keyboard` | test_keyboard_init_and_shared_base_aliases_round_trip, ctest:test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter | R4 third batch closed by shared base API equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `ldKeyboard_t* ldKeyboard_init(ld_scene_t *ptScene, ldKeyboard_t *ptWidget, uint16_t nameId, uint16_t parentNameId, arm_2d_font_t *ptFont);` |
| 15 | `ldKeyboard_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldKeyboard_on_frame_complete(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 16 | `ldKeyboard_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldKeyboard_on_frame_start(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 17 | `ldKeyboard_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldKeyboard_on_load(ld_scene_t *ptScene, ldKeyboard_t *ptWidget);` |
| 18 | `ldKeyboard_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by rendering pipeline. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldKeyboard_show(ld_scene_t *pScene, ldKeyboard_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 current public API/unit/runtime/visible gate 证据；`tinyui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
