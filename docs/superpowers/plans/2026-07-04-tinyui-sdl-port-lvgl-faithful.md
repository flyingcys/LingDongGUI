# TinyUI SDL Port — LVGL-Faithful 重构实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 TinyUI 的帧循环像 LVGL 一样完全归 core,SDL 退化为带 `create()` 的驱动;删除 port 端的转发壳、隐藏链接契约与 `step.c` 的通用编排,并把 `observe.c` 测试脚手架移出生产 port。

**Architecture:** 对齐 LVGL `lv_port_pc_vscode` 模型——`lv_timer_handler()` 在 core 全包,只回调 `flush_cb`/`read_cb`,SDL 通过 `lv_sdl_window_create()`/`lv_sdl_mouse_create()` 注册能力。TinyUI 把已下沉的 `tinyui_backend_neutral_step` 升为**唯一**帧循环,新增 display `present_cb` 与 indev `read_cb` 两个能力回调;SDL 的窗口/present/事件泵(现 `hal.c`)封装为 `tinyui_sdl_window_create`/`tinyui_sdl_mouse_create`/`tinyui_sdl_quit`。

**Tech Stack:** C11、CMake、SDL2、ARM-2D/LingDongGUI backend。

## Global Constraints

- **不得引入 weak 符号做平台分派**(静态库 archive 抽取顺序坑,已被上一轮否决)。跨库调用一律走强符号 + 运行期注册回调。
- **每个 Task 结束必须过验证闸门**(单元测试基线在本分支已坏,改用 build + contract + demo 冒烟,见 [[tinyui-build-test-gate]]):
  - 构建:`cmake --build build-debug --target tinyui_demo --target ldgui_sdl_demo`,exit 0。
  - 合约:`ctest --test-dir build-debug -R check_tinyui -E "runtime|perf|binary_size|object_overhead|visible_ui|backend_mapping"` → **7/7**。
  - 冒烟(headless):`cd build-debug/examples/sdl && TINYUI_DEMO_AUTO_QUIT_MS=600 TINYUI_CAPTURE_FILE=x.ppm SDL_VIDEODRIVER=dummy ./tinyui_demo theme_showcase` → exit 0,capture **460815** 字节;`table_basic` 同。与基线 PPM 逐字节一致(见 scratchpad `base_*.ppm`)。
  - MCU 无 SDL:`cmake --build build-debug --target mcu_host_smoke && ./build-debug/tests/tinyui/port_mcu/mcu_host_smoke` → `MCU_HOST_SMOKE_OK`,且 `nm` 显示 **0 个 SDL 符号**。
- **SDL 默认行为逐字节不回归**;`tinyui_demo`/`ldgui_sdl_demo` 显示与交互不变。
- 已知无关失败:`test_tinyui_runtime_model`(ARM_2D_FONT/`tinyui_backend_step` 链接)是分支既有坏目标,不在闸门内,不修。
- 采用 CMake;无 backend;worktree 合并可忽略主仓库文档自动变更。

