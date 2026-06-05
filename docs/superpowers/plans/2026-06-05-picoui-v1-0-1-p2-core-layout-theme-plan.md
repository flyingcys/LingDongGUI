# PicoUI v1.0.1 P2 Core Layout Theme Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build native widget tree, geometry, layout, and theme foundation.

**Architecture:** This phase makes native widget state real before adding more controls. Layout and theme must not call ldgui internals.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P2: Core Widget / Layout / Theme

### Task P2-A: Native widget tree contract

**Files:**
- Create: `tests/picoui/native/test_picoui_native_widget_tree.c`
- Create/Modify: `picoui/src/native/native_widget.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`

- [ ] **Step 1: GitNexus impact**

Run before editing widget creation code:

```text
mcp__gitnexus.impact({repo:"LingDongGUI", target:"picoui_widget", direction:"upstream"})
```

Expected: report risk. If HIGH/CRITICAL, stop and report before editing.

- [ ] **Step 2: 写 RED test**

Create `tests/picoui/native/test_picoui_native_widget_tree.c` with assertions:

```c
#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_widget *root;
    struct picoui_widget *a;
    struct picoui_widget *b;

    assert(picoui_init() == 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    root = picoui_window_create_root(screen);
    assert(root != 0);
    a = picoui_label_create(root, "A");
    b = picoui_button_create(root, "B");
    assert(a != 0);
    assert(b != 0);
    assert(picoui_widget_get_parent(a) == root);
    assert(picoui_widget_get_parent(b) == root);
    assert(picoui_widget_get_first_child(root) == a);
    assert(picoui_widget_get_next_sibling(a) == b);
    assert(picoui_widget_get_next_sibling(b) == 0);
    assert(picoui_widget_get_screen(root) == screen);
    picoui_deinit();
    return 0;
}
```

- [ ] **Step 3: 注册 RED test**

Add `native/test_picoui_native_widget_tree.c` to `PICOUI_NATIVE_TESTS`.

- [ ] **Step 4: Run RED**

```bash
rtk cmake --build build --target test_picoui_native_widget_tree
```

Expected: FAIL because native tree query APIs are missing.

- [ ] **Step 5: Implement minimal tree**

Implement native parent/child/sibling storage in `picoui/src/native/native_widget.c`. Do not call `picoui_backend_widget_create()` or any `ld*` function in this file.

- [ ] **Step 6: GREEN**

```bash
rtk cmake --build build --target test_picoui_native_widget_tree
rtk ctest --test-dir build -R test_picoui_native_widget_tree --output-on-failure
rtk git diff --check
```

Expected: PASS.

- [ ] **Step 7: Update docs and commit**

Add P2-A evidence to `02-core-layout-theme.md`, then commit:

```bash
git add tests/picoui/native/test_picoui_native_widget_tree.c picoui/src/native/native_widget.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/02-core-layout-theme.md
git commit -m "feat: add picoui native widget tree"
```

### Task P2-B: Native geometry and invalidation contract

**Files:**
- Create: `tests/picoui/native/test_picoui_native_geometry_dirty.c`
- Modify: `picoui/src/native/native_widget.c`
- Create/Modify: `picoui/src/native/native_dirty.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`

- [ ] **Step 1: 写 RED test**

Create a test that calls `picoui_widget_set_pos(widget, 10, 20)`, `picoui_widget_set_size(widget, 80, 24)`, then asserts:

```c
assert(picoui_widget_get_x(widget) == 10);
assert(picoui_widget_get_y(widget) == 20);
assert(picoui_widget_get_width(widget) == 80);
assert(picoui_widget_get_height(widget) == 24);
assert(picoui_widget_is_dirty(widget));
```

- [ ] **Step 2: Run RED**

```bash
rtk cmake --build build --target test_picoui_native_geometry_dirty
```

Expected: FAIL because dirty state query is missing or always false.

- [ ] **Step 3: Implement geometry state**

