# PicoUI 抽象层设计文档

> 日期：2026-05-26  
> 适用仓库：`/Users/cys/embedded/LingDongGUI`  
> 目标：在尽量不大改 `LingDongGUI` 内核的前提下，封装一个新的上层 API `PicoUI`，统一用户编程入口，隐藏 `LingDongGUI` 与 `ARM-2D` 细节，降低学习成本。

---

## 1. 背景与目标

当前 `LingDongGUI` 已经具备较完整的轻量型 GUI 基础控件与 `switch / flex / grid` 主路径能力，但对最终用户来说，仍存在几个学习成本高的问题：

- 需要理解 `ld*` 控件 API
- 容易接触到 `ARM-2D` 类型与概念
- 控件 API 风格不完全统一
- demo 与教程入口偏 `LingDongGUI`/`ARM-2D` 心智，而不是面向上层 UI 使用者

因此本轮不继续直接扩展单个控件，而是在 `LingDongGUI` 之上新增一个上层抽象：`PicoUI`。

`PicoUI` 的第一阶段目标：

1. 用户只使用统一的 `picoui_*` API
2. 用户不需要关心底层到底调用 `LingDongGUI` 还是 `ARM-2D`
3. 提供统一 demo 与统一学习入口
4. 对外 API 遵循 Linux 风格命名与函数规则
5. 尽量不大幅修改 `LingDongGUI` 内核

---

## 2. 设计原则

### 2.1 第一原则：用户看不到底层实现细节

`PicoUI` 对外 public API 中：

- 不出现 `ld*` 类型或函数
- 不出现 `arm_2d_*` 类型或函数
- 不要求用户理解 `SIGNAL_*`、`ptScene`、`arm_2d_tile_t`、`arm_2d_font_t`

### 2.2 第二原则：Linux 风格命名与函数规则

对外 API、文件、类型与常量遵守以下约束：

- 文件名：小写 + 下划线
- 函数名：`picoui_*`
- 类型名：`struct picoui_*`
- 枚举名：`enum picoui_*`
- 枚举值：`PICOUI_*`
- 优先真实函数，少用宏
- 创建失败返回 `NULL`
- 设置类接口返回 `int`，`0` 成功，负值为错误码

### 2.3 第三原则：不重写 LingDongGUI

`PicoUI` 是 `LingDongGUI` 上层抽象，不是新 GUI 内核。

优先级固定为：

1. PicoUI 统一 public API
2. PicoUI backend 做适配
3. 必要时对 `LingDongGUI` 做最小补洞
4. 不做大规模内核翻修

### 2.4 第四原则：第一阶段只做可闭环的最小范围

第一阶段只覆盖：

- 基础控件
- 基础布局
- `theme v0`
- 统一 demo

明确不做复杂输入控件、数据绑定、国际化系统、多 backend 等大范围扩展。

---

## 3. 第一阶段范围

### 3.1 纳入范围

#### 基础控件

- `window`
- `label`
- `text`
- `image`
- `button`
- `checkbox`
- `switch`
- `slider`

#### 基础布局

- `flex`
- `grid`

#### 视觉统一能力

- `theme v0`
- 最小 `state` / `part` 词汇表
- 控件默认样式下发

#### 用户入口

- 统一 demo
- 面向 `PicoUI` 的文档入口

### 3.2 明确不纳入范围

第一阶段不做：

- `list`
- `table`
- `line_edit`
- `keyboard`
- `combo_box`
- 通用数据绑定系统
- 国际化系统
- 声明式节点树 runtime
- 多 backend
- 通用 CSS 式样式系统
- 复杂动画系统
- `LingDongGUI` 全仓大重构

---

## 4. 总体架构

`PicoUI` 作为上层抽象，`LingDongGUI` 作为当前唯一 backend/runtime。

### 4.1 分层

- `PicoUI public API`
  - 用户直接使用
  - 只暴露 `picoui_*`
- `PicoUI core`
  - app/context/widget 基础对象
  - 通用属性与生命周期
- `PicoUI widgets/layout/theme`
  - 统一控件、布局、主题语义
- `PicoUI backend/ldgui`
  - 映射到 `LingDongGUI`
- `LingDongGUI`
  - 当前唯一绘制/事件/runtime backend

### 4.2 定位关系

- `PicoUI`：应用层抽象 / 用户 API
- `LingDongGUI`：承载控件与运行时的 backend
- `ARM-2D`：更底层实现，不暴露给 `PicoUI` 用户

### 4.3 关键边界

#### public 层禁止泄漏

所有 `picoui/include/picoui/*.h` 中：

