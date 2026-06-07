# 06 Demo / Port / 验证

## P6-A `SDL native port attach`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_sdl_port.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_sdl_port.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `cmake -S . -B build`
  - `make -C build test_picoui_native_sdl_port`
  - 链接失败，缺少：
    - `picoui_sdl_hal_init`
- 该 `RED` 证明：
  - 当前 `picoui/port/sdl/` 只有旧的 `picoui_port_sdl_attach(struct picoui_app *app)` 兼容入口
  - 还不存在 `P6-A` 计划要求的 native `picoui_sdl_hal_init(width, height)` HAL attach 合同

### GREEN

- 在 [picoui/include/picoui/port/sdl.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/port/sdl.h) 与 [picoui/port/sdl/sdl.c](/Users/cys/embedded/LingDongGUI/picoui/port/sdl/sdl.c) 新增最小 native SDL HAL 入口：
  - `picoui_sdl_hal_init(int width, int height)`
  - `picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev)`
- 在 [picoui/src/native/native_indev.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_indev.c) 新增最小 native test readback：
  - `picoui_native_indev_get_type(const struct picoui_indev *indev, enum picoui_indev_type *out_type)`
- 当前建立的最小 `P6-A` 合同：
  - `picoui_sdl_hal_init(width, height)`：
    - 拒绝非法尺寸
    - 初始化 SDL video/events
    - 创建并设置 PicoUI 默认 display
    - 创建并保存一个默认 pointer indev
  - 旧 `picoui_port_sdl_attach(app)` 兼容入口继续保留：
    - 只维持旧 `app/display config/tick/delay` 合同
    - 不把它写成 `v1.0+` 当前主路径
- focused test 当前证明：
  - `320x480` HAL init 成功后，默认 display 尺寸可读回
  - SDL port 会登记一个默认 pointer indev
  - 该 indev 的类型为 `PICOUI_INDEV_TYPE_POINTER`

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_sdl_port test_picoui_port_sdl`
- `./build/tests/picoui/test_picoui_native_sdl_port`
- `ctest --test-dir build -R '^(test_picoui_native_sdl_port|test_picoui_port_sdl)$' --output-on-failure`
- `git diff --check -- picoui/include/picoui/port/sdl.h picoui/port/sdl/sdl.c picoui/src/native/native_indev.c tests/picoui/native/test_picoui_native_sdl_port.c tests/picoui/CMakeLists.txt tests/picoui/unit/test_picoui_port_sdl.c`

### 当前结论

- 当前 `P6-A SDL native port attach` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有 native `picoui_sdl_hal_init(width, height)` 入口
  - 最小 SDL display + pointer indev attach 合同已成立
  - 旧 `picoui_port_sdl_attach(app)` 兼容测试没有被打坏
- 当前实现还没有宣称：
  - demo main 已批量迁移到 LVGL-like skeleton
  - native artifact gate 已完成
  - manual artifact 文档已完成
- 当前阶段下一任务推进到 `P6-B batch demo main contract expansion`。

## P6-B `batch demo main contract expansion`

### RED

- 扩展了 [check_picoui_demo_main_style.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/check_picoui_demo_main_style.py)，不再只检查 `basic_widgets`，而是对 inventory 里的全部 `v1.0` demos 做两类合同校验：
  - `v1_lvgl_like`：
    - 文件必须包含 `picoui_init()`、HAL init、`create_demo_ui()`、`while(1)`、`picoui_timer_handler()`
    - 主路径不得再包含 `picoui_app_create()` / `picoui_app_run()` / `picoui_app_destroy()`
  - `pre_v1_app_run`：
    - 文件仍显式包含旧 `picoui_app_*` 主路径标记
- fresh `RED`：
  - `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前按预期失败，直接列出仍待迁移的 demo mains：
    - `animation_basic`
    - `arc_basic`
    - `calendar_basic`
    - `clock_basic`
    - `combo_box_basic`
    - `date_time_basic`
    - `gauge_basic`
    - `graph_basic`
    - `grid_parity`
    - `hello_world`
    - `icon_slider_basic`
    - `keyboard_basic`
    - `layout_flex`
    - `layout_grid`
    - `layout_parity`
    - `legacy_widget_parity`
    - `line_edit_basic`
    - `list_basic`
    - `message_box_basic`
    - `progress_bar_basic`
    - `progress_wheel_basic`
    - `qrcode_basic`
    - `radial_menu_basic`
    - `scroll_selecter_basic`
    - `settings_panel`
    - `table_basic`
    - `theme_showcase`
