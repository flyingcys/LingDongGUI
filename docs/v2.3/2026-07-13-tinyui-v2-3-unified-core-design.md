# TinyUI v2.3 轻量统一设计

> 本文是 TinyUI v2.3 非 port 工作的设计真相源。v2.3 允许一次破坏式公共 API 收口，不以兼容现有 legacy ABI 为完成约束。任何阶段计划、实现和发布结论都必须服从本文。

## 1. 产品定位

TinyUI 的定位固定为：

> **LVGL 风格的易用 API + LingDongGUI/Arm-2D 的轻量实现，面向 LVGL 无法承载的小型 MCU。**

TinyUI 参考 LVGL 的 API 组织和用户体验，但不复制 LVGL 的内部架构。TinyUI 不是第二套 GUI renderer，也不是另一套完整 GUI framework。

TinyUI 的价值是让用户通过统一、简单的 `tinyui_*` API 使用 LingDongGUI 已有能力，不再直接理解 `ld*`、Arm-2D、signal、PFB 和复杂宿主流程，同时保持 LingDongGUI 原有的小巧、可裁剪和低资源消耗特性。

## 2. 背景

当前 TinyUI 已经建立真实的 LingDongGUI/Arm-2D 渲染链，大多数控件、flex/grid 和 demo 已映射到真实 LingDongGUI 对象。当前主要问题不在于缺少一套新框架，而在于封装没有完全收敛：

- canonical runtime 与 legacy app 两套生命周期并存。
- screen、window、widget、obj 等公共对象类型不能自然组合。
- 部分通用 setter 只修改 TinyUI 镜像，没有更新真实 LingDongGUI 对象。
- 事件 callback、焦点、theme、资源和错误返回的形式不统一。
- 公共头仍泄漏 native、Arm-2D 或 backend 概念。
- grid 等 API 存在魔数和隐藏容量。
- 能力矩阵证明了符号分类，但没有逐项证明真实 backend 行为。
- 默认全量构建与源码 inventory 仍存在已知失败。

v2.3 的目标是统一这些封装问题，而不是增加一套独立运行机制。

## 3. 核心原则

v2.3 必须遵守以下原则：

1. TinyUI 是 LingDongGUI 的薄上层 API。
2. 每个 TinyUI 对象直接对应一个真实 LingDongGUI 对象。
3. LingDongGUI tree 是唯一对象树真值；TinyUI 不建立第二套 ownership tree、布局器、绘制器、样式引擎或资源中心。
4. TinyUI 只保存 backend 无法查询、但完成用户契约确实需要的最少状态。
5. 通用 setter 直接映射到真实 `ld*` 能力；无法支持时返回 `TINYUI_ERROR_NOT_SUPPORTED`。
6. 不通过软件模拟扩展 LingDongGUI 不具备的控件能力。
7. flex/grid 直接映射 LingDongGUI layout，不进行第二次布局计算。
8. 事件只统一事件码和 callback 形式，不实现冒泡、capture 或通用事件队列。
9. style 使用固定轻量 descriptor 直接应用，不实现字符串 class、级联和 selector 引擎。
10. 图片与字体使用调用者持有的 descriptor，不在 core 中实现引用计数、缓存或资源管理器。
11. UI 主循环不得产生隐式逐帧动态分配。
12. 控件和可选能力必须支持编译期裁剪。
13. “100% 覆盖”只指 LingDongGUI 已有用户能力都能通过 TinyUI public API 使用，不追求 LVGL 功能覆盖。
14. v2.3 中间态不作为完整版本发布；结束时只冻结一次 canonical ABI。

## 4. 目标

v2.3 必须完成：

- 恢复构建、测试、inventory 和公共 API 的事实可信度。
- 删除两套生命周期和 public app handle 模型。
- 统一 `tinyui_obj_t *` 对象与 `create(parent)` 形式。
- 让所有成功返回的公开 setter 都作用到真实 LingDongGUI 对象。
- 统一 callback、事件码、焦点和 timer API，但不增加重型调度机制。
- 用轻量 descriptor 统一 theme、style、图片和字体入口。
- 规范 flex/grid API，并保持直接 LingDongGUI 映射。
- 对全部控件和全部 required 用户能力逐项闭环。
- 收紧公共头、构建 target、安装内容和 native 边界。
- 迁移全部 demo、示例和用户文档到唯一 canonical API。
- 建立 L1-L5 能力证据和硬性能门禁。

## 5. 非目标

v2.3 明确不做：

- 不实现 LVGL 兼容层或 LVGL ABI。
- 不实现通用 retained property 数据库。
- 不实现事件冒泡、capture、DOM 式传播或动态事件队列。
- 不实现 CSS 式 selector、字符串 style class registry 或级联样式引擎。
- 不实现资源引用计数、缓存、异步加载或全局资源中心。
- 不实现第二套 layout solver。
- 不创建可插拔 renderer 框架。
- 不机械包装每一个 `ld*` 函数。
- 不修改 SDL、MCU 或其他具体 port 的生产实现。
- 不解决显示格式、flush/DMA、PFB、平台时钟、SDL 生命周期、RTOS、ISR 或真机落板问题。
- 不以兼容 v2.2 legacy ABI 为约束。

现有 SDL 只作为 v2.3 的测试宿主，用于取得真实 LingDongGUI 像素和事件证据；这不构成 SDL port 完成或生产可用声明。

## 6. 轻量硬预算

轻量不是主观描述，必须由硬门禁约束。

### 6.1 对象内存

沿用现有 baseline：

- `widget_wrapper_struct_bytes` 当前为 `184 B`，v2.3 上限为 `192 B`。
- `switch_wrapper_struct_delta_bytes` 上限保持 `32 B`。
- `backend_widget_struct_bytes` 当前为 `496 B`，上限保持 `512 B`。
- M0 必须记录每一种 wrapper 的 `sizeof`，不只检查基础 widget 和 switch。
- 每一种 wrapper 的 v2.3 增量不得同时超过 `5%` 和 `8 B`；任何单项越过已记录的绝对安全上限也直接失败。
- 32 位 MCU ABI 下，timer pool、event callback pool 和 runtime bookkeeping 的静态 RAM 总和不得超过 `1024 B`。

公共 API 统一不得通过给每个对象增加大型事件表、style 表、属性表或资源引用表实现。

### 6.2 二进制大小

沿用现有双阈值策略。相对 M0 冻结 baseline：

