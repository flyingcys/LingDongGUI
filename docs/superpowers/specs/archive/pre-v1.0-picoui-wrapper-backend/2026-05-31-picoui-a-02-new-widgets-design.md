# PicoUI a-02 低耦合新控件扩面设计

> 日期：2026-05-31
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/picoui-serial/a-02-线计划索引.md`
> 目标：在不争抢 shared-owner 写面的前提下，把 `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 拆成一条长期串行新控件 vertical-slice worktree，用最少 shared 改动换取最快覆盖率提升，并为 fresh subagent 提供单线可执行边界。

---

## 1. 背景

当前 PicoUI 想加速，不能只看“还有多少控件没做”，还要看“哪些控件能不打架地做”。

仓库现状已经明确：

1. 当前已有 backlog 控件 `image / text / checkbox / switch / list`，这些控件天然会碰 shared widget/style/event/app/release-matrix 语义。
2. 这些 shared 热点已经集中交给 `a-01` 持有。
3. 若第二条长期 worktree 也去吃高耦合 shared 主题，结果不会是加速，而是 merge 冲突与合同漂移。

因此 `a-02` 必须与 `a-01` 形成明确分工：

- `a-01` 负责 current backlog 与 shared-owner
- `a-02` 负责低耦合新控件 vertical slice

`a-02` 的关键不是“挑最炫的控件”，而是挑那些能在尽量少修改 shared 层前提下，快速拉高 PicoUI 覆盖率的控件。

---

## 2. a-02 唯一目标

`a-02` 的唯一目标是：

在 `.worktree/a-02` 内，按固定顺序串行实现低耦合新控件 vertical slice，使每个新增控件都具备最小完整闭环：

1. public API
2. widget implementation
3. backend mapping
4. demo
5. unit / contract / runtime / mapping / visible 证据入口

这条线只回答四件事：

1. 哪些新控件当前最适合低冲突扩面。
2. 每个控件的最小 vertical slice 到哪里为止。
3. 什么情况下允许在阶段末尾做一次最小 shared 接入。
4. 怎样保证这条线对 `a-01` 的共享语义依赖最小化。

---

## 3. 非目标

`a-02` 不处理以下事情：

1. 不修 `image / text / checkbox / switch / list` backlog。
2. 不修改 release matrix 总口径。
3. 不争抢 `picoui/src/core/widget.c`、`backend_style_apply.c`、`backend_event.c`、`backend_app.c` 的持续写权。
4. 不做 `line_edit`、`combo_box`、`keyboard`、`table`、`graph`、`calendar`、`scroll selecter` 等高耦合输入/focus/dropdown/data-model 合同。
5. 不把单个 demo 通过写成“PicoUI 发布能力完成”。
6. 不把“先把控件创建出来”冒充成 vertical slice 完成。

---

## 4. 为什么是这六个控件

`a-02` 当前只允许以下候选：

1. `progress_bar`
2. `qrcode`
3. `progress_wheel`
4. `message_box`
5. `date_time`
6. `clock`

### 4.1 `progress_bar`

优先级最高。

理由：

1. 是高价值基础显示/进度控件。
2. 交互耦合低。
3. 与已有 `slider`、image-skin 经验相邻，但不直接要求输入、focus、keyboard 模型。

### 4.2 `qrcode`

第二优先级。

理由：

1. 是显示型控件。
2. 结构单一，低交互。
3. 可以明显提升覆盖率，但不强依赖 shared event/focus 合同。

### 4.3 `progress_wheel`

第三优先级。

理由：

1. 仍属进度类控件。
2. 与 `progress_bar` 相邻，便于复用进度类建模经验。
3. 相比输入/数据模型类控件，shared 冲突仍然较小。

### 4.4 `message_box`

第二批起点。

理由：

1. 复合结构比前两者更重。
2. 涉及 callback、按钮、文本和容器组合。
3. 仍比 `line_edit/combo_box` 更可控，但复杂度明显高于 `progress_bar/qrcode`。

### 4.5 `date_time`

第二批优先。

理由：

1. 本质上更接近轻量文本显示控件。
2. 能提升时间/日期场景覆盖率。
3. 不需要引入完整输入/focus 系统。

### 4.6 `clock`

第二批末尾。

理由：

1. 仍是显示型控件。
2. 包含指针/图片语义，但输入耦合低。
3. 复杂度高于 `date_time`，仍低于输入/数据模型类控件。

