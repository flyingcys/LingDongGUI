# PicoUI Native Demo 运行与验证指南

本文是 `v1.0-native` 路线下 `picoui/demo/*` 的当前运行与验证入口。

当前真相源：

- [docs/picoui-serial/README.md](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/README.md)
- [docs/picoui-serial/v1.0-native/线计划索引.md](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/v1.0-native/%E7%BA%BF%E8%AE%A1%E5%88%92%E7%B4%A2%E5%BC%95.md)
- [docs/picoui-serial/v1.0-native/06-demo-port验证.md](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/v1.0-native/06-demo-port%E9%AA%8C%E8%AF%81.md)
- [tests/picoui/contract/picoui_demo_main_style_inventory.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_demo_main_style_inventory.json)
- [tests/picoui/runtime/picoui_native_artifact_manifest.json](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/picoui_native_artifact_manifest.json)

旧 `pre-v1.0 wrapper` 文档只作为历史参考，不再作为当前实施口径：

- [docs/picoui-serial/archive/pre-v1.0-wrapper-backend/](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/archive/pre-v1.0-wrapper-backend)

## 1. 当前路线和边界

`v1.0+` 的 PicoUI demo 走的是 native-only 路线：

```text
PicoUI public API -> PicoUI native runtime/widgets -> ARM-2D -> PicoUI port/display
```

当前 `picoui/demo/*/main.c` 已全部迁移到 LVGL-like skeleton：

- `picoui_init()`
- `picoui_sdl_hal_init(width, height)`
- `picoui_screen_active()`
- `picoui_window_create_root()`
- `while (1) { picoui_timer_handler(); }`

`picoui_app_*` 不再是 `v1.0+` demo 的主路径。

## 2. Demo 目标名

`picoui/demo` 不走顶层 `USE_DEMO` 单入口，而是在 `examples/sdl/CMakeLists.txt` 中按独立 target 构建。

当前 demo targets：

- `picoui_hello_world_demo`
- `picoui_basic_widgets_demo`
- `picoui_layout_flex_demo`
- `picoui_layout_grid_demo`
- `picoui_theme_showcase_demo`
- `picoui_settings_panel_demo`
- `picoui_list_basic_demo`
- `picoui_progress_bar_basic_demo`
- `picoui_arc_basic_demo`
- `picoui_gauge_basic_demo`
- `picoui_icon_slider_basic_demo`
- `picoui_radial_menu_basic_demo`
- `picoui_progress_wheel_basic_demo`
- `picoui_qrcode_basic_demo`
- `picoui_message_box_basic_demo`
- `picoui_date_time_basic_demo`
- `picoui_clock_basic_demo`
- `picoui_keyboard_basic_demo`
- `picoui_line_edit_basic_demo`
- `picoui_combo_box_basic_demo`
- `picoui_scroll_selecter_basic_demo`
- `picoui_table_basic_demo`
- `picoui_graph_basic_demo`
- `picoui_calendar_basic_demo`
- `picoui_animation_basic_demo`
- `picoui_legacy_widget_parity_demo`
- `picoui_layout_parity_demo`
- `picoui_grid_parity_demo`

## 3. 构建方法

建议从仓库根目录执行：

```bash
cd /Users/cys/embedded/LingDongGUI
rtk cmake -S . -B build
```

构建单个 demo：

```bash
rtk cmake --build build --target picoui_basic_widgets_demo
```

构建多个 demo：

```bash
rtk cmake --build build --target \
  picoui_basic_widgets_demo \
  picoui_settings_panel_demo \
  picoui_qrcode_basic_demo
```

## 4. 运行单个 demo

正常窗口运行：

```bash
./build/examples/sdl/picoui_basic_widgets_demo
```

如果要让 demo 自动退出，便于脚本或人工快速 smoke：

```bash
PICOUI_DEMO_AUTO_QUIT_MS=1200 ./build/examples/sdl/picoui_basic_widgets_demo
```

如果要让 demo 生成当前帧截图 artifact：

```bash
PICOUI_DEMO_AUTO_QUIT_MS=1200 \
PICOUI_CAPTURE_FILE=/tmp/picoui-basic-widgets.ppm \
./build/examples/sdl/picoui_basic_widgets_demo
```

