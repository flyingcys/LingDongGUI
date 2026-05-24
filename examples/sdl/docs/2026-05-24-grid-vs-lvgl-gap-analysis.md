# LingDongGUI Grid 对齐 LVGL 差距分析与补足步骤

> 日期：2026-05-24  
> 范围：`src/gui/ldWindow.*` 当前 grid 能力、layout 测试与 demo，对比 `third_party/lvgl` 的 `lv_grid`  
> 当前阶段约束：**只补 grid 能力，不做统一 layout 抽象，不做 style/property 框架抽象**

## 1. 结论先说

LingDongGUI 当前的 `grid` 不是 LVGL 语义上的二维显式网格，而是：

**带 padding / gap 的等宽分栏 + 按可见直属子项顺序自动换行。**

它已经能做简单 dashboard 平铺，但还不能做 LVGL 用户预期里的这些能力：

- 先定义 row / column descriptor
- 指定 child 放到哪一格
- child 跨多列 / 多行
- cell 内 start / center / end / stretch
- 轨道支持固定值 / `CONTENT` / `FR`
- 容器整体的 grid align

所以这轮 grid 的重点，不是“修几个 setter”，而是把模型从“顺排容器”升级成“显式网格容器”。

### 1.1 2026-05-24 当前完成状态回写

本轮实现后，LingDongGUI 的 grid 已经补到下面这一级别：

- `ldWindow` 可声明 column / row descriptor
- `ldBase` 可声明 child cell 位置、span、cell 内对齐
- solver 已按 explicit cell 计算，不再只靠 row-first 顺排
- 轨道已支持 fixed / `CONTENT` / `FR`
- demo 已从“3 列 dashboard”改成 descriptor + span + center 的组合示例
- host-side 测试已覆盖 descriptor、`CONTENT`、`FR`、span、container align、cell align、非法输入钳制、legacy fallback

但这不等于和 LVGL 完全对齐；下面这些仍然明确不支持：

- subgrid
- RTL
- ignore-layout / floating child

这些边界本轮只记账，不伪装成已支持。

---

## 2. 当前 LingDongGUI grid 的真实能力

### 2.1 当前公开接口

当前容器级 grid 只有 3 个公开 API：

- `ldWindowSetGridColumns`
- `ldWindowSetGridGap`
- `ldWindowSetGridPadding`

这说明当前 grid 能力仍是“容器自己决定怎么顺排”，而不是“用户声明网格结构和 child cell 语义”。

### 2.2 当前容器侧数据模型

当前 `ldWindow_t` 对 grid 只保存了：

- `gridColumns`
- `gridRowGap`
- `gridColumnGap`
- `gridPadding`

没有这些关键字段：

- `column descriptors`
- `row descriptors`
- `grid align`
- `subgrid`
- `child cell metadata`

### 2.3 当前排布算法实际在做什么

当前 `ldWindowApplyGridLayout()` 的逻辑大意是：

1. 取直属可见子项
2. 依据 `gridColumns` 计算平均列宽
3. 每一行的高度取该行最高 child
4. child 按 row-first 顺序塞进各格
5. child 宽度最多被 clamp 到列宽
6. child 不会 stretch 到 cell
7. child 没有显式 grid 属性参与排布

这说明当前 grid 只是“基于列数的自动排版”，而不是“二维 declarative grid”。

### 2.4 当前测试覆盖到哪里

当前 `examples/sdl/tests/layout/test_layout_window.c` 对 grid 的覆盖主要是：

- setter 会标脏
- hidden child 会跳过
- 2 列 + gap + padding 时 row-first 摆放正确

没有覆盖这些关键语义：

- span
- align
- descriptor
- `FR`
- `CONTENT`
- invalid settings
- subgrid
- RTL
- ignore layout

### 2.5 当前 demo 覆盖到哪里

当前 `examples/common/demo/layout/uiLayout.c` 里的 grid demo 本质是：

- 3 列 dashboard
- 固定 gap
- 固定 padding
- 几个高矮不同的 panel 顺排

它能证明“grid 已经接进运行链”，但不能证明“grid 语义已经接近 LVGL”。

---

## 3. LVGL grid 用户视角的基线

LVGL grid 的核心不是“有几列”，而是“先定义轨道，再定义 cell”。

从用户视角，LVGL grid 至少包括这些能力：

1. **Grid descriptor**
   - 列模板数组
   - 行模板数组
   - 轨道支持固定像素 / `LV_GRID_CONTENT` / `LV_GRID_FR(x)`

2. **显式 cell**
   - child 必须声明在哪个 column / row
   - child 可以设置 `col_span` / `row_span`

3. **容器级 align**
   - 整个 grid 可以 `start/end/center/space-evenly/space-around/space-between`

4. **cell 内对齐**
   - child 在 cell 内可 `start/center/end/stretch`

5. **额外语义**
   - subgrid
   - RTL
   - ignore layout
   - invalid settings 容错

这套心智模型和 LingDongGUI 当前“按列数顺排”完全不是一回事。

---

## 4. 当前与 LVGL 的关键差距

