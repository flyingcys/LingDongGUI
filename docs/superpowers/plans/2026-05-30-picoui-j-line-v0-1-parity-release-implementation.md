# PicoUI J线 `v0.1` 对齐发布 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `J线` 串行收口为 `v0.1 = window / label / button / slider` 的首版功能对齐发布线，并把其余 `5` 个已做控件稳定转入 `v0.2` backlog。

**Architecture:** 本计划不新增控件，不追求一次吃掉全部 `9` 个控件。执行顺序固定为：先冻结 `v0.1/v0.2` 边界和矩阵状态，再逐控件收口 `window / label / button / slider`，最后统一改合同、清单、发布说明和证据层。每个 Task 使用 fresh subagent，写面不得重叠；涉及 C/Python 符号改动前必须跑 GitNexus impact。

**Tech Stack:** Markdown serial docs、PicoUI C API、LingDongGUI backend、Python3 contract gate、CTest、runtime visible/manual artifact scripts、GitNexus

---

## 0. 执行规则

- 每次只推进一个 Task。
- 每个 Task 使用一个 fresh subagent 执行；review 由独立 review subagent 只读完成。
- review 不通过时，由原执行 subagent 在同一上下文里修，不开新修复线程。
- 当前任务若在独立 worktree 执行，worktree 必须创建在 `.worktree/`，并先执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- 纯文档 Task 可不跑 GitNexus impact。
- 涉及 C / Python 符号修改的 Task，改动前必须对关键符号跑 GitNexus impact；若结果为 `HIGH` 或 `CRITICAL`，先在任务汇报中显式说明，再决定是否继续。
- 每个 Task 结束前至少运行 `git diff --check`。
- 涉及代码改动的 Task 必须运行对应单测 / gate / targeted runtime 验证。
- 若要提交，提交前必须运行 `gitnexus_detect_changes(scope="all")`，确认影响面仍然集中在当前 Task。

## 1. 文件结构与写面分组

### J线文档基线组

**文件：**
- Modify: `docs/picoui-serial/J-线计划索引.md`
- Create: `docs/superpowers/specs/2026-05-30-picoui-j-line-v0-1-parity-release-design.md`
- Create: `docs/superpowers/plans/2026-05-30-picoui-j-line-v0-1-parity-release-implementation.md`
- Modify: `docs/picoui-serial/H-线计划索引.md`
- Modify: `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`

职责：

- 冻结 `J线` 的 `v0.1/v0.2` 边界。
- 明确 `H线` 与 `J线` 的职责分流。

### parity matrix / gate 组

**文件：**
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Optional Create: `tests/picoui/contract/check_picoui_j_line_parity.py`
- Modify: `tests/picoui/CMakeLists.txt`

职责：

- 在不破坏 `H线` schema 的前提下，把 `wrapped` 与 `parity-complete` 分开表达。
- 为 `J线` 增加机器可读的 `v0.1/v0.2` 分流 gate。

### `window` 对齐组

**文件：**
- Modify: `picoui/include/picoui/window.h`
- Modify: `picoui/src/widgets/window.c`
- Modify: `picoui/src/backend/ldgui/backend_window.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_theme.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

职责：

- 收口 `window` 的 image / padding-group / readback 缺口。

### `label` 对齐组

**文件：**
- Modify: `picoui/include/picoui/label.h`
- Modify: `picoui/src/widgets/label.c`
- Modify: `picoui/src/backend/ldgui/backend_label.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

职责：

- 收口 `label` 的 transparent / align / background image / readback。

### `button` 对齐组

**文件：**
- Modify: `picoui/include/picoui/button.h`
- Modify: `picoui/src/widgets/button.c`
- Modify: `picoui/src/backend/ldgui/backend_button.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_event.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

职责：

- 收口 `button` 的 image skin / transparent / font / checkable / key_value / pressed state。

### `slider` 对齐组

**文件：**
- Modify: `picoui/include/picoui/slider.h`
- Modify: `picoui/src/widgets/slider.c`
- Modify: `picoui/src/backend/ldgui/backend_slider.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_event.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

