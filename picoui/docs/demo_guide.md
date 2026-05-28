# PicoUI Demo 运行指南

本文说明 `picoui/demo` 下各个 demo 的构建方式、启动方式和适用场景，并明确哪些证据属于真实 backend 能力，哪些只属于 smoke/capture。

## 一、先说明入口

`picoui/demo` 不走顶层 `USE_DEMO` 入口。它们在 `examples/sdl/CMakeLists.txt` 中被单独编成独立目标，目标名如下：

- `picoui_hello_world_demo`
- `picoui_basic_widgets_demo`
- `picoui_layout_flex_demo`
- `picoui_layout_grid_demo`
- `picoui_theme_showcase_demo`
- `picoui_settings_panel_demo`

## 二、依赖环境

### Linux

- CMake 3.16 或更高版本
- GCC 或兼容的 C 编译器
- `pkg-config`
- SDL2 开发包

Debian / Ubuntu 可安装：

```bash
sudo apt-get install build-essential cmake pkg-config libsdl2-dev
```

### Windows

- CMake 3.16 或更高版本
- MinGW-w64 或其他兼容 GCC 的编译器
- 仓库自带 SDL2 依赖

## 三、构建方式

建议从仓库根目录执行：

```bash
cd /Users/cys/embedded/LingDongGUI
rtk cmake -S . -B build
rtk cmake --build build -j8 --target picoui_hello_world_demo
```

如果要指定其他 demo，只需要替换目标名。

如果只想跑 runtime smoke 脚本，则使用仓库既有入口：

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
```

注意：

- 该脚本会使用独立的 `build/picoui-runtime` 目录
- 它属于 smoke / 启动 / capture 回归检查，不等于 visible correctness

如果要验证真实可见结果，使用：

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
```

## 四、运行方式

### Linux

```bash
./build/picoui-runtime/examples/sdl/picoui_hello_world_demo
```

### Windows

```bash
build\picoui-runtime\examples\sdl\picoui_hello_world_demo.exe
```

## 五、各 demo 说明

### `picoui/demo/hello_world`

最小示例。创建 `app`、`window`、`label`、`button`，然后设置文本。

适合用途：

- 确认环境能编译和启动
- 看 PicoUI 最小生命周期
- 看 static widget 的真实 backend 映射最小闭环

### `picoui/demo/basic_widgets`

基础控件合集。包含：

- `switch`
- `checkbox`
- `slider`
- `button`
- `text`
- `image`

适合用途：

- 看控件创建方式
- 看事件回调绑定方式
- 看控件 API 的基本形态

当前口径：

- `button/text/image` 已有真实 backend 对象映射
- `checkbox/switch/slider` 已有真实对象映射与 native event
- `image` 当前使用真实 `ldImage` 对象；无真实图片源时，以显式占位 mask 进入 visible evidence
- 可见正确性由 `check_picoui_visible_ui.py --demo basic_widgets` 验证

### `picoui/demo/layout_flex`

只演示 `flex` 相关布局配置：

- flow
- align
- gap

适合用途：

- 理解窗口布局如何配置
- 看 flex 参数如何传给窗口
- 看 layout 语义如何走真实 backend，而不是靠 demo 硬编码补丁顶住

### `picoui/demo/layout_grid`

只演示 `grid` 相关布局配置：

- 列定义
- 行定义
- 间距设置

适合用途：

- 理解网格布局的基本写法
- 看固定轨道和自动轨道的配置方式
- 看 grid cell 语义如何传到底层 `LingDongGUI`

### `picoui/demo/theme_showcase`

演示主题挂载方式。先创建 `theme`，再把它设置到 `app` 上，然后创建控件。

适合用途：

- 理解 theme 的生命周期
- 看统一主题如何影响控件外观

当前口径：

- `window/button/checkbox/switch/slider/label/text` 已有真实 backend style apply
- `image` 当前**不支持** theme/style apply；A6 口径是明确拒绝 `PICOUI_PART_MAIN`，不是默认支持

### `picoui/demo/settings_panel`

综合型示例。包含：

- `theme`
- `flex(column)`
- `label`
- `switch`
- `slider`
- `button_props`

适合用途：

- 看真实面板类页面的组织方式
- 看 props 创建控件的方式

当前口径：

- `title/wifi/brightness/apply` 均走真实 backend 映射
- 该 demo 不再输出 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`
- 可见正确性由 `check_picoui_visible_ui.py --demo settings_panel` 验证

## 六、证据层级说明

1. `ctest` / unit test：证明 contract、backend 字段同步、事件桥接等实现约束。
2. `tests/picoui/runtime/check_picoui_runtime.py`：证明 demo 可 build、可启动、可 capture、可回归。
3. `tests/picoui/runtime/check_picoui_backend_mapping.py`：证明 demo 的真实 backend 映射与 fallback marker 口径。
4. `tests/picoui/runtime/check_picoui_visible_ui.py --all`：证明 6 个 demo 在 `SDL_VIDEODRIVER=dummy + PPM readback` 下的 automatic visible correctness。
5. `manual window artifact gate`：证明人工 OS 窗口验收通过；这属于 `C线` 的 `C6`，需要单独运行并记录 artifact。

换句话说：

- `runtime smoke = 已启动`
- `backend 完成态 = 真实对象/布局/事件/theme 已闭环`
- `automatic visible gate = dummy SDL + PPM readback 下可显示、可读、可判定`
- `manual window artifact gate = 有平台、SDL video driver、demo target、artifact 路径和人工结论记录`

这些不是同一层证据。`capture` 非空仍不能单独证明 UI 正常显示；automatic visible gate 通过也不能写成人工窗口验收通过，除非已经执行 `C6 / manual window artifact gate`。

运行 demo、自动 visible gate、人工窗口观察也不是同一件事：

- 直接运行 demo：用于本地观察交互和窗口行为，不自动生成可追溯验收结论。
- 自动 visible gate：脚本设置 dummy SDL，通过 PPM readback 做可重复判定，适合 CI/回归。
- 人工窗口观察：需要真实窗口环境和 artifact 记录，只有 `C6 / manual window artifact gate` 才能支撑“人工窗口验收通过”。

## 七、推荐阅读顺序

1. `hello_world`
2. `basic_widgets`
3. `layout_flex`
4. `layout_grid`
5. `theme_showcase`
6. `settings_panel`

## 八、最常用命令

构建某个 demo：

```bash
rtk cmake --build build -j8 --target picoui_basic_widgets_demo
```

运行某个 demo：

```bash
./build/picoui-runtime/examples/sdl/picoui_basic_widgets_demo
```

重新配置后再编译：

```bash
rtk cmake -S . -B build
rtk cmake --build build -j8
```

## 九、常见问题

### 1. 我传了 `-DUSE_DEMO=2`，为什么没跑 `picoui` demo

因为 `USE_DEMO` 是 `examples/sdl` 的老入口，用来控制 `ldgui_sdl_demo`。  
`picoui/demo` 是独立 target，不通过 `USE_DEMO` 切换。

### 2. 为什么只构建了一个可执行文件

这是正常的。每个 `picoui` demo 都是一个单独的可执行目标，需要你用 `--target` 指定。

### 3. 启动后没有看到内容

先确认：

- SDL2 依赖是否安装完整
- 运行的是对应的 demo target
- 构建目录里的可执行文件是否最新

如果后续继续扩展 PicoUI，请基于当前已收口主线另开新阶段或新计划，而不是回到 `backend_app.c` 继续堆积过渡逻辑。
