# TINYUI a-01 已 Wrapped Backlog 收口设计

> 日期：2026-05-31
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/tinyui-serial/a-01-线计划索引.md`
> 目标：把当前已经 `wrapped` 但仍未真正收口的 `image / text / checkbox / switch / list` 五个 TINYUI backlog 控件，放进一个长期串行 worktree 中做实合同、真实 backend 语义和证据层，同时把 shared 写面集中到同一条线上，减少多 worktree 互撞。

---

## 1. 背景

当前 TINYUI 并行开发要解决的不是“有没有任务可做”，而是“怎么拆才能真加速而不是互相冲突”。

仓库现状已经明确：

1. `J线` 已把当前 `9` 个 TINYUI 控件分成两层：
   - `v0.1 parity`：`window / label / button / slider`
   - `v0.2 backlog`：`checkbox / switch / text / image / list`
2. `image / text / checkbox / switch / list` 这 `5` 个控件都不是“完全没做”，而是：
   - 已有 TINYUI public widget
   - 已有真实 LingDongGUI backend mapping
   - 但仍有能力缺口、合同边界不稳或 shared 语义未完全钉死
3. 这些 backlog 控件很容易碰 shared 热点文件：
   - `tinyui/src/core/widget.c`
   - `tinyui/src/backend/ldgui/backend_style_apply.c`
   - `tinyui/src/backend/ldgui/backend_event.c`
   - `tinyui/src/backend/ldgui/backend_app.c`
   - `tests/tinyui/contract/tinyui_release_capability_matrix.json`

因此不能把这 `5` 个 backlog 控件拆成多个长期并行 worktree 分头改。那样表面上人多线多，实际会在 shared 合同、release matrix、event/style 语义上频繁撞车。

`a-01` 的设计目标，就是把这些“天然会碰 shared 层”的 backlog 控件合并进同一条长期 worktree，内部严格串行开发，外部与 `a-02` 形成最小冲突拓扑。

---

## 2. a-01 唯一目标

`a-01` 的唯一目标是：

在 `.worktree/a-01` 内，按固定顺序串行收口 `image / text / checkbox / switch / list` 五个 backlog 控件，使其当前公开承诺的 TINYUI 合同更真实、更稳定、证据更完整，同时把 shared 层改动集中在同一条线内。

这条线只回答四件事：

1. 当前 backlog 控件各自承诺什么、不承诺什么。
2. 哪些能力要升级成真实 public contract。
3. 哪些能力必须继续冻结为 `reject / deferred / incomplete_contract`。
4. 哪些 shared 语义必须由同一条线统一持有，避免与新控件线互撞。

---

## 3. 非目标

`a-01` 不处理以下事情：

1. 不新增 `progress_bar`、`qrcode`、`message_box`、`line_edit`、`combo_box`。
2. 不回头重开 `window / label / button / slider` 的 `v0.1 parity` 主收口。
3. 不把 `H线/J线` 发布 closeout 文档改成当前开发主索引。
4. 不通过改 demo 用户意图、硬编码 `set_size()/set_pos()`、补 fake visual，来掩盖 backend/layout/theme 缺口。
5. 不把 “已 wrapped” 偷换成 “parity complete”。
6. 不追求这五个控件一次性达到 `public v1.0` 全能力镜像。

---

## 4. 为什么 a-01 必须独占 shared 写面

`image / text / checkbox / switch / list` 看起来是五个控件，但它们共享多条容易扩散的合同轴：

1. **通用 widget 语义**
   - visible
   - enabled
   - style_class
   - user_data
   - padding
2. **shared style/theme 语义**
   - `bg_color / text_color / border_color / radius`
   - `PICOUI_PART_*` 映射
   - “有 style setter” 是否真的等于 backend 有自然语义
3. **shared event / bridge 语义**
   - toggled / selected / disabled
   - callback 和 native state 是否一致
4. **shared runtime / mapping 口径**
   - runtime marker 的语义边界
   - 真实 widget id 与 payload marker 的区分
5. **shared release matrix 口径**
   - 哪些能力是 `support`
   - 哪些能力是 `reject / deferred / incomplete_contract`

这些问题如果分散到多个 worktree，merge 阶段几乎必然出现：

1. 同一 shared 文件被多线并行改。
2. 不同控件对相同公共语义给出相互矛盾的答案。
3. release matrix 被多次重写。
4. 某条线把另一条线刚刚收紧的边界又放松回去。

因此本设计明确规定：`a-01` 是 current backlog 的 shared-owner 线。

---

## 5. 范围与顺序

### 5.1 固定控件范围

`a-01` 只处理：

- `image`
- `text`
- `checkbox`
- `switch`
- `list`

### 5.2 固定串行顺序

内部顺序固定为：

1. `a-01-A1 image`
2. `a-01-A2 text`
3. `a-01-A3 checkbox`
4. `a-01-A4 switch`
5. `a-01-A5 list`

### 5.3 顺序原因

1. `image`
   - 最适合先钉死“哪些 style/theme/enabled 语义根本不存在”
   - 能最早减少错误扩张
2. `text`
   - 主要是字体、背景、readback 合同
   - 交互耦合低
3. `checkbox`
   - 会碰 toggled、checked、style
4. `switch`
   - 会继续碰 checked/disabled/style/event
   - 与 `checkbox` 相邻处理更利于 shared 语义保持一致
5. `list`
   - 最容易扩散到 marker、callback、selection bridge、shared app/event 口径
   - 必须放最后

---

## 6. 共享所有权设计

### 6.1 `a-01` 独占文件

以下文件默认只允许 `a-01` 持续修改：

- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_style_apply.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tinyui/src/backend/ldgui/backend_app.c`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

