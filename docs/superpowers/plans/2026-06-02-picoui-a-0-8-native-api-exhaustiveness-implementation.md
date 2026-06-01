# PicoUI a-0.8 Native API Exhaustiveness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 PicoUI `v0.7` 的手写 native-100 口径升级为可机器证明的 LingDongGUI native public API exhaustiveness，并关闭当前代码里所有真实控件能力和证据缺口。

**Architecture:** `a-0.8` 固定依赖为 `R0 -> R1 -> R2 -> R3/R4/R5 -> R6 -> R7 -> R8`。先建立 `ld*.h` public API extractor、`native_api_gap_ledger.json` 和 exhaustiveness gate，再按 ledger 分批补 shared policy 与全部控件缺口，最后串行收敛 matrix、runtime/mapping/visible/manual evidence 和 closeout 文档。任何 full parity 结论必须来自真实 backend 字段、unit、contract、runtime/mapping/visible/manual gate，不接受 hand-written matrix 自证，也不允许为 lifecycle/internal 能力过度新增 public wrapper。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 主线 worktree 固定：`.worktree/a-0.8`
- 建议分支：`feat/picoui-a-0-8-native-api-exhaustiveness`
- 创建 worktree 后必须执行：

```bash
git worktree add .worktree/a-0.8 -b feat/picoui-a-0-8-native-api-exhaustiveness HEAD
cd .worktree/a-0.8
git submodule sync --recursive
git submodule update --init --recursive
```

