# PicoUI C线门禁工程化实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**目标:** 把 PicoUI B 线已有的自动 visible correctness 结果，工程化为文档口径准确、CTest 可执行、mapping matrix 可维护、可选人工 artifact 可追溯的长期门禁。

**架构:** C 线严格串行推进：先修证据口径，再接入 CTest，再写 gate 执行矩阵，再扩 backend mapping matrix，再补可选人工窗口 artifact，最后固化长期规则并做独立 review。每个任务都限制写面，避免多个 subagent 同时改同一批文档或脚本。

**技术栈:** CMake、CTest、Python3、SDL2 host runtime、PicoUI runtime scripts、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 每个 Task 由一个 fresh subagent 执行。
- 同一时间只推进一个 Task；本计划是串行计划，不并行开发。
- 每个 Task 完成后必须由独立 review subagent 做只读 review；review 不通过时，原执行 subagent 在同一上下文内修复。
- 修改代码符号前必须按仓库规则运行 GitNexus impact；纯文档任务不需要 impact。
- 提交前必须运行 `gitnexus_detect_changes(scope="all")` 或说明本轮没有提交。
- 每个 Task 的最终回复必须包含：
  - 修改文件
  - 验证命令和结果
  - 未覆盖风险
  - 是否需要进入下一 Task

## 1. 文件结构与写面分组

### G1 文档口径组

**文件:**
- Modify: `docs/picoui-serial/B-线计划索引.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- Modify: `picoui/docs/demo_guide.md`

职责：

- 收紧“真实窗口”表述。
- 固定 smoke / mapping / automatic visible / manual artifact 的边界。
- 写清本地和 CI 运行口径。
- 写清长期新增规则。

### G2 CTest wiring 组

**文件:**
- Modify: `tests/picoui/CMakeLists.txt`
- Optional Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Optional Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`

职责：

- 把 visible gate 和 backend mapping gate 接入 CTest。
- 保持 standalone Python 脚本仍可直接运行。
- 保持 failure message 可定位。

### G3 backend mapping matrix 组

**文件:**
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Optional Modify: `picoui/demo/*/main.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`

职责：

- 结构化 mapping matrix。
- 明确 6 个 demo 的 coverage 或豁免。
- 不为了 marker 改变 demo 用户意图。

### G4 manual artifact 组

**文件:**
- Create: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- Create: `docs/picoui-serial/C-线人工窗口验收记录.md`
- Modify: `picoui/docs/demo_guide.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`

职责：

- 提供可选人工窗口 artifact gate。
- 默认不让无窗口 CI 因该 gate 失败。
- 把人工结论和自动 gate 结论分开。

---

## 2. 任务拆分

### Task 1: C1 文档口径收紧

**负责人:** G1 文档口径组
**文件:**
- Modify: `docs/picoui-serial/B-线计划索引.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- Modify: `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 搜索所有容易过度声称的表述**

运行：

```bash
rg -n "真实窗口|人眼|人工|manual|visible gate|PICOUI_CAPTURE_FILE|SDL_VIDEODRIVER|capture 非空" \
  docs/picoui-serial \
  docs/superpowers/specs \
  docs/superpowers/plans \
  picoui/docs
```

期望：

- 找到 B 线和 demo guide 中所有相关表述。
- 记录哪些表述需要保留为历史背景，哪些需要改成当前证据层级。

- [ ] **Step 2: 修改 B 线 closeout 口径**

要求：

- 把“真实窗口里 UI 可正常显示、可读、可判定”改成“自动 visible gate 已证明 dummy SDL + PPM readback 下可显示、可读、可判定”。
- 保留用户真实窗口反馈作为 B 线启动原因。
- 明确人工窗口验收属于 C6，不属于 B 线自动 gate。

- [ ] **Step 3: 修改测试架构和 demo guide 口径**

要求：

- 在测试架构文档中新增或修正四类 gate：
  - smoke gate
  - backend mapping gate
  - automatic visible gate
  - manual window artifact gate
- 在 demo guide 中写清运行 demo、自动 visible gate、人工窗口观察三者的区别。

- [ ] **Step 4: 回查 C 线索引**

运行：

