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

### 15.1 当前能力矩阵（按 A1-A6 现态回写）

| 能力 | 当前状态 | 备注 |
| --- | --- | --- |
| `window/label/button/text` 真实 backend 映射 | 已完成 | 已建立真实 `LingDongGUI` 对象映射 |
| `checkbox/switch/slider` 真实 backend 映射 | 已完成 | 已建立真实对象、值同步与 native event |
| `image` 对象映射与 source 绑定 | 已完成 | `picoui_image_source` 已绑定到底层 `ldImage` |
| `image` theme/style apply | 当前拒绝 | 当前不是默认支持；`PICOUI_PART_MAIN` 明确拒绝 |
| `flex/grid` 真实 layout 映射 | 已完成 | 已映射到底层 `ldWindow/ldBase` 语义 |
| `theme/state/part/style` | 部分完成 | `window/button/checkbox/switch/slider/label/text` 已完成，`image` 仍拒绝 |
| runtime/capture | smoke 级证据 | 证明启动/capture/回归，不等于最终 backend closeout |
| `backend_app.c` 角色 | temporary smoke path | 当前仍保留 host runtime/fallback/capture 过渡职责 |

---

## 16. 最终结论

`PicoUI` 第一阶段的正确定位，不是重写 `LingDongGUI`，也不是再造一个完整 `LVGL`，而是：

- 在现有 `LingDongGUI` 主干能力之上
- 建立一套更统一、对用户更友好的 Linux 风格 C API
- 隐藏 `LingDongGUI` 与 `ARM-2D` 细节
- 提供统一 demo 与统一学习路径
- 以最小改动方式显著降低学习成本

这套设计如果按边界执行，`PicoUI` 将更像一个稳定的应用层框架入口，而不是简单的 `ld*` 换前缀包装壳。

> 现态补充（A7 final closeout）：上述“最终结论”现在已对应当前主线真相。`backend_app.c` 仍保留 `temporary smoke path` / host harness 职责，但它已不再承担正式 fake renderer 主输出职责；runtime/capture 仍只属于 smoke 级证据，而不是超出当前范围的额外承诺。

---

## 17. 二次 Review 结论（2026-05-26）

> 本节基于当前仓库代码与测试现状复盘，用于回答“是否已全部开发完成”。

> 文档口径修正：本节之后若出现“已完成收敛”“runtime 已含启动级 smoke”“文档与代码已对齐”之类结论，均应以后续实际代码与测试真相为准。按当前主线核对，`tests/picoui/runtime/check_picoui_runtime.py` 虽然已经会构建并逐个启动 6 个 demo，但它使用独立 build tree `build/picoui-runtime`，且 PicoUI demo 的构建链仍经由根入口默认开启的 SDL 子树，不应被表述为“已完全脱离 SDL、已全部闭环”。

### 17.1 总结论

> 以下 `17.x` 内容是 2026-05-26 当时的阶段性审计快照，不再代表当前最终完成判定；当前最终口径以上文 `15.1 当前能力矩阵` 与 A 线索引、Stage G closeout 门禁为准。

**未全部完成。**

当前状态是：`PicoUI` 的目录骨架、public API 基础形态、contract 检查和最小编译闭环已经落地；但“真正完成抽象层并映射到 LingDongGUI 行为”这件事仍未闭环，核心差距集中在 backend 实映射、事件链路、theme 真实下发和 demo 可运行闭环。

### 17.2 已完成项（可确认）

1. 目录结构与主要文件已落地（`picoui/include`、`picoui/src`、`picoui/demo`、`picoui/docs`、`tests/picoui`）。
2. public header 边界已建立：`picoui/include/picoui/*.h` 未泄漏 `ld*` / `arm_2d_*` / `SIGNAL_*`。
3. 第一阶段声明的 8 个基础控件 + `flex/grid` + `theme v0` 基础 API 已有最小接口外形。
4. PicoUI 测试分层已落地到 `tests/picoui/{unit,contract,runtime}`，并可通过 `ctest -L picoui` 跑通。
5. `README.md` 与教程已加入 PicoUI 入口，具备最小文档导航。

