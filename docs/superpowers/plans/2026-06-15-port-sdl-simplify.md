# Port SDL 简化实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `tinyui/port/sdl/runtime_host.c`（1023 行）拆成三个职责清晰的文件，对齐 lv_port_pc_vscode 的 hal_init / timer_handler / CI-observe 分层模型，同时删除死代码 `sdl.c`。

**Architecture:** `hal.c` 只做 SDL 平台适配（窗口、输入、像素呈现）；`step.c` 只做帧步进调度（arm_2d、ldGui、渲染）；`observe.c` 只做 CI/测试基础设施（日志、capture、widget marker）。三者通过私有头文件 `host_internal.h` 共享 state struct 和跨文件函数声明。

**Tech Stack:** C99, SDL2, LingDongGUI / arm_2d, CMake

---

## 文件结构

| 操作 | 路径 |
|------|------|
| 新建 | `tinyui/port/sdl/host_internal.h` |
| 新建 | `tinyui/port/sdl/hal.c` |
| 新建 | `tinyui/port/sdl/observe.c` |
| 新建 | `tinyui/port/sdl/step.c` |
| 删除 | `tinyui/port/sdl/runtime_host.c` |
| 删除 | `tinyui/port/sdl/sdl.c` |
| 删除 | `tinyui/include/port/sdl.h` |
| 删除 | `tests/tinyui/unit/test_tinyui_port_sdl.c` |
| 修改 | `cmake/LingDongGUI.cmake`（tinyui_port_sdl 源文件列表） |
| 修改 | `tests/tinyui/CMakeLists.txt`（删除 test_tinyui_port_sdl 条目） |

## 函数归属表

**`hal.c`**（SDL 平台，纯 SDL 调用，无 LingDongGUI）

| 函数 | runtime_host.c 行号 | 链接性 |
|------|---------------------|--------|
| `tinyui_runtime_host_default_tick_source` | 94 | 外部（step.c 用） |
| `tinyui_runtime_host_default_delay` | 100 | 外部（step.c 用） |
| `tinyui_runtime_host_pixel_to_rgb888` | 448 | 外部（observe.c 用） |
| `tinyui_runtime_host_pixel_to_argb8888` | 468 | `static`（仅 hal.c 内部） |
| `tinyui_runtime_host_ensure_window` | 525 | 外部（step.c 用） |
| `tinyui_runtime_host_present_real_frame` | 658 | 外部（step.c 用） |
| `tinyui_runtime_host_pump_sdl_events` | 879 | 外部（step.c 用） |

**`observe.c`**（CI/测试，printf + capture + widget 分类，零渲染副作用）

| 函数 | runtime_host.c 行号 | 链接性 |
|------|---------------------|--------|
| `tinyui_runtime_host_touch_log_enabled` | 66 | 外部（hal.c 用） |
| `tinyui_runtime_host_benchmark_log_enabled` | 80 | `static` |
| `tinyui_runtime_host_log_screen_create_benchmark` | 152 | 外部（step.c 用） |
| `tinyui_runtime_host_log_first_frame_benchmark` | 172 | 外部（step.c 用） |
| `tinyui_runtime_host_widget_is_supported_real` | 205 | `static` |
| `tinyui_runtime_host_widget_is_real_mapped` | 245 | `static` |
| `tinyui_runtime_host_append_id` | 254 | `static` |
| `tinyui_runtime_host_widget_needs_fallback` | 286 | `static` |
| `tinyui_runtime_host_window_has_real_layout` | 294 | 外部（step.c 用） |
| `tinyui_runtime_host_widget_excludes_formal_mapping` | 309 | `static` |
| `tinyui_runtime_host_widget_allows_smoke_layout` | 323 | 外部（step.c 用） |
| `tinyui_runtime_host_append_widget_ids` | 337 | `static` |
| `tinyui_runtime_host_log_mapping_markers` | 360 | 外部（step.c 用） |
| `tinyui_runtime_host_log_image_source_marker` | 407 | 外部（step.c 用） |
| `tinyui_runtime_host_parse_auto_quit_ms` | 427 | 外部（step.c 用） |
| `tinyui_runtime_host_write_capture` | 473 | 外部（step.c 用） |
| `tinyui_runtime_host_log_runtime_ready` | 834 | 外部（step.c 用） |

