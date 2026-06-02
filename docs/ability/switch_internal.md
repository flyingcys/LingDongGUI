# `switch internal helper`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`switch_internal`
- LingDongGUI API 条目数：`6`
- LingDongGUI 分类统计：`getter`: 1, `helper`: 5
- PicoUI 覆盖统计：`allowlisted`: 6
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldSwitchInternal.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldSwitchAdvanceAnimation` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim, uint16_t deltaMs, uint16_t *pOutProgress);` |
| 2 | `ldSwitchLayerUsesImage` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldSwitchLayerUsesImage(const void *ptImgTile, const void *ptMaskTile);` |
| 3 | `ldSwitchResolveAxisMetrics` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width, int16_t height, uint16_t knobPadding, bool isHorizontal);` |
| 4 | `ldSwitchResolveGeometry` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `ldSwitchGeometry_t ldSwitchResolveGeometry(int16_t width, int16_t height, uint16_t knobPadding, ldSwitchDirection_t direction, uint16_t animProgress);` |
| 5 | `ldSwitchResolveIsHorizontal` | `getter` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldSwitchResolveIsHorizontal(int16_t width, int16_t height, ldSwitchDirection_t direction);` |
| 6 | `ldSwitchResolveKnobOffset` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics, uint16_t animProgress);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
