# LingDongGUI Flex 对齐 LVGL 差距分析与补足步骤

> 日期：2026-05-24  
> 范围：`src/gui/ldWindow.*` 当前 flex 能力、layout 测试与 demo，对比 `third_party/lvgl` 的 `lv_flex`  
> 当前阶段约束：**只补 flex 能力，不做统一 layout 抽象，不做通用样式/属性系统**

## 1. 结论先说

LingDongGUI 当前 flex 已经不是空白，而是：

**最小一维 flex 已接进正式运行链路。**

它已经能做：

- row / column 两种 flow
- main axis 的 start / center / end / space-between
- cross axis 的 start / center / end
- padding / gap
- hidden child 跳过
- child / parent 改动后下一帧重排

但它和 LVGL 用户视角里的 flex，仍然差了一整层关键语义：

- wrap
- reverse
- 多 track
- track align
- grow
- new track
- ignore layout
- RTL
- gap 的 row/column 细分

所以这轮 flex 的重点，不是“再加几个枚举名字”，而是把模型从“单轨一维排队”升级到“多轨 flex 容器”。

---

## 2. 当前 LingDongGUI flex 的真实能力

### 2.1 当前公开接口

当前 flex 公开接口已有：

- `ldWindowSetFlexFlow`
- `ldWindowSetFlexAlign`
- `ldWindowSetPadding`
- `ldWindowSetGap`

只要调用 `ldWindowSetFlexFlow()`，容器就会自动切到 `layoutFlex`。

### 2.2 当前容器侧数据模型

当前 `ldWindow_t` 对 flex 只保存：

- `flexFlow`
- `flexMainAlign`
- `flexCrossAlign`
- `flexPadding`
- `flexGap`

这说明当前实现明确是：

- 容器有少量 flex 配置
- child 自身没有任何 flex 元数据

### 2.3 当前排布算法在做什么

当前 `ldWindowApplyFlexLayout()` 的逻辑大意是：

1. 收集直属可见 child
2. 计算 inner box
3. 累加主轴内容长度
4. 按主轴对齐算起点
5. 按交叉轴对齐算每个 child 的 cross 位置
6. 重写 child 位置

它**不会**做这些事：

- 不分 track
- 不 wrap
- 不 reverse
- 不 grow
- 不改 child 尺寸
- 不看 margin / min / max / percent
- 不看 child 级 flex 属性

所以当前 flex 本质是“单轨一维定位器”。

### 2.4 当前 dirty / 重排基础设施

这一块其实是 LingDongGUI 当前 flex 的一个优点。

目前这些动作都会触发父容器 layout dirty：

- `ldBaseSetHidden`
- `ldBaseSetWidth`
- `ldBaseSetHeight`
- `ldBaseSetRegion`
- `ldBaseNodeAdd`
- `ldBaseNodeRemove`

这意味着 flex 未来要扩功能时，不需要先重做整套重排触发链路。

### 2.5 当前测试和 demo

当前 host-side 测试已覆盖：

- 主轴起点解析
- setter 标脏
- row hidden reflow
- column 对齐

当前 demo 已覆盖：

- flex row
- flex column
- gap / padding
- hidden reflow
- 容器宽度变化后重新布局

这说明 flex 已经“跑进了主线”，但还没有形成 LVGL 那样的能力面与验证面。

---

## 3. LVGL flex 用户视角的基线

LVGL flex 的用户心智，至少包含下面几层：

1. **Flow**
   - row / column
   - row_wrap / column_wrap
   - row_reverse / column_reverse
   - row_wrap_reverse / column_wrap_reverse

2. **Align**
   - main axis：start / end / center / space-evenly / space-around / space-between
   - cross axis：start / end / center
   - track cross axis：align-content

3. **Child 级语义**
   - `grow`
   - `new track`
   - `ignore layout`

4. **方向语义**
   - RTL

5. **更成熟的尺寸联动**
   - margin
   - min/max
   - content-size
   - percent

这也是为什么 LVGL flex 用户会觉得它更像“可声明的布局系统”，而不是“容器帮我排一下队”。

---

## 4. 当前与 LVGL 的关键差距

## 4.1 flow 只完成 2/8

LingDongGUI 当前只有：

- row
- column

缺失：

- row_wrap
- column_wrap
- row_reverse
- column_reverse
- row_wrap_reverse
- column_wrap_reverse

这不是“高级特性”，而是 LVGL flex 的基本使用面。

只支持 row / column 的结果是：

- 一旦 child 放不下，LingDongGUI flex 就只能溢出或让用户自己再套容器
- 典型 tag / chip / toolbar / dashboard 流式布局做不出来

## 4.2 没有 track 概念

这是当前 flex 最大的结构性缺口。

LVGL flex 内部是“先分 track，再排 child”。  
LingDongGUI 当前只有一条主轴，没有轨道这个中间层。

