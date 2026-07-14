# TINYUI 实现与移植层全面评审

## 1. 评审信息

- 评审日期：2026-07-13
- 评审分支：`dev-nanoui`
- 评审提交：`4d65262`
- 评审范围：`tinyui/include`、`tinyui/src`、`tinyui/port`、`tinyui/demo`、`tinyui_demo`、相关 CMake、单元测试、契约测试、运行时测试、能力清单与现有文档
- 评审目标：判断 TINYUI 是否已经成为一套让用户无需理解 LingDongGUI 和 Arm-2D 内部流程、但仍能完整使用原有控件能力的上层 API

本评审以“能力是否真实可用”为准，不要求 `tinyui_*` 与 `ld*` 一一对应。声明存在、状态可读写、对象真实创建、画面真实变化、事件真实送达、端口可在目标平台落地，是不同层次的证据，不能互相替代。

## 2. 执行结论

当前 TINYUI 不是假渲染器，也不是只有接口壳。它已经建立了真实链路：TINYUI 控件映射到 LingDongGUI 对象，布局映射到 LingDongGUI 布局能力，帧渲染经 `ldGuiDraw()` 和 Arm-2D PFB 输出，再交给 SDL 或 MCU 刷新回调。这是当前实现最重要、也最值得保留的成果。

但当前版本还不能被客观描述为“封装完成”“能力 100% 覆盖”或“用户无需了解 Arm-2D 即可稳定移植”。主要原因不是控件数量不足，而是公共契约尚未收敛，部分通用 API 返回成功却没有作用到真实对象，显示与输入端口存在正确性缺口，MCU 路径仍是 smoke 级脚手架，能力矩阵和契约测试又没有覆盖这些语义问题。

综合成熟度判断：

| 维度 | 当前判断 | 说明 |
| --- | --- | --- |
| 真实 LingDongGUI 映射 | 中等偏高 | 大多数控件、flex/grid、渲染主链已接入真实对象 |
| 公共 API 一致性 | 低 | canonical 与 legacy 两套生命周期冲突，对象、事件、错误模型不统一 |
| 控件能力真实性 | 中等偏低 | 许多专用 API 有效，但若干通用 setter 只更新镜像状态 |
| SDL 端口 | 中等 | 可运行、可演示，但生命周期、尺寸重建、输入时序和错误传播不完整 |
| MCU 端口 | 低 | 能做 host smoke，尚不具备可直接落板的生产契约 |
| 测试与能力证明 | 中等偏低 | 测试数量多，但声明、JSON 自洽和 smoke 多，语义及硬件证据不足 |
| 文档可信度 | 低 | 快速开始、移植规则、历史审计与当前实现互相矛盾 |
| 对外发布就绪度 | 低 | 建议按预览版或内部开发版管理，不冻结 ABI |

若必须给出单一量化值，当前发布就绪度约为 **4/10**。这不是对已有工作的否定，而是说明已经打通“真实后端”，但尚未完成“稳定、诚实、可移植的用户契约”。

## 3. 已经做对的部分

### 3.1 渲染链路是真实的

`tinyui/src/drivers/tinyui_ldgui_disp_adapter.c` 中的帧处理调用 `ldGuiDraw()`，PFB 完成后把真实像素区域交给显示刷新回调。`tinyui_ldgui_neutral_runtime.c` 负责把输入、消息和帧步骤串接起来。当前生产路径没有继续依赖固定坐标的 TINYUI 专属 fake renderer。

因此，现阶段的核心问题应定义为“封装和 port 的正确性不足”，而不是“需要重新写一个渲染器”。后续也不应再引入第二套控件绘制路径。

### 3.2 控件与布局已有广泛映射

控件实现普遍直接创建和操作 `ld*` 对象。`tinyui/src/layout/flex.c` 与 `tinyui/src/layout/grid.c` 也调用真实 LingDongGUI 布局 API，而不是在 demo 中补固定坐标。这符合 TINYUI 作为上层 API 的定位。

### 3.3 测试基础设施已有规模

当前已有大量 TINYUI 单元测试、契约检查、runtime capture、MCU host smoke 和能力矩阵。这些基础设施适合继续扩展，不需要推倒重来。问题在于证据层级没有区分清楚，导致“检查通过”被解释成了超出测试实际证明范围的结论。