Store `x/y/w/h` and dirty flag in native widget state. `set_pos` and `set_size` must dirty the widget and bubble dirty state to the owning screen/display.

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_native_geometry_dirty
rtk ctest --test-dir build -R test_picoui_native_geometry_dirty --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/native/test_picoui_native_geometry_dirty.c picoui/src/native/native_widget.c picoui/src/native/native_dirty.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/02-core-layout-theme.md
git commit -m "feat: track picoui native geometry dirty state"
```

### Task P2-C: Native flex layout contract

**Files:**
- Create: `tests/picoui/native/test_picoui_native_flex_layout.c`
- Modify: `picoui/src/native/native_layout.c`
- Modify: `picoui/src/layout/flex.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`

- [ ] **Step 1: 写 RED test**

Create a 300x100 root, enable row flex, set gap 10, add three children sized 50x20, run layout, then assert child x positions are `0`, `60`, `120`.

- [ ] **Step 2: Run RED**

```bash
rtk cmake --build build --target test_picoui_native_flex_layout
```

Expected: FAIL because native layout does not compute child positions.

- [ ] **Step 3: Implement row/column flex**

Implement native row and column flex in `picoui/src/native/native_layout.c`; keep wrapping, justify, align as explicit unsupported returns until their own tests are added.

- [ ] **Step 4: Guard no ldgui call**

```bash
rtk rg -n "ldWindowLayoutInternal|ldGrid|ldFlex|ld[A-Z]" picoui/src/native picoui/src/layout
```

Expected: no native-path layout dependency on `ld*`.

- [ ] **Step 5: GREEN**

```bash
rtk cmake --build build --target test_picoui_native_flex_layout
rtk ctest --test-dir build -R test_picoui_native_flex_layout --output-on-failure
rtk git diff --check
```

- [ ] **Step 6: Commit**

```bash
git add tests/picoui/native/test_picoui_native_flex_layout.c picoui/src/native/native_layout.c picoui/src/layout/flex.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/02-core-layout-theme.md
git commit -m "feat: add picoui native flex layout"
```

### Task P2-D: Native grid layout contract

**Files:**
- Create: `tests/picoui/native/test_picoui_native_grid_layout.c`
- Modify: `picoui/src/native/native_layout.c`
- Modify: `picoui/src/layout/grid.c`
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`

- [ ] **Step 1: 写 RED test**

Create a 320x240 root with two columns and two rows. Add child A at cell `(0,0)`, child B at `(1,0)`, child C spanning `(0,1)` to `(1,1)`. Assert calculated geometry:

```c
assert(picoui_widget_get_x(a) == 0);
assert(picoui_widget_get_x(b) > picoui_widget_get_x(a));
assert(picoui_widget_get_width(c) == picoui_widget_get_width(a) + picoui_widget_get_width(b));
```

- [ ] **Step 2: Run RED**

```bash
rtk cmake --build build --target test_picoui_native_grid_layout
```

- [ ] **Step 3: Implement fixed/percent grid subset**

Implement the subset used by `basic_widgets`, `layout_parity`, and `grid_parity`: fixed tracks, percent tracks, gap, cell assignment, row/column span.

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_native_grid_layout
rtk ctest --test-dir build -R test_picoui_native_grid_layout --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/native/test_picoui_native_grid_layout.c picoui/src/native/native_layout.c picoui/src/layout/grid.c docs/picoui-serial/v1.0-native/02-core-layout-theme.md
git commit -m "feat: add picoui native grid layout"
```

### Task P2-E: Native theme/style contract

**Files:**
- Create: `tests/picoui/native/test_picoui_native_style_theme.c`
- Modify: `picoui/src/native/native_style.c`
- Modify: `picoui/src/theme/theme.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`

- [ ] **Step 1: 写 RED test**

Create `window/label/button`, apply background, text color, border, radius, padding through public style/theme APIs, then assert native style readback matches.

- [ ] **Step 2: Run RED**

```bash
rtk cmake --build build --target test_picoui_native_style_theme
```

Expected: FAIL because native style cache is missing.

- [ ] **Step 3: Implement style cache**

Add native style storage and inheritance rules for:

- Background color
- Text color
- Border color/width
- Radius
- Padding left/right/top/bottom
- Disabled/pressed/focused state overrides

- [ ] **Step 4: GREEN**

```bash
rtk cmake --build build --target test_picoui_native_style_theme
rtk ctest --test-dir build -R test_picoui_native_style_theme --output-on-failure
rtk git diff --check
```

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/native/test_picoui_native_style_theme.c picoui/src/native/native_style.c picoui/src/theme/theme.c cmake/LingDongGUI.cmake docs/picoui-serial/v1.0-native/02-core-layout-theme.md
git commit -m "feat: add picoui native style theme state"
```

### Task P2-F: P2 closeout

**Files:**
- Modify: `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`
- Modify: `docs/picoui-serial/v1.0-native/线计划索引.md`
- Modify: `tests/picoui/contract/picoui_native_migration_ledger.json`

- [ ] **Step 1: Run P2 verification**

```bash
rtk ctest --test-dir build -R 'widget_tree|geometry_dirty|flex_layout|grid_layout|style_theme' --output-on-failure
rtk git diff --check
```

- [ ] **Step 2: Update ledger**

Set `P2-widget-tree-layout-theme` to `covered`; include every P2 test command in `evidence`.

- [ ] **Step 3: GitNexus detect changes**

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"all"})
```

- [ ] **Step 4: Commit**

```bash
git add docs/picoui-serial/v1.0-native/02-core-layout-theme.md docs/picoui-serial/v1.0-native/线计划索引.md tests/picoui/contract/picoui_native_migration_ledger.json
git commit -m "docs: close picoui native core layout theme"
```

---
