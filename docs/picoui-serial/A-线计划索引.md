# PicoUI A线计划索引

- `A线` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `A线` 总实施口径：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- `PicoUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `PicoUI` 当前已经具备：
  - `A1-A7` 已按当前口径收口
  - `PicoUI -> LingDongGUI` 真实 backend 主线已经成为当前正式实现
  - `runtime/capture` 证据链已回到 smoke / 回归层级，不再冒充最终 UI 证据
  - `picoui/src/backend/ldgui/backend_app.c` 当前仅保留 `temporary smoke path` / host harness 职责，不再承担正式 fake renderer 主输出职责

## 历史背景（已过时）

- 2026-05-26 较早阶段，主线曾存在一个明确问题：
  - `picoui/src/backend/ldgui/backend_app.c` 一度承担过多 fake renderer 职责
  - 当时“能看到画面”不能等于“真实 backend 映射完成”
- 该问题已在 `A7` 收口中被纠偏；上面这段仅保留为历史背景，不代表当前现态

## A线目标

- **A线唯一目标**：把 `PicoUI` 从“临时可视 smoke”纠偏成“真实 `PicoUI -> LingDongGUI` backend 映射主线”。

## 当前游标

- 当前活跃游标：`A7`
- 当前允许进入：无
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
- 当前状态：**已完成**
- 已完成内容：
  - 补齐真实 backend `root/tree/owner/parent/child` 结构
  - `backend_app.c` 已不再依赖 runtime 私有线性 widget 链表，改为直接消费 backend tree
  - `backend_widget_tree.c` 已正式接入构建，不再靠 `.c include` 临时接线
- 当前证据：
  - `ctest --test-dir build -R test_picoui_widgets --output-on-failure` 通过
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过

### A2 静态控件真实映射

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage B`
- 当前状态：**已完成**
- 已完成内容：
  - `window/label/button/text/image` 已建立真实 `LingDongGUI` 对象映射
  - `backend_app.c` 对 static widgets 的可见输出已切到 `ldGuiFrameStart()` / `ldGuiDraw()` / `ldGuiFrameComplete()` 真实输出链
  - `settings_panel` 当前作为 mixed-tree 边界样本：`title/apply` 走 real static mapping，`wifi/brightness` 仍保留 A3 fallback
  - `image` 已具备最小真实绑定：`picoui_image_source -> ldImageSetImage()`
- 当前证据：
  - `ctest --test-dir build -R test_picoui_widgets --output-on-failure` 通过
  - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py` 通过
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过

### A3 交互控件真实映射

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage C`
- 当前状态：**已完成**
- 已完成内容：
  - `checkbox/switch/slider` 已建立真实 `LingDongGUI` 对象映射
  - `set_checked/set_value` 与 `create_with_props` 初值都会同步到底层真实 `ldCheckBox/ldSwitch/ldSlider`
  - `basic_widgets` 的交互控件已进入 REAL 路径，不再走 fake overlay
  - `settings_panel` 当前仍保留 Stage C fallback 边界，但理由已改成显式 `layout bridge` 阶段门，不再依赖偶然 sibling 结构
- 当前证据：
  - `ctest --test-dir build -R test_picoui_widgets --output-on-failure` 通过
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过
  - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py` 通过

### A4 真实 flex/grid layout

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage D`
- 当前状态：**已完成**
- 已完成内容：
  - `backend_layout` 已把 `flex/grid` 容器级 setter 真实映射到 `ldWindowSetFlex* / ldWindowSetGrid*`
  - 子项级 layout setter 已真实映射到 `ldBaseSetFlexGrow / FlexNewTrack / IgnoreLayout / GridCell`
  - `layout_flex` / `layout_grid` / `settings_panel` 已去掉 A4 禁止的 demo 侧硬编码尺寸/位置补丁
  - `settings_panel` 当前已改成 grid 语义与 grid cell 放置，不再靠固定 `width/height` 顶住画面
- 当前证据：
  - `ctest --test-dir build -R test_picoui_layout --output-on-failure` 通过
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 4 --build-dir build/capture-demo-4` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 5 --build-dir build/capture-demo-5` 通过

