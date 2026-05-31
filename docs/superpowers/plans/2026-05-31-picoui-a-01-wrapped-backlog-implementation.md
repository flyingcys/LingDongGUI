# PicoUI a-01 已 Wrapped Backlog 收口 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-01` 中串行收口 `image / text / checkbox / switch / list` 五个已经 `wrapped` 的 backlog 控件，使其 public contract、真实 backend 语义、shared widget/style/event/app 口径和测试证据更稳定、更诚实。

**Architecture:** `a-01` 是 shared-owner 线。开发顺序固定为 `image -> text -> checkbox -> switch -> list`，先冻结最容易误扩张的 `image` 语义，再补 `text` 显示合同，然后连续处理 `checkbox/switch` 的 shared style/event 语义，最后收口最易扩散的 `list` marker/callback/runtime 口径。shared-owner 文件只允许由 `a-01` 持续修改，聚合文件在每阶段末尾一次性最小接入。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-01`
- 建议分支：`feat/picoui-a-01-wrapped-backlog`
- 创建或切换后必须执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- `a-01` 严格串行：`A0 -> A1 -> A2 -> A3 -> A4 -> A5 -> A6`
- 每个 Task 使用 fresh subagent 执行
- 每个 Task 做独立只读 review；review 不通过时，回原执行 subagent 修复
- 涉及 C / Python 符号修改前必须运行 GitNexus impact
- 每个 Task 结束前至少运行 `git diff --check`
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### a-01 独占 shared-owner 文件

以下文件默认只允许 `a-01` 持续修改：

- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_app.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

### a-01 聚合文件

以下文件只允许在每个控件阶段末尾做一次最小接入：

- `picoui/include/picoui/picoui.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_public_api.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`
- `docs/picoui-serial/a-01-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-picoui-a-01-wrapped-backlog-design.md`

## 1. 文件结构与阶段边界

### A1 `image`

**Modify:**
- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_image.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

### A2 `text`

**Modify:**
- `picoui/include/picoui/text.h`
- `picoui/src/widgets/text.c`
- `picoui/src/backend/ldgui/backend_text.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/unit/test_picoui_theme.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`

### A3 `checkbox`

**Modify:**
- `picoui/include/picoui/checkbox.h`
- `picoui/src/widgets/checkbox.c`
- `picoui/src/backend/ldgui/backend_checkbox.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`

### A4 `switch`

**Modify:**
- `picoui/include/picoui/switch.h`
- `picoui/src/widgets/switch.c`
- `picoui/src/backend/ldgui/backend_switch.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `tests/picoui/unit/test_picoui_widgets.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`

### A5 `list`

**Modify:**
- `picoui/include/picoui/list.h`
- `picoui/src/widgets/list.c`
- `picoui/src/backend/ldgui/backend_list.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_app.c`
- `tests/picoui/unit/test_picoui_list.c`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/contract/picoui_release_capability_matrix.json`

### A6 文档与 closeout

**Modify:**
- `docs/picoui-serial/a-01-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-picoui-a-01-wrapped-backlog-design.md`
- `picoui/docs/demo_guide.md`

## 2. Tasks

### Task A0: worktree 准备和 baseline

**Files:**
- Read: `docs/picoui-serial/a-01-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-31-picoui-a-01-wrapped-backlog-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/a-01 -b feat/picoui-a-01-wrapped-backlog HEAD
cd .worktree/a-01
git submodule sync --recursive
git submodule update --init --recursive
```

- [ ] **Step 2: 跑 baseline**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected:

- baseline 全绿
- 若 baseline 红，停止并汇报 `A0 blocker`

### Task A1: `image` 语义冻结

**Files:**
- Modify: `picoui/src/core/widget.c`
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `picoui/src/backend/ldgui/backend_image.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_widget_set_enabled", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_widget_apply_style", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first image 语义测试**

在 `tests/picoui/unit/test_picoui_widgets.c` 增加最小 RED 用例，覆盖：

```c
static void test_image_style_class_and_user_data_are_metadata_only_contract(void);
static void test_image_theme_style_parts_remain_explicitly_rejected(void);
static void test_image_enabled_remains_rejected_contract(void);
```

断言目标：

- `style_class / user_data` 只证明 widget/backend wrapper metadata 存储
- `theme/bg_color/text_color/border_color/radius` 不得误成 support
- `enabled` 继续 reject，不偷换成 `visible/opacity/selectable`

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected:

- 新增 image 语义测试失败

- [ ] **Step 4: 实现最小收口**

Requirements:

- 仅在测试证明现状不稳定时修改 `picoui/src/core/widget.c`
- 仅在测试证明现状不稳定时修改 `backend_style_apply.c`
- `backend_image.c` 只允许补最小稳定 reject / metadata-only 路径
- 不新增任何 image disabled backend 或 fake style backend

- [ ] **Step 5: 同步 release matrix**

Update `tests/picoui/contract/picoui_release_capability_matrix.json`，至少固定：

- `theme`: `reject`
- `style_class / user_data`: `incomplete_contract`
- `bg_color / text_color / border_color / radius`: `reject`
- `padding`: `deferred`
- `enabled`: `reject`

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task A2: `text` 显示合同与 readback 收口

**Files:**
- Modify: `picoui/include/picoui/text.h`
- Modify: `picoui/src/widgets/text.c`
- Modify: `picoui/src/backend/ldgui/backend_text.c`
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/unit/test_picoui_theme.c`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_text_set_font", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_text_set_text", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first text 测试**

在 `tests/picoui/unit/test_picoui_widgets.c` / `tests/picoui/unit/test_picoui_theme.c` 增加 RED 用例，至少覆盖：

```c
static void test_text_font_contract_round_trip(void);
static void test_text_background_and_text_color_contract(void);
static void test_text_align_and_readback_contract(void);
```

断言目标：

- `font` 不是纯字段缓存
- `text/bg/align/font` 读写语义自洽
- 不直接复用 `label` 结论

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^(test_picoui_widgets|test_picoui_theme)$' --output-on-failure
```

