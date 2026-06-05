# PicoUI J线 `v0.1` 对齐发布设计

> 日期：2026-05-30
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/picoui-serial/J-线计划索引.md`
> 目标：把 `H线` 之后的首版对齐发布路线冻结成独立 `J线`，避免继续把“发布准备”“9 控件完全对齐”“`v0.2` backlog”混写。

---

## 1. 背景

`H线` 已经把以下事实冻结下来：

1. PicoUI 当前已做控件固定为 `window/label/button/checkbox/switch/slider/text/image/list` 这 `9` 个。
2. `H线` 的主责任是发布准备：release matrix、当前 9 控件合同、known limitations、demo catalog、manual artifact、closeout review。
3. `H线` 文档里已经出现另一条更强口径：如果首版不是保守 internal candidate，而是要求“当前已做控件先做实功能对齐”，则应把任务从发布准备线中分离出来。

当前风险不是缺文档，而是路线混淆：

- 一部分文档还在表达 `internal v0.1` 的保守发布口径；
- 一部分结论已经开始表达“当前 9 控件完全对齐版”；
- 用户又明确接受拆成 `v0.1` / `v0.2`，并要求先挑简单控件做实。

因此必须新开 `J线`，把 `H线` 之后的首版功能对齐发布工作单独收口。

---

## 2. J线唯一目标

`J线` 的唯一目标是：

把当前 `9` 个已做控件拆成两组，先完成一组低复杂度控件与对应 `LingDongGUI` 控件的首版功能对齐，并以此形成 `v0.1` 发布面。

这条线只回答三件事：

1. 哪些当前已做控件进入 `v0.1` 对齐发布范围。
2. 对这些控件，“功能对齐完成”的判定标准是什么。
3. 哪些当前已做控件继续保留为 `wrapped but not parity-complete`，并转入 `v0.2` backlog。

---

## 3. 非目标

`J线` 不处理以下事情：

1. 不新增 `progress_bar`、`line_edit`、`combo_box` 或其他新控件。
2. 不追求“当前 9 控件全部在同一版本完全对齐”。
3. 不改写 `H线` 既有发布准备结论，只做后续分流。
4. 不把 `checkbox / switch / text / image / list` 偷渡进 `v0.1` 完成条件。
5. 不通过弱化 public contract、降低 gate、修改 demo 意图或添加 fake 路径来制造“已对齐”。
6. 不把单层证据外推成“功能对齐已完成”。

---

## 4. 发布范围定义

### 4.1 `v0.1` 对齐发布范围

`J线` 固定 `v0.1` 只覆盖以下 `4` 个控件：

- `window`
- `label`
- `button`
- `slider`

选择原则：

1. 缺口主要集中在单控件能力和视觉映射。
2. 不依赖复杂输入模型、item model、navigation、资源生命周期耦合链。
3. 适合用严格串行 subagent 逐控件收口。

### 4.2 `v0.2` backlog 范围

以下 `5` 个控件固定转入 `v0.2` backlog：

- `checkbox`
- `switch`
- `text`
- `image`
- `list`

它们的统一口径是：

- 已 wrapped；
- 已有真实 LingDongGUI backend mapping；
- 但尚未完成与对应 `ld*` 控件的功能对齐；
- 不得在 `v0.1` 文案中写成“已对齐”。

---

## 5. “对齐完成”的定义

对 `J线` 而言，“对齐完成”不是“API 能调用”或“demo 能跑”，而是以下五项同时成立：

1. **public API 对齐**
   - PicoUI public API 已覆盖本次首版承诺的 `ld*` 能力面。
   - 不再靠“底层有能力但 PicoUI 暂不承诺”回避缺口。
2. **真实 backend 对齐**
   - 对应能力真实落到 `ld*` 或真实 `ldBase` 语义。
   - 不是 wrapper-only 状态，也不是 metadata-only 占位。
3. **读写一致**
   - 新增 setter / getter / callback / native bridge 在语义上是一致的。
   - 不能只补 setter，不补 readback。
4. **证据层完整**
   - `unit / contract / mapping / visible / manual artifact` 五层证据齐全。
5. **文案真实**
   - 发布说明、控件清单、release matrix、合同文档都明确承认当前完成面，不再保留“最小子集”“metadata-only”掩饰性写法。

---

## 6. 四个 `v0.1` 控件的对齐目标

### 6.1 `window`

当前主要缺口：

- `background image/mask`
- `PaddingGroup` 粒度语义
- 最小 readback

`J线` 要求：

1. 明确 `window background image` 的 PicoUI public contract。
2. 明确 `PaddingGroup` 是否暴露为 PicoUI public API，或定义等价高层语义。
3. 至少补足 `window` 对应已承诺样式/布局状态的最小 readback。
4. 文档不再把 `window` 写成“只支持基础容器子集”。

### 6.2 `label`

当前主要缺口：

- `transparent`
- `align`
- `background image/mask`
- getter/readback

`J线` 要求：

1. `transparent` 成为真实合同。
2. `align` 成为真实合同。
3. `background image/mask` 成为真实合同。
4. 补足 `text/text_color/align/bg_color/font/transparent` readback。

### 6.3 `button`

当前主要缺口：

- `release/press image`
- `transparent`
- `font`
- `checkable`
- `key_value`
- pressed state 读写

`J线` 要求：

1. `button` 双态 image skin 成为真实合同。
2. `transparent`、`font`、`checkable`、`key_value` 成为真实合同。
3. pressed state 的 setter / getter / native bridge 语义一致。
4. 文档不再把 `button` 写成“只有 clicked/pressed/released + 基础样式子集”。

### 6.4 `slider`

当前主要缺口：

- `horizontal`
- background/indicator image+mask
- `indicator width`
- `slim size`

`J线` 要求：

1. `horizontal` 成为真实合同。
2. background/indicator image+mask 成为真实合同。
3. `indicator width`、`slim size` 成为真实合同。
4. `value/range/percent/orientation` 四者语义一致。

---

## 7. 文档与矩阵设计约束

### 7.1 `wrapped` 与 `parity-complete` 必须分层

`J线` 启动后，文档和机器真相源必须能区分：

1. `已 wrapped`
2. `已达 v0.1 parity`
3. `已转入 v0.2 parity backlog`

这三者不能再混成一个“已支持”状态。

### 7.2 对现有 H 线 release matrix 采用非破坏扩展

`H线` 已有 `tests/picoui/contract/picoui_release_capability_matrix.json` 与对应 gate。`J线` 不应推翻 H 线 schema，而应在不破坏 H 线 gate 的前提下增加 `J线` 所需的对齐分流信息。

推荐做法：

1. 保留现有 `wrapped / not_wrapped / deferred` 和 capability 状态枚举。
2. 追加 `J线` 需要的补充字段或补充段。
3. 在新 gate 中检查：
   - `v0.1` 目标控件集合固定为 `window/label/button/slider`
   - `v0.2 backlog` 固定为 `checkbox/switch/text/image/list`
   - 文案与矩阵口径一致

### 7.3 合同文档必须改成 `v0.1` / `v0.2` 分流

以下文档后续必须同步改写：

- `docs/picoui-serial/H-线当前9控件发布合同.md`
- `docs/picoui-serial/H-线已支持控件清单.md`
- `docs/picoui-serial/H-线第一版发布说明.md`
- `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`

其中：

1. `v0.1` 只允许宣称 `4` 个控件已完成对齐。
2. `v0.2` 的 `5` 个控件统一写为“已 wrapped，未完成对齐”。

---

## 8. 证据层要求

`J线` 每个对齐任务都必须同时补齐以下证据：

1. `unit`
2. `contract`
3. `mapping`
4. `visible`
5. `manual artifact`

边界约束：

1. `mapping` 不能外推成“功能对齐完成”。
2. `visible` 不能外推成人工验收完成。
3. `manual artifact` 必须有最终人工观察结论，不能只保留 `artifact existence`。

---

## 9. 交付物

`J线` closeout 前至少要具备：

1. `J线` 索引文档。
2. `J线` spec。
3. `J线` implementation plan。
4. 更新后的 release matrix / gate。
5. `window / label / button / slider` 的对齐合同与测试。
6. `v0.1 / v0.2` 分流后的支持清单与发布说明。
7. `window / label / button / slider` 的 visible 与 manual artifact 证据。
8. 独立 review 与 closeout 文档。

---

## 10. blocker 定义

以下问题会阻断 `J线 v0.1` closeout：

1. `v0.1` / `v0.2` 边界未冻结。
2. `wrapped` 与 `parity-complete` 两层状态未拆开。
3. `window / label / button / slider` 任一控件仍保留首版范围内的关键未对齐能力。
4. 合同文档、清单、发布说明仍把全部 `9` 个控件混写成同一发布面。
5. 新增对齐能力没有完整五层证据。
6. manual artifact 仍然只有 artifact existence，没有最终人工观察结论。

---

## 11. 当前推荐结论

`J线` 的正确定位不是“继续往 H 线上补任务”，而是：

1. 把 `H线` 作为现状与发布准备真相源保留。
2. 把 `J线` 作为 `H线` 之后的正式后续执行入口。
3. 用 `v0.1 = window / label / button / slider`、`v0.2 = checkbox / switch / text / image / list` 把首版路线收紧到可验证、可交付、可收口的规模。

只要 `J线` 口径成立，后续实现就不应再回到“首版要不要把 9 个控件全做完”的模糊讨论，而应围绕这 `4` 个控件逐个收口。
