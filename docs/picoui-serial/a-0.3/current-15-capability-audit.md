# Current-15 Capability Audit

## 分层口径

本文件只使用三种当前层级：

1. `full parity complete`
2. `stable contract but not full parity`
3. `minimal vertical slice only`

这些都是当前盘点口径，不代表最终全部完成。

## J线 4

### `window`

- 当前层级：`full parity complete`
- public API：`tinyui_window_create()/tinyui_window_create_with_props()/tinyui_window_set_background_source()/tinyui_window_set_padding_group()/tinyui_window_get_padding_left|top|right|bottom()`
- props/create_with_props：`tinyui_window_props` 当前真实提交 `style_class/user_data/bg_color/text_color/border_color/radius/padding/background_source/padding_group`；其中 `has_padding_group` 控制是否下发四向 padding group。
- getter/readback：当前只有诚实最小读回，公开 getter 仅覆盖 `padding_left/top/right/bottom`，没有额外窗口状态 getter。
- backend mapping：真实走 `tinyui_backend_create_window()` -> `ldWindow`，背景图经 `ldWindowSetImage(...)`，背景色经 `ldWindowSetColor(...)`，padding group 经 `ldWindowSetPaddingGroup(...)`。
- 已完成项：窗口 create/create_with_props、背景图与 mask 路径、背景色、padding group 合同、以及 J 线要求的 `honest minimal readback` 已落到真实 backend。
- 未完成项：`a-0.3` 不继续把更高阶窗口语义或更多读回面写成新增 parity；`text_color/border_color/radius/padding` 目前主要仍是 widget/style 元数据提交，不应扩写成新增 window-specific getter 合同。

### `label`

- 当前层级：`full parity complete`
- public API：`tinyui_label_create()/tinyui_label_create_with_props()/set_text/get_text/set_font/set_text_color/get_text_color/set_bg_color/get_bg_color/set_transparent/get_transparent/set_align/get_align/set_background_source`
- props/create_with_props：`tinyui_label_props` 当前真实覆盖 `text/font/style_class/user_data/width/height/bg_color/text_color/border_color/radius/padding/transparent/align/background_source`，并在 create_with_props 中顺序下发到 widget + backend。
- getter/readback：当前公开读回已覆盖 `get_text/get_text_color/get_bg_color/get_transparent/get_align`；没有独立 `background_source` getter，也没有额外 font getter。
- backend mapping：真实走 `tinyui_backend_create_label()` -> `ldLabel`，文本经 `tinyui_backend_set_text()`，颜色/透明/对齐分别映射到 `ldLabel` 对应 setter，背景图走 backend label image/mask 路径。
- 已完成项：文本、字体、背景色、文字色、透明、对齐、background image/mask、以及 `text_color/bg_color/align/transparent` 读回合同都已形成当前 J 线 parity 面。
- 未完成项：`a-0.3` 不继续把更多装饰能力写成新增 parity 面；`background_source` 仍是写入合同，不应写成已有公开 readback。

### `button`

- 当前层级：`full parity complete`
- public API：`tinyui_button_create()/tinyui_button_create_with_props()/set_text/set_font/set_release_image/set_press_image/set_transparent/get_transparent/set_checkable/get_checkable/set_key_value/get_key_value/set_pressed/get_pressed/set_on_clicked/set_on_pressed/set_on_released`
- props/create_with_props：`tinyui_button_props` 当前真实覆盖 `text/font/release_image/press_image/transparent/checkable/key_value/pressed/width/height/on_clicked/user_data/style_class/bg_color/text_color/border_color/radius/padding`。
- getter/readback：当前公开读回只覆盖 `transparent/checkable/key_value/pressed`；文本、图片和字体没有对应 getter。
- backend mapping：真实走 `tinyui_backend_create_button()` -> `ldButton`，release/press image 分别映射到 `ldButtonSetImage(...)` 的 release/press 槽位，透明/checkable/key_value/pressed 都直接映射到 `ldButton` 原生状态。
- 已完成项：文本、字体、clicked/pressed/released 三类 callback、release/press image、transparent、checkable、key_value、pressed state 已形成当前 J 线 parity 合同。
- 未完成项：不把更高阶复合 button 语义提前写入本阶段；`on_pressed/on_released` 是事件注册面，不应误写成独立状态 readback。

