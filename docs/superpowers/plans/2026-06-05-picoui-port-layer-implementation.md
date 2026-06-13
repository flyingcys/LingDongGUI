# TINYUI Port Layer Implementation Plan

> 完成状态（2026-06-05）：P1 已按当前 spec 落地并完成验证。以当前 worktree 证据为准：
>
> - `ctest --test-dir build -L 'tinyui' --output-on-failure` -> `100% tests passed, 0 tests failed out of 46`
> - `ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure` -> `100% tests passed, 0 tests failed out of 3`
>
> 说明：下方 checkbox 保留为实现过程记录，不再代表当前完成态。当前完成态以上述命令与代码目录事实为准。

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first TINYUI `port` layer so SDL adapts through `tinyui/port/sdl/` instead of `examples/sdl`, while LingDongGUI/ARM-2D remain backend-private.

**Architecture:** Add small TINYUI port contracts for display, input, tick, and OS under `tinyui/include/tinyui/{display,indev,tick,osal}.h` and `tinyui/src/{display,indev,tick,osal}/`. Add `tinyui/port/sdl/` as the SDL host port. Refactor `tinyui/src/backend/ldgui/backend_app.c` to consume TINYUI port state instead of owning SDL dimensions, input, tick, and delay directly.

**Tech Stack:** C11, CMake, SDL2, TINYUI core/backend, existing `rtk cmake` and `rtk ctest` commands.

---

## 文件结构

新增：

- `tinyui/include/tinyui/port.h`：聚合 display/input/tick/os port header。
- `tinyui/include/tinyui/display.h`：display config、area、color format、flush callback。
- `tinyui/include/tinyui/indev.h`：pointer/key input push 与 readback。
- `tinyui/include/tinyui/tick.h`：tick source 注册与读取。
- `tinyui/include/tinyui/osal.h`：lock/delay callback 注册与调用。
- `tinyui/src/display/display.c`：display state 存取。
- `tinyui/src/indev/indev.c`：input state 存取。
- `tinyui/src/tick/tick.c`：tick source 存取。
- `tinyui/src/osal/osal.c`：OS callback 存取。
- `tinyui/port/sdl/sdl.c`：SDL port attach。
- `tinyui/include/tinyui/port/sdl.h`：SDL port public attach header。
- `tests/tinyui/unit/test_tinyui_port_display.c`：display contract test。
- `tests/tinyui/unit/test_tinyui_port_input.c`：input contract test。
- `tests/tinyui/unit/test_tinyui_port_tick_os.c`：tick/OS contract test。

修改：

- `tinyui/include/tinyui/tinyui.h`：include `tinyui/port.h`。
- `tinyui/src/core/internal.h`：给 `struct tinyui_app` 增加 port state。
- `tinyui/src/backend/ldgui/backend_app.c`：消费 display/input/tick/os port 状态。
- `cmake/LingDongGUI.cmake`：把 `tinyui/src/display/*.c`、`tinyui/src/indev/*.c`、`tinyui/src/tick/*.c`、`tinyui/src/osal/*.c` 加入 `tinyui_core`，把 `tinyui/port/sdl/*.c` 加入 runtime backend target。
- `tests/tinyui/CMakeLists.txt`：注册 3 个 port unit test。
- `docs/tinyui-serial/a-0.14/2026-06-05-tinyui-port-boundary-audit.md`：实现完成后更新 P1 状态。
- `docs/tinyui-serial/a-0.14-线计划索引.md`：实现完成后更新下一步状态。

---

### Task 1: Display Port Contract

**Files:**
- Create: `tinyui/include/tinyui/display.h`
- Create: `tinyui/include/tinyui/port.h`
- Create: `tinyui/src/display/display.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/tinyui/unit/test_tinyui_port_display.c`
- Modify: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: Write failing display test**

Create `tests/tinyui/unit/test_tinyui_port_display.c`:

