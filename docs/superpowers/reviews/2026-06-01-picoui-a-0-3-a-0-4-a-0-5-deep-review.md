# TINYUI a-0.3 / a-0.4 / a-0.5 深度 review

## 范围

## 后续更新说明

本文是 `2026-06-01` 当时针对 `a-0.3 / a-0.4 / a-0.5` 的深度 review 快照。

后续 `a-0.6` 主线推进后，以下结论已经不再代表当前最新状态：

1. `tinyui_wrapped_widget_total = 22`
2. `tinyui_not_wrapped_widget_total = 4`
3. 对 `2026-06-01` 这轮 review 而言，当时 machine truth-source 仍是 `a-0.3/current-15`

对 `2026-06-01` 这轮 review 而言，当时最新入口应以：

1. `tests/tinyui/contract/tinyui_release_capability_matrix.json`
2. `tinyui/docs/demo_guide.md`
3. `docs/tinyui-serial/C-线人工窗口验收记录.md`

为准。本文保留的价值是：

1. 解释为什么当时必须把 `a-0.3 current-15` 降格成历史快照
2. 记录 `a-0.4 / a-0.5` 阶段 closeout 不能被误写成 final parity 的原因

本轮 review 只评估当前主仓 `dev` 上与 `a-0.3 / a-0.4 / a-0.5` 直接相关的真实交付：

1. `docs/tinyui-serial/a-0.3/*`
2. `docs/tinyui-serial/a-0.4-线计划索引.md`
3. `docs/tinyui-serial/a-0.5-线计划索引.md`
4. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
5. `docs/superpowers/specs/2026-05-31-tinyui-a-0-4-input-shared-core-design.md`
6. `docs/superpowers/specs/2026-05-31-tinyui-a-0-5-data-model-design.md`
7. `docs/superpowers/plans/2026-05-31-tinyui-a-0-4-input-shared-core-implementation.md`
8. `docs/superpowers/plans/2026-05-31-tinyui-a-0-5-data-model-implementation.md`
9. `tinyui/src/widgets/*`
10. `tinyui/src/backend/ldgui/*`
11. `tinyui/demo/*`
12. `tests/tinyui/contract/*`
13. `tests/tinyui/runtime/*`
14. `tests/tinyui/unit/*`

本轮不重跑整仓历史 line，只回答：

1. `a-0.3 / a-0.4 / a-0.5` 当前代码与文档是否一致。
2. `a-0.4 / a-0.5` 是否可以写成“已经开发完成并 closeout”。
3. 哪些问题必须先在真相源文档里纠正。

## 基线与影响面

### git / GitNexus 结论

1. 当前主仓 `HEAD` 为 `ec2f8c7 feat(tinyui): add input and data widgets`。
2. `0ebd8ac start a-0.4/0.5` 是 `a-0.4 / a-0.5` 这批改动的直接起点。
3. `gitnexus_detect_changes(repo=\"LingDongGUI\", scope=\"compare\", base_ref=\"0ebd8ac\")` 返回：
   - `changed_count = 194`
   - `changed_files = 61`
   - `affected_count = 17`
   - `risk_level = critical`

结论：

1. `a-0.4 / a-0.5` 不是“低风险补文档”。
2. 当前主仓确实已经承载真实 shared-core 与新控件代码，不是只有 spec/plan。
3. 但这不自动等于“full parity 完成”或“全部 closeout 结论已诚实”。

## Findings

### 1. 严重：`a-0.3` 目录仍被引用成当时当前 truth-source，但它实际只覆盖 `current-15`

相关位置：

1. `docs/tinyui-serial/a-0.3/README.md:9`
2. `docs/tinyui-serial/a-0.3/README.md:33`
3. `docs/tinyui-serial/a-0.3/current-15-覆盖与分层真相源.md:14`
4. `docs/tinyui-serial/a-0.3/current-15-覆盖与分层真相源.md:58`
5. `tests/tinyui/contract/tinyui_release_capability_matrix.json:53`
6. `tinyui/include/tinyui/tinyui.h:8`

现状：

1. `a-0.3/README.md` 仍把 `current-15-覆盖与分层真相源.md` 标成“当前 truth-source”。
2. 同 README 仍写当前 public widget 总数是 `15`、未覆盖是 `11`。
3. 但主仓 `tinyui.h` 已经公开导出 `calendar / combo_box / graph / keyboard / line_edit / scroll_selecter / table`。
4. release matrix summary 也已经是 `tinyui_wrapped_widget_total = 22`、`tinyui_not_wrapped_widget_total = 4`。

结论：

