# PicoUI API 总览

第一阶段 PicoUI 提供以下能力：

- 基础控件：`window`、`label`、`text`、`image`、`button`、`checkbox`、`switch`、`slider`
- 基础布局：`flex`、`grid`
- 视觉能力：`theme v0`
- 统一 demo：`hello_world`、`basic_widgets`、`layout_flex`、`layout_grid`、`theme_showcase`、`settings_panel`

## 设计约束

- 对外只暴露 `picoui_*`
- 用户不需要直接使用 `ld*`
- 用户不需要直接使用 `ARM-2D`
- backend 适配只放在 `picoui/src/backend/ldgui/`