### 17.3 部分完成项（有接口，但未形成真实能力）

1. backend 当前是“占位实现”为主：`picoui/src/backend/ldgui/*.c` 主要做空校验或本地结构体赋值，尚未完成对 `LingDongGUI` 控件、布局、事件系统的真实映射。
2. 事件 contract 未闭环：`switch/checkbox/slider` 回调主要由 setter 主动触发，非来自底层事件；`button` 缺少 `picoui_button_set_on_clicked` 对外接口。
3. 布局 contract 未闭环：`flex/grid` 的 public setter 可调用，但 backend 仍是 no-op 形态，尚未证明真实布局行为生效。
4. theme v0 未闭环：已能存储 token，但缺“默认主题创建 + 控件级样式下发 + state/part 语义映射”。
5. demo 闭环未完成：6 个 demo 源码目录存在，但当前构建目标只明确覆盖 `settings_panel`，其余 demo 未形成统一可执行入口。

### 17.4 未完成项（与设计文档直接不一致）

1. 通用 widget API 存在缺口：设计中的 `picoui_widget_set_text`、`picoui_widget_set_style_class`、`picoui_widget_set_user_data` 未落地。
2. style 快捷接口未落地：`picoui_widget_set_bg_color`、`picoui_widget_set_text_color`、`picoui_widget_set_border_color`、`picoui_widget_set_radius`、`picoui_widget_set_padding` 未落地。
3. 主题语义缺口：`PICOUI_STATE_*` / `PICOUI_PART_*` 词汇未在 public API 中建模。
4. 资源抽象缺口：`struct picoui_font`、`picoui_label_set_font`、`picoui_text_set_font` 未落地（当前仅有 `image_source` 最小占位）。
5. 验收标准第 1 条（新用户仅靠 PicoUI 文档与 demo 即可完成完整页面搭建）当前证据不足。

---

## 18. 现阶段建议验收口径

为避免“接口齐了就算完成”的误判，建议把当前状态定性为：

- **P1（第一阶段）完成度：约 60%**
- **状态标签：`Skeleton Ready / Behavior Not Ready`**

建议把“已完成”与“已闭环”拆开：

1. **已完成（结构层）**：目录、头文件、最小 API、测试骨架、文档入口。
2. **未闭环（行为层）**：backend 映射、事件链路、theme 下发、demo 真实运行。

---

## 19. 下一步工作（按优先级）

### P0：先把“抽象层真实成立”做实

1. **backend 映射实装**：逐个控件把 `create/set/layout/event` 映射到 `LingDongGUI` 实体，不再只停留在本地占位结构体。
2. **事件链路改造**：回调触发来源从“setter 触发”改为“底层事件上送”；补齐 `button clicked` 对外接口。
3. **布局行为闭环**：让 `flex/grid` setter 真实驱动窗口布局，并补至少 1 条行为级测试（不是只测返回码）。

### P1：补齐设计 contract 缺口

1. 补齐通用 widget API：`set_text`、`set_style_class`、`set_user_data`。
2. 补齐 theme v0 常用样式接口：`bg/text/border/radius/padding`。
3. 引入最小 `state/part` public 词汇并建立映射策略。
4. 补齐 font 资源句柄与 `label/text` 字体接口。

### P2：统一 demo 与文档交付

1. 为 6 个 PicoUI demo 建立统一可执行目标（至少可批量 build）。
2. 增加 demo 运行级检查（不仅构建成功，还要最小运行返回）。
3. 在 `picoui/docs` 增加“当前已实现 vs 规划能力”矩阵，明确已知边界，避免过度承诺。

---

## 20. 建议的近期里程碑重排

为保证每一步都有可验证证据，建议把后续里程碑调整为：