```c
#include "tinyui/tinyui.h"

#include <assert.h>

static void test_default_display_config(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {0};

    assert(app != NULL);
    assert(tinyui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(config.buffer_height == 0);
    assert(config.user_data == 0);

    tinyui_app_destroy(app);
}

static void test_display_config_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {
        .width = 320,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_ARGB8888,
        .buffer_height = 32,
        .user_data = (void *)0x1234,
    };
    struct tinyui_display_config readback = {0};

    assert(app != NULL);
    assert(tinyui_display_set_config(app, &config) == 0);
    assert(tinyui_display_get_config(app, &readback) == 0);
    assert(readback.width == 320);
    assert(readback.height == 240);
    assert(readback.color_format == PICOUI_COLOR_FORMAT_ARGB8888);
    assert(readback.buffer_height == 32);
    assert(readback.user_data == (void *)0x1234);

    tinyui_app_destroy(app);
}

static void test_display_rejects_invalid_config(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {
        .width = 0,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };

    assert(app != NULL);
    assert(tinyui_display_set_config(app, NULL) == -1);
    assert(tinyui_display_get_config(NULL, &config) == -1);
    assert(tinyui_display_get_config(app, NULL) == -1);
    assert(tinyui_display_set_config(app, &config) == -1);

    config.width = 320;
    config.height = -1;
    assert(tinyui_display_set_config(app, &config) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_default_display_config();
    test_display_config_round_trip();
    test_display_rejects_invalid_config();
    return 0;
}
```

Register it in `tests/tinyui/CMakeLists.txt` inside `PICOUI_UNIT_TESTS`:

```cmake
    unit/test_tinyui_port_display.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_display
```

Expected: compile fails because `tinyui_display_config` and `tinyui_display_*` do not exist.

- [ ] **Step 3: Add display public headers**

Create `tinyui/include/tinyui/display.h`:

```c
#ifndef PICOUI_PORT_DISPLAY_H
#define PICOUI_PORT_DISPLAY_H

struct tinyui_app;

enum tinyui_color_format {
    PICOUI_COLOR_FORMAT_RGB565 = 0,
    PICOUI_COLOR_FORMAT_ARGB8888,
};

struct tinyui_area {
    int x;
    int y;
    int width;
    int height;
};

struct tinyui_display_config {
    int width;
    int height;
    enum tinyui_color_format color_format;
    int buffer_height;
    void *user_data;
};

typedef void (*tinyui_display_flush_cb_t)(const struct tinyui_area *area,
                                          const void *pixels,
                                          void *user_data);

int tinyui_display_set_config(struct tinyui_app *app,
                              const struct tinyui_display_config *config);
int tinyui_display_get_config(const struct tinyui_app *app,
                              struct tinyui_display_config *out_config);
int tinyui_display_set_flush_callback(struct tinyui_app *app,
                                      tinyui_display_flush_cb_t callback,
                                      void *user_data);

#endif
```

Create `tinyui/include/tinyui/port.h`:

```c
#ifndef PICOUI_PORT_H
#define PICOUI_PORT_H

#include "tinyui/display.h"

#endif
```

Add to `tinyui/include/tinyui/tinyui.h`:

```c
#include "tinyui/port.h"
```

- [ ] **Step 4: Add display state to internal app**

In `tinyui/src/core/internal.h`, add after `struct tinyui_app_timer`:

```c
struct tinyui_display_port_state {
    struct tinyui_display_config config;
    tinyui_display_flush_cb_t flush_callback;
    void *flush_user_data;
};
```

Add to `struct tinyui_app`:

```c
    struct tinyui_display_port_state display_port;
```

Also include the display header near other public headers:

```c
#include "tinyui/display.h"
```

- [ ] **Step 5: Implement display state**

Create `tinyui/src/display/display.c`:

```c
#include "internal.h"
#include "tinyui/display.h"

static const struct tinyui_display_config g_tinyui_default_display_config = {
    .width = 480,
    .height = 320,
    .color_format = PICOUI_COLOR_FORMAT_RGB565,
    .buffer_height = 0,
    .user_data = 0,
};

static int tinyui_display_config_is_valid(const struct tinyui_display_config *config)
{
    if (config == 0) {
        return 0;
    }
    if (config->width <= 0 || config->height <= 0 || config->buffer_height < 0) {
        return 0;
    }
    return config->color_format == PICOUI_COLOR_FORMAT_RGB565 ||
           config->color_format == PICOUI_COLOR_FORMAT_ARGB8888;
}

static void tinyui_display_ensure_default(struct tinyui_app *app)
{
    if (app == 0) {
        return;
    }
    if (app->display_port.config.width <= 0 || app->display_port.config.height <= 0) {
        app->display_port.config = g_tinyui_default_display_config;
    }
}

int tinyui_display_set_config(struct tinyui_app *app,
                              const struct tinyui_display_config *config)
{
    if (app == 0 || !tinyui_display_config_is_valid(config)) {
        return -1;
    }
    app->display_port.config = *config;
    return 0;
}

int tinyui_display_get_config(const struct tinyui_app *app,
                              struct tinyui_display_config *out_config)
{
    struct tinyui_app *mutable_app = (struct tinyui_app *)app;

    if (app == 0 || out_config == 0) {
        return -1;
    }
    tinyui_display_ensure_default(mutable_app);
    *out_config = mutable_app->display_port.config;
    return 0;
}

int tinyui_display_set_flush_callback(struct tinyui_app *app,
                                      tinyui_display_flush_cb_t callback,
                                      void *user_data)
{
    if (app == 0) {
        return -1;
    }
    app->display_port.flush_callback = callback;
    app->display_port.flush_user_data = user_data;
    return 0;
}
```

