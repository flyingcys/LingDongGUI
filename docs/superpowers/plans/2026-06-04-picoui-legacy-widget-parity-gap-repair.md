# PicoUI Legacy Widget Parity Gap Repair Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `picoui/demo/legacy_widget_parity` 从“第一版大页面骨架”推进到更接近老 SDL `uiWidgetLegacy.c` 的控件集合与图片资源绑定基线。

**Architecture:** 继续以 `examples/common/demo/widget/uiWidgetLegacy.c` 为唯一 truth-source，优先复用现有 PicoUI public API，把缺失控件样本、已有图片 source 通路、list item widget 与 child-window 小结构逐项接回 parity 页面。每一步都先增强 contract，再补 demo，避免再次停在“看起来差不多”的弱结论。

**Tech Stack:** C, CMake, PicoUI public API, SDL parity demo, Python contract checks

---

## File Structure

- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`
- Optional Modify: `picoui/docs/demo_guide.md`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`
- Reference: `picoui/demo/basic_widgets/main.c`
- Reference: `picoui/demo/radial_menu_basic/main.c`
- Reference: `picoui/demo/icon_slider_basic/main.c`
- Reference: `picoui/demo/gauge_basic/main.c`
- Reference: `picoui/demo/line_edit_basic/main.c`
- Reference: `picoui/demo/keyboard_basic/main.c`
- Reference: `picoui/demo/arc_basic/main.c`

---

### Task 1: 先把 legacy parity contract 提升到“页面级缺口”口径

**Files:**
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`
- Reference: `picoui/demo/legacy_widget_parity/main.c`

- [x] **Step 1: 扩大 legacy parity marker 集合，先让测试表达真实缺口**

把 `legacy_widget_parity` 的 marker 从当前 5 项扩到至少覆盖：

```python
(
    "picoui_image_create",
    "picoui_progress_bar_create",
    "picoui_list_create",
    "picoui_combo_box_create",
    "picoui_calendar_create",
    "picoui_scroll_selecter_create",
    "picoui_date_time_create",
    "picoui_message_box_create",
    "picoui_graph_create",
    "picoui_table_create",
    "picoui_radial_menu_create",
    "picoui_icon_slider_create",
    "picoui_qrcode_create",
    "picoui_gauge_create",
    "picoui_line_edit_create",
    "picoui_keyboard_create",
    "picoui_arc_create",
    "picoui_list_set_item_widget",
    "picoui_window_create_child",
)
```

- [x] **Step 2: 运行 contract，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: FAIL，提示 `legacy_widget_parity` 缺少新增 marker。

**Step 3: 阶段性提交说明**

本轮主线程未按阶段拆分 commit。若后续需要保留“contract 先红再补绿”的历史，可单独补做：

```bash
git add tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "test: raise legacy parity contract bar"
```

### Task 2: 把老页面缺失控件样本接回 `legacy_widget_parity`

**Files:**
- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`
- Reference: `picoui/demo/radial_menu_basic/main.c`
- Reference: `picoui/demo/icon_slider_basic/main.c`
- Reference: `picoui/demo/gauge_basic/main.c`

- [x] **Step 1: 先接 `radial_menu / icon_slider / qrcode / gauge`**

在 `make_ui()` 中新增对象与基础布局：

1. `picoui_radial_menu_create`
2. `picoui_icon_slider_create`
3. `picoui_qrcode_create`
4. `picoui_gauge_create`

要求：

1. 位置仍沿用老页面的大致区域。
2. 文案与 item id 先尽量贴近老 demo。
3. 不要先发明 helper 抽象，先把页面补齐。

- [x] **Step 2: 运行 contract，确认第一批新 marker 变绿**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: 与 `radial_menu / icon_slider / qrcode / gauge` 相关 marker PASS，其它新增 marker 可能继续 FAIL。

- [x] **Step 3: 再接 `line_edit / keyboard / arc / list item widget / child-window+inner-button`**

要求：

1. `line_edit` 绑定 `keyboard`。
2. `list` 至少一项使用 `picoui_list_set_item_widget(...)`。
3. 用 `picoui_window_create_child(...)` 补老页面里局部 container + 内部 button 的结构。
4. `arc` 进入页面，不要求首轮就带真实图片资源。

- [x] **Step 4: 再跑 contract，确认新增控件集合 marker 全绿**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: `legacy_widget_parity` 新增控件构造类 marker 全 PASS。

- [x] **Step 5: 构建单目标**

Run: `cmake --build build --target picoui_legacy_widget_parity_demo`
Expected: PASS

**Step 6: 阶段性提交说明**

本轮主线程未按 Task 2 单独提交。若后续需要保留“控件集合补齐”阶段，可补做：

```bash
git add picoui/demo/legacy_widget_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "feat: widen legacy widget parity coverage"
```

