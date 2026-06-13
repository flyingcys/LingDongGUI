# TinyUI Port 层重构实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**目标：** 将 TinyUI port 适配层从 `src/porting/` 迁移至 `tinyui/` 内部，用户移植新芯片只需使用 tinyui 公共 API，不接触 LingDongGUI 或 ARM-2D 内部。

**架构：** 新建 `tinyui_backend_ldgui_porting` CMake 静态库，包含两个文件：`tinyui_ldgui_port.c`（四类回调桥接）和 `tinyui_ldgui_disp_adapter.c`（运行期 PFB 动态配置）。原 `src/porting/` 和 `longdonggui_porting_default` 零修改，SDL 路径继续工作。picoui 兼容层 (`tinyui/include/picoui/`) 全部删除。

**技术栈：** C11, ARM-2D Helper API (`arm_2d_helper_pfb_init`, `arm_2d_helper_pfb_task`), LingDongGUI scene API (`ldGuiFrameStart/Draw/FrameComplete`), CMake static library

---

## 文件清单

| 操作 | 路径 | 职责 |
|------|------|------|
| 新建 | `tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h` | ARM-2D 功能开关（无屏幕尺寸宏） |
| 新建 | `tinyui/src/backend/ldgui/tinyui_ldgui_port.h` | backend 内部 API 声明 |
| 新建 | `tinyui/src/backend/ldgui/tinyui_ldgui_port.c` | 四类回调桥接 |
| 新建 | `tinyui/src/backend/ldgui/tinyui_ldgui_disp_adapter.c` | 运行期 PFB adapter + public `tinyui_backend_init/step` |
| 新建 | `tinyui/port/mcu/Retarget.c` | ARM MCU I/O 桩（可选，MCU 工具链需要） |
| 修改 | `tinyui/src/core/runtime_bridge.c:409-410` | 使 SDL tick/delay 默认值条件化（仅当 callback 为 NULL 时设置） |
| 修改 | `cmake/LingDongGUI.cmake` | 新增 `tinyui_backend_ldgui_porting` target，更新 `tinyui_backend_ldgui` 依赖 |
| 删除 | `tinyui/include/picoui/`（41 个文件） | 彻底移除 picoui 兼容层 |

---

## Task 1: 创建 `tinyui_ldgui_port_config.h`

**Files:**
- Create: `tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h`

- [ ] **Step 1: 确认现有 ldConfig.h 中 ARM-2D 功能开关**

```bash
grep -n "define __ARM_2D\|define LD_CFG_SCREEN" /home/share/samba/flyingcys/LingDongGUI/src/porting/ldConfig.h | head -20
```

- [ ] **Step 2: 创建新配置头文件**

创建文件 `tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h`，内容如下（只保留 ARM-2D 功能开关，**完全不含** `LD_CFG_SCREEN_WIDTH/HEIGHT/PFB_HEIGHT`）：

```c
#ifndef TINYUI_LDGUI_PORT_CONFIG_H
#define TINYUI_LDGUI_PORT_CONFIG_H

/* ARM-2D 功能特性开关 — backend 内部，用户不直接接触 */
#define __ARM_2D_HAS_ASYNC__                        0
#define __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__         1

/* 颜色深度：16-bit RGB565 */
#define __GLCD_CFG_COLOUR_DEPTH__                   16

/* PFB 使用堆分配，数量设为 0 让 disp_adapter 自己分配 */
#define __DISP0_CFG_PFB_HEAP_SIZE__                 0

/* 关闭不需要的旋转 */
#define __DISP0_CFG_ROTATE_SCREEN__                 0

/* 屏幕宽高占位宏，实际值由运行期 tinyui_display_set_config() 传入      */
/* 这里只是让依赖这些宏的 ARM-2D 头文件能通过编译，实际不在 PFB 分配中使用 */
#ifndef LD_CFG_SCREEN_WIDTH
#   define LD_CFG_SCREEN_WIDTH                      480
#endif
#ifndef LD_CFG_SCREEN_HEIGHT
#   define LD_CFG_SCREEN_HEIGHT                     320
#endif
#ifndef LD_CFG_PFB_LINES
#   define LD_CFG_PFB_LINES                         40
#endif

#endif /* TINYUI_LDGUI_PORT_CONFIG_H */
```

- [ ] **Step 3: 验证文件存在**

