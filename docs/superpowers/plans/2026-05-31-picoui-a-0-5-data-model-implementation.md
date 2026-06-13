# TINYUI a-0.5 数据 / edit model Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-0.5` 中严格串行完成 TINYUI 数据 / edit model shared-core 收口，先完成 item model 与 editable cell contract，再依次接入 `table / graph / calendar` 三个数据类控件。

**Architecture:** `a-0.5` 是 shared-model 先行线，不是三控件扩面线。阶段固定为 `R0 -> R1 -> R2 -> R3 -> R4 -> R5`。`R0 / R1 / R5` 是 shared-owner 阶段，必须主线程先做 GitNexus impact 再派单一写 subagent；`R2 / R3 / R4` 在 shared model 冻结后依次接入 `table / graph / calendar`。`table` 作为第一控件必须先验证 `0.4` 输入 shared-core 是否真实可复用。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-0.5`
- 建议分支：`feat/tinyui-a-0-5-data-model`
- 创建或切换后必须执行：

```bash
git worktree add .worktree/a-0.5 -b feat/tinyui-a-0-5-data-model HEAD
cd .worktree/a-0.5
git submodule sync --recursive
git submodule update --init --recursive
```

- `a-0.5` 严格串行：`R0 -> R1 -> R2 -> R3 -> R4 -> R5`
- 每个 Task 使用 fresh subagent 执行
- 每个 Task 做独立只读 review；review 不通过时，回原执行 subagent 修复
- 涉及 C / Python 符号修改前必须运行 GitNexus impact
- 每个 Task 结束前至少运行 `git diff --check`
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### a-0.5 独占 shared-owner 文件

以下文件默认只允许 `a-0.5` shared model 阶段持续修改：

- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

### a-0.5 聚合文件

以下文件只允许在每个阶段末尾做一次最小接入：

- `tinyui/include/tinyui/tinyui.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/docs/demo_guide.md`
- `docs/tinyui-serial/a-0.5-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-tinyui-a-0-5-data-model-design.md`

## 1. 文件结构与阶段边界

### R0 item model baseline

**Modify:**
- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tests/tinyui/unit/test_tinyui_widgets.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`

### R1 editable cell contract

**Modify:**
- `tinyui/src/core/internal.h`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/unit/test_tinyui_widgets.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

### R2 `table`

**Create:**
- `tinyui/include/tinyui/table.h`
- `tinyui/src/widgets/table.c`
- `tinyui/src/backend/ldgui/backend_table.c`
- `tinyui/demo/table_basic/main.c`
- `tests/tinyui/unit/test_tinyui_table.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R3 graph series / value model + `graph`

**Create:**
- `tinyui/include/tinyui/graph.h`
- `tinyui/src/widgets/graph.c`
- `tinyui/src/backend/ldgui/backend_graph.c`
- `tinyui/demo/graph_basic/main.c`
- `tests/tinyui/unit/test_tinyui_graph.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R4 calendar date / header / grid contract + `calendar`

**Create:**
- `tinyui/include/tinyui/calendar.h`
- `tinyui/src/widgets/calendar.c`
- `tinyui/src/backend/ldgui/backend_calendar.c`
- `tinyui/demo/calendar_basic/main.c`
- `tests/tinyui/unit/test_tinyui_calendar.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R5 文档与 closeout

**Modify:**
- `docs/tinyui-serial/a-0.5-线计划索引.md`
- `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- `docs/superpowers/specs/2026-05-31-tinyui-a-0-5-data-model-design.md`
- `tinyui/docs/demo_guide.md`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

## 2. Tasks

### Task R0: item model baseline

**Files:**
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_widget.c`
- Modify: `tests/tinyui/unit/test_tinyui_widgets.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_backend_widget_update_value", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_native_signal", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first model 基线测试**

至少新增：

```c
static void test_item_model_identity_survives_frame_update(void);
static void test_model_readback_policy_is_explicit_for_data_widgets(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_widgets$' --output-on-failure
```

