# TINYUI G线当前控件 Capability Gap Matrix

本文是 `G1` 冻结真相源，只记录当前 `TINYUI` public API、对应真实 `LingDongGUI` backend 入口、状态和证据层，不进入 `G2+` 的实现补齐。

## 口径

- `support`：当前 public API、真实 `ld*` backend 入口和至少一层证据可以共同支撑该能力项合同。
- `reject`：当前无 public API，或当前语义明确不支持，调用方不能依赖。
- `deferred`：能力方向合理，但当前阶段明确不承诺闭环，后续阶段再决定是否进入实现。
- `incomplete_contract`：当前 public API 或 backend 入口已存在，但合同边界、证据层或真实语义仍不完整，不能按完整支持对外宣称。
- 证据层沿用 `unit / contract / mapping / visible / manual artifact`。这些层级不等价，`mapping` 不等于完整 UI，`visible` 不等于所有语义已闭环。

## 控件能力矩阵

| 控件 | TINYUI public API / 能力项 | 对应 ld* 能力入口 | 状态 | 证据层 | 备注 |
| --- | --- | --- | --- | --- | --- |
| window | `tinyui_window_create()`、`tinyui_window_create_with_props()` | `ldWindow_init` | support | contract / mapping / visible | root window 与 layout 容器入口已存在 |
| window | `tinyui_flex_set_flow()`、`tinyui_flex_set_align()`、`tinyui_flex_set_gap()`、`tinyui_grid_set_columns()`、`tinyui_grid_set_rows()`、`tinyui_grid_set_gap()`、`tinyui_grid_set_align()`、`tinyui_widget_set_padding()` | `ldWindowSetLayout`、`ldWindowSetFlexFlow`、`ldWindowSetFlexAlign`、`ldWindowSetFlexTrackAlign`、`ldWindowSetFlexGap`、`ldWindowSetGridColumns`、`ldWindowSetGridDscArray`、`ldWindowSetGridGap`、`ldWindowSetGridAlign`、`ldWindowSetPadding`、`ldWindowSetGridPadding` | support | unit / mapping / visible | `window` 是当前 layout 主承载控件 |
| window | `tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_theme_apply_to_widget()` | `ldWindowSetColor`、`ldBaseSetHidden` | support | unit / mapping / visible | `enabled` 主要是 TINYUI state/event gate，非 `ldWindow` 专属 disabled 语义 |
| label | `tinyui_label_create()`、`tinyui_label_create_with_props()` | `ldLabel_init` | support | contract / mapping / visible | - |
| label | `tinyui_label_set_text()`、`tinyui_label_set_font()`、`tinyui_widget_set_text()` | `ldLabelSetText`、`ldLabelSetFont` | support | unit / mapping / visible | `tinyui_widget_set_text()` 共享落到 `ldLabelSetText` |
| label | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldLabelSetBackgroundColor`、`ldLabelSetTextColor`、`ldBaseSetHidden` | support | unit / mapping / visible | - |
| button | `tinyui_button_create()`、`tinyui_button_create_with_props()` | `ldButton_init` | support | contract / mapping / visible | - |
| button | `tinyui_button_set_text()`、`tinyui_widget_set_text()` | `ldButtonSetText` | support | unit / mapping / visible | - |
| button | `tinyui_button_set_on_clicked()`、`tinyui_button_set_on_pressed()`、`tinyui_button_set_on_released()` | `backend_event` button bridge，底层对象为 `ldButton` | support | unit / contract | public callback 合同明确，visible 不单独证明事件语义 |
| button | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldButtonSetColor`、`ldButtonSetTextColor`、`ldBaseSetHidden` | support | unit / mapping / visible | - |
| checkbox | `tinyui_checkbox_create()`、`tinyui_checkbox_create_with_props()` | `ldCheckBox_init` | support | contract / mapping / visible | - |
| checkbox | `tinyui_checkbox_set_text()`、`tinyui_widget_set_text()` | `ldCheckBoxSetText` | support | unit / mapping / visible | - |
| checkbox | `tinyui_checkbox_set_checked()`、`tinyui_checkbox_is_checked()`、`tinyui_checkbox_set_on_toggled()` | `_ldCheckBoxSetChecked`、`ldCheckBoxIsChecked`，native callback 经 `backend_event` bridge | support | unit / contract | setter-path 与 native-event-path 已区分 |
| checkbox | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldCheckBoxSetColor`、`ldCheckBoxSetTextColor`、`ldBaseSetHidden` | support | unit / mapping / visible | - |
| switch | `tinyui_switch_create()`、`tinyui_switch_create_with_props()` | `ldSwitch_init` | support | contract / mapping / visible | - |
| switch | `tinyui_switch_set_checked()`、`tinyui_switch_is_checked()`、`tinyui_switch_set_on_toggled()` | `_ldSwitchSetChecked`、`ldSwitchIsChecked`，native callback 经 `backend_event` bridge | support | unit / contract | `enabled` 额外同步 `ldSwitchSetDisabled` |
| switch | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldSwitchSetColor`、`ldBaseSetHidden`、`ldSwitchSetDisabled` | support | unit / mapping / visible | - |
| slider | `tinyui_slider_create()`、`tinyui_slider_create_with_props()` | `ldSlider_init` | support | contract / mapping / visible | - |
| slider | `tinyui_slider_set_value()`、`tinyui_slider_get_value()`、`tinyui_slider_set_on_value_changed()` | `tinyui_backend_widget_update_value`、`ldSliderSetPercent`，native callback 经 `backend_event` bridge | support | unit / contract | 对外是 value 语义，对底层主要是 percent 更新 |
| slider | `tinyui_slider_set_range()` | 无独立 `ldSlider` range API；通过 TINYUI `min_value/max_value` 与统一 backend helper 把当前 value 归一到真实 `ldSlider` percent | support | unit / contract | `G6` 已收口为 TINYUI 自身的 `min/max <-> value <-> percent` 归一化合同，不宣称底层存在独立 range 对象模型 |
| slider | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldSliderSetColor`、`ldBaseSetHidden` | support | unit / mapping / visible | - |
| text | `tinyui_text_create()`、`tinyui_text_create_with_props()` | `ldText_init` | support | contract / mapping / visible | - |
| text | `tinyui_text_set_text()`、`tinyui_widget_set_text()` | `ldTextSetText` | support | unit / mapping / visible | `tinyui_widget_set_text()` 当前共享落到 `ldTextSetText` |
| text | `tinyui_text_set_font()` | backend text font 最小内置映射/fallback + `ldText` runtime rebind + `text_box` consumed font update | support | unit / contract | `Text Font 合同线` 已收口为“描述值 font -> backend 最小映射/回退 -> 真实 `ldText/text_box` 更新”合同；当前不承诺完整 family/size 字体解析，也不承诺 public 指针身份保持 |
| text | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()`、`tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()`、`tinyui_widget_set_padding()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_enabled()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()`、`tinyui_theme_apply_to_widget()` | `ldTextSetBackgroundColor`、`ldTextSetTextColor`、`ldBaseSetHidden` | support | unit / mapping / visible | - |
| image | `tinyui_image_create()`、`tinyui_image_create_with_props()` | `ldImage_init` | support | contract / mapping / visible | - |
| image | `tinyui_image_set_source()` | `ldImageSetImage` | support | unit / mapping | 当前是 source/tile 绑定边界，不代表完整资源系统 |
| image | theme | 无；当前 theme apply 到 image 明确拒绝 | reject | unit / contract | 不得把 theme token 写成 image 已支持 |
| image | `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()` | `ldBaseSetX/Y/Width/Height/Hidden/Flex*/Grid*` | support | unit / mapping / visible | 这些 layout / visibility setter 当前都走真实 `ldBase` 通用入口，不应继续与 image style gap 混写 |
| image | `tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()` | 无真实 `ldImage` 消费链；当前仅为 TINYUI / backend wrapper metadata 存储 | incomplete_contract | unit / contract | 这两项当前只证明 metadata 被存储，不代表 image 已有真实 style/public contract |
| image | `tinyui_widget_set_bg_color()`、`tinyui_widget_set_text_color()`、`tinyui_widget_set_border_color()`、`tinyui_widget_set_radius()` | image style backend 当前明确不支持；theme / backend style dispatch 对 image 走 reject 路径 | reject | unit / contract | 不得把这些视觉 style setter 写成“只差一点闭环” |
| image | `tinyui_widget_set_padding()` | 当前仅保留 API 形状；image padding 的真实语义尚未定义 | deferred | unit / contract | 既不是现成 reject，也不是已有稳定合同；后续若要推进需先定义它代表什么 |
| image | `tinyui_widget_set_enabled()` | 无 image-specific backend bridge，也无自然的 `ldImage disabled` 语义 | reject | unit / contract | 不能把 hidden / opacity / selectable 偷换成 enabled |
| list | `tinyui_list_create()`、`tinyui_list_create_with_props()` | `ldList_init` | support | unit / contract / mapping | `tinyui_list_create_with_props()` 已能落到真实 `ldList` 创建 |
| list | `tinyui_list_add_item()` | `ldListSetText` | support | unit / contract | `G7` 已收口为 append 后触发 full text snapshot apply；`id` 仅保留为 TINYUI / wrapper 侧稳定 key，不对 item 侧提供 runtime mapping marker 结论 |
| list | `tinyui_list_set_selected_index()`、`tinyui_list_get_selected_index()` | `ldListSetSelectItem`、`ldListGetSelectItem` | support | unit / contract / mapping | 选择索引 public API 与 backend 选择入口一致 |
| list | `tinyui_list_set_on_selected()` | `backend_event` list native bridge，监听 `ldList` 发出的 `SIGNAL_CLICKED_ITEM` 并用 `native_value` 回调 TINYUI | support | unit / contract | native selection change 已能桥接到 TINYUI callback；当前证据不外推到 marker/style 等其他 list gap |
| list | item marker | 无独立 TINYUI public API；底层 `ldList` 只有 selected item 与颜色/对齐等通用能力 | reject | contract / mapping | 当前没有“marker” public 合同；`G3` 只把 `item_*` 从强 mapping marker contract 中剥离，不再对 item 侧给 runtime marker 结论 |
| list | `tinyui_theme_apply_to_widget()` with `PICOUI_PART_MAIN` | `ldListSetBackgroundColor`、`ldListSetSelectColor` | support | unit / contract | `G4` 仅收口可真实证明的 list theme/style part：`MAIN` 映射背景与选中色；不扩展到 `TEXT/INDICATOR/KNOB/TRACK` |
| list | `tinyui_widget_set_visible()` | `ldListSetHidden` -> `ldBaseSetHidden` | support | unit / contract | 真实 hidden 语义已存在，`G4` 仅补单测证据，不新增 backend 行为 |
| list | `tinyui_widget_set_enabled()` | `ldBaseSetSelectable` + `ldList` native interactive gate | support | unit / contract | `G8` 收口为真实 selectable / interactive 语义：disabled 时不处理 `PRESS/HOLD_DOWN`、不发 selected click；该结论不外推到 disabled 视觉或 theme 系统 |
| list | `tinyui_widget_set_style_class()` | 无真实 `ldList` 消费链；当前仅为 TINYUI / backend wrapper metadata 存储 | incomplete_contract | contract / mapping | 有 public API 和 wrapper 存储，但没有真实 `ldList` style 语义 |
| list | `tinyui_widget_set_user_data()` | widget-level `user_data` 仅做通用存储；不等同于 `on_selected(..., user_data)` callback cookie | incomplete_contract | contract / mapping | `on_selected` callback cookie 已由单独的 support 行覆盖，这里只表示 widget 通用 `user_data` 合同未闭环 |