```bash
ls -la /home/share/samba/flyingcys/LingDongGUI/tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h
```

- [ ] **Step 4: Commit**

```bash
git add tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h
git commit -m "feat(tinyui): add tinyui_ldgui_port_config.h without screen size macros"
```

---

## Task 2: 创建 `tinyui_ldgui_port.h`（内部 API 声明）

**Files:**
- Create: `tinyui/src/backend/ldgui/tinyui_ldgui_port.h`

- [ ] **Step 1: 创建 port 内部头文件**

```c
#ifndef TINYUI_LDGUI_PORT_H
#define TINYUI_LDGUI_PORT_H

struct tinyui_app;

/* 设置/获取当前活跃的 tinyui_app（ARM-2D 回调中使用） */
void ldgui_port_set_current_app(struct tinyui_app *app);
struct tinyui_app *ldgui_port_get_current_app(void);

#endif /* TINYUI_LDGUI_PORT_H */
```

- [ ] **Step 2: 验证文件存在**

```bash
ls -la /home/share/samba/flyingcys/LingDongGUI/tinyui/src/backend/ldgui/tinyui_ldgui_port.h
```

- [ ] **Step 3: Commit**

```bash
git add tinyui/src/backend/ldgui/tinyui_ldgui_port.h
git commit -m "feat(tinyui): add tinyui_ldgui_port.h internal API declarations"
```

---

## Task 3: 创建 `tinyui_ldgui_port.c`（四类回调桥接）

**Files:**
- Create: `tinyui/src/backend/ldgui/tinyui_ldgui_port.c`

**原理：** ARM-2D / LingDongGUI 的外部回调（`Disp0_DrawBitmap`, `VT_enter/leave_global_mutex`, `arm_2d_helper_get_system_timestamp`, `ldCfgTouchGetPoint`）都通过静态全局 `s_ldgui_current_app` 找到当前的 `tinyui_app`，再路由到 tinyui port state。

- [ ] **Step 1: 确认依赖的 tinyui 头文件路径**

```bash
ls /home/share/samba/flyingcys/LingDongGUI/tinyui/include/
ls /home/share/samba/flyingcys/LingDongGUI/tinyui/src/core/internal.h
```

- [ ] **Step 2: 创建 `tinyui_ldgui_port.c`**

```c
#include "tinyui_ldgui_port.h"
#include "tinyui_ldgui_port_config.h"
#include "internal.h"       /* struct tinyui_app, display_port, os_port, tick_port */

#include "ldConfig.h"
#include "arm_2d_types.h"
#include "indev.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ─────────────────── global app pointer ─────────────────── */

static struct tinyui_app *s_ldgui_current_app;

void ldgui_port_set_current_app(struct tinyui_app *app)
{
    s_ldgui_current_app = app;
}

struct tinyui_app *ldgui_port_get_current_app(void)
{
    return s_ldgui_current_app;
}

/* ─────────────── ① LCD flush → tinyui flush callback ─────── */

int32_t Disp0_DrawBitmap(int16_t x, int16_t y,
                         int16_t width, int16_t height,
                         const uint8_t *bitmap)
{
    struct tinyui_app *app = s_ldgui_current_app;

    if (app == NULL || app->display_port.flush_callback == NULL) {
        return 0;
    }

    struct tinyui_area area;
    area.x      = (int)x;
    area.y      = (int)y;
    area.width  = (int)width;
    area.height = (int)height;

    app->display_port.flush_callback(&area,
                                     (const void *)bitmap,
                                     app->display_port.flush_user_data);
    return 0;
}

/* ─────────────── ② mutex → tinyui osal ──────────────────── */

void VT_enter_global_mutex(void)
{
    struct tinyui_app *app = s_ldgui_current_app;

    if (app != NULL && app->os_port.enter != NULL) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void VT_leave_global_mutex(void)
{
    struct tinyui_app *app = s_ldgui_current_app;

    if (app != NULL && app->os_port.leave != NULL) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

/* ─────────────── ③ tick → tinyui tick source ─────────────── */

int64_t arm_2d_helper_get_system_timestamp(void)
{
    struct tinyui_app *app = s_ldgui_current_app;

    if (app == NULL || app->tick_port.callback == NULL) {
        return 0;
    }

    /* tinyui_tick_get 返回毫秒，ARM-2D 期望微秒 */
    return (int64_t)app->tick_port.callback(app->tick_port.user_data) * 1000;
}

/* ─────────────── ④ touch → tinyui input state ────────────── */

bool ldCfgTouchGetPoint(int16_t *x, int16_t *y)
{
    struct tinyui_app *app = s_ldgui_current_app;
    int px = 0, py = 0, pressed = 0;

    if (app == NULL) {
        *x = -1;
        *y = -1;
        return false;
    }

    tinyui_input_get_pointer(app, &px, &py, &pressed);

    if (pressed) {
        struct tinyui_display_config cfg;
        tinyui_display_get_config(app, &cfg);

        if (px < 0) px = 0;
        if (py < 0) py = 0;
        if (px > cfg.width)  px = cfg.width;
        if (py > cfg.height) py = cfg.height;

        *x = (int16_t)px;
        *y = (int16_t)py;
        return true;
    }

    *x = -1;
    *y = -1;
    return false;
}

/* ldCfgTouchSetPoint：MCU 路径不需要（用户直接调 tinyui_input_push_pointer）
 * SDL 路径通过 runtime_host.c 的弱定义调用 — 我们在这里覆盖成强定义，
 * 把 SDL touch 事件路由进 tinyui input state。
 */
void ldCfgTouchSetPoint(int16_t x, int16_t y, bool pressed)
{
    struct tinyui_app *app = s_ldgui_current_app;

    if (app != NULL) {
        tinyui_input_push_pointer(app, (int)x, (int)y, pressed ? 1 : 0);
    }
}

/* ─────────────── ARM MCU 工具链桩 ────────────────────────── */

#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)
__attribute__((weak))
void __aeabi_assert(const char *chCond, const char *chLine, int wErrCode)
{
    (void)chCond;
    (void)chLine;
    (void)wErrCode;
    while (1) {}
}
#endif
```