```bash
rg -n "真实窗口|人工|manual|dummy SDL|PPM readback|automatic visible|manual window artifact" \
  docs/picoui-serial/C-线计划索引.md
```

期望：

- C 线索引中每个“真实窗口/人工”表述都有清晰边界。

- [ ] **Step 5: 文档格式验证**

运行：

```bash
git diff --check
rg -n "TBD|TODO|待定|后续补" \
  docs/picoui-serial/B-线计划索引.md \
  docs/picoui-serial/C-线计划索引.md \
  docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md \
  docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md \
  docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
  picoui/docs/demo_guide.md
```

期望：

- `git diff --check` PASS。
- `rg` 不出现新占位内容。

- [ ] **Step 6: 提交**

```bash
git add docs/picoui-serial/B-线计划索引.md \
        docs/picoui-serial/C-线计划索引.md \
        docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md \
        docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        picoui/docs/demo_guide.md
git commit -m "docs(picoui): clarify c-line gate evidence"
```

### Task 2: C2 visible gate 接入 CTest

**负责人:** G2 CTest wiring 组
**文件:**
- Modify: `tests/picoui/CMakeLists.txt`
- Optional Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`

- [ ] **Step 1: 读取现有 CTest helper**

运行：

```bash
sed -n '1,120p' tests/picoui/CMakeLists.txt
rg -n "function\\(ld_add_python_test|ld_add_python_test" cmake tests -g '*.cmake' -g 'CMakeLists.txt'
```

期望：

- 明确 `ld_add_python_test` 当前参数格式和 label 写法。

- [ ] **Step 2: 修改 CMake 注册 visible gate**

要求：

- 在 `tests/picoui/CMakeLists.txt` 新增 `check_picoui_visible_ui`。
- 脚本为 `tests/picoui/runtime/check_picoui_visible_ui.py`。
- 参数固定为 `--all`。
- labels 至少为 `picoui;runtime;visible`。

- [ ] **Step 3: 如 helper 不支持参数，做最小兼容改动**

要求：

- 优先使用现有 helper 能力。
- 若 helper 不支持 `--all`，只做最小 CMake 层改动，不重写测试框架。
- 不改 visible 脚本的检查语义。

- [ ] **Step 4: 配置和运行 targeted CTest**

运行：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -N -R check_picoui_visible_ui
ctest --test-dir build -R check_picoui_visible_ui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
```

期望：

- `ctest -N` 能列出 `check_picoui_visible_ui`。
- targeted CTest PASS。
- `-L visible` 至少跑到该测试并 PASS。
- standalone 脚本仍 PASS。

- [ ] **Step 5: 更新文档状态**

要求：

- 在 C 线索引中把 C2 收口证据写为已完成。
- 在测试架构文档中写明 visible gate 已进入 CTest。

- [ ] **Step 6: GitNexus 变更检测**

运行：

```text
gitnexus_detect_changes(scope="all")
```

期望：

- 影响范围符合 `tests/picoui/CMakeLists.txt` 和 runtime gate wiring。
- 如果 CLI 不可用，改用 MCP `gitnexus_detect_changes(scope="all")` 并记录结果。

- [ ] **Step 7: 提交**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/runtime/check_picoui_visible_ui.py \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        docs/picoui-serial/C-线计划索引.md
git commit -m "test(picoui): wire visible gate into ctest"
```

### Task 3: C3 backend mapping gate 接入 CTest

**负责人:** G2 CTest wiring 组
**文件:**
- Modify: `tests/picoui/CMakeLists.txt`
- Optional Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`

- [ ] **Step 1: 确认 Task 2 已完成**

运行：

```bash
ctest --test-dir build -N -R check_picoui_visible_ui
ctest --test-dir build -L visible --output-on-failure
```

期望：

- visible gate 已在 CTest 中可见并通过。

- [ ] **Step 2: 修改 CMake 注册 backend mapping gate**

要求：

- 在 `tests/picoui/CMakeLists.txt` 新增 `check_picoui_backend_mapping`。
- 脚本为 `tests/picoui/runtime/check_picoui_backend_mapping.py`。
- labels 至少为 `picoui;runtime;backend;mapping`。

- [ ] **Step 3: 运行 targeted CTest**

