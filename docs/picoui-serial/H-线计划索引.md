# PicoUI H线计划索引

## H线定位

`H线` 是 PicoUI 基于当前 `9` 个已覆盖控件的第一版本发布准备线。它不是继续补 `G线` 剩余项，也不是新控件扩张线，更不是直接宣布可发布的 closeout 文档。

本线只回答三件事：

1. 离第一版本发布还有多少距离。
2. PicoUI 在控件数量上与 LingDongGUI 的差距。
3. PicoUI 在每个已覆盖控件的能力合同上与 LingDongGUI 的差距。

详细对比真相源：

- `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`
- `docs/superpowers/specs/2026-05-30-picoui-h-line-first-release-preparation-design.md`
- `docs/superpowers/plans/2026-05-30-picoui-h-line-first-release-preparation-implementation.md`

相关历史真相源：

- `docs/picoui-serial/D-线计划索引.md`
- `docs/picoui-serial/F-线计划索引.md`
- `docs/picoui-serial/G-线计划索引.md`
- `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`

## 当前结论摘要

### 发布判断

当前结论：**还没有到发布第一版本的时候**。

原因不是某一个 gate 红了，而是第一版发布所需的产品边界、发布矩阵、当前 9 控件 release contract、人工验收、文档和示例目录还没有形成完整闭环。当前 9 个 PicoUI 控件已经具备真实 LingDongGUI backend mapping 和自动 gate 证据，但这只是发布前基础条件，不是发布完成条件。

H 线后续必须按串行任务推进：`H0` 未完成前不进入 `H1`，`H1` 未完成前不进入 `H2`，依此类推。每个阶段都要有文档、测试或证据产物，不能只做口头判断。

### 控件数量距离

- LingDongGUI 当前可封装原生控件按 H 线口径统计为 `26` 个。
- PicoUI 当前 public 控件为 `9` 个：
  - `window`
  - `label`
  - `button`
  - `checkbox`
  - `switch`
  - `slider`
  - `text`
  - `image`
  - `list`
- 数量覆盖率约为 `9 / 26 = 34.6%`。
- 未覆盖原生控件为 `17` 个。

这个数字不能直接解释成“第一版只完成三分之一”。PicoUI 是上层 API，不要求逐项镜像 LingDongGUI 全部控件；第一版可以选择更小的发布面。但如果第一版目标是“覆盖 LingDongGUI 主流控件面”，当前距离仍然明显不足。

### 已覆盖控件能力距离

PicoUI 当前 9 个控件都已有真实 LingDongGUI backend mapping，并且至少有一层 automatic visible gate 证据。但这只说明“每个控件至少有一个真实映射和可见样本”，不说明每个控件的全部能力都闭环。

按当前能力矩阵统计：

- 能力行总数：`43`
- `support`：`35`
- `reject`：`4`
- `incomplete_contract`：`3`
- `deferred`：`1`

当前非 `support` 能力集中在两个控件：

- `image`：`6` 项
- `list`：`2` 项

### 第一版本距离

如果第一版本定义为“内部可试用 / 合同边界清楚 / 自动 gate 稳定”的 `v0.1`，当前仍不建议马上发布，至少要完成发布口径、发布矩阵、当前控件 release contract、known limitations、manual artifact 与 closeout review 这一组前置事项：

- 发布口径冻结。
- H 线对比文档入库并作为唯一发布差距入口。
- 第一版 release matrix 机器可读化。
- 自动 gate 接管 release matrix。
- `image` / `list` 的非 support 项做成显式限制和测试。
- demo catalog 与 known limitations 成文。
- 发布前验证命令固定。

如果第一版本定义为“对外声明 PicoUI 已覆盖 LingDongGUI 主流控件”的 `v1.0`，当前距离更远，但这已经不属于当前 `H线` closeout 范围，而应在 `H线` 基于现有 9 控件收口后，另开新的 line-level spec / plan：

- 补 3 个高价值新控件 vertical slice。
- 建立输入类控件的焦点、键盘、编辑事件合同。
- 建立更完整的 style/theme/image skin 边界。
- 补 manual artifact 或人工验收记录的发布级证据。
- 补 release checklist、demo catalog、known limitations。

## H线任务拆分

以下任务是按依赖自然形成的严格串行任务。任务数量不是目标，也不是硬性规定；如果后续分析发现某一步需要拆细，可以继续追加子阶段，如果某一步被证明不需要，也可以在文档里明确取消。关键规则是：不要把当前 9 控件的发布收口，与新控件实现、输入系统扩张混做；每个阶段完成后再进入下一项。

### H0：第一版发布口径冻结

目标：明确第一版本不是当前立即发布，而是进入发布前串行收口。

本阶段必须固定：

