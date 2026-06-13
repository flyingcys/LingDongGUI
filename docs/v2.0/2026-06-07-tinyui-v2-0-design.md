# TinyUI v2.0 设计文档

> 面向后续 agent 执行：本设计文档锁定 `TinyUI` 的命名、路线切换、架构边界与阶段推进方式。后续实现必须先基于本文拆详细 plan，不得回退到旧 `TINYUI backend` 路线，也不得把 `LingDongGUI` 改造成另一套重命名 public API。

## 1. 背景与设计结论

当前仓库已经在 `TINYUI` 上投入了大量真实工作，包括：

- 真实控件封装
- demo 页面
- unit/contract/runtime/visible gates
- `native-only` 路线的长期收口

因此，`TinyUI v2.0` 的最佳路线不是“从零新建一套最小子集目录重新开始”，也不是“整体 fork 一份 `LingDongGUI` 再重命名”，而是：

**以 `tinyui/` 为施工现场，在最大化复用既有代码、demo、测试和构建成果的前提下，分阶段重整结构，最终演化为 `TinyUI`。**

这条路线的核心价值是：

- 不重复造轮子
- 不放弃已经完成的大量 `TINYUI` 资产
- 可以逐步消灭最不想要的两层：`backend` 与 `app`
- 最终仍能收口到更轻、更直接、使用方式更接近 `LVGL` 的新产品形态

## 2. 目标

`TinyUI v2.0` 的目标不是做第二个 `LVGL`，也不是复制一整份轻量 GUI 内核，而是在保持 `LingDongGUI` 轻量内核优势的前提下，把当前 `TINYUI` 路线重整为：

- 更直接的实现路径
- 更薄的 runtime 模型
- 更轻的 public surface
- 更接近 `LVGL` 的用户使用方式
- 最终可对外收口为 `TinyUI`

本路线最终要实现以下结果：

- `TINYUI -> backend/ldgui -> LingDongGUI` 三层桥接被拆平
- `app` 模型退出主路径
- public API 最终演化为 `tinyui_*`
- `LingDongGUI` 继续作为轻量内核真相源
- demo、测试、合同、可见验证资产尽可能延续使用

## 3. 非目标

本路线明确不做以下事情：

- 不从零重写一套新 GUI
- 不整体 fork 一份 `LingDongGUI`
- 不修改 `LingDongGUI` 既有 public API 名称
- 不修改 `LingDongGUI` 既有目录结构
- 不追求 `LVGL` 那种大而全框架体量
- 不保留 `backend` 作为长期正式架构
- 不保留 `app` 作为长期正式 public model

## 4. 命名与终态定位

本路线终态命名固定如下：

- 产品名：`TinyUI`
- API 前缀：`tinyui_*`

以下命名不采用：

- `TinyGUI`
- `TUI`
- `VenusUI`
- `NanoUI`

当前阶段的关键变化是：

**`TinyUI` 不再被定义为一开始就独立目录起步的新实现，而是定义为“以 `tinyui/` 为施工现场、最终演化出的终态产品名”。**

也就是说：

- 短期施工目录可以仍在 `tinyui/`
- 长期对外结果必须收口到 `TinyUI`

## 5. 总体架构判断

### 5.1 当前问题不在“没有新目录”，而在“层次太多”

当前最主要的架构问题不是名字，而是这条链路太厚：

`public API -> app -> backend/ldgui -> ld*`

这条链路带来的问题包括：

- 运行时模型重
- 中间胶水层多
- 控件行为与 public wrapper 分散
- 迁移到新产品形态时路径过长

### 5.2 正确终态

最终终态应收敛为：

`public API -> widgets/core -> ld*`

也就是：

- public API 仍然存在
- object/runtime/screen/event 仍然存在
- 但 `backend/ldgui` 不再作为单独架构层存在
- `app` 不再作为 public 主模型存在

### 5.3 `LingDongGUI` 的角色

`LingDongGUI` 在本路线中的角色固定如下：

- 真实控件行为真相源
- scene/runtime/draw/event 内核真相源
- ARM-2D 轻量 GUI engine

它不是本路线要被品牌化重命名的对象，也不是要整体重构目录和 public API 的对象。

## 6. `LingDongGUI` 边界

本路线对 `LingDongGUI` 的约束固定如下：

- 不修改既有 public API 名称
- 不修改既有目录结构
- 不做为了 `TinyUI` 而进行的整体改名前缀工程
- 允许补充能力
- 允许修复内核缺口
- 若确需修改，只能是“补能力/修缺口”，不能是“为适配层服务的命名工程”

一句话：

`TinyUI` 和重整后的 `TINYUI` 使用 `LingDongGUI`，但不把 `LingDongGUI` 重新包装成另一套名字的 public 库。

## 7. 路线选择与最佳方案

当前实际可选路线有两条：

### 方案 A：新建 `tinyui/`，从 `TINYUI` 与 `LingDongGUI` 定向复制最小子集

优点：

- 终态干净
- 架构表达最清晰

缺点：

- 需要重复搬 demo、测试、构建、合同
- 前期重复工作明显
- 已有 `TINYUI` 资产利用率低

