# TINYUI a-0.6 全量发布缺口总账 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 TINYUI 从当前 `22/26 wrapped`、`4/26 full_parity_complete` 的中间态，严格串行收口到 `26/26` 全控件覆盖、全控件 `full_parity_complete`、并具备最终发布 truth-source / gate / docs 的版本。

**Architecture:** `a-0.6` 是全量发布总线，不是“最后 4 控件扩面线”。执行固定为 `W1 -> W2 -> W3 -> W4`，分别对应 `not_wrapped` 覆盖缺口、`stable contract` 整改、`minimal vertical slice` 升级、最终发布证据层升级。每个 `W*` 再拆成不重叠的 subagent task 包；主线程负责阶段边界、GitNexus impact、matrix/docs/gate 收敛和最终验证。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-0.6`
- 建议分支：`feat/tinyui-a-0-6-full-release`
- 创建或切换后必须执行：

```bash
git worktree add .worktree/a-0.6 -b feat/tinyui-a-0-6-full-release HEAD
cd .worktree/a-0.6
git submodule sync --recursive
git submodule update --init --recursive
```

- 严格串行：`W1 -> W2 -> W3 -> W4`
- 阶段内默认只允许一个写 subagent
- 每个 task 使用 fresh subagent 执行
- 每个 task 结束后必须做独立只读 review；review 不通过时，由原执行 subagent 修复
- 涉及 C / Python 符号修改前必须先跑 GitNexus impact
- 每个 task 结束前至少运行 `git diff --check`
- 每个 task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### a-0.6 shared-owner 文件

以下文件默认只允许主线程在阶段切换时统一收口，subagent 不得重叠写：

- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/core/event.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_widget.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

### a-0.6 聚合入口文件

以下文件只允许在每个 task 末尾最小接入一次：

- `tinyui/include/tinyui/tinyui.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/docs/demo_guide.md`
- `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- `docs/tinyui-serial/a-0.4-线计划索引.md`
- `docs/tinyui-serial/a-0.5-线计划索引.md`
- `docs/superpowers/specs/2026-06-01-tinyui-a-0-6-full-release-design.md`

## 1. 文件结构与阶段边界

### W1-A `arc / gauge`

**Create:**
- `tinyui/include/tinyui/arc.h`
- `tinyui/include/tinyui/gauge.h`
- `tinyui/src/widgets/arc.c`
- `tinyui/src/widgets/gauge.c`
- `tinyui/src/backend/ldgui/backend_arc.c`
- `tinyui/src/backend/ldgui/backend_gauge.c`
- `tinyui/demo/arc_basic/main.c`
- `tinyui/demo/gauge_basic/main.c`
- `tests/tinyui/unit/test_tinyui_arc.c`
- `tests/tinyui/unit/test_tinyui_gauge.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

### W1-B `icon_slider / radial_menu`

**Create:**
- `tinyui/include/tinyui/icon_slider.h`
- `tinyui/include/tinyui/radial_menu.h`
- `tinyui/src/widgets/icon_slider.c`
- `tinyui/src/widgets/radial_menu.c`
- `tinyui/src/backend/ldgui/backend_icon_slider.c`
- `tinyui/src/backend/ldgui/backend_radial_menu.c`
- `tinyui/demo/icon_slider_basic/main.c`
- `tinyui/demo/radial_menu_basic/main.c`
- `tests/tinyui/unit/test_tinyui_icon_slider.c`
- `tests/tinyui/unit/test_tinyui_radial_menu.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

### W2-A `checkbox / switch / text / image / list`

