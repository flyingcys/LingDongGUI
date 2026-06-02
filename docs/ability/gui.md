# `ldGui 场景/入口能力`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`gui`
- LingDongGUI API 条目数：`16`
- LingDongGUI 分类统计：`capability`: 2, `helper`: 6, `init`: 2, `macro_alias`: 5, `update_action`: 1
- PicoUI 覆盖统计：`allowlisted`: 16
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldGui.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldGuiDespose` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiDespose(ld_scene_t *ptScene);` |
| 2 | `ldGuiDraw` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiDraw(ld_scene_t *ptScene,arm_2d_tile_t *ptTile,bool bIsNewFrame);` |
| 3 | `ldGuiFrameComplete` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiFrameComplete(ld_scene_t *ptScene);` |
| 4 | `ldGuiFrameStart` | `capability` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiFrameStart(ld_scene_t *ptScene);` |
| 5 | `ldGuiInit` | `init` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiInit(ldPageFuncGroup_t *ptFuncGroup);` |
| 6 | `ldGuiJumpPage` | `macro_alias` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `#define ldGuiJumpPage(pageFuncGroupName,...) ARM_CONNECT2(ldGuiJumpPage_, __ARM_VA_NUM_ARGS(__VA_ARGS__))(pageFuncGroupName, ##__VA_ARGS__)` |
| 7 | `ldGuiJumpPageFast` | `macro_alias` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `#define ldGuiJumpPageFast(pageFuncGroupName) ldGuiJumpPage(pageFuncGroupName)` |
| 8 | `ldGuiJumpPage_0` | `macro_alias` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `#define ldGuiJumpPage_0(page) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 9 | `ldGuiJumpPage_1` | `macro_alias` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `#define ldGuiJumpPage_1(page,mode) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 10 | `ldGuiJumpPage_2` | `macro_alias` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `#define ldGuiJumpPage_2(page,mode,ms) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 11 | `ldGuiLcdTest` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLcdTest(void);` |
| 12 | `ldGuiLoad` | `capability` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLoad(ld_scene_t *ptScene);` |
| 13 | `ldGuiLoop` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLoop(void);` |
| 14 | `ldGuiSceneInit` | `init` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiSceneInit(ld_scene_t *ptScene);` |
| 15 | `ldGuiTouchProcess` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiTouchProcess(ld_scene_t *ptScene);` |
| 16 | `ldGuiUpdateScene` | `update_action` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiUpdateScene(void);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
