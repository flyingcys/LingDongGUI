# PicoUI API 总览

当前 `v1.0.1` 口径下，PicoUI 的主路径不是旧 `wrapper/backend` 路线，而是 native-only：

```text
PicoUI public API -> native runtime/widgets -> ARM-2D -> port/display
```

## 启动与运行时

`v1.0+` 主路径围绕以下 API：

- `picoui_init()`
- `picoui_deinit()`
- `picoui_timer_handler()`
- `picoui_screen_active()`
- `picoui_screen_create()`
- `picoui_screen_load()`
- `picoui_window_create_root()`

典型结构：

1. 初始化 runtime
2. 建立 display / indev
3. 创建 active screen 的 root window
4. 创建控件树
5. 载入 screen
6. 在宿主循环中周期调用 `picoui_timer_handler()`

## 平台与 port API

当前 public API 里，平台接入主要看这几组：

### SDL host

- `picoui_sdl_hal_init(width, height)`
- `picoui_port_sdl_default_pointer_indev(...)`

### display

- `picoui_display_create()`
- `picoui_display_set_default()`
- `picoui_display_get_default()`
- `picoui_display_set_flush_cb()`
- `picoui_display_get_size()`

### input

- `picoui_indev_create()`
- `picoui_indev_set_type()`
- `picoui_indev_set_read_cb()`

这几组 API 的目标是让用户直接围绕 display / indev / screen 建立 runtime，而不是先持有 public app handle。

## 控件与布局

当前 public API 主要按以下类别组织：

- root/container：`window`、`background`
- 基础控件：`label`、`button`、`checkbox`、`switch`、`slider`、`text`、`image`、`list`
- 扩展控件：`arc`、`gauge`、`graph`、`calendar`、`clock`、`date_time`
- 输入类控件：`line_edit`、`keyboard`、`combo_box`、`scroll_selecter`、`table`
- 其他 widget：`message_box`、`progress_bar`、`progress_wheel`、`qrcode`、`icon_slider`、`radial_menu`、`animation`、`canvas`
- 布局：`flex`、`grid`
- 主题/样式：`theme`、base widget style helpers
- 资源：`resource`、`font`

## compatibility-only API

根据 `P0` 冻结决策，以下 `picoui_app_*` 仍保留在 public header，但它们不是 `v1.0+` 主路径：

- `picoui_app_create()` -> compatibility-only，主路径改为 `picoui_init()`
- `picoui_app_run()` -> compatibility-only，主路径改为用户周期调用 `picoui_timer_handler()`
- `picoui_app_destroy()` -> compatibility-only，主路径改为 `picoui_deinit()`
- `picoui_app_run_background()` -> compatibility-only，主路径改为 `picoui_screen_load()`
- `picoui_app_set_window()` / `picoui_app_set_background()` -> 以 `picoui_screen_load()` 为主
- `picoui_app_switch_window()` / `picoui_app_switch_background()` -> 后续收敛到 screen load / transition

结论：

- `picoui_app_*` 仍可用于旧 demo/旧测试过渡
- `v1.0+` quick start、demo main、新测试样例不得再把它们当主路径

## 当前文档边界

若需要当前推荐入口，先看：

- [quick_start.md](./quick_start.md)
- [porting_rules.md](./porting_rules.md)

若需要旧 wrapper 路线，只能看归档文档，不再把旧口径当作当前 truth source。
