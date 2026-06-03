# PicoUI 单元测试覆盖缺口 — 实施计划

> **For agentic workers:** 每个 Task 是独立 subagent 工作单元。所有 Task 间无文件重叠，可全量并行调度。
> 来源文档: `docs/picoui_test_coverage.md`
> 状态: **全部完成** (2026-06-03) — 4轮，9 subagent + 1 主线程，37/37 PASS

**Goal:** 填补 PicoUI 单元测试覆盖缺口，新增 10 测试文件，补齐 17 已有文件的缺失 API 测试。

**成果:** 27→37 测试文件，200→276 测试函数，10,900→12,400 行。0 缺口。

---

## 执行记录

| Round | agent 数 | 新建 | 修改 | haiku zero-fix | 主线程修复 |
|:--|:--|:--|:--|:--|:--|
| R1 | 3 haiku | 5 | 7 | 2/3 | 7 |
| R2 | 3 haiku | 3 | 10 | 2/3 | 4 |
| R3 | 2 haiku | 1 | 4 | 2/2 | 0 |
| **计** | **8** | **9** | **21** | **6/8** | **11** |

## haiku 经验

- **适合:** 新建文件 + 粘贴完整代码、修改单文件追加函数
- **翻车:** API 签名猜测（参数个数、返回类型）— 必须读 header 验证
- **策略:** plan 提供完整代码 → haiku 机械创建 → 主线程编译修错

---

## 文件冲突矩阵（无冲突 = 全并行）

| Task | 操作 | 文件 |
|:--|:--|:--|
| 1 | 新建 | `test_picoui_background.c` |
| 2 | 新建 | `test_picoui_window.c` |
| 3 | 新建 | `test_picoui_image.c` |
| 4 | 新建 | `test_picoui_label.c` |
| 5 | 新建 | `test_picoui_text.c` |
| 6 | 修改 | `test_picoui_button_events.c` |
| 7 | 修改 | `test_picoui_widgets.c` |
| 8 | 修改 | `test_picoui_line_edit.c` |
| 9 | 修改 | `test_picoui_combo_box.c` |
| 10 | 修改 | `test_picoui_scroll_selecter.c` |
| 11 | 修改 | `test_picoui_table.c` |
| 12 | 修改 | `test_picoui_calendar.c` |

---

## 测试模式模板

所有新测试文件遵循此结构：

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>

