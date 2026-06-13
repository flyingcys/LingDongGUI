# TINYUI H线第一版本发布准备 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `H线` 串行收口为基于当前 `9` 个已覆盖控件的第一版本发布准备线，产出 release matrix、当前控件 release contract、最小 manual artifact、发布文档包与独立 review。

**Architecture:** 本计划不扩控件数量，不建立新输入系统，只围绕当前 `9` 控件做发布准备。任务严格串行，每个 Task 交给 fresh subagent，写面不重叠；代码改动阶段前必须跑 GitNexus impact，阶段结束后必须做独立 review。

**Tech Stack:** Markdown serial docs、Python3 gate scripts、CTest/CMake、TINYUI runtime scripts、GitNexus、LingDongGUI

---

## 0. 执行规则

- 每次只推进一个 Task。
- 每个 Task 使用一个 fresh subagent 执行；review 由独立 review subagent 只读完成。
- 同一 Task review 不通过时，由原执行 subagent 在同一上下文里修，不开新修复线程。
- 纯文档 Task 可不跑 GitNexus impact；涉及 Python/C 符号修改的 Task 必须先跑 impact。
- 提交前必须运行 `git diff --check`；涉及代码改动的 Task 还必须运行对应测试命令。
- 若要提交，提交前必须运行 `gitnexus_detect_changes(scope=\"all\")` 或明确说明本轮未提交。

## 1. 文件结构与写面分组

### 文档基线组

**文件：**
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `docs/tinyui-serial/C-线人工窗口验收记录.md`
- Optional Create: `docs/tinyui-serial/H-线第一版发布说明.md`

职责：

- 冻结 `H线` 口径。
- 维护差距对比、demo catalog、manual artifact 记录和发布说明。

### release matrix 组

**文件：**
- Create: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Create: `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- Modify: `tests/tinyui/CMakeLists.txt`

职责：

- 建立机器可读 release matrix。
- 建立 H 线专属 gate。
- 把 gate 接入 CTest。

### 当前 9 控件合同组

**文件：**
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Optional Create: `docs/tinyui-serial/H-线当前9控件发布合同.md`
- Optional Modify: `tests/tinyui/unit/test_tinyui_theme.c`
- Optional Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Optional Modify: `tests/tinyui/unit/test_tinyui_widgets.c`

职责：

- 把 `9` 个控件的 first-release wording、supported APIs、known limitations 写清。
- 对 `image/list` 的非 support 项补最小硬化测试。

### 发布收尾组

**文件：**
- Modify: `tinyui/docs/demo_guide.md`
- Optional Create: `docs/tinyui-serial/H-线发布测试矩阵.md`
- Optional Create: `docs/tinyui-serial/H-线发布closeout.md`

职责：

- 冻结发布测试矩阵。
- 整理发布文档包。
- 承接独立 review 与 closeout。

## 2. 串行任务拆分

### Task 1: H0-H1 口径冻结与基线文档收口

**Files:**
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `tinyui/docs/demo_guide.md`
- Reference: `docs/superpowers/specs/2026-05-30-tinyui-h-line-first-release-preparation-design.md`

- [ ] **Step 1: 复核当前 H 线文档和 G 线真相源**

Run:

```bash
sed -n '1,260p' docs/tinyui-serial/H-线计划索引.md
sed -n '1,260p' docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md
sed -n '1,220p' docs/tinyui-serial/G-线计划索引.md
sed -n '1,220p' docs/tinyui-serial/G-线剩余缺口合同决策入口.md
```

Expected:

- 明确当前 `H线` 只服务现有 `9` 控件的发布准备。
- 不再把 `progress_bar / line_edit / combo_box` 写成当前串行 blocker。

- [ ] **Step 2: 冻结 H 线禁止/允许口径**

要求：

- 在 `H-线计划索引.md` 与对比文档中统一写明 `internal v0.1` 与 `public v1.0` 边界。
- `blocker` 一律使用 `internal v0.1 blockers` / `post-H candidates` 两层口径。
- 保持和 spec 一致，不出现新的混合表述。

- [ ] **Step 3: 收紧 demo guide 的 H 线依赖表述**

要求：

- 在 `tinyui/docs/demo_guide.md` 里明确 demo catalog 是发布阅读材料的一部分。
- 保留证据层分层，不把 demo 可运行写成人工验收。

- [ ] **Step 4: 文档自检**

Run:

```bash
git diff --check -- docs/tinyui-serial/H-线计划索引.md \
                  docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md \
                  tinyui/docs/demo_guide.md
