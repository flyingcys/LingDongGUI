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

- 当前 canonical contract/test/perf 目录已收口为：
  - `tests/tinyui/contract/*`
  - `tests/tinyui/runtime/*`
  - `tests/tinyui/perf/*`
  - `tests/tinyui/unit/*`
- `tests/picoui/{contract,runtime,perf}` 当前只保留历史兼容入口与旧真相源验证；它们不是当前主线 canonical 执行入口。`tests/picoui/contract/README.md` 已明确旧 JSON/ledger 仅保留历史资产职责。

## 当前机器真相源入口

- 当前 canonical 机器真相源入口：
  - `tests/tinyui/contract/check_tinyui_v21_transition_guards.py`
  - `tests/tinyui/contract/tinyui_v21_transition_inventory.json`
  - `tests/tinyui/contract/check_tinyui_transition_guards.py`
  - `tests/tinyui/contract/tinyui_transition_inventory.json`
- 当前仍保留的历史兼容/旧真相源入口：
  - `tests/picoui/contract/picoui_tinyui_transition_inventory.json` 当前只保留给 `docs/v2.0/*` / `docs/v2.1` 历史记录引用，不在 live checker 执行链中
  - legacy 目录下与 canonical 完全一致的 `check_tinyui_*.py` 与 `check_picoui_*.py` thin wrapper 兼容壳已在 `V5` residue 清理批退场；当前这批旧入口只应被读取为历史文档记录，不应再视作主线 canonical truth。

## 当前实测基线值

- `picoui_dir_exists = false`
- `tinyui_dir_exists = true`
- `backend_c_files = 35`
- `picoui_public_api_count = 559`
- `tinyui_public_api_count = 39`

## 当前 broad gates

- 当前 canonical broad gate：
  - `rtk ctest --test-dir build -L 'tinyui' --output-on-failure`
  - `git diff --check`
- 当前 canonical perf/runtime/mapping gate：
  - `rtk ctest --test-dir build -L 'perf' --output-on-failure`
  - `rtk ctest --test-dir build -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure`
- `tests/picoui/runtime/*` 与 `tests/picoui/perf/*` 当前只保留兼容验证入口，不再是主线 broad gate 口径。

## V0 closeout 要点

- `v2.1` 的基线文档现在同时承接 `V0` 起跑真相与 `V1` 目录收口后的最新实况；后续阶段必须以本页当前 wording 为准。
- 后续 `V1` 到 `V5` 统一以本页和 `tinyui_v21_transition_inventory.json` 作为起跑真相，不再各自发明基线。
- 当前基线文档保留的是 `V0` 冻结值与后续阶段的最新口径；`tests/picoui/*` 旧证据线当前只应被读取为历史兼容入口，不再是主线 broad gate 口径。
- `V0` 已完成当前机器基线冻结；进入 `V1` 前不得再修改这组 inventory 数值来迎合迁移结果。
- `V1` 已完成物理目录与 include root 收口；后续 `V2` 开始可以在统一后的 `tinyui/` 根下继续做 backend 并回 widgets。
