# TinyUI 对齐 LingDongGUI demo0 单页全控件设计说明

**日期**：2026-06-27  
**状态**：已确认，可进入实现  
**范围类型**：现有 demo 对齐与能力核对

---

## 1. 目标

新增一个纯 `tinyui_*` public API 编写的单页 demo，用来对齐 `LingDongGUI` 在 `USE_DEMO=0` 时的 legacy widget 单页全控件页面。

这个 demo 的目标不是“展示一组类似控件”，而是作为 `TINYUI -> LingDongGUI` 真实 backend 映射的对比基线，验证：

- 控件能力是否等价
- 基础交互是否等价
- 位置与尺寸是否等价
- 输出效果是否可直接并排对比

同时需要把 `LingDongGUI` 原始 legacy demo 的 SDL 可执行目标编出来，方便和新的 TinyUI demo 并排运行。

---

## 2. 用户确认后的范围

### 2.1 本轮要做

- 新增一个 `tinyui` 单页对比 demo
- 该 demo 只能使用 `tinyui_*` public API
- 以 `examples/common/demo/widget/uiWidgetLegacy.c` 作为唯一版式和交互基准
- 新 demo 中控件的坐标与尺寸必须与 legacy demo0 对齐
- 新 demo 中控件的数据、默认值、文案、关联关系尽量与 legacy demo0 对齐
- 新 demo 中关键交互要与 legacy demo0 对齐：
  - button 点击后修改 image 透明度
  - switch 切换后联动旁边状态文字
  - arc / gauge 在运行时持续变化
  - keyboard / table / line_edit 的绑定关系保持一致
- SDL 窗口允许变大，但原始控件布局坐标不缩放、不重排
- 编译出 `LingDongGUI` legacy demo 的 SDL 可执行目标，用于对比

### 2.2 本轮不做

- 不修改 `LingDongGUI` legacy demo 的既有行为和布局
- 不把 demo 适配逻辑塞进 fake renderer 或 SDL 宿主专用绘制路径
- 不通过改 demo 代码硬塞 backend 暂不支持的假能力
- 不把 legacy 的分页版 widget demo 一起迁移
- 不做自动像素回归系统的完整建设；本轮只做必要的构建、运行和关键检查

---

## 3. 真值来源

本次 parity 的真值来源固定为以下文件：

- `examples/common/demo/widget/uiWidgetLegacy.c`
- `examples/sdl/user/ldConfig.h`
- `examples/common/demo/widget/fonts/*`
- `examples/common/demo/widget/images/*`

其中：

- `uiWidgetLegacy.c` 是单页模式 0 的控件、坐标、尺寸、默认值和交互真值
- `ldConfig.h` 提供 legacy SDL demo 的屏幕基准配置
- font / image 资源决定 legacy demo 的视觉素材来源

现有 `tinyui/demo/legacy_widget_parity/legacy_widget_parity.c` 不能继续作为本次 parity 的真值来源，它只能作为现状参考。

---

## 4. 方案选择

### 4.1 方案 A：直接改现有 `tinyui/demo/legacy_widget_parity`

优点：

- 复用已有聚合 demo
- 文件数少

缺点：

- 现有文件语义已经偏“能力集合页”，不是 demo0 真值复刻
- 改动后会混淆“历史抽样页”和“严格 parity 页”
- 后续做回归时不容易区分哪个 demo 才是基准

### 4.2 方案 B：新增一个专门的 demo0 parity demo

优点：

- 目标清晰，语义单一
- 可以明确对齐 legacy demo0 的位置、数据、交互
- 更适合后续做截图对比和能力回归

缺点：

- 需要新增 demo 注册和构建入口

### 4.3 方案 C：修改旧的 LingDongGUI demo，向 TinyUI 当前能力靠拢

不采用。

原因：

- 这会污染真值来源
- 无法回答 “TinyUI 是否已经达到 LingDongGUI 同等能力”

### 4.4 推荐方案

采用方案 B。

也就是：

- 保留现有 `legacy_widget_parity` 作为非严格抽样页
- 新增一个严格对齐 `uiWidgetLegacy.c` 的 `tinyui` demo
- 用这个新 demo 作为今后的单页全控件 parity 基线

---

## 5. 功能与布局设计

### 5.1 总体布局

新 demo 采用 legacy demo0 的绝对坐标布局，不引入新的 grid / flex 重排逻辑。

原因：

- 用户要求位置一模一样
- legacy demo0 的真值本身就是绝对坐标
- 用布局系统重新表达会引入不必要的偏移和解释空间

SDL 外层窗口允许更大，但控件坐标和尺寸仍以 legacy demo0 为基线，不做整体比例缩放。

### 5.2 控件覆盖范围

目标控件集与 `uiWidgetLegacyInit()` 对齐，包含：

- `window`
- `image`
- `button`
- `label`
- `checkbox` / `radio`
- `switch`
- `progress_bar`
- `text`
- `slider`（横向 / 纵向）
- `radial_menu`
- `date_time`
- `icon_slider`
- `qrcode`
- `scroll_selecter`
- `gauge`
- `combo_box`
- `graph`
- `table`
- `line_edit`
- `keyboard`
- `arc`
- `list`
- `message_box`
- `calendar`
- 子 `window`
- 嵌套 `button`

