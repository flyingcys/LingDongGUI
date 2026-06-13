# TINYUI a-02 低耦合新控件扩面 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-02` 中串行实现 `progress_bar / qrcode / progress_wheel / message_box / date_time / clock` 六个低耦合新控件的最小完整 vertical slice，同时不争抢 `a-01` 的 shared-owner 写面。

**Architecture:** `a-02` 按控件逐个推进，每个控件必须独立完成 public API、widget、backend、demo、unit/contract/runtime/mapping/visible 接入，然后再进入下一个控件。共享层文件不是长期写面，只允许在每个控件阶段末尾做一次最小聚合接入；任何 shared 语义缺口优先回流 `a-01`，而不是在 `a-02` 扩散成第二条 shared 主线。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、TINYUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- worktree 固定：`.worktree/a-02`
- 建议分支：`feat/tinyui-a-02-new-widgets`
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

- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_style_apply.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tinyui/src/backend/ldgui/backend_app.c`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

若某个控件在阶段末尾必须接入 shared 层：

1. 先把该控件私有文件全部收口
2. rebase 到 `a-01` 最近 checkpoint
3. 只做一次最小 shared 接入
4. 接入后立刻跑 targeted 验证，不继续在 shared 文件上扩写

### a-02 聚合文件

以下文件允许在阶段末尾做一次最小接入：

- `tinyui/include/tinyui/tinyui.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_public_api.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/docs/demo_guide.md`
- `docs/tinyui-serial/a-02-线计划索引.md`

## 1. 文件结构与阶段边界

### B1 `progress_bar`

**Create:**
- `tinyui/include/tinyui/progress_bar.h`
- `tinyui/src/widgets/progress_bar.c`
- `tinyui/src/backend/ldgui/backend_progress_bar.c`
- `tinyui/demo/progress_bar_basic/main.c`
- `tests/tinyui/unit/test_tinyui_progress_bar.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

### B2 `qrcode`

**Create:**
- `tinyui/include/tinyui/qrcode.h`
- `tinyui/src/widgets/qrcode.c`
- `tinyui/src/backend/ldgui/backend_qrcode.c`
- `tinyui/demo/qrcode_basic/main.c`
- `tests/tinyui/unit/test_tinyui_qrcode.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

### B3 `progress_wheel`

**Create:**
- `tinyui/include/tinyui/progress_wheel.h`
- `tinyui/src/widgets/progress_wheel.c`
- `tinyui/src/backend/ldgui/backend_progress_wheel.c`
- `tinyui/demo/progress_wheel_basic/main.c`
- `tests/tinyui/unit/test_tinyui_progress_wheel.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

### B4 `message_box`

