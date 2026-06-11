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

- [ ] **Step 1: 建立 fail-first path scan**

Use a repository search to identify product-layer shared files that still include or reference `picoui/...` paths after V1/V2:

```bash
rg -n "picoui/" tinyui/src tinyui/include cmake/LingDongGUI.cmake
```

Expected: FAIL/HITS at the start of V3.

- [ ] **Step 2: 收口 shared layer include**

Replace product-layer include path references so shared files refer to the unified `tinyui` tree instead of the old `picoui` tree.

- [ ] **Step 3: 收口 shared layer symbol naming**

Where a symbol is product-layer shared infrastructure rather than `LingDongGUI` engine truth, rename it from `picoui_*` to `tinyui_*`.

Do not rename:

- `ld*`
- `LingDongGUI` directory paths
- engine-private symbols outside the product layer

- [ ] **Step 4: 跑 focused compile proof**

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

### Task 2: 收口 shared layer target 与内部接口

**Files:**
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 更新 shared layer target naming**

Move product-layer targets toward `tinyui` naming while keeping the build graph intact.

- [ ] **Step 2: 跑 runtime/perf smoke**

Run:

```bash
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS；说明 shared layer rename 没破坏 runtime/perf proof。

- [ ] **Step 3: 更新 V3 closeout 文档**

Document:

- shared layers are now product-layer `tinyui` subsystems
- shared layer retention is intentional and LVGL-like
- backend removal did not collapse shared responsibilities into widgets
