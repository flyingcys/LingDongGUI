# TinyUI Legacy demo0 Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增一个纯 `tinyui_*` public API 的单页 demo，对齐 `LingDongGUI` `USE_DEMO=0` 的 `uiWidgetLegacy.c` 单页全控件页面，并编译出可对比的 `ldgui_sdl_demo` legacy 版本。

**Architecture:** 保留现有 `legacy_widget_parity` 作为历史抽样页，新增一个严格对齐 `uiWidgetLegacy.c` 的独立 TinyUI demo。先用 contract 测试锁定 demo 注册和真值字段，再实现新 demo 的绝对坐标布局与关键交互，最后补 runtime 可见性验证并实际构建 `tinyui_demo` 与 `ldgui_sdl_demo(USE_DEMO=0)`。

**Tech Stack:** C11, CMake, TinyUI public API, LingDongGUI SDL backend, Python contract/runtime tests

## Global Constraints

- 新 demo 只能使用 `tinyui_*` public API，禁止在 demo 源码中调用 `ld*`、`SIGNAL_*`、`arm_2d_*`
- `uiWidgetLegacy.c` 是本次 parity 的唯一布局、文案、默认值和交互真值
- SDL 窗口允许变大，但控件坐标与尺寸不缩放、不重排
- 不修改 `LingDongGUI` legacy demo 的既有行为和布局
- 任何生产符号修改前必须先做 GitNexus `impact(... direction:\"upstream\")`
- 所有实现遵循 TDD：先写失败测试，再写最小实现

---

### Task 1: 锁定新 demo 边界与注册契约

**Files:**
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Modify: `tinyui/demo/tinyui_demos.h`
- Modify: `tinyui/demo/tinyui_demos.c`
- Modify: `examples/sdl/CMakeLists.txt`
- Create: `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.h`
- Create: `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`

**Interfaces:**
- Consumes: `bool tinyui_demos_create(char *info[], int size);`, `void tinyui_demos_show_help(void);`
- Produces: `void tinyui_demo_legacy_demo0_parity(void);` and runtime name `legacy_demo0_parity`

- [ ] **Step 1: 写失败测试，要求新 demo 进入 TinyUI demo 边界**

```python
REQUIRED_DEMOS = {
    # ...
    "legacy_demo0_parity",
}

DEMO_MARKERS = {
    "legacy_demo0_parity": (
        "tinyui_image_create",
        "tinyui_button_create",
        "tinyui_checkbox_create",
        "tinyui_switch_create",
        "tinyui_progress_bar_create",
        "tinyui_text_create",
        "tinyui_slider_create",
        "tinyui_list_create",
        "tinyui_combo_box_create",
        "tinyui_calendar_create",
        "tinyui_scroll_selecter_create",
        "tinyui_date_time_create",
        "tinyui_message_box_create",
        "tinyui_graph_create",
        "tinyui_table_create",
        "tinyui_radial_menu_create",
        "tinyui_icon_slider_create",
        "tinyui_qrcode_create",
        "tinyui_gauge_create",
        "tinyui_line_edit_create_with_props",
        "tinyui_keyboard_create_with_props",
        "tinyui_arc_create_with_props",
        "tinyui_window_create_child",
        "tinyui_widget_set_opacity",
    ),
}
```

- [ ] **Step 2: 跑 contract 测试，确认当前失败**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: FAIL，提示 `missing TinyUI demos: legacy_demo0_parity` 或缺少 `legacy_demo0_parity` 源文件 / CMake 注册。

- [ ] **Step 3: 用最小改动注册新 demo 入口**

```c
/* tinyui/demo/tinyui_demos.h */
void tinyui_demo_legacy_demo0_parity(void);
```

```c
/* tinyui/demo/tinyui_demos.c */
static const demo_entry_info_t demos_entry_info[] = {
    { "animation_basic",       tinyui_demo_animation_basic       },
    /* ... */
    { "legacy_demo0_parity",   tinyui_demo_legacy_demo0_parity   },
    { "legacy_widget_parity",  tinyui_demo_legacy_widget_parity  },
    /* ... */
};
```

```cmake
# examples/sdl/CMakeLists.txt
add_tinyui_demo(tinyui_demo
    "${TINYUI_DEMO_DIR}/tinyui_demos.c"
    # ...
    "${TINYUI_DEMO_DIR}/legacy_demo0_parity/legacy_demo0_parity.c"
    "${TINYUI_DEMO_DIR}/legacy_widget_parity/legacy_widget_parity.c"
    # ...
)
```

