# TINYUI Final Release 人工窗口验收记录

本文是 `a-0.6 final release` 的 demo-level manual artifact truth-source。

它只负责两件事：

1. 记录哪些 demo 已经生成了可追溯人工窗口 artifact。
2. 记录人工是否真的观察了 OS 窗口，以及观察结论边界。

它不替代：

1. `ctest` / unit / contract
2. `runtime smoke`
3. `backend mapping gate`
4. `automatic visible gate`

## 当前范围

`a-0.6 final release` 的 manual artifact 目标集是：

- `hello_world`
- `basic_widgets`
- `layout_flex`
- `layout_grid`
- `theme_showcase`
- `settings_panel`
- `list_basic`
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
- `keyboard_basic`
- `line_edit_basic`
- `combo_box_basic`
- `scroll_selecter_basic`
- `table_basic`
- `graph_basic`
- `calendar_basic`

当前真实状态：

1. manual artifact 工具已扩到上述 final release demo 集。
2. 在当前主仓 `Darwin 25.5.0 (arm64) + cocoa + build/tinyui-runtime` 环境下，上述 `24` 个 final release demo 都已生成可追溯 `frame.ppm` artifact。
3. 当前记录层级已覆盖：
   - `24/24` demo 的 artifact existence
   - `24/24` demo 的 artifact-based visual observation
4. 这些记录仍然不等于 live OS 窗口现场验收通过，不能把 artifact-based observation 写成最终人工发布验收。

## 运行方式

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target tinyui_runtime
python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets
python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo settings_panel
```

如需补录其他 final release demo，只替换 `--demo` 参数即可。

默认 artifact 路径：

```text
artifacts/tinyui/manual-window/<demo-name>/frame.ppm
```

默认 `--build-dir`：

```text
build/tinyui-runtime
```

## 结论规则

1. `artifact 路径` 存在，只能证明脚本成功生成了可追溯文件。
2. `ARTIFACT_READY` 不等于人工窗口验收通过。
3. 只有当人工确实观察了真实 OS 窗口，并把结论写入本文件，才允许写“人工窗口验收通过”。
4. 若环境是 `SDL_VIDEODRIVER=dummy` 或脚本输出 `SKIP`，只能说明当前环境不适合做人工窗口验收。

## 记录模板

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
- demo target：`tinyui_basic_widgets_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/tinyui_basic_widgets_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/basic_widgets/frame.ppm`
- 人工结论：待人工观察 OS 窗口并填写最终结论
- 已知限制：该记录只证明 artifact 已生成，不等于 final release live OS 窗口验收通过

### 2026-05-29 settings_panel

- 日期：2026-05-29
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_settings_panel_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`/Users/cys/embedded/LingDongGUI/build/examples/sdl/tinyui_settings_panel_demo`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/settings_panel/frame.ppm`
- 人工结论：待人工观察 OS 窗口并填写最终结论
- 已知限制：该记录只证明 artifact 已生成，不等于 final release live OS 窗口验收通过

### 2026-05-30 basic_widgets

- 日期：2026-05-30
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_basic_widgets_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/basic_widgets/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见 `Basic Widgets` 文本、一个矩形按钮区域、一条水平 slider、switch、checkbox 和承载这些元素的窗口内容区域。该记录能证明 `basic_widgets` 存在可见输出与基本布局承载。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明交互行为、像素级视觉质量或发布级体验已经完成。

### 2026-05-30 settings_panel

- 日期：2026-05-30
- 平台：Darwin 25.3.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_settings_panel_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo settings_panel`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/settings_panel/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见长条 switch、水平 slider 轨道、浅色面板区域，以及承载这些元素的窗口内容区域。该记录能证明 `settings_panel` 存在可见输出与基本布局承载。
- 已知限制：本次结论同样来自 artifact 图像观察，不等于 live OS 窗口现场验收通过；且当前内容较稀疏，不能单独承担所有 final release demo 的人工结论。

### 2026-06-01 hello_world

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_hello_world_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo hello_world`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/hello_world/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见居中的标题文本条和下方按钮区域，能证明 `hello_world` 存在窗口承载与基础按钮布局输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明交互点击、字体渲染细节或发布级视觉质量已经完成。

### 2026-06-01 layout_flex

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_layout_flex_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo layout_flex`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/layout_flex/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见两条横向排列的浅色矩形区域和下方另一条内容区域，能证明 `layout_flex` 的容器承载与 flex 分布结果存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明所有 flex 求解边界或缩放行为已经完成。

### 2026-06-01 layout_grid

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_layout_grid_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo layout_grid`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/layout_grid/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和左右分栏的浅色块，能证明 `layout_grid` 的基础网格承载与列分布存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明所有 grid track 配置、边界对齐或响应式行为已经完成。