## 逐项边界说明

### list on_selected

- 当前 public API `tinyui_list_set_on_selected()` 仍由 `tinyui/src/widgets/list.c` 保存 callback 与 `user_data`，但 `tinyui/src/backend/ldgui/backend_event.c` 已补入 `PICOUI_BACKEND_WIDGET_LIST` native bridge。
- `ldList` 自己在消费 `SIGNAL_RELEASE` 后发出 `SIGNAL_CLICKED_ITEM`；TINYUI bridge 监听该 `SIGNAL_CLICKED_ITEM`，并直接使用 `native_value` 作为 selected index。
- bridge 仅当该 selected index 相对 TINYUI / backend 已知值发生变化时才更新状态并触发 callback，因此不会把相同 item 的重复 native 消息写成重复选择回调。
- 对应 `tests/tinyui/unit/test_tinyui_list.c` 已补 native selection change 的 fail-first -> green 合同测试，因此该项状态提升为 `support`。

### list add_item

- `G7` 只把 `tinyui_list_add_item()` 收口为 “append -> full text snapshot -> real ldList render/select by index” 的最小合同。
- `tinyui_list_add_item()` 当前会先准备 TINYUI / wrapper 侧的下一版 item snapshot，调用 backend 全量刷新 `ldList` 文本组；只有 backend 成功后才提交 `items[]` 与 `item_count`，失败时不会留下半提交分裂。
- `id` 继续只作为 TINYUI / wrapper 侧稳定 key 存储，用于保持 API 形状和未来扩展预留；当前不对 runtime marker、真实 backend object id、per-item native object 提供任何结论。
- `ldList` 真实闭环仍是文本数组与按 index 选择：`ldListSetText()` 消费的是整组文本快照，`set_selected_index()` 与 native selected bridge 继续按追加后的 index 工作。
- 对应 `tests/tinyui/unit/test_tinyui_list.c` 已补 snapshot 顺序、backend 失败无半提交分裂、append 后 select-by-index / native selected bridge 继续工作等合同测试，因此该项状态提升为 `support`。

