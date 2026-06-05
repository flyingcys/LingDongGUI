# PicoUI v1.0.1 P0 Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Freeze current facts and decisions before native runtime implementation starts.

**Architecture:** This phase is documentation and contract inventory only. It creates machine-readable truth sources used by all later subagents.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P0: 基线冻结与决策

### Task P0-A: 建立 v1.0 基线目录与冻结报告骨架

**Files:**
- Create: `docs/picoui-serial/v1.0-native/00-基线冻结.md`
- Modify: `docs/picoui-serial/v1.0-native/线计划索引.md`

- [ ] **Step 1: 写冻结报告骨架**

写入 `docs/picoui-serial/v1.0-native/00-基线冻结.md`：

```markdown
# PicoUI v1.0 Native P0 基线冻结

> 日期：2026-06-05
> 阶段：P0
> 状态：进行中

## 目标

冻结 v1.0 native-only 迁移前的当前事实，作为后续 subagent 执行和 review 的共同输入。

## 冻结清单

- Public API header inventory
- `picoui_app_*` compatibility decision
- Demo main style inventory
- Current ldgui backend dependency inventory
- Current contract/runtime gate inventory
- Native migration ledger seed

## 完成定义

- `tests/picoui/contract/picoui_native_migration_ledger.json` 已生成。
- `tests/picoui/contract/check_picoui_native_migration_ledger.py` 可检查 ledger schema。
- `picoui_app_*` 每个 public API 都有明确 `delete | compat_wrapper | replace` 决策。
- `picoui/demo/*/main.c` 每个 demo 都有入口风格分类。
- 后续任务不得再把 pre-v1.0 wrapper/backend 文档当当前路线。
```

- [ ] **Step 2: 更新线索引 P0 状态**

在 `docs/picoui-serial/v1.0-native/线计划索引.md` 的阶段列表后添加：

```markdown
## 当前阶段状态

- 当前阶段：`P0`
- 当前任务：`P0-A` 基线冻结报告骨架
- 下一任务：`P0-B` public API 与 app compatibility inventory
```

- [ ] **Step 3: 验证**

Run:

```bash
rtk git diff --check
rtk rg -n "P0-A|基线冻结|picoui_app_\\*" docs/picoui-serial/v1.0-native
```

Expected:

- `git diff --check` exit 0。
- `rg` 命中 P0 状态和 compatibility 文案。

- [ ] **Step 4: Commit**

```bash
git add docs/picoui-serial/v1.0-native/00-基线冻结.md docs/picoui-serial/v1.0-native/线计划索引.md
git commit -m "docs: start picoui v1 native baseline"
```

### Task P0-B: Public API 与 app compatibility inventory

**Files:**
- Create: `tests/picoui/contract/picoui_public_api_inventory_v1.json`
- Create: `tests/picoui/contract/picoui_app_compatibility_decisions.json`
- Create: `tests/picoui/contract/check_picoui_app_compatibility_decisions.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.0-native/00-基线冻结.md`

- [ ] **Step 1: 写 RED contract**

新增 `tests/picoui/contract/check_picoui_app_compatibility_decisions.py`：

```python
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DECISIONS = ROOT / "tests/picoui/contract/picoui_app_compatibility_decisions.json"
REQUIRED = {
    "picoui_app_create",
    "picoui_app_run",
    "picoui_app_run_background",
    "picoui_app_set_window",
    "picoui_app_set_background",
    "picoui_app_switch_window",
    "picoui_app_switch_background",
    "picoui_app_timer_create",
    "picoui_app_timer_start",
    "picoui_app_timer_stop",
    "picoui_app_timer_is_running",
    "picoui_app_timer_destroy",
    "picoui_app_destroy",
}
VALID_DECISIONS = {"delete", "compat_wrapper", "replace"}


def main() -> int:
    assert DECISIONS.is_file(), f"missing {DECISIONS.relative_to(ROOT)}"
    data = json.loads(DECISIONS.read_text(encoding="utf-8"))
    rows = data.get("picoui_app_apis")
    assert isinstance(rows, list), "picoui_app_apis must be a list"

    by_name = {row.get("name"): row for row in rows}
    missing = sorted(REQUIRED - set(by_name))
    assert not missing, f"missing app API decisions: {missing}"

    for name in sorted(REQUIRED):
        row = by_name[name]
        decision = row.get("decision")
        replacement = row.get("replacement")
        rationale = row.get("rationale")
        assert decision in VALID_DECISIONS, f"{name} has invalid decision {decision!r}"
        assert isinstance(rationale, str) and rationale.strip(), f"{name} missing rationale"
        if decision == "replace":
            assert isinstance(replacement, str) and replacement.startswith("picoui_"), (
                f"{name} replacement must name a picoui_* API"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

在 `tests/picoui/CMakeLists.txt` 的 contract 区域加入：

```cmake
ld_add_python_test(check_picoui_app_compatibility_decisions
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_app_compatibility_decisions.py"
    LABELS "picoui;contract;native"
)
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_app_compatibility_decisions --output-on-failure
```

Expected:

- FAIL，原因是 `picoui_app_compatibility_decisions.json` 不存在。

- [ ] **Step 4: 生成 compatibility decision JSON**

新增 `tests/picoui/contract/picoui_app_compatibility_decisions.json`：

```json
{
  "schema_version": 1,
  "route": "v1.0-native",
  "picoui_app_apis": [
    {
      "name": "picoui_app_create",
      "decision": "compat_wrapper",
      "replacement": "picoui_init",
      "rationale": "v1.0+ 主路径不暴露 app handle；短期保留兼容入口，委托 runtime 初始化。"
    },
    {
      "name": "picoui_app_run",
      "decision": "compat_wrapper",
      "replacement": "picoui_timer_handler",
      "rationale": "v1.0+ 主循环由用户周期调用 timer handler；旧 run 只作为兼容包装。"
    },
    {
      "name": "picoui_app_run_background",
      "decision": "compat_wrapper",
      "replacement": "picoui_screen_load",
      "rationale": "background 在 v1.0+ 收敛为 screen/window 加载语义。"
    },
    {
      "name": "picoui_app_set_window",
      "decision": "replace",
      "replacement": "picoui_screen_load",
      "rationale": "screen load 是 LVGL-like root 切换入口。"
    },
    {
      "name": "picoui_app_set_background",
      "decision": "replace",
      "replacement": "picoui_screen_load",
      "rationale": "background 作为 root screen/window 能力迁移到 screen load。"
    },
    {
      "name": "picoui_app_switch_window",
      "decision": "replace",
      "replacement": "picoui_screen_load",
      "rationale": "v1.0.1 首版先用 screen load 表达切换；动画模式进入后续 screen transition。"
    },
    {
      "name": "picoui_app_switch_background",
      "decision": "replace",
      "replacement": "picoui_screen_load",
      "rationale": "background switch 合并到 screen load/transition。"
    },
    {
      "name": "picoui_app_timer_create",
      "decision": "replace",
      "replacement": "picoui_timer_create",
      "rationale": "timer 属于 runtime service，不绑定 public app handle。"
    },
    {
      "name": "picoui_app_timer_start",
      "decision": "replace",
      "replacement": "picoui_timer_start",
      "rationale": "timer start 迁移到 runtime timer API。"
    },
    {
      "name": "picoui_app_timer_stop",
      "decision": "replace",
      "replacement": "picoui_timer_stop",
      "rationale": "timer stop 迁移到 runtime timer API。"
    },
    {
      "name": "picoui_app_timer_is_running",
      "decision": "replace",
      "replacement": "picoui_timer_is_running",
      "rationale": "timer readback 迁移到 runtime timer API。"
    },
    {
      "name": "picoui_app_timer_destroy",
      "decision": "replace",
      "replacement": "picoui_timer_delete",
      "rationale": "timer lifecycle 迁移到 runtime timer API。"
    },
    {
      "name": "picoui_app_destroy",
      "decision": "compat_wrapper",
      "replacement": "picoui_deinit",
      "rationale": "v1.0+ 释放由 runtime deinit 管理；旧 destroy 保留兼容包装。"
    }
  ]
}
```

- [ ] **Step 5: 生成 public API inventory JSON**

新增 `tests/picoui/contract/picoui_public_api_inventory_v1.json`：

```json
{
  "schema_version": 1,
  "route": "v1.0-native",
  "truth_source": "picoui/include/picoui",
  "notes": [
    "This inventory is seeded in P0 and expanded by later stages.",
    "picoui_app_* APIs are not the v1.0+ primary path."
  ],
  "required_v1_public_entrypoints": [
    "picoui_init",
    "picoui_deinit",
    "picoui_timer_handler",
    "picoui_display_create",
    "picoui_display_set_flush_cb",
    "picoui_display_set_buffers",
    "picoui_display_set_default",
    "picoui_indev_create",
    "picoui_indev_set_type",
    "picoui_indev_set_read_cb",
    "picoui_screen_active",
    "picoui_screen_create",
    "picoui_screen_load"
  ]
}
```

- [ ] **Step 6: 更新冻结报告**

在 `00-基线冻结.md` 添加：

```markdown
## `picoui_app_*` compatibility 决策

机器真相源：

- `tests/picoui/contract/picoui_app_compatibility_decisions.json`
- `tests/picoui/contract/check_picoui_app_compatibility_decisions.py`

当前结论：

- `picoui_app_create()`、`picoui_app_run()`、`picoui_app_destroy()` 可短期作为 compatibility wrapper。
- v1.0+ quick start、demo 主路径、新测试不得使用 `picoui_app_*`。
- timer/window/background 相关能力迁移到 runtime timer 与 screen API。
```

- [ ] **Step 7: 运行 GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_app_compatibility_decisions --output-on-failure
rtk git diff --check
```

Expected:

- `check_picoui_app_compatibility_decisions` PASS。
- `git diff --check` exit 0。

- [ ] **Step 8: Commit**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/contract/check_picoui_app_compatibility_decisions.py \
        tests/picoui/contract/picoui_app_compatibility_decisions.json \
        tests/picoui/contract/picoui_public_api_inventory_v1.json \
        docs/picoui-serial/v1.0-native/00-基线冻结.md
git commit -m "test: freeze picoui app compatibility"
```

### Task P0-C: Demo main style inventory

**Files:**
- Create: `tests/picoui/contract/picoui_demo_main_style_inventory.json`
- Create: `tests/picoui/contract/check_picoui_demo_main_style_inventory.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.0-native/00-基线冻结.md`

- [ ] **Step 1: 写 RED contract**

新增 `tests/picoui/contract/check_picoui_demo_main_style_inventory.py`：

```python
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui/demo"
INVENTORY = ROOT / "tests/picoui/contract/picoui_demo_main_style_inventory.json"


def main() -> int:
    assert INVENTORY.is_file(), f"missing {INVENTORY.relative_to(ROOT)}"
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    rows = data.get("demos")
    assert isinstance(rows, list), "demos must be a list"
    by_name = {row.get("name"): row for row in rows}
    actual = sorted(path.parent.name for path in DEMO_DIR.glob("*/main.c"))
    missing = [name for name in actual if name not in by_name]
    assert not missing, f"missing demo inventory rows: {missing}"
    for name in actual:
        row = by_name[name]
        style = row.get("main_style")
        assert style in {"pre_v1_app_run", "v1_lvgl_like"}, f"{name} invalid main_style={style!r}"
        uses = row.get("uses_picoui_app")
        assert isinstance(uses, bool), f"{name} uses_picoui_app must be bool"
        migration = row.get("migration_phase")
        assert isinstance(migration, str) and migration.startswith("P"), f"{name} invalid migration_phase"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