| 指标 | 最大相对增长 | 最大绝对增长 |
| --- | ---: | ---: |
| `__text` | `5%` | `8192 B` |
| `__data` | `25%` | `256 B` |
| `__bss` | `10%` | `8192 B` |
| `total` | `5%` | `12288 B` |

只有相对增长和绝对增长同时越线时判定失败，保持现有 checker 语义。

### 6.3 运行时分配

- `tinyui_process()` 自身每轮隐式 heap 分配次数必须为 `0`。
- layout、事件 dispatch、theme apply 和普通 getter 不得分配 heap。
- 对象创建、字符串复制等显式 API 可以使用项目统一 allocator。
- timer 和 callback 注册项使用编译期固定容量池，不在 dispatch 时分配。
- OOM 或容量耗尽必须返回明确错误，不得静默跳过。

### 6.4 编译期裁剪

- 每个控件族必须有独立 `TINYUI_ENABLE_<WIDGET>` 开关。
- theme、native interop、diagnostics 等可选模块必须可关闭。
- 未启用模块不进入链接产物，不保留空注册表或通用分发表。
- 默认配置只开启当前正式 demo 和测试需要的能力；目标项目可进一步裁剪。
- 必须提供最小裁剪 profile：只启用 runtime、screen/window、label、button，关闭 diagnostics、theme、native interop 和其他控件。
- 最小 profile 使用 map/nm 检查被关闭控件和可选模块的 public 实现符号未进入最终二进制。
- 最小 profile 的唯一配置入口为 `-DTINYUI_PROFILE=minimal`；不得再并存 preset、布尔总开关或第二套 checker 语义。

## 7. API 参考 LVGL 的边界

TinyUI 只参考以下 API 模式：

- 单一 `tinyui_init()` 启动入口。
- 不透明统一对象类型。
- 所有控件使用 `create(parent)`。
- 通用对象属性使用统一 `tinyui_obj_set_*()`。
- 控件特有能力使用 `tinyui_<widget>_*()`。
- 统一事件码和 callback 签名。
- demo 只负责创建 UI，runtime 由统一入口持有。

TinyUI 不参考以下 LVGL 内部机制：

- 完整 class/object runtime。
- 通用 property reflection。
- 多层 style selector 和级联。
- 事件冒泡和捕获。
- 通用 draw task 或 renderer plugin。
- 通用异步文件系统和资源缓存。

## 8. 公共 API 分层

### 8.1 Canonical 用户层

`tinyui/include/tinyui.h` 只聚合普通用户创建 UI 所需的 canonical API：

- runtime。
- object。
- widget。
- event。
- theme/style descriptor。
- flex/grid。
- image source/font value descriptor。
- timer 和 focus。

总头不得包含：

- legacy app API。
- backend/native internal API。
- display、indev、tick、OSAL 等平台集成细节。
- LingDongGUI 或 Arm-2D 头。
- 测试、demo 或宿主 helper。

### 8.2 显式集成层

display、indev、tick、OSAL 和 port 相关头保留为显式 include，不由 `tinyui.h` 默认导出。v2.3 只允许为适配新 canonical core 做必要签名迁移，不在本版本解决其生产契约。

### 8.3 Native 互操作层

确有需要时，native escape hatch 放入单独高级扩展头：

- 不由 `tinyui.h` 聚合。
- 名称明确包含 `native` 或 `ldgui`。
- canonical demo 和文档不得使用。
- 不参与普通用户能力覆盖证明。

### 8.4 Internal 层

`tinyui/src/core`、`tinyui/src/drivers`、LingDongGUI 和 Arm-2D include directory 必须是 PRIVATE 依赖。安装包不得包含 internal 头。

### 8.5 仓内迁移桥

M1 冻结 canonical API 后，仓内 29 个旧 demo 要到 M4 才完成迁移。为保证 M1-M3 的默认全量构建始终可执行，允许存在一条严格受限的 `temporary migration bridge`：

- 只对仓内旧 demo 和尚未迁移的旧测试启用，不由 `tinyui.h` 的默认预处理路径导出。
- 不安装、不进入能力矩阵、不进入最小 profile，也不作为用户兼容承诺。
- bridge 只能转发到真实 canonical/backend 行为；不允许空实现、假成功、fake renderer 或第二套状态。
- bridge 的声明必须位于 internal/compat 边界，并由私有编译宏显式开启；普通消费者与安装消费者必须证明无法看到这些声明和符号。
- M4 在全部 demo、文档和消费者迁移后删除 bridge；M5 对头、archive、安装树和 demo 做 legacy 零命中门禁。

这条 bridge 只解决仓内迁移顺序，不改变“v2.3 最终只剩一套 canonical ABI”的完成定义。

## 9. Runtime 与执行模型

### 9.1 Canonical 生命周期

```c
typedef enum tinyui_screen_transition {
    TINYUI_SCREEN_TRANSITION_NONE,
    TINYUI_SCREEN_TRANSITION_FADE_WHITE,
    TINYUI_SCREEN_TRANSITION_FADE_BLACK,
    TINYUI_SCREEN_TRANSITION_SLIDE_LEFT,
    TINYUI_SCREEN_TRANSITION_SLIDE_RIGHT,
    TINYUI_SCREEN_TRANSITION_SLIDE_UP,
    TINYUI_SCREEN_TRANSITION_SLIDE_DOWN,
    TINYUI_SCREEN_TRANSITION_ERASE_LEFT,
    TINYUI_SCREEN_TRANSITION_ERASE_RIGHT,
    TINYUI_SCREEN_TRANSITION_ERASE_UP,
    TINYUI_SCREEN_TRANSITION_ERASE_DOWN,
    TINYUI_SCREEN_TRANSITION_FLY_IN_LEFT,
    TINYUI_SCREEN_TRANSITION_FLY_IN_RIGHT,
    TINYUI_SCREEN_TRANSITION_FLY_IN_TOP,
    TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM,
} tinyui_screen_transition_t;

tinyui_result_t tinyui_init(void);
void tinyui_deinit(void);

tinyui_obj_t *tinyui_screen_create(void);
tinyui_obj_t *tinyui_screen_create_with_props(
    const tinyui_window_props_t *props);
tinyui_obj_t *tinyui_background_create(void);
tinyui_obj_t *tinyui_background_create_with_props(
    const tinyui_background_props_t *props);
tinyui_result_t tinyui_screen_load(tinyui_obj_t *screen,
                                   tinyui_screen_transition_t transition,
                                   uint32_t duration_ms);
tinyui_obj_t *tinyui_screen_active(void);

tinyui_result_t tinyui_process(uint32_t *next_ms);
```

