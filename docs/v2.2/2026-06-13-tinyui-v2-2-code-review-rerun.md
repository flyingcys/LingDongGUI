# TinyUI v2.2 二次代码 Review

> 日期：2026-06-13
>
> 范围：上一轮 review 问题修复后的当前工作区。
>
> 更新：本页原始结论基于修复前快照。当前代码问题已修复，focused gate 已转绿；本页保留为二次 review 历史记录，避免与当前状态混淆。
>
> 当前结论：代码层 blocker 已关闭；`S3` 仍是**部分完成**，原因是 active demo tree 仍保留 26 个旧启动骨架，属于阶段未关闭，不再是本页记录的 gate 失败问题。

## 已修复项

### 1. `backend.h` 删除导致 v2.1 guard FileNotFoundError

状态：部分修复。

证据：

- `tests/tinyui/contract/check_tinyui_v21_transition_guards.py` 已在 `backend.h` 不存在时返回空 `backend_compat_includes`
- `rtk python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py --print-current` 已不再抛 `FileNotFoundError`

仍存在问题：该 checker 仍因 inventory count drift 在 CTest 中失败，见下方 blocker 1。

### 2. `picoui/runtime.h` 旧 ABI 链接断裂

状态：已修复。

证据：

- `tinyui/include/picoui/runtime.h` 现在 include `../runtime.h`
- `tinyui/include/runtime.h` 提供 `picoui_*` static inline wrapper，转到 canonical `tinyui_*`
- 手工 legacy probe 编译、链接并运行成功：

```bash
cc -I tinyui/include /tmp/picoui_runtime_probe.c \
  build/libtinyui_backend_ldgui_runtime.a \
  build/libtinyui_core.a \
  build/liblongdonggui_porting_default.a \
  build/liblongdonggui.a \
  build/liblongdonggui_arm2d.a \
  -L/opt/homebrew/lib -lSDL2 \
  -o /tmp/picoui_runtime_probe
```

结果：`exit=0`。

### 3. `tinyui_timer_handler()` 直接 `exit()`

状态：已修复。

证据：

- `tinyui/src/core/runtime.c` 中 `tinyui_timer_handler()` 已改为返回 `<0 / 0 / >0`
- `tinyui/demo/main.c` 的 loop 由 runner 自己处理 `tinyui_deinit()` 和进程返回码
- `tests/tinyui/unit/test_tinyui_runtime_model.c` 已新增 before-init / after-init handler 测试

## 原始问题（已关闭）

### 1. 已关闭：focused gate 失败，两个 transition inventory 计数漂移

位置：

- `tests/tinyui/contract/tinyui_transition_inventory.json:6`
- `tests/tinyui/contract/tinyui_v21_transition_inventory.json:15`
- `tests/tinyui/contract/tinyui_v21_transition_inventory.json:16`

复现命令：

```bash
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
```

原始结果：56 个测试中 4 个失败，其中两个是 transition guard：

- `check_tinyui_transition_guards`：`picoui_public_api_count expected 558 actual 551`
- `check_tinyui_v21_transition_guards`：`picoui_public_api_count expected 559 actual 552`
- `check_tinyui_v21_transition_guards`：`tinyui_public_api_count expected 39 actual 37`

当前状态：

- 已同步 `tinyui_transition_inventory.json`
- 已同步 `tinyui_v21_transition_inventory.json`
- `check_tinyui_transition_guards` / `check_tinyui_v21_transition_guards` 已通过
- `rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'` 已通过

处理结果：关闭。

### 2. 已关闭：runtime / perf gate 依赖旧 demo 路径或旧 benchmark marker

位置：

- `tests/tinyui/runtime/check_tinyui_runtime.py:408`
- `tests/tinyui/runtime/check_tinyui_runtime.py:410`
- `tests/tinyui/perf/check_tinyui_perf.py:155`
- `tests/tinyui/perf/check_tinyui_perf.py:156`

复现命令：

```bash
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
```

原始结果：

- `check_tinyui_runtime` 失败：`missing compile command for tinyui/demo/basic_widgets/main.c`
- `check_tinyui_perf` 失败：缺少 `PICOUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS`