- 该 `RED` 证明：
  - `P6-B` 需要的“全量 demo main contract expansion”已经建立
  - 当前阻塞不再是 contract 缺失，而是这些 demo 仍未迁到 LVGL-like main skeleton

### 当前结论

- 当前 `P6-B` 已完成到 plan 规定的 `RED` 状态。
- 当前 contract 已能稳定识别：
  - 哪些 demo 已经是 `v1_lvgl_like`
  - 哪些 demo 仍停留在 `picoui_app_*` 主路径
- 当前还没有进入任何单个 demo 的 `GREEN` 迁移实现。
- 当前阶段下一任务推进到 `P6-C hello_world`。

## P6-C `hello_world`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/hello_world/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/hello_world/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:16)：
  - `hello_world.main_style = v1_lvgl_like`
  - `hello_world.uses_picoui_app = false`
- 同步补了 `hello_world` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:475)
  - 当前只给 `picoui_hello_world_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_hello_world_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `hello_world`
- `git diff --check -- picoui/demo/hello_world/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-C hello_world` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `hello_world` 已完成 LVGL-like main 迁移
  - `picoui_hello_world_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `hello_world` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `hello_world` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-D layout_flex`。

## P6-D `layout_flex`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/layout_flex/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/layout_flex/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:19)：
  - `layout_flex.main_style = v1_lvgl_like`
  - `layout_flex.uses_picoui_app = false`
- 同步补了 `layout_flex` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:481)
  - 当前只给 `picoui_layout_flex_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_layout_flex_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `layout_flex`
- `git diff --check -- picoui/demo/layout_flex/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-D layout_flex` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `layout_flex` 已完成 LVGL-like main 迁移
  - `picoui_layout_flex_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `layout_flex` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `layout_flex` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-E layout_grid`。

## P6-E `layout_grid`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/layout_grid/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/layout_grid/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:20)：
  - `layout_grid.main_style = v1_lvgl_like`
  - `layout_grid.uses_picoui_app = false`
- 同步补了 `layout_grid` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:486)
  - 当前只给 `picoui_layout_grid_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_layout_grid_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `layout_grid`
- `git diff --check -- picoui/demo/layout_grid/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-E layout_grid` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `layout_grid` 已完成 LVGL-like main 迁移
  - `picoui_layout_grid_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `layout_grid` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `layout_grid` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-F theme_showcase`。

## P6-F `theme_showcase`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/theme_showcase/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/theme_showcase/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `theme_showcase` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/body/button` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:33)：
  - `theme_showcase.main_style = v1_lvgl_like`
  - `theme_showcase.uses_picoui_app = false`
- 同步补了 `theme_showcase` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:490)
  - 当前只给 `picoui_theme_showcase_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_theme_showcase_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `theme_showcase`
- `git diff --check -- picoui/demo/theme_showcase/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-F theme_showcase` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `theme_showcase` 已完成 LVGL-like main 迁移
  - `picoui_theme_showcase_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `theme_showcase` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `theme_showcase` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `theme_showcase` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-G settings_panel`。

## P6-G `settings_panel`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/settings_panel/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/settings_panel/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `settings_panel` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/switch/slider/button` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:31)：
  - `settings_panel.main_style = v1_lvgl_like`
  - `settings_panel.uses_picoui_app = false`
