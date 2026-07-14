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

- [ ] **步骤 1：分析高风险符号影响面**

依次调用 GitNexus MCP：

```text
impact({target: "tinyui_init", direction: "upstream"})
impact({target: "tinyui_screen_create", direction: "upstream"})
impact({target: "tinyui_app_create", direction: "upstream"})
impact({target: "tinyui_runtime_bridge_step_app", direction: "upstream"})
```

预期：`tinyui_screen_create` 为 `CRITICAL`、`tinyui_init` 为 `HIGH`；在修改前把直接调用者按 demo、unit、runtime、port integration 四类写入执行记录。M2 只迁移 core/unit 消费者，demo 全量迁移留给 M4。

- [ ] **步骤 2：写 runtime 状态失败测试**

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

将两个函数注册到本文件现有 Unity runner。

- [ ] **步骤 3：验证测试按预期失败**

运行：

```sh
rtk cmake -S . -B build/v2.3-m2 -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_SDL_DEMO=ON -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m2 --target test_tinyui_runtime_model -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_runtime_model$' --output-on-failure
```

预期： 测试失败；重复 init 当前错误地成功，或 `tinyui_process`/transition 签名尚未实现。

- [ ] **步骤 4：实现固定 runtime 状态**

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

- [ ] **步骤 5：删除 runtime 对 public app 生命周期的依赖**

在 `runtime.c` 和 `runtime_bridge.c` 中只调用内部 backend app 构造/销毁与 scene switch helper；移除 runtime 对 public `tinyui_app_run()`、`tinyui_app_set_window()`、`tinyui_app_switch_window()` 的调用。旧 app timer 的替换由 任务 3 在 `app.c` 完成。不要修改 port 的显示、tick、PFB 或 SDL 生命周期。

- [ ] **步骤 6：运行 runtime 单测**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_runtime_model test_tinyui_app_lifecycle -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(runtime_model|app_lifecycle)$' --output-on-failure
```

预期： 两个测试通过；重复 init 返回 `INVALID_STATE`，重复 deinit 无副作用，active screen 等于真实加载 root。

- [ ] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：分析对象核心影响面**

```text
impact({target: "tinyui_widget_create_leaf", direction: "upstream"})
impact({target: "tinyui_widget_destroy_common", direction: "upstream"})
impact({target: "tinyui_widget_set_text", direction: "upstream"})
impact({target: "tinyui_app_lookup_host", direction: "upstream"})
```

预期：`tinyui_widget_set_text` 为 `HIGH`。把所有控件 creator、props creator 和 text setter 列入迁移清单；本任务只改 common path，控件专用验证由 任务 5-7 完成。

- [ ] **步骤 2：写唯一树和 ID 失败测试**

在 `tests/tinyui/unit/test_tinyui_lifecycle.c` 增加：创建 root、child window、label；断言 `get_parent/first_child/next_sibling/get_root/get_child_count` 的结果来自 backend node 顺序；显式 ID 冲突时第二个 creator 返回 `NULL`，backend child count 不变；自动 ID 非零且同 root 唯一；删除 parent 后 backend subtree 与 wrapper host 绑定一起消失。

核心断言必须包含：

```c
TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_obj_get_id(label, &label_id));
TEST_ASSERT_NOT_EQUAL(0U, label_id);
TEST_ASSERT_EQUAL_PTR(label, tinyui_obj_find_by_id(root, label_id));
TEST_ASSERT_EQUAL_PTR(parent, tinyui_obj_get_parent(label));
TEST_ASSERT_EQUAL_PTR(root, tinyui_obj_get_root(label));
TEST_ASSERT_EQUAL(TINYUI_OK, tinyui_obj_get_child_count(parent, &count));
TEST_ASSERT_EQUAL_UINT16(1U, count);
```

- [ ] **步骤 3：写文本复制与失败原子性测试**

在 `tests/tinyui/unit/test_tinyui_widgets.c` 使用栈缓冲设置文本后改写原缓冲，断言 getter 和 `ldLabelGetText()` 仍是原值；注入 allocator 下一次失败，断言返回 `TINYUI_ERROR_NO_MEMORY`，旧文本和 backend 文本都不变；替换成功和 delete 后 allocator live block 回到基线。

- [ ] **步骤 4：运行测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_lifecycle test_tinyui_widgets -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(lifecycle|widgets)$' --output-on-failure
```