```c
/* tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.h */
#ifndef TINYUI_DEMO_LEGACY_DEMO0_PARITY_H
#define TINYUI_DEMO_LEGACY_DEMO0_PARITY_H

void tinyui_demo_legacy_demo0_parity(void);

#endif
```

```c
/* tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c */
#include "legacy_demo0_parity/legacy_demo0_parity.h"
#include "tinyui.h"

void tinyui_demo_legacy_demo0_parity(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    if (screen == 0) {
        return;
    }
    tinyui_screen_load(screen);
}
```

- [ ] **Step 4: 重新跑 contract 测试，确认边界已变绿**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: PASS，至少不再报 `legacy_demo0_parity` 缺失。

- [ ] **Step 5: 提交这一阶段**

```bash
git add \
  tests/tinyui/contract/check_tinyui_demo_boundary.py \
  tinyui/demo/tinyui_demos.h \
  tinyui/demo/tinyui_demos.c \
  examples/sdl/CMakeLists.txt \
  tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.h \
  tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c
git commit -m "feat: register tinyui legacy demo0 parity demo"
```

### Task 2: 用失败测试锁定 legacy demo0 真值字段

**Files:**
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Modify: `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`

**Interfaces:**
- Consumes: `void tinyui_demo_legacy_demo0_parity(void);`
- Produces: 具备 legacy demo0 关键真值标记的单页 demo 源文件

- [ ] **Step 1: 写失败测试，锁定关键真值字段**

```python
DEMO_MARKERS["legacy_demo0_parity"] = (
    "tinyui_button_set_text(button, \"123\")",
    "tinyui_widget_set_pos((struct tinyui_widget *)button, 10, 10)",
    "tinyui_widget_set_pos((struct tinyui_widget *)text, 300, 10)",
    "tinyui_widget_set_pos((struct tinyui_widget *)sw, 300, 226)",
    "tinyui_widget_set_pos((struct tinyui_widget *)calendar, 50, 340)",
    "tinyui_widget_set_pos((struct tinyui_widget *)qrcode, 500, 10)",
    "tinyui_widget_set_pos((struct tinyui_widget *)list, 850, 280)",
    "tinyui_message_box_set_title(message_box, \"title\")",
    "tinyui_message_box_set_message(message_box, \"12345678abcdefg\\n99556\")",
    "tinyui_qrcode_set_text(qrcode, \"ldgui\")",
    "tinyui_calendar_set_date(calendar, 2026, 1, 1)",
    "tinyui_calendar_set_header_format(calendar, \"yyyy - mm - dd\")",
    "tinyui_widget_set_opacity((struct tinyui_widget *)image, 128)",
)
```

- [ ] **Step 2: 跑 contract 测试，确认这些真值当前失败**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: FAIL，提示 `legacy_demo0_parity missing marker`。

- [ ] **Step 3: 按 legacy demo0 真值填充 demo 骨架**

```c
static const char *const s_combo_ids[] = {"11", "22", "00"};
static const char *const s_combo_texts[] = {"11", "22", "00"};
static const char *const s_scroll_ids[] = {"1", "10", "123", "99", "7"};
static const char *const s_scroll_texts[] = {"1", "10", "123", "99", "7"};
static const char *const s_message_buttons[] = {"11", "22", "33"};
static const char *const s_day_names[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fir", "Sat"};

struct legacy_demo0_runtime {
    struct tinyui_image *image;
    struct tinyui_label *switch_label;
    struct tinyui_gauge *gauge;
    struct tinyui_arc *arc;
    int image_dimmed;
    float angle;
};
```

```c
if (button != 0) {
    tinyui_button_set_text(button, "123");
    tinyui_widget_set_pos((struct tinyui_widget *)button, 10, 10);
    tinyui_widget_set_size((struct tinyui_widget *)button, 79, 53);
}

if (sw != 0) {
    tinyui_switch_set_checked(sw, 0);
    tinyui_widget_set_pos((struct tinyui_widget *)sw, 300, 226);
    tinyui_widget_set_size((struct tinyui_widget *)sw, 48, 24);
}

if (switch_label != 0) {
    tinyui_label_set_text(switch_label, "OFF");
    tinyui_widget_set_pos((struct tinyui_widget *)switch_label, 356, 218);
    tinyui_widget_set_size((struct tinyui_widget *)switch_label, 60, 40);
}
```