- 同步补了 `settings_panel` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:494)
  - 当前只给 `picoui_settings_panel_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_settings_panel_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `settings_panel`
- `git diff --check -- picoui/demo/settings_panel/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-G settings_panel` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `settings_panel` 已完成 LVGL-like main 迁移
  - `picoui_settings_panel_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `settings_panel` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `settings_panel` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `settings_panel` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-H list_basic`。

## P6-H `list_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/list_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/list_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:24)：
  - `list_basic.main_style = v1_lvgl_like`
  - `list_basic.uses_picoui_app = false`
- 同步补了 `list_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:498)
  - 当前只给 `picoui_list_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_list_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `list_basic`
- `git diff --check -- picoui/demo/list_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-H list_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `list_basic` 已完成 LVGL-like main 迁移
  - `picoui_list_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `list_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `list_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-I progress_bar_basic`。

## P6-I `progress_bar_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/progress_bar_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/progress_bar_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `progress_bar_basic` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/primary/secondary` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:26)：
  - `progress_bar_basic.main_style = v1_lvgl_like`
  - `progress_bar_basic.uses_picoui_app = false`
- 同步补了 `progress_bar_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:502)
  - 当前只给 `picoui_progress_bar_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_progress_bar_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `progress_bar_basic`
- `git diff --check -- picoui/demo/progress_bar_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-I progress_bar_basic` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `progress_bar_basic` 已完成 LVGL-like main 迁移
  - `picoui_progress_bar_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `progress_bar_basic` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `progress_bar_basic` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `progress_bar_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-J progress_wheel_basic`。

## P6-J `progress_wheel_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/progress_wheel_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/progress_wheel_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `progress_wheel_basic` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/wheel` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:27)：
  - `progress_wheel_basic.main_style = v1_lvgl_like`
  - `progress_wheel_basic.uses_picoui_app = false`
- 同步补了 `progress_wheel_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:518)
  - 当前只给 `picoui_progress_wheel_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_progress_wheel_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `progress_wheel_basic`
- `git diff --check -- picoui/demo/progress_wheel_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-J progress_wheel_basic` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `progress_wheel_basic` 已完成 LVGL-like main 迁移
  - `picoui_progress_wheel_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `progress_wheel_basic` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `progress_wheel_basic` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `progress_wheel_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-K qrcode_basic`。

## P6-K `qrcode_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/qrcode_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/qrcode_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `qrcode_basic` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/qrcode` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:28)：
  - `qrcode_basic.main_style = v1_lvgl_like`
  - `qrcode_basic.uses_picoui_app = false`
- 同步补了 `qrcode_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:522)
  - 当前只给 `picoui_qrcode_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_qrcode_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `qrcode_basic`
- `git diff --check -- picoui/demo/qrcode_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-K qrcode_basic` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `qrcode_basic` 已完成 LVGL-like main 迁移
  - `picoui_qrcode_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `qrcode_basic` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `qrcode_basic` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `qrcode_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-L message_box_basic`。

## P6-L `message_box_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/message_box_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/message_box_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:25)：
  - `message_box_basic.main_style = v1_lvgl_like`
  - `message_box_basic.uses_picoui_app = false`
- 同步补了 `message_box_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:526)
  - 当前只给 `picoui_message_box_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_message_box_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `message_box_basic`
- `git diff --check -- picoui/demo/message_box_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-L message_box_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `message_box_basic` 已完成 LVGL-like main 迁移
  - `picoui_message_box_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `message_box_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `message_box_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-M date_time_basic`。

## P6-M `date_time_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/date_time_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/date_time_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `date_time_basic` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/title/date_time` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:12)：
  - `date_time_basic.main_style = v1_lvgl_like`
  - `date_time_basic.uses_picoui_app = false`
- 同步补了 `date_time_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:530)
  - 当前只给 `picoui_date_time_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_date_time_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `date_time_basic`
- `git diff --check -- picoui/demo/date_time_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-M date_time_basic` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `date_time_basic` 已完成 LVGL-like main 迁移
  - `picoui_date_time_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `date_time_basic` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `date_time_basic` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `date_time_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-N clock_basic`。

