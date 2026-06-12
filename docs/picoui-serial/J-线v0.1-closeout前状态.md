# PicoUI J线 `v0.1` closeout 前状态

## 文档定位

本文记录 `J7 closeout` 之前、当前工作树下关于 `v0.1 parity = window / label / button / slider` 的最新证据状态，以及当前仍需保留的人工证据边界。

注意：本文是 `J线` 历史 closeout 前状态快照，不是当前 `a-0.6 final release` 真相源。对这条历史线而言，当时最终发布结论以 `docs/picoui-serial/a-0.6-final-release-closeout.md` 和对应 final release truth-source 为准。

本文不是 closeout 结论文档，不写“已完成发布”或“人工验收通过”；它只回答两件事：

1. 当前哪些自动 gate 已经能为 `window / label / button / slider` 提供证据。
2. 当前人工层已经补到什么边界，以及为什么这仍不等于发布完成。

## 2026-05-30 当前自动 gate 与 artifact 入口

围绕 `v0.1` 四控件，当前已经可用的自动证据是：

- `unit`
  - `tests/picoui/unit/test_picoui_widgets.c`
  - `tests/picoui/unit/test_picoui_button_events.c`
- `contract`
  - `python3 tests/picoui/contract/check_picoui_public_api.py`
  - `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
  - `python3 tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `mapping`
  - `ctest --test-dir build --output-on-failure -L mapping`
- `visible`
  - `ctest --test-dir build --output-on-failure -L visible`
- `manual artifact entry`
  - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`
  - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel`

## 当前已闭环项

- `label` props 路径已补齐 `transparent / align / background_source`，对应 unit 已覆盖。
- `slider` release matrix 已回调到真实 public API 边界，不再要求不存在的 `get_value`。
- `H-线当前9控件发布合同.md`、`H-线已支持控件清单.md`、`H-线第一版发布说明.md`、`H-线发布差距与LingDongGUI控件对比.md` 已经改成：
  - `v0.1 parity = window / label / button / slider`
  - `v0.2 parity backlog = checkbox / switch / text / image / list`
- `manual artifact` 条目已经登记到 `C-线人工窗口验收记录.md`，且 `basic_widgets` / `settings_panel` 两条脚本入口能生成 artifact。

## 当前人工证据边界

当前已经没有“待补填的人工结论”这一类文档空洞，但仍必须保留以下证据边界：

1. `manual artifact` 的 2026-05-30 记录已经补成人眼观察结论。
   - `basic_widgets` 条目承担 `window / label / button / slider` 的主要可见样本。
   - `settings_panel` 条目补充 `window / button / slider` 的第二个样本，并明确该图像不能单独承担完整 `label` 结论。
2. 上述人工结论都来自 `frame.ppm` artifact 的人眼观察。
   - 可以认定 artifact-based visual observation 已补齐。
   - 不能认定 live OS 窗口现场验收通过。
   - 不能把这组证据上抬成 `release ready` 或完整交互体验结论。

## 当前正确结论

当前 J 线 `v0.1` 状态应写成：

- `window / label / button / slider` 的 `unit / contract / mapping / visible / manual artifact` 五层证据已经补齐
- `artifact-based visual observation` 已补齐，但它仍只是人工证据边界的一部分
- `checkbox / switch / text / image / list` 已明确转入 `v0.2 parity backlog`
- 当前人工层结论是“artifact-based visual observation 已补齐”，而不是“live OS 窗口验收通过”

因此当前最多只能写成：

- `v0.1 parity evidence complete`
- `artifact-based visual observation complete`
- `not release ready by manual-artifact evidence alone`

不能写成：

- `v0.1 release ready`
- `manual review passed`
- `live OS window acceptance passed`