约定：

- runtime 为单实例。
- 重复 init 返回 `TINYUI_ERROR_INVALID_STATE`。
- deinit 在未初始化时为空操作。
- `TINYUI_SCREEN_TRANSITION_NONE` 表示立即切换。
- 其他 transition 与 Arm-2D 现有 scene switch mode 做静态一一映射，不公开 Arm-2D 类型，也不实现新动画引擎。
- `tinyui_process()` 返回 core/backend 结果。
- `next_ms == 0` 表示应立即再次处理。
- `next_ms == UINT32_MAX` 表示当前没有 core deadline。
- 平台如何 sleep、等待事件和提供时钟不属于 v2.3。
- `tinyui_screen_create()` 和 `tinyui_background_create()` 都创建可加载的 root；后者直接映射现有 LingDongGUI background/window 能力。
- legacy `app_run()` 不再有对应阻塞 API，用户循环固定调用 `tinyui_process()`。
- legacy `run_background/switch_background` 分别由 `background_create + screen_load(NONE)` 和 `screen_load(transition)` 表达。

### 9.2 单线程约束

- 所有 canonical API 只能在 UI 线程调用。
- callback 在调用 `tinyui_process()` 的同一线程同步执行。
- canonical API 不提供线程安全保证。
- ISR 不得直接调用 TinyUI canonical API。
- port 如何把 ISR 或其他线程事件送到 UI 线程属于后续 port 工作。

### 9.3 删除 legacy app 模型

以下 public 概念删除：

- `tinyui_app_create()`。
- `tinyui_app_run()`。
- `tinyui_app_set_window()`。
- `tinyui_app_switch_window()`。
- 公开 `struct tinyui_app *` 参数。

原 app 上的用户能力必须迁移到 runtime、screen、timer、theme 和 focus API，不能随 app API 一起丢失。

## 10. 统一对象模型

### 10.1 对象类型与 creator

```c
typedef struct tinyui_obj tinyui_obj_t;

tinyui_obj_t *tinyui_label_create(tinyui_obj_t *parent);
tinyui_obj_t *tinyui_label_create_with_props(
    tinyui_obj_t *parent,
    const tinyui_label_props_t *props);
```

所有控件遵循同一形式。调用者不在 screen、window、widget 和 obj 之间强转。

props 规则：

- 每种 props 使用显式 presence bit，不依赖 `0/-1/NULL` 猜测字段是否设置。
- props creator 必须复用正式 setter。
- 任一 setter 失败时销毁已创建的 TinyUI 和 LingDongGUI 对象，parent 不保留半初始化 child。
- 普通 creator 等价于空 props。

### 10.2 ID

为避免字符串 registry，canonical ID 使用轻量整数，并只在创建时确定：

```c
tinyui_result_t tinyui_obj_get_id(const tinyui_obj_t *obj, uint16_t *id);
tinyui_obj_t *tinyui_obj_find_by_id(tinyui_obj_t *root, uint16_t id);
```

规则：

- props 中的 `id` 在创建真实 LingDongGUI 对象前分配。
- `0` 表示由 runtime 自动分配。
- 同一 root 内 ID 必须唯一；显式 ID 冲突时 creator 返回 `NULL`，且不创建 backend 对象。
- ID 创建后不可修改，避免破坏 backend host/event lookup。
- `find_by_id()` 在指定 root 的真实 LingDongGUI subtree 中查找唯一 ID。
- ID 与 LingDongGUI `nameId` 的 16 位能力直接对应；TinyUI 不维护 32 位映射表或全局字符串 ID 表。

### 10.3 父子所有权

- screen 没有 parent，由 runtime 管理。
- 普通对象必须有有效 parent。
- LingDongGUI tree 是 parent/child/root 的唯一结构真值；TinyUI 通过 `ldBaseGetParent`、node traversal 和 `pInfo/nameId` host lookup 返回 wrapper，不保存第二份 parent/child/sibling 指针树。
- parent 拥有 child；删除 parent 通过真实 LingDongGUI node tree 递归删除 child，并同步注销对应 wrapper。
- `tinyui_obj_delete()` 同步从真实 LingDongGUI tree 删除对象，再释放对应 wrapper。
- callback 内删除当前对象时，只延迟到当前 callback 返回后执行。
- 删除标记设置后，当前对象不再接收后续 callback。
- 删除当前 target 后不再继续相关事件分发。
- 对象真实释放后再次传入旧指针属于未定义行为；TinyUI 不为检测任意悬空指针增加 handle registry。

公开遍历能力固定为：

```c
tinyui_obj_t *tinyui_obj_get_parent(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_first_child(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_next_sibling(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_root(const tinyui_obj_t *obj);
tinyui_result_t tinyui_obj_get_child_count(const tinyui_obj_t *obj,
                                           uint16_t *count);
```

这些 API 直接查询 LingDongGUI tree，不维护镜像 tree。

### 10.4 数据所有权

- v2.3 将文本 setter 明确定义为复制字符串；这是对当前只保存调用者指针行为的修正。
- 文本复制使用项目统一 allocator；分配失败返回 `NO_MEMORY`，不得修改原文本或 backend。
- 对象删除和文本替换必须释放旧副本，并纳入 allocator 失败、重复替换和销毁测试。
- getter 返回的字符串在下一次修改或对象删除前有效。
- props 中的临时标量在 creator 返回后不需要保持。
- image/font descriptor 和像素数据采用调用者持有模型，见资源章节。
- callback user data 由用户持有，TinyUI 不释放。

## 11. 统一结果与诊断

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

规则：

- setter、删除、注册、布局和绑定等可能失败的操作返回结果码。
- creator 返回对象指针或 `NULL`。
- getter 若值域与错误哨兵冲突，使用 `result + out parameter`。
- 不再用合法坐标 `-1` 表示失败。
- 所有收窄转换先检查范围。
- backend 失败不得转换为成功。
- 不支持的能力返回 `TINYUI_ERROR_NOT_SUPPORTED`。

可选诊断模块：

```c
tinyui_result_t tinyui_last_result(void);
const char *tinyui_last_error_message(void);
```

