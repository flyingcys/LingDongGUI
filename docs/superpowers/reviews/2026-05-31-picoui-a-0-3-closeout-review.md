# TINYUI a-0.3 closeout review 与 0.4-0.5 串行建议

## 评审范围

本轮 review 只评估 `a-0.3` closeout 对应的真实交付：

1. `docs/tinyui-serial/a-0.3/*`
2. `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
3. `tests/tinyui/contract/tinyui_release_capability_matrix.json`
4. 与 `a-0.3` 当时 truth-source 直接相关的 runtime / backend / widget 证据链

不重新评估整个 TINYUI 仓库历史，只回答：

1. `a-0.3` 是否足够 closeout。
2. 进入 `a-0.4` / `a-0.5` 前，哪些问题必须先收敛。
3. `0.4 -> 0.5` 应该按什么串行骨架推进。

## 变更与影响面

### git / GitNexus 结论

1. `git diff --stat a3da594..HEAD` 显示本轮主要改动 `8` 个文件，主体是 truth-source 文档与 release matrix。
2. `gitnexus_detect_changes(repo="LingDongGUI", scope="compare", base_ref="a3da594")`
   - `changed_count = 49`
   - `changed_files = 8`
   - `risk_level = low`
   - `affected_processes = 0`
3. `gitnexus_impact()` 对本轮唯一真实代码检查点 `_assert_current_layers`、`_assert_summary_counts` 都是 `LOW`，影响面仅在 `tests/tinyui/contract/check_tinyui_release_capability_matrix.py` 自身。

结论：

- `a-0.3` 这轮不是执行路径级高风险改动。
- 真正需要 review 的重点，不是“会不会炸主流程”，而是“文档口径是否诚实、gate 是否把过渡态误写成完成态”。

## Findings

### 1. 严重：runtime 仍保留会主动补 layout 的 temporary smoke path

`tinyui/src/backend/ldgui/backend_app.c` 里的 `tinyui_backend_apply_smoke_cursor_layout()` 仍会在 root 没有真实 `flex/grid` 布局时，给子控件线性排位并直接改写 `ldBaseSetRegion(...)`。

相关位置：

1. `tinyui/src/backend/ldgui/backend_app.c:510`
2. `tinyui/src/backend/ldgui/backend_app.c:542`
3. `tinyui/demo/message_box_basic/main.c:3`

这不只是“host smoke capture 能看到东西”，而是在真实 runtime 里替 demo 补布局。它违反仓库规则里“demo 不能掩盖 backend/layout 缺口”的边界，也会让 visible gate 把“被补位后的可见”误判成“真实布局完成”。

对后续版本的直接要求：

1. `a-0.4` 起手必须先去掉这类通用补位逻辑，或把它严格限制为显式 `temporary smoke path` 且不进入正式 visible 结论。
2. `message_box` 这类目前依赖补位才能稳定显示的 demo，不能被拿来当 `0.4` 的 shared-core 完整输入。

### 2. 中高：`list` 的 public getter 仍是 host cache，不是 backend truth

`tinyui_list_get_selected_index()` 直接返回 `list->selected_index`，而不是回读 backend。backend 已有 `tinyui_backend_list_get_selected_index()`，但 public getter 没走这条路径。

相关位置：

1. `tinyui/src/widgets/list.c:89`
2. `tinyui/src/backend/ldgui/backend_list.c:115`
3. `tinyui/src/backend/ldgui/backend_event.c:436`
4. `docs/tinyui-serial/a-0.3/current-15-capability-audit.md:112`

当前它之所以还没暴露，是因为 `list` 的 selection 入口仍很单一，主要靠 clicked-item bridge 同步 host cache。一旦 `a-0.4` 接入 `keyboard / focus / navigation`，这个 getter 很容易失真。

对后续版本的直接要求：

1. `a-0.4` 必须先冻结 “public getter 到底返回 host cache 还是 backend truth” 的统一规则。
2. `combo_box / scroll_selecter / line_edit` 都不能复制这种“host 侧值看起来能用，但不保证事件后真相一致”的模式。

### 3. 中等：`message_box` 证据只证明 host cache + confirm bridge，不证明真实行为闭环

`message_box` 的 getter 全是 host cache；unit test 直接调用底层 callback 指针；visible gate 也没覆盖真实点击命中或多按钮行为。

相关位置：

1. `tinyui/src/widgets/message_box.c:138`
2. `tests/tinyui/unit/test_tinyui_message_box.c:47`
3. `tinyui/demo/message_box_basic/main.c:3`

所以它当前最多只能证明：

1. 真实 `ldMessageBox` 被创建了。
2. 标题/正文/确认文案能下发。
3. confirm callback bridge 能接上。

它不能证明：

1. 真实交互点击路径。
2. 按钮命中与返回模型。
3. 多按钮/取消按钮扩展能力。

这与 `a-0.3` 把它标成 `minimal vertical slice only` 是一致的，但 `0.4/0.5` 不能把这种证据级别复制到高耦合控件。

### 4. 中等：visible gate 对 a-02 多数控件仍偏“像不像”，不是“值是否真实”

例如 `date_time_basic` 的 visible 只判断有足够宽的文本带，没有验证 demo 输入的 `"2026-05-31 12:34:56"` 是否真的出现在输出里。

相关位置：

1. `tests/tinyui/runtime/check_tinyui_visible_ui.py:886`

`clock / progress_wheel / qrcode` 也类似：能防空白图，但不防“值错了、文本错了、默认值顶掉了”。

对后续版本的直接要求：

1. 后续 gate 必须拆成两层：
   - `widget exists and visible`
   - `public API value survives frame/event and matches readback`
2. `a-0.5` 的 `table / graph / calendar` 尤其不能只靠结构启发式 visible 过关。

### 5. 中等：minimal-slice 单测普遍缺 lifecycle / frame / readback 回归

`progress_bar / progress_wheel / qrcode` 当前单测大多只覆盖 setter 后立即 getter，没有系统覆盖：

1. frame lifecycle 之后的状态一致性
2. theme / layout 干扰后的读回
3. backend 主动更新后的 public readback

这对 `a-0.3` 仍可接受，因为它们被诚实标成 `minimal vertical slice only`。但对 `a-0.4` / `a-0.5` 不够，尤其 shared-core 一旦介入 focus、navigation、edit model，回归网会明显偏薄。

### 6. 低：`a-0.3` 文档 closeout 基本成立，但索引仍有小冲突

当前主口径已经一致：

1. `README`
2. truth-source
3. capability audit
4. closeout 标准
5. spec
6. plan
7. release matrix

但 `README` 原先把后续版本记录写错到 `docs/tinyui-serial/a-0.3/a-0.4-a-0.6-后续版本记录.md`，实际文件在 `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`。

本轮已顺手修正索引。

## Closeout judgement

### 可以接受的部分

1. `a-0.3` 已经完成它自己的主任务：把旧 `9` 控件世界切到 `current-15` truth-source。
2. `4 + 5 + 6` 分层口径已经稳定。
3. machine-readable matrix、中文文档、contract gate 已基本一致。
4. GitNexus 视角下，这轮改动是低风险的 contract/doc 收敛，不是执行流程级改写。

### 不能误写成已完成的部分

1. 它不代表 runtime/layout 已经完全诚实。
2. 它不代表 host cache / backend truth 的 readback 边界已经统一。
3. 它不代表 a-02 的 visible gate 已经具备强行为回归能力。

因此最终判断是：

- `a-0.3` 在当时可以作为后续 `a-0.4` 历史串行线的正式输入，但不再构成当前 `v2.1` 主线入口。
- 但 `0.4` 的第一阶段必须先处理 smoke path、readback policy、gate 分层，而不是直接平推四个新控件。

## 0.4-0.5 串行建议

### `a-0.4` 应定义成输入 shared-core 版

严格顺序：

1. runtime/layout honesty 收口
2. focus ownership
3. editable text contract
4. keyboard bridge
5. selection / navigation contract
6. dropdown contract
7. `line_edit`
8. `keyboard`
9. `combo_box`
10. `scroll_selecter`

不接受的做法：

1. 四控件平推并行
2. 先做 demo 能弹出，再回头补 shared-core
3. 继续容忍 host cache getter 和 runtime 补布局

### `a-0.5` 应定义成 data / edit model 版

严格顺序：

1. item model
2. editable cell contract
3. `table`
4. graph series / value model
5. `graph`
6. calendar date / header / grid contract
7. `calendar`

不接受的做法：

1. `graph / calendar / table` 三控件平推
2. `table` 还没验证 edit model 就先铺 `graph`
3. 把 host cache 当成 shared/core 习惯扩散

## 当时推荐的文档与执行入口

1. `docs/tinyui-serial/a-0.4-线计划索引.md`
2. `docs/tinyui-serial/a-0.5-线计划索引.md`
3. `docs/superpowers/specs/2026-05-31-tinyui-a-0-4-input-shared-core-design.md`
4. `docs/superpowers/specs/2026-05-31-tinyui-a-0-5-data-model-design.md`

这些文档在当时共同承担：

1. 版本边界
2. 串行顺序
3. shared-owner 写面
4. subagent 拆分方式
5. gate 与 closeout 口径
