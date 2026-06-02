# `calendar`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`calendar`
- group_kind：`widget`
- LingDongGUI API 条目数：`12`
- LingDongGUI 分类统计：`getter`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 4, `show`: 1
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 7
- policy_category 统计：`direct_covered`: 7, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`7`
- policy_allowlisted：`5`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 7, `policy_never_public`: 5
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖；`covered` 行有 PicoUI API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldCalendar.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldCalendarGetDate` | `getter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_getter_parity` | `picoui_calendar_get_date` | `picoui_backend_calendar_get_date` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldCalendarGetDate(ldCalendar_t *ptWidget,uint16_t* year,uint8_t* month,uint8_t* day);` |
| 2 | `ldCalendarInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_create` | `picoui_backend_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldCalendarInit(nameId,parentNameId,x,y,width,height,ptFont,year,month,day) ldCalendar_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,ptFont,year,month,day)` |
| 3 | `ldCalendarSetDate` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_date` | `picoui_backend_calendar_set_date` | test_calendar_date_readback_matches_backend_truth, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldCalendarSetDate(ldCalendar_t *ptWidget,uint16_t year,uint8_t month,uint8_t day);` |
| 4 | `ldCalendarSetDayNames` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_day_names` | `picoui_backend_calendar_set_day_names` | test_calendar_native_day_names_and_colors_round_trip, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldCalendarSetDayNames(ldCalendar_t *ptWidget,uint8_t* names[7]);` |
| 5 | `ldCalendarSetHeader` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_set_header_visible` | `picoui_backend_calendar_set_header_visible` | test_calendar_final_release_contract_covers_full_feature_boundary, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldCalendarSetHeader(ldCalendar_t *ptWidget,bool isEnable);` |
| 6 | `ldCalendarSetHeaderFormat` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_header_format` | `picoui_backend_calendar_set_header_format` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldCalendarSetHeaderFormat(ldCalendar_t *ptWidget,uint8_t* format);` |
| 7 | `ldCalendar_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_depose(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 8 | `ldCalendar_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_init` | `picoui_backend_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `ldCalendar_t* ldCalendar_init(ld_scene_t *ptScene, ldCalendar_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, arm_2d_font_t *ptFont, uint16_t year, uint8_t month, uint8_t day);` |
| 9 | `ldCalendar_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_frame_complete(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 10 | `ldCalendar_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_frame_start(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 11 | `ldCalendar_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_load(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 12 | `ldCalendar_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_show(ld_scene_t *pScene, ldCalendar_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 PicoUI public API/backend/unit/gate 证据；`picoui_api` 会被 checker 反查 `picoui/include` public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作 PicoUI direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为 PicoUI public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
