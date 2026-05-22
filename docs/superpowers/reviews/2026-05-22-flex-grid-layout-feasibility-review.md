# LingDongGUI Flex/Grid 技术方案学习版可行性审查

**日期**：2026-05-22  
**目标文档**：`docs/superpowers/specs/2026-05-22-flex-grid-layout-design.md`  
**定位**：学习文档 / 技术审查笔记  
**原则**：不修改原 spec，只基于当前仓库真实代码和文档做可行性分析

---

## 1. 这份文档解决什么问题

原始 spec 的核心目标是：为 LingDongGUI 引入类似 LVGL 的 Flex/Grid 布局能力。

这个方向本身没有问题，但如果直接把原 spec 当成实现蓝图，容易产生一个误解：

- **LingDongGUI 是不是现在连容器都还没有？**

答案是否定的。

更准确的表述是：

- **LingDongGUI 现在已经支持容器关系**
- **但还不支持一个成熟、可靠、可扩展的通用容器布局系统**

这份文档的目的，就是把这两个概念拆开讲清楚，并解释为什么在做 Flex/Grid 之前，必须先补“容器布局基础设施”。

---

## 2. 先回答核心问题：LingDongGUI 当前支不支持容器

### 2.1 支持，而且是实打实的树形容器

当前框架已经具备明确的父子树结构：

- `ldBase_t` 直接继承 `arm_2d_control_node_t`，天然具备 `ptParent / ptChildList / ptNext` 这套树节点关系  
  参考：`src/gui/ldBase.h:200`
- `ldBaseNodeAdd()`、`ldBaseNodeRemove()` 负责把控件挂入或移出树  
  参考：`src/gui/ldBase.c:166-204`
- `ldWindow_init()` 会根据 `parentNameId` 把当前 window 挂到父节点下  
  参考：`src/gui/ldWindow.c:91-96`

也就是说，LingDongGUI 不是“没有容器”，而是已经有了：

- 父子嵌套
- 相对父节点坐标
- 树状遍历
- 容器内挂载子控件
- 复合型控件组合

### 2.2 当前仓库里已经有容器嵌套的真实例子

真实代码里已经在使用容器：

- `Page1` 中创建了一个 `ldWindow`，并在这个 window 下面再挂一个 `ldLabel`  
  参考：`examples/common/demo/widget/uiWidgetPage1.c:115-123`
- `Page4` 中在 `ldList` 容器里嵌入了一个 `ldButton` 作为 item widget  
  参考：`examples/common/demo/widget/uiWidgetPage4.c:136-145`

这说明当前框架已经能做：

- 背景 + 子控件的分层
- 复合控件
- 嵌套容器
- 列表项内嵌子控件

### 2.3 真正缺的不是“容器”，而是“布局容器”

需要把两个概念分开：

| 能力 | 当前状态 | 含义 |
|------|----------|------|
| 容器关系 | 已支持 | 可以挂子节点、嵌套、遍历、绘制 |
| 手工绝对坐标布局 | 已支持 | 子控件手工写 `x/y/w/h` |
| 简单自动布局 | 有，但很弱 | `ldWindow` 仅支持水平/垂直等比分槽 |
| 成熟布局容器 | 还不具备 | 缺少稳定的布局语义、失效传播、统一属性模型 |

一句话概括：

> **LingDongGUI 现在支持 parent-child container，但还不支持成熟的 layout container。**

---

## 3. 当前容器能力到底够做哪些 UI

### 3.1 当前能力能支撑的场景

当前代码已经证明，LingDongGUI 适合这些场景：

- 绝对坐标驱动的页面
- 中小规模的手工摆放界面
- 复合型控件
- 容器内挂若干固定位置子控件
- 列表、组合框、消息框这类控件内部的专用布局

例如当前 SDL widget demo 已经拆成多页，并且本地可以编译运行；但这些页面本质仍然是绝对坐标摆放，不是通用容器布局示例。