1. `a-0.3/current-15-*` 现在只能算历史阶段快照。
2. 如果在当时继续把它引用成“当前 truth-source”，那 `a-0.4 / a-0.5 已完成` 这个文档结论没有真相源基础。

直接要求：

1. 把 `a-0.3` 目录降格成历史快照入口。
2. 把当时当前 truth-source 入口改回 machine-readable matrix + 当前 deep review。

### 2. 已修复：`keyboard` release matrix 证据层已回调到真实 gate 入口

相关位置：

1. `tests/tinyui/contract/tinyui_release_capability_matrix.json:954`
2. `tests/tinyui/runtime/check_tinyui_visible_ui.py:14`
3. `tests/tinyui/runtime/check_tinyui_backend_mapping.py:21`
4. `tests/tinyui/runtime/check_tinyui_runtime.py:12`
5. `tinyui/docs/demo_guide.md:251`

修复前现状：

1. release matrix 把 `keyboard.evidence_layers.mapping = present`、`visible = present`。
2. 但 `keyboard_basic` 只出现在 `check_tinyui_runtime.py` 的 target 集。
3. `check_tinyui_visible_ui.py` 的 `DEMOS` 不包含 `keyboard_basic`。
4. `check_tinyui_backend_mapping.py` 的 target matrix 也不包含 `tinyui_keyboard_basic_demo`。
5. `demo_guide` 反而明确写了 `keyboard_basic` 不承担 visible correctness gate。

这不是措辞小问题，而是机器真相源与真实 gate 入口直接矛盾。当前最多只能证明：

1. `keyboard` 有 runtime bridge 证据。
2. `keyboard` 通过 `line_edit` shared-core 路由输入。

不能证明：

1. `keyboard` 已有独立 visible gate。
2. `keyboard` 已有独立 backend mapping gate。

修复结论：

1. release matrix 已把 `keyboard.evidence_layers` 收回到 runtime-first 口径，不再写成独立 `mapping / visible present`。
2. `demo_guide` 与 `a-0.4 / a-0.5` 文档同步明确：`keyboard_basic` 只证明 bridge/ownership runtime 路径。
3. 若后续需要 `keyboard` 独立 mapping / visible 证据，必须新增真实 gate，而不是继续复用当前表述。

### 3. 已修复：`scroll_selecter` backend `kind` 已回到真实类型，shared data-model 元数据已建起来

相关位置：

1. `tinyui/src/backend/ldgui/backend_scroll_selecter.c:69`
2. `tinyui/src/backend/ldgui/backend.h:42`
3. `tinyui/src/backend/ldgui/backend_widget.c:18`
4. `tinyui/src/backend/ldgui/backend_event.c:278`

修复前现状：

1. 枚举已经有独立 `PICOUI_BACKEND_WIDGET_SCROLL_SELECTER`。
2. 但 `tinyui_backend_create_scroll_selecter()` 仍把 `widget->kind` 设成 `PICOUI_BACKEND_WIDGET_TEXT`。
3. `tinyui_backend_widget_bind_host()` 会调用 `tinyui_backend_widget_init_data_model()`。
4. 该初始化只会对 `PICOUI_BACKEND_WIDGET_SCROLL_SELECTER` 设置 `data_truth_policy = BACKEND_VALUE` 和非零 `data_model_identity`。

结果：

1. `scroll_selecter` 当前虽然能工作，也能通过 selected-index / edit-mode 单测。
2. 但它没有按 `a-0.4 / a-0.5` shared model 设计进入 data-model identity / truth policy 体系。
3. 这会让文档里“selection truth 已进入 shared-core”这个结论少一块真实代码支撑。

修复结论：

1. `backend_scroll_selecter.c` 已改为真实 `PICOUI_BACKEND_WIDGET_SCROLL_SELECTER`。
2. unit test 已补 `kind / data_truth_policy / data_model_identity` 断言，并验证 selected/edit-mode 路径前后不漂移。
3. 这块不再是当前 review 的开放缺口。

### 4. 中高：`a-0.4 / a-0.5` 文档把“阶段收口”写得过强，容易被读成 full parity 完成

相关位置：

1. `docs/tinyui-serial/a-0.4-线计划索引.md:169`
2. `docs/tinyui-serial/a-0.5-线计划索引.md:161`
3. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md:69`
4. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md:129`
5. `tests/tinyui/contract/tinyui_release_capability_matrix.json:883`
6. `tests/tinyui/contract/tinyui_release_capability_matrix.json:948`
7. `tests/tinyui/contract/tinyui_release_capability_matrix.json:998`
8. `tests/tinyui/contract/tinyui_release_capability_matrix.json:1048`
9. `tests/tinyui/contract/tinyui_release_capability_matrix.json:1121`
10. `tests/tinyui/contract/tinyui_release_capability_matrix.json:1169`
11. `tests/tinyui/contract/tinyui_release_capability_matrix.json:1219`

