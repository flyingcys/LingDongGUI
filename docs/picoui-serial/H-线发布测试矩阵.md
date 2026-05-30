# PicoUI H10 发布测试矩阵冻结

## 范围与非范围

本文档只冻结 `H10` 阶段“发布前必须跑什么测试、从哪里跑、哪些不能并行”的真相源。

本文档不做以下事项：

- 不编写 `H11` 的 release notes、supported controls、known limitations 全包。
- 不替代 `H8` 的人工窗口验收记录真相源。
- 不把 `artifact existence` 写成“人工验收已通过”。

## 发布测试矩阵

| 类别 | CTest label | 当前入口 | 最小执行命令 | 是否强制纳入 CTest | 备注 |
| --- | --- | --- | --- | --- | --- |
| unit | `unit`、`picoui` | `tests/picoui/unit/test_picoui_*.c` | `ctest --test-dir build --output-on-failure -L unit` | 是 | 当前 PicoUI unit 集合包含 `test_picoui_smoke/theme/widgets/button_events/layout/list`。 |
| contract | `contract`、`picoui` | `tests/picoui/contract/check_picoui_public_api.py`、`check_picoui_demo_boundary.py`、`check_picoui_widget_contract_matrix.py`、`check_picoui_release_capability_matrix.py` | `python3 tests/picoui/contract/check_picoui_public_api.py`<br>`python3 tests/picoui/contract/check_picoui_demo_boundary.py`<br>`python3 tests/picoui/contract/check_picoui_widget_contract_matrix.py`<br>`python3 tests/picoui/contract/check_picoui_release_capability_matrix.py` | 是 | `ctest --test-dir build -L contract` 当前是仓库级 contract 超集，还会命中非 PicoUI contract 测试，因此不是 H10 scoped 最小入口。 |
| public API | `contract`、`picoui` | `tests/picoui/contract/check_picoui_public_api.py` | `python3 tests/picoui/contract/check_picoui_public_api.py` | 是 | 同时也已接入 `ctest -L contract` 与 `ctest -L picoui`。 |
| demo boundary | `contract`、`picoui` | `tests/picoui/contract/check_picoui_demo_boundary.py` | `python3 tests/picoui/contract/check_picoui_demo_boundary.py` | 是 | 用于约束 demo 只使用 `picoui_*` API，不泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。 |
| release matrix | `release`、`contract`、`picoui` | `tests/picoui/contract/check_picoui_release_capability_matrix.py` | `ctest --test-dir build --output-on-failure -L release` | 是 | 当前 `release` label 在 PicoUI 侧对应 `check_picoui_release_capability_matrix`。 |
| mapping | `mapping`、`runtime`、`backend`、`picoui` | `tests/picoui/runtime/check_picoui_backend_mapping.py` | `ctest --test-dir build --output-on-failure -L mapping` | 是 | 证明真实 backend tree / widget id mapping，不等于 visible 或 manual artifact。 |
| visible | `visible`、`runtime`、`picoui` | `tests/picoui/runtime/check_picoui_visible_ui.py --all` | `ctest --test-dir build --output-on-failure -L visible` | 是 | 当前 `visible` label 只有 `check_picoui_visible_ui`；若在真实桌面环境下额外生成自动窗口截图，也仍属于 `visible` 层自动证据。 |
| runtime | `runtime`、`picoui` | `tests/picoui/runtime/check_picoui_runtime.py` | `python3 tests/picoui/runtime/check_picoui_runtime.py` | 是 | 当前 `runtime` label 会同时带上 `check_picoui_runtime`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`，因此不适合作为并行拆分入口。 |
| manual artifact | 无 | `tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets|settings_panel` | `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`<br>`python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel` | 否 | 只覆盖 `basic_widgets` / `settings_panel`，只证明 artifact 生成入口存在，不接入强制 CTest。 |

## manual artifact 边界

- 当前 `manual artifact` 只覆盖 `basic_widgets` 与 `settings_panel` 两个 demo，因为现有脚本 `tests/picoui/runtime/check_picoui_manual_window_artifact.py` 只接受这两个 `--demo` 取值。
- 它是脚本入口，不在强制 CTest 内；发布前若要补人工窗口证据，需要人工显式运行。
- 它要求可用窗口环境。`SDL_VIDEODRIVER=dummy` 只能生成 readback artifact，不能支撑人工 OS 窗口验收。
- `ARTIFACT_READY` 只表示 `frame.ppm` 等 artifact 已生成，不表示人工已经观察窗口，更不表示人工验收通过。
- 当前 `Darwin + cocoa` 主机上，还额外确认过 `ffmpeg -f avfoundation -i "0:none"` 可抓到真实桌面窗口内容；这条路径若被使用，应归到 `visible` 层自动证据补强，不并入 `manual artifact`。

## 执行顺序与互斥

推荐按以下顺序执行：

1. `unit`
2. `contract`
3. `mapping`
4. `visible`
5. `manual artifact`

互斥规则必须固定如下：

- `check_picoui_runtime.py`
- `check_picoui_visible_ui.py`
- `check_picoui_backend_mapping.py`

以上三者都共享同一个构建目录：`build/picoui-runtime`。

具体原因：

- `check_picoui_runtime.py` 会在 `build/picoui-runtime` 下重新 `cmake -S <repo> -B build/picoui-runtime -DUSE_DEMO=0`，并在该目录构建多个 demo target。
- `check_picoui_visible_ui.py` 也使用同一个 `build/picoui-runtime` 目录，重配并重建 visible gate 所需 demo。
- `check_picoui_backend_mapping.py` 同样使用 `build/picoui-runtime` 目录，重配并重建 mapping gate 所需 demo。

因此：

- 不要并行跑 `runtime` / `visible` / `mapping`。
- 不要在一个终端跑 `ctest -L runtime` 的同时，在另一个终端再跑 `ctest -L visible` 或 `ctest -L mapping`。
- 若要执行这三类检查，必须串行，避免互相抢同一个 `build/picoui-runtime` 目录导致重配、重编译或产物覆盖。

## closeout 显式确认命令集

发布测试矩阵冻结后，建议在 closeout 时显式确认以下命令：

```bash
ctest --test-dir build --output-on-failure -L picoui
ctest --test-dir build --output-on-failure -L visible
ctest --test-dir build --output-on-failure -L mapping
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

说明：

- `ctest -L picoui` 已覆盖当前 PicoUI unit、contract、runtime、visible、mapping 的强制门禁集合，所以它是总集确认入口。
- `visible` 与 `mapping` 仍单独列出，是因为这两层证据在发布 closeout 中需要被显式单跑确认，而且两者都共享 `build/picoui-runtime`，必须串行执行，不能只把它们淹没在总集里。
- `check_picoui_public_api.py` 与 `check_picoui_demo_boundary.py` 仍单独列出，是因为它们分别对应 public API 边界与 demo boundary 两个发布口径检查点；虽然已包含在 `ctest -L picoui` 内，但 closeout 需要把这两个 contract 入口单独复核出来。
- `manual artifact` 继续维持脚本入口，不并入强制 CTest；单独列出是为了明确它仍是发布前需要人工补跑、但不属于自动强制 gate 的证据层。
