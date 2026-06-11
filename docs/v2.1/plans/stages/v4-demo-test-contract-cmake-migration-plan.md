# TinyUI v2.1 V4 Demo Test Contract CMake Migration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 demo、tests、contracts、perf artifacts、CMake/CTest 命名和路径全量迁移到 `tinyui`，删除产品层公开命名中的 `picoui`。

**Architecture:** 在 `V1-V3` 已经完成目录与架构收口的前提下，V4 处理仓库里最广的一层 rename：demo 名称、测试文件名、checker 名称、JSON artifact、CTest label、CMake target。它不再改架构，只做全量迁移与 proof 迁移。

**Tech Stack:** CMake、CTest、Python 3、JSON、现有 demo/test/contract/perf tree。

---

## 文件结构

修改或迁移：

- `tests/picoui/*` -> new `tests/tinyui/*` or equivalent unified path
- `tests/picoui/perf/*`
- `tinyui/demo/*`
- `cmake/LingDongGUI.cmake`
- `tests/picoui/CMakeLists.txt` -> successor path if moved
- `docs/v2.1/*`

---

### Task 1: 迁移 demo 与测试树命名

**Files:**
- Move/Rename: demo files under the product-layer tree
- Move/Rename: test files currently named `test_picoui_*`

- [ ] **Step 1: 建立 fail-first residue scan**

Run:

```bash
rg -n "picoui" tinyui tests cmake docs/v2.1
```

Expected: still many hits at the start of V4.

- [ ] **Step 2: demo/test 文件名收口**

Rename:

- `test_picoui_*` -> `test_tinyui_*`
- `check_picoui_*` -> `check_tinyui_*`
- product demo target names -> `tinyui_*`

Keep `LingDongGUI` engine references untouched.

- [ ] **Step 3: 迁移 CTest 注册**

Update test registration so CTest names, labels, and Python checker registration all use `tinyui`.

- [ ] **Step 4: 跑 broad test gate**

Run the renamed product-layer broad gate command after migration:

```bash
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
```

V4 closeout requires this broad gate to pass with `tinyui` labels only. Do not close V4 while any active product-layer CTest registration still uses `picoui` labels or names.

### Task 2: 迁移 contract/perf artifact 命名

**Files:**
- Move/Rename: JSON inventory, baseline, contract artifacts
- Modify: Python checkers

- [ ] **Step 1: 迁移 artifact 文件名**

Rename product-layer artifact names so they no longer contain `picoui`, including:

- transition inventory
- release capability matrix
- perf baseline
- checker script names

- [ ] **Step 2: 更新 checker 内部路径**

Update every Python checker to point at the new `tinyui` artifact names and paths.

- [ ] **Step 3: 跑 contract/perf gate**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS。

### Task 3: 更新 V4 文档真相

**Files:**
- Modify: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [ ] **Step 1: 记录全量迁移范围**

Document that:

- demo/test/contract/CMake naming is now `tinyui`
- product-layer broad gates no longer rely on `picoui` naming
- any remaining `picoui` hit must be either stale docs or non-product historical context to be cleaned in V5

- [ ] **Step 2: 跑 residue scan**

Run:

```bash
rg -n "picoui" tinyui tests cmake docs/v2.1
git diff --check
```

Expected: only approved temporary/documented residue remains for V5 cleanup; no product-layer public naming residue remains.
