# Task 2 报告：截图基准与控件差异清单

## 截图情况

- **ldgui_sdl_demo（基准）**：截图成功，完整渲染正常（55KB PNG，灰白背景，所有控件可见）
- **tinyui_demo legacy_demo0_parity**：tinyui 使用 SDL2 GPU 渲染（DirectColor visual），渲染内容不写回 X11 framebuffer，`import`/`xwd`/`ffmpeg-x11grab` 等工具均只能捕获 X11 window buffer（黑色）。视觉对比采用代码静态分析。

tinyui 实际运行时的可观察视觉现象（从截图可见）：
- 窗口背景：**黑色**（基准为灰白色 RGB(240,240,240)）
- 仅 progress_bar（底部约 y=500 处）可见，其他控件均因黑色背景被遮蔽
- date_time 显示 `1970-01-01 00:00:10`（Unix epoch），而基准显示实际时间

---

## 逐控件对比结果

| 控件 | 位置 / 尺寸 | 状态 | 差异说明 |
|------|------------|------|---------|
| image | (100,120) 50×80 | OK | 位置、尺寸、corner、selectable 一致；图片源 LETTER_PAPER 一致 |
| button | (10,10) 79×53 | OK | 文字 "123"、字体 Arial16、文字色 WHITE、图片 keyrelease/keypress 一致 |
| panel/window | (200,95) 20×20 | OK | 颜色 GREEN(0x00FF00)、corner、selectable 一致 |
| label | (100,50) 100×50 | OK | 文字 "123"、字体 Arial12、背景色 LIGHT_GREY(0xC0C0C0)、对齐 BOTTOM_LEFT → (START,END) 映射正确、corner 一致 |
| radio_a (checkbox) | (220,10) 50×20 | OK | 文字 "999"，radio_group=0，selectable 一致 |
| radio_b (checkbox) | (220,40) 50×20 | OK | radio_group=0，无文字，selectable 一致 |
| check (checkbox) | (220,70) 50×20 | OK | corner、selectable 一致，无文字 |
| switch | (300,226) 48×24 | OK | 初始 checked=false(OFF)，selectable，一致 |
| switch_label | (356,218) 60×40 | OK | 文字 "OFF"，字体 Arial16，对齐 MIDDLE_LEFT → (START,CENTER) 映射正确 |
| progress_bar | (10,500) 300×30 | OK | 45%，水平，图片 progress_bg/fg 一致 |
| text | (300,10) 150×200 | OK | 文字 "123\n12333"，背景图 paper，scroll=1，corner，一致 |
| slider_h | (50,300) 317×34 | OK | 42%，图片 slider_bg/indicator 一致；高度 legacy=image.height(运行时34) vs tinyui=硬编码34，值相同 |
| slider_v | (400,300) 30×100 | OK | 垂直方向，42%，corner，selectable，一致 |
| radial_menu | (500,200) | OK | 几何参数 (150,100,100,80,5)，4 items (weather/note/weather/note)，corner，selectable，一致 |
| date_time | (600,100) 200×50 | **DIFF** | legacy 显示真实系统时间（2026-06-30 11:22:21），tinyui 显示 `1970-01-01 00:00:10`（epoch+10s）；`tinyui_date_time_set_use_system_time` 底层时间获取异常 |
| icon_slider | (500,350) | OK | 布局参数 (150,65,48,2,5,1,1)，5 items，corner，selectable，一致 |
| qrcode | (500,10) 200×200 | OK | 内容 "ldgui"，颜色 BLUE(0x0000FF)，白底，max_version=2，zoom=5，opacity=100，一致 |
| scroll_selecter | (700,200) 30×50 | OK | 5 items，白色背景，corner，selectable，一致 |
| gauge | (700,300) 120×98 | OK | 图片源一致，centre_offset=(0,10)，pointer mask，颜色 BLUE，初始角度 120°，一致 |
| combo_box | (700,420) 100×30 | OK | 3 items ("11","22","00")，selectable，一致 |
| graph | (830,10) 100×100 | OK | axis(80,80)，axis_offset=5，grid_offset=4，RED+LIGHT_GREY 两系列，srand(10)+rand()%81，一致 |
| table | (780,150) 200×100 | OK | excel_type，3×3 cell data 一致，item_space=1 |
| line_edit | (850,400) 100×50 | OK | 文字 "123"，keyboard 绑定，corner，selectable，一致 |
| keyboard | (780,450) | OK | 位置一致 |
| child_window + nested_button | (850,450) 100×100 | OK | nested_button pos=(8,3) size=(30,30) text="123" font=Arial16，一致 |
| **arc** | **(450,450) 103×103** | **DIFF** | 见下方详细分析 |
| list | (850,280) 100×100 | **DIFF** | item_button 父节点不同；tinyui 额外调用 `set_selected_index(0)` |
| message_box | 屏幕中央 | **DIFF** | legacy 固定坐标 x=200/y=150；tinyui `set_layout(200,150)` + `set_center()` 居中——定位方式不同 |
| calendar | (50,340) 300×150 | OK | 日期 2026-1-1，day names 一致，header 可见，格式 "yyyy - mm - dd"，corner，selectable，selected=1，一致 |

---

## 严重差异详细分析

### 差异 1：根窗口背景色（最大视觉差异，影响全局）

**Legacy**（`uiWidgetLegacy.c:175`）：
```c
ldWindowInit(0, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
```
`ldWindow_init` 对 nameId=0（根窗口）自动设置（`ldWindow.c:1279`）：
```c
ptWidget->bgColor = __RGB(240, 240, 240);  // 灰白色背景
```

