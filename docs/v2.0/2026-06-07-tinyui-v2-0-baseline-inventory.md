# TinyUI v2.0 Baseline Inventory

## 文档状态

- 状态：P0 已落地
- 日期：2026-06-07
- 对应阶段：`docs/v2.0/plans/stages/p0-baseline-and-guards-plan.md`

## 当前共享层

- backend 共享入口：
  - `picoui/src/backend/ldgui/backend_app.c`
  - `picoui/src/backend/ldgui/backend_widget.c`
  - `picoui/src/backend/ldgui/backend_widget_tree.c`
  - `picoui/src/backend/ldgui/backend_event.c`
  - `picoui/src/backend/ldgui/backend_layout.c`
  - `picoui/src/backend/ldgui/backend_theme.c`
  - `picoui/src/backend/ldgui/backend_style_apply.c`
- 当前 `backend_*.c` 总数：`35`
- `app` 共享入口仍存在：
  - `picoui/include/picoui/app.h`
  - `picoui/src/core/app.c`

## 当前试点控件

- 首批试点控件实现仍位于：
  - `picoui/src/widgets/window.c`
  - `picoui/src/widgets/label.c`
  - `picoui/src/widgets/button.c`
  - `picoui/src/widgets/switch.c`
- 对应 backend 映射入口：
  - `picoui/src/backend/ldgui/backend_window.c`
  - `picoui/src/backend/ldgui/backend_label.c`
  - `picoui/src/backend/ldgui/backend_button.c`
  - `picoui/src/backend/ldgui/backend_switch.c`

## 当前 public app 入口

- public header：`picoui/include/picoui/app.h`
- core source：`picoui/src/core/app.c`
- 当前 demo 主入口基线：`picoui/demo/basic_widgets/main.c`

## 当前机器基线

- inventory：`tests/picoui/contract/picoui_tinyui_transition_inventory.json`
- guard checker：`tests/picoui/contract/check_picoui_tinyui_transition_guards.py`
- 当前实测：
  - `backend_c_files = 35`
  - `app_header_exists = true`
  - `app_source_exists = true`
  - `picoui_public_api_count = 548`
  - `tinyui_public_api_count = 0`

## 当前 focused checks

- `python3 tests/picoui/contract/check_picoui_tinyui_transition_guards.py`
- `rtk ctest --test-dir build -R '^check_picoui_tinyui_transition_guards$' --output-on-failure`
- `rtk ctest --test-dir build -R 'check_picoui_tinyui_transition_guards|check_picoui_public_api|check_picoui_demo_boundary' --output-on-failure`

## 当前 broad gates

- `rtk ctest --test-dir build -L 'picoui' --output-on-failure`
- `rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
- `git diff --check`

## P0 closeout 要点

- `backend/app/tinyui_*` 迁移基线已冻结，后续阶段不得私自改写 baseline 口径。
- `picoui_public_api_count` 已按当前 checkout 实测值回填，不保留占位数。
- `tinyui_public_api_count` 当前仍为 `0`，后续只有进入 `P4` 才允许改变。