- [ ] **Step 3: 确认 `tinyui_area` 结构体定义（字段名）**

```bash
grep -n "struct tinyui_area\|tinyui_area {" /home/share/samba/flyingcys/LingDongGUI/tinyui/include/display.h
```

如果字段名与代码中不一致，按实际字段名修正 `area.x / area.y / area.width / area.height`。

- [ ] **Step 4: 确认 `tinyui_input_get_pointer` 和 `tinyui_display_get_config` 签名**

```bash
grep -n "tinyui_input_get_pointer\|tinyui_display_get_config" /home/share/samba/flyingcys/LingDongGUI/tinyui/include/indev.h /home/share/samba/flyingcys/LingDongGUI/tinyui/include/display.h
```

- [ ] **Step 5: Commit**

```bash
git add tinyui/src/backend/ldgui/tinyui_ldgui_port.c
git commit -m "feat(tinyui): add tinyui_ldgui_port.c callback bridges for ARM-2D/ldgui"
```

---

## Task 4: 创建 `tinyui_ldgui_disp_adapter.c`（运行期 PFB Adapter）

**Files:**
- Create: `tinyui/src/backend/ldgui/tinyui_ldgui_disp_adapter.c`

**原理：**
- `tinyui_backend_init(app)`：运行期读 tinyui display config，动态分配 PFB buffer（`ldMalloc`），调用 `arm_2d_helper_pfb_init`
- `tinyui_backend_step(app)`：每帧执行 `ldGuiFrameStart → ldGuiTouchProcess → ldMsgProcess → arm_2d_helper_pfb_task → ldGuiFrameComplete`
- MCU 路径：`flush_callback != NULL` → PFB path
- SDL 路径：`flush_callback == NULL` → 不初始化 PFB，仅初始化 scene（`tinyui_runtime_bridge_init_app`）

- [ ] **Step 1: 确认依赖的 ARM-2D 及 ldGui 头文件**

```bash
ls /home/share/samba/flyingcys/LingDongGUI/examples/common/Arm-2D/Helper/Include/arm_2d_helper_pfb.h
ls /home/share/samba/flyingcys/LingDongGUI/src/gui/ldGui.h
ls /home/share/samba/flyingcys/LingDongGUI/src/misc/ldMsg.h
grep -n "arm_2d_helper_pfb_t\b" /home/share/samba/flyingcys/LingDongGUI/examples/common/Arm-2D/Helper/Include/arm_2d_helper_pfb.h | head -3
```

