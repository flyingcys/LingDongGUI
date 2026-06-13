# TinyUI v2.2 代码 Review 问题文档

> 日期：2026-06-13
>
> 范围：当前工作区未提交的 `TinyUI v2.2` 改动，重点覆盖 runtime startup canonicalization、`backend.h` 退场、demo runner 迁移、contract / unit test 更新。

## Review 结论

> 更新：本页记录的是首轮 review 时发现的问题。当前其中代码层 blocker 已修复；本页保留为历史问题单。

首轮 review 结论是：当前代码可以通过 targeted build 与 focused tests，但 `v2.2` 不能按现有文档口径判定完成。

首轮主要 blocker 有三类：

1. `check_tinyui_v21_transition_guards` 仍注册在 focused gate 内，但 checker 会读取已删除的 `backend.h`，导致 `ctest -R 'test_tinyui|check_tinyui'` 失败。
2. `tinyui/include/tinyui/runtime.h` 仍把旧 `tinyui_*` 声明当外部 ABI，但 `runtime.c` 已只定义 `tinyui_*`，直接 include legacy runtime header 的旧用户会链接失败。
3. `S3 demo LVGL-like 化` 的代码事实和文档完成结论不一致。统一 runner 只接入了 `basic_widgets` 与 `settings_panel`，但 `tinyui/demo` 下大量 demo 仍保留各自 `main()`、`run_demo()`、`tinyui_app_create()`、`tinyui_app_run()` 主路径。

当前状态摘要：

- `check_tinyui_v21_transition_guards` 已修复
- `tinyui/runtime.h` legacy ABI/link 问题已修复
- `check_tinyui_runtime` / `check_tinyui_perf` 已适配 unified runner 并通过
- `rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'` 已通过
- `S3` 文档口径已下调为“部分完成（未关闭）”
- 剩余事项只在于 active demo tree 尚未全量迁移，不再是本页记录的代码 gate blocker

## GitNexus 影响分析

- `gitnexus_detect_changes(scope=all)`：
  - changed symbols：34
  - changed files：43
  - affected processes：0
  - risk：LOW
- 重点 runtime 符号 impact：
  - `tinyui_init`：LOW，direct callers 0
  - `tinyui_deinit`：LOW，direct callers 0
  - `tinyui_screen_create`：LOW，direct callers 0
  - `tinyui_timer_handler`：LOW，direct callers 0

说明：GitNexus 当前没有把这条新的 runtime surface 映射出调用链，因此本 review 以源码 diff、stage gate 和 fresh build/test 证据共同判断。

## 已验证证据

已通过：

```bash
rtk git diff --check
rtk cmake --build build
rtk cmake --build build --target tinyui_basic_widgets_demo tinyui_settings_panel_demo tinyui_unified_runner_demo test_tinyui_runtime_model
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui_runtime_model|check_tinyui_public_api|test_tinyui_list'
```

未通过：

```bash
rtk ctest --test-dir build --output-on-failure -R 'check_tinyui_v21_transition_guards|check_tinyui_transition_guards'
```

结果：`check_tinyui_transition_guards` PASS，`check_tinyui_v21_transition_guards` FAIL，错误为 `tinyui/src/backend/ldgui/backend.h` 不存在。

```bash
rtk python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py --print-current
```

结果：`FileNotFoundError: tinyui/src/backend/ldgui/backend.h`。

```bash
cc -I tinyui/include -x c -c -o /tmp/tinyui_runtime_probe.o - <<'EOF'
#include "tinyui/runtime.h"
int main(void) { return tinyui_init(); }
EOF
nm -u /tmp/tinyui_runtime_probe.o | rg 'tinyui_init|tinyui_init'
```

结果：probe object 仍引用 `_tinyui_init`；当前库只导出 `_tinyui_init`，没有 `_tinyui_init`。

未通过完成口径：

```bash
rtk rg -n 'backend\.h|tinyui_app_create|tinyui_app_run|run_demo\(|int main\(' tinyui/demo tinyui/src tinyui/include
```

该扫描仍在 `tinyui/demo` 下命中大量旧启动骨架。

## 问题列表

### 1. Blocker：旧 v2.1 transition guard 因 `backend.h` 删除直接失败

位置：

- `tests/tinyui/contract/check_tinyui_v21_transition_guards.py:56`
- `tests/tinyui/contract/check_tinyui_v21_transition_guards.py:101`