```c
if (calendar != 0) {
    tinyui_calendar_set_date(calendar, 2026, 1, 1);
    tinyui_calendar_set_day_names(calendar, s_day_names);
    tinyui_calendar_set_header_format(calendar, "yyyy - mm - dd");
    tinyui_widget_set_pos((struct tinyui_widget *)calendar, 50, 340);
    tinyui_widget_set_size((struct tinyui_widget *)calendar, 300, 150);
}
```

```c
if (message_box != 0) {
    tinyui_message_box_set_title(message_box, "title");
    tinyui_message_box_set_message(message_box, "12345678abcdefg\n99556");
    tinyui_message_box_set_buttons(message_box, s_message_buttons, 3);
}
```

- [ ] **Step 4: 再跑 contract 测试，确认真值字段通过**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: PASS，`legacy_demo0_parity` 不再缺少真值标记。

- [ ] **Step 5: 提交这一阶段**

```bash
git add \
  tests/tinyui/contract/check_tinyui_demo_boundary.py \
  tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c
git commit -m "feat: scaffold tinyui legacy demo0 parity layout"
```

### Task 3: 实现单页全控件布局与关键交互

**Files:**
- Modify: `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`
- Test: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**Interfaces:**
- Consumes: `tinyui_widget_set_pos`, `tinyui_widget_set_size`, `tinyui_widget_set_opacity`, `tinyui_table_set_keyboard_binding`, `tinyui_line_edit_set_keyboard_binding`, `tinyui_arc_set_rotation_angle`, `tinyui_gauge_set_angle`
- Produces: 完整单页 demo0 parity UI 与运行时交互

- [ ] **Step 1: 写失败测试，锁定关键交互接线**

```python
DEMO_MARKERS["legacy_demo0_parity"] += (
    "tinyui_button_set_on_clicked(button, on_button_clicked, runtime)",
    "tinyui_switch_set_on_toggled(sw, on_switch_changed, runtime)",
    "runtime->image = image",
    "runtime->switch_label = switch_label",
    "runtime->gauge = gauge",
    "runtime->arc = arc",
    "tinyui_table_set_keyboard_binding(table, 22)",
    "tinyui_line_edit_set_keyboard_binding(line_edit, 22)",
    "tinyui_list_set_item_widget(list, 1, (struct tinyui_widget *)list_item_button)",
)
```

- [ ] **Step 2: 跑 contract 测试，确认交互接线当前失败**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: FAIL，提示缺失 clicked/toggled/runtime/keyboard 相关标记。

- [ ] **Step 3: 写最小实现，补齐控件实例与回调**

```c
static void on_button_clicked(struct tinyui_widget *widget, void *user_data)
{
    struct legacy_demo0_runtime *runtime = (struct legacy_demo0_runtime *)user_data;
    (void)widget;
    if (runtime == 0 || runtime->image == 0) {
        return;
    }
    runtime->image_dimmed = runtime->image_dimmed == 0 ? 1 : 0;
    tinyui_widget_set_opacity((struct tinyui_widget *)runtime->image,
                              runtime->image_dimmed != 0 ? 128 : 255);
}

static void on_switch_changed(struct tinyui_widget *widget, int value, void *user_data)
{
    struct legacy_demo0_runtime *runtime = (struct legacy_demo0_runtime *)user_data;
    (void)widget;
    if (runtime == 0 || runtime->switch_label == 0) {
        return;
    }
    tinyui_label_set_text(runtime->switch_label, value != 0 ? "ON" : "OFF");
}
```

```c
if (table != 0) {
    tinyui_table_set_keyboard_binding(table, 22);
    tinyui_widget_set_pos((struct tinyui_widget *)table, 780, 150);
    tinyui_widget_set_size((struct tinyui_widget *)table, 200, 100);
}

if (line_edit != 0) {
    tinyui_line_edit_set_keyboard_binding(line_edit, 22);
    tinyui_widget_set_pos((struct tinyui_widget *)line_edit, 850, 400);
    tinyui_widget_set_size((struct tinyui_widget *)line_edit, 100, 50);
}
```

```c
if (button != 0) {
    tinyui_button_set_on_clicked(button, on_button_clicked, runtime);
}
if (sw != 0) {
    tinyui_switch_set_on_toggled(sw, on_switch_changed, runtime);
}
if (runtime != 0) {
    runtime->image = image;
    runtime->switch_label = switch_label;
    runtime->gauge = gauge;
    runtime->arc = arc;
    runtime->angle = 0.0f;
}
```

- [ ] **Step 4: 实现运行时动画 tick**

