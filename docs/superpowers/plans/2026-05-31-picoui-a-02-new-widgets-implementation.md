# PicoUI a-02 低耦合新控件扩面 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-02` 中串行实现 `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 六个低耦合新控件的最小完整 vertical slice，同时不争抢 `a-01` 的 shared-owner 写面。

**Architecture:** `a-02` 按控件逐个推进，每个控件必须独立完成 public API、widget、backend、demo、unit/contract/runtime/mapping/visible 接入，然后再进入下一个控件。共享层文件不是长期写面，只允许在每个控件阶段末尾做一次最小聚合接入；任何 shared 语义缺口优先回流 `a-01`，而不是在 `a-02` 扩散成第二条 shared 主线。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-02`
- 建议分支：`feat/picoui-a-02-new-widgets`
- 创建或切换后必须执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

- `a-02` 严格串行：`B0 -> B1 -> B2 -> B3 -> B4 -> B5 -> B6 -> B7`
- 每个 Task 使用 fresh subagent 执行
- 每个 Task 做独立只读 review；review 不通过时，回原执行 subagent 修复
- 涉及 C / Python 符号修改前必须运行 GitNexus impact
- 每个 Task 结束前至少运行 `git diff --check`
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### a-02 禁改 shared-owner 文件

以下文件默认禁止 `a-02` 持续修改：

- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_app.c`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`

若某个控件在阶段末尾必须接入 shared 层：

1. 先把该控件私有文件全部收口
2. rebase 到 `a-01` 最近 checkpoint
3. 只做一次最小 shared 接入
4. 接入后立刻跑 targeted 验证，不继续在 shared 文件上扩写

### a-02 聚合文件

以下文件允许在阶段末尾做一次最小接入：

- `picoui/include/picoui/picoui.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_public_api.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`
- `picoui/docs/demo_guide.md`
- `docs/picoui-serial/a-02-线计划索引.md`

## 1. 文件结构与阶段边界

### B1 `progress_bar`

**Create:**
- `picoui/include/picoui/progress_bar.h`
- `picoui/src/widgets/progress_bar.c`
- `picoui/src/backend/ldgui/backend_progress_bar.c`
- `picoui/demo/progress_bar_basic/main.c`
- `tests/picoui/unit/test_picoui_progress_bar.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B2 `qrcode`

**Create:**
- `picoui/include/picoui/qrcode.h`
- `picoui/src/widgets/qrcode.c`
- `picoui/src/backend/ldgui/backend_qrcode.c`
- `picoui/demo/qrcode_basic/main.c`
- `tests/picoui/unit/test_picoui_qrcode.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B3 `progress_wheel`

**Create:**
- `picoui/include/picoui/progress_wheel.h`
- `picoui/src/widgets/progress_wheel.c`
- `picoui/src/backend/ldgui/backend_progress_wheel.c`
- `picoui/demo/progress_wheel_basic/main.c`
- `tests/picoui/unit/test_picoui_progress_wheel.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B4 `message_box`

**Create:**
- `picoui/include/picoui/message_box.h`
- `picoui/src/widgets/message_box.c`
- `picoui/src/backend/ldgui/backend_message_box.c`
- `picoui/demo/message_box_basic/main.c`
- `tests/picoui/unit/test_picoui_message_box.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B5 `date_time`

**Create:**
- `picoui/include/picoui/date_time.h`
- `picoui/src/widgets/date_time.c`
- `picoui/src/backend/ldgui/backend_date_time.c`
- `picoui/demo/date_time_basic/main.c`
- `tests/picoui/unit/test_picoui_date_time.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B6 `clock`

**Create:**
- `picoui/include/picoui/clock.h`
- `picoui/src/widgets/clock.c`
- `picoui/src/backend/ldgui/backend_clock.c`
- `picoui/demo/clock_basic/main.c`
- `tests/picoui/unit/test_picoui_clock.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/src/backend/ldgui/backend.h`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

### B7 文档与 closeout

**Modify:**
- `picoui/docs/demo_guide.md`
- `docs/picoui-serial/a-02-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-picoui-a-02-new-widgets-design.md`

## 2. Tasks

### Task B0: worktree 准备和 baseline

**Files:**
- Read: `docs/picoui-serial/a-02-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-31-picoui-a-02-new-widgets-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/a-02 -b feat/picoui-a-02-new-widgets HEAD
cd .worktree/a-02
git submodule sync --recursive
git submodule update --init --recursive
```

Expected:

- worktree 创建成功
- submodule 状态已同步

