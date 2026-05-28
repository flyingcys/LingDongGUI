# PicoUI 与 LingDongGUI 测试架构设计文档

> 日期：2026-05-26  
> 适用仓库：`/Users/cys/embedded/LingDongGUI`  
> 目标：为 `PicoUI` 与 `LingDongGUI` 建立统一、可扩展、默认启用的测试管理结构，在不把 SDL demo 测试、库级单元测试、contract 检查混在一起的前提下，形成仓库级测试入口与清晰分层。

---

## 0. 当前主线状态说明

本文既包含目标测试架构，也回写当前主线真实状态。为避免误读，先明确以下边界：

1. **当前已完成**
   - 根 `CMakeLists.txt` 已是推荐 configure 入口，且已接入 `include(CTest)`、`tests/` 子树、`tests/picoui` 与 `tests/lingdonggui` 分层。
   - `tests/picoui/{unit,contract,runtime}` 与 `tests/lingdonggui/unit` 已落地。
2. **当前未完成**
   - 默认根构建仍保留 `LD_BUILD_SDL_DEMO=ON`，所以 `rtk cmake -S . -B build && rtk cmake --build build` 仍会进入 `examples/sdl`，默认行为仍和 SDL 依赖耦合。
   - `examples/sdl/tests/` 还保留部分库级/内部逻辑测试，不是纯粹只剩 demo 行为测试。
3. **当前实现偏差**
   - `runtime` 当前是测试 label 分层，不是默认排除；只要 `BUILD_TESTING=ON` 且 `LD_BUILD_RUNTIME_TESTS=ON`，runtime 测试就会被注册。
   - `tests/picoui/runtime/check_picoui_runtime.py` 使用独立 build tree `build/picoui-runtime`，不是复用常规 `build/`。
4. **推荐使用入口**
   - 常规开发、测试、PicoUI demo 构建建议统一从仓库根入口执行。
   - `examples/sdl` 子目录 configure 目前仍可用，但更适合作为 SDL demo 定向调试或历史兼容路径，而不是主推荐路径。
5. **当前证据层级**
   - `tests/picoui/unit` / `contract` 用于证明 public API、backend contract、style/event/layout 语义。
   - `tests/picoui/runtime/check_picoui_runtime.py` 用于证明 build、启动、capture、退出码与基本回归。
   - runtime/capture 不是“真实 backend 已全部完成”的单独证据，不能替代能力矩阵与单元测试口径。

---

## 1. 背景

当前仓库测试存在两个并行现实：

1. `gui-test-coverage` 分支里已经做过一批有价值的 `LingDongGUI` host 单元测试与根 CMake 骨架。
2. `picoui-abstraction-layer` worktree 里已经做过一批 `PicoUI` 的 smoke/theme/widgets/layout/public-api/demo-boundary/runtime 测试。

但这两批资产当前没有统一落点，主要问题有：

- 根入口虽已建立，但默认构建行为仍会因为 `LD_BUILD_SDL_DEMO=ON` 进入 SDL 子树，导致仓库根测试入口仍和 SDL 依赖耦合。
- `examples/sdl/CMakeLists.txt` 同时承担 demo 构建、部分库级/内部逻辑测试注册、Python contract、runtime 检查，文件职责仍偏重。
- `examples/sdl/tests/` 仍保留一部分库级/内部逻辑测试，和 SDL demo 行为测试职责没有完全分离。
- `PicoUI` 测试 target 重复手抄大量源文件列表，后续扩控件时维护成本高。
- 旧分支中有可复用测试骨架，但也混入了大量 `build/*`、coverage 产物和临时文件，不适合原样搬运。
- 用户要求“默认编译的时候不需关闭单元测试”，当前主线已默认 `BUILD_TESTING=ON`，但“默认构建不受 SDL 影响”这一点尚未实现。

因此本轮目标不是“再加几条 test”，而是先建立一套仓库级测试架构，把已有资产收拢到长期可维护的结构里。

---

## 2. 目标与非目标

### 2.1 目标

本轮测试架构需要满足：

