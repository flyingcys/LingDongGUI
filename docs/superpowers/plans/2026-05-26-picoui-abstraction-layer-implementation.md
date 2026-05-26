# PicoUI Abstraction Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在不大改 `LingDongGUI` 内核的前提下，落地第一阶段 `PicoUI` 抽象层，让用户能只通过 Linux 风格的 `picoui_*` API、统一 theme 与统一 demo 来使用基础控件和基础布局。

**Architecture:** `PicoUI` 作为 `LingDongGUI` 上层应用抽象，public API 只暴露 `picoui_*` 类型、函数与 props；`picoui/src/backend/ldgui/` 负责把 public 语义映射到 `LingDongGUI` 现有控件、布局和事件系统。第一阶段只覆盖基础控件、`flex/grid`、`theme v0` 和统一 demo，并通过 public-header 泄漏检查、demo 边界检查、基础 C 单测和 SDL runtime smoke 守住边界。

**Tech Stack:** C11、CMake（`examples/sdl/CMakeLists.txt`）、LingDongGUI、ARM-2D（仅 backend 内部可见）、Python3 测试脚本、CTest

---

## 文件结构与责任划分

### 新建目录与文件

- Create: `picoui/include/picoui/picoui.h`
- Create: `picoui/include/picoui/app.h`
- Create: `picoui/include/picoui/widget.h`
- Create: `picoui/include/picoui/theme.h`
- Create: `picoui/include/picoui/layout.h`
- Create: `picoui/include/picoui/window.h`
- Create: `picoui/include/picoui/label.h`
- Create: `picoui/include/picoui/text.h`
- Create: `picoui/include/picoui/image.h`
- Create: `picoui/include/picoui/button.h`
- Create: `picoui/include/picoui/checkbox.h`
- Create: `picoui/include/picoui/switch.h`
- Create: `picoui/include/picoui/slider.h`
- Create: `picoui/src/core/internal.h`
- Create: `picoui/src/core/app.c`
- Create: `picoui/src/core/widget.c`
- Create: `picoui/src/core/event.c`
- Create: `picoui/src/core/resource.c`
- Create: `picoui/src/theme/theme.c`
- Create: `picoui/src/layout/flex.c`
- Create: `picoui/src/layout/grid.c`
- Create: `picoui/src/widgets/window.c`
- Create: `picoui/src/widgets/label.c`
- Create: `picoui/src/widgets/text.c`
- Create: `picoui/src/widgets/image.c`
- Create: `picoui/src/widgets/button.c`
- Create: `picoui/src/widgets/checkbox.c`
- Create: `picoui/src/widgets/switch.c`
- Create: `picoui/src/widgets/slider.c`
- Create: `picoui/src/backend/ldgui/backend.h`
- Create: `picoui/src/backend/ldgui/backend_widget.c`
- Create: `picoui/src/backend/ldgui/backend_theme.c`
- Create: `picoui/src/backend/ldgui/backend_layout.c`
- Create: `picoui/src/backend/ldgui/backend_event.c`
- Create: `picoui/src/backend/ldgui/backend_window.c`
- Create: `picoui/src/backend/ldgui/backend_label.c`
- Create: `picoui/src/backend/ldgui/backend_text.c`
- Create: `picoui/src/backend/ldgui/backend_image.c`
- Create: `picoui/src/backend/ldgui/backend_button.c`
- Create: `picoui/src/backend/ldgui/backend_checkbox.c`
- Create: `picoui/src/backend/ldgui/backend_switch.c`
- Create: `picoui/src/backend/ldgui/backend_slider.c`
- Create: `picoui/demo/hello_world/main.c`
- Create: `picoui/demo/basic_widgets/main.c`
- Create: `picoui/demo/layout_flex/main.c`
- Create: `picoui/demo/layout_grid/main.c`
- Create: `picoui/demo/theme_showcase/main.c`
- Create: `picoui/demo/settings_panel/main.c`
- Create: `picoui/docs/quick_start.md`
- Create: `picoui/docs/api_overview.md`
- Create: `picoui/docs/demo_guide.md`
- Create: `examples/sdl/tests/picoui/test_picoui_smoke.c`
- Create: `examples/sdl/tests/picoui/test_picoui_theme.c`
- Create: `examples/sdl/tests/picoui/test_picoui_widgets.c`
- Create: `examples/sdl/tests/picoui/test_picoui_layout.c`
- Create: `examples/sdl/tests/check_picoui_public_api.py`
- Create: `examples/sdl/tests/check_picoui_demo_boundary.py`
- Create: `examples/sdl/tests/check_picoui_runtime.py`