- 页面入口：`examples/common/demo/widget/uiWidget.h:12`
- 当前屏幕配置：`examples/sdl/user/ldConfig.h:180-193`
- 页面示例：`examples/common/demo/widget/uiWidgetPage1.c:60-125`、`examples/common/demo/widget/uiWidgetPage4.c:55-171`
- SDL 构建入口：`examples/sdl/makefile`

### 3.2 当前能力还不适合的场景

下面这些是当前框架明显不适合直接承接的：

- 工具栏/导航栏自动分布
- 卡片流式布局
- 容器尺寸变化后自动重排
- 嵌套容器中的一致性布局
- 根据子项尺寸反推容器布局
- 通用 `padding / gap / align / grow / wrap`

原因不是“绘制做不到”，而是“布局契约还没建好”。

### 3.3 用 5 层视角看当前架构

如果只看“有没有容器”，很容易把问题看浅。更适合学习这件事的方式，是把当前 LingDongGUI 拆成 5 层：

1. **树结构层**
   - `ldBase_t` 继承 `arm_2d_control_node_t`
   - 已经有 `ptParent / ptChildList / ptNext`
   - 这一层解决的是“控件能不能形成容器树”
   - 参考：`src/gui/ldBase.h:200-217`

2. **几何层**
   - 每个控件都有自己的 `tRegion`
   - 也有 `ldBaseSetRegion()`、`ldBaseMove()`、`ldBaseResize()` 这类几何 setter
   - 这一层解决的是“控件的坐标和尺寸能不能改”
   - 参考：`src/gui/ldBase.c:403-413`、`src/gui/ldBase.c:1131-1218`

3. **容器布局层**
   - 当前真正接近“通用布局”的，主要就是 `ldWindow_on_frame_start()`
   - 它负责 legacy 的水平/垂直均分
   - 这一层解决的是“父容器是否会主动替子项排位置”
   - 参考：`src/gui/ldWindow.c:161-235`

4. **布局失效层**
   - 这一层现在并不完整
   - 子项几何变化后，父容器不会自动知道“我需要重新布局”
   - 这一层解决的是“什么时候应该重排”
   - 参考：`src/gui/ldWindow.h:63-66`、`src/gui/ldBase.c:1131-1218`

5. **局部刷新层**
   - 当前局部刷新链路是有的，但不是专门为布局系统设计的
   - 布局变了以后，dirty region 怎么提交给 scene / PFB，要明确接入方式
   - 这一层解决的是“重排之后怎么正确刷新屏幕”
   - 参考：`src/gui/ldGui.c:195-231`、`src/gui/ldScene0.c:176-243`、`src/gui/ldScene1.c:176-243`

把这 5 层连起来看，问题就清楚了：

- 当前 **第 1 层和第 2 层已经存在**
- 第 3 层只有一个很薄的 legacy 版本
- 第 4 层和第 5 层对布局系统来说还没有形成完整契约

所以真正缺的不是“容器”，而是：

> **从树结构层一路打通到局部刷新层的完整布局基础设施。**

---

## 4. 当前 `ldWindow` 自动布局是什么水平

### 4.1 只有非常薄的一层 legacy 自动布局

当前自动布局能力全部集中在 `ldWindow_on_frame_start()`：

- `layoutNone`
- `layoutHorizontal`
- `layoutVertical`

参考：

- `src/gui/ldBase.h:234-238`
- `src/gui/ldWindow.h:57-66`
- `src/gui/ldWindow.c:161-235`

它的本质是：

- 先按直属子节点数量算出等比分槽
- 再把子项放到各自槽位里
- 可选地读取 `pLayoutPaddingGroup[count]`，对每个 slot 应用 padding

这个能力可以叫“容器内均分摆放”，但离 Flex 还很远。

### 4.2 它不是一个干净、稳定的布局基线

这里是原 spec 最大的技术乐观点之一。

