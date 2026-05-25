# 2026-05-25 switch / flex / grid 对齐 LVGL 审计与任务板

## 1. 文档目的

这份文档不负责定义“这轮怎么做”，而负责在本轮 grid 收口后，持续记录：

- `switch`
- `flex`
- `grid`

这三条能力线距离“行为/视觉/测试对齐 LVGL 常用主干”还有多远，以及下一阶段应该先做什么。

本文档口径如下：

- 保持 LingDongGUI 现有接口命名习惯
- 以行为、视觉、demo、测试为主要对齐标准
- 区分“已对齐”“未对齐”“明确延期”

## 2. 总览

| 线别 | 当前阶段判断 | 本轮优先级 | 说明 |
| --- | --- | --- | --- |
| switch | 基本完成第一轮主干对齐 | 低 | 重点从“补存在性”转为“审边角” |
| flex | 主干已成型，但未 full parity | 中 | 已能覆盖常见主线，一些高级语义待后续 |
| grid | 当前主攻方向 | 高 | 主干已建立，仍需继续收口至 LVGL 常用语义 |

## 3. switch 审计

## 3.1 已对齐项

- 原生控件已存在，不再是 stub
- 支持 `AUTO / 横向 / 纵向`
- 支持禁用态
- 支持首帧稳态
- 支持 value change 事件
- 内部具备 track / indicator / knob 语义
- 已接入 demo
- 已有 internal/widget tests

## 3.2 仍需复核项

- 与 LVGL 在视觉细节上的差距是否还存在
- 程序设值和交互事件的边界是否完全一致
- 是否还存在“主干已对齐，但 demo 易用性不够”的问题

## 3.3 当前判断

`switch` 大概率已经达到第一轮“行为/视觉/测试主干对齐”口径，但仍需要在本轮后统一复核，避免过早把全部差距归零。

## 3.4 后续任务

### P1

- 对照现有测试与 demo，补一轮“仍未对齐 LVGL 的边角项”审计

### P2

- 若存在视觉/交互细差距，再单独开小任务收口

## 4. flex 审计

## 4.1 已对齐项

- row / column
- wrap / reverse / wrap_reverse
- main align
- cross align
- track align
- item gap / track gap
- `grow`
- `new track`
- `ignore layout`
- host-side 测试
- layout demo 承接

## 4.2 未对齐项

- `RTL`
- margin 相关语义
- percent translate / percent size
- content-size 容器联动
- 更完整的 min/max 约束闭环

## 4.3 当前判断

`flex` 已经可以视为“主干对齐完成、边缘语义待补”的状态。它不再是当前最急的基础能力缺口，但还不能宣称 full LVGL parity。

## 4.4 后续任务

### P1

- 先完成本轮 grid 主干收口，再统一复核 flex 当前主干口径

### P2

- 视用户价值，优先补 `RTL` 或尺寸联动语义

### P3

- 若未来需要，可进一步拆出 flex 第二阶段路线文档

## 5. grid 审计

## 5.1 已具备能力

- descriptor-grid 主线已建立
- 支持 `fixed / CONTENT / FR`
- 支持 explicit cell
- 支持 `col_span / row_span`
- 支持 cell align
- 支持 container align
- 支持 legacy `gridColumns` fallback
- 已有测试与 demo 基线

## 5.2 本轮必须继续收口的项

- `FR` 分配稳定性
- `CONTENT` 尺寸传播
- span + auto placement 组合
- align 行为的运行态证据
- descriptor-grid 下默认自动落位兼容
- legacy fallback 不回归

## 5.3 明确延期项

- `subgrid`
- `RTL`
- grid 下专门的 `ignore-layout`

## 5.4 当前判断

`grid` 是三条线里距离“LVGL 常用主干对齐”最近、但也最需要继续收口的一条。它已经不再是旧顺排模型，但仍处于主线强化阶段。

## 5.5 后续任务

### P0

- 完成本轮 grid 主干行为收口

### P1

- 补强 host-side 测试矩阵

### P1

- 确保 `USE_DEMO=5` 继续承担 grid 运行态证据

### P2

- 本轮完成后重新写出“已对齐 / 未对齐 / 延期”真相

## 6. 本轮实施任务板

## 6.1 P0：Grid 主干对齐

- [ ] 审核 descriptor-grid solver 当前与 LVGL 的常用行为差距
- [ ] 收口 `FR`
- [ ] 收口 `CONTENT`
- [ ] 收口 span
- [ ] 收口 container align
- [ ] 收口 cell align
- [ ] 收口 descriptor-grid 下 auto placement fallback

## 6.2 P1：验证补强

- [ ] 扩 layout host-side 测试
- [ ] 跑 `examples/sdl` 构建
- [ ] 跑 `ctest`
- [ ] 跑 `USE_DEMO=5` smoke

## 6.3 P2：三线审计回写

- [ ] 复核 `switch`
- [ ] 复核 `flex`
- [ ] 回写 `grid`
- [ ] 更新“已对齐 / 未对齐 / 延期”
- [ ] 产出下一阶段任务排序

## 7. 建议的优先级

### 先做

- grid 主干行为收口

### 再做

- layout 测试与 demo 证据补齐

### 最后做

- `switch / flex / grid` 三线统一审计

原因：

- 这样写出来的是“修完后的真相”
- 避免在代码还没收口前，文档先把状态写死

## 8. 退出条件

满足下面条件，本轮可以结束：

- grid 主干行为收口完成
- layout 测试与 demo 验证通过
- 三线审计文档回写完成
- 下一阶段任务板可直接承接后续开发
