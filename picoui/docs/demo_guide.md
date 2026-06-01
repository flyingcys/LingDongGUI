# PicoUI Demo 运行指南

本文说明 `picoui/demo` 下各个 demo 的构建方式、启动方式、适用场景，以及 `a-0.6 final release` 口径下的 gate 覆盖边界。

当前真相源分工是：

- `tests/picoui/contract/picoui_release_capability_matrix.json`
  - 机器可读 final release truth-source
- `tests/picoui/runtime/check_picoui_runtime.py`
  - runtime smoke / capture gate
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
  - backend mapping gate
- `tests/picoui/runtime/check_picoui_visible_ui.py --all`
  - automatic visible gate
- `docs/picoui-serial/C-线人工窗口验收记录.md`
  - final release manual artifact truth-source

本页负责把这些 gate 与 demo catalog 对齐，不把 runtime、automatic visible、manual artifact 混写成同一层结论。

## 一、先说明入口

`picoui/demo` 不走顶层 `USE_DEMO` 入口。它们在 `examples/sdl/CMakeLists.txt` 中被单独编成独立目标，目标名如下：

- `picoui_hello_world_demo`
- `picoui_basic_widgets_demo`
- `picoui_layout_flex_demo`
- `picoui_layout_grid_demo`
- `picoui_theme_showcase_demo`
- `picoui_settings_panel_demo`
- `picoui_list_basic_demo`
- `picoui_progress_bar_basic_demo`
- `picoui_progress_wheel_basic_demo`
- `picoui_qrcode_basic_demo`
- `picoui_arc_basic_demo`
- `picoui_gauge_basic_demo`
- `picoui_icon_slider_basic_demo`
- `picoui_radial_menu_basic_demo`
- `picoui_message_box_basic_demo`
- `picoui_date_time_basic_demo`
- `picoui_clock_basic_demo`
- `picoui_keyboard_basic_demo`
- `picoui_line_edit_basic_demo`
- `picoui_combo_box_basic_demo`
- `picoui_scroll_selecter_basic_demo`
- `picoui_graph_basic_demo`
- `picoui_table_basic_demo`
- `picoui_calendar_basic_demo`

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

如果要验证 final release backend mapping 口径，使用：

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
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

## 五、Final Gate Catalog

`a-0.6 final release` 当前固定四层 gate：

1. `runtime`
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
2. `mapping`
   - `picoui_hello_world_demo`
   - `picoui_theme_showcase_demo`
   - `picoui_basic_widgets_demo`
   - `picoui_settings_panel_demo`
   - `picoui_list_basic_demo`
   - `picoui_progress_bar_basic_demo`
   - `picoui_arc_basic_demo`
   - `picoui_gauge_basic_demo`
   - `picoui_icon_slider_basic_demo`
   - `picoui_radial_menu_basic_demo`
   - `picoui_progress_wheel_basic_demo`
   - `picoui_qrcode_basic_demo`
   - `picoui_date_time_basic_demo`
   - `picoui_clock_basic_demo`
   - `picoui_line_edit_basic_demo`
   - `picoui_combo_box_basic_demo`
   - `picoui_scroll_selecter_basic_demo`
   - `picoui_table_basic_demo`
   - `picoui_graph_basic_demo`
   - `picoui_calendar_basic_demo`
   - `picoui_layout_flex_demo`
   - `picoui_layout_grid_demo`
3. `visible`
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
   - `line_edit_basic`
   - `combo_box_basic`
   - `scroll_selecter_basic`
   - `table_basic`
   - `graph_basic`
   - `calendar_basic`
4. `manual artifact`
   - 以 `docs/picoui-serial/C-线人工窗口验收记录.md` 为 demo-level truth-source
   - 当前 final release 目标集包含：
     `hello_world / basic_widgets / layout_flex / layout_grid / theme_showcase / settings_panel / list_basic / progress_bar_basic / arc_basic / gauge_basic / icon_slider_basic / radial_menu_basic / progress_wheel_basic / qrcode_basic / message_box_basic / date_time_basic / clock_basic / keyboard_basic / line_edit_basic / combo_box_basic / scroll_selecter_basic / table_basic / graph_basic / calendar_basic`

special cases：