### 方案 B：以 `tinyui/` 为施工现场，逐步重整并最终演化为 `TinyUI`

优点：

- 最大化复用已有代码
- 最大化复用 demo、测试、gates、合同
- 先解决真正痛点：`backend` 与 `app`
- 总工作量更低

缺点：

- 过渡期会存在旧名与新终态之间的阶段性混用
- 需要更严格的阶段边界来避免结构继续变乱

**本设计文档明确选择方案 B。**

## 8. 设计原则

### 8.1 学 `LVGL` 的地方

要学习 `LVGL` 的：

- `init`
- `display`
- `indev`
- `screen`
- `timer_handler`
- `obj` 统一对象模型

### 8.2 不学 `LVGL` 的地方

不复制 `LVGL` 的：

- 大规模 class/type hierarchy
- 庞大 style stack
- selector/state 组合机制
- 通用 property 反射系统
- 过度通用化抽象

`TinyUI` 的定位是比 `LVGL` 更轻，而不是成为另一套大而全框架。

### 8.3 不重复造轮子

能复用的地方必须优先复用：

- 现有 widget wrapper 逻辑
- 现有 demo 页面
- 现有 test/gate/contracts
- 现有 native-only runtime 经验

但复用的前提是：

- 复用成果
- 不复用错误架构

## 9. 终态 public model

终态 `TinyUI` public model 固定为接近 `LVGL` 的全局单例 runtime，不暴露 `app`。

终态 public 入口至少包括：

- `tinyui_init()`
- `tinyui_deinit()`
- `tinyui_display_create(...)`
- `tinyui_indev_create(...)`
- `tinyui_screen_create()`
- `tinyui_screen_load(...)`
- `tinyui_timer_handler()`

禁止作为终态 public 主路径的接口形态：

- `*_app_create()`
- `*_app_run()`
- `*_app_*`
- 任何要求用户长期持有 runtime/app/context 对象的模型

如果内部需要 runtime 状态，只能保留在私有 `core/runtime` 中。

## 10. 终态对象模型

终态对象模型采用统一句柄：

- 所有 public 对象统一暴露为 `tinyui_obj_t *`
- widget-specific API 通过 create/set/get/callback 访问
- 不对外暴露 `ldBase_t *`、`ldSwitch_t *` 之类的 `LingDongGUI` 类型

这条设计结论不会因为施工现场在 `tinyui/` 而改变。

## 11. 样式与布局边界

### 11.1 样式系统

终态保持轻量：

- 直接 setter
- 最多一个全局 theme

明确不做：

- 多层 style stack
- selector/state 样式组合
- 重型样式系统

### 11.2 布局系统

第一阶段不做完整 `flex/grid` 收口。第一阶段布局能力只要求：

- `screen/container`
- pos
- size
- 简单 parent-child

`flex/grid` 放到后续阶段，不作为 backend/app 退场的前置阻塞项。

## 12. 第一阶段目标：先拆平 `backend`

第一阶段不追求立刻完成全部 `TinyUI` 命名与目录切换，而是先完成最重要的结构重整：

**消灭 `tinyui/src/backend/ldgui` 作为独立架构层。**

第一阶段完成后，路径应变为：

`public API -> widgets/core -> ld*`

而不是：

`public API -> backend -> ld*`

### 12.1 第一阶段的核心动作

- 把 `backend/ldgui` 中每个 widget 对应的创建/转发胶水吸收到对应 widget 实现
- 把 `backend_app`、runtime/event bridge 中和全局运行时强相关的部分吸收到 `core`
- 保留 `port` 作为 display/input/tick/SDL 宿主入口
- 不把 `backend` 仅仅重命名或挪位置后继续长期保留

### 12.2 第一阶段先做的最小控件集合

为了控制风险，第一阶段先围绕最小可用子集做结构重整：

- `label`
- `button`
- `switch`
- `screen`
- runtime 主链

### 12.3 第一阶段 demo 范围

第一阶段 demo 只要求收敛 `basic_widgets` 这类最小证明用例，证明：

- screen 创建/切换可用
- label/button/switch 可用
- 回调路径可用
- timer loop 可用

### 12.4 第一阶段暂不要求

第一阶段暂不要求：

- 全量控件全部改完
- `flex/grid` 同步完成
- 复杂 theme 收口
- 全量 rename 到 `tinyui_*`
- 目录立即整体改名到 `tinyui/`

## 13. `switch` 的重整策略

`switch` 是第一阶段代表性控件，能够清楚说明如何拆掉 backend。

观察结果：

- `src/gui/ldSwitch.c` 已有真实控件行为：按压、释放、动画、状态切换、绘制、`SIGNAL_VALUE_CHANGED`
- `tinyui/src/backend/ldgui/backend_switch.c` 主要承担创建、挂父节点、分配 `name_id`、setter 转发等胶水职责
- `tinyui/src/widgets/switch.c` 主要是 public wrapper

因此第一阶段 `switch` 的正确改法是：

