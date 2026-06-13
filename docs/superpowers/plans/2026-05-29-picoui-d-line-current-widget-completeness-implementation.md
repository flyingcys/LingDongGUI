# TINYUI D线当前控件完整性实施计划

> **给 agentic workers:** REQUIRED SUB-SKILL: 使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用 checkbox（`- [ ]`）格式跟踪。

**目标:** 让当前 TINYUI 控件从“基础 backend 和门禁已通”推进到 runtime layout、contract、props、state、event、image、theme 语义完整可测。

**架构:** D 线在 `.worktree/tinyui-d-current-widgets` 串行推进。先消除 runtime present 线性布局覆盖，再冻结当前控件合同矩阵，最后按 props/state/event/image/theme 分组收口。每个 Task 由 fresh subagent 执行，Task 后由独立 review subagent 只读 review。

**技术栈:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 创建 worktree：`.worktree/tinyui-d-current-widgets`。
- 创建或切换后必须同步并更新 submodule。
- D 线内部严格串行：`D0 -> D1 -> D2 -> D3 -> D4 -> D5 -> D6 -> D7`。
- 修改函数、方法或类前必须运行 GitNexus impact，并在 subagent 最终回复中报告风险。
- 每个 Task 完成后运行 `gitnexus_detect_changes(scope="all")`。
- 每个 Task 后必须只读 review；review 不通过时，原 subagent 在同一上下文修复。
- 不修改 F 线新控件文件；不新增新控件。

## 1. 文件结构与写面

### D1 layout present 组

- 修改: `tinyui/src/backend/ldgui/backend_app.c`
- 修改: `tinyui/src/backend/ldgui/backend_layout.c`
- 修改: `tinyui/src/layout/flex.c`
- 修改: `tinyui/src/layout/grid.c`
- 修改: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- 修改: `tests/tinyui/unit/test_tinyui_layout.c`

### D2 contract matrix 组

- 新增: `docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md`
- 可选新增: `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`
- 修改: `tests/tinyui/CMakeLists.txt` 仅当 adding a new contract script

### D3 props 组

- 修改: `tinyui/include/tinyui/{window,label,button,checkbox,switch,slider,text,image}.h`
- 修改: `tinyui/src/widgets/{window,label,button,checkbox,switch,slider,text,image}.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`

### D4 state/event/theme 组

- 修改: `tinyui/src/core/widget.c`
- 修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tinyui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tinyui/src/backend/ldgui/backend_theme.c`
- 修改: `tests/tinyui/unit/test_tinyui_button_events.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`

### D5 image 组

- 修改: `tinyui/include/tinyui/image.h`
- 修改: `tinyui/src/widgets/image.c`
- 修改: `tinyui/src/backend/ldgui/backend_image.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tinyui/docs/demo_guide.md`

## 2. Tasks

### Task D0: worktree 准备和 baseline

**文件:**
- Read: `docs/tinyui-serial/D-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-29-tinyui-d-line-current-widget-completeness-design.md`

- [x] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/tinyui-d-current-widgets HEAD
cd .worktree/tinyui-d-current-widgets
git submodule sync --recursive
git submodule update --init --recursive
```

- [x] **Step 2: 配置 baseline build**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
```

期望: configure 成功。

- [x] **Step 3: 运行 baseline gate**

```bash
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

期望: 全部通过。若失败，停止并记录为 D0 blocker，不进入 D1。

2026-05-29 证据：
- `git submodule sync --recursive && git submodule update --init --recursive`：通过。
- `rtk cmake -S . -B build -DUSE_DEMO=0`：通过。
- `rtk cmake --build build -j8`：通过。
- `ctest --test-dir build -L tinyui --output-on-failure`：10/10 通过。
- `ctest --test-dir build -L visible --output-on-failure`：通过。
- `ctest --test-dir build -L mapping --output-on-failure`：通过。

### Task D1: runtime present 真实 layout 闭环

**文件:**
- 修改: `tinyui/src/backend/ldgui/backend_app.c`
- 修改: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- 修改: `tests/tinyui/unit/test_tinyui_layout.c`
- 可选修改: `tinyui/src/backend/ldgui/backend_layout.c`