- [ ] **Step 2: 跑 baseline**

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
git diff --check
```

Expected:

- baseline 全绿
- 若 baseline 红，停止并汇报 `B0 blocker`

### Task B1: `progress_bar` vertical slice

**Files:**
- Create: `picoui/include/picoui/progress_bar.h`
- Create: `picoui/src/widgets/progress_bar.c`
- Create: `picoui/src/backend/ldgui/backend_progress_bar.c`
- Create: `picoui/demo/progress_bar_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_progress_bar.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldProgressBarSetPercent", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="test_picoui_widgets", direction="upstream", repo="LingDongGUI")
```

Expected:

- 记录 blast radius
- 若出现 `HIGH/CRITICAL`，先汇报再继续

- [ ] **Step 2: 写 public header 和 failing unit**

Create `picoui/include/picoui/progress_bar.h` and `tests/picoui/unit/test_picoui_progress_bar.c`.

Header 至少定义：

```c
struct picoui_progress_bar;

struct picoui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
};

struct picoui_progress_bar *picoui_progress_bar_create(struct picoui_widget *parent, const char *id);
struct picoui_progress_bar *picoui_progress_bar_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_bar_props *props);
int picoui_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);
int picoui_progress_bar_get_percent(const struct picoui_progress_bar *bar);
int picoui_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);
int picoui_progress_bar_get_horizontal(const struct picoui_progress_bar *bar);
```

Test 至少覆盖：

```c
static void test_progress_bar_create_and_props(void);
static void test_progress_bar_percent_bounds(void);
static void test_progress_bar_horizontal_state(void);
```

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_progress_bar --output-on-failure
```

Expected:

- `test_picoui_progress_bar` 当前因实现缺失失败

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldProgressBar`
- percent 取值限定 `0..100`
- horizontal 只处理最小布尔合同
- 不引入复杂动画或 frame 语义

Private files:

```c
picoui/src/widgets/progress_bar.c
picoui/src/backend/ldgui/backend_progress_bar.c
```

- [ ] **Step 5: 写 demo 并接入 runtime**

Create `picoui/demo/progress_bar_basic/main.c`.

Demo 只允许 `picoui_*` API，至少创建：

```c
progress_bar id = "progress_bar"
percent = 65
horizontal = 1
```

Update:

- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_progress_bar --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo progress_bar_basic
git diff --check
```

Expected:

- 全部通过

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

- 影响面集中在 `progress_bar` 相关新文件和测试入口

### Task B2: `qrcode` vertical slice

**Files:**
- Create: `picoui/include/picoui/qrcode.h`
- Create: `picoui/src/widgets/qrcode.c`
- Create: `picoui/src/backend/ldgui/backend_qrcode.c`
- Create: `picoui/demo/qrcode_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_qrcode.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldQRCodeSetText", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="check_picoui_public_api", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct picoui_qrcode;

struct picoui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
};

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id);
struct picoui_qrcode *picoui_qrcode_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_qrcode_props *props);
int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode);
```

Test 至少覆盖 create / text set-get / NULL text reject。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_qrcode --output-on-failure
```

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldQRCode`
- 只承诺文本内容合同
- 不把资源/颜色/高级配置系统扩张进来

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
qrcode id = "qrcode"
text = "https://example.local/picoui"
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_qrcode --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo qrcode_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B3: `progress_wheel` vertical slice

**Files:**
- Create: `picoui/include/picoui/progress_wheel.h`
- Create: `picoui/src/widgets/progress_wheel.c`
- Create: `picoui/src/backend/ldgui/backend_progress_wheel.c`
- Create: `picoui/demo/progress_wheel_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_progress_wheel.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldProgressWheelSetPercent", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="check_picoui_backend_mapping", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct picoui_progress_wheel;

struct picoui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
};

struct picoui_progress_wheel *picoui_progress_wheel_create(struct picoui_widget *parent, const char *id);
struct picoui_progress_wheel *picoui_progress_wheel_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_wheel_props *props);
int picoui_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);
int picoui_progress_wheel_get_percent(const struct picoui_progress_wheel *wheel);
```

Test 至少覆盖 create / percent bounds / getter。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_progress_wheel --output-on-failure
```

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldProgressWheel`
- 只承诺最小 percent 合同
- 不引入复杂动画/仪表体系

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
progress_wheel id = "progress_wheel"
percent = 72
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_progress_wheel --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo progress_wheel_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B4: `message_box` vertical slice

