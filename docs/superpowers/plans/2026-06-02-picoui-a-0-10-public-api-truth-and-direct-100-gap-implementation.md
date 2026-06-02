# PicoUI a-0.10 Public API Truth And Direct-100 Gap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 固化 PicoUI public API truth gate，修正 ability/matrix 中虚构 public API 名，并把 direct public 100% 缺口拆成可执行后续任务。

**Architecture:** `R0` 先修 truth gate 与已知漂移；`R1` 冻结 direct-100 denominator；`R2` 对 allowlisted 行再分类；`R3` 决定是否新增 public API；`R4` 同步 ability 文档；`R5` 做 fresh subagent review 与 gate closeout。所有 JSON truth-source 合流必须串行，review 与摸底必须用独立 subagent。

**Tech Stack:** Python3、C、PicoUI public headers、LingDongGUI native API inventory、Markdown serial docs、GitNexus

---

## 0. 执行规则

- 默认在当前主仓推进；若新建 worktree，必须放在 `.worktree/`，创建或切换后执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- 修改 checker 前必须跑 GitNexus impact。
- 跨模块摸底与 review 必须使用独立 subagent。
- 多个 subagent 写面不得重叠。
- 任何 “direct 100%” 结论必须有 public header、matrix、unit/gate 证据。

## 1. 文件写面

### R0 public API truth gate

**Modify:**
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `tests/picoui/contract/ldgui_public_api_inventory.json`
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/README.md`
- `docs/ability/gauge.md`
- `docs/ability/progress_bar.md`
- `docs/ability/qrcode.md`
- `docs/ability/slider.md`
- `docs/ability/keyboard.md`
- `docs/ability/radial_menu.md`

### R1 direct-100 denominator

**Modify:**
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/check_picoui_native_api_exhaustiveness.py`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `docs/ability/README.md`

### R2 allowlisted row classification

**Modify:**
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `docs/ability/*.md`

### R3 optional direct public API implementation

**Modify only if direct-100 implementation is chosen:**
- `picoui/include/picoui/*.h`
- `picoui/src/widgets/*.c`
- `picoui/src/core/*.c`
- `picoui/src/backend/ldgui/*.c`
- `tests/picoui/unit/*.c`
- `tests/picoui/contract/*.json`
- `docs/ability/*.md`

### R4/R5 docs and review closeout

**Modify:**
- `docs/ability/README.md`
- `docs/picoui-serial/a-0.10-线计划索引.md`
- `docs/superpowers/specs/2026-06-02-picoui-a-0-10-public-api-truth-and-direct-100-gap-design.md`
- `docs/superpowers/plans/2026-06-02-picoui-a-0-10-public-api-truth-and-direct-100-gap-implementation.md`

## 2. Tasks

### Task R0: Public API Truth Gate And Known Drift Fix

**Owner:** Fresh subagent `SG-a0.10-R0-public-api-truth`

**Goal:** 修正已知不存在的 `picoui_api` 字段，并让 checker 防止复发。

- [x] **Step 1: Run impact before checker edit**

Run:

```bash
gitnexus_impact target=check_picoui_release_capability_matrix.py direction=upstream repo=LingDongGUI
```

Expected: LOW 或明确报告影响面；若 HIGH/CRITICAL，先停下汇报。

执行记录（2026-06-02）：GitNexus impact 对文件 `tests/picoui/contract/check_picoui_release_capability_matrix.py` 返回 `risk=LOW`、`impactedCount=0`、`processes_affected=0`。

- [x] **Step 2: Fix known non-public PicoUI API names**

Update all three JSON truth-sources and affected ability docs:

| Wrong | Correct |
| --- | --- |
| `picoui_gauge_move` | `picoui_widget_set_pos` |
| `picoui_gauge_set_corner` | `picoui_widget_set_corner` |
| `picoui_gauge_set_hidden` | `picoui_widget_set_visible` |
| `picoui_gauge_set_opacity` | `picoui_widget_set_opacity` |
| `picoui_gauge_set_pointer_image` | `picoui_gauge_set_pointer_source` |
| `picoui_gauge_set_select` | `picoui_widget_set_selected` |
| `picoui_gauge_set_selectable` | `picoui_widget_set_selectable` |
| `picoui_progress_bar_move` | `picoui_widget_set_pos` |
| `picoui_progress_bar_set_corner` | `picoui_widget_set_corner` |
| `picoui_progress_bar_set_frame_image` | `picoui_progress_bar_set_frame_source` |
| `picoui_progress_bar_set_hidden` | `picoui_widget_set_visible` |
| `picoui_progress_bar_set_opacity` | `picoui_widget_set_opacity` |
| `picoui_progress_bar_set_select` | `picoui_widget_set_selected` |
| `picoui_progress_bar_set_selectable` | `picoui_widget_set_selectable` |
| `picoui_q_r_code_move` | `picoui_widget_set_pos` |
| `picoui_q_r_code_set_corner` | `picoui_widget_set_corner` |
| `picoui_q_r_code_set_hidden` | `picoui_widget_set_visible` |
| `picoui_q_r_code_set_opacity` | `picoui_widget_set_opacity` |
| `picoui_q_r_code_set_select` | `picoui_widget_set_selected` |
| `picoui_q_r_code_set_selectable` | `picoui_widget_set_selectable` |
| `picoui_slider_move` | `picoui_widget_set_pos` |
| `picoui_slider_set_corner` | `picoui_widget_set_corner` |
| `picoui_slider_set_hidden` | `picoui_widget_set_visible` |
| `picoui_slider_set_opacity` | `picoui_widget_set_opacity` |
| `picoui_slider_set_select` | `picoui_widget_set_selected` |
| `picoui_slider_set_selectable` | `picoui_widget_set_selectable` |
| `picoui_keyboard_btn_update` | `picoui_keyboard_button_update` |
| `picoui_radial_menu_set_offset_item` | `picoui_radial_menu_offset_item` |

- [x] **Step 3: Add public header check**

Modify `check_picoui_release_capability_matrix.py`:

```python
def _public_picoui_api_symbols() -> set[str]:
    symbols: set[str] = set()
    pattern = re.compile(r"\b(picoui_[A-Za-z0-9_]+)\s*\(")
    for header in PICOUI_INCLUDE_DIR.rglob("*.h"):
        text = header.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            symbols.add(match.group(1))
    return symbols
```

Then assert every `covered` row has public API symbols. Support `+` composition:

```python
api_symbols = [part.strip() for part in picoui_api.split("+")]
missing = [symbol for symbol in api_symbols if symbol not in public_picoui_symbols]
assert not missing
```

- [x] **Step 4: Verify R0**

Run:

```bash
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
rg -n 'picoui_gauge_move|picoui_gauge_set_corner|picoui_gauge_set_hidden|picoui_gauge_set_opacity|picoui_gauge_set_pointer_image|picoui_gauge_set_select|picoui_gauge_set_selectable|picoui_progress_bar_move|picoui_progress_bar_set_corner|picoui_progress_bar_set_frame_image|picoui_progress_bar_set_hidden|picoui_progress_bar_set_opacity|picoui_progress_bar_set_select|picoui_progress_bar_set_selectable|picoui_q_r_code_move|picoui_q_r_code_set_corner|picoui_q_r_code_set_hidden|picoui_q_r_code_set_opacity|picoui_q_r_code_set_select|picoui_q_r_code_set_selectable|picoui_slider_move|picoui_slider_set_corner|picoui_slider_set_hidden|picoui_slider_set_opacity|picoui_slider_set_select|picoui_slider_set_selectable|picoui_keyboard_btn_update|picoui_radial_menu_set_offset_item' docs/ability tests/picoui/contract
git diff --check
```

Expected:

```text
all three Python checkers exit 0
rg exits 1 with no matches
git diff --check exits 0
```

执行记录（2026-06-02）：以上验证已通过。

### Task R1: Freeze Direct-100 Denominator

**Owner:** Fresh subagent `SG-a0.10-R1-denominator`

**Goal:** 将 direct public API 100% 的 denominator 写进机器真相源和 ability README。

- [x] **Step 1: Add derived summary fields**

In `picoui_release_capability_matrix.json.summary`, add:

```json
"direct_public_covered_total": 404,
"policy_allowlisted_total": 207,
"direct_public_100_complete": false
```

- [x] **Step 2: Add checker assertions**

In `check_picoui_release_capability_matrix.py`, assert:

```python
assert summary["direct_public_covered_total"] == summary["covered_total"]
assert summary["policy_allowlisted_total"] == summary["allowlisted_total"]
assert summary["direct_public_100_complete"] is False
```

Do not infer direct 100% from `missing_gap_total=0`.

- [x] **Step 3: Update ability README**

In `docs/ability/README.md`, add a direct-100 section:

```markdown
## Direct Public API 100% 结论

当前未达到 PicoUI 对外 public API direct 100%。

- `covered=404`：有真实 PicoUI public API/backend/unit/gate。
- `allowlisted=207`：policy ledger 已闭环，但不是 direct public wrapper。
- `missing_gap_total=0` 只表示没有未建账 native API，不表示 direct 100%。
```