### 需要修改的现有文件

- Modify: `examples/sdl/CMakeLists.txt`
- Modify: `README.md`
- Modify: `docs/tutorial/02 get started.md`
- Modify: `docs/tutorial/04 api.md`

### 责任约束

- `picoui/include/picoui/*.h`：只放 PicoUI public contract，禁止泄漏 `ld*`、`arm_2d_*`
- `picoui/src/backend/ldgui/*.c`：唯一允许直接碰 `LingDongGUI` / `ARM-2D` 的适配层
- `picoui/demo/*`：只允许使用 `picoui_*` API
- `examples/sdl/tests/check_picoui_*.py`：守 public API 与 demo 边界
- `examples/sdl/tests/picoui/*.c`：守最小 C 行为合同

---

### Task 1: 搭 PicoUI 骨架、public contract 与 CMake 接线

**Files:**
- Create: `picoui/include/picoui/picoui.h`
- Create: `picoui/include/picoui/app.h`
- Create: `picoui/include/picoui/widget.h`
- Create: `picoui/include/picoui/theme.h`
- Create: `picoui/include/picoui/layout.h`
- Create: `picoui/src/core/internal.h`
- Create: `picoui/src/core/app.c`
- Create: `picoui/src/core/widget.c`
- Create: `picoui/src/core/event.c`
- Create: `picoui/src/core/resource.c`
- Create: `examples/sdl/tests/picoui/test_picoui_smoke.c`
- Create: `examples/sdl/tests/check_picoui_public_api.py`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写 public API 泄漏检查与最小 smoke test**

```python
# examples/sdl/tests/check_picoui_public_api.py
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "picoui" / "include" / "picoui"
FORBIDDEN = ["ld", "arm_2d_", "arm_2d_tile_t", "arm_2d_font_t", "SIGNAL_"]


def main() -> int:
    headers = sorted(PUBLIC_DIR.glob("*.h"))
    assert headers, "expected PicoUI public headers to exist"
    for header in headers:
        text = header.read_text(encoding="utf-8")
        for needle in FORBIDDEN:
            assert needle not in text, f"{header.name} leaks forbidden token: {needle}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

```c
/* examples/sdl/tests/picoui/test_picoui_smoke.c */
#include "picoui/picoui.h"
#include <assert.h>

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != NULL);

    struct picoui_window *win = picoui_window_create(app, "root_window");
    assert(win != NULL);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 运行检查，确认现在失败**

Run:

```bash
python3 examples/sdl/tests/check_picoui_public_api.py
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m1 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m1 --target picoui_smoke_test
```

Expected:

- Python 脚本失败：`expected PicoUI public headers to exist`
- CMake 构建失败：找不到 `picoui/picoui.h`、`picoui_app_create` 或 `picoui_window_create`

- [ ] **Step 3: 写最小骨架与 CMake 接线**

```c
/* picoui/include/picoui/app.h */
#ifndef PICOUI_APP_H
#define PICOUI_APP_H

struct picoui_app;
struct picoui_window;

struct picoui_app *picoui_app_create(void);
void picoui_app_destroy(struct picoui_app *app);

#endif
```

```c
/* picoui/include/picoui/widget.h */
#ifndef PICOUI_WIDGET_H
#define PICOUI_WIDGET_H

struct picoui_widget;

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y);
int picoui_widget_set_size(struct picoui_widget *widget, int width, int height);
int picoui_widget_set_visible(struct picoui_widget *widget, int visible);
int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled);

#endif
```

```c
/* picoui/include/picoui/picoui.h */
#ifndef PICOUI_PICOUI_H
#define PICOUI_PICOUI_H

#include "picoui/app.h"
#include "picoui/widget.h"
#include "picoui/theme.h"
#include "picoui/layout.h"
#include "picoui/window.h"

#endif
```

```c
/* picoui/src/core/internal.h */
#ifndef PICOUI_INTERNAL_H
#define PICOUI_INTERNAL_H

struct picoui_app {
    void *backend_app;
};

struct picoui_widget {
    void *backend_widget;
};

#endif
```

```c
/* picoui/src/core/app.c */
#include "picoui/app.h"
#include "internal.h"
#include <stdlib.h>

struct picoui_app *picoui_app_create(void)
{
    return calloc(1, sizeof(struct picoui_app));
}

void picoui_app_destroy(struct picoui_app *app)
{
    free(app);
}
```

```c
/* picoui/include/picoui/window.h */
#ifndef PICOUI_WINDOW_H
#define PICOUI_WINDOW_H

struct picoui_app;
struct picoui_window;

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id);

#endif
```