- 第一版本名称：建议暂称 `PicoUI first release`，不提前承诺 `v1.0`。
- 第一版本最低发布门槛：release matrix gate、demo catalog、known limitations、manual artifact 记录、closeout review 全部完成。
- 禁止发布口径：
  - “PicoUI 已完整支持 LingDongGUI。”
  - “PicoUI 已覆盖 LingDongGUI 全控件。”
  - “9 个 PicoUI 控件已 100% 镜像对应 ld* 控件能力。”
  - “visible gate 通过等于人工验收通过。”

完成条件：

- 本索引写明“当前还没有到发布第一版本的时候”。
- 后续任务必须按当前索引顺序串行推进；若调整任务数量，必须先更新本索引。

### H1：发布差距对比文档

目标：维护专门对比文档，而不是继续把发布差距塞进 G 线文件名。

产物：

- `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`

该文档必须包含：

- 原生 LingDongGUI 控件全集。
- PicoUI 当前覆盖控件。
- 数量覆盖率。
- 已覆盖控件逐项能力差距。
- 未覆盖控件分组和优先级。
- 第一版本 blocker / non-blocker。

完成条件：

- `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md` 成为对比真相源。
- 对比文档不使用 G 线文件名。
- 对比文档明确：26 个 LingDongGUI 可封装控件、9 个 PicoUI 已覆盖控件、17 个未覆盖控件、43 条当前能力项。

### H2：第一版 release matrix 设计

目标：把“第一版要承诺什么”从 Markdown 判断转成稳定矩阵设计。

设计内容：

- 控件维度：
  - `wrapped`
  - `not_wrapped`
  - `deferred`
- 能力维度：
  - `support`
  - `reject`
  - `incomplete_contract`
  - `deferred`
- 证据维度：
  - `unit`
  - `contract`
  - `mapping`
  - `visible`
  - `manual_artifact`
- 发布维度：
  - `release_blocker`
  - `known_limitation`
  - `post_release_candidate`

完成条件：

- 写出 release matrix schema 文档。
- 明确 `image` / `list` 的非 support 项在第一版中属于 blocker 还是 known limitation。
- 明确 17 个未覆盖控件的发布状态。

### H3：release matrix gate 落地

目标：当前旧 `check_picoui_widget_contract_matrix.py` 仍是 D 线旧矩阵 gate，只覆盖 8 控件 / 3 态。发布前必须让 H 线对比结论进入正式可执行检查。

建议 gate 不直接解析 Markdown 全表，而是抽出稳定机器可读源：

- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

第一阶段只检查：

- 9 个已覆盖控件必须出现在 release matrix。
- 17 个未覆盖控件必须明确标为 `not_wrapped`。
- `image` / `list` 的非 support 项必须保留，不允许被误改成 support。
- manual artifact 不能被自动 capture 替代。

完成条件：

- 新增 release matrix JSON。
- 新增 release matrix Python gate。
- 接入 CTest。
- gate 能防止误把未覆盖控件或非 support 能力写成 support。

### H4：当前 9 控件 release contract 收口

目标：逐个控件把“第一版承诺能力”和“明确不承诺能力”写成 release contract。

范围：

- `window`
- `label`
- `button`
- `checkbox`
- `switch`
- `slider`
- `text`
- `image`
- `list`

完成条件：

- 每个控件都有 release contract 小节。
- 每个控件都列：
  - supported APIs
  - backed by ld* entries
  - evidence layers
  - known limitations
  - first-release wording
- `image` 和 `list` 的限制必须显式写入。

### H5：`image` 发布限制硬化

目标：把 `image` 的非 support 项做成不会被误宣称的合同和 gate。

必须固定：

- `theme`：`reject`
- `style_class / user_data`：`incomplete_contract`
- `bg_color / text_color / border_color / radius`：`reject`
- `padding`：`deferred`
- `enabled`：`reject`

完成条件：

- 单测覆盖上述 reject / incomplete / deferred 的状态不被误改。
- release matrix gate 覆盖上述状态。
- release notes 明确 image 只发布基础 source/layout/visible 能力。

### H6：`list` 发布限制硬化

目标：把 `list` 的 per-item 和 metadata 限制做成不会被误宣称的合同和 gate。

必须固定：

- `item marker`：`reject`
- `style_class`：`incomplete_contract`
- widget-level `user_data`：`incomplete_contract`
- `on_selected(..., user_data)` callback cookie 与 widget-level `user_data` 分开。

完成条件：

- 单测覆盖 item id 不等于真实 backend widget / marker。
- release matrix gate 覆盖 list 非 support 项。
- demo guide 不把 item marker 写成真实控件。

### H7：demo catalog 与证据层目录

目标：把“已经支持什么”和“不支持什么”写成用户能读懂的 release contract。

发布说明必须包括：