**`step.c`**（帧调度，声明 `tinyui_runtime_host_step_app` / `shutdown_app`）

| 函数 | runtime_host.c 行号 | 链接性 |
|------|---------------------|--------|
| `tinyui_runtime_host_runtime_bootstrap` | 106 | `static` |
| `tinyui_runtime_host_runtime_page_init` | 118 | `static` |
| `tinyui_runtime_host_runtime_page_quit` | 123 | `static` |
| `g_tinyui_runtime_host_page` | 192 | `static` |
| `tinyui_runtime_host_state_from_app` | 514 | `static` |
| `tinyui_runtime_host_app_state_from_window` | 646 | `static` |
| `tinyui_runtime_host_apply_real_widget_layout` | 682 | `static` |
| `tinyui_runtime_host_apply_smoke_cursor_layout` | 715 | `static` |
| `tinyui_runtime_host_render` | 730 | `static` |
| `tinyui_runtime_host_prepare_runtime_state` | 772 | `static` |
| `tinyui_runtime_host_prepare_runtime_scene` | 813 | `static` |
| `tinyui_runtime_host_prepare_runtime` | 852 | `static` |
| `tinyui_runtime_host_step_app` | 944 | 外部（internal.h 已声明） |
| `tinyui_runtime_host_shutdown_app` | 985 | 外部（internal.h 已声明） |

---

## Task 1：创建 host_internal.h

**Files:**
- 新建: `tinyui/port/sdl/host_internal.h`

- [ ] **Step 1: 写文件**

```c
/* tinyui/port/sdl/host_internal.h
 * 私有头，只由 hal.c / observe.c / step.c 引用，不对外安装。
 */
#ifndef TINYUI_PORT_SDL_HOST_INTERNAL_H
#define TINYUI_PORT_SDL_HOST_INTERNAL_H

#include "internal.h"
#include "runtime_bridge.h"
#include <SDL.h>
#include "arm_2d.h"
#include "ldGui.h"

/* 共享 state — 每个 tinyui_app 实例分配一个，存在 app_state->runtime_state */
struct tinyui_runtime_host_state {
    SDL_Window    *window;
    SDL_Renderer  *renderer;
    SDL_Texture   *texture;
    COLOUR_INT    *real_pixels;
    uint32_t      *present_pixels;
    arm_2d_tile_t  real_tile;
    Uint32         start_ticks;
    Uint32         screen_create_start_ticks;
    Uint32         screen_create_end_ticks;
    Uint32         auto_quit_ms;
    int            display_width;
    int            display_height;
    int            ready_logged;
    int            capture_written;
    int            static_mapping_logged;
    int            fallback_boundary_logged;
    int            temporary_smoke_logged;
    int            smoke_layout_used;
    int            smoke_layout_marker_logged;
    int            benchmark_screen_create_logged;
    int            benchmark_first_frame_logged;
};

/* ---- hal.c → 跨文件声明 ---- */
unsigned int tinyui_runtime_host_default_tick_source(void *user_data);
void         tinyui_runtime_host_default_delay(unsigned int ms, void *user_data);
uint32_t     tinyui_runtime_host_pixel_to_rgb888(COLOUR_INT pixel);
int          tinyui_runtime_host_ensure_window(struct tinyui_app *app,
                                               struct tinyui_runtime_host_state *state);
void         tinyui_runtime_host_present_real_frame(struct tinyui_runtime_host_state *state);
int          tinyui_runtime_host_pump_sdl_events(struct tinyui_app *app,
                                                 struct tinyui_runtime_host_state *state);

/* ---- observe.c → 跨文件声明 ---- */
int  tinyui_runtime_host_touch_log_enabled(void);
void tinyui_runtime_host_log_screen_create_benchmark(struct tinyui_runtime_host_state *state);
void tinyui_runtime_host_log_first_frame_benchmark(struct tinyui_runtime_host_state *state);
int  tinyui_runtime_host_window_has_real_layout(const struct tinyui_backend_widget *widget);
int  tinyui_runtime_host_widget_allows_smoke_layout(const struct tinyui_backend_widget *widget);
void tinyui_runtime_host_log_mapping_markers(struct tinyui_runtime_host_state *state,
                                             const struct tinyui_backend_widget *root);
void tinyui_runtime_host_log_image_source_marker(const struct tinyui_backend_widget *widget);
Uint32 tinyui_runtime_host_parse_auto_quit_ms(void);
int  tinyui_runtime_host_write_capture(struct tinyui_runtime_host_state *state);
void tinyui_runtime_host_log_runtime_ready(struct tinyui_app *app,
                                           struct tinyui_runtime_host_state *state);

#endif /* TINYUI_PORT_SDL_HOST_INTERNAL_H */
```

