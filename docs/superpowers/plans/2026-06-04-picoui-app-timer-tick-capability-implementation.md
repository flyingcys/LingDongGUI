# TINYUI App Timer / Tick Capability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 TINYUI public `app` 层补一组最小但完整的 timer capability，并用它收口 `layout_parity` 的 `1200ms` 宽度切换与 `legacy_widget_parity` 的 `100ms` arc/gauge 动画。

**Architecture:** 在 `tinyui/include/tinyui/app.h` 与 `tinyui/src/core/app.c` 增加 `struct tinyui_app_timer` 与 `create/start/stop/is_running/destroy` public API；在 `tinyui/src/backend/ldgui/backend_app.c` 的现有 SDL 主循环里调度到期 timer，而不引入线程或 backend special-case。随后让两个 parity demo 真消费这套 public timer，而不是继续把行为写死在 backend 或文档里。

**Tech Stack:** C, TINYUI public API, LingDongGUI backend bridge, SDL ticks, CMake, CTest, Python contract checks, GitNexus impact

---

## 文件结构

- Modify: `tinyui/include/tinyui/app.h`
  - 新增 `struct tinyui_app_timer`、回调 typedef、public timer API 声明。
- Modify: `tinyui/src/core/internal.h`
  - 为 `struct tinyui_app` 增加 timer 持有状态；必要时声明内部 timer 结构。
- Modify: `tinyui/src/core/app.c`
  - 实现 public timer API、参数校验、生命周期管理。
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
  - 在 SDL 主循环里扫描和触发 timer。
- Modify: `tests/tinyui/CMakeLists.txt`
  - 把独立 timer unit test 挂进现有 unit test 列表。
- Create: `tests/tinyui/unit/test_tinyui_app_timer.c`
  - 新增 timer API 的 unit/contract 测试。
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
  - 若现有 backend runtime gate 需要 demo-level 断言，补最小 timer 消费痕迹。
- Modify: `tinyui/demo/layout_parity/main.c`
  - 用 public timer 补 `1200ms` 宽度切换。
- Modify: `tinyui/demo/legacy_widget_parity/main.c`
  - 用 public timer 补 `100ms` arc/gauge 动画。
- Modify: `tinyui/docs/demo_guide.md`
  - 同步新的 app timer capability 与两页 parity 状态。
- Modify: `docs/superpowers/reviews/2026-06-04-tinyui-three-parity-demo-gap-audit.md`
  - capability 补通后更新阻塞状态。
- Modify: `docs/superpowers/plans/2026-06-04-tinyui-three-parity-demo-gap-repair.md`
  - 把 Task 1/2 的“阻塞”改成“已拆成独立 timer capability 线”。

## Task 1: 先补 fail-first 测试与 impact 基线

**Files:**
- Modify: `tests/tinyui/CMakeLists.txt`
- Create: `tests/tinyui/unit/test_tinyui_app_timer.c`
- Reference: `tinyui/include/tinyui/app.h`
- Reference: `tinyui/src/core/app.c`
- Reference: `tinyui/src/backend/ldgui/backend_app.c`

- [ ] **Step 1: 对将要修改的函数先跑 GitNexus impact**

对以下 symbol 跑 upstream impact：

```text
tinyui_app_run
tinyui_app_destroy
tinyui_app_set_window
tinyui_backend_app_run
```

要求：

1. 记录 direct callers、affected processes、risk level。
2. 若任一结果为 `HIGH` 或 `CRITICAL`，先停下并回主线程重新评估边界。

- [ ] **Step 2: 新建独立 timer unit test 并挂到 CMake**

在 `tests/tinyui/CMakeLists.txt` 的 `PICOUI_UNIT_TESTS` 里追加：

```cmake
    unit/test_tinyui_app_timer.c
```

并新建 `tests/tinyui/unit/test_tinyui_app_timer.c`。

- [ ] **Step 3: 写 timer API 的失败测试骨架**

在 `tests/tinyui/unit/test_tinyui_app_timer.c` 增加至少这些测试：

```c
static void test_app_timer_create_rejects_null_app(void)
{
    assert(tinyui_app_timer_create(NULL) == NULL);
}

static void test_app_timer_start_rejects_zero_interval_and_null_callback(void)
{
    assert(!"write me");
}

static void test_app_timer_start_stop_running_state_contract(void)
{
    assert(!"write me");
}

static void test_app_timer_destroy_is_safe_after_stop(void)
{
    assert(!"write me");
}
```

- [ ] **Step 4: 补真实断言，先锁 API 合同**

目标合同至少覆盖：

1. `tinyui_app_timer_create(NULL) == NULL`
2. `start(timer, 0, ...) == -1`
3. `start(timer, 100, 1, NULL, ...) == -1`
4. `is_running()` 在 create 后为 `0`
5. `start()` 成功后为 `1`
6. `stop()` 成功后回到 `0`