预期： ID 冲突、统一遍历或文本复制至少一项失败；不得因只读取 wrapper 缓存而通过。

- [ ] **步骤 5：实现 ID、host lookup 与真实树遍历**

保留 `ldBase_t.pInfo -> tinyui_obj_t` host 绑定和 `nameId` 16 位 ID；自动 ID 从 runtime 分配，显式 ID 在创建 backend 对象前检查同 root 冲突。遍历 API调用 `ldBaseGetParent()` 和 control-node child/sibling traversal，再从 `pInfo` 找 wrapper；删除 host intrusive list 中任何充当第二 ownership tree 的 parent/child/sibling 状态。

`tinyui_obj_find_by_id(root, id)` 只遍历指定真实 subtree；ID `0` 不作为可查找对象 ID。creator 失败必须释放已分配 ID，且 backend child count 不变。

- [ ] **步骤 6：实现同步删除与 callback 延迟删除钩子**

`tinyui_obj_delete()` 在非 dispatch 状态同步发送一次 `DELETE`、清除 callback slot、调用真实 LingDongGUI depose，再释放文本和 wrapper。dispatch 中删除当前 target 时只设置 `deleting`，callback 返回后执行同一销毁 helper；`deleting` 后 setter、注册和重复删除返回 `INVALID_STATE`。

- [ ] **步骤 7：实现通用 setter 与文本复制的提交顺序**

在同一 `widget.c` 中完成全部 common setter 的静态 `switch(kind)` 骨架；每个属性只允许“调用真实 LD setter”和“返回 NOT_SUPPORTED”两条路径，后续控件任务只补各自控件文件，不得再次编辑此共享文件。common text setter 的顺序为：校验对象与支持能力；用统一 allocator 创建新副本；调用控件 adapter 更新真实 backend；backend 成功后替换 wrapper 的唯一文本副本并释放旧副本；backend 失败时释放新副本并保留旧状态。不得让控件专用 setter 再调用第二次 backend setter。

- [ ] **步骤 8：运行 object 测试**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_lifecycle test_tinyui_widgets -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(lifecycle|widgets)$' --output-on-failure
```

预期： 测试通过；树查询与删除由 LD tree 证明，文本 OOM fail-closed 且无泄漏。

- [ ] **步骤 9：运行格式检查**

```sh
rtk git diff --check
```

### 任务 3：以固定池实现 timer 和 deadline

**文件：**
- 修改： `tinyui/src/core/app.c`
- 修改： `tests/tinyui/unit/test_tinyui_app_timer.c`

**接口：**
- 输入： 任务 1 的 `tinyui_process(uint32_t *next_ms)` 和 runtime clock。
- 输出： `tinyui_timer_create/start/stop/set_interval/delete`；内部 `tinyui_timer_process(now_ms, next_ms)`。

- [ ] **步骤 1：分析 timer 影响面**

```text
impact({target: "tinyui_app_pump_timers", direction: "upstream"})
impact({target: "tinyui_app_timer_create", direction: "upstream"})
```

预期：调用者集中在 app/runtime 与 timer 单测；列出旧 app timer 到 canonical timer 的替换表。

- [ ] **步骤 2：写固定池与 mutation 失败测试**

在 `tests/tinyui/unit/test_tinyui_app_timer.c` 覆盖：创建 `TINYUI_TIMER_CAPACITY` 项成功，第 `capacity + 1` 项返回 `NULL` 且 `tinyui_last_result()==CAPACITY`；删除后槽可复用；回调中 stop/delete 当前 timer；回调中创建 timer 不在同轮触发；一次性 timer 触发一次；`UINT32_MAX - 4` 到 `3` 的回绕比较正确；最近 deadline 分别返回 `0`、剩余毫秒和 `UINT32_MAX`。

回调 mutation 断言使用：

```c
static void delete_self_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = user_data;
    ++*calls;
    tinyui_timer_delete(timer);
}
```

- [ ] **步骤 3：确认旧 snapshot heap 路径导致失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_app_timer -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_app_timer$' --output-on-failure
```