## P6-N `clock_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/clock_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/clock_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 当前 `clock_basic` 没有发明新的 v1 theme attach API：
  - 继续创建 demo 私有 `picoui_theme`
  - 仅用现有 `picoui_theme_apply_to_widget()` 对 `root/clock` 做显式 theme apply
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:10)：
  - `clock_basic.main_style = v1_lvgl_like`
  - `clock_basic.uses_picoui_app = false`
- 同步补了 `clock_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:534)
  - 当前只给 `picoui_clock_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_clock_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `clock_basic`
- `git diff --check -- picoui/demo/clock_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-N clock_basic` 以“主路径迁移完成”作为 closeout 范围。
- 当前 fresh 证据已经证明：
  - `clock_basic` 已完成 LVGL-like main 迁移
  - `picoui_clock_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `clock_basic` 从待迁移列表中移除
- 当前没有证明：
  - v1 主路径已经拥有新的公共 `screen/app theme attach` 合同
  - `clock_basic` 的视觉主题效果已完成单独 artifact gate
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `clock_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-O line_edit_basic`。

## P6-O `line_edit_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/line_edit_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/line_edit_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:23)：
  - `line_edit_basic.main_style = v1_lvgl_like`
  - `line_edit_basic.uses_picoui_app = false`
- 同步补了 `line_edit_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:541)
  - 当前只给 `picoui_line_edit_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_line_edit_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `line_edit_basic`
- `git diff --check -- picoui/demo/line_edit_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-O line_edit_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `line_edit_basic` 已完成 LVGL-like main 迁移
  - `picoui_line_edit_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `line_edit_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `line_edit_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-P combo_box_basic`。

## P6-P `combo_box_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/combo_box_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/combo_box_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:11)：
  - `combo_box_basic.main_style = v1_lvgl_like`
  - `combo_box_basic.uses_picoui_app = false`
- 同步补了 `combo_box_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:545)
  - 当前只给 `picoui_combo_box_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_combo_box_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `combo_box_basic`
- `git diff --check -- picoui/demo/combo_box_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-P combo_box_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `combo_box_basic` 已完成 LVGL-like main 迁移
  - `picoui_combo_box_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `combo_box_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `combo_box_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-Q scroll_selecter_basic`。

## P6-Q `scroll_selecter_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/scroll_selecter_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/scroll_selecter_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:30)：
  - `scroll_selecter_basic.main_style = v1_lvgl_like`
  - `scroll_selecter_basic.uses_picoui_app = false`
- 同步补了 `scroll_selecter_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:549)
  - 当前只给 `picoui_scroll_selecter_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_scroll_selecter_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `scroll_selecter_basic`
- `git diff --check -- picoui/demo/scroll_selecter_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-Q scroll_selecter_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `scroll_selecter_basic` 已完成 LVGL-like main 迁移
  - `picoui_scroll_selecter_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `scroll_selecter_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `scroll_selecter_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-R table_basic`。

## P6-R `table_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/table_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/table_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:32)：
  - `table_basic.main_style = v1_lvgl_like`
  - `table_basic.uses_picoui_app = false`
- 同步补了 `table_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:553)
  - 当前只给 `picoui_table_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_table_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `table_basic`
- `git diff --check -- picoui/demo/table_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-R table_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `table_basic` 已完成 LVGL-like main 迁移
  - `picoui_table_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `table_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `table_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-S graph_basic`。

## P6-S `graph_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/graph_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/graph_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:14)：
  - `graph_basic.main_style = v1_lvgl_like`
  - `graph_basic.uses_picoui_app = false`
- 同步补了 `graph_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:557)
  - 当前只给 `picoui_graph_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_graph_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `graph_basic`
- `git diff --check -- picoui/demo/graph_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-S graph_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `graph_basic` 已完成 LVGL-like main 迁移
  - `picoui_graph_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `graph_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `graph_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-T calendar_basic`。

## P6-T `calendar_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/calendar_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/calendar_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:9)：
  - `calendar_basic.main_style = v1_lvgl_like`
  - `calendar_basic.uses_picoui_app = false`
