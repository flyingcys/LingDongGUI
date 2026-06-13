# TINYUI a-0.11 Direct 100% Base Extension Gap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 固化 fresh subagent 重新审计结论，并把 `base` 下 `optional_public_extension=16` 明确拆成 strict 100% direct public API parity 的剩余缺口候选。

**Architecture:** `S0` 复核审计输入；`S1` 收口 ability 文档；`S2` 写入 a-0.11 serial/spec/plan；`S3` 做 gate 和 GitNexus closeout。本线只做文档和计划拆分，不实现新 public API。

**Tech Stack:** Markdown、Python3 contract checker、TINYUI public headers、LingDongGUI native API inventory、GitNexus

---

## 0. 执行规则

- 默认在当前主仓推进；若新建 worktree，必须放在 `.worktree/`，创建或切换后执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- 跨模块摸底与 review 必须使用独立 subagent。
- 本线不修改 C 源码、不新增 TINYUI public API、不改 matrix truth-source 统计。
- 若后续要实现 `optional_public_extension`，必须另开实现阶段，并在编辑函数/方法前跑 GitNexus impact。

## 1. 文件写面

### S1 ability wording closeout

**Modify:**
- `docs/ability/README.md`
- `docs/ability/base.md`

### S2 a-0.11 docs

**Create:**
- `docs/tinyui-serial/a-0.11-线计划索引.md`
- `docs/superpowers/specs/2026-06-02-tinyui-a-0-11-direct-100-base-extension-gap-design.md`
- `docs/superpowers/plans/2026-06-02-tinyui-a-0-11-direct-100-base-extension-gap-implementation.md`

## 2. Tasks

### Task S0: Fresh Audit Confirmation

**Owner:** Fresh subagent `SG-a0.11-S0-direct-100-audit`

**Goal:** 重新确认 TINYUI public API 和 `docs/ability` 当前是否达到 strict 100% direct public API parity。

- [x] **Step 1: Run read-only audit**

Audit scope:

```text
tinyui/include
tinyui/src
tests/tinyui/contract/ldgui_public_api_inventory.json
tests/tinyui/contract/native_api_gap_ledger.json
tests/tinyui/contract/tinyui_release_capability_matrix.json
docs/ability/*.md
```

Required judgement:

```text
covered 行 public API 是否都存在
docs/ability 是否暗示 direct 100% 已完成
allowlisted 行中是否有 strict 100% 目标下需要标成缺口候选的行
```

执行记录（2026-06-02）：fresh subagent 已完成只读审计。结论：未达到 `100% direct public API parity`；covered 行 public API 存在；唯一需补强文档表达的是 `base` 下 `optional_public_extension=16`。

- [x] **Step 2: Record confirmed candidate symbols**

Confirmed strict 100% candidate symbols:

```text
ldBaseAlignRegionCenter
ldBaseAutoVerticalGridAlign
ldBaseFocusNavigate
ldBaseFocusNavigateInit
ldBaseGetAbsoluteLocation
ldBaseGetAlignRegion
ldBaseGetChildCount
ldBaseGetChildList
ldBaseGetNameId
ldBaseGetNextSibling
ldBaseGetParent
ldBaseGetRelativeLocation
ldBaseGetRootNode
ldBaseGetWidget
ldBaseGetWidgetById
ldBaseGetWidgetType
```

执行记录（2026-06-02）：以上 16 个符号全部位于 `docs/ability/base.md`，当前 `direct_100_category=optional_public_extension`，不是 `covered`。

### Task S1: Ability Docs Wording Closeout

**Owner:** Main thread or docs-only worker `SG-a0.11-S1-ability-wording`

**Goal:** 强化 ability 文档，避免把 `direct_100_required_if_user_demands=0` 误读为 strict direct 100% 已完成。

- [x] **Step 1: Update `docs/ability/README.md`**

Add the strict 100% wording under Direct Public API conclusion:

```markdown
- 严格按“100% direct public API parity”目标看，`optional_public_extension=16` 仍是未暴露为 TINYUI public API 的剩余缺口候选，不是完成项。
- 当前 a-0.10 不新增 TINYUI public API；这些缺口候选集中在 `base` 的 tree/focus/nameId/geometry helper，后续若要真正 direct 100%，必须单独设计 public API、backend proof、unit/gate 与 matrix/docs。
```

Also update the shared `base` row:

```markdown
| `base` | 63 | [base](./base.md) | `allowlisted`: 30, `covered`: 33；严格 100% direct public API 缺口候选：`16` |
```

执行记录（2026-06-02）：已完成。

- [x] **Step 2: Update `docs/ability/base.md` summary**

Add:

```markdown
- 严格 100% direct public API 缺口候选：`16`；这些 `optional_public_extension` 行当前没有 TINYUI public API/backend/unit/gate，若目标是对外 direct 100%，必须单独补 public API 设计与实现。
```

执行记录（2026-06-02）：已完成。

- [x] **Step 3: Update `docs/ability/base.md` audit boundary**

Replace weak optional wording with:

```markdown
- `direct_100_category=optional_public_extension`：a-0.10 没有把它列为必做 public API；但严格按“100% direct public API parity”目标，它仍是未暴露为 TINYUI public API 的剩余缺口候选。
```

执行记录（2026-06-02）：已完成。