- `TINYUI_ENABLE_DIAGNOSTICS=0` 时不保存动态错误字符串，只返回静态结果码名称。
- 开启后使用 runtime 固定大小 buffer，不分配 heap。
- message 在下一次失败调用前有效。
- 诊断是 UI 线程局部 runtime 状态，不提供多线程存储。

## 12. 属性直接映射

### 12.1 唯一 setter 路径

所有公开 setter 使用同一直接流程：

```text
参数和对象校验
    -> 查询该控件是否有真实 LingDongGUI 能力
    -> 调用对应 ld* API
    -> 成功后更新最少必要缓存
    -> 返回结果
```

禁止：

- 只更新 TinyUI 镜像就返回成功。
- 建立第二套通用 property database。
- 为 backend 不支持的能力增加软件绘制或 fake widget。
- generic setter 和 widget-specific setter 走不同状态真值。

### 12.2 通用属性

以下能力建立静态、编译期的“控件类型 → adapter 函数”表或 switch；不得建立运行时注册表：

- text。
- bg/text/border color。
- border width、radius、padding。
- position、size、visible、enabled、opacity。
- selectable、selected、focusable。
- layout participation。

表中只有真实支持和不支持两种状态。适用控件的 setter 必须验证真实 `ld*` 状态；不适用控件返回 `NOT_SUPPORTED`。

### 12.3 getter 真值

- LingDongGUI 有 getter 时直接读取 backend。
- LingDongGUI 没有 getter、但 TinyUI 必须提供对称 getter 时，只缓存该单项最后一次成功值。
- backend setter 成功前不得更新缓存。
- native escape hatch 修改 backend 后，相关 TinyUI getter 结果不保证同步；该限制只属于高级互操作层。

### 12.4 批量 style/props

- props 失败通过销毁新对象实现整体回滚。
- style descriptor 先完整验证所有 presence 字段，再执行 setter。
- 对已存在对象应用多属性 style 时，API 明确为按字段顺序提交；若中途发生 backend 异常，返回 `BACKEND`，不伪装成原子事务。
- 普通参数错误和 `NOT_SUPPORTED` 必须在任何字段修改前完成预检查。

## 13. 轻量事件与焦点

### 13.1 事件模型

```c
typedef enum tinyui_event_code {
    TINYUI_EVENT_PRESSED,
    TINYUI_EVENT_RELEASED,
    TINYUI_EVENT_CLICKED,
    TINYUI_EVENT_VALUE_CHANGED,
    TINYUI_EVENT_FOCUSED,
    TINYUI_EVENT_DEFOCUSED,
    TINYUI_EVENT_KEY,
    TINYUI_EVENT_DELETE,
} tinyui_event_code_t;

typedef struct tinyui_key_event_data {
    uint16_t key;
    bool pressed;
} tinyui_key_event_data_t;

typedef struct tinyui_event {
    tinyui_event_code_t code;
    tinyui_obj_t *target;
    void *user_data;
    union {
        int32_t value;
        tinyui_key_event_data_t key;
    } data;
} tinyui_event_t;

typedef void (*tinyui_event_cb_t)(const tinyui_event_t *event);
typedef uint32_t tinyui_event_handle_t;

#define TINYUI_EVENT_MASK(code) (UINT32_C(1) << (code))
#define TINYUI_EVENT_MASK_ALL   UINT32_MAX
```

payload 的含义由事件码固定：

- `VALUE_CHANGED`：`data.value` 是控件新整数值；复杂控件使用控件专用查询 API 读取完整状态。
- `KEY`：使用 `data.key.key` 和 `data.key.pressed`。
- 其他事件不读取 `data`。

### 13.2 注册

```c
tinyui_result_t tinyui_obj_add_event_cb(tinyui_obj_t *obj,
                                        uint32_t event_mask,
                                        tinyui_event_cb_t cb,
                                        void *user_data,
                                        tinyui_event_handle_t *handle);
tinyui_result_t tinyui_obj_remove_event_cb(tinyui_obj_t *obj,
                                           tinyui_event_handle_t handle);
```

实现约束：

- callback 项来自 runtime 固定池。
- `TINYUI_EVENT_CB_CAPACITY` 默认 `16`，项目可编译期调整。
- 对象只保存固定池索引，不内嵌 callback 数组。
- 容量耗尽返回 `TINYUI_ERROR_CAPACITY`。
- 事件由 backend signal 同步转换并直接调用 callback。
- 不排队、不冒泡、不 capture。
- 同一对象按注册顺序调用。
- callback 内新增项不参与当前 dispatch。
- callback 内移除尚未调用的项后，该项不再调用。
- callback 内删除 target 后停止当前 target 的后续 callback。
- handle 的低 16 位编码 `slot + 1`，高 16 位编码 generation；`0` 为无效 handle。
- 固定池槽位复用时 generation 必须递增，旧 handle 不得删除新 callback。
- `DELETE` 在 wrapper 标记 deleting 后、真实 LingDongGUI 对象销毁前同步发送一次。
- `DELETE` callback 中只允许只读 getter；setter、重新注册 callback 和再次删除返回 `INVALID_STATE`。
- `DELETE` callback 返回后清理 callback slot，再销毁 LingDongGUI 对象和 wrapper。

### 13.3 焦点

```c
typedef enum tinyui_focus_direction {
    TINYUI_FOCUS_NEXT,
    TINYUI_FOCUS_PREVIOUS,
    TINYUI_FOCUS_LEFT,
    TINYUI_FOCUS_RIGHT,
    TINYUI_FOCUS_UP,
    TINYUI_FOCUS_DOWN,
} tinyui_focus_direction_t;

tinyui_result_t tinyui_focus_set(tinyui_obj_t *obj);
tinyui_result_t tinyui_focus_clear(void);
tinyui_result_t tinyui_focus_move(tinyui_focus_direction_t direction);
tinyui_obj_t *tinyui_focus_current(void);
```

焦点能力直接复用现有 LingDongGUI focus/navigation 行为。`tinyui_native_nav_dir` 不进入 public API。

### 13.4 Key

```c
typedef enum tinyui_key {
    TINYUI_KEY_ENTER,
    TINYUI_KEY_BACK,
    TINYUI_KEY_LEFT,
    TINYUI_KEY_RIGHT,
    TINYUI_KEY_UP,
    TINYUI_KEY_DOWN,
    TINYUI_KEY_NEXT,
    TINYUI_KEY_PREVIOUS,
} tinyui_key_t;

tinyui_result_t tinyui_input_send_key(tinyui_key_t key, bool pressed);
```

