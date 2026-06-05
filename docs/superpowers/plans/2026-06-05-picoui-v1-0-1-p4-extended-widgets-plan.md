# PicoUI v1.0.1 P4 Extended Widgets Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate extended widgets to native ARM-2D runtime paths.

**Architecture:** This phase migrates the larger widget set independently. Each subagent owns exactly one widget group to avoid write-surface overlap.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P4: 扩展控件迁移

每个 P4 任务只允许修改该控件 public wrapper、该控件 native 实现、该控件 focused test、CMake、P4 文档。每个任务必须先跑 GitNexus impact；HIGH/CRITICAL 停下汇报。

### Task P4-A: arc
Files: `tests/picoui/native/test_picoui_native_arc.c`, `picoui/src/widgets/arc.c`, `picoui/src/native/native_arc.c`, `cmake/LingDongGUI.cmake`, `docs/picoui-serial/v1.0-native/04-扩展控件迁移.md`.
Steps: RED create arc, set min/max/value/start/end angle, assert value readback and render smoke; implement without `ldArc*`; GREEN focused test; run `arc_basic` demo smoke; commit `feat: add picoui native arc`.

### Task P4-B: calendar
Files: `tests/picoui/native/test_picoui_native_calendar.c`, `picoui/src/widgets/calendar.c`, `picoui/src/native/native_calendar.c`, CMake, P4 docs.
Steps: RED set year/month/day and selected date; assert selected date and callback; implement month model and day hit-test without `ldCalendar*`; GREEN; run `calendar_basic`; commit `feat: add picoui native calendar`.

### Task P4-C: clock
Files: `tests/picoui/native/test_picoui_native_clock.c`, `picoui/src/widgets/clock.c`, `picoui/src/native/native_clock.c`, CMake, P4 docs.
Steps: RED set hour/min/sec, tick one second, assert wrap from `23:59:59` to `00:00:00`; implement time state/render without `ldClock*`; GREEN; run `clock_basic`; commit `feat: add picoui native clock`.

### Task P4-D: combo_box
Files: `tests/picoui/native/test_picoui_native_combo_box.c`, `picoui/src/widgets/combo_box.c`, `picoui/src/native/native_combo_box.c`, CMake, P4 docs.
Steps: RED add options, open dropdown, select option 2, assert selected text and callback; implement popup/list state without `ldComboBox*`; GREEN; run `combo_box_basic`; commit `feat: add picoui native combo box`.

### Task P4-E: date_time
Files: `tests/picoui/native/test_picoui_native_date_time.c`, `picoui/src/widgets/date_time.c`, `picoui/src/native/native_date_time.c`, CMake, P4 docs.
Steps: RED set date/time, format string, assert formatted output; implement state/format without `ldDateTime*`; GREEN; run `date_time_basic`; commit `feat: add picoui native date time`.

### Task P4-F: gauge
Files: `tests/picoui/native/test_picoui_native_gauge.c`, `picoui/src/widgets/gauge.c`, `picoui/src/native/native_gauge.c`, CMake, P4 docs.
Steps: RED set range/value/tick count; assert clamped value; implement needle/scale state without `ldGauge*`; GREEN; run `gauge_basic`; commit `feat: add picoui native gauge`.

### Task P4-G: graph
Files: `tests/picoui/native/test_picoui_native_graph.c`, `picoui/src/widgets/graph.c`, `picoui/src/native/native_graph.c`, CMake, P4 docs.
Steps: RED create series, append points, assert series count and bounds; implement native series storage/render smoke without `ldGraph*`; GREEN; run `graph_basic`; commit `feat: add picoui native graph`.

### Task P4-H: icon_slider
Files: `tests/picoui/native/test_picoui_native_icon_slider.c`, `picoui/src/widgets/icon_slider.c`, `picoui/src/native/native_icon_slider.c`, CMake, P4 docs.
Steps: RED set icons/range/value and drag; assert value and selected icon; implement as slider composition without `ldIconSlider*`; GREEN; run `icon_slider_basic`; commit `feat: add picoui native icon slider`.

### Task P4-I: keyboard and line_edit
Files: `tests/picoui/native/test_picoui_native_keyboard_line_edit.c`, `picoui/src/widgets/keyboard.c`, `picoui/src/widgets/line_edit.c`, `picoui/src/native/native_keyboard.c`, `picoui/src/native/native_line_edit.c`, CMake, P4 docs.
Steps: RED focus line edit, send keyboard key `A`, backspace, enter; assert text buffer and submit callback; implement focus/text edit/key map without `ldKeyboard*` or `ldLineEdit*`; GREEN; run `keyboard_basic` and `line_edit_basic`; commit `feat: add picoui native keyboard line edit`.

