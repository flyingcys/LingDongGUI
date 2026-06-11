# TinyUI v2.1 设计文档

> 面向后续 agent 执行：本设计文档锁定 `TinyUI v2.1` 的目标、命名、目录、架构边界与阶段推进方式。后续实现必须先基于本文拆详细 plan，不得回退到 `PicoUI` 兼容过渡态，也不得把 `LingDongGUI` 改造成另一套重命名 engine。

## 1. 背景与设计结论

`TinyUI v2.0` 已经完成一条“过渡态收口”路线：

- `backend` shared architecture 被拆平
- `app` 退出了用户主路径
- `tinyui_*` 试点 public surface 已落地
- `P5/P6` 的性能守门与 closeout/release 真相已闭环

但 `v2.0` 的完成态刻意停在过渡边界：

- 仓库里仍然存在大量 `picoui/*`
- `picoui_*` 仍然是主要 public 名称
- 顶层产品目录仍然是 `picoui/`
- `backend_*.c` 历史文件仍在
- `demo/tests/contracts/CMake` 中仍存在大量 `picoui` 痕迹

因此，`TinyUI v2.1` 的目标不是继续做另一轮局部 alias，而是：

**在不修改 `LingDongGUI` 目录与 public API 的前提下，彻底消灭产品层 `PicoUI` 痕迹，把当前 `picoui/` 原地演化为真正唯一的 `tinyui/` 产品目录，并完成目录、命名、文件、API、demo、tests、contracts、CMake 的全量收口。**

## 2. 目标

`TinyUI v2.1` 的目标固定为：

- 顶层产品目录只保留 `tinyui/`
- `picoui/` 目录整体退场
- public API 只保留 `tinyui_*`
- 不再保留 `picoui_*` 兼容 API
- `backend` 独立目录彻底消失
- widget-specific backend 文件与 API 并回 `widgets/*`
- `demo/tests/contracts/CMake/文档标题` 的产品层公开痕迹统一收口到 `TinyUI`
- `LingDongGUI` 继续作为底层 engine truth，不改它的目录和既有公开命名

一句话：

**`v2.1` 的完成态不是“能用 tinyui”，而是“仓库产品层已经不再存在 PicoUI”。**

## 3. 非目标

本路线明确不做以下事情：

- 不修改 `LingDongGUI` 既有目录结构
- 不重命名 `LingDongGUI` 的 `ld*` public API
- 不把 `LingDongGUI` 品牌化成 `TinyUI`
- 不保留长期 `picoui_*` 兼容窗口
- 不继续维持 `tinyui/` 与 `picoui/` 双目录并行
- 不通过 alias/bridge 长期容忍产品层双命名
- 不把共享 runtime/theme/layout/display/indev 盲目塞进 `widgets/*`

## 4. 命名与终态定位

`TinyUI v2.1` 的终态命名固定如下：

- 产品名：`TinyUI`
- 顶层产品目录：`tinyui/`
- public API 前缀：`tinyui_*`
- public 头文件路径：`tinyui/include/*`
- 测试、contract、demo、CMake/CTest 命名：统一使用 `tinyui`

以下命名在终态中不再允许继续作为产品层公开痕迹存在：

- `PicoUI`
- `picoui/`
- `picoui_*`

这里的“公开痕迹”包括但不限于：

- 顶层目录名
- public header 路径
- public symbol 前缀
- demo 名称
- test 名称
- contract/checker 名称
- CMake target / CTest name
- `docs/v2.1` 的正式对外文档标题

## 5. `LingDongGUI` 边界

`LingDongGUI` 在 `v2.1` 中的角色固定如下：

- 真实 widget 行为真相源
- 真实 scene/runtime/draw/event 内核真相源
- ARM-2D 轻量 GUI engine

本路线对 `LingDongGUI` 的硬边界固定如下：

- 不修改既有目录结构
- 不修改既有 `ld*` public API 命名
- 不为了 `TinyUI` 做品牌化 rename 工程
- 允许补能力
- 允许修缺口
- 若确需修改，只能是“为真实能力闭环服务”，不能是“为产品层改名服务”

