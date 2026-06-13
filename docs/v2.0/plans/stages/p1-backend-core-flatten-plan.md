# TinyUI v2.0 P1 Backend Core Flatten Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `backend/ldgui` 从共享系统路径里移出去，将 runtime/widget-tree/event/layout 胶水搬进 `tinyui/src/core/*`，使 `backend` 最多只剩 widget-local 残留；如 shared/core flatten 会打破既有非试点控件 destroy/ownership 合同，允许在本阶段携带极少量 widget-local lifecycle/ownership 修复，但不得借此展开 `P2` 式控件拆平。

**Architecture:** 在 `tinyui/src/core/` 中建立小型 backend-neutral bridge helpers，把 runtime app state 与 widget tree bookkeeping 从 `backend_app.c/backend_widget*.c/backend_event.c` 抽离，并让 `tinyui_core` 在 CMake 上正式拥有 shared layer。当前 `runtime_bridge` 不只承载 app-state / scene / window ownership，还承载 name-id、theme bind、window-switch 这类 shared mutable runtime state bridge。

**Tech Stack:** C11、现有 `tinyui` core/backend split、`LingDongGUI` `ld*` 类型、CMake、CTest。

当前 closeout 证据：

- focused tests：`rtk ctest --test-dir build -R '^(test_tinyui_native_bridge|test_tinyui_app_lifecycle|test_tinyui_app_timer|test_tinyui_layout|test_tinyui_event)$' --output-on-failure` => `5/5 PASS`
- broad gate：`rtk ctest --test-dir build -L 'tinyui' --output-on-failure` => `50/50 PASS`
- 文档与格式：`git diff --check` clean
- fresh review：阶段边界与 `runtime_bridge` 职责文档已修正后 `APPROVED`

---

## 文件结构

新增：

- `tinyui/src/core/runtime_bridge.c`
- `tinyui/src/core/runtime_bridge.h`
- `tests/tinyui/unit/test_tinyui_native_bridge.c`（扩展）

修改：

- `tinyui/src/core/internal.h`
- `tinyui/src/core/event.c`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_app.c`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tinyui/src/backend/ldgui/backend_widget_tree.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt`

受限例外：

- 仅当 shared/core flatten 后既有非试点控件的 destroy/ownership 合同会失真时，可修改对应 widget-local lifecycle/ownership 实现与 focused tests
- 例外范围只允许修复资源归属、销毁释放、backend-host/tree ownership 对齐
- 不允许把例外扩展成试点控件 flatten、public API 扩面、demo 行为重写或新能力开发

---

### Task 1: 抽出 runtime bridge

**Files:**
- Create: `tinyui/src/core/runtime_bridge.h`
- Create: `tinyui/src/core/runtime_bridge.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Modify: `cmake/LingDongGUI.cmake`

- [ ] **Step 1: 写 fail-first runtime bridge test**

Append to `tests/tinyui/unit/test_tinyui_native_bridge.c`:

```c
extern int tinyui_runtime_bridge_has_scene(const struct tinyui_app *app);

static void test_runtime_bridge_reports_scene_presence(void)
{
    struct tinyui_app *app = tinyui_app_create();

    assert(app != NULL);
    assert(tinyui_runtime_bridge_has_scene(app) == 1);
    tinyui_app_destroy(app);
}
```

- [ ] **Step 2: 运行定向 test，确认缺符号**

Run:

```bash
rtk cmake --build build --target test_tinyui_native_bridge
```

Expected: FAIL，缺少 `tinyui_runtime_bridge_has_scene`。

- [ ] **Step 3: 增加 runtime bridge 头和实现**

Create `tinyui/src/core/runtime_bridge.h`:

```c
#ifndef PICOUI_RUNTIME_BRIDGE_H
#define PICOUI_RUNTIME_BRIDGE_H

struct tinyui_app;
struct tinyui_backend_app_state;

int tinyui_runtime_bridge_has_scene(const struct tinyui_app *app);
struct tinyui_backend_app_state *tinyui_runtime_bridge_backend_state(struct tinyui_app *app);

#endif
```

Create `tinyui/src/core/runtime_bridge.c`:

```c
#include "internal.h"
#include "runtime_bridge.h"

int tinyui_runtime_bridge_has_scene(const struct tinyui_app *app)
{
    return app != 0 && app->backend_app != 0;
}