- [x] **Step 1: GitNexus impact**

运行 impact：

```text
gitnexus_impact(target="tinyui_backend_apply_real_widget_layout", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_render", direction="upstream", repo="LingDongGUI")
```

期望: 记录 direct callers、affected processes 和风险等级。若 HIGH/CRITICAL，先汇报主线程再继续。

2026-05-29 证据：GitNexus 对 `tinyui_backend_apply_real_widget_layout` / `tinyui_backend_render` 返回 `Target not found`，风险为 `UNKNOWN`。同文件 Cypher 查询未列出 `tinyui/src/backend/ldgui/backend_app.c` 符号，判断当前索引未覆盖该 static backend 文件/函数；本任务继续以 `git diff`、源码 review 和 targeted tests 作为主要证据。

- [x] **Step 2: 写 visible RED 断言**

在 `tests/tinyui/runtime/check_tinyui_visible_ui.py` 中为 `layout_flex` 和 `layout_grid` 增加结构断言：

- `layout_flex`：检查子控件在主轴方向有明显分布，不接受单列 cursor 堆叠。
- `layout_grid`：检查左右/上下单元格有独立可见区域，不接受所有子项按同一 x 坐标线性堆叠。

运行：

```bash
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_flex
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_grid
```

期望: 当前实现应暴露 layout 结构问题；如果没有失败，先说明断言不足并加强断言，不直接改实现。

2026-05-29 证据：新增 `layout_flex` / `layout_grid` 结构断言后，当前实现先暴露全局 cursor 线性覆盖问题；后续修复后两个 demo visible gate 通过。`layout_grid` 断言已调整为多 y 窗口扫描独立 column groups，避免依赖第二个 row run。

- [x] **Step 3: 收缩 cursor fallback**

修改 `backend_app.c`：

- 不再对有 flex/grid layout 语义的 backend subtree 全局调用 cursor 排布。
- 如果无 layout 语义 demo 仍需默认排布，把 fallback 限定在明确无 layout 的 root children，并用短注释标注 temporary smoke path。
- 保持 `ldGuiFrameStart() -> ldMsgProcess() -> ldGuiDraw() -> ldGuiFrameComplete()` 真实绘制链。

2026-05-29 证据：`backend_app.c` 新增 `tinyui_backend_apply_smoke_cursor_layout()`；当 root 是真实 `layoutFlex` / `layoutGrid` 时不再调用全局 cursor fallback。无 layout 语义 demo 保留局部 `temporary smoke path`，真实绘制链保持不变。

- [x] **Step 4: 补 layout unit 边界**

在 `tests/tinyui/unit/test_tinyui_layout.c` 增加：

- padding / gap 边界。
- grid cell span / align 的支持或拒绝语义。
- layout dirty / relayout 触发语义。

2026-05-29 证据：`test_tinyui_layout.c` 覆盖 padding / gap 边界、grid cell span / align 拒绝语义、layout dirty / relayout；同时补 `tinyui_widget_set_padding()` 到真实 backend 的同步，并验证 padding 在 flex/grid layout type 切换后保持。

- [x] **Step 5: 验证 D1**

```bash
ctest --test-dir build -R test_tinyui_layout --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_flex
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_grid
ctest --test-dir build -L visible --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
git diff --check
```

期望: 全部通过。