**基线事实(代码实测,供 interface 精确对齐)**:
- `struct tinyui_app`(`tinyui/src/core/internal.h`)含 `void *runtime_state; struct tinyui_display_port_state display_port; struct tinyui_input_port_state input_port; struct tinyui_tick_port_state tick_port; struct tinyui_os_port_state os_port;`。
- `struct tinyui_display_port_state`(`internal.h:148`):`{ struct tinyui_display_config config; tinyui_display_flush_cb_t flush_callback; void *flush_user_data; }`。
- `struct tinyui_input_port_state`(`internal.h:154`):`{ int pointer_x, pointer_y, pointer_pressed; enum tinyui_input_key key; int key_pressed; }`。
- `tinyui_display_flush_cb_t = void(*)(const struct tinyui_area*, const void*, void*)`(`display.h`)。
- `tinyui_tick_get_cb_t = unsigned int(*)(void*)`;`tinyui_tick_get(app)`、`tinyui_os_delay(app,ms)`、`tinyui_app_pump_timers(app,now)` 均已存在。
- 隐藏契约:`internal.h:256-257` 声明 `tinyui_runtime_host_step_app`/`shutdown_app`;`runtime_bridge.c:243/256` 调用;仅 `port/sdl/step.c:193/236` 与 `port/mcu/tinyui_port_mcu.c:26/31` 定义。
- 现中性循环:`tinyui/src/drivers/tinyui_ldgui_neutral_runtime.c` 的 `tinyui_backend_neutral_step/shutdown`(强符号,编入 `tinyui_backend_ldgui_porting`),声明在 `tinyui/src/drivers/tinyui_ldgui_port.h:20-21`。
- SDL 现成可复用件(`port/sdl/hal.c`):`tinyui_runtime_host_copy_flush_pixels`(flush)、`tinyui_runtime_host_present_real_frame`(present)、`tinyui_runtime_host_pump_sdl_events`(事件泵)、`tinyui_runtime_host_default_tick_source`(SDL_GetTicks)、`tinyui_runtime_host_ensure_window`(建窗口/缓冲)。
- 测试脚手架:`port/sdl/observe.c`(~403 行,capture/auto-quit/marker/benchmark)+ `host_internal.h` 的 ~13 个测试状态位。
- demo 入口:`tinyui_demo/main.c` 已是 `tinyui_init()`→`tinyui_display_set_default_config()`→`tinyui_demos_create()`→`for(;;){ int step=tinyui_timer_handler(); ... if(step)break; }`。

---

## File Structure

**新增**
- `tinyui/port/sdl/tinyui_sdl.h` — SDL 驱动公共头(`tinyui_sdl_window_create`/`tinyui_sdl_mouse_create`/`tinyui_sdl_quit`)。
- `tests/tinyui/runtime/tinyui_sdl_observe.c` + `.h` — 由 `observe.c` 迁出的测试脚手架(仅 `ENABLE_TEST`)。

**修改**
- `tinyui/include/display/display.h` — 加 `present_cb` typedef + setter。
- `tinyui/include/indev/indev.h` — 加 `read_cb` typedef + setter。
- `tinyui/src/core/internal.h` — 结构体加 `present_callback`/`read_callback` 字段;删 256-257 契约,改声明 neutral。
- `tinyui/src/display/display.c` — 实现 `tinyui_display_set_present_callback`。
- `tinyui/src/indev/indev.c` — 实现 `tinyui_input_set_read_callback`。
- `tinyui/src/drivers/tinyui_ldgui_neutral_runtime.c` — 中性 step 升为 LVGL 式循环(read_cb→pump→backend_step→present_cb→delay)。
- `tinyui/src/drivers/tinyui_ldgui_port.h` — 更新注释(不再是"port 转发契约")。
- `tinyui/src/core/runtime_bridge.c` — `step_app`/`shutdown_app` 直接调 neutral。
- `tinyui/port/sdl/hal.c` — 加 `tinyui_sdl_window_create`/`mouse_create`/`quit`;`host_internal.h` 精简。
- `tinyui/port/mcu/tinyui_port_mcu.c` — 删两个转发壳(保留 arm_2d 时钟 weak 默认)。
- `tinyui_demo/main.c` — 显式调 create/quit;capture/auto-quit 移到 `ENABLE_TEST` 守卫。
- `cmake/LingDongGUI.cmake` — `tinyui_port_sdl` 源去掉 `step.c`;`observe` 仅 `ENABLE_TEST` 经测试库链入。

**删除**
- `tinyui/port/sdl/step.c`(通用编排下沉 core 后无内容)。

---

## Task 1(S3a):core 加 present_cb / read_cb 能力,中性循环改为 LVGL 式(纯加法,SDL 仍走旧路径)

**Files:**
- Modify: `tinyui/include/display/display.h`、`tinyui/include/indev/indev.h`
- Modify: `tinyui/src/core/internal.h:148-160`
- Modify: `tinyui/src/display/display.c`、`tinyui/src/indev/indev.c`
- Modify: `tinyui/src/drivers/tinyui_ldgui_neutral_runtime.c`

