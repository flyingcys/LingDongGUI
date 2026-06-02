# PicoUI a-0.9 Allowlist Truth Closeout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `a-0.8` 产生的 `207` 个 allowlisted native API 行收敛为可机器校验的 PicoUI policy truth，并对 keyboard/button/background 等真实边界做明确开发或拒绝决策。

**Architecture:** `a-0.9` 固定串行入口为 `R0 policy schema`，随后 `R1 lifecycle/show` 与 `R2 runtime/internal` 可并行，`R3 base`、`R4 keyboard/button`、`R5 background` 可并行探索但必须串行合流 matrix，最后 `R6` 做 matrix/docs/gate closeout。每个 subagent 只拥有自己的写面，shared matrix 合流由 integration subagent 或主线程串行执行。

**Tech Stack:** C、Python3、CMake、CTest、PicoUI、LingDongGUI、Markdown serial docs、GitNexus

---

## 0. 执行规则

- worktree 建议：`.worktree/a-0.9`
- 分支建议：`feat/picoui-a-0-9-allowlist-truth-closeout`
- 创建或切换 worktree 后必须执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- 代码符号修改前必须运行 GitNexus impact。
- 每个 Task 使用 fresh subagent。
- review 不通过时，由原 subagent 修复。
- 多个 subagent 写面不得重叠。
- matrix / ledger / docs 合流必须串行。

## 1. 文件写面

### R0 policy schema

**Modify:**
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/ldgui_public_api_inventory.json`
- `tests/picoui/contract/check_picoui_native_api_exhaustiveness.py`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `docs/ability/README.md`

### R1 lifecycle / show policy

**Modify:**
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/*.md`

### R2 runtime/internal group policy

**Modify:**
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/gui.md`
- `docs/ability/mem.md`
- `docs/ability/switch_internal.md`
- `docs/ability/window_layout_internal.md`

### R3 base tree/resource/helper decision

**Modify if exposing new API:**
- `picoui/include/picoui/widget.h`
- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend_widget.c`
- `tests/picoui/unit/test_picoui_layout.c`

**Always modify:**
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/base.md`

### R4 keyboard weak hook / button action decision

**Modify if exposing new API:**
- `picoui/include/picoui/keyboard.h`
- `picoui/src/widgets/keyboard.c`
- `picoui/src/backend/ldgui/backend_keyboard.c`
- `tests/picoui/unit/test_picoui_keyboard.c`
- `picoui/include/picoui/button.h`
- `picoui/src/widgets/button.c`
- `picoui/src/backend/ldgui/backend_button.c`
- `tests/picoui/unit/test_picoui_button_events.c`

**Always modify:**
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/keyboard.md`
- `docs/ability/button.md`

### R5 background/root semantics

**Modify if adding matrix policy only:**
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/background.md`

**Modify if exposing new API:**
- `picoui/include/picoui/window.h`
- `picoui/src/widgets/window.c`
- `picoui/src/backend/ldgui/backend_window.c`
- `tests/picoui/unit/test_picoui_layout.c`

### R6 closeout

**Modify:**
- `docs/ability/README.md`
- `docs/picoui-serial/a-0.9-未direct覆盖能力收口.md`
- `docs/picoui-serial/a-0.9-线计划索引.md`
- `docs/picoui-serial/a-0.10-线计划索引.md` only if a real follow-up remains

## 2. Tasks

### Task R0: Policy Schema And Checker

**Owner:** Fresh subagent `SG-a0.9-R0-policy-schema`

**Goal:** Add machine-checkable policy metadata for all matrix rows and groups.

- [ ] **Step 1: Run impact before checker edits**

Run:

```bash
gitnexus_impact target=check_picoui_native_api_exhaustiveness direction=upstream repo=LingDongGUI
gitnexus_impact target=check_picoui_release_capability_matrix direction=upstream repo=LingDongGUI
```

Expected: report direct callers/tests and risk before edits.

- [ ] **Step 2: Add schema fields**

Update matrix and ledger rows:

```text
group_kind: widget | shared_base | runtime_host | internal_helper | enum_only
policy_category:
  direct_covered
  lifecycle_internal
  render_pipeline_internal
  runtime_host_internal
  layout_solver_internal
  memory_internal
  base_tree_policy
  resource_time_helper_policy
  drawing_helper_policy
  backend_private_hook
  native_action_private
  enum_only_semantics
