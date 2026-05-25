# Grid 补齐任务清单

> 日期：2026-05-24  
> 目标：把 LingDongGUI 的 grid 从“多列顺排容器”补成“显式二维网格容器”  
> 范围：只补 grid 能力；**不**做统一 layout/style/property 抽象

## 0. 2026-05-24 实施回写

- 已落地：descriptor 模型、`ldWindowSetGridDscArray()`、`ldWindowSetGridAlign()`、`ldBaseSetGridCell()`、fixed/`CONTENT`/`FR` 轨道、explicit cell、`col_span` / `row_span`、cell `start/center/end/stretch`、legacy `gridColumns` fallback、host-side grid test、`USE_DEMO=5` grid demo 改造
- 已验证：`layout_window_test` 通过；`USE_DEMO=5` 的 `ldgui_sdl_demo` 编译通过
- 截至 2026-05-26：`ignore-layout` 已按现有 `ldBaseSetIgnoreLayout()` 语义接进 grid；仍明确不支持 `subgrid`、RTL
- 轨道计算器本轮采用 `ldWindow.c` 内部 static helper 收口，没有单拆新源文件

## 1. 本轮完成标准

完成后，grid 至少需要满足：

1. 容器可声明 row / column descriptors
2. child 可声明 cell 位置 / span / cell 内对齐
3. 轨道支持固定像素 / `CONTENT` / `FR`
4. 容器支持 grid align
5. solver 按显式 cell 排布，而不是按顺序顺排
6. 补齐 descriptor / span / align / 容错测试

---

## 2. 涉及文件

### 核心实现

- `src/gui/ldWindow.h`
- `src/gui/ldWindow.c`
- `src/gui/ldBase.h`
- `src/gui/ldBase.c`

### 当前测试与 demo

- `examples/sdl/tests/layout/test_layout_window.c`
- `examples/common/demo/layout/uiLayout.c`

### 对照基线

- `third_party/lvgl/include/lvgl/layouts/lv_grid.h`
- `third_party/lvgl/src/layouts/grid/lv_grid.c`
- `third_party/lvgl/docs/src/common-widget-features/layouts/grid.mdx`
- `third_party/lvgl/tests/src/test_cases/test_grid.c`
- `third_party/lvgl/examples/layouts/grid/grid_descriptors/lv_example_grid_descriptors.c`
- `third_party/lvgl/examples/layouts/grid/grid_fr/lv_example_grid_fr.c`
- `third_party/lvgl/examples/layouts/grid/grid_cell_span/lv_example_grid_cell_span.c`
- `third_party/lvgl/examples/layouts/grid/grid_align/lv_example_grid_align.c`
- `third_party/lvgl/examples/layouts/grid/grid_subgrid/lv_example_grid_subgrid.c`

---

## 3. A 组：容器轨道模型补足

## A1. 定义 grid descriptor 数据模型

### 目标

先把 grid 容器从“只有列数”升级成“可以声明轨道”。

### 步骤

- [ ] 在 `src/gui/ldWindow.h` 为 `ldWindow_t` 增加 grid 轨道字段
  - `gridColDsc`
  - `gridRowDsc`
  - `gridColAlign`
  - `gridRowAlign`
- [ ] 定义 grid 常量
  - `LD_GRID_TEMPLATE_LAST`
  - `LD_GRID_CONTENT`
  - `LD_GRID_FR(x)`
- [ ] 设计最小 `ldGridAlign_t`
  - 至少含 `start / end / center / stretch / space-evenly / space-around / space-between`
- [ ] 保留 `gridColumns` 旧字段
  - 仅作为兼容 fallback
  - 不再作为新路径核心语义

### 验证

- [ ] 编译通过
- [ ] 现有 grid setter 仍可工作
- [ ] 旧 demo 在未切 descriptor 前不回归

## A2. 增加容器级 grid API

### 目标

让容器层可以完整表达“轨道长什么样”。

### 步骤

- [ ] 新增 `ldWindowSetGridDscArray()`
- [ ] 新增 `ldWindowSetGridAlign()`
- [ ] 保持 `ldWindowSetGridGap()` 和 `ldWindowSetGridPadding()` 继续可用
- [ ] 所有 grid setter 统一执行
  - 置 `layoutType = layoutGrid`
  - 调 layout dirty
- [ ] 明确 `NULL descriptor` 的行为
  - 若本轮不做 subgrid，则必须写清 fallback 或 reject 行为

### 验证

- [ ] setter host-side 测试补齐
- [ ] descriptor 变化后父容器会重排

## A3. 独立轨道计算器

### 目标

不要把 `CONTENT/FR/align` 逻辑全塞进 `ldWindowApplyGridLayout()`。

### 步骤

- [ ] 新建 grid calc 辅助结构
  - 保存 row / column 数量
  - 保存每列起点与宽度
  - 保存每行起点与高度
- [ ] 拆分计算步骤
  - 统计 descriptor 数量
  - 计算固定像素轨道
  - 计算 `CONTENT` 轨道
  - 计算 `FR` 轨道
  - 应用容器 grid align
- [ ] 若需要，新增内部 helper 头/源文件
  - 建议不要让 `ldWindow.c` 继续膨胀过快

### 验证