### 2026-06-01 theme_showcase

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_theme_showcase_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo theme_showcase`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/theme_showcase/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见多段标题/正文/强调区域，能证明 `theme_showcase` 存在主题文本与内容块的基础可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明主题切换、色板细节或字体资源覆盖已经完成。

### 2026-06-01 list_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_list_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo list_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/list_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和列表内容区域，能证明 `list_basic` 的窗口承载与列表主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明滚动、选择切换或长列表行为已经完成。

### 2026-06-01 progress_bar_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_progress_bar_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo progress_bar_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/progress_bar_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域及两条水平进度条区域，能证明 `progress_bar_basic` 的基础进度视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明动画、皮肤或复杂配色行为已经完成。

### 2026-06-01 arc_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_arc_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo arc_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/arc_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域及由黄色和浅绿色组成的弧形仪表段，能证明 `arc_basic` 的弧形视觉主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明动态更新、主题细节或所有角度边界已经完成。

### 2026-06-01 gauge_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_gauge_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo gauge_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/gauge_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和表盘主体，能证明 `gauge_basic` 的仪表视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明指针动画、更多刻度配置或复杂皮肤能力已经完成。

### 2026-06-01 icon_slider_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_icon_slider_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo icon_slider_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/icon_slider_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和多枚图标标签组合，能证明 `icon_slider_basic` 的复合选项视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明滑动切换、选中动画或图标资源完整性已经完成。

### 2026-06-01 radial_menu_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_radial_menu_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo radial_menu_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/radial_menu_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域以及中心轴线两侧的径向菜单弧段，能证明 `radial_menu_basic` 的复合径向视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明交互导航、旋转切换或图标布局边界已经完成。

### 2026-06-01 progress_wheel_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_progress_wheel_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo progress_wheel_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/progress_wheel_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和蓝色弧形进度段，能证明 `progress_wheel_basic` 的圆弧进度视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明动画平滑度、更多配色或尺寸配置已经完成。

### 2026-06-01 qrcode_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_qrcode_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo qrcode_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/qrcode_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和清晰的二维码矩阵图形，能证明 `qrcode_basic` 的二维码主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明扫码可用性、纠错等级切换或尺寸边距能力已经完成。

### 2026-06-01 message_box_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_message_box_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo message_box_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/message_box_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见对话框标题 `Update`、正文 `Apply settings?` 和下方 `OK` 按钮，能证明 `message_box_basic` 的弹窗主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过；且该 demo 仍保留 formal mapping exclusion 特例，不能由本条记录外推出普通静态 mapping 结论。

### 2026-06-01 date_time_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_date_time_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo date_time_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/date_time_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和日期时间文本显示区，能证明 `date_time_basic` 的基础时间内容存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明 locale、时区、编辑模式或自动同步策略已经完成。

### 2026-06-01 clock_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_clock_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo clock_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/clock_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见钟表主体轮廓与指针区域，能证明 `clock_basic` 的时钟视觉存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明复杂表盘资源、动画节奏或真实时间源切换已经完成。

### 2026-06-01 keyboard_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_keyboard_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo keyboard_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/keyboard_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见输入框及其文本内容，能证明 `keyboard_basic` 至少存在 runtime bridge 场景下的输入承载可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过；且该 demo 当前仍带 `keyboard` fake fallback 特例，不能由本条记录外推出 dedicated mapping/visible 已独立闭环。

### 2026-06-01 line_edit_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_line_edit_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo line_edit_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/line_edit_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见标题区域和单行编辑框，能证明 `line_edit_basic` 的文本输入承载存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明提交/取消原因、复杂编辑边界或输入法行为已经完成。

### 2026-06-01 combo_box_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_combo_box_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo combo_box_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/combo_box_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见 `Bluetooth` 文本及下拉指示图标，能证明 `combo_box_basic` 的下拉选择主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明展开态、主题皮肤或复杂选项行为已经完成。

### 2026-06-01 scroll_selecter_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_scroll_selecter_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo scroll_selecter_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/scroll_selecter_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见多行候选项文本，其中 `Bluetooth` 位于中心焦点区域，能证明 `scroll_selecter_basic` 的滚轮选择主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明滚动动画、循环边界或复杂视觉皮肤已经完成。

### 2026-06-01 table_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_table_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo table_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/table_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见 `A1/B1/C1` 等表格单元格和绿色焦点边框，能证明 `table_basic` 的表格主体与选中单元格存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明编辑提交、取消路径或复杂表格模型行为已经完成。

### 2026-06-01 graph_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_graph_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo graph_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/graph_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见边框区域、绿色折线和两个圆形数据点，能证明 `graph_basic` 的图表主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明高级图表模式、坐标轴配置或主题皮肤能力已经完成。

### 2026-06-01 calendar_basic

- 日期：2026-06-01
- 平台：Darwin 25.5.0 (arm64)
- SDL video driver：cocoa
- demo target：`tinyui_calendar_basic_demo`
- 构建目录：`/Users/cys/embedded/LingDongGUI/build/tinyui-runtime`
- 运行命令：`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo calendar_basic`
- artifact 路径：`/Users/cys/embedded/LingDongGUI/artifacts/tinyui/manual-window/calendar_basic/frame.ppm`
- 人工结论：基于 `frame.ppm` artifact 的人眼观察，可见日历标题 `2026/06/15`、星期标题行以及被高亮的日期单元格，能证明 `calendar_basic` 的日历网格主体存在可见输出。
- 已知限制：本次结论来自 artifact 图像观察，不等于 live OS 窗口现场验收通过，也不证明翻页、范围选择或更复杂日历模式已经完成。