职责：

- 收口 `slider` 的 horizontal / image skin / indicator width / slim size。

### 发布文案与证据收尾组

**文件：**
- Modify: `docs/picoui-serial/H-线当前9控件发布合同.md`
- Modify: `docs/picoui-serial/H-线已支持控件清单.md`
- Modify: `docs/picoui-serial/H-线第一版发布说明.md`
- Modify: `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `docs/picoui-serial/H-线发布测试矩阵.md`
- Modify: `docs/picoui-serial/C-线人工窗口验收记录.md`
- Optional Create: `docs/picoui-serial/J-线v0.1-closeout前状态.md`

职责：

- 把文案统一改成 `v0.1 已对齐 4 控件 / v0.2 backlog 5 控件`。
- 补齐 closeout 前证据与最终人工结论。

## 2. 串行任务拆分

### Task 1: J0-J1 边界冻结与 parity 状态分层

**Files:**
- Modify: `docs/picoui-serial/J-线计划索引.md`
- Modify: `docs/picoui-serial/H-线计划索引.md`
- Modify: `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Optional Create: `tests/picoui/contract/check_picoui_j_line_parity.py`
- Modify: `tests/picoui/CMakeLists.txt`
- Reference: `docs/superpowers/specs/2026-05-30-picoui-j-line-v0-1-parity-release-design.md`

- [ ] **Step 1: 复核 H/J 现状文档与当前 release matrix**

Run:

```bash
sed -n '1,260p' docs/picoui-serial/J-线计划索引.md
sed -n '1,220p' docs/picoui-serial/H-线计划索引.md
sed -n '220,420p' docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md
python3 -m json.tool tests/picoui/contract/picoui_release_capability_matrix.json >/dev/null
```

Expected:

- 当前 `v0.1 = window / label / button / slider`
- 当前 `v0.2 backlog = checkbox / switch / text / image / list`
- 现有 matrix 还没有把 `wrapped` 与 `parity-complete` 分层

- [ ] **Step 2: 对 matrix/gate 相关符号跑 GitNexus impact**

Run:

```text
impact target: check_picoui_release_capability_matrix
impact target: tests/picoui/CMakeLists.txt
```

Expected:

- 记录 blast radius
- 若为 `HIGH/CRITICAL`，先在任务汇报里说明

- [ ] **Step 3: 为 `J线` 追加非破坏 parity 分层字段**

要求：

- 不推翻 `H线` 现有 schema
- 至少能表达：
  - `wrapped`
  - `v0_1_parity_target`
  - `v0_2_parity_backlog`
  - `parity_complete`
- `window / label / button / slider` 固定进 `v0.1 target`
- `checkbox / switch / text / image / list` 固定进 `v0.2 backlog`

- [ ] **Step 4: 接入 parity gate**

Run:

```bash
ctest --test-dir build -N -R "picoui.*parity|release"
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
```

Expected:

- `ctest -N` 能列出更新后的 gate
- script 通过

- [ ] **Step 5: 文档自检**

Run:

```bash
git diff --check -- docs/picoui-serial/J-线计划索引.md \
                  docs/picoui-serial/H-线计划索引.md \
                  docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md \
                  tests/picoui/contract/picoui_release_capability_matrix.json \
                  tests/picoui/contract/check_picoui_release_capability_matrix.py \
                  tests/picoui/CMakeLists.txt
```

Expected:

- 无格式错误
- H/J 文档口径一致

- [ ] **Step 6: 独立 review**

要求：

- review subagent 重点检查：
  - H/J 职责有没有混写
  - `v0.1/v0.2` 分流是否稳定
  - matrix/gate 是否混淆 wrapped 与 parity-complete

### Task 2: J2 `window` 对齐收口

