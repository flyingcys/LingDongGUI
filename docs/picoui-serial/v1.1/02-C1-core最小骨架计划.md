# PicoUI v1.1 C1 Core 最小骨架实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 建立 PicoUI v1.1 的最小 `core + widgets + port` 骨架，并用 contract 把 `picoui_app_*`、`src/native`、`src/backend` 的删除边界固定下来。

**Architecture:** 本阶段不追求大面积功能迁移，只做三件事：先把文档/构建/contract 红线冻结，再建立新的公共 header 与最小 `core` 入口，最后让默认构建图能同时承载旧实现与新骨架，为后续 `C2` 垂直切片提供安全着陆点。

**Tech Stack:** C11, CMake, SDL2, ARM-2D, Python contract checks, GitNexus impact, `rtk cmake`, `rtk ctest`.

---

## Shared References

- 总计划：`docs/picoui-serial/v1.1/01-总实施计划.md`
- 设计文档：`docs/picoui-serial/v1.1/00-重构设计.md`
- 线索引：`docs/picoui-serial/v1.1/线计划索引.md`
- 现有构建：`cmake/LingDongGUI.cmake`
- 现有 public headers：`picoui/include/picoui/*.h`
- 现有 tests：`tests/picoui/CMakeLists.txt`

---

## Task C1-A: 冻结 v1.1 构建和删除红线

**Files:**
- Create: `docs/picoui-serial/v1.1/01-C1-阶段记录.md`
- Create: `tests/picoui/contract/picoui_v1_1_boundary_contract.json`
- Create: `tests/picoui/contract/check_picoui_v1_1_boundary_contract.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.1/线计划索引.md`

- [ ] **Step 1: 写 RED contract checker**

新增 `tests/picoui/contract/check_picoui_v1_1_boundary_contract.py`：

```python
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CONTRACT = ROOT / "tests/picoui/contract/picoui_v1_1_boundary_contract.json"
REQUIRED_DELETE_APIS = {
    "picoui_app_create",
    "picoui_app_run",
    "picoui_app_run_background",
    "picoui_app_set_window",
    "picoui_app_set_background",
    "picoui_app_switch_window",
    "picoui_app_switch_background",
    "picoui_app_timer_create",
    "picoui_app_timer_start",
    "picoui_app_timer_stop",
    "picoui_app_timer_is_running",
    "picoui_app_timer_destroy",
    "picoui_app_destroy",
}


def main() -> int:
    assert CONTRACT.is_file(), f"missing {CONTRACT.relative_to(ROOT)}"
    data = json.loads(CONTRACT.read_text(encoding="utf-8"))
    assert data.get("version") == "v1.1"
    assert set(data.get("delete_public_apis", [])) == REQUIRED_DELETE_APIS
    assert data.get("remove_directories") == [
        "picoui/src/native",
        "picoui/src/backend",
    ]
    assert data.get("forbid_runtime_bones") == [
        "ldBase_t",
        "ld_scene_t",
        "ldMsg",
        "SIGNAL_",
        "arm_2d_control_node_t",
    ]
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

在 `tests/picoui/CMakeLists.txt` 的 contract 区域加入：

```cmake
ld_add_python_test(check_picoui_v1_1_boundary_contract
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_v1_1_boundary_contract.py"
    LABELS "picoui;contract;v1_1"
)
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_v1_1_boundary_contract --output-on-failure
```

Expected:

- FAIL，原因是 `picoui_v1_1_boundary_contract.json` 不存在。

- [ ] **Step 4: 写 contract JSON**

新增 `tests/picoui/contract/picoui_v1_1_boundary_contract.json`：

```json
{
  "version": "v1.1",
  "delete_public_apis": [
    "picoui_app_create",
    "picoui_app_run",
    "picoui_app_run_background",
    "picoui_app_set_window",
    "picoui_app_set_background",
    "picoui_app_switch_window",
    "picoui_app_switch_background",
    "picoui_app_timer_create",
    "picoui_app_timer_start",
    "picoui_app_timer_stop",
    "picoui_app_timer_is_running",
    "picoui_app_timer_destroy",
    "picoui_app_destroy"
  ],
  "remove_directories": [
    "picoui/src/native",
    "picoui/src/backend"
  ],
  "forbid_runtime_bones": [
    "ldBase_t",
    "ld_scene_t",
    "ldMsg",
    "SIGNAL_",
    "arm_2d_control_node_t"
  ]
}
```

- [ ] **Step 5: 更新阶段记录和索引**

在 `docs/picoui-serial/v1.1/01-C1-阶段记录.md` 写入：

```markdown
# PicoUI v1.1 C1 阶段记录