预期： 固定容量、回绕或回调内删除测试失败；allocator 计数显示旧 `tinyui_app_pump_timers()` 使用 `ldCalloc`。

- [ ] **步骤 4：实现固定 timer pool**

内部槽必须为定长数组，字段固定为 `generation`、`interval_ms`、`deadline_ms`、`cb`、`user_data`、`allocated`、`running`、`repeat`、`born_epoch`、`deleting`。`tinyui_timer_t` 直接引用槽，不建立链表，不分配 snapshot。

到期判断使用有符号差值：

```c
static bool tinyui_time_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}
```

dispatch 开始递增 epoch；只处理 `born_epoch < dispatch_epoch` 的槽。回调后重新检查 `allocated/generation/running`，从而支持 stop/delete 自身。重复 timer 从当前 `now + interval` 安排下一次，不在一轮内追赶调用。

- [ ] **步骤 5：把 deadline 接入 process**

实现 任务 1 已接入 `tinyui_process(next_ms)` 的 `tinyui_timer_process` helper：无 deadline 写 `UINT32_MAX`，已到期写 `0`，否则写最小无符号剩余值。`next_ms == NULL` 合法，只忽略输出。

- [ ] **步骤 6：验证 timer 与零分配**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_app_timer test_tinyui_runtime_model -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(app_timer|runtime_model)$' --output-on-failure
```

预期： 全部通过；timer dispatch allocator delta 为 `0`，回绕和 mutation 结果确定。

- [ ] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

### 任务 4：以固定池实现统一事件与焦点

**文件：**
- 修改： `tinyui/src/core/event.c`
- 修改： `tests/tinyui/unit/test_tinyui_event.c`
- 修改： `tests/tinyui/unit/test_tinyui_button_events.c`

**接口：**
- 输入： 任务 2 的 deleting/delete helper 与 LD `SIGNAL_*` bridge。
- 输出： `tinyui_obj_add_event_cb`、`tinyui_obj_remove_event_cb`、`tinyui_focus_set/clear/move/current`、同步 `tinyui_event_t` dispatch。

- [ ] **步骤 1：分析事件与焦点影响面**

```text
impact({target: "tinyui_widget_dispatch_native_signal", direction: "upstream"})
impact({target: "tinyui_widget_claim_focus", direction: "upstream"})
impact({target: "tinyui_button_set_on_clicked", direction: "upstream"})
```

预期：旧控件专用 callback setter 与 backend signal bridge 均受影响；记录到 任务 6/7 的迁移清单。

- [ ] **步骤 2：写 callback pool、generation 与 mutation 测试**

在 `tests/tinyui/unit/test_tinyui_event.c` 覆盖默认容量 16、第 17 项返回 `CAPACITY`、槽复用后旧 handle 无法移除新 callback、注册顺序、callback 内新增不参加当前轮、移除后续 callback 立即生效、删除 target 停止后续 callback、`DELETE` 恰好一次且仅允许 getter。

必须断言 handle 编码：

```c
TEST_ASSERT_NOT_EQUAL(0U, handle);
TEST_ASSERT_NOT_EQUAL(0U, handle & UINT32_C(0xffff));
TEST_ASSERT_NOT_EQUAL(old_handle, reused_handle);
TEST_ASSERT_EQUAL(TINYUI_ERROR_INVALID_ARG,
                  tinyui_obj_remove_event_cb(button, old_handle));
