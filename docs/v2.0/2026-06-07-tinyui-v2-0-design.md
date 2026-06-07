# TinyUI v2.0 设计文档

> 面向后续 agent 执行：本设计文档锁定 `TinyUI` 的命名、架构边界、第一批最小子集与代码迁移原则。实现前必须先基于本文再拆详细 plan，不得回退到 `PicoUI backend` 或改造 `LingDongGUI` 既有 public API。

## 1. 目标

`TinyUI v2.0` 的目标不是做第二个 `LVGL`，也不是把 `LingDongGUI` 全量 fork 一份重命名，而是在保持 `LingDongGUI` 轻量内核的前提下，提供一套更现代、更直接、使用方式更接近 `LVGL` 的新 public API。

第一阶段只做最小可用子集，证明以下几点：

- `TinyUI` 可以作为新的 public API 入口独立存在
- `TinyUI` 不依赖 `PicoUI -> backend/ldgui` 三层桥接模型
- `TinyUI` 可以直接驱动 `LingDongGUI` 现有控件能力
- 用户使用方式可以收敛到 `init/display/indev/screen/timer_handler/obj` 这套轻量模型

## 2. 非目标

本阶段明确不做以下事情：

- 不把 `LingDongGUI` 改造成 `TinyUI`
- 不修改 `LingDongGUI` 既有 public API 命名
- 不调整 `LingDongGUI` 现有目录结构
- 不保留 `PicoUI` 的 `app` 模型
- 不保留 `PicoUI` 的 `backend/ldgui` 目录和抽象
- 不追求 `LVGL` 那种大而全对象系统、style stack、selector/state 组合系统
- 不在第一阶段实现全量控件、复杂布局系统、复杂主题系统

`LingDongGUI` 在本路线中的定位是内核/引擎真相源，不是需要被整体替换或整体重写的旧 API。

## 3. 命名与品牌

本路线最终命名固定如下：

- 产品名：`TinyUI`
- 目录名：`tinyui/`
- API 前缀：`tinyui_*`

以下命名不采用：

- `TinyGUI`
- `TUI`
- `VenusUI`
- `NanoUI`
- `nu_*`
- `tui_*`

主要原因是：

- `TinyUI` 足够短，商业名称更自然
- `tinyui_*` 不会与 `TUI = Text User Interface` 产生歧义
- `TinyUI` 更适合作为新 public 面名称

## 4. 总体架构

`TinyUI` 采用“新 public 面 + 现有轻量内核”的结构：

- `TinyUI` 负责 public API、对象模型、runtime、screen、基础控件、最小事件桥
- `LingDongGUI` 负责底层真实控件、场景、绘制、事件分发、ARM-2D 集成

运行链路固定为：

`TinyUI public API -> TinyUI core/widgets -> tinyui/internal/ld_native.h -> LingDongGUI ld*`

不允许出现以下链路：

- `TinyUI -> backend -> ldgui`
- `TinyUI -> app -> backend -> ldgui`
- `TinyUI -> duplicated LingDongGUI full fork`

### 4.1 保留的能力

保留并复用 `LingDongGUI` 已有轻量优势：

- 现有控件真实行为
- 现有 scene/runtime 绘制主链
- 现有 ARM-2D 集成
- 现有基础事件信号语义

### 4.2 不复制的复杂度

`TinyUI` 只学习 `LVGL` 的外部使用方式，不复制 `LVGL` 的复杂体量。明确不引入：

- 复杂 class/type hierarchy
- 多层 style stack
- selector/state 组合样式系统
- 通用 property 反射系统
- 纯为抽象而抽象的中间 backend 层

## 5. `LingDongGUI` 边界

本路线对 `LingDongGUI` 的约束固定如下：

- 不修改 `LingDongGUI` 现有 public API 名称
- 不修改 `LingDongGUI` 现有目录结构
- 不做品牌化改名前缀工程
- 允许为了 `TinyUI` 落地补充能力
- 若确需修改 `LingDongGUI`，必须是“补能力/修内核缺口”，不能是“适配层命名工程”

这意味着：

- `TinyUI` 永远是新 public 面
- `LingDongGUI` 永远是内核真相源
- `TinyUI` 的存在不改变 `LingDongGUI` 作为独立库的既有外观

## 6. `TinyUI` public model

`TinyUI` public model 明确采用接近 `LVGL` 的全局单例 runtime，不暴露 `app` 对象。

