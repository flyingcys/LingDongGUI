# TINYUI D线当前控件完整性设计文档

> 日期：2026-05-29
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 建议 worktree：`.worktree/tinyui-d-current-widgets`
> 入口索引：`docs/tinyui-serial/D-线计划索引.md`

## 1. 背景

`A线` 已完成真实 `TINYUI -> LingDongGUI` backend 主线，`B线` 已完成 automatic visible gate，`C线` 已把 smoke、mapping、visible、manual artifact 的门禁口径工程化。当前可以进入新能力推进，但 review 明确指出一个关键缺口：runtime present 路径仍在 `tinyui/src/backend/ldgui/backend_app.c` 中通过 `tinyui_backend_apply_real_widget_layout()` 用固定 padding、row height、row gap 和 cursor 线性排布所有 widget。

这意味着当前 mapping marker 可以证明对象进入真实 backend tree，visible gate 可以证明画面不是黑屏/近黑/空白，但还不能声明 flex/grid 可见语义已经完整由真实 `LingDongGUI` layout 结果驱动。

## 2. 目标

`D线` 的目标是把当前已暴露控件做完整，而不是先扩新控件数量。完成后应满足：

- runtime present 不再用全局线性 cursor 覆盖真实 flex/grid layout 结果。
- `window/label/button/checkbox/switch/slider/text/image` 的 public contract 有完整矩阵。
- props、enabled/visible/state、event、image、theme 的支持/拒绝语义都有测试或文档证据。
- 6 个现有 demo 的 smoke、backend mapping、automatic visible gate 仍然通过。

## 3. 非目标

- 不新增 `TINYUI` 新控件；新控件由 `F线` 负责。
- 不把 `backend_app.c` 改成新的 fake renderer。
- 不用 demo 侧固定坐标或硬编码视觉补丁掩盖 backend/layout/theme 缺口。
- 不把 manual artifact、capture 非空或 `ctest -L tinyui` 单独写成完整 UI 完成。

## 4. 设计

### 4.1 runtime present layout

当前 `backend_app.c` 在绘制前直接改写底层 `ldBase` region，这是 D 线第一优先级。目标不是删除所有 host harness 逻辑，而是把全局线性排布收缩成明确的 temporary smoke fallback，避免覆盖有 layout 语义的容器。

验收重点：

- `layout_flex` 和 `layout_grid` 的 visible gate 增加结构断言。
- 有 flex/grid 语义的 demo 不再依赖 `backend_app.c` 的全局 cursor 排布。
- 若某些无 layout 语义 demo 仍需 fallback，应在代码和文档中标注为 temporary smoke path。

### 4.2 当前控件 contract matrix

建立当前控件矩阵，覆盖：

- `create` / `create_with_props`
- text/value/checked/range/source/user_data/style_class
- enabled/visible/focus/dirty/layout/theme/event
- support / reject / deferred 三类状态
- 对应证据层：unit、contract、mapping、visible、manual artifact

矩阵不是说明性表格，而是后续代码任务的门禁输入。任何新增或修改 API 都必须先落在矩阵里。

### 4.3 props 和 state

props 补齐只处理当前控件，按 `label/text/image/window -> checkbox/switch/slider -> button` 顺序推进。`enabled/visible/state` 必须明确是否同步到底层 `LingDongGUI` 行为；不能只更新 TINYUI shadow 字段。

### 4.4 event

事件合同必须覆盖：

- `button` clicked
- `checkbox` toggled
- `switch` toggled
- `slider` value_changed
- callback user_data
- setter-path 与 native-event-path 的区别

### 4.5 image

`image` 先收口当前边界，不直接引入复杂资源系统。必须明确 source 类型、空 source、无效 source、占位显示、theme/style apply 拒绝语义。

### 4.6 theme

theme token v1 只做当前控件稳定语义，不做完整 CSS-like 样式系统。每个支持 token 必须有 unit test；不支持的 part/state 必须明确拒绝。

## 5. 并行边界

`D线` 可与 `F线` 并行，但 `D线` 独占 shared quality 写面：

- `backend_app.c`
- `backend_layout.c`
- `backend_event.c`
- `backend_style_apply.c`
- `backend_theme.c`
- 当前控件 headers / widgets / unit tests / runtime visible gate

`D线` 内部串行推进。任何 review 后修复回到原 subagent，不新开 subagent 修复，也不由主线程顺手修。

## 6. 验收

最小门禁：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
git diff --check
```

汇报时必须区分：

- 当前阶段完成
- automatic visible gate 通过
- backend mapping gate 通过
- 是否存在人工窗口验收结论