struct tinyui_backend_app_state *tinyui_runtime_bridge_backend_state(struct tinyui_app *app)
{
    if (app == 0 || app->backend_app == 0) {
        return 0;
    }
    return (struct tinyui_backend_app_state *)app->backend_app;
}
```

Add to `tinyui/src/core/internal.h` near existing forward declarations:

```c
struct tinyui_backend_app_state;
```

Add to `cmake/LingDongGUI.cmake` inside `tinyui_core`:

```cmake
        ${LD_REPO_ROOT}/tinyui/src/core/runtime_bridge.c
```

- [ ] **Step 4: 改 `backend_app.c` 读取 bridge helper**

In `tinyui/src/backend/ldgui/backend_app.c`, replace direct `app->backend_app` casts in helper functions with:

```c
#include "runtime_bridge.h"

...
struct tinyui_backend_app_state *state = tinyui_runtime_bridge_backend_state(app);
if (state == NULL) {
    return -1;
}
```

- [ ] **Step 5: 跑 focused tests**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_native_bridge|test_tinyui_app_lifecycle|test_tinyui_app_timer)$' --output-on-failure
```

Expected: PASS。

### Task 2: 抽出 widget tree / event shared glue

**Files:**
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/backend/ldgui/backend_widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_widget_tree.c`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tests/tinyui/unit/test_tinyui_event.c`
- Modify: `tests/tinyui/unit/test_tinyui_layout.c`

- [ ] **Step 1: 给 tree/event helper 写 fail-first test**

Append to `tests/tinyui/unit/test_tinyui_layout.c`:

```c
extern struct tinyui_widget *tinyui_widget_backend_parent(const struct tinyui_widget *widget);

static void test_widget_backend_parent_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *root = tinyui_window_create(app, "root");
    struct tinyui_window *child = tinyui_window_create_child(root, "child");

    assert(app != NULL);
    assert(root != NULL);
    assert(child != NULL);
    assert(tinyui_widget_backend_parent(&child->widget) == &root->widget);

    tinyui_app_destroy(app);
}
```

- [ ] **Step 2: 运行定向 tests，确认 helper 不存在**

Run:

```bash
rtk cmake --build build --target test_tinyui_layout test_tinyui_event
```

Expected: FAIL，缺少 `tinyui_widget_backend_parent` 等 helper。

- [ ] **Step 3: 把 shared tree helper 移到 `widget.c`**

Add declarations in `tinyui/src/core/internal.h`:

```c
struct tinyui_widget *tinyui_widget_backend_parent(const struct tinyui_widget *widget);
int tinyui_widget_bind_backend_parent(struct tinyui_widget *widget, void *backend_parent);
```

Add implementation in `tinyui/src/core/widget.c`:

```c
struct tinyui_widget *tinyui_widget_backend_parent(const struct tinyui_widget *widget)
{
    struct tinyui_backend_widget *backend;

    if (widget == 0 || widget->backend_widget == 0) {
        return 0;
    }
    backend = (struct tinyui_backend_widget *)widget->backend_widget;
    if (backend->parent == 0 || ((struct tinyui_backend_widget *)backend->parent)->host_widget == 0) {
        return 0;
    }
    return ((struct tinyui_backend_widget *)backend->parent)->host_widget;
}
```

- [ ] **Step 4: backend 文件只保留 widget-local glue**

In `backend_widget_tree.c` and `backend_event.c`, replace duplicated parent/host lookup helpers with calls back into `tinyui/src/core/*`.

Representative change:

```c
struct tinyui_widget *parent = tinyui_widget_backend_parent(widget);
if (parent == 0) {
    return -1;
}
```

- [ ] **Step 5: 跑 focused + broad gates**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_layout|test_tinyui_event|test_tinyui_native_bridge)$' --output-on-failure
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
```

Expected: PASS。

- [ ] **Step 6: Commit**

```bash
git add \
  tinyui/src/core/internal.h \
  tinyui/src/core/runtime_bridge.h \
  tinyui/src/core/runtime_bridge.c \
  tinyui/src/core/widget.c \
  tinyui/src/core/event.c \
  tinyui/src/backend/ldgui/backend_app.c \
  tinyui/src/backend/ldgui/backend_widget.c \
  tinyui/src/backend/ldgui/backend_widget_tree.c \
  tinyui/src/backend/ldgui/backend_event.c \
  cmake/LingDongGUI.cmake \
  tests/tinyui/unit/test_tinyui_native_bridge.c \
  tests/tinyui/unit/test_tinyui_layout.c \
  tests/tinyui/unit/test_tinyui_event.c
git commit -m "refactor: flatten shared tinyui backend core"
```