rg -n "progress_bar|line_edit|combo_box" docs/tinyui-serial/H-线计划索引.md
```

Expected:

- `git diff --check` 无输出。
- H 线主任务段不再把三个新控件写成当前阶段串行任务。

- [ ] **Step 5: 独立 review**

要求：

- 派 review subagent 只读检查上述三份文档。
- 若 review 指出 blocking，原 subagent 原地修复。

### Task 2: H2 release matrix schema 设计

**Files:**
- Create: `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`

- [ ] **Step 1: 先定义 release matrix 字段**

要求：

- 为每个控件记录 `wrapped/not_wrapped/deferred`。
- 为每个能力项记录 `support/reject/incomplete_contract/deferred`。
- 为每个发布判断记录 `internal_v0_1_blocker/internal_v0_1_known_limitation/post_h_candidate`。
- 为 `manual_artifact` 单独区分 `artifact_entry_exists` 与 `manual_review_required`，不要把人眼结论编码成自动 gate 布尔值。

- [ ] **Step 2: 写最小 JSON 初稿**

要求：

- 至少覆盖当前 `9` 个控件。
- 至少覆盖 `image` / `list` 的非 support 项。
- 明确 `17` 个未覆盖控件为 `not_wrapped`。

- [ ] **Step 3: 文档回填 schema 说明**

要求：

- 在 H 线索引和对比文档中写明 release matrix 是新的机器可读真相源。
- 明确该 matrix 不负责判断人工是否“已验收通过”。

- [ ] **Step 4: JSON 结构自检**

Run:

```bash
python3 -m json.tool tests/tinyui/contract/tinyui_release_capability_matrix.json >/dev/null
git diff --check -- tests/tinyui/contract/tinyui_release_capability_matrix.json \
                  docs/tinyui-serial/H-线计划索引.md \
                  docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md
```

Expected:

- JSON 合法。
- 文档 diff 无格式错误。

- [ ] **Step 5: 独立 review**

要求：

- review subagent 重点检查 schema 是否混淆自动 gate 与人工结论。

### Task 3: H3 release matrix gate 落地

**Files:**
- Create: `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- Modify: `tests/tinyui/CMakeLists.txt`
- Test: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 对将修改的符号跑 GitNexus impact**

Run:

```text
impact target: check_tinyui_widget_contract_matrix
impact target: tests/tinyui/CMakeLists.txt
```

Expected:

- 记录 blast radius。
- 若出现 `HIGH/CRITICAL`，先停并在任务汇报中说明。

- [ ] **Step 2: 实现最小 gate 语义**

要求：

- 检查 `9` 个已覆盖控件都存在。
- 检查 `17` 个未覆盖控件都标为 `not_wrapped`。
- 检查 `image/list` 非 support 项未被改成 `support`。
- 检查 manual artifact 相关字段存在，但不判断人工结论真假。

- [ ] **Step 3: 接入 CTest**

要求：

- 在 `tests/tinyui/CMakeLists.txt` 注册 `check_tinyui_release_capability_matrix`。
- label 至少包含 `tinyui;contract;release`.

- [ ] **Step 4: 跑 gate**

Run:

```bash
ctest --test-dir build -N -R check_tinyui_release_capability_matrix
ctest --test-dir build -R check_tinyui_release_capability_matrix --output-on-failure
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

Expected:

- `ctest -N` 能列出新 gate。
- targeted CTest 与 standalone script 都通过。

- [ ] **Step 5: detect_changes 与独立 review**

Run:

```text
gitnexus_detect_changes(scope="all")
```

Expected:

- 影响面集中在 H 线 release gate。

### Task 4: H4-H6 当前 9 控件 release contract 与 `image/list` 限制硬化

**Files:**
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Optional Create: `docs/tinyui-serial/H-线当前9控件发布合同.md`
- Optional Modify: `tests/tinyui/unit/test_tinyui_theme.c`
- Optional Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Optional Modify: `tests/tinyui/unit/test_tinyui_widgets.c`

- [ ] **Step 1: 列出现有 `image/list` 非 support 项与现有测试**

Run:

```bash
rg -n "image|list|style_class|user_data|enabled|padding|theme|marker" \
  tests/tinyui/unit \
  docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md