- [ ] **Step 6: Add source to CMake**

In `cmake/LingDongGUI.cmake`, add to `tinyui_core` sources:

```cmake
        ${LD_REPO_ROOT}/tinyui/src/display/display.c
```

- [ ] **Step 7: Verify display test passes**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_display
rtk ctest --test-dir build -R '^test_tinyui_port_display$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 2: Input Port Contract

**Files:**
- Create: `tinyui/include/tinyui/indev.h`
- Modify: `tinyui/include/tinyui/port.h`
- Create: `tinyui/src/indev/indev.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/tinyui/unit/test_tinyui_port_input.c`
- Modify: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: Write failing input test**

Create `tests/tinyui/unit/test_tinyui_port_input.c`:

```c
#include "tinyui/tinyui.h"

#include <assert.h>

static void test_pointer_input_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    int x = -1;
    int y = -1;
    int pressed = -1;

    assert(app != NULL);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    assert(tinyui_input_push_pointer(app, 17, 23, 1) == 0);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 17);
    assert(y == 23);
    assert(pressed == 1);

    assert(tinyui_input_push_pointer(app, -4, -5, 0) == 0);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    tinyui_app_destroy(app);
}

static void test_key_input_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    enum tinyui_input_key key = PICOUI_INPUT_KEY_NONE;
    int pressed = -1;

    assert(app != NULL);
    assert(tinyui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_NONE);
    assert(pressed == 0);

    assert(tinyui_input_push_key(app, PICOUI_INPUT_KEY_ENTER, 1) == 0);
    assert(tinyui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_ENTER);
    assert(pressed == 1);

    tinyui_app_destroy(app);
}

static void test_input_rejects_invalid_arguments(void)
{
    struct tinyui_app *app = tinyui_app_create();
    int x;
    int y;
    int pressed;

    assert(app != NULL);
    assert(tinyui_input_push_pointer(NULL, 1, 2, 1) == -1);
    assert(tinyui_input_get_pointer(NULL, &x, &y, &pressed) == -1);
    assert(tinyui_input_get_pointer(app, NULL, &y, &pressed) == -1);
    assert(tinyui_input_get_pointer(app, &x, NULL, &pressed) == -1);
    assert(tinyui_input_get_pointer(app, &x, &y, NULL) == -1);
    assert(tinyui_input_push_key(NULL, PICOUI_INPUT_KEY_ENTER, 1) == -1);
    assert(tinyui_input_push_key(app, (enum tinyui_input_key)999, 1) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_pointer_input_round_trip();
    test_key_input_round_trip();
    test_input_rejects_invalid_arguments();
    return 0;
}
```

Register it in `tests/tinyui/CMakeLists.txt`:

```cmake
    unit/test_tinyui_port_input.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_input
```

Expected: compile fails because `tinyui_input_*` does not exist.

- [ ] **Step 3: Add input header**

Create `tinyui/include/tinyui/indev.h`:

```c
#ifndef PICOUI_PORT_INPUT_H
#define PICOUI_PORT_INPUT_H

struct tinyui_app;

enum tinyui_input_key {
    PICOUI_INPUT_KEY_NONE = 0,
    PICOUI_INPUT_KEY_LEFT,
    PICOUI_INPUT_KEY_RIGHT,
    PICOUI_INPUT_KEY_UP,
    PICOUI_INPUT_KEY_DOWN,
    PICOUI_INPUT_KEY_ENTER,
    PICOUI_INPUT_KEY_BACK,
};

int tinyui_input_push_pointer(struct tinyui_app *app, int x, int y, int pressed);
int tinyui_input_get_pointer(const struct tinyui_app *app,
                             int *x,
                             int *y,
                             int *pressed);
int tinyui_input_push_key(struct tinyui_app *app,
                          enum tinyui_input_key key,
                          int pressed);
int tinyui_input_get_key(const struct tinyui_app *app,
                         enum tinyui_input_key *key,
                         int *pressed);

#endif
```

