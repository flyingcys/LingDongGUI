# PicoUI API 总览

当前 `a-0.7` 口径下，PicoUI 已覆盖 LingDongGUI `27` 个 widget-like 控件：

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
- 主题与样式桥：真实 backend theme/style apply
- demo/gate 入口：`runtime / mapping / visible / manual artifact`

## 设计约束

- 对外只暴露 `picoui_*`
- 用户不需要直接使用 `ld*`
- 用户不需要直接使用 `ARM-2D`
- backend 适配只放在 `picoui/src/backend/ldgui/`
- `a-0.7` 的 truth-source 以 `tests/picoui/contract/picoui_native_100_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 为准

当前状态：

- `27/27` widget-like 控件已进入 PicoUI public API，包含 `animation`
- `a-0.7` 当前处于 `R8 closeout / release readiness audit`
- 是否可以正式写成完成，以最终 fresh verification 和 `gitnexus_detect_changes` 结果为准