- [x] **Step 4: Verify**

Run:

```bash
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected: both pass.

执行记录（2026-06-02）：已在 `picoui_release_capability_matrix.json.summary` 冻结 `direct_public_covered_total=404`、`policy_allowlisted_total=207`、`direct_public_100_complete=false`；已在 checker 中断言 direct/public 派生字段只能等于 covered/allowlisted 事实，且不能从 `missing_gap_total=0` 推断 direct 100%；已在 `docs/ability/README.md` 增加 Direct Public API 100% 结论小节。验证命令 `python3 tests/picoui/contract/check_picoui_release_capability_matrix.py` 与 `git diff --check` 均通过。

### Task R2: Classify 207 Allowlisted Rows

**Owner:** Fresh subagent `SG-a0.10-R2-allowlist-classification`

**Goal:** 对 allowlisted 行标注 direct-100 分类，避免所有 allowlist 混成一个桶。

- [x] **Step 1: Add classification field**

For every allowlisted row in ledger and matrix, add:

```json
"direct_100_category": "policy_never_public"
```

Allowed values:

```text
policy_never_public
optional_public_extension
direct_100_required_if_user_demands
```

- [x] **Step 2: Classify obvious never-public rows**

Use:

```text
lifecycle_internal -> policy_never_public
render_pipeline_internal -> policy_never_public
runtime_host_internal -> policy_never_public
layout_solver_internal -> policy_never_public
memory_internal -> policy_never_public
backend_private_hook -> policy_never_public
native_action_private -> policy_never_public
enum_only_semantics -> policy_never_public
```

- [x] **Step 3: Review base/resource/drawing rows**

For:

```text
base_tree_policy
resource_time_helper_policy
drawing_helper_policy
```

decide row by row:

```text
policy_never_public
optional_public_extension
direct_100_required_if_user_demands
```

Document rationale in `allowlist_reason`.

- [x] **Step 4: Checker**

Fail if any `gap_status=allowlisted` row lacks `direct_100_category`.

- [x] **Step 5: Verify**

Run:

```bash
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected: all pass.

执行记录（2026-06-02）：已为 ledger 与 matrix 中全部 `207` 个
`gap_status=allowlisted` row 增加 `direct_100_category`，并在 matrix
summary 中固化 `direct_100_category_counts={"optional_public_extension":16,
"policy_never_public":191}`。明显 internal/private/enum policy 全部归为
`policy_never_public`。`base_tree_policy` 逐行按 a-0.9 决策保守分类：
native tree traversal、geometry/focus/nameId lookup 相关 `16` 行标为
`optional_public_extension`，只表示未来可另行设计 portable PicoUI abstraction；
tree mutation/debug/background helper `4` 行仍为 `policy_never_public`。
`resource_time_helper_policy` 与 `drawing_helper_policy` 均保持
`policy_never_public`，不新增 public API。当前没有
`direct_100_required_if_user_demands` 行。checker 已要求 allowlisted row
必须有合法 `direct_100_category`，并要求 matrix 与 ledger 字段一致。
验证：`python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py`、
`python3 tests/picoui/contract/check_picoui_release_capability_matrix.py`、
`git diff --check` 均通过。

### Task R3: Decide Optional Direct Public API Work

**Owner:** Fresh subagent `SG-a0.10-R3-direct-api-decision`

**Goal:** 若 R2 发现 `direct_100_required_if_user_demands`，把它拆成实现任务；否则明确 a-0.10 不新增 API。

- [x] **Step 1: List candidate rows**

Run:

```bash
python3 - <<'PY'
import json
d=json.load(open('tests/picoui/contract/picoui_release_capability_matrix.json'))
for w in d['widgets']:
    for c in w['capabilities']:
        if c.get('direct_100_category') == 'direct_100_required_if_user_demands':
            print(w['name'], c['native_api'], c.get('policy_category'))
PY
```

执行记录（2026-06-02）：命令无输出；R2 分类结果中 `direct_100_required_if_user_demands=0`。

- [x] **Step 2: For each candidate, choose one outcome**

Outcome A:

```text
Implement PicoUI public API.
```

Outcome B:

```text
Reclassify as optional_public_extension or policy_never_public with stricter rationale.
```

执行记录（2026-06-02）：没有 candidate row，因此本线选择“不新增 PicoUI public API”。`optional_public_extension=16` 仅作为未来可选增强，不进入 a-0.10 必做实现。

- [x] **Step 3: If implementing API, follow TDD**

For each API:

1. Add failing unit test.
2. Add header declaration.
3. Add widget/core/backend implementation.
4. Update matrix row to `covered`.
5. Update ability docs.