- [ ] **Step 5: 运行 test 证明当前必然失败**

Run:

```bash
cmake --build build --target test_tinyui_app
ctest --test-dir build -R '^test_tinyui_app$' --output-on-failure
```

Expected:

1. 因 timer API 尚不存在或断言未满足而失败。
2. 不接受“因为别的无关测试红了”这种假失败。

## Task 2: 落 public timer API 与 core 生命周期

**Files:**
- Modify: `tinyui/include/tinyui/app.h`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/app.c`
- Test: `tests/tinyui/unit/test_tinyui_app_timer.c`

- [ ] **Step 1: 在 public header 增加 timer 类型和 API**

在 `tinyui/include/tinyui/app.h` 新增：

```c
struct tinyui_app_timer;

typedef void (*tinyui_app_timer_cb_t)(struct tinyui_app *app,
                                      struct tinyui_app_timer *timer,
                                      void *user_data);

struct tinyui_app_timer *tinyui_app_timer_create(struct tinyui_app *app);
int tinyui_app_timer_start(struct tinyui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           tinyui_app_timer_cb_t callback,
                           void *user_data);
int tinyui_app_timer_stop(struct tinyui_app_timer *timer);
int tinyui_app_timer_is_running(const struct tinyui_app_timer *timer);
void tinyui_app_timer_destroy(struct tinyui_app_timer *timer);
```

- [ ] **Step 2: 在 internal state 加 timer 持有结构**

在 `tinyui/src/core/internal.h` 给 `struct tinyui_app` 增加最小持有链：

```c
struct tinyui_app_timer {
    struct tinyui_app *owner_app;
    struct tinyui_app_timer *next;
    unsigned int interval_ms;
    unsigned int next_fire_ticks;
    int repeat;
    int running;
    tinyui_app_timer_cb_t callback;
    void *user_data;
};
```

并在 `struct tinyui_app` 中加入：

```c
struct tinyui_app_timer *timers;
```

- [ ] **Step 3: 在 `app.c` 实现 create/start/stop/is_running/destroy**

要求：

1. `create()` 绑定 owner app，插入 app timer 链表。
2. `start()` 校验参数，并设置：
   - `interval_ms`
   - `repeat`
   - `callback`
   - `user_data`
   - `running = 1`
3. `stop()` 只改运行态，不销毁对象。
4. `destroy()` 从 app timer 链里摘除并释放。
5. `tinyui_app_destroy()` 要清空残留 timer。

- [ ] **Step 4: 跑 unit test 转绿**

Run:

```bash
cmake --build build --target test_tinyui_app
ctest --test-dir build -R '^test_tinyui_app$' --output-on-failure
```

Expected:

1. timer API 合同测试全部通过。
2. 旧 app 测试不回归。

## Task 3: 在 backend SDL loop 中调度 timer

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Test: `tests/tinyui/unit/test_tinyui_app_timer.c`

- [ ] **Step 1: 写 backend 调度 helper**

在 `backend_app.c` 增加类似 helper：

```c
static void tinyui_backend_app_pump_timers(struct tinyui_app *app, unsigned int now_ticks)
{
    struct tinyui_app_timer *timer = app != NULL ? app->timers : NULL;

    while (timer != NULL) {
        struct tinyui_app_timer *next = timer->next;
        if (timer->running && timer->callback != NULL && now_ticks >= timer->next_fire_ticks) {
            if (timer->repeat) {
                timer->next_fire_ticks = now_ticks + timer->interval_ms;
            } else {
                timer->running = 0;
            }
            timer->callback(app, timer, timer->user_data);
        }
        timer = next;
    }
}
```

要求：

1. 先保存 `next`，允许回调里 stop/destroy 当前 timer。
2. one-shot 回调后自动停掉。
3. repeating timer 更新时间。

- [ ] **Step 2: 在 `tinyui_backend_app_run()` 主循环接线**

在现有 while loop 中加入：

```c
unsigned int now_ticks = SDL_GetTicks();
tinyui_backend_app_pump_timers(app, now_ticks);
```

位置要求：

1. 在 SDL 事件处理之后
2. 在 render 之前

- [ ] **Step 3: 为 one-shot / repeat 行为补 unit contract**

在 `tests/tinyui/unit/test_tinyui_app_timer.c` 增加最小 helper callback + fake ticks 或 backend-friendly contract，至少证明：

1. repeat timer 多次触发后仍 running
2. one-shot timer 第一次触发后自动停掉

如果 unit 层难以直接驱动 SDL loop，可把“实际跑起来会触发”放到 demo/runtime 侧验证，但至少要保留 state contract 测试。

- [ ] **Step 4: 跑测试和 build**

Run:

```bash
cmake --build build --target test_tinyui_app
ctest --test-dir build -R '^test_tinyui_app$' --output-on-failure
```

Expected: 通过。

## Task 4: 用 public timer 收口 `layout_parity`

**Files:**
- Modify: `tinyui/demo/layout_parity/main.c`
- Reference: `examples/common/demo/layout/uiLayout.c`

- [ ] **Step 1: 在 demo 内增加周期状态**

在 `layout_parity/main.c` 增加最小状态结构，例如：

```c
struct layout_timer_state {
    struct tinyui_widget *flex_row_section;
    int compact;
};
```

- [ ] **Step 2: 增加 timer callback**

新增类似：

```c
static void layout_parity_resize_tick(struct tinyui_app *app,
                                      struct tinyui_app_timer *timer,
                                      void *user_data)
{
    struct layout_timer_state *state = (struct layout_timer_state *)user_data;
    (void)app;
    (void)timer;

    if (state == NULL || state->flex_row_section == NULL) {
        return;
    }

    state->compact = !state->compact;
    tinyui_widget_set_width(state->flex_row_section, state->compact ? 170 : 220);
}
```

要求：

1. 行为表达在 demo 层完成。
2. 不回退到 backend special-case。

- [ ] **Step 3: 创建并启动 repeating timer**

在 `main()` 中：

1. 创建 `tinyui_app_timer`
2. `start(interval_ms = 1200, repeat = 1, ...)`
3. 把 section widget 指针传给 callback

- [ ] **Step 4: 构建并跑 boundary contract**

Run:

```bash
cmake --build build --target tinyui_layout_parity_demo
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