- 每个 Task 使用 fresh subagent 执行。
- 每个 Task 结束后必须做独立只读 review。
- review 不通过时，由原执行 subagent 修复。
- 涉及 C / Python 符号修改前必须先跑 GitNexus impact。
- 每个 Task 结束前至少运行 `git diff --check`。
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`。
- `R0/R1/R2` 必须串行；`R3/R4/R5` 的具体修复清单必须来自 `native_api_gap_ledger.json`，可并行探索但合流必须串行；`R6/R7/R8` 必须串行。

## 1. 文件结构与阶段边界

### R0 native API inventory

**Create:**
- `tests/picoui/contract/ldgui_public_api_inventory.json`
- `tests/picoui/contract/native_api_gap_ledger.json`
- `tests/picoui/contract/check_ldgui_public_api_inventory.py`
- `tests/picoui/contract/ldgui_public_api_expected_symbols.json`

**Modify:**
- `docs/picoui-serial/a-0.8-线计划索引.md`

### R1 exhaustiveness gate / matrix schema

**Create:**
- `tests/picoui/contract/check_picoui_native_api_exhaustiveness.py`

**Modify:**
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `tests/picoui/contract/check_picoui_public_api.py`

### R2 shared policy / smoke path gate

**Modify:**
- `picoui/include/picoui/widget.h`
- `picoui/include/picoui/layout.h`
- `picoui/include/picoui/theme.h`
- `picoui/include/picoui/native.h`
- `picoui/src/core/widget.c`
- `picoui/src/core/native.c`
- `picoui/src/layout/flex.c`
- `picoui/src/layout/grid.c`
- `picoui/src/theme/theme.c`
- `picoui/src/backend/ldgui/backend_widget.c`
- `picoui/src/backend/ldgui/backend_layout.c`
- `picoui/src/backend/ldgui/backend_theme.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_app.c`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- `tests/picoui/unit/test_picoui_native_bridge.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/unit/test_picoui_layout.c`
- `tests/picoui/unit/test_picoui_theme.c`

### R3 ledger A：base / layout / text / image / stable old widgets

**Modify:**
- `picoui/include/picoui/text.h`
- `picoui/include/picoui/image.h`
- `picoui/include/picoui/window.h`
- `picoui/include/picoui/layout.h`
- `picoui/include/picoui/label.h`
- `picoui/include/picoui/checkbox.h`
- `picoui/include/picoui/switch.h`
- `picoui/include/picoui/list.h`
- `picoui/src/widgets/text.c`
- `picoui/src/widgets/image.c`
- `picoui/src/widgets/window.c`
- `picoui/src/backend/ldgui/backend_text.c`
- `picoui/src/backend/ldgui/backend_image.c`
- `picoui/src/backend/ldgui/backend_window.c`
- `picoui/src/backend/ldgui/backend_layout.c`
- `tests/picoui/unit/test_picoui_text.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/unit/test_picoui_layout.c`

### R4 ledger B：input / navigation / data widgets

**Modify:**
- `picoui/include/picoui/line_edit.h`
- `picoui/include/picoui/keyboard.h`
- `picoui/include/picoui/combo_box.h`
- `picoui/include/picoui/scroll_selecter.h`
- `picoui/include/picoui/table.h`
- `picoui/include/picoui/graph.h`
- `picoui/include/picoui/calendar.h`
- `picoui/src/widgets/line_edit.c`
- `picoui/src/widgets/keyboard.c`
- `picoui/src/widgets/combo_box.c`
- `picoui/src/widgets/scroll_selecter.c`
- `picoui/src/widgets/table.c`
- `picoui/src/widgets/graph.c`
- `picoui/src/widgets/calendar.c`
- `picoui/src/backend/ldgui/backend_line_edit.c`
- `picoui/src/backend/ldgui/backend_keyboard.c`
- `picoui/src/backend/ldgui/backend_combo_box.c`
- `picoui/src/backend/ldgui/backend_scroll_selecter.c`
- `picoui/src/backend/ldgui/backend_table.c`
- `picoui/src/backend/ldgui/backend_graph.c`
- `picoui/src/backend/ldgui/backend_calendar.c`
- `tests/picoui/unit/test_picoui_line_edit.c`
- `tests/picoui/unit/test_picoui_keyboard.c`
- `tests/picoui/contract/native_100_fragments/r4a-input-nav.json`
- `tests/picoui/contract/native_100_fragments/r4b-data.json`

### R5 ledger C：visual / instrument / composite / remaining widgets

**Modify:**
- `picoui/include/picoui/button.h`
- `picoui/include/picoui/slider.h`
- `picoui/include/picoui/qrcode.h`
- `picoui/include/picoui/arc.h`
- `picoui/include/picoui/gauge.h`
- `picoui/include/picoui/date_time.h`
- `picoui/include/picoui/clock.h`
- `picoui/include/picoui/progress_bar.h`
- `picoui/include/picoui/progress_wheel.h`
- `picoui/include/picoui/icon_slider.h`
- `picoui/include/picoui/radial_menu.h`
- `picoui/include/picoui/message_box.h`
- `picoui/include/picoui/animation.h`
- `picoui/src/widgets/button.c`
- `picoui/src/widgets/slider.c`
- `picoui/src/widgets/qrcode.c`
- `picoui/src/widgets/arc.c`
- `picoui/src/widgets/gauge.c`
- `picoui/src/widgets/date_time.c`
- `picoui/src/widgets/clock.c`
- `picoui/src/widgets/progress_bar.c`
- `picoui/src/widgets/progress_wheel.c`
- `picoui/src/widgets/icon_slider.c`
- `picoui/src/widgets/radial_menu.c`
- `picoui/src/widgets/message_box.c`
- `picoui/src/widgets/animation.c`
- `picoui/src/backend/ldgui/backend_button.c`
- `picoui/src/backend/ldgui/backend_slider.c`
- `picoui/src/backend/ldgui/backend_qrcode.c`
- `picoui/src/backend/ldgui/backend_arc.c`
- `picoui/src/backend/ldgui/backend_gauge.c`
- `picoui/src/backend/ldgui/backend_date_time.c`
- `picoui/src/backend/ldgui/backend_clock.c`
- `picoui/src/backend/ldgui/backend_progress_bar.c`
- `picoui/src/backend/ldgui/backend_progress_wheel.c`
- `picoui/src/backend/ldgui/backend_icon_slider.c`
- `picoui/src/backend/ldgui/backend_radial_menu.c`
- `picoui/src/backend/ldgui/backend_message_box.c`
- `picoui/src/backend/ldgui/backend_animation.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/unit/test_picoui_button_events.c`
- `tests/picoui/unit/test_picoui_qrcode.c`

### R6 message_box / keyboard evidence

**Modify:**
- `docs/picoui-serial/C-线人工窗口验收记录.md`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `picoui/docs/demo_guide.md`

### R7 final matrix / gate closeout

**Modify:**
- `tests/picoui/contract/ldgui_public_api_inventory.json`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `tests/picoui/contract/check_picoui_public_api.py`
- `picoui/docs/api_overview.md`
- `picoui/docs/demo_guide.md`

### R8 docs / release readiness

**Modify:**
- `docs/picoui-serial/a-0.8-线计划索引.md`
- `docs/picoui-serial/a-0.8-v0.7-native-100-review-gap-list.md`
- `docs/superpowers/specs/2026-06-02-picoui-a-0-8-native-api-exhaustiveness-design.md`
- `docs/superpowers/plans/2026-06-02-picoui-a-0-8-native-api-exhaustiveness-implementation.md`

## 2. Tasks

### Task R0: native API inventory

**Files:**
- Create: `tests/picoui/contract/ldgui_public_api_inventory.json`
- Create: `tests/picoui/contract/native_api_gap_ledger.json`
- Create: `tests/picoui/contract/check_ldgui_public_api_inventory.py`
- Create: `tests/picoui/contract/ldgui_public_api_expected_symbols.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="check_picoui_native_100_inventory.py", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_release_capability_matrix.json", direction="upstream", repo="LingDongGUI")
```

Expected: 记录 direct callers / affected processes / risk。若 HIGH 或 CRITICAL，先回主线程确认。

- [ ] **Step 2: 写 extractor RED**

新增 `tests/picoui/contract/check_ldgui_public_api_inventory.py`，先让它读取不存在或不完整的 inventory 并 fail。

Required behavior 必须基于全量 expected fixture，不允许只断言 3 个样本。先新增 `tests/picoui/contract/ldgui_public_api_expected_symbols.json`，内容由当前 `src/gui/ld*.h` 扫描结果冻结，至少包含：

```python
assert len(extracted_symbols) >= expected["minimum_symbol_count"]
assert set(expected["required_symbols"]).issubset(extracted_symbols)
assert "ldTextSetStaticText" in extracted_symbols
assert "ldImageSetMaskColor" in extracted_symbols
assert "ldKeyboardBtnUserDraw" in extracted_symbols
assert "ldTableGetItemRegion" in extracted_symbols
assert "ldBaseFocusNavigate" in extracted_symbols
```

Run:

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
```

