# TinyUI v2.2 S2 Backend Header Retirement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 按真实 shared/widget 边界拆散 `tinyui/src/backend/ldgui/backend.h`，清理所有依赖，并最终删除该文件。

**Architecture:** 先按 `S0` inventory 把 `backend.h` 内容分成 shared internal、runtime state、layout/window private、widget-local helper 四类；每类落到明确归属后，再清理 include 依赖并删除 `backend.h`。禁止改名保留另一份总头。

**Tech Stack:** C11、TinyUI core/runtime/layout/widgets、CMake、CTest。

---

## 文件结构

修改：

- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/core/internal.h`
- `tinyui/src/core/runtime_bridge.c`
- `tinyui/src/core/runtime_host.c`
- `tinyui/src/widgets/*`
- `tinyui/src/layout/*`
- `tinyui/src/core/*`
- `cmake/LingDongGUI.cmake`

可能新增：

- `tinyui/src/core/runtime_internal.h`
- 其他确有必要的 subsystem-private header

删除：

- `tinyui/src/backend/ldgui/backend.h`

---

### Task 1: 把 `backend.h` shared 内容迁入正确落点

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/core/internal.h`
- Create or Modify: `tinyui/src/core/runtime_internal.h`
- Modify: `tinyui/src/layout/*`
- Modify: `tinyui/src/widgets/window.c`

- [ ] **Step 1: 写 fail-first include dependency scan**

Run:

```bash
rg -n 'backend\.h' tinyui/src tinyui/include tinyui/demo
```

Expected: 当前仍有 direct include 命中。

- [ ] **Step 2: 迁 shared enum / truth policy / signal**

Move the following categories out of `backend.h` into shared internal headers:

- widget kind
- backend signal
- data truth policy
- data value source

Landing rule:

- cross-widget shared definitions -> `tinyui/src/core/internal.h`

- [ ] **Step 3: 迁 runtime/app/backend state**

Move:

- backend app/runtime state
- runtime evidence flags
- other runtime-only shared structs

Landing rule:

- runtime-only shared definitions -> `tinyui/src/core/runtime_internal.h` or equivalent runtime-private header

- [ ] **Step 4: 迁 window/layout cache**

Move:

- layout window state
- layout child state

Landing rule:

- `window` / `layout` private ownership, not core mega-header

- [ ] **Step 5: 跑 focused compile proof**

Run:

```bash
rtk cmake --build build --target test_tinyui_layout test_tinyui_runtime_model test_tinyui_widgets
```

Expected: PASS。

### Task 2: 清理 widget 对 `backend.h` 的 direct include 依赖

**Files:**
- Modify: `tinyui/src/widgets/*`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/runtime_internal.h`

- [ ] **Step 1: 逐批改 widget include**

Replace direct `#include "../backend/ldgui/backend.h"` with only the headers each widget truly needs.

End state requirement:

- no widget source directly includes `backend.h`
- widget-local helper declarations remain local

- [ ] **Step 2: 把单文件 helper 改成 `static`**

For any helper previously visible only because `backend.h` existed, convert it to `static` if it is single-file owned.

- [ ] **Step 3: 跑 focused widget proof**

Run:

```bash
rtk ctest --test-dir build --output-on-failure -R '^(test_tinyui_widgets|test_tinyui_layout|test_tinyui_runtime_model)$'
```

Expected: PASS。

### Task 3: 删除 `backend.h`

**Files:**
- Delete: `tinyui/src/backend/ldgui/backend.h`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/v2.2/线计划索引.md`
- Modify: `docs/v2.2/plans/stages/README.md`

- [ ] **Step 1: 删除文件并清理残留引用**

Delete:

```text
tinyui/src/backend/ldgui/backend.h
```

Then run:

```bash
rg -n 'backend\.h' tinyui
```

Expected: no hits in active source tree.

- [ ] **Step 2: 跑 configure/build**

Run:

```bash
rtk cmake -S . -B build
rtk cmake --build build
```

Expected: PASS。

- [ ] **Step 3: 更新 S2 文档真相**

Record in:

- `docs/v2.2/线计划索引.md`
- `docs/v2.2/plans/stages/README.md`

that:

- `backend.h` is gone
- no substitute mega-header was introduced
- remaining shared/internal boundaries are now explicit

- [ ] **Step 4: 跑 S2 closeout gate**

Run:

```bash
rg -n 'backend\.h' tinyui
git diff --check
```

Expected: zero active-source hits and format PASS。
