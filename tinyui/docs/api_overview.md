# TinyUI API 总览

当前 `v2.1` 口径下，TinyUI 的 current truth 以 canonical `tests/tinyui/contract/*` 为准，尤其是：

- `tests/tinyui/contract/tinyui_native_100_inventory.json`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/ldgui_public_api_inventory.json`

当前 widget-like 控件覆盖为 `27` 个：

- `window`
- `label`
- `button`
- `checkbox`
- `switch`
- `slider`
- `text`
- `image`
- `list`
- `progress_bar`
- `qrcode`
- `progress_wheel`
- `message_box`
- `date_time`
- `clock`
- `line_edit`
- `keyboard`
- `combo_box`
- `scroll_selecter`
- `arc`
- `gauge`
- `graph`
- `table`
- `calendar`
- `icon_slider`
- `radial_menu`
- `animation`

同时提供：

- 基础布局：`flex`、`grid`
- 通用 widget/base 能力：位置、尺寸、可见性、透明度、focus、layout 参数
- 主题与样式能力：颜色、字体、背景、控件状态和布局相关样式
- 资源句柄：`tinyui_image_source_from_builtin`、`tinyui_image_source_from_vres`、`tinyui_image_source_destroy`
- demo/gate 入口：`demo boundary / deprecated API usage / runtime / visible / manual artifact`

## 当前约束

- current-facing canonical 目录、header、contract、test、CMake truth 已统一到 `tinyui`
- 用户不需要直接使用 `ld*`
- 用户不需要直接使用 `ARM-2D`
- demo 和 current-facing 文档只推荐 canonical TinyUI API；历史兼容别名只为旧代码保留
- 资源生命周期见 [TinyUI 资源生命周期](./resource_lifetime.md)

## 当前状态

- `27/27` widget-like 控件已进入 current public API 覆盖面，包含 `animation`
- `v2.1` 当前重点是保持 canonical truth、contract gate、runtime gate、release-facing 文档一致
- 是否可以写成 release-ready，仍以 fresh verification 和最终 closeout 证据为准
