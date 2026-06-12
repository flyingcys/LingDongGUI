# PicoUI a-0.6 全量发布缺口总账设计

## 1. 背景

当前 PicoUI 已经不再处于“只有少数 public widget”的早期阶段。

真实基线已经是：

1. LingDongGUI 可封装控件总数：`26`
2. PicoUI 已进入 public widget 集的控件：`22`
3. 仍未进入 public widget 集的控件：`4`
4. 真正还能维持 `full_parity_complete` 的控件：`4`
5. 其余 `18` 个已包裹控件仍分别停在
   - `stable_contract_but_not_full_parity`
   - `minimal_vertical_slice_only`

因此，`a-0.6` 不能再被定义成“补最后 4 个控件 + 做最终审计”。

那种定义只会得到：

1. `26/26 wrapped`
2. 但不是 `26/26 full parity complete`

这会制造“全覆盖但不全对齐”的假完成。

## 2. 目标

`a-0.6` 的唯一目标：

1. 把 PicoUI 收口到真正可发布的 LingDongGUI 全量控件版本。
2. 让 `26` 个控件全部进入 PicoUI public widget 集。
3. 让 `26` 个控件全部从当前中间态收口到 `full_parity_complete`。
4. 把 release matrix、gate、中文文档升级为最终发布真相源，而不是继续停留在 `a-0.3 current-15` 盘点口径。

## 3. 非目标

`a-0.6` 明确不做：

1. 只补 `arc / gauge / icon_slider / radial_menu` 就收工。
2. 只做审计，不做缺口整改。
3. 把 `stable contract` 或 `minimal vertical slice` 当成最终发布口径。
4. 用 demo 硬编码、host cache、fake visual、放宽 gate 的方式伪造 parity。
5. 先追求“全部能显示”，再回头补 getter/readback/contract。

## 4. 当前真实差距

### 4.1 分层差距

当前 `26` 个控件分成四组：

1. `full_parity_complete`
   - `window`
   - `label`
   - `button`
   - `slider`
2. `stable_contract_but_not_full_parity`
   - `checkbox`
   - `switch`
   - `text`
   - `image`
   - `list`
   - `line_edit`
   - `keyboard`
   - `combo_box`
   - `scroll_selecter`
   - `graph`
   - `table`
   - `calendar`
3. `minimal_vertical_slice_only`
   - `progress_bar`
   - `qrcode`
   - `progress_wheel`
   - `message_box`
   - `date_time`
   - `clock`
4. `not_wrapped`
   - `arc`
   - `gauge`
   - `icon_slider`
   - `radial_menu`

### 4.2 系统性差距

当前最终发布视角下，还缺以下系统性能力：

1. final release matrix schema
2. 与 `26` 控件对齐的 full demo catalog
3. 与 `26` 控件对齐的 runtime / mapping / visible / manual artifact gate
4. 最终发布中文文档集
5. 每个控件的 capability gap 关闭标准

### 4.3 `a-0.6` 当时 truth-source 的边界

当前 `tests/picoui/contract/picoui_release_capability_matrix.json` 仍是：

1. `schema_version = a-0.3-current-15-layered-v1`
2. purpose = `current-15 truth source and layered capability matrix`
3. 明确声明它“不编码 release-ready approval”

所以它今天只能作为“当前状态盘点真相源”，不能直接当最终发布 verdict。

## 5. 方案比较

### 方案 A：只补 `4` 个 `not_wrapped`

优点：

1. 覆盖率最快到 `26/26`

缺点：

1. 不能解决其余 `18` 个控件不是 full parity 的问题
2. 容易产出“26/26 covered but not 26/26 complete”
3. 审计会被写成纸面动作

结论：拒绝。

### 方案 B：`4` 个新控件 + 同步重做 release matrix

优点：

1. 比方案 A 更诚实

缺点：

1. matrix 诚实了，产品还是没完成
2. `stable/minimal` 控件仍然不会自动变成 final parity

结论：不够。

### 方案 C：全量发布缺口总账

做法：

