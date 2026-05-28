# PicoUI B线可见 UI 收口实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `PicoUI` 的 demo 验收从“smoke 可出图”推进到“自动 visible gate 已证明 dummy SDL + PPM readback 下可显示、可读、可判定”，先完成 `picoui_basic_widgets_demo` 的 visible baseline，再推广到其他 demos。

**Architecture:** `B线` 不再继续优先扩新的 backend 能力面，而是围绕 automatic visible correctness 建立一条新主线：先建立 dummy SDL + PPM readback 证据链，再纠正 host present/readback 显示链与颜色链，再收口 `basic_widgets`，最后铺开到其他 demos，并把 automatic visible gate 固化进文档和测试。人工 OS 窗口验收不属于 `B线` 完成结论，必须等 `C6 / manual window artifact gate`。

**Tech Stack:** C11、CMake、LingDongGUI、SDL2 host runtime、Python3 验证脚本、CTest、GitNexus、PicoUI demo targets

---

## 0. 文件结构与责任分组（按 subagent 写面）

### G1 文档与验收口径锚点

**Files:**
- Modify: `docs/picoui-serial/B-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- Modify: `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `picoui/docs/demo_guide.md`

职责：

- 固定 `B0/B5` 的证据口径
- 把 smoke gate、visible gate、closeout 条件写成唯一真相源
- 在 `G2-G6` 给出稳定 evidence 前，不提前写死实现细节

### G2 visible evidence 组

**Files:**
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Create or Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`

职责：

- 定义 smoke 与 visible gate 的边界
- 为 `basic_widgets` 建立 automatic visible evidence 入口

### G3 host present/readback 显示链组

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_app.c`

职责：

- 查清并纠正 host present 显示链与 capture/readback 链的偏差
- `backend_app.c` 只允许这一组 subagent 改动，避免与其他组冲突

### G4 theme/颜色/可读性链组

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `picoui/src/backend/ldgui/backend_theme.c`
- Modify: `picoui/src/theme/theme.c`
- Modify: `tests/picoui/unit/test_picoui_theme.c`

职责：

- 查清并纠正 theme token、backend style apply 与最终可读性的偏差

### G5 `basic_widgets` 首个可见样板组

**Files:**
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `picoui/src/backend/ldgui/backend_layout.c`
- Modify: `picoui/src/backend/ldgui/backend_widget.c`
- Modify: `picoui/src/backend/ldgui/backend_image.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_layout.c`

职责：

- 收口 `basic_widgets` 的 visible correctness

### G6 multi-demo 推广组

**Files:**
- Modify: `picoui/demo/hello_world/main.c`
- Modify: `picoui/demo/layout_flex/main.c`
- Modify: `picoui/demo/layout_grid/main.c`
- Modify: `picoui/demo/theme_showcase/main.c`
- Modify: `picoui/demo/settings_panel/main.c`
- Modify: `tests/picoui/runtime/*`

职责：

- 把 `basic_widgets` visible baseline 推广到其他 demos

---

## 1. 串并边界

### 1.1 必须串行的阶段

- `Task 1` 必须先于 `Task 2`
- `Task 2` 必须先于 `Task 3`
- `Task 3` 必须先于 `Task 4`
- `Task 4` 必须先于 `Task 5`

原因：

- `Task 1` 定义 visible evidence
- `Task 2` 才能基于该 evidence 修显示链
- `Task 3` 才能收口 `basic_widgets`
- `Task 4` 才能安全铺向其他 demos
- `Task 5` 才能写 closeout

### 1.2 可由 subagent 并行推进的点

- `B0` 冻结后，可并行启动：
  - `G2` visible evidence
  - `G3` host present/readback
  - `G4` theme/颜色/可读性链
- `G5` 必须晚于 `G2`，并建议晚于 `G3/G4` 的第一轮 evidence
- `G6` 只能在 `G5` 形成稳定样板后再展开
- 同一时刻：
  - `backend_app.c` 只能有一个 subagent 改
  - `tests/picoui/runtime/*` 只能有一个 subagent 改
  - `theme.c/backend_style_apply.c/backend_theme.c` 归 `G4` 独占
  - demo 文件可按文件并行，但一旦触及共享 backend，立即收回主线程重拆

---

## 2. 任务拆分

### Task 1: 建立 automatic visible evidence 基线（对应 `G2`，`G1` 可并行回写文档边界）