在 `tests/picoui/CMakeLists.txt` 加入：

```cmake
ld_add_python_test(check_picoui_demo_main_style_inventory
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_main_style_inventory.py"
    LABELS "picoui;contract;native"
)
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_demo_main_style_inventory --output-on-failure
```

Expected:

- FAIL，原因是 inventory JSON 不存在。

- [ ] **Step 4: 生成 inventory JSON**

用当前 demo 清单生成 `tests/picoui/contract/picoui_demo_main_style_inventory.json`。初始内容允许全部标 `pre_v1_app_run`，但 `basic_widgets` 的 `migration_phase` 必须是 `P1`，其余是 `P6`。

```json
{
  "schema_version": 1,
  "route": "v1.0-native",
  "reference": "third_party/lv_port_pc_vscode/main/src/main.c",
  "demos": [
    {"name": "animation_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "arc_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "basic_widgets", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P1"},
    {"name": "calendar_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "clock_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "combo_box_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "date_time_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "gauge_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "graph_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "grid_parity", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "hello_world", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "icon_slider_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "keyboard_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "layout_flex", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "layout_grid", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "layout_parity", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "legacy_widget_parity", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "line_edit_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "list_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "message_box_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "progress_bar_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "progress_wheel_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "qrcode_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "radial_menu_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "scroll_selecter_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "settings_panel", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "table_basic", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"},
    {"name": "theme_showcase", "main_style": "pre_v1_app_run", "uses_picoui_app": true, "migration_phase": "P6"}
  ]
}
```