问题：

`check_tinyui_v21_transition_guards.py` 在 `collect_actual()` 中无条件读取：

```python
backend_text = (BACKEND_DIR / "backend.h").read_text(encoding="utf-8")
```

但本次 `v2.2` diff 已删除 `tinyui/src/backend/ldgui/backend.h`。因此 checker 不是按“backend.h 已退场”为成功，而是直接抛 `FileNotFoundError`。

影响：

文档建议的 focused gate：

```bash
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
```

会包含 `check_tinyui_v21_transition_guards`，并被该测试拉红。实际复验：

```bash
rtk ctest --test-dir build --output-on-failure -R 'check_tinyui_v21_transition_guards|check_tinyui_transition_guards'
```

结果是 `check_tinyui_transition_guards` PASS，`check_tinyui_v21_transition_guards` FAIL。

建议修复：

把该 checker 更新为 v2.2 语义：`backend.h` 不存在时视为期望状态，并继续检查 `backend_c_files=0`、active include 为零、top-level wrapper 和 public API count 与 inventory 一致。或者新增 v2.2 checker 并从当前 focused gate 中移除旧 v2.1 checker。不能只更新 inventory 数字。

### 2. Blocker：`tinyui/runtime.h` 兼容头链接到已不存在的 `tinyui_*` ABI

位置：

- `tinyui/include/tinyui/runtime.h:26`
- `tinyui/include/tinyui/runtime.h:32`
- `tinyui/src/core/runtime.c:10`
- `tinyui/src/core/runtime.c:20`
- `tinyui/src/core/runtime.c:30`
- `tinyui/src/core/runtime.c:39`
- `tinyui/src/core/runtime.c:48`

问题：

top-level `tinyui/include/runtime.h` 已改成声明 `tinyui_*` 并用 `static inline` 提供 `tinyui_*` alias。但 legacy 兼容头 `tinyui/include/tinyui/runtime.h` 仍声明外部函数：

```c
int tinyui_init(void);
void tinyui_deinit(void);
struct tinyui_window *tinyui_screen_create(void);
int tinyui_screen_load(struct tinyui_window *screen);
void tinyui_timer_handler(void);
```

并且把 `tinyui_init` 宏映射回 `tinyui_init`。当前 `runtime.c` 只定义 `tinyui_*`，库里没有 `_tinyui_init` 等符号。

影响：

旧用户如果直接 `#include "tinyui/runtime.h"`，代码能编译，但会链接到不存在的 `tinyui_init`。这是 public compatibility break，不是单纯内部 rename。

复验证据：

```bash
nm -g build/libtinyui_core.a build/libtinyui_backend_ldgui_runtime.a | rg '(_)?(tinyui_init|tinyui_init)$'
```

只看到 `_tinyui_init`。

```bash
cc -I tinyui/include -x c -c -o /tmp/tinyui_runtime_probe.o - <<'EOF'
#include "tinyui/runtime.h"
int main(void) { return tinyui_init(); }
EOF
nm -u /tmp/tinyui_runtime_probe.o | rg 'tinyui_init|tinyui_init'
```

probe object 仍引用 `_tinyui_init`。

建议修复：

同步 `tinyui/include/tinyui/runtime.h`：让它声明 canonical `tinyui_*`，再提供 `tinyui_*` static inline wrapper；或者在 `runtime.c` 保留真实 `tinyui_*` ABI wrapper。补一个 direct legacy header compile+link 测试，覆盖 `#include "tinyui/runtime.h"` 后调用 `tinyui_init()` 和 `tinyui_init()`。

### 3. Medium：public API checker 未直接覆盖 legacy `tinyui/*.h` 子树

位置：

- `tests/tinyui/contract/check_tinyui_public_api.py:220`
- `tests/tinyui/contract/check_tinyui_public_api.py:226`

问题：

`check_tinyui_public_api.py` 先取 `tinyui/include/tinyui/*.h` 的文件名，再只遍历 top-level `tinyui/include/*.h` 中同名 header。`resolve_public_header_text()` 读到的是 top-level forwarding 或 top-level header 内容，没有直接检查 `tinyui/include/tinyui/runtime.h` 自身。

影响：