```

Expected:

- 明确哪些限制已有单测。
- 明确哪些限制还只存在于文档。

- [ ] **Step 2: 写 release contract 文档**

要求：

- `9` 个控件逐项列 `supported APIs / backed by ld* / evidence / limitations / wording`。
- `image/list` 的风险项必须显式写成限制，不得模糊成“部分支持”。

- [ ] **Step 3: 只补最小硬化测试**

要求：

- 仅当现有单测不足以防止文档误宣称时才补测试。
- 不扩控件能力，不改 demo 用户意图。

- [ ] **Step 4: 跑目标测试**

Run:

```bash
ctest --test-dir build --output-on-failure -L unit
ctest --test-dir build --output-on-failure -L contract
```

Expected:

- 改动相关 unit/contract 测试通过。

- [ ] **Step 5: 独立 review**

要求：

- review subagent 检查 release wording 是否夸大。

### Task 5: H7 demo catalog 收口

**Files:**
- Modify: `tinyui/docs/demo_guide.md`
- Optional Create: `docs/tinyui-serial/H-线demo-catalog.md`

- [ ] **Step 1: 盘点当前 demo**

Run:

```bash
sed -n '1,260p' tinyui/docs/demo_guide.md
sed -n '1,120p' tests/tinyui/runtime/check_tinyui_visible_ui.py
```

Expected:

- 确认 `7` 个 visible demos 的名称。

- [ ] **Step 2: 为每个 demo 写证明/不证明边界**

要求：

- 每个 demo 写明证明的控件、能力、不证明的能力、对应 gate。
- 保持和 `demo_guide` 的证据层表述一致。

- [ ] **Step 3: 文档自检与 review**

Run:

```bash
git diff --check -- tinyui/docs/demo_guide.md docs/tinyui-serial/H-线demo-catalog.md
```

Expected:

- diff 无格式错误。

### Task 6: H8 最小 manual artifact 收口

**Files:**
- Modify: `docs/tinyui-serial/C-线人工窗口验收记录.md`
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Reference: `tests/tinyui/runtime/check_tinyui_manual_window_artifact.py`

- [ ] **Step 1: 复核现有 manual artifact 能力边界**

Run:

```bash
sed -n '1,240p' tests/tinyui/runtime/check_tinyui_manual_window_artifact.py
rg -n "basic_widgets|settings_panel" docs/tinyui-serial/C-线人工窗口验收记录.md
```

Expected:

- 明确当前脚本只覆盖两个 demo。

- [ ] **Step 2: 把 H 线最小范围固定为两个 demo**

要求：

- 在 H 线文档中把最小范围写成 `basic_widgets` + `settings_panel`。
- 若保留 7-demo 扩张想法，只能写成 post-H candidate。

- [ ] **Step 3: 补齐/整理记录模板**

要求：

- 保证每条记录有平台、命令、artifact、观察结论、已知限制。
- 不让 artifact existence 代替人工结论。

- [ ] **Step 4: 文档 review**

要求：

- 独立 review 检查范围是否仍超出现有脚本能力。

### Task 7: H9-H10 public API 边界与发布测试矩阵冻结

**Files:**
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Optional Create: `docs/tinyui-serial/H-线发布测试矩阵.md`
- Reference: `tests/tinyui/contract/check_tinyui_public_api.py`
- Reference: `tests/tinyui/CMakeLists.txt`

- [ ] **Step 1: 汇总发布前必须跑的命令**

要求：

- `unit`
- `contract`
- `release matrix`
- `mapping`
- `visible`
- `manual artifact`
- `demo boundary`
- `public API`

- [ ] **Step 2: 写清命令顺序和互斥**

要求：

- 写明 `runtime/visible/mapping` 不并行抢 `build/tinyui-runtime`。
- 写明 `manual artifact` 不接入强制 CTest。

- [ ] **Step 3: 验证 public API gate 仍可用**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

Expected:

- 两个脚本通过。

### Task 8: H11 发布文档包收口

**Files:**
- Create: `docs/tinyui-serial/H-线第一版发布说明.md`
- Optional Create: `docs/tinyui-serial/H-线已支持控件清单.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`

- [ ] **Step 1: 写 release notes 初稿**

要求：

- supported controls
- known limitations
- build / test instructions
- evidence explanation

- [ ] **Step 2: 保证文档互链闭环**

要求：

- 从 H 线索引可跳到对比文档、release matrix、demo catalog、manual artifact 记录和发布说明。

- [ ] **Step 3: 文档自检**

Run:

```bash
git diff --check -- docs/tinyui-serial/H-线第一版发布说明.md \
                  docs/tinyui-serial/H-线已支持控件清单.md \
                  docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md
```

Expected:

- diff 无格式错误。

### Task 9: H12-H13 独立发布 review 与 closeout

**Files:**
- Optional Create: `docs/tinyui-serial/H-线发布closeout.md`
- Reference: 所有 H 线文档与 gate

- [ ] **Step 1: 运行发布前命令集**

Run:

```bash
ctest --test-dir build --output-on-failure -L unit
ctest --test-dir build --output-on-failure -L contract
ctest --test-dir build --output-on-failure -R check_tinyui_release_capability_matrix
ctest --test-dir build --output-on-failure -L mapping
ctest --test-dir build --output-on-failure -L visible
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
git diff --check
```

Expected:

- 自动 gate 全通过。
- `git diff --check` 无输出。

- [ ] **Step 2: 核对 manual artifact 记录**

要求：

- 确认两个 demo 的 artifact 记录与人工结论已落盘。
- 若缺任何字段，不允许 closeout。

- [ ] **Step 3: 独立 review subagent 只读审查**

要求：

- 审查发布说明是否夸大。
- 审查 release matrix 是否与代码/文档一致。
- 审查 known limitations 是否覆盖 `image/list/font/theme/manual artifact`。

- [ ] **Step 4: 写 closeout 文档**

要求：

- 只在 review 无 blocking 时写 `可作为 internal v0.1 candidate`。
- 不写成 `public v1.0`。

---

## 3. 计划完成判定

本计划完成时，应满足：

1. `H线` 有单独 spec 与单独 implementation plan。
2. 当前 `9` 控件的第一版本发布准备任务已被拆成 fresh subagent 可接手的串行任务。
3. `internal v0.1` 与 `public v1.0` 的边界不再混写。
4. `manual artifact`、`release matrix gate`、`demo catalog`、`独立 review` 都有明确承接任务。