原 spec 把现状概括为“`ldWindow` 对子项做简单等分布局”，这个说法不够准确，因为真实实现里有两个关键偏差：

1. **分母用的是直属子节点数**
   - `ldBaseGetChildCount()` 只统计 `ptChildList` 这一层  
     参考：`src/gui/ldBase.c:727-736`

2. **重排遍历却是从第一个子节点开始做 `PREORDER_TRAVERSAL`**
   - 这会把后代子树也一起枚举进来  
     参考：`src/gui/ldWindow.c:177-230`

这意味着：

- 当前布局语义并不是“只排直属子项”
- 一旦容器里再嵌套容器，legacy 行为就会变得不稳定

所以，**现有 `ldWindow` 自动布局不能直接视作 Flex 的可靠基础层**。

---

## 5. 为什么原 spec 不能直接当实现蓝图

下面这些不是细节差异，而是决定实施路径的关键前提。

### 5.1 相对坐标模型没有 spec 写得那么“现成”

原 spec 认为现有 `ldBaseSetRegion()` 已处理相对坐标，这是不准确的。

真实代码里：

- `ldBaseSetRegion()` 只是直接覆盖 `tRegion`
- 并用 `tTempRegion` 记录新旧包围盒
- 不负责替调用者做父子坐标换算

参考：

- `src/gui/ldBase.c:1206-1218`

当前 `ldWindow` 自动布局在调用 `ldBaseSetRegion()` 之前，仍然要手工减去容器绝对坐标：

- `src/gui/ldWindow.c:204-207`
- `src/gui/ldWindow.c:224-227`

所以正确的理解应当是：

> **LingDongGUI 现有 API 支持存储相对父坐标，但并没有提供一个“自动处理布局坐标转换”的通用层。**

### 5.2 当前没有真正的 layout dirty 传播机制

原 spec 把 `isDirtyRegionUpdate` 近似当成“布局失效传播”机制，这也是不准确的。

真实情况是：

- `isDirtyRegionUpdate` 属于绘制脏区更新
- `isLayoutUpdate` 只存在于 `ldWindow_t`
- `ldBaseResize/SetX/SetY/SetWidth/SetHeight/SetRegion` 只会把当前控件标脏
- 不会自动回溯让父容器重新布局

参考：

- `src/gui/ldBase.h:200-217`
- `src/gui/ldWindow.h:63-66`
- `src/gui/ldBase.c:1131-1218`
- `src/gui/ldWindow.c:346-366`

这意味着，如果未来引入 Flex：

- 子项几何变化
- 子项隐藏/显示
- 子项增删
- 父容器尺寸变化

这些动作都需要一个明确的“父容器布局失效”机制，而当前仓库没有这层通用基础设施。

### 5.3 `frameStart` 时序和 LVGL 参考模型并不一致

原 spec 参考的是 LVGL 的 bottom-up 布局思路，但当前 LingDongGUI 的 `frameStart` 遍历是：

- `PREORDER_TRAVERSAL`
- 即父先子后

参考：`src/gui/ldGui.c:214-231`

这在当前简单布局下问题不大，但如果要做：

- 嵌套 Flex
- 内容驱动尺寸
- Grid
- 容器依赖子项最终尺寸再布局

父先子后会成为结构性风险。

换句话说：

> **现在的挂点是对的，但现在的时序不一定够用。**

### 5.4 PFB / dirty region 接入没有 spec 写得那么顺

原 spec 提到可以调用 `ldGuiWidgetUpdate()` 或类似能力来联动 PFB 脏区。

但真实代码中：

- 当前公开接口是整场景级 `ldGuiUpdateScene()`  
  参考：`src/gui/ldGui.c:195-198`
- widget 级 dirty region 更新依赖 scene 层枚举 `isDirtyRegionUpdate` 和 `tTempRegion`  
  参考：`src/gui/ldScene0.c:176-243`、`src/gui/ldScene1.c:176-243`

所以布局系统不能只“算出新坐标”就结束，还必须明确：