## 当前目标

- 冻结删除边界
- 建立新的 core/header/build 骨架
- 为 C2 垂直切片准备最小着陆点

## 当前红线

- `picoui_app_*` 全部删除，不做兼容层
- `picoui/src/native/` 与 `picoui/src/backend/` 将在 C5 真删，但 C1 起不再允许新代码依赖它们
- 不允许把 `ldBase/ldScene/ldMsg/SIGNAL/control_node` 搬进 PicoUI 新骨架
```

在 `docs/picoui-serial/v1.1/线计划索引.md` 的阶段表中把 `C1` 行状态改成 `进行中`。

- [ ] **Step 6: GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_v1_1_boundary_contract --output-on-failure
rtk git diff --check
```

Expected:

- `check_picoui_v1_1_boundary_contract` PASS
- `git diff --check` exit 0

- [ ] **Step 7: Commit**

```bash
git add \
  docs/picoui-serial/v1.1/01-C1-阶段记录.md \
  docs/picoui-serial/v1.1/线计划索引.md \
  tests/picoui/contract/picoui_v1_1_boundary_contract.json \
  tests/picoui/contract/check_picoui_v1_1_boundary_contract.py \
  tests/picoui/CMakeLists.txt
git commit -m "test: freeze picoui v1.1 boundaries"
```

## Task C1-B: 建立 v1.1 public 入口骨架

**Files:**
- Create: `picoui/include/picoui/core.h`
- Create: `picoui/include/picoui/screen.h`
- Create: `picoui/include/picoui/input.h`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/include/picoui/display.h`
- Modify: `picoui/include/picoui/indev.h`
- Create: `tests/picoui/unit/test_picoui_v1_1_public_headers.c`

- [ ] **Step 1: 写 RED public header test**

新增 `tests/picoui/unit/test_picoui_v1_1_public_headers.c`：

```c
#include "picoui/picoui.h"

int main(void)
{
    (void)picoui_init;
    (void)picoui_deinit;
    (void)picoui_timer_handler;
    (void)picoui_screen_create;
    (void)picoui_screen_load;
    (void)picoui_screen_active;
    (void)picoui_display_create;
    (void)picoui_indev_create;
    return 0;
}
```

- [ ] **Step 2: 注册 test**

在 `tests/picoui/CMakeLists.txt` 中的 unit test 列表加入：

```cmake
    unit/test_picoui_v1_1_public_headers.c
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk cmake --build build --target test_picoui_v1_1_public_headers
```

Expected:

- FAIL，缺少 `picoui_screen_*` 或 `picoui_init` 声明。

- [ ] **Step 4: 新建最小 public header**

新增 `picoui/include/picoui/core.h`：

```c
#ifndef PICOUI_CORE_H
#define PICOUI_CORE_H

int picoui_init(void);
void picoui_deinit(void);
int picoui_timer_handler(void);

#endif
```

新增 `picoui/include/picoui/screen.h`：

```c
#ifndef PICOUI_SCREEN_H
#define PICOUI_SCREEN_H

struct picoui_screen;

struct picoui_screen *picoui_screen_create(void);
struct picoui_screen *picoui_screen_active(void);
int picoui_screen_load(struct picoui_screen *screen);

#endif
```

新增 `picoui/include/picoui/input.h`：

```c
#ifndef PICOUI_INPUT_H
#define PICOUI_INPUT_H

struct picoui_indev;

struct picoui_indev *picoui_indev_create(void);

#endif
```

在 `picoui/include/picoui/picoui.h` 中加入：

```c
#include "picoui/core.h"
#include "picoui/screen.h"
#include "picoui/input.h"
```

- [ ] **Step 5: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_v1_1_public_headers
rtk ctest --test-dir build -R test_picoui_v1_1_public_headers --output-on-failure
rtk git diff --check
```

Expected:

- PASS

- [ ] **Step 6: Commit**

```bash
git add \
  picoui/include/picoui/core.h \
  picoui/include/picoui/screen.h \
  picoui/include/picoui/input.h \
  picoui/include/picoui/picoui.h \
  picoui/include/picoui/display.h \
  picoui/include/picoui/indev.h \
  tests/picoui/unit/test_picoui_v1_1_public_headers.c \
  tests/picoui/CMakeLists.txt
git commit -m "feat: add picoui v1.1 public skeleton"
```

