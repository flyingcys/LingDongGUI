# TINYUI H10 / J7 发布测试矩阵冻结

## 范围与非范围

- 作用域：本文只冻结 TINYUI `H10/J7` 历史发布线的测试矩阵与证据分层，不定义当前 `v2.1` 主线的 canonical truth；当前主线应以 `docs/v2.1/*`、`tests/tinyui/contract/*`、`tests/tinyui/runtime/*` 为准。

本文档冻结两件事：

1. `H10` 阶段“发布前必须跑什么测试、从哪里跑、哪些不能并行”的真相源。
2. `J7` 阶段 `v0.1 parity = window / label / button / slider` 的五层证据对照表。

本文档不做以下事项：

- 不编写 `H11` 的 release notes、supported controls、known limitations 全包。
- 不替代 `H8` 的人工窗口验收记录真相源。
- 不把 `artifact existence` 写成“人工验收已通过”。

## 发布测试矩阵

| 类别 | CTest label | 当前入口 | 最小执行命令 | 是否强制纳入 CTest | 备注 |
| --- | --- | --- | --- | --- | --- |
| unit | `unit`、`tinyui` | `tests/tinyui/unit/test_tinyui_*.c` | `ctest --test-dir build --output-on-failure -L unit` | 是 | 当前 TINYUI unit 集合包含 `test_tinyui_smoke/theme/widgets/button_events/layout/list`。 |
| contract | `contract`、`tinyui` | `tests/tinyui/contract/check_tinyui_public_api.py`、`check_tinyui_demo_boundary.py`、`check_tinyui_widget_contract_matrix.py`、`check_tinyui_release_capability_matrix.py` | `python3 tests/tinyui/contract/check_tinyui_public_api.py`<br>`python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`<br>`python3 tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`<br>`python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py` | 是 | `ctest --test-dir build -L contract` 当前是仓库级 contract 超集，还会命中非 TINYUI contract 测试，因此不是 H10 scoped 最小入口。 |
| public API | `contract`、`tinyui` | `tests/tinyui/contract/check_tinyui_public_api.py` | `python3 tests/tinyui/contract/check_tinyui_public_api.py` | 是 | 同时也已接入 `ctest -L contract` 与 `ctest -L tinyui`。 |
| demo boundary | `contract`、`tinyui` | `tests/tinyui/contract/check_tinyui_demo_boundary.py` | `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` | 是 | 用于约束 demo 只使用 `tinyui_*` API，不泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。 |
| release matrix | `release`、`contract`、`tinyui` | `tests/tinyui/contract/check_tinyui_release_capability_matrix.py` | `ctest --test-dir build --output-on-failure -L release` | 是 | 当前 `release` label 在 TINYUI 侧对应 `check_tinyui_release_capability_matrix`。 |
| mapping | `mapping`、`runtime`、`backend`、`tinyui` | `tests/tinyui/runtime/check_tinyui_backend_mapping.py` | `ctest --test-dir build --output-on-failure -L mapping` | 是 | 证明真实 backend tree / widget id mapping，不等于 visible 或 manual artifact。 |
| visible | `visible`、`runtime`、`tinyui` | `tests/tinyui/runtime/check_tinyui_visible_ui.py --all` | `ctest --test-dir build --output-on-failure -L visible` | 是 | 当前 `visible` label 只有 `check_tinyui_visible_ui`；若在真实桌面环境下额外生成自动窗口截图，也仍属于 `visible` 层自动证据。 |
| runtime | `runtime`、`tinyui` | `tests/tinyui/runtime/check_tinyui_runtime.py` | `python3 tests/tinyui/runtime/check_tinyui_runtime.py` | 是 | 当前 `runtime` label 会同时带上 `check_tinyui_runtime`、`check_tinyui_visible_ui`、`check_tinyui_backend_mapping`，因此不适合作为并行拆分入口。 |
| manual artifact | 无 | `tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets|settings_panel` | `python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets`<br>`python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo settings_panel` | 否 | 只覆盖 `basic_widgets` / `settings_panel`，只证明 artifact 生成入口存在，不接入强制 CTest。 |

补充说明：表中的 `tests/tinyui/*` 与 `check_tinyui_*` 入口在本文里仍是历史发布线真相源；它们当前不再代表 `v2.1` current live checker 或 canonical contract/runtime truth。

## manual artifact 边界

- 当前 `manual artifact` 只覆盖 `basic_widgets` 与 `settings_panel` 两个 demo，因为现有脚本 `tests/tinyui/runtime/check_tinyui_manual_window_artifact.py` 只接受这两个 `--demo` 取值。
- 它是脚本入口，不在强制 CTest 内；发布前若要补人工窗口证据，需要人工显式运行。
- 它要求可用窗口环境。`SDL_VIDEODRIVER=dummy` 只能生成 readback artifact，不能支撑人工 OS 窗口验收。
- `ARTIFACT_READY` 只表示 `frame.ppm` 等 artifact 已生成，不表示人工已经观察窗口，更不表示人工验收通过。
- 若后续补入真实桌面窗口截图脚本、固定命令与 artifact 记录，这条路径只能归到 `visible` 层自动证据补强，不并入 `manual artifact`。当前 H10 真相源不把它记成已固定、可复现的现状证据。

## 执行顺序与互斥

推荐按以下顺序执行：

1. `unit`
2. `contract`
3. `mapping`
4. `visible`
5. `manual artifact`

互斥规则必须固定如下：

- `check_tinyui_runtime.py`
- `check_tinyui_visible_ui.py`
- `check_tinyui_backend_mapping.py`