```

Rules:

```text
gap_status=covered -> policy_category=direct_covered
gap_status=allowlisted -> policy_category != direct_covered
gap_status=allowlisted -> required=false
gap_status=allowlisted -> allowlist_reason non-empty
```

- [ ] **Step 3: Update checker**

Modify `check_picoui_native_api_exhaustiveness.py` to fail when:

```text
row has no policy_category
allowlisted row has required=true
allowlisted row has empty allowlist_reason
covered row has policy_category != direct_covered
allowlisted row has policy_category == direct_covered
matrix group has no group_kind
```

- [ ] **Step 4: Run contract gates**

Run:

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected: all pass.

### Task R1: Lifecycle And Show Policy

**Owner:** Fresh subagent `SG-a0.9-R1-lifecycle-show`

**Goal:** Convert lifecycle/show allowlist from generic allowlist into explicit policy-complete internal rows.

- [ ] **Step 1: Scope rows**

Select rows whose symbol matches:

```text
*_depose
*_on_load
*_on_frame_start
*_on_frame_complete
*_show
```

- [ ] **Step 2: Assign policy**

Use:

```text
*_depose / *_on_load / *_on_frame_start / *_on_frame_complete -> lifecycle_internal
*_show -> render_pipeline_internal
```

Each row must keep:

```text
gap_status=allowlisted
required=false
coverage_kind=lifecycle_internal_allowlisted or non_widget_allowlisted
```

- [ ] **Step 3: Update widget judgement**

For widgets whose only non-covered rows are lifecycle/show policy rows, set widget-level status to policy-complete wording chosen by R0 schema.

- [ ] **Step 4: Sync ability docs**

Update affected `docs/ability/*.md` rows so users see:

```text
PicoUI 状态: allowlisted
policy_category: lifecycle_internal or render_pipeline_internal
说明: backend lifecycle/render pipeline owned; no public wrapper required
```

- [ ] **Step 5: Verify**

Run:

```bash
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R2: Runtime/Internal Group Policy

**Owner:** Fresh subagent `SG-a0.9-R2-runtime-internal`

**Goal:** Separate non-widget native API groups from public widget parity while keeping them in inventory.

- [ ] **Step 1: Classify groups**

Set:

```text
gui -> group_kind=runtime_host
mem -> group_kind=internal_helper
switch_internal -> group_kind=internal_helper
window_layout_internal -> group_kind=internal_helper
```

- [ ] **Step 2: Classify rows**

Use:

```text
gui lifecycle/page/runtime rows -> runtime_host_internal
mem rows -> memory_internal
switch_internal rows -> layout_solver_internal
window_layout_internal rows -> layout_solver_internal
```

- [ ] **Step 3: Checker rule**

Update checker so `runtime_host/internal_helper` groups do not count as widget public parity denominator but still must remain in `611` inventory/matrix row count.

- [ ] **Step 4: Sync docs**

Update:

```text
docs/ability/gui.md
docs/ability/mem.md
docs/ability/switch_internal.md
docs/ability/window_layout_internal.md
docs/ability/README.md
```

- [ ] **Step 5: Verify**

Run:

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R3: Base Tree/Resource/Helper Decision

**Owner:** Fresh subagent `SG-a0.9-R3-base-policy`

**Goal:** Decide whether `ldBase` tree/resource/helper rows are true PicoUI public abilities or internal policy rows.

- [ ] **Step 1: Review base rows**

Inspect `docs/ability/base.md` and classify the 30 allowlisted rows into:

```text
tree traversal / parent-child management
resource lookup
date/time helper
drawing helper
layout/helper math
```

- [ ] **Step 2: Decide tree traversal**

If exposing tree introspection, add APIs:

```c
struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget);
int picoui_widget_get_child_count(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_child(const struct picoui_widget *widget, int index);
struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget);
```

If not exposing, document:

```text
PicoUI tree mutation is owned by create/parent APIs and backend tree internals; native traversal helpers are not public PicoUI user surface.
```

- [ ] **Step 3: Implement only if exposed**

If APIs are added, implement in:

```text
picoui/include/picoui/widget.h
picoui/src/core/widget.c
picoui/src/backend/ldgui/backend_widget.c
tests/picoui/unit/test_picoui_layout.c
```

Unit must assert real backend/native parent-child fields, not host cache only.

- [ ] **Step 4: Update matrix/docs**

For exposed APIs, set rows to `covered`.
For internal policy rows, set:

```text
policy_category=base_tree_policy | resource_time_helper_policy | drawing_helper_policy | layout_solver_internal
```

- [ ] **Step 5: Verify**

Run:

```bash
ctest --test-dir build -R 'test_picoui_layout' --output-on-failure
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R4: Keyboard Weak Hook And Button Action Decision

**Owner:** Fresh subagent `SG-a0.9-R4-keyboard-button-policy`

**Goal:** Resolve the five allowlisted rows most likely to represent real user-facing native extensibility.

- [ ] **Step 1: Review exact rows**

Rows:

```text
keyboard.ldKeyboardGetTargetBtnList
keyboard.ldKeyboardCallback
keyboard.ldKeyboardBtnUserDraw
button.ldButtonActionInit
button.ldButtonActionIsPressById
```

- [ ] **Step 2: Decide keyboard target button list**

Choose one:

```text
Expose portable key layout API.
Keep backend-private because native list contains Arm-2D tile/mask details.
```

If exposed, add stable `picoui_keyboard_set_layout` data structure without leaking `kbBtnInfo_t`.

- [ ] **Step 3: Decide keyboard callback**

Choose one:

```text
Expose PicoUI keyboard event callback using existing event abstraction.
Keep backend-private if current PicoUI text/input events already cover user intent.
```

- [ ] **Step 4: Decide keyboard user draw**

Choose one:

```text
Expose style/theme-level key paint customization.
Reject raw Arm-2D tile draw callback as non-portable backend-private hook.
```

- [ ] **Step 5: Decide button action helper**

Choose one:

```text
Expose PicoUI event/action readback.
Reject native nameId/global action helper and prove existing button events/readback cover user intent.
```

- [ ] **Step 6: Implement only approved public APIs**

If public APIs are added, modify the files listed in R4 write scope and add unit tests that assert real backend/native effect.

- [ ] **Step 7: Update matrix/docs**

Each of the five rows must end as either:

```text
covered with public API/backend/unit/gate
allowlisted with policy_category=backend_private_hook or native_action_private and explicit replacement/why-not
```

- [ ] **Step 8: Verify**

Run:

```bash
ctest --test-dir build -R 'test_picoui_keyboard|test_picoui_button_events' --output-on-failure
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R5: Background/Root Semantics

**Owner:** Fresh subagent `SG-a0.9-R5-background-root`

**Goal:** Convert `background` from markdown-only boundary into matrix-recognized policy or real PicoUI abstraction.

- [ ] **Step 1: Decide background model**

Choose one:

```text
enum_only semantics: background is widgetType-only and covered through root/window/tree policies.
public abstraction: add PicoUI root/background API and proof.
```

- [ ] **Step 2: If enum-only**

Add matrix policy entry or separate policy registry entry:

```text
group=background
group_kind=enum_only
policy_category=enum_only_semantics
reason=widgetTypeBackground has no ldBackground.h public API; semantics are root/window/tree derived.
```

- [ ] **Step 3: If public abstraction**

Add public API and tests in window/root write scope. Unit must prove real root/background semantics, not only demo output.

- [ ] **Step 4: Sync docs**

Update:

```text
docs/ability/background.md
docs/ability/README.md
docs/picoui-serial/a-0.9-未direct覆盖能力收口.md
```

- [ ] **Step 5: Verify**

Run:

```bash
ctest --test-dir build -R 'test_picoui_layout|test_picoui_widgets' --output-on-failure
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R6: Closeout And Documentation Sync

**Owner:** Fresh subagent `SG-a0.9-R6-closeout`

**Goal:** Produce final a-0.9 truth source and verify all gates.

- [ ] **Step 1: Recompute summary**

Read matrix summary and write final counts into:

```text
docs/ability/README.md
docs/picoui-serial/a-0.9-未direct覆盖能力收口.md
docs/picoui-serial/a-0.9-线计划索引.md
```

- [ ] **Step 2: Check no stale language**

Search and fix:

```bash
rg -n '100% direct|full_parity_complete|artifact.*人工|allowlisted.*covered|parity_incomplete' docs/ability docs/picoui-serial/a-0.9-* tests/picoui/contract
```

Expected: no misleading stale conclusion.

- [ ] **Step 3: Run full gates**

Run:

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
git diff --check
```

Expected: all pass. Manual artifact may still say `MANUAL_REVIEW_REQUIRED=1 / MANUAL_REVIEWED_PASSED=0`; document it honestly.

- [ ] **Step 4: GitNexus detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected: changed symbols/processes match a-0.9 policy/code/doc scope.

