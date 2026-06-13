# TINYUI F线新控件垂直切片实施计划

> **给 agentic workers:** REQUIRED SUB-SKILL: 使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用 checkbox（`- [ ]`）格式跟踪。

**目标:** 新增 `tinyui_list` 的最小完整 vertical slice，同时不抢占 D 线 shared backend/layout/event/theme 写面。当前主线代码已完成该 vertical slice；本计划保留为执行记录和后续复核依据。

**架构:** F 线在 `.worktree/tinyui-f-new-widgets` 串行推进。先做 list public contract，再做 widget/backend/demo/gate，最后只读 review 和主线程合并。`line_edit` 只保留为后续候选，不在本计划实现。

**技术栈:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 创建 worktree：`.worktree/tinyui-f-new-widgets`。
- 创建或切换后必须同步并更新 submodule。
- F 线内部严格串行：`F0 -> F1 -> F2 -> F3 -> F4 -> F5`。
- F 线实现期不修改 D 线 shared files：`backend_app.c`、`backend_layout.c`、`backend_event.c`、`backend_style_apply.c`、`backend_theme.c`。当前主线合并态已在 `backend_app.c` 接入 list marker 分类，用于解除 F3 旧 blocker。
- 修改符号前必须运行 GitNexus impact。
- 每个 Task 后运行 `gitnexus_detect_changes(scope="all")`。
- 每个 Task 后做独立 review；review 不通过时回原 subagent 修复。

## 1. 文件结构与写面

### F list 独立写面

- 新增: `tinyui/include/tinyui/list.h`
- 新增: `tinyui/src/widgets/list.c`
- 新增: `tinyui/src/backend/ldgui/backend_list.c`
- 新增: `tinyui/demo/list_basic/main.c`
- 新增: `tests/tinyui/unit/test_tinyui_list.c`

### F 必要集成写面

- 修改: `tinyui/include/tinyui/tinyui.h`
- 修改: `tinyui/src/backend/ldgui/backend.h`
- 修改: `tinyui/CMakeLists.txt` 或现有 TINYUI 源文件列表位置
- 修改: `examples/sdl/CMakeLists.txt`
- 修改: `tests/tinyui/CMakeLists.txt`
- 修改: `tests/tinyui/runtime/check_tinyui_runtime.py`
- 修改: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 修改: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- 修改: `tinyui/docs/demo_guide.md`
- 修改: `docs/tinyui-serial/F-线计划索引.md`

这些集成写面可能与 D 线合并冲突，必须由主线程在合并阶段串行整合。

## 2. Tasks

### Task F0: worktree 准备和 baseline

**文件:**
- Read: `docs/tinyui-serial/F-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-29-tinyui-f-line-new-widget-vertical-slice-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/tinyui-f-new-widgets HEAD
cd .worktree/tinyui-f-new-widgets
git submodule sync --recursive
git submodule update --init --recursive
```

- [ ] **Step 2: baseline**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

期望: 全部通过。否则停止并汇报 F0 blocker。

### Task F1: list public contract 和 unit RED

**文件:**
- 新增: `tinyui/include/tinyui/list.h`
- 新增: `tests/tinyui/unit/test_tinyui_list.c`
- 修改: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: 写 public header**

创建 `tinyui/include/tinyui/list.h`，包含设计文档中的最小 API。不得出现 `ld*`、`arm_2d_*`、`SIGNAL_*`。

- [ ] **Step 2: 写 unit RED 测试**

创建 `test_tinyui_list.c`，覆盖：

- create / create_with_props
- add item
- selected index 初值和 setter/getter
- callback user_data 保存
- NULL parent / NULL text 的拒绝语义

运行：

```bash
ctest --test-dir build -R test_tinyui_list --output-on-failure
```

期望: 当前应因实现缺失失败。

### Task F2: list widget 和 backend mapping

**文件:**
- 新增: `tinyui/src/widgets/list.c`
- 新增: `tinyui/src/backend/ldgui/backend_list.c`
- 修改: `tinyui/src/backend/ldgui/backend.h`
- 修改: TINYUI source list CMake file
- 修改: `tinyui/include/tinyui/tinyui.h`

- [ ] **Step 1: GitNexus impact**

对要接入的 backend factory / widget create 路径运行 impact。若新增符号无历史 impact，至少对相邻模式符号运行：

