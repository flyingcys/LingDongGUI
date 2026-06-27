# `switch_internal`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 TinyUI 当前覆盖状态。

## 覆盖摘要

- 能力分组：`switch_internal`
- group_kind：`internal_helper`
- LingDongGUI API 条目数：`6`
- LingDongGUI 分类统计：`getter`: 1, `helper`: 5
- TinyUI 当前覆盖统计：`allowlisted`: 6
- policy_category 统计：`layout_solver_internal`: 6
- direct public API 100%：不适用；该 group 不进入 widget public parity denominator。
- direct_public_covered：`0`
- policy_allowlisted：`6`
- direct_100_gap：`0`
- direct_100_category 统计：`policy_never_public`: 6
- TinyUI 当前覆盖结论：不是当前用户态 direct wrapper 100% 覆盖；`covered` 行有 current public API/unit/runtime/visible gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`non_widget_policy_complete` / `non_widget_policy_complete`
- 来源 header：`src/gui/ldSwitchInternal.h`

## API 能力与 TinyUI 当前覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | TinyUI 状态 | 覆盖类型 | TinyUI API | 历史证据字段 | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldSwitchAdvanceAnimation` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim, uint16_t deltaMs, uint16_t *pOutProgress);` |
| 2 | `ldSwitchLayerUsesImage` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `bool ldSwitchLayerUsesImage(const void *ptImgTile, const void *ptMaskTile);` |
| 3 | `ldSwitchResolveAxisMetrics` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width, int16_t height, uint16_t knobPadding, bool isHorizontal);` |
| 4 | `ldSwitchResolveGeometry` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `ldSwitchGeometry_t ldSwitchResolveGeometry(int16_t width, int16_t height, uint16_t knobPadding, ldSwitchDirection_t direction, uint16_t animProgress);` |
| 5 | `ldSwitchResolveIsHorizontal` | `getter` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable TinyUI user-facing public capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `bool ldSwitchResolveIsHorizontal(int16_t width, int16_t height, ldSwitchDirection_t direction);` |
| 6 | `ldSwitchResolveKnobOffset` | `helper` | `internal_helper` | `layout_solver_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal runtime structure or demo host helper, not a TinyUI user-facing native capability. 非 covered 行，不要求 TinyUI public API/unit 证据。 | `uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics, uint16_t animProgress);` |

## 审计边界

- `covered`：必须有真实 current public API/unit/runtime/visible gate 证据；`tinyui_api` 字段当前仍记录 public C API 过渡态符号，并由 checker 反查 canonical public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作用户态 direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为用户态 public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
