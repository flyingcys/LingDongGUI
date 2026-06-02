# `radial_menu`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`radial_menu`
- group_kind：`widget`
- LingDongGUI API 条目数：`11`
- LingDongGUI 分类统计：`capability`: 1, `init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 3, `show`: 1
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 6
- policy_category 统计：`direct_covered`: 6, `lifecycle_internal`: 4, `render_pipeline_internal`: 1
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖；`covered` 行有 PicoUI API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`policy_complete_not_direct_100` / `policy_complete`
- 来源 header：`src/gui/ldRadialMenu.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldRadialMenuAddItem` | `capability` | `widget` | `direct_covered` | `true` | `covered` | `direct_field_parity` | `picoui_radial_menu_add_item` | `picoui_backend_radial_menu_add_item` | test_radial_menu_add_item_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldRadialMenuAddItem(ldRadialMenu_t *ptWidget,arm_2d_tile_t *ptImgTile,arm_2d_tile_t *ptMaskTile);` |
| 2 | `ldRadialMenuInit` | `macro_alias` | `widget` | `direct_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_radial_menu_init` | `picoui_backend_radial_menu_init` | test_radial_menu_init_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `#define ldRadialMenuInit(nameId,parentNameId,x,y,width,height,xAxis,yAxis,itemMax) ldRadialMenu_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,xAxis,yAxis,itemMax)` |
| 3 | `ldRadialMenuSetClickItem` | `setter` | `widget` | `direct_covered` | `true` | `covered` | `native_setter_parity` | `picoui_radial_menu_set_click_item` | `picoui_backend_radial_menu_set_click_item` | test_radial_menu_set_click_item_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldRadialMenuSetClickItem(ldRadialMenu_t *ptWidget,uint8_t num);` |
| 4 | `ldRadialMenuSetDefaultItem` | `setter` | `widget` | `direct_covered` | `true` | `covered` | `native_setter_parity` | `picoui_radial_menu_set_default_item` | `picoui_backend_radial_menu_set_default_item` | test_radial_menu_set_default_item_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldRadialMenuSetDefaultItem(ldRadialMenu_t *ptWidget,uint8_t num);` |
| 5 | `ldRadialMenuSetOffsetItem` | `setter` | `widget` | `direct_covered` | `true` | `covered` | `native_setter_parity` | `picoui_radial_menu_set_offset_item` | `picoui_backend_radial_menu_set_offset_item` | test_radial_menu_set_offset_item_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldRadialMenuSetOffsetItem(ldRadialMenu_t *ptWidget,int8_t offset);` |
| 6 | `ldRadialMenu_depose` | `lifecycle` | `widget` | `lifecycle_internal` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldRadialMenu_depose(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 7 | `ldRadialMenu_init` | `init` | `widget` | `direct_covered` | `true` | `covered` | `shared_api_equivalence` | `picoui_radial_menu_init` | `picoui_backend_radial_menu_init` | test_radial_menu_init_native_parity, ctest:test_picoui_icon_slider|test_picoui_radial_menu | R5 fifth batch closed icon_slider/radial_menu rows with concrete PicoUI API or shared alias equivalence plus focused unit proof. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `ldRadialMenu_t* ldRadialMenu_init(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, uint16_t xAxis, uint16_t yAxis, uint8_t itemMax);` |
| 8 | `ldRadialMenu_on_frame_complete` | `lifecycle` | `widget` | `lifecycle_internal` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldRadialMenu_on_frame_complete(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 9 | `ldRadialMenu_on_frame_start` | `lifecycle` | `widget` | `lifecycle_internal` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldRadialMenu_on_frame_start(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 10 | `ldRadialMenu_on_load` | `lifecycle` | `widget` | `lifecycle_internal` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldRadialMenu_on_load(ld_scene_t *ptScene, ldRadialMenu_t *ptWidget);` |
| 11 | `ldRadialMenu_show` | `show` | `widget` | `render_pipeline_internal` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldRadialMenu_show(ld_scene_t *pScene, ldRadialMenu_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应字段存在。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility、private hook 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