- `keyboard`
  - final release 主要依赖 `runtime` 和 `manual artifact`
  - 当前没有 dedicated mapping gate，也没有 dedicated visible gate
  - 其视觉与宿主证据通过配套输入 demo 间接覆盖，不应误写成独立 mapping/visible 已闭环
- `message_box`
  - 当前仍保留 formal mapping exclusion
  - final release 结论依赖 `unit + contract + visible + runtime + manual artifact`
  - 不能把它写成与普通静态 widget 完全同构的 mapping 证明
## 六、各 demo 说明

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
- `image` 无 source 时 `automatic visible gate` 只能证明 demo 中 image 区域或真实对象路径可见、可捕获；不证明占位资源绑定，也不证明真实图片加载完成
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

适合用途：

- 看 `picoui_list` 的最小 API 写法
- 看 list demo 如何接入 runtime、mapping、visible matrix

当前口径：

- 该 demo 为 `list` 提供真实 widget 样本
- `picoui_list_basic_demo` 已进入 `runtime smoke`
- `backend mapping gate` 已覆盖该 demo 的真实 backend 映射样本
- `PICOUI_BACKEND_REAL_WIDGET_IDS` 当前只把 `list` 记为真实 backend widget；`item_*` 只是 PicoUI payload marker，不是独立 backend widget id
- `automatic visible gate` 已接入 `check_picoui_visible_ui.py --all`；该证据只证明 dummy SDL + PPM readback 下的 automatic visible correctness
- manual artifact gate 仍按 `C线` 证据层级单独记录；不能由 `runtime smoke`、`backend mapping gate` 或 `automatic visible gate` 代替
- 该 demo 不证明更细粒度 item 行为或更高阶交互模式
- widget-level `user_data` 与 `on_selected(..., user_data)` callback cookie 是两套语义，不应混写成同一合同

### `picoui/demo/line_edit_basic`

`a-0.4` editable text contract 示例。包含：

- `line_edit`

适合用途：

- 看 `picoui_line_edit` 的 public API 与 props
- 看 text/type/keyboard binding/readback 如何走真实 `ldLineEdit`

当前口径：

- `line_edit` 已走真实 backend mapping
- public getter 以 backend truth/readback 为准
- 当前只承诺 `editing/finished boundary`，不承诺 `submit/cancel reason`

### `picoui/demo/keyboard_basic`

`a-0.4` keyboard bridge 示例。包含：

- `keyboard`
- `line_edit`

适合用途：

- 看 keyboard bridge 如何把输入发给 focus/editing owner
- 看 keyboard runtime gate 如何证明 bridge 已接通

当前口径：

- `keyboard` 已进入 PicoUI public widget 集
- `keyboard_basic` 主要由 runtime gate 证明 bridge/ownership 路径，当前没有 dedicated backend mapping gate，也没有 dedicated visible correctness gate
- 该 demo 不证明完整视觉/theme parity

### `picoui/demo/combo_box_basic`

`a-0.4` dropdown contract 示例。包含：

- `label`
- `combo_box`

适合用途：

- 看 `picoui_combo_box` 的 item/selected/open-close API
- 看 dropdown contract 如何落到真实 `ldComboBox`

当前口径：

- `combo_box` 已走真实 backend mapping，不再记为 fallback widget
- `selected item` getter 以 backend truth/readback 为准
- `automatic visible gate` 已覆盖 `combo_box_basic`
- 该 demo 不证明完整 theme/skin parity

### `picoui/demo/scroll_selecter_basic`

`a-0.4` selection/edit-mode 示例。包含：

- `label`
- `scroll_selecter`

适合用途：

- 看 `picoui_scroll_selecter` 的 item/selected/edit-mode API
- 看 `ldScrollSelecter` 的真实 backend mapping

当前口径：

- `scroll_selecter` 已走真实 backend mapping
- selected item getter 以 backend truth/readback 为准
- `edit mode` 与 `navigation mode` 已有最小合同边界
- `automatic visible gate` 已覆盖 `scroll_selecter_basic`

### `picoui/demo/arc_basic`

`a-0.6` 仪表类显示控件示例。包含：

- `label`
- `arc`

适合用途：