**Create:**
- `tinyui/include/tinyui/message_box.h`
- `tinyui/src/widgets/message_box.c`
- `tinyui/src/backend/ldgui/backend_message_box.c`
- `tinyui/demo/message_box_basic/main.c`
- `tests/tinyui/unit/test_tinyui_message_box.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

### B5 `date_time`

**Create:**
- `tinyui/include/tinyui/date_time.h`
- `tinyui/src/widgets/date_time.c`
- `tinyui/src/backend/ldgui/backend_date_time.c`
- `tinyui/demo/date_time_basic/main.c`
- `tests/tinyui/unit/test_tinyui_date_time.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target="Function:src/gui/ldDateTime.c:ldDateTimeSetFormat", direction="upstream", repo="LingDongGUI")`：`LOW`
- `gitnexus_impact(target="Function:src/gui/ldDateTime.c:ldDateTimeSetDate", direction="upstream", repo="LingDongGUI")`：`LOW`
- `gitnexus_impact(target="Function:src/gui/ldDateTime.c:ldDateTimeSetTime", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_date_time --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo date_time_basic` 通过
- `git diff --check` 通过

本轮额外确认点：

- `ldDateTime_on_frame_start()` 已尊重 `isAutoSysTime`，手动设置值不会再在 runtime 中被覆盖回系统时间。
- unit 已补成真实回归用例，会在设置 `2026-05-31 12:34:56` 后直接调用 `ldDateTime_on_frame_start()` 验证底层 `formatStrTemp` 保持稳定。
- `visible_ui` 对 `date_time_basic` 已切换为真实输出契约：验证单行日期时间文本可见，不再误要求额外标题层或大面积多色区域。

### B6 `clock`

**Create:**
- `tinyui/include/tinyui/clock.h`
- `tinyui/src/widgets/clock.c`
- `tinyui/src/backend/ldgui/backend_clock.c`
- `tinyui/demo/clock_basic/main.c`
- `tests/tinyui/unit/test_tinyui_clock.c`

**Modify:**
- `tinyui/include/tinyui/tinyui.h`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target="Function:src/gui/ldClock.c:ldClockSetBackgroundImage", direction="upstream", repo="LingDongGUI")`：`LOW`
- `gitnexus_impact(target="Function:src/gui/ldClock.c:ldClockSetStepSecond", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_clock --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo clock_basic` 通过
- `git diff --check` 通过

本轮额外确认点：

- `clock_basic` 当前真实输出契约是不带背景表盘的三针时钟；`visible_ui` 已按中心枢纽和三向指针重写，不再错误强求圆盘背景。
- 当前 TINYUI `clock` 只承诺最小 `step_second` 显示合同，不暴露背景资源或更复杂时钟配置。

### B7 文档与 closeout

**Modify:**
- `tinyui/docs/demo_guide.md`
- `docs/tinyui-serial/a-02-线计划索引.md`
- `docs/superpowers/specs/2026-05-31-tinyui-a-02-new-widgets-design.md`

## 2. Tasks

### Task B0: worktree 准备和 baseline

**Files:**
- Read: `docs/tinyui-serial/a-02-线计划索引.md`
- Read: `docs/superpowers/specs/2026-05-31-tinyui-a-02-new-widgets-design.md`

- [ ] **Step 1: 创建 worktree**

```bash
git worktree add .worktree/a-02 -b feat/tinyui-a-02-new-widgets HEAD
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
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
git diff --check
```

Expected:

- baseline 全绿
- 若 baseline 红，停止并汇报 `B0 blocker`

### Task B1: `progress_bar` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/progress_bar.h`
- Create: `tinyui/src/widgets/progress_bar.c`
- Create: `tinyui/src/backend/ldgui/backend_progress_bar.c`
- Create: `tinyui/demo/progress_bar_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_progress_bar.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target="ldProgressBarSetPercent", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_progress_bar --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo progress_bar_basic` 通过
- `git diff --check` 通过
- fresh 独立 review：无 findings，`B1` 可收口

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldProgressBarSetPercent", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="test_tinyui_widgets", direction="upstream", repo="LingDongGUI")
```

Expected:

- 记录 blast radius
- 若出现 `HIGH/CRITICAL`，先汇报再继续

结果：

- [x] 已完成，`ldProgressBarSetPercent` 上游影响 `LOW`

- [ ] **Step 2: 写 public header 和 failing unit**

Create `tinyui/include/tinyui/progress_bar.h` and `tests/tinyui/unit/test_tinyui_progress_bar.c`.

Header 至少定义：

```c
struct tinyui_progress_bar;

struct tinyui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
};

struct tinyui_progress_bar *tinyui_progress_bar_create(struct tinyui_widget *parent, const char *id);
struct tinyui_progress_bar *tinyui_progress_bar_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_progress_bar_props *props);
int tinyui_progress_bar_set_percent(struct tinyui_progress_bar *bar, int percent);
int tinyui_progress_bar_get_percent(const struct tinyui_progress_bar *bar);
int tinyui_progress_bar_set_horizontal(struct tinyui_progress_bar *bar, int horizontal);
int tinyui_progress_bar_get_horizontal(const struct tinyui_progress_bar *bar);
```

Test 至少覆盖：

```c
static void test_progress_bar_create_and_props(void);
static void test_progress_bar_percent_bounds(void);
static void test_progress_bar_horizontal_state(void);
```

结果：

- [x] 已完成，header 与 unit 已落地

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_progress_bar --output-on-failure
```

Expected:

- `test_tinyui_progress_bar` 当前因实现缺失失败

结果：