**Modify:**
- `tinyui/src/widgets/image.c`
- `tinyui/src/widgets/text.c`
- `tinyui/src/widgets/checkbox.c`
- `tinyui/src/widgets/switch.c`
- `tinyui/src/widgets/list.c`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_image.c`
- `tinyui/src/backend/ldgui/backend_text.c`
- `tinyui/src/backend/ldgui/backend_checkbox.c`
- `tinyui/src/backend/ldgui/backend_switch.c`
- `tinyui/src/backend/ldgui/backend_list.c`
- `tests/tinyui/unit/test_tinyui_image.c`
- `tests/tinyui/unit/test_tinyui_text.c`
- `tests/tinyui/unit/test_tinyui_checkbox.c`
- `tests/tinyui/unit/test_tinyui_switch.c`
- `tests/tinyui/unit/test_tinyui_list.c`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tinyui/docs/demo_guide.md`

### W2-B `line_edit / keyboard / combo_box / scroll_selecter`

**Modify:**
- `tinyui/src/widgets/line_edit.c`
- `tinyui/src/widgets/keyboard.c`
- `tinyui/src/widgets/combo_box.c`
- `tinyui/src/widgets/scroll_selecter.c`
- `tinyui/src/core/internal.h`
- `tinyui/src/core/event.c`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_line_edit.c`
- `tinyui/src/backend/ldgui/backend_keyboard.c`
- `tinyui/src/backend/ldgui/backend_combo_box.c`
- `tinyui/src/backend/ldgui/backend_scroll_selecter.c`
- `tests/tinyui/unit/test_tinyui_line_edit.c`
- `tests/tinyui/unit/test_tinyui_keyboard.c`
- `tests/tinyui/unit/test_tinyui_combo_box.c`
- `tests/tinyui/unit/test_tinyui_scroll_selecter.c`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tinyui/docs/demo_guide.md`

### W2-C `graph / table / calendar`

**Modify:**
- `tinyui/src/widgets/graph.c`
- `tinyui/src/widgets/table.c`
- `tinyui/src/widgets/calendar.c`
- `tinyui/src/core/internal.h`
- `tinyui/src/core/event.c`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_graph.c`
- `tinyui/src/backend/ldgui/backend_table.c`
- `tinyui/src/backend/ldgui/backend_calendar.c`
- `tests/tinyui/unit/test_tinyui_graph.c`
- `tests/tinyui/unit/test_tinyui_table.c`
- `tests/tinyui/unit/test_tinyui_calendar.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tinyui/docs/demo_guide.md`

### W3-A `progress_bar / qrcode / progress_wheel`

**Modify:**
- `tinyui/src/widgets/progress_bar.c`
- `tinyui/src/widgets/qrcode.c`
- `tinyui/src/widgets/progress_wheel.c`
- `tinyui/src/backend/ldgui/backend_progress_bar.c`
- `tinyui/src/backend/ldgui/backend_qrcode.c`
- `tinyui/src/backend/ldgui/backend_progress_wheel.c`
- `tests/tinyui/unit/test_tinyui_progress_bar.c`
- `tests/tinyui/unit/test_tinyui_qrcode.c`
- `tests/tinyui/unit/test_tinyui_progress_wheel.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tinyui/docs/demo_guide.md`

### W3-B `message_box / date_time / clock`

**Modify:**
- `tinyui/src/widgets/message_box.c`
- `tinyui/src/widgets/date_time.c`
- `tinyui/src/widgets/clock.c`
- `tinyui/src/backend/ldgui/backend_message_box.c`
- `tinyui/src/backend/ldgui/backend_date_time.c`
- `tinyui/src/backend/ldgui/backend_clock.c`
- `tests/tinyui/unit/test_tinyui_message_box.c`
- `tests/tinyui/unit/test_tinyui_date_time.c`
- `tests/tinyui/unit/test_tinyui_clock.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tinyui/docs/demo_guide.md`

### W4-A final release matrix + full gate catalog

**Modify:**
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/docs/demo_guide.md`

### W4-B manual artifact + 中文发布文档集

