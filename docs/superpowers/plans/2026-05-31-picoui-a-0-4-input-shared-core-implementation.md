# TINYUI a-0.4 输入 shared-core Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-0.4` 中严格串行完成 TINYUI 输入 shared-core 收口，先解决 runtime/layout honesty、getter truth policy、focus/edit/navigation/dropdown 合同，再接入 `line_edit / keyboard / combo_box / scroll_selecter` 四个高耦合输入控件。

**Architecture:** `a-0.4` 不是四控件平推线，而是 shared-core 先行线。阶段固定为 `R0 -> R1 -> R2 -> R3 -> R4 -> R5 -> R6 -> R7`。`R0 / R1 / R4 / R7` 是 shared-owner 阶段，必须主线程先做 GitNexus impact 再派单一写 subagent；`R2 / R3 / R5 / R6` 在前置 shared-core 冻结后按控件串行接入。每阶段结束都做独立 review；review 不通过时，由同一执行 subagent 修复。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-0.4`
- 建议分支：`feat/tinyui-a-0-4-input-shared-core`
- 创建或切换后必须执行：

```bash
git worktree add .worktree/a-0.4 -b feat/tinyui-a-0-4-input-shared-core HEAD
cd .worktree/a-0.4
git submodule sync --recursive
git submodule update --init --recursive
```

- `a-0.4` 严格串行：`R0 -> R1 -> R2 -> R3 -> R4 -> R5 -> R6 -> R7`
- 每个 Task 使用 fresh subagent 执行
- 每个 Task 做独立只读 review；review 不通过时，回原执行 subagent 修复
- 涉及 C / Python 符号修改前必须运行 GitNexus impact
- 每个 Task 结束前至少运行 `git diff --check`
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### a-0.4 独占 shared-owner 文件

以下文件默认只允许 `a-0.4` shared-core 阶段持续修改：

- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tinyui/src/backend/ldgui/backend_app.c`
- `tinyui/src/backend/ldgui/backend_layout.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

### a-0.4 聚合文件

以下文件只允许在每个阶段末尾做一次最小接入：

- `tinyui/include/tinyui/tinyui.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/docs/demo_guide.md`
- `docs/tinyui-serial/a-0.4-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-tinyui-a-0-4-input-shared-core-design.md`

## 1. 文件结构与阶段边界

### R0 honesty + readback policy

**Modify:**
- `tinyui/src/backend/ldgui/backend_app.c`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `docs/superpowers/reviews/2026-05-31-tinyui-a-0-3-closeout-review.md`

### R1 focus ownership

**Modify:**
- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/unit/test_tinyui_widgets.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`

### R2 editable text contract + `line_edit`

**Create:**
- `tinyui/include/tinyui/line_edit.h`
- `tinyui/src/widgets/line_edit.c`
- `tinyui/src/backend/ldgui/backend_line_edit.c`
- `tinyui/demo/line_edit_basic/main.c`
- `tests/tinyui/unit/test_tinyui_line_edit.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R3 keyboard bridge + `keyboard`

**Create:**
- `tinyui/include/tinyui/keyboard.h`
- `tinyui/src/widgets/keyboard.c`
- `tinyui/src/backend/ldgui/backend_keyboard.c`
- `tinyui/demo/keyboard_basic/main.c`
- `tests/tinyui/unit/test_tinyui_keyboard.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`

### R4 selection / navigation contract

**Modify:**
- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/widgets/list.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_list.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/unit/test_tinyui_list.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

### R5 dropdown contract + `combo_box`

**Create:**
- `tinyui/include/tinyui/combo_box.h`
- `tinyui/src/widgets/combo_box.c`
- `tinyui/src/backend/ldgui/backend_combo_box.c`
- `tinyui/demo/combo_box_basic/main.c`
- `tests/tinyui/unit/test_tinyui_combo_box.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R6 `scroll_selecter`

**Create:**
- `tinyui/include/tinyui/scroll_selecter.h`
- `tinyui/src/widgets/scroll_selecter.c`
- `tinyui/src/backend/ldgui/backend_scroll_selecter.c`
- `tinyui/demo/scroll_selecter_basic/main.c`
- `tests/tinyui/unit/test_tinyui_scroll_selecter.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`

### R7 文档与 closeout

**Modify:**
- `docs/tinyui-serial/a-0.4-线计划索引.md`
- `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- `docs/superpowers/specs/2026-05-31-tinyui-a-0-4-input-shared-core-design.md`
- `tinyui/docs/demo_guide.md`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

## 2. Tasks

### Task R0: honesty + readback policy baseline

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_widget.c`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_backend_app_run", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_native_signal", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_list_get_selected_index", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first honesty / readback 测试**

至少新增：

```c
static void test_runtime_does_not_apply_generic_smoke_layout_for_non_layout_root(void);
static void test_list_selected_index_readback_matches_backend_truth_after_native_signal(void);
```

断言目标：