### `slider`

- 当前层级：`full parity complete`
- public API：`tinyui_slider_create()/tinyui_slider_create_with_props()/set_value/set_range/set_horizontal/get_horizontal/set_background_source/set_indicator_source/set_indicator_width/set_slim_size/get_percent/set_on_value_changed`
- props/create_with_props：`tinyui_slider_props` 当前真实覆盖 `min_value/max_value/value/on_value_changed/user_data/style_class/width/height/bg_color/text_color/border_color/radius/padding/horizontal/background_source/indicator_source/indicator_width/slim_size`，其中方向和图像/尺寸项由 `has_*` 开关控制是否下发。
- getter/readback：当前公开读回只覆盖 `get_horizontal/get_percent`；没有 `get_value/get_range/get_indicator_width/get_slim_size` 这类公开 getter。
- backend mapping：真实走 `tinyui_backend_create_slider()` -> `ldSlider`，值与范围通过 backend value update 驱动原生 percent，方向经 `ldSliderSetHorizontal(...)`，背景/指示器图像与 mask、indicator width、slim size 都直接映射到 `ldSlider`。
- 已完成项：`create_with_props`、值、范围、方向、background/indicator image+mask、indicator width、slim size、`on_value_changed`，以及 `percent + orientation` 诚实读回一致性都已落到真实 backend。
- 未完成项：`a-0.3` 不再扩增 slider 新合同；当前必须保持 release matrix 的诚实边界，不把不存在的 `value/range` getter 或更多 slider 状态读回写成已支持。

## a-01 5

### `image`

- 当前层级：`stable contract but not full parity`
- public API：`create/create_with_props/set_source`
- props/create_with_props：`tinyui_image_props` 当前接受 `id/source/style_class/user_data/width/height/bg_color/text_color/border_color/radius/padding`，但只有 `source/size` 属于真实稳定能力；`style_class/user_data` 只是 metadata 写入；颜色、圆角、padding 只保留 widget 层记录与 reject/deferred 边界，不应宣称为 image 已支持样式面
- getter/readback：无独立 `source` getter；当前只能诚实承认 `image->source` 与 backend `image_source` 在实现/测试层保持一致，`style_class/user_data` 也只是 widget/backend wrapper metadata 可读，不构成更高阶 public readback 合同
- backend mapping：真实走 `ldImage`
- 已完成项：`tinyui_image_set_source()` 真实下沉到 `ldImageSetImage()`；`create_with_props` 可稳定落 `source` 与 `size`；空 source/清空 source/无遮罩 source 都已有单测边界；`style_class/user_data` metadata-only 合同已有测试固定
- 未完成项：
  - `theme` 仍是明确 `reject`，`PICOUI_PART_MAIN` / `PICOUI_PART_TEXT` 都不能写成已支持
  - `style_class/user_data` 仍是 metadata-only `incomplete_contract`，当前没有真实 backend 消费语义
  - `bg_color/text_color/border_color/radius` 不应写成 image 已支持样式能力；`padding` 也不是当前稳定 image 合同
  - `enabled` 仍是明确 `reject`；`tinyui_widget_set_enabled()` 对 image 返回失败且不会改写 enabled state

### `text`