Expected: FAIL，原因是 `ldgui_public_api_inventory.json` 不存在、symbol count 不足、或缺任一 expected required symbol。

- [ ] **Step 3: 实现 header parser**

Parser 必须扫描：

```text
src/gui/ld*.h
```

并抽取：

```text
ld[A-Z][A-Za-z0-9_]* function declarations
#define ldXxxSetHidden ldBaseSetHidden macro aliases
```

最小分类：

```text
init / lifecycle / show / setter / getter / callback_hook / update_action / macro_alias / helper
```

- [ ] **Step 4: 写 inventory JSON**

新增 `tests/picoui/contract/ldgui_public_api_inventory.json`。

必须由 extractor 输出全量 `src/gui/ld*.h` public API rows，不允许只包含 review 已知缺口。下面只是 row 结构示例：

```json
{
  "schema_version": "a-0.8-ldgui-public-api-inventory-v1",
  "source": "src/gui/ld*.h",
  "widgets": [
    {
      "name": "text",
      "required_native_apis": [
        {
          "ldgui_symbol": "ldTextSetStaticText",
          "category": "setter",
          "required": true,
          "picoui_api": "picoui_text_set_static_text",
          "backend_proof": "picoui_backend_text_set_static_text",
          "unit_test": "test_text_static_text_uses_backend_static_storage",
          "gap_status": "missing_picoui_api"
        }
      ]
    }
  ],
  "allowlist": []
}
```

- [ ] **Step 5: 生成 gap ledger**

新增 `tests/picoui/contract/native_api_gap_ledger.json`。

ledger 每条 row 必须包含：

```json
{
  "widget": "text",
  "ldgui_symbol": "ldTextSetStaticText",
  "gap_status": "missing_picoui_api",
  "planned_task": "R3",
  "overwrap_risk": false,
  "notes": "No current PicoUI API exposes static text lifetime semantics."
}
```

`gap_status` 只能是：

```text
covered
missing_picoui_api
missing_backend_proof
missing_unit
missing_gate
overwrapped
allowlisted
```

要求：

1. 所有 `setter / getter / init_parameter / callback_hook / update_action / macro_alias` 必须在 ledger 中出现。
2. lifecycle-only / render-only API 默认进入 allowlist，不能生成 public wrapper 任务。
3. 如果现有 shared API 已覆盖 native 能力，必须标 `covered` 并写 `equivalence_proof`。
4. `ldBase*` shared getter/setter/tree/focus helper 必须显式归入 shared widget/layout/navigation、backend proof 或 allowlist，不能隐式跳过。
5. `label / checkbox / combo_box / date_time / list / scroll_selecter / progress_bar / switch / graph / calendar / table / icon_slider / radial_menu / gauge / arc / progress_wheel / clock / message_box` 的 public API 必须全部分配到 R3/R4/R5/R6。

- [ ] **Step 6: GREEN**

Run:

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
assert any(row['gap_status'] == 'missing_picoui_api' for row in ledger['rows'])
assert not any(row['gap_status'] == 'overwrapped' and not row.get('notes') for row in ledger['rows'])
PY
git diff --check
```

Expected: PASS。

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R1: exhaustiveness gate / matrix schema

**Files:**
- Create: `tests/picoui/contract/check_picoui_native_api_exhaustiveness.py`
- Modify: `tests/picoui/contract/native_api_gap_ledger.json`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `tests/picoui/contract/check_picoui_public_api.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="check_picoui_release_capability_matrix.py", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="check_picoui_public_api.py", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 exhaustiveness RED**

新增 checker，先要求 matrix row 中必须存在：

```text
native_api
coverage_kind
picoui_api
backend_proof
unit_test
gate_evidence
gap_status
```

Run:

```bash
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
```

Expected: FAIL，指出当前 matrix 缺 native API aligned rows。

- [ ] **Step 3: 升级 matrix schema**

每个 capability row 改为：

```json
{
  "name": "ldImageSetMaskColor",
  "status": "support",
  "native_api": "ldImageSetMaskColor",
  "coverage_kind": "native_setter_parity",
  "picoui_api": "picoui_image_set_mask_color",
  "backend_proof": "picoui_backend_image_set_mask_color",
  "unit_test": "test_image_mask_color_writes_ld_image_field",
  "gate_evidence": ["contract", "unit"],
  "gap_status": "covered",
  "capability_release_judgement": "final_release_ready"
}
```

Valid `coverage_kind`:

```text
native_setter_parity
native_getter_parity
init_parameter_parity
direct_field_parity
macro_alias_parity
lifecycle_internal_allowlisted
non_widget_allowlisted
shared_api_equivalence
```

- [ ] **Step 4: 校验真实符号存在**

Checker 必须验证：

```text
required=true and non-allowlisted rows: picoui_api exists in picoui/include/picoui/*.h
required=true and non-allowlisted rows: backend_proof exists in picoui/src/backend/ldgui/*.c or backend.h
required=true and non-allowlisted rows: unit_test exists in tests/picoui/unit/*.c
allowlisted rows: allowlist_reason exists and coverage_kind is allowlisted
gap_status is covered or allowlisted at closeout
```

- [ ] **Step 5: overwrap checker**

Checker 必须 fail 这些情况：

