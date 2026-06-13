# TINYUI H线第一版本发布准备设计

> 日期：2026-05-30
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/tinyui-serial/H-线计划索引.md`
> 目标：把 `TINYUI` 的 `H线` 冻结成“基于当前 9 个已覆盖控件的第一版本发布准备线”，避免和新控件扩张线、输入系统线、后续 `public v1.0` 候选线混写。

---

## 1. 背景

`G线` 已把当前控件 capability gap 收紧到以下事实：

1. 当前已覆盖控件固定为 `window/label/button/checkbox/switch/slider/text/image/list` 这 `9` 个。
2. `text font`、`image 语义`、`list metadata` 的最小合同线已收口到现有 authoritative 结论。
3. `G线` 下已不存在“自然、最小、可直接验证”的下一条实现线；若要继续推进新能力，必须先开新的 line-level 合同决策。

因此 `H线` 不能再把“现有 9 控件的发布准备”和“新增控件/输入系统扩张”混成同一条串行主线。`H线` 唯一合理定位，是先把当前 9 控件的发布口径、release matrix、known limitations、manual artifact 和 closeout review 收口成可发布但不夸大的第一版本准备线。

---

## 2. H线唯一目标

`H线` 的唯一目标是：为当前 `9` 个已覆盖控件准备一个口径诚实、证据层清楚、可通过自动 gate 与人工记录共同支撑的第一版本发布包。

这条线只回答三件事：

1. 当前 `9` 个控件距离第一版本发布还有多少差距。
2. 当前 `9` 个控件哪些能力可以作为 `support` 发布，哪些只能作为 `known limitation` 或 `reject/incomplete/deferred` 对外说明。
3. 发布前必须补哪些文档、gate、artifact 和 review，才能避免把当前研发状态夸写成“TINYUI 已完整覆盖 LingDongGUI”。

---

## 3. 非目标

`H线` 不处理以下事情：

1. 不新增 `progress_bar`、`line_edit`、`combo_box` 或其他新控件。
2. 不建立新的输入/焦点系统合同。
3. 不把 `public v1.0` 需要的控件扩张，强行写成当前 `H线` blocker。
4. 不把 `automatic visible gate`、`mapping gate`、`artifact existence` 任一单层证据写成人工窗口验收通过。
5. 不顺手重做 `G线` 已冻结的 capability matrix 口径。
6. 不通过修改 demo 用户意图、硬编码布局、假视觉修补，来制造“可发布”假象。

如果需要推进新控件或新输入合同，必须在 `H线` closeout 后新开 line-specific spec / plan。

---

## 4. 发布层级定义

### 4.1 `internal v0.1`

`H线` 的 closeout 目标只能是一个保守的 `internal v0.1` 候选：

1. 只基于当前 `9` 个控件。
2. 只承诺当前已被 `unit / contract / mapping / visible / manual artifact` 证据共同约束的合同子集。
3. 明确写出 `image/list/font/theme/manual artifact` 等 known limitations。
4. 不对外宣称“TINYUI 已覆盖 LingDongGUI 主流控件面”。

### 4.2 `public v1.0`

`public v1.0` 不属于当前 `H线` closeout 目标。它至少要求：

1. 新控件扩张。
2. 输入/焦点合同。
3. 更完整的 style/theme/resource 边界。
4. 更强的 release evidence。

这些内容应在 `H线` 收口后另开新线。

---

## 5. H线设计约束

### 5.1 现有 9 控件是固定发布面

当前 `H线` 的发布面固定为：

- `window`
- `label`
- `button`
- `checkbox`
- `switch`
- `slider`
- `text`
- `image`
- `list`

`H线` 可以定义这些控件的 first-release wording、known limitations 和 release matrix 状态，但不能在本线内扩控件数量。

### 5.2 `image` / `list` 的非 support 项只能收口为发布限制

`image` 与 `list` 仍是当前发布面上风险最高的两个控件。`H线` 只能做两类事情：

1. 把现有 `reject / incomplete_contract / deferred` 项冻结成发布限制和 gate。
2. 防止文档、demo、release notes 把这些项误写成 `support`。

`H线` 不能把这些限制直接升级为新能力开发线。

### 5.3 自动 gate 与人工结论必须分层

`H线` 里的自动 gate 只能检查：

1. release matrix 状态和 schema。
2. 现有脚本入口与 artifact 元数据是否齐全。
3. 非 support 项没有被误写成 support。

自动 gate 不能检查“人眼是否验收通过”。人工窗口验收结论必须继续由单独 artifact 记录和独立 review 文档承担。

### 5.4 manual artifact 最小范围必须服从现有工具链

当前可执行、已有口径支撑的最小 manual artifact 范围，只能先固定为：

1. `tinyui_basic_widgets_demo`
2. `tinyui_settings_panel_demo`

若未来要扩到 `7` 个 visible demos，必须先开独立子任务扩脚本入口、artifact 模板和文档矩阵；不能直接把 `7` 个 demo 写成当前阶段的默认完成条件。

---

## 6. H线交付物

### 6.1 发布口径冻结

必须冻结以下口径：

1. 第一版本名称暂称 `TINYUI first release` 或 `internal v0.1 candidate`。
2. 禁止表述：
   - `TINYUI 已完整支持 LingDongGUI`
   - `TINYUI 已覆盖 LingDongGUI 全控件`
   - `9 个 TINYUI 控件已 100% 镜像对应 ld* 控件能力`
   - `visible gate 通过等于人工验收通过`
3. 允许表述：
   - `当前 9 个控件已走真实 LingDongGUI backend mapping`
   - `当前第一版本只承诺明确列出的合同子集`
   - `未支持能力与未覆盖控件已在 release matrix / known limitations 中显式列出`

### 6.2 发布差距对比文档

必须维护 `H线` 自己的对比真相源，回答：

1. LingDongGUI 可封装控件总数与 TINYUI 已覆盖数量。
2. 当前 `9` 控件逐项能力差距。
3. `internal v0.1` 与 `public v1.0` 的边界差异。
4. 哪些问题是当前 H 线 blocker，哪些是 H 线后的候选主题。

### 6.3 release matrix schema 与 gate

release matrix 必须成为机器可读真相源，并至少表达：

1. 控件维度：`wrapped / not_wrapped / deferred`
2. 能力维度：`support / reject / incomplete_contract / deferred`
3. 证据维度：`unit / contract / mapping / visible / manual_artifact`
4. 发布维度：
   - `internal_v0_1_blocker`
   - `internal_v0_1_known_limitation`
   - `post_h_candidate`

其中：

1. `release matrix gate` 只检查 schema、状态、入口脚本和元数据完整性。
2. `manual artifact` 的“存在”与“人工验收通过”必须分开表示。
3. `H线` 不要求自动 gate 去证明人工是否真的看过窗口。

### 6.4 当前 9 控件 release contract

每个控件的 release contract 必须至少包含：

1. `supported APIs`
2. `backed by ld* entries`
3. `evidence layers`
4. `known limitations`
5. `first-release wording`

其中 `image` 与 `list` 必须单列风险说明。

### 6.5 `image` / `list` 发布限制硬化

`image` 至少要固定：

1. `theme`: `reject`
2. `style_class / user_data`: `incomplete_contract`
3. `bg_color / text_color / border_color / radius`: `reject`
4. `padding`: `deferred`
5. `enabled`: `reject`

`list` 至少要固定：

1. `item marker`: `reject`
2. `style_class`: `incomplete_contract`
3. `widget-level user_data`: `incomplete_contract`
4. `on_selected(..., user_data)` callback cookie 与 widget-level `user_data` 分离

### 6.6 demo catalog 与证据层目录

必须有一份面向发布阅读者的 demo catalog，明确：

1. 每个 demo 证明哪些控件。
2. 每个 demo 证明哪些能力。
3. 每个 demo 不证明什么。
4. 对应哪个自动 gate 或人工 artifact 路径。

### 6.7 manual artifact 与人工验收记录

`H线` 的最小 manual artifact 交付只要求覆盖：

1. `tinyui_basic_widgets_demo`
2. `tinyui_settings_panel_demo`

每条记录至少包含：

1. 平台
2. SDL video driver
3. demo target
4. 运行命令
5. artifact 路径
6. 人工观察结论
7. 已知限制

### 6.8 public API 边界与发布测试矩阵

第一版本前必须再次冻结：

1. `tinyui/include/tinyui/*.h` 不泄漏 `ld*`
2. 不泄漏 `arm_2d_*`
3. 不泄漏 `SIGNAL_*`
4. 发布测试矩阵的命令顺序和互斥关系

尤其要明确：`runtime / visible / mapping` 共用 `build/tinyui-runtime`，不能并行抢目录。

### 6.9 发布文档包、独立 review 与 closeout

`H线` closeout 前必须具备：

1. release notes
2. supported controls
3. known limitations
4. demo catalog
5. capability matrix / release matrix
6. build / test instructions
7. evidence explanation
8. 独立发布 review

closeout 只能在这些文档、gate 和记录全部齐备后进行。

---

## 7. blocker 语义

为避免 `blocker` 混义，`H线` 统一采用两层口径：

### 7.1 `internal v0.1 blockers`

以下问题会阻断当前 `H线` closeout：

1. release matrix schema / gate 未落地。
2. 当前 `9` 控件 release contract 未写清。
3. `image/list` 的发布限制未硬化。
4. demo catalog、known limitations、release notes 未成文。
5. 最小 manual artifact 记录未完成。
6. 独立发布 review 未完成。

### 7.2 `post-H candidates`

以下不是当前 `H线` blocker，但属于 `H线` 后的候选主题：

1. `progress_bar / line_edit / combo_box`
2. 输入事件与焦点合同
3. 更完整的 style/theme/image skin 系统
4. 更完整的字体/资源系统
5. 其他未覆盖控件的路线冻结

---

## 8. 验收标准

`H线` 设计被正确执行后，应满足：

1. `H线` 不再混入新控件扩张任务。
2. `internal v0.1` 与 `public v1.0` 的发布层级边界明确。
3. `manual artifact` 的元数据存在、脚本入口存在、人工结论记录存在，但自动 gate 不冒充人工结论。
4. 当前 `9` 控件的支持范围与限制范围都能在 release matrix 与文档包中直接读到。
5. closeout 结论不会再依赖读者自行拼接 `D/F/G` 历史文档。

---

## 9. H线后续主题

`H线` closeout 后，如需继续推进，建议新开以下候选线，而不是在 `H线` 内继续扩写：

1. 输入事件与焦点合同线
2. `progress_bar` vertical slice
3. `line_edit` vertical slice
4. `combo_box` vertical slice
5. style/theme `v1` 增强线
6. 资源与字体系统增强线