```

- [ ] **步骤 3：写无冒泡与 payload 测试**

给 parent 和 child 同时注册 `CLICKED`；向 child 注入真实 LD click signal，只允许 child callback 增加。对 slider 注入真实 value signal，断言 `event.code==VALUE_CHANGED`、`event.target==slider`、`event.data.value` 为 canonical value；对 key 断言 `data.key.key/pressed`。

- [ ] **步骤 4：运行事件测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_event test_tinyui_button_events -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(event|button_events)$' --output-on-failure
```

预期： 统一 callback handle、容量或 mutation 测试失败。

- [ ] **步骤 5：实现固定 callback pool**

runtime 内保存 `TINYUI_EVENT_CB_CAPACITY` 个槽；每槽保存 object、mask、cb、user_data、generation、registration_order、allocated、born_epoch。对象只保存第一个槽索引或无效索引；不得在 wrapper 内嵌 callback 数组。

dispatch 顺序扫描固定池并按 registration_order 处理；开始时固定 epoch，新增槽的 born_epoch 等于当前 epoch，因此下一轮生效。每次调用前重新检查 allocated、generation、object 和 mask。只转换当前设计列出的八种事件，不排队、不冒泡、不 capture。

- [ ] **步骤 6：实现焦点直连与 delete 事件**

`focus_set/clear/move/current` 直接复用现有 LingDongGUI navigation；TinyUI 只保存 backend 无 getter 时必需的当前 wrapper。`DELETE` 在 deleting 标记后、LD depose 前由 `tinyui_event_emit_delete()` 同步发出；回调返回后清理该对象所有槽，再调用 任务 2 已实现的真实销毁 helper，不再次修改 `widget.c`。

- [ ] **步骤 7：验证事件、焦点和零分配**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_event test_tinyui_button_events test_tinyui_lifecycle -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(event|button_events|lifecycle)$' --output-on-failure
```

预期： 全部通过；event dispatch allocator delta 为 `0`，parent callback 不因 child 事件执行。

- [ ] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

### 任务 5：建立直接 setter adapter 并闭环 label

**文件：**
- 修改： `tinyui/src/widgets/label.c`
- 修改： `tests/tinyui/unit/test_tinyui_core_helpers.c`
- 修改： `tests/tinyui/unit/test_tinyui_label.c`

**接口：**
- 输入： M1 的 `tinyui_obj_set_*` 和 任务 2 的 text 提交顺序。
- 输出： 静态“kind -> adapter”路径；label 的 create/props/text/font/color/transparent/align/background 能力。

- [ ] **步骤 1：分析 setter 与 label 影响面**

```text
impact({target: "tinyui_widget_set_text", direction: "upstream"})
impact({target: "tinyui_widget_set_size", direction: "upstream"})
impact({target: "tinyui_label_set_text", direction: "upstream"})
```

预期：text 为 `HIGH`；先列出 label/button/checkbox/text/line_edit/qrcode 等调用者，M2 只关闭四样板，其余进入 M3。

- [ ] **步骤 2：写 adapter 选择与假成功拒绝测试**

在 `tests/tinyui/unit/test_tinyui_core_helpers.c` 创建四样板真实对象，逐项调用 position、size、visible、enabled、opacity、selectable、selected、bg/text/border color、border width、radius、padding、text。每次从对应 `ldBase_t` 或具体 `ld*` 结构读取状态；不支持组合必须返回 `TINYUI_ERROR_NOT_SUPPORTED`，wrapper 镜像不得变化。

- [ ] **步骤 3：写 label L3/L4 测试**

在 `tests/tinyui/unit/test_tinyui_label.c` 覆盖空 parent、错误 kind、越界尺寸/opacity、临时 props、props 中途失败回滚；成功后验证真实 `ldLabel_t` 的 text、font、color、align、transparent、background tile 和 node region。

- [ ] **步骤 4：运行测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_core_helpers test_tinyui_label -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(core_helpers|label)$' --output-on-failure
```

预期： 至少一个通用 setter 只更新 wrapper，或不支持能力错误地返回成功。

- [ ] **步骤 5：实现静态 adapter 路由**