### list item marker

- 当前 `TINYUI` public header 没有 marker、bullet、leading icon、selected indicator 等 list item marker API。
- `ldList` 底层存在选中项、文本、颜色等内部绘制能力，但这不自动构成 TINYUI marker 合同。
- `G3` 已把 `item_wifi/item_bluetooth/item_display` 从 `PICOUI_BACKEND_REAL_WIDGET_IDS` 中移出，避免把这些 id 解释成“真实 widget id”。
- 当前 runtime mapping gate 不再对 item 侧给出独立强/弱 marker 结论，因此更不能把它们写成 marker / highlight / icon / bullet 已 support。
- 因此该项状态固定为 `reject`，后续若要支持，必须新增 public API、backend 映射和证据层。

### list theme/style 与 visible/enabled

- `G4` 只收口 `list` 上能被真实 `ldList` 入口证明的 theme/style 子合同，不再把 `style/visible/enabled` 混成一条笼统能力。
- `tinyui_theme_apply_to_widget()` 现在只对 `PICOUI_BACKEND_WIDGET_LIST` 开放 `PICOUI_PART_MAIN`：
  - `PICOUI_PART_MAIN` 映射到 `ldListSetBackgroundColor()` 与 `ldListSetSelectColor()`。
- `PICOUI_PART_TEXT` 当前如果写成成功路径，会让 TINYUI `widget` 状态与 backend 实际生效范围分裂，因此本轮按更保守口径回退为不支持。
- `INDICATOR/KNOB/TRACK` 没有对应的 list public contract，也没有在本阶段被虚构成支持，因此继续拒绝。
- `tinyui_widget_set_visible()` 对 list 的真实 hidden 语义原本就经 `ldListSetHidden -> ldBaseSetHidden` 闭环；`G4` 只补 unit test 证据，不改实现。
- `G8` 已把 `tinyui_widget_set_enabled()` 收口为 list 的真实 selectable / interactive 子合同：
  - TINYUI `enabled` 会同步到底层 `ldBaseSetSelectable()`。
  - `ldList` 的真实 native interactive 路径在 disabled 时会拦掉 `PRESS/HOLD_DOWN`，并禁止非 hold cleanup 场景下发 `SIGNAL_CLICKED_ITEM`。
  - hold 中途 disable 后的 `RELEASE` 只做本次交互丢弃与状态回滚，不继续产生 scroll reset、dirty、click 或 callback。