- 禁止出现 `ld*`
- 禁止出现 `arm_2d_*`
- 禁止 include backend 专用头

#### backend 层独占底层映射

只有 `picoui/src/backend/ldgui/` 允许直接碰：

- `ldWindow_t`
- `ldButton_t`
- `ldSwitch_t`
- `arm_2d_tile_t`
- `arm_2d_font_t`
- `SIGNAL_*`

---

## 5. 目录结构

建议采用独立子树，避免直接混入 `src/gui/`：

```text
picoui/
  include/picoui/
    picoui.h
    app.h
    widget.h
    theme.h
    layout.h
    window.h
    label.h
    text.h
    image.h
    button.h
    checkbox.h
    switch.h
    slider.h

  src/core/
    app.c
    widget.c
    event.c
    resource.c
    internal.h

  src/theme/
    theme.c

  src/layout/
    flex.c
    grid.c

  src/widgets/
    window.c
    label.c
    text.c
    image.c
    button.c
    checkbox.c
    switch.c
    slider.c

  src/backend/ldgui/
    backend.h
    backend_widget.c
    backend_theme.c
    backend_layout.c
    backend_event.c
    backend_window.c
    backend_label.c
    backend_text.c
    backend_image.c
    backend_button.c
    backend_checkbox.c
    backend_switch.c
    backend_slider.c

  demo/
    hello_world/
    basic_widgets/
    layout_flex/
    layout_grid/
    theme_showcase/
    settings_panel/

  docs/
    quick_start.md
    api_overview.md
    demo_guide.md
```

---

## 6. Public API 设计

### 6.1 用户可见头文件

用户只 include：

- `picoui/picoui.h`

或按需 include：

- `picoui/app.h`
- `picoui/widget.h`
- `picoui/theme.h`
- `picoui/layout.h`
- `picoui/button.h`
- `picoui/switch.h`
- ...

### 6.2 对外类型

对外只暴露 opaque handle：

- `struct picoui_app`
- `struct picoui_widget`
- `struct picoui_window`
- `struct picoui_label`
- `struct picoui_text`
- `struct picoui_image`
- `struct picoui_button`
- `struct picoui_checkbox`
- `struct picoui_switch`
- `struct picoui_slider`
- `struct picoui_theme`
- `struct picoui_font`
- `struct picoui_image_source`

backend 内部再持有真实 `LingDongGUI` widget 指针。

### 6.3 双入口 API 形态

第一阶段采用：

1. `create + set` 过程式 API
2. `props struct` 便捷创建 API

所有控件都必须有 `create`，常用控件提供 `create_with_props`。

#### 示例 1：过程式 API

```c
struct picoui_button *btn = picoui_button_create(parent, "ok_button");
picoui_widget_set_size((struct picoui_widget *)btn, 80, 32);
picoui_button_set_text(btn, "OK");
picoui_button_set_on_clicked(btn, on_ok_clicked, user_data);
```

#### 示例 2：props API

```c
struct picoui_button_props props = {
    .id = "ok_button",
    .text = "OK",
    .width = 80,
    .height = 32,
    .on_clicked = on_ok_clicked,
    .user_data = user_data,
};

struct picoui_button *btn =
    picoui_button_create_with_props(parent, &props);
```

### 6.4 通用 widget API

所有控件共享的基础 contract：

- `picoui_widget_set_pos`
- `picoui_widget_set_size`
- `picoui_widget_set_visible`
- `picoui_widget_set_enabled`
- `picoui_widget_set_text`
- `picoui_widget_set_style_class`
- `picoui_widget_set_user_data`
- `picoui_widget_set_flex_grow`
- `picoui_widget_set_flex_new_track`
- `picoui_widget_set_ignore_layout`
- `picoui_widget_set_grid_cell`

### 6.5 控件专有 API

#### button

- `picoui_button_create`
- `picoui_button_create_with_props`
- `picoui_button_set_text`
- `picoui_button_set_on_clicked`

#### checkbox

- `picoui_checkbox_create`
- `picoui_checkbox_set_text`
- `picoui_checkbox_set_checked`
- `picoui_checkbox_is_checked`
- `picoui_checkbox_set_on_toggled`

#### switch

- `picoui_switch_create`
- `picoui_switch_set_checked`
- `picoui_switch_is_checked`
- `picoui_switch_set_on_toggled`

#### slider

- `picoui_slider_create`
- `picoui_slider_set_value`
- `picoui_slider_get_value`
- `picoui_slider_set_range`
- `picoui_slider_set_on_value_changed`

### 6.6 props 设计原则

`props` 只包含创建时高频初始化项：