### 3.4 demo 边界总体方向正确

已有契约检查限制 demo 使用 `tinyui_*` API。这是必要门槛。后续应继续把 demo 当作公共 API 消费者，而不是 backend 缺口的修补区。

## 4. 关键问题清单

### 4.1 P0：通用属性 API 返回成功，但没有更新真实控件

证据：

- `tinyui/src/core/widget.c:567` 的 `tinyui_widget_set_text()` 只更新 TINYUI 镜像状态。
- 同文件 `tinyui_widget_set_backend_text()` 才执行按控件类型分发的真实 backend 更新，但它没有成为通用 setter 的统一落点。
- `tinyui_widget_set_style_class()`、背景色、文本色、边框、圆角等通用 API 多数只保存字段。
- `tinyui/src/widgets/button.c:204` 和 `tinyui/src/widgets/combo_box.c:246` 的 props 创建流程又依赖这些通用 setter。

影响：用户收到成功返回值，也能从 TINYUI 镜像中读回新值，但真实 LingDongGUI 对象和画面可能没有变化。这是最危险的一类错误，因为测试若只做 set/get round-trip 会稳定通过。

结论：这是阻断发布的问题。任何公开 setter 都必须满足以下三种契约之一，不能处于当前模糊状态：

1. 同步更新真实对象并返回成功。
2. 明确返回“不支持”，且不修改镜像状态。
3. 作为 retained style/property，在下一次确定的 apply 阶段更新真实对象，并有可验证的生命周期说明。

建议优先建立统一属性分发层，让通用属性和控件专用属性最终落到同一个 backend adapter，而不是继续增加镜像字段。

### 4.2 P0：公共生命周期存在两套互不闭合的模型

证据：

- `tinyui/include/tinyui.h:22` 同时公开 `core/app.h`、`core/runtime.h` 和 `core/native.h`。
- `core/app.h` 文件注释称其为内部、非 canonical API，但其中的 `tinyui_app_create()` 等接口仍通过总头文件公开。
- `core/runtime.h` 提供 `tinyui_init()`、screen 和 timer handler，形成类似 LVGL 的全局运行时模型。
- display、indev、tick、OSAL、theme 等 API 又要求传入 `struct tinyui_app *`。
- canonical API 没有稳定方式取得当前 app。
- `tinyui/docs/quick_start.md` 仍推荐 legacy app/window 流程。
- `tinyui/demo/hello_world/hello_world.c:40` 创建 screen 后，把 `tinyui_obj_t *` 强转成 `tinyui_window *`。

影响：用户无法判断哪套 API 会长期保留；port、theme 和 widget 的所有权关系也无法形成一致心智模型。当前同时承诺了“LVGL 风格单例”和“多 app 实例”，而实现中的 current app、PFB、neutral runtime、SDL 状态又大量使用静态全局变量。

建议：当前实现更适合先明确为**单实例、LVGL 风格运行时**。删除总头文件对 legacy app 和 native API 的默认暴露；若 legacy API 暂时保留，应放入明确的兼容头文件并标记迁移期限。不要在静态全局 backend 尚未消除前承诺多实例。

### 4.3 P0：公开声明存在无实现符号

`tinyui/include/widgets/window.h:74` 声明了 `tinyui_window_get_layout_type()` 和 `tinyui_window_get_gap()`，但当前源码中没有实现。头文件扫描和 demo 编译无法发现这种问题，只有完整公共符号链接测试才能证明接口可用。

建议增加“每个公开函数都被独立测试目标引用并完成链接”的 ABI/link gate。仅用正则扫描函数名不能作为公共 API 完整性证据。

### 4.4 P0：ARGB8888 是虚假能力声明

证据：

- `tinyui/include/display/display.h:6` 公开 RGB565 和 ARGB8888。
- `tinyui/src/display/display.c:25` 接受两种格式。
- `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:233` 固定按 RGB565、每像素 2 字节初始化 PFB。
- `tests/tinyui/unit/test_tinyui_port_display.c:63` 只验证配置值可读回，没有验证真实帧格式。

影响：选择 ARGB8888 的 port 会得到与声明不一致的 buffer、stride 和颜色解释，可能造成越界或错误画面。