- [ ] **Step 2: 确认 `tinyui_runtime_bridge_backend_state` 返回类型和 `ld_scene` 字段**

```bash
grep -n "ld_scene\|tinyui_backend_app_state" /home/share/samba/flyingcys/LingDongGUI/tinyui/src/core/runtime_bridge.h
grep -n "ld_scene_t\|struct tinyui_backend_app_state" /home/share/samba/flyingcys/LingDongGUI/tinyui/src/backend/ldgui/backend_widget.h 2>/dev/null || grep -rn "struct tinyui_backend_app_state" /home/share/samba/flyingcys/LingDongGUI/tinyui/src/backend/ldgui/ | head -5
```

- [ ] **Step 3: 创建 `tinyui_ldgui_disp_adapter.c`**

```c
#include "tinyui_ldgui_port.h"
#include "tinyui_ldgui_port_config.h"
#include "internal.h"               /* struct tinyui_app */
#include "runtime_bridge.h"         /* tinyui_runtime_bridge_init_app, backend_state */
#include "backend_widget.h"         /* struct tinyui_backend_app_state, ld_scene */

#include "arm_2d.h"
#include "arm_2d_helper.h"
#include "arm_2d_helper_pfb.h"
#include "ldGui.h"                  /* ldGuiFrameStart/Draw/FrameComplete/TouchProcess */
#include "ldMsg.h"                  /* ldMsgProcess */
#include "ldMem.h"                  /* ldMalloc */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ─────────────── PFB helper state（单例） ─────────────────── */

static arm_2d_helper_pfb_t s_tPFBHelper;
static void               *s_pfb_mem;   /* ldMalloc 分配的 PFB 内存块 */
static int                 s_pfb_inited;

/* ─────────────── PFB draw handler ────────────────────────── */

IMPL_PFB_ON_DRAW(ldgui_port_pfb_draw_handler)
{
    struct tinyui_app *app = (struct tinyui_app *)pTarget;
    struct tinyui_backend_app_state *app_state;

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state != NULL && app_state->ld_scene != NULL) {
        ldGuiDraw(app_state->ld_scene, (arm_2d_tile_t *)ptTile, bIsNewFrame);
    }

    return arm_fsm_rt_cpl;
}

/* ─────────────── PFB flush handler ───────────────────────── */

IMPL_PFB_ON_LOW_LV_RENDERING(ldgui_port_pfb_flush_handler)
{
    const arm_2d_tile_t *ptTile = &(ptPFB->tTile);
    struct tinyui_app *app = (struct tinyui_app *)pTarget;

    if (app != NULL && app->display_port.flush_callback != NULL) {
        struct tinyui_area area;
        area.x      = (int)ptTile->tRegion.tLocation.iX;
        area.y      = (int)ptTile->tRegion.tLocation.iY;
        area.width  = (int)ptTile->tRegion.tSize.iWidth;
        area.height = (int)ptTile->tRegion.tSize.iHeight;

        app->display_port.flush_callback(&area,
                                          (const void *)ptTile->pchBuffer,
                                          app->display_port.flush_user_data);
    }

    arm_2d_helper_pfb_report_rendering_complete(&s_tPFBHelper);
}

/* ─────────────── tinyui_backend_init ─────────────────────── */

int tinyui_backend_init(struct tinyui_app *app)
{
    struct tinyui_display_config cfg;
    size_t buf_size;
    int pfb_height;

    if (app == NULL) {
        return -1;
    }

    ldgui_port_set_current_app(app);

    /* 创建 scene（分配 ld_scene，绑定 theme 等） */
    if (tinyui_runtime_bridge_init_app(app) != 0) {
        return -1;
    }

    arm_2d_init();
    arm_2d_helper_init();

    /* SDL 路径（flush_callback == NULL）：不初始化 PFB，runtime_host 负责渲染 */
    if (app->display_port.flush_callback == NULL) {
        return 0;
    }

    /* MCU 路径：动态分配 PFB buffer，初始化 PFB helper */
    tinyui_display_get_config(app, &cfg);

    pfb_height = cfg.buffer_height > 0 ? cfg.buffer_height : 40;
    /* 每个像素 2 字节（RGB565），buffer_height 行 */
    buf_size = (size_t)cfg.width * (size_t)pfb_height * 2u;

    /* ARM-2D PFB 结构：arm_2d_pfb_t 头 + 紧跟像素缓冲区 */
    s_pfb_mem = ldMalloc(sizeof(arm_2d_pfb_t) + buf_size);
    if (s_pfb_mem == NULL) {
        return -1;
    }
    memset(s_pfb_mem, 0, sizeof(arm_2d_pfb_t) + buf_size);

    arm_2d_helper_pfb_cfg_t pfb_cfg;
    memset(&pfb_cfg, 0, sizeof(pfb_cfg));

    pfb_cfg.tDisplayArea.tSize.iWidth  = (int16_t)cfg.width;
    pfb_cfg.tDisplayArea.tSize.iHeight = (int16_t)cfg.height;

    pfb_cfg.FrameBuffer.ptPFBs     = (arm_2d_pfb_t *)s_pfb_mem;
    pfb_cfg.FrameBuffer.tFrameSize.iWidth  = (int16_t)cfg.width;
    pfb_cfg.FrameBuffer.tFrameSize.iHeight = (int16_t)pfb_height;
    pfb_cfg.FrameBuffer.wBufferSize = (uint32_t)buf_size;
    pfb_cfg.FrameBuffer.u8PFBNum   = 1;
    /* RGB565 = ARM_2D_COLOUR_RGB565 = 0x04，此宏在 arm_2d_types.h 中定义 */
    pfb_cfg.FrameBuffer.u7ColourFormat = ARM_2D_COLOUR_RGB565;

    pfb_cfg.Dependency.evtOnDrawing.fnHandler         = &ldgui_port_pfb_draw_handler;
    pfb_cfg.Dependency.evtOnDrawing.pTarget            = app;
    pfb_cfg.Dependency.evtOnLowLevelRendering.fnHandler = &ldgui_port_pfb_flush_handler;
    pfb_cfg.Dependency.evtOnLowLevelRendering.pTarget   = app;

    if (arm_2d_helper_pfb_init(&s_tPFBHelper, &pfb_cfg) != ARM_2D_ERR_NONE) {
        ldFree(s_pfb_mem);
        s_pfb_mem = NULL;
        return -1;
    }

    s_pfb_inited = 1;
    return 0;
}

/* ─────────────── tinyui_backend_step ─────────────────────── */

void tinyui_backend_step(struct tinyui_app *app)
{
    struct tinyui_backend_app_state *app_state;

    if (app == NULL) {
        return;
    }

    ldgui_port_set_current_app(app);

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return;
    }

    ldGuiFrameStart(app_state->ld_scene);
    ldGuiTouchProcess(app_state->ld_scene);
    ldMsgProcess(app_state->ld_scene);

    if (s_pfb_inited) {
        /* MCU 路径：ARM-2D PFB 驱动绘制（会调用 draw handler → ldGuiDraw） */
        arm_2d_helper_pfb_task(&s_tPFBHelper, NULL);
    }

    ldGuiFrameComplete(app_state->ld_scene);
}
```

