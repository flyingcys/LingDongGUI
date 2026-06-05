# PicoUI v1.0.1 P1 Native Vertical Slice Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first LVGL-like native runtime vertical slice and migrate basic_widgets main as the sample.

**Architecture:** This phase introduces the public runtime/display/indev/screen API and a minimal native implementation. It keeps scope to one demo and one vertical render/input path.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P1: Native 垂直切片

### Task P1-A: Runtime public API RED contract

**Files:**
- Create: `picoui/include/picoui/runtime.h`
- Modify: `picoui/include/picoui/picoui.h`
- Create: `tests/picoui/native/test_picoui_native_runtime.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 写 native runtime RED test**

新增 `tests/picoui/native/test_picoui_native_runtime.c`：

```c
#include "picoui/picoui.h"

#include <assert.h>

static void test_runtime_init_deinit_is_idempotent(void)
{
    assert(picoui_init() == 0);
    assert(picoui_init() == 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    assert(picoui_init() == 0);
    picoui_deinit();
}

int main(void)
{
    test_runtime_init_deinit_is_idempotent();
    return 0;
}
```

- [ ] **Step 2: 注册 test**

在 `tests/picoui/CMakeLists.txt` 中新增 native test 列表；若尚无专用 native target，先用 `picoui_backend_ldgui` 链接保持编译通过，P7 再移除 ldgui：

```cmake
set(PICOUI_NATIVE_TESTS
    native/test_picoui_native_runtime.c
)

foreach(test_src IN LISTS PICOUI_NATIVE_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB picoui_test_support
        MAIN_LIB picoui_backend_ldgui
        LABELS "picoui;native"
    )
endforeach()
```

- [ ] **Step 3: Run RED**

Run:

```bash
rtk cmake --build build --target test_picoui_native_runtime
```

Expected:

- FAIL，`picoui_init` / `picoui_timer_handler` / `picoui_deinit` undeclared。

- [ ] **Step 4: 添加 public header**

新增 `picoui/include/picoui/runtime.h`：

```c
#ifndef PICOUI_RUNTIME_H
#define PICOUI_RUNTIME_H

int picoui_init(void);
void picoui_deinit(void);
int picoui_timer_handler(void);

#endif
```

在 `picoui/include/picoui/picoui.h` include：

```c
#include "picoui/runtime.h"
```

- [ ] **Step 5: GREEN 暂不实现**

此任务只建立 RED contract 和 public API header。实现放到 P1-B。

- [ ] **Step 6: Commit**

```bash
git add picoui/include/picoui/runtime.h picoui/include/picoui/picoui.h tests/picoui/CMakeLists.txt tests/picoui/native/test_picoui_native_runtime.c
git commit -m "test: add picoui runtime api contract"
```

### Task P1-B: Runtime minimal implementation

**Files:**
- Create: `picoui/src/native/native_runtime.c`
- Modify: `picoui/src/core/internal.h`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: GitNexus impact**

Run impact before editing:

```text
mcp__gitnexus.impact({repo:"LingDongGUI", target:"picoui_app_create", file_path:"picoui/src/core/app.c", direction:"upstream"})
```

Expected: report risk. If HIGH/CRITICAL, stop and report.

- [ ] **Step 2: 实现 minimal runtime**

新增 `picoui/src/native/native_runtime.c`：

```c
#include "picoui/runtime.h"

struct picoui_runtime_state {
    int initialized;
};

static struct picoui_runtime_state g_picoui_runtime;

int picoui_init(void)
{
    g_picoui_runtime.initialized = 1;
    return 0;
}

void picoui_deinit(void)
{
    g_picoui_runtime.initialized = 0;
}

int picoui_timer_handler(void)
{
    if (!g_picoui_runtime.initialized) {
        return -1;
    }
    return 0;
}
```

- [ ] **Step 3: 加入 CMake**

在 `cmake/LingDongGUI.cmake` 的 `picoui_core` source list 加：

```cmake
${LD_REPO_ROOT}/picoui/src/native/native_runtime.c
```

- [ ] **Step 4: 添加阶段文档**

新增或更新 `docs/picoui-serial/v1.0-native/01-native垂直切片.md`：

```markdown
# PicoUI v1.0 Native P1 垂直切片

## 当前状态

- `picoui_init()` / `picoui_deinit()` / `picoui_timer_handler()` 最小 API 已建立。
- 当前实现只证明 runtime lifecycle contract，不证明 native render 完成。
```

- [ ] **Step 5: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_native_runtime
rtk ctest --test-dir build -R test_picoui_native_runtime --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 6: Commit**

```bash
git add picoui/src/native/native_runtime.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "feat: add picoui native runtime lifecycle"
```

### Task P1-C: Display/indev/screen API contract

**Files:**
- Create: `picoui/include/picoui/screen.h`
- Modify: `picoui/include/picoui/display.h`
- Modify: `picoui/include/picoui/indev.h`
- Modify: `picoui/include/picoui/picoui.h`
- Create: `tests/picoui/native/test_picoui_native_display_indev.c`
- Create: `tests/picoui/native/test_picoui_native_screen.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 写 display/indev RED test**

新增 `tests/picoui/native/test_picoui_native_display_indev.c`：

```c
#include "picoui/picoui.h"

#include <assert.h>

static void flush_cb(const struct picoui_area *area, const void *pixels, void *user_data)
{
    (void)area;
    (void)pixels;
    int *called = (int *)user_data;
    *called = 1;
}

static void indev_read_cb(struct picoui_indev *indev, struct picoui_indev_data *data, void *user_data)
{
    (void)indev;
    (void)user_data;
    data->pointer_x = 12;
    data->pointer_y = 34;
    data->pressed = 1;
}

int main(void)
{
    int called = 0;
    struct picoui_display *display;
    struct picoui_indev *indev;

    assert(picoui_init() == 0);
    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_flush_cb(display, flush_cb, &called) == 0);
    assert(picoui_display_set_default(display) == 0);
    assert(picoui_display_get_default() == display);

    indev = picoui_indev_create();
    assert(indev != 0);
    assert(picoui_indev_set_type(indev, PICOUI_INDEV_TYPE_POINTER) == 0);
    assert(picoui_indev_set_read_cb(indev, indev_read_cb, 0) == 0);

    picoui_deinit();
    return 0;
}
```

- [ ] **Step 2: 写 screen RED test**

新增 `tests/picoui/native/test_picoui_native_screen.c`：

```c
#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *active;
    struct picoui_screen *second;

    assert(picoui_init() == 0);
    active = picoui_screen_active();
    assert(active != 0);
    second = picoui_screen_create();
    assert(second != 0);
    assert(picoui_screen_load(second) == 0);
    assert(picoui_screen_active() == second);
    picoui_deinit();
    return 0;
}
```

- [ ] **Step 3: 注册 tests**

在 `PICOUI_NATIVE_TESTS` 添加：

```cmake
    native/test_picoui_native_display_indev.c
    native/test_picoui_native_screen.c
```

- [ ] **Step 4: RED**

Run:

```bash
rtk cmake --build build --target test_picoui_native_display_indev test_picoui_native_screen
```

Expected: FAIL，相关类型/API 未定义。

- [ ] **Step 5: 添加 headers**

在 `picoui/include/picoui/display.h` 加 handle API；保留旧 app-based API 不删：

```c
struct picoui_display;

struct picoui_display *picoui_display_create(int width, int height);
int picoui_display_set_default(struct picoui_display *display);
struct picoui_display *picoui_display_get_default(void);
int picoui_display_set_flush_cb(struct picoui_display *display,
                                picoui_display_flush_cb_t callback,
                                void *user_data);
```

在 `picoui/include/picoui/indev.h` 加：

```c
struct picoui_indev;

enum picoui_indev_type {
    PICOUI_INDEV_TYPE_NONE = 0,
    PICOUI_INDEV_TYPE_POINTER,
    PICOUI_INDEV_TYPE_KEYPAD,
    PICOUI_INDEV_TYPE_ENCODER,
};

struct picoui_indev_data {
    int pointer_x;
    int pointer_y;
    int pressed;
    enum picoui_input_key key;
};

typedef void (*picoui_indev_read_cb_t)(struct picoui_indev *indev,
                                       struct picoui_indev_data *data,
                                       void *user_data);

struct picoui_indev *picoui_indev_create(void);
int picoui_indev_set_type(struct picoui_indev *indev, enum picoui_indev_type type);
int picoui_indev_set_read_cb(struct picoui_indev *indev,
                             picoui_indev_read_cb_t callback,
                             void *user_data);
```

新增 `picoui/include/picoui/screen.h`：

```c
#ifndef PICOUI_SCREEN_H
#define PICOUI_SCREEN_H

struct picoui_screen;

struct picoui_screen *picoui_screen_active(void);
struct picoui_screen *picoui_screen_create(void);
int picoui_screen_load(struct picoui_screen *screen);

#endif
```

在 `picoui/include/picoui/picoui.h` include `picoui/screen.h`。

- [ ] **Step 6: Commit**

```bash
git add picoui/include/picoui/display.h picoui/include/picoui/indev.h picoui/include/picoui/screen.h picoui/include/picoui/picoui.h tests/picoui/native/test_picoui_native_display_indev.c tests/picoui/native/test_picoui_native_screen.c tests/picoui/CMakeLists.txt
git commit -m "test: add native display indev screen contracts"
```

### Task P1-D: Display/indev/screen minimal implementation

**Files:**
- Create: `picoui/src/native/native_display.c`
- Create: `picoui/src/native/native_indev.c`
- Create: `picoui/src/native/native_screen.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: Implement display**

新增 `picoui/src/native/native_display.c`：

```c
#include "picoui/display.h"

#include <stdlib.h>

struct picoui_display {
    int width;
    int height;
    picoui_display_flush_cb_t flush_cb;
    void *flush_user_data;
};

static struct picoui_display *g_default_display;

struct picoui_display *picoui_display_create(int width, int height)
{
    struct picoui_display *display;
    if (width <= 0 || height <= 0) {
        return 0;
    }
    display = calloc(1, sizeof(*display));
    if (display == 0) {
        return 0;
    }
    display->width = width;
    display->height = height;
    if (g_default_display == 0) {
        g_default_display = display;
    }
    return display;
}

int picoui_display_set_default(struct picoui_display *display)
{
    if (display == 0) {
        return -1;
    }
    g_default_display = display;
    return 0;
}

struct picoui_display *picoui_display_get_default(void)
{
    return g_default_display;
}

int picoui_display_set_flush_cb(struct picoui_display *display,
                                picoui_display_flush_cb_t callback,
                                void *user_data)
{
    if (display == 0 || callback == 0) {
        return -1;
    }
    display->flush_cb = callback;
    display->flush_user_data = user_data;
    return 0;
}
```

- [ ] **Step 2: Implement indev**

新增 `picoui/src/native/native_indev.c`：

```c
#include "picoui/indev.h"

#include <stdlib.h>

struct picoui_indev {
    enum picoui_indev_type type;
    picoui_indev_read_cb_t read_cb;
    void *read_user_data;
};

struct picoui_indev *picoui_indev_create(void)
{
    return calloc(1, sizeof(struct picoui_indev));
}

int picoui_indev_set_type(struct picoui_indev *indev, enum picoui_indev_type type)
{
    if (indev == 0) {
        return -1;
    }
    indev->type = type;
    return 0;
}

int picoui_indev_set_read_cb(struct picoui_indev *indev,
                             picoui_indev_read_cb_t callback,
                             void *user_data)
{
    if (indev == 0 || callback == 0) {
        return -1;
    }
    indev->read_cb = callback;
    indev->read_user_data = user_data;
    return 0;
}
```

- [ ] **Step 3: Implement screen**

新增 `picoui/src/native/native_screen.c`：

```c
#include "picoui/screen.h"

#include <stdlib.h>

struct picoui_screen {
    int loaded;
};

static struct picoui_screen g_default_screen = {1};
static struct picoui_screen *g_active_screen = &g_default_screen;

struct picoui_screen *picoui_screen_active(void)
{
    return g_active_screen;
}

struct picoui_screen *picoui_screen_create(void)
{
    return calloc(1, sizeof(struct picoui_screen));
}

int picoui_screen_load(struct picoui_screen *screen)
{
    if (screen == 0) {
        return -1;
    }
    if (g_active_screen != 0) {
        g_active_screen->loaded = 0;
    }
    screen->loaded = 1;
    g_active_screen = screen;
    return 0;
}
```

- [ ] **Step 4: CMake**

把三个 `.c` 加入 `picoui_core`：

```cmake
${LD_REPO_ROOT}/picoui/src/native/native_display.c
${LD_REPO_ROOT}/picoui/src/native/native_indev.c
${LD_REPO_ROOT}/picoui/src/native/native_screen.c
```

- [ ] **Step 5: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_native_display_indev test_picoui_native_screen
rtk ctest --test-dir build -R 'test_picoui_native_display_indev|test_picoui_native_screen' --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 6: Update docs and commit**

在 `01-native垂直切片.md` 记录 display/indev/screen 最小合同已绿。

```bash
git add picoui/src/native/native_display.c picoui/src/native/native_indev.c picoui/src/native/native_screen.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "feat: add native display indev screen handles"
```

### Task P1-E: Screen-backed root window creation

**Files:**
- Modify: `picoui/include/picoui/window.h`
- Modify: `picoui/src/widgets/window.c`
- Create: `tests/picoui/native/test_picoui_native_screen_window.c`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: RED test**

新增 `tests/picoui/native/test_picoui_native_screen_window.c`：

```c
#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_window *window;

    assert(picoui_init() == 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    window = picoui_window_create_root(screen, "root");
    assert(window != 0);
    assert(picoui_widget_get_parent((struct picoui_widget *)window) == 0);
    picoui_deinit();
    return 0;
}
```

- [ ] **Step 2: 注册**

加入 `PICOUI_NATIVE_TESTS`：

```cmake
    native/test_picoui_native_screen_window.c
```

- [ ] **Step 3: RED**

Run:

```bash
rtk cmake --build build --target test_picoui_native_screen_window
```

Expected: FAIL，`picoui_window_create_root` 未定义。

- [ ] **Step 4: Header**

在 `picoui/include/picoui/window.h` forward declare `struct picoui_screen;` 并添加：

```c
struct picoui_window *picoui_window_create_root(struct picoui_screen *screen, const char *id);
```

- [ ] **Step 5: Implementation**

在 `picoui/src/widgets/window.c` 新增 root create wrapper。最小实现可以临时复用 compatibility app 直到 native widget tree 落地，但必须注释说明是 P1 compatibility shim：

```c
struct picoui_window *picoui_window_create_root(struct picoui_screen *screen, const char *id)
{
    struct picoui_app *compat_app;

    if (screen == 0 || id == 0) {
        return 0;
    }
    compat_app = picoui_app_create();
    if (compat_app == 0) {
        return 0;
    }
    return picoui_window_create(compat_app, id);
}
```

若这导致 lifecycle leak，P1-F 必须替换为 native root ownership；不得把该 shim 带到 P7。

- [ ] **Step 6: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_native_screen_window
rtk ctest --test-dir build -R test_picoui_native_screen_window --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 7: Commit**

```bash
git add picoui/include/picoui/window.h picoui/src/widgets/window.c tests/picoui/native/test_picoui_native_screen_window.c tests/picoui/CMakeLists.txt
git commit -m "feat: add screen root window entry"
```

### Task P1-F: basic_widgets LVGL-like main style contract

**Files:**
- Create: `tests/picoui/contract/check_picoui_demo_main_style.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/contract/picoui_demo_main_style_inventory.json`

- [ ] **Step 1: 写 RED contract**

新增 `tests/picoui/contract/check_picoui_demo_main_style.py`：

```python
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BASIC_WIDGETS = ROOT / "picoui/demo/basic_widgets/main.c"


def main() -> int:
    text = BASIC_WIDGETS.read_text(encoding="utf-8")
    required = [
        "static void create_demo_ui(",
        "static struct picoui_display *hal_init(",
        "picoui_init();",
        "hal_init(320, 480);",
        "create_demo_ui();",
        "while (1)",
        "picoui_timer_handler();",
    ]
    for marker in required:
        assert marker in text, f"basic_widgets main missing marker: {marker}"

    forbidden_main_path = [
        "picoui_app_create()",
        "picoui_app_run(",
        "picoui_app_destroy(",
    ]
    main_start = text.index("int main(")
    main_text = text[main_start:]
    for marker in forbidden_main_path:
        assert marker not in main_text, f"basic_widgets main path still uses {marker}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 contract**

加入 `tests/picoui/CMakeLists.txt`：

```cmake
ld_add_python_test(check_picoui_demo_main_style
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_main_style.py"
    LABELS "picoui;contract;native"
)
```

- [ ] **Step 3: RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_demo_main_style --output-on-failure
```

Expected: FAIL，`basic_widgets/main.c` 仍是 pre-v1 app style。

- [ ] **Step 4: Commit RED**

```bash
git add tests/picoui/CMakeLists.txt tests/picoui/contract/check_picoui_demo_main_style.py
git commit -m "test: require lvgl-like basic widgets main"
```

### Task P1-G: basic_widgets LVGL-like main implementation

**Files:**
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `tests/picoui/contract/picoui_demo_main_style_inventory.json`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: GitNexus impact**

Run:

```text
mcp__gitnexus.impact({repo:"LingDongGUI", target:"main", file_path:"picoui/demo/basic_widgets/main.c", direction:"upstream"})
mcp__gitnexus.impact({repo:"LingDongGUI", target:"run_demo", file_path:"picoui/demo/basic_widgets/main.c", direction:"upstream"})
```

Expected: LOW. If not LOW, stop and report.

- [ ] **Step 2: 修改 basic_widgets main 结构**

把 `picoui/demo/basic_widgets/main.c` 改成：

```c
#include "picoui/picoui.h"

#define PICOUI_BASIC_WIDGETS_WIDTH 320
#define PICOUI_BASIC_WIDGETS_HEIGHT 480

static struct picoui_window *g_root_window;

static struct picoui_display *hal_init(int width, int height)
{
    struct picoui_display *display = picoui_display_create(width, height);
    if (display == 0) {
        return 0;
    }
    if (picoui_display_set_default(display) != 0) {
        return 0;
    }
    return display;
}

/* keep existing callbacks */

static void create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win = picoui_window_create_root(screen, "root");

    if (win == 0) {
        return;
    }
    g_root_window = win;
    make_ui(win);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (picoui_init() != 0) {
        return 1;
    }
    if (hal_init(PICOUI_BASIC_WIDGETS_WIDTH, PICOUI_BASIC_WIDGETS_HEIGHT) == 0) {
        picoui_deinit();
        return 1;
    }

    create_demo_ui();
    if (g_root_window == 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        if (picoui_timer_handler() != 0) {
            picoui_deinit();
            return 1;
        }
    }

    return 0;
}
```

Important:

- Preserve existing `make_ui()` body and callbacks.
- Do not call `picoui_app_create()` / `picoui_app_run()` / `picoui_app_destroy()` in `main`.
- If infinite loop breaks runtime tests, add a P1-H runtime auto-quit hook; do not weaken the style contract.

- [ ] **Step 3: 更新 demo inventory**

In `picoui_demo_main_style_inventory.json`, update `basic_widgets` row:

```json
{"name": "basic_widgets", "main_style": "v1_lvgl_like", "uses_picoui_app": false, "migration_phase": "P1"}
```

- [ ] **Step 4: GREEN style**

Run:

```bash
rtk ctest --test-dir build -R 'check_picoui_demo_main_style|check_picoui_demo_main_style_inventory' --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 5: Build basic_widgets**

