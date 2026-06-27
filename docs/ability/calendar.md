# `calendar`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`calendar`
- group_kind：`widget`
- LingDongGUI API 条目数：`13`
- LingDongGUI 分类统计：`getter`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 5, `show`: 1
- TinyUI 当前覆盖统计：`allowlisted`: 5, `covered`: 8
- policy_category 统计：`direct_covered`: 8, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`8`
- policy_allowlisted：`5`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 8, `policy_never_public`: 5
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/unit/runtime/visible gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldCalendar.h`

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | TinyUI 状态 | 覆盖类型 | TinyUI API | 历史证据字段 | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldCalendarGetDate` | `getter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_getter_parity` | `tinyui_calendar_get_date` | `tinyui_runtime_evidence_calendar_get_date` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldCalendarGetDate(ldCalendar_t *ptWidget,uint16_t* year,uint8_t* month,uint8_t* day);` |
| 2 | `ldCalendarInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_calendar_create` | `tinyui_runtime_evidence_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldCalendarInit(nameId,parentNameId,x,y,width,height,ptFont,year,month,day) ldCalendar_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,ptFont,year,month,day)` |
| 3 | `ldCalendarSetDate` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_calendar_set_date` | `tinyui_runtime_evidence_calendar_set_date` | test_calendar_date_readback_matches_backend_truth, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldCalendarSetDate(ldCalendar_t *ptWidget,uint16_t year,uint8_t month,uint8_t day);` |
| 4 | `ldCalendarSetDayNames` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_calendar_set_day_names` | `tinyui_runtime_evidence_calendar_set_day_names` | test_calendar_native_day_names_and_colors_round_trip, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldCalendarSetDayNames(ldCalendar_t *ptWidget,uint8_t* names[7]);` |
| 5 | `ldCalendarSetHeader` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_calendar_set_header_visible` | `tinyui_runtime_evidence_calendar_set_header_visible` | test_calendar_final_release_contract_covers_full_feature_boundary, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldCalendarSetHeader(ldCalendar_t *ptWidget,bool isEnable);` |
| 6 | `ldCalendarSetHeaderFormat` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_calendar_set_header_format` | `tinyui_runtime_evidence_calendar_set_header_format` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldCalendarSetHeaderFormat(ldCalendar_t *ptWidget,uint8_t* format);` |
| 7 | `ldCalendarSetAutoSysDate` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_calendar_set_auto_sys_date` | `tinyui_runtime_evidence_calendar_set_auto_sys_date` | test_calendar_set_auto_sys_date_native_parity, ctest:test_tinyui_graph|test_tinyui_calendar | R6 closed calendar auto-sys-date parity: tinyui_calendar_set_auto_sys_date alias added; forwards to tinyui_calendar_set_use_system_date. | `void ldCalendarSetAutoSysDate(ldCalendar_t *ptWidget, bool isAutoSysDate);` |
| 8 | `ldCalendar_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldCalendar_depose(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 9 | `ldCalendar_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_calendar_init` | `tinyui_runtime_evidence_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_tinyui_graph|test_tinyui_calendar | R4 fourth batch graph/calendar rows closed with concrete TinyUI API or shared equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `ldCalendar_t* ldCalendar_init(ld_scene_t *ptScene, ldCalendar_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, arm_2d_font_t *ptFont, uint16_t year, uint8_t month, uint8_t day);` |
| 10 | `ldCalendar_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldCalendar_on_frame_complete(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 11 | `ldCalendar_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldCalendar_on_frame_start(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 12 | `ldCalendar_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldCalendar_on_load(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 13 | `ldCalendar_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by rendering pipeline. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldCalendar_show(ld_scene_t *pScene, ldCalendar_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 current public API/unit/runtime/visible gate 证据；`tinyui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