后果是：

- wrap 做不了
- track align 做不了
- new track 做不了
- reverse 也只能很难看地打补丁

如果继续在当前单 for-loop 上堆功能，后面会越修越乱。

## 4.3 align 只完成一半

当前 LingDongGUI 主轴支持：

- start
- center
- end
- space-between

交叉轴支持：

- start
- center
- end

但缺失：

- `space-evenly`
- `space-around`
- track cross align（相当于 LVGL 的 `align-content`）

这会让用户在做“多个 track 的整体对齐”时完全无能为力。

## 4.4 没有 grow

当前 LingDongGUI flex 只改 child 位置，不改 child 尺寸。  
而 LVGL 的 `grow` 会把主轴剩余空间按权重分配给 child。

没有 grow，用户就做不了这些非常典型的界面：

- 一个按钮占剩余宽度
- 左边固定、右边自适应拉伸
- 多个 child 按比例分配空间

所以 grow 不是“锦上添花”，而是 flex 走向可用的重要分界线。

## 4.5 没有 child 级 `new track` / `ignore layout`

LVGL child 可以：

- 强制从新 track 开始
- 完全跳出 flex 排版但继续显示

LingDongGUI 当前 child 除了 hidden 之外，没有别的参与语义。  
这会影响很多真实 UI：

- badge / overlay
- 漂浮提示
- 某个 item 强制换行
- 某个 child 保持绝对定位但又挂在 flex 容器下

如果不补 child 元数据，LingDongGUI flex 永远只是“容器排队器”。

## 4.6 没有 RTL 方向语义

LVGL 会在 row 布局和 column track 排列里考虑 RTL，并交换 start/end 的方向语义。  
LingDongGUI 当前完全没有这一层。

这项可以晚一点做，但必须明确记账。

## 4.7 尺寸联动能力很薄

LVGL flex 会考虑：

- margin
- min/max
- `LV_SIZE_CONTENT`
- 百分比约束

LingDongGUI 当前只看 `tRegion.tSize` 的裸宽高。  
这会直接表现为：

- 稍微复杂一点的响应式布局就不自然
- grow 一旦引入，若没有最小尺寸联动，很容易假对齐或溢出

## 4.8 gap 语义还不够细

LingDongGUI 当前只有一个 `flexGap`。  
LVGL 则区分：

- row gap
- column gap

一旦进入 wrap，多 track 情况下，这个差异会立刻放大。

---

## 5. 本轮 flex 的合理目标

这轮不做上层抽象，flex 的合理目标应收敛成：

1. 容器支持完整 flow 基本盘
2. 内部从单轨算法升级成多 track 算法
3. child 具备最小 flex 元数据
4. grow / new track / ignore layout 进入主线能力
5. 补齐 flex 行为测试矩阵和 demo 场景

只要做到这 5 条，LingDongGUI flex 就会从“最小演示版”进入“可用于真实 UI 组织”的阶段。

---

## 6. 实施步骤拆解

## A 组：容器级 flex 语义骨架补足

### A1. 扩展容器配置模型

目标：先把容器能表达的 flex 语义补全。

具体动作：

1. 扩 `ldFlexFlow_t`
   - 至少补 `row_wrap`、`column_wrap`、`row_reverse`、`column_reverse`
   - 若一步到位，可直接按 LVGL 8 种 flow 设计

2. 扩主轴对齐枚举
   - 补 `space-evenly`
   - 补 `space-around`

3. 新增 track cross align
   - 对应多 track 容器整体对齐

4. gap 拆分为：
   - `item gap`
   - `track gap`

这一步仍然只在 `ldWindow` 上做，不引入通用 layout 抽象。

### A2. 重写 flex 核心算法为“先分 track，再排 child”

目标：把当前单轨扫描升级成真正的 flex 排布骨架。

建议拆成两阶段：

1. `find_track_end`
   - 决定一个 track 收哪些 child

2. `repos_track_items`
   - 决定这些 child 在主轴/交叉轴怎么排

reverse / wrap / track align / RTL 都应建立在这套骨架上。  
不要继续在当前单一 for-loop 上堆 if/else。

### A3. 容器级能力先形成验证矩阵

目标：把“容器语义”先钉死，再进入 child 级 grow/new-track。

建议至少补这些 host-side 场景：

1. row_wrap
2. column_wrap
3. row_reverse
4. column_reverse
5. space-evenly
6. space-around
7. track align
8. RTL（若本轮纳入）

同时扩 demo：

- flow demo
- align demo
- track demo
- RTL demo（若本轮纳入）

## B 组：child 级 flex 语义补足

### B1. 给 child 增加最小 flex 元数据

目标：不做上层抽象，但补齐 flex 真正需要的 child 语义。

建议最少增加：

- `flexGrow`
- `flexInNewTrack`
- `ignoreLayout`