1. **M1.5（行为闭环里程碑）**：button/switch/slider + flex/grid 完成真实 backend 映射，新增行为测试。
2. **M2（contract 补齐里程碑）**：补齐通用 widget/style/state/part/font 缺口并完成 contract 测试。
3. **M3（交付里程碑）**：6 demo 统一可执行 + 文档能力矩阵 + 用户 onboarding 路径验收。

---

## 21. 实施结果回写（2026-05-26，当日基于 subagent 并行落地）

### 21.1 本轮已完成（对照 17.3 / 17.4）

1. **通用 widget API 缺口已补齐**
   - 已补：`picoui_widget_set_text`、`picoui_widget_set_style_class`、`picoui_widget_set_user_data`
   - 已补：`picoui_widget_set_bg_color`、`picoui_widget_set_text_color`、`picoui_widget_set_border_color`、`picoui_widget_set_radius`、`picoui_widget_set_padding`
2. **button 事件接口已补齐**
   - 已补：`picoui_button_set_on_clicked`
3. **theme 统一语义词汇已落地**
   - 已补：最小 `PICOUI_STATE_*`、`PICOUI_PART_*` public 枚举
4. **font 资源抽象已补齐最小 contract**
   - 已补：`struct picoui_font`、`picoui_label_set_font`、`picoui_text_set_font`
5. **backend 从纯 no-op 升级为“可承接状态”的实现**
   - layout/window/child setter 已有 backend 状态落点
   - style/font/user_data/image_source/theme 已有 backend 承接路径
   - 新增 clicked 统一触发入口 `picoui_backend_emit_clicked`
6. **demo 可执行闭环已补齐**
   - 6 个 PicoUI demo 均有 `main`，并在 SDL CMake 下注册独立 target
   - runtime 检查从单 target 扩展为批量构建 6 个 demo target

### 21.2 验证证据（本轮）

1. `ctest --test-dir build -L picoui --output-on-failure`：通过（7/7）
2. `tests/picoui/runtime/check_picoui_runtime.py`：已覆盖 6 个 demo target 构建
3. `tests/picoui/contract/check_picoui_demo_boundary.py`：通过
4. GitNexus `detect_changes(scope=all)`：`risk_level=low`，`affected_count=0`

### 21.3 当前剩余工作（进入下一轮）

按 17.3 / 17.4 的“未完成项”口径，本轮已全部补齐；后续属于增强项而非本轮阻塞项：

1. 可将 bridge 从“测试显式绑定”进一步演进为“真实 widget 创建链路自动绑定”。
2. 可把当前 signal 级桥接从 `VALUE_CHANGED` 扩展到更多事件类型（如 clicked/pressed/released）。
3. 可继续增加更细粒度的行为级/可视化级测试矩阵，提升回归覆盖深度。

### 21.4 本轮增量：signal 级事件桥接

1. `switch / checkbox / slider` 的值变化路径已统一走 backend dispatch，不再在 widget setter 内直接 emit。
2. backend 增加了最小 `signal` 记录能力，当前至少覆盖 `PICOUI_BACKEND_SIGNAL_VALUE_CHANGED`。
3. 新增测试证据可直接观察 `dispatch_count` 与 `last_signal`，并验证同值 setter 不会重复触发。

### 21.5 本轮增量：theme/state/part contract 补齐

1. `PICOUI_STATE_*` 已补齐到最小词汇表，且保留既有枚举值兼容性。
2. `PICOUI_PART_*` 已补齐到最小词汇表。
3. `picoui_theme_create()` 已从全零默认值切换为 theme v0 默认 token。
4. `tests/picoui/unit/test_picoui_theme.c` 已补充默认 token 非零与 `set_color` / `set_metric` 覆盖行为验证。

### 21.6 本轮增量：真实消息总线桥接闭环

