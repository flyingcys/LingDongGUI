# C线人工窗口验收记录

本文记录 `C6 / manual window artifact gate` 的人工窗口 artifact。该 gate 是可选门禁，只为“人工 OS 窗口验收通过”这类结论提供可追溯证据；它不替代 CTest、backend mapping gate 或 automatic visible gate。

## 运行方式

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target picoui_basic_widgets_demo picoui_settings_panel_demo
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

默认 artifact 路径：

```text
artifacts/picoui/manual-window/<demo-name>/frame.ppm
```

无窗口环境或 `SDL_VIDEODRIVER=dummy` 时，脚本应输出 `PICOUI_MANUAL_WINDOW_ARTIFACT=SKIP`，不得把该结果写成人工窗口验收通过。

## 验收记录模板

### YYYY-MM-DD demo-name

- 日期：
- 平台：
- SDL video driver：
- demo target：
- 构建目录：
- 运行命令：
- artifact 路径：
- 人工结论：
- 已知限制：

## 当前记录

### 2026-05-29 basic_widgets

- 日期：2026-05-29
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_basic_widgets_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/picoui_basic_widgets_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/basic_widgets/frame.ppm`
- 人工结论：artifact 已生成；待人工观察 OS 窗口并填写最终结论，脚本生成 artifact 不自动等于验收通过
- 已知限制：该 gate 是可选人工窗口证据，不接入 CTest，不替代 automatic visible gate

### 2026-05-29 settings_panel

- 日期：2026-05-29
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_settings_panel_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/picoui_settings_panel_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/settings_panel/frame.ppm`
- 人工结论：artifact 已生成；待人工观察 OS 窗口并填写最终结论，脚本生成 artifact 不自动等于验收通过
- 已知限制：该 gate 是可选人工窗口证据，不接入 CTest，不替代 automatic visible gate