```text
lifecycle-only API has proposed picoui public wrapper
render-only show API has proposed picoui public wrapper
native API already covered by picoui_widget/layout/theme/native shared API but plan proposes duplicate widget-specific wrapper
```

Expected: `overwrapped` row 必须在 R3/R4/R5 被移除或改为 `shared_api_equivalence`。

- [ ] **Step 6: public API checker 双向化**

保留禁泄漏检查，同时读取 `ldgui_public_api_inventory.json`，校验每个 row 都有 `required / coverage_kind / gap_status / rationale`。`required=true` 的 row 必须有存在的 `picoui_api` 或 `coverage_kind=shared_api_equivalence`；`required=false` 的 row 必须有 allowlist reason。

- [ ] **Step 7: GREEN**

Run:

```bash
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
git diff --check
```

Expected: PASS。

- [ ] **Step 8: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R2: shared policy / smoke path gate

**Files:**
- Modify: `picoui/include/picoui/widget.h`
- Modify: `picoui/include/picoui/layout.h`
- Modify: `picoui/include/picoui/theme.h`
- Modify: `picoui/include/picoui/native.h`
- Modify: `picoui/src/core/widget.c`
- Modify: `picoui/src/core/native.c`
- Modify: `picoui/src/layout/flex.c`
- Modify: `picoui/src/layout/grid.c`
- Modify: `picoui/src/theme/theme.c`
- Modify: `picoui/src/backend/ldgui/backend_widget.c`
- Modify: `picoui/src/backend/ldgui/backend_layout.c`
- Modify: `picoui/src/backend/ldgui/backend_theme.c`
- Modify: `picoui/src/backend/ldgui/backend_event.c`
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_layout.c`
- Modify: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_backend_apply_smoke_cursor_layout", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_native_image_wrap", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: shared base/layout ledger RED**

从 `native_api_gap_ledger.json` 抽出 `ldBase* / ldWindow* / macro_alias / focus navigate / tree helper` rows。

Run:

```bash
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
rows = [r for r in ledger['rows'] if r.get('widget') in ('base', 'window') or r.get('category') == 'macro_alias']
assert rows, 'missing shared base/window ledger rows'
for symbol in ['ldBaseGetOpacity', 'ldBaseSetHidden', 'ldBaseGetRegion', 'ldBaseFocusNavigate', 'ldWindowSetLayout', 'ldWindowSetPadding']:
    assert any(r['ldgui_symbol'] == symbol for r in rows), symbol
PY
```

Expected: PASS after R0, or FAIL if ledger still skips shared abilities.

- [ ] **Step 3: smoke path RED**

新增 runtime gate 断言：full parity demo 不能触发 `temporary smoke layout` marker。

Expected marker:

```text
PICOUI_SMOKE_LAYOUT_USED=0
```

Run:

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
```

Expected: 若当前 demo 触发 smoke layout，FAIL。

- [ ] **Step 4: 加 shared base/layout policy**

R2 必须把 shared rows 分成：

```text
shared_api_equivalence via picoui_widget_*
shared_api_equivalence via picoui_flex_* / picoui_grid_*
backend proof only
allowlisted tree/helper
missing_* to be fixed before R3
```

必须显式处理：

```text
ldBaseGetWidgetType/GetNameId/GetOpacity/IsHidden/IsSelected/IsSelectable/IsCorner
ldBaseGetParent/GetChildList/GetNextSibling/GetChildCount
ldBaseSetCenter/Hidden/Opacity/Selectable/Select/Corner/GridCell/Region/Move/Resize/X/Y/Width/Height/Flex*/IgnoreLayout
ldBaseGetRegion/GetLocation/GetSize/GetX/GetY/GetWidth/GetHeight
ldBaseGetTime/GetDate/GetWeek
ldBaseFocusNavigateInit/ldBaseFocusNavigate
ldWindowSetLayout/FlexFlow/FlexAlign/FlexTrackAlign/Padding/FlexGap/Gap/GridColumns/GridDscArray/GridAlign/GridGap/GridPadding/PaddingGroup
```

- [ ] **Step 5: 加 smoke marker**

在 `backend_app.c` 中记录 smoke layout 是否触发。formal native-100 demo 触发即返回非零或输出 fail marker。

- [ ] **Step 6: manual artifact policy**

manual artifact JSON / catalog 必须区分：

```text
artifact_entry_exists
manual_review_required
manual_reviewed_passed
```

`manual_review_required=true` 时，不允许 matrix 同时把 manual evidence 当作 pass。