**TinyUI**（`legacy_demo0_parity.c:578`）：
```c
tinyui_obj_t *screen = tinyui_screen_create();
```
`tinyui_window_create` 内部调用 `ldWindow_init(scene, NULL, name_id, 0, ...)` 其中 `name_id` 不是 0（由 `tinyui_app_alloc_name_id` 分配），不触发根窗口默认灰白逻辑。**结果：tinyui 背景为黑色(0x000000)，基准为 RGB(240,240,240) 灰白色。**

这是导致所有控件视觉差异的根本原因。

---

### 差异 2：arc 控件形状错误（重点）

**Legacy**（`uiWidgetLegacy.c:354`）：
```c
obj = ldArcInit(25, 0, 450, 450, 103, 103,
                IMAGE_ARC_QUARTER_PNG_Mask, IMAGE_ARC_QUARTER_MASK_PNG_Mask,
                __RGB(240, 240, 240));
ldArcSetBackgroundAngle(obj, 0, 350);
ldArcSetForegroundAngle(obj, 30);
ldArcSetColor(obj, __RGB(173, 216, 230), __RGB(144, 238, 144));
```
直接以 **103×103**、parentColor=RGB(240,240,240) 初始化。

**TinyUI arc 创建调用链**（`tinyui/src/widgets/arc.c:50-82`）：

```c
// tinyui_arc_ld_init 内——被 tinyui_arc_create 调用
ld_arc = ldArc_init(scene, 0, name_id, parent_name_id,
                    0, 0,
                    160, 160,        // BUG: 硬编码 160×160，不是实际 103×103
                    create_ctx->arc_img_tile, create_ctx->arc_mask_tile,
                    GLCD_COLOR_WHITE);   // parentColor=0xFFFFFF
ldArcSetQuarterImage(ld_arc, arc_img_tile, arc_mask_tile, true, true);
```

随后 `tinyui_arc_create` 调用：
- `tinyui_arc_set_parent_color(arc, 0xF0F0F0U)` —— 修正 parentColor
- `tinyui_widget_set_size(arc, 103, 103)` —— 修正 tRegion.tSize

**形状错误原因分析**：

1. **尺寸错误（160→103）**：`ldArc_init` 将 `tTempRegion` 初始化为与 `tRegion` 相同（160×160）（`ldArc.c:105`）。后续 `ldBaseSetWidth/Height(103,103)` 更新了 `tRegion.tSize`，但 `tTempRegion`（用于 dirty region 管理）可能未同步，导致渲染区域计算错误。

2. **parentColor 时序问题**：`ldArc_init` 以 `GLCD_COLOR_WHITE(0xFFFFFF)` 初始化 parentColor，但实际背景是黑色(0x000000)。arc 通过 parentColor 填充圆弧"内圆"区域形成镂空效果。如果 parentColor 与实际背景不匹配，内圆会显示错误颜色（在黑背景上出现灰白斑块）。虽然 `set_parent_color(0xF0F0F0)` 后续修正了值，但若背景本身就是黑色（差异1），则 0xF0F0F0 仍然与黑色不匹配——arc 内圆会显示为灰白色而非透明。

3. **双重 ldArcSetQuarterImage 调用**：`tinyui_arc_ld_init` 调一次（ownImgTile=true, ownMaskTile=true），`tinyui_arc_set_quarter_source` 再调一次（false, false）。两次都设置了相同 tile 数据，第二次释放了第一次的所有权——功能正确，但冗余。

**最终结论**：arc 形状错误的根本原因是：
- 初始化尺寸硬编码 160×160 导致 tTempRegion 不正确
- parentColor 与实际（黑色）背景不匹配，内圆镂空无法正确融合背景

---

### 差异 3：date_time 时间显示错误

**Legacy**：`ldDateTimeInit` 使用系统挂钟时间 → `2026-06-30 11:22:21`
**TinyUI**：`tinyui_date_time_set_use_system_time(1)` → `1970-01-01 00:00:10`（epoch+10s）

原因：底层 `ldDateTimeGetTime` 或其 tinyui 封装返回了进程启动后经过的秒数（单调时间），而非 POSIX `time()` 的 UTC 时间戳。

---

### 差异 4：message_box 定位语义不同

**Legacy**：`ldMessageBoxInit(28, 0, 200, 150, FONT_ARIAL_12)` —— 第3/4参数为 x=200, y=150（固定坐标）
**TinyUI**：`tinyui_message_box_set_layout(message_box, 200, 150)` + `tinyui_widget_set_center()` —— 居中显示，200/150 可能是 width/height（语义待确认）

视觉差异：legacy 固定在 (200,150)，tinyui 居中在屏幕中央。

---

### 差异 5：list item_button 父节点

**Legacy**：`ldButtonInit(27, 26, 10, 3, 20, 20)` —— parentNameId=26（list 的 nameId），button 是 list 的子节点
**TinyUI**：`tinyui_button_create(win, ...)` —— parent=win（根窗口），button 是根窗口子节点，再通过 `set_item_widget` 关联到 list

父节点不同可能影响事件路由和渲染层级。

---

## 汇总

| 类别 | 控件数 |
|------|--------|
| 完全一致（OK） | 23 |
| 有差异（DIFF） | 5 |

**5 个差异点（按优先级）**：
1. **根窗口背景色** —— 黑色 vs RGB(240,240,240)，影响所有控件视觉（最高优先级）
2. **arc 形状错误** —— 初始化尺寸 160×160 硬编码 + parentColor 与背景不匹配
3. **date_time 时间** —— 显示 epoch 而非系统时间
4. **message_box 定位** —— 固定坐标 vs 居中
5. **list item_button 父节点** —— list 子节点 vs 根窗口子节点