**Interfaces:**
- Produces:
  - `typedef void (*tinyui_display_present_cb_t)(void *user_data);`
  - `int tinyui_display_set_present_callback(struct tinyui_app *app, tinyui_display_present_cb_t cb, void *user_data);`
  - `typedef int (*tinyui_input_read_cb_t)(struct tinyui_app *app, void *user_data);`(返回 >0 请求退出 / 0 正常 / <0 错误)
  - `int tinyui_input_set_read_callback(struct tinyui_app *app, tinyui_input_read_cb_t cb, void *user_data);`
  - 中性循环新语义(见下)。
- Consumes:无(纯加法)。

- [ ] **Step 1: display.h 加 present 能力**

在 `tinyui_display_set_flush_callback` 声明后追加:
```c
typedef void (*tinyui_display_present_cb_t)(void *user_data);

/* 帧渲染完成后由 core 循环调用一次;窗口后端在此把缓冲推上屏幕
 * (SDL: texture upload + RenderPresent)。直刷 LCD 的 MCU 通常留 NULL。 */
int tinyui_display_set_present_callback(struct tinyui_app *app,
                                        tinyui_display_present_cb_t callback,
                                        void *user_data);
```

- [ ] **Step 2: indev.h 加 read 能力**

在 `tinyui_input_push_pointer` 声明前追加:
```c
/* core 循环每帧在处理定时器前调用一次;平台在此采集输入
 * (SDL: 泵事件后 tinyui_input_push_pointer)。返回值:>0 请求退出,
 * 0 继续,<0 错误。未注册则 core 跳过采集(app 自行 push)。 */
typedef int (*tinyui_input_read_cb_t)(struct tinyui_app *app, void *user_data);
int tinyui_input_set_read_callback(struct tinyui_app *app,
                                   tinyui_input_read_cb_t callback,
                                   void *user_data);
```

- [ ] **Step 3: internal.h 结构体加字段**

`struct tinyui_display_port_state`(`:148`)末尾加:
```c
    tinyui_display_present_cb_t present_callback;
    void *present_user_data;
```
`struct tinyui_input_port_state`(`:154`)末尾加:
```c
    tinyui_input_read_cb_t read_callback;
    void *read_user_data;
```
(`internal.h` 已 `#include` 对应公共头;若无则加 `#include "indev/indev.h"`/`"display/display.h"`。)

- [ ] **Step 4: display.c / indev.c 实现 setter**

`display.c`(仿 `tinyui_display_set_flush_callback`,`:80`):
```c
int tinyui_display_set_present_callback(struct tinyui_app *app,
                                        tinyui_display_present_cb_t callback,
                                        void *user_data)
{
    if (app == NULL) {
        return -1;
    }
    app->display_port.present_callback = callback;
    app->display_port.present_user_data = user_data;
    return 0;
}
```
`indev.c`(仿 `tinyui_input_push_pointer`,`:11`):
```c
int tinyui_input_set_read_callback(struct tinyui_app *app,
                                   tinyui_input_read_cb_t callback,
                                   void *user_data)
{
    if (app == NULL) {
        return -1;
    }
    app->input_port.read_callback = callback;
    app->input_port.read_user_data = user_data;
    return 0;
}
```

- [ ] **Step 5: 中性循环升为 LVGL 式帧循环**

`tinyui_ldgui_neutral_runtime.c` 的 `tinyui_backend_neutral_step`:保留一次性 setup 段不变(`:91-112`),把每帧段(`:114-121`)替换为:
```c
    /* LVGL 式帧:read_cb(采集/退出) → pump_timers → backend_step(→flush_cb)
     * → present_cb(上屏) → os delay。read_cb/present_cb 均为可选。 */
    if (app->input_port.read_callback != NULL) {
        int rc = app->input_port.read_callback(app, app->input_port.read_user_data);
        if (rc != 0) {
            return rc;   /* >0 退出,<0 错误,直接冒泡给 timer_handler */
        }
    }

    tinyui_app_pump_timers(app, tinyui_tick_get(app));
    tinyui_backend_step(app);

    if (app->display_port.present_callback != NULL) {
        app->display_port.present_callback(app->display_port.present_user_data);
    }

    tinyui_os_delay(app, 16);
    return 0;
```
(hooks 全 NULL 时行为与改造前逐字一致 → MCU 路径不变。)

