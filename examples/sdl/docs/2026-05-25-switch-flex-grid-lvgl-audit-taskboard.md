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
| switch | 第一轮主干与关键视觉证据都已闭环 | 低 | 主路径与关键截图证据已齐，后续只剩更高置信度增强 |
| flex | 主干已成型，但未 full parity | 中 | 已能覆盖常见主线，一些高级语义待后续 |
| grid | 主干继续收口后，已接近 LVGL 常用主线 | 高 | 仍需继续审计边角，但本轮主干缺口已明显收窄 |

## 3. switch 审计

## 3.1 已对齐项

- 原生控件已存在，不再是 stub
- 支持 `AUTO / 横向 / 纵向`
- 支持禁用态
- 支持首帧稳态
- 支持 value change 事件
- 内部具备 track / indicator / knob 语义
- 已补 switch 私有导航 API
- 已补 `ldSwitchCanNavigate()`，让通用焦点层只在“这次方向键确实会改值”时把事件交给 switch
- 已补 knob overhang 几何
- 已补 indicator ring 几何
- 已补 host-side 焦点行为断言，证明 no-op / disabled 方向会放行给 peer focus
- 已补 capture matrix，锁住 pressed knob / animation mid-frame / checked ring
- 已接入 demo
- 已有 internal/widget tests

## 3.2 仍需复核项

- 若继续追更高置信度 parity，是否需要整图截图 diff 或主题组合矩阵

## 3.3 已证实差距

- 当前这轮已不再有未补齐的主逻辑 / 关键视觉证据差距
- 后续若继续做，只是把证据从“关键点采样”升级到“更高置信度像素比对”

详见：`examples/sdl/docs/2026-05-25-switch-lvgl-edge-audit.md`

## 3.4 当前判断

`switch` 现在可以视为“第一轮主干能力已完成且关键视觉证据闭环”。更准确的状态是：行为、几何主路径与关键截图证据都已经对齐到 LVGL 常见用法，后续只剩可选的证据增强。

## 3.5 后续任务

### P1

- 若要继续追更高置信度 parity，补主题组合下的像素级断言或截图比对

### P2

- 视需要再补 host demo 的显式按键提示，帮助人工验收

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
- 支持 descriptor-grid 下的 auto placement fallback
- 支持 legacy `gridColumns` fallback
- 已有测试与 demo 基线

## 5.2 本轮已收口项

- `FR` 分配主路径
- `CONTENT` 尺寸传播主路径
- span + auto placement 组合
- align 行为测试证据
- descriptor-grid 下默认自动落位兼容
- legacy fallback 未回归

## 5.3 明确延期项

- `subgrid`
- `RTL`
- grid 下专门的 `ignore-layout`

## 5.4 当前判断

`grid` 经过本轮后，已经可以视为“LVGL 常用主干基本到位、边缘语义待继续审计”的状态。它不再只是旧顺排模型的增强版，而是以 descriptor-grid 为中心的主实现。

## 5.5 后续任务

### P0

- 复核本轮 grid 主干收口后的剩余边角差距

### P1

- 补强 host-side 测试矩阵

### P1

- 确保 `USE_DEMO=5` 继续承担 grid 运行态证据，并维持 auto placement 展示位

### P2

- 本轮完成后重新写出“已对齐 / 未对齐 / 延期”真相

## 6. 本轮实施任务板

## 6.1 P0：Grid 主干对齐

- [x] 审核 descriptor-grid solver 当前与 LVGL 的常用行为差距
- [x] 收口 `FR`
- [x] 收口 `CONTENT`
- [x] 收口 span
- [x] 收口 container align
- [x] 收口 cell align
- [x] 收口 descriptor-grid 下 auto placement fallback

## 6.2 P1：验证补强

- [x] 扩 layout host-side 测试
- [x] 跑 `examples/sdl` 构建
- [x] 跑 `ctest`
- [x] 跑 `USE_DEMO=5` smoke

## 6.3 P2：三线审计回写

- [x] 复核 `switch`
- [x] 复核 `flex`
- [x] 回写 `grid`
- [x] 更新“已对齐 / 未对齐 / 延期”
- [x] 产出下一阶段任务排序

## 6.4 P3：三控件细化任务

- [x] `switch`：补通用焦点层 routing
- [x] `switch`：补 indicator ring + knob overhang 几何
- [x] `switch`：补 pressed / animation / checked ring 视觉证据
- [x] `grid`：补 descriptor-grid auto placement 主路径
- [ ] `grid`：补更高阶语义（`subgrid` / RTL / grid ignore-layout）
- [ ] `flex`：补更高阶语义（RTL、margin、percent/content-size）

## 7. 建议的优先级

### 先做

- switch 视觉证据补强

### 再做

- grid / flex 第二阶段语义差距

### 最后做

- `switch / flex / grid` 三线统一复审

原因：

- switch 主逻辑在本轮已经闭环，接下来最短路径是补证据
- grid / flex 已有主线，不必再和 switch 的主路径问题混写

## 8. 退出条件

满足下面条件，本轮可以结束：

- grid 主干行为收口完成
- switch 主路径行为/几何闭环
- layout 测试与 demo 验证通过
- 三线审计文档回写完成
- 下一阶段任务板可直接承接后续开发