建议：在真实 32 位渲染链闭环前，公开 API 只接受 RGB565，对 ARGB8888 明确返回不支持。不能把“配置字段可保存”当成格式支持。

### 4.5 P0：显示 flush 契约无法安全支持异步或 DMA

当前 flush callback 返回 `void`，没有 `flush_ready`、fence、完成回调或等待协议。backend 在调用 flush 后立即向 Arm-2D 报告渲染完成，并复用单个 PFB。文档却把启动 DMA 描述为可行用法。

影响：真实 MCU 上若 flush 只启动 DMA 后返回，PFB 可能在 DMA 读取期间被下一帧改写。撕裂和数据竞争都不是偶发实现细节，而是接口契约本身无法表达完成时机。

建议提供两种明确模式：

- 同步 flush：回调返回时数据已经消费完毕。
- 异步 flush：回调收到 token，传输结束后调用 `tinyui_display_flush_ready()`；backend 在 ready 前不得复用对应 buffer。

端口模板必须只展示真实支持的模式。

### 4.6 P0：PFB 生命周期可能复用已释放的 app

`tinyui_ldgui_disp_adapter.c` 中 PFB helper、PFB 内存和初始化标记是静态全局状态。helper 的 target 指向 app。neutral shutdown 只清除自己的初始化标记，没有完整销毁或重绑 PFB；app 随后可以被释放。再次创建 app 时，初始化函数又可能因旧 PFB 内存仍存在而提前返回。

影响：重建运行时后可能继续持有已释放 app，形成 use-after-free。即使公开模型最终选择单实例，也必须支持清晰的 init/deinit 对称性，或明确整个进程不允许重新初始化。

建议把 PFB 资源归属到 backend context，建立严格的 `create -> bind -> run -> drain -> destroy` 生命周期，并对重复初始化、重复销毁、换 display、换 app 写专门测试。

### 4.7 P0：MCU 时间基准存在约 1000 倍偏差风险

`tinyui/src/drivers/tinyui_ldgui_port.c:140` 在 app tick callback 返回毫秒后又乘以 1000。MCU 弱实现的参考频率为 1000 Hz，而 Arm-2D helper 根据该频率换算毫秒。组合后，1 ms tick 可能被解释成约 1000 ms。

影响：动画、超时、惯性和事件节奏在 host smoke 中未必暴露，但落板后会完全错误。

建议只保留一个时间单位契约。公共 port 使用单调毫秒最简单：`uint32_t/uint64_t now_ms()`；Arm-2D 适配层负责一次且仅一次的单位换算。增加已知时间序列测试，例如 0、1、16、1000 ms，直接断言 Arm-2D 观察到的时间。

### 4.8 P0：选择 MCU 或 none 端口仍无条件依赖 SDL

`cmake/LingDongGUI.cmake:258` 在处理 `LD_TINYUI_PORT` 前无条件创建 SDL port target 并查找 SDL，真正的端口选择到后续才发生。

影响：没有 SDL 的 MCU 构建环境无法配置，即使用户从未选择 SDL。这违背 port 分层目标，也使 host smoke 不能证明交叉编译环境可用。

建议按 `LD_TINYUI_PORT` 条件创建和查找依赖；非法值直接 `FATAL_ERROR`，不要静默回退 SDL。CI 至少增加纯 MCU toolchain 配置测试，环境中不安装 SDL。

### 4.9 P0：当前默认全量构建不通过

本次全量 CMake 构建在 `tests/tinyui/unit/test_tinyui_button_events.c:432` 失败：调用了未声明的 `ldGuiClickedAction()`。函数在实现文件中存在，但没有可见声明。

这说明当前仓库不能满足最基本的“默认测试构建全绿”发布门槛。局部目标和部分 CTest 通过不能覆盖此失败。

### 4.10 P1：键盘输入只有状态存储，没有生产消费链路

`tinyui/src/indev/indev.c:48` 可以注册和读取 key 状态，但生产路径没有找到把它转换为 LingDongGUI 键盘事件的消费者。现有测试只验证状态 round-trip。

建议把输入统一建模为有序事件队列，至少覆盖 pointer、key、encoder 三类；backend 每帧按顺序消费，而不是只读取最终状态。

### 4.11 P1：同一轮事件中的按下与抬起会被折叠

