# PicoUI 单元测试覆盖率分析

> 生成日期: 2026-06-03 | 更新日期: 2026-06-03 (final) | 测试代码: ~12,400 行 | 测试函数: ~276 个 | 测试文件: 37 个

---

## 一、控件覆盖率矩阵

### 有独立测试的控件

| Public Header | 测试文件 | 函数数 | ~行数 | 覆盖内容 |
|:---|:---|:--:|:--:|:---|
| `animation.h` | `test_picoui_animation` | 2 | 118 | create / set_source / set_period_ms / show_frame / round-trip |
| `app.h` | `test_picoui_app_lifecycle` + `test_picoui_app_window_switch` | 5 | 130 | lifecycle (create/destroy/multi-window) / window switch / null args |
| `arc.h` | `test_picoui_arc` | 5 | 160 | create / 角度 / 颜色 / quarter source / parent color |
| `background.h` | `test_picoui_background` | 3 | 55 | create / backend mapping / widget API / offset round-trip |
| `button.h` | `test_picoui_button_events` | ~14 | 500 | 事件分发 / focus / press/release/click / create_with_props / set_text / style_class / 错误路径 |
| `calendar.h` | `test_picoui_calendar` | 8 | 310 | create / set_date / header / bg_color / day_names / grid_value / is_current_month_cell |
| `canvas.h` | `test_picoui_canvas` | ~5 | 65 | create / fill_rect / draw_line / draw_image / draw_text / clear / 边界检查 |
| `checkbox.h` | `test_picoui_checkbox` | 5 | 80 | create / create_with_props / set_checked / set_text / 错误路径 |
| `clock.h` | `test_picoui_clock` | 7 | 326 | create / pointer source / anchor / mask_color / use_system_time / step_second |
| `combo_box.h` | `test_picoui_combo_box` | 10 | 430 | create / static_items / selected / colors / dropdown / item_max / is_open / round-trip |
| `date_time.h` | `test_picoui_date_time` | 7 | 255 | create / format / date / time / color / transparent / system_time |
| `gauge.h` | `test_picoui_gauge` | 6 | 224 | create / angle / source / center / pointer / trail / progress / auto_move |
| `graph.h` | `test_picoui_graph` | 6 | 297 | create / add_series / set_value / axis / point_source / move_add |
| `icon_slider.h` | `test_picoui_icon_slider` | 5 | 237 | create / add_icon / horizontal / speed / selection / round-trip |
| `image.h` | `test_picoui_image` | 3 | 60 | create / backend mapping / create_with_props / set_source / null source |
| `keyboard.h` | `test_picoui_keyboard` | 17 | 572 | create / buttons / events / navigate / input_ascii / click / exit |
| `label.h` | `test_picoui_label` | 4 | 75 | create / backend mapping / set_text / create_with_props / 错误路径 |
| `layout.h` | `test_picoui_layout` | 20 | 895 | flex flow/align/gap/grow / grid columns/rows/gap/align/cell / padding |
| `line_edit.h` | `test_picoui_line_edit` | 11 | 390 | create_with_props / text / type / keyboard / align / color / edit_finished / 错误路径 / 边界值 / round-trip |
| `list.h` | `test_picoui_list` | 16 | 874 | create / items / selected / colors / align / item_height / padding / margin / round-trip |
| `message_box.h` | `test_picoui_message_box` | 8 | 402 | create / title / message / buttons / colors / confirm / round-trip |
| `progress_bar.h` | `test_picoui_progress_bar` | 7 | 273 | create / image / frame / color / inverted / round-trip |
| `progress_wheel.h` | `test_picoui_progress_wheel` | 5 | 172 | create / color / dot / round-trip |
| `qrcode.h` | `test_picoui_qrcode` | 6 | 194 | create / ecc / zoom / colors / max_version / round-trip |
| `radial_menu.h` | `test_picoui_radial_menu` | 6 | 255 | create / add_item / select / navigate / offset / round-trip |
| `scroll_selecter.h` | `test_picoui_scroll_selecter` | 9 | 340 | create / items / select / edit_mode / colors / source / speed / get_selected_text / round-trip |
| `slider.h` | `test_picoui_slider` | 4 | 100 | create / create_with_props / set_range / set_value / set_percent / 错误路径 |
| `switch.h` | `test_picoui_switch` | 4 | 75 | create / create_with_props / set_checked / is_checked / 错误路径 |
| `table.h` | `test_picoui_table` | 11 | 530 | create / cell_text / item_color / item_width / item_height / navigate / excel_type / item_image / item_button / round-trip |
| `text.h` | `test_picoui_text` | 5 | 90 | create / backend mapping / transparent / scroll_seek / scroll_move / create_with_props / 错误路径 |
| `theme.h` | `test_picoui_theme` | 2 | 610 | colors (全部 color slot) / metrics (全部 metric slot) |
| `window.h` | `test_picoui_window` | 3 | 60 | create / backend mapping / padding_group / widget API round-trip |