- [ ] **Step 4: 确认 `ARM_2D_COLOUR_RGB565` 宏值**

```bash
grep -rn "ARM_2D_COLOUR_RGB565\|ARM_2D_COLOUR_RGB16" /home/share/samba/flyingcys/LingDongGUI/examples/common/Arm-2D/Library/Include/arm_2d_types.h | head -5
```

如果宏名不同，用实际宏名替换 `pfb_cfg.FrameBuffer.u7ColourFormat` 赋值行。

- [ ] **Step 5: 确认 `ldFree` 存在**

```bash
grep -n "ldFree\|void ldFree" /home/share/samba/flyingcys/LingDongGUI/src/misc/ldMem.h
```

如果不存在，改为 `free(s_pfb_mem)` 并添加 `#include <stdlib.h>`。

- [ ] **Step 6: 确认 `ARM_2D_ERR_NONE`**

```bash
grep -n "ARM_2D_ERR_NONE" /home/share/samba/flyingcys/LingDongGUI/examples/common/Arm-2D/Library/Include/arm_2d_types.h | head -3
```

- [ ] **Step 7: Commit**

```bash
git add tinyui/src/backend/ldgui/tinyui_ldgui_disp_adapter.c
git commit -m "feat(tinyui): add runtime PFB display adapter with tinyui_backend_init/step"
```

---

## Task 5: 修改 `runtime_bridge.c`——SDL 默认值条件化