### Task P4-J: message_box
Files: `tests/picoui/native/test_picoui_native_message_box.c`, `picoui/src/widgets/message_box.c`, `picoui/src/native/native_message_box.c`, CMake, P4 docs.
Steps: RED create modal message box with OK/Cancel; click OK; assert modal close and callback id; implement overlay/dialog composition without `ldMessageBox*`; GREEN; run `message_box_basic`; commit `feat: add picoui native message box`.

### Task P4-K: progress_bar and progress_wheel
Files: `tests/picoui/native/test_picoui_native_progress.c`, `picoui/src/widgets/progress_bar.c`, `picoui/src/widgets/progress_wheel.c`, `picoui/src/native/native_progress_bar.c`, `picoui/src/native/native_progress_wheel.c`, CMake, P4 docs.
Steps: RED set value percent on bar/wheel; assert clamp `0..100` and render smoke; implement linear/circular progress without `ldProgress*`; GREEN; run `progress_bar_basic` and `progress_wheel_basic`; commit `feat: add picoui native progress widgets`.

### Task P4-L: qrcode
Files: `tests/picoui/native/test_picoui_native_qrcode.c`, `picoui/src/widgets/qrcode.c`, `picoui/src/native/native_qrcode.c`, CMake, P4 docs.
Steps: RED set payload string and ECC level; assert module matrix exists; implement native qrcode state and hand off bitmap renderer to P5; GREEN; run `qrcode_basic`; commit `feat: add picoui native qrcode`.

### Task P4-M: radial_menu
Files: `tests/picoui/native/test_picoui_native_radial_menu.c`, `picoui/src/widgets/radial_menu.c`, `picoui/src/native/native_radial_menu.c`, CMake, P4 docs.
Steps: RED add menu sectors, pointer select angle sector, assert selected item; implement angle hit-test without `ldRadialMenu*`; GREEN; run `radial_menu_basic`; commit `feat: add picoui native radial menu`.

### Task P4-N: scroll_selecter
Files: `tests/picoui/native/test_picoui_native_scroll_selecter.c`, `picoui/src/widgets/scroll_selecter.c`, `picoui/src/native/native_scroll_selecter.c`, CMake, P4 docs.
Steps: RED add values, scroll delta, assert selected index snaps to nearest item; implement inertial-free scroll selection without `ldScrollSelecter*`; GREEN; run `scroll_selecter_basic`; commit `feat: add picoui native scroll selecter`.

### Task P4-O: table
Files: `tests/picoui/native/test_picoui_native_table.c`, `picoui/src/widgets/table.c`, `picoui/src/native/native_table.c`, CMake, P4 docs.
Steps: RED set row/column/cell text, select cell, assert readback and callback; implement table storage/layout without `ldTable*`; GREEN; run `table_basic`; commit `feat: add picoui native table`.

### Task P4-P: animation and canvas
Files: `tests/picoui/native/test_picoui_native_animation_canvas.c`, `picoui/src/widgets/animation.c`, `picoui/src/widgets/canvas.c`, `picoui/src/native/native_animation.c`, `picoui/src/native/native_canvas.c`, CMake, P4 docs.
Steps: RED start animation, advance timer, assert frame index; draw canvas rect/line/circle, assert command list; implement native animation tick and canvas command buffer without ldgui; GREEN; run `animation_basic`; commit `feat: add picoui native animation canvas`.

### Task P4-Q: P4 closeout
Files: `docs/picoui-serial/v1.0-native/04-扩展控件迁移.md`, `docs/picoui-serial/v1.0-native/线计划索引.md`, ledger.
Steps:

- [ ] Run `rtk ctest --test-dir build -R 'native_(arc|calendar|clock|combo_box|date_time|gauge|graph|icon_slider|keyboard_line_edit|message_box|progress|qrcode|radial_menu|scroll_selecter|table|animation_canvas)' --output-on-failure`.
- [ ] Run all P4 demo smokes listed above.
- [ ] Update ledger `P4-extended-widgets` to `covered` with test/demo evidence.
- [ ] Run GitNexus detect changes.
- [ ] Commit `docs: close picoui native extended widgets`.

---
