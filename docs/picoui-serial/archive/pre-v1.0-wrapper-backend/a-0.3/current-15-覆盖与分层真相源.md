# Current-15 覆盖与分层真相源

> 说明：这份文档是 `a-0.3` 在 2026-05-31 冻结的 `current-15` 阶段快照，不再代表当前主仓 truth-source。当前主仓以 `tests/picoui/contract/picoui_release_capability_matrix.json` 与 `docs/superpowers/reviews/2026-06-01-picoui-a-0-3-a-0-4-a-0-5-deep-review.md` 为准。

## 目标

这份文档只回答 `a-0.3-R1 / R2` 的两个问题：

1. `a-0.3` 冻结时的 PicoUI public widget 到底已经覆盖到哪里。
2. 当时 `15` 个已接入控件分别处在哪个中间层级。

它不是当前主仓 release closeout，也不是后续版本计划。

## current PicoUI public widget 清单

以 `a-0.3` 冻结时的 `picoui/include/picoui/*.h` 导出集合为准，排除 `app/layout/theme/widget/picoui` 五个基础入口后，当时 current public widget 共 `15` 个：

1. `window`
2. `label`
3. `button`
4. `checkbox`
5. `switch`
6. `slider`
7. `text`
8. `image`
9. `list`
10. `progress_bar`
11. `qrcode`
12. `progress_wheel`
13. `message_box`
14. `date_time`
15. `clock`

## 与 LingDongGUI 26 控件的映射

当时的 `current-15` 审计仍以 LingDongGUI `26` 个可封装控件为全集。

current-15 对应关系如下：

| PicoUI public widget | LingDongGUI backend widget | 当前层级 |
| --- | --- | --- |
| `window` | `ldWindow` | `full parity complete` |
| `label` | `ldLabel` | `full parity complete` |
| `button` | `ldButton` | `full parity complete` |
| `checkbox` | `ldCheckBox` | `stable contract but not full parity` |
| `switch` | `ldSwitch` | `stable contract but not full parity` |
| `slider` | `ldSlider` | `full parity complete` |
| `text` | `ldText` | `stable contract but not full parity` |
| `image` | `ldImage` | `stable contract but not full parity` |
| `list` | `ldList` | `stable contract but not full parity` |
| `progress_bar` | `ldProgressBar` | `minimal vertical slice only` |
| `qrcode` | `ldQRCode` | `minimal vertical slice only` |
| `progress_wheel` | `ldProgressWheel` | `minimal vertical slice only` |
| `message_box` | `ldMessageBox` | `minimal vertical slice only` |
| `date_time` | `ldDateTime` | `minimal vertical slice only` |
| `clock` | `ldClock` | `minimal vertical slice only` |

## 当前未覆盖控件

在该快照时点，current public widget 之外，仍未接入 PicoUI public API 的 LingDongGUI 控件共 `11` 个：

1. `line_edit`
2. `keyboard`
3. `combo_box`
4. `scroll_selecter`
5. `arc`
6. `gauge`
7. `graph`
8. `table`
9. `calendar`
10. `icon_slider`
11. `radial_menu`

## 为什么旧 J1 matrix 已经过时

旧 `J1` machine-readable matrix 的问题不是“完全错误”，而是口径已不再覆盖当前代码事实：

1. 它只记录 `9` 个 wrapped 控件。
2. 它把 `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 六个已接入 public widget 仍写成 `not_wrapped`。
3. 它没有 current-15 三层分级，只能表达旧 `v0.1 / v0.2` parity 世界。
4. 它不能诚实区分：
   - `full parity complete`
   - `stable contract but not full parity`
   - `minimal vertical slice only`

因此 `a-0.3` 的 truth-source rebuild 必须把 machine-readable matrix 与当前 public API/代码/既有 serial 文档对齐。

## current-15 分层定义

`a-0.3` 当前只允许三种中间态：

1. `full parity complete`
   - 当前控件已按既有 parity 口径收口，matrix 可以写 `parity_complete`。
2. `stable contract but not full parity`
   - 当前控件的主合同、读写路径、backend bridge 已稳定，但还不能夸大成完整 parity。
3. `minimal vertical slice only`
   - 当前控件已具备 public API、backend 映射、demo/runtime 证据，但只证明最小垂直切片，不证明完整功能面对齐。

## current-15 分层结果

### full parity complete

1. `window`
2. `label`
3. `button`
4. `slider`

### stable contract but not full parity

1. `checkbox`
2. `switch`
3. `text`
4. `image`
5. `list`

### minimal vertical slice only

1. `progress_bar`
2. `qrcode`
3. `progress_wheel`
4. `message_box`
5. `date_time`
6. `clock`

## 计数口径

该历史快照的计数固定为：

1. `ldgui_wrappable_widget_total = 26`
2. `picoui_wrapped_widget_total = 15`
3. `picoui_not_wrapped_widget_total = 11`
4. `full_parity_complete_total = 4`
5. `stable_contract_total = 5`
6. `minimal_vertical_slice_total = 6`

这些数字只要求与 `a-0.3` 冻结时的 machine-readable matrix summary、gate 断言、能力审计保持一致；不再要求与 2026-06-01 主仓 current-22 状态一致。
