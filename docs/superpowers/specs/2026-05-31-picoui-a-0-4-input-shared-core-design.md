# PicoUI a-0.4 输入 shared-core 串行设计

## 1. 背景

`a-0.3` 已把 PicoUI 当前 `15` 个 public widget 的真实分层写实，但 review 结论也很明确：

1. runtime 仍存在会替 demo 补 layout 的 `temporary smoke path`
2. `list` 已暴露 host cache 与 backend truth 不统一的问题
3. `message_box`、`date_time` 等 a-02 证据层更适合 vertical slice，不适合直接扩写成高耦合输入控件模式

因此 `a-0.4` 不能复制 `a-02` 的“低耦合扩面”路线，必须先做输入 shared-core。

## 2. 目标

`a-0.4` 的唯一目标：

1. 建立 PicoUI 输入/焦点/选择 shared-core
2. 在这套 shared-core 上串行接入 `line_edit / keyboard / combo_box / scroll_selecter`
3. 让这些控件从第一天起就走真实 backend mapping、真实 focus/navigation 语义、真实 readback 边界

## 3. 非目标

`a-0.4` 明确不做：

1. `table / graph / calendar`
2. `arc / gauge / icon_slider / radial_menu`
3. 靠 demo 硬编码或 runtime 补位掩盖 layout/focus 缺口
4. 先把控件“画出来”，再回头补 shared-core

## 4. 方案比较

### 方案 A：四控件平推

优点：

1. 看起来覆盖率增长最快

缺点：

1. `line_edit` 与 `keyboard` 会互相卡住
2. `combo_box` 与 `scroll_selecter` 会各自重造 selection/navigation
3. 最终 shared-core 仍会退化成 demo glue code

结论：拒绝。

### 方案 B：shared-core 先行，控件严格串行

优点：

1. 能先收口 readback / focus / navigation 规则
2. 后续 `table` 可直接复用 `0.4` 基础
3. 更符合仓库“不能靠 demo 掩盖 backend/layout 缺口”的规则

缺点：

1. 前两阶段看起来“没有新增很多 public widget”
2. 文档、gate、shared-owner 收敛工作更重

结论：推荐。

### 方案 C：先做 `line_edit + keyboard`，再回填 selection/dropdown

优点：

1. 能更早看到输入控件 demo

缺点：

1. `combo_box / scroll_selecter` 后面仍会重写 shared-core
2. 容易把 `list` 当前 host cache 问题复制过去

结论：不推荐。

## 5. 推荐架构

### 5.1 shared-core 先收口三条底线

#### 底线一：runtime / layout honesty

必须先清掉或严格隔离会在 runtime 阶段替 demo 补 region 的通用逻辑，避免 visible gate 继续把“被补位后的可见”误写成“真实布局完成”。

#### 底线二：readback truth policy

必须明确：

1. 哪些 public getter 返回 backend truth
2. 哪些 getter 只是 host cache
3. 如果只是 host cache，文档与 matrix 必须显式标注为中间态

`a-0.4` 新控件默认不允许继续复制 `list` 当前这种模糊边界。

#### 底线三：focus ownership

必须先定义：

1. 当前谁拥有 focus
2. focus 如何切换
3. keyboard 输入发往谁
4. hidden / disabled / dropdown open 状态如何影响 focus

这三条底线稳定后，才允许开始单控件接入。

### 5.2 输入 shared-core 组件

`a-0.4` 需要建立的 shared-core 组件：

1. focus ownership contract
2. editable text contract
3. keyboard bridge
4. selection / navigation contract
5. dropdown contract

建议主要写面：

1. `picoui/src/core/internal.h`
2. `picoui/src/core/widget.c`
3. `picoui/src/core/event.c`
4. `picoui/src/backend/ldgui/backend.h`
5. `picoui/src/backend/ldgui/backend_widget.c`
6. `picoui/src/backend/ldgui/backend_event.c`
7. `picoui/src/backend/ldgui/backend_app.c`
8. `picoui/src/backend/ldgui/backend_layout.c`
9. `tests/picoui/runtime/check_picoui_backend_mapping.py`
10. `tests/picoui/runtime/check_picoui_visible_ui.py`

