# PicoUI A线计划索引

- `A线` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `A线` 总实施口径：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- `PicoUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `PicoUI` 当前已经具备：
  - public API 基础骨架
  - demo target 可构建
  - SDL 窗口可启动
  - runtime/capture smoke 基础证据
- 但当前主线仍存在一个明确问题：
  - `picoui/src/backend/ldgui/backend_app.c` 仍承担大量 fake renderer 职责
  - 这说明当前“能看到画面”并不等于“真实 backend 映射完成”

## A线目标

- **A线唯一目标**：把 `PicoUI` 从“临时可视 smoke”纠偏成“真实 `PicoUI -> LingDongGUI` backend 映射主线”。

## 当前游标

- 当前活跃游标：`A0`
- 当前允许进入：`A1`
- 当前禁止进入：任何继续强化 fake renderer 的工作

## 阶段导航

- `A0`：纠偏确认与停止项冻结
- `A1`：真实 backend tree
- `A2`：静态控件真实映射
- `A3`：交互控件真实映射
- `A4`：真实 flex/grid layout
- `A5`：真实 event/message pipeline
- `A6`：真实 theme/state/part/style
- `A7`：demo/UI 收口与 fake renderer 收缩

## 逐阶段入口

### A0 纠偏确认

- 真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md` 第 24 节
- 当前结论：
  - 当前思路已经确认偏向 fake renderer
  - 后续不再以“把 `backend_app.c` 修得更像控件”为主线

### A1 真实 backend tree

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage A`
- 当前目标：
  - 补真实 backend root/tree/owner/parent/child 结构
  - 去掉对 fake traversal 的依赖

### A2 静态控件真实映射

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage B`
- 当前目标：
  - `window/label/button/text/image` 映射到真实 `LingDongGUI` 控件

### A3 交互控件真实映射

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage C`
- 当前目标：
  - `checkbox/switch/slider` 映射到真实 `LingDongGUI` 控件
  - 消灭 `basic_widgets` 只有色块的现象

### A4 真实 flex/grid layout

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage D`
- 当前目标：
  - 不再靠 demo 里的硬编码尺寸凑画面
  - layout 语义必须真实落到 `LingDongGUI`

### A5 真实 event/message pipeline

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage E`
- 当前目标：
  - 从 setter-path 过渡到 native-event-path

### A6 真实 theme/state/part/style

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage F`
- 当前目标：
  - `theme/state/part` 从 contract 升级成真实样式应用

### A7 demo/UI 收口

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage G`
- 当前目标：
  - fake renderer 缩回纯 smoke
  - 文档与代码重新对齐

## 当前规则

- 顺序固定为：`A0 -> A1 -> A2 -> A3 -> A4 -> A5 -> A6 -> A7`
- `A1` 未完成前，不进入 `A2`
- `A2` 未完成前，不进入 `A3`
- `A3` 未完成前，不进入 `A4`
- `A4` 未完成前，不进入 `A5`
- `A5` 未完成前，不进入 `A6`
- `A6` 未完成前，不进入 `A7`
- 所有阶段都禁止把 fake renderer 扩成正式方案
- 所有阶段都禁止靠 demo 页面硬编码补丁掩盖 backend/layout 缺口

## 当前明确做什么

1. 做真实 backend tree
2. 做真实 widget mapping
3. 做真实 layout/event/theme 映射
4. 用 runtime/capture 只做 smoke 与回归，不拿它伪装正式 backend 闭环

## 当前明确不做什么

1. 不继续美化 `backend_app.c` 假控件画法
2. 不继续靠 `set_size()/set_pos()` 拼 demo 效果
3. 不把“窗口能弹出来”当成“PicoUI 适配完成”
4. 不把 SDL host 改造成 `PicoUI` 专属渲染系统

## 推荐阅读顺序

1. 先读 `docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md` 第 24 节
2. 再读 `docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
3. 后续每次开工前先看本文，确认当前游标在哪、下一步该做哪一段

## 退出口径

只有下面条件同时成立，`A线` 才算收口：

- `A1` 完成
- `A2` 完成
- `A3` 完成
- `A4` 完成
- `A5` 完成
- `A6` 完成
- `A7` 完成
- `backend_app.c` 不再承担正式 fake renderer 职责
- `PicoUI` demo 的可见 UI 来自真实 `LingDongGUI` backend 映射，而不是手工拼图
