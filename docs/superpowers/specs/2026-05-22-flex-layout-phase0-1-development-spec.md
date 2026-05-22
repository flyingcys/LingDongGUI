# LingDongGUI Flex Phase 0/1 开发规格说明

**日期**：2026-05-22  
**状态**：拟开发  
**范围类型**：本轮开发 spec  
**关联文档**：

- `docs/superpowers/specs/2026-05-22-flex-grid-layout-design.md`
- `docs/superpowers/reviews/2026-05-22-flex-grid-layout-feasibility-review.md`

---

## 1. 文档目标

本文件用于定义 **LingDongGUI 布局系统第一轮可执行开发范围**。

本轮只覆盖：

- legacy `ldWindow` 自动布局语义修正
- `layout dirty` 基础设施
- 最小一维 Flex

本轮**不开发 Grid**，但会在文档中明确 Grid 的远期协商边界，避免后续再次回到“大而空”的布局讨论。

---

## 2. 本轮目标

本轮目标不是一次性做出完整 Flex/Grid 系统，而是先建立一个**可靠的一维布局基础层**。

具体目标如下：

1. 让 `ldWindow` 的自动布局语义变得稳定、可预测
2. 建立独立于绘制脏区的 `layout dirty` 机制
3. 支持最小一维 Flex：
   - `row`
   - `column`
   - `padding`
   - `gap`
   - 主轴/交叉轴基础对齐
4. 让布局变化能够正确进入现有 dirty region / PFB 刷新链
5. 提供一组 SDL 侧可重复验证的布局样例和回归口径

---

## 3. 非目标

下面这些内容在本轮**明确不做**：

- Grid 布局
- Flex wrap
- Flex grow / shrink
- 内容驱动尺寸（content-size）
- 百分比尺寸
- rowspan / colspan
- 多轨道布局
- 完整的 bottom-up 布局调度框架
- 把所有控件都升级成通用布局宿主

这些内容不是否定，而是明确延期。

---

## 4. 本轮核心设计原则

### 4.1 先做稳定的一维布局，再讨论二维系统

本轮不追求功能最全，而追求：

- 直属子项语义稳定
- 父容器重排触发稳定
- dirty region 联动稳定

### 4.2 `paint dirty` 和 `layout dirty` 必须分离

当前仓库已有：

- `isDirtyRegionUpdate`：表示控件需要重绘  
  参考：`src/gui/ldBase.h:211`

本轮新增或规范化：

- `isLayoutUpdate`：表示布局容器需要重新排版  
  当前已有字段位于 `ldWindow_t`  
  参考：`src/gui/ldWindow.h:66`

原则：

- `paint dirty` 只解决重绘
- `layout dirty` 只解决重排
- 两者可以协作，但不能共用一个标志

### 4.3 第一阶段只让 `ldWindow` 成为布局容器

本轮不把通用布局能力扩散到所有控件类型。

第一阶段的 layout host 只有：

- `ldWindow`

这样做的原因：

- 当前已有 `ldWindow_t.isLayoutUpdate`
- `ldWindow` 已经承担 legacy 自动布局
- 这样改动最小，能尽快验证布局基础设施

### 4.4 只处理直属子项

这是本轮最重要的布局语义约束：

> **无论 legacy 还是 Flex，本轮自动布局都只处理直属子项，不递归重排后代子树。**

这条约束是后续一切布局能力的基础。

---

## 5. 代码组织决策

### 5.1 本轮不引入 `src/layouts/`

虽然从架构纯度看，布局系统独立目录是合理方向，但本轮为了控制范围，决定：

- **本轮不引入 `src/layouts/` 目录**
- **布局相关实现仍放在 `src/gui/` 范围内**

原因：

- 当前聚合入口是 `src/gui/ldGui.h`
- 当前 API 文档生成脚本只扫描 `src/gui/*.c`  
  参考：`docs/tutorial/generate_api.py:213-260`
- 模板脚本也围绕 `src/gui` 工作  
  参考：`src/template/widgetCreate.py:67-83`

因此本轮采用更现实的组织方式：

- 公共 API 继续定义在 `ldWindow.h` / `ldBase.h`
- 内部实现放在 `src/gui/` 下的新旧文件中

### 5.2 本轮优先减少公开结构体改动