- [ ] **Step 6: 闸门(此阶段 SDL 仍由 step.c 驱动,验证加法无害)**

Run: 构建 `tinyui_demo`/`ldgui_sdl_demo`(exit 0)+ contract 7/7 + `mcu_host_smoke`(`MCU_HOST_SMOKE_OK`,证明新循环在无 hook 时 MCU 正常)。
Expected: 全绿;capture 460815 与基线一致。

- [ ] **Step 7: Commit**
```bash
git add tinyui/include/display/display.h tinyui/include/indev/indev.h \
        tinyui/src/core/internal.h tinyui/src/display/display.c \
        tinyui/src/indev/indev.c tinyui/src/drivers/tinyui_ldgui_neutral_runtime.c
git commit -m "tinyui(port): 加 display present_cb / indev read_cb,中性循环改 LVGL 式(纯加法)"
```

---

## Task 2(S3b):SDL 退化为驱动 + core 接管循环(高风险落地)

**Files:**
- Create: `tinyui/port/sdl/tinyui_sdl.h`
- Modify: `tinyui/port/sdl/hal.c`、`tinyui/port/sdl/host_internal.h`
- Modify: `tinyui/src/core/runtime_bridge.c:241-263`
- Modify: `tinyui_demo/main.c`
- Modify: `cmake/LingDongGUI.cmake`(`tinyui_port_sdl` 源去掉 `step.c`)
- Delete: `tinyui/port/sdl/step.c`

**Interfaces:**
- Consumes:Task 1 的 `tinyui_display_set_present_callback`、`tinyui_input_set_read_callback`、中性循环。
- Produces:
  - `int  tinyui_sdl_window_create(struct tinyui_app *app, int width, int height);`
  - `int  tinyui_sdl_mouse_create(struct tinyui_app *app);`
  - `void tinyui_sdl_quit(void);`

- [ ] **Step 1: 新建 `tinyui/port/sdl/tinyui_sdl.h`**
```c
#ifndef TINYUI_PORT_SDL_H
#define TINYUI_PORT_SDL_H

struct tinyui_app;

/* 建 SDL 窗口/渲染器/纹理与像素缓冲,并注册 display flush + present + tick。
 * 对齐 LVGL lv_sdl_window_create。返回 0 成功,-1 失败。 */
int tinyui_sdl_window_create(struct tinyui_app *app, int width, int height);

/* 注册 indev read_cb:每帧泵 SDL 事件并 push 指针;SDL_QUIT 时返回 >0。
 * 对齐 LVGL lv_sdl_mouse_create。返回 0 成功,-1 失败。 */
int tinyui_sdl_mouse_create(struct tinyui_app *app);

/* 释放 SDL 窗口/渲染器/纹理与缓冲并 SDL_Quit。对齐 LVGL lv_sdl_quit。 */
void tinyui_sdl_quit(void);

#endif
```

- [ ] **Step 2: hal.c 增加驱动入口(复用现有 SDL 件)**

