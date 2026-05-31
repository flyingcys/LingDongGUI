# C线人工窗口验收记录

本文记录 `C6 / manual window artifact gate` 的人工窗口 artifact。该 gate 是可选门禁，只为“人工 OS 窗口验收通过”这类结论提供可追溯证据；它不替代 CTest、backend mapping gate 或 automatic visible gate。

`J7` 之后，这份文档同时承担 `v0.1 parity = window / label / button / slider` 的人工观察真相源。当前四控件都通过 `basic_widgets` / `settings_panel` 两个 demo 条目复核，不额外扩张 demo 粒度。

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

### 2026-05-30 basic_widgets

- 日期：2026-05-30
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_basic_widgets_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/basic_widgets/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见 `Basic Widgets` 文本、一个矩形按钮区域、一条水平 slider、以及承载这些元素的窗口内容区域；同时还能看到 switch 与 checkbox。就 `window / label / button / slider` 而言，当前可确认该 demo 有可见输出与基本布局承载，但版面仍然简陋，且这条结论只成立于 artifact 图像观察层。
- 已知限制：本次人工结论来自 `ARTIFACT_READY` 后生成的 `frame.ppm` 图像观察，不等于 live OS 窗口现场验收通过，也不证明交互行为、像素级视觉质量或发布级体验已经完成。

### 2026-05-30 settings_panel

- 日期：2026-05-30
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`picoui_settings_panel_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/picoui/manual-window/settings_panel/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见长条 switch、水平 slider 轨道、浅色面板区域，以及承载这些元素的窗口内容区域。就 `window / button / slider` 而言，该 demo 有可见输出；`label` 在这张 artifact 中不显著，但结合 `basic_widgets` 条目，当前 `v0.1` 四控件的人工观察样本已齐。
- 已知限制：本次人工结论同样来自 `ARTIFACT_READY` 后生成的 artifact 图像观察，不等于 live OS 窗口现场验收通过；且 `settings_panel` 这张图像内容较稀疏，不能单独承担完整的 label 可见性结论。
