# PicoUI port 层设计

> 适用仓库：`/Users/cys/embedded/LingDongGUI`  
> 日期：2026-06-05  
> 输入文档：`docs/picoui-serial/a-0.14/2026-06-05-picoui-port-boundary-audit.md`

## 背景

PicoUI 已经把控件 public API 与大部分 widget backend 映射收进 `picoui/`，但芯片、屏幕、输入、tick、OS、资源适配仍主要依赖 `LingDongGUI` / `ARM-2D` 的旧 porting 入口。开发者如果要适配 PicoUI 到 SDL 或真实屏幕，仍会接触 `ldConfig.*`、`arm_2d_disp_adapter_0.*`、`Disp0_DrawBitmap()`、`LD_CFG_*`、`__DISP0_CFG_*`。

这和 PicoUI 目标冲突。PicoUI 应让开发者只面对 PicoUI 自己的 port 合同，而不是理解 LingDongGUI 与 ARM-2D 的存在。

## 目标

1. 建立 PicoUI 自己的 `port` 命名和目录，参考 LVGL 的 `lv_port_*` 心智。
2. 首批只做 SDL port 最小闭环：display、input、tick、OS。
3. 新 SDL 适配落在 `picoui/port/sdl/`，不再挂到 `examples/sdl` 后面。
4. 不新增 `platform/ldgui`、`port/ldgui` 或等价目录；LingDongGUI 映射继续归属 `picoui/src/backend/ldgui/`。
5. 保持 PicoUI public header 不泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
6. 保持现有 PicoUI unit/runtime/mapping/visible gate 可继续运行。

## 非目标

1. 不重写 LingDongGUI renderer。
2. 不把 `LD_CFG_*` 简单复制成 `PICOUI_CFG_*` 后宣称完成。
3. 不在首批同时做 MH2103C 裸机 port。
4. 不在首批迁移 image/font/resource 的全部 public API。
5. 不删除 `examples/sdl` 旧入口；本轮只是不再把新增 PicoUI SDL port 挂到那里。
6. 不把 `direct_public_100_complete` 表述成 port 完成。

## 命名与目录

推荐结构：

```text
picoui/include/picoui/port.h
picoui/include/picoui/port/display.h
picoui/include/picoui/port/input.h
picoui/include/picoui/port/tick.h
picoui/include/picoui/port/os.h

picoui/src/port/
  port.c
  display.c
  input.c
  tick.c
  os.c

picoui/port/sdl/
  picoui_port_sdl.c
  picoui_port_sdl_display.c
  picoui_port_sdl_input.c
  picoui_port_sdl_tick.c
  picoui_port_sdl_os.c
```

`picoui/src/port/` 是 PicoUI core contract 和默认状态实现。`picoui/port/sdl/` 是 SDL host 端口实现。`picoui/src/backend/ldgui/` 只负责把 PicoUI app/widget/port 状态映射到底层 LingDongGUI/ARM-2D。

## Display 合同

首批 display 合同只覆盖 runtime 当前真实需要的内容：

```c
enum picoui_color_format {
    PICOUI_COLOR_FORMAT_RGB565 = 0,
    PICOUI_COLOR_FORMAT_ARGB8888,
};

struct picoui_area {
    int x;
    int y;
    int width;
    int height;
};

struct picoui_display_config {
    int width;
    int height;
    enum picoui_color_format color_format;
    int buffer_height;
    void *user_data;
};

typedef void (*picoui_display_flush_cb_t)(const struct picoui_area *area,
                                          const void *pixels,
                                          void *user_data);
```

首批 public 函数：

```c
int picoui_display_set_config(struct picoui_app *app,
                              const struct picoui_display_config *config);
int picoui_display_get_config(const struct picoui_app *app,
                              struct picoui_display_config *out_config);
int picoui_display_set_flush_callback(struct picoui_app *app,
                                      picoui_display_flush_cb_t callback,
                                      void *user_data);
```

默认 config：

- width: `480`
- height: `320`
- color_format: `PICOUI_COLOR_FORMAT_RGB565`
- buffer_height: `0`
- user_data: `NULL`

这些默认值保留现有 SDL runtime 行为，但变成 PicoUI port 状态，而不是散在 `backend_app.c` 的宏。

## Input 合同

首批 input 采用 event push。这样更贴近当前 SDL event pump，也便于后续裸机中断或轮询驱动复用。

```c
enum picoui_input_key {
    PICOUI_INPUT_KEY_NONE = 0,
    PICOUI_INPUT_KEY_LEFT,
    PICOUI_INPUT_KEY_RIGHT,
    PICOUI_INPUT_KEY_UP,
    PICOUI_INPUT_KEY_DOWN,
    PICOUI_INPUT_KEY_ENTER,
    PICOUI_INPUT_KEY_BACK,
};

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed);
int picoui_input_get_pointer(const struct picoui_app *app,
                             int *x,
                             int *y,
                             int *pressed);
int picoui_input_push_key(struct picoui_app *app,
                          enum picoui_input_key key,
                          int pressed);
int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed);
```