### Task 3: 把老页面已有图片资源通路接进 parity 页面

**Files:**
- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`
- Reference: `picoui/demo/basic_widgets/main.c`

- [x] **Step 1: 先写图片绑定 marker**

在 `check_picoui_demo_boundary.py` 继续增加至少这些 marker：

```python
(
    "picoui_image_set_source",
    "picoui_button_set_image",
    "picoui_progress_bar_set_image",
    "picoui_slider_set_image",
    "picoui_text_set_background_source",
)
```

- [x] **Step 2: 运行 contract，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: FAIL，提示当前 parity 页面还没做图片 source 绑定。

- [x] **Step 3: 在 demo 里补第一批图片绑定**

优先补：

1. image -> 老页面纸张图
2. button -> release/press 图
3. progress bar -> bg/fg 图
4. text -> background 图
5. slider -> background/indicator 图

要求：

1. 只用 `picoui_*` public API。
2. 如果某张资源在 PicoUI source 结构里还需要桥接，先在 demo 内按现有 `image_source` 用法接，不绕后门。

- [x] **Step 4: 再跑 contract**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: 图片绑定 marker PASS。

- [x] **Step 5: 构建单目标**

Run: `cmake --build build --target picoui_legacy_widget_parity_demo`
Expected: PASS

**Step 6: 阶段性提交说明**

本轮主线程未按 Task 3 单独提交。若后续需要保留“第一批图片 source 绑定”阶段，可补做：

```bash
git add picoui/demo/legacy_widget_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "feat: bind legacy parity image sources"
```

### Task 4: 把图片型控件样本也接到真实 source

**Files:**
- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`
- Reference: `picoui/demo/radial_menu_basic/main.c`
- Reference: `picoui/demo/icon_slider_basic/main.c`
- Reference: `picoui/demo/gauge_basic/main.c`
- Reference: `picoui/demo/arc_basic/main.c`

- [x] **Step 1: 给 `radial_menu / icon_slider` 切到带 source 的 item API**

从纯文本 item 改到：

1. `picoui_radial_menu_add_item_with_source(...)`
2. `picoui_icon_slider_add_item_with_source(...)`

目标是与老页面那批 `weather / note / book / chart` 图标关系对齐。

- [x] **Step 2: 给 `gauge / arc` 绑定真实 source**

补：

1. `picoui_gauge_set_bg_source(...)`
2. `picoui_gauge_set_pointer_source(...)`
3. `picoui_arc_set_quarter_source(...)`

- [x] **Step 3: 运行 contract 与构建**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: PASS

Run: `cmake --build build --target picoui_legacy_widget_parity_demo`
Expected: PASS

**Step 4: 阶段性提交说明**

本轮主线程未按 Task 4 单独提交。若后续需要保留“图片型控件 source 补齐”阶段，可补做：

```bash
git add picoui/demo/legacy_widget_parity/main.c
git commit -m "feat: align legacy widget media-backed controls"
```

### Task 5: 收口文档与整体验证

**Files:**
- Optional Modify: `picoui/docs/demo_guide.md`
- Reference: `docs/superpowers/reviews/2026-06-04-picoui-legacy-widget-parity-gap-audit.md`

- [x] **Step 1: 更新 demo guide 口径**

把 `legacy_widget_parity` 的说明从“快速看控件集合是否齐”更新成当前真实状态：

1. 哪些老页面控件已进入 parity 页面；
2. 哪些图片资源绑定已接；
3. 哪些仍是剩余 gap。

补充：

1. 将 `docs/superpowers/reviews/2026-06-04-picoui-legacy-widget-parity-gap-audit.md` 的过时状态同步为当前真实状态；
2. 明确当前 source 绑定是“public API 已接通”，不是“老资源逐张精确一致”。

- [x] **Step 2: 运行完整验证**

Run: `cmake -S . -B build`
Expected: PASS

Run: `cmake --build build --target picoui_legacy_widget_parity_demo picoui_layout_parity_demo picoui_grid_parity_demo`
Expected: PASS

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
Expected: PASS

Run: `git diff --check`
Expected: no output

**Step 3: 提交收口说明**

本轮已完成代码、文档与验证收口，但未在当前运行里执行 git commit。若后续需要形成单次收口提交，可执行：

```bash
git add picoui/docs/demo_guide.md picoui/demo/legacy_widget_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "docs: sync legacy widget parity status"
```

## Self-Review

- spec coverage：本计划覆盖了控件集合补齐、图片 source 绑定、contract 提升、文档同步四个真实缺口。
- placeholder scan：无 `TBD/TODO/implement later`。
- type consistency：所有步骤都以当前仓库已有 `picoui_*` public API 为前提，没有引入未定义新接口名。
