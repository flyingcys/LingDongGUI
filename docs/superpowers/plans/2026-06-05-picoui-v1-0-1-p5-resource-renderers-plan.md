# PicoUI v1.0.1 P5 Resource Renderers Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement native resource, font, image, canvas, qrcode, and animation renderer bridges.

**Architecture:** This phase connects renderable resources to ARM-2D-oriented native drawing. Widget state must already exist before these renderer bridges land.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Master plan: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-implementation.md`
- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Repo instructions: `AGENTS.md`

---

## P5: Resource / Font / Image / Text

### Task P5-A: resource handle design contract

Files: `picoui/include/picoui/resource.h`, `tests/picoui/native/test_picoui_native_resource.c`, `picoui/src/native/native_resource.c`, CMake, `docs/picoui-serial/v1.0-native/05-resource-font-image.md`.
Steps:

- [ ] RED: create resource from const memory, file path, and null source; assert size/type/refcount.
- [ ] Implement `struct picoui_resource` handle with ref/unref and type enum.
- [ ] GREEN: `rtk ctest --test-dir build -R test_picoui_native_resource --output-on-failure`.
- [ ] Commit `feat: add picoui native resource handle`.

### Task P5-B: ARM-2D font bridge

Files: `tests/picoui/native/test_picoui_native_font_renderer.c`, `picoui/src/native/native_text_renderer.c`, `picoui/include/picoui/font.h`, CMake, P5 docs.
Steps:

- [ ] RED: render ASCII text into a test buffer and assert non-empty dirty area.
- [ ] Implement ARM-2D font adapter for default font and fallback glyph.
- [ ] GREEN: focused test plus `test_picoui_native_text`.
- [ ] Commit `feat: add picoui arm2d font bridge`.

### Task P5-C: image tile/mask/source bridge

Files: `tests/picoui/native/test_picoui_native_image_renderer.c`, `picoui/src/native/native_image_renderer.c`, `picoui/include/picoui/image.h`, CMake, P5 docs.
Steps:

- [ ] RED: draw a 2x2 ARGB tile, masked tile, and invalid source; assert target buffer pixels.
- [ ] Implement ARM-2D tile copy/blend path.
- [ ] GREEN: focused test plus `test_picoui_native_image`.
- [ ] Commit `feat: add picoui arm2d image renderer`.

### Task P5-D: canvas primitive renderer

Files: `tests/picoui/native/test_picoui_native_canvas_renderer.c`, `picoui/src/native/native_canvas_renderer.c`, `picoui/src/native/native_canvas.c`, CMake, P5 docs.
Steps:

- [ ] RED: draw line, rect, fill rect, circle into software buffer and assert dirty area/pixel changes.
- [ ] Implement primitive renderer using ARM-2D helpers where available and local raster fallback only for tests.
- [ ] GREEN: focused test plus `test_picoui_native_animation_canvas`.
- [ ] Commit `feat: add picoui native canvas renderer`.

### Task P5-E: qrcode bitmap renderer

Files: `tests/picoui/native/test_picoui_native_qrcode_renderer.c`, `picoui/src/native/native_qrcode_renderer.c`, `picoui/src/native/native_qrcode.c`, CMake, P5 docs.
Steps:

- [ ] RED: encode payload `PICOUI`, render matrix to buffer, assert finder-pattern pixels.
- [ ] Implement matrix-to-bitmap renderer.
- [ ] GREEN: focused test plus `test_picoui_native_qrcode`.
- [ ] Commit `feat: add picoui native qrcode renderer`.

### Task P5-F: animation frame source

Files: `tests/picoui/native/test_picoui_native_animation_frames.c`, `picoui/src/native/native_animation.c`, `picoui/src/native/native_image_renderer.c`, CMake, P5 docs.
Steps:

- [ ] RED: register three frame resources, advance timer twice, assert frame index and dirty state.
- [ ] Implement frame source binding and timer integration.
- [ ] GREEN: focused test plus `animation_basic` smoke.
- [ ] Commit `feat: add picoui native animation frames`.

### Task P5-G: P5 closeout

Files: P5 docs, line index, ledger.
Steps:

- [ ] Run `rtk ctest --test-dir build -R 'native_(resource|font_renderer|image_renderer|canvas_renderer|qrcode_renderer|animation_frames)' --output-on-failure`.
- [ ] Run text/image/canvas/qrcode/animation demos.
- [ ] Update ledger `P5-resource-font-image` to `covered`.
- [ ] Run GitNexus detect changes.
- [ ] Commit `docs: close picoui native resource renderers`.

---
