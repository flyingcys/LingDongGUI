# TINYUI H线 Review Blockers Repair Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复 H 线独立 review 暴露的 3 个当前 blocker，使 release matrix gate、list 合同单测和发布文档口径重新与真实状态对齐。

**Architecture:** 本计划只修 review 已确认的最小问题，不扩 H 线范围、不新增新控件能力。执行顺序固定为：先补 gate 强度，再收紧 list 单测合同，最后同步文档口径与验证记录。

**Tech Stack:** Python3 gate script、C unit test、Markdown serial docs、CTest、GitNexus、LingDongGUI

---

## 0. 执行规则

- 每次只推进一个 Task。
- 代码改动前先记录 GitNexus impact 结果；若索引未命中私有 helper，明确按文件级低风险脚本改动处理。
- 所有代码改动遵守 TDD：先写/改 failing test，再看它按预期失败，再写最小实现。
- review 不通过时，由同一个执行 subagent 原地修复，不开新修复线程。
- 提交前必须运行 `git diff --check`、对应测试命令、`gitnexus_detect_changes(scope="all")`。

## 1. 文件结构与写面分组

### release matrix 修复组

**文件：**
- Modify: `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- Reference: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

职责：

- 补齐 `wrapped` 控件状态断言。
- 补齐 `summary.capability_status_counts` 与实际 capability 条目一致性校验。

### list 合同测试修复组

**文件：**
- Modify: `tests/tinyui/unit/test_tinyui_list.c`

职责：

- 删除跨层指针同一性假合同。
- 仅保留 H 线当前需要的 “item id 不是 widget/backend identity” 约束。

### 文档收口组

**文件：**
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线第一版发布说明.md`
- Modify: `docs/tinyui-serial/H-线发布测试矩阵.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `docs/tinyui-serial/C-线人工窗口验收记录.md`

职责：

- 把 review blockers 和修复状态写进 H 线真相源。
- 去掉未固化的桌面截图现状表述。

## 2. 串行任务拆分

### Task 1: 修复 release matrix gate 强度

**Files:**
- Modify: `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- Reference: `tests/tinyui/contract/tinyui_release_capability_matrix.json`

- [ ] **Step 1: 记录 impact 与当前基线**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
ctest --test-dir build --output-on-failure -R check_tinyui_release_capability_matrix
```

Expected:

- 当前 gate 通过，作为修复前基线。
- 任务说明里记录：GitNexus 对该私有 helper 未命中，按单脚本低风险改动处理。

- [ ] **Step 2: 先写会失败的 gate 测试思路并手工触发 RED**

做法：

- 临时复制 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 到临时文件。
- 在临时文件里把一个 wrapped 控件改成 `deferred`，再把 `summary.capability_status_counts.support` 改错。
- 让脚本支持读取临时路径，或在测试步骤里临时替换目标文件后运行脚本，确认当前版本不会报错。

Run:

```bash
cp tests/tinyui/contract/tinyui_release_capability_matrix.json /tmp/tinyui_release_capability_matrix.json
# 手工把 /tmp 中一个 wrapped 控件改成 deferred，并改错 summary 统计
PICOUI_RELEASE_MATRIX_JSON=/tmp/tinyui_release_capability_matrix.json \
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

Expected:

- 当前脚本错误地仍然通过，证明 RED 成立。

- [ ] **Step 3: 写最小实现**

要求：

- 为 `EXPECTED_WRAPPED_WIDGETS` 逐个断言 `widget_status == "wrapped"`。
- 逐个断言 wrapped 控件当前 `widget_release_judgement == "internal_v0_1_known_limitation"`。
- 从全部 `widgets[].capabilities[]` 实际计数，校验 `summary.capability_status_counts` 与 capability 条目一致。
- 校验 `summary.capability_entry_total` 等于 capability 实际总数。

- [ ] **Step 4: 跑 GREEN**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
ctest --test-dir build --output-on-failure -R check_tinyui_release_capability_matrix
PICOUI_RELEASE_MATRIX_JSON=/tmp/tinyui_release_capability_matrix.json \
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

Expected:

- 正常 matrix 通过。
- 篡改后的临时 matrix 失败，且报错点落在 wrapped 状态或 summary 统计不一致。

### Task 2: 收紧 list 单测合同

**Files:**
- Modify: `tests/tinyui/unit/test_tinyui_list.c`

- [ ] **Step 1: 记录 impact 与当前测试基线**