**Files:**
- Modify: `tinyui/src/core/runtime_bridge.c:409-410`

**原因：** 当前第 409-410 行无条件覆盖 tick/delay 回调。MCU 用户在 `tinyui_backend_init` 之前已通过 `tinyui_tick_set_source` 设置了真实回调，但随后 `tinyui_runtime_bridge_init_app` 又把它们覆盖成 SDL 实现，导致 MCU tick 丢失。

- [ ] **Step 1: 确认当前代码**

```bash
sed -n '405,415p' /home/share/samba/flyingcys/LingDongGUI/tinyui/src/core/runtime_bridge.c
```

预期输出包含：
```c
    (void)tinyui_tick_set_source(app, tinyui_runtime_bridge_default_tick_source, NULL);
    (void)tinyui_os_set_delay_callback(app, tinyui_runtime_bridge_default_delay, NULL);
```

- [ ] **Step 2: 改为条件化设置**

将这两行替换为：

```c
    if (app->tick_port.callback == NULL) {
        (void)tinyui_tick_set_source(app, tinyui_runtime_bridge_default_tick_source, NULL);
    }
    if (app->os_port.delay == NULL) {
        (void)tinyui_os_set_delay_callback(app, tinyui_runtime_bridge_default_delay, NULL);
    }
```

使用 Edit 工具精确替换这两行，确保缩进与周围代码一致。

- [ ] **Step 3: 验证修改后 SDL 路径仍能编译**

```bash
cd /home/share/samba/flyingcys/LingDongGUI && cmake --build build --target tinyui_backend_ldgui 2>&1 | tail -20
```

如果 build 目录不存在：
```bash
mkdir -p /home/share/samba/flyingcys/LingDongGUI/build && cd /home/share/samba/flyingcys/LingDongGUI/build && cmake .. && cmake --build . --target tinyui_backend_ldgui 2>&1 | tail -30
```

- [ ] **Step 4: Commit**

```bash
git add tinyui/src/core/runtime_bridge.c
git commit -m "fix(tinyui): make SDL tick/delay defaults conditional to avoid overwriting MCU port"
```

---

## Task 6: 更新 CMake——新增 `tinyui_backend_ldgui_porting` target

**Files:**
- Modify: `cmake/LingDongGUI.cmake`

**原则：**
- `longdonggui_porting_default` 保持不变（链接 `ldConfig.c` + `arm_2d_disp_adapter_0.c`，供非 tinyui 路径使用）
- 新增 `tinyui_backend_ldgui_porting` 链接新建的两个 .c 文件
- `tinyui_backend_ldgui` 和 `tinyui_backend_ldgui_runtime` 改为链接 `tinyui_backend_ldgui_porting` 而非 `longdonggui_porting_default`

- [ ] **Step 1: 查看当前 cmake 结构**

```bash
sed -n '208,245p' /home/share/samba/flyingcys/LingDongGUI/cmake/LingDongGUI.cmake
```

预期看到 `foreach(LD_TINYUI_BACKEND_TARGET IN ITEMS tinyui_backend_ldgui tinyui_backend_ldgui_runtime)` 和 `target_link_libraries ... longdonggui_porting_default`。

- [ ] **Step 2: 在 `longdonggui_porting_default` 定义之后、`tinyui_core` 定义之前插入新 target**

在 `ld_apply_common_target_config(longdonggui_porting_default)` 这行之后，找到合适位置添加以下 cmake 块：

```cmake
    set(LD_TINYUI_BACKEND_LDGUI_DIR "${LD_REPO_ROOT}/tinyui/src/backend/ldgui")

    add_library(tinyui_backend_ldgui_porting STATIC
        "${LD_TINYUI_BACKEND_LDGUI_DIR}/tinyui_ldgui_port.c"
        "${LD_TINYUI_BACKEND_LDGUI_DIR}/tinyui_ldgui_disp_adapter.c"
        ${LD_PERF_COUNTER_SOURCES}
    )
    target_include_directories(tinyui_backend_ldgui_porting PUBLIC
        "${LD_TINYUI_BACKEND_LDGUI_DIR}"
        "${LD_REPO_ROOT}/tinyui/include"
        "${LD_REPO_ROOT}/tinyui/src/core"
        "${LD_REPO_ROOT}/tinyui/src/backend/ldgui"
        ${LD_COMMON_INCLUDE_DIRS}
    )
    target_compile_definitions(tinyui_backend_ldgui_porting PRIVATE
        __ARM_2D_USER_APP_CFG_H__="tinyui_ldgui_port_config.h"
    )
    target_link_libraries(tinyui_backend_ldgui_porting PUBLIC longdonggui tinyui_core)
    ld_apply_common_target_config(tinyui_backend_ldgui_porting)
```

