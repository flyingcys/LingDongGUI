# PicoUI B线可见 UI 收口设计文档

> 日期：2026-05-27  
> 适用仓库：`/Users/cys/embedded/LingDongGUI`  
> 目标：把 `PicoUI` 从“backend 主线已通、smoke 可出图”的状态，推进到“真实窗口里 UI 可正常显示、可读、可判定”的状态。

---

## 1. 背景

`A线` 已完成 `PicoUI -> LingDongGUI` 的 backend 主线收口：

- 基础控件、布局、事件、theme 已有真实 backend 映射
- `runtime/capture` 已能证明 demo 可启动、可出首帧、可回归
- `backend_app.c` 已收缩到 `temporary smoke path / host harness`

但 `A线` 的成功边界从一开始就不是“真实窗口里 UI 已经达到用户可接受的显示质量”。`A线` 证明的是：

1. `PicoUI` 不再主要依赖 fake renderer 主输出
2. demo 能启动
3. capture 能出图
4. backend 主线、layout/event/theme 的基本映射已闭环

这不等于：

1. 真实窗口里没有黑屏/近黑屏/错色问题
2. 用户看到的 UI 已经可读
3. 显示结果与控件树语义一致
4. smoke 证据已经上升为 visible correctness

---

## 2. 当前新事实

`2026-05-27` 对 `picoui_basic_widgets_demo` 的复核，给出了 `A线` 之后必须切到 `B线` 的直接证据：

1. 用户在真实运行时反馈“UI 一片黑”。
2. dummy runtime capture 证明该 demo 并非完全无像素。
3. 首帧图像显示：
   - 画面存在重复列/重复控件现象
   - 画面配色与可读性异常
4. 这说明当前问题已从“backend 主线是否存在”切换为“真实可见结果是否可信”。

因此，下一条主线不应继续优先扩控件/API/theme 能力面，而应先把可见 UI 收口成稳定基线。

---

## 3. B线唯一目标

**B线唯一目标**：建立并收口 `PicoUI` 的 visible correctness，使 demo 在真实窗口里达到“可显示、可读、可判定”的状态。

这里的“visible correctness”至少包含三层：

1. **能显示**
   - 不是纯黑/纯空/只有背景色
   - 不是只在 capture 里有内容、真实窗口里近黑或不可见
2. **可读**
   - 文字、按钮、控件边界、状态对比可被人眼区分
   - 颜色组合不会造成“虽然有像素，但看起来像坏图/脏图/黑屏”
3. **可判定**
   - 显示结果与预期控件树和布局语义一致
   - 不出现重复列、重复控件、错误布局、错误占位等会误导验收的问题

---

## 4. 为什么 B线 必须先于其他工作

当前最容易误判的方向，是把 `A线` 的 smoke 绿灯继续外推成“可以开始做更多 PicoUI 功能”。这在 `B线` 启动时是错误顺序，原因有四个：

1. **用户感知已经给出反证**
   - 用户直接运行 `picoui_basic_widgets_demo` 时看到的是“黑屏感”，这比脚本绿灯更接近真实产品输入。
2. **当前 visible 证据与 smoke 证据错位**
   - capture 非空、runtime 通过，并不能解释“为什么用户看到近黑或不可读画面”。
3. **如果先扩能力面，会把 visible correctness 问题扩散到更多 demos**
   - 新控件、新主题、新 demo 都会建立在不稳定的显示基线上。
4. **B线 是后续任何更高层能力的基础门**
   - 如果最小 `basic_widgets` 样本都不能稳定可见，后续 theme show、settings panel、更多控件能力都缺少可信验收基线。

所以 `B线` 的优先级高于继续扩控件、高于继续扩 theme 能力、高于继续做更复杂 demo。

---

## 5. B线 的核心诊断对象

### 5.1 一号样本：`picoui_basic_widgets_demo`

这是 `B线` 的最小可信样本，原因：

- 同时覆盖 `switch/checkbox/slider/button/text/image`
- 足够小，便于快速复现和比对
- 足够复杂，能暴露真实可见问题

`B线` 不应一开始就平均铺向所有 demos，而应先把 `basic_widgets` 收成真正的 visible baseline。

### 5.2 关键风险点

`B线` 当前主要风险集中在三类：

1. **显示链风险**
   - 真实窗口显示链
   - capture/readback 链
   - SDL host present 链
   - 这些路径可能并不完全同构
2. **颜色链风险**
   - theme token
   - backend style apply
   - `LingDongGUI` 底层颜色表达
   - SDL 呈现/读回格式
3. **布局/对象树呈现风险**
   - 控件树逻辑上正确，但最终呈现重复或错位
   - dummy capture 能看见像素，但实际窗口结果不可信

---

## 6. B线 分层边界

### 6.1 B线 要解决的事情

- visible evidence 建立
- 窗口显示链与 capture 链一致性
- 可读性和配色基线
- 最小 demo 的可见正确性
- 多 demo 的 visible gate

### 6.2 B线 不解决的事情

