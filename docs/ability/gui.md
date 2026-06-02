# `ldGui 场景/入口能力`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`gui`
- group_kind：`runtime_host`
- LingDongGUI API 条目数：`16`
- LingDongGUI 分类统计：`capability`: 2, `helper`: 6, `init`: 2, `macro_alias`: 5, `update_action`: 1
- PicoUI 覆盖统计：`allowlisted`: 11, `covered`: 5
- policy_category 统计：`direct_covered`: 5, `runtime_host_internal`: 11
- direct public API 100%：不适用；该 group 不进入 widget public parity denominator。
- direct_public_covered：`5`
- policy_allowlisted：`11`
- direct_100_gap：`0`
- direct_100_category 统计：`policy_never_public`: 11
- PicoUI 覆盖结论：不是 PicoUI 直接 100% wrapper 覆盖；`covered` 行有 PicoUI API/backend/unit/gate 证据，`allowlisted` 行是 policy 处置且不是 direct public wrapper 覆盖。
- matrix layer：`a_0_8_ledger_truth`
- matrix judgement：`non_widget_policy_complete` / `non_widget_policy_complete`
- 来源 header：`src/gui/ldGui.h`

## 控件能力等价缺口

本页当前没有剩余 user-facing 缺口。a-0.13 item 6 已通过 `picoui_app_set_window()` / `picoui_app_switch_window()` 补齐当前配置下的页面/窗口切换用户态能力；其余 runtime host API 继续保持 backend/runtime 内部项。

## API 能力与 PicoUI 覆盖清单

| # | LingDongGUI symbol | 分类 | group_kind | policy_category | direct_100_category | required | PicoUI 状态 | 覆盖类型 | PicoUI API | backend proof | unit/gate | 说明 | signature |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `ldGuiDespose` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiDespose(ld_scene_t *ptScene);` |
| 2 | `ldGuiDraw` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiDraw(ld_scene_t *ptScene,arm_2d_tile_t *ptTile,bool bIsNewFrame);` |
| 3 | `ldGuiFrameComplete` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiFrameComplete(ld_scene_t *ptScene);` |
| 4 | `ldGuiFrameStart` | `capability` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiFrameStart(ld_scene_t *ptScene);` |
| 5 | `ldGuiInit` | `init` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiInit(ldPageFuncGroup_t *ptFuncGroup);` |
| 6 | `ldGuiJumpPage` | `macro_alias` | `runtime_host` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `picoui_app_set_window + picoui_app_switch_window` | `picoui_app_set_window + picoui_app_switch_window` | test_app_set_window_switches_active_root_and_focus_scope + test_app_switch_window_persists_switch_metadata_contract, ctest:test_picoui_app_window_switch | a-0.13 item 6 以 PicoUI active-window switch 合同补齐当前配置下的页面/窗口切换用户意图。 | `#define ldGuiJumpPage(pageFuncGroupName,...) ARM_CONNECT2(ldGuiJumpPage_, __ARM_VA_NUM_ARGS(__VA_ARGS__))(pageFuncGroupName, ##__VA_ARGS__)` |
| 7 | `ldGuiJumpPageFast` | `macro_alias` | `runtime_host` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `picoui_app_set_window + picoui_app_switch_window` | `picoui_app_set_window + picoui_app_switch_window` | test_app_set_window_switches_active_root_and_focus_scope + test_app_switch_window_persists_switch_metadata_contract, ctest:test_picoui_app_window_switch | 快速页面切换用户意图由同一 PicoUI active-window switch 合同覆盖。 | `#define ldGuiJumpPageFast(pageFuncGroupName) ldGuiJumpPage(pageFuncGroupName)` |
| 8 | `ldGuiJumpPage_0` | `macro_alias` | `runtime_host` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `picoui_app_set_window + picoui_app_switch_window` | `picoui_app_set_window + picoui_app_switch_window` | test_app_set_window_switches_active_root_and_focus_scope + test_app_switch_window_persists_switch_metadata_contract, ctest:test_picoui_app_window_switch | 无动画页面切换用户意图由 PicoUI active-window switch 合同覆盖。 | `#define ldGuiJumpPage_0(page) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 9 | `ldGuiJumpPage_1` | `macro_alias` | `runtime_host` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `picoui_app_set_window + picoui_app_switch_window` | `picoui_app_set_window + picoui_app_switch_window` | test_app_set_window_switches_active_root_and_focus_scope + test_app_switch_window_persists_switch_metadata_contract, ctest:test_picoui_app_window_switch | 带切换模式的页面切换用户意图由 PicoUI active-window switch 合同覆盖。 | `#define ldGuiJumpPage_1(page,mode) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 10 | `ldGuiJumpPage_2` | `macro_alias` | `runtime_host` | `direct_covered` |  | `true` | `covered` | `shared_api_equivalence` | `picoui_app_set_window + picoui_app_switch_window` | `picoui_app_set_window + picoui_app_switch_window` | test_app_set_window_switches_active_root_and_focus_scope + test_app_switch_window_persists_switch_metadata_contract, ctest:test_picoui_app_window_switch | 带切换模式和时长的页面切换用户意图由 PicoUI active-window switch 合同覆盖。 | `#define ldGuiJumpPage_2(page,mode,ms) __ldGuiJumpPage((ldPageFuncGroup_t *)NULL,&ARM_2D_SCENE_SWITCH_MODE_NONE,0)` |
| 11 | `ldGuiLcdTest` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLcdTest(void);` |
| 12 | `ldGuiLoad` | `capability` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLoad(ld_scene_t *ptScene);` |
| 13 | `ldGuiLoop` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiLoop(void);` |
| 14 | `ldGuiSceneInit` | `init` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiSceneInit(ld_scene_t *ptScene);` |
| 15 | `ldGuiTouchProcess` | `helper` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared tree/helper/lifecycle API: internal backend structure or demo host helper, not a PicoUI user-facing native capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiTouchProcess(ld_scene_t *ptScene);` |
| 16 | `ldGuiUpdateScene` | `update_action` | `runtime_host` | `runtime_host_internal` | `policy_never_public` | `false` | `allowlisted` | `non_widget_allowlisted` |  |  |  | R2 shared helper/lifecycle/layout solver API: internal scene/runtime/navigation/layout decision helper, not a stable PicoUI user-facing public capability. 非 covered 行，不要求 PicoUI public/backend/unit 证据。 | `void ldGuiUpdateScene(void);` |

## 审计边界

- `covered`：必须有真实 PicoUI public API/backend/unit/gate 证据；`picoui_api` 会被 checker 反查 `picoui/include` public header。
- `allowlisted`：记录为 LingDongGUI 原生 API，但当前判定为 policy 处置；不能当作 PicoUI direct public wrapper 覆盖。
- `direct_100_category=policy_never_public`：不应暴露为 PicoUI public API。
- `direct_100_category=optional_public_extension`：未来可单独开线设计，但不是 a-0.10 必做 public API。
- `direct_100_category=direct_100_required_if_user_demands`：若出现，必须拆 public API 实现任务；当前 a-0.10 为 `0`。
- `group_kind=runtime_host/internal_helper` 的分组保留在 native API inventory 审计中，但不进入 widget public parity denominator。
- 若后续 LingDongGUI header、inventory 或 matrix 更新，本页必须同步更新。
- 本页不是独立 widgetType 控件页，但属于 `src/gui/ld*.h` 原生能力覆盖范围。