在 `hal.c` 末尾加(state 用文件静态单实例,匹配单窗口/单 PFB 模型):
```c
#include "tinyui_sdl.h"

static struct tinyui_runtime_host_state s_sdl_state;

/* present_cb:帧渲染完成后把 real_pixels 上屏 */
static void tinyui_sdl_present_cb(void *user_data)
{
    struct tinyui_runtime_host_state *state = (struct tinyui_runtime_host_state *)user_data;
    if (state == NULL || state->renderer == NULL) {
        return;
    }
    SDL_SetRenderDrawColor(state->renderer, 0x2E, 0x34, 0x40, 0xFF);
    SDL_RenderClear(state->renderer);
    tinyui_runtime_host_present_real_frame(state);
    SDL_RenderPresent(state->renderer);
}

/* read_cb:泵 SDL 事件 → push 指针;SDL_QUIT → 返回 1 请求退出 */
static int tinyui_sdl_read_cb(struct tinyui_app *app, void *user_data)
{
    struct tinyui_runtime_host_state *state = (struct tinyui_runtime_host_state *)user_data;
    return tinyui_runtime_host_pump_sdl_events(app, state);  /* 已返回 1 on SDL_QUIT */
}

int tinyui_sdl_window_create(struct tinyui_app *app, int width, int height)
{
    struct tinyui_display_config cfg = {0};

    if (app == NULL) {
        return -1;
    }
    /* 分辨率/格式/PFB 行高由 demo 预先 set_default_config 决定;这里同步 w/h。 */
    (void)tinyui_display_get_config(app, &cfg);
    cfg.width = width;
    cfg.height = height;
    if (tinyui_display_set_config(app, &cfg) != 0) {
        return -1;
    }

    s_sdl_state.display_width = width;
    s_sdl_state.display_height = height;
    if (tinyui_runtime_host_ensure_window(app, &s_sdl_state) != 0) {
        return -1;
    }

    (void)tinyui_display_set_flush_callback(app, tinyui_runtime_host_copy_flush_pixels, &s_sdl_state);
    (void)tinyui_display_set_present_callback(app, tinyui_sdl_present_cb, &s_sdl_state);
    if (app->tick_port.callback == NULL) {
        (void)tinyui_tick_set_source(app, tinyui_runtime_host_default_tick_source, NULL);
    }
    if (app->os_port.delay == NULL) {
        (void)tinyui_os_set_delay_callback(app, tinyui_runtime_host_default_delay, NULL);
    }
    return 0;
}

int tinyui_sdl_mouse_create(struct tinyui_app *app)
{
    if (app == NULL) {
        return -1;
    }
    return tinyui_input_set_read_callback(app, tinyui_sdl_read_cb, &s_sdl_state);
}

void tinyui_sdl_quit(void)
{
    struct tinyui_runtime_host_state *state = &s_sdl_state;
    free(state->present_pixels);
    free(state->real_pixels);
    state->present_pixels = NULL;
    state->real_pixels = NULL;
    if (state->texture)  { SDL_DestroyTexture(state->texture);   state->texture = NULL; }
    if (state->renderer) { SDL_DestroyRenderer(state->renderer); state->renderer = NULL; }
    if (state->window)   { SDL_DestroyWindow(state->window);     state->window = NULL; }
    SDL_Quit();
}
```
> 注:`s_sdl_state` 的 `real_pixels`/`present_pixels`/窗口由 `tinyui_runtime_host_ensure_window` 分配;`display_width/height` 供 flush 裁剪与 present 遍历使用。auto-quit/capture 暂由 Task 2 的 demo 侧临时保留(见 Step 5),Task 4 再彻底移出。

- [ ] **Step 3: runtime_bridge 直接调 neutral(切换循环所有权)**

`runtime_bridge.c`:确保顶部可见 neutral 声明(加 `#include "../drivers/tinyui_ldgui_port.h"` 或在文件内前置声明 `int tinyui_backend_neutral_step(struct tinyui_app*); void tinyui_backend_neutral_shutdown(struct tinyui_app*);`)。改:
```c
int tinyui_runtime_bridge_step_app(struct tinyui_app *app)
{
    return tinyui_backend_neutral_step(app);
}
```
`tinyui_runtime_bridge_shutdown_app`(`:256`)里 `tinyui_runtime_host_shutdown_app(app);` 改为 `tinyui_backend_neutral_shutdown(app);`。

- [ ] **Step 4: 删除 `tinyui/port/sdl/step.c`,CMake 去引用**