---

## 5. 为什么不做 `line_edit / combo_box`

这两类控件价值确实高，但当前不适合作为 `a-02` 首轮长期线范围。

原因：

1. **输入耦合重**
   - text editing
   - focus
   - keyboard
   - cursor/state machine
2. **shared event/focus 语义会立刻膨胀**
3. **容易把 `a-02` 从“低耦合扩面线”拖成“新输入系统线”**
4. **与 `a-01` 的 shared-owner 边界会变得模糊**

因此当前设计明确把：

- `line_edit`
- `combo_box`
- `keyboard`

全部排除出 `a-02` 首轮范围。

---

## 6. 固定范围与顺序

### 6.1 固定控件范围

`a-02` 只处理：

- `progress_bar`
- `qrcode`
- `progress_wheel`
- `message_box`
- `date_time`
- `clock`

### 6.2 固定串行顺序

内部顺序固定为：

1. `a-02-B1 progress_bar`
2. `a-02-B2 qrcode`
3. `a-02-B3 progress_wheel`
4. `a-02-B4 message_box`
5. `a-02-B5 date_time`
6. `a-02-B6 clock`

### 6.3 顺序原因

1. `progress_bar`
   - 最容易形成低冲突、高价值的第一批产出
2. `qrcode`
   - 继续扩大显示型控件覆盖率
3. `progress_wheel`
   - 先把进度类扩面做成一组
4. `message_box`
   - 再处理更重的复合控件
5. `date_time`
   - 用轻量文本显示控件补覆盖率
6. `clock`

## 6.4 当前执行状态

截至 `2026-05-31` 当前 worktree 真相：

- `a-02-B1 progress_bar` 已完成并收口
- `a-02-B2 qrcode` 已完成并收口
- `a-02-B3 progress_wheel` 已完成并收口
- `a-02-B4 message_box` 已完成并收口
- `a-02-B5 date_time` 已完成并收口
- `a-02-B6 clock` 已完成并收口
- `a-02-B7 closeout` 已完成并收口
- 后续阶段仍按既定固定顺序串行推进

`B1 progress_bar` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. fresh 独立 review 无 findings

它不等于：

1. `a-02` 整线完成
2. 所有后续控件能力已验证
3. PicoUI 发布能力已经整体 closeout

`B2 qrcode` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. fresh 独立 review 无 findings

它同样不等于：

1. `a-02` 整线完成
2. `progress_wheel / message_box / date_time / clock` 已验证
3. PicoUI 发布能力已经整体 closeout

`B3 progress_wheel` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. release-only crash 根因已修到 `ldProgressWheel_init()` 的 scene 参数传递
4. fresh 独立 review 无 findings

它同样不等于：

1. `a-02` 整线完成
2. `message_box / date_time / clock` 已验证
3. PicoUI 发布能力已经整体 closeout
   - 最后处理更重的显示型图片/指针控件

`B4 message_box` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. `confirm` callback 合同已真实接到底层 `ldMessageBoxSetCallback()`
4. fresh 独立 review 无 findings

它同样不等于：

1. `a-02` 整线完成
2. `date_time / clock` 已验证
3. PicoUI 发布能力已经整体 closeout
   - 最后处理更重的显示型图片/指针控件

`B5 date_time` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. `ldDateTime` 手动时间显示合同已修正，runtime 中不再被 `frame_start` 强制覆盖回系统时间
4. 当前回合受工具策略约束未再新开独立 reviewer；已补做主线程本地 review，未见新的阻断项

它同样不等于：

1. `a-02` 整线完成
2. `clock` 已验证
3. PicoUI 发布能力已经整体 closeout

`B6 clock` 的当前收口口径仅限：

1. public API、widget、backend、demo、unit 已落地
2. runtime / mapping / visible / contract 入口已接通并通过当前 targeted 验证
3. 当前 PicoUI 只承诺最小 `step_second` 指针显示合同，不承诺背景资源或高级动画系统
4. 当前回合受工具策略约束未再新开独立 reviewer；已补做主线程本地 review，未见新的阻断项

它同样不等于：

1. PicoUI 整体发布能力已经 closeout
2. 更高耦合输入类 backlog 已完成
3. 所有 LingDongGUI 原生时钟资源语义都已对 PicoUI 暴露