1. 先把 `a-0.6` 定义成全量发布总线
2. 并行认知上覆盖 `26` 个控件，但执行上严格串行分包
3. 先补 `4` 个 `not_wrapped`
4. 再把 `12 stable` 与 `6 minimal` 逐组收口到 `full_parity_complete`
5. 最后升级 release truth-source / gate / docs

优点：

1. 目标与最终发布一致
2. 每一类缺口都有明确归属
3. 不会把审计和整改拆散
4. 适合 subagent 串行推进

缺点：

1. 文档和计划规模更大
2. 必须更严格地冻结阶段边界和写面

结论：推荐。

## 6. 推荐架构

### 6.1 顶层阶段

`a-0.6` 固定拆成 `W1 -> W2 -> W3 -> W4`：

1. `W1`: 覆盖缺口补齐
   - `arc / gauge / icon_slider / radial_menu`
2. `W2`: `stable_contract_but_not_full_parity` 收口
   - `checkbox / switch / text / image / list`
   - `line_edit / keyboard / combo_box / scroll_selecter`
   - `graph / table / calendar`
3. `W3`: `minimal_vertical_slice_only` 升级
   - `progress_bar / qrcode / progress_wheel`
   - `message_box / date_time / clock`
4. `W4`: 最终发布证据层升级
   - final release matrix
   - full gate catalog
   - manual artifact 闭环
   - 中文文档/closeout/release notes

### 6.2 为什么这样拆

#### 先 `W1`

原因：

1. 不补齐 `4` 个 `not_wrapped`，永远谈不上全量覆盖
2. `arc/gauge` 与 `icon_slider/radial_menu` 分属两种不同 shared 能力组
3. 这组最适合作为“覆盖缺口收口”独立阶段

#### 再 `W2`

原因：

1. `stable contract` 组的危险最大，因为最容易被误判成“已经差不多完成”
2. 这 `12` 个控件多数已有真实 backend 与 public API，收口难点在 parity、readback、theme、event 语义
3. 它们需要按 shared 依赖分组，而不是按加入历史分组

#### 再 `W3`

原因：

1. `minimal vertical slice` 组六个控件都已经有 demo/unit/gate，最容易被误判成已完成
2. 这组的真实工作是从“单条竖切片”升格到“完整发布合同”
3. 它们的风险主要是配置面、readback 面、theme/skin 面、复杂行为面

#### 最后 `W4`

原因：

1. final release matrix / gate / docs 必须以已完成代码态为准
2. 太早做 final release schema，只会反复返工
3. manual artifact 也必须建立在最终控件集合与最终 gate catalog 之上

## 7. 分组细化

### 7.1 `W1` 两个子包

#### `W1-A`: `arc / gauge`

共享点：

1. angle / pointer / progress contract
2. 仪表类 visual truth
3. readback 不允许退化成 host shadow

#### `W1-B`: `icon_slider / radial_menu`

共享点：

1. composite navigation contract
2. 复合 item / icon / selection 语义
3. visible 不能只证明“画出来了”

### 7.2 `W2` 三个子包

#### `W2-A`: `checkbox / switch / text / image / list`

共享点：

1. 老控件 parity 收口
2. metadata / style / user_data 边界
3. readback 面补齐

#### `W2-B`: `line_edit / keyboard / combo_box / scroll_selecter`

共享点：

1. `a-0.4` 输入 shared-core
2. focus/edit/select/navigation
3. `keyboard` runtime-first 证据提升

#### `W2-C`: `graph / table / calendar`

共享点：

1. `a-0.5` data-model shared-core
2. backend truth/readback
3. final public contract 强度提升

### 7.3 `W3` 两个子包

#### `W3-A`: `progress_bar / qrcode / progress_wheel`

共享点：

1. 当前都只有窄 API 面
2. theme/skin/config 面缺口明显
3. 要从 vertical slice 升级到发布合同

#### `W3-B`: `message_box / date_time / clock`

共享点：

1. 当前都只有最小场景合同
2. host cache / readback / mode/config 面不足
3. 可见结果与复杂公共行为面还未闭环

### 7.4 `W4` 四个子包

#### `W4-A`: final release matrix schema

目标：