Update `tinyui/include/tinyui/port.h`:

```c
#include "tinyui/indev.h"
```

- [ ] **Step 4: Add input state to app**

In `tinyui/src/core/internal.h`, include:

```c
#include "tinyui/indev.h"
```

Add:

```c
struct tinyui_input_port_state {
    int pointer_x;
    int pointer_y;
    int pointer_pressed;
    enum tinyui_input_key key;
    int key_pressed;
};
```

Add to `struct tinyui_app`:

```c
    struct tinyui_input_port_state input_port;
```

- [ ] **Step 5: Implement input state**

Create `tinyui/src/indev/indev.c`:

```c
#include "internal.h"
#include "tinyui/indev.h"

static int tinyui_input_key_is_valid(enum tinyui_input_key key)
{
    return key >= PICOUI_INPUT_KEY_NONE && key <= PICOUI_INPUT_KEY_BACK;
}

int tinyui_input_push_pointer(struct tinyui_app *app, int x, int y, int pressed)
{
    if (app == 0) {
        return -1;
    }
    app->input_port.pointer_x = x < 0 ? 0 : x;
    app->input_port.pointer_y = y < 0 ? 0 : y;
    app->input_port.pointer_pressed = pressed ? 1 : 0;
    return 0;
}

int tinyui_input_get_pointer(const struct tinyui_app *app,
                             int *x,
                             int *y,
                             int *pressed)
{
    if (app == 0 || x == 0 || y == 0 || pressed == 0) {
        return -1;
    }
    *x = app->input_port.pointer_x;
    *y = app->input_port.pointer_y;
    *pressed = app->input_port.pointer_pressed;
    return 0;
}

int tinyui_input_push_key(struct tinyui_app *app,
                          enum tinyui_input_key key,
                          int pressed)
{
    if (app == 0 || !tinyui_input_key_is_valid(key)) {
        return -1;
    }
    app->input_port.key = key;
    app->input_port.key_pressed = pressed ? 1 : 0;
    return 0;
}

int tinyui_input_get_key(const struct tinyui_app *app,
                         enum tinyui_input_key *key,
                         int *pressed)
{
    if (app == 0 || key == 0 || pressed == 0) {
        return -1;
    }
    *key = app->input_port.key;
    *pressed = app->input_port.key_pressed;
    return 0;
}
```

- [ ] **Step 6: Add source to CMake**

In `cmake/LingDongGUI.cmake`, add to `tinyui_core` sources:

```cmake
        ${LD_REPO_ROOT}/tinyui/src/indev/indev.c
```

- [ ] **Step 7: Verify input test passes**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_input
rtk ctest --test-dir build -R '^test_tinyui_port_input$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 3: Tick And OS Port Contracts

**Files:**
- Create: `tinyui/include/tinyui/tick.h`
- Create: `tinyui/include/tinyui/osal.h`
- Modify: `tinyui/include/tinyui/port.h`
- Create: `tinyui/src/tick/tick.c`
- Create: `tinyui/src/osal/osal.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/tinyui/unit/test_tinyui_port_tick_os.c`
- Modify: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: Write failing tick/OS test**

Create `tests/tinyui/unit/test_tinyui_port_tick_os.c`:

```c
#include "tinyui/tinyui.h"

#include <assert.h>

struct port_probe {
    unsigned int tick;
    unsigned int delay_ms;
    int enter_count;
    int leave_count;
};

static unsigned int probe_tick(void *user_data)
{
    struct port_probe *probe = (struct port_probe *)user_data;
    return probe->tick;
}

static void probe_enter(void *user_data)
{
    struct port_probe *probe = (struct port_probe *)user_data;
    probe->enter_count += 1;
}

static void probe_leave(void *user_data)
{
    struct port_probe *probe = (struct port_probe *)user_data;
    probe->leave_count += 1;
}

static void probe_delay(unsigned int ms, void *user_data)
{
    struct port_probe *probe = (struct port_probe *)user_data;
    probe->delay_ms = ms;
}

static void test_tick_source_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct port_probe probe = {.tick = 1234};

    assert(app != NULL);
    assert(tinyui_tick_set_source(app, probe_tick, &probe) == 0);
    assert(tinyui_tick_get(app) == 1234);
    probe.tick = 5678;
    assert(tinyui_tick_get(app) == 5678);

    tinyui_app_destroy(app);
}

static void test_os_callbacks_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct port_probe probe = {0};

    assert(app != NULL);
    assert(tinyui_os_set_lock_callbacks(app, probe_enter, probe_leave, &probe) == 0);
    tinyui_os_enter(app);
    tinyui_os_leave(app);
    assert(probe.enter_count == 1);
    assert(probe.leave_count == 1);

    assert(tinyui_os_set_delay_callback(app, probe_delay, &probe) == 0);
    tinyui_os_delay(app, 42);
    assert(probe.delay_ms == 42);

    tinyui_app_destroy(app);
}

static void test_tick_os_rejects_invalid_arguments(void)
{
    struct tinyui_app *app = tinyui_app_create();

    assert(app != NULL);
    assert(tinyui_tick_set_source(NULL, probe_tick, NULL) == -1);
    assert(tinyui_tick_set_source(app, NULL, NULL) == -1);
    assert(tinyui_os_set_lock_callbacks(NULL, NULL, NULL, NULL) == -1);
    assert(tinyui_os_set_delay_callback(NULL, NULL, NULL) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_tick_source_round_trip();
    test_os_callbacks_round_trip();
    test_tick_os_rejects_invalid_arguments();
    return 0;
}
```

Register it in `tests/tinyui/CMakeLists.txt`:

```cmake
    unit/test_tinyui_port_tick_os.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_tick_os
```

Expected: compile fails because `tinyui_tick_*` and `tinyui_os_*` do not exist.

- [ ] **Step 3: Add tick and OS headers**

Create `tinyui/include/tinyui/tick.h`:

```c
#ifndef PICOUI_PORT_TICK_H
#define PICOUI_PORT_TICK_H

struct tinyui_app;

typedef unsigned int (*tinyui_tick_get_cb_t)(void *user_data);

int tinyui_tick_set_source(struct tinyui_app *app,
                           tinyui_tick_get_cb_t callback,
                           void *user_data);
unsigned int tinyui_tick_get(struct tinyui_app *app);

#endif
```

Create `tinyui/include/tinyui/osal.h`:

```c
#ifndef PICOUI_PORT_OS_H
#define PICOUI_PORT_OS_H

struct tinyui_app;

typedef void (*tinyui_os_lock_cb_t)(void *user_data);
typedef void (*tinyui_os_delay_cb_t)(unsigned int ms, void *user_data);

int tinyui_os_set_lock_callbacks(struct tinyui_app *app,
                                 tinyui_os_lock_cb_t enter,
                                 tinyui_os_lock_cb_t leave,
                                 void *user_data);
void tinyui_os_enter(struct tinyui_app *app);
void tinyui_os_leave(struct tinyui_app *app);
int tinyui_os_set_delay_callback(struct tinyui_app *app,
                                 tinyui_os_delay_cb_t delay,
                                 void *user_data);
void tinyui_os_delay(struct tinyui_app *app, unsigned int ms);

#endif
```

Update `tinyui/include/tinyui/port.h`:

```c
#include "tinyui/tick.h"
#include "tinyui/osal.h"
```

- [ ] **Step 4: Add tick/OS state to app**

In `tinyui/src/core/internal.h`, include:

```c
#include "tinyui/tick.h"
#include "tinyui/osal.h"
```

Add:

```c
struct tinyui_tick_port_state {
    tinyui_tick_get_cb_t callback;
    void *user_data;
};

struct tinyui_os_port_state {
    tinyui_os_lock_cb_t enter;
    tinyui_os_lock_cb_t leave;
    void *lock_user_data;
    tinyui_os_delay_cb_t delay;
    void *delay_user_data;
};
```

Add to `struct tinyui_app`:

```c
    struct tinyui_tick_port_state tick_port;
    struct tinyui_os_port_state os_port;
```

- [ ] **Step 5: Implement tick and OS state**

Create `tinyui/src/tick/tick.c`:

```c
#include "internal.h"
#include "tinyui/tick.h"

int tinyui_tick_set_source(struct tinyui_app *app,
                           tinyui_tick_get_cb_t callback,
                           void *user_data)
{
    if (app == 0 || callback == 0) {
        return -1;
    }
    app->tick_port.callback = callback;
    app->tick_port.user_data = user_data;
    return 0;
}

unsigned int tinyui_tick_get(struct tinyui_app *app)
{
    if (app == 0 || app->tick_port.callback == 0) {
        return 0;
    }
    return app->tick_port.callback(app->tick_port.user_data);
}
```