- [x] 已完成，后续已进入 GREEN 验证并通过

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldProgressBar`
- percent 取值限定 `0..100`
- horizontal 只处理最小布尔合同
- 不引入复杂动画或 frame 语义

Private files:

```c
tinyui/src/widgets/progress_bar.c
tinyui/src/backend/ldgui/backend_progress_bar.c
```

结果：

- [x] 已完成，backend 真实落到 `ldProgressBar`

- [ ] **Step 5: 写 demo 并接入 runtime**

Create `tinyui/demo/progress_bar_basic/main.c`.

Demo 只允许 `tinyui_*` API，至少创建：

```c
progress_bar id = "progress_bar"
percent = 65
horizontal = 1
```

Update:

- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`

结果：

- [x] 已完成，demo 使用纯 `tinyui_*` API，runtime/mapping/visible/contract 已接入

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_tinyui_progress_bar --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo progress_bar_basic
git diff --check
```

Expected:

- 全部通过

结果：

- [x] 已完成，命令链全部 fresh 通过

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

- 影响面集中在 `progress_bar` 相关新文件和测试入口

结果：

- [x] 已执行，但当前 `gitnexus_detect_changes(scope=\"all\")` 返回与真实 diff 不一致，不作为 `B1` 收口证据

### Task B2: `qrcode` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/qrcode.h`
- Create: `tinyui/src/widgets/qrcode.c`
- Create: `tinyui/src/backend/ldgui/backend_qrcode.c`
- Create: `tinyui/demo/qrcode_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_qrcode.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target_uid="Function:src/gui/ldQRCode.c:ldQRCodeSetText", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_qrcode --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo qrcode_basic` 通过
- `git diff --check` 通过
- fresh 独立 review：无 findings，`B2` 可收口

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldQRCodeSetText", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="check_tinyui_public_api", direction="upstream", repo="LingDongGUI")
```

结果：

- [x] 已完成，`ldQRCodeSetText` 上游影响 `LOW`
- [x] 计划里的 `check_tinyui_public_api` 不是 GitNexus 可解析符号，未作为有效 impact 证据使用

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct tinyui_qrcode;

struct tinyui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
};

struct tinyui_qrcode *tinyui_qrcode_create(struct tinyui_widget *parent, const char *id);
struct tinyui_qrcode *tinyui_qrcode_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_qrcode_props *props);
int tinyui_qrcode_set_text(struct tinyui_qrcode *qrcode, const char *text);
const char *tinyui_qrcode_get_text(const struct tinyui_qrcode *qrcode);
```

Test 至少覆盖 create / text set-get / NULL text reject。

结果：

- [x] 已完成，header 与 unit 已落地

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_qrcode --output-on-failure
```

结果：

- [x] 已完成，先以链接期 `undefined symbols` 验证实现缺失，再进入 GREEN

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldQRCode`
- 只承诺文本内容合同
- 不把资源/颜色/高级配置系统扩张进来

结果：

- [x] 已完成，backend 真实落到 `ldQRCode` / `ldQRCodeSetText`

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
qrcode id = "qrcode"
text = "https://example.local/tinyui"
```

结果：

- [x] 已完成，demo 使用纯 `tinyui_*` API，runtime/mapping/visible/contract 已接入

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_tinyui_qrcode --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo qrcode_basic
git diff --check
```

结果：

- [x] 已完成，命令链全部 fresh 通过

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

结果：

- [x] 已执行，但当前 `gitnexus_detect_changes(scope=\"all\")` 返回与真实 diff 不一致，不作为 `B2` 收口证据

### Task B3: `progress_wheel` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/progress_wheel.h`
- Create: `tinyui/src/widgets/progress_wheel.c`
- Create: `tinyui/src/backend/ldgui/backend_progress_wheel.c`
- Create: `tinyui/demo/progress_wheel_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_progress_wheel.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target="ldProgressWheelSetProgress", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_progress_wheel --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo progress_wheel_basic` 通过
- `git diff --check` 通过
- fresh 独立 review：无 findings，`B3` 可收口

本阶段额外记录：

- release 崩溃根因来自 `src/gui/ldProgressWheel.c` 中 `progress_wheel_init()` 误传 scene 指针；已改为 `&ptScene->use_as__arm_2d_scene_t`
- `tests/tinyui/runtime/check_tinyui_visible_ui.py` 已从单纯 ring 像素检测补强为 `colored ring + adjacent white dot` 联合判定

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldProgressWheelSetProgress", direction="upstream", repo="LingDongGUI")
```

结果：

- [x] 已完成，`ldProgressWheelSetProgress` 上游影响 `LOW`

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct tinyui_progress_wheel;

struct tinyui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
};

struct tinyui_progress_wheel *tinyui_progress_wheel_create(struct tinyui_widget *parent, const char *id);
struct tinyui_progress_wheel *tinyui_progress_wheel_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_progress_wheel_props *props);
int tinyui_progress_wheel_set_percent(struct tinyui_progress_wheel *wheel, int percent);
int tinyui_progress_wheel_get_percent(const struct tinyui_progress_wheel *wheel);
```

Test 至少覆盖 create / percent bounds / getter。

结果：

- [x] 已完成，header 与 unit 已落地

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_progress_wheel --output-on-failure
```

Expected:

- `test_tinyui_progress_wheel` 当前因实现缺失失败

结果：

- [x] 已完成，RED 后已进入实现并收口

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldProgressWheel`
- 只承诺最小 percent 合同
- 不引入复杂动画/仪表体系

结果：

- [x] 已完成，并补齐了正式构建所需的 Arm-2D define / asset 接线
- [x] 已修正 `ldProgressWheel_init()` 的 scene 参数传递，消除 release-only crash

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
progress_wheel id = "progress_wheel"
percent = 72
```

结果：

- [x] 已完成，`progress_wheel_basic` demo 已接入 runtime/mapping/visible

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_tinyui_progress_wheel --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo progress_wheel_basic
git diff --check
```

结果：

- [x] 已完成，fresh 验证全绿

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

结果：

- [x] 已执行，但当前 `gitnexus_detect_changes(scope="all")` 返回与真实 diff 不一致，不作为 `B3` 收口证据

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B4: `message_box` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/message_box.h`
- Create: `tinyui/src/widgets/message_box.c`
- Create: `tinyui/src/backend/ldgui/backend_message_box.c`
- Create: `tinyui/demo/message_box_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_message_box.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

**当前状态：已完成并收口**

当前已确认的收口证据：

- `gitnexus_impact(target="Function:src/gui/ldMessageBox.c:ldMessageBoxSetTitle", direction="upstream", repo="LingDongGUI")`：`LOW`
- `gitnexus_impact(target="Function:src/gui/ldMessageBox.c:ldMessageBoxSetCallback", direction="upstream", repo="LingDongGUI")`：`LOW`
- `ctest --test-dir build -R test_tinyui_message_box --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo message_box_basic` 通过
- `git diff --check` 通过
- fresh 独立 review：无 findings，`B4` 可收口

本阶段额外记录：

- `tinyui_message_box_set_on_confirm()` 当前已通过 backend 真正桥接到底层 `ldMessageBoxSetCallback()`
- unit 已补成真实 callback 触发验证，不再只是字段存储验证
- `visible_ui` 对 `message_box_basic` 当前只承诺结构可见，不夸写成完整 modal/focus 交互证明

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldMessageBoxSetTitle", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldMessageBoxSetCallback", direction="upstream", repo="LingDongGUI")
```

结果：

- [x] 已完成，两者上游影响均为 `LOW`

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct tinyui_message_box;

typedef void (*tinyui_message_box_callback_t)(struct tinyui_message_box *box, void *user_data);

struct tinyui_message_box_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
};

struct tinyui_message_box *tinyui_message_box_create(struct tinyui_widget *parent, const char *id);
struct tinyui_message_box *tinyui_message_box_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_message_box_props *props);
int tinyui_message_box_set_title(struct tinyui_message_box *box, const char *title);
int tinyui_message_box_set_message(struct tinyui_message_box *box, const char *message);
int tinyui_message_box_set_confirm_text(struct tinyui_message_box *box, const char *text);
void tinyui_message_box_set_on_confirm(
    struct tinyui_message_box *box,
    tinyui_message_box_callback_t callback,
    void *user_data);
```