- [ ] **Step 2: 验证头文件可解析**

```bash
cd /home/share/samba/flyingcys/LingDongGUI
python3 -c "import pathlib; print(pathlib.Path('tinyui/port/sdl/host_internal.h').read_text()[:50])"
```

期望：输出文件前几个字符，无报错。

- [ ] **Step 3: Commit**

```bash
git add tinyui/port/sdl/host_internal.h
git commit -m "refactor(port/sdl): add host_internal.h with shared state struct"
```

---

## Task 2：创建 hal.c

**Files:**
- 新建: `tinyui/port/sdl/hal.c`
- 参考: `tinyui/port/sdl/runtime_host.c` 行 1-30（includes）、行 94-104、行 448-644、行 658-679、行 879-941

- [ ] **Step 1: 写 hal.c**

把以下函数从 `runtime_host.c` 复制到 `hal.c`，其中 `pixel_to_argb8888` 保持 `static`，其余去掉 `static` 关键字（使其成为外部可调用）。

```c
/* tinyui/port/sdl/hal.c
 * SDL 平台适配层 — 对应 lv_port_pc_vscode 的 hal_init()。
 * 只做 SDL 调用：窗口生命周期、输入事件泵、像素呈现。
 * 不含任何 LingDongGUI / arm_2d 调用。
 */
#include "host_internal.h"
#include "observe.h"   /* 仅需 tinyui_runtime_host_touch_log_enabled */
#include "display.h"
#include "tick.h"
#include "osal.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

然后依次粘贴（保持原函数体不变，只去掉 `static`）：

1. `tinyui_runtime_host_default_tick_source`（原 runtime_host.c:94）
2. `tinyui_runtime_host_default_delay`（原 runtime_host.c:100）
3. `tinyui_runtime_host_pixel_to_rgb888`（原 runtime_host.c:448）
4. `static uint32_t tinyui_runtime_host_pixel_to_argb8888`（原 runtime_host.c:468，保留 static）
5. `tinyui_runtime_host_ensure_window`（原 runtime_host.c:525）
6. `tinyui_runtime_host_present_real_frame`（原 runtime_host.c:658）
7. `tinyui_runtime_host_pump_sdl_events`（原 runtime_host.c:879）

注意：`pump_sdl_events` 内部调用了 `tinyui_runtime_host_touch_log_enabled()`，需要引用 `host_internal.h` 中的声明，已在 Step 1 的 include 中覆盖。

- [ ] **Step 2: 确认无语法问题（cmake configure 检测）**

```bash
cd /home/share/samba/flyingcys/LingDongGUI
mkdir -p build-hal-check && cmake -S . -B build-hal-check -DCMAKE_BUILD_TYPE=Debug 2>&1 | grep -i "error\|warning" | head -20
```

期望：此时 configure 不报 syntax error（源文件还未从 cmake 切换，只确认文件能被识别）。

- [ ] **Step 3: Commit**

```bash
git add tinyui/port/sdl/hal.c
git commit -m "refactor(port/sdl): extract hal.c — SDL window, input, pixel ops"
```

---

## Task 3：创建 observe.c

**Files:**
- 新建: `tinyui/port/sdl/observe.c`
- 参考: `tinyui/port/sdl/runtime_host.c` 行 66-92、行 152-190、行 205-426、行 473-511、行 834-850

- [ ] **Step 1: 写 observe.c**

```c
/* tinyui/port/sdl/observe.c
 * CI / 测试基础设施 — 与生产渲染路径零耦合。
 * 包含：环境变量标志、widget 分类谓词、printf 日志、PPM capture。
 */
#include "host_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

然后依次粘贴（所有函数去掉 `static`，除列表中明确标注保持 static 的）：

