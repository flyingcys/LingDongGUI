# TINYUI 与 LingDongGUI 测试架构 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `gui-test-coverage` 分支和 `tinyui-abstraction-layer` worktree 里已有测试资产收拢为仓库级统一测试架构，让 `LingDongGUI` 与 `TINYUI` 都拥有分层清晰、默认启用、可按 label 运行的测试体系。

**Architecture:** 根 `CMakeLists.txt` 成为仓库 canonical 入口，`cmake/LingDongGUI.cmake` 承担公共 target / source list / helper function；仓库根新增 `tests/` 作为统一测试树，将 `LingDongGUI` host 单测放入 `tests/lingdonggui/unit/`，将 `TINYUI` 单测/contract/runtime 放入 `tests/tinyui/`，SDL demo 专属测试继续留在 `examples/sdl/tests/`。整个迁移按“先骨架、后 host 单测、再 TINYUI target、再 TINYUI tests、最后瘦 SDL CMake”的顺序串行推进。

**Tech Stack:** C11、CMake、CTest、Python3、LingDongGUI、TINYUI、Arm-2D host stubs、SDL demo 构建链

---

## 文件结构与责任划分

### 新建目录与文件

- Create: `CMakeLists.txt`
- Create: `cmake/LingDongGUI.cmake`
- Create: `tests/CMakeLists.txt`
- Create: `tests/support/CMakeLists.txt`
- Create: `tests/support/ldgui_test_support.c`
- Create: `tests/support/ldgui_test_support.h`
- Create: `tests/support/tinyui_test_support.c`
- Create: `tests/support/tinyui_test_support.h`
- Create: `tests/lingdonggui/CMakeLists.txt`
- Create: `tests/lingdonggui/unit/test_ldbase.c`
- Create: `tests/lingdonggui/unit/test_ldlabel.c`
- Create: `tests/lingdonggui/unit/test_ldprogressbar.c`
- Create: `tests/lingdonggui/unit/test_layout_window.c`
- Create: `tests/lingdonggui/unit/test_ldswitch_internal.c`
- Create: `tests/lingdonggui/unit/test_ldswitch_widget.c`
- Create: `tests/tinyui/CMakeLists.txt`
- Create: `tests/tinyui/unit/test_tinyui_smoke.c`
- Create: `tests/tinyui/unit/test_tinyui_theme.c`
- Create: `tests/tinyui/unit/test_tinyui_widgets.c`
- Create: `tests/tinyui/unit/test_tinyui_layout.c`
- Create: `tests/tinyui/contract/check_tinyui_public_api.py`
- Create: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Create: `tests/tinyui/runtime/check_tinyui_runtime.py`

### 需要修改的现有文件

- Modify: `examples/sdl/CMakeLists.txt`
- Modify: `tinyui/include/tinyui/*.h` 相关 include 结构（仅在迁 TINYUI tests 时按需要微调）
- Modify: `tinyui/src/...` 的 target 归属（通过 CMake 接线，不优先改逻辑）
- Delete or stop referencing: `examples/sdl/tests/tinyui/*`
- Delete or stop referencing: `examples/sdl/tests/check_tinyui_*.py`

### 责任约束

- 根 `CMakeLists.txt`：仓库 canonical 入口；决定是否启用测试。
- `cmake/LingDongGUI.cmake`：公共 sources、公共 targets、公共 test helper。
- `tests/support/`：唯一放公共 host stub / fixture helper 的位置。
- `tests/lingdonggui/`：只放 `LingDongGUI` 库级单测。
- `tests/tinyui/`：只放 `TINYUI` 单测 / contract / runtime。
- `examples/sdl/tests/`：只放 SDL demo 行为测试。

---

### Task 1: 建立根 CMake 与统一测试树骨架

**Files:**
- Create: `CMakeLists.txt`
- Create: `cmake/LingDongGUI.cmake`
- Create: `tests/CMakeLists.txt`
- Create: `tests/support/CMakeLists.txt`

- [ ] **Step 1: 写根 CMake 失败前提检查清单**

记录当前缺口，作为迁移前 baseline：

```text
- 仓库根没有 canonical CMake 入口
- tests/ 目录不存在
- examples/sdl/CMakeLists.txt 同时承担 demo + unit + contract + runtime
- gui-test-coverage 分支的 host test helper 还没有迁回主线
```

- [ ] **Step 2: 先写根 `CMakeLists.txt` 最小骨架**