Create `tinyui/src/osal/osal.c`:

```c
#include "internal.h"
#include "tinyui/osal.h"

int tinyui_os_set_lock_callbacks(struct tinyui_app *app,
                                 tinyui_os_lock_cb_t enter,
                                 tinyui_os_lock_cb_t leave,
                                 void *user_data)
{
    if (app == 0) {
        return -1;
    }
    app->os_port.enter = enter;
    app->os_port.leave = leave;
    app->os_port.lock_user_data = user_data;
    return 0;
}

void tinyui_os_enter(struct tinyui_app *app)
{
    if (app != 0 && app->os_port.enter != 0) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void tinyui_os_leave(struct tinyui_app *app)
{
    if (app != 0 && app->os_port.leave != 0) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

int tinyui_os_set_delay_callback(struct tinyui_app *app,
                                 tinyui_os_delay_cb_t delay,
                                 void *user_data)
{
    if (app == 0) {
        return -1;
    }
    app->os_port.delay = delay;
    app->os_port.delay_user_data = user_data;
    return 0;
}

void tinyui_os_delay(struct tinyui_app *app, unsigned int ms)
{
    if (app != 0 && app->os_port.delay != 0) {
        app->os_port.delay(ms, app->os_port.delay_user_data);
    }
}
```

- [ ] **Step 6: Add sources to CMake**

In `cmake/LingDongGUI.cmake`, add to `tinyui_core` sources:

```cmake
        ${LD_REPO_ROOT}/tinyui/src/tick/tick.c
        ${LD_REPO_ROOT}/tinyui/src/osal/osal.c
```

- [ ] **Step 7: Verify tick/OS test passes**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_tick_os
rtk ctest --test-dir build -R '^test_tinyui_port_tick_os$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 4: SDL Port Attach

**Files:**
- Create: `tinyui/include/tinyui/port/sdl.h`
- Create: `tinyui/port/sdl/sdl.c`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/tinyui/unit/test_tinyui_port_sdl.c`
- Modify: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: Write failing SDL attach test**

Create `tests/tinyui/unit/test_tinyui_port_sdl.c`:

```c
#include "tinyui/tinyui.h"
#include "tinyui/port/sdl.h"

#include <assert.h>

static void test_sdl_attach_sets_port_defaults(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {0};

    assert(app != NULL);
    assert(tinyui_port_sdl_attach(app) == 0);
    assert(tinyui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(tinyui_tick_get(app) >= 0);
    tinyui_os_delay(app, 0);

    tinyui_app_destroy(app);
}

static void test_sdl_attach_rejects_null_app(void)
{
    assert(tinyui_port_sdl_attach(NULL) == -1);
}

int main(void)
{
    test_sdl_attach_sets_port_defaults();
    test_sdl_attach_rejects_null_app();
    return 0;
}
```

Register it in `tests/tinyui/CMakeLists.txt`:

```cmake
    unit/test_tinyui_port_sdl.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_sdl
```

Expected: compile fails because `tinyui/port/sdl.h` and `tinyui_port_sdl_attach()` do not exist.

- [ ] **Step 3: Add SDL port header**

Create `tinyui/include/tinyui/port/sdl.h`:

```c
#ifndef PICOUI_PORT_SDL_H
#define PICOUI_PORT_SDL_H

struct tinyui_app;

int tinyui_port_sdl_attach(struct tinyui_app *app);

#endif
```

- [ ] **Step 4: Add SDL port implementation**

Create `tinyui/port/sdl/sdl.c`:

```c
#include "tinyui/app.h"
#include "tinyui/port.h"
#include "tinyui/port/sdl.h"

#include <SDL.h>

static unsigned int tinyui_port_sdl_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

static void tinyui_port_sdl_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay(ms);
}