- 该项只证明 list 的 disabled interactive 语义已闭环，不代表 disabled 视觉、灰态主题或更高层系统级交互体验已一并 support。

### slider range

- `G6` 已把 `tinyui_slider_set_range()` 收口为 TINYUI 自身的 `min/max <-> value <-> percent` 归一化合同。
- 当前仍未引入独立 `ldSlider` range setter；真实 backend 继续主要消费归一化后的当前值，而不是“底层也有一套 min/max range 对象模型”。
- 现在 `tinyui_slider_set_range()` 在更新 `min_value/max_value`、钳制当前 value 后，会通过统一 backend helper 把真实 `ldSlider` percent 同步到一致状态。
- `range <= 0` 的退化区间语义也已统一：`create_with_props(min=max=value)` 与 runtime `set_range(min,max)` 均归一到相同 native percent 表示，不再出现初始化路径与变更路径不一致。
- 对应 `tests/tinyui/unit/test_tinyui_widgets.c` 已补 mutation path 与 single-point initialization path 的合同测试，因此该项状态提升为 `support`。

### text font

- `Text Font 合同线` 最终没有把 `struct tinyui_font { family, size }` 升级成 backend resource handle，也没有把 public 合同写成“保持输入指针身份”；它继续是描述值语义。
- `tinyui_text_set_font()` 现在会先走 text 专属 backend font 路径；backend 成功后才提交 TINYUI `widget.font` 与 backend wrapper `font` 状态，不再保留“先改 public 状态、再尝试 backend”的分裂路径。
- `font == NULL` 时，backend 继续把真实 `text_box` consumed font 保持在非空默认/回退字体态；`font != NULL` 时，backend 只做当前已实现的最小内置字体映射/回退，再让真实 `ldText/text_box` 完成 runtime rebind，并刷新 line metrics / reflow。
- 当前实现只证明 `Sans` 大号描述可映射到已有 `ARM_2D_FONT_16x24`、其它描述回退到已有 `ARM_2D_FONT_6x8`；不宣称完整 family/size 解析或动态字体资源加载能力。
- `ldText` 侧已经显式区分 runtime font 与 consumed font 的 owning/shared 状态，并在 `ldText_depose()` 中按 owning/shared 清理，避免误释放共享默认字体或共享 consumed font。
- 对应 `tests/tinyui/unit/test_tinyui_widgets.c` 现已覆盖：
  - `NULL` fallback 合同
  - runtime rebind 合同
  - approx backend failure 下 widget/backend/runtime/consumed font 的原子稳定性