复核 任务 2 已建立的编译期 `switch(kind)`，本任务仅通过 label 专用 adapter 被该 switch 调用。执行顺序固定为：完整参数预检、kind 支持预检、LD 调用、最小缓存提交。不得增加运行时注册表或通用 property database。

getter 优先调用 LD getter；确无 getter且 M1 契约要求对称 getter时，只保存该单项最后成功值。坐标、尺寸和树关系不得从 wrapper 镜像读取。

- [ ] **步骤 6：收敛 label creator 与专用 setter**

`tinyui_label_create(parent)` 直接接受统一 parent；`create_with_props` 先检查全部 presence 字段，再调用普通 creator 和正式 setter。任何 setter 失败都调用统一 delete，parent backend child count 回滚。`tinyui_label_set_text` 只转发 `tinyui_obj_set_text`，不得二次调用 `tinyui_widget_set_backend_text`。

- [ ] **步骤 7：验证 label L1-L4**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_core_helpers test_tinyui_label -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(core_helpers|label)$' --output-on-failure
```

预期： 全部通过；所有成功 setter 的 LD 状态与请求一致，所有不支持组合明确返回 `NOT_SUPPORTED`。

- [ ] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

### 任务 6：闭环 button 的真实状态与事件

**文件：**
- 修改： `tinyui/src/widgets/button.c`
- 修改： `tests/tinyui/unit/test_tinyui_button_events.c`

**接口：**
- 输入： 任务 4 的统一 callback、任务 5 的 common setter。
- 输出： button text/font/release-press color/image/transparent/checkable/key/pressed 与 `PRESSED/RELEASED/CLICKED`。

- [ ] **步骤 1：分析 button 影响面**

```text
impact({target: "tinyui_button_create", direction: "upstream"})
impact({target: "tinyui_button_set_pressed", direction: "upstream"})
impact({target: "tinyui_button_set_on_clicked", direction: "upstream"})
```

预期：旧 `set_press/set_pressed` 和专用 callback setter 有重复语义；执行记录只保留 M1 冻结的 canonical 名称。

- [ ] **步骤 2：写 button L3/L4/L5-E 失败测试**

在现有 button test 中验证每个 setter 后真实 `ldButton_t` 字段；对真实 `SIGNAL_PRESSED`、`SIGNAL_RELEASED`、click action 注入，断言统一 event code、target、user_data 和注册顺序。disabled/hidden button 不产生交互事件；旧 handle 不得影响新 callback。

- [ ] **步骤 3：运行失败测试**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_button_events -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_button_events$' --output-on-failure
```

预期： 专用 callback 字段或重复 press API 绕过统一事件池，测试失败。

- [ ] **步骤 4：实现 button 直接映射**

删除 wrapper 中 `on_clicked/on_pressed/on_released` 三组 callback 状态，专用注册 API若仍在 M1 canonical 头中，只作为 `tinyui_obj_add_event_cb` 的窄转发。pressed、checkable、颜色、图片和 key 必须调用真实 `ldButton*` 能力；backend 无法表达的组合返回 `NOT_SUPPORTED`。