- [ ] **Step 7: GREEN**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
git diff --check
```

Expected: PASS。

- [ ] **Step 8: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R3: ledger A fixes

**Files:**
- Modify: `picoui/include/picoui/text.h`
- Modify: `picoui/include/picoui/image.h`
- Modify: `picoui/include/picoui/window.h`
- Modify: `picoui/include/picoui/layout.h`
- Modify: `picoui/include/picoui/label.h`
- Modify: `picoui/include/picoui/checkbox.h`
- Modify: `picoui/include/picoui/switch.h`
- Modify: `picoui/include/picoui/list.h`
- Modify: `picoui/src/widgets/text.c`
- Modify: `picoui/src/widgets/image.c`
- Modify: `picoui/src/widgets/window.c`
- Modify: `picoui/src/widgets/label.c`
- Modify: `picoui/src/widgets/checkbox.c`
- Modify: `picoui/src/widgets/switch.c`
- Modify: `picoui/src/widgets/list.c`
- Modify: `picoui/src/backend/ldgui/backend_text.c`
- Modify: `picoui/src/backend/ldgui/backend_image.c`
- Modify: `picoui/src/backend/ldgui/backend_window.c`
- Modify: `picoui/src/backend/ldgui/backend_layout.c`
- Modify: `picoui/src/backend/ldgui/backend_label.c`
- Modify: `picoui/src/backend/ldgui/backend_checkbox.c`
- Modify: `picoui/src/backend/ldgui/backend_switch.c`
- Modify: `picoui/src/backend/ldgui/backend_list.c`
- Modify: `tests/picoui/unit/test_picoui_text.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_layout.c`
- Modify: `tests/picoui/unit/test_picoui_list.c`
- Modify: `tests/picoui/contract/native_100_fragments/r3b-selection.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_text_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_image_set_source", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_flex_set_gap", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: derive R3 worklist from ledger**

Run:

```bash
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
rows = [r for r in ledger['rows'] if r.get('planned_task') == 'R3' and r['gap_status'] != 'covered']
assert rows, 'R3 has no ledger rows'
for row in rows:
    print(row['widget'], row['ldgui_symbol'], row['gap_status'])
PY
```

Expected: 输出 R3 必须关闭的全部 rows。不得只修本文档手写列出的样本。

- [ ] **Step 3: text RED**

新增 unit tests：

```text
test_text_static_text_uses_backend_static_storage
test_text_transparent_writes_ld_text_field
test_text_background_source_writes_ld_text_image_mask
test_text_scroll_seek_and_move_write_scroll_offsets
```

Run:

```bash
ctest --test-dir build -R 'test_picoui_text|test_picoui_widgets' --output-on-failure
```

Expected: FAIL。

- [ ] **Step 4: image/window RED**

新增 unit tests：

```text
test_image_mask_color_writes_ld_image_field
test_window_layout_type_writes_ld_window_layout
test_window_padding_and_grid_padding_write_distinct_fields
test_window_generic_gap_writes_ld_window_gap
```

Run:

```bash
ctest --test-dir build -R 'test_picoui_layout|test_picoui_widgets' --output-on-failure
```

Expected: FAIL。

- [ ] **Step 5: implement text API or equivalence**

只有在 ledger 证明现有 shared API 不足时，才新增 public API：

```c
int picoui_text_set_transparent(struct picoui_text *text, int transparent);
int picoui_text_set_static_text(struct picoui_text *text, const char *value);
int picoui_text_set_text_color(struct picoui_text *text, unsigned int rgb);
int picoui_text_set_bg_color(struct picoui_text *text, unsigned int rgb);
int picoui_text_set_background_source(struct picoui_text *text, struct picoui_image_source *source);
int picoui_text_set_consumed_font(struct picoui_text *text, const struct picoui_font *font);
int picoui_text_scroll_seek(struct picoui_text *text, int offset);
int picoui_text_scroll_move(struct picoui_text *text, int delta);
```

Backend 或 equivalence proof 必须调用或证明：

```text
ldTextSetTransparent
ldTextSetStaticText
ldTextSetTextColor
ldTextSetBackgroundImage
ldTextSetBackgroundColor
ldTextSetConsumedFont
ldTextScrollSeek
ldTextScrollMove
```

- [ ] **Step 6: implement image/window API or equivalence**

只有在 ledger 证明现有 shared API 不足时，才新增 public API：

```c
int picoui_image_set_mask_color(struct picoui_image *image, unsigned int rgb);
int picoui_window_set_layout(struct picoui_window *window, int layout);
int picoui_window_set_padding(struct picoui_window *window, int padding);
int picoui_window_set_grid_padding(struct picoui_window *window, int padding);
int picoui_window_set_gap(struct picoui_window *window, int gap);
```

如果 `picoui_widget_set_padding`、`picoui_flex_set_gap`、`picoui_grid_set_gap`、`picoui_grid_set_columns` 已等价覆盖，对应 matrix row 必须使用 `shared_api_equivalence`，不得重复包装。

Backend 必须证明：

```text
ldImage_t.maskColor
ldWindow_t.layoutTpye
ldWindow_t.flexPadding
ldWindow_t.gridPadding
ldWindow_t.flexGap
ldWindow_t.gridRowGap / gridColumnGap
```

- [ ] **Step 7: update matrix/inventory/ledger rows**

把新增 APIs 写入：

```text
tests/picoui/contract/ldgui_public_api_inventory.json
tests/picoui/contract/native_api_gap_ledger.json
tests/picoui/contract/picoui_release_capability_matrix.json
```