SDL 端口一次排空事件队列，并不断覆盖单个 pointer state。backend 随后只读取最终状态。若 mouse down 和 mouse up 在同一轮 pump 中到达，按下边沿可能丢失，点击行为依赖帧时序。

建议保留事件顺序，或至少为边沿建立队列。输入测试应注入 `down -> up`、`down -> move -> up`、多事件同帧、窗口失焦等序列，验证真实控件 callback，而不是只验证最后一个状态值。

### 4.12 P1：初始化顺序可以造成永久无渲染

显示 adapter 在未注册 flush callback 时可以返回成功而不创建 PFB；neutral runtime 随后把自身标为已初始化。若稍后才注册 flush，后续步骤不会重新初始化 PFB。

建议初始化必须是事务性的：必要 port 依赖缺失时返回明确错误，不能把半初始化状态记为成功。也可以要求先注册 display，再允许 `tinyui_init()`，并在 API 上固化顺序。

### 4.13 P1：SDL 重复建窗并改变尺寸可能越界

SDL 状态先更新 width/height，但若旧 buffer 已存在则提前返回，buffer 仍按旧尺寸分配。之后渲染可能按新尺寸访问旧内存。

建议二选一：禁止运行时改变尺寸并返回错误；或实现完整的 renderer、texture、buffer 原子重建。必须用 ASan 覆盖放大、缩小和反复重建。

### 4.14 P1：错误被逐层吞掉

backend step 返回 `void`，PFB 错误、迭代上限、SDL texture/update/present 失败不能可靠传到用户。neutral runtime 最终常返回成功。

建议建立小而稳定的错误域，例如 `TINYUI_OK`、`INVALID_ARG`、`INVALID_STATE`、`NOT_SUPPORTED`、`NO_MEMORY`、`IO_ERROR`、`BACKEND_ERROR`。运行时保留可查询的最后错误和诊断 hook，port 错误不得变成成功。

### 4.15 P1：时钟不是严格单调，timer 也不安全处理回绕

POSIX 路径使用 wall clock，并可能绕过已注册 tick。app timer 以 `now >= next_fire` 比较 32 位时间，回绕时会出错。

建议所有平台只向核心提供单调时钟；32 位毫秒比较使用有符号差值语义，或核心内部统一为 64 位。wall clock 不应参与动画和超时调度。

### 4.16 P1：MCU 路径存在隐藏堆分配和逐帧分配

PFB 使用 `calloc`；app timer 为遍历快照逐帧分配，OOM 时可能静默跳过。对很多 MCU 项目，这既影响确定性，也使内存预算不可审计。

建议支持用户提供静态 PFB、allocator 和容量；timer 使用侵入链表、固定池或可配置静态数组。若仍允许 heap，必须在配置和错误路径中显式体现。

### 4.17 P1：SDL port target 不是完整自包含的运行时包

SDL 所需的 Arm-2D 参考时钟 stub 位于 `tinyui/port/sdl/tinyui_demo_runtime_stubs.c`，当前主要由 demo 单独加入。仅链接 `tinyui_port_sdl` 的普通应用不一定获得完整依赖。

建议 SDL port target 自身提供完整、非 demo 专属的实现；demo 只链接公开 target，不再手工补源文件。这也是验证 port 包边界的最直接方式。

### 4.18 P1：MCU retarget stub 可能与真实 SDK 冲突

MCU target 无条件打包 retarget stub。其中 `_write`、`_read`、`_close`、`_fstat`、`_isatty`、`_lseek` 等 syscall stub 是强符号，可能直接与 SDK 实现冲突；“伪成功”的 IO 或系统调用实现也会掩盖集成错误。

建议 retarget 作为显式可选组件，默认关闭；生产 port 只声明平台必须提供的 hook，不代替 SDK 行为。

### 4.19 P1：资源 API 仍泄漏底层概念

总头文件公开 `core/native.h`，其中暴露 tile、mask、font、native signal 等无类型句柄。image 公共结构还包含 `img_tile`、`mask_tile`、kind、VRES 等字段。内置资源可以隐藏部分 Arm-2D 细节，但自定义图片仍要求用户理解底层资源模型。

建议建立 TINYUI 自有资源类型和创建入口：

- 只读内存图像：像素格式、尺寸、stride、数据指针。
- 外部存储资源：资源 ID、读取回调、缓存策略。
- 字体：内置字体 ID 或 TINYUI 字体描述符。
- backend native handle：只放在高级互操作扩展头中，默认不公开。

