# TinyUI v2.3 M1 轻量公共契约实施计划

> **供代理执行者：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，按任务逐项实施本计划。所有步骤使用复选框（`- [ ]`）跟踪。

**目标：** 在 M0 事实门禁持续通过的前提下，一次性冻结单实例 runtime、统一 object/result/event/timer/focus、轻量 style/theme、直接 layout track、调用者持有 image/font descriptor 的 canonical public API，删除 legacy、错拼和重复 ABI，并建立编译期裁剪与反重型静态门禁。

**架构：** `tinyui/include/tinyui.h` 只聚合普通用户 API，platform integration 与 native escape hatch 通过显式头分离。所有 public 控件句柄统一为不透明 `tinyui_obj_t *`，每个 creator 固定为 `create(parent)` 或 `create_with_props(parent, props)`；M1 只冻结 L1/L2 和静态轻量约束，真实 backend L3-L5 行为由 M2/M3 闭环。固定容量、descriptor 和裁剪选项只声明一种机制，不引入动态 registry、事件队列、selector、资源管理器或第二套 layout。

**技术栈：** C11 public ABI、C++17 consumer、CMake 3.16、CTest、Python 3 contract checker、LingDongGUI/Arm-2D private backend、GitNexus。

## 全局约束

- 必须先完成 M0；M0 的全量构建、inventory、递归 public header、逐符号链接、尺寸、分配、最小产物和性能门禁在每个任务后保持绿色。
- TinyUI 只参考 LVGL 的 API 组织，不实现 LVGL ABI、class runtime、property reflection、事件冒泡/capture、动态事件队列、style selector/级联、资源缓存或 renderer plugin。
- `LingDongGUI tree` 是唯一对象树真值；public contract 不出现第二份 parent/child/sibling ownership 数据结构。
- 所有 canonical API 只能在 UI 线程调用；callback 在 `tinyui_process()` 所在线程同步执行；ISR 和跨线程投递属于延期 port 工作。
- 本阶段不创建 worktree，不修改 port 生产实现，不把 SDL smoke 写成 backend 或 port 完成证据。
- 修改任何函数前先调用 GitNexus `impact({target: "符号名", direction: "upstream"})`；风险等级以当次 fresh 结果为准，结果为 `HIGH/CRITICAL` 时必须在修改前形成完整调用者迁移清单并告警。
- 每个 shell 命令都以 `rtk` 开头；使用 CMake 构建。
- M1 允许破坏 v2.2 ABI；旧名不进入 canonical manifest、安装树、最小 profile 或普通 `tinyui.h` 路径。为保证尚待 M4 迁移的仓内 demo/旧测试可构建，只允许私有 `temporary migration bridge`，并由 M4 删除。
- 新 public 函数必须在同一任务提供可链接定义；未到 M2 的 required 行为只能明确返回 `TINYUI_ERROR_NOT_SUPPORTED` 或 `NULL`，不得返回假成功。
- 每项任务按“失败 contract 测试 → 最小声明/链接实现 → 窄门禁 → `rtk git diff --check`”执行；实际提交不属于本计划。

---

## 文件结构

- `tinyui/include/core/result.h`：唯一结果码。
- `tinyui/include/core/obj.h`：不透明对象、ID、树查询、通用属性与删除。
- `tinyui/include/core/runtime.h`：单实例 init/deinit、screen/background、load/active/process。
- `tinyui/include/core/event.h`：固定事件码、payload、mask、callback handle。
- `tinyui/include/core/timer.h`：固定池 timer public contract。
- `tinyui/include/core/focus.h`：焦点方向与焦点操作。
- `tinyui/include/style/style.h`：值语义 style、固定 part/state。
- `tinyui/include/theme/theme.h`：调用者持有的固定 theme descriptor。
- `tinyui/include/layout/layout.h`：flex 与有类型 grid track。
- `tinyui/include/resource/image_source.h`：RGB565/builtin/VRES 借用 descriptor。
- `tinyui/include/resource/font.h`：builtin/VRES font descriptor。
- `tinyui/include/integration/input.h`：显式 key input 集成头，不由 widget 重复声明。
- `tinyui/include/extensions/ldgui_native.h`：可关闭的高级 native escape hatch，不由 `tinyui.h` 聚合。
- `tinyui/include/internal/v22_demo_bridge.h`：仅在私有宏开启时供仓内旧 demo/测试迁移，禁止安装和能力声明。
- `tinyui/src/compat/v22_demo_bridge.c`：只转发 canonical/backend 真实行为的临时迁移实现，M4 删除。
- `tinyui/include/widgets/*.h`：全部控件统一 `tinyui_obj_t *`、`create(parent)` 与 presence-bit props。
- `tinyui/include/tinyui.h`：只聚合 canonical 用户层。
- `tinyui/src/core/*.c`、`tinyui/src/theme/theme.c`、`tinyui/src/layout/*.c`、`tinyui/src/widgets/*.c`：提供 M1 可链接定义并隐藏 legacy public symbol。
- `cmake/LingDongGUI.cmake`：每个控件族及 optional 模块的编译期选项与 source gating。
- `cmake/LingDongGUI.cmake` 中的 `TINYUI_PROFILE`：`minimal` 是最小 profile 的唯一确定性配置入口。
- `tests/tinyui/contract/tinyui_v23_public_api.json`：M1 冻结的 public 类型、函数、宏和归属头 manifest。
- `tests/tinyui/contract/check_tinyui_v23_public_api.py`：声明、签名、聚合边界和旧名消失门禁。
- `tests/tinyui/contract/check_tinyui_lightweight_architecture.py`：反重型概念与禁止分配路径静态门禁。
- `tests/tinyui/contract/check_tinyui_feature_options.py`：source gating、宏值和最小 profile 门禁。
- `tests/tinyui/abi/test_tinyui_public_abi.c`：host ABI、descriptor 和固定容量静态断言。

## 任务 1：建立风险清单和 M1 contract manifest

**文件：**

- 创建：`tests/tinyui/contract/tinyui_v23_public_api.json`
- 创建：`tests/tinyui/contract/check_tinyui_v23_public_api.py`
- 创建：`tests/tinyui/contract/test_check_tinyui_v23_public_api.py`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 消费：M0 生成的 public header/function manifest 与设计文档第 9-17 章。
- 产出：schema `tinyui-v2.3-public-api-v1`，每项含 `kind`、`name`、`header`、`signature`、`availability`。