- runtime 不再为无 layout root 通用补位
- `list` 在 native selection 变化后，public getter 与 backend truth 一致

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_(list|widgets)' --output-on-failure
```

Expected:

- 新增 honesty/readback 测试失败

- [ ] **Step 4: 做最小实现**

Requirements:

- 移除或严格隔离 `temporary smoke path`
- 冻结 getter truth policy
- 若某 getter 当前只能是 host cache，必须同步 matrix / 文档标注

- [ ] **Step 5: 拆 gate**

Requirements:

- `visible_ui` 只证明 widget exists / visible
- 新增或强化 runtime / unit 校验，证明值经过 frame/event 后仍一致

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L tinyui --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R1: focus ownership

**Files:**
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tests/tinyui/unit/test_tinyui_widgets.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_backend_widget_dispatch_event", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_signal", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first focus 测试**

至少新增：

```c
static void test_focus_owner_switches_between_widgets(void);
static void test_hidden_or_disabled_widget_cannot_keep_focus(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_widgets$' --output-on-failure
```

- [ ] **Step 4: 实现最小 focus ownership**

Requirements:

- 统一 focus owner 存储位置
- 统一 focus enter/leave 流程
- hidden / disabled 与 focus 的关系必须明确

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

### Task R2: editable text contract + `line_edit`

**Files:**
- Create: `tinyui/include/tinyui/line_edit.h`
- Create: `tinyui/src/widgets/line_edit.c`
- Create: `tinyui/src/backend/ldgui/backend_line_edit.c`
- Create: `tinyui/demo/line_edit_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_line_edit.c`
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
gitnexus_impact(target="ldLineEditSetText", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldLineEditSetKeyboard", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldLineEditGetText", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_line_edit_create_with_props_sets_text_and_type(void);
static void test_line_edit_readback_matches_backend_after_edit_commit(void);
static void test_line_edit_rejects_invalid_keyboard_binding(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_line_edit$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `line_edit`**

Requirements:

- 真实映射 `ldLineEdit`
- 文本、编辑类型、keyboard 绑定走真实 backend
- getter 规则必须遵守 `R0` 的 truth policy

- [ ] **Step 5: 接入 contract / visible / mapping**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo line_edit_basic
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_line_edit$' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo line_edit_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R3: keyboard bridge + `keyboard`

**Files:**
- Create: `tinyui/include/tinyui/keyboard.h`
- Create: `tinyui/src/widgets/keyboard.c`
- Create: `tinyui/src/backend/ldgui/backend_keyboard.c`
- Create: `tinyui/demo/keyboard_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_keyboard.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldKeyboardInputAscii", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldKeyboardNavigate", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldKeyboardClick", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_keyboard_dispatches_ascii_into_focused_line_edit(void);
static void test_keyboard_navigation_signal_respects_focus_owner(void);
static void test_keyboard_exit_clears_focus_or_edit_session(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_keyboard$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `keyboard` bridge**

Requirements:

- 真实映射 `ldKeyboard`
- 输入必须发到当前 focus/edit target
- 退出/导航/点击信号必须统一走 shared-core

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_keyboard$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R4: selection / navigation contract

**Files:**
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/widgets/list.c`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_list.c`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_list_get_selected_index", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_list_set_selected_index", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_native_signal", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first selection 测试**

至少覆盖：

```c
static void test_list_selection_readback_survives_native_navigation(void);
static void test_selection_contract_rejects_out_of_range_focus_move(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_list$' --output-on-failure
```

- [ ] **Step 4: 实现 shared selection / navigation**

Requirements:

- `list` readback 改成 backend truth 或明确同步真值
- 统一 selection move / confirm / cancel 规则
- 为 `combo_box / scroll_selecter` 预留复用接口

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_list$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R5: dropdown contract + `combo_box`

**Files:**
- Create: `tinyui/include/tinyui/combo_box.h`
- Create: `tinyui/src/widgets/combo_box.c`
- Create: `tinyui/src/backend/ldgui/backend_combo_box.c`
- Create: `tinyui/demo/combo_box_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_combo_box.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldComboBoxSetSelectItem", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldComboBoxSetStaticItems", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldComboBoxGetSelectItem", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_combo_box_open_close_and_selected_item_truth(void);
static void test_combo_box_reuses_selection_contract(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_combo_box$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `combo_box`**

Requirements:

- 真实映射 `ldComboBox`
- selected item truth 遵守 `R0/R4`
- dropdown open/close 不能靠 demo 补位

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_combo_box$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo combo_box_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R6: `scroll_selecter`

**Files:**
- Create: `tinyui/include/tinyui/scroll_selecter.h`
- Create: `tinyui/src/widgets/scroll_selecter.c`
- Create: `tinyui/src/backend/ldgui/backend_scroll_selecter.c`
- Create: `tinyui/demo/scroll_selecter_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_scroll_selecter.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldScrollSelecterSetSelectItemNum", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldScrollSelecterGetSelectItemNum", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldScrollSelecterSetEditMode", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit**

至少覆盖：

```c
static void test_scroll_selecter_selected_item_matches_backend_truth(void);
static void test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_tinyui_scroll_selecter$' --output-on-failure
```

- [ ] **Step 4: 实现最小 `scroll_selecter`**

Requirements:

- 真实映射 `ldScrollSelecter`
- 选中项 truth/readback 遵守 `R0/R4`
- edit mode 与 navigation mode 边界清楚

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_tinyui_scroll_selecter$' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo scroll_selecter_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R7: 文档与 closeout

**Files:**
- Modify: `docs/tinyui-serial/a-0.4-线计划索引.md`
- Modify: `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- Modify: `docs/superpowers/specs/2026-05-31-tinyui-a-0-4-input-shared-core-design.md`
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 回写 serial 文档**

同步：

- `0.4` 新增四控件
- shared-core 完成状态
- honesty / truth policy 约束

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

Plan complete and saved to `docs/superpowers/plans/2026-05-31-tinyui-a-0-4-input-shared-core-implementation.md`. Two execution options:

1. Subagent-Driven (recommended) - I dispatch a fresh subagent per task, review between tasks, fast iteration
2. Inline Execution - Execute tasks in this session using executing-plans, batch execution with checkpoints
