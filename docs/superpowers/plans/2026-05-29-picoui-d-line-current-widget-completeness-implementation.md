# PicoUI D线当前控件完整性实施计划

> **给 agentic workers:** REQUIRED SUB-SKILL: 使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用 checkbox（`- [ ]`）格式跟踪。

**目标:** 让当前 PicoUI 控件从“基础 backend 和门禁已通”推进到 runtime layout、contract、props、state、event、image、theme 语义完整可测。

**架构:** D 线在 `.worktree/picoui-d-current-widgets` 串行推进。先消除 runtime present 线性布局覆盖，再冻结当前控件合同矩阵，最后按 props/state/event/image/theme 分组收口。每个 Task 由 fresh subagent 执行，Task 后由独立 review subagent 只读 review。

**技术栈:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 创建 worktree：`.worktree/picoui-d-current-widgets`。
- 创建或切换后必须同步并更新 submodule。
- D 线内部严格串行：`D0 -> D1 -> D2 -> D3 -> D4 -> D5 -> D6 -> D7`。
- 修改函数、方法或类前必须运行 GitNexus impact，并在 subagent 最终回复中报告风险。
- 每个 Task 完成后运行 `gitnexus_detect_changes(scope="all")`。
- 每个 Task 后必须只读 review；review 不通过时，原 subagent 在同一上下文修复。
- 不修改 F 线新控件文件；不新增新控件。

## 1. 文件结构与写面

### D1 layout present 组

- 修改: `picoui/src/backend/ldgui/backend_app.c`
- 修改: `picoui/src/backend/ldgui/backend_layout.c`
- 修改: `picoui/src/layout/flex.c`
- 修改: `picoui/src/layout/grid.c`
- 修改: `tests/picoui/runtime/check_picoui_visible_ui.py`
- 修改: `tests/picoui/unit/test_picoui_layout.c`

### D2 contract matrix 组

- 新增: `docs/superpowers/specs/2026-05-29-picoui-d-line-widget-contract-matrix.md`
- 可选新增: `tests/picoui/contract/check_picoui_widget_contract_matrix.py`
- 修改: `tests/picoui/CMakeLists.txt` 仅当 adding a new contract script

### D3 props 组

- 修改: `picoui/include/picoui/{window,label,button,checkbox,switch,slider,text,image}.h`
- 修改: `picoui/src/widgets/{window,label,button,checkbox,switch,slider,text,image}.c`
- 修改: `tests/picoui/unit/test_picoui_widgets.c`

### D4 state/event/theme 组

- 修改: `picoui/src/core/widget.c`
- 修改: `picoui/src/backend/ldgui/backend_event.c`
- 修改: `picoui/src/backend/ldgui/backend_style_apply.c`
- 修改: `picoui/src/backend/ldgui/backend_theme.c`
- 修改: `tests/picoui/unit/test_picoui_button_events.c`
- 修改: `tests/picoui/unit/test_picoui_theme.c`

### D5 image 组

- 修改: `picoui/include/picoui/image.h`
- 修改: `picoui/src/widgets/image.c`
- 修改: `picoui/src/backend/ldgui/backend_image.c`
- 修改: `tests/picoui/unit/test_picoui_widgets.c`
- 修改: `picoui/docs/demo_guide.md`

## 2. Tasks

### Task D0: worktree 准备和 baseline

**文件:**
- Read: `docs/picoui-serial/D-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-29-picoui-d-line-current-widget-completeness-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/picoui-d-current-widgets HEAD
cd .worktree/picoui-d-current-widgets
git submodule sync --recursive
git submodule update --init --recursive
```

- [ ] **Step 2: 配置 baseline build**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
```

期望: configure 成功。

- [ ] **Step 3: 运行 baseline gate**

```bash
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

期望: 全部通过。若失败，停止并记录为 D0 blocker，不进入 D1。

### Task D1: runtime present 真实 layout 闭环

**文件:**
- 修改: `picoui/src/backend/ldgui/backend_app.c`
- 修改: `tests/picoui/runtime/check_picoui_visible_ui.py`
- 修改: `tests/picoui/unit/test_picoui_layout.c`
- 可选修改: `picoui/src/backend/ldgui/backend_layout.c`

