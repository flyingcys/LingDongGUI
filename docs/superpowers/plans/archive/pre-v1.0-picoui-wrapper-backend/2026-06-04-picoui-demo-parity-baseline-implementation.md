# PicoUI Demo Parity Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增 `legacy-widget`、`layout`、`grid` 三个 PicoUI parity baseline demo，使其页面结构更接近老 SDL demo，便于双边截图对比定位 PicoUI 真缺口。

**Architecture:** 以老 demo 源码为真相源，在 `picoui/demo` 下新增三个独立入口，全部只使用 `picoui_*` public API。实现顺序先锁定 target 注册与 demo boundary，再逐页落 `legacy_widget_parity`、`layout_parity`、`grid_parity`，最后做编译与 contract 验证。

**Tech Stack:** C, CMake, PicoUI public API, SDL demo targets, Python contract checks

---

## File Structure

- Create: `picoui/demo/legacy_widget_parity/main.c`
- Create: `picoui/demo/layout_parity/main.c`
- Create: `picoui/demo/grid_parity/main.c`
- Optional Create: `picoui/demo/common/` 下少量纯 PicoUI helper
- Modify: `examples/sdl/CMakeLists.txt`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`
- Optional Modify: `picoui/docs/demo_guide.md`

---

### Task 1: 先把新 demo target 注册面锁住

**Files:**
- Modify: `examples/sdl/CMakeLists.txt`
- Test: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 写失败断言，要求新 target 名字出现在 SDL CMake**

在 `tests/picoui/contract/check_picoui_demo_boundary.py` 新增源码级断言，检查以下 target 字符串存在：

```python
required_targets = (
    "picoui_legacy_widget_parity_demo",
    "picoui_layout_parity_demo",
    "picoui_grid_parity_demo",
)
```

- [ ] **Step 2: 运行测试，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: FAIL，提示缺少 parity demo target 或对应 source path。

- [ ] **Step 3: 在 `examples/sdl/CMakeLists.txt` 注册 3 个新 target**

按现有 `add_picoui_demo(...)` 模式新增：

```cmake
add_picoui_demo(picoui_legacy_widget_parity_demo
    "${SDL_EXAMPLE_DIR}/../../picoui/demo/legacy_widget_parity/main.c"
)
add_picoui_demo(picoui_layout_parity_demo
    "${SDL_EXAMPLE_DIR}/../../picoui/demo/layout_parity/main.c"
)
add_picoui_demo(picoui_grid_parity_demo
    "${SDL_EXAMPLE_DIR}/../../picoui/demo/grid_parity/main.c"
)
```

- [ ] **Step 4: 再跑 contract，确认 target 注册面转绿**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: 与 target 注册相关的断言 PASS；后续可能还因 source 文件不存在继续 FAIL。

- [ ] **Step 5: Commit**

```bash
git add examples/sdl/CMakeLists.txt tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "test: lock picoui parity demo targets"
```

### Task 2: 建立最小 demo skeleton，并保持 boundary 纯净

**Files:**
- Create: `picoui/demo/legacy_widget_parity/main.c`
- Create: `picoui/demo/layout_parity/main.c`
- Create: `picoui/demo/grid_parity/main.c`
- Test: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 为 3 个 demo 写最小入口骨架**

每个文件先用与现有 demo 一致的最小模板：

```c
#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    (void)win;
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }

    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