int tinyui_port_sdl_attach(struct tinyui_app *app)
{
    struct tinyui_display_config config = {
        .width = 480,
        .height = 320,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };

    if (app == 0) {
        return -1;
    }
    if (tinyui_display_set_config(app, &config) != 0) {
        return -1;
    }
    if (tinyui_tick_set_source(app, tinyui_port_sdl_tick_get, 0) != 0) {
        return -1;
    }
    if (tinyui_os_set_delay_callback(app, tinyui_port_sdl_delay, 0) != 0) {
        return -1;
    }
    return 0;
}
```

- [ ] **Step 5: Add SDL port to CMake**

In `cmake/LingDongGUI.cmake`, add the SDL port source and include directory to both backend targets inside the existing `foreach(LD_PICOUI_BACKEND_TARGET IN ITEMS tinyui_backend_ldgui tinyui_backend_ldgui_runtime)` loop. Both targets already link SDL in the current CMake file, so this keeps unit tests and runtime builds consistent:

```cmake
        target_sources(${LD_PICOUI_BACKEND_TARGET} PRIVATE
            ${LD_REPO_ROOT}/tinyui/port/sdl/sdl.c
        )
        target_include_directories(${LD_PICOUI_BACKEND_TARGET} PUBLIC
            ${LD_REPO_ROOT}/tinyui/port/sdl
        )
```

Do not add any new TINYUI port files under `examples/sdl`.

- [ ] **Step 6: Verify SDL attach test passes**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_sdl
rtk ctest --test-dir build -R '^test_tinyui_port_sdl$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 5: Backend App Consumes Port State

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Test: `tests/tinyui/unit/test_tinyui_port_display.c`
- Test: `tests/tinyui/unit/test_tinyui_port_input.c`
- Test: `tests/tinyui/unit/test_tinyui_port_tick_os.c`

- [ ] **Step 1: Add backend assertions to focused tests**

Extend `test_tinyui_port_display.c` with a regression assertion that changing display config remains visible before runtime:

```c
static void test_display_config_can_drive_runtime_dimensions(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {
        .width = 300,
        .height = 200,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 20,
        .user_data = 0,
    };
    struct tinyui_display_config readback = {0};

    assert(app != NULL);
    assert(tinyui_display_set_config(app, &config) == 0);
    assert(tinyui_display_get_config(app, &readback) == 0);
    assert(readback.width == 300);
    assert(readback.height == 200);

    tinyui_app_destroy(app);
}
```

Call it from `main()`.

- [ ] **Step 2: Replace hardcoded runtime size reads**

In `backend_app.c`, add a helper:

```c
static struct tinyui_display_config tinyui_backend_display_config_from_app(struct tinyui_app *app)
{
    struct tinyui_display_config config;

    if (tinyui_display_get_config(app, &config) != 0) {
        config.width = PICOUI_RUNTIME_WIDTH;
        config.height = PICOUI_RUNTIME_HEIGHT;
        config.color_format = PICOUI_COLOR_FORMAT_RGB565;
        config.buffer_height = 0;
        config.user_data = NULL;
    }
    return config;
}
```

Use the helper in `tinyui_backend_ensure_window()`, tile allocation, texture creation, present loop, capture writing, and render clearing. Keep `PICOUI_RUNTIME_WIDTH/HEIGHT` as fallback constants only.

- [ ] **Step 3: Route SDL input through TINYUI input port**

In `tinyui_backend_commit_pointer_event()`, replace direct state write with:

```c
    (void)tinyui_input_push_pointer(state_app, mapped_x, mapped_y, pressed != 0);
```

If current function lacks `app`, change signature to:

```c
static void tinyui_backend_commit_pointer_event(struct tinyui_app *app,
                                                struct tinyui_backend_runtime_state *state,
                                                int x,
                                                int y,
                                                int pressed)
```

Then after successful push, keep backend-private bridge:

```c
    ldCfgTouchSetPoint(mapped_x, mapped_y, pressed != 0);
```

This is an interim bridge: SDL talks to TINYUI input first; LingDongGUI remains backend-private.

- [ ] **Step 4: Route tick and delay through TINYUI port**

Replace timer pump call:

```c
tinyui_backend_pump_timers(app, tinyui_tick_get(app));
```

If `tinyui_tick_get(app)` returns `0` before SDL attach is wired, fallback to `SDL_GetTicks()` until Task 6 attaches SDL by default.

Replace:

```c
SDL_Delay(16);
```

with:

```c
tinyui_os_delay(app, 16);
```

Use SDL fallback only when no delay callback is installed.

- [ ] **Step 5: Verify focused tests**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os
rtk ctest --test-dir build -R 'test_tinyui_port_display|test_tinyui_port_input|test_tinyui_port_tick_os' --output-on-failure
```

Expected: all selected tests pass.

---

