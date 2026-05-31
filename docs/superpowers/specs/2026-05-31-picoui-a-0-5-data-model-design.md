# PicoUI a-0.5 数据 / edit model 串行设计

## 1. 背景

`a-0.5` 不是简单“再加三个控件”。它建立在 `a-0.4` 已经收口以下能力之上：

1. focus ownership
2. editable text contract
3. keyboard bridge
4. selection / navigation contract
5. honesty / readback policy

如果这些前置能力没有先稳定，`table` 一接入就会把 `0.4` 返工。

## 2. 目标

`a-0.5` 的唯一目标：

1. 建立 PicoUI 数据模型 / 可编辑单元格 shared model
2. 在这套 shared model 上串行接入 `table / graph / calendar`
3. 让数据类控件从第一天起就走真实 model、真实 readback、真实 frame/event 一致性

## 3. 非目标

`a-0.5` 明确不做：

1. `arc / gauge / icon_slider / radial_menu`
2. 把 `graph` 做成静态画图 demo
3. 把 `calendar` 做成只显示、不承诺真实 date contract 的壳
4. 复制 host cache getter 模式

## 4. 方案比较

### 方案 A：三控件平均推进

优点：

1. 表面上覆盖率前进更快

缺点：

1. `table` 的 model/edit 问题最晚才暴露
2. `graph`、`calendar` 很容易各自造数据结构
3. 共享的 readback 与 frame consistency 无法统一

结论：拒绝。

### 方案 B：shared model 先行，从 `table` 起步

优点：

1. 最早暴露 `0.4` 输入 shared-core 是否够用
2. `graph` 与 `calendar` 都可以复用 model / truth policy
3. 更容易把 gate 拆成“存在/可见”和“数据一致性”

缺点：

1. 前期 shared 文档和测试工作较重

结论：推荐。

### 方案 C：先做 `graph` 或 `calendar`

优点：

1. 视觉上更容易看到新 demo

缺点：

1. 无法验证 edit model
2. 容易把 `table` 的最难 shared 问题留到最后

结论：不推荐。

## 5. 推荐架构

### 5.1 先定义 shared model，不先推控件

`a-0.5` 必须先收口：

1. item model
2. editable cell contract
3. graph series / value model
4. calendar date / header / grid contract

其中最优先的是：

1. item model
2. editable cell contract
3. `0.4` 输入 shared-core 与数据模型的联动边界

### 5.2 数据类控件的 truth policy

`a-0.5` 必须禁止以下失真模式：

1. model 真值在 backend，public getter 只读 host cache，但文档不标明
2. visible PASS 只证明“像表格/像日历/像折线图”，不证明值对
3. demo 里临时维护一套假数据，绕过真实 model

所有新 getter 都必须明确属于：

1. backend truth
2. host cache 中间态
3. 暂不提供 public readback

默认推荐第一种。

## 6. 严格串行阶段

### R0：shared model baseline

完成内容：

1. item model 结构
2. row/column/item identity
3. model 读写生命周期
4. truth/readback 规则

### R1：editable cell contract

完成内容：

1. 单元格进入编辑/退出编辑
2. 提交/取消/导航切换
3. 与 `keyboard / line_edit` 的桥接
4. disabled / readonly / hidden 行为

### R2：`table`

完成内容：

1. public API
2. backend mapping 到 `ldTable`
3. 当前 cell / 当前 row/column 的真实读回
4. 数据更新与 frame/event 一致性

`table` 是 `a-0.5` 的首控件，不允许后置。

### R3：graph series / value model + `graph`

完成内容：

1. series identity
2. value update contract
3. frame 后 readback 与 visible 一致性
4. backend mapping 到 `ldGraph`

### R4：calendar date / header / grid contract + `calendar`

完成内容：

1. 当前日期真值
2. header / day names / grid 的合同
3. visible 与 date readback 的强绑定
4. backend mapping 到 `ldCalendar`

### R5：matrix / gate / docs closeout

完成内容：

1. current truth-source 与 capability audit 更新
2. 数据类 gate 收口
3. serial 文档与 closeout 术语统一
4. `table / graph / calendar` 在 release matrix 中不再记为 `not_wrapped`

## 7. Subagent 拆分原则

推荐拓扑：

1. 主线程
   - 冻结 shared model 边界
   - 做 GitNexus impact
   - 做 matrix / docs 收口与阶段验收
2. `SG-0.5-R0`
   - 只做 shared model baseline
3. `SG-0.5-R1`
   - 只做 editable cell contract
4. `SG-0.5-R2`
   - 只做 `table`
5. `SG-0.5-R3`
   - 只做 `graph`
6. `SG-0.5-R4`
   - 只做 `calendar`
7. `SG-0.5-Review`
   - 每阶段独立 review

规则：

1. `table` 前两阶段不过，不允许开 `graph` / `calendar`。
2. 每阶段只允许一个写 subagent。
3. review 不通过时，由同阶段 subagent 修复。

## 8. 验证要求

每阶段至少补齐：

1. unit
2. contract
3. mapping
4. visible
5. 数据 readback 一致性测试
6. `git diff --check`

对 `table / calendar / graph` 必须额外确认：

1. frame 之后值不漂移
2. model 更新后 visible 与 getter 一致
3. 不靠 demo 私有数据路径补真值

## 9. 完成定义

`a-0.5` 完成时至少满足：

1. `table / graph / calendar` 进入 PicoUI public widget
2. `table` 证明 edit model 可复用，不是 demo 假逻辑
3. `graph` 证明 series / value 合同真实落到底层
4. `calendar` 证明 date / header / grid 合同真实落到底层
5. `0.4` 输入 shared-core 与 `0.5` 数据 model 无口径冲突

若 `table` 不能证明真实 edit model，则 `a-0.5` 不能算完成，即使 `graph` 和 `calendar` 能显示。
