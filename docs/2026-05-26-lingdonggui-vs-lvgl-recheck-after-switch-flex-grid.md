# 2026-05-26 LingDongGUI vs LVGL 复盘：补齐 switch / flex / grid 之后差距还大吗

## 1. 这份文档回答什么

本次只回答两个问题：

1. 补齐 `switch / flex / grid` 之后，LingDongGUI 和 LVGL 在这三条能力线上是不是已经“差异很小”。
2. 除了这三个点之外，LingDongGUI 现有其它基础控件，是否已经足以支撑一个**轻量型 GUI 系统**。

本文不是重新做一次全量功能清单，也不是把 LingDongGUI 直接写成“已经追平 LVGL”。重点是把比较口径拆成两层：

- **控件主路径口径**：常见界面是否已经能做，行为是否已经接近。
- **平台系统口径**：样式、事件、数据绑定、国际化、主题、统一对象模型这些系统能力是否也接近。

只有把这两层拆开，结论才不会失真。

## 2. 参考依据

本次复盘主要基于以下现状文件：

- 旧技术总对比：`LingDongGUI_vs_LVGL_技术对比.md`
- 旧用户视角分析：`lingdonggui-vs-lvgl-user-analysis.md`
- 三条能力线当前状态：`examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md`
- 三条能力线审计板：`examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md`
- switch 边角审计：`examples/sdl/docs/2026-05-25-switch-lvgl-edge-audit.md`
- 当前 layout / base / switch 代码入口：`src/gui/ldWindow.h`、`src/gui/ldBase.h`、`src/gui/ldSwitch.h`、`src/gui/ldSwitch.c`
- 当前消息与 GUI 入口：`src/misc/ldMsg.h`、`src/gui/ldGui.h`
- 当前控件头文件集合：`src/gui/ld*.h`

## 3. 先给结论

### 3.1 一句话结论

**如果只看 `switch / flex / grid` 三条“常用主路径能力”，现在 LingDongGUI 和 LVGL 的差距已经明显缩小，其中 `switch` 已经接近“小差距”，`flex / grid` 在轻量型 GUI 的常用范围内也已经进入“小到中等差距”。**

但如果比较口径升到 **LVGL 作为统一 UI 平台** 的层面，差距**并没有缩到很小**，只是从“控件缺失”转移成了“系统能力不统一”。

### 3.2 更准确的判断

| 维度 | 当前判断 |
| --- | --- |
| `switch` 单控件常用能力 | 差距很小 |
| `flex` 常见一维布局能力 | 差距小到中等 |
| `grid` 常见二维布局能力 | 差距小到中等 |
| 基础控件覆盖面 | 已足够支撑轻量型 GUI 系统 |
| 统一 style / state / part / theme / binding / i18n 平台能力 | 差距仍然较大 |
| 整体定位 | 更像“轻量型 GUI 工具箱”，还不是“LVGL 那样的统一 UI 平台” |

## 4. `switch / flex / grid` 现在到底差多少

## 4.1 switch：常用能力已经接近“小差距”

从 `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md` 和 `examples/sdl/docs/2026-05-25-switch-lvgl-edge-audit.md` 看，当前 `switch` 已经完成了这几个关键点：

- 原生控件主线成立，不再是用其它控件替代。
- 支持 `AUTO / 横向 / 纵向`。
- 支持 `disabled`、首帧稳态、`SIGNAL_VALUE_CHANGED`。
- 内部已拆出 `track / indicator / knob` 语义。
- 已补 `ldSwitchNavigate()` / `ldSwitchCanNavigate()`，接入通用焦点导航。
- 已补 `pressed`、中间动画帧、checked ring、knob overhang 等关键视觉证据。

所以如果问题是：

- “能不能像 LVGL 那样把它当正式 switch 用？”
- “轻量项目里开关交互会不会一眼看起来很落后？”

当前答案基本是：**可以用，而且差距已经不大。**

但这不等于 `switch` 在所有层面都和 LVGL 很接近。剩余差距主要已经上移到系统层：

- 还没有挂到 LVGL 那种统一 `part / state / style` 体系上。
- 还没有进入统一 theme 机制。
- 更高置信度的整图 diff / 主题组合矩阵还没做。

所以对 `switch` 最准确的说法是：

**单控件主路径差距已经很小；平台级差距还在。**

## 4.2 flex：轻量 GUI 常用主路径已经够强，但不能说 full parity

从 `src/gui/ldWindow.h`、`src/gui/ldBase.h` 和 `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md` 看，当前 flex 已具备：

- 8 种 flow：`row / column / wrap / reverse / wrap_reverse`
- `main / cross / track` 对齐
- `item gap / track gap`
- child 级 `grow / new track / ignore layout`
- 公开的 absolute `min/max` clamp API

这意味着对于轻量型 GUI 最常见的一维场景：