- [x] **步骤 1：对高风险入口执行 upstream impact**

依次调用 GitNexus：

```text
impact({target: "tinyui_screen_create", direction: "upstream"})
impact({target: "tinyui_init", direction: "upstream"})
impact({target: "tinyui_widget_set_text", direction: "upstream"})
impact({target: "tinyui_theme_apply_to_widget", direction: "upstream"})
```

把 direct callers、execution flows 和 fresh 风险等级写入本阶段执行记录。若结果为 `CRITICAL/HIGH`，调用者清单至少覆盖 `tinyui/demo/**`、`tests/tinyui/**`、`tinyui/port/**` 和 core internal。M1 只迁移 core/test contract，demo 全量迁移在 M4；仓内旧消费者通过受限迁移桥保持默认构建，不得继续暴露为普通用户 API。

- [x] **步骤 2：写 manifest checker 的失败测试**

fixture 分别删除 `tinyui_process`、篡改 `tinyui_event_cb_t`、把 `core/app.h` 加入 `tinyui.h`，checker 必须输出 `missing_symbol`、`signature_mismatch`、`forbidden_aggregate_include`。

运行：

```bash
rtk python3 tests/tinyui/contract/test_check_tinyui_v23_public_api.py -v
```

预期：失败，提示 checker 不存在。

- [x] **步骤 3：实现结构化 manifest checker**

checker 复用 M0 parser，不用正则猜完整 C 语法；`availability` 只允许 `always` 或一个精确 `TINYUI_ENABLE_*` 宏。它必须验证 manifest 与实时 public header 双向相等，并拒绝未登记 public 函数。

- [x] **步骤 4：登记 M1 精确 public 集合并注册 CTest**

先登记本计划任务 2-6 的全部类型、宏和函数；此时运行应为红，证明 legacy header 不能满足新 manifest。

```bash
rtk cmake -S . -B build/v2.3-m1-contract -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none
rtk ctest --test-dir build/v2.3-m1-contract --output-on-failure -R '^check_tinyui_v23_public_api$'
```

预期：失败并列出尚未实现的 canonical 声明与仍存在的 legacy 声明。

执行记录：fresh upstream impact 中 `tinyui_screen_create` 为 HIGH，29 个直接消费者；`tinyui_init` 为 MEDIUM，2 个直接消费者；`tinyui_widget_set_text` 为 MEDIUM，5 个直接消费者；`tinyui_theme_apply_to_widget` 为 LOW，无直接消费者。checker 的 TDD 单测为 10/10 通过；CTest `check_tinyui_v23_public_api` 按预期失败，报告了缺失 canonical 声明、legacy/native/错拼符号、重复符号和非法聚合头。

- [x] **步骤 5：运行格式检查**

```bash
rtk git diff --check
```

任务 1 评审记录：独立 subagent 已完成评审并在同一写集内补齐嵌套函数参数解析、重复符号检测、manifest 哈希漂移检查及对应测试；Python 单测 10/10 通过，`rtk git diff --check` 通过。

## 任务 2：冻结 result、object 与 canonical runtime

**文件：**

- 创建：`tinyui/include/core/result.h`
- 重写：`tinyui/include/core/obj.h`
- 重写：`tinyui/include/core/runtime.h`
- 修改：`tinyui/src/core/runtime.c`
- 修改：`tinyui/src/core/widget.c`
- 修改：`tinyui/src/core/internal.h`
- 测试：`tests/tinyui/contract/test_tinyui_v23_runtime_contract.c`
- 测试：`tests/tinyui/unit/test_tinyui_runtime_model.c`

**接口：**

- 产出：`tinyui_result_t`、不透明 `tinyui_obj_t`、单实例 runtime、root 创建/加载/查询、对象 ID/树查询/删除。

- [x] **步骤 1：写 canonical runtime 编译与链接测试**

测试只能 include `core/runtime.h` 和 `core/obj.h`，必须编译以下调用且不出现 `struct tinyui_app`、`struct tinyui_window` 或 `struct tinyui_widget`：

```c
uint32_t next_ms = UINT32_MAX;
assert(tinyui_init() == TINYUI_OK);
tinyui_obj_t *screen = tinyui_screen_create();
assert(screen != NULL);
assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
assert(tinyui_screen_active() == screen);
assert(tinyui_process(&next_ms) == TINYUI_OK);
tinyui_deinit();
```

运行：

```bash
rtk cmake --build build/v2.3-m1-contract --target test_tinyui_v23_runtime_contract -j
```

预期：因结果类型、transition 参数和 `tinyui_process` 缺失而失败。

- [x] **步骤 2：定义唯一结果码**

`tinyui/include/core/result.h` 的完整枚举固定为：

```c
typedef enum tinyui_result {
    TINYUI_OK = 0,
    TINYUI_ERROR_INVALID_ARG,
    TINYUI_ERROR_INVALID_OBJECT,
    TINYUI_ERROR_INVALID_STATE,
    TINYUI_ERROR_NOT_SUPPORTED,
    TINYUI_ERROR_OUT_OF_RANGE,
    TINYUI_ERROR_NO_MEMORY,
    TINYUI_ERROR_CAPACITY,
    TINYUI_ERROR_BACKEND,
} tinyui_result_t;
```

所有收窄 getter 使用 `tinyui_result_t + out parameter`；creator 仍返回指针或 `NULL`。

- [x] **步骤 3：定义不透明对象与树查询**

`core/obj.h` 只前置声明 `typedef struct tinyui_obj tinyui_obj_t;`，并声明：

```c
tinyui_result_t tinyui_obj_delete(tinyui_obj_t *obj);
tinyui_result_t tinyui_obj_get_id(const tinyui_obj_t *obj, uint16_t *id);
tinyui_obj_t *tinyui_obj_find_by_id(tinyui_obj_t *root, uint16_t id);
tinyui_obj_t *tinyui_obj_get_parent(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_first_child(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_next_sibling(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_root(const tinyui_obj_t *obj);
tinyui_result_t tinyui_obj_get_child_count(const tinyui_obj_t *obj, uint16_t *count);
```