1. backend widget 已补最小可选 bridge 绑定，支持 `scene + sender` 的内部消息总线对接。
2. `value_changed` 路径在 bridge 已绑定且 `scene->ptMsgQueue` 存在时，会额外调用 `ldMsgEmit(..., SIGNAL_VALUE_CHANGED, value)`。
3. 现有 callback 路径保持不变；无 bridge 时行为与原实现一致。
4. 单测已补真实队列收包证据：`ld_scene_t + ldMsgInit queue`、绑定 bridge、触发 `value_changed`、`xQueueDequeue` 校验 signal/value。

---

## 22. 三次 Review 复盘（2026-05-26，主线程 + subagent 复核）

> 本节用于回答“`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md` 是否已全部开发完成”。
>
> 文档口径修正：以下 `22.x` 内容属于 2026-05-26 当日的阶段性 review 复盘快照，不再代表当前最终完成判定。当前主线真相以第 15.1 节能力矩阵、`docs/picoui-serial/A-线计划索引.md` 的 `A7` 收口状态，以及最新 Stage G 门禁复验结果为准。
>
> 现态补充（A7 final closeout）：`backend_app.c` 当前仍保留 `temporary smoke path` / host harness 职责，但已不再承担正式 fake renderer 主输出职责；`runtime/capture` 仍只属于 smoke / 回归证据，不能单独上升为最终 UI 证据。

### 22.1 审计范围与证据

本轮对照了：

- 设计文档条目：范围（第 3 节）、API（第 6 节）、主题语义（第 7 节）、事件（第 9 节）、资源（第 10 节）、demo（第 11 节）、验收标准（第 15 节）。
- 代码与头文件：`picoui/include/picoui/*.h`、`picoui/src/**/*`、`picoui/demo/*/main.c`、`examples/sdl/CMakeLists.txt`。
- 测试：`tests/picoui/{unit,contract,runtime}`。
- 运行证据：`rtk ctest --test-dir build -L picoui --output-on-failure`（本轮复验 8/8 通过）。

### 22.2 总结论

> 历史结论（已过时）：以下结论仅描述 2026-05-26 当次复盘时点，不代表当前现态。

**截至该次复盘时点，尚未“全部开发完成”。**

说明：

1. 按 17.3 / 17.4 缺口口径，本轮确实补齐了当时列出的主要未完成项。
2. 但按整份设计文档全文口径与第 15 节验收标准核对，仍存在若干“接口存在但行为未闭环”或“设计项未落地”的差距。
3. 因此该次复盘时点的状态更准确应为：**`Contract Mostly Ready / Behavior Partially Ready`**。

> A7 closeout 更新：上述否定性结论已被后续 `A1-A7` 收口覆盖；当前口径应视为“第一阶段按既定边界已收口，但 `backend_app.c` 仍保留 temporary smoke path / host harness，runtime/capture 仍仅是 smoke 证据”。

### 22.3 已完成项（本轮可确认）

1. 6 个 PicoUI demo 构建目标齐全，并已接入 runtime 构建检查（`picoui_*_demo` 全部纳入）。
2. `PICOUI_STATE_*` / `PICOUI_PART_*` 已在 public API 建模。
3. `picoui_button_set_on_clicked`、通用 widget 文本/样式/用户数据与常用样式快捷接口已补齐。
4. `struct picoui_font`、`picoui_label_set_font`、`picoui_text_set_font` 已提供最小 contract。
5. `ctest -L picoui` 当前链路可运行，但其 runtime 子项依赖独立 build tree 与 SDL demo 构建链，文档不能把它简化成“纯 PicoUI、自身完全独立”的闭环。

### 22.4 未完成或部分完成项（阻塞“全部完成”判定）

> 历史清单（已过时）：以下条目是 2026-05-26 当次 review 时用于阻塞“全部完成”判定的缺口清单；其中与 `A1-A7` closeout 直接相关的项，已由后续实现、门禁复验和索引回写覆盖，不应再被读取为当前仍然成立的现态阻塞项。当前若需判断最终状态，应回到第 15.1 节能力矩阵与 A 线索引。

