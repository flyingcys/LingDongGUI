# TINYUI G线当前控件能力缺口收口实施计划

> **给 agentic workers:** REQUIRED SUB-SKILL: 使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用 checkbox（`- [ ]`）格式跟踪。

**目标:** 把 TINYUI 当前已完成控件的能力缺口、过强口径和高优先级补齐顺序收口成可执行主线，避免把真实 backend 子集误写成 100% 能力封装。

**架构:** G 线在 `.worktree/tinyui-g-current-capability-gap` 串行推进。先冻结 capability gap 真相源，再按 `list contract -> marker 语义 -> slider range -> shared base -> widget-specific` 顺序收口。每个 Task 由 fresh subagent 执行，Task 后由独立 review subagent 只读 review。

**技术栈:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 创建 worktree：`.worktree/tinyui-g-current-capability-gap`。
- 创建或切换后必须同步并更新 submodule。
- G 线内部严格串行：`G0 -> G1 -> G2 -> G3 -> G4 -> G5 -> G6 -> G7 -> G8`。
- 修改函数、方法或类前必须运行 GitNexus impact，并在 subagent 最终回复中报告 blast radius。
- 每个 Task 完成后运行 `gitnexus_detect_changes(scope="all")`。
- 每个 Task 后必须只读 review；review 不通过时，原 subagent 在同一上下文修复。
- 主线程负责：方案决策、impact 判断、结果收敛、最终验证。
- 不新增新控件；不把 G 线改写成新 widget 扩张线。

## 1. 文件结构与写面

### G1 真相源 / 矩阵组

- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 新增: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- 可选修改: `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`

### G2 list contract 组

- 修改: `tinyui/include/tinyui/list.h`
- 修改: `tinyui/src/widgets/list.c`
- 修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tests/tinyui/unit/test_tinyui_list.c`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

### G3 list marker 组

- 修改: `tinyui/src/backend/ldgui/backend_app.c`
- 修改: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 修改: `tinyui/docs/demo_guide.md`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

### G4 slider range 组

- 修改: `tinyui/include/tinyui/slider.h`
- 修改: `tinyui/src/widgets/slider.c`
- 必要时修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

### G5 shared base 组

- 修改: `tinyui/include/tinyui/widget.h`
- 修改: `tinyui/src/core/widget.c`
- 修改: `tinyui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tinyui/src/backend/ldgui/backend_theme.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`

### G6 交互控件组

- 修改: `tinyui/include/tinyui/{button,checkbox,switch,slider}.h`
- 修改: `tinyui/src/widgets/{button,checkbox,switch,slider}.c`
- 必要时修改: `tinyui/src/backend/ldgui/backend_{button,checkbox,switch,slider}.c`
- 修改: `tests/tinyui/unit/test_tinyui_button_events.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`

### G7 展示控件组

- 修改: `tinyui/include/tinyui/{window,label,text,image}.h`
- 修改: `tinyui/src/widgets/{window,label,text,image}.c`
- 必要时修改: `tinyui/src/backend/ldgui/backend_{window,label,text,image}.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`
- 修改: `tinyui/docs/demo_guide.md`

### G8 closeout 组

- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- 必要时修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-design.md`

## 2. Tasks

### Task G0: worktree 准备和 baseline

**文件:**
- Read: `docs/tinyui-serial/G-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/tinyui-g-current-capability-gap HEAD
cd .worktree/tinyui-g-current-capability-gap
git submodule sync --recursive
git submodule update --init --recursive
```

- [ ] **Step 2: 配置 baseline build**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
```

期望：configure 成功。

- [ ] **Step 3: 运行 baseline gate**

```bash
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

期望：全部通过。否则停止并记录为 G0 blocker，不进入 G1。

### Task G1: 冻结 capability gap 真相源

**文件:**
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 新增: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- 可选修改: `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`

- [ ] **Step 1: 生成矩阵文档骨架**

矩阵必须逐行覆盖：

- `window -> ldWindow`
- `label -> ldLabel`
- `button -> ldButton`
- `checkbox -> ldCheckBox`
- `switch -> ldSwitch`
- `slider -> ldSlider`
- `text -> ldText`
- `image -> ldImage`
- `list -> ldList`

每行至少包含：

- TINYUI public API
- 对应 `ld*` 能力入口
- `support/reject/deferred/incomplete_contract`
- `unit/contract/mapping/visible/manual artifact`

- [ ] **Step 2: 回写 G线索引**

把 `G线` 索引中的“当前完成态”“review 结论”“当前优先缺口”压缩成入口摘要，并链接到矩阵文档，不再让索引承担全部细节。

- [ ] **Step 3: 可选扩展 contract script**