```c
/* picoui/src/widgets/window.c */
#include "picoui/window.h"
#include <stdlib.h>

struct picoui_window {
    void *backend_widget;
};

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id)
{
    (void)app;
    (void)id;
    return calloc(1, sizeof(struct picoui_window));
}
```

```cmake
# examples/sdl/CMakeLists.txt 追加
file(GLOB PICOUI_CORE_SOURCES CONFIGURE_DEPENDS
    "${SDL_EXAMPLE_DIR}/../../picoui/src/core/*.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/widgets/window.c"
)

add_executable(picoui_smoke_test
    "${SDL_EXAMPLE_DIR}/tests/picoui/test_picoui_smoke.c"
    ${PICOUI_CORE_SOURCES}
)

target_include_directories(picoui_smoke_test PRIVATE
    "${SDL_EXAMPLE_DIR}/../../picoui/include"
)

add_test(NAME picoui_smoke_test COMMAND picoui_smoke_test)
add_test(NAME check_picoui_public_api COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_picoui_public_api.py")
set_tests_properties(check_picoui_public_api PROPERTIES WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}")
```

- [ ] **Step 4: 跑测试，确认骨架成立**

Run:

```bash
python3 examples/sdl/tests/check_picoui_public_api.py
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m1 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m1 --target picoui_smoke_test
rtk ctest --test-dir examples/sdl/build-picoui-m1 -R 'picoui_smoke_test|check_picoui_public_api' --output-on-failure
```

Expected:

- `check_picoui_public_api` PASS
- `picoui_smoke_test` PASS

- [ ] **Step 5: Commit**

```bash
git add picoui/include/picoui/picoui.h \
        picoui/include/picoui/app.h \
        picoui/include/picoui/widget.h \
        picoui/include/picoui/theme.h \
        picoui/include/picoui/layout.h \
        picoui/include/picoui/window.h \
        picoui/src/core/internal.h \
        picoui/src/core/app.c \
        picoui/src/core/widget.c \
        picoui/src/core/event.c \
        picoui/src/core/resource.c \
        picoui/src/widgets/window.c \
        examples/sdl/tests/picoui/test_picoui_smoke.c \
        examples/sdl/tests/check_picoui_public_api.py \
        examples/sdl/CMakeLists.txt

git commit -m "feat(picoui): add public api skeleton"
```

### Task 2: 落地 theme v0、资源句柄与 window/label/button 最小桥接

**Files:**
- Create: `picoui/include/picoui/label.h`
- Create: `picoui/include/picoui/button.h`
- Create: `picoui/src/theme/theme.c`
- Create: `picoui/src/widgets/label.c`
- Create: `picoui/src/widgets/button.c`
- Create: `picoui/src/backend/ldgui/backend.h`
- Create: `picoui/src/backend/ldgui/backend_theme.c`
- Create: `picoui/src/backend/ldgui/backend_window.c`
- Create: `picoui/src/backend/ldgui/backend_label.c`
- Create: `picoui/src/backend/ldgui/backend_button.c`
- Create: `examples/sdl/tests/picoui/test_picoui_theme.c`
- Create: `picoui/demo/hello_world/main.c`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写失败的 theme / button / label 合同测试**

```c
/* examples/sdl/tests/picoui/test_picoui_theme.c */
#include "picoui/picoui.h"
#include <assert.h>

int main(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    assert(theme != NULL);

    assert(picoui_theme_set_color(theme, PICOUI_COLOR_ACCENT, 0x112233) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_RADIUS, 6) == 0);

    struct picoui_app *app = picoui_app_create();
    assert(app != NULL);
    assert(picoui_app_set_theme(app, theme) == 0);

    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_button *button = picoui_button_create(win, "ok");
    assert(label != NULL);
    assert(button != NULL);

    assert(picoui_label_set_text(label, "hello") == 0);
    assert(picoui_button_set_text(button, "OK") == 0);

    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}
```

- [ ] **Step 2: 运行，确认缺少 theme / label / button API**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m1 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m1 --target picoui_theme_test
```

Expected:

- 编译失败：缺少 `picoui_theme_create`、`picoui_label_create`、`picoui_button_create`

- [ ] **Step 3: 写最小实现与 backend 映射骨架**

```c
/* picoui/include/picoui/theme.h */
#ifndef PICOUI_THEME_H
#define PICOUI_THEME_H

enum picoui_color_id {
    PICOUI_COLOR_TEXT_PRIMARY,
    PICOUI_COLOR_BG,
    PICOUI_COLOR_PANEL,
    PICOUI_COLOR_BORDER,
    PICOUI_COLOR_ACCENT,
    PICOUI_COLOR_DISABLED,
};