1. **checkbox 文本 API 与文档不一致**
   - 第 6.5 节声明了 `picoui_checkbox_set_text`，当前头文件/实现未提供该接口。
2. **事件词汇未全落地**
   - 第 9.2 节包含 `pressed/released`，当前 public API 与 backend signal 仅实装到 `value_changed` 主路径，缺少 `pressed/released` 对外 contract 与测试。
3. **state/part 仅有词汇，缺应用语义闭环**
   - 当前主要是枚举存在，尚未形成面向控件的 `part/state` 样式应用入口与验证证据。
4. **font / style_class / user_data 的 backend 下发链路不完整**
   - `label/text` 的字体设置目前主要停留在 PicoUI 本地字段写入，未完整串联到通用 backend 同步路径。
   - style_class / user_data 也以本地状态承接为主，缺“统一 setter -> backend 同步 -> 行为断言”的闭环证据。
5. **layout 行为证据深度不足**
   - `flex/grid` 的 API 可调用，但 `new_track` / `ignore_layout` / `grid_align` 等能力缺行为级断言，当前以状态承接测试为主。
6. **demo 教学覆盖与第 11.2 节存在差距**
   - `basic_widgets` 现状未覆盖文档声明的完整控件集合（例如 `button/image/text` 教学示例不足）。
7. **测试与入口文档仍有表述偏差**
   - 根 `CMakeLists.txt` 是当前推荐入口，但默认 `LD_BUILD_SDL_DEMO=ON`，所以默认构建仍与 SDL 子树耦合。
   - runtime 只是 label 分层，不是默认排除。
   - `picoui/docs/demo_guide.md` 仍把 `examples/sdl` 子目录 configure 描述为主路径，这与当前推荐入口不一致。

### 22.5 下一步工作（按优先级）

#### P0：先补“文档直连缺口 + 事件闭环”

1. 补 `picoui_checkbox_set_text`（header + impl + unit test + contract 用例）。
2. 增补 `pressed/released` 最小 public API（建议先从 button 起步），并补 backend signal 与测试。
3. 在文档中明确：哪些事件当前是“setter 触发语义”，哪些是“底层上送语义”。

#### P1：补“行为闭环证据”

1. 为 `picoui_widget_set_flex_new_track`、`picoui_widget_set_ignore_layout`、`picoui_grid_set_align` 增加行为级断言。
2. 完善字体/样式/user_data 的 backend 同步链路，并补对应断言。
3. 为 `state/part` 增加最小应用接口（内部或半公开均可），至少覆盖 `button/checkbox/switch/slider`。

#### P2：补“demo 与验收可视证据”

1. 对齐 `basic_widgets` 与第 11.2 节目标示例内容。
2. 在 runtime 检查中增加最小运行 smoke（不仅 build 成功，还验证可启动/退出码）。
3. 在 `picoui/docs` 增加“已实现能力矩阵（已完成 / 部分完成 / 规划）”防止过度承诺。

### 22.6 更新后的完成判定门禁（建议）

只有同时满足以下条件，才可把“第一阶段全部开发完成”标为 true：

1. 第 6/7/9/10/11 节所有显式 API 与示例 contract 在代码中有对应实现。
2. 关键行为项（layout、事件、theme part/state 应用）具备行为级测试证据，而非仅返回码/结构体字段断言。
3. `ctest -L picoui` 全通过，且 runtime 至少包含最小启动级 smoke。
4. 文档能力矩阵与代码现状一致，不存在“文档声明已完成、代码仅部分实现”的偏差。

---

## 23. 基于 22.5 的实现进展（2026-05-26，subagent 并行落地）

### 23.1 本轮已完成（对照 22.5 的 P0）

1. **`picoui_checkbox_set_text` 已落地**
   - 已补 public API：`picoui/include/picoui/checkbox.h`
   - 已补实现：`picoui/src/widgets/checkbox.c`
   - 已补 props 文本初始化：`picoui_checkbox_create_with_props` 支持 `text`