---

## 7. vertical slice 定义

对 `a-02` 来说，“一个控件完成”不是只新增几个文件名，而是必须形成最小闭环：

1. **public API**
   - 对外头文件存在
   - 不泄漏 `ld*` / `arm_2d_*` / `SIGNAL_*`
2. **widget implementation**
   - PicoUI 层对象、属性、生命周期、最小状态语义完整
3. **backend mapping**
   - 真实落到对应 `ld*` 控件
   - 不是 fake path
4. **demo**
   - demo 只使用 `picoui_*` API
   - 不靠 demo 代码承担适配补丁
5. **证据**
   - 至少有 unit / contract 入口
   - 若已接入 runtime demo，则同步接入 runtime / mapping / visible

这就是 `a-02` 的 vertical slice 最小完成定义。

---

## 8. 文件边界设计

### 8.1 每控件私有文件

`progress_bar` 阶段：

- `picoui/include/picoui/progress_bar.h`
- `picoui/src/widgets/progress_bar.c`
- `picoui/src/backend/ldgui/backend_progress_bar.c`
- `picoui/demo/progress_bar_basic/main.c`
- `tests/picoui/unit/test_picoui_progress_bar.c`

`qrcode` 阶段：

- `picoui/include/picoui/qrcode.h`
- `picoui/src/widgets/qrcode.c`
- `picoui/src/backend/ldgui/backend_qrcode.c`
- `picoui/demo/qrcode_basic/main.c`
- `tests/picoui/unit/test_picoui_qrcode.c`

`progress_wheel` 阶段：

- `picoui/include/picoui/progress_wheel.h`
- `picoui/src/widgets/progress_wheel.c`
- `picoui/src/backend/ldgui/backend_progress_wheel.c`
- `picoui/demo/progress_wheel_basic/main.c`
- `tests/picoui/unit/test_picoui_progress_wheel.c`

`message_box` 阶段：

- `picoui/include/picoui/message_box.h`
- `picoui/src/widgets/message_box.c`
- `picoui/src/backend/ldgui/backend_message_box.c`
- `picoui/demo/message_box_basic/main.c`
- `tests/picoui/unit/test_picoui_message_box.c`

`date_time` 阶段：

- `picoui/include/picoui/date_time.h`
- `picoui/src/widgets/date_time.c`
- `picoui/src/backend/ldgui/backend_date_time.c`
- `picoui/demo/date_time_basic/main.c`
- `tests/picoui/unit/test_picoui_date_time.c`

`clock` 阶段：

- `picoui/include/picoui/clock.h`
- `picoui/src/widgets/clock.c`
- `picoui/src/backend/ldgui/backend_clock.c`
- `picoui/demo/clock_basic/main.c`
- `tests/picoui/unit/test_picoui_clock.c`

### 8.2 默认禁改 shared-owner 文件

以下文件默认禁止 `a-02` 持续修改：

- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_app.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

### 8.3 聚合文件策略

以下文件允许在每个控件阶段末尾做一次最小接入，但不要高频反复改：

- `picoui/include/picoui/picoui.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_public_api.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`
- `picoui/docs/demo_guide.md`

---

## 9. 与 a-01 的协作规则

`a-02` 必须把 `a-01` 视作 shared-owner，而不是并列任意改。

固定规则：

1. `a-02` 不修 backlog 五控件。
2. `a-02` 不改 release matrix 状态定义。
3. `a-02` 若发现 shared 语义缺口，优先记录到阶段文档或 review，由 `a-01` 吸收。
4. 只有当某个新控件在阶段末尾必须接入 shared 层时，`a-02` 才允许做一次最小 shared 接入。
5. 做 shared 接入前，必须先 rebase 到 `a-01` 最近 checkpoint。

这样设计的目的，是保证 `a-02` 永远还是“低耦合扩面线”，而不是逐步演化成第二条 shared-contract 主线。

---

## 10. 各阶段的设计要求

### 10.1 `a-02-B1 progress_bar`

重点：

1. 先拿一个基础高价值新控件起量。
2. 明确 value/percent/orientation/image-skin 的最小合同。
3. 避免直接复制 `slider` 的历史接口误差。

至少要保证：

- public API 边界清晰
- 真实落到 `ldProgressBar`
- demo 只表达用户意图

### 10.2 `a-02-B2 qrcode`

