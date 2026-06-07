# TinyUI v2.0 P5 Closeout And Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 用更新后的文档、release-facing 真相、broad gates 和干净索引完成 `v2.0` 收口，同时避免夸大完成度。

**Architecture:** 把 closeout 当作 evidence-driven 工作：更新 contract/runtime gate 文案，明确哪些部分已收口到 `TinyUI`、哪些仍是过渡态，并让 `docs/v2.0/线计划索引.md` 成为本线唯一入口。

**Tech Stack:** Markdown 文档、Python gate scripts、CTest、当前 `v2.0` spec/plan 文档、现有 `picoui` runtime/demos/tests。

---

## 文件结构

新增：

- `docs/v2.0/v2.0-closeout.md`
- `docs/v2.0/v2.0-release-matrix.md`

修改：

- `docs/v2.0/线计划索引.md`
- `docs/v2.0/plans/stages/README.md`
- `docs/v2.0/2026-06-07-tinyui-v2-0-design.md`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`

---

### Task 1: closeout 文档与索引收口

**Files:**
- Create: `docs/v2.0/v2.0-closeout.md`
- Create: `docs/v2.0/v2.0-release-matrix.md`
- Modify: `docs/v2.0/线计划索引.md`
- Modify: `docs/v2.0/plans/stages/README.md`

- [ ] **Step 1: 写 closeout 文档**

Create `docs/v2.0/v2.0-closeout.md`:

```md
# TinyUI v2.0 Closeout

## 已完成

- `backend` shared/core 已拆平
- pilot widgets 已走 `widgets/core -> ld*`
- 用户主路径已不依赖 `app`
- `tinyui_*` 试点 public API 已落地

## 仍处过渡态

- 非试点控件仍可能保留 `picoui_*` 命名
- 部分目录仍保留施工现场历史命名

## 真实 gate

- `rtk ctest --test-dir build -L 'picoui' --output-on-failure`
- `rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
- `git diff --check`
```

- [ ] **Step 2: 写 release matrix**

Create `docs/v2.0/v2.0-release-matrix.md`:

```md
# TinyUI v2.0 Release Matrix

| Area | Expected State | Evidence |
|------|----------------|----------|
| Backend layer | No longer a shared architecture layer | focused code diff + runtime mapping gate |
| Runtime model | app-free public main path | `basic_widgets` startup path + runtime test |
| Pilot widgets | direct `ld*` path | `test_picoui_window/label/button/switch` |
| Public API | `tinyui_*` pilot surface exists | headers + demo compile |
| Docs truth | `docs/v2.0/线计划索引.md` points to spec/plan/closeout | doc review |
```

- [ ] **Step 3: 更新线索引状态**

Append to `docs/v2.0/线计划索引.md`:

```md
## 当前完成态

- `backend` 已退出 shared architecture
- `app` 已退出用户主路径
- `tinyui_*` 已在试点范围落地
- closeout 真相源：
  - `docs/v2.0/v2.0-closeout.md`
  - `docs/v2.0/v2.0-release-matrix.md`
```

- [ ] **Step 4: 阶段 README 改成 closeout 视角**

In `docs/v2.0/plans/stages/README.md`, append:

```md
## Closeout Truth

完成态以 `v2.0-closeout.md` 和 `v2.0-release-matrix.md` 为准；阶段 plan checkbox 仅保留为执行记录，不再代表当前状态。
```

- [ ] **Step 5: 跑 `git diff --check`**

Run:

```bash
git diff --check
```

Expected: PASS。

### Task 2: broad gates 与 release-facing wording 收口

**Files:**
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `docs/v2.0/2026-06-07-tinyui-v2-0-design.md`

- [ ] **Step 1: release matrix checker 接受过渡完成态**

In `tests/picoui/contract/check_picoui_release_capability_matrix.py`, extend summary wording handling so it accepts:

```python
"tinyui_pilot_ready"
"backend_flattened"
"app_free_main_path"
```

when those states are what `v2.0` closeout explicitly claims.

- [ ] **Step 2: runtime mapping checker 文案去 backend-central assumption**

Update `tests/picoui/runtime/check_picoui_backend_mapping.py` to stop requiring every widget to map through a dedicated `backend_*.c` file and instead require:

```python
"direct_ld_binding" or "legacy_backend_bridge"
```

with pilot widgets expected to use `direct_ld_binding`.

- [ ] **Step 3: visible gate 文案同步 TinyUI closeout**

In `tests/picoui/runtime/check_picoui_visible_ui.py`, update failure/help text so `basic_widgets` is described as the `TinyUI v2.0` pilot startup proof, not purely `PicoUI basic_widgets`.

- [ ] **Step 4: spec 增加 closeout note**

Append to `docs/v2.0/2026-06-07-tinyui-v2-0-design.md`:

```md
## Closeout Note

当 `backend` shared layer、`app` main path、`tinyui_*` pilot public surface 已闭环时，`v2.0` 以 `docs/v2.0/v2.0-closeout.md` 和 `docs/v2.0/v2.0-release-matrix.md` 作为 completion truth。
```

- [ ] **Step 5: 跑 final gates**

Run:

```bash
rtk ctest --test-dir build -L 'picoui' --output-on-failure
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure
git diff --check
```

Expected: PASS。

- [ ] **Step 6: Commit**

```bash
git add \
  docs/v2.0/2026-06-07-tinyui-v2-0-design.md \
  docs/v2.0/线计划索引.md \
  docs/v2.0/plans/stages/README.md \
  docs/v2.0/v2.0-closeout.md \
  docs/v2.0/v2.0-release-matrix.md \
  tests/picoui/contract/check_picoui_release_capability_matrix.py \
  tests/picoui/runtime/check_picoui_backend_mapping.py \
  tests/picoui/runtime/check_picoui_visible_ui.py
git commit -m "docs: close out tinyui v2.0 line"
```