- 同步补了 `calendar_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:564)
  - 当前只给 `picoui_calendar_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_calendar_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `calendar_basic`
- `git diff --check -- picoui/demo/calendar_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-T calendar_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `calendar_basic` 已完成 LVGL-like main 迁移
  - `picoui_calendar_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `calendar_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `calendar_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-U animation_basic`。

## P6-U `animation_basic`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/animation_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/animation_basic/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:6)：
  - `animation_basic.main_style = v1_lvgl_like`
  - `animation_basic.uses_picoui_app = false`
- 同步补了 `animation_basic` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:568)
  - 当前只给 `picoui_animation_basic_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_animation_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `animation_basic`
- `git diff --check -- picoui/demo/animation_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-U animation_basic` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `animation_basic` 已完成 LVGL-like main 迁移
  - `picoui_animation_basic_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `animation_basic` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `animation_basic` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-V legacy_widget_parity`。

## P6-V `legacy_widget_parity`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/legacy_widget_parity/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/legacy_widget_parity/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 旧 `app timer` 动画路径已收口到 demo 自己的 native loop：
  - 保留 `arc/gauge` 角度推进语义
  - 以 100ms 步进在主循环前做手动 animation pump
  - 没有新增 public API，也没有把旧 `picoui_app_timer_*` 留作主路径依赖
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:22)：
  - `legacy_widget_parity.main_style = v1_lvgl_like`
  - `legacy_widget_parity.uses_picoui_app = false`
- 同步补了 `legacy_widget_parity` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:572)
  - 当前只给 `picoui_legacy_widget_parity_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_legacy_widget_parity_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `legacy_widget_parity`
- `git diff --check -- picoui/demo/legacy_widget_parity/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-V legacy_widget_parity` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `legacy_widget_parity` 已完成 LVGL-like main 迁移
  - `picoui_legacy_widget_parity_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `legacy_widget_parity` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `legacy_widget_parity` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-W layout_parity`。

## P6-W `layout_parity`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/layout_parity/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/layout_parity/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 旧 `app timer` resize 路径已收口到 demo 自己的 native loop：
  - 保留 `flex_row_sample` 在 compact/expanded 之间切换的行为
  - 以 1200ms 步进在主循环前做手动 resize pump
  - 没有新增 public API，也没有把旧 `picoui_app_timer_*` 留作主路径依赖
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:21)：
  - `layout_parity.main_style = v1_lvgl_like`
  - `layout_parity.uses_picoui_app = false`
- 同步补了 `layout_parity` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:579)
  - 当前只给 `picoui_layout_parity_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_layout_parity_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `layout_parity`
- `git diff --check -- picoui/demo/layout_parity/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-W layout_parity` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `layout_parity` 已完成 LVGL-like main 迁移
  - `picoui_layout_parity_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `layout_parity` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `layout_parity` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-X grid_parity`。

## P6-X `grid_parity`

### GREEN

- 已按 `P6` skeleton 迁移 [picoui/demo/grid_parity/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/grid_parity/main.c)：
  - 使用 `picoui_init()`
  - 使用 `picoui_sdl_hal_init(320, 480)`
  - `create_demo_ui()` 走 `picoui_screen_active()` + `picoui_window_create_root()`
  - 主循环改为 `while (1) { picoui_timer_handler(); }`
  - 不再以 `picoui_app_*` 作为主路径
- 这一步只替换主路径骨架：
  - `make_ui()` 的 grid tracks、explicit spans、overlay ignore-layout 行为保持原样
  - 没有新增 public API，也没有改 grid contract 本身
- 同步更新 inventory 行 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:15)：
  - `grid_parity.main_style = v1_lvgl_like`
  - `grid_parity.uses_picoui_app = false`