### Task S2: Create a-0.11 Serial Spec And Plan

**Owner:** Main thread

**Goal:** 将本次梳理正式整理到 a-0.11，并拆分 spec 与 plan。

- [x] **Step 1: Create serial index**

Create:

```text
docs/tinyui-serial/a-0.11-线计划索引.md
```

Required sections:

```text
a-0.11 线定位
a-0.11 做什么
a-0.11 不做什么
串行阶段
最小验收
完成定义
```

执行记录（2026-06-02）：已完成。

- [x] **Step 2: Create spec**

Create:

```text
docs/superpowers/specs/2026-06-02-tinyui-a-0-11-direct-100-base-extension-gap-design.md
```

Required sections:

```text
背景
目标
非目标
缺口候选清单
后续实现方向
验收
当前执行记录
```

执行记录（2026-06-02）：已完成。

- [x] **Step 3: Create plan**

Create:

```text
docs/superpowers/plans/2026-06-02-tinyui-a-0-11-direct-100-base-extension-gap-implementation.md
```

Required tasks:

```text
S0 Fresh Audit Confirmation
S1 Ability Docs Wording Closeout
S2 Create a-0.11 Serial Spec And Plan
S3 Gate And Closeout
S4 Future Implementation Handoff
```

执行记录（2026-06-02）：已完成。

### Task S3: Gate And Closeout

**Owner:** Main thread

**Goal:** 验证本线只强化文档口径，没有破坏 a-0.10 truth-source。

- [x] **Step 1: Run contract checkers**

Run:

```bash
python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py
python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

Expected:

```text
all exit 0
```

执行记录（2026-06-02）：三项 contract checker 均 exit 0。

- [x] **Step 2: Run docs/diff checks**

Run:

```bash
rg -n "严格.*100% direct public API|剩余缺口候选|optional_public_extension=16" docs/ability/README.md docs/ability/base.md docs/tinyui-serial/a-0.11-线计划索引.md docs/superpowers/specs/2026-06-02-tinyui-a-0-11-direct-100-base-extension-gap-design.md
git diff --check
```

Expected:

```text
rg finds the strict 100% gap wording
git diff --check exits 0
```

执行记录（2026-06-02）：strict 100% gap wording 搜索命中 `docs/ability/README.md`、`docs/ability/base.md`、a-0.11 index/spec；`git diff --check` exit 0。

- [x] **Step 3: Run GitNexus change detection**

Run:

```bash
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

```text
risk low
affected_processes empty or limited to docs/checker-neutral paths
```

执行记录（2026-06-02）：GitNexus detect_changes 返回 `risk_level=low`、`affected_count=0`、`affected_processes=[]`。GitNexus 只报告已索引的 ability 文档影响面；新增 a-0.11 markdown 文档不产生代码符号影响。

### Task S4: Future Implementation Handoff

**Owner:** Future worker, only if user asks to implement direct 100%

**Goal:** Split the 16 candidates into future implementation lanes without leaking LingDongGUI internals into TINYUI public API. This is a handoff record only; a-0.11 does not execute these implementation lanes.

- [x] **Step 1: Geometry helper design lane**

Candidate APIs:

```text
ldBaseAlignRegionCenter
ldBaseAutoVerticalGridAlign
ldBaseGetAbsoluteLocation
ldBaseGetAlignRegion
ldBaseGetRelativeLocation
```

Decision required before code:

```text
Design portable TINYUI geometry/alignment helpers or explicitly keep them policy-only.
```

执行记录（2026-06-02）：已拆成 future-only lane；本线不设计或实现 public API。

- [x] **Step 2: Focus navigation design lane**

Candidate APIs:

```text
ldBaseFocusNavigate
ldBaseFocusNavigateInit
```

Decision required before code:

```text
Design TINYUI focus/navigation public API without exposing ld_scene_t directly.
```

执行记录（2026-06-02）：已拆成 future-only lane；本线不设计或实现 public API。

- [x] **Step 3: Tree traversal design lane**

Candidate APIs:

```text
ldBaseGetChildCount
ldBaseGetChildList
ldBaseGetNextSibling
ldBaseGetParent
ldBaseGetRootNode
```

Decision required before code:

```text
Design TINYUI widget tree traversal API or reject public traversal with explicit policy.
```

执行记录（2026-06-02）：已拆成 future-only lane；本线不设计或实现 public API。

- [x] **Step 4: Name/type lookup design lane**

Candidate APIs:

```text
ldBaseGetNameId
ldBaseGetWidget
ldBaseGetWidgetById
ldBaseGetWidgetType
```

Decision required before code:

```text
Design portable widget id/name/type API without leaking ldWidgetType_t or scene macros.
```

执行记录（2026-06-02）：已拆成 future-only lane；本线不设计或实现 public API。

## 3. Self-review checklist

- [x] Spec requirement “record fresh audit result” is covered by S0/S2.
- [x] Spec requirement “ability docs stronger strict 100% wording” is covered by S1.
- [x] Spec requirement “a-0.11 serial/spec/plan split” is covered by S2.
- [x] Spec requirement “do not implement new API in this line” is preserved by file write scope.
- [x] Spec requirement “future implementation handoff” is covered by S4.
- [x] Verification evidence is recorded after S3 runs.