### Task 6: Runtime Default SDL Attach And Public API Gate

**Files:**
- Modify: `tinyui/src/core/app.c`
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Modify: `cmake/LingDongGUI.cmake`
- Test: `tests/tinyui/contract/check_tinyui_public_api.py`

- [ ] **Step 1: Attach SDL port for runtime target**

In `tinyui/src/core/app.c`, do not include SDL headers. Instead, keep core backend-neutral. In `backend_app.c`, include:

```c
#include "tinyui/port/sdl.h"
```

In `tinyui_backend_app_init()`, after `app_state->runtime_state = state;`, attach SDL only when the runtime target enables the compile definition:

```c
#if defined(PICOUI_ENABLE_SDL_PORT)
    (void)tinyui_port_sdl_attach(app);
#endif
```

- [ ] **Step 2: Update CMake compile definitions**

In `cmake/LingDongGUI.cmake`, for `tinyui_backend_ldgui_runtime`, add:

```cmake
            target_compile_definitions(${LD_PICOUI_BACKEND_TARGET} PRIVATE PICOUI_ENABLE_SDL_PORT=1)
```

Do not add new TINYUI port files under `examples/sdl`.

- [ ] **Step 3: Run public API leak check**

Run:

```bash
rtk ctest --test-dir build -R '^check_tinyui_public_api$' --output-on-failure
```

Expected: PASS. If it fails because `tinyui/include/tinyui/{display,indev,tick,osal}.h.h` exposes `ld*`, `arm_2d_*`, or `SIGNAL_*`, remove those identifiers from public headers.

- [ ] **Step 4: Run focused port tests**

Run:

```bash
rtk cmake --build build --target test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os test_tinyui_port_sdl
rtk ctest --test-dir build -R 'test_tinyui_port_display|test_tinyui_port_input|test_tinyui_port_tick_os|test_tinyui_port_sdl|check_tinyui_public_api' --output-on-failure
```

Expected: all selected tests pass.

---

### Task 7: Documentation Sync

**Files:**
- Modify: `docs/tinyui-serial/a-0.14/2026-06-05-tinyui-port-boundary-audit.md`
- Modify: `docs/tinyui-serial/a-0.14-线计划索引.md`

- [ ] **Step 1: Update audit document P1 status**

In `docs/tinyui-serial/a-0.14/2026-06-05-tinyui-port-boundary-audit.md`, under `P1：SDL port 先行`, add a completion note with exact evidence:

```markdown
> P1 实现状态：已完成/部分完成。证据以本轮实际命令为准：
>
> - `rtk ctest --test-dir build -R 'test_tinyui_port_display|test_tinyui_port_input|test_tinyui_port_tick_os|test_tinyui_port_sdl|check_tinyui_public_api' --output-on-failure`
```

Use `已完成` only if the command actually passes.

- [ ] **Step 2: Update a-0.14 index**

In `docs/tinyui-serial/a-0.14-线计划索引.md`, update the `当前进度` section. Use this wording only after tests pass:

```markdown
  - TINYUI port P1：display/input/tick/OS 合同与 SDL port 最小闭环
```

If tests do not pass, write:

```markdown
  - TINYUI port P1：部分完成，剩余失败见最新测试输出
```

- [ ] **Step 3: Verify docs contain no stale recommendation**

Run:

```bash
rtk rg -n 'platform/ldgui|port/ldgui|platform/sdl|examples/sdl.*(新增|承载|推荐|port)' docs/tinyui-serial/a-0.14 docs/superpowers/specs/2026-06-05-tinyui-port-layer-design.md docs/superpowers/plans/2026-06-05-tinyui-port-layer-implementation.md
```

Expected: no matches except explicit negative statements saying not to use those paths.

---

## Final Verification

Run:

```bash
rtk cmake --build build --target test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os test_tinyui_port_sdl
rtk ctest --test-dir build -R 'test_tinyui_port_display|test_tinyui_port_input|test_tinyui_port_tick_os|test_tinyui_port_sdl|check_tinyui_public_api' --output-on-failure
rtk git diff --stat
```

Expected:

- All selected tests pass.
- New files live under `tinyui/include/tinyui/port*`, `tinyui/src/display|indev|tick|osal/`, and `tinyui/port/sdl/`.
- No new TINYUI SDL port implementation lives under `examples/sdl`.
- Public headers do not expose `ld*`, `arm_2d_*`, or `SIGNAL_*`.
