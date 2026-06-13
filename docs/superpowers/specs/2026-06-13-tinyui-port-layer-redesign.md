# TinyUI Port 层重构设计

**日期**：2026-06-13  
**分支**：dev-nanoui  
**目标**：用户移植新芯片时只需了解 tinyui，不需接触 LingDongGUI 或 ARM-2D 任何内容。

---

## 背景与问题

当前 porting 层位于 `src/porting/`，属于 LingDongGUI 目录体系，存在以下问题：

1. **用户暴露 LingDongGUI 内部**：移植新芯片需修改 `ldConfig.h`、`ldConfig.c`，涉及 `LD_CFG_SCREEN_WIDTH` 等 ARM-2D 宏，用户必须了解 LingDongGUI/ARM-2D 内部机制。
2. **屏幕参数编译期绑定**：PFB buffer 为静态数组，分辨率由编译期宏决定，多芯片适配需多份编译配置。
3. **picoui 兼容层冗余**：`tinyui/include/picoui/` 共 41 个 wrapper 头文件，应彻底移除。

---

## 设计目标

- 用户移植新芯片，只在 `tinyui/port/<chip>/` 下新建文件，只调用 tinyui 公共 API。
- 屏幕分辨率、PFB 高度、颜色格式全部通过 `tinyui_display_set_config()` 运行期传入。
- 对标 LVGL API 风格：flush callback / tick source / lock callback / delay 均为运行期注册。
- `src/porting/` 保持不动，`longdonggui_porting_default` cmake 目标保留，不影响原有 LingDongGUI 用法。

---

## 约束

- **不修改** `src/porting/` 下任何文件。
- **不修改** `longdonggui_porting_default` cmake 目标。
- tinyui 所需的 port 层能力全部**新建文件**实现。
- 重构完成后**不得出现 picoui 相关内容**。

---

## 用户使用模型（目标态）

移植新芯片（以 STM32H7 为例），用户只创建：

```
tinyui/port/stm32h7/
    stm32h7_port.c
    CMakeLists.txt
```

`stm32h7_port.c` 内容只使用 tinyui API，无任何 ldgui / arm2d 头文件：

```c
#include "tinyui.h"

static void my_flush(const struct tinyui_area *area, const void *pixels, void *ud) {
    LCD_DrawBitmap(area->x, area->y, area->width, area->height, pixels);
}

static uint32_t my_tick(void *ud) { return HAL_GetTick(); }
static void my_lock(void *ud)     { osMutexAcquire(g_mutex, osWaitForever); }
static void my_unlock(void *ud)   { osMutexRelease(g_mutex); }
static void my_delay(uint32_t ms, void *ud) { osDelay(ms); }

void tinyui_port_stm32h7_attach(struct tinyui_app *app) {
    tinyui_display_set_config(app, &(struct tinyui_display_config){
        .width        = 480,
        .height       = 320,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 40,
    });
    tinyui_display_set_flush_callback(app, my_flush, NULL);
    tinyui_tick_set_source(app, my_tick, NULL);
    tinyui_os_set_lock_callbacks(app, my_lock, my_unlock, NULL);
    tinyui_os_set_delay_callback(app, my_delay, NULL);
}
```

用户 main.c 示例：

```c
#include "tinyui.h"

int main(void) {
    struct tinyui_app *app = tinyui_app_create();
    tinyui_port_stm32h7_attach(app);

    struct tinyui_window *win = tinyui_window_create(app);
    // ... 创建 widget ...
    tinyui_app_run(app, win);
}
```

---

## 新建文件清单

`src/porting/` 保持零修改，只在 tinyui 内新增以下文件：

| 新建文件 | 职责 |
|---|---|
| `tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h` | ARM-2D 功能特性开关（无屏幕尺寸宏） |
| `tinyui/src/backend/ldgui/tinyui_ldgui_port.h` | backend 内部 API 声明 |
| `tinyui/src/backend/ldgui/tinyui_ldgui_port.c` | flush/tick/mutex/touch 接 tinyui port state |
| `tinyui/src/backend/ldgui/tinyui_ldgui_disp_adapter.c` | 运行期动态配置的 display adapter |
| `tinyui/port/mcu/Retarget.c` | ARM 工具链 I/O 桩（MCU 按需，非 Apple/Linux） |

删除：`tinyui/include/picoui/`（41 个文件，兼容层完全移除）

---

## 各文件设计

### `tinyui_ldgui_port_config.h`

只保留 ARM-2D 功能特性开关，**不含屏幕尺寸宏**（尺寸由运行期 tinyui display config 提供）：

```c
// 功能开关——backend 内部，用户不接触
#define __ARM_2D_HAS_ASYNC__                0
#define __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__ 1
// alpha blending / transform / filter 开关
// 无 LD_CFG_SCREEN_WIDTH / HEIGHT / PFB_HEIGHT
```

cmake 中对 `tinyui_backend_ldgui_porting` 目标使用：
```cmake
"-D__ARM_2D_USER_APP_CFG_H__=\"tinyui_ldgui_port_config.h\""
```

---

### `tinyui_ldgui_port.c` — 四类回调桥接

将 LingDongGUI/ARM-2D 期望的外部回调全部接到 tinyui port state：