Run:

```bash
ctest --test-dir build --output-on-failure -R test_tinyui_list
```

Expected:

- 当前 `test_tinyui_list` 通过，作为修复前基线。
- GitNexus 对 `tests/tinyui/unit/test_tinyui_list.c:main` 上游风险为 `LOW`。

- [ ] **Step 2: 先把测试改成更小合同并看 RED**

要求：

- 删除以下跨层指针同一性断言：
  - `list->items[0].id == list->backend_item_ids[0]`
  - `list->items[0].id == backend->list_item_ids[0]`
  - `list->items[1].id == list->backend_item_ids[1]`
  - `list->items[1].id == backend->list_item_ids[1]`
- 保留并强化 “item id 不等于 list/widget/backend identity” 这一层合同。
- 若需要 RED，可新增一条更精确断言名或注释说明当前合同边界。

Run:

```bash
ctest --test-dir build --output-on-failure -R test_tinyui_list
```

Expected:

- 测试保持通过，且合同边界更准确。

- [ ] **Step 3: 补最小注释或命名收紧**

要求：

- 只在测试函数前补 1 行短注释，明确本测试只约束 “TINYUI item id 不是 backend widget identity”，不约束 backend 内部存储策略。

- [ ] **Step 4: 跑 GREEN**

Run:

```bash
ctest --test-dir build --output-on-failure -R test_tinyui_list
ctest --test-dir build --output-on-failure -L unit
```

Expected:

- `test_tinyui_list` 通过。
- `unit` 集合通过。

### Task 3: 同步 H 线文档与验证记录

**Files:**
- Modify: `docs/tinyui-serial/H-线计划索引.md`
- Modify: `docs/tinyui-serial/H-线第一版发布说明.md`
- Modify: `docs/tinyui-serial/H-线发布测试矩阵.md`
- Modify: `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `docs/tinyui-serial/C-线人工窗口验收记录.md`

- [ ] **Step 1: 写入 review blockers 与修复后 current state**

要求：

- `H-线计划索引.md` 明确 H12 当前 blocking 及其修复状态。
- `H-线发布差距与LingDongGUI控件对比.md` 明确 release matrix gate 当前已补强到什么程度。
- `H-线第一版发布说明.md` 只保留仓库内可复现证据；桌面截图只能写成可选补强路径。

- [ ] **Step 2: 收紧 H10 测试矩阵表述**

要求：

- 删除或改写所有把 `Darwin + cocoa` 自动桌面截图写成“当前已固定现状”的句子。
- 保留“若未来补入脚本/产物记录，仍只属于 visible 层补强”。

- [ ] **Step 3: 更新 manual artifact 记录**

要求：

- 保留 `2026-05-30` 两条 `ARTIFACT_READY` 记录。
- 人工结论继续写“待人工观察”，不强化。

- [ ] **Step 4: 文档自检**

Run:

```bash
git diff --check -- docs/tinyui-serial/H-线计划索引.md \
                  docs/tinyui-serial/H-线第一版发布说明.md \
                  docs/tinyui-serial/H-线发布测试矩阵.md \
                  docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md \
                  docs/tinyui-serial/C-线人工窗口验收记录.md
rg -n "Darwin \\+ cocoa.*桌面窗口截图|自动桌面窗口截图存在" docs/tinyui-serial
```

Expected:

- `git diff --check` 无输出。
- 不再把桌面截图写成当前已固定证据。

### Task 4: 总体验证与收口

**Files:**
- Verify only

- [ ] **Step 1: 跑修复后目标验证**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
ctest --test-dir build --output-on-failure -R check_tinyui_release_capability_matrix
ctest --test-dir build --output-on-failure -R test_tinyui_list
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
git diff --check
```

Expected:

- 所有目标验证通过。

- [ ] **Step 2: 跑扩面验证**

Run:

```bash
ctest --test-dir build --output-on-failure -L unit
ctest --test-dir build --output-on-failure -L contract
ctest --test-dir build --output-on-failure -L tinyui
```

Expected:

- `unit`、`contract`、`tinyui` 全通过。

- [ ] **Step 3: detect_changes 与 review**

Run:

```text
gitnexus_detect_changes(scope="all")
```

Expected:

- 影响面集中在 H 线 release gate、list unit test、文档真相源。
- 独立 review 若无新 blocking，再把 H12 current-state 更新为“review blockers 已修复，待 closeout”。