2026-05-29 证据：
- `ctest --test-dir build -R test_tinyui_layout --output-on-failure`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_flex`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_grid`：通过。
- `ctest --test-dir build -L visible --output-on-failure`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`：通过。
- `git diff --check`：通过。
- D1 spec review：通过。
- D1 code quality review：通过。

- [x] **Step 6: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: 只影响 layout/present 相关符号和 TINYUI runtime gate。

2026-05-29 证据：`gitnexus_detect_changes(scope="all", repo="LingDongGUI")` 返回 `No changes detected`，与 `git diff` 不一致；判定为当前 GitNexus 对 linked worktree / static backend 写面映射不足，不能作为本任务主要影响证据。实际 diff 限定在 D1 layout/present/runtime gate 写面。

### Task D2: 当前控件合同矩阵

**文件:**
- 新增: `docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md`
- 可选新增: `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`
- 可选修改: `tests/tinyui/CMakeLists.txt`

- [x] **Step 1: 生成合同矩阵文档**

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

2026-05-29 证据：已新增 `docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md`，覆盖 `window/label/button/checkbox/switch/slider/text/image` 当前控件矩阵。矩阵区分 `support/reject/deferred`，并明确 `unit/contract/mapping/visible/manual artifact` 五层证据；`visible` 能力列只表示 `tinyui_widget_set_visible` setter 口径，不等于 visible gate 证据层。

- [x] **Step 2: 可选 contract script**

如果矩阵可以机械检查，新增 `check_tinyui_widget_contract_matrix.py`，至少检查 public header 与矩阵中声明的 API 名称一致。

2026-05-29 证据：已新增 `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 并接入 `tests/tinyui/CMakeLists.txt`。脚本检查 public header 与矩阵 API 名称一致，并增加 drift guard：从 `widget.h` 读取全部 `tinyui_widget_*`，要求每个 API 进入显式 policy；policy 同时校验适用控件必须包含、不适用控件不得误列。脚本还覆盖 window layout API、child layout API、theme API 必填/不适用边界，以及能力状态矩阵行完整性和 `support/reject/deferred` 状态值。

- [x] **Step 3: 验证 D2**

```bash
git diff --check
rg -n "support|reject|deferred|window|label|button|checkbox|switch|slider|text|image" docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md
ctest --test-dir build -L contract --output-on-failure
```

期望: 文档无占位，contract gate 通过。

2026-05-29 证据：
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `ctest --test-dir build -L contract --output-on-failure`：7/7 通过。
- `git diff --check`：通过。
- `rg -n "support|reject|deferred|window|label|button|checkbox|switch|slider|text|image" docs/superpowers/specs/2026-05-29-tinyui-d-line-widget-contract-matrix.md`：命中矩阵内容。
- D2 spec review 首轮和第二轮问题已在同一 subagent 修复：补齐 direct style setter、非 window padding，并收紧 contract script。
- D2 code quality review 首轮和第二轮问题已在同一 subagent 修复：新增 `widget.h` drift guard、适用/不适用双向校验、layout/theme required policy、状态矩阵校验和更友好的矩阵 marker 失败消息。

- [x] **Step 4: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: 只影响 D2 文档/contract 写面；若 GitNexus 对 linked worktree 返回 `No changes detected`，如实记录为弱证据。

2026-05-29 证据：`gitnexus_detect_changes(scope="all", repo="LingDongGUI")` 返回 `No changes detected`、`risk_level: none`。这与 `git diff` 中 D2 文档/contract 改动不一致，判断仍是当前 GitNexus 对 linked worktree 的变更识别弱证据；D2 实际 diff 限定在合同矩阵文档、contract script 和 `tests/tinyui/CMakeLists.txt` 接入。

### Task D3: 当前控件 props 补齐

**文件:**
- 修改: `tinyui/include/tinyui/{window,label,button,checkbox,switch,slider,text,image}.h`
- 修改: `tinyui/src/widgets/{window,label,button,checkbox,switch,slider,text,image}.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`

- [x] **Step 1: GitNexus impact**

对每个要改的 create / create_with_props / setter 运行 upstream impact。若一次涉及多个符号，至少覆盖每个控件的主要 create_with_props 和 setter。

2026-05-29 证据：GitNexus impact 覆盖 `tinyui_window_create`、`tinyui_label_create`、`tinyui_text_create`、`tinyui_image_create`、`tinyui_button_create_with_props`、`tinyui_checkbox_create_with_props`、`tinyui_switch_create_with_props`、`tinyui_slider_create_with_props`；均返回 LOW，direct callers 0，affected processes 0。

- [x] **Step 2: 写 props 测试**

在 `test_tinyui_widgets.c` 中按合同矩阵补 props 初值测试：

