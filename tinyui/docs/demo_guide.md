# TinyUI Demo 运行指南

本文只描述当前 `v2.1` 的 current-facing demo 真相，不复述旧 `PicoUI/a-0.x` serial 文档阶段细节。

## 真相源

- demo target 清单：`examples/sdl/CMakeLists.txt`
- runtime smoke：`tests/tinyui/runtime/check_tinyui_runtime.py`
- backend mapping：`tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- automatic visible：`tests/tinyui/runtime/check_tinyui_visible_ui.py --all`
- manual artifact 生成入口：`tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- manual artifact truth-source：`docs/picoui-serial/C-线人工窗口验收记录.md`
- release / capability truth-source：`tests/tinyui/contract/tinyui_release_capability_matrix.json`

## demo target

当前 SDL demo 以独立 target 构建，不走顶层 `USE_DEMO` 切换。

- `tinyui_hello_world_demo`
- `tinyui_basic_widgets_demo`
- `tinyui_layout_flex_demo`
- `tinyui_layout_grid_demo`
- `tinyui_theme_showcase_demo`
- `tinyui_settings_panel_demo`
- `tinyui_list_basic_demo`
- `tinyui_progress_bar_basic_demo`
- `tinyui_progress_wheel_basic_demo`
- `tinyui_qrcode_basic_demo`
- `tinyui_arc_basic_demo`
- `tinyui_gauge_basic_demo`
- `tinyui_icon_slider_basic_demo`
- `tinyui_radial_menu_basic_demo`
- `tinyui_message_box_basic_demo`
- `tinyui_date_time_basic_demo`
- `tinyui_clock_basic_demo`
- `tinyui_keyboard_basic_demo`
- `tinyui_line_edit_basic_demo`
- `tinyui_combo_box_basic_demo`
- `tinyui_scroll_selecter_basic_demo`
- `tinyui_graph_basic_demo`
- `tinyui_table_basic_demo`
- `tinyui_calendar_basic_demo`
- `tinyui_animation_basic_demo`
- `tinyui_legacy_widget_parity_demo`
- `tinyui_layout_parity_demo`
- `tinyui_grid_parity_demo`

## 构建

建议从仓库根目录执行：

```bash
cd /Users/cys/embedded/LingDongGUI
rtk cmake -S . -B build
rtk cmake --build build -j8 --target tinyui_hello_world_demo
```

运行某个 demo：

```bash
./build/tinyui-runtime/examples/sdl/tinyui_hello_world_demo
```

Windows 对应路径：

```text
build\tinyui-runtime\examples\sdl\tinyui_hello_world_demo.exe
```

## 自动 gate

当前 demo 证据分四层：

1. `runtime smoke`
   - `python3 tests/tinyui/runtime/check_tinyui_runtime.py`
   - 证明可 build、可启动、可 capture、可回归
2. `backend mapping`
   - `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`
   - 证明 marker 覆盖的真实 backend 映射，不等于 visible
3. `automatic visible`
   - `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all`
   - 证明 `SDL_VIDEODRIVER=dummy + PPM readback` 下可显示、可读、可判定
4. `manual artifact`
   - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`
   - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel`
   - 这是历史保留的人工窗口 artifact 入口；脚本当前实际构建和运行的是 `tinyui_*_demo`，默认 build 目录也是 `build/tinyui-runtime`

这些证据层不能混写：

- `runtime smoke` 通过，不等于 visible 正确
- `automatic visible` 通过，不等于人工窗口验收通过
- `manual artifact` 只在有真实桌面窗口、artifact 路径和人工记录时，才允许支撑人工窗口结论

## 最小本地门禁

```bash
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
rtk ctest --test-dir build -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure
```

完整本地门禁：

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
```

## manual artifact

当前 manual artifact truth 仍保留在历史链路：

- 脚本入口：`tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- artifact 根目录：`artifacts/tinyui/manual-window/`
- 记录真相源：`docs/picoui-serial/C-线人工窗口验收记录.md`

这是当前仓库的过渡态，不应误写成 canonical `tests/tinyui/runtime/*` 已完全承接 manual artifact。

## 常见问题

### 为什么 `-DUSE_DEMO=...` 不影响 TinyUI demo

`USE_DEMO` 控制的是老 `examples/sdl` 入口，不控制 `tinyui_*_demo` 独立 target。

### 为什么只构建了一个可执行文件

每个 TinyUI demo 都是单独 target，需要显式指定 `--target`。

### 为什么文档里还会看到 `picoui`

当前允许两类保留命中：

- 历史 serial 文档和历史 manual artifact 入口
- 仍处过渡态的 public C API / compatibility subtree

`v2.1` 当前要求的是 canonical truth、header、contract、test、CMake 和 current-facing 文档统一到 `tinyui`，不是把所有 `picoui` 文本命中清零。