2. **`pressed/released` 最小事件 contract 已落地（button）**
   - 已补 public API：`picoui_button_set_on_pressed`、`picoui_button_set_on_released`
   - 已补 backend signal：`PICOUI_BACKEND_SIGNAL_PRESSED`、`PICOUI_BACKEND_SIGNAL_RELEASED`
   - 已补 dispatch 入口：`picoui_backend_widget_dispatch_event(...)`
3. **单测闭环已补**
   - 新增：`tests/picoui/unit/test_picoui_button_events.c`
   - 既有：`tests/picoui/unit/test_picoui_widgets.c` 已补 checkbox 文本路径断言
   - 测试接线：`tests/picoui/CMakeLists.txt` 新增 `test_picoui_button_events`

### 23.2 本轮验证证据

1. `rtk cmake --build build --target test_picoui_smoke test_picoui_theme test_picoui_widgets test_picoui_button_events test_picoui_layout picoui_hello_world_demo picoui_basic_widgets_demo picoui_layout_flex_demo picoui_layout_grid_demo picoui_theme_showcase_demo picoui_settings_panel_demo`：通过。
2. `rtk ctest --test-dir build -L picoui --output-on-failure`：通过（8/8）。
3. GitNexus `detect_changes(scope=all)`：`risk_level=low`，`affected_count=0`（当前改动范围未扩散到已建模流程）。

### 23.3 更新后的剩余工作（进入下一轮）

> 历史快照（已过时）：以下条目是 2026-05-26 当次 review 结束时，准备进入下一轮时记录的剩余工作清单，对应当时的 `22.4` 阻塞项；它们用于解释那一刻为什么尚不能判定“全部开发完成”，但不再代表当前 A 线 final closeout 的现态 blocker。`A7` 后续收口、门禁复验与索引回写完成后，当前最终状态应以第 `15.1` 节能力矩阵、`docs/picoui-serial/A-线计划索引.md` 的 `A7` 收口状态，以及最新 Stage G 门禁结果为准。
>
> 当时记录的剩余工作如下：
>
> 1. `state/part` 从“仅词汇”升级为“可应用语义”的最小闭环（至少覆盖 button/checkbox/switch/slider）。
> 2. `font/style_class/user_data` 的 backend 同步链路与行为级断言补齐。
> 3. runtime 仍缺“最小启动级 smoke”证据（当前 runtime 以构建检查为主，尚未加入启动/退出码验证）。

### 23.4 本轮增量进展（2026-05-26，继续并行推进）

1. **layout 行为级证据已补强**
   - `tests/picoui/unit/test_picoui_layout.c` 已新增对 `picoui_widget_set_flex_new_track`、`picoui_widget_set_ignore_layout`、`picoui_grid_set_align` 的行为断言。
   - 断言同时覆盖 PicoUI 前端状态与 backend 状态承接字段，减少“仅返回码通过”的弱证据。
2. **`basic_widgets` demo 覆盖已对齐**
   - `picoui/demo/basic_widgets/main.c` 在原有 `switch/checkbox/slider` 之外，新增了 `button/text/image` 的创建与最小设置调用。
   - 保持 demo 边界约束：未泄漏 `ld*` / `arm_2d_*` / `SIGNAL_*`。
3. **验证结论**
   - `rtk cmake --build build --target test_picoui_layout picoui_basic_widgets_demo`：通过。
   - `rtk ctest --test-dir build -L picoui --output-on-failure`：通过（8/8）。
   - GitNexus `detect_changes(scope=all)`：`risk_level=low`，`affected_count=0`。

### 23.5 收敛更新（2026-05-26，继续并行推进）

1. **state/part 最小应用语义已落地**
   - 新增 public API：`picoui_theme_apply_to_widget(theme, widget, part, state)`。
   - 已补 widget kind 与 part 的最小匹配策略（button/checkbox/switch/slider）。
   - 已补 part/state 到样式字段（`bg_color/text_color/border_color`）的可重复映射。