## 6. 架构判断

### 6.1 当前真实问题

当前产品层还残留三类问题：

- 目录名仍然停在 `picoui/`
- widget 能力仍有大量 `backend_*` 风格边界
- 公开命名、测试命名、合同命名没有真正收口

也就是说，`v2.0` 已经解决了“shared backend 与 app 主路径”问题，但没有解决“产品层品牌与文件组织的最终落地”问题。

### 6.2 `v2.1` 的正确终态

`v2.1` 的正确终态应收敛为：

`tinyui public API -> tinyui shared layers/widgets -> LingDongGUI ld*`

具体含义是：

- 不再存在独立 `backend/` 架构层
- widget-specific LingDongGUI 对接逻辑回到各自 widget 文件
- 共享 runtime/object/display/indev/layout/theme/tick 保留薄层
- `TinyUI` 作为单一产品层，对接 `LingDongGUI`

### 6.3 参考 LVGL 的地方

参考本仓库本地 `third_party/lv_port_pc_vscode/lvgl/src` 的组织方式，`v2.1` 应学习的是：

- `core`
- `display`
- `indev`
- `layouts`
- `themes`
- `tick`
- `widgets`

它不应继续保留“产品层 backend + widgets 双轨”。

### 6.4 不参考 LVGL 的地方

`v2.1` 不复制 `LVGL` 的：

- 庞大 draw/driver/plugin 体系
- 大规模类系统
- 过重的 property/style 反射抽象
- 为了目录整洁而强制拆成大量新 subsystem

## 7. 目录与文件组织原则

`v2.1` 的目录组织原则固定如下：

### 7.1 顶层产品目录

最终只保留一个顶层产品目录：

- `tinyui/`

当前 `picoui/` 的 `include/src/demo` 等内容原地迁移到这个目录中。

现有试点 `tinyui/include/*` 不继续作为“并行试点树”存在，而是吸收到统一产品目录里。

### 7.2 共享层

应保留为独立共享层的目录：

- `tinyui/src/core`
- `tinyui/src/display`
- `tinyui/src/indev`
- `tinyui/src/layout`
- `tinyui/src/theme`
- `tinyui/src/tick`
- `tinyui/src/osal`（若当前确有独立共享职责）

这些目录只保留跨控件共享职责，不允许继续承载产品层 backend bridge。

### 7.3 控件层

控件层目录固定为：

- `tinyui/src/widgets`

每个控件文件负责：

- 自己的 public API
- 自己的状态
- 自己对 `LingDongGUI` 的真实 widget-local binding

不再把 widget-specific 能力继续拆给 `backend_*` 文件。

### 7.4 必须消失的目录

以下目录在 `v2.1` 完成时必须消失：

- `picoui/`
- `tinyui/` 试点并行树（被统一吸收后不再双轨）
- `tinyui/src/backend`
- `picoui/src/backend`

如果底层对接仍需要少量共享 helper，这些 helper 必须迁入 `core` 或对应 subsystem，而不是继续保留 `backend/` 目录。

## 8. API 与文件收口原则

### 8.1 Public API

终态只允许：

- `tinyui_*`

终态不允许：

- `picoui_*`
- `tinyui_*` -> `picoui_*` 长期 alias
- `picoui/include/*` 继续作为公开入口

### 8.2 文件名

终态产品层文件名不应再出现：

- `picoui`
- `backend_`

允许保留的底层 `LingDongGUI` 文件名仍然是：

- `ld*`

因为这属于底层 engine truth，不属于产品层 rename 范围。

### 8.3 测试与合同

以下内容都要同步收口：

- `tests/picoui/*` -> 对应 `tests/tinyui/*` 或统一新路径
- `check_picoui_*` -> `check_tinyui_*`
- `test_picoui_*` -> `test_tinyui_*`
- `picoui_*` baseline/contract/json artifact 名称 -> `tinyui_*`

终态不允许靠“名字旧但内容新”来蒙混收口。