Run:

```bash
rtk cmake --build build --target picoui_basic_widgets_demo
```

Expected: build succeeds. If target not present in `build`, run runtime build:

```bash
rtk cmake -S . -B build/picoui-runtime -DUSE_DEMO=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build/picoui-runtime --target picoui_basic_widgets_demo
```

- [ ] **Step 6: 更新阶段文档**

在 `01-native垂直切片.md` 添加：

```markdown
## basic_widgets main 样板

状态：已迁移为 LVGL-like 入口结构。

入口形态：

- `picoui_init()`
- `hal_init(320, 480)`
- `create_demo_ui()`
- `while (1) picoui_timer_handler()`

限制：

- 当前仍可能通过 compatibility shim 到旧 backend；这不等于 native runtime 完成。
```

- [ ] **Step 7: Commit**

```bash
git add picoui/demo/basic_widgets/main.c \
        tests/picoui/contract/picoui_demo_main_style_inventory.json \
        docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "refactor: use lvgl-like basic widgets main"
```

### Task P1-H: Runtime auto-quit and capture compatibility

**Files:**
- Modify: `picoui/src/native/native_runtime.c`
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: Run current runtime**

Run:

```bash
rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets
```

Expected:

- If PASS, only update docs.
- If timeout/fail due infinite loop or missing runtime marker, continue steps.