## 4.1 轨道描述能力缺失

这是最大的结构性缺口。

LVGL grid 的第一步是声明：

- 每一列多宽
- 每一行多高
- 哪些轨道按内容走
- 哪些轨道按剩余空间比例分配

LingDongGUI 当前只有 `gridColumns` 整数列数。  
这意味着用户只能表达：

- “给我 3 列”

却不能表达：

- “第一列 120px，第二列 content，第三列吃剩余空间”
- “上面一行 48px，下面一行 1fr”

如果这一层不补，LingDongGUI 就永远不是 LVGL 意义上的 grid。

## 4.2 容器级 grid align 缺失

当前 LingDongGUI grid 永远从 `padding.left/top` 开始铺。  
没有“整张 grid 在容器剩余空间里如何摆”的语义。

这使得用户无法表达：

- 整体居中
- 整体贴右 / 贴底
- 轨道 evenly / around / between 分布

这也是 LVGL grid 用户非常自然会期待的能力。

## 4.3 child 没有显式 cell 语义

这是第二个结构性缺口。

LVGL 的 child 必须显式声明：

- `column_pos`
- `row_pos`
- `column_span`
- `row_span`
- `column_align`
- `row_align`

LingDongGUI 当前则是：

- 所有可见直属 child 自动按顺序塞格子

这会导致两个根本问题：

1. 用户无法手动控制网格结构
2. grid 只能表达“顺排”，无法表达“布局”

## 4.4 span 能力完全缺失

LVGL 可以让 child：

- 跨 2 列
- 跨 2 行
- 占一个更大的区域

LingDongGUI 当前因为根本没有 occupancy map / span 逻辑，所以做不到：

- 卡片跨两列
- 列表说明区跨两行
- 子容器占一块矩形区域

这一层对 dashboard、设置页、信息面板类 UI 都非常关键。

## 4.5 cell 内 align 缺失

当前 LingDongGUI child 放入 grid cell 后，本质是把左上角落在 cell 起点。  
它不会：

- 水平居中
- 垂直底对齐
- stretch 填满 cell

这和 LVGL grid 差异很大。

对用户来说，这意味着：

- 同一个 grid 中不同尺寸 child 很难排齐
- 经常还要手工改 child 自己的尺寸和坐标
- 失去“用 grid 帮我摆好”的价值

## 4.6 `CONTENT` / `FR` 缺失

LVGL grid 的核心优势之一，是：

- 一些轨道按内容大小决定
- 一些轨道按剩余空间比例分配

LingDongGUI 现在只有平均列宽。  
这意味着：

- 复杂信息卡片布局做不出来
- 表单布局做不出来
- 左侧标签 / 右侧自适应内容这种典型界面做不出来

## 4.7 subgrid / RTL / ignore-layout / 容错能力都还没开始

这些不一定都要在第一轮完成，但至少要明确：

- 当前还没有
- 第一轮做不做
- 不做的话如何记账

否则后续补 grid 时，边界会反复漂移。

当前回写：

- subgrid：未做
- RTL：未做
- ignore-layout：未做
- 容错：`pos/span` 钳制已补，descriptor 缺失时走 legacy `gridColumns` fallback

---

## 5. 本轮 grid 的合理目标

本轮不做统一 layout 平台，目标应收敛成：

1. `ldWindow` 能表达 row / column descriptors
2. `ldBase` 能表达 child 的 cell 位置 / 跨度 / cell 内对齐
3. `ldWindowApplyGridLayout()` 变成真正的 grid solver
4. 补齐 descriptor / span / align / `FR` / `CONTENT` 的测试基线
5. demo 从“3 列平铺”升级到“多个 grid 语义场景”

只要做到这 5 条，LingDongGUI 的 grid 用户感知就会发生质变。

---

## 6. 实施步骤拆解

## A 组：容器轨道模型补足

### A1. 定义 grid descriptor 数据模型

目标：先把“列数”升级成“轨道描述”。

具体动作：

1. 在 `ldWindow_t` 中新增：
   - `gridColDsc`
   - `gridRowDsc`
   - `gridColAlign`
   - `gridRowAlign`

2. 定义 grid 常量：
   - `LD_GRID_TEMPLATE_LAST`
   - `LD_GRID_CONTENT`
   - `LD_GRID_FR(x)`

3. 明确兼容策略：
   - 现有 `gridColumns` 先保留
   - 仅作为 legacy fallback
   - 新路径主走 descriptor

4. 先只建立“数据可表达”的能力，不急于一次写完 solver

### A2. 增加容器级 grid API

目标：给 `ldWindow` 一个完整但仍然局部化的 grid 容器接口。

建议新增：

- `ldWindowSetGridDscArray(window, col_dsc, row_dsc)`
- `ldWindowSetGridAlign(window, col_align, row_align)`

setter 统一规则：

- 自动切到 `layoutGrid`
- 标记 layout dirty
- descriptor 变化后触发重新布局

### A3. 写轨道预计算器

目标：先把网格容器的“轨道怎么算”独立出来，不在 `ldWindowApplyGridLayout()` 里写成一坨。