- `id`
- `text`
- `width`
- `height`
- `checked`
- `value`
- `style_class`
- callback
- `user_data`

不包含 backend 私有细节，不出现：

- `arm_2d_tile_t *`
- `arm_2d_font_t *`
- `ldColor`
- `ld*` 指针

---

## 7. 主题与最小视觉语义

### 7.1 theme v0 目标

第一阶段不做通用样式引擎，只做最小统一视觉层：

- 默认颜色
- 默认字体
- 默认间距
- 默认圆角
- 默认控件高度
- 基础状态色

### 7.2 theme token

#### 颜色

- 主文本色
- 次文本色
- 背景色
- 面板色
- 边框色
- 强调色
- 禁用色
- 可选：成功/警告/错误色

#### 字体

- 默认字体
- 标题字体
- 小字字体

#### 尺寸

- 默认间距
- 小/中/大 padding
- 默认圆角
- 默认边框宽度
- 默认控件高度

### 7.3 最小 state 词汇

- `PICOUI_STATE_DEFAULT`
- `PICOUI_STATE_PRESSED`
- `PICOUI_STATE_CHECKED`
- `PICOUI_STATE_DISABLED`
- `PICOUI_STATE_FOCUSED`

### 7.4 最小 part 词汇

- `PICOUI_PART_MAIN`
- `PICOUI_PART_TEXT`
- `PICOUI_PART_INDICATOR`
- `PICOUI_PART_KNOB`
- `PICOUI_PART_TRACK`

### 7.5 映射原则

第一阶段 `part/state` 主要是 **PicoUI 统一语义层**，不强求 `LingDongGUI` 立即变成统一样式引擎。

backend 可以按控件分别映射：

- `button`：`MAIN` + `TEXT`
- `checkbox`：`MAIN` + `INDICATOR` + `TEXT`
- `switch`：`TRACK` + `INDICATOR` + `KNOB`
- `slider`：`TRACK` + `INDICATOR` + `KNOB`

### 7.6 第一阶段样式落点

第一阶段只做两层：

1. theme 默认样式
2. 控件级少量覆盖

建议开放的 widget 常用样式接口：

- `picoui_widget_set_bg_color`
- `picoui_widget_set_text_color`
- `picoui_widget_set_border_color`
- `picoui_widget_set_radius`
- `picoui_widget_set_padding`

不做：

- 任意属性系统
- 样式选择器
- CSS 式继承
- 动态样式计算器

---

## 8. 布局 contract

### 8.1 总原则

PicoUI 只暴露两套布局：

- `flex`
- `grid`

不发明第三套布局语义，直接把现有 `LingDongGUI` 已补齐主干能力整理成统一 public API。

### 8.2 flex 语义

#### 容器级接口

- `picoui_flex_set_flow`
- `picoui_flex_set_align`
- `picoui_flex_set_gap`

#### 子项级接口

- `picoui_widget_set_flex_grow`
- `picoui_widget_set_flex_new_track`
- `picoui_widget_set_ignore_layout`

#### 第一阶段保证映射的能力

- row / column / wrap / reverse
- main / cross / track align
- gap
- grow
- new track
- ignore layout

### 8.3 grid 语义

#### 容器级接口

- `picoui_grid_set_columns`
- `picoui_grid_set_rows`
- `picoui_grid_set_gap`
- `picoui_grid_set_align`

#### 子项级接口

- `picoui_widget_set_grid_cell`

#### 第一阶段保证映射的能力

- fixed / content / fr
- explicit cell
- span
- cell align
- container align
- auto placement
- ignore layout

### 8.4 第一阶段延期语义

不在第一阶段承诺：

- flex `RTL`
- flex margin / percent / content-size 完整联动
- grid `subgrid`
- grid `RTL`

---

## 9. 事件 contract

### 9.1 总原则

用户不直接碰 `SIGNAL_*`，PicoUI 对外统一成 callback 风格。

### 9.2 第一阶段统一事件名词

- `clicked`
- `value_changed`
- `toggled`
- `pressed`
- `released`

### 9.3 回调签名

建议基础回调：

```c
typedef void (*picoui_event_cb)(struct picoui_widget *widget,
                                void *user_data);
```

值变化回调：

```c
typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);
```

### 9.4 对外接口示例

- `picoui_button_set_on_clicked`
- `picoui_switch_set_on_toggled`
- `picoui_slider_set_on_value_changed`
- `picoui_checkbox_set_on_toggled`

### 9.5 映射原则

backend 内部再将这些统一事件映射到：

