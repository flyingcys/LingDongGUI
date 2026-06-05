# PicoUI port 边界审计与建议

> 日期：2026-06-05  
> 范围：PicoUI 的芯片、屏幕、输入、tick、OS、资源 port 边界  
> 结论类型：边界审计 + P1 实现结果收口

> P1 实现状态：已完成。以当前 worktree 命令证据为准：
>
> - `ctest --test-dir build -L 'picoui' --output-on-failure`
> - `ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure`
>
> 当前结果：
>
> - `build`：`100% tests passed, 0 tests failed out of 46`
> - `build/picoui-runtime`：`100% tests passed, 0 tests failed out of 3`

## 结论

PicoUI 当前已经把控件 public API 与大部分 widget backend 映射放到了 `picoui/` 侧，但芯片、屏幕、输入适配链仍主要借用 `LingDongGUI` 与 `ARM-2D` 的旧 porting 入口。开发者如果要适配芯片、屏幕和输入，仍需要理解 `ldConfig.*`、`arm_2d_disp_adapter_0.*`、`Disp0_DrawBitmap()`、`LD_CFG_*`、`__DISP0_CFG_*` 这些底层概念。

这不符合 PicoUI 的长期目标。正确方向不是把 `LingDongGUI` 的所有底层 API 改名前缀后搬进 PicoUI，也不是让 PicoUI 重写渲染器；正确方向是参考 LVGL 的 `lv_port_*` 心智，新增 PicoUI 自己的 `port` 合同，让开发者只面对 PicoUI 的显示、输入、tick、OS、资源接口。`LingDongGUI` 与 `ARM-2D` 继续作为当前 backend/runtime 实现细节存在，但不再是 PicoUI 使用者的适配入口。

## 当前事实

### 1. PicoUI runtime 仍直接握住 SDL、LingDongGUI、ARM-2D

`picoui/src/backend/ldgui/backend_app.c` 同时包含这些职责：

- SDL window、renderer、texture 创建。
- `arm_2d_tile_t` framebuffer 创建。
- `ld_scene_t` 创建和 `ldGuiFrameStart()` / `ldGuiTouchProcess()` / `ldMsgProcess()` / `ldGuiDraw()` / `ldGuiFrameComplete()` 调度。
- SDL mouse event 到 `ldCfgTouchSetPoint()` 的转换。
- runtime capture、mapping marker、temporary smoke layout marker。

关键证据：

- `backend_app.c:22-27` 直接 include `SDL.h`、`arm_2d.h`、`ldConfig.h`、`ldBase.h`、`ldGui.h`、`arm_2d_disp_adapter_0.h`。
- `backend_app.c:32-36` 硬编码 `PICOUI_RUNTIME_WIDTH/HEIGHT` 与 smoke layout 尺寸。
- `backend_app.c:124-152` 把 SDL pointer event 直接映射到 `LD_CFG_SCREEN_WIDTH/HEIGHT`，再调用 `ldCfgTouchSetPoint()`。
- `backend_app.c:633-652` 直接构造 `arm_2d_tile_t`。
- `backend_app.c:757-763` 直接跑 LingDongGUI frame/touch/message/draw/present 链路。

这说明 `backend_app.c` 已经不是单纯的 PicoUI app backend；它同时承担 host、display、input、frame scheduler、evidence harness、LingDongGUI scene bridge。

### 2. 屏幕和 PFB 配置仍以 LingDongGUI/ARM-2D 宏为源

`src/porting/ldConfig.h` 是当前屏幕和内存配置入口：

- `LD_CFG_COLOR_DEPTH`
- `LD_CFG_SCREEN_WIDTH`
- `LD_CFG_SCREEN_HEIGHT`
- `LD_CFG_PFB_WIDTH`
- `LD_CFG_PFB_HEIGHT`
- `LD_MEM_MODE`
- `LD_MEM_SIZE`
- `USE_VIRTUAL_RESOURCE`
- `__DISP0_CFG_*`

这些名字对 PicoUI 开发者来说都是 LingDongGUI/ARM-2D 心智。PicoUI 如果继续让用户通过这些宏完成适配，就只能说 public widget API 已隐藏底层，不能说平台适配流程已隐藏底层。

### 3. 资源 API 已类型擦除，但语义仍偏 ARM-2D

