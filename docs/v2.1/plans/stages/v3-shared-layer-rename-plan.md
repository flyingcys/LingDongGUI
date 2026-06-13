# TinyUI v2.1 V3 Shared Layer Rename Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 收口 shared layers 的产品层命名、include path、target 与内部引用，让产品层共享目录统一到 `tinyui`。

**Architecture:** V3 只处理 shared subsystems，不处理 demo/test/contract 全量迁移。重点是把 `core/display/indev/layout/theme/tick/osal` 的产品层路径、头文件引用、target 命名从 `tinyui` 收到 `tinyui`，同时保证共享层仍然薄化。

**Tech Stack:** C11、CMake、现有 shared layer 源码、unit/runtime/perf gates。

---

## 文件结构

修改：

- `tinyui/src/core/*`
- `tinyui/src/display/*`
- `tinyui/src/indev/*`
- `tinyui/src/layout/*`
- `tinyui/src/theme/*`
- `tinyui/src/tick/*`
- `tinyui/src/osal/*`
- `tinyui/include/*`
- `cmake/LingDongGUI.cmake`

---

### Task 1: 统一 shared layer 文件与 include path

**Files:**
- Modify: shared source files and headers listed above

- [x] **Step 1: 建立 fail-first path scan**

Use a repository search to identify product-layer shared files that still include or reference `tinyui/...` paths after V1/V2:

```bash
rg -n "tinyui/" tinyui/src tinyui/include cmake/LingDongGUI.cmake
```

Expected: FAIL/HITS at the start of V3.

当前真相：

- 已在 `V3` 起步时完成 fail-first 扫描，命中过 shared layer 对 `tinyui/...` include path 的直接依赖
- 对应命中随后已在 `V3 Task 1 Step 2` 中被清零；当前 shared 源码层不再直接 `#include "tinyui/..."`

- [x] **Step 2: 收口 shared layer include**

Replace product-layer include path references so shared files refer to the unified `tinyui` tree instead of the old `tinyui` tree.

当前真相：

- `tinyui/src/{core,display,indev,layout,theme,tick,osal}` 已全部改走统一入口，不再直接引用旧 `tinyui/...` 路径
- `tinyui/include/*.h` 已补齐 shared 层需要的顶层 wrapper 入口，用于承接统一 include path
- 这一批没有触碰 public `tinyui_*` API、`tests/tinyui/*` 路径、`test_tinyui_*` / `check_tinyui_*` 文件名或 CTest label

- [x] **Step 3: 收口 shared layer symbol naming**

Where a symbol is product-layer shared infrastructure rather than `LingDongGUI` engine truth, rename it from `tinyui_*` to `tinyui_*`.

Do not rename:

- `ld*`
- `LingDongGUI` directory paths
- engine-private symbols outside the product layer

当前真相：

- internal/shared 命名收口已经完成到当前 `V3` 计划边界，覆盖：
  - `runtime_bridge` internal/shared 接口
  - `window_apply_*`、`native_*`
  - `display/indev` 文件内 static helper
  - `theme.c`、`layout/{flex,grid}.c` pure-static helper
  - `runtime_host.c` internal runtime helper 全链路
  - `core/widget.c` pure-static/internal helper
  - `shared core internal event/callback seam`：`tinyui_backend_emit_value_changed()`、`tinyui_backend_emit_event()`、`tinyui_backend_emit_clicked()` 已统一收口为 `tinyui_widget_emit_*`
  - `theme.c` 的 backend-facing internal helper：`tinyui_theme_apply_widget_style()` 已收口为 `tinyui_theme_apply_widget_style()`
  - `shared widget-tree lifecycle seam` 的第一小批：`tinyui_backend_widget_is_kind()` 已收口为 `tinyui_widget_is_kind()`
  - `shared widget-tree lifecycle seam` 的第二小批：`tinyui_backend_widget_init_root()`、`tinyui_backend_widget_init_child()`、`tinyui_backend_widget_attach_child()` 已统一收口为 `tinyui_widget_init_root()`、`tinyui_widget_init_child()`、`tinyui_widget_attach_child()`
  - `list selected-index seam`：`tinyui_list_backend_set_selected_index()`、`tinyui_list_backend_get_selected_index()`、`tinyui_list_backend_sync_selected_index()` 已统一收口为 `tinyui_list_set_selected_index()`、`tinyui_list_get_selected_index()`、`tinyui_list_sync_selected_index()`
  - `combo_box` 的 internal/widget-local helper seam：`tinyui_backend_combo_box_*`、`tinyui_combo_box_create_backend_local()`、`tinyui_backend_combo_box_native_slot()` 已统一收口为 `tinyui_combo_box_*`
  - `line_edit` 的 internal/widget-local helper seam：`tinyui_backend_line_edit_*`、`tinyui_line_edit_create_backend_local()`、`tinyui_backend_line_edit_native_slot()` 已统一收口为 `tinyui_line_edit_*`
  - `core/event` shared helper seam：`tinyui_widget_accepts_event()`、`tinyui_widget_slider_value_to_percent()`、`tinyui_widget_slider_percent_to_value()`、`tinyui_widget_sync_ld_value()`、`tinyui_widget_emit_ld_event_bridge()`、`tinyui_widget_claim_focus_for_signal()`、`tinyui_widget_restore_rejected_list_selection()`、`tinyui_widget_get_owner_app()`、`tinyui_widget_note_focus_event()` 已统一收口为 `tinyui_widget_*`
  - `text` 的 widget-local/internal seam：`tinyui_backend_text_*` 与 `tinyui_backend_text_test_fail_next_set_font()` 已统一收口为 `tinyui_text_*`
  - `list` 的剩余 widget-local/internal seam：`tinyui_list_rgb_to_ld_color()`、`tinyui_list_map_align()`、`tinyui_list_backend()`、`tinyui_list_ld_widget()` 以及 `tinyui_backend_list_{set_items,set_item_height,set_padding_group,set_margin_group,set_text_color,set_bg_color,set_select_color,set_align,set_item_widget}()` 已统一收口为 `tinyui_list_*`
