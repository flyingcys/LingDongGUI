# TinyUI v2.1 Baseline Inventory

## 文档状态

- 状态：`V0` 已完成
- 日期：2026-06-10
- 对应阶段：`docs/v2.1/plans/stages/v0-baseline-and-rename-guards-plan.md`

## 当前顶层产品目录

- 当前顶层产品目录已收口为：
  - `tinyui/`
- 顶层 `picoui/` 已物理消失；当前不再允许把产品层写成双顶层目录并存。
- 当前 canonical umbrella header 是 `tinyui/include/tinyui.h`；`tinyui/include/picoui/*`
  兼容 include 子树仍暂留，后续阶段再退场。

## 当前 shared layers

- 当前 shared layer 目录：
  - `tinyui/src/core`
  - `tinyui/src/display`
  - `tinyui/src/indev`
  - `tinyui/src/layout`
  - `tinyui/src/theme`
  - `tinyui/src/tick`
- 当前 backend 目录：
  - `tinyui/src/backend/ldgui`
- 当前 widget 目录：
  - `tinyui/src/widgets`

## 当前 contract / test / perf 目录

- 当前 contract/test/perf 仍统一位于：
  - `tests/picoui/contract/*`
  - `tests/picoui/runtime/*`
  - `tests/picoui/perf/*`
  - `tests/picoui/unit/*`

## 当前机器真相源入口

- `tests/picoui/contract/check_tinyui_v21_transition_guards.py`
- `tests/picoui/contract/tinyui_v21_transition_inventory.json`
- `tests/picoui/contract/check_picoui_tinyui_transition_guards.py`
- `tests/picoui/contract/picoui_tinyui_transition_inventory.json`

## 当前实测基线值

- `picoui_dir_exists = false`
- `tinyui_dir_exists = true`
- `backend_c_files = 35`
- `picoui_public_api_count = 559`
- `tinyui_public_api_count = 39`

## 当前 broad gates

- `rtk ctest --test-dir build -L 'picoui' --output-on-failure`
- `rtk ctest --test-dir build -L 'perf' --output-on-failure`
- `rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
- `git diff --check`

## V0 closeout 要点

- `v2.1` 的基线文档现在同时承接 `V0` 起跑真相与 `V1` 目录收口后的最新实况；后续阶段必须以本页当前 wording 为准。
- 后续 `V1` 到 `V5` 统一以本页和 `tinyui_v21_transition_inventory.json` 作为起跑真相，不再各自发明基线。
- 当前 broad gates 仍沿用 `tests/picoui/*` 证据线；只有后续阶段完成迁移后，才允许改写 test/perf/runtime 路径口径。
- `V0` 已完成当前机器基线冻结；进入 `V1` 前不得再修改这组 inventory 数值来迎合迁移结果。
- `V1` 已完成物理目录与 include root 收口；后续 `V2` 开始可以在统一后的 `tinyui/` 根下继续做 backend 并回 widgets。
