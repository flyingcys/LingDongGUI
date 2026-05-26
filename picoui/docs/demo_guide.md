# PicoUI Demo 运行指南

本文说明 `picoui/demo` 下各个 demo 的构建方式、启动方式和适用场景。

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
cmake -S examples/sdl -B examples/sdl/build
cmake --build examples/sdl/build -j8 --target picoui_hello_world_demo
```

如果要指定其他 demo，只需要替换目标名。

## 四、运行方式

### Linux

```bash
./examples/sdl/build/picoui_hello_world_demo
```

### Windows

```bash
examples\sdl\build\picoui_hello_world_demo.exe
```

## 五、各 demo 说明

### `picoui/demo/hello_world`

最小示例。创建 `app`、`window`、`label`、`button`，然后设置文本。

适合用途：

- 确认环境能编译和启动
- 看 PicoUI 最小生命周期

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

### `picoui/demo/layout_flex`

只演示 `flex` 相关布局配置：

- flow
- align
- gap

适合用途：

- 理解窗口布局如何配置
- 看 flex 参数如何传给窗口

### `picoui/demo/layout_grid`

只演示 `grid` 相关布局配置：

- 列定义
- 行定义
- 间距设置

适合用途：

- 理解网格布局的基本写法
- 看固定轨道和自动轨道的配置方式

### `picoui/demo/theme_showcase`

演示主题挂载方式。先创建 `theme`，再把它设置到 `app` 上，然后创建控件。

适合用途：

- 理解 theme 的生命周期
- 看统一主题如何影响控件外观

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

## 六、推荐阅读顺序

1. `hello_world`
2. `basic_widgets`
3. `layout_flex`
4. `layout_grid`
5. `theme_showcase`
6. `settings_panel`

## 七、最常用命令

构建某个 demo：

```bash
cmake --build examples/sdl/build -j8 --target picoui_basic_widgets_demo
```

运行某个 demo：

```bash
./examples/sdl/build/picoui_basic_widgets_demo
```

重新配置后再编译：

```bash
cmake -S examples/sdl -B examples/sdl/build
cmake --build examples/sdl/build -j8
```

## 八、常见问题

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

如果要，我可以继续补一版“每个 demo 的源码入口和界面表现”说明，直接贴到 `README.md` 里。 