`picoui/include/picoui/native.h` 没有直接暴露 `arm_2d_tile_t` 或 `arm_2d_font_t`，但 `struct picoui_native_image` 仍是 `void *tile`、`void *mask`，`struct picoui_native_font` 仍是 `void *font`。

`picoui/include/picoui/image.h` 的 `struct picoui_image_source` 仍有 `img_tile`、`mask_tile`、`vres_addr`。这些字段不直接泄漏类型名，但使用者仍必须知道底层 tile、mask、virtual resource 地址模型。

这类 API 可以作为过渡兼容入口保留，但不能作为 PicoUI 平台资源抽象的终点。

### 4. 裸机示例仍是 LingDongGUI/ARM-2D 适配链

MH2103C 示例当前是典型旧链路：

1. 初始化屏幕驱动和输入驱动。
2. 初始化 `arm_2d` 和 `disp_adapter0`。
3. 主循环跑 `disp_adapter0_task()`。
4. flush 走 `Disp0_DrawBitmap()` 到屏幕驱动。

这对 LingDongGUI 是合理的，但对 PicoUI 不够。PicoUI 需要一个同等能力的 PicoUI board port 示例，而不是让用户照着 LingDongGUI 工程理解底层。

## LVGL 对照

LVGL 的 porting 模型有一个关键优点：porting 层只注册平台服务，不承担控件能力适配。

### display

LVGL display port 创建 display，设置分辨率、flush callback、render buffer：

- `lv_display_create(width, height)`
- `lv_display_set_flush_cb(disp, disp_flush)`
- `lv_display_set_buffers(...)`
- flush 完成后调用 `lv_display_flush_ready()`

display port 不关心 button、label、switch、list 的业务语义。

### input

LVGL input port 创建 input device，设置类型和 read callback：

- pointer
- keypad
- encoder
- button

read callback 只填统一输入数据，控件如何响应由 LVGL 内核处理。

### tick / loop / OS

LVGL 把 tick 来源、timer handler、OS lock/sleep 作为单独平台服务处理。主循环职责清楚，display flush、input read、timer handler 不混成一个文件。

### 对 PicoUI 的真实借鉴点

PicoUI 不应照搬 LVGL 控件 API，因为 PicoUI 的目标是封装 LingDongGUI 的用户可用能力。但 PicoUI 应该借鉴 LVGL 的 port 命名和边界：

- `port` 只提供显示、输入、tick、OS、资源服务。
- 控件映射继续留在 `picoui/src/backend/ldgui/backend_*.c`。
- SDL 和芯片驱动不能继续承担控件布局、fake fallback、证据 marker 等职责。

## 推荐架构

新增 PicoUI `port` 层，作为面向移植者的合同。命名参考 LVGL，但使用 PicoUI 自己的前缀和目录。建议目录先按下面组织：

```text
picoui/include/picoui/port.h
picoui/include/picoui/display.h
picoui/include/picoui/indev.h
picoui/include/picoui/tick.h
picoui/include/picoui/osal.h
picoui/include/picoui/port/sdl.h

picoui/src/display/
  display.c
picoui/src/indev/
  indev.c
picoui/src/tick/
  tick.c
picoui/src/osal/
  osal.c

picoui/port/sdl/
  sdl.c

picoui/port/mh2103c/
  mh2103c_display.c
  mh2103c_indev.c
  mh2103c_tick.c
  mh2103c_osal.c
```

不再建议新增 `platform/ldgui` 或 `port/ldgui`。LingDongGUI 映射已经有自然归属：`picoui/src/backend/ldgui/`。SDL port 直接适配到 PicoUI port 合同；backend 私有层消费 port 状态并映射到 LingDongGUI/ARM-2D。这样不会把 LingDongGUI bridge 包装成另一个正式移植目标。

未来 PicoUI SDL 适配也不应继续放在 `examples/sdl` 后面。`examples/sdl` 可以作为历史事实和旧 demo 构建入口保留，但新的 PicoUI SDL port 应落到 `picoui/port/sdl/`。

## 核心合同建议

### 1. display port contract

PicoUI display 需要表达这些内容：

- 分辨率：width / height。
- 像素格式：RGB565、ARGB8888 等 portable enum。
- render buffer / partial buffer 配置。
- flush callback：把 PicoUI backend 产出的像素区域提交给屏幕。
- driver private data：允许芯片驱动或 SDL driver 挂私有状态。

