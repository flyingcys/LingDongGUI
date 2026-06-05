# PicoUI Port Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first PicoUI `port` layer so SDL adapts through `picoui/port/sdl/` instead of `examples/sdl`, while LingDongGUI/ARM-2D remain backend-private.

**Architecture:** Add small PicoUI port contracts for display, input, tick, and OS under `picoui/include/picoui/port*.h` and `picoui/src/port/`. Add `picoui/port/sdl/` as the SDL host port. Refactor `picoui/src/backend/ldgui/backend_app.c` to consume PicoUI port state instead of owning SDL dimensions, input, tick, and delay directly.

**Tech Stack:** C11, CMake, SDL2, PicoUI core/backend, existing `rtk cmake` and `rtk ctest` commands.

---

## 文件结构

新增：

- `picoui/include/picoui/port.h`：聚合 display/input/tick/os port header。
- `picoui/include/picoui/port/display.h`：display config、area、color format、flush callback。
- `picoui/include/picoui/port/input.h`：pointer/key input push 与 readback。
- `picoui/include/picoui/port/tick.h`：tick source 注册与读取。
- `picoui/include/picoui/port/os.h`：lock/delay callback 注册与调用。
- `picoui/src/port/display.c`：display state 存取。
- `picoui/src/port/input.c`：input state 存取。
- `picoui/src/port/tick.c`：tick source 存取。
- `picoui/src/port/os.c`：OS callback 存取。
- `picoui/port/sdl/picoui_port_sdl.c`：SDL port attach。
- `picoui/port/sdl/picoui_port_sdl.h`：SDL port public attach header。
- `tests/picoui/unit/test_picoui_port_display.c`：display contract test。
- `tests/picoui/unit/test_picoui_port_input.c`：input contract test。
- `tests/picoui/unit/test_picoui_port_tick_os.c`：tick/OS contract test。

修改：

- `picoui/include/picoui/picoui.h`：include `picoui/port.h`。
- `picoui/src/core/internal.h`：给 `struct picoui_app` 增加 port state。
- `picoui/src/backend/ldgui/backend_app.c`：消费 display/input/tick/os port 状态。
- `cmake/LingDongGUI.cmake`：把 `picoui/src/port/*.c` 加入 `picoui_core`，把 `picoui/port/sdl/*.c` 加入 runtime backend target。
- `tests/picoui/CMakeLists.txt`：注册 3 个 port unit test。
- `docs/picoui-serial/a-0.14/2026-06-05-picoui-port-boundary-audit.md`：实现完成后更新 P1 状态。
- `docs/picoui-serial/a-0.14-线计划索引.md`：实现完成后更新下一步状态。

---

### Task 1: Display Port Contract

**Files:**
- Create: `picoui/include/picoui/port/display.h`
- Create: `picoui/include/picoui/port.h`
- Create: `picoui/src/port/display.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/unit/test_picoui_port_display.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: Write failing display test**

Create `tests/picoui/unit/test_picoui_port_display.c`:

```c
#include "picoui/picoui.h"

#include <assert.h>

static void test_default_display_config(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {0};

    assert(app != NULL);
    assert(picoui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(config.buffer_height == 0);
    assert(config.user_data == 0);

    picoui_app_destroy(app);
}

static void test_display_config_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {
        .width = 320,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_ARGB8888,
        .buffer_height = 32,
        .user_data = (void *)0x1234,
    };
    struct picoui_display_config readback = {0};

    assert(app != NULL);
    assert(picoui_display_set_config(app, &config) == 0);
    assert(picoui_display_get_config(app, &readback) == 0);
    assert(readback.width == 320);
    assert(readback.height == 240);
    assert(readback.color_format == PICOUI_COLOR_FORMAT_ARGB8888);
    assert(readback.buffer_height == 32);
    assert(readback.user_data == (void *)0x1234);

    picoui_app_destroy(app);
}