- 因此该项现可提升为 `support`；但合同边界仍然是“描述值 font -> backend 最小映射/回退 -> 真实 `ldText/text_box` 更新”，不对完整 family/size 解析、动态字体资源加载或 public 指针身份保持作任何结论。

### image layout / visible 与 style / enabled

- `image` 这组 public setter 不能再笼统写成“一整组都未闭环”。当前 `tinyui_widget_set_pos()`、`tinyui_widget_set_size()`、`tinyui_widget_set_visible()`、`tinyui_widget_set_flex_grow()`、`tinyui_widget_set_flex_new_track()`、`tinyui_widget_set_ignore_layout()`、`tinyui_widget_set_grid_cell()` 都已走真实 `ldBase` 通用入口。
- 对应现有 unit test 也已经固定了 `image source` 的真实绑定边界，以及 `theme apply to image` 明确拒绝但不污染既有 source / widget state 的合同。
- 当前真正未闭环的是 `image` 的 style/state 子合同：`style_class / user_data / bg_color / text_color / border_color / radius / padding / enabled` 还没有稳定可宣称的 image 专属真实 backend 语义。
- 因此后续若继续推进 `image`，应按更窄的 `image style` 或 `image enabled` 子线重开，而不是把 layout / visible 与 style / enabled 再绑成一条 broad gap。

