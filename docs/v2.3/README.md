# TinyUI v2.3 文档入口

## 版本目标

TinyUI v2.3 是一次破坏式、轻量化的 canonical API 收口：参考 LVGL 的 API 使用模式，继续以 LingDongGUI/Arm-2D 为唯一真实实现，面向 LVGL 无法承载的小型 MCU。

本版本不实现第二套对象树、renderer、layout solver、style selector、事件传播系统或资源中心。

## 阅读顺序

1. `docs/v2.3/2026-07-13-tinyui-v2-3-unified-core-design.md`
2. `docs/v2.3/线计划索引.md`
3. `docs/v2.3/plans/v2.3-orchestration-plan.md`
4. `docs/v2.3/plans/stages/README.md`
5. 按 `M0 -> M1 -> M2 -> M3 -> M4 -> M5` 阅读阶段计划
6. `docs/v2.3/v2.3-capability-evidence-matrix.md`
7. `docs/v2.3/v2.3-migration-guide.md`
8. `docs/v2.3/v2.3-release-gates.md`
9. `docs/v2.3/deferred-port-work.md`

## 固定阶段

| 阶段 | 目标 | 状态 |
| --- | --- | --- |
| M0 | 恢复事实可信度并冻结轻量基线 | 已完成 |
| M1 | 冻结轻量 canonical 公共契约 | 进行中 |
| M2 | 核心纵向闭环与四个样板控件 | 未开始 |
| M3 | 全控件与全部 required 能力闭环 | 未开始 |
| M4 | demo、文档、安装和仓外消费者迁移 | 未开始 |
| M5 | 全量硬化、独立评审和 ABI 冻结 | 未开始 |

状态只能在对应阶段 fresh gate 全部通过后更新，不以提交数量或局部测试通过代替完成证据。

## 轻量硬约束

- 基础 wrapper：当前 `184 B`，上限 `192 B`。
- 每种 wrapper：相对 M0 baseline 的增量不得同时超过 `5%` 和 `8 B`。
- backend widget：当前 `496 B`，上限 `512 B`。
- 32 位 ABI runtime pool：timer、event callback 和 bookkeeping 合计不超过 `1024 B`。
- image source：32 位 ABI 不超过 `80 B`。
- font value：32 位 ABI 不超过 `16 B`。
- steady-state `tinyui_process()` 隐式 heap 分配为 `0`。
- 未启用控件和可选模块不得进入最小裁剪二进制。
- binary size 沿用 `5%/8192 B` 等双阈值策略，详见设计与 release gate。

## 完成口径

v2.3 只在以下条件同时满足时关闭：

- 只剩一套 canonical runtime、object、event、timer、theme 和资源 value API。
- 所有 required 能力达到 L4。
- 可见能力达到 L5-V，可操作能力达到 L5-E。
- 全量构建、CTest、ASan、UBSan、安装消费者和文档示例通过。
- Flash、RAM、对象尺寸和零稳态分配门禁通过。
- 独立评审没有未关闭的 P0/P1 非 port 问题。
- port 延期项仍被明确标记为未完成。

## 执行约束

- 当前项目不创建 worktree。
- 所有 shell 命令使用 `rtk` 前缀。
- 修改任何函数前先运行 GitNexus upstream impact；`HIGH/CRITICAL` 必须先告警并形成调用者迁移清单。
- 每阶段结束运行 GitNexus `detect_changes()`。
- 多个 subagent 写面不得重叠。
- review 不通过时，由原 review subagent 跟踪修复。
- demo 只表达用户意图，不承担 backend/layout 修补。

## 当前声明边界

规划完成不等于实现完成。当前文档只锁定 v2.3 的目标、任务和门禁，不改变 TinyUI 当前 Alpha 状态，也不改变 port 尚未生产闭环的事实。

## 当前执行进度

M0 已完成。M1 已完成任务 1-6：public contract、runtime/object、event/timer/focus、style/theme/diagnostics、flex/grid/image/font descriptor、全控件 creator/props/object 参数统一。任务 7-10 尚未开始，因此 M1 仍保持“进行中”。

M1 任务 6 的窄验证为 widget creator contract checker 绿（29 控件，含 presence-mask 静态扫描）、全部 `create_with_props` 按 `props->fields` 门控 setter（含 half-size 保留当前边、table/graph 尺寸进入 create、setter kind 失败返回 -1）、`tinyui_core` 构建通过、`tinyui_public_header_probes_{c,cpp}` 通过；`tinyui_public_symbol_link_probes` 仍被既有 `tinyui_last_error_message` 诊断开关扫描问题挡住。`scroll_selecter.h` 已重命名为 `scroll_selector.h`，源文件改名与 legacy aggregate 清理留给任务 7。
