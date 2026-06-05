# PicoUI v1.0.1 P8 Release Gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the v1.0.1 native-only release gate.

**Architecture:** This phase collects release evidence and updates user-facing docs. It must not introduce new runtime behavior except release-gate fixes.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P8: v1.0.1 Release Gate

### Task P8-A: native release matrix

Files: `tests/picoui/contract/picoui_native_release_matrix.json`, `tests/picoui/contract/check_picoui_native_release_matrix.py`, `tests/picoui/CMakeLists.txt`, `docs/picoui-serial/v1.0-native/08-v1.0.1发布门禁.md`.
Steps:

- [ ] RED checker validates every ledger entry is `covered` or `non_user_capability`, every demo has style/runtime result, and no default ldgui dependency remains.
- [ ] RED command: `rtk ctest --test-dir build-native -R check_picoui_native_release_matrix --output-on-failure`.
- [ ] Write matrix JSON with phases P0-P8, demo rows, widget rows, dependency rows.
- [ ] GREEN command above.
- [ ] Commit `test: add picoui v1 native release matrix`.

### Task P8-B: docs/ability native-only sync

Files: `docs/ability/*.md`, `docs/ability/README.md`, `docs/picoui-serial/v1.0-native/08-v1.0.1发布门禁.md`.
Steps:

- [ ] Replace wrapper-backend language with native ARM-2D runtime language.
- [ ] Preserve old wrapper docs only via archive links.
- [ ] Run `rtk rg -n "backend wrapper|LingDongGUI backend|ldgui backend|pre-v1.0" docs/ability picoui/docs`.
- [ ] Commit `docs: sync picoui ability docs for native runtime`.

### Task P8-C: quick start and porting docs

Files: `picoui/docs/quick_start.md`, `picoui/docs/porting_rules.md`, `picoui/docs/api_overview.md`.
Steps:

- [ ] Quick start must show `picoui_init()`, `picoui_sdl_hal_init(320, 480)`, UI creation, and `picoui_timer_handler()` loop.
- [ ] Porting docs must explain display flush callback, input read callback, buffers, and no public app object.
- [ ] API overview must mark `picoui_app_*` compatibility-only or removed according to P0 decision.
- [ ] Run docs grep: `rtk rg -n "picoui_app_create|picoui_app_run" picoui/docs`.
- [ ] Commit `docs: add picoui native quick start`.

### Task P8-D: release notes and closeout

Files:

- `docs/picoui-serial/v1.0-native/08-v1.0.1发布门禁.md`
- release notes path selected by repo convention
- `docs/picoui-serial/v1.0-native/线计划索引.md`

Final verification:

```bash
rtk cmake -S . -B build-native-release -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build-native-release
rtk ctest --test-dir build-native-release -L picoui --output-on-failure
rtk python3 tests/picoui/contract/check_picoui_demo_main_style.py
rtk python3 tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py
rtk python3 tests/picoui/contract/check_picoui_native_migration_ledger.py
rtk python3 tests/picoui/contract/check_picoui_native_release_matrix.py
rtk git diff --check
```

Main thread:

```text
mcp__gitnexus.detect_changes({repo:"LingDongGUI", scope:"all"})
```

- [ ] Update ledger `P8-release-gate`.
- [ ] Update line index status to `v1.0.1 ready`.
- [ ] Commit `docs: close picoui v1 native release gate`.

---