**Files:**
- Modify: `picoui/include/picoui/window.h`
- Modify: `picoui/src/widgets/window.c`
- Modify: `picoui/src/backend/ldgui/backend_window.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Optional Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 复核 `window` 现状与 `ldWindow` 缺口**

Run:

```bash
sed -n '1,220p' src/gui/ldWindow.h
sed -n '1,220p' picoui/include/picoui/window.h
sed -n '1,220p' picoui/src/widgets/window.c
sed -n '1,220p' picoui/src/backend/ldgui/backend_window.c
rg -n "window" tests/picoui/unit/test_picoui_widgets.c tests/picoui/unit/test_picoui_theme.c
```

Expected:

- 明确当前缺口是 `background image/mask`、`PaddingGroup` 语义、readback

- [ ] **Step 2: 对 `window` 相关符号跑 GitNexus impact**

Run:

```text
impact target: picoui_window_create
impact target: picoui_backend_create_window
impact target: ldWindowSetImage
```

Expected:

- 记录直接调用方、相关流程、风险级别

- [ ] **Step 3: 先补单测 / 合同 RED**

要求：

- 为 `background image/mask`
- `PaddingGroup` 或等价高层合同
- `window` 最小 readback
- 各自补明确失败测试

- [ ] **Step 4: 实现最小通过代码**

要求：

- 真实落到 `ldWindow`
- 不引入 demo 侧硬编码补丁
- 不把 wrapper shadow state 冒充 backend readback

- [ ] **Step 5: 跑 targeted 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target test_picoui_widgets test_picoui_theme
ctest --test-dir build -R "test_picoui_widgets|test_picoui_theme" --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

Expected:

- 相关测试通过
- public API 不泄漏底层符号

- [ ] **Step 6: 独立 review**

要求：

- review 重点检查：
  - `window` image/padding/readback 是否真落到 backend
  - 是否出现 wrapper/backend 状态分裂

### Task 3: J3 `label` 对齐收口

**Files:**
- Modify: `picoui/include/picoui/label.h`
- Modify: `picoui/src/widgets/label.c`
- Modify: `picoui/src/backend/ldgui/backend_label.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Optional Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 复核 `label` 现状与 `ldLabel` 缺口**

Run:

```bash
sed -n '1,220p' src/gui/ldLabel.h
sed -n '1,220p' picoui/include/picoui/label.h
sed -n '1,220p' picoui/src/widgets/label.c
sed -n '1,220p' picoui/src/backend/ldgui/backend_label.c
rg -n "label" tests/picoui/unit/test_picoui_widgets.c tests/picoui/unit/test_picoui_theme.c
```

Expected:

- 明确当前缺口是 `transparent`、`align`、`background image/mask`、readback

- [ ] **Step 2: 对 `label` 相关符号跑 GitNexus impact**

Run:

```text
impact target: picoui_label_set_text
impact target: picoui_label_set_font
impact target: picoui_backend_create_label
```

Expected:

- 记录 blast radius

- [ ] **Step 3: 先补单测 / 合同 RED**

要求：

- 为 `transparent`
- `align`
- `background image/mask`
- `text/text_color/align/bg_color/font/transparent` readback
- 各自补失败测试

- [ ] **Step 4: 实现最小通过代码**

要求：

- 真正走 `ldLabelSetTransparent / SetAlign / SetBackgroundImage`
- readback 不能只读 PicoUI shadow state，需和 backend 语义一致

- [ ] **Step 5: 跑 targeted 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target test_picoui_widgets test_picoui_theme
ctest --test-dir build -R "test_picoui_widgets|test_picoui_theme" --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

Expected:

- 相关测试通过

- [ ] **Step 6: 独立 review**

要求：

- review 重点检查：
  - label readback 是否真实
  - 背景图与 transparent 是否只停留在 shadow state

### Task 4: J4 `button` 对齐收口