### list style_class / user_data

- `list style_class` 与 `list user_data` 不该继续绑成一行。
- `tinyui_widget_set_style_class()` 当前只把字符串分别存进 TINYUI `widget` 与 backend wrapper，没有真实 `ldList` 消费链，因此继续保持 `incomplete_contract`。
- `tinyui_widget_set_user_data()` 这条通用链路同样只做存储；而 `tinyui_list_set_on_selected(..., user_data)` 使用的是 `list` 自己的 callback 私有 `user_data` 字段，并不等同于 widget 通用 `user_data` 合同。
- 因此 `list user_data` 这条 widget-level 合同也继续保持 `incomplete_contract`；`on_selected(..., user_data)` 的 callback cookie 语义仍由单独的 support 行覆盖。

### G9 锁线结论

- `G9` 本轮完成了对剩余 gap 的再拆分取证，并推动 `Text Font`、`Image 语义`、`List Metadata` 三条后续合同线收口。
- `text font` 已从 wrapper-only 升级为最小内置映射/fallback + runtime rebind 合同；更完整的 family/size 解析或动态字体资源加载仍不在当前 G 线 support 结论内。
- `image style` 当前缺少 image-style 的真实 backend 承接点；theme / backend style dispatch 还把 image 明确固定为 reject 路径。
- `image enabled` 当前既没有 image-specific backend bridge，也没有 `ldImage` / `ldBase` 下自然的 disabled image 语义，不能把 hidden / opacity / selectable 偷换成 enabled。
- `list style_class / user_data` 仍是“wrapper 存储 + callback 私有语义”的混合体，不是单一最小缺口。
- 因此从当前证据出发，后续若继续推进，必须先定义新的 line-level 合同边界，再进入下一轮串行实现；不能把这些项直接当成现成 `G10` 实现线硬做。

## 证据层摘要

下表仅表示该控件在该证据层存在至少一个已支撑能力，不代表该控件所有能力项均为 `support`。

| 控件 | unit | contract | mapping | visible | manual artifact |
| --- | --- | --- | --- | --- | --- |
| window | support | support | support | support | deferred |
| label | support | support | support | support | deferred |
| button | support | support | support | support | deferred |
| checkbox | support | support | support | support | deferred |
| switch | support | support | support | support | deferred |
| slider | support | support | support | support | deferred |
| text | support | support | support | support | deferred |
| image | support | support | support | support | deferred |
| list | support | support | support | support | deferred |

## 当前阶段不做的事

- 不把旧 `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 迁到 G 线矩阵；它当前仍是 D 线旧矩阵 gate，只覆盖 8 控件 / 3 态。
- `G1` 只冻结 G 线真相源，不做 gate 迁移；若现在改脚本，就会把任务扩成 G 线 gate 接管，越界到 `G2+`。
- 不在 `G1` 顺手补 `list` native callback bridge、marker API、slider backend range 语义。
- 不把 `mapping`、`visible` 或 `manual artifact` 任一单层证据外推成“当前控件能力已完整闭环”。

## 旧 gate 关系说明

- 现有 `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 仍可通过。
- 这只证明未破坏旧 D 线 gate，因为该脚本继续校验 D 线旧矩阵的 8 控件 / 3 态口径。
- 这不代表 G 线矩阵已被 gate 接管；G 线矩阵在 `G1` 只是新的文档真相源，还没有进入 contract gate 迁移阶段。