`check_tinyui_public_api.py` 当前能 PASS，但无法捕获 `tinyui/runtime.h` 直连 include 的 ABI 断裂。

建议修复：

新增 legacy subtree direct include matrix。至少加一个 probe：

```c
#include "tinyui/runtime.h"
int main(void) { return tinyui_init(); }
```

并链接当前 TinyUI runtime 库。更稳妥的做法是对 `tinyui/include/tinyui/*.h` 做 direct include / compile / link 覆盖。

### 4. Blocker：S3 已完成结论与 demo 代码事实冲突

位置：

- `docs/v2.2/线计划索引.md:50`
- `docs/v2.2/线计划索引.md:71`
- `docs/v2.2/plans/stages/README.md:94`
- `examples/sdl/CMakeLists.txt:482`
- `examples/sdl/CMakeLists.txt:489`
- `examples/sdl/CMakeLists.txt:503`

问题：

`线计划索引` 写明 `S3 已完成`，但同一文档后面又承认“主线 demo 当前仍多为各自自带 `main()/run_demo()` 的结构”。`examples/sdl/CMakeLists.txt` 也仍把多数 demo target 指向各自目录下的 `main.c`，例如：

- `tinyui_hello_world_demo` -> `tinyui/demo/hello_world/main.c`
- `tinyui_layout_flex_demo` -> `tinyui/demo/layout_flex/main.c`
- `tinyui_list_basic_demo` -> `tinyui/demo/list_basic/main.c`
- 其他大量 `*_basic_demo` 仍同样走旧入口

影响：

这直接违反 `v2.2` spec 的完成条件：

- demo `.c` 已转为 build API 文件
- 统一 `main` 已成为主线 demo 唯一启动入口
- 主线 demo 已无各自 `main/run_demo/tinyui_app_run` 主路径

当前状态最多只能说“`basic_widgets` 和 `settings_panel` 两个 demo 已迁移”，不能说 `S3` 或 `v2.2` 完成。

建议修复：

要么继续迁移所有纳入 `examples/sdl/CMakeLists.txt` 的 TinyUI demo target，让每个 demo 只暴露 `tinyui_demo_xxx_build(screen)`，并由统一 `tinyui/demo/main.c` 持有 lifecycle；要么把文档状态降级为 `S3 部分完成 / remaining demos pending`，并把 closeout gate 改成只覆盖已迁移 demo。

### 5. Blocker：S3 closeout gate 要求残留扫描清空，但当前扫描仍大量命中

位置：

- `docs/v2.2/plans/stages/s3-demo-lvgl-like-runner-plan.md:151`
- `docs/v2.2/plans/stages/s3-demo-lvgl-like-runner-plan.md:162`
- `tinyui/demo/list_basic/main.c:56`
- `tinyui/demo/arc_basic/main.c:51`
- `tinyui/demo/layout_grid/main.c:54`
- `tinyui/demo/clock_basic/main.c:43`
- `tinyui/demo/graph_basic/main.c:74`

问题：

S3 plan 要求执行：

```bash
rg -n 'tinyui_app_create|tinyui_app_run|run_demo\(|int main\(' tinyui/demo
```

并期望 active demo tree 不再显示 per-demo startup skeletons。但当前实际扫描仍命中大量旧入口：

- `run_demo()`
- `int main()`
- `tinyui_app_create()`
- `tinyui_app_run()`

影响：

当前 closeout gate 自身失败。即使 build 和 focused tests 通过，也不能把 S3 视为完成。

建议修复：

把该扫描纳入 CTest 或新增 contract checker，避免只在文档中列命令但不执行。修复后要求扫描只允许 `tinyui/demo/main.c` 作为唯一 `main()`，并禁止其他 demo 目录出现 `run_demo()` / `tinyui_app_*` 主路径。

### 6. High：`tinyui_timer_handler()` 作为 canonical public API 仍直接终止进程

位置：

- `tinyui/include/runtime.h:36`
- `tinyui/src/core/runtime.c:48`
- `tinyui/src/core/runtime.c:57`
- `tinyui/src/core/runtime.c:61`

问题：

`runtime.h` 把 `tinyui_timer_handler()` 定义为 canonical startup loop API，但实现中：

```c
if (step < 0) {
    tinyui_deinit();
    exit(1);
}
if (step > 0) {
    tinyui_deinit();
    exit(0);
}
```

