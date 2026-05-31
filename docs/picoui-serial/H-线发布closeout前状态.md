# PicoUI H线发布 closeout 前状态

## 文档定位

本文记录 `H13 closeout` 之前、当前工作树下最新一轮 fresh 验证证据，以及仍需诚实保留的证据边界。

本文不是 `closeout` 结论文档，不写“可发布”；它只回答两件事：

1. 当前哪些自动 gate 已在最新一轮验证中通过。
2. 当前还剩哪些证据边界，导致 `H13` 仍不能写成完成或发布就绪。

## 2026-05-31 最新自动验证

本轮按 `H-线发布测试矩阵.md` 的 closeout 命令顺序重跑，结果如下：

- `ctest --test-dir build --output-on-failure -L picoui`
  - 结果：通过，`13/13` 通过
- `ctest --test-dir build --output-on-failure -L visible`
  - 结果：通过，`1/1` 通过
- `ctest --test-dir build --output-on-failure -L mapping`
  - 结果：通过，`1/1` 通过
- `python3 tests/picoui/contract/check_picoui_release_capability_matrix.py`
  - 结果：通过
- `python3 tests/picoui/contract/check_picoui_public_api.py`
  - 结果：通过
- `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
  - 结果：通过
- `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`
  - 结果：输出 `PICOUI_MANUAL_WINDOW_ARTIFACT=ARTIFACT_READY`
- `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel`
  - 结果：输出 `PICOUI_MANUAL_WINDOW_ARTIFACT=ARTIFACT_READY`
- `git diff --check`
  - 结果：通过，无输出

## 当前已闭环项

- release matrix gate 已补强：
  - `wrapped` 控件状态会被正式断言
  - `v0_1_parity_target / v0_2_parity_backlog / parity_complete` 分流会被正式断言
  - `window / label / button / slider` 的关键 parity capability rows 会被正式断言
  - `summary.capability_entry_total` 与 `capability_status_counts` 会和实际 capability 条目对账
- `label` 的 `create_with_props` 已补齐当前 `v0.1 parity` 合同内的 props 落点：
  - `transparent`
  - `align`
  - `background_source`
- `slider` 的 release matrix 已回到当前真实 public API 边界，不再把不存在的 `get_value` 写成 `support`。
- `list` 合同单测已收紧到当前 H 线发布口径，不再把 backend 内部存储策略写成跨层合同。
- 发布文档、测试矩阵、差距文档、manual artifact 记录已经与当前自动验证状态同步。
- `C-线人工窗口验收记录.md` 的 `2026-05-30 basic_widgets/settings_panel` 两条记录已补成 `artifact-based visual observation` 结论。

## 当前未闭环项

以下事项仍阻止 `H13` 写成完成：

1. 当前人工层只到 `artifact-based visual observation`。
   - `docs/picoui-serial/C-线人工窗口验收记录.md` 中 `2026-05-30 basic_widgets`
   - `docs/picoui-serial/C-线人工窗口验收记录.md` 中 `2026-05-30 settings_panel`
   - 两条记录都来自 `frame.ppm` artifact 的人眼观察，不是 live OS 窗口现场验收。
2. 因为证据边界仍在这里，当前只能认定：
   - artifact existence 成立
   - 最小范围的 artifact-based visual observation 成立
   - 自动 gate fresh 通过
   - 不能认定 live OS 窗口验收通过
   - 不能认定 `H13 closeout` 完成或 `release ready`

## 当前正确结论

当前 H 线状态是：

- `H12 review blockers` 已修复并通过最新自动验证。
- `H13 closeout` 的自动门禁部分已 fresh 通过。
- `manual artifact` 最小范围的人眼观察结论已经补齐，但仍停留在 artifact-based visual observation 层。

因此，当前只能写成：

- `artifact-based visual observation complete`
- `not release ready by manual-artifact evidence alone`

不能写成：

- `ready for release`
- `closeout complete`
- `live OS window acceptance passed`