2. **font/style_class/user_data backend 同步链路已闭环**
   - `picoui_widget_set_style_class`、`picoui_widget_set_user_data` 已在 core setter 内同步 backend。
   - `picoui_label_set_font`、`picoui_text_set_font` 已同步 backend font 字段。
   - `test_picoui_widgets` 已补 backend 字段断言（style_class/user_data/font）。
3. **runtime 最小启动级 smoke 已落地**
   - `tests/picoui/runtime/check_picoui_runtime.py` 由“仅构建”升级为“构建 + 逐个启动 6 个 demo + 退出码校验 + 超时保护”。
4. **复验证据**
   - `rtk cmake --build build --target test_picoui_smoke test_picoui_theme test_picoui_widgets test_picoui_button_events test_picoui_layout picoui_hello_world_demo picoui_basic_widgets_demo picoui_layout_flex_demo picoui_layout_grid_demo picoui_theme_showcase_demo picoui_settings_panel_demo`：通过。
   - `rtk ctest --test-dir build -L picoui --output-on-failure`：通过（8/8，runtime 启动级 smoke 纳入通过）。
   - GitNexus `detect_changes(scope=all)`：`risk_level=low`，`affected_count=0`。

### 23.5A 当前文档口径修正（2026-05-26，基于主线真相）

1. **runtime 的真实含义**
   - 当前 runtime 已具备“构建 + 启动”检查能力，这一点是已完成项。
   - 但 runtime 仍通过 `build/picoui-runtime` 独立 build tree 执行，不是复用常规 `build/`。
2. **入口的真实含义**
   - 当前推荐 configure/build 入口是仓库根 `CMakeLists.txt`。
   - 但由于 `LD_BUILD_SDL_DEMO=ON` 默认开启，根入口默认仍会进入 SDL 子树，不能写成“根测试入口已与 SDL 解耦”。
3. **分层的真实含义**
   - `tests/picoui/runtime` 与 `ctest -L runtime` 说明 runtime 已分层。
   - 这只代表“可以按 label 单独执行”，不代表 runtime 默认不注册或默认被排除。
4. **demo 文档的真实含义**
   - `picoui/docs/demo_guide.md` 当前仍保留 `cmake -S examples/sdl -B examples/sdl/build` 作为主要示例。
   - 该文件现有用户脏改未提交，因此本轮只在上层文档中提示偏差，不直接修改该文件。

### 23.6 当前判定（对照 22.6 门禁）

> 历史快照（已过时）：
> 本小节记录的是 `2026-05-26` 当次 review 对照 `22.6` 门禁时的阶段性判定，不再代表当前 `A线` final closeout 的现态结论。当前应以 `15.1` 能力矩阵、`docs/picoui-serial/A-线计划索引.md` 的 `A7` 收口状态，以及最新 `Stage G` fresh gate 为准。

按第 22.6 的 4 条门禁逐项核对：

1. 第 6/7/9/10/11 节中的主要 contract 缺口已有代码与测试证据。  
2. 关键行为项已不再只依赖返回码，runtime 也已包含最小启动级 smoke。  
3. 但 `ctest -L picoui` 的 runtime 仍依赖独立 build tree 与 SDL demo 构建链。  
4. 文档层面仍存在入口与测试分层表述偏差，需要像本轮这样显式更正。  

**历史结论（已过时）**：代码与测试链路当时虽已明显收敛，但文档口径尚未完全对齐，所以那一时点还不能把状态写成“全部完成且文档已完全对齐”。

**A7 closeout 更新**：上述文档对齐缺口已在后续 `A7` 收口中补齐。当前可以成立的口径是：代码主线、fresh gate 与文档真相源已按最新边界重新对齐；但 `runtime/capture` 仍只应被读取为 smoke / 回归证据，`backend_app.c` 也仍保留 `temporary smoke path` / host harness 职责，而不是“已经完全消失”。