1. `tinyui_runtime_host_touch_log_enabled`（原:66，去 static）
2. `static tinyui_runtime_host_benchmark_log_enabled`（原:80，保留 static）
3. `tinyui_runtime_host_log_screen_create_benchmark`（原:152，去 static）
4. `tinyui_runtime_host_log_first_frame_benchmark`（原:172，去 static）
5. `static tinyui_runtime_host_widget_is_supported_real`（原:205，保留 static）
6. `static tinyui_runtime_host_widget_is_real_mapped`（原:245，保留 static）
7. `static tinyui_runtime_host_append_id`（原:254，保留 static）
8. `static tinyui_runtime_host_widget_needs_fallback`（原:286，保留 static）
9. `tinyui_runtime_host_window_has_real_layout`（原:294，去 static）
10. `static tinyui_runtime_host_widget_excludes_formal_mapping`（原:309，保留 static）
11. `tinyui_runtime_host_widget_allows_smoke_layout`（原:323，去 static）
12. `static tinyui_runtime_host_append_widget_ids`（原:337，保留 static）
13. `tinyui_runtime_host_log_mapping_markers`（原:360，去 static）
14. `tinyui_runtime_host_log_image_source_marker`（原:407，去 static）
15. `tinyui_runtime_host_parse_auto_quit_ms`（原:427，去 static）
16. `tinyui_runtime_host_write_capture`（原:473，去 static）
17. `tinyui_runtime_host_log_runtime_ready`（原:834，去 static）

注意：`write_capture` 内部调用 `tinyui_runtime_host_pixel_to_rgb888`（在 hal.c 中定义），通过 `host_internal.h` 的声明调用即可。

- [ ] **Step 2: Commit**

```bash
git add tinyui/port/sdl/observe.c
git commit -m "refactor(port/sdl): extract observe.c — CI logging, capture, widget predicates"
```

---

## Task 4：创建 step.c

**Files:**
- 新建: `tinyui/port/sdl/step.c`
- 参考: `tinyui/port/sdl/runtime_host.c` 行 1-65（weak symbols + VT stubs）、行 106-127、行 192-203、行 514-523、行 646-648、行 682-769、行 772-876、行 944-1023

- [ ] **Step 1: 写 step.c**

```c
/* tinyui/port/sdl/step.c
 * 帧调度层 — 对应 lv_port_pc_vscode 的 lv_timer_handler() 内部。
 * arm_2d 初始化、ldGui 帧循环、渲染、自动退出。
 * 对外只暴露两个函数：tinyui_runtime_host_step_app / shutdown_app。
 */
#include "host_internal.h"
#include "arm_2d.h"
#include "arm_2d_helper.h"
#include "ldConfig.h"
#include "ldBase.h"
#include "ldGui.h"
#include "arm_2d_disp_adapter_0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

粘贴以下内容（所有函数保持 `static`，仅最后两个去掉 `static`）：

1. `__attribute__((weak)) VT_enter_global_mutex`（原:43）
2. `__attribute__((weak)) VT_leave_global_mutex`（原:47）
3. `__attribute__((weak)) ldCfgTouchSetPoint`（原:51）
4. `static tinyui_runtime_host_runtime_bootstrap`（原:106）
5. `static tinyui_runtime_host_runtime_page_init`（原:118）
6. `static tinyui_runtime_host_runtime_page_quit`（原:123）
7. `static g_tinyui_runtime_host_page`（原:192）
8. `static tinyui_runtime_host_state_from_app`（原:514）
9. `static tinyui_runtime_host_app_state_from_window`（原:646）
10. `static tinyui_runtime_host_apply_real_widget_layout`（原:682）
11. `static tinyui_runtime_host_apply_smoke_cursor_layout`（原:715）
12. `static tinyui_runtime_host_render`（原:730）
13. `static tinyui_runtime_host_prepare_runtime_state`（原:772）
14. `static tinyui_runtime_host_prepare_runtime_scene`（原:813）
15. `static tinyui_runtime_host_prepare_runtime`（原:852）
16. `tinyui_runtime_host_step_app`（原:944，去 static，此函数在 internal.h 中已声明）
17. `tinyui_runtime_host_shutdown_app`（原:985，去 static）

注意 `TINYUI_RUNTIME_PADDING`、`TINYUI_RUNTIME_ROW_HEIGHT`、`TINYUI_RUNTIME_ROW_GAP` 宏：把原 runtime_host.c 顶部第 33-35 行的三个 `#define` 也复制到 step.c 文件头。

