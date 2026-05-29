# PicoUI D线当前控件合同矩阵

本文是 D2 建立、D3 同步更新的当前控件合同矩阵，只记录现有 `PicoUI` public API、真实 backend 映射和证据层状态，不扩展 F 线新控件，不替代后续控件实现计划。

## 口径

- `support`：当前 public header 中存在 API，且已有 unit / contract / mapping / visible / manual artifact 中至少一层证据支撑其合同口径。
- `reject`：当前明确没有 public API，或者当前实现对该控件明确返回失败；调用方不能依赖该能力。
- `deferred`：能力方向合理，但当前 public API、backend 语义或证据层尚未闭环；后续阶段再决定。
- 证据层不等价：`unit` 证明字段、事件、layout、theme 等代码合同；`contract` 证明 public/demo/doc 边界；`mapping` 证明 demo 中真实 LingDongGUI backend 对象；`visible` 证明 dummy SDL + PPM readback 下可显示、可读、可判定；`manual artifact` 只在单独人工窗口 artifact gate 执行后成立。
- `capture` 非空不能单独证明 visible correctness；`mapping` 通过也不能写成完整 UI 完成。

## Public API 名称矩阵

下表的能力列 `visible` 只表示 `picoui_widget_set_visible` setter 口径，不等于后文证据层里的 visible gate。

| 控件 | create | create_with_props | text | value | checked | range | source | user_data | style_class | style_value | enabled | visible | focus | dirty | layout | theme | event |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| window | `picoui_window_create` | `picoui_window_create_with_props` | reject | reject | reject | reject | reject | `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_flex_set_flow`, `picoui_flex_set_align`, `picoui_flex_set_gap`, `picoui_grid_set_columns`, `picoui_grid_set_rows`, `picoui_grid_set_gap`, `picoui_grid_set_align`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | reject |
| label | `picoui_label_create` | `picoui_label_create_with_props` | `picoui_label_set_text`, `picoui_label_set_font`, `picoui_widget_set_text` | reject | reject | reject | reject | `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | reject |
| button | `picoui_button_create` | `picoui_button_create_with_props` | `picoui_button_set_text`, `picoui_widget_set_text` | reject | reject | reject | reject | `picoui_button_set_on_clicked`, `picoui_button_set_on_pressed`, `picoui_button_set_on_released`, `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | `picoui_button_set_on_clicked`, `picoui_button_set_on_pressed`, `picoui_button_set_on_released` |
| checkbox | `picoui_checkbox_create` | `picoui_checkbox_create_with_props` | `picoui_checkbox_set_text`, `picoui_widget_set_text` | reject | `picoui_checkbox_set_checked`, `picoui_checkbox_is_checked` | reject | reject | `picoui_checkbox_set_on_toggled`, `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | `picoui_checkbox_set_on_toggled` |
| switch | `picoui_switch_create` | `picoui_switch_create_with_props` | reject | reject | `picoui_switch_set_checked`, `picoui_switch_is_checked` | reject | reject | `picoui_switch_set_on_toggled`, `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | `picoui_switch_set_on_toggled` |
| slider | `picoui_slider_create` | `picoui_slider_create_with_props` | reject | `picoui_slider_set_value`, `picoui_slider_get_value` | reject | `picoui_slider_set_range` | reject | `picoui_slider_set_on_value_changed`, `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | `picoui_slider_set_on_value_changed` |
| text | `picoui_text_create` | `picoui_text_create_with_props` | `picoui_text_set_text`, `picoui_text_set_font`, `picoui_widget_set_text` | reject | reject | reject | reject | `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | `picoui_theme_apply_to_widget` | reject |
| image | `picoui_image_create` | `picoui_image_create_with_props` | reject | reject | reject | reject | `picoui_image_set_source` | `picoui_widget_set_user_data` | `picoui_widget_set_style_class` | `picoui_widget_set_bg_color`, `picoui_widget_set_text_color`, `picoui_widget_set_border_color`, `picoui_widget_set_radius` | `picoui_widget_set_enabled` | `picoui_widget_set_visible` | reject | reject | `picoui_widget_set_pos`, `picoui_widget_set_size`, `picoui_widget_set_flex_grow`, `picoui_widget_set_flex_new_track`, `picoui_widget_set_ignore_layout`, `picoui_widget_set_grid_cell`, `picoui_widget_set_padding` | reject | reject |