- 设置页纵向排布
- 工具栏横向排布
- tag / chip 自动换行
- 简单 dashboard 卡片流式排列

LingDongGUI 已经不是“只能勉强排一下”的状态，而是**有正式 flex 主干能力**。

但如果比较对象仍然是 LVGL 的完整 flex 体系，差距还没有小到可以忽略。审计文档里明确保留的未对齐项包括：

- `RTL`
- margin 语义
- percent translate / percent size
- content-size 容器联动
- 更完整的尺寸联动闭环

所以对 flex 的判断不该写成“差异很小了”，更准确是：

**对轻量 GUI 常见一维布局，差距已经缩到小到中等；对 LVGL 完整 flex 体系，仍然有一层明确未补语义。**

## 4.3 grid：主实现已经成立，但高阶语义还没追完

从 `src/gui/ldWindow.h`、`src/gui/ldBase.h` 和 `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md` 看，当前 grid 已经不再是旧的“按列数顺排容器”，而是有了明确的 descriptor-grid 主线：

- `fixed / CONTENT / FR`
- explicit cell
- `col_span / row_span`
- cell align
- container align
- descriptor-grid 下 auto placement fallback
- `ignoreLayout` overlay
- legacy `gridColumns` fallback 兼容

这对轻量型 GUI 非常重要，因为它已经足够表达：

- dashboard 卡片区
- 设置页混合布局
- 说明区跨列
- 某些 overlay 不参与排版

也就是说，**grid 现在已经能承担“正式二维布局骨架”的角色。**

但与 LVGL 相比，当前仍明确延期：

- `subgrid`
- `RTL`

因此 grid 的结论和 flex 类似：

**常用主路径已经很接近“够用且像样”，但还不是 LVGL 全量 grid 语义。**

## 4.4 三条线合起来怎么看

把这三条能力线放在一起看，变化最大的不是“100% 追平 LVGL”，而是：

**LingDongGUI 已经跨过了“布局和开关还是明显短板”的阶段。**

现在更接近真实情况的表述是：

- `switch`：主路径差距很小
- `flex`：主路径已成型，边角语义未补完
- `grid`：主路径已成型，高阶语义未补完

所以如果用户只是想做一个轻量 MCU UI，今天真正挡路的点，已经**不再主要是这三个控件本身**。

## 5. 其它基础控件，是否已满足轻量型 GUI 系统

## 5.1 先看覆盖面：答案是“基本满足”

当前 `src/gui/ld*.h` 可见的正式控件头文件共 27 个，覆盖面已经不算薄：

- 容器与页面：`ldWindow`
- 基础展示：`ldImage`、`ldLabel`、`ldText`
- 基础交互：`ldButton`、`ldCheckBox`、`ldSlider`、`ldSwitch`
- 输入：`ldLineEdit`、`ldKeyboard`
- 数据展示：`ldProgressBar`、`ldProgressWheel`、`ldGraph`、`ldTable`、`ldList`
- 设备型控件：`ldGauge`、`ldClock`、`ldDateTime`、`ldCalendar`、`ldQRCode`
- 定制交互：`ldComboBox`、`ldIconSlider`、`ldRadialMenu`、`ldScrollSelecter`
- 对话与动效：`ldMessageBox`、`ldAnimation`、`ldArc`

如果把目标限定为“轻量型 GUI 系统”，它通常至少需要下面这些基础块：

1. 页面/容器
2. 图片/文本/按钮
3. 复选/开关/滑条
4. 进度与状态显示
5. 列表/表格
6. 文本输入
7. 简单布局

现在 LingDongGUI 这几块都已经具备，而且不只是“有名字”，而是已经在 `README.md`、`docs/tutorial/04 api.md`、SDL demo 和若干控件实现中形成了可使用的主线。

因此从**控件覆盖面**看，我认为答案是：

**是，已经满足轻量型 GUI 系统。**

## 5.2 为什么我说“满足轻量型 GUI 系统”，但不说“已经接近 LVGL 平台”

因为“控件够不够”是一层，“平台统一不统一”是另一层。

当前 LingDongGUI 已经有轻量 GUI 所需的大多数基础积木，但它的组织方式仍然更像“控件工具箱”：

- 许多能力以**控件专用 setter** 形式分散在各头文件中。
- 有些 API 仍依赖 `ptScene` 宏包装，而不是纯对象方法。
- 事件模型以 `SIGNAL_* + connect(...)` 为主，够用，但没有 LVGL 那种更完整的统一事件语义。
- 一些布局或视觉语义仍然是局部实现，不是全局统一 contract。
- 代码和文档都继续暴露了大量 ARM-2D 类型与思维方式，比如 `arm_2d_tile_t`、`arm_2d_font_t`、`arm_2d_align_t`。

换句话说：

- **作为轻量 GUI 系统**：它已经够用。
- **作为统一 UI 平台**：它还没有封装到 LVGL 那么完整。