- label/text/image/window
- checkbox/switch/slider
- button

期望: 先看到缺失 API 或行为不一致的失败。

2026-05-29 证据：`tests/tinyui/unit/test_tinyui_widgets.c` 已覆盖 8 个当前控件的 props 初值、通用 style/size/padding/user_data 同步、交互控件 callback/user_data、image source 初始绑定，并新增 invalid props 不污染 backend tree 的失败路径测试。RED 阶段先暴露 invalid props 在 setter 失败时会留下已 attach backend child，后续通过 props 预校验修复。

- [x] **Step 3: 最小实现**

只实现矩阵中 D3 声明为 support 的 props，不顺手扩未计划 API。所有 public API 仍只暴露 `tinyui_*`。

2026-05-29 证据：已为 `window/label/text/image` 新增 props struct 和 `create_with_props`；已补齐 `button/checkbox/switch/slider` props 字段。实现走已有 create/setter 路径；invalid props 在创建前预校验，避免失败时污染 backend tree。D2 合同矩阵和 contract script 已同步 8 个 `create_with_props`，未引入 F 线新控件。

- [x] **Step 4: 验证 D3**

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
git diff --check
```

期望: 全部通过。

2026-05-29 证据：
- `ctest --test-dir build -R test_tinyui_widgets --output-on-failure`：通过。
- `python3 tests/tinyui/contract/check_tinyui_public_api.py`：通过。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`：通过。
- `git diff --check`：通过。
- D3 spec review：通过。残余风险为未来 setter 新增失败条件时需补统一 detach/destroy，invalid props 测试不是全字段穷举。
- D3 code quality review：通过。残余风险为 props struct ABI 稳定性、字符串字面量指针相等测试可移植性、8 个控件 props 应用逻辑重复。

- [x] **Step 5: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: 只影响 D3 当前控件 props/header/widget/unit/contract 写面；若 GitNexus 对 linked worktree 返回 `No changes detected`，如实记录为弱证据。

2026-05-29 证据：`gitnexus_detect_changes(scope="all", repo="LingDongGUI")` 返回 `No changes detected`、`risk_level: none`。这与 `git diff` 中 D3 代码和测试改动不一致，判断仍是当前 GitNexus 对 linked worktree 的变更识别弱证据；D3 实际 diff 限定在当前 8 个控件 props/header/widget、`test_tinyui_widgets.c`、合同矩阵和 contract script。

### Task D4: enabled / visible / state 和 event 合同

