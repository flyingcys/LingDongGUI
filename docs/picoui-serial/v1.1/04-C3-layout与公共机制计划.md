# PicoUI v1.1 C3 Layout 与公共机制实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 补齐 PicoUI v1.1 的公共机制，让后续控件迁移建立在稳定 `core` 之上，而不是每个控件各写一套 tree/layout/focus/dirty 逻辑。

**Architecture:** 本阶段聚焦四类公共能力：几何/dirty、flex/grid、focus/input dispatch、最小 theme/style。每条能力都先以 focused test 锁住行为，再把对应实现收口到 `core`，避免继续依赖旧 `native/backend` helper。

**Tech Stack:** C11, CMake, ARM-2D, SDL2, Python contract checks, `rtk cmake`, `rtk ctest`.

---

## Shared References

- 总计划：`docs/picoui-serial/v1.1/01-总实施计划.md`
- C2 记录：`docs/picoui-serial/v1.1/02-C2-阶段记录.md`
- 设计文档：`docs/picoui-serial/v1.1/00-重构设计.md`
- 现有 layout helper：`picoui/src/layout/flex.c`, `grid.c`
- 参考算法：`src/gui/ldWindow.c`, `src/gui/ldBase.c`

---

## Task C3-A: 几何与 dirty 公共契约

**Files:**
- Create: `tests/picoui/native/test_picoui_v1_1_geometry_dirty.c`
- Create: `picoui/src/core/dirty.c`
- Modify: `picoui/src/core/runtime_state.h`
- Modify: `picoui/src/core/core.c`
- Modify: `picoui/src/widgets/window.c`
- Modify: `picoui/src/widgets/label.c`
- Modify: `picoui/src/widgets/button.c`

- [ ] **Step 1: 写 RED geometry/dirty test**

新增 `tests/picoui/native/test_picoui_v1_1_geometry_dirty.c`：

```c
#include "picoui/picoui.h"
#include "picoui/window.h"
#include "picoui/label.h"

#include <assert.h>

int main(void)
{
    struct picoui_window *root;
    struct picoui_label *label;

    assert(picoui_init() == 0);
    root = picoui_window_create_root(picoui_screen_active());
    label = picoui_label_create(root, "title");
    assert(label != 0);

    assert(picoui_widget_set_pos(&label->widget, 10, 20) == 0);
    assert(picoui_widget_set_size(&label->widget, 80, 24) == 0);
    assert(picoui_widget_get_x(&label->widget) == 10);
    assert(picoui_widget_get_y(&label->widget) == 20);
    assert(picoui_widget_get_width(&label->widget) == 80);
    assert(picoui_widget_get_height(&label->widget) == 24);
    assert(picoui_widget_is_dirty(&label->widget) == 1);
    assert(picoui_timer_handler() == 0);
    assert(picoui_widget_is_dirty(&label->widget) == 0);

    picoui_deinit();
    return 0;
}
```

- [ ] **Step 2: 运行 RED**

```bash
rtk cmake --build build --target test_picoui_v1_1_geometry_dirty
```

Expected:

- FAIL，因为 `picoui_widget_set_pos/size` 或 dirty 状态未形成公共 truth。

- [ ] **Step 3: 实现公共几何 API**

在 `picoui/src/core/dirty.c` 实现：

```c
int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y)
{
    if (widget == 0) {
        return -1;
    }
    widget->x = x;
    widget->y = y;
    widget->dirty = 1;
    return 0;
}

int picoui_widget_set_size(struct picoui_widget *widget, int width, int height)
{
    if ((widget == 0) || (width < 0) || (height < 0)) {
        return -1;
    }
    widget->width = width;
    widget->height = height;
    widget->dirty = 1;
    return 0;
}
```

并补齐 getter / `picoui_widget_is_dirty()`。

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_v1_1_geometry_dirty
rtk ctest --test-dir build -R test_picoui_v1_1_geometry_dirty --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/native/test_picoui_v1_1_geometry_dirty.c \
  picoui/src/core/dirty.c \
  picoui/src/core/runtime_state.h \
  picoui/src/core/core.c \
  picoui/src/widgets/window.c \
  picoui/src/widgets/label.c \
  picoui/src/widgets/button.c
git commit -m "feat: add picoui v1.1 geometry dirty core"
```

## Task C3-B: flex 公共调度

**Files:**
- Create: `tests/picoui/native/test_picoui_v1_1_flex_layout.c`
- Modify: `picoui/src/core/layout.c`
- Modify: `picoui/src/layout/flex.c`
- Modify: `picoui/include/picoui/layout.h`
- Modify: `docs/picoui-serial/v1.1/03-C3-阶段记录.md`

- [ ] **Step 1: 写 RED flex test**

新增 `tests/picoui/native/test_picoui_v1_1_flex_layout.c`：

```c
#include "picoui/picoui.h"
#include "picoui/window.h"
#include "picoui/label.h"