- 谁负责把布局变化写回 dirty region
- 是局部更新，还是整场景刷新
- 哪些情况下需要扩大包围盒

### 5.5 公开 API 与工具链导出链没有打通

原 spec 希望把布局能力拆到 `src/layouts/`，这在工程分层上是合理的，但当前仓库的真实对外链路并不是这么工作的。

当前 API / 模板 / 文档导出链主要围绕 `src/gui/`：

- `ldGui.h` 是聚合入口  
  参考：`src/gui/ldGui.h`
- `widgetCreate.py` 自动把新控件头文件插入 `ldGui.h`  
  参考：`src/template/widgetCreate.py:67-83`
- `generate_api.py` 只扫描 `src/gui/*.c` 生成 `04 api.md`  
  参考：`docs/tutorial/generate_api.py:213-260`
- 页面模板和教程也是围绕 `ldGui.h + void *obj + nameId/parentNameId` 的使用方式  
  参考：`src/template/uiTemplate.c:20-31`、`docs/tutorial/05 development.md:274-301`

如果新布局 API 直接放到 `src/layouts/`，但不改上述链路，会发生这些问题：

- API 不会自动进入 `ldGui.h`
- 文档生成脚本不会收录
- 模板不会感知
- 教程示例继续与真实 API 脱节

所以这不是一个“补几个 include”就能忽略的小问题。

### 5.6 向后兼容的表述偏乐观

原 spec 里还有一个表述需要特别小心：

- 把 `layoutTpye -> layoutType` 改名
- 在 `ldBase_t` / `ldWindow_t` 尾部加字段
- 然后说“向后兼容”

这个说法对“源码名兼容”也许能部分成立，但对“结构布局兼容”并不成立。

原因是：

- `ldBase_t`、`ldWindow_t` 是公开基类
- 很多控件直接 `implement(ldBase_t)` 或 `implement(ldWindow_t)`
- 新增字段、位域、指针都会改变内存布局

参考：

- `src/gui/ldBase.h:200-217`
- `src/gui/ldWindow.h:57-66`
- `src/gui/ldButton.h:53`
- `src/template/widgetTemplate.h:49`

因此更严谨的说法应该是：

- **最多做到源码级兼容**
- **不能轻易宣称 ABI / struct layout 兼容**

---

## 6. “支持容器”和“支持成熟布局容器”的真正差别

这是学习这件事最关键的一节。

### 6.1 只支持容器关系，意味着什么

如果一个 GUI 框架只是支持容器关系，它通常能做到：

- A 是 B 的父节点
- B 的坐标相对 A
- A 绘制时顺便绘制 B
- A 可以持有多个子控件
- 通过手工坐标把页面搭出来

LingDongGUI 当前已经达到这个层级。

### 6.2 支持成熟布局容器，意味着什么

成熟布局容器至少要再多出下面这些能力：

- **清晰的布局语义**  
  只排直属子项，不误伤后代子树

- **统一的布局属性模型**  
  `padding / gap / align / grow / wrap / track`

- **确定的失效传播规则**  
  子项变化后，父容器知道自己该重排

- **稳定的嵌套时序**  
  父子容器重排顺序可预期

- **与局部刷新系统正确联动**  
  改布局后既不漏刷，也不乱刷

- **对外 API 和工具链一致**  
  头文件、模板、教程、API 文档导出都同步

这就是为什么：

> **“已经有容器”不等于“已经能无痛上 Flex/Grid”。**

---

## 7. 为什么做 Flex 前必须先补容器布局基础设施

如果直接跳过基础设施补丁，开始实现 Flex/Grid，短期内可能会出现这些情况：

- 简单 demo 能跑
- 嵌套容器时布局结果变怪
- 改一个子项尺寸，父容器不重排
- dirty region 漏刷新或出现残影
- API 看起来能用，但模板和文档全没跟上
- legacy 布局和新布局的边界越来越乱