- [ ] **Step 5: 更新冻结报告**

在 `00-基线冻结.md` 添加：

```markdown
## Demo main style inventory

机器真相源：

- `tests/picoui/contract/picoui_demo_main_style_inventory.json`
- `tests/picoui/contract/check_picoui_demo_main_style_inventory.py`

当前结论：

- `basic_widgets` 是 P1 样板迁移目标。
- 其余 demo 在 P6 按样板批量迁移。
```

- [ ] **Step 6: GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_demo_main_style_inventory --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 7: Commit**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/contract/check_picoui_demo_main_style_inventory.py \
        tests/picoui/contract/picoui_demo_main_style_inventory.json \
        docs/picoui-serial/v1.0-native/00-基线冻结.md
git commit -m "test: freeze picoui demo main inventory"
```

### Task P0-D: ldgui dependency inventory

**Files:**
- Create: `tests/picoui/contract/picoui_ldgui_dependency_inventory.json`
- Create: `tests/picoui/contract/check_picoui_ldgui_dependency_inventory.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.0-native/00-基线冻结.md`

- [ ] **Step 1: 写 RED contract**

新增 `tests/picoui/contract/check_picoui_ldgui_dependency_inventory.py`：

```python
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests/picoui/contract/picoui_ldgui_dependency_inventory.json"
REQUIRED_GROUPS = {"picoui_core", "picoui_backend_ldgui", "picoui_backend_ldgui_runtime", "picoui_demo_targets"}