enum picoui_metric_id {
    PICOUI_METRIC_PADDING,
    PICOUI_METRIC_RADIUS,
    PICOUI_METRIC_BORDER_WIDTH,
    PICOUI_METRIC_CONTROL_HEIGHT,
};

struct picoui_theme;

struct picoui_theme *picoui_theme_create(void);
void picoui_theme_destroy(struct picoui_theme *theme);
int picoui_theme_set_color(struct picoui_theme *theme, enum picoui_color_id id, unsigned int rgb);
int picoui_theme_set_metric(struct picoui_theme *theme, enum picoui_metric_id id, int value);
int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme);

#endif
```

```c
/* picoui/include/picoui/label.h */
#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

struct picoui_window;
struct picoui_label;

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);
int picoui_label_set_text(struct picoui_label *label, const char *text);

#endif
```

```c
/* picoui/include/picoui/button.h */
#ifndef PICOUI_BUTTON_H
#define PICOUI_BUTTON_H

struct picoui_window;
struct picoui_button;

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id);
int picoui_button_set_text(struct picoui_button *button, const char *text);

#endif
```

```c
/* picoui/src/backend/ldgui/backend.h */
#ifndef PICOUI_BACKEND_LDGUI_H
#define PICOUI_BACKEND_LDGUI_H

struct picoui_app;
struct picoui_widget;
struct picoui_theme;

int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme);
void *picoui_backend_create_window(struct picoui_app *app, const char *id);
void *picoui_backend_create_label(void *parent, const char *id);
void *picoui_backend_create_button(void *parent, const char *id);
int picoui_backend_set_text(void *backend_widget, const char *text);

#endif
```

```c
/* picoui/demo/hello_world/main.c */
#include "picoui/picoui.h"

static void build_demo(struct picoui_app *app)
{
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_button *button = picoui_button_create(win, "ok");

    picoui_label_set_text(label, "Hello PicoUI");
    picoui_button_set_text(button, "OK");
}
```

```cmake
# examples/sdl/CMakeLists.txt 追加
add_executable(picoui_theme_test
    "${SDL_EXAMPLE_DIR}/tests/picoui/test_picoui_theme.c"
    ${PICOUI_CORE_SOURCES}
    "${SDL_EXAMPLE_DIR}/../../picoui/src/theme/theme.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/widgets/label.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/widgets/button.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/backend/ldgui/backend_theme.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/backend/ldgui/backend_window.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/backend/ldgui/backend_label.c"
    "${SDL_EXAMPLE_DIR}/../../picoui/src/backend/ldgui/backend_button.c"
)
add_test(NAME picoui_theme_test COMMAND picoui_theme_test)
```

- [ ] **Step 4: 跑 theme / hello_world 验证**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m1 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m1 --target picoui_theme_test
rtk ctest --test-dir examples/sdl/build-picoui-m1 -R 'picoui_theme_test' --output-on-failure
```

Expected:

- `picoui_theme_test` PASS
- `hello_world` demo 至少能编译链接（若单独做 target，也应 PASS）

- [ ] **Step 5: Commit**

```bash
git add picoui/include/picoui/theme.h \
        picoui/include/picoui/label.h \
        picoui/include/picoui/button.h \
        picoui/src/theme/theme.c \
        picoui/src/widgets/label.c \
        picoui/src/widgets/button.c \
        picoui/src/backend/ldgui/backend.h \
        picoui/src/backend/ldgui/backend_theme.c \
        picoui/src/backend/ldgui/backend_window.c \
        picoui/src/backend/ldgui/backend_label.c \
        picoui/src/backend/ldgui/backend_button.c \
        picoui/demo/hello_world/main.c \
        examples/sdl/tests/picoui/test_picoui_theme.c \
        examples/sdl/CMakeLists.txt

git commit -m "feat(picoui): add theme and basic widget bridge"
```

### Task 3: 落地 checkbox/switch/slider 与统一事件桥接

**Files:**
- Create: `picoui/include/picoui/checkbox.h`
- Create: `picoui/include/picoui/switch.h`
- Create: `picoui/include/picoui/slider.h`
- Create: `picoui/src/widgets/checkbox.c`
- Create: `picoui/src/widgets/switch.c`
- Create: `picoui/src/widgets/slider.c`
- Create: `picoui/src/backend/ldgui/backend_event.c`
- Create: `picoui/src/backend/ldgui/backend_checkbox.c`
- Create: `picoui/src/backend/ldgui/backend_switch.c`
- Create: `picoui/src/backend/ldgui/backend_slider.c`
- Create: `examples/sdl/tests/picoui/test_picoui_widgets.c`
- Create: `picoui/demo/basic_widgets/main.c`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写失败的事件/值变化合同测试**