该 API 位于显式 integration input 头，不由普通控件头重复声明。它只能在 UI 线程调用，同步把一个抽象 key 交给当前焦点对象，并转换为 `TINYUI_EVENT_KEY` 或控件专用行为。具体 SDL/MCU 如何采集、缓存和跨线程传递 key 属于 port，不在 v2.3 实现。

## 14. Timer

```c
typedef struct tinyui_timer tinyui_timer_t;
typedef void (*tinyui_timer_cb_t)(tinyui_timer_t *timer, void *user_data);

tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms,
                                    bool repeat,
                                    tinyui_timer_cb_t cb,
                                    void *user_data);
tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer,
                                          uint32_t interval_ms);
void tinyui_timer_delete(tinyui_timer_t *timer);
```

约束：

- timer 使用固定池，`TINYUI_TIMER_CAPACITY` 默认 `16`。
- timer 比较必须处理毫秒回绕。
- callback 内允许停止或删除当前 timer。
- callback 内创建 timer 只影响下一轮调度。
- timer 调度不创建 snapshot heap。
- 最近 deadline 通过 `tinyui_process(next_ms)` 返回。

## 15. 轻量 Style 与 Theme

### 15.1 Style descriptor

`tinyui_style_t` 是调用者栈上、静态区或只读区均可创建的普通值结构，包含：

- presence mask。
- bg color。
- text color。
- border color 和 width。
- radius。
- padding。
- opacity。
- font descriptor 指针。

```c
typedef struct tinyui_font tinyui_font_t;

typedef enum tinyui_style_field {
    TINYUI_STYLE_BG_COLOR     = 1u << 0,
    TINYUI_STYLE_TEXT_COLOR   = 1u << 1,
    TINYUI_STYLE_BORDER_COLOR = 1u << 2,
    TINYUI_STYLE_BORDER_WIDTH = 1u << 3,
    TINYUI_STYLE_RADIUS       = 1u << 4,
    TINYUI_STYLE_PADDING      = 1u << 5,
    TINYUI_STYLE_OPACITY      = 1u << 6,
    TINYUI_STYLE_FONT         = 1u << 7,
} tinyui_style_field_t;

typedef struct tinyui_style {
    uint32_t fields;
    uint32_t bg_color;
    uint32_t text_color;
    uint32_t border_color;
    int16_t border_width;
    int16_t radius;
    int16_t padding;
    uint8_t opacity;
    const tinyui_font_t *font;
} tinyui_style_t;
```

part 和 state 是固定枚举，不支持用户动态扩展：

```c
typedef enum tinyui_part {
    TINYUI_PART_MAIN,
    TINYUI_PART_TEXT,
    TINYUI_PART_INDICATOR,
    TINYUI_PART_KNOB,
    TINYUI_PART_TRACK,
} tinyui_part_t;

typedef enum tinyui_state {
    TINYUI_STATE_DEFAULT,
    TINYUI_STATE_DISABLED,
    TINYUI_STATE_PRESSED,
    TINYUI_STATE_CHECKED,
    TINYUI_STATE_FOCUSED,
} tinyui_state_t;
```

```c
tinyui_result_t tinyui_obj_apply_style(tinyui_obj_t *obj,
                                       tinyui_part_t part,
                                       tinyui_state_t state,
                                       const tinyui_style_t *style);
```

行为：

- 调用时立即把 descriptor 字段映射到真实 LingDongGUI setter。
- TinyUI 不保存 style 指针。
- TinyUI 不维护 style class、selector 或级联关系。
- 调用返回后，调用者可释放临时 style descriptor。
- state style 仅映射 LingDongGUI 已有 native state 配置；backend 不支持时返回 `NOT_SUPPORTED`。

### 15.2 固定 part/state

canonical part 只保留当前 LingDongGUI 能直接表达的集合：

- `MAIN`。
- `TEXT`。
- `INDICATOR`。
- `KNOB`。
- `TRACK`。

canonical state：

- `DEFAULT`。
- `DISABLED`。
- `PRESSED`。
- `CHECKED`。
- `FOCUSED`。

每个控件支持的 part/state 组合进入静态能力矩阵，不做运行时发现。

### 15.3 Theme

`tinyui_theme_t` 是固定颜色和 metric 的轻量 descriptor，沿用现有 color/metric 枚举，不增加动态 style 集合。

```c
typedef struct tinyui_theme {
    uint32_t colors[TINYUI_COLOR_COUNT];
    int16_t metrics[TINYUI_METRIC_COUNT];
} tinyui_theme_t;
```

```c
tinyui_result_t tinyui_theme_set(const tinyui_theme_t *theme);
const tinyui_theme_t *tinyui_theme_get(void);
tinyui_result_t tinyui_theme_apply(tinyui_obj_t *obj);
```

约束：

- theme 由调用者持有，设置后必须保持有效直到替换或 deinit。
- 新对象创建时同步应用当前 theme。
- 替换 theme 时不自动遍历全树，避免不可控时延；用户显式调用 apply。
- apply 直接调用各控件 adapter，不创建 style 对象、不分配 heap。

## 16. Layout 直接映射

### 16.1 Flex

flex API 统一接收 `tinyui_obj_t *container`，直接调用 LingDongGUI flex：

- flow。
- main/cross/track align。
- item/track gap。
- child grow。
- min/max size。
- new track。
- ignore layout。

TinyUI 不重复计算几何结果。

### 16.2 Grid

grid 使用有类型 track，不再使用负数魔数：

```c
typedef enum tinyui_grid_unit {
    TINYUI_GRID_UNIT_PX,
    TINYUI_GRID_UNIT_FR,
    TINYUI_GRID_UNIT_CONTENT,
} tinyui_grid_unit_t;

typedef struct tinyui_grid_track {
    tinyui_grid_unit_t unit;
    uint16_t value;
} tinyui_grid_track_t;

tinyui_result_t tinyui_grid_set_columns(
    tinyui_obj_t *container,
    const tinyui_grid_track_t *tracks,
    uint8_t count);
```

约束：

- 使用显式 count，不使用 END sentinel。
- 最大 track 数公开为 `TINYUI_GRID_MAX_TRACKS=16`，与当前实现容量一致。
- PX 合法范围为 `1..32767`。
- FR 合法范围为 `1..255`。
- CONTENT 的 value 必须为 `0`。
- TinyUI 只做参数转换和检查，然后调用 LingDongGUI grid。

### 16.3 Reflow