- [ ] **Step 2: Commit**

```bash
git add tinyui/port/sdl/step.c
git commit -m "refactor(port/sdl): extract step.c — frame loop, render, step_app/shutdown"
```

---

## Task 5：更新 cmake，删除死文件

**Files:**
- 修改: `cmake/LingDongGUI.cmake:228-230`
- 修改: `tests/tinyui/CMakeLists.txt:23,72`
- 删除: `tinyui/port/sdl/runtime_host.c`
- 删除: `tinyui/port/sdl/sdl.c`
- 删除: `tinyui/include/port/sdl.h`
- 删除: `tests/tinyui/unit/test_tinyui_port_sdl.c`

- [ ] **Step 1: 更新 cmake/LingDongGUI.cmake**

将：
```cmake
    add_library(tinyui_port_sdl STATIC
        ${LD_REPO_ROOT}/tinyui/port/sdl/sdl.c
        ${LD_REPO_ROOT}/tinyui/port/sdl/runtime_host.c
    )
```
改为：
```cmake
    add_library(tinyui_port_sdl STATIC
        ${LD_REPO_ROOT}/tinyui/port/sdl/hal.c
        ${LD_REPO_ROOT}/tinyui/port/sdl/observe.c
        ${LD_REPO_ROOT}/tinyui/port/sdl/step.c
    )
```

- [ ] **Step 2: 更新 tests/tinyui/CMakeLists.txt**

从 `TINYUI_UNIT_TESTS` 列表中删除这一行：
```cmake
    unit/test_tinyui_port_sdl.c
```

删除文件末尾这一行：
```cmake
target_link_libraries(test_tinyui_port_sdl PRIVATE tinyui_port_sdl)
```

- [ ] **Step 3: 删除死文件**

```bash
rm tinyui/port/sdl/runtime_host.c
rm tinyui/port/sdl/sdl.c
rm tinyui/include/port/sdl.h
rm tests/tinyui/unit/test_tinyui_port_sdl.c
```

- [ ] **Step 4: Commit**

```bash
git add cmake/LingDongGUI.cmake tests/tinyui/CMakeLists.txt
git add -u tinyui/port/sdl/runtime_host.c tinyui/port/sdl/sdl.c
git add -u tinyui/include/port/sdl.h tests/tinyui/unit/test_tinyui_port_sdl.c
git commit -m "refactor(port/sdl): wire hal/observe/step into cmake, delete dead sdl.c + runtime_host.c"
```

---

## Task 6：构建验证 + 合约测试

**Files:** 无变更，纯验证步骤。

- [ ] **Step 1: cmake configure**

```bash
cd /home/share/samba/flyingcys/LingDongGUI
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -5
```

期望：`-- Build files have been written to: .../build-debug`，无 error。

- [ ] **Step 2: 构建 tinyui_port_sdl + tinyui_demo**

```bash
cmake --build build-debug --target tinyui_port_sdl tinyui_demo -j$(nproc) 2>&1 | grep -E "error:|warning:" | grep -v "note:" | head -20
```

期望：无 error，warning 可忽略（来自 arm_2d 第三方代码）。

- [ ] **Step 3: 运行合约测试**

```bash
cd build-debug && ctest -L "tinyui;contract" --output-on-failure 2>&1 | tail -15
```

期望：所有合约测试通过，`check_tinyui_demo_boundary` 报 PASS。

- [ ] **Step 4: 运行单元测试**

```bash
ctest -L "tinyui;unit" --output-on-failure -j$(nproc) 2>&1 | tail -20
```

期望：所有单元测试通过（`test_tinyui_port_sdl` 已删除，不应出现）。

- [ ] **Step 5: 确认文件行数符合预期**

```bash
wc -l tinyui/port/sdl/hal.c tinyui/port/sdl/observe.c tinyui/port/sdl/step.c tinyui/port/sdl/host_internal.h
```

期望：hal.c ≤ 220、observe.c ≤ 430、step.c ≤ 380、host_internal.h ≤ 60。

- [ ] **Step 6: Commit**

无需新 commit（测试只验证，不改代码）。若有修复，按修复内容 commit。