```c
/* examples/sdl/tests/picoui/test_picoui_widgets.c */
#include "picoui/picoui.h"
#include <assert.h>

static int toggled_value = -1;
static int slider_value = -1;

static void on_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    toggled_value = value;
}

static void on_slider(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    slider_value = value;
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");

    assert(sw && cb && slider);
    assert(picoui_switch_set_on_toggled(sw, on_toggle, NULL) == 0);
    assert(picoui_checkbox_set_on_toggled(cb, on_toggle, NULL) == 0);
    assert(picoui_slider_set_on_value_changed(slider, on_slider, NULL) == 0);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(picoui_slider_set_value(slider, 42) == 0);
    assert(picoui_slider_get_value(slider) == 42);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 运行，确认 switch/checkbox/slider API 缺失**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m2 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m2 --target picoui_widgets_test
```

Expected:

- 编译失败：缺少 `picoui_switch_create`、`picoui_checkbox_create`、`picoui_slider_create` 与统一 callback 接口

- [ ] **Step 3: 实现控件桥与统一事件回调适配**

```c
/* picoui/include/picoui/switch.h */
#ifndef PICOUI_SWITCH_H
#define PICOUI_SWITCH_H

struct picoui_window;
struct picoui_switch;
struct picoui_widget;

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id);
int picoui_switch_set_checked(struct picoui_switch *sw, int checked);
int picoui_switch_is_checked(struct picoui_switch *sw);
int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data);

#endif
```

```c
/* picoui/include/picoui/checkbox.h */
#ifndef PICOUI_CHECKBOX_H
#define PICOUI_CHECKBOX_H

struct picoui_window;
struct picoui_checkbox;
struct picoui_widget;

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id);
int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked);
int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox);
int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data);

#endif
```

```c
/* picoui/include/picoui/slider.h */
#ifndef PICOUI_SLIDER_H
#define PICOUI_SLIDER_H

struct picoui_window;
struct picoui_slider;
struct picoui_widget;

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id);
int picoui_slider_set_value(struct picoui_slider *slider, int value);
int picoui_slider_get_value(struct picoui_slider *slider);
int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value);
int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data);

#endif
```

```c
/* picoui/demo/basic_widgets/main.c */
static void on_wifi_changed(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void build_demo(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");

    picoui_switch_set_on_toggled(sw, on_wifi_changed, NULL);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, NULL);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, NULL);
}
```

- [ ] **Step 4: 跑 widgets 合同测试**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m2 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m2 --target picoui_widgets_test
rtk ctest --test-dir examples/sdl/build-picoui-m2 -R 'picoui_widgets_test' --output-on-failure
```

Expected:

- `picoui_widgets_test` PASS

- [ ] **Step 5: Commit**

```bash
git add picoui/include/picoui/checkbox.h \
        picoui/include/picoui/switch.h \
        picoui/include/picoui/slider.h \
        picoui/src/widgets/checkbox.c \
        picoui/src/widgets/switch.c \
        picoui/src/widgets/slider.c \
        picoui/src/backend/ldgui/backend_event.c \
        picoui/src/backend/ldgui/backend_checkbox.c \
        picoui/src/backend/ldgui/backend_switch.c \
        picoui/src/backend/ldgui/backend_slider.c \
        picoui/demo/basic_widgets/main.c \
        examples/sdl/tests/picoui/test_picoui_widgets.c \
        examples/sdl/CMakeLists.txt

git commit -m "feat(picoui): add interactive widgets and events"
```

### Task 4: 落地 flex/grid Public API 与布局测试

**Files:**
- Create: `picoui/src/layout/flex.c`
- Create: `picoui/src/layout/grid.c`
- Create: `picoui/src/backend/ldgui/backend_layout.c`
- Create: `examples/sdl/tests/picoui/test_picoui_layout.c`
- Create: `picoui/demo/layout_flex/main.c`
- Create: `picoui/demo/layout_grid/main.c`
- Modify: `picoui/include/picoui/layout.h`
- Modify: `picoui/include/picoui/widget.h`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写失败的 flex/grid 合同测试**

```c
/* examples/sdl/tests/picoui/test_picoui_layout.c */
#include "picoui/picoui.h"
#include <assert.h>

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    struct picoui_button *b = picoui_button_create(win, "b");

    assert(picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(picoui_flex_set_align(win,
                                 PICOUI_ALIGN_START,
                                 PICOUI_ALIGN_CENTER,
                                 PICOUI_ALIGN_SPACE_BETWEEN) == 0);
    assert(picoui_flex_set_gap(win, 8, 12) == 0);
    assert(picoui_widget_set_flex_grow((struct picoui_widget *)a, 1) == 0);

    assert(picoui_grid_set_columns(win, (int[]){80, -1, 0}, 3) == 0);
    assert(picoui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)b,
                                       1, 0, 1, 1,
                                       PICOUI_ALIGN_CENTER,
                                       PICOUI_ALIGN_CENTER) == 0);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 运行，确认布局 API 缺失**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m3 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m3 --target picoui_layout_test