建议形态：

```c
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

实现时不要急着让它替代所有 LingDongGUI 配置。第一步只要让 `backend_app.c` 不再直接硬编码 SDL window 尺寸和 `arm_2d_tile_t` 创建逻辑。

### 2. input port contract

PicoUI input 需要表达这些内容：

- pointer：x/y/pressed。
- keypad：key code/state。
- encoder：diff/button state。
- button point：物理按键映射屏幕点。
- read callback 或 event push 两种模式，先选一种主路径。

建议优先做 event push：

```c
int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed);
int picoui_input_push_key(struct picoui_app *app, int key, int pressed);
```

`backend_app.c` 的 SDL event 应调用 PicoUI input API，而不是直接调用 `ldCfgTouchSetPoint()`。LingDongGUI backend 再在私有层把 PicoUI input event 写入 `ldCfgTouchSetPoint()` 或等价输入桥。

### 3. tick port contract

PicoUI 已经有 app timer，但平台 tick 仍由 SDL `SDL_GetTicks()` 驱动。后续需要分清：

- port tick：当前时间源。
- app timer：应用层 timer。
- frame scheduler：什么时候 pump input、timer、draw、flush。

建议：

```c
typedef unsigned int (*picoui_tick_get_cb_t)(void *user_data);
int picoui_tick_set_source(picoui_tick_get_cb_t callback, void *user_data);
```

SDL host 可以用 SDL tick，裸机可以用 SysTick、RTOS tick 或硬件 timer。

### 4. OS port contract

不要一开始做完整 OSAL。先只抽 PicoUI 当前实际需要的最小集合：

- mutex enter/leave。
- sleep/delay。
- optional critical section。

当前 `backend_app.c` 有 `VT_enter_global_mutex()` / `VT_leave_global_mutex()` weak stub，这类入口应逐步归入 PicoUI OS contract，而不是继续散在 backend app。

### 5. resource port contract

资源层要解决 `void *tile`、`void *font`、`vres_addr` 的语义偏底层问题。

建议引入带 kind 的 PicoUI resource：

```c
enum picoui_resource_kind {
    PICOUI_RESOURCE_IMAGE_MEMORY,
    PICOUI_RESOURCE_IMAGE_VRES,
    PICOUI_RESOURCE_FONT_MEMORY,
    PICOUI_RESOURCE_FONT_VRES,
};

