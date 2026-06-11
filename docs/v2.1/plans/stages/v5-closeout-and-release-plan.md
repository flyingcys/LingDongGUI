# TinyUI v2.1 V5 Closeout And Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 清零剩余 `picoui` 产品层痕迹，完成 `v2.1` closeout/release 真相收口，并给出最终完成态。

**Architecture:** V5 不再改路线，只做 residue cleanup、closeout 文档、release matrix、performance baseline 迁移、final gates 与 release-facing proof。完成态必须严格证明：产品层没有 `picoui`，独立 backend 已消失，`LingDongGUI` 目录未动。

**Tech Stack:** Markdown、Python checker、CTest、现有 runtime/visible/perf gates。

---

## 文件结构

新增：

- `docs/v2.1/v2.1-closeout.md`
- `docs/v2.1/v2.1-release-matrix.md`
- `docs/v2.1/v2.1-performance-baseline.md`

修改：

- `docs/v2.1/线计划索引.md`
- `docs/v2.1/plans/stages/README.md`
- `docs/v2.1/2026-06-10-tinyui-v2-1-design.md`
- renamed `tests/tinyui/*` checkers

---

### Task 1: 清零产品层 `picoui` 残留

**Files:**
- Modify or delete any final product-layer residue still matching `picoui`

- [ ] **Step 1: 跑最终 residue scan**

Run:

```bash
rg -n "picoui" tinyui tests cmake docs/v2.1
```

Expected: either zero hits or only explicitly allowed historical references outside the product-layer truth surface.

- [ ] **Step 2: 清理剩余命名残留**

For every remaining product-layer hit found in Step 1:

- rename it to `tinyui`, or
- delete it if it is obsolete migration residue

Do not touch:

- `LingDongGUI` directory names
- `ld*` engine API names
- historical `docs/v2.0/*` records that remain as prior-version truth

### Task 2: 建立 v2.1 closeout truth

**Files:**
- Create: `docs/v2.1/v2.1-closeout.md`
- Create: `docs/v2.1/v2.1-release-matrix.md`
- Create: `docs/v2.1/v2.1-performance-baseline.md`

- [ ] **Step 1: 写 closeout 文档**

Document:

- what is completed
- what `v2.1` explicitly proves
- what is still outside scope
- the final gate commands

- [ ] **Step 2: 写 release matrix**

Document at least:

- top-level directory unified to `tinyui/`
- product public API unified to `tinyui_*`
- independent backend removed
- shared layers retained intentionally
- tests/contracts/CMake renamed to `tinyui`
- `LingDongGUI` directories and `ld*` API left unchanged

- [ ] **Step 3: 迁移 performance baseline 文档**

Move the `v2.0` performance truth into `v2.1` naming without weakening:

- binary size guard
- runtime perf guard
- wrapper struct overhead guard
- blocking policy

### Task 3: 跑 final gates 并更新索引

**Files:**
- Modify: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [ ] **Step 1: 跑 final broad gates**

Run:

```bash
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
rtk ctest --test-dir build -L 'perf' --output-on-failure
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure
git diff --check
```

Expected: PASS。

- [ ] **Step 2: 更新顶层索引**

Record in `docs/v2.1/线计划索引.md`:

- `V0-V5` completion
- final truth sources
- remaining non-goals

- [ ] **Step 3: 更新阶段 README**

Record in `docs/v2.1/plans/stages/README.md`:

- `v2.1` 已完成 closeout / release truth
- completion means no product-layer `picoui` remains
- completion still does not rename `LingDongGUI`
