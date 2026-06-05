# PicoUI v1.0.1 P6 Demo Port Artifact Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Attach the native SDL port, migrate demo mains, and establish runtime/visible artifacts.

**Architecture:** This phase changes developer-facing demo startup to the LVGL-like model. Each demo migration is one isolated subagent task.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P6: Demo / Port / Artifact

### Task P6-A: SDL native port attach

Files: `picoui/port/sdl/sdl.c`, `picoui/include/picoui/port/sdl.h`, `tests/picoui/native/test_picoui_native_sdl_port.c`, `docs/picoui-serial/v1.0-native/06-demo-port验证.md`.
Steps:

- [ ] RED: initialize SDL port with width/height, assert display and pointer indev are registered.
- [ ] Implement `picoui_sdl_hal_init(width, height)` matching LVGL reference `hal_init(320, 480)`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_sdl_port --output-on-failure`.
- [ ] Commit `feat: add picoui native sdl hal init`.

### Task P6-B: batch demo main contract expansion

Files: `tests/picoui/contract/check_picoui_demo_main_style.py`, inventory JSON.

- [ ] Expand contract from `basic_widgets` to all v1.0 demos.
- [ ] RED command: `rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py`.
- [ ] Expected: FAIL on remaining `picoui_app_*` demo mains.
- [ ] Commit RED contract and inventory with `git commit -m "test: require lvgl-like picoui demo mains"`.

### Task P6-C through P6-Z: migrate each demo main

One subagent per demo. Each task may edit exactly one `picoui/demo/<demo>/main.c` plus `tests/picoui/contract/picoui_demo_main_style_inventory.json`.

Each demo main must follow this skeleton:

```c
int main(void)
{
    if (picoui_init() != 0) {
        return 1;
    }
    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }
    create_demo_ui();
    while (1) {
        int rc = picoui_timer_handler();
        if (rc > 0) {
            break;
        }
        if (rc < 0) {
            picoui_deinit();
            return 1;
        }
    }
    picoui_deinit();
    return 0;
}
```

Task order and commit subjects:

- `P6-C hello_world`: build `picoui_hello_world_demo`; commit `refactor: migrate hello world demo main`.
- `P6-D layout_flex`: build `picoui_layout_flex_demo`; commit `refactor: migrate layout flex demo main`.
- `P6-E layout_grid`: build `picoui_layout_grid_demo`; commit `refactor: migrate layout grid demo main`.
- `P6-F theme_showcase`: build `picoui_theme_showcase_demo`; commit `refactor: migrate theme showcase demo main`.
- `P6-G settings_panel`: build `picoui_settings_panel_demo`; commit `refactor: migrate settings panel demo main`.
- `P6-H list_basic`: build `picoui_list_basic_demo`; commit `refactor: migrate list basic demo main`.
- `P6-I progress_bar_basic`: build `picoui_progress_bar_basic_demo`; commit `refactor: migrate progress bar demo main`.
- `P6-J progress_wheel_basic`: build `picoui_progress_wheel_basic_demo`; commit `refactor: migrate progress wheel demo main`.
- `P6-K qrcode_basic`: build `picoui_qrcode_basic_demo`; commit `refactor: migrate qrcode demo main`.
- `P6-L message_box_basic`: build `picoui_message_box_basic_demo`; commit `refactor: migrate message box demo main`.
- `P6-M date_time_basic`: build `picoui_date_time_basic_demo`; commit `refactor: migrate date time demo main`.
- `P6-N clock_basic`: build `picoui_clock_basic_demo`; commit `refactor: migrate clock demo main`.
- `P6-O line_edit_basic`: build `picoui_line_edit_basic_demo`; commit `refactor: migrate line edit demo main`.
- `P6-P combo_box_basic`: build `picoui_combo_box_basic_demo`; commit `refactor: migrate combo box demo main`.
- `P6-Q scroll_selecter_basic`: build `picoui_scroll_selecter_basic_demo`; commit `refactor: migrate scroll selecter demo main`.
- `P6-R table_basic`: build `picoui_table_basic_demo`; commit `refactor: migrate table demo main`.
- `P6-S graph_basic`: build `picoui_graph_basic_demo`; commit `refactor: migrate graph demo main`.
- `P6-T calendar_basic`: build `picoui_calendar_basic_demo`; commit `refactor: migrate calendar demo main`.
- `P6-U animation_basic`: build `picoui_animation_basic_demo`; commit `refactor: migrate animation demo main`.
- `P6-V legacy_widget_parity`: build `picoui_legacy_widget_parity_demo`; commit `refactor: migrate legacy widget parity demo main`.
- `P6-W layout_parity`: build `picoui_layout_parity_demo`; commit `refactor: migrate layout parity demo main`.
- `P6-X grid_parity`: build `picoui_grid_parity_demo`; commit `refactor: migrate grid parity demo main`.
- `P6-Y remaining inventory demos`: handle demos discovered by inventory but not listed above, one commit per demo.
- `P6-Z demo main contract GREEN`: run full contract and commit inventory-only cleanup if needed.

For every P6-C..P6-Y task:

- [ ] Run `rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py` and confirm target demo is one of the failures.
- [ ] Replace only that demo `main()` with the skeleton above while preserving its `create_demo_ui()` / `make_ui()` body.
- [ ] Update that demo inventory row to `"main_style": "v1_lvgl_like"` and `"uses_picoui_app": false`.
- [ ] Build target listed in the task.
- [ ] Run style contract again.
- [ ] Run `rtk git diff --check`.
- [ ] Commit with the subject listed in the task.

### Task P6-AA: native artifact gate

Files: `tests/picoui/runtime/check_picoui_native_visible_ui.py`, `tests/picoui/runtime/picoui_native_artifact_manifest.json`, P6 docs.
Steps:

- [ ] Add manifest entries for every demo with expected non-empty screenshot/artifact.
- [ ] Run `rtk python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --all-demos`.
- [ ] If a demo lacks a stable artifact, mark it `runtime_only` with reason in manifest, not in code.
- [ ] Commit `test: add picoui native demo artifact gate`.

### Task P6-AB: manual artifact docs

Files: `docs/picoui-serial/v1.0-native/06-demo-port验证.md`, `picoui/docs/demo_guide.md`.
Steps:

- [ ] Document exact commands to run one demo, all demos, and capture artifacts.
- [ ] State that visible artifact evidence is required for “UI 完成”.
- [ ] Link archived pre-v1.0 wrapper docs only as history.
- [ ] Commit `docs: document picoui native demo verification`.

### Task P6-AC: P6 closeout

Files: P6 docs, line index, ledger.
Steps:

- [ ] Run `rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py`.
- [ ] Run `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --all-demos`.
- [ ] Run `rtk python3 tests/picoui/runtime/check_picoui_native_visible_ui.py --all-demos`.
- [ ] Update ledger `P6-demo-port-artifact`.
- [ ] Run GitNexus detect changes.
- [ ] Commit `docs: close picoui native demo port`.

---