运行：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -N -R check_picoui_backend_mapping
ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
```

期望：

- `ctest -N` 能列出 `check_picoui_backend_mapping`。
- targeted CTest PASS。
- `-L mapping` 至少跑到该测试并 PASS。
- standalone 脚本仍 PASS。

- [ ] **Step 4: 更新文档状态**

要求：

- 在 C 线索引中把 C3 收口证据写为已完成。
- 在测试架构文档中写明 backend mapping gate 已进入 CTest。

- [ ] **Step 5: GitNexus 变更检测**

运行：

```text
gitnexus_detect_changes(scope="all")
```

期望：

- 影响范围符合 CTest wiring 和 backend mapping gate。
- 如果 CLI 不可用，改用 MCP `gitnexus_detect_changes(scope="all")` 并记录结果。

- [ ] **Step 6: 提交**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/runtime/check_picoui_backend_mapping.py \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        docs/picoui-serial/C-线计划索引.md
git commit -m "test(picoui): wire backend mapping gate into ctest"
```

### Task 4: C4 gate 执行矩阵与本地/CI 口径

**负责人:** G1 文档口径组
**文件:**
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`
- Modify: `picoui/docs/demo_guide.md`
- Optional Modify: existing CI config if present

- [ ] **Step 1: 查找现有 CI 配置**

运行：

```bash
rg --files -g '*.yml' -g '*.yaml' .github .gitlab ci 2>/dev/null || true
```

期望：

- 如果存在 CI 配置，记录路径。
- 如果不存在，不新增复杂 CI，只写本地运行口径。

- [ ] **Step 2: 写最小 gate matrix**

要求在测试架构文档和 C 线索引中写清：

```bash
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

含义：

- `picoui` 是总入口。
- `visible` 是自动 visible correctness。
- `mapping` 是 backend marker/matrix。

- [ ] **Step 3: 写完整 gate matrix**

要求写清完整本地验收：

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

- [ ] **Step 4: 若存在 CI，做最小接入**

要求：

- 只在现有 CI job 中追加 CTest label 命令。
- 不新增复杂矩阵。
- 不把 manual artifact gate 放入无窗口 CI 必跑项。

- [ ] **Step 5: 验证文档和命令**

运行：

```bash
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
rg -n "smoke gate|visible gate|backend mapping gate|manual artifact gate|ctest --test-dir build -L" \
  docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
  docs/picoui-serial/C-线计划索引.md \
  picoui/docs/demo_guide.md
git diff --check
```

期望：

- 三组 CTest PASS。
- 文档能查到唯一口径。
- `git diff --check` PASS。

- [ ] **Step 6: 提交**

```bash
git add docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        docs/picoui-serial/C-线计划索引.md \
        picoui/docs/demo_guide.md
# 如果 Step 4 修改了现有 CI 配置，把对应 CI 文件显式加入 git add。
git commit -m "docs(picoui): document c-line gate matrix"
```

### Task 5: C5 扩展 backend mapping matrix

**负责人:** G3 backend mapping matrix 组
**文件:**
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Optional Modify: `picoui/demo/hello_world/main.c`
- Optional Modify: `picoui/demo/basic_widgets/main.c`
- Optional Modify: `picoui/demo/layout_flex/main.c`
- Optional Modify: `picoui/demo/layout_grid/main.c`
- Optional Modify: `picoui/demo/theme_showcase/main.c`
- Optional Modify: `picoui/demo/settings_panel/main.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`

- [ ] **Step 1: 修改前做 GitNexus impact**

Run impact before editing any symbol that may change:

```text
gitnexus_impact(target="check_picoui_backend_mapping.py", direction="upstream")
gitnexus_impact(target="picoui_backend_app_run", direction="upstream")
```

期望：

- 如果 GitNexus 对 Python 文件或 static/internal 符号无法定位，记录“索引未命中”，再用源码阅读和测试补证。
- 如果 HIGH/CRITICAL，先停止并汇报。

- [ ] **Step 2: 结构化 mapping matrix**

要求：

- 把脚本中的 `TARGETS`、`STATIC_TARGETS`、`INTERACTIVE_TARGETS` 合并或重排成单一显式 matrix。
- 每个 target 至少包含：
  - `category`
  - `required_markers`
  - `required_real_ids`
  - `forbidden_markers`
  - `coverage_note`

