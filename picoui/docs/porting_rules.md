# PicoUI Port 分层与适配规则

## 目标

`v1.0.1` 的 PicoUI 目标是让开发者在没有 public app object 的前提下，把显示、输入、tick 和 OS 适配接到 native runtime：

```text
public API -> native runtime/widgets -> ARM-2D -> port/display
```

因此 port 文档的重点不是 `picoui_app_*`，而是：

- display flush callback
- input read callback
- buffer / stripe height
- host main loop 与 `picoui_timer_handler()`

## 主路径约束

当前推荐主路径：

1. `picoui_init()`
2. 建立默认 display / indev
3. `picoui_screen_active()` + `picoui_window_create_root()`
4. `picoui_screen_load()`
5. 宿主周期调用 `picoui_timer_handler()`
6. `picoui_deinit()`

明确约束：

- 不要求用户先拿 `struct picoui_app *`
- 不把 `picoui_app_create()` / `picoui_app_run()` 当作新 port 的中心
- `picoui_app_*` 若继续存在，只能视为 compatibility wrapper

## display 适配

### SDL host

SDL host 直接用：

```c
picoui_sdl_hal_init(320, 480);
```

它会建立默认 display，并注册默认 pointer indev。

### 自定义板卡/宿主

最小 display 路径：

```c
struct picoui_display *display = picoui_display_create(width, height);
picoui_display_set_default(display);
picoui_display_set_flush_cb(display, my_flush_cb, my_user_data);
```

`my_flush_cb` 需要接收：

- `const struct picoui_area *area`
- `const void *pixels`
- `void *user_data`

回调语义：

- `area` 描述当前 flush 的矩形区域
- `pixels` 指向该区域的像素缓冲
- port 负责把这块像素刷到真实屏幕

### buffer / stripe height

PicoUI runtime 采用分块 flush 的思路，不要求整屏一次性提交。

当前需要明确的 buffer 概念：

- `struct picoui_display_config.buffer_height`
  - 表达一次 flush 的 stripe / PFB 高度
  - 适合 compatibility path 或 host 集成层保存配置时使用
- SDL host helper 会提供默认内部缓冲
- 自定义板卡应保证：
  - flush callback 看到的像素块大小和底层屏幕驱动一致
  - stripe height 不要和底层 DMA / PFB 真实大小长期漂移

## 输入设备适配

最小 indev 路径：

```c
struct picoui_indev *indev = picoui_indev_create();
picoui_indev_set_type(indev, PICOUI_INDEV_TYPE_POINTER);
picoui_indev_set_read_cb(indev, my_indev_read_cb, my_user_data);
```

`my_indev_read_cb` 需要填写：

```c
struct picoui_indev_data {
    int pointer_x;
    int pointer_y;
    int pressed;
    enum picoui_input_key key;
};
```

常见模式：

- pointer/touch：
  - 填 `pointer_x / pointer_y / pressed`
- keypad/encoder：
  - 填 `key`

可用类型：

- `PICOUI_INDEV_TYPE_POINTER`
- `PICOUI_INDEV_TYPE_KEYPAD`
- `PICOUI_INDEV_TYPE_ENCODER`

## 目录职责

### `picoui/src/`

放 PicoUI 固定实现：

- core/runtime/widget/layout/theme
- native runtime 状态
- `picoui/src/backend/ldgui/` 私有桥接

不放：

- 板级驱动差异
- SDL/RTOS/BSP 适配细节

### `picoui/port/`

放开发者需要按平台维护的适配层：

- `picoui/port/sdl/`
- `picoui/port/<board>/`
- `picoui/port/<rtos>/`

这层负责：

- 创建 display
- 注册 flush callback
- 创建 indev
- 注册输入回调
- 接入 tick / delay / lock 等 OS 能力

### `picoui/demo/`

只表达用户意图和示例页面：

- 不承担板级驱动适配
- 不承载 fake renderer 补丁
- 不把平台初始化细节散回 demo

## backend 与 port 的边界

`picoui/src/backend/ldgui/` 负责：

- 把 PicoUI widget/runtime 状态映射到 LingDongGUI/ARM-2D 私有实现
- 维持内部 bridge truth

`picoui/port/*` 负责：

- 把真实宿主 display/input/tick 能力接到 PicoUI public contract

明确禁止：

- 不把 LingDongGUI backend 私有实现挪进 `picoui/port/ldgui`
- 不把 demo 逻辑放进 port
- 不把平台驱动细节重新塞回 `picoui/src/core`

## compatibility 说明

以下 API 仍可见，但不应成为新 port 的中心：

- `picoui_app_create()`
- `picoui_app_run()`
- `picoui_app_destroy()`
- `picoui_display_set_config(struct picoui_app *, ...)`
- `picoui_display_set_flush_callback(struct picoui_app *, ...)`
- `picoui_input_push_pointer(struct picoui_app *, ...)`
- `picoui_input_push_key(struct picoui_app *, ...)`

它们保留的理由是：

- 承接 pre-v1.0 wrapper 路线
- 给兼容 demo / 旧测试留过渡入口

但 `v1.0+` 新文档、新 demo、新测试，应围绕：

- `picoui_init()`
- `picoui_display_*`
- `picoui_indev_*`
- `picoui_screen_*`
- `picoui_timer_handler()`

## 推荐 bring-up 清单

1. 先确认 `picoui_init()` / `picoui_deinit()` 生命周期。
2. 建 display，并接通 flush callback。
3. 建 indev，并接通 pointer/key read callback。
4. 明确 buffer/stripe height。
5. 创建 active screen 和 root window。
6. 用最小 demo 验证 `picoui_timer_handler()` 真在周期运行。
