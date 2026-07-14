# TinyUI v2.3 阶段计划入口

## 阅读方式

执行前依次阅读：

1. `docs/v2.3/2026-07-13-tinyui-v2-3-unified-core-design.md`
2. `docs/v2.3/plans/v2.3-orchestration-plan.md`
3. 本文件
4. 当前阶段计划

## 阶段文件

| 阶段 | 文件 | 可否与下一阶段并行 |
| --- | --- | --- |
| M0 | `m0-truth-baseline-plan.md` | 否 |
| M1 | `m1-lightweight-public-contract-plan.md` | 否 |
| M2 | `m2-core-vertical-plan.md` | 否 |
| M3 | `m3-full-capability-plan.md` | 内部可按无重叠控件家族并行 |
| M4 | `m4-consumer-delivery-plan.md` | M3 关门后开始 |
| M5 | `m5-hardening-freeze-plan.md` | 否 |

## 每阶段固定动作

1. 读取设计与阶段 plan。
2. 对待修改符号运行 GitNexus upstream impact。
3. `HIGH/CRITICAL` 时向用户报告风险和调用者迁移范围。
4. 按 TDD 顺序执行：失败测试、最小实现、focused pass、broad pass。
5. 运行轻量 gate，确认尺寸、静态 RAM、裁剪和稳态分配没有越线。
6. 更新证据矩阵和阶段状态。
7. 使用独立 subagent review；不通过时由原 reviewer 跟踪修复。
8. 运行 GitNexus `detect_changes()`。
9. 运行 `rtk git diff --check`。

## 禁止事项

- 不创建 worktree。
- 不并行修改共享写面。
- 不为旧 demo 保留第二套 public 生命周期。
- 不新增第二套 tree、layout、renderer、style selector、事件传播或资源 runtime。
- 不用 TinyUI 镜像 round-trip 代替真实 LingDongGUI 状态证据。
- 不用截图代替事件证据，也不用事件代替像素证据。
- 不改 port 实现来让阶段 gate 变绿。
- 不把延期 port 项标为 v2.3 已完成。

## 关门纪律

只有当前阶段计划中的 focused、broad、轻量、文档和 review gate 全部通过，才能进入下一阶段。任何 baseline 缺失、checker fallback 或人工证据为空都必须 fail-closed。
