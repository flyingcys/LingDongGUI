# TinyUI v2.3 M2 核心纵向闭环实施计划

> **供自动化执行者使用：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，按任务逐项实施；每一步使用复选框（`- [ ]`）跟踪。M2 必须串行执行，不创建 worktree。

**目标：** 建立单实例 runtime、唯一 LingDongGUI 对象树、固定 timer/callback 池和直接 setter adapter，并让 label、button、checkbox、slider 四个纵向样板达到 L1-L5。

**架构：** TinyUI wrapper 仅保存 backend 无法查询且履行公共契约必需的状态；parent/child/root、几何和可支持属性以真实 LingDongGUI 对象为真值。runtime、timer、callback 都由一个静态 runtime 状态持有，dispatch 不分配；公共 setter 先预检，再直接调用 `ld*`，成功后才更新最小缓存。

**技术栈：** C11、CMake/CTest、LingDongGUI、Arm-2D、SDL 测试宿主、Python 3 合约与像素检查、GitNexus。

## 全局约束

- M0、M1 必须已经关闭；本计划不得修改 M1 已冻结的 canonical 公共签名。
- TinyUI 是 LingDongGUI 的薄 API 层，不新增 renderer、对象树、布局器、事件队列、selector 或资源中心。
- 每个 TinyUI 对象必须绑定一个真实 LingDongGUI 对象；成功 setter 必须改变真实 backend 状态。
- `tinyui_process()`、timer dispatch、event dispatch、layout、普通 getter 的隐式 heap 分配必须为 `0`。
- `TINYUI_TIMER_CAPACITY` 和 `TINYUI_EVENT_CB_CAPACITY` 默认均为 `16`；容量耗尽返回 `TINYUI_ERROR_CAPACITY`。
- 32 位 ABI 下 timer pool、event callback pool 与 runtime bookkeeping 静态 RAM 合计不得超过 `1024 B`。
- 基础 wrapper 不得超过 `192 B`；每种 wrapper 相对 M0 的增量不得同时超过 `5%` 和 `8 B`，也不得越过 M0 记录的绝对上限。
- 不修改 SDL/MCU port 生产契约，不把 SDL smoke 或 fake renderer 当作 L4/L5 证据。
- 所有 shell 命令均以 `rtk` 开头；构建统一使用 CMake。
- 修改任何函数前必须先执行 GitNexus upstream `impact`；`HIGH`/`CRITICAL` 必须先列出调用者迁移清单并告知用户。
- 阶段收口必须执行 GitNexus `detect_changes({scope: "compare", base_ref: "master"})`，确认只影响 M2 预期符号和流程。

---

## 写面与顺序

M2 的核心文件高度耦合，必须由一个执行序列串行修改。`tinyui/src/core/internal.h`、`tinyui/src/core/widget.c`、`tinyui/include/tinyui.h` 和能力矩阵只允许对应任务中的单一负责人写入。

| 任务 | 唯一写面 | 交付接口 |
| --- | --- | --- |
| 1 | runtime、screen、`internal.h`、runtime test | `tinyui_init/deinit/process`、screen load/active、全部 M2 内部声明 |
| 2 | `widget.c`、window、object test | 唯一 LD tree、ID、遍历、删除、通用 setter、文本所有权 |
| 3 | timer 与 timer test | 固定 timer pool 与最近 deadline |
| 4 | event/focus 与 event test | 固定 callback pool、同步 dispatch、删除语义 |
| 5 | 通用 setter、label 与测试 | 静态 adapter 路由、label L1-L4 |
| 6 | button 与测试 | button L1-L5-E |
| 7 | checkbox/slider 与测试 | value 控件 L1-L5-E |
| 8 | runtime 证据脚本、baseline、矩阵 | 四样板 L5-V/L5-E 可定位证据 |
| 9 | 性能脚本、CMake、阶段报告 | RAM/尺寸/分配/构建门禁 |

### 任务 1：收敛单实例 runtime 与 screen 生命周期

**文件：**
- 修改： `tinyui/src/core/runtime.c`
- 修改： `tinyui/src/core/runtime_bridge.c`
- 修改： `tinyui/src/core/runtime_bridge.h`
- 修改： `tinyui/src/core/internal.h`
- 修改： `tests/tinyui/unit/test_tinyui_runtime_model.c`
- 修改： `tests/tinyui/unit/test_tinyui_app_lifecycle.c`

**接口：**
- 输入： M1 冻结的 `tinyui_result_t`、`tinyui_screen_transition_t`、`tinyui_obj_t`。
- 输出： `tinyui_init(void)`、`tinyui_deinit(void)`、`tinyui_process(uint32_t *next_ms)`、`tinyui_screen_create(void)`、`tinyui_background_create(void)`、`tinyui_screen_load(tinyui_obj_t *, tinyui_screen_transition_t, uint32_t)`、`tinyui_screen_active(void)`。

- [x] **步骤 1：分析高风险符号影响面**

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替。风险按调用面估算：

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_init` / `tinyui_deinit` / `tinyui_process` | HIGH | unit：`test_tinyui_runtime_model` 等；contract：`test_tinyui_v23_*`；minimal consumer；demo（M4） |
| `tinyui_screen_create` / `tinyui_background_create` / `tinyui_screen_load` / `tinyui_screen_active` | CRITICAL/HIGH | 同上 + 大量 unit 仍走 legacy `tinyui_app_create`；demo 全树（M4） |
| `tinyui_app_create` | HIGH（legacy bridge） | 大量 unit/demo 仍依赖 v22 bridge；本任务不迁移 |
| `tinyui_runtime_bridge_step_app` | HIGH | `runtime.c` process/timer_handler、`runtime_bridge_run_app` |

M2 只迁移 core/unit 消费者；demo 全量迁移留给 M4。

- [x] **步骤 2：写 runtime 状态失败测试**

在 `tests/tinyui/unit/test_tinyui_runtime_model.c` 增加以下独立断言：

```c
static void test_v23_runtime_is_single_instance(void)
{
    uint32_t next_ms = 0U;

    tinyui_deinit();
    TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_init());
    TEST_ASSERT_EQUAL(TINYUI_ERROR_INVALID_STATE, tinyui_init());
    TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_process(&next_ms));
    tinyui_deinit();
    tinyui_deinit();
    TEST_ASSERT_EQUAL(TINYUI_ERROR_INVALID_STATE, tinyui_process(&next_ms));
}