### 共享测试覆盖的控件 (在 `test_picoui_widgets` 中)

| Public Header | 测试函数 | 覆盖内容 |
|:---|:--:|:---|
| `widget.h` (共用 API) | 31 | set_pos / set_size / set_visible / set_opacity / set_selectable / set_selected / set_corner / set_radius / set_padding / focus / user_data / style_class / is_hidden / destroy |
| `checkbox.h` | 2 | radio group / image mode / native value / create_with_props (独立测试在 test_picoui_checkbox.c) |
| `switch.h` | 3 | direction / navigation / image skin / disabled / create_with_props (独立测试在 test_picoui_switch.c) |
| `slider.h` | 1 | J5 contract / min-max range (独立测试在 test_picoui_slider.c) |

### 混合测试 (在 `test_picoui_native_bridge` 中)

| 覆盖项 | 测试函数 | 覆盖内容 |
|:---|:--:|:---|
| `native.h` | 6 | signal dispatch / align 映射 / nav_dir 映射 / readback policy |

---

## 二、核心模块覆盖深度

| Module | 源文件 | 测试覆盖 | 状态 |
|:---|:---|:---|:--:|
| `widget.h` | `core/widget.c` | test_picoui_widgets (30函数) + 各控件 round-trip + is_hidden | **厚** |
| `window.h` | `widgets/window.c` | test_picoui_window (3函数) | **中** |
| `background.h` | `widgets/background.c` | test_picoui_background (3函数) | **中** |
| `app.h` | `core/app.c` | app_window_switch (2函数) + smoke | **薄** |
| `theme.h` | `theme/theme.c` | test_picoui_theme (2函数) | **中** |
| `native.h` | `core/native.c` | test_picoui_native_bridge (6函数) | **中** |
| `event.h` | `core/event.c` | 散落在各控件测试中 | **薄** |
| `resource.h` | `core/resource.c` | test_picoui_widgets (VRES, `#if` 守卫) | **薄** |
| layout (flex) | `layout/flex.c` | test_picoui_layout | **厚** |
| layout (grid) | `layout/grid.c` | test_picoui_layout | **厚** |

---

## 三、本次补齐 (2026-06-03)

### Round 1: 新建 5 + 补齐 7

### 新建测试文件 (5个)

| 文件 | 函数数 | 行数 |
|:---|:--:|:--:|
| `test_picoui_background.c` | 3 | 55 |
| `test_picoui_window.c` | 3 | 60 |
| `test_picoui_image.c` | 3 | 60 |
| `test_picoui_label.c` | 4 | 75 |
| `test_picoui_text.c` | 5 | 90 |

### 已有文件补齐 (7个)

| 文件 | 新增函数 | 覆盖内容 |
|:---|:--:|:---|
| `test_picoui_button_events.c` | +4 | create_with_props / set_text / style_class / 错误路径 |
| `test_picoui_widgets.c` | +1 | is_hidden contract |
| `test_picoui_line_edit.c` | +2 | 错误路径 NULL / 边界值 |
| `test_picoui_combo_box.c` | +2 | is_open / get_selected_index |
| `test_picoui_scroll_selecter.c` | +2 | get_selected_index / get_selected_text |
| `test_picoui_table.c` | +3 | excel_type / item_image / item_button |
| `test_picoui_calendar.c` | +2 | grid_value / is_current_month_cell |

### 已消除的缺口 (Round 1 + Round 2)

| 原缺口 | Round | 状态 |
|:---|:--:|:--:|
| `background.h` 零覆盖 | R1 | ✅ |
| `window.h` API | R1 | ✅ |
| `button.h` 基础 API | R1 | ✅ |
| `label.h` 基础 API | R1 | ✅ |
| `image.h` 基础 API | R1 | ✅ |
| `text.h` 独立控件 | R1 | ✅ |
| `combo_box.h` is_open/get_selected | R1 | ✅ |
| `scroll_selecter.h` get_selected | R1 | ✅ |
| `table.h` excel/item_image/item_button | R1 | ✅ |
| `calendar.h` grid_value | R1 | ✅ |
| `widget.h` is_hidden | R1 | ✅ |
| `line_edit.h` 错误路径 | R1 | ✅ |
| `checkbox.h` 独立测试 | R2 | ✅ |
| `switch.h` 独立测试 | R2 | ✅ |
| `slider.h` 独立测试 | R2 | ✅ |
| `widget.h` destroy | R2 | ✅ |
| 错误路径系统化 (9控件) | R2 | ✅ — gauge/graph/icon_slider/progress_bar/progress_wheel/qrcode/radial_menu/arc/message_box |
| `image.h` set_pivot | R2 | ❌ — 非 public API, 移除 |