如果 `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 能低风险扩展，则把 `list` 和 `incomplete_contract` 状态纳入检查；若现有脚本不适合直接扩展，则只在文档中明确并记录豁免原因。

- [ ] **Step 4: 验证 G1**

```bash
git diff --check
rg -n "incomplete_contract|ldList|ldSlider|ldWindow|ldLabel|ldButton|ldCheckBox|ldSwitch|ldText|ldImage" \
  docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md \
  docs/tinyui-serial/G-线计划索引.md
python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py
```

期望：文档无占位，contract 检查通过；若脚本未扩展，则必须在 subagent 总结中明确“本阶段只冻结真相源，未改变 contract script 行为”。

- [ ] **Step 5: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望：只影响 G 线文档 / contract 写面。

### Task G2: list selection contract 收口

**文件:**
- 修改: `tinyui/include/tinyui/list.h`
- 修改: `tinyui/src/widgets/list.c`
- 修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tests/tinyui/unit/test_tinyui_list.c`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

- [ ] **Step 1: GitNexus impact**

运行：

```text
gitnexus_impact(target="tinyui_list_set_on_selected", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_widget_dispatch_signal", direction="upstream", repo="LingDongGUI")
```

期望：记录 direct callers、affected processes 和风险等级。若 HIGH/CRITICAL，先汇报主线程再继续。

- [ ] **Step 2: 决定 contract 收口方式**

只能二选一，不允许模糊口径：

1. 补真实 native selection event bridge，并把 `on_selected` 提升为 `support`
2. 保持当前实现，但把 public API 明确降级为 `incomplete_contract` 或改成 reject/deferred 语义

subagent 必须在本阶段回复里先说明选了哪条路，以及为什么。

- [ ] **Step 3: 先写 RED 测试**

在 `tests/tinyui/unit/test_tinyui_list.c` 新增至少一种 fail-first 断言：

- 若选择补 bridge：native selection 改变必须触发 callback
- 若选择降级：public API 调用后必须有明确返回值 / 文档边界 / 断言行为，不允许继续“默默保存 callback 但永远不触发”

- [ ] **Step 4: 实现最小修复**

只修复 `list selection contract`，不要顺手扩 `itemHeight`、`padding`、`align` 等新能力。

- [ ] **Step 5: 验证 G2**

```bash
ctest --test-dir build -R test_tinyui_list --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
git diff --check
```

期望：list unit 通过，文档与 public API 口径一致。

- [ ] **Step 6: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望：只影响 list contract / event bridge 写面。

### Task G3: list marker 语义拆分

**文件:**
- 修改: `tinyui/src/backend/ldgui/backend_app.c`
- 修改: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 修改: `tinyui/docs/demo_guide.md`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

- [ ] **Step 1: GitNexus impact**

运行：

```text
gitnexus_impact(target="tinyui_backend_log_mapping_markers", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_backend_append_widget_ids", direction="upstream", repo="LingDongGUI")
```

期望：记录风险。若 GitNexus 对 static backend 函数缺失，明确记为弱证据，再继续。

- [ ] **Step 2: 先写 mapping RED**

在 `tests/tinyui/runtime/check_tinyui_backend_mapping.py` 先写清新的期望语义：

- `list` 属于真实 backend widget id
- `item_*` 若继续输出，必须走单独 marker 名称或单独解释路径
- 不允许再把 `item_*` 和普通 widget id 混成同一强语义集合

- [ ] **Step 3: 改 marker 输出**

在 `backend_app.c` 把 list item payload marker 从 widget marker 里拆出来。命名优先：

- `PICOUI_BACKEND_LIST_ITEM_IDS`

若受兼容性限制暂时保留旧 marker，则必须同时输出新 marker，并在脚本和文档中把旧 marker 视为兼容层，不能继续作为主证据。

- [ ] **Step 4: 同步脚本与文档**

同步 `demo_guide`、`G线索引`、matrix 文档，明确：

- widget marker 证明什么
- item payload marker 证明什么
- 哪些证据仍不能推出“独立 backend widget”

- [ ] **Step 5: 验证 G3**

```bash
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo list_basic
git diff --check
```

期望：mapping 和 list visible 通过，文档口径一致。

- [ ] **Step 6: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望：只影响 list marker / runtime mapping 写面。

### Task G4: slider range contract 收口

**文件:**
- 修改: `tinyui/include/tinyui/slider.h`
- 修改: `tinyui/src/widgets/slider.c`
- 必要时修改: `tinyui/src/backend/ldgui/backend_event.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`

- [ ] **Step 1: GitNexus impact**

运行：