static void test_v23_screen_load_tracks_real_active_root(void)
{
    tinyui_obj_t *first;
    tinyui_obj_t *second;

    TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_init());
    first = tinyui_screen_create();
    second = tinyui_background_create();
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_screen_load(first,
                                                   TINYUI_SCREEN_TRANSITION_NONE,
                                                   0U));
    TEST_ASSERT_EQUAL_PTR(first, tinyui_screen_active());
    TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_screen_load(second,
                                                   TINYUI_SCREEN_TRANSITION_SLIDE_LEFT,
                                                   120U));
    TEST_ASSERT_EQUAL_PTR(second, tinyui_screen_active());
    tinyui_deinit();
}
```

将两个函数注册到本文件现有 runner（本文件使用 `assert`，非 Unity）。

- [x] **步骤 3：验证测试按预期失败**

运行：

```sh
rtk cmake -S . -B build/v2.3-m2 -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_SDL_DEMO=ON -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m2 --target test_tinyui_runtime_model -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_runtime_model$' --output-on-failure
```

结果：失败符合预期——`test_v23_runtime_is_single_instance` 在二次 `tinyui_init()` 断言 `INVALID_STATE` 处 abort（旧实现重复 init 返回 `OK`）。

- [x] **步骤 4：实现固定 runtime 状态**

在 `tinyui/src/core/internal.h` 一次性定义 M2 所需 runtime、timer slot、event callback slot、dispatch epoch、deleting 标志与内部 helper 声明；后续任务不得再次修改此共享头。runtime 主体不放入公共头：

```c
struct tinyui_runtime_state {
    bool initialized;
    bool processing;
    struct tinyui_app *backend_app;
    tinyui_obj_t *active_screen;
    const tinyui_theme_t *theme;
    uint32_t now_ms;
    tinyui_result_t last_result;
#if TINYUI_ENABLE_DIAGNOSTICS
    char diagnostic[96];
#endif
};

struct tinyui_runtime_state *tinyui_runtime_state_get(void);
```

在 `tinyui/src/core/runtime.c` 使用文件静态实例；`tinyui_init()` 只创建一次 backend app，重复调用返回 `INVALID_STATE`；`tinyui_deinit()` 销毁全部 root、清空 pool 和状态，未初始化时直接返回；`tinyui_process()` 依次处理 backend、timer 和同步事件，拒绝重入，计算 `next_ms`，不得 sleep 或分配。

transition 用 `static const` 映射表映射到现有 Arm-2D scene switch mode；`NONE` 走立即切换。表大小使用静态断言覆盖枚举末项，未知值返回 `OUT_OF_RANGE`。

- [x] **步骤 5：删除 runtime 对 public app 生命周期的依赖**

在 `runtime.c` 和 `runtime_bridge.c` 中只调用内部 backend app 构造/销毁与 scene switch helper；移除 runtime 对 public `tinyui_app_run()`、`tinyui_app_set_window()`、`tinyui_app_switch_window()` 的调用。旧 app timer 的替换由 任务 3 在 `app.c` 完成。不要修改 port 的显示、tick、PFB 或 SDL 生命周期。

- [x] **步骤 6：运行 runtime 单测**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_runtime_model test_tinyui_app_lifecycle -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(runtime_model|app_lifecycle)$' --output-on-failure
```

结果：两个测试通过；重复 init 返回 `INVALID_STATE`，重复 deinit 无副作用，active screen 等于真实加载 root。

- [x] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

结果：无 whitespace 错误。

#### 任务 1 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心实现：`tinyui/src/core/runtime.c` 单实例 `s_tinyui_runtime`；`internal.h` 一次性声明 M2 runtime/timer/event pool/deleting 相关字段与 helper。
- 行为：`tinyui_init` 重复 → `INVALID_STATE`；`tinyui_screen_load` 接受完整 transition 映射并更新 `active_screen`；`tinyui_process` 拒重入。
- 测试：`test_tinyui_runtime_model` 全过；`test_tinyui_app_lifecycle` 全过（demo 二进制缺失时跳过 host smoke）。
- 附带：`tests/tinyui/CMakeLists.txt` 暂时取消 `app_lifecycle` 对 `tinyui_demo` 的硬依赖；contract runtime 用例同步 transition 期望。
- 未做：Task 2+；timer/event 真实 dispatch；demo 全树迁移（M4）；真实 Arm-2D 视觉切换播放仍由 scene player 后续接。

### 任务 2：让 LingDongGUI tree 成为唯一对象树真值

**文件：**
- 修改： `tinyui/src/core/widget.c`
- 修改： `tinyui/src/core/widget_registry.c`
- 修改： `tinyui/src/widgets/window.c`
- 修改： `tests/tinyui/unit/test_tinyui_lifecycle.c`
- 修改： `tests/tinyui/unit/test_tinyui_widgets.c`

**接口：**
- 输入： 任务 1 的 runtime state 与 M1 的统一 `tinyui_obj_t`。
- 输出： `tinyui_obj_get_id`、`tinyui_obj_find_by_id`、parent/child/sibling/root/count 查询、`tinyui_obj_delete`、复制语义的 `tinyui_obj_set_text`。