- 保留 public wrapper 这一层职责
- 把 `backend_switch.c` 里的创建/转发胶水并入对应 widget/core
- 让 `switch` 实现直接调用 `ldSwitch*`

最终 `switch` 路径应变成：

`public switch api -> switch widget impl -> ldSwitch*`

而不是：

`public switch api -> backend_switch -> ldSwitch*`

这条规则同样适用于 `button`、`label` 等基础控件。

## 14. 代码复用策略

### 14.1 可以直接复用的资产

可以继续直接复用：

- 现有 `TINYUI` widget wrapper 基础代码
- 现有 `TINYUI` demo 页面
- 现有 `TINYUI` test/gate/contract 体系
- 现有 `TINYUI` runtime/nativity 路线经验
- `LingDongGUI` 真实控件能力

### 14.2 不应复用为长期架构的部分

不应继续保留为长期正式架构：

- `tinyui/src/backend/ldgui/*`
- `tinyui_app_*` 主模型
- `public api -> app -> backend -> ld*` 这条链

### 14.3 复用原则

复用既有成果，但不复用多余层级。

也就是说：

- demo 可保留
- tests 可保留
- widget wrappers 可吸收和改写
- backend 不应继续保留为单独正式层

## 15. 目录与文件组织原则

本路线第一阶段不要求立刻整体重命名目录，但要求文件职责重整：

- `widgets/` 负责 widget public implementation
- `core/` 负责 runtime/object/event/screen 主链
- `port/` 负责 display/input/tick/SDL 宿主入口
- `backend/` 逐步退出

第一阶段的关键不是新建新树，而是让旧树的职责重新正确分布。

## 16. 阶段推进顺序

本路线的推进顺序固定如下：

### 阶段 1：拆平 `backend`

目标：

- `backend/ldgui` 不再作为独立架构层
- `widgets/core` 直接调 `ld*`

### 阶段 2：消灭 `app`

目标：

- public 主模型从 `app` 转向 `init/display/indev/screen/timer_handler`
- `app` 只保留兼容层或迁移残留语义，不能再是主路径

### 阶段 3：统一 public API

目标：

- 从 `tinyui_*` 演化到 `tinyui_*`
- 统一头文件与 public surface

### 阶段 4：性能与内存守门

目标：

- 用真实数据量化 `TinyUI` 相对 `LingDongGUI` 的额外开销
- 单独守门性能、速度、RAM、二进制体积
- 先拿到证据，再允许写最终 release-facing 结论

### 阶段 5：目录/文档/产品名收口

目标：

- 完成目录、头文件、文档、外部表述上的 `TinyUI` 收口

## 17. 风险与控制

### 17.1 风险

- 如果只挪文件、不改运行时职责，`backend` 会变相存活
- 如果只改名字、不改主模型，`app` 会继续成为主路径
- 如果不设阶段边界，`tinyui_*`、`tinyui_*`、`ld*` 会在过渡期混乱

### 17.2 控制策略

- 第一阶段先盯结构，不盯大规模 rename
- 先消灭 `backend`
- 再消灭 `app`
- 只有在结构正确之后，才进入 `tinyui_*` 收口
- `LingDongGUI` 只补能力，不改既有 public API 和目录

## 18. 验收标准

### 第一阶段验收

第一阶段完成时，至少满足：

- `backend/ldgui` 不再是正式独立架构层
- `label/button/switch` 至少一批基础控件已变成 `widgets/core -> ld*` 直连模式
- `basic_widgets` 一类最小 demo 仍可运行
- 现有 gates 能继续复用或平滑迁移

### 终态验收

终态完成时，至少满足：

- public 主模型不再依赖 `app`
- public API 收口到 `tinyui_*`
- 产品对外名称收口到 `TinyUI`
- `LingDongGUI` 既有 public API 名称与目录结构保持不变
- 已有性能、速度、RAM、二进制体积守门证据
- 整体仍然保持比 `LVGL` 更轻量的定位

## 19. 结论

`TinyUI v2.0` 的最佳方案已经确定：

- 不从零新建一套最小实现重新开始
- 不整体 fork 一份 `LingDongGUI`
- 以 `tinyui/` 为施工现场
- 优先复用既有代码、demo、测试、合同、构建成果
- 第一阶段先拆平 `backend`
- 第二阶段再消灭 `app`
- 第三阶段统一到 `tinyui_*`
- 第四阶段先完成性能/速度/RAM/体积守门
- 第五阶段完成目录、文档、产品名收口

这条路线在当前仓库实际投入与目标约束下，是复用率最高、返工最少、总体最优的方案。

## 20. Closeout Note

当 `backend` shared layer、`app`-free main path、`tinyui_*` pilot public surface 与 `P5` 性能守门都已经闭环后，`TinyUI v2.0` 的 completion truth 以：

- `docs/v2.0/v2.0-closeout.md`
- `docs/v2.0/v2.0-release-matrix.md`
- `docs/v2.0/v2.0-performance-baseline.md`

为准。

这一定义只说明 closeout/release-facing 真相源已经收口到上述证据文件，不额外声称 full rename、full direct binding 或 full memory proof。
