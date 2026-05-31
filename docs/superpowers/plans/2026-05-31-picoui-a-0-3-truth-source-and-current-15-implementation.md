# PicoUI a-0.3 truth-source 重建与 current-15 审计 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 只完成 `a-0.3` 本阶段工作：重建 current truth-source、完成 current-15 capability audit、冻结 `a-0.3` closeout 标准。当前不展开 `a-0.4+` 的详细 spec 和 plan。

**Architecture:** 执行顺序固定为 `R1 -> R2 -> R3 -> R4 -> R5`。其中 `R1 / R2 / R4 / R5` 严格串行，`R3` 采用并行 subagent：按 `J线 4 / a-01 5 / a-02 6` 三组拆开做 capability audit，主线程最后统一收敛 matrix、gate 和文档。review 独立，repair 回原执行 subagent。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 本阶段只做 `a-0.3`
- 不创建 `a-0.4 / a-0.5 / a-0.6` 的详细 spec 或 detailed implementation plan
- 涉及 `tests/picoui/contract/*.py` 或 `*.json` 的符号修改前，先做 GitNexus impact
- `review / repair / audit` 必须独立子任务
- 每个阶段结束都要同步索引、spec、plan、真相源文档
- 所有 markdown 统一中文

## 0.1 执行拓扑

### 串行阶段

- `R1`
- `R2`
- `R4`
- `R5`

### 并行阶段

- `R3-J-Audit`
- `R3-a01-Audit`
- `R3-a02-Audit`
- `R3-Independent-Review`

### 主线程强制保留职责

- current truth 总数决策
- matrix 主结构收敛
- gate 用词收敛
- 最终 closeout 标准冻结

## 1. 文件结构

### Task R1: current truth-source rebuild

**Files:**
- Modify: `docs/picoui-serial/a-0.3/README.md`
- Create: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`

### Task R2: current-15 parity stratification

**Files:**
- Modify: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- Create: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

### Task R3: current-15 capability audit

**Files:**
- Modify: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

### Task R4: truth-source / docs / gate consistency repair

**Files:**
- Modify: `docs/picoui-serial/a-0.3/README.md`
- Modify: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- Modify: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-0-3-truth-source-and-current-15-design.md`

### Task R5: a-0.3 closeout 标准冻结

**Files:**
- Create: `docs/picoui-serial/a-0.3/a-0.3-closeout-标准.md`
- Modify: `docs/picoui-serial/a-0.3/README.md`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-0-3-truth-source-and-current-15-design.md`
- Modify: `docs/superpowers/plans/2026-05-31-picoui-a-0-3-truth-source-and-current-15-implementation.md`

## 2. Tasks

### Task R1: 重建 current truth-source

**Files:**
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Create: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="check_picoui_release_capability_matrix", direction="upstream", repo="LingDongGUI")
```

Expected:

- 风险不高，影响面主要在 contract gate 和文档真相源

- [ ] **Step 2: 核对 current public widget 集**

Run:

```bash
for f in picoui/include/picoui/*.h; do
  bn=$(basename "$f")
  case "$bn" in
    app.h|layout.h|theme.h|widget.h|picoui.h) continue ;;
  esac
  echo "$bn"
done
```

Expected:

- 得到 current PicoUI public widget 清单
- 当前应至少识别到 `15` 个 widget

- [ ] **Step 3: 写真相源文档**

在 `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md` 写清：

```md
# Current-15 覆盖与分层真相源

- 当前 public widget 列表
- 对应 LingDongGUI 26 控件映射
- 当前未覆盖控件
- 旧 J1 matrix 过时原因
```

- [ ] **Step 4: 更新 matrix 到 current truth**

Requirements:

- `picoui_wrapped_widget_total` 从旧 `9` 改到 current truth
- 纳入 `progress_bar / qrcode / progress_wheel / message_box / date_time / clock`
- 先诚实写当前层级，禁止直接把六个 `a-02` 控件全写成 `parity_complete`

- [ ] **Step 5: 更新 gate**

Requirements:

- `check_picoui_release_capability_matrix.py` 必须校验：
  - current widget 总数
  - current uncovered widget 总数
  - current-15 新增控件存在
  - 不允许 minimal slice 伪装为 parity complete

- [ ] **Step 6: 验证**

Run:

