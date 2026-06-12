# `clock`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`clock`
- group_kind：`widget`
- LingDongGUI API 条目数：`12`
- LingDongGUI 分类统计：`init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 5, `show`: 1
- TinyUI 当前覆盖统计：`allowlisted`: 5, `covered`: 7
- policy_category 统计：`direct_covered`: 7, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`7`
- policy_allowlisted：`5`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 7, `policy_never_public`: 5
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldClock.h`

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldClockInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_clock_init` | `picoui_backend_clock_init` | test_clock_init_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `#define ldClockInit(nameId,parentNameId,x,y,width,height) ldClock_init(ptScene,NULL,nameId,parentNameId,x,y,width,height)` |
| 2 | `ldClockSetBackgroundImage` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_background_image` | `picoui_backend_clock_set_background_image` | test_clock_set_background_image_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldClockSetBackgroundImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor);` |
| 3 | `ldClockSetHourPointerImage` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_hour_pointer_image` | `picoui_backend_clock_set_hour_pointer_image` | test_clock_set_hour_pointer_image_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldClockSetHourPointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 4 | `ldClockSetMinutePointerImage` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_minute_pointer_image` | `picoui_backend_clock_set_minute_pointer_image` | test_clock_set_minute_pointer_image_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldClockSetMinutePointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 5 | `ldClockSetSecondPointerImage` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_second_pointer_image` | `picoui_backend_clock_set_second_pointer_image` | test_clock_set_second_pointer_image_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldClockSetSecondPointerImage(ldClock_t *ptWidget, arm_2d_tile_t *ptImgTile, arm_2d_tile_t *ptMaskTile, ldColor maskColor, float x, float y);` |
| 6 | `ldClockSetStepSecond` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `picoui_clock_set_step_second` | `picoui_backend_clock_set_step_second` | test_clock_set_step_second_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `void ldClockSetStepSecond(ldClock_t *ptWidget, bool isStepSecond);` |
| 7 | `ldClock_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_depose(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 8 | `ldClock_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_clock_init` | `picoui_backend_clock_init` | test_clock_init_native_parity, ctest:test_picoui_date_time|test_picoui_clock | R5 fourth batch closed date_time/clock rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API 字段在 public header 中存在；backend proof 当前为 ledger 证据标签，尚未被 checker 反查为真实 backend 符号或完整行为。 | `ldClock_t* ldClock_init(ld_scene_t *ptScene, ldClock_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height);` |
| 9 | `ldClock_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_frame_complete(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 10 | `ldClock_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_frame_start(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 11 | `ldClock_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_on_load(ld_scene_t *ptScene, ldClock_t *ptWidget);` |
| 12 | `ldClock_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldClock_show(ld_scene_t *pScene, ldClock_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 current public API/backend/unit/gate 证据；`picoui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