现状：

1. `a-0.4 / a-0.5` 相关索引多处写成“当前主仓已完成并收口”。
2. 但 release matrix 对 `line_edit / keyboard / combo_box / scroll_selecter / table / graph / calendar` 的正式层级仍统一是：
   - `current_layer = stable_contract_but_not_full_parity`
   - `parity_status = parity_incomplete`
3. 这些控件的 `audit_note` 也都明确写着“不 claim full visual/theme/feature parity”。

因此当前正确结论应是：

1. `a-0.4 / a-0.5` 作为 shared-core 阶段任务，大体已经落地到主仓。
2. 但它们的产物仍是 `stable contract but not full parity`。
3. 不能把“阶段 closeout”偷换成“这些控件已经最终完成”。

直接要求：

1. 文档必须显式补一句：`a-0.4 / a-0.5` 完成的是 shared-core 与当前层级收口，不等于 full parity。
2. 索引里要把未收敛项单列出来，而不是只写“唯一剩余任务是文档同步”。

### 5. 中：`a-0.4` 的“证据齐全”表述过强，`keyboard` 仍只有 runtime 证据

### 6. 已收口：`table` 文档与 matrix 已降到真实强度，只宣称 commit boundary

相关位置：

1. `tests/tinyui/unit/test_tinyui_table.c:55`
2. `tinyui/src/backend/ldgui/backend_table.c:93`
3. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md:124`
4. `tests/tinyui/contract/tinyui_release_capability_matrix.json:1201`

收口前现状：

1. `table` 单测目前只覆盖 editable cell 的 commit 路径。
2. backend 侧也是在 `SIGNAL_FINISHED` 时按 commit 结果收口。
3. 文档和 matrix 现在把它写成“editable cell contract 已真实成立”。

这比当前测试和代码实际证明的范围更强。更准确的说法应是：

1. commit boundary 已验证。
2. cancel/abort reason 仍未证明成稳定 public contract。

相关位置：

1. `docs/tinyui-serial/a-0.4-线计划索引.md:166`
2. `docs/tinyui-serial/a-0.4-线计划索引.md:171`
3. `tinyui/docs/demo_guide.md:246`
4. `tests/tinyui/runtime/check_tinyui_runtime.py:12`

`a-0.4` 索引当前写法容易让人读成四个控件都已经具备同层证据。实际不是：

1. `line_edit / combo_box / scroll_selecter` 确实已有 visible gate。
2. `keyboard` 仍主要靠 runtime bridge 证明。
3. 这与 `demo_guide` 的当前口径一致，但与索引的笼统表述不一致。

直接要求：

1. `a-0.4` 索引要把四个控件的证据层拆开写。
2. 明确 `keyboard` 目前是 runtime-first 证据，而非 visible-first。

## Review judgement

### 可以确认的部分

1. `a-0.3` truth-source / matrix / closeout 标准主线仍成立。
2. `a-0.4` 与 `a-0.5` 不是空规划，主仓里确实已有真实 backend/widget/unit/runtime/mapping/gate 落地。
3. `line_edit / combo_box / scroll_selecter / table / graph / calendar` 的 public API 与 backend truth 路径大体已经成形。

### 不能继续误写的部分

1. 不能把 `a-0.4 / a-0.5` 的 shared-core 阶段收口写成 full parity 完成。
2. 不能把 `keyboard` 的 runtime-only 证据写成 visible + mapping 都已 present。
3. 不能忽略 `scroll_selecter` 当前 `kind` 失真导致的 shared data-model 元数据缺口。

最终判断：

1. `a-0.4 / a-0.5` 可以写成“代码主线已落地，阶段文档需要收口”。
2. 不能直接写成“已经开发完成，且所有 closeout 证据已完全一致”。
3. 当前更准确的口径应是：shared-core 任务基本完成，但 release truth-source 仍有两处需要先纠正。

## 文档修正建议

1. `docs/tinyui-serial/a-0.4-线计划索引.md`
   - 改成“shared-core 阶段已落地，但仍属 stable contract / parity incomplete”
   - 单列 `keyboard evidence mismatch`
   - 单列 `scroll_selecter kind / shared model metadata gap`
2. `docs/tinyui-serial/a-0.5-线计划索引.md`
   - 改成“data-model 阶段主路径已落地，不等于 full parity”
   - 显式引用 release matrix 当前层级
3. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
   - 同步上述口径，避免“已完成并收口”被读成最终完成