- [ ] **Step 2: Add runtime loop exit API if needed**

If needed, add internal behavior to `picoui_timer_handler()`:

- Read `PICOUI_DEMO_AUTO_QUIT_MS`.
- Return `1` when elapsed time exceeds requested quit.
- Keep `0` for continue, `-1` for error.

Use portable tick source already available where possible. If no tick source exists, use SDL only in SDL port, not in core runtime.

- [ ] **Step 3: Adjust basic_widgets loop**

In `main`, handle return:

```c
while (1) {
    int rc = picoui_timer_handler();
    if (rc > 0) {
        break;
    }
    if (rc < 0) {
        picoui_deinit();
        return 1;
    }
}

picoui_deinit();
return 0;
```

- [ ] **Step 4: GREEN runtime**

Run:

```bash
rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets
rtk git diff --check
```

Expected: PASS or documented known limitation if native render not yet ready.

- [ ] **Step 5: Commit**

```bash
git add picoui/src/native/native_runtime.c picoui/demo/basic_widgets/main.c tests/picoui/runtime/check_picoui_runtime.py docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "fix: preserve basic widgets runtime automation"
```

### Task P1-I: Native label/button render smoke

**Files:**
- Create: `tests/picoui/native/test_picoui_native_render_window_label_button.c`
- Create: `picoui/src/native/native_render.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: RED render test**

Create a C test that:

- Calls `picoui_init()`.
- Creates display/screen/root window/label/button.
- Calls `picoui_timer_handler()`.
- Asserts handler returns `0` or `1`, not `-1`.

Do not assert pixel correctness yet; visible artifact comes P1-J.

- [ ] **Step 2: RED**

Run:

```bash
rtk cmake --build build --target test_picoui_native_render_window_label_button
```

Expected: FAIL until test registered/renderer present.

- [ ] **Step 3: Minimal render implementation**

Implement `picoui_native_render_once()` or equivalent private function in `native_render.c`.

Minimum behavior:

- Traverse active screen root tree if present.
- Return success without calling ldgui.
- Hook from `picoui_timer_handler()`.

- [ ] **Step 4: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_native_render_window_label_button
rtk ctest --test-dir build -R test_picoui_native_render_window_label_button --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/native/test_picoui_native_render_window_label_button.c picoui/src/native/native_render.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "feat: add native render smoke"
```

