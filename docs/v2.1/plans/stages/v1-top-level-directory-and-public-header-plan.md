# TinyUI v2.1 V1 Top-Level Directory And Public Header Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让产品顶层目录只剩唯一 `tinyui/`，并把 public include 树与入口头文件统一收口到 `tinyui`。

**Architecture:** 先完成目录层面的原地演化，再收口 public include 路径、顶层 umbrella header 和 include path，保留内部实现暂时仍可通过旧测试树编译，但不再保留双顶层目录并行。

**Tech Stack:** Git move/rename、CMake include path、C11 头文件、现有 `tinyui/include` 与试点 `tinyui/include`。

---

## 文件结构

新增：

- `tinyui/include/tinyui.h`（若最终位置调整后需要重建）

修改：

- 顶层目录：`tinyui/ -> tinyui/`
- `tinyui/include/*`
- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt`
- `docs/v2.1/线计划索引.md`
- `docs/v2.1/plans/stages/README.md`

删除：

- 旧并行试点 `tinyui/include/*` 树（被统一吸收后删除旧位置）

---

### Task 1: 收口顶层产品目录

**Files:**
- Move: `tinyui/` -> `tinyui/`
- Delete later: legacy parallel `tinyui/include/*` source locations if duplicated

- [x] **Step 1: 写 fail-first 目录 contract**

Extend `tests/tinyui/contract/check_tinyui_v21_transition_guards.py` so V1 closeout expects:

- `tinyui_dir_exists == false`
- `tinyui_dir_exists == true`
- `tinyui/include/*` contains the unified public include tree

- [x] **Step 2: 运行 checker，确认 V1 前为 fail**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py
```

Expected: FAIL，因为当前仍同时存在 `tinyui/` 与试点 `tinyui/`。

- [x] **Step 3: 执行目录迁移**

Perform the top-level directory move so product-layer source lives under a single `tinyui/` root.

Required end state after the move:

- `tinyui/include/*`
- `tinyui/src/*`
- `tinyui/demo/*`
- no parallel product root named `tinyui/`

- [x] **Step 4: 吸收试点 include 树**

Merge the current pilot `tinyui/include/*` content into the unified `tinyui/include/*` tree, removing duplicate parallel file locations and preserving only one canonical public header path.

- [x] **Step 5: 回填 inventory**

Update `tests/tinyui/contract/tinyui_v21_transition_inventory.json`:

```json
{
  "baseline": {
    "tinyui_dir_exists": false,
    "tinyui_dir_exists": true,
    "backend_c_files": 35,
    "tinyui_public_api_count": 559,
    "tinyui_public_api_count": 39
  }
}
```

Use measured counts if they changed.

- [x] **Step 6: 跑目录 gate**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py
git diff --check
```

Expected: PASS。

### Task 2: 收口 public header 与 include path

**Files:**
- Modify: unified `tinyui/include/*`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `tests/tinyui/CMakeLists.txt`

- [x] **Step 1: 统一 umbrella header**

Ensure the canonical public umbrella header is:

```c
#ifndef TINYUI_H
#define TINYUI_H

#include "core.h"
#include "obj.h"
#include "screen.h"
#include "label.h"
#include "button.h"
#include "switch.h"

#endif
```

and that the canonical product umbrella header is `tinyui/include/tinyui.h`.

At the end of `V1`, `tinyui/include/tinyui/*` may still remain as a temporary
compatibility subtree so existing product-layer callers can keep compiling.
Removing that compatibility umbrella belongs to later naming-cleanup phases,
not `V1`.

- [x] **Step 2: 更新 include path**

Update build scripts and tests so product-layer include roots resolve through `tinyui/include` only.

Concrete places to update:

- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt` or its renamed successor once V4 migrates test directories

- [x] **Step 3: 跑 focused compile proof**

Run:

```bash
rtk cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build --target test_tinyui_runtime_model test_tinyui_switch
```

Expected: PASS，说明目录迁移后最小 runtime/widget 入口仍可编译。

- [x] **Step 4: 更新 V1 文档真相**

Update:

- `docs/v2.1/线计划索引.md`
- `docs/v2.1/plans/stages/README.md`

to record that the product top-level directory is now unified under `tinyui/`, while backend/test/demo naming migration remains for later phases.