- [ ] host-side 测试：固定像素 descriptor
- [ ] host-side 测试：`FR`
- [ ] host-side 测试：`CONTENT`
- [ ] host-side 测试：container align

---

## 4. B 组：child cell 语义补足

## B1. 给 child 增加最小 grid 元数据

### 目标

让 child 能表达“我在第几格，占几格，格内怎么摆”。

### 步骤

- [x] 在 `src/gui/ldBase.h` 增加最小 grid 字段
  - `gridColPos`
  - `gridRowPos`
  - `gridColSpan`
  - `gridRowSpan`
  - `gridCellXAlign`
  - `gridCellYAlign`
  - 复用现有 `ignoreLayout`
- [x] 新增 `ldBaseSetGridCell()`
- [x] 若纳入 ignore-layout，则复用现有 `ldBaseSetIgnoreLayout()`
- [x] child 属性改变时，必须回标父容器 layout dirty

### 验证

- [ ] host-side 测试：`ldBaseSetGridCell()` 会触发父容器重排
- [ ] host-side 测试：cell 属性可存取并编译通过

## B2. 重写 `ldWindowApplyGridLayout()`

### 目标

把 grid 从“顺序切行”升级成“按 cell 语义摆放”。

### 步骤

- [x] 保留 hidden child 跳过语义
- [ ] 读取每个 child 的 cell 信息
- [ ] 计算 child 覆盖的行列范围
- [ ] 支持单格摆放
- [ ] 支持 `col_span`
- [ ] 支持 `row_span`
- [ ] 支持 cell 内 `start / center / end / stretch`
- [x] `ignoreLayout` child 保留手工坐标且不占 auto placement 槽位
- [ ] 保留 child 旧/新 region 合并脏区逻辑

### 验证

- [ ] host-side 测试：单格定位
- [ ] host-side 测试：跨 2 列
- [ ] host-side 测试：跨 2 行
- [ ] host-side 测试：cell 内 `center`
- [ ] host-side 测试：cell 内 `stretch`
- [x] host-side 测试：descriptor-grid ignore-layout overlay
- [x] host-side 测试：legacy grid fallback ignore-layout overlay

## B3. 容错与边界语义收口

### 目标

让 grid 进入长期可维护状态。

### 步骤

- [ ] `col_span <= 0` 归一到 1
- [ ] `row_span <= 0` 归一到 1
- [ ] `col_pos < 0` 钳到 0
- [ ] `row_pos < 0` 钳到 0
- [ ] 超范围 `pos/span` 自动裁剪到合法范围
- [ ] 明确 descriptor 缺失时的行为
- [ ] 若本轮不做 subgrid，必须在文档中记账并写清不支持
- [ ] 若本轮不做 RTL，也在文档中记账并写清不支持

### 验证

- [ ] host-side 测试：非法 span
- [ ] host-side 测试：负位置
- [ ] host-side 测试：超范围位置
- [ ] host-side 测试：空 descriptor / fallback

---

## 5. C 组：验证与 demo 补足

## C1. 补 grid 测试矩阵

### 最少补齐的用例

- [ ] fixed descriptor
- [ ] `FR`
- [ ] `CONTENT`
- [ ] grid align
- [ ] child cell align
- [ ] `col_span`
- [ ] `row_span`
- [ ] hidden child
- [x] ignore-layout overlay：继续可见、保留手工坐标、不占 grid slot
- [ ] invalid settings
- [ ] child cell 改变触发父布局重排

### 建议文件

- 继续扩 `examples/sdl/tests/layout/test_layout_window.c`
- 或新增 grid 专属测试文件，减小 `test_layout_window.c` 膨胀速度

## C2. 升级 grid demo

### 目标

从“3 列 dashboard”升级成“多个 grid 语义示例”。

### 建议场景

- [ ] fixed/fixed/fixed
- [ ] `FR`
- [ ] `CONTENT + FR`
- [ ] span
- [ ] cell align

### 修改点

- [ ] 继续扩 `examples/common/demo/layout/uiLayout.c`
- [ ] 或拆分出更清晰的 grid demo helper，避免一个 demo 文件继续变大

## C3. 文档同步

### 目标

让 grid 的用户说明和现状分析同步。

### 步骤

- [ ] 更新 `examples/sdl/docs/2026-05-24-grid-vs-lvgl-gap-analysis.md`
  - 回写哪些已完成，哪些延期
- [ ] 若新增 API，补充教程或 README 中相关说明
- [ ] 若保留 legacy `gridColumns` fallback，要明确写出它只是兼容入口

---

## 6. 推荐顺序

### 第一阶段

- A1 descriptor 数据模型
- A2 grid API
- B1 child cell 元数据

### 第二阶段

- A3 轨道计算器
- B2 真正的 grid solver

### 第三阶段

- B3 容错收口
- C1 测试矩阵
- C2 demo 升级
- C3 文档同步

---

## 7. 完成判定

满足下面条件才算 grid 第一轮补齐完成：

- 容器能声明 row / column descriptor
- child 能声明 cell / span / align
- grid solver 不再依赖顺排逻辑
- `FR` / `CONTENT` 至少进入第一轮主线
- 测试矩阵覆盖 descriptor / span / align / 容错
- demo 不再只是一页 3 列平铺