## 5.3 现在真正的短板，已经从“控件数量”转移到了“系统一致性”

结合 `lingdonggui-vs-lvgl-user-analysis.md` 的旧结论，以及这次三条能力线的更新，我认为当前差距最大的地方已经不再是：

- “没有 switch”
- “没有 flex”
- “没有 grid”

而是这些更系统的层面：

### 1. 缺统一 style / state / part 体系

LVGL 的优势不只是控件多，而是：

- 同一套 style 规则可以落到不同控件
- 同一套 state 语义可以解释 pressed / checked / disabled
- 同一套 part 语义可以解释 main / indicator / knob / scrollbar 等局部结构

LingDongGUI 现在虽然某些控件内部已经开始出现类似结构，比如 `switch` 的 `track / indicator / knob`，但这仍是**控件内局部语义**，不是平台级统一语义。

### 2. 缺统一 theme 能力

当前更像每个控件自己配颜色、图片、边角、字体。  
这对轻量项目可接受，但不利于：

- 全局换肤
- 主题复用
- 同一设计语言批量落到多个控件

### 3. 缺统一 binding / translation / observer 层

对于轻量项目，这些能力不是必须第一天就有。  
但一旦用户想把它当“长期 UI 平台”使用，这些能力会明显影响开发体验：

- 数据状态变化能否直接绑定到控件
- 多语言文案能否统一收口
- 页面逻辑能否从手写 connect / emit 继续上抽象

### 4. 抽象边界仍然偏 ARM-2D 导向

`README.md` 明确把“支持 ARM-2D 原生 API”“LDGUI 和 ARM-2D 混合编程”写成优势。  
这对嵌入式工程师是优点，对想把它当通用 UI 平台的用户则意味着：

**抽象边界还没有完全封住。**

所以，今天 LingDongGUI 相比 LVGL 的主要差距，已经从“控件有没有”转移成“平台边界、系统一致性、开发心智是否统一”。

## 6. 对“差异很小了吗”的最终判断

如果用户问的是一句很口语的话：

> “通过上一轮补齐 switch flex grid，现在你重新对比一下 LingDongGUI 和 LVGL，这 3 个控件是不是差异很小了？”

我会给出这样的回答：

### 可以说“差距已经显著缩小”的部分

- `switch`：可以，差异已经很小。
- `flex`：对轻量 GUI 常见主路径来说，可以说已经不大。
- `grid`：对轻量 GUI 常见主路径来说，也可以说已经不大。

### 不能直接说“已经很小”的部分

- 如果把比较口径拉到 LVGL 的完整布局和统一平台语义，`flex / grid` 仍有明确缺口。
- 如果把比较对象从单控件提升到“整套 UI 平台体验”，差距仍然不小。

所以一句最稳妥、也最接近现在代码真相的话是：

**LingDongGUI 已经把 `switch / flex / grid` 这三条最明显的控件差距大幅缩小；但它和 LVGL 的主要差距，已经转移到统一 style / theme / binding / i18n / 抽象边界这些平台层。**

## 7. 对“是否满足轻量型 GUI 系统”的最终判断

如果问题换成：

> “其它基础控件是否满足轻量型 GUI 系统？”

我的结论是：

**满足。**

但这句话后面必须跟两个限定：

1. 它满足的是“轻量型 GUI 系统”的基础能力要求，不是“完整通用 UI 平台”的全部期待。
2. 它现在更适合“会接受控件级 setter、接受 ARM-2D 背景、接受局部式抽象”的嵌入式团队，而不是追求 LVGL 那种统一开发心智的用户。

## 8. 如果下一阶段还要继续追 LVGL，优先级应该怎么排

在 `switch / flex / grid` 主路径已经补到今天这个程度后，我不建议再把主要精力继续投入“基础控件存在性”。

更值得做的顺序是：

### P0：补统一平台层，而不是继续堆单控件

优先考虑最小可用的统一层：

- 基础 style 词汇
- 基础 state 词汇
- 少量通用 part 词汇
- 最小 theme 入口

因为这是当前和 LVGL 感知差距最大的地方。

### P1：继续补 flex / grid 的高阶语义

重点还是审计板里已经列出的内容：

- flex：`RTL`、margin、percent/content-size
- grid：`subgrid`、`RTL`

### P2：把“轻量 GUI 工具箱”升级成“更统一的轻量平台”

也就是把今天已经存在的 27 个控件，慢慢从“各自能用”整理成“用户能用一套统一心智去用”。

## 9. 最后一句结论

**今天的 LingDongGUI，已经足以作为一个轻量型 GUI 系统来使用；并且 `switch / flex / grid` 这三条过去最显眼的短板已经明显收窄。它和 LVGL 的主差距，已经不再主要是基础控件缺失，而是平台系统化程度。**
