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

当前摘要：

- baseline inventory 与 transition guard 真相源已建立
- 具体 inventory 数值与机器态，统一回看 `docs/v2.0/2026-06-07-tinyui-v2-0-baseline-inventory.md` 和对应 contract artifact

### P1 backend shared/core 拆平

文件：`docs/v2.0/plans/stages/p1-backend-core-flatten-plan.md`

目标：

- 先处理 shared runtime/tree/event/layout glue
- 让 `backend/ldgui` 不再承担 shared system layer 职责

当前摘要：

- `runtime_bridge.c`、`core/widget.c` 等 shared/core flatten 事实已落地
- `P1` 只承认 shared/core flatten 主线与必要的少量 lifecycle/ownership 兜底，不把后续试点控件拆平混写进本阶段
- 具体 gate 结果与动态验证状态，不在本文件重复维护

### P2 试点控件拆平

文件：`docs/v2.0/plans/stages/p2-pilot-widgets-flatten-plan.md`

目标：

- `window/label/button/switch` 走通 `widgets/core -> ld*`
- 用最小控件集合验证 architecture

当前摘要：

- `window/label/button/switch` 的试点控件主路径已切到 widget-local direct `ld*` binding
- 旧 `backend_*` 文件在本阶段只按兼容残留理解，不再被当作试点主路径
- 具体 focused / runtime / visible / mapping gate 输出，不在本文件重复维护

### P3 app 退场与 runtime 转换

文件：`docs/v2.0/plans/stages/p3-app-removal-runtime-transition-plan.md`

目标：

- 用户主路径不再依赖 `app`
- demo 启动模型改成 `init/display/indev/screen/timer_handler`

当前摘要：

- `runtime.h/runtime.c` 与 app-free runtime 主路径事实已落地
- `basic_widgets` 作为当前 runtime proof 载体仍保留在 `picoui_*` 用户路径
- 具体 runtime / visible gate 输出，不在本文件重复维护

### P4 TinyUI public API 过渡

文件：`docs/v2.0/plans/stages/p4-tinyui-api-transition-plan.md`

目标：

- 收口 `tinyui_*` API
- 维持必要兼容层
- 让对外 public surface 有明确终态

当前摘要：

- `tinyui/include/*` 已开始提供试点 `tinyui_*` public surface
- `picoui/include/picoui/*` 中的兼容桥仍存在；本阶段仍是过渡态，不写成全量切换完成
- 具体 API inventory 与 gate 输出，不在本文件重复维护

### P5 性能与内存守门

文件：`docs/v2.0/plans/stages/p5-performance-and-memory-guard-plan.md`

目标：

- 对 `TinyUI` 相对 `LingDongGUI` 的额外开销量化守门
- 用真实数据约束速度、RAM、二进制体积，不靠感觉下结论

当前摘要：

- `binary_size`、`runtime_perf`、`wrapper_struct_overhead` 的 checker、baseline 与 `P6 Blocking Policy` 真相源已建立
- `P5` 的当前状态、证据口径和是否阻塞 `P6`，统一以 `docs/v2.0/v2.0-performance-baseline.md` 为准
- 本文件不单独维护 `P5` 是否完成、是否可进入 `P6`、以及具体 gate 输出数字

### P6 Closeout 与 release 收口

文件：`docs/v2.0/plans/stages/p6-closeout-and-release-plan.md`

目标：

- broad gates、docs、release-facing truth、索引、收口文案统一

## 执行纪律

- 任何阶段未完成 focused tests + broad gates + docs update，不得进入下一阶段
- 每阶段建议独立 fresh subagent
- 跨阶段共享文件冲突时，由主线程先重新裁边界
- `P6` 不得跳过 `P5`。没有性能/速度/RAM/体积证据，不允许写最终 closeout 结论。

## Closeout Truth

- `P6` 完成态以以下三份文档为准：
  - `docs/v2.0/v2.0-closeout.md`
  - `docs/v2.0/v2.0-release-matrix.md`
  - `docs/v2.0/v2.0-performance-baseline.md`
- 若本文件、阶段 plan checkbox、历史阶段 summary 与上述三份文档 wording 不一致，优先以后者和 fresh gate 输出为准。
- `P0` 到 `P4` 的已收口事实仍按各阶段 closeout 要点读取；涉及性能、速度、RAM、体积的结论，必须服从 `v2.0-performance-baseline.md` 的窄口径与 `P6 Blocking Policy`。
- `P6` 当前已完成 closeout / release 真相收口，但这仍只代表本线当前定义范围内的证据闭环，不自动等于“全量 `tinyui_*` public API 替换完成”或“更宽口径性能/RAM 证明完成”。