`cmake/LingDongGUI.cmake:258-261` 的 `tinyui_port_sdl` 源列表删掉 `${LD_REPO_ROOT}/tinyui/port/sdl/step.c` 一行(保留 `hal.c`、`observe.c`)。删除 `git rm tinyui/port/sdl/step.c`。
> `tinyui_runtime_host_step_app`/`shutdown_app` 随 step.c 消失;runtime_bridge 已不再引用它们。

- [ ] **Step 5: demo main.c 显式安装 SDL 驱动**

`tinyui_demo/main.c`:`#include "tinyui_sdl.h"`;在 `tinyui_demos_create(...)` 成功后、进入 `for(;;)` 前插入:
```c
    if (tinyui_sdl_window_create(g_app_or_via_accessor, display_config.width, display_config.height) != 0 ||
        tinyui_sdl_mouse_create(g_app_or_via_accessor) != 0) {
        tinyui_deinit();
        return 1;
    }
```
> app 句柄:若 demo 无直接 `struct tinyui_app*`,用现有 runtime 全局访问(核对 `tinyui_demos`/`runtime.c` 暴露的取 app 途径;必要时加最小 accessor,不改 public 语义)。循环末尾 `if (step > 0)`/`if (step < 0)` 分支的 `tinyui_deinit()` 之外补 `tinyui_sdl_quit();`。auto-quit(`TINYUI_DEMO_AUTO_QUIT_MS`)与 capture(`TINYUI_CAPTURE_FILE`)本阶段仍由 `observe.c` 经 SDL 驱动内保留(present_cb/read_cb 里临时调用),确保闸门不断;Task 4 再移出。

- [ ] **Step 6: 闸门(完整)**

Run: 构建两 demo(exit 0)+ contract 7/7 + 冒烟 `theme_showcase`/`table_basic`(exit 0,capture 460815,与基线 PPM 逐字节一致)+ `mcu_host_smoke`。
Expected: 全绿且截图与基线一致(SDL 逐字节不回归)。

- [ ] **Step 7: 独立 subagent 评审(CLAUDE.md 强制:结构性落地必须独立评审)**

派 review subagent 检查:present/read/quit 时序是否等价旧 `step.c`;单实例 state 生命周期(create→quit 可重入);`ensure_window` 与 `set_config` 顺序;是否引入 SDL 逐字节差异;是否残留对已删符号的引用。评审不过 → 同一 subagent 内修复,不新开。

- [ ] **Step 8: Commit**
```bash
git add tinyui/port/sdl/tinyui_sdl.h tinyui/port/sdl/hal.c tinyui/port/sdl/host_internal.h \
        tinyui/src/core/runtime_bridge.c tinyui_demo/main.c cmake/LingDongGUI.cmake
git rm tinyui/port/sdl/step.c
git commit -m "tinyui(port/sdl): SDL 退化为 window/mouse/quit 驱动,core 接管帧循环,删 step.c"
```

---

## Task 3(S3c):删除隐藏链接契约与 MCU 转发壳

**Files:**
- Modify: `tinyui/src/core/internal.h:256-257`
- Modify: `tinyui/src/drivers/tinyui_ldgui_port.h`(注释)
- Modify: `tinyui/port/mcu/tinyui_port_mcu.c:24-34`

**Interfaces:**
- Consumes:Task 2 已让 runtime_bridge 走 neutral。
- Produces:`tinyui_runtime_host_step_app`/`shutdown_app` 符号在全仓消失。

- [ ] **Step 1: internal.h 删契约、改声明**

删除 `internal.h:256-257`:
```c
int tinyui_runtime_host_step_app(struct tinyui_app *app);
void tinyui_runtime_host_shutdown_app(struct tinyui_app *app);
```
替换为(core 现直接调 neutral,声明就近可见):
```c
/* 平台无关帧循环(实现于 tinyui_ldgui_neutral_runtime.c,编入
 * tinyui_backend_ldgui_porting);runtime_bridge 直接驱动它。 */
int  tinyui_backend_neutral_step(struct tinyui_app *app);
void tinyui_backend_neutral_shutdown(struct tinyui_app *app);
```
(若 Task 2 Step 3 已用 `#include tinyui_ldgui_port.h` 解决声明,此处仅删除 256-257 两行即可,不重复声明。)

