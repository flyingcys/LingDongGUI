# TinyUI v2.0 P0 Baseline And Guards Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 冻结当前 `picoui` 基线，为 `backend`/`app`/`tinyui_*` 迁移建立机器守门，并在结构改造前固定真相源。

**Architecture:** 新增一个迁移守门 checker 和一个机器可读 inventory，挂入 `tests/picoui/CMakeLists.txt` 与 `docs/v2.0` 索引，使后续各阶段都能量化 `backend`/`app` 是否真正下降，而不是靠主观判断。

**Tech Stack:** Python 3、CMake/CTest、JSON、现有 `tests/picoui/contract/*`、现有 `docs/v2.0/*`。

---

## 文件结构

新增：

- `tests/picoui/contract/check_picoui_tinyui_transition_guards.py`
- `tests/picoui/contract/picoui_tinyui_transition_inventory.json`
- `docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md`

修改：

- `tests/picoui/CMakeLists.txt`
- `docs/v2.0/线计划索引.md`
- `docs/v2.0/plans/stages/README.md`

---

### Task 1: 建立 backend/app/tinyui guard checker

**Files:**
- Create: `tests/picoui/contract/check_picoui_tinyui_transition_guards.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Create: `tests/picoui/contract/picoui_tinyui_transition_inventory.json`

- [ ] **Step 1: 写 fail-first guard checker**

Create `tests/picoui/contract/check_picoui_tinyui_transition_guards.py`:

```python
#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests" / "picoui" / "contract" / "picoui_tinyui_transition_inventory.json"

BACKEND_GLOB = sorted((ROOT / "picoui" / "src" / "backend" / "ldgui").glob("backend_*.c"))
APP_HEADER = ROOT / "picoui" / "include" / "picoui" / "app.h"
APP_SOURCE = ROOT / "picoui" / "src" / "core" / "app.c"
PUBLIC_HEADERS = sorted((ROOT / "picoui" / "include" / "picoui").glob("*.h"))

def count_api(prefix: str) -> int:
    pattern = re.compile(rf"\\b{re.escape(prefix)}[a-zA-Z0-9_]*\\s*\\(")
    total = 0
    for header in PUBLIC_HEADERS:
        text = header.read_text(encoding="utf-8")
        total += len(pattern.findall(text))
    return total

def main() -> int:
    if not INVENTORY.exists():
        print(f"missing inventory: {INVENTORY}", file=sys.stderr)
        return 1
    inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
    actual = {
        "backend_c_files": len(BACKEND_GLOB),
        "app_header_exists": APP_HEADER.exists(),
        "app_source_exists": APP_SOURCE.exists(),
        "picoui_public_api_count": count_api("picoui_"),
        "tinyui_public_api_count": count_api("tinyui_"),
    }
    for key, expected in inv["baseline"].items():
        if actual[key] != expected:
            print(f"baseline drift: {key} expected {expected} actual {actual[key]}", file=sys.stderr)
            return 1
    print("tinyui transition baseline guard OK")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
```

Create `tests/picoui/contract/picoui_tinyui_transition_inventory.json`:

```json
{
  "baseline": {
    "backend_c_files": 35,
    "app_header_exists": true,
    "app_source_exists": true,
    "picoui_public_api_count": 0,
    "tinyui_public_api_count": 0
  }
}
```

Note: in the first implementation pass, replace `picoui_public_api_count: 0` with the real count reported by the temporary failing run below.

- [ ] **Step 2: 运行 checker，拿真实 baseline 数**

Run:

```bash
python3 tests/picoui/contract/check_picoui_tinyui_transition_guards.py
```

Expected: FAIL 一次，因为 `picoui_public_api_count` 初始占位仍是 `0`。

- [ ] **Step 3: 用真实 baseline 数更新 inventory**

Update `tests/picoui/contract/picoui_tinyui_transition_inventory.json`:

```json
{
  "baseline": {
    "backend_c_files": 35,
    "app_header_exists": true,
    "app_source_exists": true,
    "picoui_public_api_count": 548,
    "tinyui_public_api_count": 0
  }
}
```

若当前 checkout 实测值与 `548` 不一致，以实测值为准，不要保留旧示例数。

- [ ] **Step 4: 在 CTest 注册 checker**

Add to `tests/picoui/CMakeLists.txt` near other contract tests:

```cmake
ld_add_python_test(check_picoui_tinyui_transition_guards
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_tinyui_transition_guards.py"
    LABELS "picoui;contract;transition"
)
```

- [ ] **Step 5: 跑 checker 到通过**

Run:

```bash
python3 tests/picoui/contract/check_picoui_tinyui_transition_guards.py
rtk ctest --test-dir build -R '^check_picoui_tinyui_transition_guards$' --output-on-failure
```

Expected: PASS。

### Task 2: 建立 v2.0 baseline inventory 文档

**Files:**
- Create: `docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md`
- Modify: `docs/v2.0/线计划索引.md`
- Modify: `docs/v2.0/plans/stages/README.md`

- [ ] **Step 1: 写基线清单文档**

Create `docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md`:

```md
# TinyUI v2.0 Baseline Inventory

## 当前共享层

- `picoui/src/backend/ldgui/backend_app.c`
- `picoui/src/backend/ldgui/backend_widget.c`
- `picoui/src/backend/ldgui/backend_widget_tree.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_layout.c`
- `picoui/src/backend/ldgui/backend_theme.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`

## 当前试点控件

- `picoui/src/widgets/window.c`
- `picoui/src/widgets/label.c`
- `picoui/src/widgets/button.c`
- `picoui/src/widgets/switch.c`

## 当前 public app 入口

- `picoui/include/picoui/app.h`
- `picoui/src/core/app.c`
- `picoui/demo/basic_widgets/main.c`

## 当前 broad gates

- `rtk ctest --test-dir build -L 'picoui' --output-on-failure`
- `rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
- `git diff --check`
```

- [ ] **Step 2: 把基线文档挂到索引**

Add to `docs/v2.0/线计划索引.md`:

```md
- 当前基线清单：`docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md`
```

- [ ] **Step 3: 把 P0 状态写入 stages README**

Add under `P0` in `docs/v2.0/plans/stages/README.md`:

```md
当前 closeout 要点：

- backend/app/tinyui 迁移 guard 已注册
- baseline inventory 文档已建立
- 后续 phase 不再各自发明 baseline
```

- [ ] **Step 4: 跑 P0 broad checks**

Run:

```bash
rtk ctest --test-dir build -R 'check_picoui_tinyui_transition_guards|check_picoui_public_api|check_picoui_demo_boundary' --output-on-failure
git diff --check
```

Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add \
  tests/picoui/CMakeLists.txt \
  tests/picoui/contract/check_picoui_tinyui_transition_guards.py \
  tests/picoui/contract/picoui_tinyui_transition_inventory.json \
  docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md \
  docs/v2.0/线计划索引.md \
  docs/v2.0/plans/stages/README.md
git commit -m "docs: add tinyui v2.0 baseline guards"
```