- 看 `picoui_arc` 的背景角度、前景角度、旋转与颜色 API
- 看仪表类只读显示 contract 如何走真实 `ldArc`

当前口径：

- `arc` 已走真实 backend mapping
- angle/rotation/color readback 以 backend truth 为准
- `automatic visible gate` 已覆盖 `arc_basic`
- 该 demo 不证明更复杂资源皮肤或主题注入系统

### `picoui/demo/gauge_basic`

`a-0.6` 仪表指针控件示例。包含：

- `label`
- `gauge`

适合用途：

- 看 `picoui_gauge` 的 angle、pointer color、auto-move 合同
- 看仪表指针控件如何走真实 `ldGauge`

当前口径：

- `gauge` 已走真实 backend mapping
- angle/pointer color/auto-move readback 以 backend truth 为准
- `automatic visible gate` 已覆盖 `gauge_basic`
- 该 demo 不证明更复杂表盘资源替换或主题注入系统

### `picoui/demo/icon_slider_basic`

`a-0.6` 复合导航控件示例。包含：

- `label`
- `icon_slider`

适合用途：

- 看 `picoui_icon_slider` 的 selection/value/item catalog API
- 看复合导航控件如何走真实 `ldIconSlider`

当前口径：

- `icon_slider` 已走真实 backend mapping
- selection/value/item catalog 以 backend truth/readback 为准
- `automatic visible gate` 已覆盖 `icon_slider_basic`
- 该 demo 不证明完整图标资源系统或更高阶动画能力

### `picoui/demo/radial_menu_basic`

`a-0.6` 径向导航控件示例。包含：

- `label`
- `radial_menu`

适合用途：

- 看 `picoui_radial_menu` 的 selection/offset/item catalog API
- 看复合径向导航控件如何走真实 `ldRadialMenu`

当前口径：

- `radial_menu` 已走真实 backend mapping
- selection/offset/item catalog 以 backend truth/readback 为准
- `automatic visible gate` 已覆盖 `radial_menu_basic`
- 该 demo 不证明完整主题资源、分页动画或更复杂菜单系统

### `picoui/demo/graph_basic`

`a-0.5` graph data-model 示例。包含：

- `label`
- `graph`

适合用途：

- 看 `picoui_graph` 的 series/value/move-add API
- 看 graph readback 如何走真实 `ldGraph`

当前口径：

- `graph` 已走真实 backend mapping
- `series count`、`value readback`、`move_add shift` 以 backend truth 为准
- `automatic visible gate` 已覆盖 `graph_basic`
- 该 demo 不证明完整 theme 或更高阶 chart parity

### `picoui/demo/table_basic`

`a-0.5` table shared-core 示例。包含：

- `table`

适合用途：

- 看 `picoui_table` 的 cell/current-cell/editable API
- 看 editable cell 如何复用 `line_edit/keyboard` shared-core

当前口径：

- `table` 已走真实 backend mapping
- current cell 与 cell text getter 以 backend truth/readback 为准
- commit 与 keyboard-exit cancel 边界都已进入最终合同
- `automatic visible gate` 已覆盖 `table_basic`
- 该 demo 不证明完整 table feature parity

### `picoui/demo/calendar_basic`

`a-0.5` calendar data-view 示例。包含：

- `label`
- `calendar`

适合用途：

- 看 `picoui_calendar` 的 date/header/grid API
- 看 calendar readback 如何走真实 `ldCalendar`

当前口径：

- `calendar` 已走真实 backend mapping
- `date/header/grid/current-month flag` 以 backend truth/readback 为准
- `automatic visible gate` 已覆盖 `calendar_basic`
- 该 demo 不证明完整 theme parity

### `picoui/demo/progress_bar_basic`

`a-0.6` final release 进度条示例。包含：

- `progress_bar`

适合用途：

- 看横向和纵向进度条的最小 API 写法
- 看进度类控件如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `primary/secondary` 两个进度条都真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 证明 `primary/secondary/title` 的真实 backend 对象路径
- `automatic visible gate` 证明一条长横向进度条和一条高纵向进度条在 dummy SDL + PPM readback 下都可见
- 不证明更复杂主题皮肤、动画过渡或资源注入系统

### `picoui/demo/progress_wheel_basic`

