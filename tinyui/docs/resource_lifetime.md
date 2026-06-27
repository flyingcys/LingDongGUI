# TinyUI 资源生命周期

本文说明 `tinyui_image_source` 和 `tinyui_font` 的所有权规则。用户代码应只通过 TinyUI public API 创建、传递和销毁资源句柄。

## 图像资源

`struct tinyui_image_source` 是图像句柄。不要手写它的底层字段，也不要在 demo 或业务代码中引用底层图片宏、底层 tile 类型或底层头文件。

推荐来源：

1. `tinyui_image_source_from_builtin(image, out)`：取得 TinyUI 内置图片资源。
2. `tinyui_image_source_from_vres(vres, out)`：从 vres 数据创建图片资源。
3. 外部资源：当前不提供推荐 factory；如需接入外部宿主管理的资源，应先在业务层封装成稳定的 TinyUI 资源句柄，再按本页生命周期规则管理。

销毁规则：

1. builtin：资源由 TinyUI 管理，`tinyui_image_source_destroy()` 只清空句柄，不释放静态资源。
2. vres：资源由创建函数分配，必须在不再被 widget 使用后调用 `tinyui_image_source_destroy()`。
3. external：资源由调用方管理，`tinyui_image_source_destroy()` 只清空句柄，不释放外部资源。
4. 对空句柄或已清空句柄重复调用 destroy 是安全的。

生命周期规则：

1. 传给 widget 的 source 对象必须活得比该 widget 的使用期更久。
2. 不要把栈上临时 source 传给长期存在的 widget。
3. 需要长期显示的图片，建议把 source 放在静态存储、应用状态或页面状态中。

## 字体资源

`struct tinyui_font` 是字体句柄。用户代码同样不应接触底层字体结构。

推荐来源：

1. `tinyui_font_from_vres(vres, out)`：从 vres 数据创建字体资源。

销毁规则：

1. vres 字体在不再被 widget 使用后调用 `tinyui_font_destroy()`。
2. 重复 destroy 已清空字体句柄是安全的。
3. 字体句柄必须活得比使用它的 label、button、text 等 widget 更久。

## demo 约束

`tinyui/demo/*` 和 `tinyui_demo/*` 只展示 TinyUI public API。demo 需要图片时，应使用 `tinyui_image_source_from_builtin()` 或 `tinyui_image_source_from_vres()`，不能展示底层资源字段或底层头文件。