### 6.1 必须具备的入口

第一阶段 public runtime 入口至少包括：

- `tinyui_init()`
- `tinyui_deinit()`
- `tinyui_display_create(...)`
- `tinyui_indev_create(...)`
- `tinyui_screen_create()`
- `tinyui_screen_load(tinyui_obj_t *screen)`
- `tinyui_timer_handler()`

### 6.2 明确禁止的模型

禁止出现：

- `tinyui_app_create()`
- `tinyui_app_run()`
- `tinyui_app_*`
- 任何需要用户显式持有 runtime/app/context 对象的主模型

如果内部需要 runtime 状态，必须放在 `TinyUI` 私有 runtime 中，不暴露给用户。

## 7. 对象模型

`TinyUI` 采用统一对象句柄模型：

- 所有 public 对象统一暴露为 `tinyui_obj_t *`
- widget-specific API 通过 create/set/get/callback 访问
- 不对外暴露 `ldBase_t *`、`ldSwitch_t *` 等 `LingDongGUI` 内核类型

这样做的原因：

- 降低 public surface
- 贴近 `LVGL` 用户习惯
- 避免把 `LingDongGUI` 内部结构泄漏到新 API

## 8. 样式与布局边界

### 8.1 样式系统

第一阶段只保留轻量样式模型：

- 直接 setter
- 最多一个全局 theme

明确不做：

- 多层 style stack
- selector/state 样式组合
- 大量抽象化 style property 反射

### 8.2 布局系统

第一阶段不做完整 `flex/grid`。第一阶段布局能力仅限：

- `screen/container`
- `tinyui_obj_set_pos(...)`
- `tinyui_obj_set_size(...)`
- 简单 parent-child

`flex/grid` 明确延期到第二批。

## 9. 第一阶段最小可用子集

第一阶段只做最小可用集合，具体范围固定如下：

### 9.1 核心

- runtime init/deinit
- display create
- indev create
- screen create/load
- timer handler
- `tinyui_obj_t`
- parent-child 绑定
- pos/size 基础操作

### 9.2 基础控件

- `label`
- `button`
- `switch`

### 9.3 demo

- `basic_widgets`

第一阶段 demo 只需证明：

- screen 能创建并加载
- label 能显示
- button 能显示并触发回调
- switch 能显示、切换并触发回调
- pos/size 生效
- timer loop 能稳定驱动界面

### 9.4 第二阶段再做

以下内容推迟到第二批：

- `image`
- `slider`
- `checkbox`
- `flex/grid`
- 更复杂控件
- 更复杂主题系统

## 10. 目录结构

`TinyUI` 第一阶段目录结构固定如下：

- `tinyui/include/tinyui.h`
- `tinyui/include/core.h`
- `tinyui/include/display.h`
- `tinyui/include/indev.h`
- `tinyui/include/obj.h`
- `tinyui/include/screen.h`
- `tinyui/include/label.h`
- `tinyui/include/button.h`
- `tinyui/include/switch.h`
- `tinyui/internal/ld_native.h`
- `tinyui/internal/runtime.h`
- `tinyui/src/core/tinyui_runtime.c`
- `tinyui/src/core/tinyui_obj.c`
- `tinyui/src/core/tinyui_event.c`
- `tinyui/src/core/tinyui_screen.c`
- `tinyui/src/widgets/tinyui_label.c`
- `tinyui/src/widgets/tinyui_button.c`
- `tinyui/src/widgets/tinyui_switch.c`
- `tinyui/src/port/tinyui_port_sdl.c`
- `tinyui/demo/basic_widgets/main.c`

额外规则：

- `include/` 下不再嵌套一层 `tinyui/`
- `internal/` 只允许 `TinyUI` 自己使用
- 不新建 `backend/`
- 不新建 `app/`

## 11. `switch` 的实现策略

`switch` 是第一阶段的代表性控件，设计上明确采用“直接吸收 backend 胶水，不保留 backend 架构”的做法。

判断依据：

- `src/gui/ldSwitch.c` 中已经有真实控件行为：按压、释放、状态切换、动画、绘制、`SIGNAL_VALUE_CHANGED`
- `picoui/src/backend/ldgui/backend_switch.c` 主要是创建 `ldSwitch`、挂父节点、分配 `name_id`、转发 setter
- `picoui/src/widgets/switch.c` 主要是 public wrapper