所以真正稳妥的做法不是“直接实现新布局算法”，而是按下面顺序推进：

1. 先把 legacy 容器布局语义收干净  
2. 再把 layout dirty 模型补出来  
3. 再决定是否需要改 `frameStart` 时序  
4. 然后才是最小 Flex  
5. Grid 放到更后面

这也是为什么我把原始 spec 的状态判断，从：

- “完全可行，推荐分阶段实施”

收紧成：

- “方向可行，但当前只能作为架构方向稿，不能直接当实现稿”

---

## 8. 更现实的实施路径：Phase 0 / Phase 1

### 8.1 Phase 0：先清基线

建议先做这些，不碰 Grid：

1. **修正 legacy `ldWindow` 布局语义**
   - 只处理直属子项
   - 明确隐藏项是否占位

2. **补 layout dirty 机制**
   - 明确哪些动作会让父容器重新布局
   - 例如：子项增删、几何变化、隐藏显示、父容器尺寸变化

3. **明确 dirty region 接入规则**
   - 布局变化如何更新 `tTempRegion`
   - 哪些情况走局部刷新
   - 哪些情况需要 `ldGuiUpdateScene()`

4. **明确兼容边界**
   - `layoutHorizontal/Vertical` 如何保留
   - `pLayoutPaddingGroup` 与未来容器 `padding` 的关系
   - 兼容目标是源码兼容还是结构布局兼容

### 8.2 Phase 1：最小可落地 Flex

第一阶段建议只做一维 Flex 基础版：

- `row`
- `column`
- 容器 `padding`
- `gap`
- 主轴/交叉轴基础对齐
- 固定尺寸 item

先**不做**：

- `wrap`
- `grid`
- `content-size`
- 百分比尺寸
- `grow/shrink` 的复杂策略
- colspan / rowspan

这样做的价值是：

- 能覆盖最常见的一维布局需求
- 算法复杂度和集成风险都可控
- 可以先把容器基础设施验证扎实

#### 8.2.1 为什么第一版只做 `row / column + gap + padding + align`

这不是为了“保守”，而是为了让第一版真正可验证。

先做这 4 类能力，原因分别是：

- **`row / column`**
  - 它只要求一维主轴布局
  - 不会立刻把问题升级成二维轨道系统
  - 适合先验证“直属子项语义”和“父容器重排机制”

- **`gap`**
  - 它是最基础、最常见的间距需求
  - 对算法影响可控，不会引入复杂依赖
  - 适合先替代当前零散的手工间距写法

- **`padding`**
  - 它能把“容器内边距”从子项坐标里分离出来
  - 是从“手工绝对定位”走向“容器规则定位”的关键一步
  - 也能帮助和当前 `pLayoutPaddingGroup` 的 legacy 语义做切分

- **`align`**
  - 它能验证主轴/交叉轴对齐逻辑是否稳定
  - 这是 Flex 真正有别于 legacy 等比分槽的最小价值点

而这些能力暂时不应该先做：

- **`wrap`**
  - 一旦加入换行，布局就从“一条主轴”变成“多轨道”
  - 会立即依赖更复杂的 track 计算和 dirty 行为

- **`grid`**
  - 它不是一维布局增强，而是二维布局系统
  - 对时序、轨道、span、内容尺寸依赖都更高

- **`content-size` / 百分比尺寸 / 复杂 grow-shrink**
  - 这些能力都要求更稳定的父子重排契约
  - 在当前基础设施还没补齐时，会放大集成风险

所以第一版应优先回答的，不是“功能够不够全”，而是：

> **当一个父容器替直属子项做一维排布时，语义、失效传播和刷新链路是否已经稳定。**

### 8.3 Phase 2：再看是否需要 Grid

只有在下面这些都稳定后，Grid 才值得进场：

- 直属子项布局语义稳定
- layout dirty 传播稳定
- 嵌套容器行为稳定
- dirty region 联动稳定
- Flex demo 和回归测试稳定

