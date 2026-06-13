# TINYUI `legacy_widget_parity` 对老 SDL `uiWidgetLegacy` 的缺口审计

## 结论

当前仓库不能诚实地写成：

1. `legacy_widget_parity` 已与 `LingDongGUI` 老 `legacy-widget` demo 控件全一致。
2. `legacy_widget_parity` 已与老 demo 图片资源绑定全一致。

当前真实状态是：

1. `layout_parity` 与 `grid_parity` 已建立可对照的结构基线。
2. `legacy_widget_parity` 已完成第一版多控件大页面骨架，并覆盖了大部分 legacy 控件样本。
3. `legacy_widget_parity` 的主要缺口不是 TINYUI 没有这些控件，而是：
   - 当前接入的是通用 source，占位证明 public image pipeline 已闭环，但不是老 demo 原图资源逐张一致；
   - 还没有 TINYUI / 老 SDL 双开截图和 visible diff 证据；
   - contract 已抬高到页面级 marker，但还不是 visible parity。

## 真相源

本审计只以当前仓库代码为准：

1. 老 SDL truth-source：
   - `examples/common/demo/widget/uiWidgetLegacy.c`
2. 当前 TINYUI parity 页面：
   - `tinyui/demo/legacy_widget_parity/main.c`
3. 当前 demo boundary contract：
   - `tests/tinyui/contract/check_tinyui_demo_boundary.py`
4. 当前图片通路口径：
   - `tinyui/docs/demo_guide.md`

## 逐项对照

### 1. 已接入或基本已接入

以下项目已经出现在 `legacy_widget_parity`，但多数仍只是结构/样本级接入，不代表视觉和资源完全一致：

1. `image`
2. `button`
3. `label`
4. `checkbox / radio`
5. `switch + OFF/ON label`
6. `progress_bar`
7. `text`
8. `slider`
9. `list`
10. `combo_box`
11. `calendar`
12. `scroll_selecter`
13. `date_time`
14. `message_box`
15. `graph`
16. `table`

对应证据见：

1. `tinyui/demo/legacy_widget_parity/main.c:101-121`
2. `tinyui/demo/legacy_widget_parity/main.c:123-285`

### 2. 已进入 `legacy_widget_parity` 的补齐项

以下项目在老 `uiWidgetLegacy.c` 中存在，且当前 parity 页面已经补入：

1. `radial_menu`
2. `icon_slider`
3. `qrcode`
4. `gauge`
5. `line_edit`
6. `keyboard`
7. `arc`
8. `list item widget`
9. 子 `window` 内嵌 `button` 的局部结构

当前 parity 页面接入证据见：

1. `tinyui/demo/legacy_widget_parity/main.c`

### 3. 图片资源还没对齐的项目

老 `uiWidgetLegacy.c` 当前使用了明确的资源图：

1. `IMAGE_LETTER_PAPER_BMP`
2. `IMAGE_KEYRELEASE_PNG` / `IMAGE_KEYPRESS_PNG`
3. `IMAGE_PROGRESSBARBG_BMP` / `IMAGE_PROGRESSBARFG_BMP`
4. `IMAGE_SLIDER_PNG` / `IMAGE_INDICATOR_PNG`
5. `IMAGE_WEATHER_PNG` / `IMAGE_NOTE_PNG` / `IMAGE_BOOK_PNG` / `IMAGE_CHART_PNG`
6. `IMAGE_GAUGE_PNG` / `IMAGE_GAUGEPOINTER_PNG_Mask`
7. `IMAGE_ARC_QUARTER_PNG_Mask` / `IMAGE_ARC_QUARTER_MASK_PNG_Mask`

证据见：

1. `examples/common/demo/widget/uiWidgetLegacy.c:177-186`
2. `examples/common/demo/widget/uiWidgetLegacy.c:242-256`
3. `examples/common/demo/widget/uiWidgetLegacy.c:264-280`
4. `examples/common/demo/widget/uiWidgetLegacy.c:294-295`
5. `examples/common/demo/widget/uiWidgetLegacy.c:354-356`

而当前 `legacy_widget_parity` 的图片相关真实状态是：

1. `image / button / progress_bar / text / slider` 已接通用 `tinyui_image_source` 绑定。
2. `radial_menu / icon_slider` 已切到带 source 的 item API。
3. `gauge / arc` 已绑定 source 级 public API。
4. 这些绑定目前证明的是 TINYUI public image 通路已进入 parity 页面，不证明已经与老 demo 的原始图片资源逐张一致。