- 支持控件列表。
- 每个控件支持的核心能力。
- 明确不支持项。
- 证据层解释：unit / contract / mapping / visible / manual artifact 不等价。
- demo 列表和每个 demo 证明的范围。

完成条件：

- 新增或更新 demo catalog 文档。
- 7 个 visible demo 每个都写明：
  - 证明的控件。
  - 证明的能力。
  - 不证明的能力。
  - 对应 gate。

### H8：manual artifact 与人工验收记录

目标：补第一版发布前的人眼验收证据。

最小范围：

- `picoui_hello_world_demo`
- `picoui_basic_widgets_demo`
- `picoui_layout_flex_demo`
- `picoui_layout_grid_demo`
- `picoui_theme_showcase_demo`
- `picoui_settings_panel_demo`
- `picoui_list_basic_demo`

完成条件：

- 每个 demo 有 artifact。
- 每个 demo 有人工观察记录。
- 文档明确 artifact existence 不等于人工验收通过。

### H9：public API 边界冻结

目标：第一版前冻结 public header 泄漏规则和 API 命名。

必须检查：

- `picoui/include/picoui/*.h` 不泄漏 `ld*`。
- 不泄漏 `arm_2d_*`。
- 不泄漏 `SIGNAL_*`.
- public API 命名稳定。
- deprecated / rejected 能力不出现在 release support 表里。

完成条件：

- public API gate 通过。
- release notes 中 public API 表完成。

### H10：发布测试矩阵冻结

目标：把发布前必须跑的测试固定下来。

必须包含：

- unit
- contract
- mapping
- visible
- manual artifact
- release matrix
- demo boundary
- public API

完成条件：

- CTest label 和脚本入口清楚。
- 明确 runtime / visible / mapping 不能并行抢 `build/picoui-runtime`。

### H11：发布文档包

目标：把用户读到的第一版文档整理完整。

必须包含：

- release notes
- supported controls
- known limitations
- demo catalog
- capability matrix
- build / test instructions
- evidence explanation

完成条件：

- 文档之间链接闭环。
- 不再需要读 G/D/F 历史文档才能理解第一版支持范围。

### H12：独立发布 review

目标：在发布 closeout 前做只读 review。

review 必须检查：

- 发布说明是否夸大。
- 控件数量是否准确。
- release matrix 是否和代码一致。
- known limitations 是否覆盖 `image/list/font/theme/manual artifact`。
- demo 是否未泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。

完成条件：

- 独立 review 无 blocking。
- 若有 blocking，由原实现 subagent 修，不在主线程顺手修。

### H13：发布 closeout

建议 closeout 命令：

```bash
ctest --test-dir build --output-on-failure -L picoui
ctest --test-dir build --output-on-failure -L visible
ctest --test-dir build --output-on-failure -L mapping
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
git diff --check
```

注意：runtime / visible / mapping 脚本共用 `build/picoui-runtime`，不要并行跑这些标签，否则可能互相抢同一 build 目录。

完成条件：

- 所有发布 gate 通过。
- manual artifact 记录完成。
- release notes 和 known limitations 已冻结。
- H 线 closeout 文档写明可发布。

## 第一版发布建议

推荐发布路线：

1. 当前先不发布第一版本。
2. 按 H 线索引顺序严格串行推进；任务数量服从发布要求，不硬凑固定数量。
3. 不再继续扩大 G 线文件职责。
4. H 线先把发布差距、控件数量、能力差距、证据层边界、release matrix 与文档包讲清楚，再进入 closeout。
5. `progress_bar / line_edit / combo_box` 若要作为对外 `v1.0` 前置项，应在 `H线` closeout 后另开新线，不并入当前 H 线。

## H线外候选后续主题

以下主题保留为 `H线` 之后的候选方向，但不属于当前 `H线` closeout 范围，也不应作为当前发布准备线的串行 blocker：

1. 输入事件与焦点合同设计。
2. `progress_bar` vertical slice。
3. `line_edit` vertical slice。
4. `combo_box` vertical slice。
5. style/theme 更完整的 `v1` 边界。
6. 资源与字体系统更完整的 release contract。
7. 未覆盖控件的后续路线冻结。

当前不建议直接对外宣称：

- “PicoUI 已完整支持 LingDongGUI。”
- “PicoUI 已覆盖 LingDongGUI 全控件。”
- “9 个 PicoUI 控件已 100% 镜像对应 ld* 控件能力。”
- “visible gate 通过等于人工验收通过。”

可以诚实宣称：

- “PicoUI 已有 9 个上层控件，它们都走真实 LingDongGUI backend mapping。”
- “当前支持能力是明确的合同子集，并有 unit / contract / mapping / visible 证据。”
- “未支持能力和未封装控件已在 H 线对比文档中列出。”

但这些只能作为当前研发状态，不能直接作为第一版发布结论。
