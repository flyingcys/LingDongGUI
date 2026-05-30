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
- `picoui_list_basic_demo`

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
- `image` 当前创建真实 `ldImage` 对象；`picoui_image_set_source()` 只绑定调用方提供的 tile 指针，不做资源加载
- `image` 允许空 source/清空 source，此时 backend 保持真实 `ldImage` 对象，`img_tile/mask_tile` 均为空
- `image` 无 source 时 visible gate 只能证明 demo 中 image 区域或真实对象路径可见、可捕获；不证明占位资源绑定，也不证明真实图片加载完成
- `image` 非空 source 必须提供 `img_tile`；`mask_tile` 可为空，表示无遮罩图片
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
- `image` 当前**不支持** theme/style apply；口径是明确拒绝 `PICOUI_PART_MAIN`，不会借 theme/style 改写 image source 或 backend tile

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

### `picoui/demo/list_basic`

`F线` 新控件 vertical slice 示例。包含：

- `label`
- `list`
- 三个 list item：`item_wifi`、`item_bluetooth`、`item_display`

适合用途：

- 看 `picoui_list` 的最小 API 写法
- 看 list demo 如何接入 runtime、mapping、visible matrix

当前口径：

- `picoui_list` 的 public API、unit test 和真实 `ldList` backend mapping 已接入
- `picoui_list_basic_demo` 已能 build、run，并进入 runtime smoke
- backend mapping gate 现在只把 `list` 纳入 `PICOUI_BACKEND_REAL_WIDGET_IDS`，用于证明 list 本体进入真实 `ldList` backend
- `item_wifi/item_bluetooth/item_display` 不再出现在 runtime mapping marker 里，避免把 list item id 误读成真实 widget/object id 或 marker contract
- 因此当前 runtime mapping gate 不再对 item 侧给出任何强/弱 marker 结论；`list item marker` 仍必须按 `reject` 处理
- automatic visible gate 已接入 `check_picoui_visible_ui.py --all`；该证据只证明 dummy SDL + PPM readback 下的 automatic visible correctness
- manual window artifact gate 仍按 `C线` 证据层级单独记录；不能由 smoke、mapping 或 automatic visible gate 代替
- 该 demo 不证明 multi-select、virtualization、drag reorder、keyboard navigation

## 六、证据层级说明

1. `ctest` / unit test：证明 contract、backend 字段同步、事件桥接等实现约束。
2. `tests/picoui/runtime/check_picoui_runtime.py`：证明 demo 可 build、可启动、可 capture、可回归。
3. `tests/picoui/runtime/check_picoui_backend_mapping.py`：证明 demo 的真实 backend 映射与 fallback marker 口径。
4. `tests/picoui/runtime/check_picoui_visible_ui.py --all`：证明 visible-gate demo 在 `SDL_VIDEODRIVER=dummy + PPM readback` 下的 automatic visible correctness。
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

### manual window artifact gate

`C6 / manual window artifact gate` 只在需要人工 OS 窗口证据时单独运行，默认不接入 CTest，也不让无窗口 CI 因缺少桌面环境失败。

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target picoui_basic_widgets_demo picoui_settings_panel_demo
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

脚本会记录并输出平台、SDL video driver、demo target、构建目录、运行命令和 artifact 路径。默认 artifact 路径为：

```text
artifacts/picoui/manual-window/<demo-name>/frame.ppm
```

只有同时满足以下条件，才允许写“人工窗口验收通过”：

- 使用非 `dummy` 的 SDL video driver，并且脚本没有输出 `SKIP`。
- 运行时确实出现 OS 窗口，人工观察结果符合 demo 预期。
- `docs/picoui-serial/C-线人工窗口验收记录.md` 已记录日期、平台、SDL video driver、demo target、构建目录、运行命令、artifact 路径、人工结论和已知限制。

