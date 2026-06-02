# `window layout internal helper`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`window_layout_internal`
- LingDongGUI API 条目数：`8`
- LingDongGUI 分类统计：`capability`: 1, `getter`: 4, `helper`: 3
- PicoUI 覆盖统计：`allowlisted`: 8
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖：全部行已处置，但包含 allowlisted 内部/helper/lifecycle 行；covered 行有 PicoUI 证据，allowlisted 行不是 PicoUI public 能力。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`native_api_gap_tracked` / `parity_incomplete`
- 来源 header：`src/gui/ldWindowLayoutInternal.h`

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldBaseMarkParentLayoutDirty` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget);` |
| 2 | `ldFlexClampAbsoluteSize` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `arm_2d_size_t ldFlexClampAbsoluteSize(const ldBase_t *ptWidget, arm_2d_size_t tSize);` |
| 3 | `ldFlexFlowIsColumn` | `getter` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldFlexFlowIsColumn(ldFlexFlow_t flow);` |
| 4 | `ldFlexFlowIsReverse` | `getter` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldFlexFlowIsReverse(ldFlexFlow_t flow);` |
| 5 | `ldFlexFlowIsWrap` | `getter` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldFlexFlowIsWrap(ldFlexFlow_t flow);` |
| 6 | `ldFlexFlowIsWrapReverse` | `getter` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `bool ldFlexFlowIsWrapReverse(ldFlexFlow_t flow);` |
| 7 | `ldFlexResolveMainStart` | `capability` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align, int16_t innerMainSize, int16_t contentMainSize, uint16_t visibleCount, int16_t gap, int16_t *pResolvedGap);` |
| 8 | `ldWindowCollectDirectChildren` | `helper` | `true` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow, ldBase_t **ppChildren, uint16_t maxCount, bool skipHidden);` |

## 审计边界

- `covered`：必须有 PicoUI API/backend/unit/gate 证据；本次生成时已校验对应 token 能在当前仓库中找到。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为内部 helper、生命周期、host utility 或非 PicoUI user-facing public 能力；不能当作 PicoUI direct wrapper 覆盖。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