以上三者都共享同一个构建目录：`build/tinyui-runtime`。

具体原因：

- `check_tinyui_runtime.py` 会在 `build/tinyui-runtime` 下重新 `cmake -S <repo> -B build/tinyui-runtime -DUSE_DEMO=0`，并在该目录构建多个 demo target。
- `check_tinyui_visible_ui.py` 也使用同一个 `build/tinyui-runtime` 目录，重配并重建 visible gate 所需 demo。
- `check_tinyui_backend_mapping.py` 同样使用 `build/tinyui-runtime` 目录，重配并重建 mapping gate 所需 demo。

因此：

- 不要并行跑 `runtime` / `visible` / `mapping`。
- 不要在一个终端跑 `ctest -L runtime` 的同时，在另一个终端再跑 `ctest -L visible` 或 `ctest -L mapping`。
- 若要执行这三类检查，必须串行，避免互相抢同一个 `build/tinyui-runtime` 目录导致重配、重编译或产物覆盖。

## closeout 显式确认命令集

发布测试矩阵冻结后，建议在 closeout 时显式确认以下命令：

```bash
ctest --test-dir build --output-on-failure -L tinyui
ctest --test-dir build --output-on-failure -L visible
ctest --test-dir build --output-on-failure -L mapping
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo basic_widgets
python3 tests/tinyui/runtime/check_tinyui_manual_window_artifact.py --demo settings_panel
```

说明：

- `ctest -L tinyui` 已覆盖当前 TINYUI unit、contract、runtime、visible、mapping 的强制门禁集合，所以它是总集确认入口。
- `visible` 与 `mapping` 仍单独列出，是因为这两层证据在发布 closeout 中需要被显式单跑确认，而且两者都共享 `build/tinyui-runtime`，必须串行执行，不能只把它们淹没在总集里。
- `check_tinyui_public_api.py` 与 `check_tinyui_demo_boundary.py` 仍单独列出，是因为它们分别对应 public API 边界与 demo boundary 两个发布口径检查点；虽然已包含在 `ctest -L tinyui` 内，但 closeout 需要把这两个 contract 入口单独复核出来。
- `manual artifact` 继续维持脚本入口，不并入强制 CTest；单独列出是为了明确它仍是发布前需要人工补跑、但不属于自动强制 gate 的证据层。

## J7 `v0.1` 四控件证据矩阵

以下矩阵只回答 `window / label / button / slider` 当前由哪些证据承担 `unit / contract / mapping / visible / manual artifact` 结论。

| 控件 | unit | contract | mapping | visible | manual artifact |
| --- | --- | --- | --- | --- | --- |
| `window` | `tests/tinyui/unit/test_tinyui_widgets.c` 中 `window` J2 相关断言 | `check_tinyui_public_api.py`、`check_tinyui_demo_boundary.py`、`check_tinyui_release_capability_matrix.py` | `check_tinyui_backend_mapping.py` + `basic_widgets/settings_panel` backend tree | `check_tinyui_visible_ui.py --all`，样本 demo 为 `basic_widgets` / `settings_panel` | `C-线人工窗口验收记录.md` 中 `basic_widgets` 与 `settings_panel` 条目承担人工结论 |
| `label` | `tests/tinyui/unit/test_tinyui_widgets.c` 中 `test_label_parity_contract()` | `check_tinyui_public_api.py`、`check_tinyui_demo_boundary.py`、`check_tinyui_release_capability_matrix.py` | `check_tinyui_backend_mapping.py` + `basic_widgets` / `settings_panel` 中真实 `ldLabel` 路径 | `check_tinyui_visible_ui.py --all`，当前 visible 样本仍依赖 `basic_widgets` / `settings_panel` | `C-线人工窗口验收记录.md` 中 `basic_widgets` 与 `settings_panel` 条目承担人工结论 |
| `button` | `tests/tinyui/unit/test_tinyui_widgets.c` 中 `test_button_j4_contract()`；`tests/tinyui/unit/test_tinyui_button_events.c` | `check_tinyui_public_api.py`、`check_tinyui_demo_boundary.py`、`check_tinyui_release_capability_matrix.py` | `check_tinyui_backend_mapping.py` + `basic_widgets` / `settings_panel` 中真实 `ldButton` 路径 | `check_tinyui_visible_ui.py --all`，当前 visible 样本仍依赖 `basic_widgets` / `settings_panel` | `C-线人工窗口验收记录.md` 中 `basic_widgets` 与 `settings_panel` 条目承担人工结论 |
| `slider` | `tests/tinyui/unit/test_tinyui_widgets.c` 中 J5 合同断言；`tests/tinyui/unit/test_tinyui_button_events.c` | `check_tinyui_public_api.py`、`check_tinyui_demo_boundary.py`、`check_tinyui_release_capability_matrix.py` | `check_tinyui_backend_mapping.py` + `basic_widgets` / `settings_panel` 中真实 `ldSlider` 路径 | `check_tinyui_visible_ui.py --all`，当前 visible 样本仍依赖 `basic_widgets` / `settings_panel` | `C-线人工窗口验收记录.md` 中 `basic_widgets` 与 `settings_panel` 条目承担人工结论 |

说明：

- 当前 `visible` 与 `manual artifact` 仍以 `basic_widgets` / `settings_panel` 两个 demo 为主样本，不是四控件一控件一 demo 的拆分证据。
- `manual artifact` 当前只能证明 artifact 条目存在且可被人工复核；未填写人眼观察结论前，不得写成 `manual passed`。