**文件:**
- 修改: `tinyui/src/core/widget.c`
- 修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tinyui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tests/tinyui/unit/test_tinyui_button_events.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`

- [x] **Step 1: GitNexus impact**

覆盖：

```text
tinyui_widget_set_enabled
tinyui_widget_set_visible
tinyui_backend_ld_event_bridge_slot
tinyui_backend_apply_style
```

2026-05-29 证据：主线程已完成 impact，`tinyui_widget_set_enabled` 和 `tinyui_widget_set_visible` 均为 LOW、direct callers 0、affected processes 0；`tinyui_backend_ld_event_bridge_slot` 和 `tinyui_backend_apply_style` 返回 Target not found，风险 UNKNOWN。D4 未新增修改其他函数。

- [x] **Step 2: 写 state/event RED 测试**

测试必须覆盖：

- disabled 控件事件是否被拦截，或明确仍允许但文档说明。
- hidden 控件是否从 visible/readback 语义中排除，或明确只做 state shadow。
- checkbox/switch/slider callback user_data。
- setter-path 与 native-event-path 区分。

2026-05-29 RED 证据：新增 `test_tinyui_button_events` 和 `test_tinyui_theme` 断言后，`ctest --test-dir build -R 'test_tinyui_button_events|test_tinyui_theme' --output-on-failure` 失败。失败点：`test_tinyui_theme` 中 `tinyui_widget_set_visible(..., 0)` 未同步到底层 `ldBaseIsHidden == true`；`test_tinyui_button_events` 中 disabled button 收到 native `SIGNAL_PRESS` 后仍触发 callback。

- [x] **Step 3: 实现状态同步**

实现只覆盖合同矩阵已支持项。拒绝项必须测试锁定。

2026-05-29 证据：`tinyui_widget_set_visible` 同步到底层 `ldBaseSetHidden`，hidden 控件从真实 LingDongGUI 绘制/readback 语义中排除；`tinyui_widget_set_enabled` 保持通用 TINYUI enabled state，并对 switch 同步 `ldSwitchSetDisabled`。backend event/native dispatch 对 disabled 或 hidden 控件返回 no-op，不触发 callback、不推进 dispatch_count。checkbox/switch/slider setter-path 只同步状态，native-event-path 触发 callback并传递 user_data。D4 spec review 首轮要求补齐 hidden event no-op 测试，已在同一 subagent 内修复：覆盖 hidden button direct/native event no-op，以及 hidden checkbox/switch/slider `SIGNAL_VALUE_CHANGED` no-op。

- [x] **Step 4: 验证 D4**

```bash
ctest --test-dir build -R 'test_tinyui_button_events|test_tinyui_theme|test_tinyui_widgets' --output-on-failure
ctest --test-dir build -L visible --output-on-failure
git diff --check
```

期望: 全部通过。

2026-05-29 验证证据：
- `ctest --test-dir build -R 'test_tinyui_button_events|test_tinyui_theme|test_tinyui_widgets' --output-on-failure`：3/3 通过。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`：通过；过程中 CMake 仍输出既有 `ARM_SECTION(x)=` function-style preprocessor warning，但命令返回 0。
- `ctest --test-dir build -L visible --output-on-failure`：1/1 通过。
- `git diff --check`：通过。
- D4 spec review：首轮 FAIL 后同一 subagent 修复 hidden event no-op 测试，复审通过。
- D4 code quality review：通过。残余风险为 `widget.c` 直接 forward declare `ldSwitchSetDisabled`，若后续更多控件接入 native disabled，建议收敛成 backend helper；hidden/disabled no-op 返回 0 会降低 invalid signal 调试严格性，但符合 D4 合同。

- [x] **Step 5: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

2026-05-29 证据：返回 `No changes detected`、`risk_level: none`。这与 linked worktree 中 D1-D4 diff 不一致，沿用 D1-D3 结论：当前 GitNexus 对 linked worktree/static backend 写面变更识别是弱证据；D4 实际新增 diff 限定在 state/event/theme unit、widget/event 实现、合同矩阵和本计划记录。

### Task D5: image 能力边界收口

**文件:**
- 修改: `tinyui/include/tinyui/image.h`
- 修改: `tinyui/src/widgets/image.c`
- 修改: `tinyui/src/backend/ldgui/backend_image.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tinyui/docs/demo_guide.md`

- [x] **Step 1: GitNexus impact**

覆盖：

```text
tinyui_image_create
tinyui_image_set_source
tinyui_backend_create_image
```

2026-05-29 证据：主线程已完成 GitNexus impact：`tinyui_image_create` LOW、direct callers 0、affected processes 0；`tinyui_image_set_source` LOW、direct callers 0、affected processes 0；`tinyui_backend_create_image` LOW、direct callers 0、affected processes 0。D5 额外确认 `tinyui_backend_set_image_source` 实现符号为 LOW、direct callers 0、affected processes 0；`tinyui_image_create_with_props` 当前索引未命中，风险记为 UNKNOWN 弱证据。

- [x] **Step 2: 写 image contract 测试**

覆盖：

- 空 source。
- 无效 source。
- 当前占位显示语义。
- theme/style apply 拒绝语义是否继续保留。

2026-05-29 证据：`tests/tinyui/unit/test_tinyui_widgets.c` 新增 image 边界测试：空 props source 创建、`tinyui_image_set_source(image, NULL)` 清空绑定、`img_tile == NULL` 无效 source 拒绝且不污染 backend、`mask_tile == NULL` 无遮罩 source 允许、真实 `ldImage` 存在但无 source 时 `ptImgTile/ptMaskTile == NULL`、image theme apply 继续拒绝。RED 阶段在 `empty_image != 0` 断言失败，暴露 `create_with_props` 仍拒绝空 source。

