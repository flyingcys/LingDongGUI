# 2026-05-25 Grid 对齐 LVGL 主干设计

## 1. 背景

LingDongGUI 当前三条相关能力线状态如下：

- `switch`：已经完成原生控件化，主线具备方向、禁用态、首帧稳态、动画、demo 与测试覆盖。
- `flex`：主干一维布局语义已成型，已支持 wrap、reverse、grow、new track、ignore layout、track align 等第一轮核心能力。
- `grid`：已经从旧 `gridColumns` 顺排容器升级到 descriptor-grid 主线，但仍处于“主干已建立、与 LVGL 常用语义尚需继续收口”的阶段。

当前用户目标不是统一改 API 命名，而是保持 LingDongGUI 现有接口风格，在行为、视觉和测试层面尽量对齐 LVGL 的常用主干能力。

因此本轮只聚焦一件事：

**继续优化 `grid`，把 LingDongGUI 的 grid 行为收敛到接近 LVGL 常用主干语义；随后再回头审计 `switch / flex / grid` 三条线当前是否已达到“行为对齐”口径，并把结果细化进文档。**

## 2. 本轮目标

### 2.1 主目标

继续优化 `src/gui/ldWindow.*` 与 `src/gui/ldBase.*` 中的 grid 主线，使其在下面这些常见场景下与 LVGL 的常用 grid 语义保持一致或可接受等价：

- descriptor grid
- `fixed / CONTENT / FR`
- explicit cell
- `col_span / row_span`
- cell 内 `start / center / end / stretch`
- container 级 `start / center / end / space-between / space-around / space-evenly / stretch`
- 未显式设置 cell 时的自动落位兼容

### 2.2 次目标

在 grid 收口完成后，对 `switch / flex / grid` 三条线做一次中文审计，给出：

- 已对齐项
- 未对齐项
- 明确延期项
- 下一阶段任务排序

### 2.3 完成口径

本轮“完成”不等于 API 改名对齐 LVGL，而是满足下面 4 条：

1. `grid` 常用行为对齐 LVGL 主干语义
2. 测试能锁住关键语义，不靠口头判断
3. demo 能作为运行态证据
4. `switch / flex / grid` 三线现状被整理成可执行任务板

## 3. 范围

### 3.1 本轮要做

- 继续收口 descriptor-grid solver
- 继续收口 `FR`、`CONTENT`、span、align、auto placement 等行为
- 补 host-side layout 测试
- 必要时调整 `USE_DEMO=5` 对应的 grid 演示内容
- 产出三线审计与任务板文档

### 3.2 本轮不做

- `subgrid`
- `RTL`
- grid 专属的 `ignore-layout` 语义扩展
- 统一 style/property/theme 抽象
- 为了贴 LVGL 而重命名 LingDongGUI 对外 API
- 把 flex 或 switch 顺手扩成下一阶段大范围重构

## 4. 设计原则

### 4.1 行为优先，不做命名迁移

目标是让 LingDongGUI 用户在使用 `switch / flex / grid` 时，实际得到与 LVGL 接近的行为结果，而不是把接口字面量改成 LVGL 风格。

### 4.2 先补主干，再补高级语义

`subgrid`、`RTL`、grid 下 `ignore-layout` 属于高阶或边缘语义。本轮先把主干做实，避免把有限 scope 拖成长尾工程。

### 4.3 兼容旧入口，但降低其主路径地位

`gridColumns` 旧入口继续保留，用于兼容已有 demo 和调用方；但文档上要明确：

- 它只是 legacy fallback
- descriptor-grid 才是 LingDongGUI grid 的主实现方向

### 4.4 证据优先

任何“已经对齐 LVGL”的结论都必须至少满足以下之一：

- 代码路径可明确映射到对应语义
- host-side 测试覆盖该语义
- demo/build/smoke 证明确实跑通

## 5. 实施拆分

## 5.1 P1：Grid 主干行为收口

围绕 `src/gui/ldWindow.c` 中 descriptor-grid solver 做继续收口，重点检查和修正：