- [ ] **Step 8: GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_text|test_picoui_widgets|test_picoui_layout' --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
bad = [r for r in ledger['rows'] if r.get('planned_task') == 'R3' and r['gap_status'] not in ('covered', 'allowlisted')]
assert not bad, bad[:5]
PY
git diff --check
```

Expected: PASS。

- [ ] **Step 9: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R4: ledger B fixes

**Files:**
- Modify: `picoui/include/picoui/line_edit.h`
- Modify: `picoui/include/picoui/keyboard.h`
- Modify: `picoui/include/picoui/combo_box.h`
- Modify: `picoui/include/picoui/scroll_selecter.h`
- Modify: `picoui/include/picoui/table.h`
- Modify: `picoui/include/picoui/graph.h`
- Modify: `picoui/include/picoui/calendar.h`
- Modify: `picoui/src/widgets/line_edit.c`
- Modify: `picoui/src/widgets/keyboard.c`
- Modify: `picoui/src/widgets/combo_box.c`
- Modify: `picoui/src/widgets/scroll_selecter.c`
- Modify: `picoui/src/widgets/table.c`
- Modify: `picoui/src/widgets/graph.c`
- Modify: `picoui/src/widgets/calendar.c`
- Modify: `picoui/src/backend/ldgui/backend_line_edit.c`
- Modify: `picoui/src/backend/ldgui/backend_keyboard.c`
- Modify: `picoui/src/backend/ldgui/backend_combo_box.c`
- Modify: `picoui/src/backend/ldgui/backend_scroll_selecter.c`
- Modify: `picoui/src/backend/ldgui/backend_table.c`
- Modify: `picoui/src/backend/ldgui/backend_graph.c`
- Modify: `picoui/src/backend/ldgui/backend_calendar.c`
- Modify: `tests/picoui/unit/test_picoui_line_edit.c`
- Modify: `tests/picoui/unit/test_picoui_keyboard.c`
- Modify: `tests/picoui/unit/test_picoui_combo_box.c`
- Modify: `tests/picoui/unit/test_picoui_scroll_selecter.c`
- Modify: `tests/picoui/unit/test_picoui_table.c`
- Modify: `tests/picoui/unit/test_picoui_graph.c`
- Modify: `tests/picoui/unit/test_picoui_calendar.c`
- Modify: `tests/picoui/contract/native_100_fragments/r4a-input-nav.json`
- Modify: `tests/picoui/contract/native_100_fragments/r4b-data.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_line_edit_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_keyboard_navigate", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: derive R4 worklist from ledger**

Run:

```bash
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
rows = [r for r in ledger['rows'] if r.get('planned_task') == 'R4' and r['gap_status'] != 'covered']
assert rows, 'R4 has no ledger rows'
for row in rows:
    print(row['widget'], row['ldgui_symbol'], row['gap_status'])
PY
```

Expected: 输出 R4 必须关闭的全部 rows。不得只修 line_edit / keyboard 样本。

- [ ] **Step 3: line_edit RED**

新增 tests：

```text
test_line_edit_align_writes_backend_align
test_line_edit_color_writes_text_background_frame
test_line_edit_finished_signal_parity
test_line_edit_commit_cancel_result_is_picoui_internal_not_native_reason
```

Run:

```bash
ctest --test-dir build -R '^test_picoui_line_edit$' --output-on-failure
```

Expected: FAIL。

- [ ] **Step 4: keyboard RED**

新增 tests：

```text
test_keyboard_target_button_list_decision_or_allowlist
test_keyboard_callback_decision_or_allowlist
test_keyboard_user_draw_is_backend_private_or_allowlisted
test_keyboard_update_and_button_update_contract_or_allowlist
```

Run:

```bash
ctest --test-dir build -R 'test_picoui_keyboard|test_picoui_line_edit' --output-on-failure
```

Expected: FAIL。

- [ ] **Step 5: implement line_edit API or equivalence**

只允许新增 align/color 这类 native public setter 对应 API。不要新增 finished reason public API；native 只有 `SIGNAL_FINISHED`，PicoUI commit/cancel 只能作为内部 edit_result 证据，不计入 native parity public surface。

```c
int picoui_line_edit_set_align(struct picoui_line_edit *line_edit, int align);
int picoui_line_edit_set_color(struct picoui_line_edit *line_edit,
                               unsigned int text_rgb,
                               unsigned int bg_rgb,
                               unsigned int frame_rgb);
```

Matrix 要求：

```text
ldLineEditSetAlign -> native_setter_parity
ldLineEditSetColor -> native_setter_parity
SIGNAL_FINISHED -> event parity via existing finished callback
commit/cancel edit_result -> PicoUI internal evidence, not native API row
```

- [ ] **Step 6: implement keyboard policy**

先决策，不默认新增 public API。`ldKeyboardGetTargetBtnList / ldKeyboardCallback / ldKeyboardBtnUserDraw` 是 weak extension hook；其中 user draw 牵涉 Arm-2D draw context，默认应是 backend-private extension 或 allowlist，不得直接暴露为跨平台 PicoUI API。

允许的处理方式：

```c
int picoui_keyboard_update(struct picoui_keyboard *keyboard);
int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code);
```

如果决定公开 target list 或 callback，必须先定义稳定 key list schema，不得用 `void *` 糊掉 `kbBtnInfo_t` 合同。若某项必须 allowlist，必须在 inventory 中写 explicit reason，并让 checker enforce。

- [ ] **Step 7: update matrix/inventory/ledger rows**

更新：

```text
tests/picoui/contract/ldgui_public_api_inventory.json
tests/picoui/contract/native_api_gap_ledger.json
tests/picoui/contract/picoui_release_capability_matrix.json
tests/picoui/contract/native_100_fragments/r4a-input-nav.json
```