- 当前层级：`stable contract but not full parity`
- public API：`create/create_with_props/set_text/set_font`
- props/create_with_props：`tinyui_text_props` 当前接受 `id/text/font/style_class/user_data/width/height/bg_color/text_color/border_color/radius/padding`；`text/font/bg_color/text_color/size` 已有真实写入路径，`style_class/user_data` 仍只是 metadata，border/radius/padding 只可写成当前实现记录与 theme/style 子集的一部分，不能外推成完整文本样式系统
- getter/readback：没有独立 `get_text/get_font` public API；当前稳定 readback 边界是 `widget.text`、`widget.font` 与 backend font 指针在单测层可核对，颜色/背景则通过 theme/style 测试验证 widget 与 `ldText` 一致，不等于公开完整读回接口
- backend mapping：真实走 `ldText`
- 已完成项：`tinyui_text_set_text()` 真实走 `tinyui_backend_set_text()` -> `ldTextSetText()`；`tinyui_text_set_font()` 已收口到真实 font resolve/apply/fallback 路径；`text/font/bg/text_color/align` 已被 a-01 文档定为当前稳定读写合同；theme 的 `PICOUI_PART_TEXT` 路径已有 `ldText` 样式同步测试
- 未完成项：
  - 不把更重文本模型、owned/static text 生命周期、复杂字体资源系统、scroll/seek/background image 等更高阶能力写成当前支持
  - 公开 readback 面仍弱于 `J线 4` 控件；现在是“实现可核对 + 合同已稳定”，不是“public getter 已齐”

### `checkbox`

- 当前层级：`stable contract but not full parity`
- public API：`create/create_with_props/set_checked/is_checked/set_text/set_on_toggled`
- props/create_with_props：`tinyui_checkbox_props` 当前接受 `id/text/checked/on_toggled/user_data/style_class/width/height/bg_color/text_color/border_color/radius/padding`；其中 `checked/text/on_toggled` 是稳定能力，`style_class/user_data` 仍主要是 metadata/回调 cookie 边界，颜色与尺寸通过真实 backend/style 路径生效，但不等于完整 checkbox parity
- getter/readback：公开 getter 只有 `is_checked`；文本、style、user_data 目前仍主要依赖 widget/backend 结构体与测试核对，不是独立 public readback 面
- backend mapping：真实走 `ldCheckBox`
- 已完成项：`checked` 与 native `ldCheckBox` 状态同步；`set_text` 真实走 backend 文本下沉；`toggled` callback 走 native event bridge，不是 shadow state 自转；theme/style 可真实作用到 `ldCheckBox` 颜色与文本色
- 未完成项：
  - 不把 radio group、image mode、text/image 组合排版、导航等更重能力扩写成当前已完成
  - 现有 getter/readback 仍以 `checked` 为主，不能写成 checkbox 全量状态/样式读回已齐

### `switch`

- 当前层级：`stable contract but not full parity`
- public API：`create/create_with_props/set_checked/is_checked/set_on_toggled`
- props/create_with_props：`tinyui_switch_props` 当前接受 `id/checked/on_toggled/user_data/style_class/width/height/bg_color/text_color/border_color/radius/padding`；`checked/on_toggled` 是当前稳定合同，`style_class/user_data` 仍偏 metadata，颜色/尺寸/padding 通过真实 backend style 与 size 路径下沉，但不应外推成完整 switch skin 系统
- getter/readback：公开 getter 只有 `is_checked`；disabled/readback 目前是通过 widget state 与 `ldSwitchIsDisabled()` 测试固定，不是独立 public getter 集合
- backend mapping：真实走 `ldSwitch`
- 已完成项：`checked` 与 native state 同步；`toggled` callback 走真实 native bridge；`tinyui_widget_set_enabled()` 已把 disabled 状态同步到底层 `ldSwitchSetDisabled()`；theme/style 已能真实驱动 track/edge 等当前子集样式
- 未完成项：
  - 不把 `direction/navigation`、track/knob image skin 等更重语义写成 support
  - 公开 readback 仍以 `checked` 为主，不能写成 switch 全量状态/视觉合同都已收口

### `list`

- 当前层级：`stable contract but not full parity`
- public API：`create/create_with_props/add_item/set_selected_index/get_selected_index/set_on_selected`
- props/create_with_props：`tinyui_list_props` 当前只接受 `id/style_class/user_data`；`items` 与 `selected_index` 不是 props 字段，而是通过 `add_item()` / `set_selected_index()` 后续建立合同。`style_class/user_data` 当前都只是 metadata-only 边界，不能写成 backend 已消费列表外观/业务数据
- getter/readback：公开 getter 只有 `get_selected_index`；当前稳定读回是 selection index，以及 widget-level `user_data` 与 callback cookie 的明确分离。item id / backend internal storage strategy 不属于 public readback 合同
- backend mapping：真实走 `ldList`
- 已完成项：`add_item()` 真实下沉到 `ldListSetText()`；`set_selected_index/get_selected_index` 与 backend selected item 一致；enabled 真实同步到底层 `ldBaseSetSelectable()`；`on_selected` callback 走 native selected bridge，且 callback cookie 与 widget-level `user_data` 已稳定分离
- 未完成项：
  - `item_marker` 明确不是独立 backend widget，也不能把 item id 写成 list/backend/widget identity
  - `style_class` 与 widget-level `user_data` 仍是 metadata-only `incomplete_contract`，不是 backend 已消费能力
  - 虽然 widget-level `user_data` 与 callback cookie 已分离，但这只说明合同边界稳定，不等于 list 全面 parity

