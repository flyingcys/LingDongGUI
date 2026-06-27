# `radial_menu`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`radial_menu`
- group_kind：`widget`
- LingDongGUI API 条目数：`11`
- LingDongGUI 分类统计：`capability`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 3, `show`: 1
- TinyUI 当前覆盖统计：`allowlisted`: 5, `covered`: 6
- policy_category 统计：`direct_covered`: 6, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- direct public API 100%：否；当前为 policy complete, not direct public 100%。
- direct_public_covered：`6`
- policy_allowlisted：`5`
- direct_100_gap：`0`
- direct_100_category 统计：`direct_public_covered`: 6, `policy_never_public`: 5
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/unit/runtime/visible gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldRadialMenu.h`

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | TinyUI 状态 | 覆盖类型 | TinyUI API | 历史证据字段 | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldRadialMenuAddItem` | `capability` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `direct_field_parity` | `tinyui_radial_menu_add_item` | `tinyui_runtime_evidence_radial_menu_add_item` | test_radial_menu_add_item_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldRadialMenuAddItem(ldRadialMenu_t *ptWidget,arm_2d_tile_t *ptImgTile,arm_2d_tile_t *ptMaskTile);` |
| 2 | `ldRadialMenuInit` | `macro_alias` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_radial_menu_init` | `tinyui_runtime_evidence_radial_menu_init` | test_radial_menu_init_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `#define ldRadialMenuInit(nameId,parentNameId,x,y,width,height,xAxis,yAxis,itemMax) ldRadialMenu_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,xAxis,yAxis,itemMax)` |
| 3 | `ldRadialMenuSetClickItem` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_radial_menu_set_click_item` | `tinyui_runtime_evidence_radial_menu_set_click_item` | test_radial_menu_set_click_item_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldRadialMenuSetClickItem(ldRadialMenu_t *ptWidget,uint8_t num);` |
| 4 | `ldRadialMenuSetDefaultItem` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_radial_menu_set_default_item` | `tinyui_runtime_evidence_radial_menu_set_default_item` | test_radial_menu_set_default_item_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldRadialMenuSetDefaultItem(ldRadialMenu_t *ptWidget,uint8_t num);` |
| 5 | `ldRadialMenuSetOffsetItem` | `setter` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `native_setter_parity` | `tinyui_radial_menu_offset_item` | `tinyui_runtime_evidence_radial_menu_set_offset_item` | test_radial_menu_set_offset_item_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `void ldRadialMenuSetOffsetItem(ldRadialMenu_t *ptWidget,int8_t offset);` |
| 6 | `ldRadialMenu_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldRadialMenu_depose(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 7 | `ldRadialMenu_init` | `init` | `widget` | `direct_covered` | `direct_public_covered` | `true` | `covered` | `shared_api_equivalence` | `tinyui_radial_menu_init` | `tinyui_runtime_evidence_radial_menu_init` | test_radial_menu_init_native_parity, ctest:test_tinyui_icon_slider|test_tinyui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete TinyUI API or shared alias equivalence plus focused unit proof. 已校验：TinyUI API 字段在 public header 中存在；历史证据字段 当前为 ledger 证据标签，尚未被 checker 反查为底层行为 符号或完整行为。 | `ldRadialMenu_t* ldRadialMenu_init(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, uint16_t xAxis, uint16_t yAxis, uint8_t itemMax);` |
| 8 | `ldRadialMenu_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldRadialMenu_on_frame_complete(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 9 | `ldRadialMenu_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldRadialMenu_on_frame_start(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 10 | `ldRadialMenu_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `policy_never_public` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldRadialMenu_on_load(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 11 | `ldRadialMenu_show` | `show` | `widget` | `render_pipeline_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by rendering pipeline. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `void ldRadialMenu_show(ld_scene_t *pScene, ldRadialMenu_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有真实 current public API/unit/runtime/visible gate 证据；`tinyui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