- [ ] **Step 8: GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_line_edit|test_picoui_keyboard' --output-on-failure
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
bad = [r for r in ledger['rows'] if r.get('planned_task') == 'R4' and r['gap_status'] not in ('covered', 'allowlisted')]
assert not bad, bad[:5]
PY
git diff --check
```

Expected: PASS。

- [ ] **Step 9: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R5: ledger C fixes

**Files:**
- Modify: `picoui/include/picoui/button.h`
- Modify: `picoui/include/picoui/slider.h`
- Modify: `picoui/include/picoui/qrcode.h`
- Modify: `picoui/include/picoui/progress_bar.h`
- Modify: `picoui/include/picoui/progress_wheel.h`
- Modify: `picoui/include/picoui/arc.h`
- Modify: `picoui/include/picoui/gauge.h`
- Modify: `picoui/include/picoui/date_time.h`
- Modify: `picoui/include/picoui/clock.h`
- Modify: `picoui/include/picoui/icon_slider.h`
- Modify: `picoui/include/picoui/radial_menu.h`
- Modify: `picoui/include/picoui/message_box.h`
- Modify: `picoui/include/picoui/animation.h`
- Modify: `picoui/src/widgets/button.c`
- Modify: `picoui/src/widgets/slider.c`
- Modify: `picoui/src/widgets/qrcode.c`
- Modify: `picoui/src/widgets/progress_bar.c`
- Modify: `picoui/src/widgets/progress_wheel.c`
- Modify: `picoui/src/widgets/arc.c`
- Modify: `picoui/src/widgets/gauge.c`
- Modify: `picoui/src/widgets/date_time.c`
- Modify: `picoui/src/widgets/clock.c`
- Modify: `picoui/src/widgets/icon_slider.c`
- Modify: `picoui/src/widgets/radial_menu.c`
- Modify: `picoui/src/widgets/message_box.c`
- Modify: `picoui/src/widgets/animation.c`
- Modify: `picoui/src/backend/ldgui/backend_slider.c`
- Modify: `picoui/src/backend/ldgui/backend_button.c`
- Modify: `picoui/src/backend/ldgui/backend_qrcode.c`
- Modify: `picoui/src/backend/ldgui/backend_progress_bar.c`
- Modify: `picoui/src/backend/ldgui/backend_progress_wheel.c`
- Modify: `picoui/src/backend/ldgui/backend_arc.c`
- Modify: `picoui/src/backend/ldgui/backend_gauge.c`
- Modify: `picoui/src/backend/ldgui/backend_date_time.c`
- Modify: `picoui/src/backend/ldgui/backend_clock.c`
- Modify: `picoui/src/backend/ldgui/backend_icon_slider.c`
- Modify: `picoui/src/backend/ldgui/backend_radial_menu.c`
- Modify: `picoui/src/backend/ldgui/backend_message_box.c`
- Modify: `picoui/src/backend/ldgui/backend_animation.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_button_events.c`
- Modify: `tests/picoui/unit/test_picoui_qrcode.c`
- Modify: `tests/picoui/unit/test_picoui_progress_bar.c`
- Modify: `tests/picoui/unit/test_picoui_progress_wheel.c`
- Modify: `tests/picoui/unit/test_picoui_arc.c`
- Modify: `tests/picoui/unit/test_picoui_gauge.c`
- Modify: `tests/picoui/unit/test_picoui_date_time.c`
- Modify: `tests/picoui/unit/test_picoui_clock.c`
- Modify: `tests/picoui/unit/test_picoui_icon_slider.c`
- Modify: `tests/picoui/unit/test_picoui_radial_menu.c`
- Modify: `tests/picoui/unit/test_picoui_message_box.c`
- Modify: `tests/picoui/unit/test_picoui_animation.c`
- Modify: `tests/picoui/contract/native_100_fragments/r5a-progress-qrcode.json`
- Modify: `tests/picoui/contract/native_100_fragments/r5b-instrument-clock.json`
- Modify: `tests/picoui/contract/native_100_fragments/r6a-composite.json`
- Modify: `tests/picoui/contract/native_100_fragments/r6b-modal-animation.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_slider_set_value", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_button_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_qrcode_set_text", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: derive R5 worklist from ledger**

Run:

```bash
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
rows = [r for r in ledger['rows'] if r.get('planned_task') == 'R5' and r['gap_status'] != 'covered']
assert rows, 'R5 has no ledger rows'
for row in rows:
    print(row['widget'], row['ldgui_symbol'], row['gap_status'])
PY
```

Expected: 输出 R5 必须关闭的全部 rows。不得只修 slider/button/qrcode 样本。

- [ ] **Step 3: RED**

新增 tests：

```text
test_slider_color_triple_maps_to_ld_slider_color
test_button_getters_cover_font_text_text_color_release_press_color
test_qrcode_init_only_fields_are_marked_direct_field_parity
```

Run:

```bash
ctest --test-dir build -R 'test_picoui_widgets|test_picoui_button_events|test_picoui_qrcode' --output-on-failure
```

Expected: FAIL。

- [ ] **Step 4: implement ledger C proof**

只有在 ledger 证明现有 shared API 不足时，才新增 public API。已知样本：

