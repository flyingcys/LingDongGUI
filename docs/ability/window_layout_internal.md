# `window_layout_internal`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`window_layout_internal`
- group_kind：`internal_helper`
- LingDongGUI API 条目数：`8`
- LingDongGUI 分类统计：`capability`: 1, `getter`: 4, `helper`: 3
- TinyUI 当前覆盖统计：`allowlisted`: 8
- policy_category 统计：`layout_solver_internal`: 8
- direct public API 100%：不适用；该 group 不进入 widget public parity denominator。
- direct_public_covered：`0`
- policy_allowlisted：`8`
- direct_100_gap：`0`
- direct_100_category 统计：`policy_never_public`: 8
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`non_widget_policy_complete` / `non_widget_policy_complete`
- 来源 header：`src/gui/ldWindowLayoutInternal.h`

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | TinyUI 状态 | 覆盖类型 | TinyUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldBaseMarkParentLayoutDirty` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget);` |
| 2 | `ldFlexClampAbsoluteSize` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `arm_2d_size_t ldFlexClampAbsoluteSize(const ldBase_t *ptWidget, arm_2d_size_t tSize);` |
| 3 | `ldFlexFlowIsColumn` | `getter` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `bool ldFlexFlowIsColumn(ldFlexFlow_t flow);` |
| 4 | `ldFlexFlowIsReverse` | `getter` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `bool ldFlexFlowIsReverse(ldFlexFlow_t flow);` |
| 5 | `ldFlexFlowIsWrap` | `getter` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `bool ldFlexFlowIsWrap(ldFlexFlow_t flow);` |
| 6 | `ldFlexFlowIsWrapReverse` | `getter` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `bool ldFlexFlowIsWrapReverse(ldFlexFlow_t flow);` |
| 7 | `ldFlexResolveMainStart` | `capability` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align, int16_t innerMainSize, int16_t contentMainSize, uint16_t visibleCount, int16_t gap, int16_t *pResolvedGap);` |
| 8 | `ldWindowCollectDirectChildren` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public/backend/unit 证据。 | `uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow, ldBase_t **ppChildren, uint16_t maxCount, bool skipHidden);` |

## 审计边界

- `covered`：必须有真实 current public API/backend/unit/gate 证据；`tinyui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