**Modify:**
- `docs/tinyui-serial/C-线人工窗口验收记录.md`
- `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- `docs/tinyui-serial/a-0.4-线计划索引.md`
- `docs/tinyui-serial/a-0.5-线计划索引.md`
- `docs/superpowers/reviews/2026-06-01-tinyui-a-0-3-a-0-4-a-0-5-deep-review.md`
- `docs/superpowers/specs/2026-06-01-tinyui-a-0-6-full-release-design.md`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`

## 2. Tasks

### Task W1-A: `arc / gauge`

**Files:**
- Create: `tinyui/include/tinyui/arc.h`
- Create: `tinyui/include/tinyui/gauge.h`
- Create: `tinyui/src/widgets/arc.c`
- Create: `tinyui/src/widgets/gauge.c`
- Create: `tinyui/src/backend/ldgui/backend_arc.c`
- Create: `tinyui/src/backend/ldgui/backend_gauge.c`
- Create: `tinyui/demo/arc_basic/main.c`
- Create: `tinyui/demo/gauge_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_arc.c`
- Create: `tests/tinyui/unit/test_tinyui_gauge.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/contract/check_tinyui_public_api.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldArc", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldGauge", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit 与 contract**

至少新增：

```c
static void test_arc_value_and_angle_readback_match_backend_truth(void);
static void test_gauge_value_and_pointer_contract_match_backend_truth(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_arc|test_tinyui_gauge' --output-on-failure
```

- [ ] **Step 4: 实现最小真实映射**

Requirements:

- `arc/gauge` 必须进入 public widget 集
- value/range/angle/pointer 不能退化成 host cache 假读回
- visible 不允许只证明“看起来像圆形仪表”

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_arc|test_tinyui_gauge' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo arc_basic
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo gauge_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W1-B: `icon_slider / radial_menu`

**Files:**
- Create: `tinyui/include/tinyui/icon_slider.h`
- Create: `tinyui/include/tinyui/radial_menu.h`
- Create: `tinyui/src/widgets/icon_slider.c`
- Create: `tinyui/src/widgets/radial_menu.c`
- Create: `tinyui/src/backend/ldgui/backend_icon_slider.c`
- Create: `tinyui/src/backend/ldgui/backend_radial_menu.c`
- Create: `tinyui/demo/icon_slider_basic/main.c`
- Create: `tinyui/demo/radial_menu_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_icon_slider.c`
- Create: `tests/tinyui/unit/test_tinyui_radial_menu.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/contract/check_tinyui_public_api.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldIconSlider", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldRadialMenu", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first unit 与 contract**

至少新增：

```c
static void test_icon_slider_selection_and_value_follow_backend_truth(void);
static void test_radial_menu_navigation_and_selection_follow_backend_truth(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_icon_slider|test_tinyui_radial_menu' --output-on-failure
```

- [ ] **Step 4: 实现最小真实映射**

Requirements:

- 不允许只做表层 icon / radial visual
- selection / navigation / callback 必须走真实 backend 语义
- visible 不能替代 contract 验证

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_icon_slider|test_tinyui_radial_menu' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo icon_slider_basic
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo radial_menu_basic
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W2-A: `checkbox / switch / text / image / list`

**Files:**
- Modify: `tinyui/src/widgets/image.c`
- Modify: `tinyui/src/widgets/text.c`
- Modify: `tinyui/src/widgets/checkbox.c`
- Modify: `tinyui/src/widgets/switch.c`
- Modify: `tinyui/src/widgets/list.c`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_image.c`
- Modify: `tinyui/src/backend/ldgui/backend_text.c`
- Modify: `tinyui/src/backend/ldgui/backend_checkbox.c`
- Modify: `tinyui/src/backend/ldgui/backend_switch.c`
- Modify: `tinyui/src/backend/ldgui/backend_list.c`
- Modify: `tests/tinyui/unit/test_tinyui_image.c`
- Modify: `tests/tinyui/unit/test_tinyui_text.c`
- Modify: `tests/tinyui/unit/test_tinyui_checkbox.c`
- Modify: `tests/tinyui/unit/test_tinyui_switch.c`
- Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_image_set_source", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_text_set_font", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_checkbox_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_switch_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_list_set_selected_index", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first parity gap 测试**

至少新增：

```c
static void test_image_rejects_or_supports_final_release_metadata_contract_explicitly(void);
static void test_text_public_readback_matches_documented_contract(void);
static void test_checkbox_and_switch_expose_final_release_state_contract(void);
static void test_list_metadata_and_selection_contract_are_release_ready(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_image|test_tinyui_text|test_tinyui_checkbox|test_tinyui_switch|test_tinyui_list' --output-on-failure
```

- [ ] **Step 4: 实现最小 parity 收口**

Requirements:

- 每个控件都要么补齐缺口，要么把 reject-with-rationale 固定为最终发布合同
- 不允许保留含糊的 metadata-only / incomplete_contract 终态
- 不允许靠文档降格逃避代码缺口

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_image|test_tinyui_text|test_tinyui_checkbox|test_tinyui_switch|test_tinyui_list' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W2-B: `line_edit / keyboard / combo_box / scroll_selecter`

**Files:**
- Modify: `tinyui/src/widgets/line_edit.c`
- Modify: `tinyui/src/widgets/keyboard.c`
- Modify: `tinyui/src/widgets/combo_box.c`
- Modify: `tinyui/src/widgets/scroll_selecter.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_line_edit.c`
- Modify: `tinyui/src/backend/ldgui/backend_keyboard.c`
- Modify: `tinyui/src/backend/ldgui/backend_combo_box.c`
- Modify: `tinyui/src/backend/ldgui/backend_scroll_selecter.c`
- Modify: `tests/tinyui/unit/test_tinyui_line_edit.c`
- Modify: `tests/tinyui/unit/test_tinyui_keyboard.c`
- Modify: `tests/tinyui/unit/test_tinyui_combo_box.c`
- Modify: `tests/tinyui/unit/test_tinyui_scroll_selecter.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_line_edit_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_keyboard_create", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_combo_box_set_selected_index", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_scroll_selecter_set_selected_index", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first parity gap 测试**

至少新增：

```c
static void test_line_edit_submit_cancel_reason_contract_is_release_ready(void);
static void test_keyboard_has_explicit_final_gate_coverage_contract(void);
static void test_combo_box_final_visual_and_selection_contract_is_release_ready(void);
static void test_scroll_selecter_final_visual_and_edit_contract_is_release_ready(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter' --output-on-failure
```

- [ ] **Step 4: 实现最小 parity 收口**

Requirements:

- `keyboard` 不能继续停在 runtime-first 但 matrix 写更强的状态
- `line_edit` 需要收口 submit/cancel reason 或明确最终 reject-with-rationale
- `combo_box/scroll_selecter` 必须把 visual/theme gap 处理到最终发布口径

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_line_edit|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_scroll_selecter' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W2-C: `graph / table / calendar`

**Files:**
- Modify: `tinyui/src/widgets/graph.c`
- Modify: `tinyui/src/widgets/table.c`
- Modify: `tinyui/src/widgets/calendar.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/event.c`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_graph.c`
- Modify: `tinyui/src/backend/ldgui/backend_table.c`
- Modify: `tinyui/src/backend/ldgui/backend_calendar.c`
- Modify: `tests/tinyui/unit/test_tinyui_graph.c`
- Modify: `tests/tinyui/unit/test_tinyui_table.c`
- Modify: `tests/tinyui/unit/test_tinyui_calendar.c`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_graph_add_series", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_table_set_cell_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_calendar_set_date", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first parity gap 测试**

至少新增：

```c
static void test_graph_final_release_contract_covers_advanced_readback_boundary(void);
static void test_table_final_release_contract_covers_non_commit_exit_boundary(void);
static void test_calendar_final_release_contract_covers_full_feature_boundary(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_graph|test_tinyui_table|test_tinyui_calendar' --output-on-failure
```

- [ ] **Step 4: 实现最小 parity 收口**

Requirements:

- `graph/table/calendar` 不允许停在“shared-core 阶段已经落地”口径
- `table` 必须处理 commit boundary 之外的最终发布边界
- visible / mapping / unit / contract 必须对齐同一强度

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_graph|test_tinyui_table|test_tinyui_calendar' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W3-A: `progress_bar / qrcode / progress_wheel`

**Files:**
- Modify: `tinyui/src/widgets/progress_bar.c`
- Modify: `tinyui/src/widgets/qrcode.c`
- Modify: `tinyui/src/widgets/progress_wheel.c`
- Modify: `tinyui/src/backend/ldgui/backend_progress_bar.c`
- Modify: `tinyui/src/backend/ldgui/backend_qrcode.c`
- Modify: `tinyui/src/backend/ldgui/backend_progress_wheel.c`
- Modify: `tests/tinyui/unit/test_tinyui_progress_bar.c`
- Modify: `tests/tinyui/unit/test_tinyui_qrcode.c`
- Modify: `tests/tinyui/unit/test_tinyui_progress_wheel.c`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_progress_bar_set_percent", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_qrcode_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_progress_wheel_set_percent", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first release-upgrade 测试**

至少新增：

```c
static void test_progress_bar_release_contract_covers_theme_and_config_boundary(void);
static void test_qrcode_release_contract_covers_configuration_boundary(void);
static void test_progress_wheel_release_contract_covers_animation_and_style_boundary(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_progress_bar|test_tinyui_qrcode|test_tinyui_progress_wheel' --output-on-failure
```

- [ ] **Step 4: 实现最小 release 升级**

Requirements:

- 不能继续停在 narrow API vertical slice
- 要么补齐配置/readback/theme 面，要么把最终 reject-with-rationale 固定清楚
- 不能只改 matrix 文案

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_progress_bar|test_tinyui_qrcode|test_tinyui_progress_wheel' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W3-B: `message_box / date_time / clock`

**Files:**
- Modify: `tinyui/src/widgets/message_box.c`
- Modify: `tinyui/src/widgets/date_time.c`
- Modify: `tinyui/src/widgets/clock.c`
- Modify: `tinyui/src/backend/ldgui/backend_message_box.c`
- Modify: `tinyui/src/backend/ldgui/backend_date_time.c`
- Modify: `tinyui/src/backend/ldgui/backend_clock.c`
- Modify: `tests/tinyui/unit/test_tinyui_message_box.c`
- Modify: `tests/tinyui/unit/test_tinyui_date_time.c`
- Modify: `tests/tinyui/unit/test_tinyui_clock.c`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="tinyui_message_box_set_message", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_date_time_set_format", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_clock_set_step_second", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first release-upgrade 测试**

至少新增：

```c
static void test_message_box_release_contract_covers_multi_action_and_readback_boundary(void);
static void test_date_time_release_contract_covers_public_readback_and_modes(void);
static void test_clock_release_contract_covers_time_source_and_configuration_boundary(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R 'test_tinyui_message_box|test_tinyui_date_time|test_tinyui_clock' --output-on-failure
```

- [ ] **Step 4: 实现最小 release 升级**

Requirements:

- `message_box` 不能继续停在单 confirm + host cache
- `date_time` 不能继续停在 format-only readback
- `clock` 不能继续停在 step-second-only public contract

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_tinyui_message_box|test_tinyui_date_time|test_tinyui_clock' --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W4-A: final release matrix + full gate catalog

**Files:**
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_public_api.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- Modify: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="main", file_path="tests/tinyui/contract/check_tinyui_release_capability_matrix.py", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="main", file_path="tests/tinyui/runtime/check_tinyui_backend_mapping.py", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="main", file_path="tests/tinyui/runtime/check_tinyui_visible_ui.py", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first final release 校验**

至少新增检查：

1. final schema 不再是 `a-0.3-current-15-layered-v1`
2. `26` 控件全部进入 final release matrix
3. runtime / mapping / visible / manual artifact 覆盖 catalog 对齐同一控件集
4. `keyboard` 等特殊证据层按最终口径显式表达

- [ ] **Step 3: 验证 RED**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_runtime.py
```

- [ ] **Step 4: 实现 final truth-source / gate catalog**

Requirements:

- 不允许继续混用 current-truth 与 final-release judgement
- 每层 gate 必须对自己的覆盖边界诚实
- final release matrix 必须服务发布，而不是仅服务盘点

- [ ] **Step 5: 验证 GREEN**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
```

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task W4-B: manual artifact + 中文发布文档集

**Files:**
- Modify: `docs/tinyui-serial/C-线人工窗口验收记录.md`
- Modify: `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- Modify: `docs/tinyui-serial/a-0.4-线计划索引.md`
- Modify: `docs/tinyui-serial/a-0.5-线计划索引.md`
- Modify: `docs/superpowers/reviews/2026-06-01-tinyui-a-0-3-a-0-4-a-0-5-deep-review.md`
- Modify: `docs/superpowers/specs/2026-06-01-tinyui-a-0-6-full-release-design.md`
- Modify: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 回写 final 发布文档集**

必须同步：

1. `a-0.6` 当时 final release truth-source 入口
2. `26` 控件完成态总表
3. manual artifact 入口
4. 发布说明 / closeout / review 术语

- [ ] **Step 2: 跑 full verify**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
git status --short --branch --ignore-submodules=all
```

- [ ] **Step 3: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

## 3. 阶段验收门禁

### W1 通过门禁

1. `4` 个 `not_wrapped` 全部进入 public widget 集。
2. 新增 demo / unit / mapping / visible 证据全部落地。
3. release matrix 中不再有这 `4` 个控件的 `not_wrapped` 终态。

### W2 通过门禁

1. `12` 个 `stable contract` 控件的真实 parity 缺口全部被补齐或被最终 reject-with-rationale 固定。
2. 不再保留含糊的 `deferred` / `incomplete_contract` 作为发布阻塞项。
3. `keyboard` 等特殊证据层有诚实且稳定的最终口径。

### W3 通过门禁

1. `6` 个 `minimal vertical slice` 控件全部从“窄合同竖切片”升级到可发布合同。
2. 不再只依赖最小 getter / 单场景 demo。
3. 所有升级后的能力都能在 unit / mapping / visible / docs 中对齐。

### W4 通过门禁

1. final release matrix 不再是 current-15 schema。
2. 全量 gate catalog 与 `26` 控件对齐。
3. manual artifact 与中文发布文档完成收口。
4. 用户可以从单一入口回答“最终发布还差什么”。

## 4. 执行顺序建议

推荐严格顺序：

1. `W1-A`
2. `W1-B`
3. `W2-A`
4. `W2-B`
5. `W2-C`
6. `W3-A`
7. `W3-B`
8. `W4-A`
9. `W4-B`

原因：

1. 先补覆盖缺口，再谈 full parity。
2. 先收口 shared-heavy `stable` 组，再升级 `minimal` 组。
3. final truth-source / gate / docs 只能在代码态稳定后收口。

## 5. 最终完成标准

只有以下全部满足，`a-0.6` 才算完成：

1. `26/26` 全控件进入 TINYUI public widget 集。
2. `26/26` 全控件进入最终 `full_parity_complete`。
3. final release matrix、gate catalog、manual artifact、中文发布文档全部闭环。
4. `gitnexus_detect_changes()` 在最终收口时只显示预期影响面。
5. 全量验证命令通过：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L tinyui --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_runtime.py
git diff --check
```