LingDongGUI backend 后续应从 PicoUI input state 读取 pointer，再映射到 `ldCfgTouchSetPoint()` 或等价内部桥；SDL port 不直接调用 `ldCfgTouchSetPoint()`。

## Tick 合同

tick 合同为 app timer 和 runtime loop 提供时间源。

```c
typedef unsigned int (*picoui_tick_get_cb_t)(void *user_data);

int picoui_tick_set_source(struct picoui_app *app,
                           picoui_tick_get_cb_t callback,
                           void *user_data);
unsigned int picoui_tick_get(struct picoui_app *app);
```

默认 tick 可退回到 backend 当前实现；SDL port 应注册 SDL tick source。

## OS 合同

首批只覆盖当前实际需要的最小 OS 能力。

```c
typedef void (*picoui_os_lock_cb_t)(void *user_data);
typedef void (*picoui_os_delay_cb_t)(unsigned int ms, void *user_data);

int picoui_os_set_lock_callbacks(struct picoui_app *app,
                                 picoui_os_lock_cb_t enter,
                                 picoui_os_lock_cb_t leave,
                                 void *user_data);
void picoui_os_enter(struct picoui_app *app);
void picoui_os_leave(struct picoui_app *app);
int picoui_os_set_delay_callback(struct picoui_app *app,
                                 picoui_os_delay_cb_t delay,
                                 void *user_data);
void picoui_os_delay(struct picoui_app *app, unsigned int ms);
```

默认 lock 是 no-op。默认 delay 可由 backend 保留原行为，SDL port 注册 SDL delay。

## SDL port

SDL port 只适配 PicoUI port 合同，不承载控件映射、不写 fake layout、不输出 backend mapping marker。

建议入口：

```c
int picoui_port_sdl_attach(struct picoui_app *app);
```

`picoui_port_sdl_attach()` 的职责：

1. 设置 display 默认 config。
2. 设置 tick source。
3. 设置 delay callback。
4. 后续由 runtime event pump 把 SDL event 转成 `picoui_input_push_pointer()` / `picoui_input_push_key()`。

## Backend 边界

`picoui/src/backend/ldgui/` 继续是 LingDongGUI/ARM-2D 唯一私有映射位置。允许它 include `ldConfig.h`、`ldGui.h`、`arm_2d.h`。不允许把这层拆成 `port/ldgui`。

首批 backend 调整原则：

1. `backend_app.c` 创建 window/texture/tile 时从 `picoui_display_get_config()` 读取尺寸。
2. SDL event pump 改为调用 PicoUI input API。
3. LingDongGUI input bridge 再从 PicoUI input state 推到底层。
4. timer pump 使用 `picoui_tick_get()`，不直接散落 SDL tick。
5. delay 使用 `picoui_os_delay()`，不直接散落 SDL delay。

## 测试要求

新增 focused unit test：

- `tests/picoui/unit/test_picoui_port_display.c`
- `tests/picoui/unit/test_picoui_port_input.c`
- `tests/picoui/unit/test_picoui_port_tick_os.c`

新增或扩展 contract：

- `tests/picoui/contract/check_picoui_public_api.py` 继续防止 public header 泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。

首批验证命令：

```bash
rtk cmake --build build --target test_picoui_port_display test_picoui_port_input test_picoui_port_tick_os
rtk ctest --test-dir build -R 'test_picoui_port_display|test_picoui_port_input|test_picoui_port_tick_os|check_picoui_public_api' --output-on-failure
```

若本地使用 `build/picoui-runtime`，对应命令改为同一 build dir。

## 分阶段

### P1：SDL port 最小闭环

交付：

- PicoUI port header/core 状态。
- SDL attach 入口在 `picoui/port/sdl/`。
- backend app 从 port config/input/tick/os 读取状态。
- focused unit test + public API leak check。

### P2：resource port

后续单独设计，不混入 P1。

### P3：MH2103C board port

后续单独设计，不混入 P1。目标是 `picoui/port/mh2103c/` + `picoui/docs/porting_guide.md`。

## 成功标准

1. 新增 PicoUI port API 不泄漏 LingDongGUI/ARM-2D 类型或名字。
2. SDL port 位于 `picoui/port/sdl/`。
3. 新增 PicoUI SDL port 不依赖 `examples/sdl` 作为承载路径。
4. `backend_app.c` 的尺寸、pointer、tick、delay 不再直接以 SDL/LingDongGUI 旧宏为唯一源。
5. 现有 PicoUI app/widget API 不破坏。
6. 相关 focused tests 和 public API leak check 通过。