- `FR` 剩余空间分配是否稳定
- `CONTENT` 轨道是否由 child 尺寸正确驱动
- span 是否正确占位且不破坏后续 auto placement
- container align 是否与 LVGL 常用语义一致
- cell align 是否与 cell 尺寸/child 尺寸交互正确
- auto placement fallback 是否在 descriptor-grid 下也能兼容旧心智模型

需要关注的核心边界：

- hidden child 不应污染轨道尺寸与落位
- explicit cell 和 auto placement 混用时不能互相覆盖
- span 与 `CONTENT/FR` 叠加时不能产生非预期压缩或错位
- align 不能只停留在 setter 赋值，需要落到真实 region 结果

## 5.2 P2：验证面补强

验证分三层：

### host-side 行为测试

主测试文件优先继续使用：

- `examples/sdl/tests/layout/test_layout_window.c`

重点补或继续强化：

- descriptor-grid 自动落位
- `SpaceBetween`
- `SpaceAround`
- `SpaceEvenly`
- `Stretch`
- span 与 align 组合
- `CONTENT + FR` 混合
- hidden child 与 explicit cell 混用

### build 验证

至少保留：

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-review -DUSE_DEMO=4
rtk cmake --build examples/sdl/build-review
rtk ctest --test-dir examples/sdl/build-review --output-on-failure
```

### demo smoke

至少保留：

```bash
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-review/ldgui_sdl_demo
rtk cmake -S examples/sdl -B examples/sdl/build-grid-review -DUSE_DEMO=5
rtk cmake --build examples/sdl/build-grid-review --target ldgui_sdl_demo
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-grid-review/ldgui_sdl_demo
```

如果本轮 grid demo 有新增组合场景，也必须被上述 smoke 覆盖，而不是停留在只改单测。

## 5.3 P3：三线审计与任务板

在 grid 主干收口完成之后，再回头统一审计：

### switch

重点确认：

- 是否已达到“行为/视觉/测试对齐 LVGL 常用主干”口径
- 是否仍有边缘体验差距，例如视觉细节、交互时序、易用性 API 等

### flex

重点确认：

- 当前主干 flow/align/wrap/grow/new-track/ignore-layout 是否可视为第一轮行为对齐
- 尚未覆盖的差距是否主要集中在 `RTL / margin / percent / content-size / 更完整 min/max`

### grid

重点确认：

- 本轮之后哪些主干语义已经可标记为“已对齐”
- 哪些仍属于明确后续项
- 哪些应继续保留为“延期/不做”

## 6. 风险与控制

## 6.1 风险

- grid solver 继续演化时，容易破坏 legacy fallback
- span / align / auto placement 混用时，容易出现隐性错位
- demo 正常不代表测试齐全，测试正常也不代表 demo 真场景稳定
- 若顺手扩到 `subgrid / RTL`，scope 会快速失控

## 6.2 控制

- 只改 grid 主干，不主动扩高级语义
- 每次修改非平凡 layout 入口前，先做 GitNexus impact 分析
- 代码结论必须由测试或 smoke 支撑
- 旧兼容入口继续保留，并在文档中明确定位

## 7. 验收标准

### 7.1 Grid 行为验收

- descriptor-grid 主路径可稳定工作
- `fixed / CONTENT / FR` 组合结果正确
- explicit cell、span、cell align、container align 都有行为证据
- auto placement 在 descriptor-grid 路径下继续可用
- `gridColumns` legacy fallback 不回归

### 7.2 验证验收

- layout host-side 测试通过
- `examples/sdl` build 通过
- grid demo smoke 通过

### 7.3 文档验收

- 产出三线审计与任务板文档
- 文档中每条结论都能落到代码、测试或 demo 证据
- 明确区分“已完成”“未完成”“延期”

## 8. 产物

本轮应至少产出：

- grid 主干代码收口
- 补强后的 layout 测试
- 必要的 grid demo 调整
- `switch / flex / grid` 三线审计与任务板

推荐配套文档路径：

- `docs/superpowers/specs/2026-05-25-grid-lvgl-mainline-design.md`
- `examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md`

## 9. 下一步

本设计确认后，下一步不是立刻写实现，而是把本轮任务进一步转成 implementation plan：

- 明确代码文件 ownership
- 明确测试先后顺序
- 明确每个阶段的验证命令
- 明确提交边界与回归门禁