```bash
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected:

- matrix gate 通过
- truth-source 与 current 文档一致

### Task R2: current-15 分层

**Files:**
- Modify: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- Create: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 固定三层中间态**

在 audit 文档固定：

```md
1. full parity complete
2. stable contract but not full parity
3. minimal vertical slice only
```

- [ ] **Step 2: 把 current-15 逐个归层**

必须覆盖三组来源：

```md
- J线 4 控件
- a-01 5 控件
- a-02 6 控件
```

- [ ] **Step 3: 回写 matrix**

Requirements:

- current-15 每个控件都要写 current layer
- 中间态标签只能表达当前状态，不能偷换成最终完成

- [ ] **Step 4: 验证**

Run:

```bash
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
rg -n "full parity complete|stable contract|minimal vertical slice" docs/picoui-serial/a-0.3
git diff --check
```

### Task R3: current-15 capability audit

**Execution mode:** 并行 subagent

**Parallel workers:**
- `R3-J-Audit`
- `R3-a01-Audit`
- `R3-a02-Audit`

**Serial merge owner:** 主线程

**Files:**
- Modify: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 逐控件写 capability checklist**

每个控件至少回答：

```md
- public API 到哪里
- props/create_with_props 到哪里
- getter/readback 到哪里
- backend mapping 到哪里
- 已完成项
- 未完成项
```

- [ ] **Step 2: 把主要缺口回写 matrix**

Requirements:

- 不需要在本阶段补完所有 capability
- 但必须把 current major gaps 诚实写出来

- [ ] **Step 2.1: 并行拆分 subagent 写面**

固定拆分：

```md
- R3-J-Audit: 只写 J线 4
- R3-a01-Audit: 只写 a-01 5
- R3-a02-Audit: 只写 a-02 6
```

硬规则：

```md
- 不改 README
- 不改 closeout 标准
- 不重写 matrix 总数
- 只提交各自控件组 audit 结论
```

- [ ] **Step 2.2: 主线程串行合并 audit 结果**

Requirements:

- 主线程统一把三组 audit 合并进
  - `current-15-capability-audit.md`
  - `picoui_release_capability_matrix.json`
- 若三组对同一层级定义冲突，主线程裁决，不交给 subagent 各自决定

- [ ] **Step 3: 审核**

Run:

```bash
rg -n "未完成|限制|缺口" docs/picoui-serial/a-0.3/current-15-capability-audit.md
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected:

- current-15 没有哪个控件处于“状态不明”

### Task R4: truth-source / docs / gate 一致性修复

**Execution mode:** 严格串行

**Files:**
- Modify: `docs/picoui-serial/a-0.3/README.md`
- Modify: `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- Modify: `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-0-3-truth-source-and-current-15-design.md`

- [ ] **Step 1: 对齐术语**

术语必须统一：

```md
- current-15
- full parity complete
- stable contract but not full parity
- minimal vertical slice only
```

- [ ] **Step 2: 消灭冲突口径**

检查并修正：

```md
- README
- truth-source 文档
- capability audit 文档
- spec
- matrix
- gate
```

- [ ] **Step 3: 验证**

Run:

```bash
rg -n "current-15|full parity complete|stable contract but not full parity|minimal vertical slice only" docs/picoui-serial/a-0.3 docs/superpowers/specs tests/picoui/contract
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R5: 冻结 a-0.3 closeout 标准

**Execution mode:** 严格串行

**Files:**
- Create: `docs/picoui-serial/a-0.3/a-0.3-closeout-标准.md`
- Modify: `docs/picoui-serial/a-0.3/README.md`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-0-3-truth-source-and-current-15-design.md`
- Modify: `docs/superpowers/plans/2026-05-31-picoui-a-0-3-truth-source-and-current-15-implementation.md`

- [ ] **Step 1: 写 closeout 标准**

至少覆盖：

```md
- a-0.3 必交文档
- a-0.3 必过 gate
- a-0.3 完成时必须能回答的问题
- a-0.3 明确不等于什么
```

- [ ] **Step 2: 回写索引与 spec**

Requirements:

- README 必须能直接跳转到 closeout 标准
- spec 的成功/失败判定要与 closeout 标准一致

- [ ] **Step 3: 总体验证**

Run:

```bash
rg -n "closeout|完成标准|不等于" docs/picoui-serial/a-0.3 docs/superpowers/specs docs/superpowers/plans
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

## 3. 自检清单

- [ ] 当前 truth-source 是否已不再停留在旧 `9` 控件
- [ ] current-15 是否全部进入 truth-source
- [ ] current-15 是否全部完成 capability audit
- [ ] `R3` 是否按 `J / a01 / a02` 并行拆开
- [ ] 串行阶段是否没有被并行误改
- [ ] matrix / gate / 文档术语是否一致
- [ ] 是否已冻结 `a-0.3` closeout 标准
- [ ] 是否明确 `a-0.3` 只做本阶段，不展开后续 detailed plan

## 4. 执行交接

Plan complete and saved to `docs/superpowers/plans/2026-05-31-picoui-a-0-3-truth-source-and-current-15-implementation.md`. Two execution options:

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

Which approach?