这意味着任何用户按 canonical loop 调用 `tinyui_timer_handler()`，一旦 backend step 返回结束或错误，库函数会直接 `exit()` 整个进程。作为 public runtime handler，这会剥夺宿主处理退出、错误上报、资源收尾和测试断言的控制权。

影响：

`v2.2` 目标是让启动模型更像 LVGL：用户循环里调用 handler。LVGL-like handler 通常不应直接终止宿主进程。当前实现会让嵌入式宿主、测试宿主或未来多实例 runner 无法自行决定退出策略。

建议修复：

将 handler 改成可返回状态，例如 `int tinyui_timer_handler(void)`，返回 `<0` error、`0` running、`>0` finished；由 demo runner 的 `main()` 决定是否 `tinyui_deinit()` 和返回 exit code。若必须保持 `void` API，至少新增 `tinyui_should_quit()` / `tinyui_last_error()` 一类状态查询，避免 public handler 内直接 `exit()`。

### 7. Medium：runtime focused test 没覆盖 canonical loop 的退出语义

位置：

- `tests/tinyui/unit/test_tinyui_runtime_model.c:37`
- `tests/tinyui/unit/test_tinyui_runtime_model.c:48`

问题：

当前 runtime test 只覆盖：

- `tinyui_init()`
- `tinyui_screen_create()`
- `tinyui_screen_load()`
- `tinyui_deinit()`

没有覆盖 `tinyui_timer_handler()`，也没有覆盖 step 返回 `<0` / `>0` 时 public API 的行为。

影响：

`tinyui_timer_handler()` 是 v2.2 canonical startup chain 的核心之一，但最危险的控制流没有被测试保护。现在 `exit()` 行为不会被 focused test 暴露。

建议修复：

补一个可注入 step result 的 runtime test，验证：

- handler 对未初始化 runtime 是 no-op 或返回稳定状态
- step error 不直接杀进程
- step finished 后 runner 可自行 deinit 并返回

如果保留当前 `void + exit()` 设计，也至少要把这个行为写成显式 contract，并用子进程测试证明 exit code。

### 8. Medium：`tinyui_basic_widgets_demo` 与 `tinyui_unified_runner_demo` 构建同一套源文件，target 语义重复

位置：

- `examples/sdl/CMakeLists.txt:485`
- `examples/sdl/CMakeLists.txt:572`

问题：

两个 target 都使用：

- `tinyui/demo/main.c`
- `tinyui/demo/basic_widgets/basic_widgets.c`

区别只在 target 名称。`tinyui_unified_runner_demo` 没有新的运行语义，`tinyui_basic_widgets_demo` 也已经是 unified runner。

影响：

这会让验证和文档口径混乱：到底哪个 target 是 canonical runner proof，哪个 target 是 basic_widgets demo proof 不清楚。后续迁移其他 demo 时容易继续复制 target，而不是收敛到“一个 runner + 一个被选中的 build API”。

建议修复：

保留一个明确 target。推荐让 `tinyui_basic_widgets_demo` 作为当前 runner proof，并删除 `tinyui_unified_runner_demo`；或反过来保留 `tinyui_unified_runner_demo`，把 `tinyui_basic_widgets_demo` 改成 alias / 不再单独构建。文档和 CMake target 命名必须统一。

## 非问题确认

- `tinyui/src/backend/ldgui/backend.h` 已删除，源码 include 扫描未发现新的 `backend.h` 代码 include。
- `tinyui/src/core/runtime_internal.h` 约 237 行，不是把原 1502 行 `backend.h` 原样搬迁；其内容主要是 shared runtime tree、layout cache 和 app state，当前不按“替代 mega-header”判定。
- 全量 `rtk cmake --build build` 通过，说明当前构建面没有立即断裂。

## 建议下一步

1. 先修 S3 口径：选择“全量迁移 demo”或“文档降级为部分完成”，不要保持当前矛盾状态。
2. 修 `check_tinyui_v21_transition_guards` 与 `tinyui/runtime.h` 兼容头，这两个会直接破坏 focused gate / legacy include。
3. 修 `tinyui_timer_handler()` 的 public API 控制权问题，或把 `exit()` 行为变成显式 contract 并补子进程测试。
4. 把 S3 残留扫描变成自动 gate，避免后续再次把局部迁移写成整阶段完成。