通用 setter/getter 统一使用 `tinyui_obj_set_*`/`tinyui_obj_get_*`；至少覆盖 position、size、text、bg/text/border color、border width、radius、padding、visible、enabled、opacity、selectable、selected、focusable 和 layout participation。不得公开字符串 `style_class` 或用户可修改 ID setter。

- [x] **步骤 4：定义完整 runtime transition 与 process**

`tinyui_screen_transition_t` 必须逐项包含设计文档列出的 `NONE`、两种 fade、六种 slide/erase、四种 fly-in，共 15 项。runtime 函数固定为：

```c
tinyui_result_t tinyui_init(void);
void tinyui_deinit(void);
tinyui_obj_t *tinyui_screen_create(void);
tinyui_obj_t *tinyui_screen_create_with_props(const tinyui_window_props_t *props);
tinyui_obj_t *tinyui_background_create(void);
tinyui_obj_t *tinyui_background_create_with_props(
    const tinyui_background_props_t *props);
tinyui_result_t tinyui_screen_load(tinyui_obj_t *screen,
                                   tinyui_screen_transition_t transition,
                                   uint32_t duration_ms);
tinyui_obj_t *tinyui_screen_active(void);
tinyui_result_t tinyui_process(uint32_t *next_ms);
```

M1 定义只转发现有 runtime 可表达路径；无法表达的非 `NONE` transition 返回 `TINYUI_ERROR_NOT_SUPPORTED`，不得模拟动画。`next_ms=UINT32_MAX` 表示无 core deadline。`tinyui_background_create_with_props()` 必须提供 canonical typedef、声明、可链接定义和 contract 覆盖；当前 backend 无法表达该 props 映射时返回 `NULL`，不得伪造成功。旧 app/id creator 已改名为非 canonical 头路径的临时迁移入口。

- [x] **步骤 5：迁移 M0 分配 probe 到 `tinyui_process`**

把 `tests/tinyui/unit/test_tinyui_steady_state_allocation.c` 的 100 轮调用改为 `tinyui_process(&next_ms)`，继续断言零隐式 heap 分配。

运行：

```bash
rtk cmake --build build/v2.3-m1-contract --target test_tinyui_v23_runtime_contract test_tinyui_runtime_model test_tinyui_steady_state_allocation -j
rtk ctest --test-dir build/v2.3-m1-contract --output-on-failure -R 'test_tinyui_v23_runtime_contract|test_tinyui_runtime_model|test_tinyui_steady_state_allocation'
```

预期：三项通过；M0 wrapper 尺寸门禁仍通过。本轮窄 CTest 三项均通过，steady probe 输出零隐式分配。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

任务 2 执行记录：canonical contract、runtime model、steady allocation 三个目标使用 CMake 窄构建并通过 CTest `3/3`；覆盖唯一 result、opaque object、15 项 transition、screen/background creator（含 `background_create_with_props` 的可链接 NULL 路径）、`screen_load/process`、对象树查询和 100 轮 `tinyui_process` 零分配。未启动全量构建；`rtk git diff --check` 通过。

## 任务 3：冻结轻量 event、timer 与 focus 契约

**文件：**

- 创建：`tinyui/include/core/event.h`
- 创建：`tinyui/include/core/timer.h`
- 创建：`tinyui/include/core/focus.h`
- 创建：`tinyui/include/integration/input.h`
- 修改：`tinyui/src/core/event.c`
- 修改：`tinyui/src/core/runtime.c`
- 修改：`tinyui/src/core/internal.h`
- 测试：`tests/tinyui/contract/test_tinyui_v23_event_timer_focus_contract.c`
- 测试：`tests/tinyui/abi/test_tinyui_public_abi.c`

**接口：**

- 产出：同步事件 callback、32 位 generation handle、默认 16 项 event/timer 固定容量、焦点和抽象 key。

- [x] **步骤 1：写类型、签名与常量失败测试**

测试 `_Static_assert(TINYUI_EVENT_CB_CAPACITY == 16)`、`_Static_assert(TINYUI_TIMER_CAPACITY == 16)`、`_Static_assert(sizeof(tinyui_event_handle_t) == 4)`，并取地址验证所有函数可链接。

- [x] **步骤 2：定义事件值类型**

`event.h` 必须包含设计中的 8 个事件码、`tinyui_key_event_data_t`、`tinyui_event_t`、`tinyui_event_cb_t`、`tinyui_event_handle_t`、`TINYUI_EVENT_MASK(code)` 和 `TINYUI_EVENT_MASK_ALL`，函数固定为：

```c
tinyui_result_t tinyui_obj_add_event_cb(tinyui_obj_t *obj,
                                        uint32_t event_mask,
                                        tinyui_event_cb_t cb,
                                        void *user_data,
                                        tinyui_event_handle_t *handle);
tinyui_result_t tinyui_obj_remove_event_cb(tinyui_obj_t *obj,
                                           tinyui_event_handle_t handle);
```

public header 不声明 bubble、capture、priority、queue 或 async dispatch。

- [x] **步骤 3：定义 timer 固定池契约**

```c
#ifndef TINYUI_TIMER_CAPACITY
#define TINYUI_TIMER_CAPACITY 16
#endif

typedef struct tinyui_timer tinyui_timer_t;
typedef void (*tinyui_timer_cb_t)(tinyui_timer_t *timer, void *user_data);
tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms, bool repeat,
                                    tinyui_timer_cb_t cb, void *user_data);
tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer, uint32_t interval_ms);
void tinyui_timer_delete(tinyui_timer_t *timer);
```

M1 只固定池容量和可链接入口；回绕、callback mutation 与删除语义由 M2 TDD 实现。

- [x] **步骤 4：定义 focus 和显式 key integration**

`focus.h` 包含 `NEXT/PREVIOUS/LEFT/RIGHT/UP/DOWN` 以及 `tinyui_focus_set/clear/move/current`。`integration/input.h` 包含 8 个 `tinyui_key_t` 和 `tinyui_input_send_key(tinyui_key_t key, bool pressed)`；该头不由任何 widget header 重复声明。

- [x] **步骤 5：提供 fail-closed 最小链接实现**

现有 backend 已能直接表达的 focus/key 转发真实实现；固定池尚未在 M2 完成的 event/timer 操作必须返回 `TINYUI_ERROR_NOT_SUPPORTED` 或 creator `NULL`，同时 `tinyui_last_result()` 可观察对应错误。不得动态分配 callback/timer 节点绕过固定池契约。