建议拆分为几步：

1. 统计 row / column 数量
2. 计算固定像素轨道
3. 计算 `CONTENT` 轨道
4. 计算 `FR` 轨道
5. 计算容器级 align 后的最终 `pos + size`

输出结果应至少包括：

- 每列起点和宽度
- 每行起点和高度

这是整个 grid 补足里最关键的底座。

## B 组：child cell 语义补足

### B1. 给 child 增加最小 grid 元数据

目标：让 child 终于能表达“我在第几格，占几格”。

建议在 `ldBase_t` 侧增加最小字段：

- `gridColPos`
- `gridRowPos`
- `gridColSpan`
- `gridRowSpan`
- `gridCellXAlign`
- `gridCellYAlign`
- 可选 `ignoreLayout`

建议新增 `ldBase` 级 API：

- `ldBaseSetGridCell(widget, col_align, col_pos, col_span, row_align, row_pos, row_span)`

注意：

- 这不是上层抽象
- 这是 grid 自身不可缺少的 child 语义

### B2. 重写 `ldWindowApplyGridLayout()`

目标：把当前“顺排算法”升级成真正的 cell 驱动网格布局。

新的排布流程应该是：

1. 先算容器轨道
2. 再读每个 child 的 cell 属性
3. 计算它覆盖的区域
4. 在 cell 内按 start/center/end/stretch 放置
5. hidden child 跳过
6. 可选 ignore-layout child 跳过

这一步完成后，LingDongGUI 才算真正拥有 grid 布局，而不是“多列顺排”。

### B3. 补合法化与 dirty 传播

目标：让 grid 不只是“能排”，还要“能长期维护”。

具体动作：

1. child 的 `span <= 0` 自动归一到 1
2. `pos < 0` 钳到 0
3. 越界 `pos/span` 自动裁剪到合法范围
4. `ldBaseSetGridCell()` 必须触发父容器 layout dirty
5. descriptor 变化必须触发重排
6. 继续复用 `ldWindowApplyLayoutRegion()` 的旧/新包围盒脏区合并逻辑

## C 组：验证闭环补足

### C1. 补 grid host-side 测试矩阵

至少覆盖：

1. 固定像素 descriptor
2. `FR` 轨道分配
3. `CONTENT` 轨道
4. child `start / center / end / stretch`
5. col / row span
6. invalid settings 钳制
7. hidden / ignore-layout
8. child cell 属性变化导致父容器重排

### C2. 升级 grid demo

当前 demo 只证明“3 列 dashboard 能跑”。  
建议补成至少 4 组场景：

1. fixed/fixed/fixed
2. `FR`
3. `CONTENT + FR`
4. span + align

### C3. 对照 LVGL example/test 做验收

重点参考：

- `grid_descriptors`
- `grid_fr`
- `grid_cell_span`
- `grid_align`
- `test_grid.c`

目标不是 API 名字完全照抄，而是用户能复现同类布局结果。

---

## 7. 实施优先级建议

### 第一优先级

先做这 3 项：

1. A1 descriptor 数据模型
2. A2 grid 容器 API
3. B1 child cell 元数据

原因：先把“语义能表达”补出来。

### 第二优先级

再做：

1. A3 轨道计算器
2. B2 真正的 grid solver

原因：没有这层，grid 仍然只是旧算法打补丁。

### 第三优先级

最后补：

1. B3 容错与 dirty 传播收口
2. C1 测试矩阵
3. C2 demo 升级

### 可延后项

若本轮需要控 scope，可以延后：

- subgrid
- RTL
- 更深的 ignore-layout / floating 语义
- style/property 层映射

但要在文档里明确记账。

---

## 8. 建议重点盯住的文件

### LingDongGUI 侧

- `src/gui/ldWindow.h`
- `src/gui/ldWindow.c`
- `src/gui/ldBase.c`
- `src/gui/ldBase.h`
- `examples/sdl/tests/layout/test_layout_window.c`
- `examples/common/demo/layout/uiLayout.c`
- `lingdonggui-vs-lvgl-user-analysis.md`

### LVGL 侧

- `third_party/lvgl/include/lvgl/layouts/lv_grid.h`
- `third_party/lvgl/src/layouts/grid/lv_grid.c`
- `third_party/lvgl/docs/src/common-widget-features/layouts/grid.mdx`
- `third_party/lvgl/examples/layouts/grid/grid_descriptors/lv_example_grid_descriptors.c`
- `third_party/lvgl/examples/layouts/grid/grid_fr/lv_example_grid_fr.c`
- `third_party/lvgl/examples/layouts/grid/grid_cell_span/lv_example_grid_cell_span.c`
- `third_party/lvgl/examples/layouts/grid/grid_align/lv_example_grid_align.c`
- `third_party/lvgl/tests/src/test_cases/test_grid.c`

---

## 9. 一句话收口

这轮 grid 的关键，不是“把 `gridColumns` 再修一修”，而是：

**先不做统一抽象，直接把 LingDongGUI 的 grid 从“顺排容器”补成“显式二维网格容器”。**