TinyUI 只调用 LingDongGUI 已有 reflow/dirty 接口，不维护第二套 dirty tree。若 LingDongGUI 某项变化当前不能触发正确 reflow，应修复真实 LingDongGUI 映射，不在 demo 中补固定坐标。

## 17. 轻量图片与字体

v2.3 只公开能够静态、直接适配现有 LingDongGUI/Arm-2D 资源入口的轻量 value descriptor，不设计新的字体协议、文件系统或资源 provider。

### 17.1 Image source

```c
typedef enum tinyui_image_source_kind {
    TINYUI_IMAGE_SOURCE_EMPTY,
    TINYUI_IMAGE_SOURCE_RGB565_MEMORY,
    TINYUI_IMAGE_SOURCE_BUILTIN,
    TINYUI_IMAGE_SOURCE_VRES,
} tinyui_image_source_kind_t;

typedef struct tinyui_image_source {
    tinyui_image_source_kind_t kind;
    uint16_t width;
    uint16_t height;
    uint32_t stride;
    const uint16_t *pixels;
    const uint8_t *mask;
    uint32_t mask_stride;
    uintptr_t _image_private[6];
    uintptr_t _mask_private[6];
} tinyui_image_source_t;

tinyui_result_t tinyui_image_source_from_rgb565(
    const uint16_t *pixels,
    uint16_t width,
    uint16_t height,
    uint32_t stride,
    const uint8_t *mask,
    uint32_t mask_stride,
    tinyui_image_source_t *out);

tinyui_result_t tinyui_image_source_from_builtin(
    tinyui_builtin_image_t image,
    tinyui_image_source_t *out);

tinyui_result_t tinyui_image_source_from_vres(
    uint32_t address,
    tinyui_image_source_t *out);

void tinyui_image_source_deinit(tinyui_image_source_t *source);
```

约束：

- RGB565 memory source 直接在 `_image_private` 和 `_mask_private` 中构造 backend 所需 image/mask tile view，不分配 heap、不复制像素。
- mask 固定为 A8 借用数据。
- builtin 映射现有只读静态资源。
- VRES 复用 LingDongGUI 已有 VRES 获取和释放能力。
- source 绑定对象期间，source、pixels 和 mask 必须保持有效。
- 两块 private storage 只作为固定大小 backend storage，不暴露 Arm-2D 类型。
- internal ABI probe 必须断言每块 storage 都不小于当前 `sizeof(arm_2d_tile_t)`；不足时构建失败，不允许转为隐式 heap。
- 32 位 ABI 下 `sizeof(tinyui_image_source_t) <= 80 B`。
- ARGB8888 在证明可直接适配且不增加转换缓存前不进入 canonical 资源格式；请求时返回 `NOT_SUPPORTED`。

### 17.2 Font

```c
typedef enum tinyui_builtin_font {
    TINYUI_FONT_6X8,
    TINYUI_FONT_16X24,
    TINYUI_FONT_ARIAL_12,
    TINYUI_FONT_ARIAL_16_A8,
} tinyui_builtin_font_t;

typedef enum tinyui_font_kind {
    TINYUI_FONT_KIND_BUILTIN,
    TINYUI_FONT_KIND_VRES,
} tinyui_font_kind_t;

struct tinyui_font {
    tinyui_font_kind_t kind;
    union {
        tinyui_builtin_font_t builtin;
        uint32_t vres_address;
    } value;
    uintptr_t _private[2];
};

tinyui_result_t tinyui_font_from_builtin(tinyui_builtin_font_t builtin,
                                         tinyui_font_t *out);
tinyui_result_t tinyui_font_from_vres(uint32_t address,
                                      tinyui_font_t *out);
void tinyui_font_deinit(tinyui_font_t *font);
```

约束：

- builtin 一一映射当前已使用的静态 Arm-2D font。
- VRES 复用 `ldBaseGetVresFont()`，不设计 glyph callback、cache 或动态字体引擎。
- font 绑定对象期间必须保持有效。
- `_private` 只保存固定大小 backend handle/storage。
- 32 位 ABI 下 `sizeof(tinyui_font_t) <= 16 B`。
- 需要任意 native font 的高级用户只能使用显式 native 扩展头；该路径不进入 canonical 能力声明。

### 17.3 禁止项

- 不实现 external resource provider。
- 不实现异步加载。
- 不实现图片格式转换缓存。
- 不实现 glyph callback 协议。
- 不在普通 public descriptor 中出现 tile、mask tile、VRES object 或 Arm-2D font 类型。

## 18. 控件能力闭环

覆盖目标是用户能力，不是逐函数 wrapper。

每个控件逐项审查：

- create(parent)。
- props。
- 通用 set/get。
- 专用 set/get。
- event 和 focus。
- theme/style 直接映射。
- layout participation。
- image/font。
- destroy 和错误路径。

复杂控件 canvas、table、keyboard、graph、animation 等不得只做 smoke。

命名统一：

- 删除 `tabel` 错拼。
- 删除 `scroll_selecter` 错拼，改为 `scroll_selector`。
- 删除 `q_r_code` 异常拆词。
- 合并 `set_press/set_pressed` 等重复语义。
- 迁移指南记录旧名到新名，但不承诺旧 ABI。

## 19. 证据等级

| 等级 | 证明内容 |
| --- | --- |
| L1 | canonical 公共声明存在 |
| L2 | 独立消费者可编译并链接 |
| L3 | 参数、错误、容量、所有权和状态语义正确 |
| L4 | 真实 LingDongGUI 对象状态发生预期变化 |
| L5-V | 用户可见能力有真实像素证据 |
| L5-E | 用户可操作能力有真实事件证据 |
| L6 | 目标 port 上的资源、时序和生命周期证据 |

v2.3 要求：

- 所有 required 能力至少达到 L4。
- 可见能力必须达到 L5-V，不能用事件代替像素。
- 可操作能力必须达到 L5-E，不能用截图代替事件。
- 同时可见和可操作的能力必须同时达到 L5-V 与 L5-E。
- L6 明确延期，不纳入 v2.3 核心完成判断。
- 每个矩阵项必须绑定具体测试名或证据文件。

`policy_never_public` 只能用于生命周期、渲染管线、内存、宿主、调试或 backend-private helper，不能隐藏真实用户能力。

## 20. 构建、公共头与安装边界

v2.3 必须建立：

