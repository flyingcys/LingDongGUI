# SDL CMake 迁移 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `examples/sdl` 改为仅通过 CMake 构建，删除 Makefile/Qt 工程入口，并保留 Windows + Linux 的 SDL 构建行为。

**Architecture:** 在 `examples/sdl/CMakeLists.txt` 中集中声明公共源码、demo 源码、平台 SDL2 接入和编译宏；Windows 使用仓库内 SDL2 资源，Linux 使用 `pkg-config`；README 只保留 CMake 用法说明。

**Tech Stack:** CMake, C11, SDL2, pkg-config, GCC/MinGW

---

### Task 1: 建立唯一 CMake 构建入口

**Files:**
- Create: `examples/sdl/CMakeLists.txt`
- Reference: `examples/sdl/makefile`
- Reference: `examples/sdl/qt_sdl.pro`

- [ ] **Step 1: 先跑失败的配置命令，确认当前缺少 CMake 入口**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
cmake -S examples/sdl -B build/sdl-test
```

Expected: 失败，并提示 `examples/sdl` 缺少 `CMakeLists.txt`。

- [ ] **Step 2: 新建最小 `CMakeLists.txt` 骨架**

```cmake
cmake_minimum_required(VERSION 3.16)
project(ldgui_sdl_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(USE_DEMO 2 CACHE STRING "SDL demo id (1=startup, 2=widget, 3=printer)")
set_property(CACHE USE_DEMO PROPERTY STRINGS 1 2 3)
```

- [ ] **Step 3: 补齐源码、头文件目录、编译宏与 demo 选择逻辑**

```cmake
set(SDL_DEMO_COMMON_SOURCES
    virtualNor/virtualNor.c
    user/main.c
    user/ldConfig.c
    user/Virtual_TFT_Port.c
    user/arm_2d_disp_adapter_0.c)

if(USE_DEMO STREQUAL "1")
    file(GLOB SDL_DEMO_APP_SOURCES CONFIGURE_DEPENDS ../common/demo/startup/*.c ../common/demo/startup/fonts/*.c ../common/demo/startup/images/*.c)
elseif(USE_DEMO STREQUAL "2")
    file(GLOB SDL_DEMO_APP_SOURCES CONFIGURE_DEPENDS ../common/demo/widget/*.c ../common/demo/widget/fonts/*.c ../common/demo/widget/images/*.c)
elseif(USE_DEMO STREQUAL "3")
    file(GLOB SDL_DEMO_APP_SOURCES CONFIGURE_DEPENDS ../common/demo/printer/*.c ../common/demo/printer/fonts/*.c ../common/demo/printer/images/*.c)
else()
    message(FATAL_ERROR "Unsupported USE_DEMO=${USE_DEMO}")
endif()
```

- [ ] **Step 4: 接入 SDL2 平台分支、目标定义和 Windows DLL 复制逻辑**

```cmake
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/64")
    else()
        set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/32")
    endif()

    target_include_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/include/SDL2")
    target_link_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/lib")
    target_link_libraries(ldgui_sdl_demo PRIVATE SDL2 SDL2main pthread)

    add_custom_command(TARGET ldgui_sdl_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL2_ROOT}/bin/SDL2.dll"
                "$<TARGET_FILE_DIR:ldgui_sdl_demo>/SDL2.dll")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    target_include_directories(ldgui_sdl_demo PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_directories(ldgui_sdl_demo PRIVATE ${SDL2_LIBRARY_DIRS})
    target_link_libraries(ldgui_sdl_demo PRIVATE ${SDL2_LIBRARIES} pthread)
endif()
```

- [ ] **Step 5: 重新跑配置命令，确认 CMake 能成功生成构建文件**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
cmake -S examples/sdl -B build/sdl-test -DUSE_DEMO=2
```

Expected: 配置成功，生成 `build/sdl-test`。

### Task 2: 清理旧入口并更新 README

**Files:**
- Modify: `examples/sdl/README.md`
- Delete: `examples/sdl/makefile`
- Delete: `examples/sdl/qt_sdl.pro`

- [ ] **Step 1: 先写 README 新结构，只保留 CMake 用法**

```markdown
# SDL Demo

## 依赖

### Windows
- CMake
- MinGW-w64 或兼容 GCC

### Linux
- CMake
- gcc
- pkg-config
- SDL2 开发包
```

- [ ] **Step 2: 在 README 中加入实际命令与 demo 切换方法**

```markdown
## 配置与编译

```bash
cmake -S . -B build -DUSE_DEMO=2
cmake --build build
```

## 运行

```bash
./build/ldgui_sdl_demo
```
```

- [ ] **Step 3: 删除旧入口文件，确保目录只剩 CMake 正式入口**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523/examples/sdl
rm -f makefile qt_sdl.pro
```

- [ ] **Step 4: 搜索旧入口关键字，确认 README 不再残留旧构建方式**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
rg -n "mingw32-make|qt_sdl\.pro|Makefile SDL Demo|VSCode SDL Demo|Qt SDL Demo" examples/sdl/README.md
```

Expected: 无输出。

### Task 3: 构建验证与收尾

**Files:**
- Verify: `examples/sdl/CMakeLists.txt`
- Verify: `examples/sdl/README.md`

- [ ] **Step 1: 运行完整构建，验证 CMake 迁移可用**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
cmake -S examples/sdl -B build/sdl-test -DUSE_DEMO=2
cmake --build build/sdl-test
```

Expected: 配置和编译均成功。

- [ ] **Step 2: 检查生成产物与 Windows DLL 复制规则**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
ls -la build/sdl-test
cmake --build build/sdl-test --target help | sed -n 1,120p
```

Expected: 出现 `ldgui_sdl_demo` 目标；Windows 环境下输出目录包含 `SDL2.dll`。

- [ ] **Step 3: 检查最终变更集，确认只包含本次迁移相关文件**

```bash
cd /Users/cys/embedded/LingDongGUI/.worktree/cmake-sdl-20260523
git status --short
```

Expected: 只出现 `examples/sdl/CMakeLists.txt`、`examples/sdl/README.md`、`examples/sdl/makefile`、`examples/sdl/qt_sdl.pro` 以及本次新增 spec/plan 文档。