1. `LingDongGUI` 与 `PicoUI` 都有专属测试区域。
2. 单元测试、contract 检查、runtime 检查按层分开。
3. 默认 configure/build 时不需要显式关闭测试。
4. `ctest` 可以按 label 选择性执行不同层次测试。
5. 目标方向上，SDL demo 测试只负责 SDL demo 行为；但当前主线仍允许少量内部逻辑测试暂存于 SDL 子树。
6. 最大化复用 `gui-test-coverage` 与 `picoui-abstraction-layer` 现有测试资产。

### 2.2 非目标

本轮明确不做：

- 不在本轮引入完整 screenshot baseline / visual regression 系统。
- 不在本轮重写 `LingDongGUI` 内核实现。
- 不在本轮引入外部第三方测试框架（如 gtest / Catch2）。
- 不在本轮把所有 demo 行为测试都迁离 `examples/sdl/tests/`。
- 不在本轮解决所有控件 coverage 缺口；先搭结构，再逐步补测试。

---

## 3. 设计原则

### 3.1 测试归属按“被验证对象”划分

- 验证 `LingDongGUI` core/widget/layout 行为的测试，归 `tests/lingdonggui/`。
- 验证 `PicoUI` public API、布局、theme、widget contract 的测试，归 `tests/picoui/`。
- 验证 SDL demo 行为、输入/渲染节奏、截图矩阵的测试，继续归 `examples/sdl/tests/`。
- 当前偏差：`examples/sdl/tests/` 里仍存在 `ldswitch_internal_test`、`ldswitch_widget_test`、`uiwidget_swipe_logic_test`、`layout_window_test` 这类内部逻辑或库级测试。

### 3.2 测试层次按成本与稳定性划分

- `unit`：纯 C 可执行测试，运行快，默认编译，默认纳入常规 `ctest`。
- `contract`：Python 或脚本检查，验证 public API 边界、demo 边界、目录/命名合同。
- `runtime`：需要实际触发 configure/build/demo target 的检查，当前默认注册；这里的“分层”含义是按 label 选择执行，不是默认排除。
- `visual`：后续可加，当前仅预留开关与 label，不纳入本轮主闭环。

### 3.3 公共构建逻辑只放一处

重复的 source list、include dirs、compile definitions、test helper 注册逻辑，统一收敛到根 `CMakeLists.txt` 与 `cmake/LingDongGUI.cmake`。这是推荐目标；当前 `examples/sdl/CMakeLists.txt` 仍承载部分非 demo 纯行为测试注册。

### 3.4 默认启用测试，但不默认执行重测试

“默认不需关闭单元测试”解释为：

- 默认 `BUILD_TESTING=ON`。
- 默认单元测试 target 可参与构建。
- 默认 `ctest` 可运行 unit/contract/runtime。
- runtime/visual 通过 label 控制的是“是否执行”，不是“是否注册”。
- 当前偏差：由于 `LD_BUILD_SDL_DEMO=ON` 默认开启，默认 build 仍会解析 SDL 相关目标与依赖。

---

## 4. 目标目录结构

```text
tests/
  CMakeLists.txt

  support/
    CMakeLists.txt
    ldgui_test_support.c
    ldgui_test_support.h
    picoui_test_support.c
    picoui_test_support.h

  lingdonggui/
    CMakeLists.txt
    unit/
      test_ldbase.c
      test_ldlabel.c
      test_ldprogressbar.c
      test_layout_window.c
      test_ldswitch_internal.c
      test_ldswitch_widget.c

  picoui/
    CMakeLists.txt
    unit/
      test_picoui_smoke.c
      test_picoui_theme.c
      test_picoui_widgets.c
      test_picoui_layout.c
    contract/
      check_picoui_public_api.py
      check_picoui_demo_boundary.py
    runtime/
      check_picoui_runtime.py

examples/sdl/tests/
  layout/
  switch/
  swipe/
  check_use_demo_*.py
  check_stage_*.sh
```

### 4.1 目录责任