### 5.3 数据与默认值

新 demo 的文案、选项、初始值、二维码内容、日历时间、表格内容、message box 内容，默认以 `uiWidgetLegacy.c` 为准。

如果现有 TinyUI demo 中已有不同数据，不沿用旧 TinyUI 数据，必须回到 legacy demo0 真值。

### 5.4 关键交互

必须对齐的交互有四类：

1. `button` 触发图片透明度切换  
2. `switch` 触发状态文字 `ON/OFF` 更新  
3. `arc` / `gauge` 在运行时持续变化  
4. `table` / `line_edit` 与 `keyboard` 的关联关系保持对齐

本轮不要求把 legacy 内部所有键盘焦点语义都一比一重建，但如果 `tinyui` public API 已经具备同等语义，优先按真实能力对齐，而不是绕开。

---

## 6. 架构与代码落点

### 6.1 TinyUI demo 落点

新增一个独立 demo，放在 `tinyui/demo/` 体系内，并注册到 `tinyui_demo` 的运行参数表中。

建议结构：

- 新增 demo 目录与 `*.c/*.h`
- 在 `tinyui/demo/tinyui_demos.c`
- 在 `tinyui/demo/tinyui_demos.h`
- 在对应的 CMake / demo 源文件汇总中注册

### 6.2 运行时状态

由于存在图片透明度切换和 `arc/gauge` 持续变化，demo 需要自己的 runtime 状态结构，至少保存：

- 目标 `image`
- 目标 `switch_label`
- `arc`
- `gauge`
- 动画或步进状态

不采用把这些状态散落在全局静态变量中的做法；应当与 demo 自身绑定，保持单一职责。

### 6.3 能力缺口处理原则

如果在实现过程中发现某个 legacy demo0 依赖的行为无法仅靠现有 `tinyui_*` public API 实现，处理顺序必须是：

1. 先确认是否已经存在 public API 但 demo 还没正确使用
2. 如果 public API 缺口真实存在，补 `TINYUI -> LingDongGUI` 的真实 backend 映射
3. 不允许走 SDL fake path 或在 demo 里偷偷调用 `ld*` API

---

## 7. 构建与对比设计

### 7.1 TinyUI 目标

继续使用现有 `tinyui_demo` 可执行文件，通过新增 demo 名称运行。

### 7.2 LingDongGUI 目标

需要编译出 `ldgui_sdl_demo` 的 legacy demo0 版本，确保其对应：

- CMake 目标：`ldgui_sdl_demo`
- 配置：`USE_DEMO=0`
- 入口：`uiWidgetLegacyFunc`

### 7.3 对比方式

本轮至少应支持：

- 独立运行 legacy demo0
- 独立运行新的 TinyUI parity demo
- 通过截图或人工并排观察对比位置、控件种类、默认数据和关键交互

---

## 8. 测试策略

遵循最小但真实的 TDD 和验证闭环。

### 8.1 先补测试 / 检查点

优先为以下内容建立失败基线：

- 新 demo 被正确注册到 `tinyui_demos`
- 新 demo 采用 legacy demo0 的关键文案 / 坐标 / 控件集
- 如需补 public API / backend 行为，先补对应单元测试

### 8.2 运行期验证

至少完成以下验证：

- `tinyui_demo <new_demo>` 可成功构建和启动
- `ldgui_sdl_demo` 在 `USE_DEMO=0` 下可成功构建和启动
- 关键控件文本不丢失
- button / switch / arc / gauge 的关键交互按预期工作

### 8.3 非目标

本轮不要求一次性补齐完整自动截图断言矩阵，但实现过程应尽量为后续截图回归保留稳定入口。

---

## 9. 风险与影响

本轮预计会碰到两类风险：

### 9.1 Demo 级风险

- 新增 demo 注册点后，`tinyui_demo` 的参数表会变化
- 构建脚本可能需要把新增 demo 文件纳入编译

这类风险可控，范围主要在 demo 层。

### 9.2 能力缺口风险

- 某些 legacy demo0 行为可能无法仅靠现有 TinyUI public API 完成
- 一旦发现缺口，就会触及对应 widget 或 backend 映射实现

这类风险取决于具体符号的 GitNexus impact 结果。实际修改前必须逐个做 upstream impact，不提前假设低风险。

---

## 10. 完成标准

本轮完成的标准是：

- 新增的 TinyUI demo 只使用 `tinyui_*` public API
- 该 demo 的控件集合与 `uiWidgetLegacy.c` 单页模式 0 对齐
- 关键控件位置和尺寸与 legacy demo0 对齐
- 关键数据和交互与 legacy demo0 对齐
- `tinyui_demo` 和 `ldgui_sdl_demo(USE_DEMO=0)` 都能构建并运行
- 用户可以直接拿两个可执行目标做人工对比

未满足以上任一项，都不能称为“demo0 parity 完成”。
