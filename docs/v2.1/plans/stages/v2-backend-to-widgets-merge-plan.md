# TinyUI v2.1 V2 Backend To Widgets Merge Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 删除独立 `backend/` 产品层目录，把 widget-specific backend 文件和 API 全部并回 `widgets/*`。

**Architecture:** 以控件为单位迁移，不搞“一次性全仓大合并”。先建立 fail-first backend inventory contract，再按控件批次把 `backend_*.c` 中的 widget-specific 行为合并到对应 widget 文件，只把确实跨控件共享的 helper 留在 `core` 或其他 shared subsystem。

**Tech Stack:** C11、现有 widget 源码、现有 `backend/ldgui` 源码、unit/runtime/mapping gates、GitNexus impact。

---

## 文件结构

修改：

- `tinyui/src/widgets/*`
- `tinyui/src/core/*`
- `tinyui/src/display/*`
- `tinyui/src/theme/*`
- `tinyui/src/indev/*`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/contract/tinyui_v21_transition_inventory.json`

删除：

- `tinyui/src/backend/ldgui/backend_*.c`
- `tinyui/src/backend/ldgui/backend.h`
- 仅在确认无引用后删除其余 backend 目录文件

---

### Task 1: 建立 backend 退场 contract

**Files:**
- Modify: `tests/picoui/contract/check_tinyui_v21_transition_guards.py`
- Modify: `tests/picoui/contract/tinyui_v21_transition_inventory.json`

- [ ] **Step 1: 收紧 guard，要求 backend 文件数下降到 0**

Update the V2 closeout target in `tinyui_v21_transition_inventory.json` so:

```json
"backend_c_files": 0
```

and make the checker fail if any `backend_*.c` remains under the product-layer source tree.

- [ ] **Step 2: 运行 checker，确认 V2 前为 fail**

Run:

```bash
python3 tests/picoui/contract/check_tinyui_v21_transition_guards.py
```

Expected: FAIL，因为当前 backend 文件仍存在。

### Task 2: 按控件批次并回 backend

**Files:**
- Modify: `tinyui/src/widgets/window.c`
- Modify: `tinyui/src/widgets/label.c`
- Modify: `tinyui/src/widgets/button.c`
- Modify: `tinyui/src/widgets/switch.c`
- Modify later: the rest of `tinyui/src/widgets/*.c`
- Delete matching `backend_*.c`

- [ ] **Step 1: 先迁移已试点控件**

Start with:

- `window`
- `label`
- `button`
- `switch`

because these already have `v2.0` direct-binding truth and are the lowest-risk templates for the rest of the merge.

- [ ] **Step 2: 每迁一个控件，就删掉对应 backend 文件**

For each widget batch:

- merge create/set/get/bind/update behavior into the widget file
- remove product-layer `picoui_backend_*`/`tinyui_backend_*` function boundaries for that widget
- delete the corresponding `backend_*.c`

- [ ] **Step 3: 共享 helper 迁入薄共享层，不保留 backend 边界**

For every helper confirmed to be cross-widget shared:

- move it into `tinyui/src/core/*` or the appropriate shared subsystem
- do not leave it under a `backend/` path

- [ ] **Step 4: 跑 focused proof**

Run after each batch:

```bash
rtk ctest --test-dir build -R '^(test_picoui_window|test_picoui_label|test_picoui_button_events|test_picoui_switch)$' --output-on-failure
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_backend_mapping|check_picoui_visible_ui' --output-on-failure
```

Expected: PASS。

### Task 3: 清零 backend 目录并收口文档

**Files:**
- Delete: remaining `tinyui/src/backend/*`
- Modify: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [ ] **Step 1: 删除剩余 backend 目录**

Only after all product-layer references are removed, delete the now-empty backend directory tree.

- [ ] **Step 2: 跑 broad gate**

Run:

```bash
rtk ctest --test-dir build -L 'picoui' --output-on-failure
python3 tests/picoui/contract/check_tinyui_v21_transition_guards.py
git diff --check
```

Expected: PASS；inventory reports `backend_c_files = 0`。

- [ ] **Step 3: 更新 V2 closeout 真相**

Update docs so they explicitly say:

- independent product-layer backend directory is gone
- widget-specific binding now lives in `widgets/*`
- shared helpers were moved into thin shared subsystems, not a renamed backend