```

Expected:

- 编译失败：缺少 `picoui_flex_set_flow`、`picoui_grid_set_columns` 等布局接口

- [ ] **Step 3: 实现布局 API 与 backend 映射**

```c
/* picoui/include/picoui/layout.h */
#ifndef PICOUI_LAYOUT_H
#define PICOUI_LAYOUT_H

enum picoui_align {
    PICOUI_ALIGN_START,
    PICOUI_ALIGN_CENTER,
    PICOUI_ALIGN_END,
    PICOUI_ALIGN_STRETCH,
    PICOUI_ALIGN_SPACE_EVENLY,
    PICOUI_ALIGN_SPACE_AROUND,
    PICOUI_ALIGN_SPACE_BETWEEN,
};

enum picoui_flex_flow {
    PICOUI_FLEX_FLOW_ROW,
    PICOUI_FLEX_FLOW_COLUMN,
    PICOUI_FLEX_FLOW_ROW_WRAP,
    PICOUI_FLEX_FLOW_COLUMN_WRAP,
    PICOUI_FLEX_FLOW_ROW_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_REVERSE,
    PICOUI_FLEX_FLOW_ROW_WRAP_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_WRAP_REVERSE,
};

struct picoui_window;

int picoui_flex_set_flow(struct picoui_window *window, enum picoui_flex_flow flow);
int picoui_flex_set_align(struct picoui_window *window,
                          enum picoui_align main_align,
                          enum picoui_align cross_align,
                          enum picoui_align track_align);
int picoui_flex_set_gap(struct picoui_window *window, int item_gap, int track_gap);

int picoui_grid_set_columns(struct picoui_window *window, const int *tracks, int count);
int picoui_grid_set_rows(struct picoui_window *window, const int *tracks, int count);
int picoui_grid_set_gap(struct picoui_window *window, int row_gap, int col_gap);
int picoui_grid_set_align(struct picoui_window *window,
                          enum picoui_align col_align,
                          enum picoui_align row_align);

#endif
```

```c
/* picoui/include/picoui/widget.h 追加 */
int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow);
int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);
int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);
int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align);
```

```c
/* picoui/demo/layout_flex/main.c */
static void build_demo(struct picoui_window *win)
{
    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP);
    picoui_flex_set_align(win,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_SPACE_AROUND);
    picoui_flex_set_gap(win, 8, 12);
}
```

```c
/* picoui/demo/layout_grid/main.c */
static const int cols[] = {80, -2, 0};
static const int rows[] = {32, -2, 0};

static void build_demo(struct picoui_window *win)
{
    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);
}
```

- [ ] **Step 4: 跑布局合同测试**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-m3 -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-m3 --target picoui_layout_test
rtk ctest --test-dir examples/sdl/build-picoui-m3 -R 'picoui_layout_test' --output-on-failure
```

Expected:

- `picoui_layout_test` PASS

- [ ] **Step 5: Commit**

```bash
git add picoui/include/picoui/layout.h \
        picoui/include/picoui/widget.h \
        picoui/src/layout/flex.c \
        picoui/src/layout/grid.c \
        picoui/src/backend/ldgui/backend_layout.c \
        picoui/demo/layout_flex/main.c \
        picoui/demo/layout_grid/main.c \
        examples/sdl/tests/picoui/test_picoui_layout.c \
        examples/sdl/CMakeLists.txt

git commit -m "feat(picoui): add flex and grid api"
```

### Task 5: 加 props struct 双入口、text/image、demo 边界检查与 runtime smoke

**Files:**
- Create: `picoui/include/picoui/text.h`
- Create: `picoui/include/picoui/image.h`
- Create: `picoui/src/widgets/text.c`
- Create: `picoui/src/widgets/image.c`
- Create: `picoui/src/backend/ldgui/backend_text.c`
- Create: `picoui/src/backend/ldgui/backend_image.c`
- Create: `examples/sdl/tests/check_picoui_demo_boundary.py`
- Create: `examples/sdl/tests/check_picoui_runtime.py`
- Create: `picoui/demo/theme_showcase/main.c`
- Create: `picoui/demo/settings_panel/main.c`
- Modify: `picoui/include/picoui/button.h`
- Modify: `picoui/include/picoui/checkbox.h`
- Modify: `picoui/include/picoui/switch.h`
- Modify: `picoui/include/picoui/slider.h`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写失败的 demo 边界检查与 runtime smoke**