- [ ] **Step 3: 在 foreach 内将 `longdonggui_porting_default` 替换为 `tinyui_backend_ldgui_porting`**

找到这行：
```cmake
        target_link_libraries(${LD_TINYUI_BACKEND_TARGET} PUBLIC tinyui_core longdonggui longdonggui_porting_default)
```

替换为：
```cmake
        target_link_libraries(${LD_TINYUI_BACKEND_TARGET} PUBLIC tinyui_core longdonggui tinyui_backend_ldgui_porting)
```

- [ ] **Step 4: 验证 cmake 能成功配置**

```bash
cd /home/share/samba/flyingcys/LingDongGUI && rm -rf build_verify && mkdir build_verify && cd build_verify && cmake .. 2>&1 | grep -E "error|Error|warning|tinyui_backend" | head -20
```

- [ ] **Step 5: 验证新 target 能编译**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake --build . --target tinyui_backend_ldgui_porting 2>&1 | tail -20
```

预期：编译成功，无链接错误。

- [ ] **Step 6: Commit**

```bash
git add cmake/LingDongGUI.cmake
git commit -m "feat(cmake): add tinyui_backend_ldgui_porting target, decouple from longdonggui_porting_default"
```

---

## Task 7: 删除 `tinyui/include/picoui/` 并修复 include 引用

**Files:**
- Delete: `tinyui/include/picoui/`（41 个文件）
- Scan & fix: `tinyui/demo/`, `tinyui/port/`, `tests/`, `examples/` 中的 picoui include

- [ ] **Step 1: 扫描所有 picoui 引用**

```bash
grep -rn '#include.*picoui/' /home/share/samba/flyingcys/LingDongGUI/ --include="*.c" --include="*.h" --exclude-dir=".git" 2>/dev/null
```

记录所有涉及的文件路径。

- [ ] **Step 2: 逐文件替换 picoui include**

对每个有 `#include "picoui/..."` 的文件，将其改为对应的 tinyui include。例如：

```c
// 旧
#include "picoui/tinyui.h"
#include "picoui/window.h"
#include "picoui/button.h"

// 新（统一改为）
#include "tinyui.h"
```

通配性更强的替换命令（对非 git 跟踪文件使用 sed 处理）：

```bash
find /home/share/samba/flyingcys/LingDongGUI/tinyui/demo \
     /home/share/samba/flyingcys/LingDongGUI/tinyui/port \
     /home/share/samba/flyingcys/LingDongGUI/tests \
     /home/share/samba/flyingcys/LingDongGUI/examples \
     -name "*.c" -o -name "*.h" 2>/dev/null | \
  xargs grep -l 'picoui/' 2>/dev/null
```

对每个文件用 Edit 工具逐个将 `#include "picoui/xxx.h"` 替换为 `#include "tinyui.h"` 或对应的 tinyui 头文件。

- [ ] **Step 3: 删除 picoui 目录**

```bash
rm -rf /home/share/samba/flyingcys/LingDongGUI/tinyui/include/picoui
```

- [ ] **Step 4: 验证无 picoui 残留**

```bash
grep -rn 'picoui' /home/share/samba/flyingcys/LingDongGUI/ --include="*.c" --include="*.h" --include="*.cmake" --include="CMakeLists.txt" --exclude-dir=".git" 2>/dev/null
```

预期：无输出（或只有文档/注释中的历史说明，代码中应清零）。

- [ ] **Step 5: 验证 cmake 和编译不受影响**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake .. && cmake --build . --target tinyui_backend_ldgui 2>&1 | tail -20
```

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(tinyui): remove picoui compat layer (41 files), update includes to tinyui.h"
```

---

## Task 8: 创建 `tinyui/port/mcu/Retarget.c`（MCU I/O 桩）

**Files:**
- Create: `tinyui/port/mcu/Retarget.c`

