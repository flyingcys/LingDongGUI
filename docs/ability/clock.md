# `clock`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`clock`
- LingDongGUI API 条目数：`12`
- LingDongGUI 分类统计：`init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 5, `show`: 1
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 7
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldClock.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldClockInit` | `macro_alias` | `true` | `covered` | `shared_api_equivalence` | `picoui_clock_init` | `picoui_backend_clock_init` | test_clock_init_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `#define ldClockInit(nameId,parentNameId,x,y,width,height) ldClock_init(ptScene,NULL,nameId,parentNameId,x,y,width,height)` |
| 2 | `ldClockSetBackgroundImage` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_background_image` | `picoui_backend_clock_set_background_image` | test_clock_set_background_image_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldClockSetBackgroundImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor);` |
| 3 | `ldClockSetHourPointerImage` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_hour_pointer_image` | `picoui_backend_clock_set_hour_pointer_image` | test_clock_set_hour_pointer_image_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldClockSetHourPointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 4 | `ldClockSetMinutePointerImage` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_minute_pointer_image` | `picoui_backend_clock_set_minute_pointer_image` | test_clock_set_minute_pointer_image_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldClockSetMinutePointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 5 | `ldClockSetSecondPointerImage` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_second_pointer_image` | `picoui_backend_clock_set_second_pointer_image` | test_clock_set_second_pointer_image_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldClockSetSecondPointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 6 | `ldClockSetStepSecond` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_step_second` | `picoui_backend_clock_set_step_second` | test_clock_set_step_second_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldClockSetStepSecond(ldClock_t *ptWidget, bool isStepSecond);` |
| 7 | `ldClock_depose` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_depose(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 8 | `ldClock_init` | `init` | `true` | `covered` | `shared_api_equivalence` | `picoui_clock_init` | `picoui_backend_clock_init` | test_clock_init_native_parity, ctest:test_picoui_date_time\|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `ldClock_t* ldClock_init(ld_scene_t *ptScene, ldClock_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height);` |
| 9 | `ldClock_on_frame_complete` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_frame_complete(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 10 | `ldClock_on_frame_start` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_frame_start(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 11 | `ldClock_on_load` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_load(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 12 | `ldClock_show` | `show` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_show(ld_scene_t *pScene, ldClock_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