证据见：

1. `tinyui/demo/legacy_widget_parity/main.c`

## 缺口分类

### A. `public API 已接进 parity demo`

当前这批不再是“demo 未接”，而是已经进入 parity 页面：

1. `tinyui_image_set_source()`
2. `tinyui_button_set_image()`
3. `tinyui_progress_bar_set_image()`
4. `tinyui_slider_set_image()`
5. `tinyui_text_set_background_source()`
6. `tinyui_radial_menu_create()` / `tinyui_radial_menu_add_item_with_source()`
7. `tinyui_icon_slider_create()` / `tinyui_icon_slider_add_item_with_source()`
8. `tinyui_qrcode_create()`
9. `tinyui_gauge_create()` / `tinyui_gauge_set_bg_source()` / `tinyui_gauge_set_pointer_source()`
10. `tinyui_line_edit_create()`
11. `tinyui_keyboard_create()`
12. `tinyui_arc_create()` / `tinyui_arc_set_quarter_source()`
13. `tinyui_list_set_item_widget()`
14. `tinyui_calendar_set_day_names()`
15. `tinyui_window_create_child()`

对应 public 证据见：

1. `tinyui/include/tinyui/image.h`
2. `tinyui/include/tinyui/button.h`
3. `tinyui/include/tinyui/progress_bar.h`
4. `tinyui/include/tinyui/slider.h`
5. `tinyui/include/tinyui/text.h`
6. `tinyui/include/tinyui/radial_menu.h`
7. `tinyui/include/tinyui/icon_slider.h`
8. `tinyui/include/tinyui/qrcode.h`
9. `tinyui/include/tinyui/gauge.h`
10. `tinyui/include/tinyui/line_edit.h`
11. `tinyui/include/tinyui/keyboard.h`
12. `tinyui/include/tinyui/arc.h`
13. `tinyui/include/tinyui/list.h`
14. `tinyui/include/tinyui/calendar.h`
15. `tinyui/include/tinyui/window.h`

### B. `图片资源通路` 还没形成 demo 级真闭环

这类已经不是“source API 没接进 demo”，而是：

1. parity demo 当前绑定的是通用 source，占位证明 TINYUI public image pipeline 已闭环；
2. demo guide 仍明确 `tinyui_image_set_source()` 只绑定调用方提供的 tile，不负责资源加载；
3. 因而“视觉上是否真是老 demo 的同一张图”目前还没有页面级证据。

证据见：

1. `tinyui/docs/demo_guide.md:241-243`

### C. `contract 已抬高，但还不是 visible parity`

当前 `check_tinyui_demo_boundary.py` 对 `legacy_widget_parity` 已锁到：

1. 多数 legacy 控件 create marker
2. `list item widget`
3. `child window`
4. 第一批图片 source 绑定
5. 第二批图片型控件 source 绑定

这能证明页面结构和 source 调用范围已经明显扩大，但仍不能证明：

1. 老页面控件集合齐；
2. 已与老 demo 原图资源绑定一致；
3. visible/theme/spacing 已截图对齐；
4. backend 不再存在剩余差异。

证据见：

1. `tests/tinyui/contract/check_tinyui_demo_boundary.py`

## 结论分层

### 可以说的

1. `layout_parity` 和 `grid_parity` 已建立结构对照基线。
2. `legacy_widget_parity` 已建立第一版大页面多控件 baseline。
3. `legacy_widget_parity` 已覆盖老页面中的大部分核心控件样本。
4. `legacy_widget_parity` 已接通两批图片 source 级 public API。

### 不能说的

1. 不能说 `legacy_widget_parity` 已与老 `uiWidgetLegacy` 控件全一致。
2. 不能说图片资源已经与老 demo 绑定一致。
3. 不能说 visible 差异现在只剩 backend bug；目前很大一部分差异仍可能只是 parity 页面没接全。

## 建议的下一步

建议按以下顺序收口，不要并行乱补：

1. 把当前通用 source 进一步替换成老 demo 对应图片资源，而不是共享占位 tile。
2. 做 TINYUI / 老 SDL 双开截图与 visible diff，确认 spacing、theme、控件内容差异。
3. 若 visible 差异仍大，再区分是 demo 页面没复刻全，还是 backend/layout 真缺口。

这三步完成前，不要再宣称 `legacy_widget_parity` 已完成老页面 parity。