- [ ] **Step 3: 覆盖或豁免 6 个 demo**

要求：

- 6 个 demo 都必须出现在 matrix 或显式豁免列表中：
  - `picoui_hello_world_demo`
  - `picoui_basic_widgets_demo`
  - `picoui_layout_flex_demo`
  - `picoui_layout_grid_demo`
  - `picoui_theme_showcase_demo`
  - `picoui_settings_panel_demo`
- 对没有具体 id 要求的 demo，`coverage_note` 必须解释原因。
- 不为 marker 改 demo 用户意图。

- [ ] **Step 4: 运行 mapping 和 visible 验证**

运行：

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
ctest --test-dir build -L mapping --output-on-failure
```

期望：

- backend mapping PASS。
- visible gate 仍覆盖 6 个 demo 并 PASS。

- [ ] **Step 5: 更新文档**

要求：

- 在测试架构文档写清 mapping matrix 的覆盖范围。
- 在 C 线索引中写清 C5 是否已完成。

- [ ] **Step 6: GitNexus detect changes**

运行：

```text
gitnexus_detect_changes(scope="all")
```

期望：

- 影响范围符合 backend mapping 脚本和必要 marker 输出。

- [ ] **Step 7: 提交**

```bash
git add tests/picoui/runtime/check_picoui_backend_mapping.py \
        picoui/demo/hello_world/main.c \
        picoui/demo/basic_widgets/main.c \
        picoui/demo/layout_flex/main.c \
        picoui/demo/layout_grid/main.c \
        picoui/demo/theme_showcase/main.c \
        picoui/demo/settings_panel/main.c \
        picoui/src/backend/ldgui/backend_app.c \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        docs/picoui-serial/C-线计划索引.md
git commit -m "test(picoui): expand backend mapping matrix"
```

### Task 6: C6 可选人工窗口 artifact gate

**负责人:** G4 manual artifact 组
**文件:**
- Create: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- Create: `docs/picoui-serial/C-线人工窗口验收记录.md`
- Modify: `picoui/docs/demo_guide.md`
- Modify: `docs/picoui-serial/C-线计划索引.md`

- [ ] **Step 1: 定义脚本行为**

要求：

- 脚本默认不设置 `SDL_VIDEODRIVER=dummy`。
- 支持 `--demo basic_widgets` 和 `--demo settings_panel`。
- 支持 `--artifact-dir artifacts/picoui/manual-window`。
- 输出 artifact 路径。
- 若环境无窗口或 demo 无法运行，失败信息必须说明这是 manual gate，不是 CI blocker。

- [ ] **Step 2: 写 artifact 记录文档模板**

Create `docs/picoui-serial/C-线人工窗口验收记录.md` with sections:

```markdown
# PicoUI C线人工窗口验收记录

## 使用规则

- 本文只记录 manual window artifact gate。
- 没有本文记录时，不允许声称人工窗口验收通过。
- 本 gate 不替代 visible/mapping CTest。

## 记录模板

### YYYY-MM-DD <demo-name>

- 平台：
- SDL video driver：
- demo target：
- 构建目录：
- 运行命令：
- artifact 路径：
- 人工结论：
- 已知限制：
```

- [ ] **Step 3: 实现脚本**

要求：

- 复用 `check_picoui_visible_ui.py` 的 demo target 查找思路。
- 只覆盖 `basic_widgets` 和 `settings_panel`。
- 运行 demo 时设置：
  - `PICOUI_DEMO_AUTO_QUIT_MS=1200`
  - `PICOUI_CAPTURE_FILE=<artifact path>`
- 不设置 `SDL_VIDEODRIVER=dummy`，除非用户显式传入环境变量。

- [ ] **Step 4: 运行 manual gate**

运行：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target picoui_basic_widgets_demo picoui_settings_panel_demo
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

期望：

- 在有窗口环境时生成 artifact。
- 在无窗口环境时，输出清楚的 manual gate failure，不把它写成 CTest 必跑 blocker。

- [ ] **Step 5: 更新 demo guide 和 C 线索引**

要求：

- demo guide 写清 manual artifact 运行方式。
- C 线索引写清 C6 的完成状态或环境限制。

- [ ] **Step 6: 提交**

```bash
git add tests/picoui/runtime/check_picoui_manual_window_artifact.py \
        docs/picoui-serial/C-线人工窗口验收记录.md \
        picoui/docs/demo_guide.md \
        docs/picoui-serial/C-线计划索引.md
