# TinyUI v2.0 P5 Performance And Memory Guard Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 用真实测量数据守住 `TinyUI` 相对 `LingDongGUI` 的额外开销，确保在性能、速度、RAM 和二进制体积上没有明显退化。

**Architecture:** 把“更轻量”从口头判断变成单独证据线。`P5` 不负责继续改架构，只负责给 `P1` 到 `P4` 的实现建立可重复、可审计的量化基线和守门阈值；`P6` 才负责最终 release-facing closeout。

**Tech Stack:** C11、CMake、SDL2、现有 `picoui` runtime demo、Python gate scripts、`size` 等本机二进制分析工具。

---

## 文件结构

新增：

- `tests/picoui/perf/check_picoui_tinyui_perf.py`
- `tests/picoui/perf/check_picoui_tinyui_binary_size.py`
- `tests/picoui/perf/check_picoui_tinyui_object_overhead.py`
- `tests/picoui/perf/picoui_tinyui_perf_baseline.json`
- `docs/v2.0/v2.0-performance-baseline.md`

修改：

- `tests/picoui/CMakeLists.txt`
- `picoui/demo/basic_widgets/main.c`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `docs/v2.0/线计划索引.md`
- `docs/v2.0/plans/stages/README.md`

---

### Task 1: 固定性能与体积基线

**Files:**
- Create: `tests/picoui/perf/check_picoui_tinyui_binary_size.py`
- Create: `tests/picoui/perf/picoui_tinyui_perf_baseline.json`
- Create: `docs/v2.0/v2.0-performance-baseline.md`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 记录二进制体积 baseline**

Run:

```bash
size build/picoui-runtime/examples/sdl/picoui_basic_widgets_demo
```

Expected: 记录当前 `text/data/bss/dec` 基线到 `picoui_tinyui_perf_baseline.json` 与 `v2.0-performance-baseline.md`，不得写占位值。

- [ ] **Step 2: 建立二进制体积 checker**

Create `tests/picoui/perf/check_picoui_tinyui_binary_size.py`:

```python
# 读取 baseline json 与当前 demo binary 的 `size` 输出
# 比较 text/data/bss/dec 增幅是否超阈值
# 阈值由 baseline 文件显式给出，不得硬编码成“无限制”
```

- [ ] **Step 3: 注册 perf/size gate**

In `tests/picoui/CMakeLists.txt`, add labeled checks:

```cmake
add_test(NAME check_picoui_tinyui_binary_size ...)
set_tests_properties(check_picoui_tinyui_binary_size PROPERTIES LABELS "picoui;perf;size")
```

- [ ] **Step 4: 写 baseline 文档**

Create `docs/v2.0/v2.0-performance-baseline.md`:

```md
# TinyUI v2.0 Performance Baseline

## Binary Size

- command: `size build/picoui-runtime/examples/sdl/picoui_basic_widgets_demo`
- baseline date: `2026-06-07`
- threshold policy: 以百分比和绝对字节双阈值控制，避免只看一个指标
```

- [ ] **Step 5: 跑 size gate**

Run:

```bash
rtk ctest --test-dir build -R 'check_picoui_tinyui_binary_size' --output-on-failure
```

Expected: PASS。

### Task 2: 固定 runtime 速度与对象额外开销

**Files:**
- Create: `tests/picoui/perf/check_picoui_tinyui_perf.py`
- Create: `tests/picoui/perf/check_picoui_tinyui_object_overhead.py`
- Modify: `picoui/demo/basic_widgets/main.c`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 给 demo 增加 benchmark 日志点**

In `picoui/demo/basic_widgets/main.c`, add a guarded benchmark log path:

```c
/* When PICOUI_BENCHMARK_LOG is set, emit:
 * screen_object_create_ms=...
 * capture_ready_ms=...
 * legacy-compatible aliases for runtime checker only
 */
```

Expected: 不改变默认用户路径；只在 benchmark env 打开时输出可解析数据。

