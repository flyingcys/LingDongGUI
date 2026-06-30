# 设计文档：TinyUI legacy_demo0_parity 与 LingDongGUI 控件完全对齐

**日期**：2026-06-30  
**分支**：dev-nanoui  
**目标**：使 `./build/examples/sdl/tinyui_demo legacy_demo0_parity` 与 `./build-legacy0/ldgui_sdl_demo` 在所有控件的视觉外观和行为上完全一致。

---

## 背景

用户运行两个 demo 进行对比，发现多处差异：arc 控件形状错误最为严重，其他控件也存在若干视觉差异。需要系统性梳理所有 25+ 个控件，逐一对齐。

---

## 对比范围

两个 demo 使用相同的显示尺寸（1024×600），相同的字体（`tinyui_resolve_ld_font` 已正确映射到 FONT_ARIAL_12 / FONT_ARIAL_16_A8）。

需对比的控件完整列表：

| 控件 | 关键对比维度 |
|------|------------|
| image | 图片内容、位置、尺寸、opacity |
| button | 图片/文字/字体/颜色、点击交互 |
| panel/window | 颜色、圆角 |
| label | 文字、字体、背景色、对齐方式 |
| checkbox radio_a / radio_b | 分组、文字 |
| checkbox check | 圆角 |
| switch | 初始状态（OFF）、toggle 动画 |
| switch_label | 文字、字体、对齐方式 |
| progress_bar | 进度值（45%）、图片 |
| text | 文字内容、背景图、滚动 |
| slider H | 百分比（42%）、图片、高度 |
| slider V | 百分比（42%）、方向 |
| radial_menu | 几何参数、图标 |
| date_time | 系统时间显示、字体 |
| icon_slider | 布局参数、图标 |
| qrcode | 内容、颜色、缩放、opacity |
| scroll_selecter | 条目内容、尺寸 |
| gauge | 背景/指针图片、初始角度、旋转动画 |
| combo_box | 条目 |
| graph | 数据系列、坐标轴 |
| table | 单元格文字 |
| line_edit | 文字内容 |
| keyboard | 布局 |
| **arc** | **形状、颜色、角度、旋转动画** |
| list | 条目内容、对齐、嵌套 button |
| message_box | 标题、消息、按钮 |
| calendar | 日期、header 格式 |

---

## Arc 专项诊断

### 现象
用户报告 arc 控件「形状错误」（形状不正确，非颜色/角度参数问题）。

### 根因分析
代码层面参数完全匹配：
- background angle: 0–350°
- foreground angle: 0–30°
- 尺寸：103×103
- parent_color：0xF0F0F0
- colors：0xADD8E6 / 0x90EE90

但 TinyUI arc 创建流程存在时序问题：
1. `tinyui_arc_ld_init` 以 **160×160** 调用 `ldArc_init`（硬编码默认尺寸）
2. `tinyui_arc_create` 在 160×160 状态下初始化角度/颜色
3. 用户代码后续调用 `tinyui_widget_set_size(103, 103)` 修正尺寸

`ldArc_show` 中的渲染中心计算：
```
quarterMaskCenter = (imgWidth-2, imgHeight-2) = (51, 51)   // 来自 53×53 图片
bgCentre          = (canvasWidth/2, canvasHeight/2)         // 来自 widget canvas
```

若 canvas 在第一次渲染时已正确更新为 103×103，则 bgCentre=(51,51)，两者吻合。但如果 ARM-2D OP 对象在 160×160 状态被初始化并缓存，则 bgCentre=(80,80)，导致形状错误。

### 诊断步骤
1. 截图对比 arc 区域：确认是圆心偏移、象限翻转、还是半径错误
2. 在 `ldArc_show` 打印 `tTarget_canvas.tSize` 验证实际 canvas 尺寸
3. 根据截图结果选择修复位置

### 候选修复
- **方案 B1**：修改 `tinyui_arc_ld_init`，使用合理默认尺寸（如 103×103）而非 160×160
- **方案 B2**：在 `tinyui_arc_create` 中，将初始角度/颜色设置推迟到 `tinyui_widget_set_size` 之后（需 API 改动）
- **方案 C**：若是 ldgui 底层 OP 缓存问题，在 `ldArc_show` 修复重置逻辑

---

## 修复分类

### 类别 A：demo 代码层（`tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`）
- 参数传递与 LingDongGUI 不一致时直接修改
- 风险低，不影响其他 demo

### 类别 B：TinyUI widget 实现（`tinyui/src/widgets/`）
- Arc 默认尺寸问题（`arc.c`）
- 其他 widget 初始化时序或默认值问题
- 需要 GitNexus impact 分析，改动影响所有使用该 widget 的 demo

### 类别 C：ldgui 底层（`src/gui/`）
- 仅在确认底层渲染 bug 时动
- 风险最高，需全量 regression 验证

---

## 执行流程

```
1. 编译 ldgui_sdl_demo
   cmake -S examples/sdl -B build-legacy0 -DUSE_DEMO=0
   cmake --build build-legacy0 -j8

2. 截图全页面（两个 demo 同时运行）

3. Arc 专项
   - 截图对比 arc 区域
   - 定位根因（B1 / B2 / C）
   - fix + 重新编译验证

4. 逐控件对比（可并行 subagent，写面不重叠）
   - 发现差异 → 判断类别 → fix
   - 每轮 fix 后重新编译截图验证

5. 最终全页面截图对比，确认完全一致

6. detect_changes() 验证影响范围，提交
```

---

## 约束

- worktree 合并时，主仓库文档变更可忽略
- 并行 subagent 修复时写面不得重叠
- 类别 B/C 修复前必须运行 `impact()` 分析
- 不得引入新的公共 API（除非 CLAUDE.md 中用户可见能力缺口）

---

## 成功标准

两个 demo 并排运行，所有 27 个控件视觉外观（位置、尺寸、颜色、字体、形状）和动画行为（gauge/arc 旋转，switch toggle）完全一致，肉眼无法区分差异。