`a-0.6` final release 进度轮示例。包含：

- `progress_wheel`

适合用途：

- 看进度轮控件的最小 API 写法
- 看进度类旋转控件如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `wheel` 真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 证明 `wheel` 的真实 backend 对象路径
- `automatic visible gate` 证明彩色环和邻接白点在 dummy SDL + PPM readback 下可判定
- 不证明更复杂的动画调度或资源系统

### `picoui/demo/qrcode_basic`

`a-0.6` final release 二维码示例。包含：

- `qrcode`

适合用途：

- 看二维码控件的最小 API 写法
- 看显示型控件如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `qrcode` 真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 证明 `qrcode` 的真实 backend 对象路径
- `automatic visible gate` 证明二维码模块结构在 dummy SDL + PPM readback 下可判定
- 不证明扫码内容被外部设备读取，也不证明更高阶资源加载语义

### `picoui/demo/message_box_basic`

`a-0.6` final release 对话框示例。包含：

- `message_box`

适合用途：

- 看复合显示控件和 confirm callback 合同的最小 API 写法
- 看复合结构控件如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `message_box` 宿主真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 对该 demo 保持 formal mapping exclusion 口径
- `automatic visible gate` 证明文本层和按钮层在 dummy SDL + PPM readback 下可判定
- 单标题/消息/confirm 文案 getter 与 confirm callback bridge 已形成稳定公开合同
- 不证明多按钮、多动作或更复杂对话框流程

### `picoui/demo/date_time_basic`

`a-0.6` final release 日期时间示例。包含：

- `date_time`

适合用途：

- 看轻量日期时间显示控件的最小 API 写法
- 看手动 `set_format/set_date/set_time` 合同如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `date_time` 真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 证明 `date_time` 的真实 backend 对象路径
- `automatic visible gate` 证明单行日期时间文本在 dummy SDL + PPM readback 下可判定
- `format/date/time` 写入与 `get_format/get_date/get_time` 读回已形成稳定公开合同
- 不证明完整日期编辑、时区、日历或输入系统

### `picoui/demo/clock_basic`

`a-0.6` final release 时钟示例。包含：

- `clock`

适合用途：

- 看时钟控件的最小 `step_second` 显示合同
- 看指针型显示控件如何进入 runtime、mapping、visible gate

当前口径：

- 证明 `clock` 真实落到 LingDongGUI backend
- `runtime smoke` 证明 demo 可启动、可 capture
- `backend mapping gate` 证明 `clock` 的真实 backend 对象路径
- `automatic visible gate` 证明中心枢纽和三向指针在 dummy SDL + PPM readback 下可判定
- `step_second` 开关与读回已形成稳定公开合同，三指针资源链走真实 backend
- 不证明背景表盘资源、复杂主题资源系统或高级动画控制

## 七、证据层级说明

1. `ctest` / unit test：证明 contract、backend 字段同步、事件桥接等实现约束。
2. `tests/picoui/runtime/check_picoui_runtime.py`：证明 demo 可 build、可启动、可 capture、可回归。
3. `tests/picoui/runtime/check_picoui_backend_mapping.py`：证明 demo 的真实 backend 映射与 fallback marker 口径。
4. `tests/picoui/runtime/check_picoui_visible_ui.py --all`：证明 `automatic visible gate` 覆盖的 demo 在 `SDL_VIDEODRIVER=dummy + PPM readback` 下可显示、可读、可判定。
5. `manual artifact gate`：证明人工 OS 窗口验收通过；这属于 final release demo-level truth-source，需要单独运行并记录 artifact。

换句话说：

- `runtime smoke = 已启动`
- `backend mapping gate = marker 覆盖的真实 backend 映射，或已声明 special-case exclusion 的正式 gate`
- `automatic visible gate = dummy SDL + PPM readback 下可显示、可读、可判定`
- `manual artifact gate = 有平台、SDL video driver、demo target、artifact 路径和人工结论记录`

这些不是同一层证据。`capture` 非空仍不能单独证明 UI 正常显示；`automatic visible gate` 通过也不能写成人工窗口验收通过，除非已经执行 final release `manual artifact gate`。

运行 demo、`automatic visible gate`、人工窗口观察也不是同一件事：