- [ ] **步骤 5：验证 button L1-L4/L5-E**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_button_events test_tinyui_event -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(button_events|event)$' --output-on-failure
```

预期： 测试通过，三种用户操作事件均来自真实 LD signal/action 路径。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

### 任务 7：闭环 checkbox 与 slider 的 value 语义

**文件：**
- 修改： `tinyui/src/widgets/checkbox.c`
- 修改： `tinyui/src/widgets/slider.c`
- 修改： `tests/tinyui/unit/test_tinyui_checkbox.c`
- 修改： `tests/tinyui/unit/test_tinyui_slider.c`

**接口：**
- 输入： 任务 4 的 `VALUE_CHANGED` payload、任务 5 的 common setter。
- 输出： checkbox checked/text/check color/image/radio/spacing；slider range/value/percent/orientation/image/color/width/slim size。

- [ ] **步骤 1：分析两个 value 控件影响面**

```text
impact({target: "tinyui_checkbox_set_checked", direction: "upstream"})
impact({target: "tinyui_slider_set_value", direction: "upstream"})
impact({target: "tinyui_widget_sync_ld_value", direction: "upstream"})
```

预期：shared value helper 同时影响 switch/list 等 M3 控件；M2 不扩展到未列控件。

- [ ] **步骤 2：写 checkbox 真实状态与事件测试**

验证 checked、text、check/text color、unchecked/checked image、radio group、string spacing 直接改变 `ldCheckBox_t`；真实 toggle signal 产生一次 `VALUE_CHANGED` 且 `data.value` 为 `0/1`；programmatic setter 不伪造用户事件；props 中任何不支持字段不修改对象。

- [ ] **步骤 3：写 slider 范围映射与事件测试**

覆盖 `[-50, 150]` 与 `[7, 7]`；canonical value 与 LD percent 的双向换算必须确定，越界返回 `OUT_OF_RANGE` 且不改值。真实 LD permille 输入映射到 canonical value并产生一次 `VALUE_CHANGED`。orientation、图片、颜色、indicator width、slim size 均从 `ldSlider_t` 读取验证。

- [ ] **步骤 4：运行失败测试**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_checkbox test_tinyui_slider -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(checkbox|slider)$' --output-on-failure
```

预期： 旧 wrapper callback、先写 cache 后写 backend、或 percent/value 换算至少一项失败。

- [ ] **步骤 5：实现 checkbox 提交顺序**

所有参数和支持能力预检完成后调用 `ldCheckBox*`；成功后才更新必需缓存。checked getter 直接读取 LD 状态。旧 `cb/user_data` 从 wrapper 删除，专用 toggled 注册若保留则转发统一 event API。

- [ ] **步骤 6：实现 slider 双向值映射**

range/value 使用 64 位中间值避免溢出：

```c
percent = (int32_t)(((int64_t)(value - min_value) * 100) /
                    (max_value - min_value));
value = min_value + (int32_t)(((int64_t)(max_value - min_value) * permille) /
                              1000);
```

零范围固定映射到 min。先调用 `ldSliderSetPercent`，成功后提交 min/max/value 缓存；真实 signal 反向换算后同步触发统一事件。

- [ ] **步骤 7：验证两控件 L1-L4/L5-E**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_checkbox test_tinyui_slider test_tinyui_event -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_(checkbox|slider|event)$' --output-on-failure
```

预期： 全部通过；事件来自真实 LD 路径，setter 失败不污染缓存。

- [ ] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：定义确定性证据场景**

在 SDL observe 测试入口创建一个真实 LD screen，使用 flex 排列 label、button、checkbox、slider；场景只用 canonical `tinyui_*` API，不写固定位置补偿 backend/layout。为每个可见能力定义矩形采样区域和至少一个 RGB565 非背景像素断言；为 button click、checkbox toggle、slider change 定义按顺序的 event trace。

- [ ] **步骤 2：先写失败的证据检查**

`check_tinyui_visible_ui.py` 增加 `v23_core_vertical` 场景，要求 capture 非空、尺寸匹配、四个区域的像素差超过各自阈值。`check_tinyui_backend_mapping.py` 要求 trace 精确包含：

```text
button:PRESSED
button:RELEASED
button:CLICKED
checkbox:VALUE_CHANGED:1
slider:VALUE_CHANGED:75
```

- [ ] **步骤 3：运行检查确认无证据时失败**

运行：

```sh
rtk cmake --build build/v2.3-m2 -j
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_core_vertical
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --scenario v23_core_vertical
```

预期： baseline 或 trace 缺失导致失败，不允许自动接受空白图。

- [ ] **步骤 4：生成并人工核验真实 baseline**

通过测试宿主 capture 生成 PPM，确认像素来自 LingDongGUI/Arm-2D 渲染链；禁止在 `tinyui/src/backend/ldgui/backend_app.c`、SDL host 或检查器中画假控件。将经审查的图保存为 `tests/tinyui/runtime/baselines/v23_core_vertical.ppm`。

- [ ] **步骤 5：绑定 L1-L5 证据矩阵**

能力矩阵中 label/button/checkbox/slider 的每个 required 项必须给出 public symbol、link consumer、L3 unit test、L4 backend assertion、L5-V 场景/区域或 L5-E trace 名。可见且可操作的项同时列 L5-V 与 L5-E；不得以 smoke、窗口弹出或 policy 替代。

- [ ] **步骤 6：运行证据 gate**

运行：

```sh
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_core_vertical
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --scenario v23_core_vertical
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