## 控件能力状态

| 控件 | create | create_with_props | text | value | checked | range | source | user_data | style_class | style_value | enabled | visible | focus | dirty | layout | theme | event |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| window | support | support | reject | reject | reject | reject | reject | support | support | support | support | support | reject | reject | support | support | reject |
| label | support | support | support | reject | reject | reject | reject | support | support | support | support | support | reject | reject | support | support | reject |
| button | support | support | support | reject | reject | reject | reject | support | support | support | support | support | reject | reject | support | support | support |
| checkbox | support | support | support | reject | support | reject | reject | support | support | support | support | support | reject | reject | support | support | support |
| switch | support | support | reject | reject | support | reject | reject | support | support | support | support | support | reject | reject | support | support | support |
| slider | support | support | reject | support | reject | support | reject | support | support | support | support | support | reject | reject | support | support | support |
| text | support | support | support | reject | reject | reject | reject | support | support | support | support | support | reject | reject | support | support | reject |
| image | support | support | reject | reject | reject | reject | support | support | support | support | support | support | reject | reject | support | reject | reject |

说明：

- `style_value` 指 `picoui_widget_set_bg_color`、`picoui_widget_set_text_color`、`picoui_widget_set_border_color`、`picoui_widget_set_radius` 四个直接 style 字段 setter；当前有 unit 字段合同，真实 backend 视觉应用仍主要由 theme apply 承担。
- `enabled/visible` 有 public setter、内部字段和 D4 unit 证据：`visible` 同步到底层 `ldBaseSetHidden`，hidden 控件从真实绘制/readback 语义中排除；`enabled` 至少在 PicoUI event bridge 层拦截 disabled/hidden 控件事件，`switch` 额外同步到真实 `ldSwitchSetDisabled`。
- `focus` 没有 public focus API；`PICOUI_STATE_FOCUSED` 只是 theme state，不等于控件 focus 语义。
- `dirty` 没有 public dirty API；D1 的 runtime present/layout 闭环不能扩写成 dirty 合同。
- `image` 的 source 合同只表示 tile 指针绑定：`source == NULL` 是无源/清空绑定，此时 backend 保持真实 `ldImage`，`img_tile/mask_tile` 均为空；非空 source 必须提供 `img_tile`，`mask_tile == NULL` 表示无遮罩图片。
- `image` 的 theme 当前明确拒绝；`picoui_theme_apply_to_widget(... image ..., PICOUI_PART_MAIN, ...)` 返回失败是当前合同，且不得污染 image source 或 backend tile 绑定。
- `layout` 对子控件是通用 child layout API；对 `window` 是 flex/grid window layout API。
- `picoui_widget_set_padding` 对非 window 控件当前只记录 PicoUI widget 字段/unit 合同；真实 LingDongGUI backend window padding 行为由 `window` 行的 layout 证据支撑。

## 证据层矩阵