- 当前 shared 层残留的 `tinyui_*` 主要是 public API、public type、或现有 internal/shared entry seam；这些不应在 `V3` 被草率混同为全量 rename，更不应提前跨进 `V4`
- `list` 当前 widget-local/internal seam 已在当前真相下继续收完：除前一批 `selected-index` seam 外，本轮又把 `rgb_to_ld_color/map_align/backend/get_ld` 与 moved helper fail-closed surface 一并统一到 `tinyui_list_*`
- 到当前顺序，`list` 这条 retained internal seam 已继续向 `tinyui_*` 收口；下一自然批次前移到其他仍残留较密集 `tinyui_*` widget-local seam 的控件批次
- `combo_box` 这一步也继续保持 internal-only 边界：只收 `combo_box.c` 里的 widget-local helper 名称，不触碰 public `tinyui_combo_box_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `line_edit` 这一步也继续保持 internal-only 边界：只收 `line_edit.c` 里的 widget-local helper 名称，不触碰 public `tinyui_line_edit_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `core/event` 这一步也继续保持 internal-only 边界：只收 `event.c` 里的 shared helper 名称，不触碰 `tinyui_widget_update_value()`、`tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()` 这些 shared/public entry，不触碰 public `tinyui_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `text` 这一步也继续保持 internal-only 边界：只收 `text.c` 里的 widget-local helper 与 test seam 名称，不触碰 public `tinyui_text_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `list` 这一步也继续保持 internal-only 边界：只收 `list.c` 里的 widget-local helper 名称和对应 focused proof，不触碰 public `tinyui_list_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `calendar` 这一步也继续保持 internal-only 边界：只收 `calendar.c` 与 `backend.h` 里的 widget-local helper 名称和对应 focused proof，不触碰 public `tinyui_calendar_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `calendar` 的 widget-local/internal seam 已在当前真相下收完：旧的 day-name globals、native readback/helper、widget-local create helper，以及整组 date/header/color/grid/system-date retained helper 已统一收口为 `g_tinyui_calendar_*` 与 `tinyui_calendar_*`
- `window` 这一步已先完成最小的 `color test seam`：旧的 bg-color fail flag、RGB/native color conversion helper 与对应 test seam 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：只收 `window.c + test_tinyui_window` 里的 color helper 与 test seam 名称，不触碰 public `tinyui_window_*` API，也不把 `root/layout/background` 大链混进同一批
- `window` 这一步又继续完成了更小的 `layout mapper seam`：旧的 flex/grid align-flow-track mapping helper 与 grid-track copy helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`layout mapper seam` 只收 `window.c + test_tinyui_window` 里的 pure-static mapper/copy helper 名称，不触碰 public `tinyui_window_*` API，也不把 `get_ld_window/get_root_size/init_defaults/dispose_partial/apply_*` 与 `root/layout/background` 大链混进来
- `window` 这一步又继续完成了更小的 `native access seam`：旧的 backend/native access helper 与 layout-type mapping helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`native access seam` 只收 `window.c + test_tinyui_window` 里的 `get_backend/get_backend_host/get_ld_window/map_layout_type` 这组 static helper 名称，不触碰 public `tinyui_window_*` API，也不把 `get_root_size/init_defaults/dispose_partial/apply_*` 与 `root/layout/background` 大链混进来
- `window` 这一步又继续完成了更小的 `validation seam`：旧的 `is_valid/props_are_valid` 这组 static 校验 helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`validation seam` 只收 `window.c + test_tinyui_window` 里的 pure-static 校验 helper 名称，不触碰 public `tinyui_window_*` API，也不把 `get_root_size/init_defaults/dispose_partial/apply_*` 与 `root/layout/background` 大链混进来
- `window` 这一步又继续完成了更小的 `init-defaults seam`：旧的 `init_defaults` static helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`init-defaults seam` 只收 `window.c + test_tinyui_window` 里的单一 static 初始化 helper 名称，不触碰 public `tinyui_window_*` API，也不把 `get_root_size/dispose_partial` 或 `apply_*` 与 `root/layout/background` 大链混进来
- `window` 这一步又继续完成了更小的 `root-size seam`：旧的 `get_root_size` static helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`root-size seam` 只收 `window.c + test_tinyui_window` 里的单一 static root-size helper 名称，不触碰 public `tinyui_window_*` API，也不把 `dispose_partial` 或 `apply_*` 与 `root/layout/background` 大链混进来
- `window` 这一步又继续完成了更小的 `dispose-partial seam`：旧的 `dispose_partial` static helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`dispose-partial seam` 只收 `window.c + test_tinyui_window` 里的单一 static lifecycle helper 名称，不触碰 public `tinyui_window_*` API，也不把 `set_padding_group_impl/apply_padding_contract/apply_*_impl` 那条 layout-contract 大链混进来
- `window` 这一步又继续完成了更小的 `generic-gap seam`：旧的 `apply_generic_gap_impl` static helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：`generic-gap seam` 只收 `window.c + test_tinyui_window` 里的单一 static gap helper 名称，不触碰 public `tinyui_window_*` API，也不把 `set_padding_group_impl/apply_padding_contract/apply_layout_type_impl/apply_flex_contract_impl` 那条剩余 layout-contract 主链混进来
- `window` 这一步又继续完成了 `padding-contract seam` 的第一批：旧的 `set_padding_group_impl` 与共享核心 `apply_padding_contract` 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：这一批只收 `window.c + test_tinyui_window` 里的 `set_padding_group_impl + apply_padding_contract` 这一组内部 padding-contract helper 名称，不触碰 public `tinyui_window_*` API，也不把 `apply_layout_type_impl` 或死 helper `apply_flex_contract_impl` 混进来
- `window` 这一步又继续完成了 `layout-type seam`：旧的 `apply_layout_type_impl` static helper 已统一收口为 `tinyui_window_*`
- `window` 这一步继续保持 internal-only 边界：这一批只收 `window.c + test_tinyui_window` 里的单一 `apply_layout_type_impl` helper 名称，不触碰 public `tinyui_window_*` API，也不把最后的死 helper `apply_flex_contract_impl` 混进来
  - `window` 这一步又继续完成了最后一个 `flex-contract dead-helper seam`：旧的 `apply_flex_contract_impl` static dead helper 已统一收口为 `tinyui_window_*`
  - `window` 在 `V3 Step 3` 里的 internal-only rename 线到当前真相已经收口：`color test / layout mapper / native access / validation / init-defaults / root-size / dispose-partial / generic-gap / padding-contract / layout-type / dead-helper` 这些 widget-local/internal seam 都已统一切到 `tinyui_window_*`
  - 当前 `window.c` 在 `V3 Step 3` 下不再残留 `tinyui_window_*` internal helper 命名；后续若还有 `window` 相关工作，边界应转到 public API、shared truth 或 `V4` 资产面，而不是继续做同类 internal rename
  - `shared core host-binding seam` 当前也已继续完成一小批：`tinyui_widget_bind_backend_host()`、`tinyui_widget_backend_host()`、`tinyui_widget_backend_detach()`、`tinyui_widget_owner_app()`、`tinyui_widget_has_ld_binding()` 已统一收口为 `tinyui_widget_*`，改动范围只覆盖 `core/internal.h + core/widget.c + core/runtime_bridge.c + widgets/list.c` 与 focused proof `test_tinyui_widgets + test_tinyui_button_events + 多个只读 extern 调用测试`
  - 这一步继续保持 internal-only 边界：只收 shared/core 的 host-binding helper cluster，不触碰 `tinyui_widget_update_value()`、`tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()` 或 `tinyui_backend_set_text()`；因此 `V3 Step 3` 仍在进行中，不能写成代码面整体 closeout
  - `shared core update-value seam` 当前也已继续完成一小批：`tinyui_widget_update_value()` 已统一收口为 `tinyui_widget_update_value()`，改动范围只覆盖 `core/internal.h + core/widget.c + widgets/checkbox.c + widgets/switch.c + widgets/slider.c` 与 focused proof `test_tinyui_widgets`
  - 这一步继续保持 internal-only 边界：本批只收 `update_value` 这一条 shared setter-helper，不触碰 `tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()` 或 `tinyui_backend_set_text()`
  - `shared core dispatch seam` 当前也已继续完成一小批：`tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()` 已统一收口为 `tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()`，改动范围只覆盖 `core/internal.h + core/event.c + core/runtime_bridge.c + test_tinyui_button_events + test_tinyui_widgets + test_tinyui_keyboard`
  - 这一步继续保持 internal-only 边界：本批只收 `dispatch_*` 这一组 shared dispatch helper，不触碰 public `tinyui_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `progress_wheel` 当前也已继续完成一小批 internal/widget-local/test seam：`tinyui_backend_progress_wheel_fail_next_set_percent`、`struct tinyui_progress_wheel_cfg_bridge`、`tinyui_progress_wheel_rgb_to_ld_color()`、`struct tinyui_progress_wheel_test_dispose_snapshot`、`tinyui_progress_wheel_last_dispose_snapshot*`、`tinyui_progress_wheel_backend()`、`tinyui_progress_wheel_get_ld()`、`tinyui_progress_wheel_finish_detach_after_backend_failure()`、`tinyui_progress_wheel_test_reset_internal_state()`、`tinyui_backend_progress_wheel_test_{reset_state,take_last_dispose_snapshot,capture_dispose_snapshot,fail_next_set_percent}()`、`tinyui_progress_wheel_test_finish_detach_after_backend_failure()`、`tinyui_progress_wheel_disable_dirty_regions()`、`tinyui_progress_wheel_dispose_partial_impl()` 与 `tinyui_progress_wheel_props_are_valid()` 已统一收口为 `tinyui_progress_wheel_*`，改动范围只覆盖 `widgets/progress_wheel.c + test_tinyui_progress_wheel`
  - 这一步继续保持 internal-only 边界：`progress_wheel` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_progress_wheel_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `switch` 当前也已继续完成一小批 internal/widget-local seam：`tinyui_switch_backend()`、`tinyui_switch_get_ld()`、`tinyui_switch_nav_dir_to_ld()`、`tinyui_switch_attach_native()`、`tinyui_switch_props_are_valid()` 已统一收口为 `tinyui_switch_*`，改动范围只覆盖 `widgets/switch.c`
  - 这一步继续保持 internal-only 边界：`switch` 这轮只收 widget-local/internal helper 名称，不触碰 public `tinyui_switch_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；到当前真相，下一自然顺序已前移到其他仍保留较密集 `tinyui_*` widget-local seam 的控件批次，如 `keyboard` / `table`
  - `keyboard` 当前也已继续完成一小批 internal/widget-local seam：`tinyui_keyboard_free_layout()`、`tinyui_keyboard_props_are_valid()`、`tinyui_keyboard_get_selected_key_code_internal()`、`tinyui_keyboard_get_ld_widget()`、`tinyui_keyboard_get_target_line_edit_local()`、`tinyui_keyboard_get_custom_button_list_local()`、`tinyui_keyboard_prepare_local()` 已统一收口为 `tinyui_keyboard_*`，主要改动 `widgets/keyboard.c`
  - 这一步继续保持 internal-only 边界：`keyboard` 这轮只收 widget-local/internal helper 名称，不触碰 public `tinyui_keyboard_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `table` 当前也已继续完成一小批 internal/widget-local/test seam：`struct tinyui_table_test_dispose_snapshot`、`tinyui_table_last_dispose_snapshot*`、`tinyui_backend_table_fail_next_set_keyboard_binding`、`tinyui_table_dims_are_valid()`、`tinyui_table_keyboard_binding_is_valid()`、`tinyui_table_finish_detach_after_backend_failure()`、`tinyui_table_dispose_partial()`、`tinyui_table_props_are_valid()`、`tinyui_table_get_ld_widget()`、`tinyui_table_get_strict_ld_widget()`、`tinyui_table_align_to_ld()`、`tinyui_table_sync_current_cell_local()`、`tinyui_table_native_slot()`、`tinyui_table_bind_native_slot()` 以及对应测试 seam 已统一收口为 `tinyui_table_*`，改动范围覆盖 `widgets/table.c + test_tinyui_table`
  - 这一步继续保持 internal-only 边界：`table` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_table_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；到当前真相，下一自然顺序继续前移到剩余仍保留较密集 `tinyui_*` widget-local seam 的控件批次
  - `checkbox` 当前也已继续完成一小批 internal/widget-local seam：`tinyui_checkbox_fail_next_set_check_color`、`tinyui_checkbox_rgb_to_ld_color()`、`tinyui_checkbox_get_ld()`、`tinyui_checkbox_props_are_valid()`、`tinyui_checkbox_dispose_partial()`、`tinyui_backend_checkbox_test_fail_next_set_check_color()` 已统一收口为 `tinyui_checkbox_*`，改动范围覆盖 `widgets/checkbox.c + test_tinyui_checkbox`
  - 这一步继续保持 internal-only 边界：`checkbox` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_checkbox_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；到当前真相，下一自然顺序已前移到 `combo_box` / `date_time`
  - `combo_box` 当前也已继续完成一小批 residual internal seam：`tinyui_combo_box_props_are_valid()`、`tinyui_combo_box_dispose_partial()` 已统一收口为 `tinyui_combo_box_*`，改动范围覆盖 `widgets/combo_box.c + test_tinyui_combo_box`
  - 这一步继续保持 internal-only 边界：`combo_box` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_combo_box_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `date_time` 当前也已继续完成一小批 residual internal seam：`tinyui_date_time_rgb_to_ld_color()`、`tinyui_date_time_map_align()`、`tinyui_date_time_get_ld()`、`tinyui_date_time_props_are_valid()` 已统一收口为 `tinyui_date_time_*`，改动范围覆盖 `widgets/date_time.c + test_tinyui_date_time`
  - 这一步继续保持 internal-only 边界：`date_time` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_date_time_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `line_edit` 当前也已继续完成一小批 residual internal seam：`tinyui_line_edit_type_is_valid()`、`tinyui_line_edit_keyboard_binding_is_valid()`、`tinyui_line_edit_props_are_valid()`、`tinyui_line_edit_dispose_partial()` 已统一收口为 `tinyui_line_edit_*`，改动范围覆盖 `widgets/line_edit.c + test_tinyui_line_edit`
  - 这一步继续保持 internal-only 边界：`line_edit` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_line_edit_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `calendar` 当前也已继续完成一小批 residual internal seam：`tinyui_calendar_props_are_valid()`、`tinyui_calendar_sync_grid()`、`tinyui_calendar_dispose_partial()` 已统一收口为 `tinyui_calendar_*`，改动范围覆盖 `widgets/calendar.c + test_tinyui_calendar`
  - 这一步继续保持 internal-only 边界：`calendar` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_calendar_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `progress_bar` 当前也已继续完成一小批 residual internal/test seam：`tinyui_progress_bar_last_dispose_snapshot*`、`tinyui_progress_bar_test_reset_internal_state()`、`tinyui_progress_bar_rgb_to_ld_color()`、`tinyui_progress_bar_backend()`、`tinyui_progress_bar_get_ld()`、`tinyui_progress_bar_finish_detach_after_backend_failure()`、`tinyui_progress_bar_dispose_partial_impl()`、`tinyui_progress_bar_props_are_valid()`、`tinyui_progress_bar_create_with_props_impl()`、`tinyui_backend_progress_bar_test_{dispose_partial,reset_state,take_last_dispose_snapshot,create_with_props_fail_before_inverted}()` 已统一收口为 `tinyui_progress_bar_*`，改动范围覆盖 `widgets/progress_bar.c + test_tinyui_progress_bar`
  - 这一步继续保持 internal-only 边界：`progress_bar` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_progress_bar_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `gauge` 当前也已继续完成一小批 residual internal/test seam：`tinyui_gauge_props_are_valid()`、`tinyui_gauge_rgb_to_ld_color()`、`tinyui_gauge_ld_color_to_rgb()`、`tinyui_gauge_backend()`、`tinyui_gauge_get_ld()`、`tinyui_gauge_finish_detach_after_backend_failure()`、`tinyui_gauge_dispose_partial_impl()`、`tinyui_gauge_last_dispose_snapshot*`、`tinyui_backend_gauge_test_{take_last_dispose_snapshot,create_with_props_fail_before_centre_offset}()` 已统一收口为 `tinyui_gauge_*`，改动范围覆盖 `widgets/gauge.c + test_tinyui_gauge`
  - 这一步继续保持 internal-only 边界：`gauge` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_gauge_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；主线程随后又补掉了 `gauge.c` 里 2 个漏网旧调用点，把内部取 ld 路径统一切到 `tinyui_gauge_get_ld()`
  - `text` 当前也已继续完成一小批 residual internal seam：`tinyui_text_dispose_partial()`、`tinyui_text_props_are_valid()` 已统一收口为 `tinyui_text_*`，改动范围覆盖 `widgets/text.c + test_tinyui_text`
  - 这一步继续保持 internal-only 边界：`text` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_text_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `qrcode` 当前也已继续完成一小批 residual internal/test seam：`tinyui_qrcode_props_are_valid()`、`tinyui_qrcode_get_ld()`、`tinyui_qrcode_finish_detach_after_backend_failure()`、`tinyui_qrcode_dispose_partial_impl()`、`tinyui_qrcode_create_with_props_impl()` 与 `tinyui_backend_qrcode_test_{reset_state,create_with_props_fail_before_text,take_last_dispose_snapshot}()` 已统一收口为 `tinyui_qrcode_*`，改动范围覆盖 `widgets/qrcode.c + test_tinyui_qrcode`
  - 这一步继续保持 internal-only 边界：`qrcode` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_qrcode_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；主线程随后又补掉了 `qrcode.c` 里 1 个漏网旧调用点，把 `set_zoom()` 的内部取 ld 路径统一切到 `tinyui_qrcode_get_ld()`
  - `button` 当前也已继续完成一小批 residual internal/test seam：`tinyui_button_fail_next_set_font`、`tinyui_button_get_ld()`、`tinyui_button_default_font()`、`tinyui_button_resolve_font()`、`tinyui_button_dispose_partial()`、`tinyui_backend_button_test_fail_next_set_font()`、`tinyui_button_props_are_valid()` 已统一收口为 `tinyui_button_*`，改动范围覆盖 `widgets/button.c + test_tinyui_button_events`
  - 这一步继续保持 internal-only 边界：`button` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_button_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `slider` 当前也已继续完成一小批 residual internal/test seam：`tinyui_slider_fail_indicator_width_id`、`tinyui_slider_last_disposed_backend_snapshot`、`tinyui_slider_last_disposed_backend_valid`、`tinyui_slider_backend()`、`tinyui_slider_get_ld()`、`tinyui_slider_should_fail_indicator_width()`、`tinyui_slider_dispose_partial()`、`tinyui_slider_props_are_valid()` 与 `tinyui_backend_slider_test_{fail_indicator_width_for_id,last_disposed_backend}()` 已统一收口为 `tinyui_slider_*`，改动范围覆盖 `widgets/slider.c + test_tinyui_slider`
  - 这一步继续保持 internal-only 边界：`slider` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_slider_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；到当前真相，下一自然顺序已前移到仍残留较密集 `tinyui_*` widget-local seam 的控件批次
  - `label` 当前也已继续完成一小批 residual internal seam：`tinyui_backend_label_get_ld()`、`tinyui_label_props_are_valid()`、`tinyui_label_dispose_partial()` 已统一收口为 `tinyui_label_*`，并把 `label.c` 内部 shared text bridge 调用统一切到 `tinyui_widget_set_backend_text()`，改动范围覆盖 `widgets/label.c + test_tinyui_label`
  - 这一步继续保持 internal-only 边界：`label` 这轮只收 widget-local/internal helper 名称，不触碰 public `tinyui_label_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `image` 当前也已继续完成一小批 residual internal/test seam：`tinyui_image_rgb_to_ld_color()`、`tinyui_image_props_are_valid()`、`tinyui_image_finish_detach_after_backend_failure()`、`tinyui_image_dispose_partial_impl()`、`tinyui_image_create_with_props_impl()` 与 `tinyui_backend_image_test_{reset_state,create_with_props_fail_before_size,take_last_dispose_snapshot}()` 已统一收口为 `tinyui_image_*`，改动范围覆盖 `widgets/image.c + test_tinyui_image`
  - 这一步继续保持 internal-only 边界：`image` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_image_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `graph` 当前也已继续完成一小批 residual internal/test seam：`tinyui_graph_backend()`、`tinyui_graph_backend_const()`、`tinyui_graph_get_ld()`、`tinyui_graph_get_ld_const()`、`tinyui_graph_props_are_valid()`、`tinyui_graph_apply_native_geometry_candidate()` 已统一收口为 `tinyui_graph_*`，改动范围覆盖 `widgets/graph.c + test_tinyui_graph`
  - 这一步继续保持 internal-only 边界：`graph` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_graph_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `arc` 当前也已继续完成一小批 residual internal/test seam：`struct tinyui_arc_test_dispose_snapshot`、`tinyui_arc_last_dispose_snapshot*`、`tinyui_arc_props_are_valid()`、`tinyui_arc_rgb_to_ld_color()`、`tinyui_arc_ld_color_to_rgb()`、`tinyui_arc_backend()`、`tinyui_arc_get_ld()`、`tinyui_arc_finish_detach_after_backend_failure()`、`tinyui_arc_dispose_partial_impl()` 与 `tinyui_backend_arc_test_{take_last_dispose_snapshot,create_with_props_fail_before_parent_color}()` 已统一收口为 `tinyui_arc_*`，改动范围覆盖 `widgets/arc.c + test_tinyui_arc`
  - 这一步继续保持 internal-only 边界：`arc` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_arc_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `clock` 当前也已继续完成一小批 residual internal seam：`tinyui_clock_props_are_valid()`、`tinyui_clock_get_ld()`、`tinyui_clock_apply_background()`、`tinyui_clock_apply_pointer()` 已统一收口为 `tinyui_clock_*`，改动范围覆盖 `widgets/clock.c + test_tinyui_clock`
  - 这一步继续保持 internal-only 边界：`clock` 这轮只收 widget-local/internal helper 名称，不触碰 public `tinyui_clock_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `message_box` 当前也已继续完成一小批 residual internal/test seam：`tinyui_message_box_props_are_valid()`、`tinyui_message_box_get_ld()`、`tinyui_message_box_confirm_bridge()` 已统一收口为 `tinyui_message_box_*`，改动范围覆盖 `widgets/message_box.c + test_tinyui_message_box`
  - 这一步继续保持 internal-only 边界：`message_box` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_message_box_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `animation` 当前也已继续完成一小批 residual internal/test seam：`tinyui_animation_get_ld()`、`tinyui_animation_props_are_valid()`、`tinyui_animation_attach_native()` 已统一收口为 `tinyui_animation_*`，改动范围覆盖 `widgets/animation.c + test_tinyui_animation`
  - 这一步继续保持 internal-only 边界：`animation` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_animation_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `scroll_selecter` 当前也已继续完成一小批 residual internal/test seam：`tinyui_scroll_selecter_props_are_valid()`、`tinyui_scroll_selecter_rgb_to_ld_color()`、`tinyui_scroll_selecter_backend_from_widget()`、`tinyui_scroll_selecter_ld_from_backend()`、`tinyui_scroll_selecter_selected_text_from_public_state()` 已统一收口为 `tinyui_scroll_selecter_*`，改动范围覆盖 `widgets/scroll_selecter.c + test_tinyui_scroll_selecter`
  - 这一步继续保持 internal-only 边界：`scroll_selecter` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_scroll_selecter_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `icon_slider` 当前也已继续完成一小批 residual internal/test seam：`tinyui_icon_slider_get_ld()`、`tinyui_icon_slider_backend_set_selected_index()`、`tinyui_icon_slider_native_slot()`、`tinyui_icon_slider_backend_add_item()`、`tinyui_icon_slider_backend_add_item_with_source()`、`tinyui_icon_slider_backend_get_selected_index()`、`tinyui_icon_slider_backend_set_horizontal()`、`tinyui_icon_slider_backend_get_horizontal()`、`tinyui_icon_slider_backend_set_speed()`、`tinyui_icon_slider_bind_host()`、`tinyui_icon_slider_props_are_valid()` 已统一收口为 `tinyui_icon_slider_*`，改动范围覆盖 `widgets/icon_slider.c + test_tinyui_icon_slider`
  - 这一步继续保持 internal-only 边界：`icon_slider` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_icon_slider_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `radial_menu` 当前也已继续完成一小批 residual internal/test seam：`tinyui_radial_menu_get_ld()`、`tinyui_radial_menu_backend_set_selected_index()`、`tinyui_radial_menu_native_slot()`、`tinyui_radial_menu_backend_add_item()`、`tinyui_radial_menu_backend_add_item_with_source()`、`tinyui_radial_menu_backend_get_selected_index()`、`tinyui_radial_menu_backend_offset_selection()`、`tinyui_radial_menu_backend_set_default_item()`、`tinyui_radial_menu_backend_click_item()`、`tinyui_radial_menu_backend_offset_item()`、`tinyui_radial_menu_bind_host()`、`tinyui_radial_menu_props_are_valid()` 已统一收口为 `tinyui_radial_menu_*`，改动范围覆盖 `widgets/radial_menu.c + test_tinyui_radial_menu`
  - 这一步继续保持 internal-only 边界：`radial_menu` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_radial_menu_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `canvas` 当前也已继续完成一小批 residual internal/test seam：`tinyui_canvas_rgb_to_ld()`、`tinyui_canvas_align_to_ld()`、`tinyui_canvas_push_native()`、`tinyui_canvas_clear_native()`、`tinyui_canvas_is_valid()` 已统一收口为 `tinyui_canvas_*`，改动范围覆盖 `widgets/canvas.c + test_tinyui_canvas`
  - 这一步继续保持 internal-only 边界：`canvas` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_canvas_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `background` 当前也已继续完成一小批 residual internal/test seam：`struct tinyui_background_backend_host`、`tinyui_background_get_root_size()` 已统一收口为 `tinyui_background_*`，改动范围覆盖 `widgets/background.c + test_tinyui_background`
  - 这一步继续保持 internal-only 边界：`background` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `tinyui_background_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名；到当前真相，`label/image/graph/arc/clock/message_box/animation/scroll_selecter/icon_slider/radial_menu/canvas/background` 这几条 retained internal seam 也已继续向 `tinyui_*` 收口，下一自然顺序已前移到仍残留较密集 `tinyui_*` widget-local seam 的控件批次，如 `list`
  - `list` 当前也已继续完成一小批 residual internal seam：`tinyui_list_rgb_to_ld_color()`、`tinyui_list_map_align()`、`tinyui_list_backend()`、`tinyui_list_ld_widget()` 以及 `tinyui_backend_list_{set_items,set_item_height,set_padding_group,set_margin_group,set_text_color,set_bg_color,set_select_color,set_align,set_item_widget}()` 已统一收口为 `tinyui_list_*`，改动范围覆盖 `widgets/list.c + test_tinyui_list`
  - 这一步继续保持 internal-only 边界：`list` 这轮只收 widget-local/internal helper 名称与 focused proof，不触碰 public `tinyui_list_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `label` 当前也已继续完成一小批 residual internal seam：`tinyui_backend_rgb_to_ld_color()`、`tinyui_backend_ld_color_to_rgb()`、`tinyui_backend_map_label_align()`、`tinyui_backend_unmap_label_align()` 已统一收口为 `tinyui_label_*`，改动范围覆盖 `widgets/label.c + test_tinyui_label`
  - 这一步继续保持 internal-only 边界：`label` 这轮只收 widget-local/internal helper 名称与 focused proof，不触碰 public `tinyui_label_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `button` 当前也已继续完成一小批 residual internal seam：`tinyui_button_alloc()`、`tinyui_button_set_event()` 已统一收口为 `tinyui_button_*`，改动范围覆盖 `widgets/button.c + test_tinyui_button_events`
  - 这一步继续保持 internal-only 边界：`button` 这轮只收 widget-local/internal helper 名称与 focused proof，不触碰 public `tinyui_button_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `core/app` 当前也已继续完成一小批 residual internal seam：`tinyui_app_timer_unlink()` 已统一收口为 `tinyui_app_timer_unlink()`，改动范围覆盖 `core/app.c + test_tinyui_app_timer`
  - 这一步继续保持 internal-only 边界：`core/app` 这轮只收单个 static helper 名称与 focused proof，不触碰 public `tinyui_app_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `core/widget` 当前也已继续完成一小批 residual internal seam：`tinyui_align_to_ld_horizontal()`、`tinyui_align_to_ld_vertical()`、`tinyui_rect_to_ld_region()`、`tinyui_ld_location_to_arm()`、`tinyui_ld_location_from_arm()`、`tinyui_ld_region_to_arm()`、`tinyui_ld_region_from_arm()`、`tinyui_rect_from_ld_region()`、`tinyui_widget_type_from_backend_kind()` 已统一收口为 `tinyui_*`，改动范围覆盖 `core/widget.c + test_tinyui_widgets`
  - 这一步继续保持 internal-only 边界：`core/widget` 这轮只收 pure-static geometry/type helper 名称与 focused proof，不触碰 public `tinyui_widget_*` / `tinyui_rect_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `core/runtime` 当前也已继续完成一小批 residual internal seam：`g_tinyui_runtime_app` 已统一收口为 `g_tinyui_runtime_app`，改动范围覆盖 `core/runtime.c + test_tinyui_runtime_model`
  - 这一步继续保持 internal-only 边界：`core/runtime` 这轮只收单个 static runtime state 名称与 focused proof，不触碰 public `tinyui_init()` / `tinyui_deinit()` / `tinyui_screen_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `radial_menu` 当前也已继续完成一小批 residual internal seam：`tinyui_radial_menu_create_with_backend_config()` 已统一收口为 `tinyui_radial_menu_create_with_backend_config()`，改动范围覆盖 `widgets/radial_menu.c + test_tinyui_radial_menu`
  - 这一步继续保持 internal-only 边界：`radial_menu` 这轮只收单个 widget-local create helper 名称与 focused proof，不触碰 public `tinyui_radial_menu_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `icon_slider` 当前也已继续完成一小批 residual internal seam：`tinyui_icon_slider_create_with_backend_config()` 已统一收口为 `tinyui_icon_slider_create_with_backend_config()`，改动范围覆盖 `widgets/icon_slider.c + test_tinyui_icon_slider`
  - 这一步继续保持 internal-only 边界：`icon_slider` 这轮只收单个 widget-local create helper 名称与 focused proof，不触碰 public `tinyui_icon_slider_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `canvas` 当前也已继续完成一小批 residual internal seam：`tinyui_canvas_push()` 已统一收口为 `tinyui_canvas_push()`，改动范围覆盖 `widgets/canvas.c + test_tinyui_canvas`
  - 这一步继续保持 internal-only 边界：`canvas` 这轮只收单个 widget-local command-push helper 名称与 focused proof，不触碰 public `tinyui_canvas_*` API、`tests/tinyui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
  - `V3 Step 3` 的最后一批 shared/internal seam 也已完成：`tinyui_backend_widget_init_data_model()`、`tinyui_backend_widget_claim_focus()`、`tinyui_backend_widget_release_focus()` 与 `tinyui_app_pump_timers()` 已统一收口为 `tinyui_widget_init_data_model()`、`tinyui_widget_claim_backend_focus()`、`tinyui_widget_release_backend_focus()` 与 `tinyui_app_pump_timers()`，改动范围覆盖 `core/widget.c + core/event.c + core/runtime_bridge.c + core/app.c + core/runtime_host.c + backend.h`，以及直系 caller `combo_box/icon_slider/line_edit/radial_menu/table` 和 focused proof `test_tinyui_{widgets,app_timer,combo_box,icon_slider,line_edit,radial_menu,table,runtime_model}`
  - 到当前真相，`V3 Step 3` 这条 old internal seam rename 线已经收口：`static tinyui_*` / `g_tinyui_*` / retained `create_with_backend_config` / backend focus-data-model seam / internal timer pump 这类 shared/widget-local residual seam 在 `tinyui/src/{core,widgets}` 下已清零
  - 当前收口证据：`rg -n "static .*tinyui_|g_tinyui_|tinyui_[A-Za-z0-9_]+create_with_backend_config|tinyui_backend_widget_(claim_focus|release_focus|init_data_model)|tinyui_app_pump_timers" tinyui/src/core tinyui/src/widgets | sort` 已不再命中旧命名 residual；同时 `test_tinyui_{widgets,button_events,icon_slider,radial_menu,list,app_timer,runtime_model,label,canvas,combo_box,line_edit,table}` 这批 focused gate 已 fresh 通过

- [x] **Step 4: 跑 focused compile proof**

Run:

```bash
rtk cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build --target \
  test_tinyui_runtime_model \
  test_tinyui_layout \
  test_tinyui_theme \
  test_tinyui_native_bridge