- `tests/support/`：公共 stub、fixture、timestamp/mock helper。
- `tests/lingdonggui/unit/`：`LingDongGUI` 库级 host 单测。
- `tests/picoui/unit/`：`PicoUI` 纯 C 单元测试。
- `tests/picoui/contract/`：public API / demo 边界 / 命名与泄漏检查。
- `tests/picoui/runtime/`：实际 configure/build 某 demo target 的运行时检查。
- `examples/sdl/tests/`：目标上应收敛为 SDL demo 专属行为测试；当前仍保留少量内部逻辑/库级测试。

---

## 5. CMake 架构

### 5.1 根入口

根 `CMakeLists.txt` 成为仓库 canonical 入口，负责：

- `include(CTest)`
- 默认 `BUILD_TESTING=ON`
- 定义 `LD_BUILD_SDL_DEMO`、`LD_BUILD_RUNTIME_TESTS`、`LD_BUILD_VISUAL_TESTS`、`LD_ENABLE_COVERAGE`
- `include(cmake/LingDongGUI.cmake)`
- 定义 core targets
- `if(LD_BUILD_SDL_DEMO) add_subdirectory(examples/sdl) endif()`
- `if(BUILD_TESTING) add_subdirectory(tests) endif()`

当前主线对这套入口的真实解释是：

- 推荐 configure 入口已经是仓库根，而不是 `examples/sdl` 子目录。
- 但根入口默认仍会因为 `LD_BUILD_SDL_DEMO=ON` 继续进入 SDL 子树，所以“根入口已统一”和“默认已去 SDL 耦合”不是同一件事。

### 5.2 公共 CMake 模块

`cmake/LingDongGUI.cmake` 负责：

- 汇总 `LingDongGUI`、Arm-2D、misc、demo 的 source list / include dirs。
- 定义 `longdonggui_arm2d`、`longdonggui`、`longdonggui_porting_default`、`longdonggui_host`。
- 定义 `picoui_core`、`picoui_backend_ldgui`。
- 封装公共 helper：
  - `ld_add_c_unit_test(...)`
  - `ld_add_python_test(...)`
  - `ld_add_shell_test(...)`
- 统一 compile definitions / compile options / coverage options。

### 5.3 测试支撑库

新增两类 support library：

- `ldgui_test_support`
  - 复用旧分支 `test_support.c` 思路
  - 提供 host 时间戳、异步等待 stub、scene update stub
- `picoui_test_support`
  - 提供 `PicoUI` 测试公用 helper
  - 链接 `picoui_core` 与 `picoui_backend_ldgui`

### 5.4 测试注册方式

- 每个 C 单测文件 -> 一个独立 `add_executable()`
- 每个 Python/shell contract/runtime 检查 -> 一个独立 `add_test()`
- 每个 test 都设置 label，便于 `ctest -L ...` 分层执行

---

## 6. 目标测试分层

### 6.1 LingDongGUI / unit

直接复用或迁移以下旧资产：

- `test_ldbase.c`
- `test_ldlabel.c`
- `test_ldprogressbar.c`
- `test_layout_window.c`
- 后续再逐步接入 `ldSwitch`、grid、flex 相关库级测试

这层主要守：

- widget/tree helper 行为
- dirty flag / layout dirty 传播
- label/progressbar 基础 contract
- flex/grid setter 语义
- 不依赖 SDL 窗口的库级逻辑

### 6.2 PicoUI / unit

迁移当前 worktree 中已有测试：

- `test_picoui_smoke.c`
- `test_picoui_theme.c`
- `test_picoui_widgets.c`
- `test_picoui_layout.c`

这层主要守：

- app/window/theme 生命周期
- widget setter 与 callback contract
- flex/grid public API contract
- PicoUI 对外最小闭环

### 6.3 PicoUI / contract

迁移：

- `check_picoui_public_api.py`
- `check_picoui_demo_boundary.py`

这层主要守：

- public header 不泄漏 `ld*` / `arm_2d_*` / `SIGNAL_*`
- demo 只使用 `picoui_*` API
- required demo 集合完整

### 6.4 PicoUI / runtime

迁移：

- `check_picoui_runtime.py`
- `check_picoui_visible_ui.py`