### 6.2 `a-01` 控件内文件

以下文件允许按阶段修改，但每个阶段只碰本控件相关逻辑：

- `tinyui/src/widgets/image.c`
- `tinyui/src/widgets/text.c`
- `tinyui/src/widgets/checkbox.c`
- `tinyui/src/widgets/switch.c`
- `tinyui/src/widgets/list.c`
- `tinyui/src/backend/ldgui/backend_image.c`
- `tinyui/src/backend/ldgui/backend_text.c`
- `tinyui/src/backend/ldgui/backend_checkbox.c`
- `tinyui/src/backend/ldgui/backend_switch.c`
- `tinyui/src/backend/ldgui/backend_list.c`
- `tests/tinyui/unit/test_tinyui_widgets.c`
- `tests/tinyui/unit/test_tinyui_theme.c`
- `tests/tinyui/unit/test_tinyui_list.c`

### 6.3 聚合文件策略

以下聚合文件不应在日常开发中高频来回改，统一留到阶段末尾一次性接入：

- `tinyui/include/tinyui/tinyui.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

---

## 7. 各阶段的设计要求

### 7.1 `a-01-A1 image`

重点：

1. 把 `image` 当前真实支持面写清。
2. 把不存在自然 backend 语义的项目钉死为限制。
3. 防止后续其他控件或文档把 `image` 误写成“也支持通用 widget style 子集”。

至少要保证：

- `theme` 不被偷写成 support
- `enabled` 不被偷换成 `visible/opacity/selectable`
- `style_class / user_data` 如仍无真实 backend 语义，明确保持 `incomplete_contract`

### 7.2 `a-01-A2 text`

重点：

1. 收口字体、背景、颜色、readback 语义。
2. 明确 `text` 与 `label` 的边界，不把 `label` 的结论直接复用到 `text`。

至少要保证：

- `font` 是真实合同，不是仅存储字段
- `text/bg/align/font` 的读写语义自洽

### 7.3 `a-01-A3 checkbox`

重点：

1. 收口 checked/toggled 路径。
2. 明确样式支持面与限制。

至少要保证：

- checked state 与 native state 一致
- callback 不只是 shadow state 自转
- `radio group / image mode` 这类未承诺能力继续诚实限制

### 7.4 `a-01-A4 switch`

重点：

1. 收口 checked/disabled/style/event 语义。
2. 和 `checkbox` 保持 shared style/event 判断标准一致。

至少要保证：

- disabled 不是伪语义
- toggled callback 与 native state 一致
- `direction/navigation` 等更重能力不被偷渡进当前完成面

### 7.5 `a-01-A5 list`

重点：

1. 最后处理最易扩散的 selection / marker / callback / app runtime 口径。
2. 明确 item marker 不是独立 backend widget。
3. 明确 widget-level `user_data` 与 callback cookie 的关系。

至少要保证：

- runtime marker 不夸大
- `on_selected` 若未完全 native bridge，就不能写成完成
- selection / selected index / callback 三者语义一致

---

## 8. 与 a-02 的隔离规则

`a-01` 与 `a-02` 不是对等混写关系，而是：

- `a-01` 持有 shared-owner 权
- `a-02` 走低耦合新控件 vertical slice

因此必须固定以下隔离规则：

1. `a-01` 不碰 `progress_bar / qrcode / message_box` 新控件文件。
2. 只要 `a-01` 正在改 shared-owner 文件，`a-02` 不得并行改这些文件。
3. `a-02` 若必须在阶段末尾接入共享聚合文件，必须先 rebase 到 `a-01` 最近 checkpoint。
4. `a-01` 发现新控件线依赖的 shared 语义缺口时，可以定义边界，但不直接接手新控件实现。

---

## 9. 证据层与验收要求

`a-01` 每个阶段都必须继续使用 TINYUI 当前五层证据口径：

1. `unit`
2. `contract`
3. `mapping`
4. `visible`
5. `manual_artifact`

但 `a-01` 是 backlog 收口线，不是发布 closeout 线，所以：

1. 阶段性完成至少要补齐 `unit / contract`，按需补 `mapping / visible`
2. 不能把某一层通过外推成“该控件已经 release ready”
3. 若阶段只收口 shared 合同，不强制同步补全部人工证据，但必须保证文档口径诚实

每阶段最小验收至少包含：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L tinyui --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
git diff --check
```