执行记录（2026-06-02）：本阶段没有 API implementation 写面，因此无需新增 header/source/unit。后续 R4 只同步 docs direct-100 表达。

- [x] **Step 4: Verify**

Run focused unit tests plus:

```bash
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

执行记录（2026-06-02）：`python3 tests/picoui/contract/check_picoui_release_capability_matrix.py`、`git diff --check` 已在 R2/R3 复核中通过。

### Task R4: Ability Docs Direct-100 Closeout

**Owner:** Fresh subagent `SG-a0.10-R4-ability-docs`

**Goal:** 每个 ability 页面都能回答 direct public 100% 是否达成。

- [x] **Step 1: Update doc generator or mechanically sync docs**

For every `docs/ability/*.md`, add to summary:

```markdown
- direct public API 100%：否
- direct_public_covered：N
- policy_allowlisted：N
- direct_100_gap：N
```

If a page is non-widget/internal, write:

```markdown
- direct public API 100%：不适用；该 group 不进入 widget public parity denominator。
```

执行记录（2026-06-02）：已从 `picoui_release_capability_matrix.json` 机械同步 matrix-backed `docs/ability/*.md`，并单独补 `background.md` enum-only 口径。每个 matrix-backed 页面已写入 direct public API 100% 结论、`direct_public_covered`、`policy_allowlisted`、`direct_100_gap`、`direct_100_category` 统计。

- [x] **Step 2: Add per-row direct-100 category**

Add table column:

```markdown
direct_100_category
```

Rows with `covered` use:

```text
direct_public_covered
```

执行记录（2026-06-02）：matrix-backed ability 表格已新增 `direct_100_category` 列；`covered` 行显示 `direct_public_covered`，allowlisted 行显示 R2 分类。

- [x] **Step 3: Verify docs**

Run:

```bash
rg -n 'direct public API 100%：|direct_100_category' docs/ability
rg -n 'PicoUI 对外 public API direct 100%|policy complete, not direct public 100%' docs/ability
git diff --check
```

Expected: every ability page has the new summary or an explicit non-widget exemption.

执行记录（2026-06-02）：执行 R4 文档搜索与 `git diff --check`，确认 ability 文档均有 direct-100 摘要或非 widget 例外，且无 markdown whitespace 问题。

### Task R5: Fresh Review And Gate Closeout

**Owner:** Fresh subagent `SG-a0.10-R5-review`

**Goal:** 独立复核 code/docs/matrix 是否一致。

- [x] **Step 1: Run independent review**

Ask a fresh subagent:

```text
Review PicoUI a-0.10 direct public API truth. Check picoui/include, matrix JSON, native ledger, docs/ability. Find any covered row whose picoui_api is not public, and any docs that imply direct 100% incorrectly.
```

执行记录（2026-06-02）：fresh reviewer `Nietzsche` 已复核 `picoui/include`、三份 JSON truth-source、`docs/ability/*`、a-0.10 spec/plan/index。结论为未发现问题。复核覆盖：`covered=404` public header 反查无缺失、旧 `28` 个虚构 API 名无残留、组合 API 拆分后均存在、ability 文档 direct-100 数量与 matrix 一致、README/ledger/matrix/spec/plan 统计一致。

- [x] **Step 2: Fix findings in same reviewer/worker loop**

If review fails, the same responsible subagent fixes it. Do not fix review findings in main thread unless subagent fails 3 times.

执行记录（2026-06-02）：review 无发现，因此无修复写面。

- [x] **Step 3: Final gates**

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

Expected:

```text
contract/runtime/mapping/visible/manual artifact gates pass
manual artifact remains artifact evidence only unless manual reviewed passed is explicitly true
```

执行记录（2026-06-02）：最终 gate 已通过：

```text
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
git diff --check
```

`check_picoui_manual_window_artifact.py --all` 只证明 `artifact_entry_exists` / `PICOUI_MANUAL_WINDOW_ARTIFACT=ARTIFACT_READY`；输出仍为 `MANUAL_REVIEW_REQUIRED=1`、`MANUAL_REVIEWED_PASSED=0`，不能写成人工验收通过。

## 3. Self-review checklist

- [x] Spec requirement “public API must exist” is covered by R0.
- [x] Spec requirement “direct-100 denominator” is covered by R1.
- [x] Spec requirement “207 allowlisted rows classified” is covered by R2.
- [x] Spec requirement “optional implementation decision” is covered by R3.
- [x] Spec requirement “ability docs answer direct 100%” is covered by R4.
- [x] Spec requirement “fresh subagent review” is covered by R5.