**Files:**
- Modify: `picoui/include/picoui/button.h`
- Modify: `picoui/src/widgets/button.c`
- Modify: `picoui/src/backend/ldgui/backend_button.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_event.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Optional Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 复核 `button` 现状与 `ldButton` 缺口**

Run:

```bash
sed -n '1,220p' src/gui/ldButton.h
sed -n '1,220p' picoui/include/picoui/button.h
sed -n '1,240p' picoui/src/widgets/button.c
sed -n '1,240p' picoui/src/backend/ldgui/backend_button.c
rg -n "button" tests/picoui/unit/test_picoui_widgets.c tests/picoui/unit/test_picoui_theme.c
```

Expected:

- 明确缺口是 image skin、transparent、font、checkable、key_value、pressed read/write

- [ ] **Step 2: 对 `button` 相关符号跑 GitNexus impact**

Run:

```text
impact target: picoui_button_set_on_clicked
impact target: picoui_button_set_text
impact target: picoui_backend_create_button
```

Expected:

- 记录 blast radius

- [ ] **Step 3: 先补单测 / 合同 RED**

要求：

- 为 release/press image skin
- `transparent`
- `font`
- `checkable`
- `key_value`
- pressed state getter/setter/native bridge
- 分别补失败测试

- [ ] **Step 4: 实现最小通过代码**

要求：

- 真实落到 `ldButton`
- pressed state 与事件路径一致
- 不牺牲现有 clicked/pressed/released 回调语义

- [ ] **Step 5: 跑 targeted 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target test_picoui_widgets test_picoui_theme
ctest --test-dir build -R "test_picoui_widgets|test_picoui_theme" --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

Expected:

- 相关测试通过

- [ ] **Step 6: 独立 review**

要求：

- review 重点检查：
  - pressed state 是否真实
  - checkable/key_value 是否只是 wrapper 存储
  - 双态 skin 是否真的进 backend

### Task 5: J5 `slider` 对齐收口

**Files:**
- Modify: `picoui/include/picoui/slider.h`
- Modify: `picoui/src/widgets/slider.c`
- Modify: `picoui/src/backend/ldgui/backend_slider.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_event.c`
- Optional Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Optional Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Optional Modify: `tests/picoui/unit/test_picoui_theme.c`

- [ ] **Step 1: 复核 `slider` 现状与 `ldSlider` 缺口**

Run:

```bash
sed -n '1,220p' src/gui/ldSlider.h
sed -n '1,220p' picoui/include/picoui/slider.h
sed -n '1,240p' picoui/src/widgets/slider.c
sed -n '1,220p' picoui/src/backend/ldgui/backend_slider.c
rg -n "slider" tests/picoui/unit/test_picoui_widgets.c tests/picoui/unit/test_picoui_theme.c
```

Expected:

- 明确缺口是 `horizontal`、image skin、indicator width、slim size

- [ ] **Step 2: 对 `slider` 相关符号跑 GitNexus impact**

Run:

```text
impact target: picoui_slider_set_value
impact target: picoui_slider_set_range
impact target: picoui_backend_create_slider
```

Expected:

- 记录 blast radius

- [ ] **Step 3: 先补单测 / 合同 RED**

要求：

- 为 `horizontal`
- background/indicator image+mask
- `indicator width`
- `slim size`
- `value/range/percent/orientation` 一致性
- 分别补失败测试

- [ ] **Step 4: 实现最小通过代码**

要求：

- 真正走 `ldSlider`
- 不破坏当前 `min/max/value/percent` 归一化合同

- [ ] **Step 5: 跑 targeted 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target test_picoui_widgets test_picoui_theme
ctest --test-dir build -R "test_picoui_widgets|test_picoui_theme" --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

Expected:

- 相关测试通过

- [ ] **Step 6: 独立 review**

要求：

- review 重点检查：
  - orientation / value/range/percent 是否仍一致
  - 新增 style/image 参数是否真落到 backend

### Task 6: J6 发布合同、清单、release wording 重写

**Files:**
- Modify: `docs/picoui-serial/H-线当前9控件发布合同.md`
- Modify: `docs/picoui-serial/H-线已支持控件清单.md`
- Modify: `docs/picoui-serial/H-线第一版发布说明.md`
- Modify: `docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md`
- Optional Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 复核 4 个已对齐控件与 5 个 backlog 控件口径**

Run:

```bash
sed -n '1,420p' docs/picoui-serial/H-线当前9控件发布合同.md
sed -n '1,220p' docs/picoui-serial/H-线已支持控件清单.md
sed -n '1,220p' docs/picoui-serial/H-线第一版发布说明.md
rg -n "v0.1|v0.2|已对齐|wrapped" docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md
```

Expected:

- 当前文案仍以 9 控件同一 first-release wording 为主

- [ ] **Step 2: 重写合同与清单**

要求：

- `window / label / button / slider` 写成 `v0.1 parity-complete`
- `checkbox / switch / text / image / list` 写成 `wrapped but not parity-complete`
- 不再把 9 控件放进同一 release wording

- [ ] **Step 3: 文档自检**

Run:

```bash
git diff --check -- docs/picoui-serial/H-线当前9控件发布合同.md \
                  docs/picoui-serial/H-线已支持控件清单.md \
                  docs/picoui-serial/H-线第一版发布说明.md \
                  docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md