```cmake
cmake_minimum_required(VERSION 3.16)

project(longdonggui LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

include(CTest)

option(LD_BUILD_SDL_DEMO "Build SDL demo targets" ON)
option(LD_BUILD_RUNTIME_TESTS "Register runtime tests" ON)
option(LD_BUILD_VISUAL_TESTS "Register visual tests" OFF)
option(LD_ENABLE_COVERAGE "Enable coverage flags" OFF)

set(USE_DEMO "2" CACHE STRING "SDL demo id")
set_property(CACHE USE_DEMO PROPERTY STRINGS 0 1 2 3 4 5 6)

include(cmake/LingDongGUI.cmake)

ld_define_core_targets()

if(LD_BUILD_SDL_DEMO)
    add_subdirectory(examples/sdl)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

- [ ] **Step 3: 写 `tests/CMakeLists.txt` 与 `tests/support/CMakeLists.txt` 空骨架**

```cmake
# tests/CMakeLists.txt
add_subdirectory(support)
add_subdirectory(lingdonggui)
add_subdirectory(tinyui)
```

```cmake
# tests/support/CMakeLists.txt
add_library(ldgui_test_support STATIC ldgui_test_support.c)
add_library(tinyui_test_support STATIC tinyui_test_support.c)
```

- [ ] **Step 4: 在 `cmake/LingDongGUI.cmake` 建公共 helper 雏形**

```cmake
function(ld_add_c_unit_test target)
    cmake_parse_arguments(LDTEST "" "SUPPORT_LIB;MAIN_LIB" "SOURCES;LABELS" ${ARGN})
    add_executable(${target} ${LDTEST_SOURCES})
    target_link_libraries(${target} PRIVATE ${LDTEST_SUPPORT_LIB} ${LDTEST_MAIN_LIB})
    add_test(NAME ${target} COMMAND ${target})
    if(LDTEST_LABELS)
        set_tests_properties(${target} PROPERTIES LABELS "${LDTEST_LABELS}")
    endif()
endfunction()
```

- [ ] **Step 5: 运行 configure，确认骨架可被 CMake 识别**

Run:

```bash
rtk cmake -S . -B build/test-arch-s1
```

Expected:

- configure 成功
- 如果 `tests/lingdonggui`、`tests/tinyui` 尚未创建，报的是“缺子目录”这类结构性错误，而不是根入口本身损坏

- [ ] **Step 6: 补齐缺失子目录最小入口并重新 configure**

```cmake
# tests/lingdonggui/CMakeLists.txt
# placeholder subdir for upcoming LingDongGUI unit tests
```

```cmake
# tests/tinyui/CMakeLists.txt
# placeholder subdir for upcoming TINYUI tests
```

Run:

```bash
rtk cmake -S . -B build/test-arch-s1
```

Expected:

- configure 成功

- [ ] **Step 7: 提交骨架**

```bash
git add CMakeLists.txt cmake/LingDongGUI.cmake tests/
git commit -m "build: add repository test architecture skeleton"
```

---

### Task 2: 迁回 LingDongGUI host test support 与旧单测

**Files:**
- Create: `tests/support/ldgui_test_support.c`
- Create: `tests/support/ldgui_test_support.h`
- Create: `tests/lingdonggui/CMakeLists.txt`
- Create: `tests/lingdonggui/unit/test_ldbase.c`
- Create: `tests/lingdonggui/unit/test_ldlabel.c`
- Create: `tests/lingdonggui/unit/test_ldprogressbar.c`
- Create: `tests/lingdonggui/unit/test_layout_window.c`
- Optionally Create: `tests/lingdonggui/unit/test_ldswitch_internal.c`
- Optionally Create: `tests/lingdonggui/unit/test_ldswitch_widget.c`

- [ ] **Step 1: 从旧分支复制 `test_support.c` 为新支撑库**

迁移时保留这些 stub：

```c
bool arm_2d_op_wait_async(arm_2d_op_core_t *ptOP) { (void)ptOP; return true; }
void VT_enter_global_mutex(void) {}
void VT_leave_global_mutex(void) {}
int64_t arm_2d_helper_get_system_timestamp(void) { return g_ld_test_timestamp; }
void ldGuiUpdateScene(void) {}
```

并新增头文件暴露：

```c
void ld_test_set_system_timestamp(int64_t timestamp);
```

- [ ] **Step 2: 在 `tests/support/CMakeLists.txt` 把 `ldgui_test_support` 链到 `longdonggui_host`**

```cmake
add_library(ldgui_test_support STATIC ldgui_test_support.c)
target_include_directories(ldgui_test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ldgui_test_support PUBLIC longdonggui_host)
ld_apply_common_target_config(ldgui_test_support)
```

- [ ] **Step 3: 迁移 `test_ldbase.c`、`test_ldlabel.c`、`test_ldprogressbar.c`、`test_layout_window.c`**

来源：

```text
remotes/flyingcys/gui-test-coverage:src/gui/test/test_ldbase.c
remotes/flyingcys/gui-test-coverage:src/gui/test/test_ldlabel.c
remotes/flyingcys/gui-test-coverage:src/gui/test/test_ldprogressbar.c
remotes/flyingcys/gui-test-coverage:src/gui/test/test_layout_window.c
```

要求：

- 文件路径改到 `tests/lingdonggui/unit/`
- include 路径按新目录修正
- 逻辑断言先尽量保持原样

- [ ] **Step 4: 写 `tests/lingdonggui/CMakeLists.txt` 注册 host unit tests**

```cmake
set(LDGUI_UNIT_TESTS
    unit/test_ldbase.c
    unit/test_ldlabel.c
    unit/test_ldprogressbar.c
    unit/test_layout_window.c
)