运行：

```bash
rtk cmake --build build/v2.3-m1-contract --target test_tinyui_v23_event_timer_focus_contract test_tinyui_public_abi -j
rtk ctest --test-dir build/v2.3-m1-contract --output-on-failure -R 'test_tinyui_v23_event_timer_focus_contract|test_tinyui_public_abi'
```

预期：声明、常量和链接通过；没有 event queue 或动态 timer list 新增到 wrapper。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

任务 3 执行记录：先以缺失 `core/event.h` 的窄构建失败确认红测；随后新增 event/timer/focus/input 四个公共头，补充 `tinyui_last_result()`，并在不改变旧 focus 内部调用签名的前提下复用现有 backend focus/key 路径。event callback 与 timer 入口保持 fail-closed：分别返回 `TINYUI_ERROR_NOT_SUPPORTED` 或 `NULL`，不创建动态 callback/timer 节点。`build/v2.3-m1-contract` 窄构建通过；`test_tinyui_v23_event_timer_focus_contract` 与 `test_tinyui_public_abi` CTest `2/2` 通过；manifest checker 单测通过，整体 checker 仍因 legacy/聚合头及后续任务未迁移 API 按计划失败；`rtk git diff --check` 通过。

## 任务 4：冻结 style/theme 与 diagnostics 值契约

**文件：**

- 创建：`tinyui/include/style/style.h`
- 重写：`tinyui/include/theme/theme.h`
- 修改：`tinyui/include/core/result.h`
- 修改：`tinyui/src/theme/theme.c`
- 修改：`tinyui/src/core/runtime.c`
- 测试：`tests/tinyui/contract/test_tinyui_v23_style_theme_contract.c`

**接口：**

- 产出：固定 `tinyui_style_t`、`tinyui_part_t`、`tinyui_state_t`、调用者持有 `tinyui_theme_t`、可关闭 diagnostics。

- [x] **步骤 1：写 style 值语义失败测试**

测试在栈上构造 `tinyui_style_t`，调用 `tinyui_obj_apply_style` 后销毁局部变量；public ABI test 断言 style 中没有 class 名、链表节点或 selector 指针。

- [x] **步骤 2：定义固定 style descriptor**

使用设计文档的 8 位 presence mask 字段：bg/text/border color、border width、radius、padding、opacity、font；固定 part 为 `MAIN/TEXT/INDICATOR/KNOB/TRACK`，state 为 `DEFAULT/DISABLED/PRESSED/CHECKED/FOCUSED`。唯一入口为：

```c
tinyui_result_t tinyui_obj_apply_style(tinyui_obj_t *obj,
                                       tinyui_part_t part,
                                       tinyui_state_t state,
                                       const tinyui_style_t *style);
```

M1 的实现先完整校验 fields、part 和 state；backend 未闭环项返回 `NOT_SUPPORTED`，不得保存 `style` 指针。

- [x] **步骤 3：把 theme 改为调用者持有值 descriptor**

```c
typedef struct tinyui_theme {
    uint32_t colors[TINYUI_COLOR_COUNT];
    int16_t metrics[TINYUI_METRIC_COUNT];
} tinyui_theme_t;

tinyui_result_t tinyui_theme_set(const tinyui_theme_t *theme);
const tinyui_theme_t *tinyui_theme_get(void);
tinyui_result_t tinyui_theme_apply(tinyui_obj_t *obj);
```

删除 public `tinyui_theme_create/destroy`、`tinyui_app_set_theme` 和 `tinyui_theme_apply_to_widget`；theme pointer 生命周期由调用者负责，不自动遍历对象树。

- [x] **步骤 4：定义可关闭 diagnostics**

`result.h` 在 `TINYUI_ENABLE_DIAGNOSTICS` 开启时声明 `tinyui_last_result()` 与 `tinyui_last_error_message()`；关闭时仍保留静态结果名，不保存动态字符串。固定诊断 buffer 属于 runtime bookkeeping，纳入 1024 B 静态 RAM 合计。

运行：

```bash
rtk cmake --build build/v2.3-m1-contract --target test_tinyui_v23_style_theme_contract -j
rtk ctest --test-dir build/v2.3-m1-contract --output-on-failure -R '^test_tinyui_v23_style_theme_contract$'
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py --probe build/v2.3-m1-contract/tests/tinyui/test_tinyui_wrapper_struct_overhead --baseline tests/tinyui/perf/tinyui_perf_baseline.json
```

预期：contract 通过；每对象 wrapper 没有 style 表或 callback 数组增长。

- [x] **步骤 5：运行格式检查**

```bash
rtk git diff --check
```

任务 4 执行记录：style contract 先以缺失公共头形成红测，随后冻结 8 位 fields、固定 part/state 和调用者持有 descriptor；`tinyui_obj_apply_style()` 完整校验后对未闭环 backend 返回 `TINYUI_ERROR_NOT_SUPPORTED`，不保存 style 指针。theme 提供值数组 descriptor 的 set/get/apply，legacy theme API 不进入 canonical manifest；diagnostics 保留静态结果码文本并按开关声明错误消息。独立 reviewer 修复了 public `struct tinyui_theme` 与内部旧布局同名冲突，改为 `struct tinyui_legacy_theme`。窄构建及 CTest `2/2` 通过，manifest 中 theme 专项错误为 0，checker 单测 `10/10` 通过，`rtk git diff --check` 通过。

## 任务 5：冻结 flex/grid 与 image/font 轻量 descriptor

**文件：**

- 重写：`tinyui/include/layout/layout.h`
- 创建：`tinyui/include/resource/image_source.h`
- 创建：`tinyui/include/resource/font.h`
- 修改：`tinyui/include/widgets/image.h`
- 修改：`tinyui/src/layout/flex.c`
- 修改：`tinyui/src/layout/grid.c`
- 修改：`tinyui/src/core/resource.c`
- 测试：`tests/tinyui/contract/test_tinyui_v23_layout_resource_contract.c`
- 测试：`tests/tinyui/abi/test_tinyui_public_abi.c`

**接口：**

- 产出：以 `tinyui_obj_t *` 为 container 的 flex、显式 count grid track、RGB565/builtin/VRES image、builtin/VRES font。

- [x] **步骤 1：写有类型 grid 和资源 ABI 失败测试**