static void test_display_rejects_invalid_config(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {
        .width = 0,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };

    assert(app != NULL);
    assert(picoui_display_set_config(app, NULL) == -1);
    assert(picoui_display_get_config(NULL, &config) == -1);
    assert(picoui_display_get_config(app, NULL) == -1);
    assert(picoui_display_set_config(app, &config) == -1);

    config.width = 320;
    config.height = -1;
    assert(picoui_display_set_config(app, &config) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_default_display_config();
    test_display_config_round_trip();
    test_display_rejects_invalid_config();
    return 0;
}
```

Register it in `tests/picoui/CMakeLists.txt` inside `PICOUI_UNIT_TESTS`:

```cmake
    unit/test_picoui_port_display.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_picoui_port_display
```

Expected: compile fails because `picoui_display_config` and `picoui_display_*` do not exist.

- [ ] **Step 3: Add display public headers**

Create `picoui/include/picoui/port/display.h`:

```c
#ifndef PICOUI_PORT_DISPLAY_H
#define PICOUI_PORT_DISPLAY_H

struct picoui_app;

enum picoui_color_format {
    PICOUI_COLOR_FORMAT_RGB565 = 0,
    PICOUI_COLOR_FORMAT_ARGB8888,
};

struct picoui_area {
    int x;
    int y;
    int width;
    int height;
};

struct picoui_display_config {
    int width;
    int height;
    enum picoui_color_format color_format;
    int buffer_height;
    void *user_data;
};

typedef void (*picoui_display_flush_cb_t)(const struct picoui_area *area,
                                          const void *pixels,
                                          void *user_data);

int picoui_display_set_config(struct picoui_app *app,
                              const struct picoui_display_config *config);
int picoui_display_get_config(const struct picoui_app *app,
                              struct picoui_display_config *out_config);
int picoui_display_set_flush_callback(struct picoui_app *app,
                                      picoui_display_flush_cb_t callback,
                                      void *user_data);

#endif
```

Create `picoui/include/picoui/port.h`:

```c
#ifndef PICOUI_PORT_H
#define PICOUI_PORT_H

#include "picoui/port/display.h"

#endif
```

Add to `picoui/include/picoui/picoui.h`:

```c
#include "picoui/port.h"
```

- [ ] **Step 4: Add display state to internal app**

In `picoui/src/core/internal.h`, add after `struct picoui_app_timer`:

```c
struct picoui_display_port_state {
    struct picoui_display_config config;
    picoui_display_flush_cb_t flush_callback;
    void *flush_user_data;
};
```

Add to `struct picoui_app`:

```c
    struct picoui_display_port_state display_port;
```

Also include the display header near other public headers:

```c
#include "picoui/port/display.h"
```

- [ ] **Step 5: Implement display state**

Create `picoui/src/port/display.c`:

```c
#include "internal.h"
#include "picoui/port/display.h"

static const struct picoui_display_config g_picoui_default_display_config = {
    .width = 480,
    .height = 320,
    .color_format = PICOUI_COLOR_FORMAT_RGB565,
    .buffer_height = 0,
    .user_data = 0,
};

static int picoui_display_config_is_valid(const struct picoui_display_config *config)
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

static void picoui_display_ensure_default(struct picoui_app *app)
{
    if (app == 0) {
        return;
    }
    if (app->display_port.config.width <= 0 || app->display_port.config.height <= 0) {
        app->display_port.config = g_picoui_default_display_config;
    }
}

int picoui_display_set_config(struct picoui_app *app,
                              const struct picoui_display_config *config)
{
    if (app == 0 || !picoui_display_config_is_valid(config)) {
        return -1;
    }
    app->display_port.config = *config;
    return 0;
}

int picoui_display_get_config(const struct picoui_app *app,
                              struct picoui_display_config *out_config)
{
    struct picoui_app *mutable_app = (struct picoui_app *)app;

    if (app == 0 || out_config == 0) {
        return -1;
    }
    picoui_display_ensure_default(mutable_app);
    *out_config = mutable_app->display_port.config;
    return 0;
}

int picoui_display_set_flush_callback(struct picoui_app *app,
                                      picoui_display_flush_cb_t callback,
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

In `cmake/LingDongGUI.cmake`, add to `picoui_core` sources:

```cmake
        ${LD_REPO_ROOT}/picoui/src/port/display.c
```

- [ ] **Step 7: Verify display test passes**

Run:

```bash
rtk cmake --build build --target test_picoui_port_display
rtk ctest --test-dir build -R '^test_picoui_port_display$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 2: Input Port Contract

**Files:**
- Create: `picoui/include/picoui/port/input.h`
- Modify: `picoui/include/picoui/port.h`
- Create: `picoui/src/port/input.c`
- Modify: `picoui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/unit/test_picoui_port_input.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: Write failing input test**

Create `tests/picoui/unit/test_picoui_port_input.c`:

```c
#include "picoui/picoui.h"

#include <assert.h>

static void test_pointer_input_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    int x = -1;
    int y = -1;
    int pressed = -1;

    assert(app != NULL);
    assert(picoui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    assert(picoui_input_push_pointer(app, 17, 23, 1) == 0);
    assert(picoui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 17);
    assert(y == 23);
    assert(pressed == 1);

    assert(picoui_input_push_pointer(app, -4, -5, 0) == 0);
    assert(picoui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    picoui_app_destroy(app);
}

static void test_key_input_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    enum picoui_input_key key = PICOUI_INPUT_KEY_NONE;
    int pressed = -1;

    assert(app != NULL);
    assert(picoui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_NONE);
    assert(pressed == 0);

    assert(picoui_input_push_key(app, PICOUI_INPUT_KEY_ENTER, 1) == 0);
    assert(picoui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_ENTER);
    assert(pressed == 1);

    picoui_app_destroy(app);
}

static void test_input_rejects_invalid_arguments(void)
{
    struct picoui_app *app = picoui_app_create();
    int x;
    int y;
    int pressed;

    assert(app != NULL);
    assert(picoui_input_push_pointer(NULL, 1, 2, 1) == -1);
    assert(picoui_input_get_pointer(NULL, &x, &y, &pressed) == -1);
    assert(picoui_input_get_pointer(app, NULL, &y, &pressed) == -1);
    assert(picoui_input_get_pointer(app, &x, NULL, &pressed) == -1);
    assert(picoui_input_get_pointer(app, &x, &y, NULL) == -1);
    assert(picoui_input_push_key(NULL, PICOUI_INPUT_KEY_ENTER, 1) == -1);
    assert(picoui_input_push_key(app, (enum picoui_input_key)999, 1) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_pointer_input_round_trip();
    test_key_input_round_trip();
    test_input_rejects_invalid_arguments();
    return 0;
}
```

Register it in `tests/picoui/CMakeLists.txt`:

```cmake
    unit/test_picoui_port_input.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_picoui_port_input
```

Expected: compile fails because `picoui_input_*` does not exist.

- [ ] **Step 3: Add input header**

Create `picoui/include/picoui/port/input.h`:

```c
#ifndef PICOUI_PORT_INPUT_H
#define PICOUI_PORT_INPUT_H

struct picoui_app;

enum picoui_input_key {
    PICOUI_INPUT_KEY_NONE = 0,
    PICOUI_INPUT_KEY_LEFT,
    PICOUI_INPUT_KEY_RIGHT,
    PICOUI_INPUT_KEY_UP,
    PICOUI_INPUT_KEY_DOWN,
    PICOUI_INPUT_KEY_ENTER,
    PICOUI_INPUT_KEY_BACK,
};

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed);
int picoui_input_get_pointer(const struct picoui_app *app,
                             int *x,
                             int *y,
                             int *pressed);
int picoui_input_push_key(struct picoui_app *app,
                          enum picoui_input_key key,
                          int pressed);
int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed);

#endif
```

Update `picoui/include/picoui/port.h`:

```c
#include "picoui/port/input.h"
```

- [ ] **Step 4: Add input state to app**

In `picoui/src/core/internal.h`, include:

```c
#include "picoui/port/input.h"
```

Add:

```c
struct picoui_input_port_state {
    int pointer_x;
    int pointer_y;
    int pointer_pressed;
    enum picoui_input_key key;
    int key_pressed;
};
```

Add to `struct picoui_app`:

```c
    struct picoui_input_port_state input_port;
```

- [ ] **Step 5: Implement input state**

Create `picoui/src/port/input.c`:

```c
#include "internal.h"
#include "picoui/port/input.h"

static int picoui_input_key_is_valid(enum picoui_input_key key)
{
    return key >= PICOUI_INPUT_KEY_NONE && key <= PICOUI_INPUT_KEY_BACK;
}

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed)
{
    if (app == 0) {
        return -1;
    }
    app->input_port.pointer_x = x < 0 ? 0 : x;
    app->input_port.pointer_y = y < 0 ? 0 : y;
    app->input_port.pointer_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_pointer(const struct picoui_app *app,
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

int picoui_input_push_key(struct picoui_app *app,
                          enum picoui_input_key key,
                          int pressed)
{
    if (app == 0 || !picoui_input_key_is_valid(key)) {
        return -1;
    }
    app->input_port.key = key;
    app->input_port.key_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
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

In `cmake/LingDongGUI.cmake`, add to `picoui_core` sources:

```cmake
        ${LD_REPO_ROOT}/picoui/src/port/input.c
```

- [ ] **Step 7: Verify input test passes**

Run:

```bash
rtk cmake --build build --target test_picoui_port_input
rtk ctest --test-dir build -R '^test_picoui_port_input$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 3: Tick And OS Port Contracts

**Files:**
- Create: `picoui/include/picoui/port/tick.h`
- Create: `picoui/include/picoui/port/os.h`
- Modify: `picoui/include/picoui/port.h`
- Create: `picoui/src/port/tick.c`
- Create: `picoui/src/port/os.c`
- Modify: `picoui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/unit/test_picoui_port_tick_os.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: Write failing tick/OS test**

Create `tests/picoui/unit/test_picoui_port_tick_os.c`:

```c
#include "picoui/picoui.h"

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
    struct picoui_app *app = picoui_app_create();
    struct port_probe probe = {.tick = 1234};

    assert(app != NULL);
    assert(picoui_tick_set_source(app, probe_tick, &probe) == 0);
    assert(picoui_tick_get(app) == 1234);
    probe.tick = 5678;
    assert(picoui_tick_get(app) == 5678);

    picoui_app_destroy(app);
}

static void test_os_callbacks_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    struct port_probe probe = {0};

    assert(app != NULL);
    assert(picoui_os_set_lock_callbacks(app, probe_enter, probe_leave, &probe) == 0);
    picoui_os_enter(app);
    picoui_os_leave(app);
    assert(probe.enter_count == 1);
    assert(probe.leave_count == 1);

    assert(picoui_os_set_delay_callback(app, probe_delay, &probe) == 0);
    picoui_os_delay(app, 42);
    assert(probe.delay_ms == 42);

    picoui_app_destroy(app);
}

