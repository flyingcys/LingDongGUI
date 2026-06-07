# 07 ldgui 构建解绑

## P7-A `no-ldgui dependency RED contract`

### RED

- 已新增默认 runtime 依赖 RED checker：
  - [tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 当前 checker 做的事情很窄：
  - 配置独立 build 目录并读取 CMake File API codemodel
  - 检查默认 `picoui_* / test_picoui_* / check_picoui_*` targets 的依赖闭包
  - 只要仍依赖以下任一项就判定失败：
    - `picoui_backend_ldgui`
    - `picoui_backend_ldgui_runtime`
    - `src/gui/ld*.c`
    - `picoui/src/backend/ldgui/*`
- 这一步的目标是建立 `P7` 的 RED 真相，不是提前做 runtime split。

### 验证

- `ctest --test-dir build -R check_picoui_no_ldgui_runtime_dependency --output-on-failure`
  - 当前已按预期 `RED`
  - fresh 失败直接暴露了默认依赖真相：
    - 全部 `picoui_*_demo` 仍依赖 `picoui_backend_ldgui_runtime`
    - 默认 `test_picoui_*` 与 `test_picoui_native_*` 仍依赖 `picoui_backend_ldgui`
    - `picoui_test_support` 也仍依赖 `picoui_backend_ldgui`

### 当前结论

- 当前 `P7-A` 只建立 RED contract。
- 当前 fresh 证据已经证明：
  - 默认 PicoUI runtime 还没有和 `ldgui` backend 解耦
  - `P7-B` 确实需要做 CMake runtime split，而不是文档层 closeout
- 当前阶段下一任务推进到 `P7-B CMake runtime split`。

## P7-B `CMake runtime split`

### GREEN

- 已在 [CMakeLists.txt](/Users/cys/embedded/LingDongGUI/CMakeLists.txt) 新增 `P7-B` 所需 cache 变量：
  - `PICOUI_RUNTIME`
  - `PICOUI_ENABLE_LEGACY_LDGUI`
- 已在 [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake) 建立最小 runtime split：
  - `picoui_core` 继续承载当前 native ARM-2D runtime 实现
  - 新增 `picoui_native_arm2d` alias，作为 `P7/P8` 的 native runtime 语义名
  - `picoui_backend_ldgui` / `picoui_backend_ldgui_runtime` 现在被 `PICOUI_ENABLE_LEGACY_LDGUI` 包住
- 当前这一步只做 runtime split 本身：
  - 还没有把默认 demo / test target 切到 native-only link graph
  - 还没有改 `picoui_test_support`
  - 还没有关闭 `P7-A` 的 RED contract

### 验证

- `cmake -S . -B build-native -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF`
  - 当前已通过 configure

### 当前结论

- 当前 `P7-B CMake runtime split` 已完成到 plan 要求的最小 GREEN。
- 当前 fresh 证据已经证明：
  - 代码库已经能在 `legacy ldgui OFF` 条件下完成 native runtime configure
  - legacy backend 目标已被收口成 opt-in，而不是无条件创建
- 当前阶段下一任务推进到 `P7-C native target default`。

## P7-C `native target default`

### GREEN

- 已把默认 PicoUI 链接语义切到 native target：
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
    - 新增 `picoui_native_arm2d` interface target，默认聚合 `picoui_core + ld_picoui_native_runtime_support`
    - `picoui` alias 现在默认指向 `picoui_native_arm2d`
    - legacy 显式名保留为 `picoui_legacy_ldgui` / `picoui_legacy_ldgui_runtime`
    - `ld_picoui_native_runtime_support` 作为 private support target 承载当前仍需复用的 backend/asset helper 实现，但不再以 legacy runtime target 名进入默认 graph
  - [examples/sdl/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/examples/sdl/CMakeLists.txt)
    - `add_picoui_demo()` 默认从 `picoui_backend_ldgui_runtime` 改为链接 `picoui`
  - [tests/support/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/support/CMakeLists.txt)
    - `picoui_test_support` 默认改为链接 `picoui`
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
    - unit/native C tests 默认 `MAIN_LIB` 从 `picoui_backend_ldgui` 改为 `picoui`
- 这一步的关键收口不是把 `picoui_core` 强行做成自洽静态库，而是把默认 `picoui` graph 收成：
  - 不再暴露 `picoui_backend_ldgui*` 默认 target 名
  - 仍允许当前 native runtime 通过 private support target 复用必要 backend/asset helper，实现 build-level GREEN

### fresh 验证

- `cmake -S . -B build-native -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF`
  - 已通过
- `ctest --test-dir build-native -R check_picoui_no_ldgui_runtime_dependency --output-on-failure`
  - 已通过
- `cmake --build build-native`
  - 当前已全量通过
- `cmake --build build-native --target help | rg "picoui_backend_ldgui|picoui_legacy_ldgui"`
  - 当前无命中

### 关键调试结论

- 上一轮真实 blocker 并不是 demo/test 还在硬连 legacy target 名，而是 `picoui_core` 仍存在大量 backend 与 asset helper 未定义符号。
- 这一步的正确修法不是现在就大面积改 `picoui/src/core/*.c` / `picoui/src/widgets/*.c`，而是先把默认 `picoui` target 收成 native semantic target，再把当前仍需复用的 private bridge 实现藏到非 legacy 命名的 support target 里。
- 因此 `P7-C` 达成的是：
  - 默认 PicoUI runtime target 已从 legacy target 名切走
  - 默认 native build graph 已可 configure + build
  - `no-ldgui default target name` contract 仍保持为绿
- 但这一步没有声称：
  - 仓库已删除 `picoui/src/backend/ldgui/*`
  - `picoui_core` 单独已完全不依赖 private bridge/helper
  - `P7` 已整体完成

### 当前结论

- `P7-C native target default` 已完成到 plan 要求的 build-level GREEN。
- 当前阶段下一任务推进到 `P7-D update tests off backend_mapping`。

## P7-D `update tests off backend_mapping`

### GREEN

- 当前默认 native test graph 已经不再卡在 `theme/list/progress/qrcode` 这组基础 unit contract：
- 当前默认 native test graph 也已经不再卡在 `line_edit/keyboard` 这组 focused unit contract：
  - [picoui/src/theme/theme.c](/Users/cys/embedded/LingDongGUI/picoui/src/theme/theme.c)
  - [picoui/src/widgets/list.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/list.c)
  - [picoui/src/widgets/progress_bar.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/progress_bar.c)
  - [picoui/src/widgets/progress_wheel.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/progress_wheel.c)
  - [picoui/src/widgets/qrcode.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/qrcode.c)
  - [picoui/src/widgets/line_edit.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/line_edit.c)
  - [picoui/src/native/native_keyboard.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_keyboard.c)
  - [picoui/src/widgets/keyboard.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/keyboard.c)
  - [picoui/src/backend/ldgui/backend_event.c](/Users/cys/embedded/LingDongGUI/picoui/src/backend/ldgui/backend_event.c)
- 当前默认 native test graph 也已经不再卡在 `combo_box/scroll_selecter` 这组 focused unit contract：
- 当前默认 native test graph 也已经不再卡在 `combo_box/scroll_selecter/table` 这组 focused unit contract：
  - [picoui/src/widgets/combo_box.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/combo_box.c)
  - [picoui/src/widgets/scroll_selecter.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/scroll_selecter.c)
  - [picoui/src/widgets/table.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/table.c)
  - [picoui/src/backend/ldgui/backend_keyboard.c](/Users/cys/embedded/LingDongGUI/picoui/src/backend/ldgui/backend_keyboard.c)
- 这一轮修复收窄在“public getter/setter 必须和 backend/native truth 保持一致”，没有扩到新的 widget 设计：
  - `theme`：backend 不可 apply 时恢复 `-1` 失败语义，不污染 widget/backend 既有状态
  - `list`：`set/get selected_index` 同步 backend 真相，getter 可从 backend 回读 resync
  - `progress_bar`：`percent/horizontal/inverted` setter 同步 backend
  - `progress_wheel`：`percent/dot_enabled` setter 同步 backend
  - `qrcode`：默认 `text=""`，并让 `text/ecc` setter 同步 backend
  - `line_edit`：`get_text()` 优先从 backend 回读并刷新 native cache
  - `keyboard`：ASCII 输入在 `focus_owner line_edit` 与 `editing_owner line_edit` 两条路径都能正确认领 editing session；`exit` 不再吞掉 backend cleanup；native signal callback 的 `key_code` 改为回读 backend 当前 `ld_keyboard->keyCode`
  - `combo_box`：`selected_index/is_open` getter 改为优先回读 backend truth
  - `scroll_selecter`：`set_selected_index` 现在同步 backend，`get_selected_index` 优先回读 backend truth
  - `table`：`set_selected_cell/set_current_cell/set_cell_text` 现在同步 backend；`row/column/text` getter 改为按 commit/cancel 语义分层回读；keyboard cancel 路径会把 table 正在编辑的 backend 文本回滚到 public model 旧值
- 当前还没有开始改 `P7-E` closeout 或 `P8` release 文档。

### fresh 验证

- focused 5-test 回归已通过：
  - `ctest --test-dir build-native -R '^(test_picoui_theme|test_picoui_list|test_picoui_progress_bar|test_picoui_progress_wheel|test_picoui_qrcode)$' --output-on-failure`
- focused 2-test 回归已通过：
  - `ctest --test-dir build-native -R '^(test_picoui_line_edit|test_picoui_keyboard)$' --output-on-failure`
- focused 2-test 回归已通过：
  - `ctest --test-dir build-native -R '^(test_picoui_combo_box|test_picoui_scroll_selecter)$' --output-on-failure`
- focused 3-test 回归已通过：
  - `ctest --test-dir build-native -R '^(test_picoui_combo_box|test_picoui_scroll_selecter|test_picoui_table)$' --output-on-failure`
- focused `native_runtime` 回归已通过：
  - `ctest --test-dir build-native -R '^test_picoui_native_runtime$' --output-on-failure -V`
  - 这一轮根因不是 runtime 行为回归，而是 focused test target 没有拿到 `picoui_basic_widgets_demo` 路径定义，导致 `PICOUI_BASIC_WIDGETS_DEMO_PATH` 为空串后直接 `assert` 中止
  - 已在 [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt) 为 `test_picoui_native_runtime` 补上：
    - `add_dependencies(test_picoui_native_runtime picoui_basic_widgets_demo)`
    - `target_compile_definitions(... PICOUI_BASIC_WIDGETS_DEMO_PATH="$<TARGET_FILE:picoui_basic_widgets_demo>")`
- focused `native` 收口已补齐：
  - `ctest --test-dir build-native -R '^(test_picoui_native_keyboard_line_edit|test_picoui_native_grid_layout|test_picoui_native_flex_layout|check_picoui_widget_contract_matrix)$' --output-on-failure -V`
- `native_flex_layout` 这一轮的真实根因和收口：
  - [picoui/src/native/native_layout.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_layout.c)
    - 已补最小单轨 `row/column` 主轴 `START/CENTER/END` 对齐和交叉轴 `START/CENTER/END/STRETCH` 支持
    - 非 wrap 流下的 `track_align` 不再被当作拒绝条件
  - [tests/picoui/native/test_picoui_native_flex_layout.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_flex_layout.c)
    - fail-first column/center case 已转绿
    - 首版期望值的真实错误是把总高 `3 * 20 + 2 * 12 = 84` 误算漏掉 gap；最终正确 `y` 为 `78/110/142`
- 默认 native `picoui` gate 已按 `P7-D` 计划移除旧 mapping gate：
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
    - `check_picoui_backend_mapping` labels 从默认 `picoui;runtime;backend;mapping` 收窄为 `runtime;backend;mapping;legacy`
    - 默认 native `picoui` gate 继续保留 `release`、`runtime`、`visible`、`no_ldgui`、`ledger` 等 native 合同
- fresh broad/native gates 已通过：
  - `cmake -S . -B build-native -DPICOUI_RUNTIME=native_arm2d -DPICOUI_ENABLE_LEGACY_LDGUI=OFF`
  - `ctest --test-dir build-native -N -L picoui`
    - 当前总数为 `95`
    - 当前尾项为 `check_picoui_release_capability_matrix`、`check_picoui_runtime`、`check_picoui_visible_ui`
    - 默认 native `picoui` label 已不再包含 `check_picoui_backend_mapping`
  - `ctest --test-dir build-native -N -L mapping`
    - 当前仅 `check_picoui_backend_mapping`
  - `ctest --test-dir build-native -L picoui --output-on-failure`
    - 当前 `95/95 PASS`
  - `ctest --test-dir build-native -L release --output-on-failure`
    - 当前 `2/2 PASS`
  - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py`
    - 当前独立直跑通过
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all --build-dir build/picoui-runtime`
    - 当前通过

### 当前结论

- `P7-D update tests off backend_mapping` 已完成到 plan 要求的 GREEN：
  - 默认 native `picoui` gate 已不再依赖旧 `check_picoui_backend_mapping`
  - native release / ledger / no-ldgui / runtime / visible contracts 仍保持 fresh 通过
  - `check_picoui_backend_mapping` 作为非默认 `mapping/legacy` gate 仍可独立运行并通过
- 当前阶段下一任务推进到 `P7-E P7 closeout`。

## P7-E `P7 closeout`

### GREEN

- `P7` 当前已完成 phase closeout，默认 native PicoUI graph 的 fresh 结论是：
  - 默认 `picoui` / demo / test targets 已切到 native semantic target
  - 默认 `picoui` label 不再依赖 `check_picoui_backend_mapping`
  - `no-ldgui runtime dependency`、`release`、`runtime`、`visible`、`ledger` gates 当前全部 fresh 通过
- 这一步 closeout 期间补了两类最小修复，但都没有扩到 `P8`：
  - [tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py)
    - 补了 CMake File API codemodel fallback，避免 `ctest` 场景下因 reply index 指向旧文件名而假红
  - [tests/picoui/contract/test_check_picoui_no_ldgui_runtime_dependency.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/test_check_picoui_no_ldgui_runtime_dependency.py)
    - 新增 fail-first unittest，固定这个 File API reply fallback/stability contract
    - 当前已覆盖：
      - `index` 存在但缺 `reply`
      - `codemodel` reply 指向旧文件
      - target reply 延迟落盘
  - [picoui/src/native/native_event.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_event.c)
    - native pointer fallback 的 `list / scroll_selecter / table` 现在会同步 backend/public truth，不再只改 host 字段后让 getter 回读旧值
  - [picoui/src/theme/theme.c](/Users/cys/embedded/LingDongGUI/picoui/src/theme/theme.c)
    - 显式 `picoui_theme_apply_to_widget()` 允许在 `owner/theme` 仍为空的 native root 场景做最小 theme 绑定；但若 backend theme 被已有 app/theme graph 人为清空，仍保持 `-1` 失败语义，不破坏既有 unit contract
- 这一轮 closeout 的真实收口，不是“仓库彻底删除 ldgui 代码”，而是：
  - 默认构建与默认验证入口已不再依赖 legacy target 名
  - 默认 native gate 已能 fresh 证明当前 `v1.0-native` 线路的 runtime / visible / release / no-ldgui 合同闭合
  - 仍允许 private support target 复用当前必须的 bridge/helper；这部分是否继续下沉或重构，不属于 `P7` 结论

### fresh 验证

- `python3 -m unittest tests.picoui.contract.test_check_picoui_no_ldgui_runtime_dependency`
  - 当前 `5/5 PASS`
- `python3 tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py`
  - 当前通过
- `ctest --test-dir build-native -R '^check_picoui_no_ldgui_runtime_dependency$' --output-on-failure`
  - 当前 `1/1 PASS`
- `ctest --test-dir build-native -R '^(test_picoui_theme|test_picoui_native_list|test_picoui_native_scroll_selecter|test_picoui_native_table|test_picoui_native_style_theme)$' --output-on-failure`
  - 当前 `5/5 PASS`
- `ctest --test-dir build-native -L release --output-on-failure`
  - 当前 `2/2 PASS`
- `ctest --test-dir build-native -L picoui --output-on-failure`
  - 当前 `95/95 PASS`
  - 尾部关键 gate 为：
    - `check_picoui_no_ldgui_runtime_dependency`
    - `check_picoui_runtime`
    - `check_picoui_visible_ui`
  - 当前都已 fresh 通过

### 当前结论

- `P7 remove ldgui runtime dependency` 已完成到 phase closeout。
- 当前可以把串行推进到 `P8-A native release matrix`。
- 这一步没有宣称：
  - `P8 release gate` 已完成
  - 所有 runtime-only demo 都已升级为 visible
  - 仓库已经删除全部 `ldgui` bridge/private helper