### A5 真实 event/message pipeline

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage E`
- 当前状态：**已完成**
- 已完成内容：
  - `backend_event` 已接入真实消息队列/控件事件，不再只靠单测手调 helper
  - `button/checkbox/switch/slider` 已建立 native-event-path 到 PicoUI callback/state 的真实桥接
  - `setter-path` 已收窄成状态同步优先，不再作为主事件入口
  - `test_picoui_button_events` / `test_picoui_widgets` 已区分 `setter-path` 与 `native-event-path` 合同
- 当前证据：
  - `ctest --test-dir build -R 'test_picoui_widgets|test_picoui_button_events' --output-on-failure` 通过
  - `backend_app.c` 每帧真实执行 `ldGuiFrameStart() -> ldMsgProcess() -> ldGuiDraw() -> ldGuiFrameComplete()`

### A6 真实 theme/state/part/style

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage F`
- 当前状态：**已完成**
- 已完成内容：
  - `theme/state/part/style` 已从 contract-level shadow 升级到真实 backend apply
  - `window/button/checkbox/switch/slider/label/text` 已建立到底层 `LingDongGUI` 控件的真实样式同步
  - `image` 不再假装支持真实 apply；当前合同明确为拒绝 `PICOUI_PART_MAIN`
  - `backend_style_apply.c` 已正式纳入构建，不再靠 `.c include` 临时接线
- 当前证据：
  - `ctest --test-dir build -R test_picoui_theme --output-on-failure` 通过
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过

### A7 demo/UI 收口

- 入口 plan：`docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- 对应阶段：`Stage G`
- 当前状态：**已完成**
- 已完成内容：
  - 文档、能力矩阵、README、demo guide 已与当前 `A1-A6` 代码现状重新对齐
  - `backend_app.c` 已收缩到 `temporary smoke path` / host smoke-capture 主责，不再承担正式 fake renderer 主输出职责
  - `runtime/capture`、`temporary smoke path`、`真实 backend 完成态` 的证据层级已统一写清
  - `image` 的当前合同已统一：对象映射/source 绑定已完成，但 `theme/style apply` 当前明确拒绝 `PICOUI_PART_MAIN`
- 当前证据：
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过
  - `ctest --test-dir build -L picoui --output-on-failure` 通过
  - `python3 examples/sdl/tests/check_use_demo_runtime.py --demo 0 --build-dir build/verify-demo-0` 通过
  - `python3 examples/sdl/tests/check_use_demo_runtime.py --demo 6 --build-dir build/verify-demo-6` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 1 --build-dir build/capture-demo-1` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 2 --build-dir build/capture-demo-2` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 3 --build-dir build/capture-demo-3` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 4 --build-dir build/capture-demo-4` 通过
  - `python3 examples/sdl/tests/check_use_demo_capture.py --demo 5 --build-dir build/capture-demo-5` 通过
- 非 A线 blocker：
  - `python3 examples/sdl/tests/check_switch_capture_matrix.py --build-dir build/switch-capture-verify` 当前仍暴露 legacy `USE_DEMO=0` / `uiWidgetLegacy` 的 SDL demo 侧问题，按测试架构文档归类为 `examples/sdl/tests/` 行为层回归，不纳入 `PicoUI` A 线最终 closeout gate。

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

1. 维持当前 `PicoUI -> backend/ldgui -> LingDongGUI -> SDL host` 主线
2. 把 `runtime/capture` 继续当作 smoke / 回归证据使用
3. 后续若新增目标，需基于当前已收口主线另开阶段或新计划

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

下面条件现已全部满足，`A线` 已收口：

- `A1` 完成
- `A2` 完成
- `A3` 完成
- `A4` 完成
- `A5` 完成
- `A6` 完成
- `A7` 完成
- `backend_app.c` 不再承担正式 fake renderer 职责
- `PicoUI` demo 的可见 UI 来自真实 `LingDongGUI` backend 映射，而不是手工拼图
- `Stage G` 的最终全量门禁已按最新口径重新执行并通过
