# TinyUI 快速开始

TinyUI 是构建在 LingDongGUI（与 Arm-2D）之上的**轻量 canonical API**，不是第二套 GUI 对象树或渲染器。

- 用户只通过 `tinyui_*` public API 表达界面意图。
- 真实绘制、布局求解与控件行为仍由 LingDongGUI 后端完成。
- API 使用模式参考 LVGL，但**不兼容 LVGL**，也不复制其内部机制。

安装后的消费方式（仓外工程）：

```cmake
find_package(TinyUI 2.3 CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE TinyUI::tinyui)
```

统一入口头：`#include <tinyui.h>`。

## 可链接示例：资源与结果码

下列示例只依赖安装包中的 public 头与 `TinyUI::tinyui`，不调用需要平台宿主的 runtime 生命周期，适合作为仓外 consumer 的最小编译链接探针。

```c compile
#include <tinyui.h>

int main(void)
{
    uint16_t pixels[4] = {0, 0, 0, 0};
    tinyui_image_source_t source;
    tinyui_font_t font;
    tinyui_result_t result;

    /* stride 为字节：2 像素 * sizeof(uint16_t) = 4 */
    result = tinyui_image_source_from_rgb565(pixels,
                                            2,
                                            2,
                                            4,
                                            0,
                                            0,
                                            &source);
    if (result != TINYUI_OK) {
        return 1;
    }

    result = tinyui_font_from_builtin(TINYUI_FONT_6X8, &font);
    if (result != TINYUI_OK) {
        tinyui_image_source_deinit(&source);
        return 2;
    }

    /* 未绑定到对象前即可释放 descriptor */
    tinyui_font_deinit(&font);
    tinyui_image_source_deinit(&source);

    (void)tinyui_last_result();
    return 0;
}
```

C++ 同样可直接包含 `<tinyui.h>`（头文件已提供 `extern "C"`）：

```cpp compile
#include <tinyui.h>

int main()
{
    tinyui_image_source_t source;
    tinyui_result_t result =
        tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &source);
    if (result != TINYUI_OK) {
        return 1;
    }
    tinyui_image_source_deinit(&source);
    return 0;
}
```

## 最小完整生命周期（阅读示例）

完整 UI 生命周期需要平台宿主提供显示/输入/时钟集成（例如仓库内 SDL demo runner）。**仅链接安装包中的三个静态库时，`tinyui_init` / `tinyui_process` 可能因宿主符号无法完成链接**；因此下面用普通代码块展示完整流程，不作为 `compile` 门禁。

平台 `sleep` / 时钟推进只属于**集成层职责**，不要把伪 port 或 SDL 细节写进业务代码。

```c
#include <tinyui.h>

static void on_button_event(const tinyui_event_t *event)
{
    if (event == NULL) {
        return;
    }
    if (event->code == TINYUI_EVENT_CLICKED) {
        /* 处理点击；user_data 来自注册时传入的指针 */
        (void)event->user_data;
    }
}

int app_main(void)
{
    tinyui_result_t result;
    tinyui_obj_t *screen = NULL;
    tinyui_obj_t *label = NULL;
    tinyui_obj_t *button = NULL;
    tinyui_event_handle_t handle = 0;
    uint32_t next_ms = 0;
    int running = 1;

    result = tinyui_init();
    if (result != TINYUI_OK) {
        return 1;
    }

    screen = tinyui_screen_create();
    if (screen == NULL) {
        tinyui_deinit();
        return 1;
    }

    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    if (label == NULL || button == NULL) {
        tinyui_obj_delete(screen);
        tinyui_deinit();
        return 1;
    }

    (void)tinyui_label_set_text(label, "Hello TinyUI");
    (void)tinyui_button_set_text(button, "OK");

    /* 统一事件回调；也可用 tinyui_button_set_on_clicked */
    result = tinyui_obj_add_event_cb(button,
                                     TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                     on_button_event,
                                     NULL,
                                     &handle);
    if (result != TINYUI_OK) {
        tinyui_obj_delete(screen);
        tinyui_deinit();
        return 1;
    }

    result = tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0);
    if (result != TINYUI_OK) {
        tinyui_obj_delete(screen);
        tinyui_deinit();
        return 1;
    }

    while (running) {
        result = tinyui_process(&next_ms);
        if (result != TINYUI_OK) {
            break;
        }
        /* 集成层：按 next_ms 休眠 / 推进时钟 / 泵送输入 */
        (void)next_ms;
    }

    tinyui_obj_delete(screen);
    tinyui_deinit();
    return 0;
}
```

错误路径要点：

1. `tinyui_init` 失败则不要继续创建对象。
2. 控件创建失败时删除已创建的 screen（子对象随树释放），再 `tinyui_deinit`。
3. `tinyui_screen_load` 使用 `TINYUI_SCREEN_TRANSITION_NONE` 与 `duration_ms = 0` 作为无动画默认。

## 图片 descriptor 小片段

长期显示的 `tinyui_image_source` 应放在静态存储、页面状态或应用状态中；绑定到对象期间 descriptor 必须保持有效。

```c
static tinyui_image_source_t s_note_icon;

void page_bind_note_icon(tinyui_obj_t *image)
{
    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE,
                                         &s_note_icon) != TINYUI_OK) {
        return;
    }
    (void)tinyui_image_set_source(image, &s_note_icon);
}
```

完整规则见 [资源生命周期](./resource_lifetime.md)。

## 推荐步骤

1. `find_package(TinyUI)` 并链接 `TinyUI::tinyui`
2. 平台宿主完成显示/输入/时钟集成
3. `tinyui_init` → `tinyui_screen_create` → 创建控件
4. 设置布局、theme/style、事件回调
5. `tinyui_screen_load(..., TINYUI_SCREEN_TRANSITION_NONE, 0)`
6. 循环调用 `tinyui_process(&next_ms)`，由集成层处理休眠
7. 退出时 `tinyui_obj_delete` / `tinyui_deinit`

## 进一步阅读

- [API 总览](./api_overview.md)
- [资源生命周期](./resource_lifetime.md)
- [Demo 运行指南](./demo_guide.md)
- [Port 分层与适配规则](./porting_rules.md)（port 生产闭环仍属延期项）
