# `calendar`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`calendar`
- LingDongGUI API 条目数：`12`
- LingDongGUI 分类统计：`getter`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 4, `show`: 1
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 7
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldCalendar.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldCalendarGetDate` | `getter` | `true` | `covered` | `native_getter_parity` | `picoui_calendar_get_date` | `picoui_backend_calendar_get_date` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldCalendarGetDate(ldCalendar_t *ptWidget,uint16_t* year,uint8_t* month,uint8_t* day);` |
| 2 | `ldCalendarInit` | `macro_alias` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_create` | `picoui_backend_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `#define ldCalendarInit(nameId,parentNameId,x,y,width,height,ptFont,year,month,day) ldCalendar_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,ptFont,year,month,day)` |
| 3 | `ldCalendarSetDate` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_date` | `picoui_backend_calendar_set_date` | test_calendar_date_readback_matches_backend_truth, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldCalendarSetDate(ldCalendar_t *ptWidget,uint16_t year,uint8_t month,uint8_t day);` |
| 4 | `ldCalendarSetDayNames` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_day_names` | `picoui_backend_calendar_set_day_names` | test_calendar_native_day_names_and_colors_round_trip, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldCalendarSetDayNames(ldCalendar_t *ptWidget,uint8_t* names[7]);` |
| 5 | `ldCalendarSetHeader` | `setter` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_set_header_visible` | `picoui_backend_calendar_set_header_visible` | test_calendar_final_release_contract_covers_full_feature_boundary, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldCalendarSetHeader(ldCalendar_t *ptWidget,bool isEnable);` |
| 6 | `ldCalendarSetHeaderFormat` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_calendar_set_header_format` | `picoui_backend_calendar_set_header_format` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldCalendarSetHeaderFormat(ldCalendar_t *ptWidget,uint8_t* format);` |
| 7 | `ldCalendar_depose` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_depose(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 8 | `ldCalendar_init` | `init` | `true` | `covered` | `shared_api_equivalence` | `picoui_calendar_init` | `picoui_backend_create_calendar` | test_calendar_init_and_aliases_match_backend_truth, ctest:test_picoui_graph\|test_picoui_calendar | R4 fourth batch graph/calendar rows closed with concrete PicoUI API or shared equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `ldCalendar_t* ldCalendar_init(ld_scene_t *ptScene, ldCalendar_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, arm_2d_font_t *ptFont, uint16_t year, uint8_t month, uint8_t day);` |
| 9 | `ldCalendar_on_frame_complete` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_frame_complete(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 10 | `ldCalendar_on_frame_start` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_frame_start(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 11 | `ldCalendar_on_load` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_on_load(ld_scene_t *ptScene, ldCalendar_t *ptWidget);` |
| 12 | `ldCalendar_show` | `show` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldCalendar_show(ld_scene_t *pScene, ldCalendar_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