**Files:**
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Create: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`

- [ ] **Step 1: 写失败检查，明确 `capture 非空` 不等于 `visible correctness`**

要求：

- 为 `picoui_basic_widgets_demo` 增加一条独立 visible 检查入口
- 先让它失败在“重复列/可读性异常/窗口显示与 smoke 不一致”这类现象上

- [ ] **Step 2: 运行失败检查，确认当前现状被正确捕获**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- FAIL，且失败原因不是“没有像素”，而是 visible correctness 不成立

- [ ] **Step 3: 在测试架构文档中补 visible gate 分层**

要求：

- 写清：
  - smoke gate
  - backend mapping gate
  - automatic visible gate
  - manual window artifact gate
  - backend correctness gate

- [ ] **Step 4: 复跑检查，确认脚本与文档同步**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- 仍 FAIL，但失败语义已稳定、可作为后续根因修复目标

- [ ] **Step 5: Commit**

```bash
git add tests/picoui/runtime/check_picoui_runtime.py \
        tests/picoui/runtime/check_picoui_visible_ui.py \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md
git commit -m "test(picoui): add visible ui gate baseline"
```

### Task 2: 修正窗口显示链与颜色链（拆成 `G3 + G4` 两个独立写面）

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `picoui/src/backend/ldgui/backend_theme.c`
- Modify: `picoui/src/theme/theme.c`
- Test: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Test: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 先定位显示链与 capture/readback 链的差异点**

检查点：

- `backend_app.c` 的 host present 路径
- `PICOUI_CAPTURE_FILE` 读回路径
- 当前像素格式/颜色语义转换

- [ ] **Step 1.1: 拆分 subagent 写面**

要求：

- `G3` 只允许改 `backend_app.c`
- `G4` 只允许改 `theme.c / backend_style_apply.c / backend_theme.c / test_picoui_theme.c`
- 两组并行，但不能互相越界

- [ ] **Step 2: 写最小失败检查锁住颜色链/显示链偏差**

Run:

```bash
ctest --test-dir build -R test_picoui_theme --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- 至少一条 FAIL，证明当前显示/颜色链仍不可信

- [ ] **Step 3: 实装最小修复，纠正主输出链与颜色链语义**

限制：

- 不得回退到 fake renderer 主输出
- 不得用 demo 侧硬编码掩盖显示问题

- [ ] **Step 4: 复跑主题和 visible 检查**

Run:

```bash
ctest --test-dir build -R test_picoui_theme --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- 主题单测 PASS
- visible 检查至少从“近黑/错色”类失败中退出

- [ ] **Step 5: Commit**

```bash
git add picoui/src/backend/ldgui/backend_app.c \
        picoui/src/backend/ldgui/backend_style_apply.c \
        picoui/src/theme/theme.c \
        tests/picoui/runtime/check_picoui_visible_ui.py \
        tests/picoui/unit/test_picoui_theme.c
git commit -m "fix(picoui): correct visible display path"
```

### Task 3: 收口 `basic_widgets` visible baseline（对应 `G5`）

**Files:**
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `picoui/src/backend/ldgui/backend_layout.c`
- Modify: `picoui/src/backend/ldgui/backend_widget.c`
- Modify: `picoui/src/backend/ldgui/backend_image.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_layout.c`

- [ ] **Step 1: 锁定 `basic_widgets` 当前不正确的 visible 结构**

要求：

- 把重复列/重复控件/布局错位写成可回归检查

- [ ] **Step 2: 运行失败检查，确认当前确实不满足 visible baseline**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- FAIL，原因指向结构/布局/可读性问题

- [ ] **Step 2.1: 进入条件检查**

进入本任务前必须满足：

- `G2` 已把 visible gate 定义稳定
- `G3/G4` 至少给出第一轮 root-cause evidence

- [ ] **Step 3: 做最小实现修复**

目标：

- 让 `switch/checkbox/slider/button/text/image` 的显示结构与预期一致
- 明确 `image` 的当前显示合同

- [ ] **Step 4: 复跑 `basic_widgets` 相关检查**

Run:

```bash
ctest --test-dir build -R test_picoui_widgets --output-on-failure
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

Expected:

- `test_picoui_widgets` PASS
- runtime smoke PASS
- `basic_widgets` visible gate PASS

- [ ] **Step 5: Commit**