目标不是禁止高级用户接触 native，而是保证普通用户完成常用图片、字体和事件需求时不需要 Arm-2D 类型。

### 4.20 P1：对象模型、事件模型和样式模型未统一

当前 creator 有的接收 `tinyui_window *`，有的接收 `tinyui_widget *`，screen 又返回 `tinyui_obj_t *` 并依赖强转。事件主要是控件专用 callback，缺少统一事件码、事件对象、用户数据、冒泡和移除机制。theme 是即时 apply，`style_class` 只保存字符串，状态变化时也没有完整的自动重应用语义。

建议借鉴 LVGL 的是“统一心智模型”，不是机械复制命名：

- 所有可挂载对象统一使用不透明 `tinyui_obj_t *`。
- creator 统一为 `tinyui_xxx_create(tinyui_obj_t *parent)`。
- 通用能力统一走对象 API；特殊能力保留类型专用 API。
- 事件统一为 `tinyui_event_t`、事件码、target/current target、user data。
- style 明确为 retained descriptor，并定义状态选择器和应用时机。
- native backend handle 只在可选互操作 API 中出现。

### 4.21 P1：grid 公共参数使用魔数编码

公共 API 用裸 `int` 数组表示 track；实现中 `0`、`-2` 和其他负数分别代表结束、content 和 FR，最大 track 数又是私有限制。

建议提供命名构造宏或结构：`TINYUI_GRID_PX(n)`、`TINYUI_GRID_FR(n)`、`TINYUI_GRID_CONTENT`、`TINYUI_GRID_END`，并公开或动态处理容量限制。用户不应依赖私有负数协议。

### 4.22 P1：错误返回、范围和命名缺乏稳定规则

部分函数返回 `int/-1`，部分返回 `void`。例如位置允许负数，但某些 getter 又用 `-1` 表示失败。输入尺寸和坐标可从 `int` 静默收窄到 `int16_t`。另有 `tabel`、`selecter`、`q_r_code` 等拼写兼容符号长期并存。

建议 ABI 冻结前统一：

- setter 返回 `tinyui_result_t`。
- getter 通过输出参数返回值，函数本身返回结果码；或为必定有效的对象返回直接值。
- 所有收窄转换先做范围检查。
- 错拼符号只放兼容头并标记弃用，正确拼写成为唯一 canonical API。

### 4.23 P1：嵌套公共头不能保证独立包含

例如 `checkbox.h` 使用 callback 类型却没有自行引入定义，`animation.h` 使用 widget 结构却缺少前置声明。当前公共 API 检查只扫描 `tinyui/include` 顶层的 `*.h`，不会覆盖 `core/`、`widgets/`、`layout/` 等嵌套头。

建议为每个公共头生成一个只包含该头的 C/C++ 编译单元，启用严格警告并纳入 CTest。总头文件能编译不等于每个公开头自洽。

### 4.24 P1：构建 target 暴露了 backend 私有目录

`tinyui_core` 和 backend interface 通过 PUBLIC include directory 暴露 `tinyui/src/core`、`tinyui/src/drivers` 及 LingDongGUI/Arm-2D 相关目录。`tinyui/include/internal/window_internal.h` 还直接包含 `ldWindow.h`。当前也没有规范的 install/export/package 边界。

建议把所有实现目录改为 PRIVATE；用户只获得 `tinyui/include`。native interop 单独形成可选 target/header。增加 install-tree consumer 测试：用一个仓库外最小应用只通过安装后的包完成编译和链接。

### 4.25 P2：runtime handler 固定休眠 16 ms

固定 16 ms 简单但不适合低功耗 MCU，也无法表达 timer 的最近 deadline。建议让 `tinyui_process()` 返回下一次建议唤醒时间，port 决定 sleep、RTOS timer 或事件等待策略。

### 4.26 P2：SDL 端口过度拥有进程级状态

SDL port 内部调用全局 `SDL_Init()`/`SDL_Quit()`，窗口标题固定，难以嵌入已有 SDL 应用。建议支持“由 TINYUI 创建窗口”和“绑定外部 SDL renderer/texture”两种模式，并只释放自己创建的资源。