Test 至少覆盖 create / title-message-confirm_text state / callback storage。

结果：

- [x] 已完成，header 与 unit 已落地

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_message_box --output-on-failure
```

Expected:

- `test_tinyui_message_box` 当前因实现缺失失败

结果：

- [x] 已完成，RED 后已进入实现并收口

- [ ] **Step 4: 实现 widget 和 backend**

Requirements:

- 使用真实 `ldMessageBox`
- 当前只承诺最小单按钮/confirm 合同
- 不引入 modal/focus 管理系统

结果：

- [x] 已完成，backend 已真实落到 `ldMessageBox`
- [x] 已补上 `confirm` callback 的真实桥接，不再只存字段

- [ ] **Step 5: 写 demo 并接入 runtime**

Demo 最少创建：

```c
message_box id = "message_box"
title = "Update"
message = "Apply settings?"
confirm_text = "OK"
```

结果：

- [x] 已完成，`message_box_basic` 已接入 runtime/mapping/visible

- [ ] **Step 6: 验证 GREEN**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R test_tinyui_message_box --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo message_box_basic
git diff --check
```

结果：

- [x] 已完成，fresh 验证全绿

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

结果：

- [x] 已执行，但当前 `gitnexus_detect_changes(scope="all")` 返回与真实 diff 不一致，不作为 `B4` 收口证据