## Task C1-C: 建立最小 core 状态和实现文件

**Files:**
- Create: `picoui/src/core/core.c`
- Create: `picoui/src/core/screen.c`
- Create: `picoui/src/core/input.c`
- Create: `picoui/src/core/runtime_state.h`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `tests/picoui/unit/test_picoui_v1_1_public_headers.c`

- [ ] **Step 1: GitNexus impact**

Run:

```text
mcp__gitnexus.impact({repo:"LingDongGUI", target:"picoui_init", direction:"upstream"})
```

Expected:

- 返回 blast radius；若 HIGH/CRITICAL，先回主线程汇报。

- [ ] **Step 2: 写 RED lifecycle test**

把 `tests/picoui/unit/test_picoui_v1_1_public_headers.c` 改成：

```c
#include "picoui/picoui.h"
#include <assert.h>

int main(void)
{
    struct picoui_screen *screen;

    assert(picoui_init() == 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    assert(picoui_screen_load(screen) == 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    return 0;
}
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk cmake --build build --target test_picoui_v1_1_public_headers
```

Expected:

- LINK FAIL，因为函数无实现。

- [ ] **Step 4: 写最小实现**

新增 `picoui/src/core/runtime_state.h`：

```c
#ifndef PICOUI_RUNTIME_STATE_H
#define PICOUI_RUNTIME_STATE_H

struct picoui_screen {
    int active;
};

struct picoui_runtime_state {
    int initialized;
    struct picoui_screen default_screen;
};

struct picoui_runtime_state *picoui_runtime_state(void);

#endif
```

新增 `picoui/src/core/core.c`：

```c
#include "runtime_state.h"
#include "picoui/core.h"

static struct picoui_runtime_state g_runtime;

struct picoui_runtime_state *picoui_runtime_state(void)
{
    return &g_runtime;
}

int picoui_init(void)
{
    struct picoui_runtime_state *rt = picoui_runtime_state();
    rt->initialized = 1;
    rt->default_screen.active = 1;
    return 0;
}

void picoui_deinit(void)
{
    struct picoui_runtime_state *rt = picoui_runtime_state();
    rt->initialized = 0;
    rt->default_screen.active = 0;
}

int picoui_timer_handler(void)
{
    return picoui_runtime_state()->initialized ? 0 : -1;
}
```

新增 `picoui/src/core/screen.c`：

```c
#include "runtime_state.h"
#include "picoui/screen.h"

struct picoui_screen *picoui_screen_create(void)
{
    return picoui_screen_active();
}

struct picoui_screen *picoui_screen_active(void)
{
    return &picoui_runtime_state()->default_screen;
}

int picoui_screen_load(struct picoui_screen *screen)
{
    return screen == 0 ? -1 : 0;
}
```

新增 `picoui/src/core/input.c`：

```c
#include "picoui/input.h"

struct picoui_indev {
    int placeholder;
};

static struct picoui_indev g_default_indev;

struct picoui_indev *picoui_indev_create(void)
{
    return &g_default_indev;
}
```

- [ ] **Step 5: 加入 CMake**

在 `cmake/LingDongGUI.cmake` 的 `picoui_core` source list 顶部加入：

```cmake
        ${LD_REPO_ROOT}/picoui/src/core/core.c
        ${LD_REPO_ROOT}/picoui/src/core/screen.c
        ${LD_REPO_ROOT}/picoui/src/core/input.c
```

- [ ] **Step 6: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_v1_1_public_headers
rtk ctest --test-dir build -R test_picoui_v1_1_public_headers --output-on-failure
rtk git diff --check
```

Expected:

- PASS

- [ ] **Step 7: Commit**

```bash
git add \
  picoui/src/core/core.c \
  picoui/src/core/screen.c \
  picoui/src/core/input.c \
  picoui/src/core/runtime_state.h \
  cmake/LingDongGUI.cmake \
  tests/picoui/unit/test_picoui_v1_1_public_headers.c
git commit -m "feat: add picoui v1.1 core skeleton"
```

## Task C1-D: 引入 v1.1 构建目标别名

**Files:**
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/contract/check_picoui_v1_1_build_graph.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.1/01-C1-阶段记录.md`

- [ ] **Step 1: 写 RED build-graph checker**

新增 `tests/picoui/contract/check_picoui_v1_1_build_graph.py`：

