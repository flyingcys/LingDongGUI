# TinyUI v2.1 V0 Baseline And Rename Guards Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 冻结 `v2.0` 末态为 `v2.1` 的起跑基线，并建立 rename/目录/backend 退场守门，避免后续大规模迁移失控。

**Architecture:** 新增 `v2.1` inventory 与 Python guard checker，量化当前 `picoui/`、`tinyui/`、`backend_*`、`picoui_*` 的真实分布，并把文档索引接到新的 `docs/v2.1` 真相源上。

**Tech Stack:** Python 3、JSON、CTest、现有 `tests/picoui/contract/*`、现有 `docs/v2.0/*` 与新 `docs/v2.1/*`。

---

## 文件结构

新增：

- `tests/picoui/contract/check_tinyui_v21_transition_guards.py`
- `tests/picoui/contract/tinyui_v21_transition_inventory.json`
- `docs/v2.1/2026-06-10-tinyui-v2-1-baseline-inventory.md`
- `docs/v2.1/线计划索引.md`

修改：

- `tests/picoui/CMakeLists.txt`
- `docs/v2.1/plans/stages/README.md`

---

### Task 1: 建立 v2.1 inventory 与 guard checker

**Files:**
- Create: `tests/picoui/contract/check_tinyui_v21_transition_guards.py`
- Create: `tests/picoui/contract/tinyui_v21_transition_inventory.json`
- Modify: `tests/picoui/CMakeLists.txt`

- [x] **Step 1: 写 baseline guard checker，并支持打印当前实测值**

Create `tests/picoui/contract/check_tinyui_v21_transition_guards.py`:

```python
#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests" / "picoui" / "contract" / "tinyui_v21_transition_inventory.json"

PICOUI_DIR = ROOT / "picoui"
TINYUI_DIR = ROOT / "tinyui"
BACKEND_DIR = ROOT / "picoui" / "src" / "backend" / "ldgui"
PICOUI_HEADERS = sorted((ROOT / "picoui" / "include").rglob("*.h"))
TINYUI_HEADERS = sorted((ROOT / "tinyui" / "include").rglob("*.h"))

def count_prefix(headers: list[Path], prefix: str) -> int:
    pattern = re.compile(rf"\\b{re.escape(prefix)}[A-Za-z0-9_]*\\s*\\(")
    total = 0
    for header in headers:
        text = header.read_text(encoding="utf-8")
        total += len(pattern.findall(text))
    return total

def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] == "--print-current":
        actual = {
            "picoui_dir_exists": PICOUI_DIR.exists(),
            "tinyui_dir_exists": TINYUI_DIR.exists(),
            "backend_c_files": len(sorted(BACKEND_DIR.glob("backend_*.c"))),
            "picoui_public_api_count": count_prefix(PICOUI_HEADERS, "picoui_"),
            "tinyui_public_api_count": count_prefix(PICOUI_HEADERS + TINYUI_HEADERS, "tinyui_"),
        }
        print(json.dumps(actual, ensure_ascii=False, sort_keys=True, indent=2))
        return 0
    if not INVENTORY.exists():
        print(f"missing inventory: {INVENTORY}", file=sys.stderr)
        return 1
    inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
    actual = {
        "picoui_dir_exists": PICOUI_DIR.exists(),
        "tinyui_dir_exists": TINYUI_DIR.exists(),
        "backend_c_files": len(sorted(BACKEND_DIR.glob("backend_*.c"))),
        "picoui_public_api_count": count_prefix(PICOUI_HEADERS, "picoui_"),
        "tinyui_public_api_count": count_prefix(PICOUI_HEADERS + TINYUI_HEADERS, "tinyui_"),
    }
    for key, expected in inv["baseline"].items():
        if actual[key] != expected:
            print(f"baseline drift: {key} expected {expected} actual {actual[key]}", file=sys.stderr)
            return 1
    print("tinyui v2.1 transition baseline guard OK")
    print(json.dumps(actual, ensure_ascii=False, sort_keys=True, indent=2))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
```

- [x] **Step 2: 先打印当前实测基线**

Run:

```bash
python3 tests/picoui/contract/check_tinyui_v21_transition_guards.py --print-current
```

Expected: 输出当前工作树的真实基线 JSON；这一步不做通过/失败判定，只采集基线。

- [x] **Step 3: 用实测值建立 inventory**

Create `tests/picoui/contract/tinyui_v21_transition_inventory.json` from the exact JSON emitted in Step 2:

```json
{
  "baseline": {
    "picoui_dir_exists": true,
    "tinyui_dir_exists": true,
    "backend_c_files": 35,
    "picoui_public_api_count": 559,
    "tinyui_public_api_count": 34
  }
}
```

Only use the exact measured values from Step 2. Do not hand-write guessed numbers.

- [x] **Step 4: 在 CTest 注册 guard**

Add to `tests/picoui/CMakeLists.txt`:

```cmake
ld_add_python_test(check_tinyui_v21_transition_guards
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_v21_transition_guards.py"
    LABELS "picoui;contract;tinyui_v21;transition"
)
```

- [x] **Step 5: 跑 guard 到通过**

Run:

```bash
python3 tests/picoui/contract/check_tinyui_v21_transition_guards.py
rtk ctest --test-dir build -R '^check_tinyui_v21_transition_guards$' --output-on-failure
```

Expected: PASS。

### Task 2: 建立 v2.1 baseline 文档与索引

**Files:**
- Create: `docs/v2.1/2026-06-10-tinyui-v2-1-baseline-inventory.md`
- Create: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [x] **Step 1: 写 baseline inventory 文档**

Create `docs/v2.1/2026-06-10-tinyui-v2-1-baseline-inventory.md` with:

- 当前顶层产品目录：`picoui/` 与试点 `tinyui/`
- 当前 shared layers：`picoui/src/core`、`display`、`indev`、`layout`、`theme`、`tick`
- 当前 backend 目录：`picoui/src/backend/ldgui`
- 当前 widget 目录：`picoui/src/widgets`
- 当前 contract/test/perf 目录：`tests/picoui/*`
- 当前 broad gates：
  - `rtk ctest --test-dir build -L 'picoui' --output-on-failure`
  - `rtk ctest --test-dir build -L 'perf' --output-on-failure`
  - `rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
  - `git diff --check`

- [x] **Step 2: 建立顶层索引**

Create `docs/v2.1/线计划索引.md` with sections:

- 当前设计真相源
- 当前机器真相源入口
- 当前基线清单
- 阅读顺序
- 固定顺序 `V0 -> V1 -> V2 -> V3 -> V4 -> V5`
- 当前进度（初始状态写为 `V0` 正在建立）

- [x] **Step 3: 在阶段 README 记录 V0 closeout 要点**

Append under `V0` in `docs/v2.1/plans/stages/README.md`:

- `v2.1` baseline inventory 已建立
- `rename/backend/api` transition guard 已注册
- 后续阶段统一服从 `v2.1` inventory，不再各自发明起跑状态

- [x] **Step 4: 跑 V0 closeout gate**

Run:

```bash
rtk ctest --test-dir build -R 'check_tinyui_v21_transition_guards|check_picoui_tinyui_transition_guards' --output-on-failure
git diff --check
```

Expected: PASS。
