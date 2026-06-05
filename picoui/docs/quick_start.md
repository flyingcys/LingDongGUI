# PicoUI 快速开始

PicoUI 是构建在 LingDongGUI 之上的应用层抽象，第一阶段提供统一的 `picoui_*` API。

## 最小示例

```c
struct picoui_app *app = picoui_app_create();
struct picoui_window *win = picoui_window_create(app, "root");
struct picoui_label *label = picoui_label_create(win, "title");
picoui_label_set_text(label, "Hello PicoUI");
```

## 推荐步骤

1. 创建 `app`
2. 创建 `window`
3. 创建基础控件
4. 设置布局与 theme
5. 绑定事件
6. 运行 demo

## 进一步阅读

- [PicoUI Port 分层与适配规则](./porting_rules.md)