static void test_tick_os_rejects_invalid_arguments(void)
{
    struct picoui_app *app = picoui_app_create();

    assert(app != NULL);
    assert(picoui_tick_set_source(NULL, probe_tick, NULL) == -1);
    assert(picoui_tick_set_source(app, NULL, NULL) == -1);
    assert(picoui_os_set_lock_callbacks(NULL, NULL, NULL, NULL) == -1);
    assert(picoui_os_set_delay_callback(NULL, NULL, NULL) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_tick_source_round_trip();
    test_os_callbacks_round_trip();
    test_tick_os_rejects_invalid_arguments();
    return 0;
}
```

Register it in `tests/picoui/CMakeLists.txt`:

```cmake
    unit/test_picoui_port_tick_os.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_picoui_port_tick_os
```

Expected: compile fails because `picoui_tick_*` and `picoui_os_*` do not exist.

- [ ] **Step 3: Add tick and OS headers**

Create `picoui/include/picoui/port/tick.h`:

```c
#ifndef PICOUI_PORT_TICK_H
#define PICOUI_PORT_TICK_H

struct picoui_app;

typedef unsigned int (*picoui_tick_get_cb_t)(void *user_data);

int picoui_tick_set_source(struct picoui_app *app,
                           picoui_tick_get_cb_t callback,
                           void *user_data);
unsigned int picoui_tick_get(struct picoui_app *app);

#endif
```

Create `picoui/include/picoui/port/os.h`:

```c
#ifndef PICOUI_PORT_OS_H
#define PICOUI_PORT_OS_H

struct picoui_app;

typedef void (*picoui_os_lock_cb_t)(void *user_data);
typedef void (*picoui_os_delay_cb_t)(unsigned int ms, void *user_data);

int picoui_os_set_lock_callbacks(struct picoui_app *app,
                                 picoui_os_lock_cb_t enter,
                                 picoui_os_lock_cb_t leave,
                                 void *user_data);
void picoui_os_enter(struct picoui_app *app);
void picoui_os_leave(struct picoui_app *app);
int picoui_os_set_delay_callback(struct picoui_app *app,
                                 picoui_os_delay_cb_t delay,
                                 void *user_data);
void picoui_os_delay(struct picoui_app *app, unsigned int ms);

#endif
```

Update `picoui/include/picoui/port.h`:

```c
#include "picoui/port/tick.h"
#include "picoui/port/os.h"
```

- [ ] **Step 4: Add tick/OS state to app**

In `picoui/src/core/internal.h`, include:

```c
#include "picoui/port/tick.h"
#include "picoui/port/os.h"
```

Add:

```c
struct picoui_tick_port_state {
    picoui_tick_get_cb_t callback;
    void *user_data;
};

struct picoui_os_port_state {
    picoui_os_lock_cb_t enter;
    picoui_os_lock_cb_t leave;
    void *lock_user_data;
    picoui_os_delay_cb_t delay;
    void *delay_user_data;
};
```

Add to `struct picoui_app`:

```c
    struct picoui_tick_port_state tick_port;
    struct picoui_os_port_state os_port;
```

- [ ] **Step 5: Implement tick and OS state**

Create `picoui/src/port/tick.c`:

```c
#include "internal.h"
#include "picoui/port/tick.h"

int picoui_tick_set_source(struct picoui_app *app,
                           picoui_tick_get_cb_t callback,
                           void *user_data)
{
    if (app == 0 || callback == 0) {
        return -1;
    }
    app->tick_port.callback = callback;
    app->tick_port.user_data = user_data;
    return 0;
}

unsigned int picoui_tick_get(struct picoui_app *app)
{
    if (app == 0 || app->tick_port.callback == 0) {
        return 0;
    }
    return app->tick_port.callback(app->tick_port.user_data);
}
```

Create `picoui/src/port/os.c`:

```c
#include "internal.h"
#include "picoui/port/os.h"

int picoui_os_set_lock_callbacks(struct picoui_app *app,
                                 picoui_os_lock_cb_t enter,
                                 picoui_os_lock_cb_t leave,
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

void picoui_os_enter(struct picoui_app *app)
{
    if (app != 0 && app->os_port.enter != 0) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void picoui_os_leave(struct picoui_app *app)
{
    if (app != 0 && app->os_port.leave != 0) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

int picoui_os_set_delay_callback(struct picoui_app *app,
                                 picoui_os_delay_cb_t delay,
                                 void *user_data)
{
    if (app == 0) {
        return -1;
    }
    app->os_port.delay = delay;
    app->os_port.delay_user_data = user_data;
    return 0;
}

void picoui_os_delay(struct picoui_app *app, unsigned int ms)
{
    if (app != 0 && app->os_port.delay != 0) {
        app->os_port.delay(ms, app->os_port.delay_user_data);
    }
}
```

- [ ] **Step 6: Add sources to CMake**

In `cmake/LingDongGUI.cmake`, add to `picoui_core` sources:

```cmake
        ${LD_REPO_ROOT}/picoui/src/port/tick.c
        ${LD_REPO_ROOT}/picoui/src/port/os.c
```

- [ ] **Step 7: Verify tick/OS test passes**

Run:

```bash
rtk cmake --build build --target test_picoui_port_tick_os
rtk ctest --test-dir build -R '^test_picoui_port_tick_os$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 4: SDL Port Attach

**Files:**
- Create: `picoui/port/sdl/picoui_port_sdl.h`
- Create: `picoui/port/sdl/picoui_port_sdl.c`
- Modify: `cmake/LingDongGUI.cmake`
- Create: `tests/picoui/unit/test_picoui_port_sdl.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: Write failing SDL attach test**

Create `tests/picoui/unit/test_picoui_port_sdl.c`:

```c
#include "picoui/picoui.h"
#include "picoui_port_sdl.h"

#include <assert.h>

static void test_sdl_attach_sets_port_defaults(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {0};

    assert(app != NULL);
    assert(picoui_port_sdl_attach(app) == 0);
    assert(picoui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(picoui_tick_get(app) >= 0);
    picoui_os_delay(app, 0);

    picoui_app_destroy(app);
}

static void test_sdl_attach_rejects_null_app(void)
{
    assert(picoui_port_sdl_attach(NULL) == -1);
}

int main(void)
{
    test_sdl_attach_sets_port_defaults();
    test_sdl_attach_rejects_null_app();
    return 0;
}
```

Register it in `tests/picoui/CMakeLists.txt`:

```cmake
    unit/test_picoui_port_sdl.c
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
rtk cmake --build build --target test_picoui_port_sdl
```

Expected: compile fails because `picoui_port_sdl.h` and `picoui_port_sdl_attach()` do not exist.

- [ ] **Step 3: Add SDL port header**

Create `picoui/port/sdl/picoui_port_sdl.h`:

```c
#ifndef PICOUI_PORT_SDL_H
#define PICOUI_PORT_SDL_H

struct picoui_app;

int picoui_port_sdl_attach(struct picoui_app *app);

#endif
```

- [ ] **Step 4: Add SDL port implementation**

Create `picoui/port/sdl/picoui_port_sdl.c`:

```c
#include "picoui/app.h"
#include "picoui/port.h"
#include "picoui_port_sdl.h"

#include <SDL.h>

static unsigned int picoui_port_sdl_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

static void picoui_port_sdl_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay(ms);
}

int picoui_port_sdl_attach(struct picoui_app *app)
{
    struct picoui_display_config config = {
        .width = 480,
        .height = 320,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };

    if (app == 0) {
        return -1;
    }
    if (picoui_display_set_config(app, &config) != 0) {
        return -1;
    }
    if (picoui_tick_set_source(app, picoui_port_sdl_tick_get, 0) != 0) {
        return -1;
    }
    if (picoui_os_set_delay_callback(app, picoui_port_sdl_delay, 0) != 0) {
        return -1;
    }
    return 0;
}
```

- [ ] **Step 5: Add SDL port to CMake**

In `cmake/LingDongGUI.cmake`, add the SDL port source and include directory to both backend targets inside the existing `foreach(LD_PICOUI_BACKEND_TARGET IN ITEMS picoui_backend_ldgui picoui_backend_ldgui_runtime)` loop. Both targets already link SDL in the current CMake file, so this keeps unit tests and runtime builds consistent:

```cmake
        target_sources(${LD_PICOUI_BACKEND_TARGET} PRIVATE
            ${LD_REPO_ROOT}/picoui/port/sdl/picoui_port_sdl.c
        )
        target_include_directories(${LD_PICOUI_BACKEND_TARGET} PUBLIC
            ${LD_REPO_ROOT}/picoui/port/sdl
        )
```

Do not add any new PicoUI port files under `examples/sdl`.

- [ ] **Step 6: Verify SDL attach test passes**

Run:

```bash
rtk cmake --build build --target test_picoui_port_sdl
rtk ctest --test-dir build -R '^test_picoui_port_sdl$' --output-on-failure
```

Expected: build succeeds and ctest reports `100% tests passed`.

---

### Task 5: Backend App Consumes Port State

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Test: `tests/picoui/unit/test_picoui_port_display.c`
- Test: `tests/picoui/unit/test_picoui_port_input.c`
- Test: `tests/picoui/unit/test_picoui_port_tick_os.c`

- [ ] **Step 1: Add backend assertions to focused tests**

Extend `test_picoui_port_display.c` with a regression assertion that changing display config remains visible before runtime:

```c
static void test_display_config_can_drive_runtime_dimensions(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {
        .width = 300,
        .height = 200,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 20,
        .user_data = 0,
    };
    struct picoui_display_config readback = {0};

    assert(app != NULL);
    assert(picoui_display_set_config(app, &config) == 0);
    assert(picoui_display_get_config(app, &readback) == 0);
    assert(readback.width == 300);
    assert(readback.height == 200);

    picoui_app_destroy(app);
}
```

Call it from `main()`.

- [ ] **Step 2: Replace hardcoded runtime size reads**

In `backend_app.c`, add a helper:

```c
static struct picoui_display_config picoui_backend_display_config_from_app(struct picoui_app *app)
{
    struct picoui_display_config config;

    if (picoui_display_get_config(app, &config) != 0) {
        config.width = PICOUI_RUNTIME_WIDTH;
        config.height = PICOUI_RUNTIME_HEIGHT;
        config.color_format = PICOUI_COLOR_FORMAT_RGB565;
        config.buffer_height = 0;
        config.user_data = NULL;
    }
    return config;
}
```

Use the helper in `picoui_backend_ensure_window()`, tile allocation, texture creation, present loop, capture writing, and render clearing. Keep `PICOUI_RUNTIME_WIDTH/HEIGHT` as fallback constants only.

- [ ] **Step 3: Route SDL input through PicoUI input port**

In `picoui_backend_commit_pointer_event()`, replace direct state write with:

```c
    (void)picoui_input_push_pointer(state_app, mapped_x, mapped_y, pressed != 0);
```

If current function lacks `app`, change signature to:

```c
static void picoui_backend_commit_pointer_event(struct picoui_app *app,
                                                struct picoui_backend_runtime_state *state,
                                                int x,
                                                int y,
                                                int pressed)
```

Then after successful push, keep backend-private bridge:

```c
    ldCfgTouchSetPoint(mapped_x, mapped_y, pressed != 0);
```

This is an interim bridge: SDL talks to PicoUI input first; LingDongGUI remains backend-private.

- [ ] **Step 4: Route tick and delay through PicoUI port**

Replace timer pump call:

```c
picoui_backend_pump_timers(app, picoui_tick_get(app));
```

If `picoui_tick_get(app)` returns `0` before SDL attach is wired, fallback to `SDL_GetTicks()` until Task 6 attaches SDL by default.

Replace:

```c
SDL_Delay(16);
```

with:

```c
picoui_os_delay(app, 16);
```

Use SDL fallback only when no delay callback is installed.

- [ ] **Step 5: Verify focused tests**

Run:

```bash
rtk cmake --build build --target test_picoui_port_display test_picoui_port_input test_picoui_port_tick_os
rtk ctest --test-dir build -R 'test_picoui_port_display|test_picoui_port_input|test_picoui_port_tick_os' --output-on-failure
```

Expected: all selected tests pass.

---

### Task 6: Runtime Default SDL Attach And Public API Gate

**Files:**
- Modify: `picoui/src/core/app.c`
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `cmake/LingDongGUI.cmake`
- Test: `tests/picoui/contract/check_picoui_public_api.py`

- [ ] **Step 1: Attach SDL port for runtime target**

In `picoui/src/core/app.c`, do not include SDL headers. Instead, keep core backend-neutral. In `backend_app.c`, include:

```c
#include "picoui_port_sdl.h"
```

In `picoui_backend_app_init()`, after `app_state->runtime_state = state;`, attach SDL only when the runtime target enables the compile definition:

```c
#if defined(PICOUI_ENABLE_SDL_PORT)
    (void)picoui_port_sdl_attach(app);
#endif
```

- [ ] **Step 2: Update CMake compile definitions**

In `cmake/LingDongGUI.cmake`, for `picoui_backend_ldgui_runtime`, add:

```cmake
            target_compile_definitions(${LD_PICOUI_BACKEND_TARGET} PRIVATE PICOUI_ENABLE_SDL_PORT=1)
```

Do not add new PicoUI port files under `examples/sdl`.

- [ ] **Step 3: Run public API leak check**

Run:

```bash
rtk ctest --test-dir build -R '^check_picoui_public_api$' --output-on-failure
```

Expected: PASS. If it fails because `picoui/include/picoui/port/*.h` exposes `ld*`, `arm_2d_*`, or `SIGNAL_*`, remove those identifiers from public headers.

- [ ] **Step 4: Run focused port tests**

Run:

```bash
rtk cmake --build build --target test_picoui_port_display test_picoui_port_input test_picoui_port_tick_os test_picoui_port_sdl
rtk ctest --test-dir build -R 'test_picoui_port_display|test_picoui_port_input|test_picoui_port_tick_os|test_picoui_port_sdl|check_picoui_public_api' --output-on-failure
```

Expected: all selected tests pass.

---

### Task 7: Documentation Sync

**Files:**
- Modify: `docs/picoui-serial/a-0.14/2026-06-05-picoui-port-boundary-audit.md`
- Modify: `docs/picoui-serial/a-0.14-线计划索引.md`

- [ ] **Step 1: Update audit document P1 status**

In `docs/picoui-serial/a-0.14/2026-06-05-picoui-port-boundary-audit.md`, under `P1：SDL port 先行`, add a completion note with exact evidence:

```markdown
> P1 实现状态：已完成/部分完成。证据以本轮实际命令为准：
>
> - `rtk ctest --test-dir build -R 'test_picoui_port_display|test_picoui_port_input|test_picoui_port_tick_os|test_picoui_port_sdl|check_picoui_public_api' --output-on-failure`
```

Use `已完成` only if the command actually passes.

- [ ] **Step 2: Update a-0.14 index**

In `docs/picoui-serial/a-0.14-线计划索引.md`, update the `当前进度` section. Use this wording only after tests pass:

```markdown
  - PicoUI port P1：display/input/tick/OS 合同与 SDL port 最小闭环
```

If tests do not pass, write:

```markdown
  - PicoUI port P1：部分完成，剩余失败见最新测试输出
```

- [ ] **Step 3: Verify docs contain no stale recommendation**

Run:

```bash
rtk rg -n 'platform/ldgui|port/ldgui|platform/sdl|examples/sdl.*(新增|承载|推荐|port)' docs/picoui-serial/a-0.14 docs/superpowers/specs/2026-06-05-picoui-port-layer-design.md docs/superpowers/plans/2026-06-05-picoui-port-layer-implementation.md
```

Expected: no matches except explicit negative statements saying not to use those paths.

---

## Final Verification

Run:

```bash
rtk cmake --build build --target test_picoui_port_display test_picoui_port_input test_picoui_port_tick_os test_picoui_port_sdl
rtk ctest --test-dir build -R 'test_picoui_port_display|test_picoui_port_input|test_picoui_port_tick_os|test_picoui_port_sdl|check_picoui_public_api' --output-on-failure
rtk git diff --stat
```

Expected:

- All selected tests pass.
- New files live under `picoui/include/picoui/port*`, `picoui/src/port/`, and `picoui/port/sdl/`.
- No new PicoUI SDL port implementation lives under `examples/sdl`.
- Public headers do not expose `ld*`, `arm_2d_*`, or `SIGNAL_*`.