### 4.27 P2：`display_config.user_data` 没有进入回调

配置允许设置 `user_data`，但刷新回调无法接收或使用它。建议把 context 显式传给所有 port callback，避免端口依赖全局变量。

## 5. Port 分层评审

### 5.1 当前真实调用链

```text
用户 tinyui_* API
        |
        v
TINYUI 对象/属性镜像 + LingDongGUI native 对象
        |
        v
tinyui neutral runtime
        |
        +--> 输入状态/消息 --> LingDongGUI 事件处理
        |
        v
ldGuiDraw() --> Arm-2D PFB --> display flush callback
                                      |
                                      +--> SDL texture/present
                                      |
                                      +--> MCU LCD/DMA（当前契约未闭环）
```

这个方向是正确的。TINYUI 不应拥有自己的绘制器；neutral runtime 也不应变成第二套 GUI 生命周期。它只应负责把稳定的 TINYUI 公共契约映射到唯一真实 backend。

### 5.2 建议的端口边界

建议把 port 拆成四类清晰契约：

| 契约 | 平台提供 | TINYUI/backend 负责 |
| --- | --- | --- |
| 时钟 | 单调毫秒时间 | timer、动画、Arm-2D 单位适配 |
| 显示 | buffer 消费、同步或异步完成通知 | 脏区、PFB、格式和 stride 校验 |
| 输入 | 有序 pointer/key/encoder 事件 | 命中、焦点、事件分发 |
| 系统 | allocator、锁、日志，可选 | 生命周期、容量、错误传播 |

端口不能要求用户调用 `tinyui_backend_neutral_step()` 或包含 `tinyui_ldgui_port.h`。这些都是 backend-private 细节。MCU 示例目前正这样做，说明公共 port API 尚未闭合。

### 5.3 SDL 端口的定位

SDL 端口目前适合作为开发、演示和回归宿主。要成为稳定 port，还需完成：

- target 自包含，不依赖 demo 补 runtime stub。
- 正确处理重复创建、尺寸变化和资源所有权。
- 输入事件保序，并覆盖键盘。
- 错误可以返回或诊断。
- 支持绑定外部 SDL 上下文。
- 用 ASan/UBSan 和真实像素断言验证越界及格式。

### 5.4 MCU 端口的定位

当前 MCU port 更准确的名称是 `temporary smoke path` 或 port 模板，不能描述为生产就绪。它至少还缺：

- 不依赖 SDL 的独立交叉编译配置。
- 明确的同步/异步 LCD flush 契约。
- 可配置静态内存与 allocator。
- 正确且单一的时钟单位。
- RTOS 锁、任务和 ISR 调用边界。
- cache clean/invalidate、buffer 对齐和 DMA 生命周期说明。
- 一块真实开发板上的持续帧、触摸、动画和重建测试证据。

## 6. 对“100% 覆盖”结论的审查

当前清单记录了 629 个 LingDongGUI 公共项，其中 445 项标记为 required/covered，184 项标记为 allowlisted 或 `policy_never_public`。分类本身大体把生命周期、渲染、宿主和内部 helper 与用户能力区分开，方向合理。

但该数字暂时只能证明“冻结 JSON 中每一行都已分类”，不能证明当前源码能力 100% 可通过 TINYUI 使用：

1. LingDongGUI 源码扫描发现冻结清单缺少 `ldBaseGetScreenSize`、`ldBaseGetScreenSizeForScene`、`ldGuiDisposeNodeTree`、`ldTextSetScrollEnabled`。
2. 该源码漂移检查没有进入本次 CTest 默认门禁。
3. TINYUI 公共 API 检查只扫描顶层头文件，忽略嵌套公共头。
4. 声明存在不代表符号可链接；当前已有两个 window getter 只有声明。
5. set/get 镜像一致不代表真实 native 对象变化；通用 setter 已证明存在该盲区。
6. runtime capture 覆盖 25 个 demo，但只有 `hello_world`、`basic_widgets`、`layout_flex` 三个执行严格像素基线比较；其余主要证明启动、截图生成和 marker，不能证明全部视觉与交互语义。
7. release matrix 中 `manual_reviewed_passed` 仍为 false。

建议把能力证据分成六级，矩阵必须记录每项达到的最高级别：