```bash
git add picoui/demo/basic_widgets/main.c \
        tests/picoui/runtime/check_picoui_runtime.py \
        tests/picoui/runtime/check_picoui_visible_ui.py \
        tests/picoui/unit/test_picoui_widgets.c
git commit -m "fix(picoui): close basic widgets visible ui"
```

### Task 4: 铺开到其他 `picoui` demos（对应 `G6`）

**Files:**
- Modify: `picoui/demo/hello_world/main.c`
- Modify: `picoui/demo/layout_flex/main.c`
- Modify: `picoui/demo/layout_grid/main.c`
- Modify: `picoui/demo/theme_showcase/main.c`
- Modify: `picoui/demo/settings_panel/main.c`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`

- [ ] **Step 1: 为 5 个其余 demos 建立逐个 visible 检查**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo hello_world
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_flex
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo layout_grid
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo theme_showcase
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo settings_panel
```

Expected:

- 当前至少部分 FAIL，说明尚未铺开

- [ ] **Step 2: 按 demo 最小修复 visible correctness**

顺序：

1. `hello_world`
2. `layout_flex`
3. `layout_grid`
4. `theme_showcase`
5. `settings_panel`

- [ ] **Step 2.1: demo 并行分派规则**

要求：

- 每个 demo 最多一个 subagent
- demo subagent 只改本 demo 文件
- 若发现需要改共享 backend，暂停并收回主线程重拆

- [ ] **Step 3: 复跑所有 PicoUI visible 检查**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_runtime.py
ctest --test-dir build -L picoui --output-on-failure
```

Expected:

- visible gate PASS
- runtime smoke PASS
- `ctest -L picoui` PASS

- [ ] **Step 4: Commit**

```bash
git add picoui/demo/hello_world/main.c \
        picoui/demo/layout_flex/main.c \
        picoui/demo/layout_grid/main.c \
        picoui/demo/theme_showcase/main.c \
        picoui/demo/settings_panel/main.c \
        tests/picoui/runtime/check_picoui_visible_ui.py
git commit -m "fix(picoui): extend visible ui baseline"
```

### Task 5: 文档回写与 B线 closeout（由 `G1` 收口）

**Files:**
- Modify: `docs/picoui-serial/B-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- Modify: `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `picoui/docs/demo_guide.md`

- [x] **Step 1: 回写 visible gate 结果与最终边界**

要求：

- 写清哪些门禁只证明 smoke
- 写清哪些门禁证明 visible correctness

- [x] **Step 2: 更新 demo guide，避免继续误导用户把 smoke 当 visible**

- [x] **Step 3: 复核 B线索引的当前游标、退出口径和下一步入口**

- [x] **Step 4: 运行最终收口验证**

Run:

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_runtime.py
ctest --test-dir build -L picoui --output-on-failure
```

Expected:

- 全部 PASS

实际收口 gate：

- `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all`：PASS
- `python3 tests/picoui/runtime/check_picoui_backend_mapping.py`：PASS
- `python3 tests/picoui/runtime/check_picoui_runtime.py`：PASS
- `ctest --test-dir build -L picoui --output-on-failure`：PASS

- [ ] **Step 5: Commit**

```bash
git add docs/picoui-serial/B-线计划索引.md \
        docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md \
        docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        picoui/docs/demo_guide.md
git commit -m "docs(picoui): close b-line visible ui plan"
```

---

## 3. 自检

### 3.1 Spec coverage

- `B线` 的核心目标“自动 visible gate 已证明 dummy SDL + PPM readback 下可显示、可读、可判定”已覆盖到：
  - visible evidence
  - 显示/颜色链
  - `basic_widgets` baseline
  - multi-demo 推广
  - 文档 closeout
- 人工窗口验收未包含在 `B线` closeout 中；需要 `C6 / manual window artifact gate` 后才能声称通过。

### 3.2 Placeholder scan

- 没有保留占位项
- 每个任务都列出了具体文件与命令

### 3.3 串并边界

- 已明确必须串行的阶段顺序
- 已明确适合 subagent 推进的非重叠写面

---

## 4. 执行建议

推荐执行方式：

1. 主线程先做 `Task 1` 的 evidence 口径冻结
2. 再按 `Task 2 -> Task 3 -> Task 4 -> Task 5` 串行推进
3. 每个任务内部再拆成非重叠 subagent 写面