- 新增 PicoUI 控件种类
- 大幅扩展 public API
- 继续扩 `theme v0` 以外的复杂样式系统
- 回到 fake renderer 主导 UI 产出
- 用 demo 侧硬编码掩盖 visible correctness 缺口

### 6.3 与 A线 的关系

- `A线` 的重点是 backend correctness
- `B线` 的重点是 visible correctness

两者不是替代关系，而是顺序关系：

1. 先有 `A线`
2. 再做 `B线`
3. `B线` 完成后，后续新能力线才能在可信显示基线上展开

---

## 7. B线 分阶段设计

### 7.1 `B0` 现状复核与验收边界冻结

目的：

- 固定 `B线` 只处理 visible correctness
- 统一 smoke 与 visible evidence 的边界

完成标志：

- 文档明确区分：
  - backend correctness
  - smoke evidence
  - visible correctness

### 7.2 `B1` 真实可见证据链建立

目的：

- 建立专门面向“人眼可见结果”的验证链

设计要求：

- 不能只看 `capture` 是否非空
- 必须能回答：
  - 真实窗口是否有可读内容
  - 显示结果是否和预期结构一致
  - 哪些异常属于黑屏、近黑、错色、重复列、错误布局

完成标志：

- `basic_widgets` 有统一 visible evidence 入口
- 之后所有 demos 都可以复用同一 visible 检查框架

### 7.3 `B2` 窗口显示链与颜色链纠偏

目的：

- 查清真实窗口与 capture 之间的偏差
- 修正颜色/显示语义错位

设计要求：

- 优先查 host present 路径
- 优先查 capture readback 路径
- 优先查 `theme -> backend style -> LingDongGUI -> SDL` 的颜色传递

完成标志：

- 不再出现“capture 看起来有东西，但真实窗口像黑屏/近黑屏”的系统性分裂

### 7.4 `B3` `basic_widgets` visible baseline 收口

目的：

- 先把 `basic_widgets` 做成第一个真正可见样板

设计要求：

- 消灭重复列/重复控件/明显错位
- 保证基础控件可读、可判定
- `image` 的当前状态必须明确，不能继续制造误判

完成标志：

- 用户不再把 `basic_widgets` 描述成黑屏/近黑屏/不可读

### 7.5 `B4` 多 demo visible correctness 铺开

目的：

- 把 `basic_widgets` 经验推广到所有 `picoui` demos

设计要求：

- 从 `hello_world` 到 `settings_panel` 顺序铺开
- `settings_panel` 仍作为后段样本，避免它反向拖垮 `basic_widgets` 基线

完成标志：

- 6 个 `picoui` demos 都达到同级 visible correctness

### 7.6 `B5` visible gate 与 closeout

目的：

- 把 `B线` 验收门禁固化

设计要求：

- 文档写清：
  - smoke gate
  - visible gate
- 测试链能拦住：
  - 黑屏
  - 近黑屏
  - 错色
  - 重复列
  - 结构与预期不一致

完成标志：

- 后续任何新能力线，都不再需要先回头解决 visible baseline

---

## 8. Subagent 推进模型

`B线` 需要拆成**主线程决策 + 子线程定点推进**的模式。

### 8.1 必须主线程负责的事情

- 现象归因与优先级判断
- 阶段门禁判断
- 最终可见证据收口
- 跨写面整合

### 8.2 适合 subagent 推进的事情

1. **visible evidence 组**
   - `tests/picoui/runtime/*`
   - 文档中的 evidence 定义
2. **display/color path 组**
   - `picoui/src/backend/ldgui/backend_app.c`
   - 颜色/显示相关辅助逻辑
3. **basic_widgets demo 组**
   - `picoui/demo/basic_widgets/main.c`
   - 与其直接相关的最小验证
4. **doc gate 组**
   - `docs/picoui-serial/*`
   - `docs/superpowers/specs/*`
   - `docs/superpowers/plans/*`

### 8.3 串并边界

必须串行：

1. `B1` 必须先于 `B2`
2. `B2` 必须先于 `B3`
3. `B3` 必须先于 `B4`
4. `B4` 必须先于 `B5`

可并行但必须非重叠：

- 在某一阶段内，文档整理可与只读复核并行
- 同一阶段内，测试写面与 demo 写面可并行，但不能同时改同一文件

---

## 9. 验收原则

`B线` 的验收原则比 `A线` 更接近用户真实体验：

1. **用户看到什么，比脚本看到什么更重要**
2. **capture 非空，不等于显示正确**
3. **有颜色，不等于可读**
4. **可见 UI 必须和预期控件树一致**

---

## 10. 最终结论

`A线` 之后，最应该做的事情已经不是继续扩 `PicoUI` 的功能面，而是建立并收口 `B线`：

- 先解决 visible correctness
- 先修真实窗口显示链与颜色链
- 先把 `basic_widgets` 收成可信样板

在 `B线` 完成之前，任何“继续做更多 PicoUI 能力”的投入，都会建立在不稳定、不可见、不可读或不可判定的基础上，优先级都应后置。