| 级别 | 证据 |
| --- | --- |
| L1 声明 | 公共头中有 canonical API |
| L2 链接 | 独立消费者可编译、链接 |
| L3 状态 | 参数校验、错误和读写语义正确 |
| L4 backend | 真实 LingDongGUI 对象状态发生预期变化 |
| L5 视觉/交互 | 真实渲染像素或真实事件序列符合预期 |
| L6 目标平台 | 在 SDL 和至少一个 MCU port 上验证资源、时序和生命周期 |

“100% 用户能力覆盖”至少要求所有 required 项达到 L4，用户可见/可操作项达到 L5；port 生产声明还需要对应平台达到 L6。

## 7. 推荐的公共封装形态

### 7.1 先选择一个 canonical 模型

建议第一阶段选择单实例模型，与当前静态 backend 和 LVGL 风格目标一致：

```c
tinyui_result_t tinyui_init(const tinyui_config_t *config);
void tinyui_deinit(void);

tinyui_display_t *tinyui_display_create(const tinyui_display_config_t *config);
tinyui_indev_t *tinyui_indev_create(const tinyui_indev_config_t *config);

tinyui_obj_t *tinyui_screen_active(void);
void tinyui_screen_load(tinyui_obj_t *screen);

uint32_t tinyui_process(void); /* 返回建议的下次唤醒毫秒数 */
```

如果未来确实有多实例需求，应先把 current app、PFB、neutral runtime、widget registry 和 port state 全部纳入实例 context，再公开 `tinyui_context_t`。不要先暴露多实例 API，再用静态全局实现。

### 7.2 统一对象，不统一到失去类型能力

所有控件共享 `tinyui_obj_t *` 父子树、位置、尺寸、样式、事件和生命周期。控件专属能力仍保留清晰类型函数，例如 table cell、graph series、keyboard target。这样既接近 LVGL 的易用性，又不需要把每个 `ld*` 函数机械翻译。

### 7.3 统一事件系统

建议最小事件模型包含：

- 事件码：pressed、released、clicked、value_changed、focus、key、delete 等。
- target、current target、user data。
- 可选冒泡。
- add/remove callback。
- 输入事件保持顺序。

控件专用 callback 可以作为语法糖，但应落到同一事件管线，避免每种控件形成不同回调约定。

### 7.4 样式应有可验证的 retained 语义

`style_class` 不能只是字符串仓库。建议明确 style selector：对象类型、class、part、state；明确优先级；对象状态变化时自动重算受影响属性。第一版不必复制 LVGL 完整样式系统，但每个公开属性必须真实落到 LingDongGUI。

### 7.5 backend 隔离不等于可插拔渲染器

建议保留一个私有 backend adapter 层，目的有三点：

1. 隔离 `ld*`、Arm-2D、signal 和 native resource 类型。
2. 集中做属性与事件映射。
3. 允许单元测试观察 native 状态。

它不需要成为通用 renderer plugin，也不应出现 TINYUI 专属绘制实现。当前唯一正式 backend 就是 LingDongGUI。

## 8. 建议整改顺序

### 阶段 0：恢复事实可信度

- 修复默认全量构建。
- 更新 LingDongGUI 源码 inventory，并把漂移扫描纳入 CTest。
- 为所有嵌套公共头增加独立编译测试。
- 为所有公开函数增加链接门禁。
- 修正文档，把历史 fake 路径标为历史，MCU 标为 `temporary smoke path`。
- 删除“629/629 即 100% 完成”式结论，改为分级证据。

验收：干净构建目录下配置、全量构建、全量 CTest 全绿；能力矩阵来自当前源码而非冻结快照。

### 阶段 1：修复运行正确性

- 修复所有通用 setter 的真实 backend 映射。
- 暂停 ARGB8888 声明或实现真实链路。
- 修复 PFB/app 生命周期。
- 修复 MCU tick 单位和单调时钟。
- 建立 flush sync/async 契约。
- 修复输入事件折叠和 key 消费。
- 修复初始化顺序、SDL resize 和错误传播。

验收：每个公开属性至少有 native state 断言；用户可见属性有像素或对象行为断言；生命周期与输入序列在 ASan/UBSan 下通过。

### 阶段 2：收敛 canonical API

- 选择单实例公共模型。
- legacy app API 移入兼容层。
- 统一 `tinyui_obj_t`、事件、错误和资源 API。
- 移除默认 native 头暴露。
- 规范 grid、命名和范围。