否则 Grid 会把问题从“一维容器不稳”放大成“二维系统性不稳”。

---

## 9. 建议的验证清单

如果未来真的要推进实现，验证必须和当前 spec 写法不同，不能只靠“SDL demo 能跑”。

### 9.1 容器语义验证

- 直属子项布局时，不应重排后代子树
- 嵌套 window / list / message box 时行为稳定
- 隐藏项是否占位要有明确且可重复的结果

### 9.2 失效传播验证

- 子项 `resize`
- 子项 `move`
- 子项 `show/hide`
- 子项增删
- 父容器自身 `resize`

以上动作都要确认父容器是否按预期重排。

### 9.3 dirty region / PFB 验证

- 布局变化后无重影
- 无旧区域残留
- `USE_SCENE_SWITCHING == 2` 下切页后仍稳定

相关代码路径：

- `src/gui/ldGui.c:214-231`
- `src/gui/ldScene0.c:176-243`
- `src/gui/ldScene1.c:176-243`

### 9.4 示例验证

建议新增一个专门的 layout demo 页面，单独验证：

- row / column
- padding / gap
- 嵌套容器
- 动态改尺寸
- 隐藏显示

当前多页 widget demo 只适合做：

- smoke test
- scene switching 回归
- 现有控件兼容性回归

不适合作为布局系统的主验证样本。

---

## 10. 本次学习的几个关键结论

### 10.1 结论一：LingDongGUI 不是没有容器

它已经有：

- 树结构
- 父子关系
- 嵌套容器
- 复合控件
- 绝对坐标页面组织能力

### 10.2 结论二：当前缺的是“容器布局基础设施”

主要缺口有：

- 干净的直属子项布局语义
- layout dirty 传播
- 稳定的父子布局时序
- 与 dirty region 的可靠联动
- 对外 API / 模板 / 文档导出的一致性

### 10.3 结论三：原 spec 有价值，但现在更像方向稿

它适合回答：

- 这个方向值不值得做
- 参考谁做
- 大致会有哪些模块

但它还不适合直接回答：

- 现在就该怎么编码
- 第一版到底实现到哪
- 兼容边界怎么定义
- 测试口径怎么落

### 10.4 结论四：最稳的路线不是“直接 Flex/Grid”

而是：

1. 先修 legacy 容器布局基线  
2. 再补 layout dirty 基础设施  
3. 先做最小一维 Flex  
4. 最后再评估 Grid

---

## 11. 建议你接下来怎么读

如果你想继续深入这个主题，建议按下面顺序读：

1. 原始方案文档  
   `docs/superpowers/specs/2026-05-22-flex-grid-layout-design.md`

2. 当前 window 自动布局真实实现  
   `src/gui/ldWindow.c:161-235`

3. 基础树结构与几何 setter  
   `src/gui/ldBase.h:200-217`  
   `src/gui/ldBase.c:727-736`  
   `src/gui/ldBase.c:1131-1218`

4. 当前 frameStart 和局部刷新路径  
   `src/gui/ldGui.c:214-231`  
   `src/gui/ldScene0.c:176-243`  
   `src/gui/ldScene1.c:176-243`

5. 当前模板与对外 API 导出方式  
   `src/template/uiTemplate.c`  
   `src/template/widgetCreate.py:67-83`  
   `docs/tutorial/generate_api.py:213-260`

读完这几组文件后，再回看原 spec，会更容易看出：

- 哪些部分是方向正确
- 哪些部分是条件不够
- 哪些部分需要在实施前先改写

---

## 12. 最后的收束

把这次分析压缩成一句话：

> **LingDongGUI 当前已经支持容器，但还没有把容器提升为可承载 Flex/Grid 的成熟布局容器。**

因此，真正应该先做的不是“再加一个布局模块”，而是：

> **先把容器布局基础设施补齐，再谈 Flex，最后再谈 Grid。**