- [ ] **Step 2: 建立 runtime perf checker**

Create `tests/picoui/perf/check_picoui_tinyui_perf.py`:

```python
# 运行 basic_widgets demo
# 读取 PICOUI_BENCHMARK_LOG
# 校验 screen_object_create_ms / capture_ready_ms 相对 baseline 不超过阈值
# baseline 缺项时 fail-closed
# 只阻塞 regression，不阻塞更优结果
```

- [ ] **Step 3: 建立对象额外开销 checker**

Create `tests/picoui/perf/check_picoui_tinyui_object_overhead.py`:

```python
# 构建并执行正式 probe target `test_picoui_wrapper_struct_overhead`
# 读取 widget_wrapper_struct_bytes / switch_wrapper_struct_delta_bytes
# 以及可选的 backend_widget_struct_bytes
# baseline 缺项时 fail-closed
# 只阻塞 regression，不阻塞更优结果
```

- [ ] **Step 4: 注册 perf/runtime/memory gate**

In `tests/picoui/CMakeLists.txt`, add:

```cmake
add_test(NAME check_picoui_tinyui_perf ...)
set_tests_properties(check_picoui_tinyui_perf PROPERTIES LABELS "picoui;perf;runtime")

add_test(NAME check_picoui_tinyui_object_overhead ...)
set_tests_properties(check_picoui_tinyui_object_overhead PROPERTIES LABELS "picoui;perf;memory")
```

- [ ] **Step 5: runtime checker 接入 benchmark 模式**

In `tests/picoui/runtime/check_picoui_runtime.py`, allow `basic_widgets` in perf mode to write benchmark artifacts without被判为异常输出。

- [ ] **Step 6: 跑 perf/memory gates**

Run:

```bash
rtk ctest --test-dir build -L 'perf' --output-on-failure
```

Expected: PASS。

### Task 3: 把轻量结论写成 hard gate

**Files:**
- Modify: `docs/v2.0/v2.0-performance-baseline.md`
- Modify: `docs/v2.0/线计划索引.md`
- Modify: `docs/v2.0/plans/stages/README.md`

- [ ] **Step 1: 写清楚阈值口径**

在 `docs/v2.0/v2.0-performance-baseline.md` 中明确：

- 哪些指标拿 `LingDongGUI` 或当前 `picoui` baseline 比较
- 哪些指标允许小幅增长
- 哪些指标一旦越线必须阻塞 `P6`

- [ ] **Step 2: 在线索引中增加性能真相源**

Append to `docs/v2.0/线计划索引.md`:

```md
- 性能与内存真相源：
  - `docs/v2.0/v2.0-performance-baseline.md`
  - `tests/picoui/perf/picoui_tinyui_perf_baseline.json`
```

- [ ] **Step 3: 阶段 README 标明 P6 依赖 P5 证据**

Append to `docs/v2.0/plans/stages/README.md`:

```md
`P6` 不得跳过 `P5`。没有性能/速度/RAM/体积证据，不允许写最终 closeout 结论。
```

- [ ] **Step 4: 跑 broad gates**

Run:

```bash
rtk ctest --test-dir build -L 'picoui|perf' --output-on-failure
rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure
git diff --check
```

Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add \
  docs/v2.0/线计划索引.md \
  docs/v2.0/plans/stages/README.md \
  docs/v2.0/v2.0-performance-baseline.md \
  picoui/demo/basic_widgets/main.c \
  tests/picoui/CMakeLists.txt \
  tests/picoui/runtime/check_picoui_runtime.py \
  tests/picoui/perf/check_picoui_tinyui_binary_size.py \
  tests/picoui/perf/check_picoui_tinyui_perf.py \
  tests/picoui/perf/check_picoui_tinyui_object_overhead.py \
  tests/picoui/perf/picoui_tinyui_perf_baseline.json
git commit -m "test: add tinyui performance guards"
```