1. 不再沿用 `a-0.3-current-15-layered-v1`
2. 引入最终发布 verdict 所需层级
3. 明确 widget parity 与 release-ready judgement 的区别与关系

#### `W4-B`: full gate catalog

目标：

1. 对齐 `26` 控件 demo/gate 覆盖
2. 对齐 runtime / mapping / visible 边界
3. 对齐 special case，例如 `keyboard`

#### `W4-C`: manual artifact 闭环

目标：

1. widget-level 与 demo-level 人工证据边界分开
2. final release 所需人工项不再散落
3. 人工窗口验收不再只服务旧线

#### `W4-D`: 中文发布文档集

目标：

1. final release 说明
2. final closeout 文档
3. 每控件 capability audit 汇总
4. 真相源入口统一

## 8. 文件与写面边界

### 8.1 shared-owner 文件

以下文件默认由 `a-0.6` 主线程控制写面，不允许多个写 subagent 重叠修改：

- `picoui/src/core/internal.h`
- `picoui/src/core/widget.c`
- `picoui/src/core/event.c`
- `picoui/src/backend/ldgui/backend.h`
- `picoui/src/backend/ldgui/backend_widget.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

### 8.2 聚合入口文件

以下文件只允许在每个子阶段末尾最小接入一次：

- `picoui/include/picoui/picoui.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/contract/check_picoui_public_api.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`
- `picoui/docs/demo_guide.md`
- `docs/picoui-serial/a-0.4-a-0.6-后续版本记录.md`
- `docs/picoui-serial/a-0.4-线计划索引.md`
- `docs/picoui-serial/a-0.5-线计划索引.md`

## 9. Subagent 策略

### 9.1 原则

1. 多个 subagent 可并行探索，但写面不得重叠。
2. 真正落代码/文档时，同一阶段只允许一个写 subagent。
3. 每个阶段必须单独 review。
4. review 不通过时，由原 subagent 修复，不新开主线程修复。

### 9.2 推荐拓扑

1. 主线程
   - 冻结阶段边界
   - 跑 GitNexus impact
   - 收敛 matrix / docs / gate
   - 做最终验证
2. `SG-a06-W1A`
   - `arc / gauge`
3. `SG-a06-W1B`
   - `icon_slider / radial_menu`
4. `SG-a06-W2A`
   - `checkbox / switch / text / image / list`
5. `SG-a06-W2B`
   - `line_edit / keyboard / combo_box / scroll_selecter`
6. `SG-a06-W2C`
   - `graph / table / calendar`
7. `SG-a06-W3A`
   - `progress_bar / qrcode / progress_wheel`
8. `SG-a06-W3B`
   - `message_box / date_time / clock`
9. `SG-a06-W4A`
   - final release matrix schema + gate catalog
10. `SG-a06-W4B`
   - manual artifact + 中文发布文档集
11. `SG-a06-Review`
   - 每阶段只读 review

## 10. 验证原则

每个阶段都至少要出以下证据：

1. unit
2. contract
3. public API
4. backend mapping
5. visible
6. runtime（如果控件需要 runtime bridge）
7. `git diff --check`

最终收口必须额外满足：

1. `26` 个控件在 final release matrix 中全部是 `full_parity_complete`
2. demo catalog、runtime、mapping、visible、manual artifact 对齐同一套控件集
3. 中文发布文档与机器真相源一致

## 11. 完成定义

只有当以下条件全部满足，`a-0.6` 才算完成：

1. `26/26` 全控件进入 PicoUI public widget 集。
2. `26/26` 全控件 capability gap 收口完毕。
3. 不再有 `stable_contract_but_not_full_parity`、`minimal_vertical_slice_only`、`not_wrapped` 作为最终态。
4. final release matrix、final gate catalog、manual artifact、中文发布文档全部收口。
5. 用户能够从 `a-0.6` 当时单一 truth-source 入口回答：
   - 哪 `26` 个控件全部完成
   - 每个控件靠什么证据证明完成
   - 最终发布版本还剩什么工作

如果只完成 `26/26 wrapped`，或只完成“paper audit”，都不算 `a-0.6` 完成。