重点：

1. 构建显示型控件 vertical slice 模板。
2. 明确 text/source/setter 边界。
3. 避免把资源管理或复杂主题系统提前吸进来。

至少要保证：

- 真实落到 `ldQRCode`
- 不把资源/颜色系统扩张成额外新线

### 10.3 `a-02-B3 progress_wheel`

重点：

1. 把进度类第二个低耦合控件补上。
2. 明确圆形/环形进度的最小合同。
3. 尽量复用 `progress_bar` 阶段已经验证过的 progress 类方法论。

至少要保证：

- 真实落到 `ldProgressWheel`
- 不把动画、复杂主题或更重仪表语义偷带进来

### 10.4 `a-02-B4 message_box`

重点：

1. 在前两者稳定后，再处理更重的复合控件。
2. 明确 title/message/button/callback 的最小合同。
3. 避免 message-box 反向侵入 shared event 主线。

至少要保证：

- callback 边界诚实
- 不把 modal、焦点系统、复杂交互全部捆进本阶段

### 10.5 `a-02-B5 date_time`

重点：

1. 用轻量日期时间显示控件继续扩覆盖率。
2. 明确 format/date/time/setter 的最小合同。
3. 不把 calendar/input/date-editing 系统吸进来。

至少要保证：

- 真实落到 `ldDateTime`
- 只承诺显示合同，不偷渡完整日期编辑系统

### 10.6 `a-02-B6 clock`

重点：

1. 在第二批末尾处理更重的显示型控件。
2. 明确 background/pointer/step-second 的最小合同。
3. 不把高级动画或复杂资源系统偷带进来。

至少要保证：

- 真实落到 `ldClock`
- 只承诺当前 PicoUI 明确暴露的显示合同

---

## 11. 证据层与验收要求

`a-02` 每个控件阶段最小完成态至少要有：

1. public header
2. widget/backend 基础实现
3. unit 或 targeted contract 入口
4. demo 与 demo boundary 验证

若阶段已接入 runtime demo，则继续要求：

1. runtime smoke
2. backend mapping
3. visible gate

每阶段最小验收至少包含：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
git diff --check
```

若已接入 demo/runtime，再补：

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo <demo_name>
```

若已新增 unit target，再补：

```bash
ctest --test-dir build -R <test_target> --output-on-failure
```

---

## 12. merge-back 设计

`a-02` 推荐按控件单独回灌：

1. `progress_bar`
2. `qrcode`
3. `progress_wheel`
4. `message_box`
5. `date_time`
6. `clock`

不建议把多个新控件积压到一个超大 diff 再回灌。原因：

1. 更容易与 `a-01` 的聚合文件接入冲突。
2. 更不利于判断某个新控件的真实稳定性。
3. 不利于主线程做 blast-radius 和验证收敛。

主线程每次回灌前至少检查：

```bash
git status --short --branch --ignore-submodules=all
git diff --stat
git diff --check
```

---

## 13. 成功标准

`a-02` 成功，不等于“一次把所有高价值新控件都做完”，而是满足以下条件：

1. `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 至少前四项能形成真实 vertical slice，完整目标是六项都形成真实 vertical slice。
2. 每个新增控件都不依赖长期争抢 shared-owner 文件。
3. `a-02` 每阶段都可单独 merge-back。
4. `a-01` 与 `a-02` 之间的边界仍清晰，没有演化成双 shared 主线。
5. 覆盖率提升来自真实新控件闭环，不是来自名义文件新增。
6. 这一轮结束后，PicoUI 数量覆盖从 `9 / 26` 提升到 `15 / 26`，约 `57.7%`，但仍不是 LingDongGUI 全控件覆盖。

---

## 14. blocker 定义

以下情况会判定 `a-02` 设计失败或执行失控：

1. `a-02` 提前吸入 `line_edit / combo_box / keyboard`。
2. `a-02` 长期重写 shared-owner 文件。
3. `a-02` 在同一阶段并行推进两个以上新控件。
4. `a-02` 的 demo/runtime 接入长期滞后，导致只新增代码不补证据。
5. `a-02` 通过修改 demo 逻辑或伪视觉来掩盖 backend 缺口。
6. `a-02` 继续向 `table / graph / calendar / scroll selecter` 这类高耦合控件无边界扩张，导致不再是低耦合扩面线。
