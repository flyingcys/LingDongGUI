# TINYUI 快速开始

TINYUI 是构建在 LingDongGUI 之上的应用层抽象，第一阶段提供统一的 `tinyui_*` API。

## 最小示例

```c
struct tinyui_app *app = tinyui_app_create();
struct tinyui_window *win = tinyui_window_create(app, "root");
struct tinyui_label *label = tinyui_label_create(win, "title");
tinyui_label_set_text(label, "Hello TINYUI");
```

## 推荐步骤

1. 创建 `app`
2. 创建 `window`
3. 创建基础控件
4. 设置布局与 theme
5. 绑定事件
6. 运行 demo

## 进一步阅读

- [TINYUI Port 分层与适配规则](./porting_rules.md)