```

- [ ] **Step 2: 跑 demo boundary 检查，确认不存在 `ld*` 泄漏**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: 新增文件不会触发 `ld*` / `arm_2d_*` / `SIGNAL_*` 泄漏告警。

- [ ] **Step 3: 构建新 target，确认 skeleton 可编译**

Run: `rtk cmake -S . -B build`  
Run: `rtk cmake --build build --target picoui_legacy_widget_parity_demo picoui_layout_parity_demo picoui_grid_parity_demo`

Expected: 3 个 target 全部编译通过。

- [ ] **Step 4: Commit**

```bash
git add picoui/demo/legacy_widget_parity/main.c picoui/demo/layout_parity/main.c picoui/demo/grid_parity/main.c
git commit -m "feat: add picoui parity demo skeletons"
```

### Task 3: 先落 `legacy_widget_parity` 页面结构

**Files:**
- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Optional Create: `picoui/demo/common/*`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`

- [ ] **Step 1: 先写失败断言，要求 demo 中出现关键 widget 构造痕迹**

在 `tests/picoui/contract/check_picoui_demo_boundary.py` 或同层新 contract 中增加源码级字符串断言，至少覆盖：

```python
required_markers = (
    "picoui_switch_create",
    "picoui_checkbox_create",
    "picoui_slider_create",
    "picoui_button_create",
    "picoui_text_create",
)
```

- [ ] **Step 2: 运行测试，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: FAIL，提示 `legacy_widget_parity` 仍是空骨架。

- [ ] **Step 3: 按老页面真相源落第一版多控件大页面**

在 `picoui/demo/legacy_widget_parity/main.c` 用 `picoui_*` 组装：

- 顶层 root/container
- image
- button
- label
- checkbox/radio 样本
- switch + ON/OFF 状态标签
- progress bar
- text
- slider

并尽量保持与老页面接近的主区域关系。

- [ ] **Step 4: 再跑 contract，确认关键控件已进入 demo**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: `legacy_widget_parity` 的关键 widget 断言 PASS。

- [ ] **Step 5: 构建单目标验证**

Run: `rtk cmake --build build --target picoui_legacy_widget_parity_demo`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add picoui/demo/legacy_widget_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "feat: add legacy widget parity baseline"
```

### Task 4: 落 `layout_parity` 页面结构

**Files:**
- Modify: `picoui/demo/layout_parity/main.c`
- Optional Create/Modify: `picoui/demo/common/*`
- Reference: `examples/common/demo/layout/uiLayout.c`

- [ ] **Step 1: 先写失败断言，要求 layout parity 出现页面标题与多组 layout 容器**

加入源码级断言，至少检查：

```python
required_markers = (
    "picoui_window_set_padding_group",
    "picoui_flex_set_direction",
    "picoui_text_set_text",
)
```

- [ ] **Step 2: 运行测试，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: FAIL，提示 `layout_parity` 尚未构造对应 layout。

- [ ] **Step 3: 按老页面落 layout parity**

在 `picoui/demo/layout_parity/main.c` 组织：

- 页面标题/提示
- legacy row 样本
- legacy column 样本
- flex row 样本
- flex column 样本

重点保持卡片数量、层次、流向、gap、padding 关系。

- [ ] **Step 4: 再跑 contract**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: `layout_parity` 关键 layout 痕迹 PASS。

- [ ] **Step 5: 构建单目标验证**

Run: `rtk cmake --build build --target picoui_layout_parity_demo`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add picoui/demo/layout_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "feat: add layout parity baseline"
```

### Task 5: 落 `grid_parity` 页面结构

**Files:**
- Modify: `picoui/demo/grid_parity/main.c`
- Optional Create/Modify: `picoui/demo/common/*`
- Reference: `examples/common/demo/layout/uiLayout.c`

- [ ] **Step 1: 先写失败断言，要求 grid parity 出现 grid columns/rows/cell/span 痕迹**

加入源码级断言，至少检查：

```python
required_markers = (
    "picoui_grid_set_columns",
    "picoui_grid_set_rows",
    "picoui_widget_set_grid_cell",
)
```

- [ ] **Step 2: 运行测试，确认先红**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: FAIL，提示 `grid_parity` 还是空骨架。

- [ ] **Step 3: 按老页面落 grid parity**

在 `picoui/demo/grid_parity/main.c` 组织：

- 页面标题/提示
- grid canvas
- A-G 面板
- overlay 面板

并显式写出主要 col/row span 与 align 关系。

- [ ] **Step 4: 再跑 contract**

Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`  
Expected: `grid_parity` 关键 grid 痕迹 PASS。

- [ ] **Step 5: 构建单目标验证**

Run: `rtk cmake --build build --target picoui_grid_parity_demo`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add picoui/demo/grid_parity/main.c tests/picoui/contract/check_picoui_demo_boundary.py
git commit -m "feat: add grid parity baseline"
```

### Task 6: 收口文档与整体验证

**Files:**
- Optional Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 更新 demo guide，登记 3 个 parity demo**

把新 demo 名称、目标、用途写进 `picoui/docs/demo_guide.md`，说明它们是截图对比用 `parity baseline`，不是普通 showcase demo。

- [ ] **Step 2: 运行完整构建与 contract 校验**

Run: `rtk cmake -S . -B build`  
Run: `rtk cmake --build build --target picoui_legacy_widget_parity_demo picoui_layout_parity_demo picoui_grid_parity_demo`  
Run: `python3 tests/picoui/contract/check_picoui_demo_boundary.py`

Expected:

- 构建 PASS
- demo boundary PASS
- 新 target 注册断言 PASS

- [ ] **Step 3: 做 diff hygiene 检查**

Run: `git diff --check`

Expected: no output

- [ ] **Step 4: Commit**

```bash
git add picoui/docs/demo_guide.md
git commit -m "docs: register picoui parity baseline demos"
```

## Self-Review

- spec coverage：覆盖了第一批 `legacy-widget/layout/grid` 三页、独立 target、boundary 纯净、最小 contract、构建验证。
- placeholder scan：无 `TBD/TODO/implement later`。
- type consistency：target 命名、目录命名、命令路径在全文保持一致。