foreach(test_src IN LISTS LDGUI_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB ldgui_test_support
        MAIN_LIB longdonggui_host
        LABELS "lingdonggui;unit"
    )
endforeach()
```

- [ ] **Step 5: configure + build 只验证 LingDongGUI 单测目标**

Run:

```bash
rtk cmake -S . -B build/test-arch-s2
rtk cmake --build build/test-arch-s2 --target test_ldbase test_ldlabel test_ldprogressbar test_layout_window
```

Expected:

- 四个 test target 成功编译

- [ ] **Step 6: 跑 `LingDongGUI` 单测**

Run:

```bash
rtk ctest --test-dir build/test-arch-s2 -L lingdonggui --output-on-failure
```

Expected:

- 迁回的 host 单测全绿
- 若有失败，优先修 support / include / target 接线，不顺手改业务逻辑

- [ ] **Step 7: 提交 LingDongGUI 测试迁移**

```bash
git add tests/support tests/lingdonggui cmake/LingDongGUI.cmake
git commit -m "test: restore LingDongGUI host unit tests"
```

---

### Task 3: 抽取 TINYUI 公共 target，消除 SDL CMake 中的重复源列表

**Files:**
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 在 `cmake/LingDongGUI.cmake` 定义 `tinyui_core`**

```cmake
add_library(tinyui_core STATIC
    ${LD_REPO_ROOT}/tinyui/src/core/app.c
    ${LD_REPO_ROOT}/tinyui/src/core/widget.c
    ${LD_REPO_ROOT}/tinyui/src/core/event.c
    ${LD_REPO_ROOT}/tinyui/src/core/resource.c
    ${LD_REPO_ROOT}/tinyui/src/theme/theme.c
    ${LD_REPO_ROOT}/tinyui/src/layout/flex.c
    ${LD_REPO_ROOT}/tinyui/src/layout/grid.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/window.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/label.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/text.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/image.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/button.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/checkbox.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/switch.c
    ${LD_REPO_ROOT}/tinyui/src/widgets/slider.c
)
```

- [ ] **Step 2: 定义 `tinyui_backend_ldgui`**

```cmake
add_library(tinyui_backend_ldgui STATIC
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_widget.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_theme.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_layout.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_event.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_window.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_label.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_text.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_image.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_button.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_checkbox.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_switch.c
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui/backend_slider.c
)
```

并链接：

```cmake
target_link_libraries(tinyui_backend_ldgui PUBLIC tinyui_core longdonggui)
```

- [ ] **Step 3: 给 TINYUI targets 加统一 include dirs / compile config**

```cmake
target_include_directories(tinyui_core PUBLIC
    ${LD_REPO_ROOT}/tinyui/include
    ${LD_REPO_ROOT}/tinyui/src/core
)

