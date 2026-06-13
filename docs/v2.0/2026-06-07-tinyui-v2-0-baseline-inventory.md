# TinyUI v2.0 Baseline Inventory

## 文档状态

- 状态：P0 已落地
- 日期：2026-06-07
- 对应阶段：`docs/v2.0/plans/stages/p0-baseline-and-guards-plan.md`
- 作用域：本文只冻结 `v2.0/P0` 当时的阶段基线，不属于当前 `v2.1` canonical truth；若要判断当前仓库主线真相，应回看 `docs/v2.1/*` 与 `tests/tinyui/contract/*`

## 当前共享层

- backend 共享入口：
  - `tinyui/src/backend/ldgui/backend_app.c`
  - `tinyui/src/backend/ldgui/backend_widget.c`
  - `tinyui/src/backend/ldgui/backend_widget_tree.c`
  - `tinyui/src/backend/ldgui/backend_event.c`
  - `tinyui/src/backend/ldgui/backend_layout.c`
  - `tinyui/src/backend/ldgui/backend_theme.c`
  - `tinyui/src/backend/ldgui/backend_style_apply.c`
- 当前 `backend_*.c` 总数：`35`
- `app` 共享入口仍存在：
  - `tinyui/include/tinyui/app.h`
  - `tinyui/src/core/app.c`

## 当前试点控件

- 首批试点控件实现仍位于：
  - `tinyui/src/widgets/window.c`
  - `tinyui/src/widgets/label.c`
  - `tinyui/src/widgets/button.c`
  - `tinyui/src/widgets/switch.c`
- 对应 backend 映射入口：
  - `tinyui/src/backend/ldgui/backend_window.c`
  - `tinyui/src/backend/ldgui/backend_label.c`
  - `tinyui/src/backend/ldgui/backend_button.c`
  - `tinyui/src/backend/ldgui/backend_switch.c`

## 当前 public app 入口

- public header：`tinyui/include/tinyui/app.h`
- core source：`tinyui/src/core/app.c`
- 当前 demo 主入口基线：`tinyui/demo/basic_widgets/main.c`

## 当前机器基线

- inventory：`tests/tinyui/contract/tinyui_tinyui_transition_inventory.json`
- guard checker：`tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py`
- 当前实测：
  - `backend_c_files = 35`
  - `app_header_exists = true`
  - `app_source_exists = true`
  - `tinyui_public_api_count = 548`
  - `tinyui_public_api_count = 0`

## 当前 focused checks

- `python3 tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py`
- `rtk ctest --test-dir build -R '^check_tinyui_tinyui_transition_guards$' --output-on-failure`
- `rtk ctest --test-dir build -R 'check_tinyui_tinyui_transition_guards|check_tinyui_public_api|check_tinyui_demo_boundary' --output-on-failure`

## 当前 broad gates

- `rtk ctest --test-dir build -L 'tinyui' --output-on-failure`
- `rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure`
- `git diff --check`

## P0 closeout 要点

- `backend/app/tinyui_*` 迁移基线已冻结，后续阶段不得私自改写 baseline 口径。
- `tinyui_public_api_count` 已按当前 checkout 实测值回填，不保留占位数。
- `tinyui_public_api_count` 当前仍为 `0`，后续只有进入 `P4` 才允许改变。