**原因：** ARM 嵌入式工具链链接时需要 `_write`、`_read` 等 syscall 桩，否则链接失败。仅在非 Apple/Linux/Windows 环境编译有效。

- [ ] **Step 1: 确认 MCU port 目录**

```bash
ls /home/share/samba/flyingcys/LingDongGUI/tinyui/port/ 2>/dev/null
```

如目录不存在：`mkdir -p /home/share/samba/flyingcys/LingDongGUI/tinyui/port/mcu`

- [ ] **Step 2: 创建 `Retarget.c`**

```c
/* Retarget.c — ARM MCU 工具链 I/O 桩（syscall stubs）
 * 仅在 MCU 工具链下编译生效，Apple/Linux/Windows 通过宏排除。
 */

#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)

#include <stdint.h>

/* 阻止 printf 依赖 semihosting */
int _write(int fd, const char *buf, int count)
{
    (void)fd;
    (void)buf;
    return count;
}

int _read(int fd, char *buf, int count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return 0;
}

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _fstat(int fd, void *st)
{
    (void)fd;
    (void)st;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

int _lseek(int fd, int offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

/* 覆盖 semihosting 初始化（防止 Keil/MDK 链接 semihosting 库） */
__attribute__((weak))
void initialise_monitor_handles(void) {}

#endif /* !__APPLE__ && !__linux__ && !_WIN32 */
```

- [ ] **Step 3: Commit**

```bash
git add tinyui/port/mcu/Retarget.c
git commit -m "feat(tinyui/port/mcu): add Retarget.c syscall stubs for ARM MCU toolchain"
```

---

## Task 9: 全量构建验证

**目标：** 确认 SDL demo 能编译运行，新 `tinyui_backend_ldgui_porting` 能编译，原 `longdonggui_porting_default` 仍完整。

- [ ] **Step 1: 查找现有可构建 target**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake --build . --target help 2>&1 | grep -E "tinyui|longdonggui" | head -30
```

- [ ] **Step 2: 构建所有 tinyui 相关 target**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake --build . --target tinyui_core tinyui_backend_ldgui tinyui_backend_ldgui_runtime tinyui_backend_ldgui_porting 2>&1 | tail -30
```

预期：全部 `0 errors`。

- [ ] **Step 3: 构建 longdonggui_porting_default（确认原路径不受影响）**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake --build . --target longdonggui_porting_default 2>&1 | tail -10
```

- [ ] **Step 4: 运行现有 SDL demo（如有）**

```bash
cd /home/share/samba/flyingcys/LingDongGUI/build_verify && cmake --build . 2>&1 | tail -10
# 找到 demo 可执行文件
find /home/share/samba/flyingcys/LingDongGUI/build_verify -name "demo*" -executable 2>/dev/null | head -5
```

如有 demo，尝试运行几秒验证无崩溃：
```bash
timeout 5 /path/to/demo || true
```

- [ ] **Step 5: 验证无 picoui 字符串残留于编译产物头文件搜索路径**

```bash
grep -rn 'picoui' /home/share/samba/flyingcys/LingDongGUI/tinyui/ --include="*.c" --include="*.h" --exclude-dir=".git" 2>/dev/null
```

预期：无输出。

- [ ] **Step 6: 最终 commit（如有遗留修改）**

```bash
git status
git add -A
git commit -m "chore(tinyui): port layer redesign build verified, all targets compile"
```

---

## 用户移植验证清单（完成后检查）

完成以上 9 个 Task 后，应满足以下验收标准：

| 验收项 | 检查方式 |
|---|---|
| MCU 用户只 `#include "tinyui.h"` 即可完成适配 | 查看 `tinyui/port/mcu/Retarget.c` + 文档中示例 |
| 无 `LD_CFG_SCREEN_WIDTH/HEIGHT` 宏需要用户设置 | grep 新建文件，确认这些宏不在用户 API 路径上 |
| PFB 宽高由 `tinyui_display_set_config()` 运行期决定 | 查看 `tinyui_backend_init` 代码 |
| SDL demo 继续运行正常 | Task 9 Step 4 通过 |
| `longdonggui_porting_default` 完整不变 | Task 9 Step 3 通过 |
| `tinyui/include/picoui/` 已不存在 | Task 7 Step 4 通过 |
| 无 picoui include 残留 | `grep -r picoui tinyui/ --include="*.c" --include="*.h"` 无输出 |