## a-02 6

### `progress_bar`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_percent/get_percent/set_horizontal/get_horizontal`
- props/create_with_props：已覆盖 `id/percent/horizontal/style_class/user_data`；`create()` 默认落 `percent=0`、`horizontal=0`，`create_with_props()` 要求 `percent` 在 `0..100` 内。
- getter/readback：`get_percent/get_horizontal` 都直接回读 backend 当前值；`horizontal` 是布尔口径，setter 会把任意非零值归一成 `1`。
- backend mapping：真实走 `tinyui_backend_create_progress_bar()` -> `ldProgressBar_init(...)`；百分比经 `ldProgressBarSetPercent(...)`，方向经 `ldProgressBarSetHorizontal(...)`，getter 直接读 `permille/isHorizontal`。
- 已完成项：基础百分比、横竖方向、`create_with_props` 最小合同、参数越界拒绝、demo/unit/runtime/mapping/visible 证据都已存在。
- 未完成项：
  - 这只证明最小 progress widget 已接到真实 backend，不证明复杂皮肤、主题扩展、尺寸/配色公开合同或更多 progress bar 配置已对齐。
  - 当前没有超出 `percent/horizontal` 的公开 getter，也不能把 backend 默认尺寸和内部配色写成 TINYUI 已承诺合同。

### `qrcode`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_text/get_text`
- props/create_with_props：已覆盖 `id/text/style_class/user_data`；`create_with_props()` 要求 `text` 非空，`create()` 默认先创建一个空字符串二维码。
- getter/readback：`get_text` 直接回读 backend `ldQRCode` 当前字符串；它证明文本写入链路，不等于二维码参数面已有完整 readback。
- backend mapping：真实走 `tinyui_backend_create_qrcode()` -> `ldQRCode_init(...)`；文本经 `ldQRCodeSetText(...)` 写入，getter 直接读 `ldQRCode->pStr`。
- 已完成项：文本写入、空文本默认态、基础读回、参数校验、demo/unit/runtime/mapping/visible 证据都已存在。
- 未完成项：
  - TINYUI 目前没有暴露纠错等级、缩放倍数、边距、前景/背景色、尺寸等公共合同，不能借 backend 初始化默认值写成已支持。
  - 当前能力只够证明“真实二维码控件 + 文本输入”这条竖切片，不是二维码配置 parity。

### `progress_wheel`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_percent/get_percent`
- props/create_with_props：已覆盖 `id/percent/style_class/user_data`；`create()` 默认 `percent=0`，`create_with_props()` 要求 `percent` 在 `0..100` 内。
- getter/readback：`get_percent` 直接回读 backend 当前进度值；没有额外动画状态、颜色状态或样式读回。
- backend mapping：真实走 `tinyui_backend_create_progress_wheel()` -> `ldProgressWheel_init(...)`；进度经 `ldProgressWheelSetProgress(...)` 写入，getter 直接读 `iProgress`；backend 还显式关闭 dirty-region helper 路径以适配 TINYUI host scene。
- 已完成项：百分比合同、参数越界拒绝、真实 backend 绑定、host runtime 崩溃规避、demo/unit/runtime/mapping/visible 证据都已存在。
- 未完成项：
  - 不证明更高阶动画、主题、尺寸、点色/轮色等能力已在 TINYUI 暴露；backend 内部默认配色不能外推成公共合同。
  - 当前 readback 只有 percent，一律不能写成 progress wheel parity complete。

