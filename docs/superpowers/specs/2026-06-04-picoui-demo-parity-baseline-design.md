# TINYUI Demo Parity Baseline Design

**日期**：2026-06-04  
**范围**：`tinyui/demo` 第一批 parity baseline：`legacy-widget`、`layout`、`grid`

## 1. 背景

当前 `tinyui/demo` 以控件级 basic demo 为主，页面结构、控件组合、文案和布局关系与 `LingDongGUI` 现有 SDL demo 差异很大。  
这会混淆两类问题：

- 是 `demo` 自己长得不一样；
- 还是 `TINYUI -> LingDongGUI` 的真实能力、布局、映射没有对齐。

本轮目标不是“再设计一套更好看的 TINYUI demo”，而是建立一套 `parity baseline`：  
让 `TINYUI` demo 在页面结构、控件集合、主要文案、主要位置关系上尽量贴近老 demo。这样后续做双开截图对比时，差异更容易直接指向 `TINYUI` 实现缺口。

## 2. 真相源

第一批 demo 的真相源固定为老 SDL demo 源码，不以当前 `tinyui/demo/*` 为准：

- `legacy-widget`：`examples/common/demo/widget/uiWidgetLegacy.c`
- `layout`：`examples/common/demo/layout/uiLayout.c`
- `grid`：`examples/common/demo/layout/uiLayout.c`
- 构建/路由入口：`examples/sdl/CMakeLists.txt`
  - `USE_DEMO=0` -> `legacy-widget`
  - `USE_DEMO=4` -> `layout`
  - `USE_DEMO=5` -> `grid`

## 3. 目标

新增 3 个独立 TINYUI demo：

- `tinyui/demo/legacy_widget_parity`
- `tinyui/demo/layout_parity`
- `tinyui/demo/grid_parity`

每个 demo 都作为独立 SDL target 构建，便于与老 demo 一一对开对比。

### 3.1 必须对齐的内容

- 页面结构
- 控件种类
- 主要文案
- 主要布局关系
- 主要尺寸/位置关系
- 截图所需的最小交互状态

### 3.2 暂不要求首轮完成的内容

- 像素级视觉一致
- 完整主题/皮肤一致
- 老 demo 所有次级交互路径
- 所有动画时序细节

## 4. 非目标

- 不做“更现代/更美观”的 TINYUI demo 改版
- 不把老 demo 内部 `ld*` API 直接暴露进 `tinyui/demo`
- 不通过 demo 侧硬编码补丁掩盖 backend/layout/public API 缺口
- 不把 `startup`、`printer`、`widget`、`widget-swipe` 一起塞进第一批

## 5. 设计原则

### 5.1 Parity baseline 优先

demo 的首要职责不是展示“TINYUI 自己想怎么用”，而是给 `LingDongGUI` 老 demo 提供可比对的等价页面。

### 5.2 老 demo 是黄金样本

若老 demo 中存在某个控件、某段文案、某个主区域布局关系，TINYUI parity demo 默认也要表达出来。  
禁止为了方便实现擅自删控件、换控件、改文案、改页面结构。

### 5.3 缺能力就暴露真缺口

若 `TINYUI` public API、layout、backend mapping 不能表达老 demo 所需能力，应：

- 先记录为真实 gap；
- 必要时补 `TINYUI` public API / backend / runtime；
- 不允许在 demo 侧通过 fake 画法、固定坐标补丁、伪控件规避。

### 5.4 demo boundary 维持纯净

`tinyui/demo/*` 只能使用 `tinyui_*` public API。  
禁止引入：

- `ld*`
- `arm_2d_*`
- `SIGNAL_*`
- backend-private helper

## 6. 第一批页面定义

### 6.1 `legacy_widget_parity`

用途：建立“大页面、多控件混排”的总览基线。  
真相源：`uiWidgetLegacy.c`。

首轮至少覆盖：

- image
- button
- color window/panel
- label
- checkbox/radio
- switch + ON/OFF label
- progress bar
- text
- slider（横向/纵向若已有对应公共能力则保留）
- list / combo box / calendar / scroll selector / date-time / message box / graph 等老页面里直接出现的控件样本

要求：

- 控件集合与主要分布关系尽量贴近老页面
- `switch` 状态文案关系保留
- 不把多控件拆散成多个单控件 demo

### 6.2 `layout_parity`

用途：验证 legacy/flex/column 这些布局关系是否能用 TINYUI 真布局表达。  
真相源：`uiLayout.c` 中 layout 页。

首轮至少覆盖：

- 页面标题/提示
- legacy row/column 卡片组
- flex row 卡片组
- flex column 卡片组
- 主要卡片尺寸层次

要求：

- 重点看容器、gap、对齐、padding、流向
- 不允许靠页面硬编码修到“看着像 flex”

### 6.3 `grid_parity`

用途：验证 grid cell、span、align、overlay 等关系是否闭环。  
真相源：`uiLayout.c` 中 grid 页。

首轮至少覆盖：

- 页面标题/提示
- grid canvas
- A-G 面板
- overlay 面板
- col/row span 与对齐语义

要求：

- 重点看 grid 语义是否真实生效
- 面板结构和文本关系要能与老页面直接对照

## 7. 构建与目录

新增目录：

- `tinyui/demo/legacy_widget_parity/main.c`
- `tinyui/demo/layout_parity/main.c`
- `tinyui/demo/grid_parity/main.c`

允许新增少量共享 helper，例如：

- `tinyui/demo/common/*`

但 helper 只负责：

- 颜色/间距常量
- 标题/说明文本构造
- 卡片/面板样板拼装

不得把 helper 做成绕过 public API 的后门。

SDL 构建入口需新增 3 个 target，命名遵循现有规则：

- `tinyui_legacy_widget_parity_demo`
- `tinyui_layout_parity_demo`
- `tinyui_grid_parity_demo`

## 8. 验证口径

### 8.1 RED/GREEN 基线

先补最小失败测试，再实现代码：

- 新 target 已注册
- demo boundary 不泄漏 `ld*`
- 新 demo 可构建

### 8.2 本轮必须通过

- 目标 target 能构建成功
- `check_tinyui_demo_boundary.py` 对新 demo 仍通过
- 至少有一条针对新 target 注册面的源码级或 contract 断言

### 8.3 本轮不强行宣称

以下内容除非有单独证据，否则不能宣称已完成：

- 真实视觉完全对齐
- 所有交互完全对齐
- backend/layout 100% 无差距

本轮更适合输出的结论是：

- 已建立可截图对照的 parity baseline demo；
- 后续差异截图更能定位到 TINYUI 实现问题，而不是 demo 设计差异。

## 9. 第二批留待后续

第二批候选：

- `widget`
- `printer`
- `widget-swipe`

等第一批 baseline 能稳定给出高信号截图后，再继续扩到更重交互页面。