### Task B5: `date_time` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/date_time.h`
- Create: `tinyui/src/widgets/date_time.c`
- Create: `tinyui/src/backend/ldgui/backend_date_time.c`
- Create: `tinyui/demo/date_time_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_date_time.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldDateTimeSetFormat", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldDateTimeSetDate", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct tinyui_date_time;

struct tinyui_date_time_props {
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

struct tinyui_date_time *tinyui_date_time_create(struct tinyui_widget *parent, const char *id);
struct tinyui_date_time *tinyui_date_time_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_date_time_props *props);
int tinyui_date_time_set_format(struct tinyui_date_time *dt, const char *format);
int tinyui_date_time_set_date(struct tinyui_date_time *dt, int year, int month, int day);
int tinyui_date_time_set_time(struct tinyui_date_time *dt, int hour, int minute, int second);
```

Test 至少覆盖 create / format state / date-time setter state / invalid range reject。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_date_time --output-on-failure
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
ctest --test-dir build -R test_tinyui_date_time --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo date_time_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B6: `clock` vertical slice

**Files:**
- Create: `tinyui/include/tinyui/clock.h`
- Create: `tinyui/src/widgets/clock.c`
- Create: `tinyui/src/backend/ldgui/backend_clock.c`
- Create: `tinyui/demo/clock_basic/main.c`
- Create: `tests/tinyui/unit/test_tinyui_clock.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tests/tinyui/CMakeLists.txt`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Modify: `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- Modify: `tests/tinyui/contract/check_tinyui_demo_boundary.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="ldClockSetBackgroundImage", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="ldClockSetStepSecond", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 写 public header 和 failing unit**

Header 至少定义：

```c
struct tinyui_clock;

struct tinyui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int step_second;
};

struct tinyui_clock *tinyui_clock_create(struct tinyui_widget *parent, const char *id);
struct tinyui_clock *tinyui_clock_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_clock_props *props);
int tinyui_clock_set_step_second(struct tinyui_clock *clock, int step_second);
int tinyui_clock_get_step_second(const struct tinyui_clock *clock);
```

Test 至少覆盖 create / step_second state / props create。

- [ ] **Step 3: 验证 RED**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_tinyui_clock --output-on-failure
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
ctest --test-dir build -R test_tinyui_clock --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo clock_basic
git diff --check
```

- [ ] **Step 7: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task B7: 文档收口与 closeout review

**当前状态：已完成并收口**

当前已确认的收口证据：

- `ctest --test-dir build -L tinyui --output-on-failure` 通过
- `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过
- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_runtime.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py` 通过
- `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all` 通过
- `git diff --check` 通过
- `tinyui/docs/demo_guide.md` 已同步六个新控件 demo 的证明边界

**Files:**
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `docs/tinyui-serial/a-02-线计划索引.md`
- Modify: `docs/superpowers/specs/2026-05-31-tinyui-a-02-new-widgets-design.md`

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
- demo 是否只用 `tinyui_*`
- 是否误改 shared-owner 文件
- 是否把显示型 demo 夸写成“完整交互能力”
- 当前 6 控件范围是否仍受控

- [ ] **Step 4: 最终验证**

Run:

```bash
git status --short --branch --ignore-submodules=all
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L tinyui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
python3 tests/tinyui/contract/check_tinyui_public_api.py
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
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