本轮尽量避免对 `ldBase_t` 做大量新增字段。

原因：

- `ldBase_t` 是公开基类
- 很多控件直接 `implement(ldBase_t)`
- 大量扩字段会迅速扩大兼容和回归范围

因此本轮策略是：

- Phase 0 不给 `ldBase_t` 增加 Flex/Grid 子项字段
- Phase 1 不支持 grow / wrap / grid
- 先只让容器端持有最小布局配置

---

## 6. Phase 0：legacy 语义清理 + layout dirty 基础设施

## 6.1 目标

Phase 0 的目标不是增加新布局功能，而是让现有容器具备可靠的重排基础。

## 6.2 交付内容

### 6.2.1 修正 legacy 自动布局语义

修改 `ldWindow_on_frame_start()`，使 `layoutHorizontal` / `layoutVertical` 满足：

- 仅遍历直属子项
- 不递归进入后代子树
- 子项顺序按 `ptChildList -> ptNext` 顺序处理

不允许再出现：

- 用直属子项计数
- 却对整棵后代子树做 `PREORDER_TRAVERSAL`

### 6.2.2 明确 hidden 子项策略

为了降低兼容风险，本轮规定：

- `layoutHorizontal` / `layoutVertical`：**保持当前兼容取向，hidden 子项仍参与 slot 计数**
- `layoutFlex`：**hidden 子项不参与布局**

说明：

- 这样可以避免在本轮同时引爆 legacy 行为变化
- 也为新布局建立更合理的默认语义

### 6.2.3 建立 `layout dirty` 冒泡机制

新增一个内部辅助能力，语义类似：

```c
void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget);
```

该能力不直接刷新屏幕，只负责：

- 从当前节点向上遍历父链
- 找到最近一个启用了自动布局的 `ldWindow`
- 将其 `isLayoutUpdate = true`
- 若存在更外层自动布局容器，继续向上冒泡

### 6.2.4 接入点

第一批必须接入 `layout dirty` 冒泡的地方：

- `ldBaseSetRegion()`
- `ldBaseResize()`
- `ldBaseSetX()`
- `ldBaseSetY()`
- `ldBaseSetWidth()`
- `ldBaseSetHeight()`
- `ldBaseSetHidden()`
- `ldBaseNodeAdd()`
- `ldBaseNodeRemove()`
- `ldWindowSetLayout()`

说明：

- 这些函数仍然保留原有 paint dirty 行为
- 但在必要时额外触发父容器 `layout dirty`

### 6.2.5 dirty region 接入规则

Phase 0 要明确以下流程：

1. 子项发生几何或可见性变化
2. 子项自身产生 paint dirty
3. 最近的布局容器被标记为 `isLayoutUpdate = true`
4. 下一个 `frameStart` 中，容器执行布局
5. 对位置发生变化的直属子项调用几何 setter
6. 子项通过现有 `tTempRegion + isDirtyRegionUpdate` 进入 dirty region 更新链

这意味着：

- 不新增 widget 级的全新刷新系统
- 继续复用现有 `ldScene0.c` / `ldScene1.c` 的 dirty region 提交路径

### 6.2.6 Phase 0 非目标

Phase 0 不做这些事情：

- 不引入 `layoutFlex`
- 不引入新的子项布局属性
- 不改现有 scene 调度模型
- 不引入 Grid 或多轨道语义

## 6.3 Phase 0 验收标准

必须满足：

1. `layoutHorizontal` / `layoutVertical` 只影响直属子项
2. 子项尺寸变化后，父容器下一帧会重新布局
3. 子项隐藏显示后，父容器行为符合本 spec 定义
4. 局部刷新无明显重影、残影、旧区域残留
5. 不破坏当前 widget 多页轮播 demo 的 scene switching 行为

---

## 7. Phase 1：最小一维 Flex

## 7.1 目标

在 Phase 0 基础上引入一个**范围严格受控**的一维 Flex。

本阶段只回答一个问题：

> **当一个 window 作为布局容器时，能否稳定替直属子项做一维排布。**

## 7.2 功能范围

本轮 Flex 只支持：

- `row`
- `column`
- 容器 `padding`
- 子项间 `gap`
- 主轴对齐
- 交叉轴对齐
- 固定尺寸子项

## 7.3 不支持的能力

明确不支持：