```c
static int tick_runtime(struct legacy_demo0_runtime *runtime)
{
    if (runtime == 0) {
        return 0;
    }
    runtime->angle += 6.0f;
    if (runtime->angle >= 360.0f) {
        runtime->angle -= 360.0f;
    }
    if (runtime->gauge != 0) {
        tinyui_gauge_set_angle(runtime->gauge, runtime->angle);
    }
    if (runtime->arc != 0) {
        tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle);
    }
    return 0;
}
```

```c
void tinyui_demo_legacy_demo0_parity(void)
{
    static struct legacy_demo0_runtime runtime;
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) {
        return;
    }
    memset(&runtime, 0, sizeof(runtime));
    make_ui(win, &runtime);
    tinyui_screen_load(screen);
    (void)tick_runtime(&runtime);
}
```

- [ ] **Step 5: 跑 contract 测试，确认单页布局和关键交互接线通过**

Run: `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`  
Expected: PASS。

- [ ] **Step 6: 提交这一阶段**

```bash
git add \
  tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c \
  tests/tinyui/contract/check_tinyui_demo_boundary.py
git commit -m "feat: implement tinyui legacy demo0 parity interactions"
```

### Task 4: 补运行时可见性验证并构建对比目标

**Files:**
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Modify: `examples/sdl/tests/check_use_demo_0_legacy_widget.py`

**Interfaces:**
- Consumes: `DEMOS` runtime map, `tinyui_demo` binary, `ldgui_sdl_demo` CMake target
- Produces: 新 demo 的运行时检查入口，以及 legacy demo0 / tinyui parity 的可构建对比方式

- [ ] **Step 1: 写失败测试，把新 demo 纳入 runtime 映射**

```python
DEMOS = {
    # ...
    "legacy_demo0_parity": "legacy_demo0_parity",
}
```

```python
elif demo == "legacy_demo0_parity":
    _assert_legacy_demo0_parity_visible(capture_path)
```

- [ ] **Step 2: 跑 runtime 脚本最小用例，确认当前失败**

Run: `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo legacy_demo0_parity --build-dir /tmp/tinyui-demo0-runtime`  
Expected: FAIL，提示 demo 未注册或缺少可见性断言。

- [ ] **Step 3: 为新 demo 增加最小可见性断言**

```python
def _assert_legacy_demo0_parity_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    assert width >= 1024, f"VISIBLE FAIL: legacy_demo0_parity width too small: {width}"
    assert height >= 600, f"VISIBLE FAIL: legacy_demo0_parity height too small: {height}"
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    assert bounds is not None, "VISIBLE FAIL: legacy_demo0_parity capture is blank"
    left, top, right, bottom = bounds
    assert left <= 12 and top <= 12, (
        f"VISIBLE FAIL: legacy_demo0_parity top-left widgets missing: {(left, top, right, bottom)}"
    )
    assert right >= 940 and bottom >= 520, (
        f"VISIBLE FAIL: legacy_demo0_parity right/bottom widget clusters missing: {(left, top, right, bottom)}"
    )
```

- [ ] **Step 4: 构建并验证两个可执行目标**

Run:

```bash
rtk cmake -S . -B build-demo0 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DUSE_DEMO=0
rtk cmake --build build-demo0 --target ldgui_sdl_demo tinyui_demo
```

Expected:

- `build-demo0/examples/sdl/ldgui_sdl_demo` 生成成功
- `build-demo0/examples/sdl/tinyui_demo` 生成成功

- [ ] **Step 5: 运行关键验证**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 examples/sdl/tests/check_use_demo_0_legacy_widget.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo legacy_demo0_parity --build-dir build-demo0/tinyui-runtime
env SDL_VIDEODRIVER=dummy ./build-demo0/examples/sdl/tinyui_demo legacy_demo0_parity
env SDL_VIDEODRIVER=dummy ./build-demo0/examples/sdl/ldgui_sdl_demo
```

Expected:

- contract 检查通过
- legacy demo0 contract 检查通过
- 新 TinyUI demo 的 runtime 可见性检查通过
- 两个二进制都能启动，不因缺字体或资源映射失败直接退出

- [ ] **Step 6: 运行 GitNexus 变更审计并提交**

Run:

```bash
gitnexus detect_changes --repo LingDongGUI --scope all
```

Then:

```bash
git add \
  tests/tinyui/runtime/check_tinyui_visible_ui.py \
  tests/tinyui/contract/check_tinyui_demo_boundary.py \
  examples/sdl/tests/check_use_demo_0_legacy_widget.py
git commit -m "test: verify tinyui legacy demo0 parity runtime"
```