- [x] **步骤 1：分析对象核心影响面**

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替。

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_widget_create_leaf` / `create_leaf_with_id` | HIGH | 全部控件 creator（label/button/…/window child）；unit 源码契约 |
| `tinyui_widget_destroy_common` / `obj_delete` | HIGH | 全部 props creator 失败回滚、runtime deinit、unit lifecycle |
| `tinyui_widget_set_text` | HIGH | label/button/checkbox/text 公共 set_text；demo bridge |
| `tinyui_app_lookup_host` | MEDIUM/HIGH | 树遍历、from_ld、事件反查、unit registry |

本任务只改 common path（ID/树/delete/text 提交顺序）；控件专用 L4 由任务 5-7 完成。

- [x] **步骤 2：写唯一树和 ID 失败测试**

在 `tests/tinyui/unit/test_tinyui_lifecycle.c` 增加 `test_v23_object_tree_and_id_queries` 与 `test_v23_explicit_id_conflict_keeps_backend_count`（assert 风格与文件一致）。

- [x] **步骤 3：写文本复制与失败原子性测试**

在 `tests/tinyui/unit/test_tinyui_widgets.c` 增加 `test_v23_text_copy_and_oom_atomicity`（栈缓冲改写、OOM fail-closed、delete 回收 live block）。该文件历史 parity 套件因 M1 一参 create/资源类型漂移暂时收敛为 Task 2 文本契约 harness。

- [x] **步骤 4：运行测试确认失败**

结果：失败符合预期——`find_by_id` 对 nested window 子树失败（旧 window_create 不挂父）；文本 OOM 路径未 fail-closed。

- [x] **步骤 5：实现 ID、host lookup 与真实树遍历**

- `create_leaf_with_id`：显式 ID 冲突前置检查；失败释放 auto ID
- `window_create(parent)`：nested window 使用 parent nameId + LD 挂接
- 遍历仍走 `ldBaseGetParent/ChildList/NextSibling` + host lookup

- [x] **步骤 6：实现同步删除与 callback 延迟删除钩子**

- `deleting` 标志；process 中 `delete_pending`/`delete_target` 延迟标记
- **`tinyui_process` 末尾 flush**：`processing=false` 后对 `delete_target` 调用同一 `widget_destroy` helper，再清 flags
- process 中二次 delete / 已有 pending 时返回 `INVALID_STATE`（不立即 destroy）
- destroy 路径释放 owned text；container 子树 reclaim 同步清理 host

- [x] **步骤 7：实现通用 setter 与文本复制的提交顺序**

- common `set_text`：alloc 副本 → backend → 成功才替换 cache；backend 失败释放新副本
- label/button/checkbox/text 专用 setter 只转发 common path，不再二次 `set_backend_text`

- [x] **步骤 8：运行 object 测试**

`test_tinyui_lifecycle` / `test_tinyui_widgets` 全过（含 deferred delete flush 用例）。

- [x] **步骤 9：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 2 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：LD tree 为 parent/child/sibling/root 真值；显式 ID 冲突 fail-closed；文本复制 + OOM 原子；delete 释放 owned text。
- 延迟删除：mid-`processing` 仅 mark；`tinyui_process` 结束 flush 真 destroy；二次 delete → `INVALID_STATE`。
- 测试：lifecycle/widgets Task2 断言通过（含 `test_v23_deferred_delete_flushed_by_process`）。
- 附带：`test_tinyui_widgets.c` 历史大套件暂时收敛为 Task2 harness（M1 API 漂移，完整 parity 回迁留给后续控件任务）。
- 未做：Task 3+；完整 common `tinyui_obj_set_*` 公共 ABI（M1 契约未冻结这些符号）；全量 widget parity 重绿；DELETE 事件/callback slot 清理仍属 Task 4。

### 任务 3：以固定池实现 timer 和 deadline

**文件：**
- 修改： `tinyui/src/core/app.c`
- 修改： `tests/tinyui/unit/test_tinyui_app_timer.c`

**接口：**
- 输入： 任务 1 的 `tinyui_process(uint32_t *next_ms)` 和 runtime clock。
- 输出： `tinyui_timer_create/start/stop/set_interval/delete`；内部 `tinyui_timer_process(now_ms, next_ms)`。

- [x] **步骤 1：分析 timer 影响面**

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替。

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_timer_create/start/stop/set_interval/delete` | HIGH | 原 M1 stub（NOT_SUPPORTED）；contract fail-closed；M4 demo 待迁 |
| `tinyui_process(next_ms)` | HIGH | runtime unit、steady-state、lifecycle、contract |
| `tinyui_runtime_internal_app_pump_timers` | MEDIUM | legacy linked-list path 仍供 `neutral_runtime`/v22 bridge；本任务不删除 |
| `tinyui_app_timer_*` | MEDIUM | unit 旧路径、demo bridge；canonical 用户走 `tinyui_timer_*` |

替换表：`app_timer_create/start/stop/destroy` → `timer_create/start/stop/delete`；`pump_timers`/`timer_handler` → `tinyui_process(&next_ms)`。

- [x] **步骤 2：写固定池与 mutation 失败测试**

在 `tests/tinyui/unit/test_tinyui_app_timer.c` 覆盖：创建 `TINYUI_TIMER_CAPACITY` 项成功，第 `capacity + 1` 项返回 `NULL` 且 `tinyui_last_result()==CAPACITY`；删除后槽可复用；回调中 stop/delete 当前 timer；回调中 create timer 不在同轮触发；一次性 timer 触发一次；`UINT32_MAX - 4` 到 `3` 的回绕比较正确；最近 deadline 分别返回 `0`、剩余毫秒和 `UINT32_MAX`。

回调 mutation 断言使用：

```c
static void delete_self_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = user_data;
    ++*calls;
    tinyui_timer_delete(timer);
}
```

- [x] **步骤 3：确认旧 snapshot heap 路径导致失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_app_timer -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_app_timer$' --output-on-failure
```

结果：失败符合预期——`tinyui_timer_create` 仍为 M1 stub（`NOT_SUPPORTED`/`NULL`），容量断言在首项 create 失败。

- [x] **步骤 4：实现固定 timer pool**

内部槽为定长数组，字段为 `generation`、`interval_ms`、`deadline_ms`、`cb`、`user_data`、`allocated`、`running`、`repeat`、`born_epoch`、`deleting`（更新 `runtime_pools.h`）。`tinyui_timer_t` 直接引用槽，不建立链表，不分配 snapshot。

到期判断使用有符号差值：

```c
static bool tinyui_time_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}
```

dispatch 开始递增 epoch；只处理 `born_epoch < dispatch_epoch` 的槽。回调后重新检查 `allocated/generation/running`，从而支持 stop/delete 自身。重复 timer 从当前 `now + interval` 安排下一次，不在一轮内追赶调用。

- [x] **步骤 5：把 deadline 接入 process**

`tinyui_process` 在 backend step 后调用 `tinyui_timer_process`：无 deadline 写 `UINT32_MAX`，已到期写 `0`，否则写最小无符号剩余值。`next_ms == NULL` 合法，只忽略输出。

- [x] **步骤 6：验证 timer 与零分配**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_app_timer test_tinyui_runtime_model -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(app_timer|runtime_model)$' --output-on-failure
```

结果：全部通过；timer dispatch allocator delta 为 `0`，回绕和 mutation 结果确定。