```python
# examples/sdl/tests/check_picoui_demo_boundary.py
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui" / "demo"
FORBIDDEN = ["ld", "arm_2d_", "SIGNAL_"]


def main() -> int:
    demo_sources = sorted(DEMO_DIR.glob("**/*.c"))
    assert demo_sources, "expected PicoUI demo sources"
    for source in demo_sources:
        text = source.read_text(encoding="utf-8")
        for needle in FORBIDDEN:
            assert needle not in text, f"{source.name} leaks forbidden token: {needle}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

```python
# examples/sdl/tests/check_picoui_runtime.py
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build-picoui-runtime"

subprocess.run([
    "rtk", "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"
], check=True)
subprocess.run([
    "rtk", "cmake", "--build", str(BUILD), "--target", "picoui_settings_panel_demo"
], check=True)
```

- [ ] **Step 2: 运行，确认 demo 尚不存在或泄漏检查失败**

Run:

```bash
python3 examples/sdl/tests/check_picoui_demo_boundary.py
python3 examples/sdl/tests/check_picoui_runtime.py
```

Expected:

- 失败：缺少 demo 源码或缺少 demo target

- [ ] **Step 3: 实现 props 双入口与完整 demo 组**

```c
/* picoui/include/picoui/button.h 追加 */
struct picoui_button_props {
    const char *id;
    const char *text;
    int width;
    int height;
    picoui_event_cb on_clicked;
    void *user_data;
};

struct picoui_button *picoui_button_create_with_props(
    struct picoui_window *parent,
    const struct picoui_button_props *props);
```

```c
/* picoui/include/picoui/text.h */
#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

struct picoui_window;
struct picoui_text;

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id);
int picoui_text_set_text(struct picoui_text *text, const char *value);

#endif
```

```c
/* picoui/include/picoui/image.h */
#ifndef PICOUI_IMAGE_H
#define PICOUI_IMAGE_H

struct picoui_window;
struct picoui_image;
struct picoui_image_source;

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id);
int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