- [ ] **Step 1: GitNexus impact**

运行 impact：

```text
gitnexus_impact(target="picoui_backend_apply_real_widget_layout", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_render", direction="upstream", repo="LingDongGUI")
```

期望: 记录 direct callers、affected processes 和风险等级。若 HIGH/CRITICAL，先汇报主线程再继续。

- [ ] **Step 2: 写 visible RED 断言**

在 `tests/picoui/runtime/check_picoui_visible_ui.py` 中为 `layout_flex` 和 `layout_grid` 增加结构断言：

- `layout_flex`：检查子控件在主轴方向有明显分布，不接受单列 cursor 堆叠。
- `layout_grid`：检查左右/上下单元格有独立可见区域，不接受所有子项按同一 x 坐标线性堆叠。

运行：

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_flex
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_grid
```

期望: 当前实现应暴露 layout 结构问题；如果没有失败，先说明断言不足并加强断言，不直接改实现。

- [ ] **Step 3: 收缩 cursor fallback**

修改 `backend_app.c`：

- 不再对有 flex/grid layout 语义的 backend subtree 全局调用 cursor 排布。
- 如果无 layout 语义 demo 仍需默认排布，把 fallback 限定在明确无 layout 的 root children，并用短注释标注 temporary smoke path。
- 保持 `ldGuiFrameStart() -> ldMsgProcess() -> ldGuiDraw() -> ldGuiFrameComplete()` 真实绘制链。

- [ ] **Step 4: 补 layout unit 边界**

在 `tests/picoui/unit/test_picoui_layout.c` 增加：

- padding / gap 边界。
- grid cell span / align 的支持或拒绝语义。
- layout dirty / relayout 触发语义。

- [ ] **Step 5: 验证 D1**

```bash
ctest --test-dir build -R test_picoui_layout --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_flex
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_grid
ctest --test-dir build -L visible --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
git diff --check
```

期望: 全部通过。

- [ ] **Step 6: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: 只影响 layout/present 相关符号和 PicoUI runtime gate。

### Task D2: 当前控件合同矩阵

**文件:**
- 新增: `docs/superpowers/specs/2026-05-29-picoui-d-line-widget-contract-matrix.md`
- 可选新增: `tests/picoui/contract/check_picoui_widget_contract_matrix.py`
- 可选修改: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 生成合同矩阵文档**

矩阵必须覆盖：

- `window`
- `label`
- `button`
- `checkbox`
- `switch`
- `slider`
- `text`
- `image`

每个控件必须列出：

- create / create_with_props
- text/value/checked/range/source/user_data/style_class
- enabled/visible/focus/dirty/layout/theme/event
- support / reject / deferred
- 证据层：unit、contract、mapping、visible、manual artifact

- [ ] **Step 2: 可选 contract script**

如果矩阵可以机械检查，新增 `check_picoui_widget_contract_matrix.py`，至少检查 public header 与矩阵中声明的 API 名称一致。

- [ ] **Step 3: 验证 D2**

```bash
git diff --check
rg -n "support|reject|deferred|window|label|button|checkbox|switch|slider|text|image" docs/superpowers/specs/2026-05-29-picoui-d-line-widget-contract-matrix.md
ctest --test-dir build -L contract --output-on-failure
```

期望: 文档无占位，contract gate 通过。

### Task D3: 当前控件 props 补齐

**文件:**
- 修改: `picoui/include/picoui/{window,label,button,checkbox,switch,slider,text,image}.h`
- 修改: `picoui/src/widgets/{window,label,button,checkbox,switch,slider,text,image}.c`
- 修改: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: GitNexus impact**

对每个要改的 create / create_with_props / setter 运行 upstream impact。若一次涉及多个符号，至少覆盖每个控件的主要 create_with_props 和 setter。

- [ ] **Step 2: 写 props 测试**

在 `test_picoui_widgets.c` 中按合同矩阵补 props 初值测试：

- label/text/image/window
- checkbox/switch/slider
- button

期望: 先看到缺失 API 或行为不一致的失败。

- [ ] **Step 3: 最小实现**

只实现矩阵中 D3 声明为 support 的 props，不顺手扩未计划 API。所有 public API 仍只暴露 `picoui_*`。

- [ ] **Step 4: 验证 D3**

```bash
ctest --test-dir build -R test_picoui_widgets --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
git diff --check
```

期望: 全部通过。

### Task D4: enabled / visible / state 和 event 合同

**文件:**
- 修改: `picoui/src/core/widget.c`
- 修改: `picoui/src/backend/ldgui/backend_event.c`
- 修改: `picoui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tests/picoui/unit/test_picoui_button_events.c`
- 修改: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: GitNexus impact**

覆盖：

```text
picoui_widget_set_enabled
picoui_widget_set_visible
picoui_backend_ld_event_bridge_slot
picoui_backend_apply_style
```

- [ ] **Step 2: 写 state/event RED 测试**

测试必须覆盖：

- disabled 控件事件是否被拦截，或明确仍允许但文档说明。
- hidden 控件是否从 visible/readback 语义中排除，或明确只做 state shadow。
- checkbox/switch/slider callback user_data。
- setter-path 与 native-event-path 区分。

- [ ] **Step 3: 实现状态同步**

实现只覆盖合同矩阵已支持项。拒绝项必须测试锁定。

- [ ] **Step 4: 验证 D4**

```bash
ctest --test-dir build -R 'test_picoui_button_events|test_picoui_theme|test_picoui_widgets' --output-on-failure
ctest --test-dir build -L visible --output-on-failure
git diff --check
```

期望: 全部通过。

### Task D5: image 能力边界收口

**文件:**
- 修改: `picoui/include/picoui/image.h`
- 修改: `picoui/src/widgets/image.c`
- 修改: `picoui/src/backend/ldgui/backend_image.c`
- 修改: `tests/picoui/unit/test_picoui_widgets.c`
- 修改: `picoui/docs/demo_guide.md`

- [ ] **Step 1: GitNexus impact**

覆盖：

```text
picoui_image_create
picoui_image_set_source
picoui_backend_create_image
```

- [ ] **Step 2: 写 image contract 测试**

覆盖：

- 空 source。
- 无效 source。
- 当前占位显示语义。
- theme/style apply 拒绝语义是否继续保留。

- [ ] **Step 3: 实现或锁定拒绝**

推荐先锁定当前边界，不引入复杂资源系统。若实现真实资源加载，必须另开设计，不在 D5 顺手扩。

- [ ] **Step 4: 验证 D5**

```bash
ctest --test-dir build -R test_picoui_widgets --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
git diff --check
```

期望: 全部通过，demo guide 不把占位显示写成真实图片加载完成。

### Task D6: theme token v1

**文件:**
- 修改: `picoui/include/picoui/theme.h`
- 修改: `picoui/src/theme/theme.c`
- 修改: `picoui/src/backend/ldgui/backend_theme.c`
- 修改: `picoui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: GitNexus impact**

覆盖 theme apply 和 style apply 入口。

- [ ] **Step 2: 写 theme token 测试**

覆盖：

- 文本色。
- 背景色。
- 边框色。
- 强调色。
- 禁用色。
- 默认间距和控件高度。
- 不支持 part/state 的拒绝语义。

- [ ] **Step 3: 实现 token v1**

只做当前控件稳定 token，不做 CSS-like 样式系统。

- [ ] **Step 4: 验证 D6**

```bash
ctest --test-dir build -R test_picoui_theme --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo theme_showcase
ctest --test-dir build -L picoui --output-on-failure
git diff --check
```

期望: 全部通过。

### Task D7: D线 closeout review

**文件:**
- 修改: `docs/picoui-serial/D-线计划索引.md`
- 可选修改: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 独立 review**

派独立 review subagent 做只读 review，重点检查：

- 是否还存在 runtime present 线性 layout 覆盖。
- 当前控件矩阵是否和 public header / tests 一致。
- 是否误把 smoke/mapping/visible/manual artifact 混写。
- F 线写面是否被 D 线污染。

- [ ] **Step 2: 最终验证**

```bash
git status --short
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
git diff --check
```

期望: 全部通过。

- [ ] **Step 3: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: affected processes 与 D 线范围一致。
