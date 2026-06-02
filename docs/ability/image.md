# `image`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`image`
- LingDongGUI API 条目数：`9`
- LingDongGUI 分类统计：`init`: 1, `lifecycle`: 4, `macro_alias`: 1, `setter`: 2, `show`: 1
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 4
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldImage.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldImageInit` | `macro_alias` | `true` | `covered` | `macro_alias_parity` | `picoui_image_create` | `picoui_backend_create_image` | test_image_source_boundary, ctest:test_picoui_widgets\|test_picoui_layout\|test_picoui_list | R3 closed by concrete PicoUI API, real LingDongGUI backend mapping, and unit coverage. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `#define ldImageInit(nameId,parentNameId,x,y,width,height,ptImgTile,ptMaskTile) ldImage_init(ptScene,NULL,nameId,parentNameId,x,y,width,height,ptImgTile,ptMaskTile)` |
| 2 | `ldImageSetImage` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_image_set_source` | `picoui_backend_set_image_source` | test_image_source_boundary, ctest:test_picoui_widgets\|test_picoui_layout\|test_picoui_list | R3 closed by concrete PicoUI API, real LingDongGUI backend mapping, and unit coverage. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldImageSetImage(ldImage_t *ptWidget, arm_2d_tile_t* ptImgTile, arm_2d_tile_t* ptMaskTile);` |
| 3 | `ldImageSetMaskColor` | `setter` | `true` | `covered` | `native_setter_parity` | `picoui_image_set_mask_color` | `picoui_backend_image_set_mask_color` | test_image_native_mask_color_round_trip, ctest:test_picoui_widgets\|test_picoui_layout\|test_picoui_list | R3 closed by concrete PicoUI API, real LingDongGUI backend mapping, and unit coverage. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `void ldImageSetMaskColor(ldImage_t *ptWidget,ldColor maskColor);` |
| 4 | `ldImage_depose` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldImage_depose(ld_scene_t *ptScene, ldImage_t *ptWidget);` |
| 5 | `ldImage_init` | `init` | `true` | `covered` | `init_parameter_parity` | `picoui_image_create` | `picoui_backend_create_image` | test_image_source_boundary, ctest:test_picoui_widgets\|test_picoui_layout\|test_picoui_list | R3 closed by concrete PicoUI API, real LingDongGUI backend mapping, and unit coverage. 已校验：PicoUI API/backend/unit/gate 字段存在。 | `ldImage_t* ldImage_init(ld_scene_t *ptScene, ldImage_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height, arm_2d_tile_t* ptImgTile, arm_2d_tile_t* ptMaskTile);` |
| 6 | `ldImage_on_frame_complete` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldImage_on_frame_complete(ld_scene_t *ptScene, ldImage_t *ptWidget);` |
| 7 | `ldImage_on_frame_start` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldImage_on_frame_start(ld_scene_t *ptScene, ldImage_t *ptWidget);` |
| 8 | `ldImage_on_load` | `lifecycle` | `false` | `allowlisted` | `lifecycle_internal_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldImage_on_load(ld_scene_t *ptScene, ldImage_t *ptWidget);` |
| 9 | `ldImage_show` | `show` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | Render/show entry point owned by backend rendering pipeline. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldImage_show(ld_scene_t *ptScene,ldImage_t *ptWidget,const arm_2d_tile_t *ptTile,bool bIsNewFrame);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
