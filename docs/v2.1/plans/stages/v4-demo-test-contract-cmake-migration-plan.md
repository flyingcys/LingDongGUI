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

- [x] **Step 1: 建立 fail-first residue scan**

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

- [x] **Step 3: 迁移 CTest 注册**

Update test registration so CTest names, labels, and Python checker registration all use `tinyui`.

当前已完成的最小闭环：

- `tests/picoui/CMakeLists.txt` 的 unit CTest 名已切到 `test_tinyui_*`
- contract/runtime/perf 的 CTest 名与 label 已切到 `check_tinyui_*` 和 `tinyui`
- Python checker 注册已切到 `tests/tinyui/{contract,runtime,perf}/*` canonical 路径
- `examples/sdl/CMakeLists.txt` 已建立 `add_tinyui_demo()`，`cmake/LingDongGUI.cmake` 已建立 `ld_apply_tinyui_runtime_screen_config()`
- `tests/tinyui/unit/*` 已建立为 canonical unit 路径；原 `tests/picoui/unit/test_picoui_*.c` 已物理迁到 `tests/tinyui/unit/test_tinyui_*.c`
- `tests/picoui/CMakeLists.txt` 现已直接引用 `../tinyui/unit/test_tinyui_*.c`，`tests/picoui/unit/` 已清空
- 这一步 fresh focused proof 已通过：`rtk ctest --test-dir build --output-on-failure -R '^(test_tinyui_app_timer|test_tinyui_app_lifecycle|test_tinyui_layout|test_tinyui_keyboard|test_tinyui_wrapper_struct_overhead)$'`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json` 的 canonical 证据串已统一到 `test_tinyui_*` / `check_tinyui_*` / `tests/tinyui/runtime/*`
- `tests/picoui/{contract,perf,runtime}` 当前剩余 `check_picoui_*` / `check_picoui_tinyui_*` 已进一步收紧为 thin wrapper，最小兼容验证已通过：`check_picoui_runtime.py --help`、`check_picoui_visible_ui.py --help`、`check_picoui_tinyui_perf.py --self-test`
- `tests/picoui/contract/*` 旧真相源里的证据串当前也已进一步统一到 TinyUI 口径；同时 `check_picoui_native_100_inventory.py` 与 `picoui_native_100_inventory.json` 已补齐 `canvas`，native-100 inventory gate 已重新通过
- include 消费面当前已完成第一小批 safe move：`tinyui/port/sdl/sdl.c` 已切到顶层 `display.h`/`osal.h`/`tick.h`，`tinyui/demo/animation_basic/main.c` 已切到顶层 `image.h`；fresh proof `rtk cmake -S . -B build` 与 `rtk cmake --build build --target tinyui_animation_basic_demo test_tinyui_port_sdl test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os` 已通过
- canonical 头可组合性当前已完成最小修复：`tinyui/include/{core,screen,label,button,switch}.h` 不再重复定义兼容头已经提供的 `tinyui_*` inline/宏；`tests/tinyui/contract/check_tinyui_v21_transition_guards.py` 已新增多头组合 `cc -fsyntax-only` probe；去重后 `tinyui_v21_transition_inventory.json` 的 `tinyui_public_api_count` 已更新为 `26`，`tinyui_transition_inventory.json` 已更新为 `21`

当前剩余：

- `test_picoui_*` / `check_picoui_*` 的仓库级文件名与证据串仍未全量清零，主要收敛到 `tests/picoui/contract/*` 的旧真相源与 docs 历史记录
- `tests/picoui/contract/*` 旧入口文件名本身与少量 legacy loader 仍在，当前验证命令仍依赖它们，留待 `V5`
- broad gate `rtk ctest --test-dir build -L 'tinyui' --output-on-failure` 尚未作为全量 V4 closeout fresh 通过
- `tinyui/include/picoui/*` 兼容 public include 子树当前不能直接删：`tinyui/include/*.h`、`tinyui/src/*`、`tinyui/demo/*`、`tests/tinyui/unit/*` 与 canonical contract checker 仍直接依赖它；最小安全顺序是先迁 consumer/header/checker 面，再在 `V5` 退场
- include consumer 面此前的多头组合 `tinyui_*` inline redefinition blocker 已修；下一批可以继续小批迁 `tinyui/demo/*`、`tests/tinyui/unit/*` 与 canonical checker 的 include consumer 面，但仍不得直接删除 `tinyui/include/picoui/*`

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

- [x] **Step 1: 迁移 artifact 文件名**

Rename product-layer artifact names so they no longer contain `picoui`, including:

- transition inventory
- release capability matrix
- perf baseline
- checker script names

- [x] **Step 2: 更新 checker 内部路径**

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

- [x] **Step 1: 记录全量迁移范围**

Document that:

- demo/test/contract/CMake naming is now `tinyui`
- product-layer broad gates no longer rely on `picoui` naming
- any remaining `picoui` hit must be either stale docs or non-product historical context to be cleaned in V5

- [x] **Step 2: 跑 residue scan**

Run:

```bash
rg -n "picoui" tinyui tests cmake docs/v2.1
git diff --check
```

Expected: only approved temporary/documented residue remains for V5 cleanup; no product-layer public naming residue remains.

当前 residue scan 真相：

- `tests/tinyui/{contract,runtime,perf}` 已是 canonical
- `tests/tinyui/unit/*` 已是 canonical；`tests/picoui/unit/` 当前已清空
- `tests/picoui/{contract,runtime,perf}` 当前只保留兼容薄壳/兼容副本
- 主要未清零残留已收敛到 `tests/picoui/contract/*` 旧入口文件名/legacy loader、docs 历史记录，以及仍在服役的 `tinyui/include/picoui/*` 兼容 public include 子树