## 6. 严格串行阶段

### R0：honesty / readback 基线

完成内容：

1. 处理 `temporary smoke path`
2. 冻结 getter 是 backend truth 还是 host cache 的规则
3. 把 gate 拆成“可见”与“值一致”两层

这是整个 `a-0.4` 的前置阶段，不允许并行跳过。

### R1：focus ownership

完成内容：

1. focus owner 结构与生命周期
2. focus enter/leave 事件
3. hidden/disabled 与 focus 的关系
4. runtime / unit / contract 基线测试

### R2：editable text contract + `line_edit`

完成内容：

1. editable text 最小合同
2. `line_edit` public API
3. backend mapping 到 `ldLineEdit`
4. 真实 text readback 与提交/取消边界

### R3：keyboard bridge + `keyboard`

完成内容：

1. 输入目标绑定
2. 键盘输出事件桥
3. 文本输入、导航输入、退出输入边界
4. backend mapping 到 `ldKeyboard`

### R4：selection / navigation contract

完成内容：

1. 单选/移动/select confirm 的统一合同
2. 与 focus、disabled、open/close 状态的联动
3. 对 `list` 风格 readback 问题做统一约束

### R5：dropdown contract + `combo_box`

完成内容：

1. 展开/收起合同
2. selected item truth/readback
3. backend mapping 到 `ldComboBox`
4. 不允许靠 demo 补布局

### R6：`scroll_selecter`

完成内容：

1. item 集合与选中项合同
2. edit mode / navigation mode 边界
3. backend mapping 到 `ldScrollSelecter`

### R7：matrix / gate / docs closeout

完成内容：

1. `current truth-source` 扩面
2. contract / runtime / visible / mapping gate 同步
3. 中文 serial 文档收口

## 7. Subagent 拆分原则

`a-0.4` 默认串行，不做多写面并行。推荐拓扑：

1. 主线程
   - 冻结 shared-core 边界
   - 做 GitNexus impact
   - 做阶段验收与文档收敛
2. `SG-0.4-R0`
   - 只做 honesty / readback 基线
3. `SG-0.4-R1`
   - 只做 focus ownership
4. `SG-0.4-R2`
   - 只做 editable text + `line_edit`
5. `SG-0.4-R3`
   - 只做 keyboard bridge + `keyboard`
6. `SG-0.4-R4`
   - 只做 selection/navigation
7. `SG-0.4-R5`
   - 只做 `combo_box`
8. `SG-0.4-R6`
   - 只做 `scroll_selecter`
9. `SG-0.4-Review`
   - 每阶段独立 review，不直接改代码

规则：

1. 任何 shared-owner 文件只允许当前阶段 subagent 写。
2. review 不通过时，在同一阶段 subagent 修，不新开主线程修补。
3. 主线程每阶段结束后才允许进入下一阶段。

## 8. 验证要求

每阶段至少补齐：

1. unit
2. contract
3. mapping
4. visible
5. `git diff --check`

对 `R0 / R1 / R4` 这类 shared-core 阶段，必须额外验证：

1. frame/event 后的 readback 一致性
2. focus 切换后 state 不漂移
3. hidden / disabled / dropdown 状态不会错误吞输入

## 9. 完成定义

`a-0.4` 完成时至少满足：

1. `line_edit / keyboard / combo_box / scroll_selecter` 进入 PicoUI public widget
2. 不再依赖 runtime 通用补 layout 逻辑制造 visible PASS
3. getter truth policy 明确，文档与 matrix 一致
4. 输入 shared-core 可被 `table` 直接复用

若做不到第 2 或第 3 条，即使四个控件都能“显示”，也不能算 `a-0.4` 完成。