这层主要守：

- 仓库级 configure/build 流程能构建 PicoUI runtime/demo target
- `PicoUI` 不只是“头文件过了”，而是至少具备最小 buildable runtime 闭环
- 当前还要明确：这层只证明 smoke / 启动 / capture / 回归，不直接证明 `backend_app.c` 已完全退出正式职责

#### 6.4.1 backend correctness gate

`backend correctness gate` 证明 `PicoUI -> LingDongGUI` 的对象、属性、事件、布局、theme 映射语义成立。主要证据来自：

- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/unit/test_picoui_layout.c`
- `tests/picoui/unit/test_picoui_theme.c`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`

这层回答“有没有落到真实 LingDongGUI 对象和行为”，不回答“真实窗口里是否已经可读、可判定”。

#### 6.4.2 smoke gate

`smoke gate` 证明 demo target 能构建、能进入 runtime loop、能退出、能产出非空 capture。主要证据来自：

- `tests/picoui/runtime/check_picoui_runtime.py`

这层只能证明启动链、capture 链和最小回归链成立。`capture` 非空、像素非空、marker 出现，都不能直接外推为 `visible correctness`。

#### 6.4.3 visible gate

`visible gate` 证明真实可见结果达到“可正常显示、可读、可判定”的最低门槛，并且显示内容与预期控件树不出现明显错位。当前 `B线 Task 1 / G2` 先建立独立入口：

```bash
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets
```

当前该入口是 `B1` 的 RED 基线：必须先失败在 visible correctness 语义上，例如近黑/可读性异常、结构覆盖不足、重复列或控件树呈现不一致；失败原因不应是无法构建、无法启动、无 capture 或无像素。

在 `B2/B3` 修正显示链、颜色链与 `basic_widgets` 可见正确性之前，`check_picoui_visible_ui.py` 不注册进默认 `CTest` 绿灯集合，避免把预期失败的 visible 基线混入 smoke/runtime 通过口径。

### 6.5 当前 PicoUI 能力矩阵与测试映射

| 能力 | 当前状态 | 主要证据层 |
| --- | --- | --- |
| static widget 真实 backend 映射 | 已完成 | `tests/picoui/unit/test_picoui_widgets.c` + `check_picoui_backend_mapping.py` |
| interactive widget 真实 backend 映射 | 已完成 | `tests/picoui/unit/test_picoui_widgets.c` + runtime marker |
| flex/grid 真实 layout 映射 | 已完成 | `tests/picoui/unit/test_picoui_layout.c` |
| native event/message pipeline | 已完成 | `tests/picoui/unit/test_picoui_button_events.c` + `test_picoui_widgets.c` |
| `window/button/checkbox/switch/slider/label/text` theme/style apply | 已完成 | `tests/picoui/unit/test_picoui_theme.c` |
| `image` theme/style apply | 当前拒绝 | `tests/picoui/unit/test_picoui_theme.c` 明确锁定拒绝语义 |
| demo 启动 / capture / smoke | 已完成 | `tests/picoui/runtime/check_picoui_runtime.py` |
| `basic_widgets` visible correctness | B1 RED 基线已建立，当前未收口 | `tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets` |
| `backend_app.c` 最终退场 | 已收缩为 temporary smoke path | `A7` 当前要求是去掉 fake renderer 主职责，不要求文件完全消失 |

---

## 7. 旧资产复用策略

### 7.1 直接复用

来自 `flyingcys/gui-test-coverage`：

- `src/gui/test/test_support.c`
- `src/gui/test/test_ldbase.c`
- `src/gui/test/test_ldlabel.c`
- `src/gui/test/test_ldprogressbar.c`
- `src/gui/test/test_layout_window.c`
- `src/gui/test/CMakeLists.txt` 中的 host-test helper 思路
- 根 `CMakeLists.txt` / `cmake/LingDongGUI.cmake` 的分层结构思路

来自 `.worktree/picoui-abstraction-layer`：

- `examples/sdl/tests/picoui/*.c`
- `examples/sdl/tests/check_picoui_public_api.py`
- `examples/sdl/tests/check_picoui_demo_boundary.py`
- `examples/sdl/tests/check_picoui_runtime.py`

