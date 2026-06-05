# PicoUI a-0.3 truth-source 重建与 current-15 审计设计

> 日期：2026-05-31
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/picoui-serial/a-0.3/README.md`
> 目标：把 PicoUI 从“代码已继续前进，但 release truth 仍停在旧 `9` 控件世界”的状态，收敛到一个诚实、可验证、可继续推进的 `a-0.3` 基线：重建 truth-source、审计 current-15、冻结 `a-0.3` closeout 标准。

---

## 1. 最终目标与本阶段边界

最终大目标不变：

1. PicoUI 覆盖 LingDongGUI 当前 `26` 个可封装原生控件。
2. 每个 PicoUI public 控件最终都与对应 `ld*` 控件完整功能对齐。

但 `a-0.3` 本阶段**不负责直接做完全部对齐**。

`a-0.3` 的职责只有：

1. 把当前真实覆盖面写实。
2. 把当前 `15` 个已接入控件逐个审计写实。
3. 把 `a-0.3` 自己的完成标准钉死。

## 2. 当前问题定义

### 2.1 truth-source 失真

当前机器可读真相源仍停在旧 `J1`：

1. `wrapped = 9`
2. `not_wrapped = 17`
3. `parity_complete = 4`
4. `parity_incomplete = 5`

但当前开发线已经继续前进：

1. `a-01` 文档声明 backlog 五控件已按当前口径收口
2. `a-02` 文档声明六个新控件已完成 vertical slice

因此当前第一问题不是“下一个控件做谁”，而是：

**现在到底已经做到了什么，真相源说不准。**

### 2.2 current-15 尚未统一审计

当前至少可以确认已接入 public widget 的控件有：

1. `window`
2. `label`
3. `button`
4. `checkbox`
5. `switch`
6. `slider`
7. `text`
8. `image`
9. `list`
10. `progress_bar`
11. `qrcode`
12. `progress_wheel`
13. `message_box`
14. `date_time`
15. `clock`

但这 `15` 个控件当前并不处于同一完成层级：

1. `J线 4`：已按旧口径到 `parity_complete`
2. `a-01 5`：偏 shared/backlog 收口
3. `a-02 6`：偏 new widget minimal slice

如果不先审计 current-15，后面任何“加速推进”都会继续混写：

1. full parity
2. stable contract
3. minimal vertical slice

## 3. a-0.3 的唯一目标

`a-0.3` 只做三件事：

### 3.1 重建 current truth-source

至少要输出：

1. current PicoUI public widget 清单
2. 与 LingDongGUI `26` 控件的对应关系
3. 当前未覆盖控件清单
4. 旧 J1 matrix 为何过时

### 3.2 审计 current-15

必须逐控件回答：

1. public API 到了哪里
2. create_with_props/props 路径到了哪里
3. getter/readback 到了哪里
4. backend mapping 到了哪里
5. 当前哪些 capability 已完成
6. 当前哪些 capability 仍未完成

### 3.3 冻结 a-0.3 closeout 标准

必须明确：

1. `a-0.3` 完成时要交付哪些文档
2. `a-0.3` 完成时 matrix 要达到什么状态
3. `a-0.3` 完成时哪些脚本/检查必须通过
4. 哪些内容仍属于后续阶段，不得提前写成已完成

## 4. 中间态的严格定义

`a-0.3` 允许使用三类中间态标签：

1. `full parity complete`
2. `stable contract but not full parity`
3. `minimal vertical slice only`

但必须明确：

1. 这些标签只用于**当前盘点**
2. 这些标签只用于**说明缺口**
3. 这些标签不能被当作最终验收结论

## 5. 为什么 a-0.3 不能继续扩散

本阶段如果继续展开后续大版本 detailed spec/plan，会产生两个问题：

1. current truth 还没写实，后续大计划就会建在不稳定基线之上
2. 讨论面会从 current-15 审计扩散到未来 11 控件实现，反而降低当前节奏

所以 `a-0.3` 设计必须刻意收紧：

1. 不展开后续版本详细 spec
2. 不展开后续版本详细 plan
3. 不在本阶段新增高耦合实现任务

## 6. a-0.3 的交付物

本阶段必须交付：

1. `docs/picoui-serial/a-0.3/README.md`
2. `docs/picoui-serial/a-0.3/current-15-覆盖与分层真相源.md`
3. `docs/picoui-serial/a-0.3/current-15-capability-audit.md`
4. `docs/picoui-serial/a-0.3/a-0.3-closeout-标准.md`
5. `tests/picoui/contract/picoui_release_capability_matrix.json`
6. `tests/picoui/contract/check_picoui_release_capability_matrix.py`
7. `docs/superpowers/specs/2026-05-31-picoui-a-0-3-truth-source-and-current-15-design.md`
8. `docs/superpowers/plans/2026-05-31-picoui-a-0-3-truth-source-and-current-15-implementation.md`

## 7. a-0.3 严格串行阶段

### R1: current truth-source rebuild

输出：

1. current PicoUI public widget 清单
2. current widget total
3. current uncovered widget total
4. 更新后的 machine-readable matrix

### R2: current-15 parity stratification

输出：

1. current-15 三层分级
2. 每个控件当前层级
3. 每个控件的主要未完成项

### R3: current-15 capability audit

输出：

1. widget-by-widget capability checklist
2. 明确支持项
3. 明确缺口项
4. 明确不能夸大的项

### R4: truth-source/docs/gate consistency repair

输出：

1. matrix 与文档术语对齐
2. gate 与 matrix 术语对齐
3. README / spec / audit 三者对齐

### R5: a-0.3 closeout freeze

输出：

1. `a-0.3` 结束条件
2. `a-0.3` 不等于什么
3. 后续阶段输入清单

## 8. 并行与串行执行模型

`a-0.3` 的执行模型必须明确，不允许执行时再临场改拓扑。

### 8.1 必须串行的阶段

以下阶段必须由主线程串行收敛：

1. `R1 current truth-source rebuild`
2. `R2 current-15 parity stratification`
3. `R4 truth-source/docs/gate consistency repair`
4. `R5 a-0.3 closeout freeze`

原因：

1. 这些阶段都以单一 truth-source 为中心。
2. 这些阶段都会修改共享文档口径或共享 matrix 口径。
3. 并行改很容易把 current total、layer 名称、closeout 标准写炸。

### 8.2 允许并行的阶段

只有 `R3 current-15 capability audit` 允许并行。

并行拆分固定为三块：

1. `J线 4`
2. `a-01 5`
3. `a-02 6`

原因：

1. 这三组来源不同。
2. 当前 audit 的主要写面可以拆开。
3. 主线程最终再把审计结果合并回统一 truth-source。

### 8.3 并行时的硬边界

并行 subagent 必须遵守：

1. 不改同一段 matrix 主结构。
2. 不同时改同一份总索引。
3. 不自己改 closeout 标准。
4. 只负责分配到的控件组审计结论。

### 8.4 主线程职责

主线程必须负责：

1. 决定 current truth 总数
2. 决定三层中间态定义
3. 合并 `R3` 各组 audit 结果
4. 收敛 matrix / gate / docs
5. 最终验证

## 9. 成功标准

`a-0.3` 成功时必须满足：

1. 当前 PicoUI public widget 总数不再含糊。
2. current-15 全部进入 truth-source。
3. current-15 每个控件都有诚实 capability audit。
4. machine-readable matrix、gate、中文文档三者一致。
5. `a-0.3` closeout 标准冻结完成，并以 `docs/picoui-serial/a-0.3/a-0.3-closeout-标准.md` 为最终判定入口。

## 10. 失败判定

以下情况视为 `a-0.3` 失败：

1. 继续沿用旧 `9` 控件 truth-source 不修
2. 把 `a-02` 的 six-slice 直接写成 full parity
3. 只写宏观路线，不写 current-15 审计
4. 在 current truth 还没写实前展开后续大版本详细 spec/plan
5. 把中间态长期保留为最终口径