验收：所有 demo 只使用 canonical 公共 API，不强转、不前置声明私有函数、不调用 backend helper。

### 阶段 3：把 port 做成可交付组件

- CMake 真正按端口选择依赖。
- SDL target 自包含并支持外部上下文。
- MCU 提供静态内存、DMA、RTOS 和 cache 契约。
- 建立安装包和外部 consumer 测试。

验收：无 SDL 环境可完成 MCU 交叉编译；最小外部工程只链接公开 target；至少一块 MCU 开发板通过持续运行测试。

### 阶段 4：再谈 ABI 和 100% 能力冻结

- 所有 required 能力达到 L4。
- 用户可见和可操作能力达到 L5。
- 声明支持的 port 达到 L6。
- 完成人工 review 并记录不公开项的理由。
- 冻结 canonical API，建立弃用周期和版本策略。

## 9. 发布门槛建议

| 声明 | 当前是否允许 | 达成条件 |
| --- | --- | --- |
| “使用真实 LingDongGUI backend” | 可以 | 当前已有源码证据 |
| “SDL 可用于开发演示” | 可以 | 保留当前限制说明 |
| “大多数 LingDongGUI 控件已有 TINYUI 映射” | 可以，需限定 | 不等同于语义完整 |
| “TINYUI API 已稳定” | 不可以 | canonical、错误、对象、事件模型收敛并冻结 |
| “ARGB8888 已支持” | 不可以 | 真实 32 位 PFB 和像素测试通过 |
| “MCU port 生产可用” | 不可以 | DMA、时钟、内存、RTOS、无 SDL 交叉构建和真机证据 |
| “用户完全无需了解 Arm-2D” | 暂不可以 | 普通资源、显示、输入和构建流程不再泄漏 native 类型或步骤 |
| “用户能力 100% 覆盖” | 暂不可以 | required 达到 L4，可见/可操作项达到 L5，inventory 与源码同步 |

## 10. 本次验证结果

使用独立构建目录 `/tmp/lingdonggui-root-review`，配置参数为：

```sh
cmake -S . -B /tmp/lingdonggui-root-review \
  -DENABLE_TEST=ON \
  -DLD_BUILD_SDL_DEMO=ON \
  -DLD_BUILD_RUNTIME_TESTS=ON \
  -DLD_BUILD_VISUAL_TESTS=OFF \
  -DLD_TINYUI_PORT=sdl
```

结果：

- CMake 配置成功。
- `tinyui_demo` 构建成功。
- `mcu_host_smoke` 构建并运行成功。
- display、input、tick/OS、app timer 四个定向单元测试通过。
- public API、demo boundary、release capability matrix 三个定向契约测试通过。
- 默认全量构建失败，失败点为 `test_tinyui_button_events.c:432` 的 `ldGuiClickedAction()` 缺少声明。
- 直接运行 LingDongGUI public inventory 漂移检查失败，发现四个当前源码符号未进入冻结清单：`ldBaseGetScreenSize`、`ldBaseGetScreenSizeForScene`、`ldGuiDisposeNodeTree`、`ldTextSetScrollEnabled`。

这些结果也说明当前测试体系的核心矛盾：部分 gate 可以全部通过，但全量构建和源码清单仍失败；因此不能只引用局部通过数作为发布结论。

## 11. 最终意见

TINYUI 当前已经跨过“概念验证”阶段：真实控件、真实布局和真实 Arm-2D 输出链已经存在，继续沿这个方向收敛是合理的。没有必要另起一套 renderer，也不应以 demo 视觉补丁替代 backend 完整性。

但它仍处于“后端已打通、公共产品尚未成型”的阶段。下一阶段最高优先级不是继续增加 API 数量，而是让现有 API 诚实：成功就必须作用到真实对象，支持的格式就必须真实渲染，port 声明就必须能在对应环境独立构建，能力矩阵就必须反映当前源码和语义证据。

推荐的总体策略是：**先修真实性和生命周期，再收敛单一 canonical API，然后完成 SDL/MCU port 契约，最后以分级证据重新声明覆盖率。** 在这些门槛完成前，应将 TINYUI 对外定位为开发预览版，并避免冻结 ABI。