### `message_box`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_title/set_message/set_confirm_text/set_on_confirm/get_title/get_message/get_confirm_text`
- props/create_with_props：已覆盖 `id/title/message/confirm_text/style_class/user_data`；`id` 必填，但 `title/message/confirm_text` 都允许缺省。
- getter/readback：`get_title/get_message/get_confirm_text` 只读 TINYUI host 侧缓存字段，不是从 `ldMessageBox` 反查出来的 backend readback；未设置时返回 `NULL`。
- backend mapping：真实走 `tinyui_backend_create_message_box()` -> `ldMessageBox_init(...)`；标题/正文/确认文案分别经 `ldMessageBoxSetTitle/SetMsg/SetBtn(...)` 写入，`set_on_confirm()` 通过 `ldMessageBoxSetCallback(...)` 挂接 confirm bridge。
- 已完成项：标题/消息/单确认按钮文案、confirm callback bridge、disabled/hidden 时 bridge 不触发的 backend 防护、demo/unit/runtime/mapping/visible 证据都已存在。
- 未完成项：
  - 当前只证明单确认按钮 message box 竖切片；不能把多按钮、取消按钮、返回值模型、模态控制、布局定制等写成已对齐。
  - getter 只是 host cache，不应误写成 message box 已有完整 backend readback 面。

### `date_time`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_format/set_date/set_time/get_format`
- props/create_with_props：已覆盖 `id/format/year/month/day/hour/minute/second/style_class/user_data`；`format` 必填，月/日/时/分/秒都做基本区间校验。`create()` 默认先落 `yyyy-mm-dd hh:nn:ss` + `2026-01-01 12:00:00`。
- getter/readback：当前只有 `get_format`，且直接回读 backend `ldDateTime` 的格式串；日期和时间没有独立 public getter。
- backend mapping：真实走 `tinyui_backend_create_date_time()` -> `ldDateTime_init(...)`；格式/日期/时间分别经 `ldDateTimeSetFormat/SetDate/SetTime(...)` 写入，getter 直接读 `formatStr`。
- 已完成项：格式、日期、时间写入合同，基础参数校验，手动值在 `ldDateTime_on_frame_start()` 后仍保留且 `isAutoSysTime == false` 的问题已由单测覆盖；demo/unit/runtime/mapping/visible 证据都已存在。
- 未完成项：
  - 不证明更复杂日期时间模式、locale、时区、系统自动同步策略或日期/时间 readback 已完整对齐。
  - 当前只能诚实写成“格式可读回，日期时间可写入”，不能扩写成完整 date-time parity。

### `clock`

- 当前层级：`minimal vertical slice only`
- public API：`create/create_with_props/set_step_second/get_step_second`
- props/create_with_props：已覆盖 `id/step_second/style_class/user_data`；`step_second` 只接受 `0/1`，`create()` 默认 `0`。
- getter/readback：`get_step_second` 直接回读 backend `isStepSecond`；没有当前时刻、表盘资源、指针样式等公共 getter。
- backend mapping：真实走 `tinyui_backend_create_clock()` -> `ldClock_init(...)`；`step_second` 经 `ldClockSetStepSecond(...)` 写入，getter 直接读 `isStepSecond`。
- 已完成项：`step_second` 最小合同、参数越界拒绝、真实 backend 绑定、三针时钟 visible 规则、demo/unit/runtime/mapping 证据都已存在。
- 未完成项：
  - 不证明背景表盘资源、复杂动画、时区/时间源、指针/刻度配置等高级时钟能力已对齐。
  - 当前只是最小 step-second 竖切片，不应写成时钟组件已达到 parity complete。

## 当前主要结论

1. current-15 中只有 `window / label / button / slider` 可以维持旧 parity complete 口径。
2. `image / text / checkbox / switch / list` 应归为 `stable contract but not full parity`，不能继续写成旧 `v0.2 backlog`。
3. `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 只能诚实写成 `minimal vertical slice only`，不能偷换成 parity complete。
4. `a-0.3` 的价值不是把这些缺口补完，而是先把当前状态写实、写稳、写一致。