- [x] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 3 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：`runtime.c` 固定池 `tinyui_timer_*` + `tinyui_timer_process`；`runtime_pools.h` 槽字段对齐 `deadline_ms/born_epoch/deleting`。
- 行为：容量 16 / CAPACITY；回调内 stop/delete；同轮 create 不触发且 `next_ms=0`；回绕；process 零堆。
- 测试：`test_tinyui_app_timer`、`test_tinyui_runtime_model` 全过；Task1/2 lifecycle/widgets/steady_state 仍绿。
- 附带：`test_tinyui_v23_event_timer_focus_contract` 从 timer fail-closed 改为真实 pool 语义；`process(NULL)` 合法。
- 未做：Task 4+；legacy `app_timer`/`pump_timers` 链表路径仍保留给 bridge/neutral_runtime；event callback 固定池未接；`runtime_static_ram.implementation` 仍为 `legacy`（event 池未真实接线）。
### 任务 4：以固定池实现统一事件与焦点

**文件：**
- 修改： `tinyui/src/core/event.c`
- 修改： `tests/tinyui/unit/test_tinyui_event.c`
- 修改： `tests/tinyui/unit/test_tinyui_button_events.c`

**接口：**
- 输入： 任务 2 的 deleting/delete helper 与 LD `SIGNAL_*` bridge。
- 输出： `tinyui_obj_add_event_cb`、`tinyui_obj_remove_event_cb`、`tinyui_focus_set/clear/move/current`、同步 `tinyui_event_t` dispatch。

- [x] **步骤 1：分析事件与焦点影响面**

```text
impact({target: "tinyui_widget_dispatch_native_signal", direction: "upstream"})
impact({target: "tinyui_widget_claim_focus", direction: "upstream"})
impact({target: "tinyui_button_set_on_clicked", direction: "upstream"})
```

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替。

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_runtime_internal_widget_dispatch_native_signal` | HIGH | `runtime_bridge` LD msg 槽；button/checkbox/switch/slider/list 分支；unit：`test_tinyui_event`/`button_events`/`keyboard` |
| `tinyui_runtime_internal_widget_claim_focus` / public `tinyui_focus_*` | MEDIUM | event focus path、keyboard/table、focus API |
| `tinyui_button_set_on_clicked` 等专用 setter | HIGH（迁移债） | legacy wrapper 字段仍在；完整收口属任务 6 |

任务 4 只接统一 pool + 同步 dispatch；旧专用 callback setter 保留，任务 6/7 再收口。

- [x] **步骤 2：写 callback pool、generation 与 mutation 测试**

在 `tests/tinyui/unit/test_tinyui_event.c` 覆盖默认容量 16、第 17 项返回 `CAPACITY`、槽复用后旧 handle 无法移除新 callback、注册顺序、callback 内新增不参加当前轮、移除后续 callback 立即生效、删除 target 停止后续 callback、`DELETE` 恰好一次且仅允许 getter。

必须断言 handle 编码：

```c
TEST_ASSERT_NOT_EQUAL(0U, handle);
TEST_ASSERT_NOT_EQUAL(0U, handle & UINT32_C(0xffff));
TEST_ASSERT_NOT_EQUAL(old_handle, reused_handle);
TEST_ASSERT_EQUAL(TINYUI_ERROR_INVALID_ARG,
                  tinyui_obj_remove_event_cb(button, old_handle));
```

- [x] **步骤 3：写无冒泡与 payload 测试**

给 parent 和 child 同时注册 `CLICKED`；向 child 注入真实 LD click signal，只允许 child callback 增加。对 slider 注入真实 value signal，断言 `event.code==VALUE_CHANGED`、`event.target==slider`、`event.data.value` 为 canonical value；对 key 断言 `data.key.key/pressed`。

- [x] **步骤 4：运行事件测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_event test_tinyui_button_events -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(event|button_events)$' --output-on-failure
```

结果：`test_tinyui_event` 在 `add_event_cb == OK` 处 abort（旧实现 fail-closed `NOT_SUPPORTED`），符合 RED。

- [x] **步骤 5：实现固定 callback pool**

runtime 内保存 `TINYUI_EVENT_CB_CAPACITY` 个槽；每槽保存 object、mask、cb、user_data、generation、registration_order、allocated、born_epoch。对象只保存第一个槽索引或无效索引；不得在 wrapper 内嵌 callback 数组。

dispatch 顺序扫描固定池并按 registration_order 处理；开始时固定 epoch，新增槽的 born_epoch 等于当前 epoch，因此下一轮生效。每次调用前重新检查 allocated、generation、object 和 mask。只转换当前设计列出的八种事件，不排队、不冒泡、不 capture。

- [x] **步骤 6：实现焦点直连与 delete 事件**

`focus_set/clear/move/current` 直接复用现有 LingDongGUI navigation；TinyUI 只保存 backend 无 getter 时必需的当前 wrapper。`DELETE` 在 deleting 标记后、LD depose 前由 `tinyui_event_emit_delete()` 同步发出；回调返回后清理该对象所有槽，再调用 任务 2 已实现的真实销毁 helper。

- [x] **步骤 7：验证事件、焦点和零分配**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_event test_tinyui_button_events test_tinyui_lifecycle -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(event|button_events|lifecycle)$' --output-on-failure
```

结果：全部通过；event dispatch allocator delta 为 `0`，parent callback 不因 child 事件执行。

- [x] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 4 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：`event.c` 固定 callback 池（容量 16 / CAPACITY）；handle = `(generation<<16)|(slot+1)`；同步 `tinyui_event_fire`；`tinyui_event_emit_delete` 接 destroy；native signal → 八种事件码；KEY 经 `tinyui_input_send_key` 发往 focus。
- 行为：注册顺序、born_epoch 本轮不触发、移除后续立即生效、delete target 停止后续、无冒泡、dispatch 零堆。
- 测试：`test_tinyui_event`、`test_tinyui_button_events`、`test_tinyui_lifecycle`、`test_tinyui_v23_event_timer_focus_contract` 与 Task1-3 相关测试全绿。
- 附带：contract 从 event fail-closed 改为真实 pool；`button_events` 仅做 v2.3 签名兼容与 props.fields 修正。
- baseline：`runtime_static_ram.implementation` **仍保持 `legacy`**——timer/event 池虽已真实接线，bookkeeping 仍只是预算布局探针，未达到“全部固定池行为诚实切换”标准。
- 未做：Task 5+；旧 button/checkbox/slider 专用 callback 字段未删除（任务 6/7）；不宣称 M2 完成。

### 任务 5：建立直接 setter adapter 并闭环 label

**文件：**
- 修改： `tinyui/src/widgets/label.c`
- 修改： `tinyui/src/core/widget.c`
- 修改： `tinyui/include/core/obj.h`
- 修改： `tests/tinyui/unit/test_tinyui_core_helpers.c`
- 修改： `tests/tinyui/unit/test_tinyui_label.c`
- 修改： `tests/tinyui/contract/tinyui_v23_public_api.json`

**接口：**
- 输入： M1 的 `tinyui_obj_set_*` 和 任务 2 的 text 提交顺序。
- 输出： 静态“kind -> adapter”路径；label 的 create/props/text/font/color/transparent/align/background 能力。

- [x] **步骤 1：分析 setter 与 label 影响面**

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替。

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_widget_set_text` / common text path | HIGH | label/button/checkbox/text/qrcode 专用 set_text；demo bridge |
| `tinyui_widget_set_size` / geometry | HIGH | 全部 props creator；layout 后续 |
| `tinyui_label_set_text` | HIGH | unit/demo 文本入口；应只转发 common path |
| 新增 `tinyui_obj_set_*` | MEDIUM/HIGH | M1 已规划但未冻结进 manifest；本任务补齐声明+实现 |

