# TINYUI J线计划索引

- `H线` 发布准备真相源：`docs/tinyui-serial/H-线计划索引.md`
- `H线` 差距对比真相源：`docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`
- `J线` design truth：`docs/superpowers/specs/2026-05-30-tinyui-j-line-v0-1-parity-release-design.md`
- `J线` implementation plan：`docs/superpowers/plans/2026-05-30-tinyui-j-line-v0-1-parity-release-implementation.md`
- `H线` 当前 9 控件合同：`docs/tinyui-serial/H-线当前9控件发布合同.md`
- `H线` 已支持控件清单：`docs/tinyui-serial/H-线已支持控件清单.md`
- `H线` 发布测试矩阵：`docs/tinyui-serial/H-线发布测试矩阵.md`
- `H线` 发布说明：`docs/tinyui-serial/H-线第一版发布说明.md`
- `H线` manual artifact 真相源：`docs/tinyui-serial/C-线人工窗口验收记录.md`
- 当前仓库硬规则：`AGENTS.md`

## J线定位

`J线` 是 `H线` 之后的**首版功能对齐发布线**。

## 2026-05-31 review 补记

- 独立代码 review 抓出的两个 blocker 已修复并通过当前 targeted 验证：
  - `label` 的 `create_with_props` 路径现已覆盖 `transparent / align / background_source`。
  - `slider` 的 release matrix 已回调到真实 public API 边界，不再把不存在的 `get_value` 写成 `support`。
- 因此当前 `J线` 可以恢复按“`window / label / button / slider` 已完成首版对齐”这条实现口径继续收口；但这仍不等于 `release ready`，因为 manual artifact 边界还在。

它的唯一目标不是“把当前 9 个控件全部一次性做满”，而是：

1. 把 `v0.1` 发布范围固定为当前 `9` 个已做控件里**相对简单、适合先做实对齐**的一组控件。
2. 把其余已 wrapped 但仍有较大功能缺口的控件，明确转入 `v0.2` backlog。
3. 让 `v0.1` 的发布口径从“当前能跑/已 wrapped”升级为“指定控件已完成与对应 LingDongGUI 控件的功能对齐”。

为防止混乱，`J线` 明确不再复用 `H线` 的“保守 internal v0.1 候选”口径。`H线` 负责发布准备与现状真相源；`J线` 负责 `H线` 之后、面向首版对齐发布的正式收口任务。

## J线不是什么

- 不是 `H线` 的补注。
- 不是“当前 9 控件全部完全对齐”的一次性大线。
- 不是新增 `progress_bar / line_edit / combo_box` 的扩控件线。
- 不是 `v0.2` 收尾线。
- 不是靠改 demo、降合同、弱化证据层来制造“可发布”结论。

## 前置条件

`J线` 默认建立在以下前提上：

1. `H线` 的现状盘点、release matrix、当前 9 控件合同、demo catalog、发布说明与测试矩阵已经入库。
2. `H线` 的 manual artifact / closeout 边界已经讲清，不再混写成“已发布”。
3. `J线` 启动时，继续沿用 `unit / contract / mapping / visible / manual artifact` 五层证据，不降低验证门槛。

## J线当前决策

为避免一次吃掉 `9` 个控件导致路线失控，`J线` 当前固定如下拆分：

### `v0.1` 对齐发布范围

- `window`
- `label`
- `button`
- `slider`

这 `4` 个控件是 `J线` 的唯一首版对齐目标。

### 转入 `v0.2` backlog 的已做控件

- `checkbox`
- `switch`
- `text`
- `image`
- `list`

这些控件当前仍属于 `wrapped but not parity-complete`，即：

- 已经接入 TINYUI；
- 已有真实 LingDongGUI backend mapping；
- 但还没有完成与对应 `ld*` 控件的功能对齐；
- 不得在 `v0.1` 文档中顺带写成“已对齐”。

## 为什么 `v0.1` 先做这 4 个

### `window`

- 缺口主要集中在 `background image/mask`、`PaddingGroup` 语义、少量 readback。
- 结构清晰，交互耦合低。

### `label`

- 缺口集中在 `transparent`、`align`、`background image`、getter/readback。
- 单控件内即可闭环，不依赖复杂输入模型。

### `button`

- 虽然仍缺 `press/release image`、`checkable`、`key_value`、pressed readback，
- 但没有 `radio group`、`navigation`、`scroll`、`item model` 这类跨对象合同。

### `slider`

- 缺口主要是 `horizontal`、`image skin`、`indicator width`、`slim size`。
- 也是单控件参数与视觉映射问题，适合单线收口。

## 为什么其余 5 个进 `v0.2`

### `checkbox`

- 牵涉 `radio group`、image mode、text/image 组合排版。

### `switch`

- 牵涉 `direction`、`navigation`、disabled interactive、track/knob image skin。

### `text`

- 牵涉 font resource、static/owned text、scroll seek/move、background image。

### `image`

- 当前最大问题不是单个 setter，而是通用 `widget` style/theme/enabled/padding 在 image 上到底有没有真实 backend 语义还没彻底定死。