rg -n "当前 9 个控件都已完成对齐|全部完全对齐" docs/picoui-serial/H-线当前9控件发布合同.md docs/picoui-serial/H-线第一版发布说明.md
```

Expected:

- `git diff --check` 无输出
- 不再存在把 `9` 个控件整体写成已对齐的表述

- [ ] **Step 4: 独立 review**

要求：

- review 重点检查：
  - `v0.1` 与 `v0.2` 是否仍有混写
  - backlog 5 控件是否仍被暗示成已对齐

### Task 7: J7 `v0.1` 证据层补齐与 closeout

**Files:**
- Modify: `docs/picoui-serial/H-线发布测试矩阵.md`
- Modify: `docs/picoui-serial/C-线人工窗口验收记录.md`
- Optional Create: `docs/picoui-serial/J-线v0.1-closeout前状态.md`
- Optional Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Optional Modify: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`

- [ ] **Step 1: 复核当前 `visible/manual artifact` 覆盖范围**

Run:

```bash
sed -n '1,240p' docs/picoui-serial/H-线发布测试矩阵.md
sed -n '1,260p' docs/picoui-serial/C-线人工窗口验收记录.md
sed -n '1,240p' docs/picoui-serial/H-线发布closeout前状态.md
```

Expected:

- 当前证据仍主要服务 `H线` 发布准备，不是 `J线` 的 4 控件对齐结论

- [ ] **Step 2: 补 `v0.1` 证据矩阵**

要求：

- 为 `window / label / button / slider` 明确：
  - 哪个 unit test 证明
  - 哪个 contract gate 证明
  - 哪个 mapping/visible demo 证明
  - 哪个 manual artifact 条目承担人工结论

- [ ] **Step 3: 跑 closeout 前验证**

Run:

```bash
ctest --test-dir build --output-on-failure -L picoui
ctest --test-dir build --output-on-failure -L visible
ctest --test-dir build --output-on-failure -L mapping
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
git diff --check
```

Expected:

- 自动门禁 fresh 通过

- [ ] **Step 4: 填人工观察结论**

要求：

- 不再只写 artifact existence
- 必须写出最终人工观察结果与已知限制

- [ ] **Step 5: detect_changes 与独立 review**

Run:

```text
gitnexus_detect_changes(scope="all")
```

Expected:

- 影响面仍集中在 `window / label / button / slider` 与 J 线文档/验证
- review subagent 确认当前可以写成 `v0.1 closeout ready`