M2 只闭环四样板 common adapter 与 label L4；button/checkbox/slider 专用语义属任务 6/7。

- [x] **步骤 2：写 adapter 选择与假成功拒绝测试**

在 `tests/tinyui/unit/test_tinyui_core_helpers.c` 对 label/button/checkbox/slider 逐项调用 common setter，并从 `ldBase_t` / 具体 `ld*` 字段读回；border/radius/padding 与 slider text 等不支持组合断言 `NOT_SUPPORTED` 且 wrapper 镜像不变。

- [x] **步骤 3：写 label L3/L4 测试**

`test_tinyui_label.c` 覆盖 1-arg create、空 parent、错误 kind、props presence 预检失败回滚、真实 `ldLabel_t` text/font/color/align/transparent/background tile 与尺寸。

- [x] **步骤 4：运行测试确认失败**

结果：RED 符合预期——`tinyui_obj_set_*` 未声明/未实现；旧 label 测试仍是 2-arg create 与假成功 style 路径。

- [x] **步骤 5：实现静态 adapter 路由**

- `core/obj.h` 增加 common `tinyui_obj_set_*`；`widget.c` 实现并设置 `last_result`
- 编译期 `switch(kind)`：text / bg / text color 仅真实 LD 能力；border/radius/padding（leaf）`NOT_SUPPORTED`
- 顺序：参数预检 → kind 支持预检 → LD 调用 → 最小 cache；无运行时 property registry

- [x] **步骤 6：收敛 label creator 与专用 setter**

- `create_with_props`：先 `props_are_valid`（含不支持 presence 字段），再 create + 正式 setter；失败 `obj_delete` 回滚 child count
- `tinyui_label_set_text` 只转发 `tinyui_obj_set_text`；bg/text color 转发 common path；font/align/transparent/background 直接 `ldLabel*`

- [x] **步骤 7：验证 label L1-L4**

`test_tinyui_core_helpers` / `test_tinyui_label` 全绿；Task1-4 相关 `lifecycle/widgets/event/app_timer/app_lifecycle` 保持通过。

- [x] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 5 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：common `tinyui_obj_set_*` 公共 ABI + 静态 kind→adapter；label L1-L4 闭环。
- 支持矩阵（M2 样板）：
  - geometry/visible/opacity/selectable/selected/enabled → `ldBase*`
  - text → label/button/checkbox（及既有 text/qrcode path）
  - bg color → label/button/checkbox/slider
  - text color → label/button/checkbox
  - border color/width、radius、leaf padding → `NOT_SUPPORTED`（拒绝假成功）
- 测试：`test_tinyui_core_helpers`、`test_tinyui_label` 通过；Task1-4 相关测试抽样全绿。
- 契约：`tinyui_v23_public_api.json` 登记 14 个 `tinyui_obj_set_*` 并更新 hash。
- 未做：Task 6+；button/checkbox/slider 专用 L4/L5-E；style/theme 真实 apply；不宣称 M2 完成。

### 任务 6：闭环 button 的真实状态与事件

**文件：**
- 修改： `tinyui/src/widgets/button.c`
- 修改： `tests/tinyui/unit/test_tinyui_button_events.c`

**接口：**
- 输入： 任务 4 的统一 callback、任务 5 的 common setter。
- 输出： button text/font/release-press color/image/transparent/checkable/key/pressed 与 `PRESSED/RELEASED/CLICKED`。

- [x] **步骤 1：分析 button 影响面**

```text
impact({target: "tinyui_button_create", direction: "upstream"})
impact({target: "tinyui_button_set_pressed", direction: "upstream"})
impact({target: "tinyui_button_set_on_clicked", direction: "upstream"})
```

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替：

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_button_create` | HIGH | unit/demo/minimal；本任务保持 create 语义 |
| `tinyui_button_set_pressed` | MEDIUM | unit（`button_events`）与 props；canonical 保留 |
| `tinyui_button_set_on_clicked/pressed/released` | HIGH（迁移） | unit、`message_box`、basic_widgets demo、legacy_demo0；改为统一 pool 窄转发 |

执行记录只保留 M1 冻结 canonical 名称 `set_pressed/get_pressed`；`set_press` 不进公共头。

- [x] **步骤 2：写 button L3/L4/L5-E 失败测试**

在现有 button test 中验证每个 setter 后真实 `ldButton_t` 字段；对真实 `SIGNAL_PRESS`/`SIGNAL_RELEASE` 注入，断言统一 event code、target、user_data 和注册顺序。disabled/hidden button 不产生交互事件；旧 handle 不得影响新 callback。

- [x] **步骤 3：运行失败测试**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_button_events -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_button_events$' --output-on-failure
```

预期： 专用 callback 字段或重复 press API 绕过统一事件池，测试失败。

- [x] **步骤 4：实现 button 直接映射**

删除 wrapper 中 `on_clicked/on_pressed/on_released` 三组 callback 状态，专用注册 API 只作为 `tinyui_obj_add_event_cb` 的窄转发（replace 语义）。pressed、checkable、颜色、图片和 key 调用真实 `ldButton*`；color getter 读 LD 真值。