- 从当前 LingDongGUI public headers 生成 inventory 的单一真相源。
- 源码 inventory 漂移 CTest。
- 每个公共头独立 C 编译测试。
- 每个公共头独立 C++ `extern "C"` 编译测试。
- 每个公开函数独立链接消费者。
- `tinyui.h` native/backend 泄漏扫描。
- install/export/find_package。
- 仓库外 C/C++ 最小 consumer。

这些工作提高交付边界，不进入固件运行路径，不得以“轻量”为由省略。

## 21. 测试策略

### 21.1 单元测试

- 参数与范围。
- 对象父子关系和删除。
- timer 回绕、停止和回调内删除。
- callback 固定池容量和 dispatch mutation。
- setter 真实 adapter 选择。
- theme/style descriptor 参数检查。
- resource descriptor 生命周期契约。
- flex/grid 参数转换。

### 21.2 Backend 语义测试

每个公开 setter 必须验证真实 `ld*` 对象状态。只从 TinyUI 缓存读回不算 L4。

### 21.3 视觉与交互测试

- 可见能力逐项绑定像素证据。
- 可操作能力逐项绑定事件证据。
- runtime capture 覆盖全部 demo。
- 像素基线按控件家族和 layout 场景扩展，不要求为每个 API 保存整屏截图，但每项可见能力必须有确定像素断言。

### 21.4 安全测试

- ASan/UBSan 覆盖 core/object/event/timer/resource descriptor。
- 重复 init/deinit 使用 mock 或 core test backend 验证。
- 该测试不宣称真实 PFB/port 重建生命周期已经闭环。
- 容量耗尽和 allocator 失败必须 fail-closed。

## 22. 发布门禁

### 22.1 标准构建

```sh
rtk cmake -S . -B build/v2.3 \
  -DENABLE_TEST=ON \
  -DLD_BUILD_SDL_DEMO=ON \
  -DLD_BUILD_RUNTIME_TESTS=ON \
  -DLD_BUILD_VISUAL_TESTS=OFF \
  -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3 -j
rtk ctest --test-dir build/v2.3 --output-on-failure
```

要求：配置、全量构建和全部注册 CTest 零失败。SDL 在此仅为测试宿主。

### 22.2 Sanitizer

分别使用 Clang ASan 和 UBSan 配置全量 TinyUI core/unit/contract 测试：

```sh
rtk cmake -S . -B build/v2.3-asan \
  -DENABLE_TEST=ON \
  -DLD_BUILD_RUNTIME_TESTS=ON \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_C_FLAGS='-fsanitize=address -fno-omit-frame-pointer'
rtk cmake --build build/v2.3-asan -j
rtk ctest --test-dir build/v2.3-asan --output-on-failure

rtk cmake -S . -B build/v2.3-ubsan \
  -DENABLE_TEST=ON \
  -DLD_BUILD_RUNTIME_TESTS=ON \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_C_FLAGS='-fsanitize=undefined -fno-omit-frame-pointer'
rtk cmake --build build/v2.3-ubsan -j
rtk ctest --test-dir build/v2.3-ubsan --output-on-failure
```

port-production 和真机测试不属于该 profile，但不得排除 TinyUI object、widget、event、timer、theme、layout、resource 和 demo boundary 测试。若当前第三方目标不能在 sanitizer 下运行，必须用明确 CTest label 排除并记录，不能使用宽泛正则跳过 TinyUI 测试。

### 22.3 性能与内存

- M0 生成 `docs/v2.3/v2.3-performance-baseline.md` 和对应机器 JSON。
- binary size 使用第 6 章阈值。
- wrapper struct 对全部控件逐项记录；基础 wrapper 使用 `184 B -> 192 B` 上限，其他 wrapper 使用第 6 章逐项规则。
- 32 位 ABI probe 断言 event pool、timer pool 和 runtime bookkeeping 合计不超过 `1024 B`。
- image source 和 font value descriptor 分别不超过 `80 B`、`16 B`，并通过 backend tile storage ABI probe。
- 最小裁剪 profile 通过 map/nm 断言关闭控件、theme、diagnostics 和 native interop 符号未进入二进制。
- 增加 allocator 计数器，断言 steady-state `tinyui_process()` 隐式分配为 `0`。
- 任一 baseline 缺失、schema 缺项或 checker fallback 都判失败。

宿主时间基线只在环境指纹完全一致时比较。指纹必须记录：

- OS 和版本。
- CPU 型号和架构。
- compiler 名称、版本和 target triple。
- CMake build type、优化级别和完整 TinyUI 选项。
- baseline 对应 git commit。

统计规则：

- 每个场景先 warmup `5` 次。
- 随后独立运行 `30` 次。
- 同时记录 median 和 p95。
- `screen_object_create_ms` 要求 p95 `<= 5 ms`。
- `capture_ready_ms` 要求 p95 `<= 40 ms`，且只作为当前 SDL 测试宿主窄口径 gate。
- 环境指纹不一致时 checker 必须 fail-closed，要求建立新机器基线，不能直接与旧数据比较。

### 22.4 Review 严重度

- P0：崩溃、内存破坏、假成功、用户能力缺失、公共 ABI 无法编译链接。
- P1：真实 backend 语义错误、明显资源回归、公共契约冲突、关键测试缺失。
- P2：不阻断能力的可维护性、诊断或文档问题。

v2.3 关闭时不得存在未解决的 P0/P1 非 port 问题。

### 22.5 人工检查表

`manual_reviewed_passed=true` 必须逐项记录：

- canonical API 无 legacy/native 泄漏。
- 每个 required 能力的 L4/L5 证据可定位。
- demo 未使用固定坐标掩盖 layout/backend 缺口。
- 无第二套 renderer、layout、style 或 resource runtime。
- 未使用 `policy_never_public` 规避用户能力。
- 性能、尺寸和零稳态分配门禁通过。
- port 延期项未被写成完成。

## 23. 阶段架构

v2.3 固定为一次发布窗口、六个可审查里程碑。

### M0 事实基线

- 修复默认全量构建。
- 更新源码 inventory 并纳入 CTest。
- 建立公共头独立编译和公共符号链接门禁。
- 清理只有声明没有实现的 API。
- 冻结尺寸、性能和分配 baseline。

门禁：干净目录全量构建和 CTest 通过，baseline 可重复。

### M1 轻量公共契约

- 落实 runtime、object、result、event、timer、focus、style/theme descriptor、layout track、image/font descriptor 公共头。
- 删除 legacy、错拼和重复 canonical API。
- 建立编译期裁剪选项。
- 用静态检查禁止新增重型 runtime 概念。

