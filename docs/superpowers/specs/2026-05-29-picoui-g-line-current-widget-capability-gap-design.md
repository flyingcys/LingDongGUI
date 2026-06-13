# TINYUI G线当前控件能力缺口收口设计文档

> 日期：2026-05-29
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 建议 worktree：`.worktree/tinyui-g-current-capability-gap`
> 入口索引：`docs/tinyui-serial/G-线计划索引.md`

## 1. 背景

`A/B/C/D/F` 之后，`TINYUI` 已有一条稳定主线：

- 当前已完成控件都已接到真实 `LingDongGUI` backend。
- `smoke`、`backend mapping`、`automatic visible`、`manual artifact` 四层证据边界已经固定。
- `window/label/button/checkbox/switch/slider/text/image/list` 已存在 public API、真实 backend 对象或 vertical slice 证据。

但最新 review 也说明，当前状态不能写成“TINYUI 已 100% 支持对应 `ld*` 控件的全部能力”。原因不是 backend 没接通，而是：

- TINYUI public API 明显比对应 `ld*` header 窄。
- 若干能力仍停在 TINYUI 字段合同、theme 子集、event bridge 子集。
- `list` 已暴露一个典型问题：public API 已经暴露，但 native selection event contract 还没闭环。

因此需要一条新的主线，把“真实 backend 已接通”与“能力是否已完整封装”这两个问题拆开，统一收口到可执行的真相源里。

## 2. 目标

`G线` 的目标不是继续扩新控件数量，而是把当前已完成控件的能力缺口、证据边界和下一阶段补齐顺序明确下来。完成后应满足：

- 有一份逐控件 capability gap matrix，直接对照 TINYUI public API 与对应 `ld*` 公开能力。
- 每个能力都明确标记为 `support`、`reject`、`deferred` 或 `incomplete_contract`。
- 当前已知过强表述被收口，不再把 vertical slice、mapping marker 或 visible gate 误写成全能力完成。
- 后续实现任务能按 shared write surface 串行推进，并适合 fresh subagent 独立接手。

## 3. 非目标

- 不新增 `TINYUI` 新控件；新控件扩张不属于 `G线`。
- 不把 `TINYUI` 改造成 `ld*` 底层 API 的逐项镜像层。
- 不用 demo 外观、readback 或 marker 通过来替代真实能力合同。
- 不重新定义既有四层证据边界。

## 4. 设计

### 4.1 G线真相源

`G线` 的第一真相源按优先级排序如下：

1. 当前源码：
   - `tinyui/include/tinyui/*.h`
   - `tinyui/src/*`
   - `src/gui/ld*.h`
2. 合同与 gate：
   - `tests/tinyui/unit/*`
   - `tests/tinyui/contract/*`
   - `tests/tinyui/runtime/*`
3. `G线` 索引、spec、implementation plan

后续若 `D/F` 文档与当前源码冲突，以源码和 `G线` 文档为准。

### 4.2 capability gap matrix 模型

`G线` 的核心产物是一张逐控件能力差距矩阵。矩阵至少覆盖：

- `window -> ldWindow`
- `label -> ldLabel`
- `button -> ldButton`
- `checkbox -> ldCheckBox`
- `switch -> ldSwitch`
- `slider -> ldSlider`
- `text -> ldText`
- `image -> ldImage`
- `list -> ldList`

每个能力项必须包含：

- TINYUI 当前 public API 或缺失状态
- 对应 `ld*` 能力入口
- 状态：
  - `support`
  - `reject`
  - `deferred`
  - `incomplete_contract`
- 证据层：
  - `unit`
  - `contract`
  - `mapping`
  - `visible`
  - `manual artifact`

`incomplete_contract` 专门用于这种情况：

- public API 已暴露
- 调用方会自然认为能力已存在
- 但真实 backend 行为或 native contract 仍未闭环

当前已知例子：`tinyui_list_set_on_selected()`。

### 4.3 当前优先缺口

#### a. list selection contract

- `tinyui_list_set_on_selected()` 目前只保存 callback/user_data。
- 当前还没有 `ldList` native selection event bridge。
- 因此该 API 不能继续写成 `support`，应先按 `incomplete_contract` 收口，再决定补实现还是明确降级。

#### b. list mapping marker 语义

- `PICOUI_BACKEND_REAL_WIDGET_IDS` 当前同时承载 `list` 和 `item_*`。
- `item_*` 现在只是写入 `ldListSetText()` 的 payload marker。
- 后续应拆成更准确的 marker 语义，例如 list widget marker 与 list item payload marker 分离。

#### c. slider range contract

- `tinyui_slider_set_range()` 当前只更新 TINYUI shadow state。
- 当 range clamp 影响当前值时，不能自动推导为底层 `ldSlider` percent 已同步。
- 这项要么补真实同步和测试，要么在矩阵里明确降级为 `deferred`。

#### d. direct style setter 基座语义

- `bg_color/text_color/border_color/radius` 当前主要是 TINYUI 字段合同。
- 若要把它们提升成“实时 backend style setter”，必须补真实 `ld*` 同步与可验证证据。
- 若不提升，文档必须继续保持“字段合同”口径。

### 4.4 后续实现顺序

`G线` 推荐顺序不是按控件数量推进，而是按风险和共享写面推进：

1. 先冻结 capability gap matrix
2. 先收口当前口径错误
3. 先补 list 这类已暴露 public contract 但未闭环的能力
4. 再处理 shared widget base 语义
5. 最后补 widget-specific 高价值缺口

这样可以避免：

- 一边补 API 一边改真相源，导致文档持续漂移
- 多个 subagent 同时写 shared backend 文件
- 把后续代码修复又写成新的“过强结论”

### 4.5 并行边界

`G线` 可以使用多个 subagent，但只允许在写面不重叠时并行。默认规则：

- 矩阵/文档类任务可单独一条线。
- `list` contract / marker 任务串行，因为会共享：
  - `tinyui/include/tinyui/list.h`
  - `tinyui/src/widgets/list.c`
  - `tinyui/src/backend/ldgui/backend_event.c`
  - `tinyui/src/backend/ldgui/backend_app.c`
  - `tests/tinyui/unit/test_tinyui_list.c`
  - `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- shared widget base 任务串行，因为会共享：
  - `tinyui/src/core/widget.c`
  - `tinyui/src/backend/ldgui/backend_style_apply.c`
  - `tinyui/src/backend/ldgui/backend_theme.c`
  - `tinyui/include/tinyui/widget.h`
- widget-specific 扩面可按“交互控件组 / 展示控件组”拆分，但每组内部仍串行。

review 后修复必须回原 subagent，不新开 subagent 修复，也不由主线程顺手修。

## 5. 验收

`G线` 的最小验收分两层：

### 文档层

- `G线` 索引、spec、plan 存在且口径一致。
- capability gap matrix 覆盖 9 类控件。
- 当前不能写成“100% 支持”的理由在文档中是可追溯的，不靠口头描述。

### 实现层

任何后续 `G线` 子任务完成时，汇报必须明确区分：

- 哪些是 public contract 已变更
- 哪些是 backend 映射已变更
- 哪些是 `unit/contract/mapping/visible/manual artifact` 证据
- 哪些能力仍是 `reject/deferred/incomplete_contract`

最小门禁仍沿用既有 TINYUI 集合：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
git diff --check
```

如果某阶段不适合跑全集，必须在阶段文档中明确缩小后的 targeted gate 和缩小原因。
