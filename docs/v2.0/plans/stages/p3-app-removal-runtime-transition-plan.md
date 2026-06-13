# TinyUI v2.0 P3 App Removal And Runtime Transition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让用户主路径摆脱 `app`，改为 LVGL-like runtime 入口，同时在迁移期保住现有内部实现的兼容性。

**Architecture:** 先在当前 `tinyui` public headers 上新增 app-free runtime surface，再把 demo 与 runtime loop 改成 `init/display/indev/screen/timer_handler`，并将 `tinyui_app_*` 降级为兼容内部实现而不是主用户模型。

**Tech Stack:** C11、SDL2、当前 `tinyui/src/core/app.c`、`tinyui/src/tick/tick.c`、demos、unit/runtime tests。

---

## 文件结构

新增：

- `tinyui/include/tinyui/runtime.h`
- `tinyui/src/core/runtime.c`
- `tests/tinyui/unit/test_tinyui_runtime_model.c`

修改：

- `tinyui/include/tinyui/tinyui.h`
- `tinyui/include/tinyui/app.h`
- `tinyui/src/core/app.c`
- `tinyui/src/core/internal.h`
- `tinyui/demo/basic_widgets/main.c`
- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`

---

### Task 1: 增加 app-free runtime API

**Files:**
- Create: `tinyui/include/tinyui/runtime.h`
- Create: `tinyui/src/core/runtime.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/tinyui/unit/test_tinyui_runtime_model.c`
- Modify: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: 写 fail-first runtime model test**

Create `tests/tinyui/unit/test_tinyui_runtime_model.c`:

```c
#include "tinyui/tinyui.h"

#include <assert.h>

static void test_runtime_init_create_load_teardown(void)
{
    struct tinyui_window *screen;

    assert(tinyui_init() == 0);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen) == 0);
    tinyui_deinit();
}

int main(void)
{
    test_runtime_init_create_load_teardown();
    return 0;
}
```

Register in `tests/tinyui/CMakeLists.txt`:

```cmake
    unit/test_tinyui_runtime_model.c
```

- [ ] **Step 2: 运行 build，确认接口不存在**

Run:

```bash
rtk cmake --build build --target test_tinyui_runtime_model
```

Expected: FAIL，`tinyui_init`/`tinyui_screen_create`/`tinyui_screen_load` 不存在。

- [ ] **Step 3: 新增 `runtime.h` 与核心实现**

Create `tinyui/include/tinyui/runtime.h`:

```c
#ifndef PICOUI_RUNTIME_H
#define PICOUI_RUNTIME_H

struct tinyui_window;

int tinyui_init(void);
void tinyui_deinit(void);
struct tinyui_window *tinyui_screen_create(void);
int tinyui_screen_load(struct tinyui_window *screen);
void tinyui_timer_handler(void);

#endif
```

Add to `tinyui/include/tinyui/tinyui.h`:

```c
#include "tinyui/runtime.h"
```

Create `tinyui/src/core/runtime.c`:

```c
#include "internal.h"
#include "tinyui/runtime.h"
#include "tinyui/window.h"

static struct tinyui_app *g_tinyui_runtime_app;

int tinyui_init(void)
{
    if (g_tinyui_runtime_app != 0) {
        return 0;
    }
    g_tinyui_runtime_app = tinyui_app_create();
    return g_tinyui_runtime_app != 0 ? 0 : -1;
}

void tinyui_deinit(void)
{
    if (g_tinyui_runtime_app != 0) {
        tinyui_app_destroy(g_tinyui_runtime_app);
        g_tinyui_runtime_app = 0;
    }
}
```

Add `runtime.c` to `tinyui_core` in `cmake/LingDongGUI.cmake`.

- [ ] **Step 4: 补齐 `screen_create/load` 与 `timer_handler`**

Extend `tinyui/src/core/runtime.c`:

```c
struct tinyui_window *tinyui_screen_create(void)
{
    if (g_tinyui_runtime_app == 0 && tinyui_init() != 0) {
        return 0;
    }
    return tinyui_window_create(g_tinyui_runtime_app, "root");
}

int tinyui_screen_load(struct tinyui_window *screen)
{
    if (g_tinyui_runtime_app == 0 || screen == 0) {
        return -1;
    }
    return tinyui_app_set_window(g_tinyui_runtime_app, screen);
}

void tinyui_timer_handler(void)
{
    if (g_tinyui_runtime_app != 0) {
        tinyui_backend_runtime_step(g_tinyui_runtime_app);
    }
}
```

Also add the private forward declaration in `tinyui/src/core/internal.h`:

```c
void tinyui_backend_runtime_step(struct tinyui_app *app);
```

- [ ] **Step 5: 跑 focused tests**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_runtime_model|test_tinyui_app_lifecycle|test_tinyui_app_timer)$' --output-on-failure
```

Expected: PASS。

### Task 2: 把 `basic_widgets` 启动模型改成 app-free path

**Files:**
- Modify: `tinyui/demo/basic_widgets/main.c`
- Modify: `tinyui/include/tinyui/app.h`
- Modify: `tinyui/src/core/app.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 把 demo 改成新启动模型**

In `tinyui/demo/basic_widgets/main.c`, replace `run_demo()` body with:

```c
static int run_demo(void)
{
    struct tinyui_window *screen;

    if (tinyui_init() != 0) {
        return 1;
    }

    screen = tinyui_screen_create();
    if (screen == 0) {
        tinyui_deinit();
        return 1;
    }

    make_ui(screen);
    if (tinyui_screen_load(screen) != 0) {
        tinyui_deinit();
        return 1;
    }

    while (1) {
        tinyui_timer_handler();
    }
}
```

If an existing runtime helper already provides the loop, call that helper instead of open-coding the `while (1)` loop.

- [ ] **Step 2: 让 `app.h` 退为兼容层文案**

In `tinyui/include/tinyui/app.h`, rewrite top comment to:

```c
/*
 * Compatibility API retained during TinyUI transition.
 * New user-facing startup path should use runtime.h.
 */
```

- [ ] **Step 3: 在 runtime checker 中要求新启动标记**

Update `tests/tinyui/runtime/check_tinyui_runtime.py` to accept log markers such as:

```python
"PICOUI_RUNTIME_READY"
"PICOUI_RUNTIME_LOOP"
```

and make the checker fail if `basic_widgets` still only proves `tinyui_app_run()` path.

- [ ] **Step 4: 跑 runtime/visible gates**

Run:

```bash
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui' --output-on-failure
```

Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add \
  tinyui/include/tinyui/runtime.h \
  tinyui/include/tinyui/tinyui.h \
  tinyui/include/tinyui/app.h \
  tinyui/src/core/internal.h \
  tinyui/src/core/runtime.c \
  tinyui/src/core/app.c \
  tinyui/demo/basic_widgets/main.c \
  cmake/LingDongGUI.cmake \
  tests/tinyui/CMakeLists.txt \
  tests/tinyui/unit/test_tinyui_runtime_model.c \
  tests/tinyui/runtime/check_tinyui_runtime.py
git commit -m "refactor: remove tinyui app from main runtime path"
```