门禁：独立 API 与轻量性 review 通过，公共契约冻结。

### M2 核心纵向闭环

- 完成 runtime、对象树、timer 固定池、callback 固定池和直接 setter adapter。
- 以 label、button、checkbox、slider 为纵向样板。
- 每个样板达到 L1-L5。
- 验证对象尺寸和零稳态分配未回归。

门禁：真实 backend、像素、事件、尺寸和分配证据通过。

### M3 全能力迁移

- 完成全部控件的属性与事件直接映射。
- 完成轻量 theme/style、flex/grid、image/font descriptor。
- 清理 native 泄漏、重复 API 和错误返回。
- 保持编译期裁剪有效。

门禁：required 达到 L4，可见/可操作能力达到对应 L5，性能不越线。

### M4 消费者与交付边界

- 全部 demo 迁移到 canonical API。
- 更新 quick start、API、生命周期、事件、style、layout、resource 文档。
- 建立 install/export/find_package 和仓库外 consumer。
- 编写 v2.2 到 v2.3 破坏性迁移指南。

门禁：demo boundary、文档代码编译和安装包消费者通过。

### M5 硬化与冻结

- 全量 CMake、CTest、ASan、UBSan、性能、尺寸和分配 gate。
- 关闭能力矩阵和人工 review。
- 独立代码评审及原 reviewer 跟踪修复。
- GitNexus `detect_changes(compare master)` 复核影响。
- 冻结 v2.3 canonical ABI。

门禁：无未关闭 P0/P1 非 port 问题，发布矩阵与实际证据一致。

## 24. 并行与共享写面

M0、M1、M2 必须串行。M2 公共骨架冻结后，M3 可按无重叠写面并行：

- 控件属性按控件家族分组。
- 控件事件按控件家族分组。
- theme/style descriptor 独立写面。
- flex/grid 独立写面。
- image/font descriptor 独立写面。
- contract 与 evidence matrix 独立写面。

以下共享文件由单一集成负责人维护：

- `tinyui/include/tinyui.h`。
- 公共基础类型和结果码头。
- `tinyui/src/core/widget.c`。
- `tinyui/src/core/internal.h` 及 runtime internal 头。
- capability inventory、ledger 和 release matrix 真相源。

多个 subagent 不得同时修改共享写面。review 不通过的任务由原 reviewer 继续跟踪修复。

## 25. 明确延期的 port 工作

以下不进入 v2.3：

- RGB565/ARGB8888 framebuffer、stride、PFB 配置。
- 同步/异步 flush、DMA ready、buffer fence。
- PFB helper、buffer 和 app target 的销毁重绑。
- MCU tick 单位和 Arm-2D 时间换算。
- `LD_TINYUI_PORT` 选择、SDL 无条件依赖和纯 MCU toolchain。
- SDL event pump、同帧 pointer 边沿、resize 和外部 context。
- SDL/MCU 显示错误传播。
- 平台 monotonic clock provider。
- PFB static buffer、MCU retarget、RTOS、ISR 和 cache。
- 真机板级验证。

与 port 相邻但属于 v2.3 core 的工作仍保留：

- 单实例 canonical runtime。
- app API 用户能力替代。
- core timer 回绕和固定池。
- callback 和 key 的抽象消费语义。
- TinyUI 结果码。
- resource descriptor 对 Arm-2D 类型的隔离。

## 26. GitNexus 风险基线

`tinyui_screen_create`、`tinyui_init`、文本 setter、theme apply 和共享 adapter 是高影响候选，但文档不冻结未经 fresh 查询支持的风险等级。每次修改前必须重新运行 upstream impact，以当次结果记录 direct callers、affected processes 和风险；当次结果为 `HIGH/CRITICAL` 时，先形成调用者迁移清单并告警。每阶段 closeout 运行 `detect_changes()`，最终使用 `compare master`。

## 27. v2.3 文档集

```text
docs/v2.3/
├── README.md
├── 2026-07-13-tinyui-v2-3-unified-core-design.md
├── 线计划索引.md
├── v2.3-capability-evidence-matrix.md
├── v2.3-performance-baseline.md
├── v2.3-migration-guide.md
├── v2.3-release-gates.md
├── deferred-port-work.md
└── plans/
    ├── v2.3-orchestration-plan.md
    └── stages/
        ├── README.md
        ├── m0-truth-baseline-plan.md
        ├── m1-lightweight-public-contract-plan.md
        ├── m2-core-vertical-plan.md
        ├── m3-full-capability-plan.md
        ├── m4-consumer-delivery-plan.md
        └── m5-hardening-freeze-plan.md
```

## 28. 发布声明边界

v2.3 完成后允许声明：

- TinyUI 已形成统一、轻量的 LVGL 风格 API。
- TinyUI required 用户能力已直接映射真实 LingDongGUI backend。
- 普通用户创建控件、布局、事件、theme、图片和字体时不需要接触 Arm-2D 类型。
- API 统一没有突破既定 Flash、RAM 和稳态分配门禁。

仍不允许声明：

- TinyUI 实现了 LVGL 功能集或兼容 LVGL。
- SDL/MCU port 已生产可用。
- DMA、异步 flush、PFB 或目标平台时序已闭环。
- 整个 TinyUI 已达到 L6。

## 29. 完成定义

只有同时满足以下条件，v2.3 才能关闭：

1. M0-M5 全部完成并有 fresh gate 证据。
2. 只剩一套 canonical runtime、object、event、timer、theme 和 resource descriptor API。
3. 不存在第二套 renderer、layout、style selector、事件传播或资源管理 runtime。
4. 所有 demo 和文档只使用 canonical API。
5. required 能力达到 L4，可见和可操作能力达到对应 L5。
6. 全量构建、CTest、ASan、UBSan、安装包 consumer 和文档示例通过。
7. wrapper、backend object、binary size 和 steady-state allocation 未突破硬预算。
8. `manual_reviewed_passed=true` 有逐项人工记录。
9. 没有通过 policy 隐藏真实用户能力。
10. port 延期项仍明确记录，没有被误写成完成。
11. 独立代码评审没有未关闭 P0/P1 非 port 问题。
12. GitNexus 最终影响分析与实际改动范围一致。

在此之前，任何阶段完成都只能描述为 v2.3 内部里程碑，不得描述为 TinyUI 封装已经全部完成。