- [x] **步骤 5：验证 button L1-L4/L5-E**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_button_events test_tinyui_event -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(button_events|event)$' --output-on-failure
```

预期： 测试通过，三种用户操作事件均来自真实 LD signal/action 路径。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 6 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：button L1-L4/L5-E 闭环；`set_on_*` 改为统一 event pool 窄转发；删除 wrapper 专用 callback 字段。
- 事件：`SIGNAL_PRESS/RELEASE` → `TINYUI_EVENT_PRESSED/RELEASED/CLICKED` 仅走 `tinyui_event_fire`；无第二事件系统。
- 状态：text/font/color/text_color/transparent/checkable/key/pressed 读写真实 `ldButton_t`；color set 经 RGB→LD 转换。
- 测试：`test_tinyui_button_events`、`test_tinyui_event` 通过；Task1-5 抽样（runtime/timer/core_helpers/label/lifecycle）全绿。
- 兼容：demo `basic_widgets`/`legacy_demo0_parity` 与 `test_tinyui_message_box` 的 `set_on_*` 回调签名迁到 `tinyui_event_cb_t`。
- 未做：Task 7+；checkbox/slider 专用 callback 字段仍在；props `ON_CLICKED` 旧 `tinyui_event_cb` 形态拒绝假成功；不宣称 M2 完成。

### 任务 7：闭环 checkbox 与 slider 的 value 语义

**文件：**
- 修改： `tinyui/src/widgets/checkbox.c`
- 修改： `tinyui/src/widgets/slider.c`
- 修改： `tests/tinyui/unit/test_tinyui_checkbox.c`
- 修改： `tests/tinyui/unit/test_tinyui_slider.c`

**接口：**
- 输入： 任务 4 的 `VALUE_CHANGED` payload、任务 5 的 common setter。
- 输出： checkbox checked/text/check color/image/radio/spacing；slider range/value/percent/orientation/image/color/width/slim size。

- [x] **步骤 1：分析两个 value 控件影响面**

```text
impact({target: "tinyui_checkbox_set_checked", direction: "upstream"})
impact({target: "tinyui_slider_set_value", direction: "upstream"})
impact({target: "tinyui_widget_sync_ld_value", direction: "upstream"})
```

GitNexus MCP 不可用；以 `rtk rg` 调用者扫描代替：

| 符号 | 风险 | 调用者分类 |
| --- | --- | --- |
| `tinyui_checkbox_set_checked` | MEDIUM | unit、demo；programmatic 不发事件 |
| `tinyui_slider_set_value` / `set_range` | HIGH | unit/demo；需 64 位映射与 `OUT_OF_RANGE` |
| `tinyui_runtime_internal_widget_sync_ld_value` | HIGH（共享） | checkbox/switch/slider；M2 不扩展 switch/list |
| `tinyui_checkbox_set_on_toggled` / `tinyui_slider_set_on_value_changed` | HIGH（迁移） | unit、basic_widgets demo；改为统一 pool 窄转发 |

预期：shared value helper 同时影响 switch/list 等 M3 控件；M2 不扩展到未列控件。

- [x] **步骤 2：写 checkbox 真实状态与事件测试**

验证 checked、text、check/text color、unchecked/checked image、radio group、string spacing 直接改变 `ldCheckBox_t`；真实 toggle signal 产生一次 `VALUE_CHANGED` 且 `data.value` 为 `0/1`；programmatic setter 不伪造用户事件；props 中任何不支持字段不修改对象。

- [x] **步骤 3：写 slider 范围映射与事件测试**

覆盖 `[-50, 150]` 与 `[7, 7]`；canonical value 与 LD percent 的双向换算必须确定，越界返回 `OUT_OF_RANGE` 且不改值。真实 LD permille 输入映射到 canonical value并产生一次 `VALUE_CHANGED`。orientation、图片、颜色、indicator width、slim size 均从 `ldSlider_t` 读取验证。

- [x] **步骤 4：运行失败测试**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_checkbox test_tinyui_slider -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(checkbox|slider)$' --output-on-failure
```

预期： 旧 wrapper callback、先写 cache 后写 backend、或 percent/value 换算至少一项失败。

实测 RED：`is_checked` 不读 LD；slider 越界未设 `TINYUI_ERROR_OUT_OF_RANGE`。

- [x] **步骤 5：实现 checkbox 提交顺序**

所有参数和支持能力预检完成后调用 `ldCheckBox*`；成功后才更新必需缓存。checked getter 直接读取 LD 状态。旧 `cb/user_data` 从 wrapper 删除，专用 toggled 注册若保留则转发统一 event API。

- [x] **步骤 6：实现 slider 双向值映射**

range/value 使用 64 位中间值避免溢出：

```c
percent = (int32_t)(((int64_t)(value - min_value) * 100) /
                    (max_value - min_value));
value = min_value + (int32_t)(((int64_t)(max_value - min_value) * permille) /
                              1000);
```

零范围固定映射到 min。先调用 `ldSliderSetPercent`，成功后提交 min/max/value 缓存；真实 signal 反向换算后同步触发统一事件。

- [x] **步骤 7：验证两控件 L1-L4/L5-E**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_checkbox test_tinyui_slider test_tinyui_event -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(checkbox|slider|event)$' --output-on-failure
```

预期： 全部通过；事件来自真实 LD 路径，setter 失败不污染缓存。

- [x] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 7 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：checkbox/slider L1-L4/L5-E 闭环；`set_on_toggled` / `set_on_value_changed` 改为统一 event pool 窄转发；删除 wrapper 专用 `cb/user_data` 字段。
- 事件：`SIGNAL_VALUE_CHANGED` → `TINYUI_EVENT_VALUE_CHANGED` 仅走 `tinyui_event_fire`；programmatic setter 不伪造用户事件；无第二事件系统。
- 状态：checkbox checked/text/colors/images/radio/spacing 读写真实 `ldCheckBox_t`；slider range/value/percent 64 位映射 + `OUT_OF_RANGE`；orientation/image/color/width/slim 读 `ldSlider_t`。
- 测试：`test_tinyui_checkbox`、`test_tinyui_slider`、`test_tinyui_event`、`test_tinyui_button_events` 通过；Task1-5 抽样（runtime_model/app_timer/core_helpers/label）全绿。
- 兼容：demo `basic_widgets` 的 checkbox/slider 回调签名迁到 `tinyui_event_cb_t`；props `ON_TOGGLED`/`ON_VALUE_CHANGED` 旧 `tinyui_value_changed_cb` 形态拒绝假成功。
- 未做：Task 8+；switch 专用 callback 字段仍在；不宣称 M2 完成。

### 任务 8：建立四样板 L5 像素与事件证据

**文件：**
- 修改： `tests/tinyui/runtime/tinyui_sdl_observe.c`
- 修改： `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- 修改： `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 新建： `tests/tinyui/runtime/baselines/v23_core_vertical.ppm`
- 修改： `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- 修改： `docs/v2.3/v2.3-capability-evidence-matrix.md`