- wrap
- grow
- shrink
- 百分比尺寸
- 内容驱动尺寸
- reverse
- track 对齐
- grid

## 7.4 对外 API

### 7.4.1 布局类型

在 `ldLayoutType_t` 中新增：

```c
layoutFlex
```

本轮**不新增**：

```c
layoutGrid
```

原因：

- 避免在“尚未开发”的情况下暴露虚假公开能力
- 也避免让外部代码误以为 Grid 已进入本轮范围

### 7.4.2 Flow

新增最小 flow 枚举：

```c
typedef enum {
    ldFlexFlowRow = 0,
    ldFlexFlowColumn,
} ldFlexFlow_t;
```

### 7.4.3 Align

本轮对齐能力建议拆成：

```c
typedef enum {
    ldFlexAlignStart = 0,
    ldFlexAlignCenter,
    ldFlexAlignEnd,
    ldFlexAlignSpaceBetween,
} ldFlexMainAlign_t;

typedef enum {
    ldFlexCrossAlignStart = 0,
    ldFlexCrossAlignCenter,
    ldFlexCrossAlignEnd,
} ldFlexCrossAlign_t;
```

说明：

- 不引入 `space-around` / `space-evenly`
- 避免第一版主轴分配策略过多

### 7.4.4 容器 API

新增公开 API：

```c
void ldWindowSetFlexFlow(ldWindow_t *ptWindow, ldFlexFlow_t flow);
void ldWindowSetFlexAlign(ldWindow_t *ptWindow,
                          ldFlexMainAlign_t mainAlign,
                          ldFlexCrossAlign_t crossAlign);
void ldWindowSetPadding(ldWindow_t *ptWindow, ldPadding_t padding);
void ldWindowSetGap(ldWindow_t *ptWindow, int16_t gap);
```

说明：

- `ldWindowSetFlexFlow()` 调用后自动切换到 `layoutFlex`
- 本轮只有单一 `gap`，不区分 `rowGap` / `colGap`
- `padding` 为容器内边距，适用于 Flex

### 7.4.5 兼容规则

- `pLayoutPaddingGroup` 仅服务 legacy `layoutHorizontal/Vertical`
- `ldWindowSetPadding()` 仅服务 `layoutFlex`
- 本轮不重命名 `layoutTpye`
- 本轮不修改现有 legacy API 名称

## 7.5 布局规则

### 7.5.1 布局对象

`layoutFlex` 只处理：

- 当前 `ldWindow` 的直属子项
- 且仅处理可见子项

hidden 子项：

- 不参与 item 数量统计
- 不参与 gap 计算
- 不占据布局空间

### 7.5.2 主轴与交叉轴

- `row`：主轴为 X，交叉轴为 Y
- `column`：主轴为 Y，交叉轴为 X

### 7.5.3 固定尺寸原则

本轮不改变子项自身宽高。

也就是说：

- 子项 `tRegion.tSize` 由自身决定
- Flex 只负责给出位置
- 不做 grow / shrink / 自适应尺寸计算

### 7.5.4 容器内布局区域

布局时先计算 inner box：

- 容器总区域减去 `padding`

Flex 的排布都在 inner box 内完成。

### 7.5.5 Gap 规则

- `gap` 只作用于相邻可见子项之间
- 首项前和末项后不自动附加 `gap`

### 7.5.6 对齐规则

主轴：

- `Start`
- `Center`
- `End`
- `SpaceBetween`

交叉轴：

- `Start`
- `Center`
- `End`

特殊规则：

- `SpaceBetween` 仅在可见子项数 `>= 2` 且有剩余空间时生效
- 如果剩余空间 `<= 0`，退化为 `Start`

### 7.5.7 Overflow 规则

如果固定尺寸子项总长度超过主轴可用长度：

- 本轮不 shrink
- 本轮不 wrap
- 仍按 `Start` 方向顺序排布
- 超出的绘制裁剪由现有显示/裁剪链处理

这样做的原因是：

- 保证第一版语义简单、可预测
- 避免在未引入 grow/shrink/wrap 前制造隐式尺寸修改

## 7.6 推荐实现方式

本轮推荐：

- 继续由 `ldWindow_on_frame_start()` 作为布局入口
- legacy 布局和 Flex 布局在这里统一分发
- 将 Flex 位置计算抽到独立内部函数