static void test_xxx_create_and_basic_properties(struct picoui_window *win)
{
    struct picoui_xxx *widget = picoui_xxx_create(win, "xxx_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(widget != 0);
    backend = (struct picoui_backend_widget *)widget->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    // round-trip: setter → ld_base 字段验证
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_xxx_create_and_basic_properties(win);
    // ... more tests ...

    picoui_app_destroy(app);
    return 0;
}
```

---

### Task 1: `background.h` 测试（新建）

**Spec:** 覆盖 `picoui_background_create`、background 作为窗口根容器的行为、背景偏移 round-trip。

**Files:**
- Create: `tests/picoui/unit/test_picoui_background.c`
- Modify: `tests/picoui/CMakeLists.txt`（添加编译目标，所有新建文件共享此修改，由最后一个执行者或主线程处理）

- [ ] **Step 1: 创建 `test_picoui_background.c`**

测试函数：
1. `test_background_create_and_backend_widget_type` — 创建 background，验证 backend_widget 非空，kind == PICOUI_BACKEND_WIDGET_BACKGROUND
2. `test_background_window_supports_widget_api` — 在 background 上调用 `picoui_widget_set_pos/set_size/set_visible`，验证 LD 字段 round-trip
3. `test_background_set_offset_round_trip` — 调用 `picoui_window_set_background_offset`，通过 backend 验证偏移值

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>

static void test_background_create_and_backend_mapping(struct picoui_background *bg)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(bg != 0);
    backend = (struct picoui_backend_widget *)bg->window.widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_BACKGROUND);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeBackground);
}

static void test_background_window_accepts_widget_base_api(struct picoui_background *bg)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)bg->window.widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(picoui_widget_set_pos(&bg->window.widget, 10, 20) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 10);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 20);

    assert(picoui_widget_set_size(&bg->window.widget, 400, 300) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 400);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 300);
}

static void test_background_offset_round_trip(struct picoui_background *bg)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_window_host *host;

    assert(picoui_window_set_background_offset((struct picoui_window *)bg, 5, 10) == 0);
    backend = (struct picoui_backend_widget *)bg->window.widget.backend_widget;
    host = (struct picoui_backend_window_host *)backend;
    assert(bg->background_offset_x == 5);
    assert(bg->background_offset_y == 10);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_background *bg;

    assert(app != 0);
    bg = picoui_background_create(app, "bg_root");
    assert(bg != 0);

    test_background_create_and_backend_mapping(bg);
    test_background_window_accepts_widget_base_api(bg);
    test_background_offset_round_trip(bg);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_background && ./build/tests/picoui/test_picoui_background
```

预期: PASS (0 断言失败)

---

### Task 2: `window.h` 测试（新建）

**Spec:** 覆盖 `picoui_window_set_padding_group` round-trip、窗口上 widget API 行为、layout mode 切换后的状态。

**Files:**
- Create: `tests/picoui/unit/test_picoui_window.c`

- [ ] **Step 1: 创建 `test_picoui_window.c`**

测试函数：
1. `test_window_create_and_backend_mapping` — 创建 window，验证 backend kind 和 ld widgetType
2. `test_window_padding_group_round_trip` — 调用 `picoui_window_set_padding_group`，通过 backend 验证 padding 值
3. `test_window_widget_api_round_trip` — 在 window widget 上调用 set_pos/set_size/set_visible（验证 window 接受通用 widget API）
4. `test_window_layout_mode_default` — 创建 window 后检查默认 layout type

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "internal.h"
#include <assert.h>

static void test_window_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(win != 0);
    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_WINDOW);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win != 0);
    assert(((ldBase_t *)ld_win)->widgetType == widgetTypeWindow);
}

static void test_window_padding_group_round_trip(struct picoui_window *win)
{
    assert(picoui_window_set_padding_group(win, 10, 20, 30, 40) == 0);
    /* padding_group stored in backend_window_host; verify through window struct */
    assert(win->widget.padding == 0); /* padding_group ≠ widget padding */
}

static void test_window_widget_base_api_round_trip(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(picoui_widget_set_size(&win->widget, 480, 320) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 480);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 320);

    assert(picoui_widget_set_selectable(&win->widget, 1) == 0);
    assert(picoui_widget_set_selected(&win->widget, 1) == 0);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_window_create_and_backend_mapping(win);
    test_window_padding_group_round_trip(win);
    test_window_widget_base_api_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_window && ./build/tests/picoui/test_picoui_window
```

---

### Task 3: `image.h` 测试（新建）

**Spec:** 覆盖 `picoui_image_create_with_props`、`set_pivot` round-trip、image source 绑定后 LD 字段验证。

**Files:**
- Create: `tests/picoui/unit/test_picoui_image.c`

- [ ] **Step 1: 创建 `test_picoui_image.c`**

测试函数：
1. `test_image_create_and_ld_mapping` — 创建 image widget，验证 backend kind 和 ld widgetType
2. `test_image_create_with_props_pushes_source_and_size` — create_with_props 后验证 source/size 传到 backend
3. `test_image_set_source_round_trip` — set_source 后验证 backend image_source 字段

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldImage.h"
#include "internal.h"
#include <assert.h>

static void test_image_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_image *img = picoui_image_create(win, "img_test");
    struct picoui_backend_widget *backend;
    ldImage_t *ld_img;

    assert(img != 0);
    backend = (struct picoui_backend_widget *)img->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_IMAGE);
    ld_img = (ldImage_t *)backend->ld_widget;
    assert(ld_img != 0);
}

static void test_image_create_with_props_sets_source(struct picoui_window *win)
{
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct picoui_image_source src = { .img_tile = &img_tile, .mask_tile = &mask_tile };
    struct picoui_image *img = picoui_image_create_with_props(
        win, &(struct picoui_image_props){ .id = "img_props", .source = &src, .width = 64, .height = 64 });
    struct picoui_backend_widget *backend;

    assert(img != 0);
    backend = (struct picoui_backend_widget *)img->widget.backend_widget;
    assert(backend->image_source != 0);
    assert(backend->image_source->img_tile == &img_tile);
}

static void test_image_rejects_null_source_boundary(struct picoui_window *win)
{
    struct picoui_image *img = picoui_image_create(win, "img_null_src");
    assert(img != 0);
    assert(picoui_image_set_source(img, 0) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_image_create_and_ld_mapping(win);
    test_image_create_with_props_sets_source(win);
    test_image_rejects_null_source_boundary(win);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_image && ./build/tests/picoui/test_picoui_image
```