**Files:**
- Create: `picoui/include/picoui/message_box.h`
- Create: `picoui/src/widgets/message_box.c`
- Create: `picoui/src/backend/ldgui/backend_message_box.c`
- Create: `picoui/demo/message_box_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_message_box.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldMessageBoxSetTitle", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldMessageBoxSetCallback", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct picoui_message_box;

typedef void (*picoui_message_box_callback_t)(struct picoui_message_box *box, void *user_data);

struct picoui_message_box_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
};

struct picoui_message_box *picoui_message_box_create(struct picoui_widget *parent, const char *id);
struct picoui_message_box *picoui_message_box_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_message_box_props *props);
int picoui_message_box_set_title(struct picoui_message_box *box, const char *title);
int picoui_message_box_set_message(struct picoui_message_box *box, const char *message);
int picoui_message_box_set_confirm_text(struct picoui_message_box *box, const char *text);
void picoui_message_box_set_on_confirm(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data);
```

Test 至少覆盖 create / title-message-confirm_text state / callback storage。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_message_box --output-on-failure
```

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldMessageBox`
- 当前只承诺最小单按钮/confirm 合同
- 不引入 modal/focus 管理系统

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
message_box id = "message_box"
title = "Update"
message = "Apply settings?"
confirm_text = "OK"
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_message_box --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo message_box_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B5: `date_time` vertical slice

**Files:**
- Create: `picoui/include/picoui/date_time.h`
- Create: `picoui/src/widgets/date_time.c`
- Create: `picoui/src/backend/ldgui/backend_date_time.c`
- Create: `picoui/demo/date_time_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_date_time.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldDateTimeSetFormat", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldDateTimeSetDate", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct picoui_date_time;

struct picoui_date_time_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *format;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

struct picoui_date_time *picoui_date_time_create(struct picoui_widget *parent, const char *id);
struct picoui_date_time *picoui_date_time_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_date_time_props *props);
int picoui_date_time_set_format(struct picoui_date_time *dt, const char *format);
int picoui_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day);
int picoui_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second);
```

Test 至少覆盖 create / format state / date-time setter state / invalid range reject。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_date_time --output-on-failure
```

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldDateTime`
- 只承诺显示合同
- 不引入 date editing / calendar 体系

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
date_time id = "date_time"
format = "YYYY-MM-DD hh:mm:ss"
date = 2026-05-31
time = 12:34:56
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_date_time --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo date_time_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B6: `clock` vertical slice

**Files:**
- Create: `picoui/include/picoui/clock.h`
- Create: `picoui/src/widgets/clock.c`
- Create: `picoui/src/backend/ldgui/backend_clock.c`
- Create: `picoui/demo/clock_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_clock.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/contract/check_picoui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldClockSetBackgroundImage", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldClockSetStepSecond", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct picoui_clock;

struct picoui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int step_second;
};

struct picoui_clock *picoui_clock_create(struct picoui_widget *parent, const char *id);
struct picoui_clock *picoui_clock_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_clock_props *props);
int picoui_clock_set_step_second(struct picoui_clock *clock, int step_second);
int picoui_clock_get_step_second(const struct picoui_clock *clock);
```

Test 至少覆盖 create / step_second state / props create。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_clock --output-on-failure
```

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldClock`
- 只承诺最小显示合同
- 不引入复杂动画、时区、定时系统

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
clock id = "clock"
step_second = 1
```

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_picoui_clock --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo clock_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B7: 文档收口与 closeout review

**Files:**
- Modify: `picoui/docs/demo_guide.md`
- Modify: `docs/picoui-serial/a-02-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-31-picoui-a-02-new-widgets-design.md`

- [ ] **Step 1: 更新 demo guide**

Write:

- `progress_bar_basic`
- `qrcode_basic`
- `progress_wheel_basic`
- `message_box_basic`
- `date_time_basic`
- `clock_basic`

每个 demo 都必须写清：

- 证明什么
- 不证明什么
- 对应哪个 runtime / mapping / visible gate

- [ ] **Step 2: 更新 a-02 索引**

记录：

- `B0-B7` 状态
- 当前已完成控件
- 当前明确不做控件
- 每阶段验证命令
- `15 / 26 = 57.7%` 的数量覆盖口径

- [ ] **Step 3: 独立 review**

Review subagent 只读检查：

- public header 是否泄漏底层标识
- demo 是否只用 `picoui_*`
- 是否误改 shared-owner 文件
- 是否把显示型 demo 夸写成“完整交互能力”
- 当前 6 控件范围是否仍受控

- [ ] **Step 4: 最终验证**

Run:

```bash
git status --short --branch --ignore-submodules=all
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
git diff --check
```

Expected:

- 全部通过

- [ ] **Step 5: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

- 影响面集中在 `a-02` 六个控件及其测试/文档/聚合入口