```c
// ① LCD flush → tinyui display flush callback
void Disp0_DrawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bmp) {
    struct tinyui_area area = {x, y, w, h};
    tinyui_display_invoke_flush(ldgui_get_current_app(), &area, bmp);
}

// ② mutex → tinyui osal
void VT_enter_global_mutex(void) { tinyui_os_lock(ldgui_get_current_app()); }
void VT_leave_global_mutex(void) { tinyui_os_unlock(ldgui_get_current_app()); }

// ③ tick → tinyui tick source
int64_t arm_2d_helper_get_system_timestamp(void) {
    return (int64_t)tinyui_tick_get(ldgui_get_current_app());
}

// ④ touch → tinyui input port state
bool ldCfgTouchGetPoint(int16_t *x, int16_t *y) {
    return tinyui_input_read_pointer(ldgui_get_current_app(), x, y);
}
```

`ldgui_get_current_app()` 为 backend 内部函数，返回当前活跃的 `struct tinyui_app *`。

---

### `tinyui_ldgui_disp_adapter.c` — 运行期 Display Adapter

**不复制 `arm_2d_disp_adapter_0.c`**，直接调用 ARM-2D Helper 公共 API（`arm_2d_helper_pfb_init` / `arm_2d_helper_pfb_task`），PFB buffer 运行期动态分配：

```c
void ldgui_disp_adapter_init(struct tinyui_app *app) {
    struct tinyui_display_config cfg;
    tinyui_display_get_config(app, &cfg);  // 读运行期参数

    size_t buf_size = (size_t)cfg.width * cfg.buffer_height * sizeof(COLOUR_INT);
    COLOUR_INT *pfb_buf = ldMalloc(buf_size);  // 运行期动态分配

    arm_2d_helper_pfb_cfg_t pfb_cfg = {
        .tDisplayArea = { .tSize = { cfg.width, cfg.height } },
        .FrameBuffer  = {
            .pBuffer     = pfb_buf,
            .wBufferSize = (uint32_t)buf_size,
            .chPFBNum    = 1,
        },
        .Dependency.evtOnDrawing = {
            .fnHandler = ldgui_pfb_draw_handler,
            .pTarget   = app,
        },
    };
    arm_2d_helper_pfb_init(&s_tPFBHelper, &pfb_cfg);
}

void ldgui_disp_adapter_task(struct tinyui_app *app) {
    arm_2d_helper_pfb_task(&s_tPFBHelper, NULL);
}
```

width / height / buffer_height 全部来自 `tinyui_display_set_config()` 的运行期调用，无编译期宏依赖。

---

## Port API 接口汇总（用户所需）

| 接口 | 必须/可选 | 说明 |
|---|---|---|
| `tinyui_display_set_config()` | 必须 | 分辨率、颜色格式、PFB stripe 高度 |
| `tinyui_display_set_flush_callback()` | 必须 | LCD 像素写入 |
| `tinyui_tick_set_source()` | 必须 | 毫秒计数器 |
| `tinyui_os_set_lock_callbacks()` | RTOS 必须 | 互斥锁 enter/leave |
| `tinyui_os_set_delay_callback()` | 可选 | 延迟 |
| `tinyui_input_push_pointer()` | 有触摸则必须 | 触摸坐标注入 |
| `tinyui_input_push_key()` | 有按键则必须 | 按键事件注入 |

---

## CMake 变更

### 新增 target（不修改任何现有 target）

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
    ${LD_COMMON_INCLUDE_DIRS}
)
target_compile_options(tinyui_backend_ldgui_porting PRIVATE
    "-D__ARM_2D_USER_APP_CFG_H__=\"tinyui_ldgui_port_config.h\""
)
target_link_libraries(tinyui_backend_ldgui_porting PUBLIC longdonggui tinyui_core)
ld_apply_common_target_config(tinyui_backend_ldgui_porting)
```

### 修改 `tinyui_backend_ldgui` 依赖

```cmake
# 原来链接 longdonggui_porting_default
# 改为链接 tinyui_backend_ldgui_porting
target_link_libraries(tinyui_backend_ldgui PUBLIC
    tinyui_core
    longdonggui
    tinyui_backend_ldgui_porting   # ← 替换 longdonggui_porting_default
)
```

`longdonggui_porting_default` 继续存在，供非 tinyui 的原有 LingDongGUI 用法使用。

---

## picoui 移除

- 删除 `tinyui/include/picoui/`（41 个文件）
- 扫描 `tinyui/demo/`、`tinyui/port/`、`tests/`、`examples/` 中所有 `#include "picoui/..."` 引用，统一改为 `#include "tinyui.h"`
- 扫描完成后确认无 picoui 字符串残留

---

## 改动范围汇总

| 类别 | 内容 |
|---|---|
| 新建文件（4个） | `tinyui/src/backend/ldgui/tinyui_ldgui_port_config.h/h/c` + `tinyui_ldgui_disp_adapter.c` |
| 新建文件（可选） | `tinyui/port/mcu/Retarget.c` |
| 删除目录 | `tinyui/include/picoui/`（41 文件） |
| cmake 新增 target | `tinyui_backend_ldgui_porting` |
| cmake 修改依赖 | `tinyui_backend_ldgui` 链接目标从 `longdonggui_porting_default` 改为 `tinyui_backend_ldgui_porting` |
| 引用扫描更新 | demo/tests/examples 中 picoui include → `tinyui.h` |
| **不改动** | `src/porting/` 全部文件 |
| **不改动** | `longdonggui_porting_default` cmake 目标 |
| **不改动** | `tinyui/include/*.h` 公共头文件 |