Expected:

1. `layout_parity` 继续通过原有边界检查。
2. 新增 timer 消费没有引入 `ld*` 泄漏。

## Task 5: 用 public timer 收口 `legacy_widget_parity` 动画

**Files:**
- Modify: `tinyui/demo/legacy_widget_parity/main.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`

- [ ] **Step 1: 增加 arc/gauge 动画状态**

例如：

```c
struct legacy_timer_state {
    struct tinyui_arc *arc;
    struct tinyui_gauge *gauge;
    unsigned int angle;
};
```

- [ ] **Step 2: 增加 `100ms` repeating callback**

要求：

1. 每次把 angle 递增
2. 回绕到 `0..359`
3. 通过现有 public API 更新 arc / gauge

- [ ] **Step 3: 创建并启动 timer**

在 app run 前启动：

```c
tinyui_app_timer_start(timer, 100, 1, legacy_widget_tick, &state);
```

- [ ] **Step 4: 构建并跑 contract**

Run:

```bash
cmake --build build --target tinyui_legacy_widget_parity_demo
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

Expected: 通过。

## Task 6: 同步文档与总计划入口

**Files:**
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `docs/superpowers/reviews/2026-06-04-tinyui-three-parity-demo-gap-audit.md`
- Modify: `docs/superpowers/plans/2026-06-04-tinyui-three-parity-demo-gap-repair.md`

- [ ] **Step 1: 更新 demo guide**

把：

1. `layout_parity` 的“缺 timer/tick capability 阻塞”
2. `legacy_widget_parity` 的“arc/gauge 动画仍卡 capability”

改成“已有独立 timer capability 线”或“已补通，剩余是资源/visible fidelity”，以真实实现进度为准。

- [ ] **Step 2: 更新总审计**

把总审计里的“唯一硬 capability gap”改成：

1. 若本线完成：已补通
2. 若本线只拆文档未实现：已拆成独立 design + implementation truth-source

- [ ] **Step 3: 更新总 repair plan**

在总 plan 顶部增加回链：

```md
- `timer capability design`: `docs/superpowers/specs/2026-06-04-tinyui-app-timer-tick-capability-design.md`
- `timer capability implementation`: `docs/superpowers/plans/2026-06-04-tinyui-app-timer-tick-capability-implementation.md`
```

## Task 7: 全量验证与变更范围确认

**Files:**
- Verify only

- [ ] **Step 1: 构建相关 targets**

Run:

```bash
cmake -S . -B build
cmake --build build --target \
  test_tinyui_app \
  tinyui_layout_parity_demo \
  tinyui_legacy_widget_parity_demo
```

Expected: 全部通过。

- [ ] **Step 2: 跑相关测试**

Run:

```bash
ctest --test-dir build -R '^test_tinyui_app$' --output-on-failure
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

Expected: 全部通过。

- [ ] **Step 3: 跑变更范围检查**

Run:

```bash
git diff --check
```

Expected: 无输出。

- [ ] **Step 4: 按仓库规则在提交前跑 detect-changes**

要求：

1. 在当前 worktree 根执行 `gitnexus_detect_changes()`
2. 确认受影响符号和执行流符合预期：
   - `tinyui_app_*`
   - `tinyui_backend_app_run`
   - `layout_parity`
   - `legacy_widget_parity`