### `list`

- 牵涉 item model、item widget、padding/margin、select 视觉、callback/user_data 边界，是当前已做控件里结构最重的一类。

## J线发布口径

若 `J线` 完成，`v0.1` 只能宣称：

1. `window / label / button / slider` 已完成与对应 LingDongGUI 控件的首版功能对齐。
2. `checkbox / switch / text / image / list` 已 wrapped，但仍在 `v0.2` 对齐 backlog。
3. 当前 `v0.1` 不是 LingDongGUI 全控件覆盖，也不是当前 `9` 控件全部完全对齐版。

禁止表述：

- “当前 9 个控件都已完成对齐”
- “TINYUI 首版已经完整覆盖 LingDongGUI 当前已做控件”
- “其余 5 个控件只是视觉小问题”

## J线任务拆分

以下任务默认严格串行：`J0` 完成前不进入 `J1`，依此类推。

### J0：冻结 `v0.1` / `v0.2` 边界

目标：

- 把 `v0.1 = window / label / button / slider`
- 与 `v0.2 = checkbox / switch / text / image / list`
- 固定为正式发布边界。

完成条件：

- `J线` 索引成为 `H线` 之后的唯一路线入口。
- `H线` 文档只保留现状与分流说明，不再承担 `v0.1` 执行索引角色。

### J1：拆开 “wrapped” 与 “parity-complete” 两层状态

目标：

- 文档和 release matrix 必须能区分：
  - 已 wrapped
  - 已达 `v0.1 parity`
  - 已转入 `v0.2 parity backlog`

完成条件：

- 发布口径不再把 “能创建/能跑 demo” 等价成 “已对齐”。
- `checkbox / switch / text / image / list` 的状态写法统一。

### J2：`window` 对齐收口

目标：

- 补齐 `window` 相对 `ldWindow` 的首版缺口。

当前必补项：

- `background image/mask`
- `PaddingGroup` 或等价高层合同
- 最小 readback

完成条件：

- `window` 不再以“仅基础容器子集”表述。
- 对应测试、合同和证据层补齐。

### J3：`label` 对齐收口

目标：

- 补齐 `label` 相对 `ldLabel` 的首版缺口。

当前必补项：

- `transparent`
- `align`
- `background image/mask`
- `text/text_color/align/bg_color/font/transparent` readback

完成条件：

- `label` 不再以“仅静态文本最小子集”表述。

### J4：`button` 对齐收口

目标：

- 补齐 `button` 相对 `ldButton` 的首版缺口。

当前必补项：

- release/press image skin
- `transparent`
- `font`
- `checkable`
- `key_value`
- pressed state 读写与交互一致性

完成条件：

- `button` 不再只停留在 clicked/pressed/released + 基础样式子集。

### J5：`slider` 对齐收口

目标：

- 补齐 `slider` 相对 `ldSlider` 的首版缺口。

当前必补项：

- `horizontal`
- background/indicator image+mask
- `indicator width`
- `slim size`
- `value/range/percent/orientation` 一致性

完成条件：

- `slider` 不再只停留在最小数值滑块子集。

### J6：`v0.1` 合同、清单、发布文案重写

目标：

- 按 `v0.1` / `v0.2` 分流重写当前合同和支持清单。

至少需要同步的文档：

- `docs/tinyui-serial/H-线当前9控件发布合同.md`
- `docs/tinyui-serial/H-线已支持控件清单.md`
- `docs/tinyui-serial/H-线第一版发布说明.md`
- `docs/tinyui-serial/H-线发布差距与LingDongGUI控件对比.md`

完成条件：

- `v0.1` 只宣称 `4` 个控件已对齐。
- `v0.2 backlog` 的 `5` 个控件统一改成“已 wrapped，未完成对齐”。

### J7：`v0.1` 证据层补齐与 closeout

目标：

- 为 `window / label / button / slider` 的对齐结论补齐完整证据。

必须覆盖：

- `unit`
- `contract`
- `mapping`
- `visible`
- `manual artifact`

完成条件：

- 不能只凭自动 gate 宣称对齐完成。
- 人工窗口验收记录要有最终观察结论。
- `v0.1` 发布说明不再依赖 `known limitations` 掩盖这 4 个控件本身的功能缺口。

## 当前推荐结论

`J线` 应作为 `H线` 之后的正式后续入口。

当前最稳妥的路线不是：

- 继续在 `H线` 里追加 `v0.1 parity` 任务，
- 也不是直接把全部 `9` 个控件都塞进首版对齐目标，

而是：

1. 先用 `J线` 冻结 `v0.1 / v0.2` 边界。
2. 把 `window / label / button / slider` 作为 `v0.1` 唯一对齐目标。
3. 把 `checkbox / switch / text / image / list` 明确转入 `v0.2` backlog。

这样做的价值是：发布口径、执行范围、验证目标都会更稳定，不会再把 `H线`、保守发布口径、9 控件完全对齐版、`v0.2` backlog 混成一条线。
