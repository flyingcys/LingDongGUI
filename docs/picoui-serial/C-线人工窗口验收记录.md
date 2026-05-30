# C线人工窗口验收记录

本文记录 `C6 / manual window artifact gate` 的人工窗口 artifact。该 gate 是可选门禁，只为“人工 OS 窗口验收通过”这类结论提供可追溯证据；它不替代 CTest、backend mapping gate 或 automatic visible gate。

当前 `H8` 只承认这份文档作为人工记录真相源，且最小 manual artifact 范围固定为以下两个 demo：

- `picoui_basic_widgets_demo`
- `picoui_settings_panel_demo`

超出这两个 demo 的 manual artifact 扩张，只能作为 `post-H candidate` 讨论；未进入当前 `H8` 完成条件。

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

使用要求：

- `artifact 路径` 只证明脚本成功生成了可追溯文件，不证明人工已观察 OS 窗口。
- `人工结论` 必须由人工填写最终观察结果；若尚未观察，只能明确写“待人工观察”，不能写成通过。
- `已知限制` 必须保留当前证据边界，例如 `artifact existence != 人工验收通过`。

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

说明：

- 当前仅登记 `basic_widgets` 与 `settings_panel` 两个 demo。
- 这表示当前 `H8` 最小 manual artifact 范围只覆盖这两个 demo。
- 记录中“artifact 已生成”只表示 artifact existence 已满足，不自动等于人工验收通过。

### 2026-05-29 basic_widgets

- 日期：2026-05-29
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_basic_widgets_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/picoui_basic_widgets_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/basic_widgets/frame.ppm`
- 人工结论：待人工观察 OS 窗口并填写最终结论
- 已知限制：该 gate 是可选人工窗口证据，不接入 CTest，不替代 automatic visible gate

### 2026-05-29 settings_panel

- 日期：2026-05-29
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_settings_panel_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/picoui_settings_panel_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/settings_panel/frame.ppm`
- 人工结论：待人工观察 OS 窗口并填写最终结论
- 已知限制：该 gate 是可选人工窗口证据，不接入 CTest，不替代 automatic visible gate