- [ ] **Step 4: 实现最小 item model baseline**

Requirements:

- 冻结 row/column/item identity
- 冻结 data widget truth/readback policy
- 明确 frame/event 后 model 真值存放位置

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_widgets$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R1: editable cell contract

**Files:**
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tests/tinyui/unit/test_tinyui_widgets.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_backend_widget_dispatch_event", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_signal", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first edit contract 测试**

至少新增：

```c
static void test_editable_cell_commit_and_cancel_paths_are_distinct(void);
static void test_editable_cell_navigation_preserves_model_truth(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_widgets$' --output-on-failure
```

- [ ] **Step 4: 实现最小 editable cell contract**

Requirements:

- 提交/取消必须区分
- 与 `line_edit / keyboard` 桥接必须依赖 `0.4` shared-core
- 不允许 cell edit 使用 demo 私有临时状态

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_widgets$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R2: `table`

**Files:**
- Create: `tinyui/include/tinyui/table.h`
- Create: `tinyui/src/widgets/table.c`
- Create: `tinyui/src/backend/ldgui/backend_table.c`
- Create: `tinyui/demo/table_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_table.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/contract/check_tinyui_public_api.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldTableGetItem", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldTableCurrentRow", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldTableCurrentColumn", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_table_current_cell_matches_backend_truth(void);
static void test_table_edit_commit_updates_model_and_visible_text(void);
static void test_table_reuses_editable_cell_contract(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_table$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `table`**

Requirements:

- 真实映射 `ldTable`
- current row/column/cell 必须有真实 readback
- edit model 必须走 `R1` 合同

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_table$' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo table_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R3: graph series / value model + `graph`

**Files:**
- Create: `tinyui/include/tinyui/graph.h`
- Create: `tinyui/src/widgets/graph.c`
- Create: `tinyui/src/backend/ldgui/backend_graph.c`
- Create: `tinyui/demo/graph_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_graph.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldGraphAddSeries", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldGraphSetValue", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldGraphMoveAdd", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_graph_series_value_readback_survives_frame_update(void);
static void test_graph_visible_output_matches_series_updates(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_graph$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `graph`**

Requirements:

- 真实映射 `ldGraph`
- series/value model 不能退化成 demo 假数据
- visible 与 model truth 必须能联动验证

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_graph$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo graph_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R4: calendar date / header / grid contract + `calendar`

**Files:**
- Create: `tinyui/include/tinyui/calendar.h`
- Create: `tinyui/src/widgets/calendar.c`
- Create: `tinyui/src/backend/ldgui/backend_calendar.c`
- Create: `tinyui/demo/calendar_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_calendar.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldCalendarSetDate", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldCalendarGetDate", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldCalendarSetHeaderFormat", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_calendar_date_readback_matches_backend_truth(void);
static void test_calendar_header_and_grid_visible_output_match_date_contract(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_calendar$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `calendar`**

Requirements:

- 真实映射 `ldCalendar`
- date/header/grid 合同必须可读回
- visible 不能只验证“像日历”

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_calendar$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo calendar_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R5: 文档与 closeout

**Files:**
- Modify: `docs/tinyui-serial/a-0.5-线计划索引.md`
- Modify: `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- Modify: `docs/superpowers/specs/2026-05-31-tinyui-a-0-5-data-model-design.md`
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 回写 serial 文档**

同步：

- `0.5` 新增三控件
- shared model 完成状态
- `table -> graph -> calendar` 串行依据

- [ ] **Step 2: 跑 full verify**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L tinyui --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
```

- [ ] **Step 3: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

## 3. 执行交接

Plan complete and saved to `docs/superpowers/plans/2026-05-31-tinyui-a-0-5-data-model-implementation.md`. Two execution options:

1. Subagent-Driven (recommended) - I dispatch a fresh subagent per task, review between tasks, fast iteration
2. Inline Execution - Execute tasks in this session using executing-plans, batch execution with checkpoints