```

Expected: PASS。

当前真相：

- 该 focused compile proof 已 fresh 通过
- 对应命令：

```bash
rtk cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build --target \
  test_tinyui_runtime_model \
  test_tinyui_layout \
  test_tinyui_theme \
  test_tinyui_native_bridge
```

### Task 2: 收口 shared layer target 与内部接口

**Files:**
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `tests/tinyui/CMakeLists.txt`

- [x] **Step 1: 更新 shared layer target naming**

Move product-layer targets toward `tinyui` naming while keeping the build graph intact.

当前真相：

- `cmake/LingDongGUI.cmake` 已建立 `tinyui_core`、`tinyui_backend_ldgui`、`tinyui_backend_ldgui_runtime`、`tinyui_port_sdl` 实名
- `tests/support/CMakeLists.txt` 已建立 `tinyui_test_support` 实名
- 为避免提前撞进 `V4` 的 test/demo/contract/archive 命名面，当前仍保留 `tinyui_*` CMake alias 兼容层
- `tests/tinyui/CMakeLists.txt` 的 `SUPPORT_LIB` / `MAIN_LIB` / `target_link_libraries(test_tinyui_port_sdl ...)` 已切到 `tinyui_*` 实名

- [x] **Step 2: 跑 runtime/perf smoke**

Run:

```bash
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS；说明 shared layer rename 没破坏 runtime/perf proof。

当前真相：

- 该步已 fresh 通过
- `test_tinyui_wrapper_struct_overhead` 已从 `perf` label 退回 `probe` label，`ctest -L perf` 不再因为未预构建裸 probe 出现假红
- `backend_widget_struct_bytes` 已从回归态 `536` 压回 `464`，重新低于当前 perf baseline gate `<=512`
- `rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure` 已于 2026-06-12 fresh 全绿
- `rtk ctest --test-dir build -L 'perf' --output-on-failure` 已于 2026-06-12 fresh 全绿

- [x] **Step 3: 更新 V3 closeout 文档**

Document:

- shared layers are now product-layer `tinyui` subsystems
- shared layer retention is intentional and LVGL-like
- backend removal did not collapse shared responsibilities into widgets

当前待办边界：

- 这一步已经完成，当前 closeout 文档落在 `docs/v2.1/v2.1-closeout.md`
- 文档当前明确记录：
  - shared layer 已按产品层 `tinyui` 子系统收口
  - shared 层保留是有意设计，不是 backend 清零后的残留杂质
  - backend removal 没有把 shared responsibilities 粗暴塌缩进 widgets
  - 当前只是 `V3` closeout truth，不等同于 `v2.1` 全阶段最终 release closeout