target_include_directories(tinyui_backend_ldgui PUBLIC
    ${LD_REPO_ROOT}/tinyui/include
    ${LD_REPO_ROOT}/tinyui/src/core
    ${LD_REPO_ROOT}/tinyui/src/backend/ldgui
)
```

- [ ] **Step 4: 在 `examples/sdl/CMakeLists.txt` 把 TINYUI demo target 改为链接库而不是手抄 sources**

把类似下面这类重复段删掉：

```cmake
add_executable(tinyui_settings_panel_demo
    ... lots of tinyui/src/*.c ...
)
```

改为：

```cmake
add_executable(tinyui_settings_panel_demo
    "${SDL_EXAMPLE_DIR}/../../tinyui/demo/settings_panel/main.c"
)
target_link_libraries(tinyui_settings_panel_demo PRIVATE tinyui_backend_ldgui)
```

- [ ] **Step 5: 只编 TINYUI demo target，确认接线成功**

Run:

```bash
rtk cmake -S . -B build/test-arch-s3
rtk cmake --build build/test-arch-s3 --target tinyui_settings_panel_demo
```

Expected:

- `tinyui_settings_panel_demo` 编译成功
- `examples/sdl/CMakeLists.txt` 中不再存在多份重复 TINYUI 源列表

- [ ] **Step 6: 提交 TINYUI target 收敛**

```bash
git add cmake/LingDongGUI.cmake examples/sdl/CMakeLists.txt
git commit -m "build: factor TINYUI into reusable targets"
```

---

### Task 4: 迁移 TINYUI unit / contract / runtime tests 到统一测试树

**Files:**
- Create: `tests/support/tinyui_test_support.c`
- Create: `tests/support/tinyui_test_support.h`
- Create: `tests/tinyui/CMakeLists.txt`
- Create: `tests/tinyui/unit/test_tinyui_smoke.c`
- Create: `tests/tinyui/unit/test_tinyui_theme.c`
- Create: `tests/tinyui/unit/test_tinyui_widgets.c`
- Create: `tests/tinyui/unit/test_tinyui_layout.c`
- Create: `tests/tinyui/contract/check_tinyui_public_api.py`
- Create: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Create: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 迁移 TINYUI C 单测到 `tests/tinyui/unit/`**

来源：

```text
.worktree/tinyui-abstraction-layer/examples/sdl/tests/tinyui/test_tinyui_smoke.c
.worktree/tinyui-abstraction-layer/examples/sdl/tests/tinyui/test_tinyui_theme.c
.worktree/tinyui-abstraction-layer/examples/sdl/tests/tinyui/test_tinyui_widgets.c
.worktree/tinyui-abstraction-layer/examples/sdl/tests/tinyui/test_tinyui_layout.c
```

要求：

- include 仍使用 `#include "tinyui/tinyui.h"`
- 逻辑保持原样
- 不再从 `examples/sdl/CMakeLists.txt` 注册这些 unit tests

- [ ] **Step 2: 迁移 TINYUI Python tests 到 `tests/tinyui/contract/` 和 `tests/tinyui/runtime/`**

来源：

```text
.worktree/tinyui-abstraction-layer/examples/sdl/tests/check_tinyui_public_api.py
.worktree/tinyui-abstraction-layer/examples/sdl/tests/check_tinyui_demo_boundary.py
.worktree/tinyui-abstraction-layer/examples/sdl/tests/check_tinyui_runtime.py
```

调整：

- 路径改到 `tests/tinyui/...`
- `ROOT` 解析改为以仓库根为准
- `BUILD` 路径使用 `build/tinyui-runtime` 或 `${CMAKE_BINARY_DIR}` 注入方式

- [ ] **Step 3: 写 `tinyui_test_support` 最小骨架**

```c
#include "tinyui_test_support.h"

int tinyui_test_support_stub(void)
{
    return 0;
}
```

```cmake
add_library(tinyui_test_support STATIC tinyui_test_support.c)
target_include_directories(tinyui_test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tinyui_test_support PUBLIC tinyui_core tinyui_backend_ldgui)
ld_apply_common_target_config(tinyui_test_support)
```

- [ ] **Step 4: 写 `tests/tinyui/CMakeLists.txt`**

```cmake
set(PICOUI_UNIT_TESTS
    unit/test_tinyui_smoke.c
    unit/test_tinyui_theme.c
    unit/test_tinyui_widgets.c
    unit/test_tinyui_layout.c
)

foreach(test_src IN LISTS PICOUI_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB tinyui_test_support
        MAIN_LIB tinyui_backend_ldgui
        LABELS "tinyui;unit"
    )
endforeach()

ld_add_python_test(check_tinyui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_public_api.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(check_tinyui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_demo_boundary.py"
    LABELS "tinyui;contract"
)

if(LD_BUILD_RUNTIME_TESTS)
    ld_add_python_test(check_tinyui_runtime
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_runtime.py"
        LABELS "tinyui;runtime"
    )
endif()
```

- [ ] **Step 5: 从 `examples/sdl/CMakeLists.txt` 删除 TINYUI test 注册**

删除：

```text
tinyui_smoke_test
tinyui_theme_test
tinyui_widgets_test
tinyui_layout_test
check_tinyui_public_api
check_tinyui_demo_boundary
check_tinyui_runtime
```

保留：

- `tinyui_settings_panel_demo` 这类 demo target
- SDL demo 专属 check

- [ ] **Step 6: configure + build + 运行 TINYUI tests**

Run:

```bash
rtk cmake -S . -B build/test-arch-s4
rtk cmake --build build/test-arch-s4 --target test_tinyui_smoke test_tinyui_theme test_tinyui_widgets test_tinyui_layout tinyui_settings_panel_demo
rtk ctest --test-dir build/test-arch-s4 -L tinyui --output-on-failure
```

Expected:

- TINYUI unit / contract / runtime tests 全部注册
- TINYUI demo 仍可编
- `examples/sdl/CMakeLists.txt` 不再承担 TINYUI unit test 注册

- [ ] **Step 7: 提交 TINYUI 测试迁移**

```bash
git add tests/tinyui tests/support examples/sdl/CMakeLists.txt cmake/LingDongGUI.cmake
git commit -m "test: move TINYUI tests into repository test tree"
```

---

### Task 5: 瘦身 SDL 测试入口并验证分层执行体验

**Files:**
- Modify: `examples/sdl/CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Modify: `cmake/LingDongGUI.cmake`

- [ ] **Step 1: 审核 `examples/sdl/tests/` 里仍应留在 SDL 层的测试**

保留清单应覆盖：

```text
examples/sdl/tests/layout/*
examples/sdl/tests/switch/*
examples/sdl/tests/swipe/*
examples/sdl/tests/check_use_demo_*.py
examples/sdl/tests/check_stage_*.sh
```

- [ ] **Step 2: 为 SDL 层测试补清晰 label**

示例：

```cmake
set_tests_properties(check_switch_focus_routing PROPERTIES LABELS "sdl;runtime")
set_tests_properties(check_use_demo_5_grid_widget PROPERTIES LABELS "sdl;runtime")
```

- [ ] **Step 3: 确认 `ctest -N` 能看到分层标签**

Run:

```bash
rtk ctest --test-dir build/test-arch-s4 -N
```

Expected:

- 能看到 `lingdonggui`、`tinyui`、`unit`、`contract`、`runtime` 至少五类标签对应的测试

- [ ] **Step 4: 分层验证运行体验**

Run:

```bash
rtk ctest --test-dir build/test-arch-s4 -L unit --output-on-failure
rtk ctest --test-dir build/test-arch-s4 -L contract --output-on-failure
rtk ctest --test-dir build/test-arch-s4 -L runtime --output-on-failure
```

Expected:

- `unit` 只跑 C 单测
- `contract` 只跑 Python contract 检查
- `runtime` 只跑 runtime / SDL 行为检查

- [ ] **Step 5: 最终 diff 健康检查**

Run:

```bash
rtk proxy git diff --check
```

Expected:

- 无 whitespace / conflict-marker 问题

- [ ] **Step 6: 最终提交**

```bash
git add CMakeLists.txt cmake/LingDongGUI.cmake tests examples/sdl/CMakeLists.txt
git commit -m "build: reorganize TINYUI and LingDongGUI tests"
```

---

## 自检

### Spec coverage

- 统一 `tests/` 测试树：Task 1
- LingDongGUI host 单测迁回：Task 2
- TINYUI target 收敛：Task 3
- TINYUI unit/contract/runtime 迁移：Task 4
- SDL 测试职责收口与 label 分层：Task 5

### Placeholder scan

- 无 `TODO` / `TBD`
- 每个任务都给了明确文件路径
- 每个验证步骤都给了命令和预期

### Type consistency

- 公共 helper 名统一为 `ld_add_c_unit_test` / `ld_add_python_test`
- TINYUI target 名统一为 `tinyui_core` / `tinyui_backend_ldgui`
- label 统一为 `lingdonggui` / `tinyui` / `unit` / `contract` / `runtime`