#include <assert.h>

int main(void)
{
    struct picoui_window *root;
    struct picoui_label *a;
    struct picoui_label *b;
    struct picoui_label *c;

    assert(picoui_init() == 0);
    root = picoui_window_create_root(picoui_screen_active());
    assert(picoui_widget_set_size(&root->widget, 300, 100) == 0);
    assert(picoui_window_set_flex_flow(root, PICOUI_FLEX_FLOW_ROW) == 0);
    assert(picoui_window_set_flex_item_gap(root, 10) == 0);

    a = picoui_label_create(root, "a");
    b = picoui_label_create(root, "b");
    c = picoui_label_create(root, "c");
    assert(picoui_widget_set_size(&a->widget, 50, 20) == 0);
    assert(picoui_widget_set_size(&b->widget, 50, 20) == 0);
    assert(picoui_widget_set_size(&c->widget, 50, 20) == 0);

    assert(picoui_timer_handler() == 0);
    assert(picoui_widget_get_x(&a->widget) == 0);
    assert(picoui_widget_get_x(&b->widget) == 60);
    assert(picoui_widget_get_x(&c->widget) == 120);

    picoui_deinit();
    return 0;
}
```

- [ ] **Step 2: 运行 RED**

```bash
rtk cmake --build build --target test_picoui_v1_1_flex_layout
```

- [ ] **Step 3: 实现最小 row/column flex**

在 `picoui/src/core/layout.c` 中实现：

```c
static void picoui_layout_apply_row(struct picoui_window *window)
{
    int cursor = 0;
    struct picoui_widget *child = window->widget.first_child;

    while (child != 0) {
        child->x = cursor;
        child->y = 0;
        cursor += child->width + window->flex_item_gap;
        child = child->next_sibling;
    }
}
```

列布局先明确实现为：

```c
static void picoui_layout_apply_column(struct picoui_window *window)
{
    int cursor = 0;
    struct picoui_widget *child = window->widget.first_child;

    while (child != 0) {
        child->x = 0;
        child->y = cursor;
        cursor += child->height + window->flex_item_gap;
        child = child->next_sibling;
    }
}
```

其他复杂对齐先明确返回默认行为，不偷接旧 `ldWindow`。

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_v1_1_flex_layout
rtk ctest --test-dir build -R test_picoui_v1_1_flex_layout --output-on-failure
rtk rg -n "ldWindow|ldFlex|ldBase" picoui/src/core/layout.c picoui/src/layout/flex.c
rtk git diff --check
```

Expected:

- 测试 PASS
- `rg` 不应命中新依赖

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/native/test_picoui_v1_1_flex_layout.c \
  picoui/src/core/layout.c \
  picoui/src/layout/flex.c \
  picoui/include/picoui/layout.h \
  docs/picoui-serial/v1.1/03-C3-阶段记录.md
git commit -m "feat: add picoui v1.1 flex layout"
```

## Task C3-C: grid 公共调度

**Files:**
- Create: `tests/picoui/native/test_picoui_v1_1_grid_layout.c`
- Modify: `picoui/src/core/layout.c`
- Modify: `picoui/src/layout/grid.c`
- Modify: `picoui/include/picoui/layout.h`

- [ ] **Step 1: 写 RED grid test**

新增测试，创建 `320x240` root、2 列 2 行，断言 `(0,0)`, `(1,0)`, `(0..1,1)` 三个 child 的 region。

核心断言示例：

```c
assert(picoui_widget_get_x(&a->widget) == 0);
assert(picoui_widget_get_y(&a->widget) == 0);
assert(picoui_widget_get_x(&b->widget) == 160);
assert(picoui_widget_get_y(&b->widget) == 0);
assert(picoui_widget_get_width(&c->widget) == 320);
assert(picoui_widget_get_y(&c->widget) == 120);
```

- [ ] **Step 2: 运行 RED**

```bash
rtk cmake --build build --target test_picoui_v1_1_grid_layout
```

- [ ] **Step 3: 实现最小 grid**

在 `picoui/src/core/layout.c` 中增加最小 grid 计算：

```c
static void picoui_layout_apply_grid(struct picoui_window *window)
{
    int cell_w = window->widget.width / 2;
    int cell_h = window->widget.height / 2;
    /* 先只支持 fixed 2x2 与 span，后续复杂轨道在 C4/C5 再扩 */
}
```

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_v1_1_grid_layout
rtk ctest --test-dir build -R test_picoui_v1_1_grid_layout --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/native/test_picoui_v1_1_grid_layout.c \
  picoui/src/core/layout.c \
  picoui/src/layout/grid.c \
  picoui/include/picoui/layout.h
git commit -m "feat: add picoui v1.1 grid layout"
```