### 7.2 只借思路，不直接搬运

- coverage target 思路可保留
- 但 coverage 产物、构建目录、`.gcda/.gcno` 不迁
- `build/*`、`.tmp_gui_tests/*` 一律不作为资产导入主线

### 7.3 不原样复用的部分

- 不继续把 `PicoUI` 单测留在 `examples/sdl/tests/picoui/`
- 不继续在 `examples/sdl/CMakeLists.txt` 手抄多份 `PicoUI` 源文件列表
- 不把旧分支 `LD_BUILD_TESTS=OFF` 的默认行为带回主线

---

## 8. 默认行为定义

### 8.1 configure / build

推荐默认入口：

```bash
rtk cmake -S . -B build
rtk cmake --build build
```

当前主线下的真实预期：

- `LingDongGUI` 与 `PicoUI` 相关库可编
- unit test executable 可编
- contract/runtime tests 被注册到 `CTest`
- build 默认仍依赖 SDL 相关目标可配置
- 不要求 build 阶段自动执行所有测试

### 8.2 ctest 层次

- `rtk ctest --test-dir build -L unit`
- `rtk ctest --test-dir build -L contract`
- `rtk ctest --test-dir build -L runtime`
- `rtk ctest --test-dir build -L lingdonggui`
- `rtk ctest --test-dir build -L picoui`

默认常规开发建议以 `unit + contract` 为主；`runtime` 单独触发。当前 `runtime` 测试会被注册，且 `check_picoui_runtime.py` 会在独立的 `build/picoui-runtime` 目录中执行 configure/build/run。

---

## 9. 风险与对策

### 9.1 风险：根 CMake 与 `examples/sdl/CMakeLists.txt` 责任重叠

对策：

- 先把公共 target 定义统一到 `cmake/LingDongGUI.cmake`
- `examples/sdl/CMakeLists.txt` 只保留 SDL 目标与 SDL 专属测试

### 9.2 风险：PicoUI test target 迁移后 include/link 关系断裂

对策：

- 先抽 `picoui_core` / `picoui_backend_ldgui` 两层 target
- 再迁 PicoUI 单测
- 不边迁目录边继续复制源列表

### 9.3 风险：旧分支测试依赖 host stub，迁移后缺符号

对策：

- 先迁 `ldgui_test_support.c/.h`
- 先让 `LingDongGUI` host 单测恢复为独立可运行闭环

### 9.4 风险：默认启用测试导致日常 build 过重

对策：

- 默认编译 unit targets，但不自动运行全部 runtime/visual
- 通过 label 分层执行，避免把重测试塞进每次本地 build

---

## 10. 验收标准

本轮 spec 的完成标准：

1. 仓库根存在统一测试入口 `tests/`。
2. `LingDongGUI` 与 `PicoUI` 测试目录分开。
3. `examples/sdl/tests/` 中的 PicoUI 库级单测已迁出；但 SDL 子树内仍允许少量尚未迁完的内部逻辑测试，文档必须显式标注。
4. 根 CMake 默认 `BUILD_TESTING=ON`，无需手工关单测；同时要明确默认 build 仍与 SDL 依赖耦合。
5. `ctest -N` 能看到按 label 分层注册的测试，其中 `runtime` 为“已注册、推荐按需执行”。
6. 至少一组 `LingDongGUI unit` 与一组 `PicoUI unit/contract/runtime` 能跑通；其中 `PicoUI runtime` 使用独立 build tree。

---

## 11. 本轮实施边界

为了控制范围，本轮实施顺序固定为：

1. 先搭根 CMake + `tests/` 骨架
2. 先迁 `LingDongGUI` 旧 host 单测
3. 再抽 `PicoUI` 公共 target
4. 再迁 `PicoUI` 单测 / contract / runtime
5. 最后瘦身 `examples/sdl/CMakeLists.txt`

不跳步骤；不先做大规模 demo 测试迁移；不把“测试重构”顺手扩成“全仓构建系统重写”。