```python
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
TEXT = (ROOT / "cmake/LingDongGUI.cmake").read_text(encoding="utf-8")


def main() -> int:
    assert "add_library(picoui_core_v1_1 STATIC" in TEXT
    assert "add_library(picoui_widgets_v1_1 STATIC" in TEXT
    assert "add_library(picoui_port_v1_1 INTERFACE" in TEXT
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

在 `tests/picoui/CMakeLists.txt` 中加入：

```cmake
ld_add_python_test(check_picoui_v1_1_build_graph
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_v1_1_build_graph.py"
    LABELS "picoui;contract;v1_1"
)
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_v1_1_build_graph --output-on-failure
```

Expected:

- FAIL，因为 `picoui_core_v1_1` 等 target 尚不存在。

- [ ] **Step 4: 加入新 target 占位**

在 `cmake/LingDongGUI.cmake` 中 `ld_build_picoui` 内添加：

```cmake
    add_library(picoui_core_v1_1 STATIC
        ${LD_REPO_ROOT}/picoui/src/core/core.c
        ${LD_REPO_ROOT}/picoui/src/core/screen.c
        ${LD_REPO_ROOT}/picoui/src/core/input.c
    )
    target_include_directories(picoui_core_v1_1 PUBLIC
        ${LD_REPO_ROOT}/picoui/include
        ${LD_REPO_ROOT}/picoui/src/core
        ${LD_COMMON_INCLUDE_DIRS}
    )
    ld_apply_common_target_config(picoui_core_v1_1)

    add_library(picoui_widgets_v1_1 STATIC)
    target_link_libraries(picoui_widgets_v1_1 PUBLIC picoui_core_v1_1)
    ld_apply_common_target_config(picoui_widgets_v1_1)

    add_library(picoui_port_v1_1 INTERFACE)
    target_link_libraries(picoui_port_v1_1 INTERFACE picoui_port_sdl)
```

- [ ] **Step 5: GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_v1_1_build_graph --output-on-failure
rtk git diff --check
```

Expected:

- PASS

- [ ] **Step 6: 更新阶段记录并提交**

在 `docs/picoui-serial/v1.1/01-C1-阶段记录.md` 增加：

```markdown
## 当前完成

- `picoui_core_v1_1`
- `picoui_widgets_v1_1`
- `picoui_port_v1_1`

说明：这些 target 当前只是占位骨架，用于后续 C2/C3/C4 渐进迁移，不代表旧 target 已下线。
```

```bash
git add \
  cmake/LingDongGUI.cmake \
  tests/picoui/contract/check_picoui_v1_1_build_graph.py \
  tests/picoui/CMakeLists.txt \
  docs/picoui-serial/v1.1/01-C1-阶段记录.md
git commit -m "build: add picoui v1.1 target skeleton"
```

## Task C1-E: C1 阶段 closeout

**Files:**
- Modify: `docs/picoui-serial/v1.1/01-C1-阶段记录.md`
- Modify: `docs/picoui-serial/v1.1/线计划索引.md`

- [ ] **Step 1: 跑 C1 验证集**

Run:

```bash
rtk ctest --test-dir build -R 'check_picoui_v1_1_boundary_contract|check_picoui_v1_1_build_graph|test_picoui_v1_1_public_headers' --output-on-failure
rtk git diff --check
```

Expected:

- 三项 PASS
- `git diff --check` exit 0

- [ ] **Step 2: 主线程跑 detect_changes**

Run:

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"unstaged"})
```

Expected:

- 变更范围只落在 `docs/picoui-serial/v1.1`、`tests/picoui/contract`、`tests/picoui/CMakeLists.txt`、`picoui/include/picoui`、`picoui/src/core`、`cmake/LingDongGUI.cmake`

- [ ] **Step 3: 更新 closeout 状态**

在 `docs/picoui-serial/v1.1/01-C1-阶段记录.md` 追加：

```markdown
## Closeout

- `C1 gate`：PASS
- 下一阶段：`C2`
- 允许进入 `window/label/button` 垂直切片
```

在 `docs/picoui-serial/v1.1/线计划索引.md` 中：

- 把 `C1` 改成 `已完成`
- 把 `C2` 改成 `进行中`

- [ ] **Step 4: Commit**

```bash
git add docs/picoui-serial/v1.1/01-C1-阶段记录.md docs/picoui-serial/v1.1/线计划索引.md
git commit -m "docs: close picoui v1.1 c1 phase"
```
