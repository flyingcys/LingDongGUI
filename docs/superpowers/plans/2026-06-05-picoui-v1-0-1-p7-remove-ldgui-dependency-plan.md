# PicoUI v1.0.1 P7 Remove ldgui Dependency Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the default PicoUI runtime dependency on the LingDongGUI backend.

**Architecture:** This phase changes build defaults after native coverage exists. Legacy ldgui backend becomes opt-in only.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P7: 删除默认 ldgui backend 依赖

### Task P7-A: no-ldgui dependency RED contract

Files: `tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py`, `tests/picoui/CMakeLists.txt`, `docs/picoui-serial/v1.0-native/07-ldgui构建解绑.md`.
Steps:

- [ ] Write checker that configures or inspects native build graph and fails if default PicoUI target links `picoui_backend_ldgui`, `picoui_backend_ldgui_runtime`, or `src/gui/ld*.c`.
- [ ] Register contract.
- [ ] RED: `rtk ctest --test-dir build -R check_picoui_no_ldgui_runtime_dependency --output-on-failure`.
- [ ] Commit `test: require picoui native build without ldgui`.

### Task P7-B: CMake runtime split

Files: `cmake/LingDongGUI.cmake`, root `CMakeLists.txt`.

Add:

```cmake
set(PICOUI_RUNTIME "native_arm2d" CACHE STRING "PicoUI runtime backend")
option(PICOUI_ENABLE_LEGACY_LDGUI "Build legacy PicoUI ldgui backend" OFF)
```

Steps:

- [ ] Add cache variables above.
- [ ] Create `picoui_native_arm2d` library from `picoui/src/native/*.c`.
- [ ] Keep `picoui_backend_ldgui` behind `PICOUI_ENABLE_LEGACY_LDGUI`.
- [ ] GREEN configure: `rtk cmake -S . -B build-native -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF`.
- [ ] Commit `build: split picoui native and legacy runtimes`.

### Task P7-C: native target default

Files: `cmake/LingDongGUI.cmake`, demo CMake definitions, tests CMake.
Steps:

- [ ] Make default `picoui` / demo targets link native runtime.
- [ ] Keep legacy target name explicit, for example `picoui_legacy_ldgui`.
- [ ] Build native default: `rtk cmake --build build-native`.
- [ ] Verify no default link to legacy target with `rtk cmake --build build-native --target help | rtk rg "picoui_backend_ldgui|legacy"`.
- [ ] Commit `build: make picoui native runtime default`.

### Task P7-D: update tests off backend_mapping

Files: `tests/picoui/CMakeLists.txt`, `tests/picoui/contract/check_picoui_backend_mapping.py`, native release matrix docs.
Steps:

- [ ] Remove `check_picoui_backend_mapping` from default native label; keep it under legacy label if retained.
- [ ] Add native release matrix contract to default `picoui;release`.
- [ ] Run `rtk ctest --test-dir build-native -L picoui --output-on-failure`.
- [ ] Commit `test: move picoui release gates to native runtime`.

### Task P7-E: P7 closeout

Run:

```bash
rtk cmake -S . -B build-native -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF
rtk cmake --build build-native
rtk ctest --test-dir build-native -L picoui --output-on-failure
```

- [ ] Run `rtk python3 tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py`.
- [ ] Update ledger `P7-remove-ldgui-runtime-dependency`.
- [ ] Run GitNexus detect changes.
- [ ] Commit `docs: close picoui ldgui dependency removal`.

---