- [ ] **Step 2: 更新 `tinyui_ldgui_port.h` 注释**

把 `:16-19` "A platform port (mcu / none) satisfies the core frame-driver contract (tinyui_runtime_host_step_app / ...) by forwarding to these helpers." 改为准确描述:"core 的 runtime_bridge 直接驱动该帧循环;平台通过 display flush/present + indev read_cb + tick 注册能力,无需定义任何 host 符号。"

- [ ] **Step 3: 删除 MCU 转发壳**

`tinyui_port_mcu.c`:删除 `:26-34` 的 `tinyui_runtime_host_step_app`/`tinyui_runtime_host_shutdown_app` 两个函数及其上方"Core frame-driver link contract"注释。**保留** `arm_2d_helper_get_reference_clock_frequency` weak 默认(`:42-46`,真默认非壳)与 `#include`。更新文件头注释:不再提"port 的 link contract 贡献"。

- [ ] **Step 4: 闸门**

Run: 构建两 demo + contract 7/7 + 冒烟(capture 460815)+ `mcu_host_smoke`;并 `nm build-debug/tests/tinyui/port_mcu/mcu_host_smoke | grep -i SDL` → **空**。
Expected: 全绿;MCU 二进制 0 个 SDL 符号;`grep -rn tinyui_runtime_host_step_app tinyui/` 仅命中历史文档。

- [ ] **Step 5: Commit**
```bash
git add tinyui/src/core/internal.h tinyui/src/drivers/tinyui_ldgui_port.h tinyui/port/mcu/tinyui_port_mcu.c
git commit -m "tinyui(port): 删除 host_step_app/shutdown_app 隐藏链接契约与 MCU 转发壳"
```

---

## Task 4(S2):observe.c 移出生产 port

**Files:**
- Create: `tests/tinyui/runtime/tinyui_sdl_observe.c`、`tests/tinyui/runtime/tinyui_sdl_observe.h`
- Modify: `tinyui/port/sdl/hal.c`、`host_internal.h`(去测试状态位)、`tinyui_demo/main.c`
- Modify: `cmake/LingDongGUI.cmake`、`tests/tinyui/CMakeLists.txt`(或相应测试 CMake)
- Delete: `tinyui/port/sdl/observe.c`

**Interfaces:**
- Consumes:Task 2/3 的驱动结构;`tinyui_runtime_host_state`(为测试暴露最小 accessor)。
- Produces:生产 `tinyui_port_sdl`(无 `ENABLE_TEST`)不含任何 capture/auto-quit/marker/benchmark。

- [ ] **Step 1: 迁移文件**

`git mv tinyui/port/sdl/observe.c tests/tinyui/runtime/tinyui_sdl_observe.c`;为其导出函数建 `tinyui_sdl_observe.h`(声明现 `host_internal.h:54-65` 的那批 `tinyui_runtime_host_log_*`/`parse_auto_quit_ms`/`write_capture` 等)。

- [ ] **Step 2: 定义编译期开关**

新建 `tinyui/port/sdl/tinyui_sdl_observe_hooks.h`:
```c
#ifndef TINYUI_SDL_OBSERVE_HOOKS_H
#define TINYUI_SDL_OBSERVE_HOOKS_H
#if defined(ENABLE_TEST)
#include "tinyui_sdl_observe.h"
#define TINYUI_OBS_CAPTURE(state)        tinyui_runtime_host_write_capture(state)
#define TINYUI_OBS_PARSE_AUTOQUIT()      tinyui_runtime_host_parse_auto_quit_ms()
/* ...其余按需 */
#else
#define TINYUI_OBS_CAPTURE(state)        ((void)0)
#define TINYUI_OBS_PARSE_AUTOQUIT()      0u
#endif
#endif
```
`hal.c` 的 present_cb/read_cb 改调这些宏(生产构建展开为空)。auto-quit 状态(`auto_quit_ms`/`start_ticks`)保留在 state,但仅 `ENABLE_TEST` 下被赋值/检查。