struct picoui_resource {
    enum picoui_resource_kind kind;
    const void *data;
    unsigned int address;
    void *backend_data;
};
```

`picoui_native_image_wrap()`、`picoui_native_font_wrap()` 可以保留为兼容入口，但文档必须写清：这是 native bridge，不是推荐的新平台资源模型。

说明：`resource port` 暂不并入本轮首批固定目录。本轮先冻结 `display / indev / tick / osal` 四类最小合同，`resource` 作为后续独立设计线推进。

## 分阶段落地建议

### P0：文档和边界冻结

目标：先冻结口径，避免后续继续把 port 能力塞进 `backend_app.c` 或 `examples/sdl`。

动作：

1. 在 a-0.14 索引挂本审计文档。
2. 后续新增芯片/屏幕/输入能力时，必须先进入 PicoUI port contract，不允许直接扩 `ldConfig.*`、`backend_app.c` 或 `examples/sdl`。
3. 明确 `backend_app.c` 当前是 host harness + LingDongGUI bridge 的混合过渡文件，不是最终平台适配结构。

### P1：SDL port 先行

目标：把最明显的 SDL 与 LingDongGUI 穿透点拆出来。

动作：

1. 增加 `picoui_display_config` 与 display flush/readback 最小结构。
2. 增加 pointer input push API。
3. 在 `picoui/port/sdl/` 新建 SDL display/input/tick/os port。
4. SDL port 直接适配 PicoUI port 合同，不再挂到 `examples/sdl`。
5. LingDongGUI backend 私有层继续把这些入口映射到 `arm_2d_tile_t` 和 `ldCfgTouchSetPoint()`。

验收：

- `backend_app.c` 不再直接以 `LD_CFG_SCREEN_WIDTH/HEIGHT` 作为 SDL pointer 映射源。
- SDL runtime 尺寸来自 PicoUI display config。
- PicoUI SDL port 入口在 `picoui/port/sdl/`，不是 `examples/sdl`。
- 现有 PicoUI runtime/mapping/visible gate 不退化。

### P2：tick/OS/resource 抽象

目标：让裸机和 SDL 都能用同一 PicoUI port 合同。

动作：

1. tick source 从 SDL 调用点抽出。
2. weak mutex/delay 归入 PicoUI OS port。
3. image/font/vres 迁移到 PicoUI resource kind。
4. `native.h` / `image.h` 的旧 native bridge 保留，但标注为兼容路径。

验收：

- app timer 测试继续通过。
- image/font 旧 API 兼容。
- 新 resource API 有 focused unit test 与 LingDongGUI backend proof。

### P3：board port 示例

目标：给开发者一个不用读 LingDongGUI/ARM-2D 文档也能适配屏幕的 PicoUI 示例。

建议新增：

```text
picoui/port/sdl/
picoui/port/mh2103c/
picoui/docs/porting_guide.md
```

示例应展示：

- 如何声明 display config。
- 如何实现 flush 到 ST7789 或 SDL。
- 如何上报 pointer/key/encoder。
- 如何提供 tick。
- 如何跑 PicoUI app。

不要让示例要求用户 include `ldConfig.h`、`arm_2d_disp_adapter_0.h` 或直接调用 `ldGui*`。

## 不建议做的事

1. 不建议把 `LD_CFG_*` 简单复制成 `PICOUI_CFG_*` 就收工。这样只是改名，开发者仍需要理解 LingDongGUI/ARM-2D 的 PFB 和 display adapter 语义。
2. 不建议在 `backend_app.c` 继续堆平台逻辑。这个文件已经同时承担太多职责。
3. 不建议让 demo 通过固定坐标、fake fallback、手工 present 来证明适配完成。demo 只能表达用户意图。
4. 不建议把 `picoui_native_image_wrap(void *tile, ...)` 视为最终资源抽象。它是兼容桥，不是 portable resource contract。
5. 不建议把 `direct_public_100_complete` 写成平台适配完成。两者是不同维度。

## 建议门禁

后续真正实施时，至少增加这些 gate：

1. public header 泄漏检查：继续禁止 `ld*`、`arm_2d_*`、`SIGNAL_*` 出现在 `picoui/include/picoui/*.h`。
2. port contract unit test：display config、input push、tick source、resource kind。
3. backend proof：PicoUI port input 能驱动 LingDongGUI touch/event；display config 能驱动真实 framebuffer 尺寸。
4. SDL port gate：`picoui/port/sdl/` 能 build/run/capture；不再以 `examples/sdl` 作为新增 PicoUI port 承载路径。
5. board port smoke：至少一个非 SDL port 示例能编译，最好先以 MH2103C 为目标。
6. 文档 gate：`picoui/docs/porting_guide.md` 不出现要求用户直接调用 `ldGui*`、`ldCfg*`、`arm_2d_*` 的步骤。

## 推荐下一步

P1 已完成。后续如果继续推进，应进入 P2/P3，而不是回退到 `examples/sdl` 或重新把 port 逻辑塞回 `backend_app.c`。相关设计入口保留如下：

- spec：`docs/superpowers/specs/2026-06-05-picoui-port-layer-design.md`
- plan：`docs/superpowers/plans/2026-06-05-picoui-port-layer-implementation.md`

第一轮实现已落在 `display / indev / tick / osal + picoui/port/sdl/`。后续不要重新拆散这层边界；新增能力应继续按 P2/P3 串行推进。

## 最终口径

PicoUI 的平台适配目标应定义为：

> 开发者只需要实现 PicoUI `port` 下的 display/input/tick/resource/OS port，就能把 PicoUI 应用跑到 SDL 或真实芯片屏幕上；开发者不需要知道当前 backend 是否借用了 LingDongGUI、ARM-2D、display adapter 或 `ldConfig`。

当前仓库对 P1 已达到这个口径的最小闭环：

- 开发者不需要改 `picoui/src/` 固定核心实现就能接 SDL port。
- SDL 适配落在 `picoui/port/sdl/`，不是新增到 `examples/sdl`。
- backend 继续留在 `picoui/src/backend/ldgui/`，不引入 `port/ldgui`。

更宽的芯片/board/resource 移植能力仍属于 P2/P3，不应和本轮 P1 已完成结论混写。
