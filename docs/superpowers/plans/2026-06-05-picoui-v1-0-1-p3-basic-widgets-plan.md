# PicoUI v1.0.1 P3 Basic Widgets Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate basic user-facing widgets to native ARM-2D runtime paths.

**Architecture:** This phase migrates common controls one at a time. Every widget gets a focused native test and basic demo smoke evidence.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P3: 基础控件迁移

### Task P3-A: checkbox native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_checkbox.c`
- Modify: `picoui/src/widgets/checkbox.c`
- Create: `picoui/src/native/native_checkbox.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: create checkbox, set checked true/false, toggle by pointer click, assert callback count and checked state.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_checkbox`.
- [ ] Implement native checkbox state/render path without `ldCheckBox*`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_checkbox --output-on-failure`.
- [ ] Demo smoke: `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets`.
- [ ] Commit: `git commit -m "feat: add picoui native checkbox"`.

### Task P3-B: switch native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_switch.c`
- Modify: `picoui/src/widgets/switch.c`
- Create: `picoui/src/native/native_switch.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: create switch, set on/off, pointer toggle, assert callback value sequence `1,0,1`.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_switch`.
- [ ] Implement native switch state/render path without `ldSwitch*`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_switch --output-on-failure`.
- [ ] Demo smoke: `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets`.
- [ ] Commit: `git commit -m "feat: add picoui native switch"`.

### Task P3-C: slider native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_slider.c`
- Modify: `picoui/src/widgets/slider.c`
- Create: `picoui/src/native/native_slider.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: set range `0..100`, set value `25`, drag to right edge, assert value reaches `100` and change callback fires once per value change.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_slider`.
- [ ] Implement native slider range/value/drag math without `ldSlider*`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_slider --output-on-failure`.
- [ ] Demo smoke: `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets`.
- [ ] Commit: `git commit -m "feat: add picoui native slider"`.

### Task P3-D: text native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_text.c`
- Modify: `picoui/src/widgets/text.c`
- Create: `picoui/src/native/native_text.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: create text widget, set UTF-8 text, set wrap width, assert public readback and render smoke success.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_text`.
- [ ] Implement native text state; renderer can use P5 text renderer hook if glyph drawing is not complete.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_text --output-on-failure`.
- [ ] Commit: `git commit -m "feat: add picoui native text widget"`.

### Task P3-E: image native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_image.c`
- Modify: `picoui/src/widgets/image.c`
- Create: `picoui/src/native/native_image.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: create image, assert empty source is accepted as blank widget, set tile source pointer, assert source readback.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_image`.
- [ ] Implement native image source binding without `ldImage*`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_image --output-on-failure`.
- [ ] Commit: `git commit -m "feat: add picoui native image widget"`.

### Task P3-F: list native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_list.c`
- Modify: `picoui/src/widgets/list.c`
- Create: `picoui/src/native/native_list.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: add three items, select item 2, assert selected index/text and callback user data pointer.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_list`.
- [ ] Implement native list item storage, selection, click dispatch without `ldList*`.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_list --output-on-failure`.
- [ ] Demo smoke: `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo list_basic`.
- [ ] Commit: `git commit -m "feat: add picoui native list widget"`.

### Task P3-G: background native migration

**Files:**
- Create: `tests/picoui/native/test_picoui_native_background.c`
- Modify: `picoui/src/widgets/background.c`
- Create: `picoui/src/native/native_background.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`

- [ ] RED: create background as screen root, set fill color/image, load screen, assert background covers display size.
- [ ] RED command: `rtk cmake --build build --target test_picoui_native_background`.
- [ ] Implement native background as normal root widget, not app background.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_background --output-on-failure`.
- [ ] Commit: `git commit -m "feat: add picoui native background widget"`.

### Task P3-H: basic widgets native artifact

**Files:** runtime visible script naming/doc sync, docs, ledger.

- [ ] Use existing `tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets` as the P3-H visible gate; do not add a second alias script unless the repo later needs a distinct native-only contract.
- [ ] Run `rtk python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets`.
- [ ] Run `rtk python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets`.
- [ ] Update ledger `P3-basic-widgets` to `covered` only if runtime and visible checks both pass.
- [ ] Run `rtk git diff --check`.
- [ ] Commit: `git commit -m "test: verify basic widgets native artifact"`.

---
