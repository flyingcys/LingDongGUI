# PicoUI v1.0.1 Native ARM-2D Runtime Master Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement the phase plan documents task-by-task. This file is the dispatcher index; do not use it as the only execution context for a subagent.

**Goal:** Build PicoUI v1.0.1 as a native-only ARM-2D GUI runtime with an LVGL-like public development model.

**Architecture:** PicoUI moves from `PicoUI public API -> LingDongGUI backend wrapper -> ARM-2D` to `PicoUI public API -> PicoUI native runtime/widgets -> ARM-2D -> PicoUI port/display`. Execution is split by phase so each subagent reads only the relevant phase plan plus shared references.

**Tech Stack:** C11, CMake, SDL2, ARM-2D, existing PicoUI public headers, `rtk cmake`, `rtk ctest`, Python contract checks.

---

## Shared References

- Spec: `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`
- Serial index: `docs/picoui-serial/v1.0-native/线计划索引.md`
- LVGL main reference: `third_party/lv_port_pc_vscode/main/src/main.c`
- Current wrapper archive: `docs/picoui-serial/archive/pre-v1.0-wrapper-backend/`
- Current PicoUI API headers: `picoui/include/picoui/*.h`
- Current PicoUI demos: `picoui/demo/*/main.c`

## Execution Rules

- 严格按 P0 到 P8 顺序推进；阶段未 closeout 不进入下一阶段。
- 每个阶段使用对应 phase plan；subagent 不应加载其他阶段 plan，除非当前阶段明确引用。
- 每个任务开始前读本阶段 plan、spec、`AGENTS.md`。
- 修改函数/符号前必须跑 GitNexus impact；HIGH/CRITICAL 先停下汇报。
- 每个任务先 RED，再实现，再 GREEN，再 `rtk git diff --check`。
- 每阶段结束后主线程运行 GitNexus detect_changes。
- review 不通过时，同一个 subagent 修复。

## Phase Plan Index

| Phase | Plan | Goal |
| --- | --- | --- |
| P0 | `2026-06-05-picoui-v1-0-1-p0-baseline-plan.md` | Freeze current facts and decisions before native runtime implementation starts. |
| P1 | `2026-06-05-picoui-v1-0-1-p1-native-vertical-slice-plan.md` | Build the first LVGL-like native runtime vertical slice and migrate basic_widgets main as the sample. |
| P2 | `2026-06-05-picoui-v1-0-1-p2-core-layout-theme-plan.md` | Build native widget tree, geometry, layout, and theme foundation. |
| P3 | `2026-06-05-picoui-v1-0-1-p3-basic-widgets-plan.md` | Migrate basic user-facing widgets to native ARM-2D runtime paths. |
| P4 | `2026-06-05-picoui-v1-0-1-p4-extended-widgets-plan.md` | Migrate extended widgets to native ARM-2D runtime paths. |
| P5 | `2026-06-05-picoui-v1-0-1-p5-resource-renderers-plan.md` | Implement native resource, font, image, canvas, qrcode, and animation renderer bridges. |
| P6 | `2026-06-05-picoui-v1-0-1-p6-demo-port-artifact-plan.md` | Attach the native SDL port, migrate demo mains, and establish runtime/visible artifacts. |
| P7 | `2026-06-05-picoui-v1-0-1-p7-remove-ldgui-dependency-plan.md` | Remove the default PicoUI runtime dependency on the LingDongGUI backend. |
| P8 | `2026-06-05-picoui-v1-0-1-p8-release-gate-plan.md` | Close the v1.0.1 native-only release gate. |

## Execution Order

1. P0: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p0-baseline-plan.md`
2. P1: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p1-native-vertical-slice-plan.md`
3. P2: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p2-core-layout-theme-plan.md`
4. P3: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p3-basic-widgets-plan.md`
5. P4: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p4-extended-widgets-plan.md`
6. P5: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p5-resource-renderers-plan.md`
7. P6: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p6-demo-port-artifact-plan.md`
8. P7: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p7-remove-ldgui-dependency-plan.md`
9. P8: `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-p8-release-gate-plan.md`

## Phase Gates

- P0 gate: baseline documents and machine-readable inventories exist and pass contract checks.
- P1 gate: LVGL-like public runtime/display/indev/screen vertical slice works and `basic_widgets` main uses the new style.
- P2 gate: native widget tree, geometry, flex/grid layout, and theme/style state pass focused tests without ldgui layout calls.
- P3 gate: basic widgets pass native focused tests plus `basic_widgets` visible/runtime evidence.
- P4 gate: extended widgets pass focused tests and their demo runtime smokes.
- P5 gate: resource/font/image/canvas/qrcode/animation render bridges pass native tests.
- P6 gate: all demo mains use LVGL-like startup and runtime/visible artifact checks pass.
- P7 gate: default PicoUI native build does not link ldgui backend/runtime.
- P8 gate: native release matrix, docs, and final build/test commands pass.

## 自检结果

- 单文件计划已拆分为 master + P0-P8 phase plans，便于 subagent 低上下文执行。
- Spec coverage remains mapped across P0-P8.
- Placeholder scan: no unresolved filler terms intended for execution.
- Type consistency: planned public names are `picoui_init`, `picoui_deinit`, `picoui_timer_handler`, `picoui_display_*`, `picoui_indev_*`, `picoui_screen_*`; `picoui_app_*` remains compatibility-only until P0 decision.