---

## 24. A线纠偏结论（2026-05-26）

> 本节是当前 `PicoUI` 主线的纠偏真相源，用于回答“为什么现在能弹窗但 UI 效果仍不对，以及后续应该按什么方向继续开发”。

### 24.1 当前问题不是“SDL 不支持 PicoUI”

当前问题更准确应表述为：

- `SDL` 作为宿主层本身没有问题；
- `PicoUI` 当前已经能构建、能启动、能在 SDL 窗口内输出画面；
- 当前主输出链已经是 `ldGuiFrameStart() -> ldMsgProcess() -> ldGuiDraw() -> ldGuiFrameComplete()` 的真实 `LingDongGUI` 绘制路径；
- `picoui/src/backend/ldgui/backend_app.c` 仍然存在，但当前口径应明确为 `temporary smoke path` / host harness：它还负责 SDL host、capture、marker、placeholder 等过渡职责，但不再承担正式 fake renderer 主输出职责；
- 因此，“窗口起来了”依然不等于“单靠 runtime/capture 就能证明最终 UI 完成”；真正成立的是：`PicoUI -> LingDongGUI` 真实 backend 主线已经建起来，而 `runtime/capture` 继续只承担 smoke / 回归证据角色。

### 24.2 当前错误方向

当前主线里以下现象应被明确视为**错误方向**，不能继续扩展：

1. `backend_app.c` 用固定坐标、固定行高、固定轨道长度、固定按钮宽度、固定占位线条来“画出像控件的东西”。
2. `picoui/demo/*/main.c` 通过补 `set_size()`、`set_pos()` 或其他人工摆位来配合 fake renderer。
3. runtime/capture 测试把“非空画面”错误地外推成“backend 已闭环”。

### 24.3 正确方向

`PicoUI` 后续必须回到以下正确分层：

- `PicoUI public API`
- `PicoUI backend/ldgui`
- `LingDongGUI` 真实控件/布局/事件/runtime
- `SDL host`

也就是说：

1. `PicoUI` 负责统一 API、状态和用户入口；
2. backend 层负责把 `window/label/button/checkbox/switch/slider/text/image`、`flex/grid`、`theme/event` 映射到真实 `LingDongGUI` 对象；
3. `LingDongGUI` 负责真实布局、绘制、消息与交互；
4. SDL 只负责宿主显示，不再为 `PicoUI` 单独维护一套视觉系统。

### 24.4 A线当前口径

从本节开始，`PicoUI` 的当前主线统一记为 **A线**，其正式目标不是“继续把 fake renderer 修漂亮”，而是：

- **A线目标**：把 `PicoUI` 从“临时可视 smoke”纠偏成“真实 `PicoUI -> LingDongGUI` backend 映射主线”。

### 24.5 A线当前禁止项

后续开发中，以下行为默认禁止：

1. 继续在 `picoui/src/backend/ldgui/backend_app.c` 中新增固定坐标、固定尺寸、手工 line/rect/circle 控件画法。
2. 继续在 `picoui/demo/*/main.c` 里增加为了“看起来像 UI”而写的硬编码尺寸/位置补丁。
3. 继续把 “能弹窗 / 有 capture / 画面非空” 当成 “backend 映射已完成”。
4. 借着修 demo 外观，顺手把 `LingDongGUI` 渲染链重写成 `PicoUI` 专属实现。

### 24.6 A线正式入口

后续关于 `PicoUI` 当前该做什么、已做到哪一阶段、哪些不能做，统一以：

- `docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `docs/superpowers/plans/2026-05-26-picoui-abstraction-layer-implementation.md`
- `docs/picoui-serial/A-线计划索引.md`

三者为真相源。

其中：

- 本 `spec` 负责解释设计目标与纠偏原则；
- `implementation plan` 负责阶段拆分与实施路径；
- `A-线计划索引` 负责“当前游标在哪、下一步做什么、哪些不做”的快速导航。