## 9. Runtime 与共享层判断

`v2.1` 不把所有实现都压进 `widgets/*`。

最佳做法是：

- widget-specific backend 代码并回 `widgets/*`
- runtime/object tree/display/indev/layout/theme/tick 保留独立共享薄层

这样做的原因是：

- 更接近 LVGL 的真实组织方式
- 不会把共享逻辑打散到每个控件里
- 不会为了消灭 `backend` 而把 `widgets/*` 膨胀成新的垃圾场

一句话：

**删除独立 backend 层，不等于删除共享层。**

## 10. 完成标准

`v2.1` 的完成标准固定为：

### 10.1 目录完成标准

- 顶层产品目录只剩 `tinyui/`
- 不再存在 `picoui/`
- 不再存在独立 `backend/` 产品层目录

### 10.2 命名完成标准

- public API 只剩 `tinyui_*`
- public header 路径只剩 `tinyui/include/*`
- demo/test/contract/CMake/文档标题的产品层公开痕迹不再出现 `picoui`

### 10.3 架构完成标准

- widget-specific backend 文件与 API 全部并回 `widgets/*`
- 共享 runtime/object/display/indev/layout/theme/tick 以薄层继续存在
- 真实 `LingDongGUI` 绑定继续成立
- 不允许靠假 bridge、假兼容层、假 renderer 通过 gate

### 10.4 证据完成标准

- focused tests 绿
- broad product-layer gates 绿
- runtime / visible / backend mapping 对应新 `tinyui` 口径绿
- perf/size/memory baseline 与 gate 已迁到 `tinyui` 命名并 fresh 通过
- closeout / release 文档与索引完成迁移

## 11. 阶段建议

`v2.1` 建议拆成以下串行阶段：

### `V0` 基线冻结与 rename/目录守门

- 冻结当前 `v2.0` 末态 inventory
- 增加 `v2.1` machine-readable baseline
- 让后续目录/API rename 有可审计 guard

### `V1` 顶层目录与 public header 收口

- `picoui/` 原地演化成唯一 `tinyui/`
- public include 树统一
- 去掉并行 `tinyui` 试点树

### `V2` widget-specific backend 并回 `widgets/*`

- 消灭独立 `backend_*` widget 文件
- 每个控件回到 widget-local real binding

### `V3` 共享层命名与 include path 收口

- `core/display/indev/layout/theme/tick/osal` 等共享层统一改到 `tinyui`
- include path、target、内部引用统一收口

### `V4` demo/test/contract/CMake 全量迁移

- demo/test/contract/json/python checker/CMake/CTest 全面改到 `tinyui`
- 删除 `picoui` 命名残留

### `V5` closeout 与 release 收口

- 删除剩余 `picoui` 痕迹
- 迁移 perf baseline / visible proof / runtime proof 的命名与真相源
- 写 `v2.1` closeout/release truth

## 12. 风险与约束

`v2.1` 的主要风险不是 widget 能力，而是“全仓大规模 rename + 目录迁移”。

必须控制的风险有：

- include path 雪崩
- CMake target / CTest 名称大面积漂移
- contract/json/python checker 路径断裂
- visible/runtime/perf gate 名称与文档不同步
- 目录迁移时把 `LingDongGUI` 底层边界误改坏

因此，`v2.1` 必须坚持：

- 严格串行阶段线
- 每阶段都要有 focused proof + broad gate + docs update
- 每阶段结束都要更新索引与真相源

## 13. 结论

`TinyUI v2.1` 的最佳路线已经确定：

- 不动 `LingDongGUI` 目录
- 不保留 `PicoUI` 兼容窗口
- 不继续维持双目录并行
- 以当前 `picoui/` 为唯一迁移母体
- 最终演化为唯一的 `tinyui/` 顶层产品目录
- 删除独立 `backend/`，但保留薄共享层
- 把产品层的公开痕迹彻底从 `PicoUI` 收口到 `TinyUI`

这条路线是当前用户目标下最直接、最一致、也最不含糊的方案。