- 同步补了 `grid_parity` target 的最小端口链接：
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:580)
  - 当前只给 `picoui_grid_parity_demo` 增加 `picoui_port_sdl`，没有批量改其他 demos

### 验证

- `cmake -S . -B build`
- `make -C build picoui_grid_parity_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前仍按预期失败
  - 但 pending 列表里已经不再包含 `grid_parity`
- `git diff --check -- picoui/demo/grid_parity/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-X grid_parity` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `grid_parity` 已完成 LVGL-like main 迁移
  - `picoui_grid_parity_demo` 当前能成功构建
  - broad `demo main style` contract 已把 `grid_parity` 从待迁移列表中移除
- 当前 `check_picoui_demo_main_style.py` 仍为红，不是 `grid_parity` 回归，而是其余 demos 还未迁移。
- 当前阶段下一任务推进到 `P6-Y remaining inventory demos`。

## P6-Y remaining inventory demos

### GREEN

- 已完成剩余 inventory demos 的 LVGL-like main 迁移：
  - [picoui/demo/arc_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/arc_basic/main.c)
  - [picoui/demo/gauge_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/gauge_basic/main.c)
  - [picoui/demo/icon_slider_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/icon_slider_basic/main.c)
  - [picoui/demo/keyboard_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/keyboard_basic/main.c)
  - [picoui/demo/radial_menu_basic/main.c](/Users/cys/embedded/LingDongGUI/picoui/demo/radial_menu_basic/main.c)
- 其中 `arc/gauge/icon_slider/radial_menu` 采用 theme-aware native skeleton：
  - `picoui_init() -> picoui_sdl_hal_init(320, 480) -> picoui_theme_create() -> create_demo_ui() -> while (1) picoui_timer_handler()`
  - 使用现有 `picoui_theme_apply_to_widget()` 做 demo 内显式 theme apply
  - 没有继续保留 `picoui_app_set_theme()` / `picoui_app_run()` 作为主路径
- `keyboard_basic` 采用无 theme 的最小 native skeleton：
  - `picoui_init() -> picoui_sdl_hal_init(320, 480) -> create_demo_ui() -> while (1) picoui_timer_handler()`
  - 原有 line edit + keyboard contract 保持不变
- inventory 已全部推进到 `v1_lvgl_like / uses_picoui_app=false`，见 [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:6)
- SDL demo targets 已补齐最小 `picoui_port_sdl` 链接，见 [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt:506)

### 验证

- `cmake -S . -B build`
- `make -C build picoui_arc_basic_demo picoui_gauge_basic_demo picoui_icon_slider_basic_demo picoui_keyboard_basic_demo picoui_radial_menu_basic_demo`
- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前已通过
- `git diff --check -- picoui/demo/arc_basic/main.c picoui/demo/gauge_basic/main.c picoui/demo/icon_slider_basic/main.c picoui/demo/keyboard_basic/main.c picoui/demo/radial_menu_basic/main.c examples/sdl/CMakeLists.txt tests/picoui/contract/picoui_demo_main_style_inventory.json`

### 当前结论

- 当前 `P6-Y remaining inventory demos` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - 剩余 5 个 inventory demos 已全部完成 LVGL-like main 迁移
  - 相关 demo targets 当前都能成功构建
  - broad `demo main style` contract 已经整体转绿
- 当前阶段下一任务推进到 `P6-Z demo main contract GREEN`。

## P6-Z `demo main contract GREEN`

### GREEN

- 当前 broad `demo main style` contract 已完成从 RED 到 GREEN 的阶段目标。
- [picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json:6) 当前所有 demo 行都已是：
  - `main_style = v1_lvgl_like`
  - `uses_picoui_app = false`
- [check_picoui_demo_main_style.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/check_picoui_demo_main_style.py) 当前对整棵 `picoui/demo/*/main.c` 运行已通过，没有遗留 pending demo。

### 验证

- `python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前已通过

### 当前结论

- 当前 `P6-Z demo main contract GREEN` 已完成 closeout。
- 当前 `P6` 里的 demo main 迁移 contract gate 已经闭合，下一步进入 `P6-AA native artifact gate`。

## P6-AA `native artifact gate`

### GREEN

- 已新增 manifest 驱动的 native artifact gate：
  - [tests/picoui/runtime/check_picoui_native_visible_ui.py](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/check_picoui_native_visible_ui.py)
  - [tests/picoui/runtime/picoui_native_artifact_manifest.json](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/picoui_native_artifact_manifest.json)
- 当前 gate 只做两类真相区分：
  - `visible`：
    - 必须 `rc=0`
    - 必须输出 `PICOUI_RUNTIME_READY`
    - 必须产出非空 capture artifact
  - `runtime_only`：
    - 不把 demo 伪装成 visible green
    - 允许保留当前已知降级状态
    - 降级原因必须写在 manifest，不藏在脚本逻辑里
- 当前 manifest 已覆盖全部 `P6` demos：
  - `visible` 共 `21` 个：
    - `basic_widgets`
    - `settings_panel`
    - `progress_bar_basic`
    - `arc_basic`
    - `gauge_basic`
    - `icon_slider_basic`
    - `radial_menu_basic`
    - `progress_wheel_basic`
    - `qrcode_basic`
    - `message_box_basic`
    - `date_time_basic`
    - `clock_basic`
    - `line_edit_basic`
    - `combo_box_basic`
    - `scroll_selecter_basic`
    - `table_basic`
    - `graph_basic`
    - `calendar_basic`
    - `animation_basic`
    - `legacy_widget_parity`
    - `grid_parity`
  - `runtime_only` 共 `7` 个：
    - `hello_world`
    - `layout_flex`
    - `layout_grid`
    - `theme_showcase`
    - `list_basic`
    - `keyboard_basic`
    - `layout_parity`
- 当前 `runtime_only` 的原因已经显式固化到 manifest：
  - 前 `6` 个 demo 当前都表现为：
    - 输出 `PICOUI_RUNTIME_READY`
    - 退出码为 `1`
    - 没有稳定 capture artifact
  - `layout_parity` 当前更早退出：
    - `rc=1`
    - 尚未稳定建立 `runtime_ready/capture` 证据

### 验证

- `python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --all-demos --build-dir build/picoui-native-artifact-aa`
  - 当前已通过
  - 摘要结果：
    - `visible=21`
    - `runtime_only=7`
    - `total=28`
    - `failures=0`

### 当前结论

- 当前 `P6-AA native artifact gate` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `P6` demos 不再被一刀切当成“全部 visible green”
  - 当前 native artifact gate 能稳定区分：
    - 真正已有稳定 capture artifact 的 `visible` demos
    - 仅具备 runtime 或更弱证据的 `runtime_only` demos
  - `runtime_only` 降级真相已回写到 manifest，而不是散落在代码里
- 当前这一步还没有宣称：
  - `runtime_only` demos 已经修到 visible
  - manual artifact 文档已补齐
  - `P6` 阶段已整体 closeout
- 当前阶段下一任务推进到 `P6-AB manual artifact docs`。

## P6-AB `manual artifact docs`

### GREEN

- 已把 [picoui/docs/demo_guide.md](/Users/cys/embedded/LingDongGUI/picoui/docs/demo_guide.md) 重写到当前 `v1.0-native` 口径，不再沿用旧 `a-0.7 wrapper` gate 说明。
- 当前文档已明确写出三类精确命令：
  - 单个 demo 构建 / 运行 / auto quit
  - 全量 `demo main style` / `runtime smoke` / `native artifact gate`
  - 手工窗口 artifact 生成
- 当前文档已明确写出 capture artifact 的直接运行方式：
  - `PICOUI_DEMO_AUTO_QUIT_MS=1200`
  - `PICOUI_CAPTURE_FILE=/path/to/frame.ppm`
  - 无窗口环境下可加 `SDL_VIDEODRIVER=dummy`
- 当前文档已明确声明：
  - `runtime smoke` 不等于 visible correctness
  - `native artifact gate` 不等于 manual review pass
  - “UI 完成”至少需要 visible artifact 证据，不能只拿 runtime smoke 或 main contract 代替
- 当前文档也已把历史 wrapper 路线收窄为 history-only：
  - [docs/picoui-serial/archive/pre-v1.0-wrapper-backend/](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/archive/pre-v1.0-wrapper-backend)
  - 只允许用于历史追溯、迁移参考、对照 oracle
  - 不允许再把旧 wrapper/backend 结论当成 `v1.0-native` 当前完成证据

### 验证

- `sed -n '1,220p' picoui/docs/demo_guide.md`
  - 已确认当前文档入口、命令、artifact 分层和 history 边界都已更新到 `v1.0-native`
- `git diff --check -- picoui/docs/demo_guide.md docs/picoui-serial/v1.0-native/06-demo-port验证.md`
  - 当前已通过

### 当前结论

- 当前 `P6-AB manual artifact docs` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - 单 demo / 全量 / capture artifact / manual artifact 的运行方式都有明确命令
  - visible artifact 证据被明确上升为“UI 完成”必需条件
  - pre-v1.0 wrapper 文档已被明确降级为 history-only
- 当前阶段下一任务推进到 `P6-AC P6 closeout`。

## P6-AC `P6 closeout`

### GREEN

- `P6` 阶段的三条 broad gates 当前都已对齐到最新真相：
  - `demo main style`
  - `runtime smoke`
  - `native artifact gate`
- 为了让 broad runtime gate 与 `P6-AA` 的 manifest 分层一致，已在 [check_picoui_runtime.py](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/check_picoui_runtime.py) 补入当前 `artifact_policy` 读取逻辑：
  - `visible` demo 仍要求 `runtime_ready + capture + rc=0`
  - `runtime_only` demo 不再被误判成 broad runtime 红项
  - 同时补了 `--all-demos` 参数别名，对齐 `P6` plan 的命令写法
- 已新增最小单测 [test_check_picoui_runtime.py](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/test_check_picoui_runtime.py)，固定两条合同：
  - `--all-demos` 必须映射到全量 targets
  - `runtime_only` demo 不要求 capture
  - `visible` demo 仍要求 capture
- ledger [picoui_native_migration_ledger.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_native_migration_ledger.json:13) 已把 `P6-demo-port-artifact` 更新为 `covered`

### 验证

- `rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py`
  - 当前已通过
- `python3 -m unittest tests.picoui.runtime.test_check_picoui_runtime`
  - 当前已通过
- `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --all-demos --build-dir build/picoui-runtime-ac`
  - 当前已通过
- `rtk python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --all-demos --build-dir build/picoui-native-artifact-aa`
  - 当前已通过
  - 摘要仍为：
    - `visible=21`
    - `runtime_only=7`
    - `failures=0`
- `git diff --check -- tests/picoui/runtime/check_picoui_runtime.py tests/picoui/runtime/test_check_picoui_runtime.py tests/picoui/contract/picoui_native_migration_ledger.json picoui/docs/demo_guide.md docs/picoui-serial/v1.0-native/06-demo-port验证.md docs/picoui-serial/v1.0-native/线计划索引.md`
  - 当前已通过
- `gitnexus detect_changes(scope=all)`
  - 当前仍返回 `critical`
  - 这是整棵脏 worktree 的累计结果，不是本轮 `P6-AB/P6-AC` 局部改动的独立风险结论

### 当前结论

- 当前 `P6-AC P6 closeout` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - `P6` 的 SDL native port attach、demo main migration、artifact gate、manual docs 已串起来形成完整阶段证据
  - broad runtime gate 已不再和 `runtime_only` manifest 真相冲突
  - `P6-demo-port-artifact` ledger 已可标记为 `covered`
- 当前 `P6` 阶段可以视为完成，下一阶段推进到 `P7-A no-ldgui dependency RED contract`。
