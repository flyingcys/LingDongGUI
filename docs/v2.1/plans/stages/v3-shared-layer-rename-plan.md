# TinyUI v2.1 V3 Shared Layer Rename Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 收口 shared layers 的产品层命名、include path、target 与内部引用，让产品层共享目录统一到 `tinyui`。

**Architecture:** V3 只处理 shared subsystems，不处理 demo/test/contract 全量迁移。重点是把 `core/display/indev/layout/theme/tick/osal` 的产品层路径、头文件引用、target 命名从 `picoui` 收到 `tinyui`，同时保证共享层仍然薄化。

**Tech Stack:** C11、CMake、现有 shared layer 源码、unit/runtime/perf gates。

---

## 文件结构

修改：

- `tinyui/src/core/*`
- `tinyui/src/display/*`
- `tinyui/src/indev/*`
- `tinyui/src/layout/*`
- `tinyui/src/theme/*`
- `tinyui/src/tick/*`
- `tinyui/src/osal/*`
- `tinyui/include/*`
- `cmake/LingDongGUI.cmake`

---

### Task 1: 统一 shared layer 文件与 include path

**Files:**
- Modify: shared source files and headers listed above

- [x] **Step 1: 建立 fail-first path scan**

Use a repository search to identify product-layer shared files that still include or reference `picoui/...` paths after V1/V2:

```bash
rg -n "picoui/" tinyui/src tinyui/include cmake/LingDongGUI.cmake
```

Expected: FAIL/HITS at the start of V3.

当前真相：

- 已在 `V3` 起步时完成 fail-first 扫描，命中过 shared layer 对 `picoui/...` include path 的直接依赖
- 对应命中随后已在 `V3 Task 1 Step 2` 中被清零；当前 shared 源码层不再直接 `#include "picoui/..."`

- [x] **Step 2: 收口 shared layer include**

Replace product-layer include path references so shared files refer to the unified `tinyui` tree instead of the old `picoui` tree.

当前真相：

- `tinyui/src/{core,display,indev,layout,theme,tick,osal}` 已全部改走统一入口，不再直接引用旧 `picoui/...` 路径
- `tinyui/include/*.h` 已补齐 shared 层需要的顶层 wrapper 入口，用于承接统一 include path
- 这一批没有触碰 public `picoui_*` API、`tests/picoui/*` 路径、`test_picoui_*` / `check_picoui_*` 文件名或 CTest label

- [x] **Step 3: 收口 shared layer symbol naming**

Where a symbol is product-layer shared infrastructure rather than `LingDongGUI` engine truth, rename it from `picoui_*` to `tinyui_*`.

Do not rename:

- `ld*`
- `LingDongGUI` directory paths
- engine-private symbols outside the product layer

当前真相：

- internal/shared 命名收口已经完成到当前 `V3` 计划边界，覆盖：
  - `runtime_bridge` internal/shared 接口
  - `window_apply_*`、`native_*`
  - `display/indev` 文件内 static helper
  - `theme.c`、`layout/{flex,grid}.c` pure-static helper
  - `runtime_host.c` internal runtime helper 全链路
  - `core/widget.c` pure-static/internal helper
  - `shared core internal event/callback seam`：`picoui_backend_emit_value_changed()`、`picoui_backend_emit_event()`、`picoui_backend_emit_clicked()` 已统一收口为 `tinyui_widget_emit_*`
  - `theme.c` 的 backend-facing internal helper：`picoui_theme_apply_widget_style()` 已收口为 `tinyui_theme_apply_widget_style()`
  - `shared widget-tree lifecycle seam` 的第一小批：`picoui_backend_widget_is_kind()` 已收口为 `tinyui_widget_is_kind()`
  - `shared widget-tree lifecycle seam` 的第二小批：`picoui_backend_widget_init_root()`、`picoui_backend_widget_init_child()`、`picoui_backend_widget_attach_child()` 已统一收口为 `tinyui_widget_init_root()`、`tinyui_widget_init_child()`、`tinyui_widget_attach_child()`
- 当前 shared 层残留的 `picoui_*` 主要是 public API、public type、或现有 internal/shared entry seam；这些不应在 `V3` 被草率混同为全量 rename，更不应提前跨进 `V4`
- `picoui_list_backend_*` 当前仍属于后续独立小批；`list` 这轮只跟随通用 `tinyui_widget_attach_child()` 切到 shared tree lifecycle contract，没有顺手改 list selected-index 私有 seam

- [x] **Step 4: 跑 focused compile proof**

Run:

```bash
rtk cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build --target \
  test_picoui_runtime_model \
  test_picoui_layout \
  test_picoui_theme \
  test_picoui_native_bridge
```

Expected: PASS。

当前真相：

- 该 focused compile proof 已 fresh 通过
- 对应命令：

```bash
rtk cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
rtk cmake --build build --target \
  test_picoui_runtime_model \
  test_picoui_layout \
  test_picoui_theme \
  test_picoui_native_bridge
```

### Task 2: 收口 shared layer target 与内部接口

**Files:**
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `tests/picoui/CMakeLists.txt`

- [x] **Step 1: 更新 shared layer target naming**

Move product-layer targets toward `tinyui` naming while keeping the build graph intact.

当前真相：

- `cmake/LingDongGUI.cmake` 已建立 `tinyui_core`、`tinyui_backend_ldgui`、`tinyui_backend_ldgui_runtime`、`tinyui_port_sdl` 实名
- `tests/support/CMakeLists.txt` 已建立 `tinyui_test_support` 实名
- 为避免提前撞进 `V4` 的 test/demo/contract/archive 命名面，当前仍保留 `picoui_*` CMake alias 兼容层
- `tests/picoui/CMakeLists.txt` 的 `SUPPORT_LIB` / `MAIN_LIB` / `target_link_libraries(test_picoui_port_sdl ...)` 已切到 `tinyui_*` 实名

- [x] **Step 2: 跑 runtime/perf smoke**

Run:

```bash
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS；说明 shared layer rename 没破坏 runtime/perf proof。

当前真相：

- 该步已 fresh 通过
- `test_picoui_wrapper_struct_overhead` 已从 `perf` label 退回 `probe` label，`ctest -L perf` 不再因为未预构建裸 probe 出现假红
- `backend_widget_struct_bytes` 已从回归态 `536` 压回 `464`，重新低于当前 perf baseline gate `<=512`

- [ ] **Step 3: 更新 V3 closeout 文档**

Document:

- shared layers are now product-layer `tinyui` subsystems
- shared layer retention is intentional and LVGL-like
- backend removal did not collapse shared responsibilities into widgets

当前待办边界：

- 这一步不该等同于继续改代码命名，而是需要把 `V3` 当前阶段真相沉淀到面向 closeout/release 的文档里
- 在未进入 `V4` 前，文档必须明确：
  - shared layer 已按产品层 `tinyui` 子系统收口
  - shared 层保留是有意设计，不是 backend 清零后的残留杂质
  - backend removal 没有把 shared responsibilities 粗暴塌缩进 widgets