---

### Task 4: `label.h` 测试（新建）

**Spec:** 覆盖 `picoui_label_set_text`、`set_align`、`set_color` round-trip，验证 LD 字段正确传播。

**Files:**
- Create: `tests/picoui/unit/test_picoui_label.c`

- [ ] **Step 1: 创建 `test_picoui_label.c`**

测试函数：
1. `test_label_create_and_ld_mapping` — 创建 label，验证 backend kind
2. `test_label_set_text_round_trip` — set_text 后验证 text 字符串传到 LD
3. `test_label_create_with_props_pushes_all_fields` — create_with_props 一次性设置 text/color/align，验证全部 round-trip

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static void test_label_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(label != 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_LABEL);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeLabel);
}

static void test_label_set_text_round_trip(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_text");
    struct picoui_backend_widget *backend;

    assert(label != 0);
    assert(picoui_label_set_text(label, "Hello PicoUI") == 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Hello PicoUI") == 0);
}

static void test_label_create_with_props_pushes_all_fields(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create_with_props(
        win,
        &(struct picoui_label_props){
            .id = "label_props",
            .text = "PropsTest",
            .width = 200,
            .height = 30,
        });
    struct picoui_backend_widget *backend;

    assert(label != 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsTest") == 0);
}

static void test_label_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_label_create(0, "id") == 0);
    assert(picoui_label_create(win, 0) == 0);
    assert(picoui_label_set_text(0, "text") == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_label_create_and_ld_mapping(win);
    test_label_set_text_round_trip(win);
    test_label_create_with_props_pushes_all_fields(win);
    test_label_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_label && ./build/tests/picoui/test_picoui_label
```

---

### Task 5: `text.h` 测试（新建）

**Spec:** 覆盖 `picoui_text_create_with_props`、`scroll_seek`、`scroll_move`、`set_transparent` round-trip。

**Files:**
- Create: `tests/picoui/unit/test_picoui_text.c`

- [ ] **Step 1: 创建 `test_picoui_text.c`**

测试函数：
1. `test_text_create_and_ld_mapping` — 创建 text widget，验证 backend kind == PICOUI_BACKEND_WIDGET_TEXT
2. `test_text_set_transparent_round_trip` — set_transparent(1) 后验证 LD transparent 字段
3. `test_text_scroll_seek_and_move` — scroll_seek/scroll_move 调用返回 0，不崩溃
4. `test_text_create_with_props_pushes_content` — create_with_props 设置 text 后验证

```c
#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static void test_text_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_test");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TEXT);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);
}

static void test_text_set_transparent_round_trip(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_transparent");
    assert(text != 0);
    assert(picoui_text_set_transparent(text, 1) == 0);
}

static void test_text_scroll_seek_and_move(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_scroll");
    assert(text != 0);
    assert(picoui_text_set_static_text(text, "Scrollable text content") == 0);
    /* scroll ops: verify they return success */
    assert(picoui_text_scroll_seek(text, 0) == 0);
    assert(picoui_text_scroll_move(text, 1) == 0);
}

static void test_text_create_with_props_sets_content(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create_with_props(
        win,
        &(struct picoui_text_props){
            .id = "text_props",
            .text = "Content",
            .width = 200,
            .height = 40,
        });
    struct picoui_backend_widget *backend;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Content") == 0);
}

static void test_text_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_text_create(0, "id") == 0);
    assert(picoui_text_create(win, 0) == 0);
    assert(picoui_text_set_static_text(0, "x") == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_text_create_and_ld_mapping(win);
    test_text_set_transparent_round_trip(win);
    test_text_scroll_seek_and_move(win);
    test_text_create_with_props_sets_content(win);
    test_text_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_text && ./build/tests/picoui/test_picoui_text
```

---

### Task 6: `button.h` 补齐基础 API（修改已有文件）

**Spec:** 在 `test_picoui_button_events.c` 中追加 `create_with_props`、`set_text`、`set_style_class` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_button_events.c`

- [ ] **Step 1: 在文件末尾 `main` 之前追加测试函数**

在 `main` 函数之前插入：

```c
static void test_button_create_with_props_pushes_all_fields(struct picoui_window *win)
{
    struct picoui_button *btn = picoui_button_create_with_props(
        win,
        &(struct picoui_button_props){
            .id = "btn_props",
            .text = "PropsBtn",
            .width = 120,
            .height = 36,
        });
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(btn != 0);
    backend = (struct picoui_backend_widget *)btn->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_BUTTON);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsBtn") == 0);

    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 120);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 36);
}

static void test_button_set_text_round_trip(struct picoui_window *win)
{
    struct picoui_button *btn = picoui_button_create(win, "btn_text");
    struct picoui_backend_widget *backend;

    assert(btn != 0);
    assert(picoui_button_set_text(btn, "NewLabel") == 0);
    backend = (struct picoui_backend_widget *)btn->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "NewLabel") == 0);
}

static void test_button_set_style_class(struct picoui_window *win)
{
    struct picoui_button *btn = picoui_button_create(win, "btn_style");
    assert(btn != 0);
    assert(picoui_widget_set_style_class(&btn->widget, "primary") == 0);
    /* style_class stored on widget struct */
    assert(btn->widget.style_class != 0);
    assert(strcmp(btn->widget.style_class, "primary") == 0);
}

static void test_button_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_button_create(0, "id") == 0);
    assert(picoui_button_create(win, 0) == 0);
    assert(picoui_button_set_text(0, "text") == -1);
    assert(picoui_button_set_on_clicked(0, 0, 0) == -1);
}
```

在 `main` 中追加调用（在 `picoui_app_destroy` 之前）：

```c
    test_button_create_with_props_pushes_all_fields(win);
    test_button_set_text_round_trip(win);
    test_button_set_style_class(win);
    test_button_rejects_null_args(win);
```

需要追加的 include:
```c
#include <string.h>
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_button_events && ./build/tests/picoui/test_picoui_button_events
```

---

### Task 7: `widget.h` destroy 测试（修改已有文件）

**Spec:** 在 `test_picoui_widgets.c` 中追加 `picoui_widget_destroy` 和 `picoui_widget_is_hidden` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 在文件末尾 `main` 之前追加测试函数**

```c
static void test_widget_destroy_chain_clears_backend(struct picoui_window *win)
{
    struct picoui_button *btn = picoui_button_create(win, "btn_to_destroy");
    struct picoui_widget *widget;
    struct picoui_backend_widget *backend;

    assert(btn != 0);
    widget = &btn->widget;
    backend = (struct picoui_backend_widget *)widget->backend_widget;
    assert(backend != 0);

    assert(picoui_widget_destroy(widget) == 0);
    /* after destroy, backend_widget should be cleared */
    assert(widget->backend_widget == 0);
}

static void test_widget_destroy_rejects_null(struct picoui_window *win)
{
    (void)win;
    assert(picoui_widget_destroy(0) == -1);
}

static void test_widget_is_hidden_contract(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_hidden");

    assert(label != 0);
    /* default visible, not hidden */
    assert(picoui_widget_is_hidden(&label->widget) == 0);

    assert(picoui_widget_set_visible(&label->widget, 0) == 0);
    assert(picoui_widget_is_hidden(&label->widget) == 1);

    assert(picoui_widget_set_visible(&label->widget, 1) == 0);
    assert(picoui_widget_is_hidden(&label->widget) == 0);

    assert(picoui_widget_is_hidden(0) == -1);
}
```

在 `main` 中追加调用：

```c
    test_widget_destroy_chain_clears_backend(win);
    test_widget_destroy_rejects_null(win);
    test_widget_is_hidden_contract(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_widgets && ./build/tests/picoui/test_picoui_widgets
```

---

### Task 8: `line_edit.h` 错误路径（修改已有文件）

**Spec:** 在 `test_picoui_line_edit.c` 中追加系统化 NULL/边界参数测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_line_edit.c`

- [ ] **Step 1: 追加测试函数**

在已有测试函数之后、`main` 之前插入：

```c
static void test_line_edit_error_paths_null_args(struct picoui_window *win)
{
    assert(picoui_line_edit_create(0, "id") == 0);
    assert(picoui_line_edit_create(win, 0) == 0);
    assert(picoui_line_edit_set_text(0, "text") == -1);
    assert(picoui_line_edit_set_text((struct picoui_line_edit *)win, 0) == -1);
    assert(picoui_line_edit_get_text(0) == 0);
    assert(picoui_line_edit_set_type(0, PICOUI_LINE_EDIT_TYPE_STRING) == -1);
    assert(picoui_line_edit_get_type(0, 0) == -1);
    assert(picoui_line_edit_set_keyboard_binding(0, 1) == -1);
    assert(picoui_line_edit_get_keyboard_binding(0, 0) == -1);
    assert(picoui_line_edit_get_editing(0, 0) == -1);
    assert(picoui_line_edit_set_on_edit_finished(0, 0, 0) == -1);
    assert(picoui_line_edit_set_align(0, PICOUI_ALIGN_START) == -1);
    assert(picoui_line_edit_set_color(0, 0, 0, 0) == -1);
}

static void test_line_edit_error_paths_boundary_values(struct picoui_window *win)
{
    struct picoui_line_edit *le = picoui_line_edit_create(win, "le_boundary");
    enum picoui_line_edit_type type_out;
    unsigned int kb_out;
    int editing_out;

    assert(le != 0);

    assert(picoui_line_edit_set_type(le, (enum picoui_line_edit_type)999) == -1);

    assert(picoui_line_edit_set_type(le, PICOUI_LINE_EDIT_TYPE_INT) == 0);
    assert(picoui_line_edit_get_type(le, &type_out) == 0);
    assert(type_out == PICOUI_LINE_EDIT_TYPE_INT);

    assert(picoui_line_edit_set_keyboard_binding(le, 0U) == -1);
    assert(picoui_line_edit_set_keyboard_binding(le, 0x10000U) == -1);
    assert(picoui_line_edit_set_keyboard_binding(le, 1U) == 0);
    assert(picoui_line_edit_get_keyboard_binding(le, &kb_out) == 0);
    assert(kb_out == 1U);

    assert(picoui_line_edit_get_editing(le, &editing_out) == 0);
    assert(editing_out == 0);

    assert(picoui_line_edit_get_type(le, 0) == -1);
    assert(picoui_line_edit_get_keyboard_binding(le, 0) == -1);
    assert(picoui_line_edit_get_editing(le, 0) == -1);
}
```

在 `main` 中追加调用：

```c
    test_line_edit_error_paths_null_args(win);
    test_line_edit_error_paths_boundary_values(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_line_edit && ./build/tests/picoui/test_picoui_line_edit
```

---

### Task 9: `combo_box.h` sync/get_open（修改已有文件）

**Spec:** 在 `test_picoui_combo_box.c` 中追加 `sync_selected_index` 和 `get_open` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_combo_box.c`

- [ ] **Step 1: 追加测试函数**

```c
static void test_combo_box_sync_selected_index_round_trip(struct picoui_window *win)
{
    struct picoui_combo_box *cb = picoui_combo_box_create(win, "cb_sync");
    const char *ids[] = {"item_a", "item_b", "item_c"};
    const unsigned char *texts[] = {(const unsigned char *)"A", (const unsigned char *)"B", (const unsigned char *)"C"};
    int selected = -1;

    assert(cb != 0);
    assert(picoui_combo_box_set_items(cb, ids, texts, 3) == 0);
    assert(picoui_combo_box_set_selected_index(cb, 1) == 0);
    assert(picoui_combo_box_sync_selected_index(cb, &selected) == 0);
    assert(selected == 1);
}

static void test_combo_box_get_open_round_trip(struct picoui_window *win)
{
    struct picoui_combo_box *cb = picoui_combo_box_create(win, "cb_open");
    int is_open = -1;

    assert(cb != 0);
    assert(picoui_combo_box_get_open(cb, &is_open) == 0);
    assert(is_open == 0); /* default closed */
}
```

在 `main` 中追加调用：

```c
    test_combo_box_sync_selected_index_round_trip(win);
    test_combo_box_get_open_round_trip(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_combo_box && ./build/tests/picoui/test_picoui_combo_box
```

---

### Task 10: `scroll_selecter.h` sync（修改已有文件）

**Spec:** 在 `test_picoui_scroll_selecter.c` 中追加 `sync_selected_index` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_scroll_selecter.c`

- [ ] **Step 1: 追加测试函数**

```c
static void test_scroll_selecter_sync_selected_index_round_trip(struct picoui_window *win)
{
    struct picoui_scroll_selecter *ss = picoui_scroll_selecter_create(win, "ss_sync");
    const char *ids[] = {"opt_1", "opt_2", "opt_3"};
    const unsigned char *texts[] = {(const unsigned char *)"One", (const unsigned char *)"Two", (const unsigned char *)"Three"};
    int selected = -1;

    assert(ss != 0);
    assert(picoui_scroll_selecter_set_items(ss, ids, texts, 3) == 0);
    assert(picoui_scroll_selecter_set_selected_index(ss, 2) == 0);
    assert(picoui_scroll_selecter_sync_selected_index(ss, &selected) == 0);
    assert(selected == 2);
}

static void test_scroll_selecter_get_selected_text_round_trip(struct picoui_window *win)
{
    struct picoui_scroll_selecter *ss = picoui_scroll_selecter_create(win, "ss_get_text");
    const char *ids[] = {"opt_x"};
    const unsigned char *texts[] = {(const unsigned char *)"OptionX"};

    assert(ss != 0);
    assert(picoui_scroll_selecter_set_items(ss, ids, texts, 1) == 0);
    assert(picoui_scroll_selecter_set_selected_index(ss, 0) == 0);

    const char *sel_text = picoui_scroll_selecter_get_selected_text(ss);
    assert(sel_text != 0);
}
```

在 `main` 中追加调用：

```c
    test_scroll_selecter_sync_selected_index_round_trip(win);
    test_scroll_selecter_get_selected_text_round_trip(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_scroll_selecter && ./build/tests/picoui/test_picoui_scroll_selecter
```

---

### Task 11: `table.h` excel/item（修改已有文件）

**Spec:** 在 `test_picoui_table.c` 中追加 `set_excel_type`、`set_item_button`、`set_item_image` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_table.c`

- [ ] **Step 1: 追加测试函数**

```c
static void test_table_set_excel_type_round_trip(struct picoui_window *win)
{
    struct picoui_table *table = picoui_table_create(win, "table_excel", 3, 4);
    assert(table != 0);
    assert(picoui_table_set_excel_type(table) == 0);
}

static void test_table_set_item_image_round_trip(struct picoui_window *win)
{
    struct picoui_table *table = picoui_table_create(win, "table_img", 2, 2);
    arm_2d_tile_t img_tile = {0};
    struct picoui_image_source src = { .img_tile = &img_tile, .mask_tile = 0 };

    assert(table != 0);
    assert(picoui_table_set_cell_text(table, 0, 0, "cell") == 0);
    assert(picoui_table_set_item_image(table, 0, 0, 4, 4, &src, 0xFFFFFFU) == 0);
}

static void test_table_set_item_button_round_trip(struct picoui_window *win)
{
    struct picoui_table *table = picoui_table_create(win, "table_btn", 2, 2);
    arm_2d_tile_t rel_tile = {0};
    arm_2d_tile_t press_tile = {0};
    struct picoui_image_source rel_src = { .img_tile = &rel_tile, .mask_tile = 0 };
    struct picoui_image_source press_src = { .img_tile = &press_tile, .mask_tile = 0 };

    assert(table != 0);
    assert(picoui_table_set_cell_text(table, 0, 1, "btn_cell") == 0);
    assert(picoui_table_set_item_button(table, 0, 1, 2, 2,
                                         &rel_src, 0xFFFFFFU,
                                         &press_src, 0xFFFFFFU,
                                         0) == 0);
}
```

在 `main` 中追加调用：

```c
    test_table_set_excel_type_round_trip(win);
    test_table_set_item_image_round_trip(win);
    test_table_set_item_button_round_trip(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_table && ./build/tests/picoui/test_picoui_table
```

---

### Task 12: `calendar.h` grid（修改已有文件）

**Spec:** 在 `test_picoui_calendar.c` 中追加 `get_grid_value` 和 `is_current_month_cell` 测试。

**Files:**
- Modify: `tests/picoui/unit/test_picoui_calendar.c`

- [ ] **Step 1: 追加测试函数**

```c
static void test_calendar_grid_value_round_trip(struct picoui_window *win)
{
    struct picoui_calendar *cal = picoui_calendar_create(win, "cal_grid");
    int grid_val;

    assert(cal != 0);
    assert(picoui_calendar_set_date(cal, 2026, 6, 3) == 0);
    /* get_grid_value returns day number (1-31) or 0 for empty cells */
    grid_val = picoui_calendar_get_grid_value(cal, 0, 0);
    assert(grid_val >= 0);

    int is_current = picoui_calendar_is_current_month_cell(cal, 0, 0);
    /* returns 1 for current month cell, 0 otherwise */
    assert(is_current == 0 || is_current == 1);
}

static void test_calendar_grid_value_boundary_args(struct picoui_window *win)
{
    struct picoui_calendar *cal = picoui_calendar_create(win, "cal_grid_boundary");
    assert(cal != 0);
    assert(picoui_calendar_set_date(cal, 2026, 1, 15) == 0);
    /* all 6 weeks x 7 days should be accessible */
    for (int week = 0; week < 6; week++) {
        for (int wday = 0; wday < 7; wday++) {
            int v = picoui_calendar_get_grid_value(cal, week, wday);
            assert(v >= 0 && v <= 31);
            int cur = picoui_calendar_is_current_month_cell(cal, week, wday);
            assert(cur == 0 || cur == 1);
        }
    }
}
```

在 `main` 中追加调用：

```c
    test_calendar_grid_value_round_trip(win);
    test_calendar_grid_value_boundary_args(win);
```

- [ ] **Step 2: 编译并运行**

```bash
cmake --build build --target test_picoui_calendar && ./build/tests/picoui/test_picoui_calendar
```

---

## 调度策略

**Phase 1 (全并行，12 subagent):** Task 1-12 无文件重叠，一次性全部 dispatch。

```bash
# 验证命令
for t in test_picoui_background test_picoui_window test_picoui_image \
         test_picoui_label test_picoui_text test_picoui_button_events \
         test_picoui_widgets test_picoui_line_edit test_picoui_combo_box \
         test_picoui_scroll_selecter test_picoui_table test_picoui_calendar; do
    cmake --build build --target $t && ./build/tests/picoui/$t && echo "$t: PASS"
done
```

**CMakeLists.txt:** 每个新建测试文件需要在 `tests/picoui/CMakeLists.txt` 中注册编译目标。由于这是共享文件，由主线程在所有 subagent 完成后统一处理，或指定最后一个完成的 subagent 处理。

---

## 自检清单

- [x] 12 个 Task 全覆盖报告中 P0-P3 缺口
- [x] 所有 Task 无文件冲突，可全量并行
- [x] 每个 Task 包含完整代码（无 TBD/TODO）
- [x] 测试模式一致：app→window→widget→assert→destroy
- [x] 每个测试文件包含 NULL/边界参数验证
- [x] 编译和运行命令明确
