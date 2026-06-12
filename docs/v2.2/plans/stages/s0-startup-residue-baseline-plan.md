# TinyUI v2.2 S0 Startup Residue Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 冻结 `v2.2` 的启动残留面、`backend.h` 依赖面和 demo 启动残留面，给后续结构改动提供稳定起跑边界。

**Architecture:** 先用源码扫描和 focused file inventory 建立 `v2.2` 当前真相，再把结果固化到 `docs/v2.2` 索引与阶段 README。`S0` 不做架构修改，只做 inventory、guard 和执行边界锁定。

**Tech Stack:** `rg`、C11 源码扫描、Markdown、现有 TinyUI runtime/core/demo tree。

---

## 文件结构

新增：

- `docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`

修改：

- `docs/v2.2/线计划索引.md`
- `docs/v2.2/plans/stages/README.md`

---

### Task 1: 冻结 startup / app / demo 残留面

**Files:**
- Create: `docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`
- Read: `tinyui/include/runtime.h`
- Read: `tinyui/include/app.h`
- Read: `tinyui/src/core/app.c`
- Read: `tinyui/demo/*`

- [ ] **Step 1: 扫描 startup 与 demo 残留面**

Run:

```bash
rg -n 'picoui_app_create|picoui_app_run|run_demo\(|int main\(' tinyui/include tinyui/src tinyui/demo
```

Expected: 输出当前旧 app 主路径、demo 自带 `main()/run_demo()` 的真实命中。

- [ ] **Step 2: 盘点主线 demo 文件列表**

Run:

```bash
rg --files tinyui/demo | sort
```

Expected: 输出当前 demo 文件清单，供后续标注哪些目录要从 `main.c` 形态迁到 `build API` 形态。

- [ ] **Step 3: 写 startup baseline inventory 文档**

Create `docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`，至少包含：

- 当前 canonical 启动相关头/源：
  - `tinyui/include/runtime.h`
  - `tinyui/include/app.h`
  - `tinyui/src/core/app.c`
  - `tinyui/src/core/runtime_host.c`
  - `tinyui/src/core/runtime_bridge.c`
- 当前 demo 启动真相：
  - 哪些 demo 目录仍有 `main.c`
  - 哪些 demo 仍保留 `run_demo()`
  - 哪些 demo 已直接依赖 `picoui_app_*`
- 当前目标终态：
  - demo `.c` 只导出 build API
  - `tinyui/demo/main.c` 作为统一 runner
  - 切 demo 靠手工替换 build API

- [ ] **Step 4: 跑格式 gate**

Run:

```bash
git diff --check -- docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md
```

Expected: PASS。

### Task 2: 冻结 `backend.h` 依赖与拆分 inventory

**Files:**
- Create: `docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`
- Read: `tinyui/src/backend/ldgui/backend.h`
- Read: `tinyui/src/widgets/*`
- Read: `tinyui/src/core/internal.h`

- [ ] **Step 1: 扫描直接 include `backend.h` 的调用面**

Run:

```bash
rg -n 'backend\.h' tinyui/src tinyui/include tinyui/demo
```

Expected: 输出所有直接 include 残留，作为 `S2` 的起跑边界。

- [ ] **Step 2: 记录 `backend.h` 内容分层建议**

Append to `docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`:

- shared enum / signal / truth policy
- backend widget tree / runtime state
- window/layout cache
- widget-local helper / constructor / bridge

For each category, note the intended landing zone:

- `core` internal
- `runtime` internal
- `layout/window` private
- widget-local `.c` / private header

- [ ] **Step 3: 标记禁止回退边界**

Append to the same inventory doc:

- 不允许新建另一份 mega-header 取代 `backend.h`
- 不允许把 widget-local helper 再提升成 shared 总入口

- [ ] **Step 4: 跑 residue scan 复核**

Run:

```bash
rg -n 'backend\.h' tinyui
```

Expected: 输出稳定、可审计，供 `S2` 之后对比收窄。

### Task 3: 更新 S0 文档真相

**Files:**
- Modify: `docs/v2.2/线计划索引.md`
- Modify: `docs/v2.2/plans/stages/README.md`

- [ ] **Step 1: 更新索引中的当前进度**

Record that:

- `S0` baseline inventory 已建立
- startup / `backend.h` / demo residue 面已冻结
- `S1-S3` 不得再重新定义起跑边界

- [ ] **Step 2: 更新阶段 README 的 S0 closeout 要点**

Append under `S0` in `docs/v2.2/plans/stages/README.md`:

- startup baseline inventory 已建立
- `backend.h` include 残留面已冻结
- demo `main/run_demo/app_run` 残留面已冻结

- [ ] **Step 3: 跑 S0 closeout gate**

Run:

```bash
rg -n 'backend\.h|picoui_app_create|picoui_app_run|run_demo\(|int main\(' tinyui
git diff --check
```

Expected: 输出稳定且格式 PASS。
