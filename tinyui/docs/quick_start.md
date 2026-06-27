# TINYUI 快速开始

TINYUI 是构建在 LingDongGUI 之上的应用层抽象，第一阶段提供统一的 `tinyui_*` API。

## 最小示例

```c
struct tinyui_app *app = tinyui_app_create();
struct tinyui_window *win = tinyui_window_create(app, "root");
struct tinyui_label *label = tinyui_label_create(win, "title");
tinyui_label_set_text(label, "Hello TINYUI");
```

## 图片资源

```c
static struct tinyui_image_source s_note_icon;

if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &s_note_icon) == 0) {
    struct tinyui_image *image = tinyui_image_create(win, "note_icon");
    tinyui_image_set_source(image, &s_note_icon);
}
```

`tinyui_image_source` 是资源句柄。长期使用的 source 要放在应用状态、页面状态或静态存储中。完整规则见 [TinyUI 资源生命周期](./resource_lifetime.md)。

## 推荐步骤

1. 创建 `app`
2. 创建 `window`
3. 创建基础控件
4. 设置布局与 theme
5. 绑定事件
6. 运行 demo

## 进一步阅读

- [TinyUI 资源生命周期](./resource_lifetime.md)
- [TINYUI Port 分层与适配规则](./porting_rules.md)