#endif
```

```c
/* picoui/demo/settings_panel/main.c */
static void build_demo(struct picoui_window *win)
{
    struct picoui_button_props apply_props = {
        .id = "apply",
        .text = "Apply",
        .width = 96,
        .height = 36,
    };

    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_COLUMN);
    picoui_flex_set_gap(win, 12, 12);

    struct picoui_label *title = picoui_label_create(win, "title");
    struct picoui_switch *wifi = picoui_switch_create(win, "wifi");
    struct picoui_slider *brightness = picoui_slider_create(win, "brightness");
    struct picoui_button *apply = picoui_button_create_with_props(win, &apply_props);

    picoui_label_set_text(title, "Settings");
    picoui_switch_set_checked(wifi, 1);
    picoui_slider_set_value(brightness, 75);
    (void)apply;
}
```

```cmake
# examples/sdl/CMakeLists.txt 追加
add_test(NAME check_picoui_demo_boundary COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_picoui_demo_boundary.py")
set_tests_properties(check_picoui_demo_boundary PROPERTIES WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}")

add_test(NAME check_picoui_runtime COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_picoui_runtime.py")
set_tests_properties(check_picoui_runtime PROPERTIES WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}")
```

- [ ] **Step 4: 跑边界检查和 runtime smoke**

Run:

```bash
python3 examples/sdl/tests/check_picoui_demo_boundary.py
rtk cmake -S examples/sdl -B examples/sdl/build-picoui-runtime -DUSE_DEMO=0
rtk cmake --build examples/sdl/build-picoui-runtime --target picoui_settings_panel_demo
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-picoui-runtime/picoui_settings_panel_demo
rtk ctest --test-dir examples/sdl/build-picoui-runtime -R 'check_picoui_demo_boundary|check_picoui_runtime' --output-on-failure
```

Expected:

- `check_picoui_demo_boundary` PASS
- `check_picoui_runtime` PASS
- `picoui_settings_panel_demo` 在 dummy SDL 下可启动

- [ ] **Step 5: Commit**

```bash
git add picoui/include/picoui/text.h \
        picoui/include/picoui/image.h \
        picoui/include/picoui/button.h \
        picoui/include/picoui/checkbox.h \
        picoui/include/picoui/switch.h \
        picoui/include/picoui/slider.h \
        picoui/src/widgets/text.c \
        picoui/src/widgets/image.c \
        picoui/src/backend/ldgui/backend_text.c \
        picoui/src/backend/ldgui/backend_image.c \
        picoui/demo/theme_showcase/main.c \
        picoui/demo/settings_panel/main.c \
        examples/sdl/tests/check_picoui_demo_boundary.py \
        examples/sdl/tests/check_picoui_runtime.py \
        examples/sdl/CMakeLists.txt

git commit -m "feat(picoui): add props api and demo suite"
```

### Task 6: 回写用户文档与仓库入口

**Files:**
- Create: `picoui/docs/quick_start.md`
- Create: `picoui/docs/api_overview.md`
- Create: `picoui/docs/demo_guide.md`
- Modify: `README.md`
- Modify: `docs/tutorial/02 get started.md`
- Modify: `docs/tutorial/04 api.md`

- [ ] **Step 1: 写失败的文档边界检查**

```python
# 可直接复用 check_picoui_public_api.py / check_picoui_demo_boundary.py
# 这里不新增脚本，直接把文档更新作为缺失项暴露在 review 中
```

先写出文档必须包含的最小片段：

```markdown
## PicoUI 快速开始

```c
struct picoui_app *app = picoui_app_create();
struct picoui_window *win = picoui_window_create(app, "root");
struct picoui_label *label = picoui_label_create(win, "title");
picoui_label_set_text(label, "Hello PicoUI");
```
```

- [ ] **Step 2: 运行检查，确认当前 README/tutorial 还没有 PicoUI 入口**

Run:

```bash
rtk rg -n "PicoUI|picoui_" README.md docs/tutorial
```

Expected:

- 在更新前，PicoUI 入口为空或不完整

- [ ] **Step 3: 写最小用户文档与入口回写**

```markdown
<!-- README.md 追加 -->
## PicoUI

PicoUI 是构建在 LingDongGUI 之上的应用层抽象，提供统一的 Linux 风格 `picoui_*` API。

- 用户不需要直接使用 `ld*`
- 用户不需要直接使用 `ARM-2D`
- 推荐从 `picoui/demo/hello_world` 开始
```

```markdown
<!-- picoui/docs/quick_start.md -->
# PicoUI 快速开始

1. 创建 `app`
2. 创建 `window`
3. 创建基础控件
4. 设置布局与 theme
5. 绑定事件
6. 运行 demo
```

```markdown
<!-- docs/tutorial/04 api.md 追加 -->
## PicoUI API

第一阶段 PicoUI 提供：

- `window`
- `label`
- `text`
- `image`
- `button`
- `checkbox`
- `switch`
- `slider`
- `flex`
- `grid`
```

- [ ] **Step 4: 复跑核心测试，确认文档更新没有破坏构建链**

Run:

```bash
python3 examples/sdl/tests/check_picoui_public_api.py
python3 examples/sdl/tests/check_picoui_demo_boundary.py
rtk ctest --test-dir examples/sdl/build-picoui-runtime --output-on-failure
```

Expected:

- 所有 PicoUI 边界与 smoke 测试继续 PASS

- [ ] **Step 5: Commit**

```bash
git add picoui/docs/quick_start.md \
        picoui/docs/api_overview.md \
        picoui/docs/demo_guide.md \
        README.md \
        docs/tutorial/02\ get\ started.md \
        docs/tutorial/04\ api.md

git commit -m "docs(picoui): add quick start and api guide"
```

---

## 自检

### Spec coverage

- `PicoUI` 作为上层抽象：Task 1、Task 2、Task 3、Task 4 完成 public API 与 backend 分层
- Linux 风格命名/函数规则：Task 1 public header 骨架与 `check_picoui_public_api.py`
- 用户不关心 `LingDongGUI` / `ARM-2D`：Task 1、Task 5 的 public/demo 边界检查
- 基础控件范围：Task 2、Task 3、Task 5
- `flex/grid`：Task 4
- `theme v0`：Task 2
- 统一 demo：Task 2、Task 3、Task 4、Task 5
- 用户文档入口：Task 6

### Placeholder scan

- 已清除占位词与延期标记
- 每个代码步骤都给了明确文件与最小代码片段
- 每个验证步骤都给了明确命令与预期

### Type consistency

- public 对外统一使用 `struct picoui_*`
- 事件签名统一为 `picoui_event_cb` / `picoui_value_changed_cb`
- `create + set` 与 `create_with_props` 形态在各任务中保持一致

---

Plan complete and saved to `docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`. Two execution options:

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
