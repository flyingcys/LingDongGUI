# TinyUI v2.0 P3 App Removal And Runtime Transition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让用户主路径摆脱 `app`，改为 LVGL-like runtime 入口，同时在迁移期保住现有内部实现的兼容性。

**Architecture:** 先在当前 `picoui` public headers 上新增 app-free runtime surface，再把 demo 与 runtime loop 改成 `init/display/indev/screen/timer_handler`，并将 `picoui_app_*` 降级为兼容内部实现而不是主用户模型。

**Tech Stack:** C11、SDL2、当前 `picoui/src/core/app.c`、`picoui/src/tick/tick.c`、demos、unit/runtime tests。

---

## 文件结构

新增：

- `picoui/include/picoui/runtime.h`
- `picoui/src/core/runtime.c`
- `tests/picoui/unit/test_picoui_runtime_model.c`

修改：

- `picoui/include/picoui/picoui.h`
- `picoui/include/picoui/app.h`
- `picoui/src/core/app.c`
- `picoui/src/core/internal.h`
- `picoui/demo/basic_widgets/main.c`
- `cmake/LingDongGUI.cmake`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`

---

### Task 1: 增加 app-free runtime API

**Files:**
- Create: `picoui/include/picoui/runtime.h`
- Create: `picoui/src/core/runtime.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/unit/test_picoui_runtime_model.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 写 fail-first runtime model test**

Create `tests/picoui/unit/test_picoui_runtime_model.c`:

```c
#include "picoui/picoui.h"

#include <assert.h>

static void test_runtime_init_create_load_teardown(void)
{
    struct picoui_window *screen;

    assert(picoui_init() == 0);
    screen = picoui_screen_create();
    assert(screen != NULL);
    assert(picoui_screen_load(screen) == 0);
    picoui_deinit();
}

int main(void)
{
    test_runtime_init_create_load_teardown();
    return 0;
}
```

Register in `tests/picoui/CMakeLists.txt`:

```cmake
    unit/test_picoui_runtime_model.c
```

- [ ] **Step 2: 运行 build，确认接口不存在**

Run:

```bash
rtk cmake --build build --target test_picoui_runtime_model
```

Expected: FAIL，`picoui_init`/`picoui_screen_create`/`picoui_screen_load` 不存在。

- [ ] **Step 3: 新增 `runtime.h` 与核心实现**

Create `picoui/include/picoui/runtime.h`:

```c
#ifndef PICOUI_RUNTIME_H
#define PICOUI_RUNTIME_H

struct picoui_window;

int picoui_init(void);
void picoui_deinit(void);
struct picoui_window *picoui_screen_create(void);
int picoui_screen_load(struct picoui_window *screen);
void picoui_timer_handler(void);

#endif
```

Add to `picoui/include/picoui/picoui.h`:

```c
#include "picoui/runtime.h"
```

Create `picoui/src/core/runtime.c`:

```c
#include "internal.h"
#include "picoui/runtime.h"
#include "picoui/window.h"

static struct picoui_app *g_picoui_runtime_app;

int picoui_init(void)
{
    if (g_picoui_runtime_app != 0) {
        return 0;
    }
    g_picoui_runtime_app = picoui_app_create();
    return g_picoui_runtime_app != 0 ? 0 : -1;
}

void picoui_deinit(void)
{
    if (g_picoui_runtime_app != 0) {
        picoui_app_destroy(g_picoui_runtime_app);
        g_picoui_runtime_app = 0;
    }
}
```

Add `runtime.c` to `picoui_core` in `cmake/LingDongGUI.cmake`.

- [ ] **Step 4: 补齐 `screen_create/load` 与 `timer_handler`**

Extend `picoui/src/core/runtime.c`:

```c
struct picoui_window *picoui_screen_create(void)
{
    if (g_picoui_runtime_app == 0 && picoui_init() != 0) {
        return 0;
    }
    return picoui_window_create(g_picoui_runtime_app, "root");
}

int picoui_screen_load(struct picoui_window *screen)
{
    if (g_picoui_runtime_app == 0 || screen == 0) {
        return -1;
    }
    return picoui_app_set_window(g_picoui_runtime_app, screen);
}

void picoui_timer_handler(void)
{
    if (g_picoui_runtime_app != 0) {
        picoui_backend_runtime_step(g_picoui_runtime_app);
    }
}
```

Also add the private forward declaration in `picoui/src/core/internal.h`:

```c
void picoui_backend_runtime_step(struct picoui_app *app);
```

- [ ] **Step 5: 跑 focused tests**

Run:

```bash
rtk ctest --test-dir build -R '^(test_picoui_runtime_model|test_picoui_app_lifecycle|test_picoui_app_timer)$' --output-on-failure
```

Expected: PASS。

### Task 2: 把 `basic_widgets` 启动模型改成 app-free path

**Files:**
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `picoui/include/picoui/app.h`
- Modify: `picoui/src/core/app.c`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`

- [ ] **Step 1: 把 demo 改成新启动模型**

In `picoui/demo/basic_widgets/main.c`, replace `run_demo()` body with:

```c
static int run_demo(void)
{
    struct picoui_window *screen;

    if (picoui_init() != 0) {
        return 1;
    }

    screen = picoui_screen_create();
    if (screen == 0) {
        picoui_deinit();
        return 1;
    }

    make_ui(screen);
    if (picoui_screen_load(screen) != 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        picoui_timer_handler();
    }
}
```

If an existing runtime helper already provides the loop, call that helper instead of open-coding the `while (1)` loop.

- [ ] **Step 2: 让 `app.h` 退为兼容层文案**

In `picoui/include/picoui/app.h`, rewrite top comment to:

```c
/*
 * Compatibility API retained during TinyUI transition.
 * New user-facing startup path should use runtime.h.
 */
```

- [ ] **Step 3: 在 runtime checker 中要求新启动标记**

Update `tests/picoui/runtime/check_picoui_runtime.py` to accept log markers such as:

```python
"PICOUI_RUNTIME_READY"
"PICOUI_RUNTIME_LOOP"
```

and make the checker fail if `basic_widgets` still only proves `picoui_app_run()` path.

- [ ] **Step 4: 跑 runtime/visible gates**

Run:

```bash
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui' --output-on-failure
```

Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add \
  picoui/include/picoui/runtime.h \
  picoui/include/picoui/picoui.h \
  picoui/include/picoui/app.h \
  picoui/src/core/internal.h \
  picoui/src/core/runtime.c \
  picoui/src/core/app.c \
  picoui/demo/basic_widgets/main.c \
  cmake/LingDongGUI.cmake \
  tests/picoui/CMakeLists.txt \
  tests/picoui/unit/test_picoui_runtime_model.c \
  tests/picoui/runtime/check_picoui_runtime.py
git commit -m "refactor: remove picoui app from main runtime path"
```