## Task C3-D: focus 与 input dispatch 最小集合

**Files:**
- Create: `tests/picoui/native/test_picoui_v1_1_focus_input.c`
- Modify: `picoui/src/core/input.c`
- Create: `picoui/src/core/focus.c`
- Modify: `picoui/include/picoui/widget.h`

- [ ] **Step 1: 写 RED focus/input test**

测试目标：

- button 可被设为 focus owner
- pointer press/release 可命中 button
- press/release 通过公共 dispatch 路径触发 clicked

最小断言示例：

```c
assert(picoui_focus_set(&button->widget) == 0);
assert(picoui_focus_get() == &button->widget);
assert(picoui_input_dispatch_pointer(5, 5, 1) == 0);
assert(picoui_input_dispatch_pointer(5, 5, 0) == 0);
assert(clicked_count == 1);
```

- [ ] **Step 2: 运行 RED**

```bash
rtk cmake --build build --target test_picoui_v1_1_focus_input
```

- [ ] **Step 3: 实现最小 focus/dispatch**

在 `picoui/src/core/focus.c` 中实现：

```c
static struct picoui_widget *g_focus_owner;

int picoui_focus_set(struct picoui_widget *widget)
{
    g_focus_owner = widget;
    return widget == 0 ? -1 : 0;
}

struct picoui_widget *picoui_focus_get(void)
{
    return g_focus_owner;
}
```

在 `picoui/src/core/input.c` 中先以 root child 简单命中实现 pointer dispatch，不回退到旧 `ldMsg/SIGNAL`。

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_v1_1_focus_input
rtk ctest --test-dir build -R test_picoui_v1_1_focus_input --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/native/test_picoui_v1_1_focus_input.c \
  picoui/src/core/input.c \
  picoui/src/core/focus.c \
  picoui/include/picoui/widget.h
git commit -m "feat: add picoui v1.1 focus input core"
```

## Task C3-E: 最小 theme/style 公共能力

**Files:**
- Create: `tests/picoui/native/test_picoui_v1_1_theme_style.c`
- Modify: `picoui/src/core/theme.c`
- Modify: `picoui/src/widgets/label.c`
- Modify: `picoui/src/widgets/button.c`

- [ ] **Step 1: 写 RED style test**

验证：

- 设置 text/bg/border 颜色后，widget 本地 state 更新
- `picoui_timer_handler()` 后 dirty 清空，但样式保持

最小断言示例：

```c
assert(picoui_widget_set_text_color(&label->widget, 0x112233) == 0);
assert(picoui_widget_get_text_color(&label->widget) == 0x112233);
assert(picoui_widget_set_bg_color(&button->widget, 0x445566) == 0);
assert(picoui_widget_get_bg_color(&button->widget) == 0x445566);
```

- [ ] **Step 2: 运行 RED**

```bash
rtk cmake --build build --target test_picoui_v1_1_theme_style
```

- [ ] **Step 3: 实现最小 style storage**

在 `picoui/src/core/theme.c` 中实现一组最小 setter/getter，直接写到 widget 本地字段，不调用旧 `backend_style_apply`。

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_v1_1_theme_style
rtk ctest --test-dir build -R test_picoui_v1_1_theme_style --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/native/test_picoui_v1_1_theme_style.c \
  picoui/src/core/theme.c \
  picoui/src/widgets/label.c \
  picoui/src/widgets/button.c
git commit -m "feat: add picoui v1.1 theme style core"
```

## Task C3-F: C3 closeout

**Files:**
- Modify: `docs/picoui-serial/v1.1/03-C3-阶段记录.md`
- Modify: `docs/picoui-serial/v1.1/线计划索引.md`

- [ ] **Step 1: 跑 C3 focused 验证集**

Run:

```bash
rtk ctest --test-dir build -R 'test_picoui_v1_1_geometry_dirty|test_picoui_v1_1_flex_layout|test_picoui_v1_1_grid_layout|test_picoui_v1_1_focus_input|test_picoui_v1_1_theme_style' --output-on-failure
rtk git diff --check
```

- [ ] **Step 2: 主线程跑 detect_changes**

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"unstaged"})
```

- [ ] **Step 3: 更新 closeout**

在 `docs/picoui-serial/v1.1/03-C3-阶段记录.md` 写：

```markdown
## Closeout

- 几何/dirty：PASS
- flex/grid：PASS
- focus/input：PASS
- theme/style 最小集合：PASS
- 下一阶段：`C4`
```

索引中把 `C3` 改为 `已完成`，`C4` 改为 `进行中`。

- [ ] **Step 4: Commit**

```bash
git add docs/picoui-serial/v1.1/03-C3-阶段记录.md docs/picoui-serial/v1.1/线计划索引.md
git commit -m "docs: close picoui v1.1 c3 phase"
```