```text
gitnexus_impact(target="tinyui_button_create", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_create_button", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 实现最小 widget state**

实现：

- list owner widget
- item id/text 存储
- selected index
- callback + user_data
- add item 边界检查

- [ ] **Step 3: 实现 backend list 映射**

使用真实 `LingDongGUI` list 控件。如果底层 `ldList` API 不支持某项语义，必须在代码和 F 线索引中标注 reject/deferred，不得 fake。

- [ ] **Step 4: 验证 F2**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_list --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
git diff --check
```

期望: 全部通过。当前主线状态：已通过。

### Task F3: list demo 和 runtime gate

**文件:**
- 新增: `tinyui/demo/list_basic/main.c`
- 修改: `examples/sdl/CMakeLists.txt`
- 修改: `tests/tinyui/runtime/check_tinyui_runtime.py`
- 修改: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 修改: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 写 demo**

`tinyui/demo/list_basic/main.c` 只能使用 `tinyui_*` API。demo 内容：

- window title：`List`
- list id：`list`
- items：
  - `item_wifi` / `Wi-Fi`
  - `item_bluetooth` / `Bluetooth`
  - `item_display` / `Display`
- selected index 初值：0

- [ ] **Step 2: 注册 target**

在 `examples/sdl/CMakeLists.txt` 增加 `tinyui_list_basic_demo`。

- [ ] **Step 3: 更新 runtime/mapping/visible matrix**

要求：

- runtime smoke 包含 `tinyui_list_basic_demo`。
- backend mapping 当前要求 `PICOUI_BACKEND_REAL_WIDGET_IDS=list,item_wifi,item_bluetooth,item_display`，但应把 `item_*` 理解为 list item marker，不是独立 backend widget id。
- visible gate 对 list demo 做非黑、bounds、结构可读检查。

- [ ] **Step 4: 验证 F3**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target tinyui_list_basic_demo
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
git diff --check
```

期望: 全部通过。

### Task F4: docs 和 F线索引收口

**文件:**
- 修改: `tinyui/docs/demo_guide.md`
- 修改: `docs/tinyui-serial/F-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-f-line-new-widget-vertical-slice-design.md` 仅当 implementation exposes a documented reject/deferred item

- [ ] **Step 1: 更新 demo guide**

写清：

- `tinyui_list_basic_demo` 的用途。
- 它证明哪些 gate。
- 它不证明哪些能力，例如 virtualization、multi-select、keyboard navigation。

- [ ] **Step 2: 更新 F 线索引**

记录：

- F0-F4 状态。
- 当前支持/拒绝/推迟能力。
- 验证命令。
- 与 D 线的写面冲突是否存在。

- [ ] **Step 3: 验证 docs**

```bash
git diff --check
rg -n "tinyui_list|tinyui_list_basic_demo|list_basic|multi-select|virtualization|keyboard navigation" \
  tinyui/docs/demo_guide.md \
  docs/tinyui-serial/F-线计划索引.md \
  docs/superpowers/specs/2026-05-29-tinyui-f-line-new-widget-vertical-slice-design.md
```

期望: 口径完整，无占位内容。

### Task F5: F线 closeout review

**文件:**
- 修改: `docs/tinyui-serial/F-线计划索引.md`

- [ ] **Step 1: 独立 review**

派独立 review subagent，只读检查：

- demo 是否只用 `tinyui_*`。
- public header 是否泄漏底层。
- F 是否修改 D 独占 shared backend 文件。
- runtime/mapping/visible matrix 是否同步。
- list 是否被过度声明。

- [ ] **Step 2: 最终验证**

```bash
git status --short
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_list --output-on-failure
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
git diff --check
```

期望: 全部通过。

- [ ] **Step 3: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望: affected scope 只包含 `tinyui_list`、list demo 和 runtime matrix。

## 3. 当前 closeout 补充

截至当前主线，`F0 -> F5` 已收口：

- `tinyui_list` public API、unit test、真实 `ldList` backend mapping、`list_basic` demo、runtime/mapping/visible matrix 已接入。
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过。
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo list_basic` 通过。
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过。
- `python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 通过。

能力边界：

- 支持：create、create_with_props、add item、selected index setter/getter（TINYUI shadow state）、真实 `ldListSetText`、`ldListSetSelectItem` backend 映射。
- 暂不支持：multi-select、virtualization、drag reorder、keyboard navigation、item remove/reorder、native list selection event bridge。
- `tinyui_list_set_on_selected()` 当前只保存 callback/user_data，不能写成 native selection 事件已闭环；该公开 API 当前仍是 incomplete contract。
- 当前 mapping gate 用 `PICOUI_BACKEND_REAL_WIDGET_IDS` 同时承载 list widget id 和 list item marker；后续应拆分出 list item 专用 marker，避免把 item 数据项误读为独立 backend widget。
