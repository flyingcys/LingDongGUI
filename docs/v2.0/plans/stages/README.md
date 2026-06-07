# TinyUI v2.0 Stages

## 读法

先读：

1. `docs/v2.0/2026-06-07-tinyui-v2-0-design.md`
2. `docs/v2.0/plans/v2.0-orchestration-plan.md`
3. 本文件

然后严格按下面顺序执行阶段 plan。

## 阶段顺序

### P0 基线与守门

文件：`docs/v2.0/plans/stages/p0-baseline-and-guards-plan.md`

目标：

- 冻结当前 `backend/app` 基线
- 给 `backend`、`app`、`tinyui_*` 迁移建立 contract guards
- 让后续阶段不在移动基线中作业

### P1 backend shared/core 拆平

文件：`docs/v2.0/plans/stages/p1-backend-core-flatten-plan.md`

目标：

- 先处理 shared runtime/tree/event/layout glue
- 让 `backend/ldgui` 不再承担 shared system layer 职责

### P2 试点控件拆平

文件：`docs/v2.0/plans/stages/p2-pilot-widgets-flatten-plan.md`

目标：

- `window/label/button/switch` 走通 `widgets/core -> ld*`
- 用最小控件集合验证 architecture

### P3 app 退场与 runtime 转换

文件：`docs/v2.0/plans/stages/p3-app-removal-runtime-transition-plan.md`

目标：

- 用户主路径不再依赖 `app`
- demo 启动模型改成 `init/display/indev/screen/timer_handler`

### P4 TinyUI public API 过渡

文件：`docs/v2.0/plans/stages/p4-tinyui-api-transition-plan.md`

目标：

- 收口 `tinyui_*` API
- 维持必要兼容层
- 让对外 public surface 有明确终态

### P5 性能与内存守门

文件：`docs/v2.0/plans/stages/p5-performance-and-memory-guard-plan.md`

目标：

- 对 `TinyUI` 相对 `LingDongGUI` 的额外开销量化守门
- 用真实数据约束速度、RAM、二进制体积，不靠感觉下结论

### P6 Closeout 与 release 收口

文件：`docs/v2.0/plans/stages/p6-closeout-and-release-plan.md`

目标：

- broad gates、docs、release-facing truth、索引、收口文案统一

## 执行纪律

- 任何阶段未完成 focused tests + broad gates + docs update，不得进入下一阶段
- 每阶段建议独立 fresh subagent
- 跨阶段共享文件冲突时，由主线程先重新裁边界
- `P6` 不得跳过 `P5`。没有性能/速度/RAM/体积证据，不允许写最终 closeout 结论。