- [ ] **Step 3: host_internal.h 去测试状态位**

删除 `:28-38` 的 `*_logged`/`smoke_*`/`benchmark_*`/`capture_written`/`static_mapping_logged` 等纯测试字段;capture 所需最小字段(如 `capture_written`)迁入 observe 侧或以 `#if ENABLE_TEST` 保留。保留 SDL 句柄与 `real_pixels`/`present_pixels`/尺寸/`auto_quit_ms`/`start_ticks`。

- [ ] **Step 4: CMake 接线**

`cmake/LingDongGUI.cmake:258-261`:`tinyui_port_sdl` 源仅剩 `hal.c`(去掉 `observe.c`)。仅当 `ENABLE_TEST` 把 `tests/tinyui/runtime/tinyui_sdl_observe.c` 编入 `tinyui_port_sdl`(或建 `tinyui_sdl_testhooks` 静态库并 `ENABLE_TEST` 下 `target_link_libraries(tinyui_port_sdl ... )`)。给 observe 目标 `target_include_directories` 指向 `port/sdl` 私有头。

- [ ] **Step 5: 闸门(含"生产干净"验证)**

Run: (a) 默认(`ENABLE_TEST=ON`)两 demo + contract 7/7 + 冒烟 capture 460815 + `mcu_host_smoke`;(b) 净化验证——`cmake -S . -B build-prod -DENABLE_TEST=OFF && cmake --build build-prod --target ldgui_sdl_demo`,并 `nm` 生产 `libtinyui_port_sdl.a` 确认无 `write_capture`/`parse_auto_quit`/`benchmark`/`marker` 符号。
Expected: 测试构建全绿;生产库不含任何 observe 符号。

- [ ] **Step 6: Commit**
```bash
git add tests/tinyui/runtime/tinyui_sdl_observe.* tinyui/port/sdl/*.h tinyui/port/sdl/hal.c \
        tinyui_demo/main.c cmake/LingDongGUI.cmake tests/tinyui/CMakeLists.txt
git rm tinyui/port/sdl/observe.c
git commit -m "tinyui(port/sdl): observe 测试脚手架移出生产 port,仅 ENABLE_TEST 链入"
```

---

## Self-Review(对照 spec 检查)

- **Spec 覆盖**:①删转发壳=Task 3 Step 3;②删隐藏契约=Task 3 Step 1;③step.c 通用编排下沉 core=Task 1 Step 5 + Task 2 Step 4;④SDL 两个 create + read_cb setter(用户已确认)=Task 2 Step 1/2 + Task 1 Step 2;⑤observe 移出=Task 4;⑥MCU 零壳可用=Task 3 + `mcu_host_smoke` 闸门。全覆盖。
- **类型一致**:`tinyui_display_present_cb_t`(void(*)(void*))、`tinyui_input_read_cb_t`(int(*)(app*,void*))在 Task 1 定义,Task 2 hal.c 的 `tinyui_sdl_present_cb`/`tinyui_sdl_read_cb` 签名与之匹配;`tinyui_backend_neutral_step/shutdown` 名称全程一致。
- **占位符**:无 TBD;所有 code step 给出完整代码或明确的现有函数复用点。
- **风险点**:Task 2 是唯一高风险(循环所有权切换),故内置独立评审 + 逐字节 PPM 比对;每 Task 独立提交可回滚。

## 待执行时确认的两处实现细节(不阻塞计划)
1. demo `main.c` 取 `struct tinyui_app*` 的既有途径(全局 or accessor)——Task 2 Step 5 落地时以代码为准,必要时加最小内部 accessor。
2. Task 4 CMake 到底"把 observe 源直接编入 `tinyui_port_sdl`(ENABLE_TEST)"还是"建独立 testhooks 库"——取决于 `write_capture` 对 `host_internal.h` 私有结构的依赖面,落地时择简。