- `SIGNAL_PRESS`
- `SIGNAL_RELEASE`
- `SIGNAL_VALUE_CHANGED`

用户不需要知道 `LingDongGUI` 消息系统的存在。

---

## 10. 资源抽象

为避免图片/字体再次泄漏底层类型，第一阶段引入最小资源句柄：

- `struct picoui_font`
- `struct picoui_image_source`

用户只通过 PicoUI 资源句柄使用：

- `picoui_label_set_font`
- `picoui_text_set_font`
- `picoui_image_set_source`

backend 内部再映射到：

- `arm_2d_font_t *`
- `arm_2d_tile_t *`

---

## 11. 统一 demo 设计

### 11.1 目标

目标不是再提供一个零散示例，而是建立新的学习入口：

- 用户通过 PicoUI demo 学会 PicoUI
- 不需要先理解 LingDongGUI 旧 demo 结构
- 不需要先理解 ARM-2D

### 11.2 demo 结构

- `hello_world`
  - 最短路径
  - app / window / label / button
- `basic_widgets`
  - `button / checkbox / switch / slider / image / text`
- `layout_flex`
  - row / column / wrap / grow
- `layout_grid`
  - columns / rows / span / align
- `theme_showcase`
  - 同一 theme 落到多个控件
- `settings_panel`
  - 第一份综合示例
  - 接近真实产品页面

### 11.3 demo 约束

所有 PicoUI demo 源码中：

- 不出现 `ld*`
- 不出现 `arm_2d_*`
- 不出现 `SIGNAL_*`

---

## 12. 实现边界与最小补洞策略

### 12.1 默认策略

优先在 PicoUI 层完成统一与适配，不直接改 LingDongGUI 内核。

### 12.2 允许的最小补洞

只有当 PicoUI 确实依赖某个缺失能力时，才允许对 `LingDongGUI` 做最小增补，例如：

- 小 API 增补
- 小行为修正
- 小测试补齐

### 12.3 禁止事项

- 为 PicoUI 重写 `LingDongGUI` renderer
- 为 PicoUI 大规模改造 `LingDongGUI` 控件架构
- 为 PicoUI 引入超大范围样式系统翻修

---

## 13. 建议落地顺序

### 阶段 1：PicoUI 骨架

- 建目录结构
- 建 public header
- 建 core/app/widget 基础对象
- 建 CMake 接线

### 阶段 2：backend 最小桥

- app/window/widget 生命周期
- visible/enabled/pos/size/text 等通用属性桥接

### 阶段 3：theme v0

- theme token
- 默认主题
- widget 默认样式下发

### 阶段 4：先接关键控件

- `window`
- `label`
- `button`
- `switch`

### 阶段 5：统一事件层

- clicked
- toggled
- value_changed

### 阶段 6：接布局

- flex
- grid

### 阶段 7：补剩余第一阶段控件

- `text`
- `image`
- `checkbox`
- `slider`

### 阶段 8：统一 demo

- 从 `hello_world` 到 `settings_panel`

### 阶段 9：PicoUI 用户文档

- 快速开始
- API 概览
- demo 学习路径
- PicoUI 与 LingDongGUI 边界说明

---

## 14. 里程碑

### M1

- app/window/label/button
- theme v0
- `hello_world`

### M2

- switch/checkbox/slider
- 统一事件
- `basic_widgets`

### M3

- flex/grid
- `theme_showcase`
- `settings_panel`
- PicoUI 用户文档

---

## 15. 验收标准

第一阶段完成时，应满足：

1. 新用户只看 PicoUI 文档与 demo，即可完成：
   - 建页面
   - 放控件
   - 用 flex/grid 布局
   - 收事件
   - 应用 theme
2. PicoUI demo 源码不泄漏 `ld*` / `arm_2d_*`
3. public header 不泄漏 `LingDongGUI` 与 `ARM-2D` 类型
4. Linux 风格命名规则在 PicoUI public API 中保持一致
5. `LingDongGUI` 本体只发生最小必要补洞，不出现大规模重构

---

## 16. 最终结论

`PicoUI` 第一阶段的正确定位，不是重写 `LingDongGUI`，也不是再造一个完整 `LVGL`，而是：

- 在现有 `LingDongGUI` 主干能力之上
- 建立一套更统一、对用户更友好的 Linux 风格 C API
- 隐藏 `LingDongGUI` 与 `ARM-2D` 细节
- 提供统一 demo 与统一学习路径
- 以最小改动方式显著降低学习成本

这套设计如果按边界执行，`PicoUI` 将更像一个稳定的应用层框架入口，而不是简单的 `ld*` 换前缀包装壳。