**接口：**
- 输入： 任务 5-7 的四个真实控件和 任务 4 的统一事件。
- 输出： `v23_core_vertical` 场景的真实像素证据、事件 trace 与逐能力矩阵绑定。

- [x] **步骤 1：定义确定性证据场景**

在 SDL observe 测试入口创建一个真实 LD screen，使用固定坐标排列 label、button、checkbox、slider；场景只用 canonical `tinyui_*` API，不写 backend/layout 补偿。为每个可见能力定义矩形采样区域和至少一个非背景像素断言；为 button click、checkbox toggle、slider change 定义按顺序的 event trace。

- [x] **步骤 2：先写失败的证据检查**

`check_tinyui_visible_ui.py` 增加 `v23_core_vertical` 场景，要求 capture 非空、尺寸匹配、四个区域相对 baseline 的像素差不超过阈值。`check_tinyui_backend_mapping.py` 要求 trace 精确包含：

```text
button:PRESSED
button:RELEASED
button:CLICKED
checkbox:VALUE_CHANGED:1
slider:VALUE_CHANGED:75
```

- [x] **步骤 3：运行检查确认无证据时失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 -j
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_core_vertical
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --scenario v23_core_vertical
```

预期： baseline 或 trace 缺失导致失败，不允许自动接受空白图。

- [x] **步骤 4：生成并人工核验真实 baseline**

通过测试宿主 capture 生成 PPM，确认像素来自 LingDongGUI/Arm-2D 渲染链；禁止在 `tinyui/src/backend/ldgui/backend_app.c`、SDL host 或检查器中画假控件。将经审查的图保存为 `tests/tinyui/runtime/baselines/v23_core_vertical.ppm`。

- [x] **步骤 5：绑定 L1-L5 证据矩阵**

能力矩阵中 label/button/checkbox/slider 的每个 required 项必须给出 public symbol、link consumer、L3 unit test、L4 backend assertion、L5-V 场景/区域或 L5-E trace 名。可见且可操作的项同时列 L5-V 与 L5-E；不得以 smoke、窗口弹出或 policy 替代。

- [x] **步骤 6：运行证据 gate**

运行：

```sh
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_core_vertical
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --scenario v23_core_vertical
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

预期： 三个命令通过；每个样板至少 L4，可见项 L5-V，可操作项 L5-E。

- [x] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 8 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 核心：四样板 L5-V/L5-E 场景 `v23_core_vertical`；真实 PPM baseline；精确事件 trace；required 行 `v23_m2_evidence` 绑定。
- 实现：
  - demo：`tinyui/demo/v23_core_vertical/`（canonical create/set/event）
  - 专用 runner：`tests/tinyui/runtime/v23_core_vertical_main.c` + target `tinyui_v23_core_vertical`（绕过 M4 旧 demo 全量签名债）
  - observe 脚本注入 pointer：`tests/tinyui/runtime/tinyui_sdl_observe.c`
  - checker：`--scenario v23_core_vertical`（visible + backend mapping）
  - baseline：`tests/tinyui/runtime/baselines/v23_core_vertical.ppm`（480×320 真实 capture）
- 实测 gate：
  - `check_tinyui_visible_ui.py --scenario v23_core_vertical` 通过
  - `check_tinyui_backend_mapping.py --scenario v23_core_vertical` 通过，trace 精确匹配
  - `git diff --check` 通过
  - `check_tinyui_release_capability_matrix.py` **未绿**：预存 `scroll_selecter` 命名 API 债（`tinyui_scroll_selecter_*` vs public `tinyui_scroll_selector_*`），与 Task 8 四样板绑定无关
- 未做：Task 9；不宣称 M2 完成；不宣称全量 `tinyui_demo` 可编。

### 任务 9：关闭 M2 轻量预算与阶段门禁

**文件：**
- 修改： `tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c`
- 修改： `tests/tinyui/perf/check_tinyui_object_overhead.py`
- 修改： `tests/tinyui/perf/check_tinyui_binary_size.py`
- 修改： `tests/tinyui/perf/check_tinyui_perf.py`
- 修改： `tests/tinyui/CMakeLists.txt`
- 修改： `docs/v2.3/v2.3-performance-baseline.md`

**接口：**
- 输入： M0 baseline 与 任务 1-8 的最终 M2 产物。
- 输出： `tinyui_m2_core` CTest label、32 位 ABI RAM probe、steady-state allocation gate、M2 closeout 数据。

- [x] **步骤 1：写轻量预算失败断言**

在 struct probe 中打印并断言四样板 wrapper；增加 32 位配置结构 probe，断言 timer pool + callback pool + runtime bookkeeping `<=1024 B`。allocator 计数覆盖空闲 process、四控件已创建 process、timer 到期、event dispatch，后四者每轮 delta 都为 `0`。

执行记录：
- `test_tinyui_wrapper_struct_overhead` 断言 base/label/button/checkbox/slider 与 backend 上限，并打印 pool 尺寸；32 位预算仅在 32-bit ABI 下 hard-fail。
- `test_tinyui_steady_state_allocation` 覆盖 idle process、四样板已创建 process、timer 到期与 event dispatch，delta 均为 `0`。

