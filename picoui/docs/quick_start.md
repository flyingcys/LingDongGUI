# PicoUI 快速开始

当前 `v1.0.1` 的 PicoUI 主路径是 native-only：

```text
picoui_init() -> port/display/indev -> active screen/root window -> picoui_timer_handler()
```

`picoui_app_*` 仍在 public header 中保留，但它们只是 pre-v1.0 compatibility wrapper，不是 `v1.0+` quick start 的推荐入口。

## 最小 SDL host 示例

```c
#include "picoui/picoui.h"
#include "picoui/port/sdl.h"

static int create_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *root = picoui_window_create_root(screen, "root");
    struct picoui_label *label;

    if (root == 0) {
        return -1;
    }

    label = picoui_label_create(root, "title");
    if (label == 0) {
        return -1;
    }

    picoui_label_set_text(label, "Hello PicoUI");
    return picoui_screen_load(screen);
}

int main(void)
{
    int rc;

    rc = picoui_init();
    if (rc != 0) {
        return 1;
    }

    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }

    if (create_ui() != 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        rc = picoui_timer_handler();
        if (rc < 0) {
            picoui_deinit();
            return 1;
        }
        if (rc > 0) {
            break;
        }
    }

    picoui_deinit();
    return 0;
}
```

## 最小步骤

1. 调 `picoui_init()` 初始化 runtime。
2. 通过 `picoui_sdl_hal_init()` 或自定义 port 创建默认 display / indev。
3. 用 `picoui_screen_active()` 取得 active screen。
4. 用 `picoui_window_create_root()` 创建 root window，再创建子控件。
5. 用 `picoui_screen_load()` 把 screen 载入当前 runtime。
6. 在宿主主循环里周期调用 `picoui_timer_handler()`。
7. 退出前调用 `picoui_deinit()`。

## 自定义板级移植时的最小差异

- SDL host：直接用 `picoui_sdl_hal_init(width, height)`。
- 自定义板卡/RTOS：
  - 创建 `picoui_display`
  - 注册 flush callback
  - 创建 `picoui_indev`
  - 注册 pointer/key read callback
  - 再进入同样的 `screen/root window/timer_handler` 主路径

## 进一步阅读

- [PicoUI Port 分层与适配规则](./porting_rules.md)
- [PicoUI API 总览](./api_overview.md)