```c
int picoui_slider_set_color(struct picoui_slider *slider,
                            unsigned int bg_rgb,
                            unsigned int frame_rgb,
                            unsigned int indicator_rgb);
int picoui_button_get_text(struct picoui_button *button, const char **text);
int picoui_button_get_text_color(struct picoui_button *button, unsigned int *rgb);
int picoui_button_get_release_color(struct picoui_button *button, unsigned int *rgb);
int picoui_button_get_press_color(struct picoui_button *button, unsigned int *rgb);
```

`qrcode` 若继续 direct field write，matrix row 必须使用：

```text
coverage_kind = direct_field_parity
```

并带 unit proof。

- [ ] **Step 5: update matrix/inventory/ledger rows**

更新：

```text
tests/picoui/contract/ldgui_public_api_inventory.json
tests/picoui/contract/native_api_gap_ledger.json
tests/picoui/contract/picoui_release_capability_matrix.json
```

- [ ] **Step 6: GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_widgets|test_picoui_button_events|test_picoui_qrcode' --output-on-failure
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
bad = [r for r in ledger['rows'] if r.get('planned_task') == 'R5' and r['gap_status'] not in ('covered', 'allowlisted')]
assert not bad, bad[:5]
PY
git diff --check
```

Expected: PASS。

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R6: message_box / keyboard evidence repair

**Files:**
- Modify: `docs/picoui-serial/C-线人工窗口验收记录.md`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="check_picoui_backend_mapping.py", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="check_picoui_manual_window_artifact.py", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: RED**

Add assertions:

```text
message_box_basic must not have formal mapping exclusion
keyboard_basic must not have fake fallback special-case
manual_review_required must not imply final manual pass
```

Run:

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
```

Expected: FAIL if old special-case remains.

- [ ] **Step 3: refresh evidence**

重录或更新：

```text
message_box_basic mapping evidence
keyboard_basic mapping evidence
message_box_basic manual artifact note
keyboard_basic manual artifact note
```

旧文字不得继续包含：

```text
formal mapping exclusion
fake fallback 特例
```

- [ ] **Step 4: update matrix manual state**

matrix manual object 必须区分：

```json
{
  "artifact_entry_exists": true,
  "manual_review_required": true,
  "manual_reviewed_passed": false
}
```

只有真实人工复核后才能写：

```json
{
  "manual_reviewed_passed": true
}
```

- [ ] **Step 5: GREEN**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected: PASS。

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R7: final matrix / gate closeout

**Files:**
- Modify: `tests/picoui/contract/ldgui_public_api_inventory.json`
- Modify: `tests/picoui/contract/native_api_gap_ledger.json`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `tests/picoui/contract/check_picoui_public_api.py`
- Modify: `picoui/docs/api_overview.md`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: run full contract suite**

Run:

```bash
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
```

Expected: PASS。

- [ ] **Step 1b: assert full ledger closure**

Run:

```bash
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
bad = [r for r in ledger['rows'] if r['gap_status'] not in ('covered', 'allowlisted')]
assert not bad, bad[:20]
assert all(r.get('allowlist_reason') for r in ledger['rows'] if r['gap_status'] == 'allowlisted')
PY
```

Expected: PASS。

- [ ] **Step 2: run build/unit**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
```

Expected: PASS。

- [ ] **Step 3: run runtime gates**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
```

Expected: PASS。

- [ ] **Step 4: update docs**

Update:

```text
picoui/docs/api_overview.md
picoui/docs/demo_guide.md
```

Required wording:

```text
a-0.8 native API exhaustiveness gate covers ldgui public API inventory.
manual artifact existence is not manual acceptance.
temporary smoke layout is excluded from full parity evidence.
```

- [ ] **Step 5: final diff checks**

Run:

```bash
git diff --check
git status --short
```

Expected: no whitespace errors; changed files match `a-0.8` scope.

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R8: docs / release readiness

**Files:**
- Modify: `docs/picoui-serial/a-0.8-线计划索引.md`
- Modify: `docs/picoui-serial/a-0.8-v0.7-native-100-review-gap-list.md`
- Modify: `docs/superpowers/specs/2026-06-02-picoui-a-0-8-native-api-exhaustiveness-design.md`
- Modify: `docs/superpowers/plans/2026-06-02-picoui-a-0-8-native-api-exhaustiveness-implementation.md`

- [ ] **Step 1: closeout wording audit**

Search:

```bash
rg -n "100%|full_parity_complete|final_release_ready|manual_review_required|temporary smoke|fake fallback|formal mapping exclusion" docs/picoui-serial docs/superpowers picoui/docs tests/picoui/contract
```

Expected: no stale claim says v0.7 achieved native 100% without a-0.8 gates.

- [ ] **Step 2: update closeout docs**

Required wording:

```text
a-0.8 完成 native API exhaustiveness 修正后，才能重新声明 100% 控件与能力覆盖。
artifact existence 不等于人工验收通过。
temporary smoke path 不参与 full parity 结论。
```

- [ ] **Step 3: final verification**

Run:

```bash
ctest --test-dir build -L picoui --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
python3 - <<'PY'
import json
ledger = json.load(open('tests/picoui/contract/native_api_gap_ledger.json'))
bad = [r for r in ledger['rows'] if r['gap_status'] not in ('covered', 'allowlisted')]
assert not bad, bad[:20]
PY
git diff --check
```

Expected: PASS。

- [ ] **Step 4: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected: affected scope matches `a-0.8` native API exhaustiveness and PicoUI gate/docs updates.