def main() -> int:
    assert INVENTORY.is_file(), f"missing {INVENTORY.relative_to(ROOT)}"
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    groups = {row.get("name") for row in data.get("targets", [])}
    missing = sorted(REQUIRED_GROUPS - groups)
    assert not missing, f"missing dependency target groups: {missing}"
    for row in data["targets"]:
        assert isinstance(row.get("currently_links_ldgui"), bool), f"{row.get('name')} missing bool"
        assert row.get("v1_target") in {"remove_dependency", "legacy_opt_in", "native_only"}, (
            f"{row.get('name')} invalid v1_target"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

加入 `tests/picoui/CMakeLists.txt`：

```cmake
ld_add_python_test(check_picoui_ldgui_dependency_inventory
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_ldgui_dependency_inventory.py"
    LABELS "picoui;contract;native"
)
```

- [ ] **Step 3: 运行 RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_ldgui_dependency_inventory --output-on-failure
```

Expected: FAIL，inventory 不存在。

- [ ] **Step 4: 写 inventory**

新增 `tests/picoui/contract/picoui_ldgui_dependency_inventory.json`：

```json
{
  "schema_version": 1,
  "route": "v1.0-native",
  "targets": [
    {
      "name": "picoui_core",
      "currently_links_ldgui": false,
      "v1_target": "native_only",
      "notes": "Current core includes backend headers and calls backend API, but ldgui objects are linked through backend targets."
    },
    {
      "name": "picoui_backend_ldgui",
      "currently_links_ldgui": true,
      "v1_target": "legacy_opt_in",
      "notes": "Legacy wrapper backend remains only for oracle/compat until P7."
    },
    {
      "name": "picoui_backend_ldgui_runtime",
      "currently_links_ldgui": true,
      "v1_target": "legacy_opt_in",
      "notes": "Runtime wrapper target must not be default after P7."
    },
    {
      "name": "picoui_demo_targets",
      "currently_links_ldgui": true,
      "v1_target": "remove_dependency",
      "notes": "Demo targets migrate to native runtime and must not link ldgui backend by v1.0.1."
    }
  ]
}
```

- [ ] **Step 5: 更新冻结报告**

在 `00-基线冻结.md` 添加 dependency inventory 章节，引用上面两个文件。

- [ ] **Step 6: GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_ldgui_dependency_inventory --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 7: Commit**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/contract/check_picoui_ldgui_dependency_inventory.py \
        tests/picoui/contract/picoui_ldgui_dependency_inventory.json \
        docs/picoui-serial/v1.0-native/00-基线冻结.md
git commit -m "test: freeze picoui ldgui dependency inventory"
```

### Task P0-E: native migration ledger seed

**Files:**
- Create: `tests/picoui/contract/picoui_native_migration_ledger.json`
- Create: `tests/picoui/contract/check_picoui_native_migration_ledger.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `docs/picoui-serial/v1.0-native/00-基线冻结.md`

- [ ] **Step 1: 写 RED contract**

新增 `tests/picoui/contract/check_picoui_native_migration_ledger.py`：

```python
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
LEDGER = ROOT / "tests/picoui/contract/picoui_native_migration_ledger.json"
REQUIRED_PHASES = {"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"}
VALID_STATUS = {"not_started", "in_progress", "covered", "non_user_capability"}


def main() -> int:
    assert LEDGER.is_file(), f"missing {LEDGER.relative_to(ROOT)}"
    data = json.loads(LEDGER.read_text(encoding="utf-8"))
    entries = data.get("entries")
    assert isinstance(entries, list) and entries, "entries must be a non-empty list"
    phases = {entry.get("phase") for entry in entries}
    missing = sorted(REQUIRED_PHASES - phases)
    assert not missing, f"ledger missing phase coverage: {missing}"
    for entry in entries:
        assert entry.get("status") in VALID_STATUS, f"{entry.get('id')} invalid status"
        assert isinstance(entry.get("capability"), str) and entry["capability"], "missing capability"
        assert isinstance(entry.get("evidence"), list), f"{entry.get('id')} evidence must be list"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 2: 注册 RED contract**

加入 `tests/picoui/CMakeLists.txt`：

```cmake
ld_add_python_test(check_picoui_native_migration_ledger
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_native_migration_ledger.py"
    LABELS "picoui;contract;native;release"
)
```

- [ ] **Step 3: RED**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_native_migration_ledger --output-on-failure
```

Expected: FAIL，ledger 不存在。

- [ ] **Step 4: 写 ledger seed**

新增 `tests/picoui/contract/picoui_native_migration_ledger.json`：

```json
{
  "schema_version": 1,
  "route": "v1.0-native",
  "entries": [
    {"id": "P1-runtime-init", "phase": "P1", "capability": "LVGL-like runtime init/deinit", "status": "not_started", "evidence": []},
    {"id": "P1-display-indev-screen", "phase": "P1", "capability": "Display, input device, and active screen without public app handle", "status": "not_started", "evidence": []},
    {"id": "P1-window-label-button", "phase": "P1", "capability": "Native window, label, button vertical slice", "status": "not_started", "evidence": []},
    {"id": "P1-basic-widgets-main-style", "phase": "P1", "capability": "basic_widgets main follows LVGL-like structure", "status": "not_started", "evidence": []},
    {"id": "P2-widget-tree-layout-theme", "phase": "P2", "capability": "Native widget tree, layout, and theme", "status": "not_started", "evidence": []},
    {"id": "P3-basic-widgets", "phase": "P3", "capability": "Native checkbox, switch, slider, text, image, list, background", "status": "not_started", "evidence": []},
    {"id": "P4-extended-widgets", "phase": "P4", "capability": "Native extended widget set", "status": "not_started", "evidence": []},
    {"id": "P5-resource-font-image", "phase": "P5", "capability": "Native resource, font, image, text rendering", "status": "not_started", "evidence": []},
    {"id": "P6-demo-port-artifact", "phase": "P6", "capability": "Demo main migration, SDL port, native artifacts", "status": "not_started", "evidence": []},
    {"id": "P7-remove-ldgui-runtime-dependency", "phase": "P7", "capability": "Default PicoUI runtime does not link ldgui backend", "status": "not_started", "evidence": []},
    {"id": "P8-release-gate", "phase": "P8", "capability": "v1.0.1 native-only release gate", "status": "not_started", "evidence": []}
  ]
}
```

- [ ] **Step 5: 更新冻结报告**

加入 ledger 章节，说明所有后续阶段必须更新 `status` 和 `evidence`。

- [ ] **Step 6: GREEN**

Run:

```bash
rtk ctest --test-dir build -R check_picoui_native_migration_ledger --output-on-failure
rtk git diff --check
```

Expected: PASS。

- [ ] **Step 7: Commit**

```bash
git add tests/picoui/CMakeLists.txt \
        tests/picoui/contract/check_picoui_native_migration_ledger.py \
        tests/picoui/contract/picoui_native_migration_ledger.json \
        docs/picoui-serial/v1.0-native/00-基线冻结.md
git commit -m "test: seed picoui native migration ledger"
```

### Task P0-F: P0 review and closeout

**Files:**
- Modify: `docs/picoui-serial/v1.0-native/00-基线冻结.md`
- Modify: `docs/picoui-serial/v1.0-native/线计划索引.md`

- [ ] **Step 1: 运行 P0 verification**

Run:

```bash
rtk ctest --test-dir build -R 'check_picoui_app_compatibility_decisions|check_picoui_demo_main_style_inventory|check_picoui_ldgui_dependency_inventory|check_picoui_native_migration_ledger' --output-on-failure
rtk git diff --check
```

Expected: all listed tests PASS。

- [ ] **Step 2: GitNexus detect changes**

Run from main thread:

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"all"})
```

Expected:

- Changes limited to docs and contract files.
- No production runtime symbol impact.

- [ ] **Step 3: 更新 closeout**

在 `00-基线冻结.md` 添加：

```markdown
## P0 closeout

状态：完成

验证：

- `ctest -R app_compatibility|demo_main_style|ldgui_dependency|native_migration_ledger` 通过。
- `git diff --check` 通过。
- GitNexus detect_changes 已复核为文档/contract 范围。

下一阶段：`P1` native 垂直切片。
```

- [ ] **Step 4: 更新线索引**

将 `线计划索引.md` 当前阶段改为：

```markdown
## 当前阶段状态

- 当前阶段：`P1`
- 当前任务：`P1-A` runtime public API RED contract
- 已完成：`P0`
```

- [ ] **Step 5: Commit**

```bash
git add docs/picoui-serial/v1.0-native/00-基线冻结.md docs/picoui-serial/v1.0-native/线计划索引.md
git commit -m "docs: close picoui native baseline"
```

---