测试拒绝负数 sentinel，要求 `TINYUI_GRID_MAX_TRACKS == 16`；32 位 ABI 条件下静态断言 `sizeof(tinyui_image_source_t) <= 80`、`sizeof(tinyui_font_t) <= 16`。host probe 还要断言两块 image private storage 各至少 `6 * sizeof(uintptr_t)`。

- [x] **步骤 2：定义直接映射 layout contract**

flex 的 flow、main/cross/track align、item/track gap 全部接收 `tinyui_obj_t *container`；child grow/min/max/new track/ignore layout 使用 `tinyui_obj_set_*`。grid 固定为：

```c
#define TINYUI_GRID_MAX_TRACKS 16
typedef enum tinyui_grid_unit {
    TINYUI_GRID_UNIT_PX,
    TINYUI_GRID_UNIT_FR,
    TINYUI_GRID_UNIT_CONTENT,
} tinyui_grid_unit_t;
typedef struct tinyui_grid_track {
    tinyui_grid_unit_t unit;
    uint16_t value;
} tinyui_grid_track_t;
tinyui_result_t tinyui_grid_set_columns(tinyui_obj_t *container,
                                        const tinyui_grid_track_t *tracks,
                                        uint8_t count);
tinyui_result_t tinyui_grid_set_rows(tinyui_obj_t *container,
                                     const tinyui_grid_track_t *tracks,
                                     uint8_t count);
```

PX 只允许 `1..32767`，FR 只允许 `1..255`，CONTENT 的 value 必须为 `0`；M1 只做参数转换并调用现有 LingDongGUI layout，不计算几何。

- [x] **步骤 3：定义 image source 借用 descriptor**

`tinyui_image_source_kind_t` 固定 `EMPTY/RGB565_MEMORY/BUILTIN/VRES`；descriptor 字段严格使用设计中的 width/height/stride/pixels/mask/mask_stride、`uintptr_t _image_private[6]` 和 `_mask_private[6]`。函数固定为 `from_rgb565`、`from_builtin`、`from_vres`、`deinit`；删除 public `void *img_tile`、`void *mask_tile` 和 `destroy` 命名。

- [x] **步骤 4：定义 font 借用 descriptor**

builtin 固定 `6X8/16X24/ARIAL_12/ARIAL_16_A8`，kind 固定 `BUILTIN/VRES`，private storage 为 `uintptr_t _private[2]`。函数固定为 `tinyui_font_from_builtin`、`tinyui_font_from_vres`、`tinyui_font_deinit`。public header 不出现 Arm-2D font、glyph callback 或 cache。

- [x] **步骤 5：增加 internal tile storage ABI probe**

在只获得 private Arm-2D include 的 internal probe 中断言：

```c
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_image_private) >= sizeof(arm_2d_tile_t),
               "image private storage is too small for arm_2d_tile_t");
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_mask_private) >= sizeof(arm_2d_tile_t),
               "mask private storage is too small for arm_2d_tile_t");
```

失败时必须增加明确的固定 storage 或缩小 backend view；禁止改成 heap。

运行：

```bash
rtk cmake --build build/v2.3-m1-contract --target test_tinyui_v23_layout_resource_contract test_tinyui_public_abi -j
rtk ctest --test-dir build/v2.3-m1-contract --output-on-failure -R 'test_tinyui_v23_layout_resource_contract|test_tinyui_public_abi'
```

预期：public descriptor 不泄漏 native 类型，layout/resource 声明与 L2 链接通过。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

任务 5 执行记录：grid 使用显式 typed track、count 和 16 项固定上限，flex/grid 均通过不透明 container 映射真实 `ldWindow_t`。image descriptor 仅保留 RGB565/builtin/VRES 字段及两块 `uintptr_t[6]` private storage；Arm-2D tile 通过 `internal.h` helper 从 private storage 读取，所有当前 widget 资源消费者已迁移，public header 不再包含 `void *`、`img_tile` 或 `mask_tile`。font 保留 Task7 迁移边界内的旧 widget 兼容分支，未扩大 legacy aggregate 迁移。新增 checker fixture 覆盖 image public header 禁止字段。窄构建目标通过，CTest `2/2` 通过，checker 单测 `11/11` 通过，`rtk git diff --check` 通过；未执行全量构建，未开始任务 6。

## 任务 6：统一全部控件 creator、props 和对象参数

**文件：**

- 修改：`tinyui/include/widgets/animation.h`
- 修改：`tinyui/include/widgets/arc.h`
- 修改：`tinyui/include/widgets/background.h`
- 修改：`tinyui/include/widgets/button.h`
- 修改：`tinyui/include/widgets/calendar.h`
- 修改：`tinyui/include/widgets/canvas.h`
- 修改：`tinyui/include/widgets/checkbox.h`
- 修改：`tinyui/include/widgets/clock.h`
- 修改：`tinyui/include/widgets/combo_box.h`
- 修改：`tinyui/include/widgets/date_time.h`
- 修改：`tinyui/include/widgets/gauge.h`
- 修改：`tinyui/include/widgets/graph.h`
- 修改：`tinyui/include/widgets/icon_slider.h`
- 修改：`tinyui/include/widgets/image.h`
- 修改：`tinyui/include/widgets/keyboard.h`
- 修改：`tinyui/include/widgets/label.h`
- 修改：`tinyui/include/widgets/line_edit.h`
- 修改：`tinyui/include/widgets/list.h`
- 修改：`tinyui/include/widgets/message_box.h`
- 修改：`tinyui/include/widgets/progress_bar.h`
- 修改：`tinyui/include/widgets/progress_wheel.h`
- 修改：`tinyui/include/widgets/qrcode.h`
- 修改：`tinyui/include/widgets/radial_menu.h`
- 重命名：`tinyui/include/widgets/scroll_selecter.h` → `tinyui/include/widgets/scroll_selector.h`
- 修改：`tinyui/include/widgets/slider.h`
- 修改：`tinyui/include/widgets/switch.h`
- 修改：`tinyui/include/widgets/table.h`
- 修改：`tinyui/include/widgets/text.h`
- 修改：`tinyui/include/widgets/window.h`
- 修改：对应 `tinyui/src/widgets/*.c`
- 测试：`tests/tinyui/contract/check_tinyui_widget_creator_contract.py`

**接口：**