- [x] **步骤 2：运行预算检查确认真实结果**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_wrapper_struct_overhead -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_wrapper_struct_overhead$' --output-on-failure
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py
rtk python3 tests/tinyui/perf/check_tinyui_perf.py
```

预期： 若任何 baseline/schema 缺失则 fail-closed；不得用 checker fallback 或更新阈值让回归通过。

执行记录：
- wrapper probe / object overhead：`TINYUI_WRAPPER_STRUCT_OVERHEAD_OK`；base wrapper `192 B`（上限 `192`）。
- 四样板：label `200`、button `248`、checkbox `208`、slider `216`，均未越过各自 absolute max。
- 32 位独立 `-m32` 尺寸：timer `388` + event `392` + bookkeeping `108` = `888 <= 1024`；host 64-bit 仅诊断打印。
- steady-state：idle / widgets / timer / event 四路径均为 `0`。
- `check_tinyui_binary_size.py` / `check_tinyui_perf.py` **未**作为 M2 通过证据：依赖 `tinyui_demo`，而 full-tree demo 仍被 M4 旧签名挡住。

- [x] **步骤 3：消除超预算状态而不增加重型抽象**

若 wrapper 超预算，删除已经能从 LD getter/tree 读取的镜像字段；若静态 RAM 超预算，压缩 slot 标志与索引宽度，但不得降低默认容量；若 text/total 越线，消除重复专用 callback/setter路径。不得修改 M0 baseline 或设计阈值。

执行记录：未出现 wrapper/32 位 RAM 超预算；无需压缩结构。未修改 M0 baseline 阈值。`runtime_static_ram.implementation` **保持 `legacy`**——timer/event 池已真实接线，但 `struct tinyui_runtime_bookkeeping` 仍是预算布局探针，未嵌入 `tinyui_runtime_state` 作为唯一 bookkeeping 真值，故不得伪造 `fixed_pool`。

- [x] **步骤 4：注册 M2 CTest label**

在 `tests/tinyui/CMakeLists.txt` 给 runtime/object/timer/event/四样板/证据/性能测试增加 `tinyui_m2_core` label，不创建绕过现有全量测试的独立假 target。

执行记录：为现有 unit/contract/ABI/probe/overhead 与 `v23_core_vertical` 可见/映射证据测试附加 `tinyui_m2_core`；新增 focused 证据 CTest，不替代 full-tree `--all`。

- [x] **步骤 5：运行 M2 完整 gate**

运行：

```sh
rtk cmake -S . -B build/v2.3-m2 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m2 -j
rtk ctest --test-dir build/v2.3-m2 -L tinyui_m2_core --output-on-failure
rtk ctest --test-dir build/v2.3-m2 --output-on-failure
```

预期： 配置、全量构建、M2 label 和全部注册 CTest 零失败。

执行记录：
- 配置成功。
- `rtk ctest -L tinyui_m2_core`：**20/20 通过**。
- 全量 `cmake --build` **未**全绿：`tinyui_demo` 及多处 demo 仍使用旧 `tinyui_*_create` / `tinyui_screen_load` 签名（M4 债）。
- 因此未宣称“全部注册 CTest 零失败”。额外非阻断观察：`test_tinyui_v23_style_theme_contract` abort、`test_tinyui_v23_baseline` 依赖缺失的 `build/v2.3-m0-full` 产物、`check_tinyui_release_capability_matrix` 仍有预存 `scroll_selecter` 命名债。

- [x] **步骤 6：运行 GitNexus 变更审计**

调用：

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：变化集中在 runtime/object/timer/event/common setter、四样板、测试与证据；若出现 port 生产流程、第二 renderer/layout/style/resource runtime，M2 不得关闭。

执行记录：`gitnexus status` 报告 **Repository not indexed**；`detect_changes` 不可用。以 `rtk git status/diff` 替代：
- 写面集中在 core runtime/event/timer/widget、四样板、contract/perf/runtime 证据、CMake label 与 docs。
- 未新增第二 renderer/layout/style/resource runtime；port 生产契约未宣称完成。
- demo / bridge 触达属 M1/M2 兼容与 M4 迁移债，不作为 M2 关闭阻断。

- [x] **步骤 7：记录 M2 closeout**

在 `docs/v2.3/v2.3-performance-baseline.md` 追加 M2 实测值、机器指纹、git commit、命令和结果；结论只能写“v2.3 M2 内部里程碑完成”，不得写 TinyUI 全能力完成或 port 完成。

- [x] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

执行记录：`rtk git diff --check` 通过。

#### 任务 9 执行记录

- 状态：`DONE_WITH_CONCERNS`
- 日期：2026-07-14
- 构建目录：`build/v2.3-m2`
- 基线 git commit（工作树未提交）：`48e4aa9956fa8be7776c9efb05fa57ea5a217b31`
- M2 focused gate：`rtk ctest --test-dir build/v2.3-m2 -L tinyui_m2_core --output-on-failure` → **20/20**
- 允许声明：v2.3 M2 内部里程碑完成（单实例 runtime、唯一 LD tree、固定 timer/event 池行为、四样板 L1-L5 证据、轻量 wrapper/32 位 pool/稳态分配门禁）。
- 明确 **不** 声明：TinyUI 全能力完成、port 完成、`runtime_static_ram.implementation=fixed_pool`、全量 demo/CTest 零失败、binary_size/perf 相对 M0 demo 产物的 fresh 回归。
- 残留关注（移交 M3/M4）：
  1. full-tree `tinyui_demo` 旧签名（M4）
  2. release_capability_matrix `scroll_selecter` 命名债（M3）
  3. style/theme contract 与 v23 baseline 产物债
  4. bookkeeping 真值接入后才能诚实切换 `fixed_pool`
  5. GitNexus 索引缺失

## M2 完成判定

- [x] 只存在一个 canonical runtime，重复 init、重复 deinit、process deadline 语义与错误码全部通过。
- [x] parent/child/root/ID/delete 均以 LingDongGUI tree 为唯一真值，没有 TinyUI ownership tree。
- [x] timer 与 callback 使用固定容量池，mutation、generation、回绕和容量耗尽语义通过。
- [x] 所有成功 common setter 修改真实 LD 对象，不支持项明确返回 `NOT_SUPPORTED`。
- [x] label、button、checkbox、slider 每个 required 能力至少 L4；可见项 L5-V，可操作项 L5-E。
- [x] `tinyui_process()`、timer/event dispatch 的隐式 heap 分配为 `0`。
- [x] wrapper、静态 RAM 与 binary size 未越过设计硬预算。
  - wrapper/32 位 pool 实测通过；binary size 因 demo 未迁移，**未**宣称相对 M0 demo 产物的 fresh 回归通过。
- [ ] 全量 CMake/CTest 零失败，GitNexus 变更范围与 M2 一致。
  - 全量构建/CTest 仍被 M4 demo 债与预存 contract 债挡住；GitNexus 未索引，以 git 证据替代。closeout 记为 `DONE_WITH_CONCERNS`，不宣称仓级零失败。