```text
gitnexus_impact(target="tinyui_slider_set_range", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="tinyui_slider_set_value", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 RED 测试**

在 `tests/tinyui/unit/test_tinyui_widgets.c` 增加至少两类断言：

- range clamp 改值时，TINYUI `value` 行为
- 若合同要求同步到底层，则 `ldSlider.permille` 也必须同步

- [ ] **Step 3: 二选一收口**

只能二选一：

1. 把 clamp 后的 value 同步到底层 `ldSlider`
2. 保持当前 shadow state 行为，但在 public contract / matrix 中明确降级

subagent 必须在回复里说明选项及理由。

- [ ] **Step 4: 验证 G4**

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
git diff --check
```

期望：slider 相关断言通过，文档和代码口径一致。

- [ ] **Step 5: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望：只影响 slider 写面。

### Task G5: shared base 语义决议

**文件:**
- 修改: `tinyui/include/tinyui/widget.h`
- 修改: `tinyui/src/core/widget.c`
- 修改: `tinyui/src/backend/ldgui/backend_style_apply.c`
- 修改: `tinyui/src/backend/ldgui/backend_theme.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`

- [ ] **Step 1: 冻结决策表**

先在 subagent 回复中明确下面各项是要 `support`、`reject` 还是 `deferred`：

- direct style setter 是否实时下沉
- `enabled` 是否所有交互控件都同步到底层
- 是否公开 `selectable`
- 是否公开 `corner`
- 是否公开 `focus`
- 是否公开 `dirty`

主线程 review 通过前，不写实现。

- [ ] **Step 2: 按决策写 RED 测试**

根据 Step 1 的决策，在 `test_tinyui_widgets.c` / `test_tinyui_theme.c` 写 fail-first 断言。

- [ ] **Step 3: 实现最小 shared base 变更**

只做 Step 1 已批准的决策，不顺手扩 widget-specific 能力。

- [ ] **Step 4: 验证 G5**

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
ctest --test-dir build -R test_tinyui_theme --output-on-failure
git diff --check
```

期望：shared base 相关单测通过。

### Task G6: 交互控件高价值缺口

**文件:**
- 修改: `tinyui/include/tinyui/{button,checkbox,switch,slider}.h`
- 修改: `tinyui/src/widgets/{button,checkbox,switch,slider}.c`
- 必要时修改: backend 对应文件
- 修改: `tests/tinyui/unit/test_tinyui_button_events.c`
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`

- [ ] **Step 1: 只选一批最小能力**

本阶段只允许从下面列表里选一小批：

- `switch direction/orientation/image`
- `slider orientation/image/indicatorWidth/slimSize`
- `checkbox radio/image mode`
- `button checkable/image`

必须先在 subagent 回复中列出“本批只做哪些，不做哪些”。

- [ ] **Step 2: 写 RED 测试**

为本批能力写最小 fail-first 单测。

- [ ] **Step 3: 实现最小代码**

只改本批能力。若触达 shared base，停止并回主线程拆任务。

- [ ] **Step 4: 验证 G6**

```bash
ctest --test-dir build -R test_tinyui_button_events --output-on-failure
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
git diff --check
```

期望：交互控件相关断言通过。

### Task G7: 展示控件高价值缺口

**文件:**
- 修改: `tinyui/include/tinyui/{window,label,text,image}.h`
- 修改: `tinyui/src/widgets/{window,label,text,image}.c`
- 必要时修改: backend 对应文件
- 修改: `tests/tinyui/unit/test_tinyui_widgets.c`
- 修改: `tests/tinyui/unit/test_tinyui_theme.c`
- 修改: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 只选一批最小能力**

本阶段只允许从下面列表里选一小批：

- `text scroll/static/background image`
- `label align/transparent/background image`
- `window image/grid padding/padding group`
- `image mask color`

- [ ] **Step 2: 写 RED 测试**

为本批能力写 fail-first 单测。

- [ ] **Step 3: 实现最小代码**

只改本批能力；若触达 shared base，停止并回主线程拆任务。

- [ ] **Step 4: 验证 G7**

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
ctest --test-dir build -R test_tinyui_theme --output-on-failure
git diff --check
```

期望：展示控件相关断言通过。

### Task G8: G线 closeout

**文件:**
- 修改: `docs/tinyui-serial/G-线计划索引.md`
- 修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- 必要时修改: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-design.md`

- [ ] **Step 1: 更新索引与矩阵**

把每一阶段的完成态、reject/deferred/incomplete_contract 边界和剩余缺口回写到索引与 matrix。

- [ ] **Step 2: 独立 review**

派独立 review subagent，只读检查：

- 文档是否仍把子集写成 100% 封装
- `mapping/visible/manual artifact` 是否重新写混
- reject/deferred/incomplete_contract 是否与代码一致
- 新增 public API 是否都有最小测试

- [ ] **Step 3: 最终验证**

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

期望：全部通过，且文档不再存在过强表述。

- [ ] **Step 4: detect changes**

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

期望：affected scope 与各阶段目标一致。