例如：

```c
static void ldWindowLayoutLegacy(ldWindow_t *ptWindow);
static void ldWindowLayoutFlex(ldWindow_t *ptWindow);
```

是否再抽成独立 `ldFlex.c` 文件，由实现时决定；但无论如何，本轮仍保持在 `src/gui/` 范围内。

## 7.7 Phase 1 验收标准

必须满足：

1. `row` 和 `column` 行为稳定
2. `padding`、`gap`、`align` 对直属可见子项生效
3. hidden 子项在 Flex 中不占位
4. 子项几何变化后，Flex 容器下一帧自动重排
5. 布局变化后 dirty region 无明显残影
6. 新增 layout demo 页面能稳定运行

---

## 8. 验证与回归要求

## 8.1 专用 layout demo

本轮必须新增一个专用 layout demo 页面，至少覆盖：

- row
- column
- padding
- gap
- main/cross align
- hidden 子项参与/不参与行为
- 容器 resize 后的自动重排

当前 widget 多页 demo 不作为主验证样本，只作为回归烟测。

## 8.2 必测场景

1. **直属子项语义**
   - 父容器只影响直属子项
   - 后代子树位置不被误改

2. **layout dirty 冒泡**
   - 子项 `resize`
   - 子项 `move`
   - 子项 `show/hide`
   - 节点增删
   - 父容器自身 `resize`

3. **dirty region**
   - 无重影
   - 无旧区域残留
   - `USE_SCENE_SWITCHING == 2` 下行为稳定

4. **回归**
   - 当前 widget 多页轮播可继续运行
   - scene switching 不被破坏

---

## 9. Grid 远期协商边界

这一章只做协商，不进入本轮开发。

## 9.1 本轮对 Grid 的明确结论

Grid 在本轮：

- **不开发**
- **不排期**
- **不验收**
- **不暴露公开 API**
- **不新增 `layoutGrid` 枚举**

## 9.2 为什么现在不做 Grid

Grid 依赖的基础能力明显高于本轮：

- 更稳定的直属子项语义
- 更稳定的 layout dirty 机制
- 更清晰的父子调度时序
- 更强的二维轨道计算能力
- 更复杂的 dirty region 影响范围

在 Phase 0 / Phase 1 尚未稳定前，Grid 只会把问题放大。

## 9.3 本轮允许为 Grid 保留的原则

本轮只允许做这类“不会扩大范围”的预留：

- 代码命名尽量使用通用布局术语
- `layout dirty` 机制按“未来可能有多种布局”设计
- layout demo 组织方式可为未来扩展留空间

本轮不允许做这类“名义预留、实质扩 scope”的事情：

- 先加 `layoutGrid`
- 先加 Grid setter
- 先加 row/col/span 字段
- 先加二维轨道模板数组

## 9.4 Grid 的未来前置条件

只有在以下条件全部满足后，才允许启动 Grid 新 spec：

1. Phase 0 / Phase 1 已完成并稳定
2. Flex 一维布局验证覆盖充分
3. layout dirty / dirty region 行为稳定
4. 直属子项语义与 hidden 策略已定型

---

## 10. 预计改动范围

预计会涉及以下文件：

- `src/gui/ldBase.h`
- `src/gui/ldBase.c`
- `src/gui/ldWindow.h`
- `src/gui/ldWindow.c`
- `src/gui/ldGui.h`（如需聚合新类型）
- `examples/common/demo/widget/*` 或新增独立 layout demo
- `examples/sdl/*`（如需接入 demo）
- `docs/tutorial/04 api.md`
- 相关教程或说明文档

本轮不应涉及：

- `src/layouts/`
- Grid 专属文件
- 大规模重构 scene 调度系统

---

## 11. 完成定义

只有满足下面条件，才能认为本轮开发完成：

1. Phase 0 与 Phase 1 功能全部落地
2. layout demo 可运行
3. 当前 SDL demo 无明显回归
4. API 文档与真实头文件同步
5. 本轮范围内没有残留“已声明但未实现”的 Grid 接口

---

## 12. 一句话收束

本轮开发的真正目标不是“把 Flex/Grid 框架一次做完”，而是：

> **先把 `ldWindow` 变成一个可靠的一维布局容器，并建立它与现有刷新系统之间的稳定契约。**