如果脚本输出 `PICOUI_MANUAL_WINDOW_ARTIFACT=SKIP`，只能说明当前环境不适合执行人工窗口验收；如果使用 `SDL_VIDEODRIVER=dummy` 生成 PPM，也只能作为 readback artifact，不能写成人工窗口结论。该 gate 也不能替代 `ctest`、backend mapping gate 或 automatic visible gate。

## 七、PicoUI 本地门禁矩阵

当前主项目存在 `.github/workflows/cmake-single-platform.yml`，但它是 `workflow_dispatch` / `release published` 触发的 `build pack` workflow，执行 `gen_pack.sh` 与 `Open-CMSIS-Pack/gen-pack-action`，不是现有测试 workflow，也不适合在 `C4` 内低风险最小接入 PicoUI gate。因此当前只固定本地运行口径，不改 workflow、不新造 CI 框架。以后若给主项目 CI 接入 PicoUI gate，应复用本节同一矩阵，不另开一套说法。

每次 PicoUI 改动后的最小本地门禁是：

```bash
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

需要完整本地门禁时，运行：

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
```

汇报规则固定为：

- `smoke gate` 通过：只能说可启动、可进入 runtime loop。
- `visible gate` 通过：只能说自动 visible correctness 通过。
- `backend mapping gate` 通过：只能说 marker 覆盖的 backend 映射通过。
- `manual artifact gate` 通过后：才允许说人工窗口验收通过。

因此，`ctest --test-dir build -L picoui --output-on-failure` 不能单独替代 `visible` 和 `mapping` label，也不能替代 standalone runtime 脚本的完整本地门禁。

## 八、新增 demo/widget/layout/theme 的 gate 同步规则

后续新增 demo、新增 widget、新增 layout 或新增 theme 能力时，必须同步维护 gate matrix，不能只改 demo 或只改 backend 后用 `ctest -L picoui` 代替 visible/mapping/manual artifact 层级。

### 新增 demo

新增 demo 必须同步：

- `tests/picoui/runtime/check_picoui_runtime.py`：加入 smoke 覆盖，或写明该 demo 不适合 runtime smoke 的豁免原因。
- `tests/picoui/runtime/check_picoui_visible_ui.py`：加入 visible matrix，或写明不可见理由。
- `tests/picoui/runtime/check_picoui_backend_mapping.py`：加入 mapping matrix，或在脚本和文档里写明豁免说明。
- 本文档：加入 demo 名称、目标、用途、运行方式、当前证据层级。
- 对应 serial 文档阶段状态：说明该 demo 是否已经进入 smoke、visible、mapping、manual artifact 证据层。

### 新增 widget

新增 widget 必须同步：

- public API contract，确认 public header 仍只暴露 `picoui_*` API。
- backend mapping test，证明 widget 进入真实 `LingDongGUI` backend，或明确拒绝/暂不支持。
- visible gate 样本，或明确该 widget 不可见、不可由 readback 判定的理由。
- theme/style 支持或拒绝说明，避免把未实现 style 能力误写成默认支持。

### 新增 layout / 新增 theme

新增 layout 或新增 theme 能力必须同步：

- unit/contract test，锁定 public API、参数语义、拒绝语义和 backend 字段映射。
- runtime demo 样本，证明真实 demo 链路会用到该能力。
- visible gate，或明确不可见理由。
- gate matrix 文档，说明该能力落在哪些 smoke、visible、mapping、manual artifact 层。

### 禁止替代关系

- 禁止只改 demo，不更新 runtime/visible/mapping matrix。
- 禁止只改 backend，不更新 demo、public contract、visible 样本和 gate matrix。
- 禁止用 `ctest --test-dir build -L picoui --output-on-failure` 单独替代 `ctest -L visible`、`ctest -L mapping`、standalone runtime 脚本或 manual artifact gate。

## 九、推荐阅读顺序

1. `hello_world`
2. `basic_widgets`
3. `layout_flex`
4. `layout_grid`
5. `theme_showcase`
6. `settings_panel`

## 十、最常用命令

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

## 十一、常见问题

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