- [x] **Step 3: 实现或锁定拒绝**

推荐先锁定当前边界，不引入复杂资源系统。若实现真实资源加载，必须另开设计，不在 D5 顺手扩。

2026-05-29 证据：`tinyui_image_create_with_props` 允许 `source == NULL`；`tinyui_image_set_source` 与 backend setter 允许 `source == NULL` 清空绑定；非空 source 拒绝 `img_tile == NULL`，允许 `mask_tile == NULL`；source 绑定只写入调用方 tile 指针并调用 `ldImageSetImage`，未引入资源加载系统。

- [x] **Step 4: 验证 D5**

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo basic_widgets
git diff --check
```

期望: 全部通过，demo guide 不把占位显示写成真实图片加载完成。

2026-05-29 证据：
- `rtk cmake --build build -j8`：通过，确认重新编译 `tinyui/src/widgets/image.c` 和 `test_tinyui_widgets`。
- `ctest --test-dir build -R test_tinyui_widgets --output-on-failure`：通过。
- `ctest --test-dir build -R test_tinyui_theme --output-on-failure`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo basic_widgets`：通过；过程中仍有既有 `ARM_SECTION(x)=` function-style preprocessor warning，但命令返回 0。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `git diff --check`：通过。
- D5 spec review：首轮 FAIL 后同一 subagent 修复文档证据、invalid non-null source props 测试、image theme reject 不污染 source/backend tile 测试，复审通过。
- D5 code quality review：首轮 FAIL 指出 D6 theme token/style 改动混入 D5；同一 subagent 移除 D6 范围 diff 后复审通过。当前 `theme.c` 和 `backend_style_apply.c` 无 D5 diff，`test_tinyui_theme.c` 只保留 D4 visible/enabled 测试。
- `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`：返回 `No changes detected`、`risk_level: none`。这与 linked worktree 中 D5 diff 不一致，沿用 D1-D4 结论：当前 GitNexus 对 linked worktree 的变更识别是弱证据；D5 实际新增 diff 限定在 image source 边界、unit 测试、demo guide、合同矩阵和本计划记录。

### Task D6: theme token v1

**文件:**
- 修改: `tinyui/include/tinyui/theme.h`
- 修改: `tinyui/src/theme/theme.c`
- 修改: `tinyui/src/backend/ldgui/backend_theme.c`
- 修改: `tinyui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`

- [x] **Step 1: GitNexus impact**

覆盖 theme apply 和 style apply 入口。

2026-05-29 证据：主线程已完成 impact，`tinyui_theme_apply_to_widget`（`tinyui/src/theme/theme.c`）和 `tinyui_backend_widget_apply_style`（`tinyui/src/backend/ldgui/backend_style_apply.c`）均返回 Target/Symbol not found，风险 UNKNOWN。D6 subagent 额外确认 `tinyui_theme_map_widget_colors`、`tinyui_backend_apply_button_style`、`tinyui_backend_apply_checkbox_style`、`tinyui_backend_apply_switch_style`、`tinyui_backend_apply_slider_style` 也返回 Target not found，风险 UNKNOWN；本轮以源码 review、targeted unit 和 gate 结果作为主要证据。

- [x] **Step 2: 写 theme token 测试**

覆盖：

- 文本色。
- 背景色。
- 边框色。
- 强调色。
- 禁用色。
- 默认间距和控件高度。
- 不支持 part/state 的拒绝语义。

2026-05-29 RED 证据：`ctest --test-dir build -R test_tinyui_theme --output-on-failure` 失败。失败点先为 `widget->radius == radius`，暴露 theme apply 未同步 metric token；重新构建后继续暴露 button backend release/press 字段期望与真实 token 映射不一致，修正为当前 state bg/accent 同步语义。

- [x] **Step 3: 实现 token v1**

只做当前控件稳定 token，不做 CSS-like 样式系统。

