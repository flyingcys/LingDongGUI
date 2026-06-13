# TinyUI v2.1 V4 Demo Test Contract CMake Migration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 demo、tests、contracts、perf artifacts、CMake/CTest 命名和路径全量迁移到 `tinyui`，删除产品层公开命名中的 `tinyui`。

**Architecture:** 在 `V1-V3` 已经完成目录与架构收口的前提下，V4 处理仓库里最广的一层 rename：demo 名称、测试文件名、checker 名称、JSON artifact、CTest label、CMake target。它不再改架构，只做全量迁移与 proof 迁移。

**Tech Stack:** CMake、CTest、Python 3、JSON、现有 demo/test/contract/perf tree。

---

## 文件结构

修改或迁移：

- `tests/tinyui/*` -> new `tests/tinyui/*` or equivalent unified path
- `tests/tinyui/perf/*`
- `tinyui/demo/*`
- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt` -> successor path if moved
- `docs/v2.1/*`

---

### Task 1: 迁移 demo 与测试树命名

**Files:**
- Move/Rename: demo files under the product-layer tree
- Move/Rename: test files currently named `test_tinyui_*`

- [x] **Step 1: 建立 fail-first residue scan**

Run:

```bash
rg -n "tinyui" tinyui tests cmake docs/v2.1
```

Expected: still many hits at the start of V4.

- [x] **Step 2: demo/test 文件名收口**

Rename:

- `test_tinyui_*` -> `test_tinyui_*`
- `check_tinyui_*` -> `check_tinyui_*`
- product demo target names -> `tinyui_*`

Keep `LingDongGUI` engine references untouched.

当前完成态：

- canonical `tests/tinyui/*`、`check_tinyui_*` 与 `test_tinyui_*` 已建立
- `tests/tinyui/*` 当前只保留历史兼容入口、compat shell 与历史资产说明，不再代表 current live checker / broad gate 主线

- [x] **Step 3: 迁移 CTest 注册**

Update test registration so CTest names, labels, and Python checker registration all use `tinyui`.

当前已完成的最小闭环：

- `tests/tinyui/CMakeLists.txt` 的 unit CTest 名已切到 `test_tinyui_*`
- contract/runtime/perf 的 CTest 名与 label 已切到 `check_tinyui_*` 和 `tinyui`
- Python checker 注册已切到 `tests/tinyui/{contract,runtime,perf}/*` canonical 路径
- `examples/sdl/CMakeLists.txt` 已建立 `add_tinyui_demo()`，`cmake/LingDongGUI.cmake` 已建立 `ld_apply_tinyui_runtime_screen_config()`
- `tests/tinyui/unit/*` 已建立为 canonical unit 路径；原 `tests/tinyui/unit/test_tinyui_*.c` 已物理迁到 `tests/tinyui/unit/test_tinyui_*.c`
- `tests/tinyui/CMakeLists.txt` 现已直接引用 `../tinyui/unit/test_tinyui_*.c`，`tests/tinyui/unit/` 已清空
- 这一步 fresh focused proof 已通过：`rtk ctest --test-dir build --output-on-failure -R '^(test_tinyui_app_timer|test_tinyui_app_lifecycle|test_tinyui_layout|test_tinyui_keyboard|test_tinyui_wrapper_struct_overhead)$'`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json` 的 canonical 证据串已统一到 `test_tinyui_*` / `check_tinyui_*` / `tests/tinyui/runtime/*`
- `tests/tinyui/{contract,perf,runtime}` 当前剩余 `check_tinyui_*` / `check_tinyui_tinyui_*` 已进一步收紧为 thin wrapper，最小兼容验证已通过：`check_tinyui_runtime.py --help`、`check_tinyui_visible_ui.py --help`、`check_tinyui_tinyui_perf.py --self-test`
- `tests/tinyui/contract/*` 旧真相源里的证据串当前也已进一步统一到 TinyUI 口径；同时 `check_tinyui_native_100_inventory.py` 与 `tinyui_native_100_inventory.json` 已补齐 `canvas`，native-100 inventory gate 已重新通过
- include 消费面当前已完成第一小批 safe move：`tinyui/port/sdl/sdl.c` 已切到顶层 `display.h`/`osal.h`/`tick.h`，`tinyui/demo/animation_basic/main.c` 已切到顶层 `image.h`；fresh proof `rtk cmake -S . -B build` 与 `rtk cmake --build build --target tinyui_animation_basic_demo test_tinyui_port_sdl test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os` 已通过
- canonical 头可组合性当前已完成最小修复：`tinyui/include/{core,screen,label,button,switch}.h` 不再重复定义兼容头已经提供的 `tinyui_*` inline/宏；`tests/tinyui/contract/check_tinyui_v21_transition_guards.py` 已新增多头组合 `cc -fsyntax-only` probe；去重后 `tinyui_v21_transition_inventory.json` 的 `tinyui_public_api_count` 已更新为 `26`，`tinyui_transition_inventory.json` 已更新为 `21`
- broad gate 修复当前已完成：完整执行 `rtk cmake --build build` 后，`rtk ctest --test-dir build -L 'tinyui' --output-on-failure` 已 fresh 通过 `56/56`；`test_tinyui_{gauge,image,graph,qrcode}` 与 `check_tinyui_release_capability_matrix` 的 V4 迁移后路径/旧证据串红项已修复

当前剩余：

- `test_tinyui_*` / `check_tinyui_*` 的仓库级文件名与证据串仍未全量清零，主要收敛到 `tests/tinyui/contract/*` 的旧真相源与 docs 历史记录
- `check_tinyui_demo_boundary` 与 `check_tinyui_widget_contract_matrix` 已切到 `tests/tinyui/contract/*` canonical 入口；`tests/tinyui/contract/check_tinyui_{demo_boundary,widget_contract_matrix}.py` 当前只保留兼容转发壳
- baseline inventory、线计划索引与 orchestration 文档当前已明确：`tests/tinyui/contract/*` 是 canonical 入口，`tests/tinyui/contract/*` 是历史兼容/旧真相源入口
- `check_tinyui_public_api.py`、`check_tinyui_tinyui_transition_guards.py`、`check_tinyui_release_capability_matrix.py` 当前已收口为 canonical `tests/tinyui/contract/*` 转发壳；与 canonical 完全一致的 `tinyui_{transition_inventory,v21_transition_inventory,release_capability_matrix}.json` 兼容副本已退场；`tests/tinyui/contract/*` 下剩余待退场重点已收敛到旧真相源文件名与仍有独立用途的旧 JSON/ledger 资产
- `tinyui/include/tinyui/*` 兼容 public include 子树当前不能直接删：`tinyui/include/*.h`、`tinyui/src/*`、`tinyui/demo/*`、`tests/tinyui/unit/*` 与 canonical contract checker 仍直接依赖它；最小安全顺序是先迁 consumer/header/checker 面，再在 `V5` 退场
- include consumer 面此前的多头组合 `tinyui_*` inline redefinition blocker 已修；下一批可以继续小批迁 `tinyui/demo/*`、`tests/tinyui/unit/*` 与 canonical checker 的 include consumer 面，但仍不得直接删除 `tinyui/include/tinyui/*`

- [x] **Step 4: 跑 broad test gate**

Run the renamed product-layer broad gate command after migration:

```bash
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
```

V4 closeout requires this broad gate to pass with `tinyui` labels only. Do not close V4 while any active product-layer CTest registration still uses `tinyui` labels or names.

当前 fresh proof：

```bash
rtk cmake --build build
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
```

结果：`100% tests passed, 0 tests failed out of 56`。

### Task 2: 迁移 contract/perf artifact 命名

**Files:**
- Move/Rename: JSON inventory, baseline, contract artifacts
- Modify: Python checkers

- [x] **Step 1: 迁移 artifact 文件名**

Rename product-layer artifact names so they no longer contain `tinyui`, including:

- transition inventory
- release capability matrix
- perf baseline
- checker script names

- [x] **Step 2: 更新 checker 内部路径**

Update every Python checker to point at the new `tinyui` artifact names and paths.

- [x] **Step 3: 跑 contract/perf gate**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS。

当前 fresh proof：

- `python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- `python3 tests/tinyui/contract/check_tinyui_public_api.py`
- `python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py`
- `rtk ctest --test-dir build -L 'perf' --output-on-failure`

结果：PASS。

### Task 3: 更新 V4 文档真相

**Files:**
- Modify: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [x] **Step 1: 记录全量迁移范围**

Document that:

- demo/test/contract/CMake naming is now `tinyui`
- product-layer broad gates no longer rely on `tinyui` naming
- any remaining `tinyui` hit must be either stale docs or non-product historical context to be cleaned in V5

- [x] **Step 2: 跑 residue scan**

Run:

```bash
rg -n "tinyui" tinyui tests cmake docs/v2.1
git diff --check
```

Expected: only approved temporary/documented residue remains for V5 cleanup; no product-layer public naming residue remains.

当前 residue scan 真相：

- `tests/tinyui/{contract,runtime,perf}` 已是 canonical
- `tests/tinyui/unit/*` 已是 canonical；`tests/tinyui/unit/` 当前已清空
- `tests/tinyui/{contract,runtime,perf}` 当前只保留兼容薄壳/兼容副本
- 主要未清零残留已收敛到 `tests/tinyui/contract/*` 旧入口文件名/legacy loader、docs 历史记录，以及仍在服役的 `tinyui/include/tinyui/*` 兼容 public include 子树