### Task P1-J: Native input button smoke

**Files:**
- Create: `tests/picoui/native/test_picoui_native_input_button.c`
- Create: `picoui/src/native/native_event.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`

- [ ] **Step 1: RED input test**

Write a test that:

- Creates root screen, button.
- Registers button click callback increments counter.
- Pushes pointer down/up inside button geometry.
- Calls `picoui_timer_handler()` after each input state.
- Expects counter increments once.

- [ ] **Step 2: RED**

Run:

```bash
rtk cmake --build build --target test_picoui_native_input_button
```

Expected: FAIL until native event path exists.

- [ ] **Step 3: Minimal event implementation**

Implement:

- Pointer read from default indev or input state.
- Hit-test active screen tree.
- Button press/release state.
- Click callback dispatch.

Keep scope to button only.

- [ ] **Step 4: GREEN**

Run:

```bash
rtk cmake --build build --target test_picoui_native_input_button
rtk ctest --test-dir build -R test_picoui_native_input_button --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/native/test_picoui_native_input_button.c picoui/src/native/native_event.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/01-native垂直切片.md
git commit -m "feat: add native button input smoke"
```

### Task P1-K: P1 closeout review

**Files:**
- Modify: `docs/picoui-serial/v1.0-native/01-native垂直切片.md`
- Modify: `docs/picoui-serial/v1.0-native/线计划索引.md`
- Modify: `tests/picoui/contract/picoui_native_migration_ledger.json`

- [ ] **Step 1: Run P1 verification**

Run:

```bash
rtk ctest --test-dir build -R 'test_picoui_native_runtime|test_picoui_native_display_indev|test_picoui_native_screen|test_picoui_native_screen_window|test_picoui_native_render_window_label_button|test_picoui_native_input_button|check_picoui_demo_main_style' --output-on-failure
rtk cmake --build build --target picoui_basic_widgets_demo
rtk git diff --check
```

- [ ] **Step 2: Update ledger**

Mark these entries `covered` with evidence:

- `P1-runtime-init`
- `P1-display-indev-screen`
- `P1-window-label-button`
- `P1-basic-widgets-main-style`

- [ ] **Step 3: GitNexus detect changes**

Main thread:

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"all"})
```

- [ ] **Step 4: Update docs**

Close `01-native垂直切片.md`; move serial current stage to `P2-A`.

- [ ] **Step 5: Commit**

```bash
git add docs/picoui-serial/v1.0-native/01-native垂直切片.md docs/picoui-serial/v1.0-native/线计划索引.md tests/picoui/contract/picoui_native_migration_ledger.json
git commit -m "docs: close picoui native vertical slice"
```

---