2026-05-29 证据：`tinyui_theme_apply_to_widget` 在已支持控件上应用当前 theme 的 `padding/radius/control_height`，并继续拒绝 image；颜色 token 保持 `text/bg/panel/border/accent/disabled` 映射。`backend_style_apply.c` 补齐当前 state 的真实 LingDongGUI 字段同步：button release/press、switch knob/track、checkbox、slider、label/text/window 均由单测直接读取真实 backend 字段验证。未引入 CSS-like 样式系统、资源系统或 demo 视觉补丁。

- [x] **Step 4: 验证 D6**

```bash
ctest --test-dir build -R test_tinyui_theme --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo theme_showcase
ctest --test-dir build -L tinyui --output-on-failure
git diff --check
```

期望: 全部通过。

2026-05-29 验证证据：
- `ctest --test-dir build -R test_tinyui_theme --output-on-failure`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo theme_showcase`：通过。
- `ctest --test-dir build -L tinyui --output-on-failure`：通过。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `git diff --check`：通过。
- D6 spec review：通过。确认 color token、metric token、part/state reject、image reject、真实 backend 字段同步和证据层边界符合 D6。
- D6 code quality review：通过。残余风险为 metrics apply 当前非 rollback 原子顺序但现实失败路径弱；`PICOUI_METRIC_BORDER_WIDTH` 仅存储/校验，backend 同步 deferred；window padding theme apply 会覆盖显式 padding，未来若需要优先级需另定义。
- `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`：返回 `No changes detected`、`risk_level: none`。这与 linked worktree 中 D6 diff 不一致，沿用 D1-D5 结论：当前 GitNexus 对 linked worktree 的变更识别是弱证据；D6 实际新增 diff 限定在 theme token/style apply unit、theme/style 实现和本计划记录。

### Task D7: D线 closeout review

**文件:**
- 修改: `docs/tinyui-serial/D-线计划索引.md`
- 可选修改: `tinyui/docs/demo_guide.md`

- [x] **Step 1: 独立 review**

派独立 review subagent 做只读 review，重点检查：

- 是否还存在 runtime present 线性 layout 覆盖。
- 当前控件矩阵是否和 public header / tests 一致。
- 是否误把 smoke/mapping/visible/manual artifact 混写。
- F 线写面是否被 D 线污染。

2026-05-29 证据：D7 closeout review 首轮 FAIL，指出 runtime render 仍对无 source image 注入 placeholder mask，和 D5 source 清空合同冲突；同一 D5 subagent 修复后复审通过。复审确认：smoke cursor layout 只做位置排布，不再伪造 image source；未发现 F 线新控件、demo 硬编码补视觉或证据层混写。

- [x] **Step 2: 最终验证**

```bash
git status --short
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
git diff --check
```

期望: 全部通过。

2026-05-29 证据：
- `git status --short`：显示 D1-D7 累计 dirty diff；`AGENTS.md` / `CLAUDE.md` 仅为 GitNexus 索引统计数字变化，不作为 D 线功能证据。
- `rtk cmake -S . -B build -DUSE_DEMO=0`：通过；仍有既有 `ARM_SECTION(x)=` function-style preprocessor warning。
- `ctest --test-dir build -L tinyui --output-on-failure`：11/11 通过。
- `ctest --test-dir build -L visible --output-on-failure`：1/1 通过。
- `ctest --test-dir build -L mapping --output-on-failure`：1/1 通过。
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all`：通过。
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`：通过。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`：通过。
- `git diff --check`：通过。

- [x] **Step 3: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: affected processes 与 D 线范围一致。

2026-05-29 证据：`gitnexus_detect_changes(scope="all", repo="LingDongGUI")` 返回 `No changes detected`、`risk_level: none`。这与 linked worktree 中 D1-D7 累计 diff 不一致，沿用 D1-D6 结论：当前 GitNexus 对 linked worktree/static backend 写面变更识别是弱证据，不能作为 D 线 closeout 的权威影响证据。D7 closeout 权威证据为独立 review、源码 diff、contract/visible/mapping/runtime/unit gate 和 `git diff --check`。