当前状态：

- `check_tinyui_runtime.py` 已切到 `tinyui/demo/main.c`
- unified runner 已补 `PICOUI_RUNTIME_LOOP`
- runtime host 已补 benchmark marker：
  - `PICOUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS`
  - `PICOUI_BENCHMARK_CAPTURE_READY_MS`
  - 兼容 runtime checker 的 `PICOUI_BENCHMARK_SCREEN_CREATE_MS`
  - `PICOUI_BENCHMARK_FIRST_FRAME_MS`
- `check_tinyui_perf` / `check_tinyui_runtime` 已通过

处理结果：关闭。

### 3. 当前剩余事项：S3 未关闭，active demo tree 仍大量保留旧启动骨架

位置：

- `docs/v2.2/线计划索引.md:50`
- `docs/v2.2/线计划索引.md:71`
- `docs/v2.2/线计划索引.md:84`
- `docs/v2.2/plans/stages/README.md:96`
- `docs/v2.2/plans/stages/README.md:104`
- `examples/sdl/CMakeLists.txt:482`

复现命令：

```bash
rtk rg -n 'backend\.h|picoui_app_create|picoui_app_run|run_demo\(|int main\(' tinyui/demo tinyui/src tinyui/include
```

结果：

`tinyui/demo` 下仍大量命中旧启动路径，包括 `hello_world`、`layout_flex`、`layout_grid`、`list_basic`、`arc_basic`、`progress_bar_basic`、`legacy_widget_parity` 等。

当前判断：

这仍与 `docs/v2.2/2026-06-13-tinyui-v2-2-lvgl-like-startup-spec.md` 的完成定义冲突：

- “demo `.c` 已转为 build API 文件”
- “统一 `main` 已成为主线 demo 唯一启动入口”
- “主线 demo 已无各自 `main/run_demo/picoui_app_run` 主路径”

当前处理：

- `docs/v2.2/线计划索引.md`
- `docs/v2.2/plans/stages/README.md`

已改为 `S3 部分完成（未关闭）`，与代码事实一致。

剩余动作：

- 继续迁移所有 active demo target
- 完成 residue scan closeout 后，才能把 `S3` 改回已完成

### 4. 后续增强项：legacy runtime header checker 只做 syntax-only，不能防 ABI/link 回归

位置：

- `tests/tinyui/contract/check_tinyui_public_api.py:227`
- `tests/tinyui/contract/check_tinyui_public_api.py:236`

问题：

`check_tinyui_public_api.py` 新增了 `picoui/runtime.h` direct include probe，但命令是：

```bash
cc -fsyntax-only ...
```

这只能证明 header 可编译，不能证明 ABI / link 可用。上一轮 `picoui/runtime.h` 问题正是“能编译但链接失败”类型。

状态：

- 手工 legacy runtime link probe 已验证通过
- 当前 focused gate 已不再因该问题失败

建议：

把手工 link probe 固化进 CTest 或 checker，至少链接当前 TinyUI runtime 库和 SDL2，覆盖：

```c
#include "picoui/runtime.h"
int main(void) { return picoui_init() != 0 ? tinyui_init() : 0; }
```

## 当前验证记录

已运行：

```bash
rtk git diff --check
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
rtk rg -n 'backend\.h|picoui_app_create|picoui_app_run|run_demo\(|int main\(' tinyui/demo tinyui/src tinyui/include
rtk python3 tests/tinyui/contract/check_tinyui_transition_guards.py --print-current
rtk python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py --print-current
```

结果摘要：

- `git diff --check`：PASS
- focused gate：PASS，56 个测试全部通过
- legacy runtime link probe：PASS
- residue scan：仍大量命中旧 demo startup skeleton

## 最终建议

本页对应的代码 blocker 已关闭，不再维持 `REQUEST_CHANGES` 结论。

当前剩余顺序：

1. 继续迁移剩余 active demo 到 unified runner
2. 跑 S3 residue scan closeout
3. 把 legacy runtime header link probe 固化成自动 gate