- 产出：每个控件恰好一个 `tinyui_<widget>_create(tinyui_obj_t *parent)` 和一个 `tinyui_<widget>_create_with_props(tinyui_obj_t *parent, const tinyui_<widget>_props_t *props)`。

- [x] **步骤 1：写全控件 creator manifest 失败测试**

checker 从启用控件清单生成预期签名，拒绝返回 `struct tinyui_<widget> *`、parent 为 `struct tinyui_window *`、额外字符串 ID 参数、`*_init` creator 和缺失 presence mask 的 props。

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_widget_creator_contract.py
```

预期：列出当前全部非 canonical creator。

- [x] **步骤 2：给每种 props 增加显式 presence mask**

每种 props 第一组字段固定包含 `uint32_t fields` 与 `uint16_t id`；`id=0` 表示 runtime 自动分配，但是否提供 id 仍由对应 presence bit 决定。颜色值 0、坐标 0、NULL resource 不再被当作“未设置”。普通 creator 等价于 `props=NULL`。

- [x] **步骤 3：统一全部 public 对象参数**

所有 widget-specific setter/getter 的对象参数改为 `tinyui_obj_t *` 或 `const tinyui_obj_t *`；实现入口先校验 internal kind，再转换为 concrete wrapper。public header 不再前置声明 concrete widget struct。

- [x] **步骤 4：保证 props creator 只复用正式 setter**

每个 `create_with_props` 必须先创建真实对象，再按固定字段顺序调用 canonical setter；任一 setter 失败时删除 backend 对象和 wrapper，返回 `NULL`，parent 不留下 child。M1 只要求链接与失败清理路径，真实 setter L4 映射在 M2/M3 验证。

- [x] **步骤 5：运行全控件 public contract 与逐符号链接**

```bash
rtk python3 tests/tinyui/contract/check_tinyui_widget_creator_contract.py
rtk cmake -S . -B build/v2.3-m1-widgets -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m1-widgets --target tinyui_public_header_probes tinyui_public_symbol_link_probes -j
```

预期：所有启用控件使用统一 object API；每个声明独立 C/C++ 编译并链接。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

任务 6 执行记录：creator/props/`tinyui_obj_t *` 统一 + presence-mask 门控已落地（checker 含 fields/anti-pattern/void-props/half-size/kind-fail 扫描，TDD 红→绿）。质量修复：setter kind 失败返回 -1；line_edit as_ 做 kind 校验；create_with_props 对有 setter 的字段禁止 `(void)props->`；WIDTH/HEIGHT 半边保留当前尺寸；table rows/cols 与 graph series_max 进入 create 路径；qrcode 去掉 present_mask。`tinyui_core` 与 creator contract 通过。scroll_selecter.c 改名/demo/aggregate 仍属任务 7；composite 半字段缺省读 host 当前值（radial/icon/clock/calendar/date_time/gauge），禁止 0/1970/0.5f 硬编码。

## 任务 7：移出 canonical legacy、错拼、重复 ABI 并建立受限迁移桥

**文件：**

- 从 canonical 边界移除：`tinyui/include/core/app.h`
- 从 canonical 边界移除：`tinyui/include/core/widget.h`
- 从 canonical 边界移除：`tinyui/include/core/native.h`
- 新增：`tinyui/include/internal/v22_demo_bridge.h`
- 新增：`tinyui/src/compat/v22_demo_bridge.c`
- 创建：`tinyui/include/extensions/ldgui_native.h`
- 重写：`tinyui/include/tinyui.h`
- 重命名：`tinyui/src/widgets/scroll_selecter.c` → `tinyui/src/widgets/scroll_selector.c`
- 修改：`tinyui/src/core/app.c`
- 修改：`tinyui/src/core/native.c`
- 修改：`tinyui/src/core/internal.h`
- 创建：`tests/tinyui/contract/check_tinyui_removed_api.py`
- 修改：`tests/tinyui/contract/check_tinyui_public_api.py`
- 修改：`cmake/LingDongGUI.cmake`

**接口：**

- 产出：普通用户聚合头、安装树、最小 profile 和能力声明只含 canonical API；legacy/错拼入口只允许存在于显式私有迁移桥，并在 M4 删除。

- [x] **步骤 1：写 removed API 红色清单**

checker 必须拒绝 canonical manifest、默认预处理的 public header、安装树或 minimal archive 中出现：`tinyui_app_*`、`tinyui_timer_handler`、`tinyui_widget_*`、`tinyui_tabel_*`、`tinyui_scroll_selecter_*`、`tinyui_q_r_code_*`、`tinyui_button_set_press`、`tinyui_button_get_press`、`tinyui_theme_create/destroy`、`tinyui_theme_apply_to_widget`、`tinyui_app_set_theme`、`tinyui_*_init`（唯一例外 `tinyui_init`）和 `tinyui_widget_set_style_class`。M1-M3 的默认 full archive 因仓内迁移桥可临时保留受控 legacy symbol，checker 必须验证这些符号只来自 `v22_demo_bridge.c`。

- [x] **步骤 2：把 app/widget/native legacy 移入私有迁移边界**

把 backend 尚需的 concrete struct、app helper 和 native converter 移入 `tinyui/src/core/internal.h` 或按职责拆分的 private 头。`app.c` 中仍需的内部函数改名为 `tinyui_runtime_internal_*` 并设为 private visibility。仓内旧 demo/测试所需入口集中放入 `internal/v22_demo_bridge.h` 与 `src/compat/v22_demo_bridge.c`，只在 `TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE=1` 时编译；该宏只能以 PRIVATE 方式赋给仓内旧消费者。

bridge 每个函数必须转发 canonical API 或现有真实 backend 实现，错误原样传播；不允许返回固定成功、复制第二份状态或增加 fake renderer。`tinyui.h` 只在私有宏开启时条件包含 bridge，安装配置永远不定义该宏且不安装 internal 头。

- [x] **步骤 3：完成精确错拼和重复语义收口**

- `scroll_selecter` 文件、类型、宏和函数统一为 `scroll_selector`。
- `tinyui_tabel_show_keyboard` 删除，只保留 `tinyui_table_show_keyboard`。
- `tinyui_q_r_code_init/set_text` 删除，只保留 `tinyui_qrcode_create/set_text`。
- button 只保留 `tinyui_button_set_pressed` 与 `tinyui_button_get_pressed`。

canonical 层不提供宏 alias 或 inline alias。旧符号若为仓内 demo 构建必需，只能由迁移桥导出到 full 测试构建；M4 删除后 M5 要求 archive 零命中。

- [x] **步骤 4：重写 `tinyui.h` 聚合边界**

它只能聚合 core result/obj/runtime/event/timer/focus、style/theme、layout、resource 和启用的 widget 头。禁止聚合 display、indev、tick、OSAL、port、integration/input 或 extensions/native；禁止出现 `ld*`、`arm_2d_*`、`SIGNAL_*`、`tile`、PFB 和 backend include。

- [x] **步骤 5：运行头、链接、泄漏和 removed API 门禁**

```bash
rtk cmake --build build/v2.3-m1-widgets --target tinyui_public_header_probes tinyui_public_symbol_link_probes -j
rtk python3 tests/tinyui/contract/check_tinyui_removed_api.py --archive build/v2.3-m1-widgets/libtinyui_core.a
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
```

预期：canonical manifest、默认 `tinyui.h`、安装投影和 minimal archive 的旧符号为零；full archive 中任何临时旧符号都可追溯到唯一 bridge translation unit；`tinyui.h` 默认递归闭包没有 integration、native、backend 或 platform 泄漏。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

## 任务 8：建立每控件编译期裁剪和最小 profile

**文件：**

- 修改：`cmake/LingDongGUI.cmake`
- 创建：`tinyui/include/tinyui_config.h.in`
- 创建：`tests/tinyui/contract/check_tinyui_feature_options.py`
- 修改：`tests/tinyui/perf/check_tinyui_minimal_symbols.py`
- 修改：`tests/tinyui/minimal/minimal_consumer.c`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 产出：每个 widget 一个 `TINYUI_ENABLE_<WIDGET>` CMake cache BOOL 和同名值宏；optional 模块为 `TINYUI_ENABLE_THEME`、`TINYUI_ENABLE_DIAGNOSTICS`、`TINYUI_ENABLE_NATIVE_INTEROP`。

- [x] **步骤 1：写 feature/source 双向一致测试**

checker 读取控件 manifest、CMake cache 和 `tinyui_core` sources：宏为 0 时对应 `tinyui/src/widgets/<widget>.c` 不得进入 target，宏为 1 时必须进入；每个控件恰有一个选项。特殊文件名 `qrcode.c`、`progress_wheel.c`、`scroll_selector.c` 在 manifest 中显式登记。

- [x] **步骤 2：定义全部控件开关**

为当前 29 个控件族逐一增加 BOOL：`WINDOW`、`BACKGROUND`、`LABEL`、`BUTTON`、`CHECKBOX`、`SWITCH`、`SLIDER`、`TEXT`、`IMAGE`、`LINE_EDIT`、`KEYBOARD`、`CANVAS`、`COMBO_BOX`、`SCROLL_SELECTOR`、`TABLE`、`GRAPH`、`CALENDAR`、`ARC`、`GAUGE`、`ICON_SLIDER`、`RADIAL_MENU`、`PROGRESS_BAR`、`PROGRESS_WHEEL`、`QRCODE`、`ANIMATION`、`DATE_TIME`、`CLOCK`、`LIST`、`MESSAGE_BOX`。默认配置开启正式 demo/测试所需项；开关必须直接控制 `target_sources`，不能只控制 header 宏。

- [x] **步骤 3：定义最小 profile**

唯一入口 `-DTINYUI_PROFILE=minimal` 强制只开启 runtime、window/background root、label、button，关闭其余 25 个控件、theme、diagnostics、native interop 和 internal v2.2 demo bridge。不创建 `cmake/TinyUIMinimal.cmake`，也不增加第二个 core library 实现。

- [x] **步骤 4：把 public 聚合头与开关同步**

generated `tinyui_config.h` 的宏值必须是 0/1；`tinyui.h` 在宏为 1 时 include 对应 widget 头。直接 include 被关闭 widget 头必须明确产生预处理错误 `TINYUI_ENABLE_<WIDGET> is disabled`，避免声明存在但实现被裁掉。

- [x] **步骤 5：先运行最小 profile 符号红门禁，再修 source gating**

```bash
rtk cmake -S . -B build/v2.3-m1-minimal -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none -DTINYUI_PROFILE=minimal
rtk cmake --build build/v2.3-m1-minimal --target tinyui_minimal_consumer -j
rtk python3 tests/tinyui/perf/check_tinyui_minimal_symbols.py --mode enforce --binary build/v2.3-m1-minimal/tests/tinyui/tinyui_minimal_consumer --map build/v2.3-m1-minimal/tests/tinyui/tinyui_minimal_consumer.map
```

预期：第一次报告残留模块；修复 target source 和静态引用后，所有关闭控件、theme、diagnostics、native interop 的 public 实现符号为零。

在 `tests/tinyui/CMakeLists.txt` 注册唯一 CTest `check_tinyui_minimal_profile`，它只调用 `check_tinyui_minimal_symbols.py --mode enforce`，label 为 `tinyui;contract;size;minimal`。不得再增加第二个 minimal checker。

- [x] **步骤 6：验证默认 full profile 不丢能力**

```bash
rtk cmake -S . -B build/v2.3-m1-full -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m1-full -j
rtk ctest --test-dir build/v2.3-m1-full --output-on-failure -L '^contract$'
```

预期：默认 profile 的 M0 inventory/header/link gate 通过，所有正式测试所需控件仍启用。

- [ ] **步骤 7：运行格式检查**

```bash
rtk git diff --check
```

## 任务 9：建立静态反重型和 32 位预算门禁

**文件：**

- 创建：`tests/tinyui/contract/check_tinyui_lightweight_architecture.py`
- 创建：`tests/tinyui/contract/test_check_tinyui_lightweight_architecture.py`
- 修改：`tests/tinyui/abi/test_tinyui_public_abi.c`
- 创建：`tests/tinyui/abi/test_tinyui_internal_abi.c`
- 修改：`tests/tinyui/CMakeLists.txt`
- 修改：`tests/tinyui/perf/tinyui_perf_baseline.json`

**接口：**

- 产出：`scan_public_identifiers()`、`scan_forbidden_allocations(functions)`、host/32-bit 条件静态断言。

- [x] **步骤 1：写反重型 checker fixture 测试**

fixture 分别加入 `tinyui_style_class_registry`、`tinyui_event_bubble`、`tinyui_resource_cache`、`tinyui_renderer_plugin`、`tinyui_property_database`，以及在 `tinyui_process` 函数体调用 `malloc`；checker 必须逐项拒绝。注释和字符串不计为标识符命中，防止误报文档描述。

- [x] **步骤 2：实现 public 与 runtime 两层扫描**

public 禁止标识符前缀/片段固定为 `style_class`、`selector`、`cascade`、`bubble`、`capture`、`property_registry`、`resource_manager`、`resource_cache`、`renderer_plugin`、`draw_task`。runtime 禁止在 `tinyui_process`、event dispatch、timer dispatch、`tinyui_theme_apply`、layout setter 和普通 getter 的函数体调用 `malloc/calloc/realloc/free/ldMalloc/ldCalloc/ldFree`。

- [x] **步骤 3：增加 fixed-pool 结构预算断言**

当 `UINTPTR_MAX == UINT32_MAX` 时，internal ABI target 必须断言：

```c
_Static_assert(sizeof(struct tinyui_timer_pool) +
               sizeof(struct tinyui_event_callback_pool) +
               sizeof(struct tinyui_runtime_bookkeeping) <= 1024,
               "TinyUI static runtime budget exceeds 1024 B on 32-bit ABI");