因此 `TinyUI switch` 的实现方式固定如下：

- `tinyui/src/widgets/tinyui_switch.c` 直接成为 public widget 实现层
- 创建时直接访问 `TinyUI` 私有 runtime 里的 `scene/root/next_name_id`
- 通过 `tinyui/internal/ld_native.h` 直接调用 `ldSwitch_init(...)`
- setter/getter 直接调用或读取 `ldSwitch*`
- 事件通过 `TinyUI` 轻量事件桥把 `SIGNAL_VALUE_CHANGED` 转成 `tinyui` 回调

最终链路必须是：

`tinyui_switch_* -> ldSwitch*`

中间不允许再插入 `backend widget/backend app state`

## 12. 代码 copy 与复用策略

### 12.1 总原则

第一阶段不从零硬写，也不复制整个 `LingDongGUI`。采用“按最小子集定向 copy，再改成直连 `ld*`”的策略。

### 12.2 可以复用的来源

可以复用的内容：

- `PicoUI` 的 public API 形状参考
- `PicoUI` demo 页面写法参考
- `PicoUI` 的部分对象包装思路
- `PicoUI` 测试/合同/可见效果基线
- `LingDongGUI` 的真实控件行为与底层能力

### 12.3 明确不复用的架构

不复用以下架构：

- `picoui/src/backend/ldgui/*`
- `picoui_app_*`
- `PicoUI` 的三层桥接模型
- `LingDongGUI` 全量内核 fork

### 12.4 第一阶段代码来源策略

#### `switch`

- 参考 `picoui/src/widgets/switch.c`
- 吸收 `picoui/src/backend/ldgui/backend_switch.c` 的创建/转发胶水
- 真实行为直接落到 `src/gui/ldSwitch.c`

#### `button`

- 参考对应 `PicoUI` public wrapper
- 直接改成 `TinyUI -> ldButton*`

#### `label`

- 参考对应 `PicoUI` public wrapper
- 直接改成 `TinyUI -> ldLabel*`

#### demo

- 参考 `picoui/demo/basic_widgets/main.c`
- 改写成 `tinyui_init/screen_load/timer_handler` 模型

## 13. 文档与阶段组织

`TinyUI` 文档归档到 `docs/v2.0/`，不混入旧 `PicoUI` serial 线。

本设计文档是 `TinyUI v2.0` 的设计真相源，后续需要：

- 再写一份 implementation plan
- implementation plan 细化到 subagent/agent 可直接执行
- 第一阶段完成后，再写 phase closeout / release-facing summary

## 14. 风险与约束

### 14.1 主要风险

- 直接调用 `ld*` 时，若 runtime/scene/name_id/事件桥边界不收紧，`TinyUI` 可能重新长出一套隐式 backend
- 如果第一阶段范围失控，容易又把 `flex/grid/theme/image/slider` 一起带入，失去“最小可用子集”的节奏
- 如果 `LingDongGUI` 改动不设边界，可能退化成“为了 `TinyUI` 重写 `LingDongGUI`”

### 14.2 控制策略

- 严守第一阶段最小范围
- `LingDongGUI` 只补能力，不改既有 public API 和目录
- `TinyUI` 私有桥接只允许收在 `tinyui/internal/ld_native.h`
- 不允许新增 `backend`
- 不允许新增 `app`

## 15. 验收标准

第一阶段完成时，至少需要满足以下标准：

- `TinyUI` 目录与头文件结构落地
- `TinyUI` 可独立编译最小 runtime + screen + label + button + switch
- `TinyUI basic_widgets demo` 可以运行
- demo 使用 `tinyui_*` API，不泄漏 `ld*`
- 整个 `TinyUI` 路径中不存在 `backend/ldgui`
- 整个 `TinyUI` public model 中不存在 `app`
- `LingDongGUI` 既有 public API 名称与目录结构保持不变

## 16. 结论

`TinyUI v2.0` 的正确路线是：

- 新建 `TinyUI` 作为新 public API
- 保留 `LingDongGUI` 作为轻量内核
- 第一阶段只做最小可用子集
- 定向 copy 需要的上层代码到 `tinyui/` 自己目录后修改
- 不保留 `backend`
- 不保留 `app`
- 不改 `LingDongGUI` 既有 public API 与目录

这是当前约束下工作量最小、风险最低、且最符合“比 `LVGL` 更轻量”的路线。