- 直接运行 demo：用于本地观察交互和窗口行为，不自动生成可追溯验收结论。
- `automatic visible gate`：脚本设置 dummy SDL，通过 PPM readback 做可重复判定，适合 CI/回归。
- 人工窗口观察：需要真实窗口环境和 artifact 记录，只有 final release `manual artifact gate` 才能支撑“人工窗口验收通过”。

### manual artifact gate

final release `manual artifact gate` 只在需要人工 OS 窗口证据时单独运行，默认不接入 CTest，也不让无窗口 CI 因缺少桌面环境失败。

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

## 八、PicoUI 本地门禁矩阵

当前主项目存在 `.github/workflows/cmake-single-platform.yml`，但它是 `workflow_dispatch` / `release published` 触发的 `build pack` workflow，执行 `gen_pack.sh` 与 `Open-CMSIS-Pack/gen-pack-action`，不是现有测试 workflow，也不适合在当前任务内低风险最小接入 PicoUI gate。因此当前只固定本地运行口径，不改 workflow、不新造 CI 框架。以后若给主项目 CI 接入 PicoUI gate，应复用本节同一矩阵，不另开一套说法。

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

- `runtime smoke` 通过：只能说可启动、可进入 runtime loop。
- `automatic visible gate` 通过：只能说自动 visible correctness 通过。
- `backend mapping gate` 通过：只能说 marker 覆盖的 backend 映射通过。
- `manual artifact gate` 通过后：才允许说人工窗口验收通过。

因此，`ctest --test-dir build -L picoui --output-on-failure` 不能单独替代 `visible` 和 `mapping` label，也不能替代 standalone runtime 脚本的完整本地门禁。

## 九、新增 demo/widget/layout/theme 的 gate 同步规则

后续新增 demo、新增 widget、新增 layout 或新增 theme 能力时，必须同步维护 gate matrix，不能只改 demo 或只改 backend 后用 `ctest -L picoui` 代替 visible/mapping/manual artifact 层级。

### 新增 demo

新增 demo 必须同步：

- `tests/picoui/runtime/check_picoui_runtime.py`：加入 `runtime smoke` 覆盖，或写明该 demo 不适合 `runtime smoke` 的豁免原因。
- `tests/picoui/runtime/check_picoui_visible_ui.py`：加入 visible matrix，或写明不可见理由。
- `tests/picoui/runtime/check_picoui_backend_mapping.py`：加入 mapping matrix，或在脚本和文档里写明豁免说明。
- 本文档：加入 demo 名称、目标、用途、运行方式、当前证据层级。
- 对应 serial 文档阶段状态：说明该 demo 是否已经进入 smoke、visible、mapping、manual artifact 证据层。

### 新增 widget

新增 widget 必须同步：

- public API contract，确认 public header 仍只暴露 `picoui_*` API。
- backend mapping test，证明 widget 进入真实 `LingDongGUI` backend，或明确拒绝/暂不支持。
- `automatic visible gate` 样本，或明确该 widget 不可见、不可由 readback 判定的理由。
- theme/style 支持或拒绝说明，避免把未实现 style 能力误写成默认支持。

### 新增 layout / 新增 theme

新增 layout 或新增 theme 能力必须同步：

- unit/contract test，锁定 public API、参数语义、拒绝语义和 backend 字段映射。
- runtime demo 样本，证明真实 demo 链路会用到该能力。
- `automatic visible gate`，或明确不可见理由。
- gate matrix 文档，说明该能力落在哪些 smoke、visible、mapping、manual artifact 层。

### 禁止替代关系

- 禁止只改 demo，不更新 runtime/visible/mapping matrix。
- 禁止只改 backend，不更新 demo、public contract、visible 样本和 gate matrix。
- 禁止用 `ctest --test-dir build -L picoui --output-on-failure` 单独替代 `ctest -L visible`、`ctest -L mapping`、standalone runtime 脚本或 manual artifact gate。

## 十、推荐阅读顺序

1. `hello_world`
2. `basic_widgets`
3. `layout_flex`
4. `layout_grid`
5. `theme_showcase`
6. `settings_panel`

## 十一、最常用命令

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

## 十二、常见问题

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