- [ ] **Step 4: 实现最小收口**

Requirements:

- 使用真实 `ldText` 路径
- 只补字体/背景/颜色/readback 最小合同
- 不引入完整资源系统

- [ ] **Step 5: 同步 release matrix**

Update:

- `text` 当前支持面
- 仍未承诺的更重能力保持诚实状态

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^(test_picoui_widgets|test_picoui_theme)$' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task A3: `checkbox` checked/toggled 合同收口

**Files:**
- Modify: `picoui/include/picoui/checkbox.h`
- Modify: `picoui/src/widgets/checkbox.c`
- Modify: `picoui/src/backend/ldgui/backend_checkbox.c`
- Modify: `picoui/src/backend/ldgui/backend_event.c`
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_checkbox_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_widget_dispatch_event", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first checkbox 测试**

至少新增：

```c
static void test_checkbox_checked_state_matches_native_state(void);
static void test_checkbox_toggled_callback_is_not_shadow_only(void);
static void test_checkbox_known_limitations_remain_honest(void);
```

断言目标：

- checked state 和 native state 一致
- callback 不是 shadow state 自转
- `radio group / image mode` 不被误写成 support

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

- [ ] **Step 4: 实现最小收口**

Requirements:

- 只补 checked/toggled/style 的最小合同
- 不引入 radio-group 新线
- `backend_event.c` 只做必要 bridge 修正

- [ ] **Step 5: 同步 release matrix**

Update checkbox 当前支持与限制口径。

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task A4: `switch` checked/disabled/event 合同收口

**Files:**
- Modify: `picoui/include/picoui/switch.h`
- Modify: `picoui/src/widgets/switch.c`
- Modify: `picoui/src/backend/ldgui/backend_switch.c`
- Modify: `picoui/src/backend/ldgui/backend_event.c`
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_switch_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_widget_set_enabled", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first switch 测试**

至少新增：

```c
static void test_switch_checked_state_matches_native_state(void);
static void test_switch_disabled_contract_is_real_not_fake(void);
static void test_switch_toggled_callback_matches_native_transition(void);
```

断言目标：

- checked 和 native state 一致
- disabled 不是伪语义
- callback 与真实切换一致

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

- [ ] **Step 4: 实现最小收口**

Requirements:

- 只收口 checked/disabled/style/event
- 不引入 direction/navigation 高耦合能力
- 和 `checkbox` 的 shared event/style 判断标准保持一致

- [ ] **Step 5: 同步 release matrix**

Update switch 当前支持与限制口径。

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task A5: `list` marker/callback/runtime 口径收口

**Files:**
- Modify: `picoui/include/picoui/list.h`
- Modify: `picoui/src/widgets/list.c`
- Modify: `picoui/src/backend/ldgui/backend_list.c`
- Modify: `picoui/src/backend/ldgui/backend_event.c`
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Modify: `tests/picoui/unit/test_picoui_list.c`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_list_set_on_selected", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_append_widget_ids", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 fail-first list 测试**

在 `tests/picoui/unit/test_picoui_list.c` 和 `tests/picoui/runtime/check_picoui_backend_mapping.py` 增加 RED 断言，至少覆盖：

```c
static void test_list_widget_user_data_is_distinct_from_callback_cookie(void);
static void test_list_selection_contract_is_honest(void);
```

以及 mapping 断言：

- `list` 真实 widget id 和 `item_*` payload marker 不得继续混成同一强语义集合

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R '^test_picoui_list$' --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
```

- [ ] **Step 4: 实现最小收口**

Requirements:

- `on_selected` 若未完全 native bridge，就保持诚实 incomplete/limited wording
- `backend_app.c` 拆清 widget marker 和 payload marker 语义
- `widget-level user_data` 与 callback cookie 分离保持稳定
- 不顺手扩 `padding / align / item child widget`

- [ ] **Step 5: 同步 release matrix**

Update:

- `item marker`
- `style_class`
- `widget-level user_data`
- `on_selected(..., user_data)` 相关当前状态

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_picoui_list$' --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task A6: 文档收口与 closeout review

**Files:**
- Modify: `docs/picoui-serial/a-01-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-01-wrapped-backlog-design.md`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: 更新 a-01 索引**

记录：

- `A0-A6` 状态
- 当前五控件 backlog 收口结论
- 三次 merge-back 节奏：
  - `image + text`
  - `checkbox + switch`
  - `list`

- [ ] **Step 2: 更新 design 结论区**

把真正实施后确认的边界写回 spec：

- `image` 哪些是稳定 reject / incomplete / deferred
- `text` 当前真实支持面
- `checkbox / switch` shared event/style 共识
- `list` marker/callback 口径

- [ ] **Step 3: 更新 demo guide（如需）**

只有当 `list_basic` 或现有 demo 的解释口径受本线影响时才更新，重点写清：

- 哪些证据证明 widget
- 哪些 payload marker 不等于独立 backend widget

- [ ] **Step 4: 独立 review**

Review subagent 只读检查：

- shared-owner 文件是否只由 `a-01` 修改
- `a-01` 是否越界吸入新控件扩张
- 五控件的 known limitations 是否更诚实而不是更模糊
- release matrix 与代码当前态是否一致

- [ ] **Step 5: 最终验证**

Run:

```bash
git status --short --branch --ignore-submodules=all
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
git diff --check
```

Expected:

- 全部通过

- [ ] **Step 6: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

- 影响面集中在 backlog 五控件和相关文档/矩阵/最小验证入口
