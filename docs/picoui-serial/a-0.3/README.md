# TINYUI a-0.3 计划索引

- 当前仓库硬规则：`AGENTS.md`
- 当前旧发布真相源：`docs/tinyui-serial/H-线计划索引.md`
- 当前旧 parity 真相源：`docs/tinyui-serial/J-线计划索引.md`
- 当前 backlog 收口线：`docs/tinyui-serial/a-01-线计划索引.md`
- 当前低耦合扩面线：`docs/tinyui-serial/a-02-线计划索引.md`
- 当时主仓机器可读 truth-source：`tests/tinyui/contract/tinyui_release_capability_matrix.json`
- 当时 deep review：`docs/superpowers/reviews/2026-06-01-tinyui-a-0-3-a-0-4-a-0-5-deep-review.md`
- 当前 `a-0.3` 历史快照：`docs/tinyui-serial/a-0.3/current-15-覆盖与分层真相源.md`
- 当前 `a-0.3` 历史 capability audit：`docs/tinyui-serial/a-0.3/current-15-capability-audit.md`
- 当前 closeout 标准：`docs/tinyui-serial/a-0.3/a-0.3-closeout-标准.md`
- 本目录设计 spec：`docs/superpowers/specs/2026-05-31-tinyui-a-0-3-truth-source-and-current-15-design.md`
- 本目录执行 plan：`docs/superpowers/plans/2026-05-31-tinyui-a-0-3-truth-source-and-current-15-implementation.md`
- 当时 closeout review：`docs/superpowers/reviews/2026-05-31-tinyui-a-0-3-closeout-review.md`
- 后续 `0.4-0.6` 记录：`docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
- `a-0.4` 串行索引：`docs/tinyui-serial/a-0.4-线计划索引.md`
- `a-0.5` 串行索引：`docs/tinyui-serial/a-0.5-线计划索引.md`

## 为什么开 a-0.3 目录

`a-0.3` 不是旧 `H/J/a-01/a-02` 的补注。

它解决的是当前最急的收敛问题：

1. 旧 truth-source 仍停在 `9` 控件世界。
2. `a-01 / a-02 / J线` 已继续推进，但当时缺少统一的 current truth。
3. 不先把 current truth、current gap、`0.3` 完成标准写实，后面再快都会继续失真。

## 当前真实基线

截至 2026-06-01，代码与机器真相源共同表明：

1. current TINYUI public widget 已达 `22` 个，不再是旧 `9` 控件世界，也不再停在 `15` 控件世界。
2. `J线 4` 仍是当前唯一可维持 `full parity complete` 口径的控件组。
3. `a-01 5 + a-0.4 4 + a-0.5 3` 当前都应诚实归为 `stable contract but not full parity`。
4. `a-02` 六控件当前仍应诚实归为 `minimal vertical slice only`。
5. 当前仍有 `4` 个 LingDongGUI 控件尚未进入 TINYUI public widget。

因此当前最保守、最诚实的工程判断是：

- TINYUI current public widget 总数当前是 `22`。
- `tests/tinyui/contract/tinyui_release_capability_matrix.json` 是 `a-0.3` 当时主仓 truth-source 入口。
- `a-0.3/current-15-*` 文档现在只能视为 2026-05-31 的阶段审计快照，不能再被引用为“当前 truth-source”。

## 最终大目标

最终大目标不变：

1. TINYUI 覆盖 LingDongGUI 当前 `26` 个可封装原生控件。
2. 每个 TINYUI public 控件都与对应 LingDongGUI 控件**完整对齐**。
3. `wrapped`、`stable contract`、`minimal vertical slice` 都只能是中间态，不能是最终口径。

但这不是 `a-0.3` 本阶段全部要做完的内容。

## a-0.3 的唯一任务

`a-0.3` 只做三件事：

1. 重建当时的 current truth-source
   - 先把旧 `9` 控件 matrix 升级到当时的 `current-15` 真相
2. 审计当时的 current-15
   - 对 `J线 4`、`a-01 5`、`a-02 6` 共 `15` 个当时已接入控件逐个做 capability audit
   - 明确哪些已 full parity，哪些只是过渡态，哪些还差什么
3. 冻结 `a-0.3` closeout 标准
   - 规定 `0.3` 完成时必须交付哪些 truth-source、matrix、文档、gate
   - 不在本阶段写后续版本的详细 spec 和 plan

## 中间态的唯一合法用途

`a-0.3` 中会继续使用：

1. `full parity complete`
2. `stable contract but not full parity`
3. `minimal vertical slice only`

但这些标签只用于：

1. 描述当前盘点结果
2. 排定后续版本优先级
3. 标注未完成项

它们**不代表最终可接受状态**。最终目标仍然只有“全控件、全功能、完整对齐”。

## a-0.3 严格串行边界

当前建议严格按以下顺序推进：

1. `a-0.3-R1` truth-source rebuild
2. `a-0.3-R2` current-15 parity stratification
3. `a-0.3-R3` current-15 widget-by-widget capability audit
4. `a-0.3-R4` truth-source / matrix / docs consistency repair
5. `a-0.3-R5` `a-0.3` closeout standard freeze

其中：

1. `R1 / R2 / R4 / R5` 必须串行。
2. `R3` 内部允许并行，但只允许按不重叠写面拆 subagent。

## a-0.3 并行策略

`a-0.3` 不是全串行，也不是无脑并行。

执行原则固定为：

1. **能并行的并行**
   - widget-by-widget capability audit
   - 不重叠文档分块
   - 独立 review
2. **不能并行的串行**
   - truth-source 总数口径
   - matrix 主文件收敛
   - gate 术语收敛
   - closeout 标准冻结

推荐 subagent 拓扑：

1. 主线程
   - 决策 current truth
   - 收敛 matrix
   - 收敛最终文档
   - 做最后验证
2. `R3-Audit-J`
   - 负责 `J线 4`：`window / label / button / slider`
3. `R3-Audit-a01`
   - 负责 `a-01 5`：`image / text / checkbox / switch / list`
4. `R3-Audit-a02`
   - 负责 `a-02 6`：`progress_bar / qrcode / progress_wheel / message_box / date_time / clock`
5. `R3-Review`
   - 独立审读 audit 输出，不直接改实现结论

## a-0.3 成功标准

`a-0.3` 完成时必须满足：

1. 当时覆盖面不再含糊，能够准确回答 2026-05-31 时点的 TINYUI public widget 总数。
2. 当时 `15` 个已接入控件全部进入新的 truth-source。
3. 每个 current-15 控件都有明确 capability audit：
   - 已完成什么
   - 未完成什么
   - 当前属于什么中间层级
4. machine-readable matrix、中文文档、gate 用词完全一致。
5. `a-0.3` closeout 标准冻结完成。

当前 `R1 / R2` 真相源入口：

1. `docs/tinyui-serial/a-0.3/current-15-覆盖与分层真相源.md`
2. `docs/tinyui-serial/a-0.3/current-15-capability-audit.md`

## a-0.3 不做什么

1. 不在本阶段详细展开 `a-0.4 / a-0.5 / a-0.6` 的 spec 和 plan。
2. 不在本阶段新增高耦合新控件实现。
3. 不把 current-15 的中间态提前写成最终 full parity。

## 后续版本记录边界

`0.4-0.6` 还是要记录，但记录方式收紧为：

1. 在单独文件里写版本边界、目标控件、shared 依赖、完成定义。
2. 不在当前阶段展开成后续详细 spec。
3. 不在当前阶段展开成后续详细 implementation plan。

## 下一步

当前只以 `a-0.3` spec/plan 为入口，先把 truth-source 和 current-15 审计做实。