建议新增 API：

- `ldBaseSetFlexGrow(ldBase_t *, uint8_t)`
- `ldBaseSetFlexNewTrack(ldBase_t *, bool)`
- `ldBaseSetIgnoreLayout(ldBase_t *, bool)`

并保证这些 setter 都能触发父容器 layout dirty。

### B2. 实现 grow / new track / ignore layout 的真实行为

目标：让 flex 不只是容器重排，而是开始具备真实空间分配能力。

具体动作：

1. `grow`
   - 按主轴剩余空间按权重分配
   - 真正改 child 主轴尺寸，不只改位置

2. `new track`
   - 只在 wrap 模式生效
   - 标记的 child 成为新轨起点

3. `ignore layout`
   - child 可见
   - 但不参与 flex 计数和占槽
   - 保持手工坐标

这 3 项完成后，LingDongGUI flex 的使用体验会和 LVGL 更接近。

### B3. 补最小尺寸联动闭环

目标：避免 grow 一上来就假对齐或溢出。

这轮可以不复制 LVGL 全量约束系统，但至少要补一层最小安全子集：

1. grow 时尊重当前尺寸作为 base size
2. 预留 min/max 挂点
3. wrap 计算考虑 gap 后剩余空间

否则 grow 一旦进入主线，很容易让布局变得不稳定。

## C 组：验证闭环补足

### C1. 补 flex host-side 测试矩阵

至少覆盖：

1. 8 种 flow 或本轮纳入的 flow 子集
2. main align 全枚举
3. track align
4. grow 权重分配
5. new track
6. ignore layout
7. hidden child 与 ignore layout 的区别
8. child 属性变化导致父容器重排

### C2. 升级 flex demo

当前 demo 只证明 row / column 能跑。  
建议扩成至少 4 组场景：

1. flow
2. align
3. grow
4. overlay / ignore-layout

### C3. 对照 LVGL examples/tests 验收

重点参考：

- `flex_flow`
- `flex_align`
- `flex_grow`
- `flex_new_track`
- `flex_ignore_layout`
- `flex_rtl`
- `test_align_flex.c`
- `test_flex_grow.c`

---

## 7. 实施优先级建议

### 第一优先级

先做：

1. A1 扩容器语义
2. A2 改成多 track 算法骨架

原因：没有这层，后面的 wrap/reverse/grow 都无从落地。

### 第二优先级

再做：

1. B1 child 最小元数据
2. B2 grow / new track / ignore layout

原因：这是 LingDongGUI flex 能否接近 LVGL 用户感知的关键分水岭。

### 第三优先级

最后补：

1. A3 / C1 测试矩阵
2. C2 demo
3. B3 最小约束联动

### 可延后项

若要控 scope，可延后：

- RTL
- 更完整的 min/max / content-size / percent 语义
- style/property 映射

但这些必须在文档里记账。

---

## 8. 建议重点盯住的文件

### LingDongGUI 侧

- `src/gui/ldWindow.h`
- `src/gui/ldWindow.c`
- `src/gui/ldWindowLayoutInternal.h`
- `src/gui/ldWindowLayoutInternal.c`
- `src/gui/ldBase.h`
- `src/gui/ldBase.c`
- `examples/sdl/tests/layout/test_layout_window.c`
- `examples/common/demo/layout/uiLayout.c`
- `docs/superpowers/specs/2026-05-22-flex-layout-phase0-1-development-spec.md`

### LVGL 侧

- `third_party/lvgl/include/lvgl/layouts/lv_flex.h`
- `third_party/lvgl/src/layouts/flex/lv_flex.c`
- `third_party/lvgl/docs/src/common-widget-features/layouts/flex.mdx`
- `third_party/lvgl/examples/layouts/flex/flex_flow/lv_example_flex_flow.c`
- `third_party/lvgl/examples/layouts/flex/flex_align/lv_example_flex_align.c`
- `third_party/lvgl/examples/layouts/flex/flex_grow/lv_example_flex_grow.c`
- `third_party/lvgl/examples/layouts/flex/flex_new_track/lv_example_flex_new_track.c`
- `third_party/lvgl/examples/layouts/flex/flex_ignore_layout/lv_example_flex_ignore_layout.c`
- `third_party/lvgl/examples/layouts/flex/flex_rtl/lv_example_flex_rtl.c`
- `third_party/lvgl/tests/src/test_cases/test_align_flex.c`
- `third_party/lvgl/tests/src/test_cases/test_flex_grow.c`
- `third_party/lvgl/tests/src/test_cases/test_margin_flex.c`

---

## 9. 一句话收口

这轮 flex 的关键，不是“把 row / column 再补两个枚举”，而是：

**先不做上层抽象，直接把 LingDongGUI 的 flex 从“单轨排队器”补成“多轨、可分配空间、带 child 语义的 flex 容器”。**