_Static_assert(sizeof(tinyui_image_source_t) <= 80,
               "tinyui_image_source_t exceeds 80 B on 32-bit ABI");
_Static_assert(sizeof(tinyui_font_t) <= 16,
               "tinyui_font_t exceeds 16 B on 32-bit ABI");
```

如果当前 host 不是 32 位，host 测试必须显示 `ABI32_NOT_EXECUTED_ON_HOST`，但 CMake 同时生成 `tinyui_abi32_compile` 目标供 32 位 MCU toolchain 执行；不得把 host skip 记录成 32 位通过。

- [x] **步骤 4：把 M0 baseline pool 状态切换为 fixed_pool**

只有 `tinyui_timer_pool`、`tinyui_event_callback_pool` 和 `tinyui_runtime_bookkeeping` 三个实际固定结构存在后，才把 JSON 的 `implementation` 从 `legacy` 改为 `fixed_pool`，并写入三个实际 32 位尺寸及 sum。若 M1 只完成声明、M2 才完成池实现，则 M1 closeout 保持 `legacy` 并明确该项是 M2 阻断门禁，不能伪造为通过。

- [x] **步骤 5：注册并运行静态门禁**

```bash
rtk python3 tests/tinyui/contract/test_check_tinyui_lightweight_architecture.py -v
rtk python3 tests/tinyui/contract/check_tinyui_lightweight_architecture.py --root .
rtk cmake --build build/v2.3-m1-full --target test_tinyui_public_abi test_tinyui_internal_abi -j
rtk ctest --test-dir build/v2.3-m1-full --output-on-failure -L '^lightweight$'
```

预期：没有重型 public/runtime 概念；host ABI probe 通过；32 位是否执行被准确标记。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

## 任务 10：冻结 M1 public contract 并执行 closeout

**文件：**

- 修改：`tests/tinyui/contract/tinyui_v23_public_api.json`（写入最终哈希）
- 检查：本计划全部文件

**接口：**

- 消费：任务 1-9 的 canonical manifest、full/minimal 构建和 M0 baseline。
- 产出：M1 public ABI freeze 哈希；不产出 L3-L5 或 port 完成声明。

- [x] **步骤 1：重新生成 public contract manifest 并确认无漂移**

```bash
rtk python3 tests/tinyui/contract/check_tinyui_v23_public_api.py --write-hash tests/tinyui/contract/tinyui_v23_public_api.json
rtk python3 tests/tinyui/contract/check_tinyui_v23_public_api.py --check tests/tinyui/contract/tinyui_v23_public_api.json
```

预期：输出稳定 SHA-256；第二次运行不修改文件。

- [x] **步骤 2：执行默认 full closeout**

```bash
rtk cmake -S . -B build/v2.3-m1-closeout -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m1-closeout -j
rtk ctest --test-dir build/v2.3-m1-closeout --output-on-failure
```

预期：配置、全量构建和全部注册 CTest 零失败；M0 inventory、公共头、逐符号链接、wrapper、分配和性能门禁保持绿色。

- [x] **步骤 3：执行最小 profile closeout**

```bash
rtk cmake -S . -B build/v2.3-m1-minimal-closeout -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none -DTINYUI_PROFILE=minimal
rtk cmake --build build/v2.3-m1-minimal-closeout --target tinyui_minimal_consumer -j
rtk python3 tests/tinyui/perf/check_tinyui_minimal_symbols.py --mode enforce --binary build/v2.3-m1-minimal-closeout/tests/tinyui/tinyui_minimal_consumer --map build/v2.3-m1-minimal-closeout/tests/tinyui/tinyui_minimal_consumer.map
```

预期：只保留 runtime、root window/background、label、button 及其不可分割 backend 依赖；关闭控件和 optional 模块的 public 实现符号为零。

- [x] **步骤 4：执行 legacy/native/重型概念终检**

```bash
rtk python3 tests/tinyui/contract/check_tinyui_removed_api.py --archive build/v2.3-m1-closeout/libtinyui_core.a
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
rtk python3 tests/tinyui/contract/check_tinyui_lightweight_architecture.py --root .
rtk ctest --test-dir build/v2.3-m1-closeout --output-on-failure -L '^contract$'
```

预期：legacy、错拼、重复 API、native 泄漏和重型 runtime 标识符均为零。

- [x] **步骤 5：运行 GitNexus 阶段影响收敛**

调用 GitNexus `detect_changes()`，确认受影响流程与任务 1 的迁移清单一致；重新对 `tinyui_screen_create`、`tinyui_init` 和 canonical replacement 做 upstream impact。任何未列出的 HIGH/CRITICAL 消费者必须在 M1 内迁移或明确阻断，不能留给运行时偶然暴露。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

M1 完成时只允许声明“canonical 公共契约已冻结、独立消费者可编译链接、轻量静态门禁和编译期裁剪已建立”。不得声明 timer/event 语义、全部 setter backend 映射、像素/事件证据或目标 port 已闭环；这些分别由 M2、M3 和延期 port 工作负责。