### Round 2 新增/修改

| 文件 | 操作 | 新增函数 | 内容 |
|:--|:--|:--:|:--|
| `test_picoui_checkbox.c` | 新建 | 5 | create/create_with_props/set_checked/set_text/null |
| `test_picoui_switch.c` | 新建 | 4 | create/create_with_props/set_checked/is_checked/null |
| `test_picoui_slider.c` | 新建 | 4 | create/create_with_props/range/value/percent/null |
| `test_picoui_widgets.c` | 修改 | +1 | destroy clears backend |
| `test_picoui_gauge.c` | 修改 | +1 | null args |
| `test_picoui_graph.c` | 修改 | +1 | null args |
| `test_picoui_icon_slider.c` | 修改 | +1 | null args |
| `test_picoui_progress_bar.c` | 修改 | +1 | null args |
| `test_picoui_progress_wheel.c` | 修改 | +1 | null args |
| `test_picoui_qrcode.c` | 修改 | +1 | null args |
| `test_picoui_radial_menu.c` | 修改 | +1 | null args |
| `test_picoui_arc.c` | 修改 | +1 | null args |
| `test_picoui_image.c` | 修改 | +1 | create_with_props null |
| `test_picoui_message_box.c` | 修改 | +1 | null args |

---

## 四、仍缺失覆盖（缺口）

**无。** 所有已识别缺口已补齐。37 测试文件覆盖全部 33 个 public header。

### Round 3 新增/修改

| 文件 | 操作 | 新增函数 | 内容 |
|:--|:--|:--:|:--|
| `test_picoui_app_lifecycle.c` | 新建 | 3 | create/destroy / multi-window / null args |
| `test_picoui_event.c` | 新建 | 6 | focus claim/release/switch/guards / edit result marking / editing claim/release/guards |
| `test_picoui_table.c` | 修改 | +1 | item_image null source |
| `test_picoui_calendar.c` | 修改 | +1 | grid out-of-bounds |
| `test_picoui_message_box.c` | 修改 | +1 | confirm callback registration |
| `test_picoui_label.c` | 修改 | +1 | destroy clears backend |

---

## 五、未覆盖的 Public API 清单

- `picoui_widget_destroy` 已覆盖基本路径。
- 各控件 NULL/边界参数已系统覆盖。
- **无已知 public API 零覆盖**。

---

## 六、建议实施优先级

**全部完成。** 无待办项。

---

## 七、测试结构现状

```
tests/picoui/unit/
├── test_picoui_animation.c       ( 118行,  2函数)
├── test_picoui_app_window_switch.c (  92行,  2函数)
├── test_picoui_arc.c             ( 170行,  6函数)
├── test_picoui_background.c      (  55行,  3函数)  ← R1
├── test_picoui_button_events.c   ( 500行, ~14函数)
├── test_picoui_calendar.c        ( 310行,  8函数)
├── test_picoui_canvas.c          (  65行, ~5函数)
├── test_picoui_checkbox.c        (  80行,  5函数)  ← R2
├── test_picoui_clock.c           ( 326行,  7函数)
├── test_picoui_combo_box.c       ( 430行, 10函数)
├── test_picoui_date_time.c       ( 255行,  7函数)
├── test_picoui_gauge.c           ( 240行,  7函数)
├── test_picoui_graph.c           ( 310行,  7函数)
├── test_picoui_icon_slider.c     ( 250行,  6函数)
├── test_picoui_image.c           (  55行,  4函数)  ← R1
├── test_picoui_keyboard.c        ( 572行, 17函数)
├── test_picoui_label.c           (  75行,  4函数)  ← R1
├── test_picoui_layout.c          ( 895行, 20函数)
├── test_picoui_line_edit.c       ( 390行, 11函数)
├── test_picoui_list.c            ( 874行, 16函数)
├── test_picoui_message_box.c     ( 420行,  9函数)
├── test_picoui_native_bridge.c   (  76行,  6函数)
├── test_picoui_progress_bar.c    ( 285行,  8函数)
├── test_picoui_progress_wheel.c  ( 185行,  6函数)
├── test_picoui_qrcode.c          ( 210行,  7函数)
├── test_picoui_radial_menu.c     ( 270行,  7函数)
├── test_picoui_scroll_selecter.c ( 340行,  9函数)
├── test_picoui_slider.c          ( 100行,  4函数)  ← R2
├── test_picoui_smoke.c           (  16行,  1函数)
├── test_picoui_switch.c          (  75行,  4函数)  ← R2
├── test_picoui_table.c           ( 530行, 11函数)
├── test_picoui_text.c            (  90行,  5函数)  ← R1
├── test_picoui_theme.c           ( 610行,  2函数)
├── test_picoui_widgets.c         (2580行, 31函数)
└── test_picoui_window.c          (  60行,  3函数)  ← R1
```