| 控件 | unit | contract | mapping | visible | manual artifact |
| --- | --- | --- | --- | --- | --- |
| window | support：`test_picoui_layout.c`、`test_picoui_theme.c` 覆盖 window layout/theme/visible state；通用 direct style setter 由 `test_picoui_widgets.c` 覆盖字段合同 | support：`check_picoui_public_api.py`、`check_picoui_demo_boundary.py`、`check_picoui_widget_contract_matrix.py` | support：各 demo root window 是真实 backend tree 根 | support：`check_picoui_visible_ui.py --all` 间接覆盖窗口承载与布局显示 | deferred：需单独运行 manual window artifact gate |
| label | support：`test_picoui_widgets.c` 覆盖 create/text/font/tree、通用 direct style 字段合同和非 window padding 字段合同，`test_picoui_theme.c` 覆盖 theme/visible state | support：public/demo/matrix contract | support：`hello_world`、`theme_showcase`、`settings_panel` 的 label id 进入真实 backend ids | support：hello/theme/settings visible gate 覆盖可见文本区域 | deferred：需单独运行 manual window artifact gate |
| button | support：`test_picoui_widgets.c`、`test_picoui_button_events.c`、`test_picoui_theme.c` 覆盖 create/props/text/event/style/theme，并覆盖 disabled/hidden event gate 和非 window padding 字段合同 | support：public/demo/matrix contract | support：`hello_world`、`basic_widgets`、`settings_panel` 的 button id 进入真实 backend ids | support：basic/settings/hello visible gate 覆盖按钮显示 | deferred：需单独运行 manual window artifact gate |
| checkbox | support：`test_picoui_widgets.c`、`test_picoui_button_events.c`、`test_picoui_theme.c` 覆盖 create/props/text/checked/event/theme/native callback user_data；通用 direct style setter 与非 window padding 由 `test_picoui_widgets.c` 覆盖字段合同 | support：public/demo/matrix contract | support：`basic_widgets` 的 `agree` 进入真实 backend ids | support：basic widgets visible gate 覆盖基础可见区域 | deferred：需单独运行 manual window artifact gate |
| switch | support：`test_picoui_widgets.c`、`test_picoui_button_events.c`、`test_picoui_theme.c` 覆盖 create/props/checked/event/theme/native callback user_data/enabled state；通用 direct style setter 与非 window padding 由 `test_picoui_widgets.c` 覆盖字段合同 | support：public/demo/matrix contract | support：`basic_widgets`、`settings_panel` 的 switch id 进入真实 backend ids | support：basic/settings visible gate 覆盖基础可见区域 | deferred：需单独运行 manual window artifact gate |
| slider | support：`test_picoui_widgets.c`、`test_picoui_button_events.c`、`test_picoui_theme.c` 覆盖 create/props/value/range/event/theme/native callback user_data；通用 direct style setter 与非 window padding 由 `test_picoui_widgets.c` 覆盖字段合同 | support：public/demo/matrix contract | support：`basic_widgets`、`settings_panel` 的 slider id 进入真实 backend ids | support：basic/settings visible gate 覆盖基础可见区域 | deferred：需单独运行 manual window artifact gate |
| text | support：`test_picoui_widgets.c` 覆盖 create/text/font/tree、通用 direct style 字段合同和非 window padding 字段合同，`test_picoui_theme.c` 覆盖 theme | support：public/demo/matrix contract | support：`theme_showcase` 的 body text id 进入真实 backend ids；`basic_widgets` 目前未把 text id 列入 mapping gate | support：theme/basic visible gate 间接覆盖文本区域；basic gate 不逐 id 判定 text | deferred：需单独运行 manual window artifact gate |
| image | support：`test_picoui_widgets.c` 覆盖 create/source/tree、空 source/清空 source时 backend 真实 `ldImage` 且 tile/mask 为空、无效 source 拒绝、无遮罩 source、theme reject 不污染 source/backend tile、通用 direct style 字段合同和非 window padding 字段合同，`test_picoui_theme.c` 覆盖 theme reject | support：public/demo/matrix contract | support：`basic_widgets` 的 `logo` 进入真实 backend ids | support：basic widgets visible gate 只能证明 image 区域或真实对象路径可见、可捕获；无真实图片源时不证明占位资源绑定，也不代表图片资源加载完成 | deferred：需单独运行 manual window artifact gate |

## 后续边界

- D3 已为当前控件补齐 `create_with_props` 一致性；本矩阵不要求当前任务修复 deferred 项。
- D3+ 若要把 `enabled/visible/focus/dirty` 从 `deferred` 或 `reject` 推到 `support`，必须同时补 public API 口径、backend 行为和对应证据层。
- F 线新控件不属于本矩阵。