若阶段涉及 mapping / visible / release matrix，再补：

```bash
ctest --test-dir build -L mapping --output-on-failure
ctest --test-dir build -L visible --output-on-failure
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

---

## 10. merge-back 设计

`a-01` 不应等五个控件全部完成后再一次性回灌。推荐三批 merge-back：

1. `image + text`
2. `checkbox + switch`
3. `list`

原因：

1. 可以更早回收已经稳定的 shared 语义。
2. 避免 worktree 长时间积压过大 diff。
3. 降低与 `a-02` 在聚合文件和验证入口上的冲突概率。

主线程回灌前至少检查：

```bash
git status --short --branch --ignore-submodules=all
git diff --stat
git diff --check
```

---

## 11. 成功标准

`a-01` 成功，不等于“五个控件全部 public v1.0 完成”，而是满足以下条件：

1. backlog 五控件的当前合同边界比现在更清晰。
2. shared widget/style/event/app 语义由同一条线统一收口，不再多线漂移。
3. `reject / deferred / incomplete_contract` 状态更稳定，不会被后续轻易误写成 support。
4. 每个阶段都能独立 merge-back，不需要等到整条线全部结束。
5. `a-02` 能在不抢 shared-owner 的前提下继续推进新控件扩面。

---

## 12. blocker 定义

以下情况会判定 `a-01` 设计失败或执行失控：

1. backlog 五控件被拆到多个 worktree 并行改 shared 语义。
2. `a-01` 长期同时推进两个以上 backlog 控件，导致阶段边界模糊。
3. `a-01` 把新控件扩张工作顺手吸进来。
4. `a-01` 频繁提前改聚合文件，导致与 `a-02` merge 冲突暴涨。
5. shared-owner 文件被 `a-02` 或其他线同时重写，且未先通过主线程收敛边界。

---

## 13. 实施后结论

当前 `.worktree/a-01` 已按本设计把 `image / text / checkbox / switch / list` 五个 backlog 控件收口到稳定实现口径。

### 13.1 image

- `theme`、`bg/text/border/radius colors`、`enabled` 保持明确 `reject`
- `style_class / user_data` 保持 metadata-only，合同状态仍是 `incomplete_contract`
- backend 不新增 fake style 或 fake disabled 路径

### 13.2 text

- `text/font/bg/text_color/align` 当前合同已由 unit + contract 证据稳定
- `font` 不是纯字段缓存，backend readback 路径已接通

### 13.3 checkbox / switch

- `checked` 与 native state 同步
- native bridge 优先，不把 callback 建在 shadow state 自转上
- `switch disabled` 是真实语义，不再靠伪状态近似

### 13.4 list

- `selected_index / callback / backend value` 语义一致
- `on_selected(..., user_data)` 的 callback cookie 与 widget-level `user_data` 保持分离
- `item_marker` 明确维持 `reject`，不把 payload marker 写成独立 backend widget

### 13.5 runtime gate 约束

- `check_tinyui_backend_mapping`
- `check_tinyui_visible_ui`
- `check_tinyui_runtime`

三个 gate 共用 `build/tinyui-runtime`，必须串行运行。

### 13.6 仍然不是的结论

本线完成后，当前 backlog 五控件的合同边界、shared 语义和自动证据层已经收紧，但这仍然不等于：

- `release ready`
- `public v1.0 parity complete`
- `manual artifact gate` 已自动完成