git commit -m "test(picoui): add manual window artifact gate"
```

### Task 7: C7 长期回归规则与新增项同步清单

**负责人:** G1 文档口径组
**文件:**
- Modify: `docs/picoui-serial/C-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 写新增 demo 同步清单**

要求列出新增 demo 必须同步：

- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `picoui/docs/demo_guide.md`
- serial docs 当前线索引

- [ ] **Step 2: 写新增 widget 同步清单**

要求列出新增 widget 必须同步：

- public API contract
- backend mapping test
- visible gate 样本或不可见理由
- theme/style 支持或拒绝说明
- demo boundary 检查

- [ ] **Step 3: 写新增 layout/theme 能力同步清单**

要求列出新增 layout/theme 必须同步：

- unit/contract test
- runtime demo 样本
- visible gate 或明确不可见理由
- mapping matrix 文档

- [ ] **Step 4: 验证唯一口径**

运行：

```bash
rg -n "新增 demo|新增 widget|新增 layout|新增 theme|gate 同步|visible matrix|mapping matrix" \
  docs/picoui-serial/C-线计划索引.md \
  docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
  picoui/docs/demo_guide.md
git diff --check
```

期望：

- 三类新增规则都有清楚命中。
- `git diff --check` PASS。

- [ ] **Step 5: 提交**

```bash
git add docs/picoui-serial/C-线计划索引.md \
        docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
        picoui/docs/demo_guide.md
git commit -m "docs(picoui): define gate sync rules"
```

### Task 8: C8 closeout review 与最终验证

**负责人:** 主线程协调，独立 review subagent 执行只读 review
**文件:**
- Modify: `docs/picoui-serial/C-线计划索引.md`
- Optional Modify: docs only if review finds wording gaps

- [ ] **Step 1: 独立 subagent 做只读 review**

Prompt:

```text
请对 /Users/cys/embedded/LingDongGUI 的 PicoUI C线做只读 closeout review。
不要修改文件。
重点检查：
1. C线 spec/plan/index 是否一致。
2. 文档是否仍混用 automatic visible gate 和 manual window artifact。
3. visible/mapping CTest 是否真实注册并可执行。
4. backend mapping matrix 是否覆盖或显式豁免 6 个 demos。
5. 新增 demo/widget/layout/theme 规则是否清楚。
请 findings first，带文件路径/行号，最后给 approve/request changes。
```

- [ ] **Step 2: 若 review 不通过，原执行 subagent 修复**

要求：

- review 不通过的任务，在同一个 subagent 修复。
- 主线程不直接修 review 问题，除非问题是单行文档笔误且用户同意。

- [ ] **Step 3: 运行最终验证**

运行：

```bash
git status --short
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
git diff --check
```

期望：

- 所有自动 gate PASS。
- manual artifact gate 若未运行，必须明确写成“未运行/非自动 blocker”，不能写通过。

- [ ] **Step 4: 更新 C 线索引 closeout**

要求：

- `当前状态` 改为 `C线 已按 C0 -> C8 顺序完成`。
- `当前游标` 改为 `C8 closeout`。
- 写入最终验证命令和结果。
- 写清是否存在人工窗口 artifact。

- [ ] **Step 5: GitNexus detect changes**

运行：

```text
gitnexus_detect_changes(scope="all")
```

期望：

- 影响范围符合 C 线门禁工程化。

- [ ] **Step 6: 提交**

```bash
git add docs/picoui-serial/C-线计划索引.md
git commit -m "docs(picoui): close c-line gate hardening"
```

---

## 3. 最终收口命令

完整 C 线最终收口必须运行：

```bash
git status --short
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
git diff --check
```

若执行了 C6，还要附加：

```bash
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

manual artifact 失败不自动阻塞无窗口 CI，但会阻塞“人工窗口验收通过”这类结论。