预期： 三个命令通过；每个样板至少 L4，可见项 L5-V，可操作项 L5-E。

- [ ] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：写轻量预算失败断言**

在 struct probe 中打印并断言四样板 wrapper；增加 32 位配置结构 probe，断言 timer pool + callback pool + runtime bookkeeping `<=1024 B`。allocator 计数覆盖空闲 process、四控件已创建 process、timer 到期、event dispatch，后四者每轮 delta 都为 `0`。

- [ ] **步骤 2：运行预算检查确认真实结果**

运行：

```sh
rtk cmake --build build/v2.3-m2 --target test_tinyui_wrapper_struct_overhead -j
rtk ctest --test-dir build/v2.3-m2 -R '^test_tinyui_wrapper_struct_overhead$' --output-on-failure
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py
rtk python3 tests/tinyui/perf/check_tinyui_perf.py
```

预期： 若任何 baseline/schema 缺失则 fail-closed；不得用 checker fallback 或更新阈值让回归通过。

- [ ] **步骤 3：消除超预算状态而不增加重型抽象**

若 wrapper 超预算，删除已经能从 LD getter/tree 读取的镜像字段；若静态 RAM 超预算，压缩 slot 标志与索引宽度，但不得降低默认容量；若 text/total 越线，消除重复专用 callback/setter路径。不得修改 M0 baseline 或设计阈值。

- [ ] **步骤 4：注册 M2 CTest label**

在 `tests/tinyui/CMakeLists.txt` 给 runtime/object/timer/event/四样板/证据/性能测试增加 `tinyui_m2_core` label，不创建绕过现有全量测试的独立假 target。

- [ ] **步骤 5：运行 M2 完整 gate**

运行：

```sh
rtk cmake -S . -B build/v2.3-m2 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m2 -j
rtk ctest --test-dir build/v2.3-m2 -L tinyui_m2_core --output-on-failure
rtk ctest --test-dir build/v2.3-m2 --output-on-failure
```

预期： 配置、全量构建、M2 label 和全部注册 CTest 零失败。

- [ ] **步骤 6：运行 GitNexus 变更审计**

调用：

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：变化集中在 runtime/object/timer/event/common setter、四样板、测试与证据；若出现 port 生产流程、第二 renderer/layout/style/resource runtime，M2 不得关闭。

- [ ] **步骤 7：记录 M2 closeout**

在 `docs/v2.3/v2.3-performance-baseline.md` 追加 M2 实测值、机器指纹、git commit、命令和结果；结论只能写“v2.3 M2 内部里程碑完成”，不得写 TinyUI 全能力完成或 port 完成。

- [ ] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

## M2 完成判定

- [ ] 只存在一个 canonical runtime，重复 init、重复 deinit、process deadline 语义与错误码全部通过。
- [ ] parent/child/root/ID/delete 均以 LingDongGUI tree 为唯一真值，没有 TinyUI ownership tree。
- [ ] timer 与 callback 使用固定容量池，mutation、generation、回绕和容量耗尽语义通过。
- [ ] 所有成功 common setter 修改真实 LD 对象，不支持项明确返回 `NOT_SUPPORTED`。
- [ ] label、button、checkbox、slider 每个 required 能力至少 L4；可见项 L5-V，可操作项 L5-E。
- [ ] `tinyui_process()`、timer/event dispatch 的隐式 heap 分配为 `0`。
- [ ] wrapper、静态 RAM 与 binary size 未越过设计硬预算。
- [ ] 全量 CMake/CTest 零失败，GitNexus 变更范围与 M2 一致。