在无桌面窗口环境下，可用 dummy SDL driver 做 readback：

```bash
SDL_VIDEODRIVER=dummy \
PICOUI_DEMO_AUTO_QUIT_MS=1200 \
PICOUI_CAPTURE_FILE=/tmp/picoui-basic-widgets.ppm \
./build/examples/sdl/picoui_basic_widgets_demo
```

## 5. 自动验证入口

### 单 demo runtime smoke

```bash
rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets
```

说明：

- 该脚本证明的是 runtime smoke / auto-quit / capture 路径。
- 它不是 visible correctness，也不是 manual artifact pass。

### 全量 runtime smoke

```bash
rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo all
```

说明：

- 当前脚本参数是 `--demo all`，不是 `--all-demos`。
- 该命令对应 `P6-AC` closeout 所需 broad runtime gate。

### 全量 demo main contract

```bash
rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py
```

说明：

- 该脚本证明所有 `picoui/demo/*/main.c` 都已是 `v1_lvgl_like` 主路径。
- 它不证明 runtime、visible 或 manual artifact。

### 全量 native artifact gate

```bash
rtk python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --all-demos
```

说明：

- 该脚本读取 [picoui_native_artifact_manifest.json](/Users/cys/embedded/LingDongGUI/tests/picoui/runtime/picoui_native_artifact_manifest.json)。
- `visible` demo 必须满足：
  - `rc=0`
  - 输出 `PICOUI_RUNTIME_READY`
  - 生成非空 capture artifact
- `runtime_only` demo 不会被伪装成 visible pass；降级原因以 manifest 为准。

### 单 demo native artifact gate

```bash
rtk python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --demo basic_widgets
```

## 6. Manual Artifact 入口

如果要为某个 demo 生成可归档的手工窗口 artifact：

```bash
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py \
  --demo basic_widgets \
  --build-dir build \
  --artifact-root artifacts/picoui/manual-window
```

如果要批量生成：

```bash
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py \
  --all \
  --build-dir build \
  --artifact-root artifacts/picoui/manual-window
```

说明：

- 该脚本要求真实窗口 driver。
- 若当前是 `SDL_VIDEODRIVER=dummy`，脚本会跳过，不会给出 manual pass。
- 它只证明 `artifact_entry_exists`，不等于人工验收已通过。

## 7. 当前 Artifact 分层真相

当前 `P6-AA` 的 native artifact gate 把 demos 分成两类：

### `visible`

当前已有稳定 capture artifact 的 demo 共 `21` 个：

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

### `runtime_only`

当前仍没有稳定 capture artifact 的 demo 共 `7` 个：

- `hello_world`
- `layout_flex`
- `layout_grid`
- `theme_showcase`
- `list_basic`
- `keyboard_basic`
- `layout_parity`

其中：

- 前 `6` 个当前表现为 `PICOUI_RUNTIME_READY + rc=1 + capture=0`
- `layout_parity` 当前更弱，为 `ready=0 + rc=1 + capture=0`

## 8. “UI 完成”所需证据

“UI 已完成”不能只靠以下任一单层证据得出：

- build 通过
- `demo main style` contract 通过
- runtime smoke 通过
- native artifact gate 通过
- manual artifact 文件存在

当前应按分层证据阅读：

- `demo main style`
  - 证明 demo 主路径已迁到 native skeleton
- `runtime smoke`
  - 证明 demo 能启动、进入 runtime 或生成 capture
- `native artifact gate`
  - 证明哪些 demo 已有稳定可读 artifact，哪些仍只是 `runtime_only`
- `manual artifact`
  - 证明存在可归档的人工窗口验收输入

因此，“UI 完成”至少需要 visible artifact 证据，不能拿 runtime smoke 或 main contract 替代。

## 9. 历史文档的使用方式

旧 wrapper 路线文档仅可用于：

- 历史追溯
- 行为对照
- 迁移参考

不可用于：

- 证明 `v1.0-native` 当前已完成
- 替代当前 `P6` runtime / artifact / manual verification 结论
- 把旧 `ldgui backend wrapper` 目标继续当成当前方向
