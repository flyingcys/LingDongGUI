# PicoUI v1.0.1 Native ARM-2D Runtime 设计

> 适用仓库：`/Users/cys/embedded/LingDongGUI`  
> 日期：2026-06-05  
> 目标版本：`PicoUI v1.0.1`  
> 路线性质：PicoUI 从 `LingDongGUI backend wrapper` 转为 `native ARM-2D GUI runtime`

## 1. 背景

当前 PicoUI 的主要链路是：

```text
PicoUI public API -> picoui_backend_* -> LingDongGUI ld* 控件 -> ARM-2D
```

这条链路已经证明了 PicoUI public API 能覆盖大量 LingDongGUI 用户可见能力，但 PicoUI 仍不是独立 GUI。它依赖 `picoui/src/backend/ldgui/` 创建和驱动真实控件，测试里也存在 `ld*` backend mapping、runtime marker 和旧 evidence gate。

新的路线要求：PicoUI 完全去掉对 LingDongGUI backend 转换层的依赖，直接拥有自己的 widget tree、layout、event、dirty/render、resource、port 和 ARM-2D 绘制路径。LingDongGUI 旧实现暂时保留在仓库中，只作为迁移参考和对照 oracle，不再进入 PicoUI runtime 构建链。

## 2. 总目标

`v1.0.1` 的目标是发布首个 native-only PicoUI 版本：

```text
PicoUI public API -> PicoUI native runtime/widgets -> ARM-2D -> PicoUI port/display
```

到 `v1.0.1` 时必须满足：

1. PicoUI runtime 构建不链接 `picoui/src/backend/ldgui/*`。
2. PicoUI runtime 不通过 `ld*` 控件实现用户可见控件能力。
3. PicoUI public header、demo、port header 不泄漏 `ld*`、`SIGNAL_*`、LingDongGUI 类型名。
4. PicoUI native runtime 直接管理 widget tree、layout、event、focus、dirty region、render loop。
5. ARM-2D 只作为绘制引擎和像素操作后端，不作为 LingDongGUI 间接入口。
6. public 开发模型尽量接近 LVGL：初始化、display/input 注册、screen 创建/加载、timer handler 主循环，不要求用户显式创建 `app`。
7. 现有 PicoUI public API 的兼容策略必须在 P0 冻结：可保留 wrapper 作为过渡，但 v1.0+ 文档和 demo 不再以 `picoui_app_*` 为主路径。
8. `src/gui/ld*.c` 可保留，但只作参考，不属于 PicoUI 构建依赖。

## 3. 非目标

`v1.0.1` 不做这些事：

1. 不删除仓库里的 LingDongGUI 主体代码。
2. 不一次性改写所有历史 demo 的视觉设计。
3. 不追求像素级复制 LingDongGUI 外观。
4. 不把 LingDongGUI 内部命名简单替换成 `picoui_*` 后宣称完成。
5. 不在 demo 侧硬编码坐标、假控件、fake renderer 来掩盖 native runtime 缺口。
6. 不把 `tests/picoui/runtime/*` 的启动成功当成 GUI native 完成。
7. 不把未迁移控件藏进 allowlist；用户可见控件能力必须进入迁移 ledger。

## 4. 架构原则

### 4.1 PicoUI 是唯一控件运行时

PicoUI native runtime 必须拥有完整控件生命周期：

- create / destroy
- parent / child tree
- geometry / layout
- state / style
- event / focus
- dirty / invalidation
- render / flush

`ldBase_t`、`ldWindow_t`、`ldLabel_t` 等不能作为 PicoUI widget 的真实存储结构。

### 4.2 ARM-2D 是绘制引擎，不是 GUI 抽象

native renderer 可以直接使用 ARM-2D tile、fill、copy、mask、text、blend 能力。ARM-2D 不负责控件语义。控件命中、状态、布局和事件必须在 PicoUI 内部完成。

### 4.3 旧 LingDongGUI 只能作参考

迁移控件时允许阅读和复制 LingDongGUI 控件算法，但落地代码必须：

- 使用 PicoUI 命名和数据结构。
- 去掉 `ld*` public/private 类型依赖。
- 接入 PicoUI widget tree、style、event、render。
- 增加 PicoUI focused test 和 native gate。

### 4.4 先垂直闭环，再批量迁移

不能先复制所有控件骨架再集中修红。第一阶段必须先用 `window + label + button + input + render loop` 跑通真实 native path，然后再扩展控件。

### 4.5 public 模型对齐 LVGL

PicoUI v1.0+ 的上层开发模式应尽量接近 LVGL，降低开发者切换成本。目标模式是：

```c
picoui_init();

struct picoui_display *display = picoui_display_create(width, height);
picoui_display_set_flush_cb(display, flush_cb, user_data);
picoui_display_set_buffers(display, buf1, buf2, buf_size, render_mode);

struct picoui_indev *indev = picoui_indev_create();
picoui_indev_set_type(indev, PICOUI_INDEV_TYPE_POINTER);
picoui_indev_set_read_cb(indev, read_cb, user_data);

struct picoui_screen *screen = picoui_screen_active();
struct picoui_label *label = picoui_label_create(screen, "title");
picoui_label_set_text(label, "Hello PicoUI");

while (1) {
    picoui_timer_handler();
    platform_sleep_ms(5);
}
```

该模式的硬要求：

- 用户不需要 `picoui_app_create()`。
- root UI 挂到 active screen，或由 `picoui_screen_create()` 创建后 `picoui_screen_load()`。
- display、input、tick、OS 是 port/runtime 服务，不依附 public app handle。
- 内部可以有全局 runtime context 或 display-local context，但不暴露为用户必须管理的 `app` 对象。
- `picoui_app_*` 若保留，只能作为 pre-v1.0 compatibility wrapper，不得出现在 v1.0+ quick start、demo 主路径和新测试样例中。

demo 的 `main.c` 也必须按这个模型重写。参考样本固定为：

- `third_party/lv_port_pc_vscode/main/src/main.c`

PicoUI demo 的目标结构应接近：

```c
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    picoui_init();
    hal_init(320, 480);

    create_demo_ui();

    while (1) {
        picoui_timer_handler();
        platform_sleep_ms(5);
    }

    return 0;
}
```

迁移要求：

- `hal_init()` 负责 display、input、tick/OS 等 runtime/port 初始化。
- `create_demo_ui()` 只表达 demo UI，不创建 app，不跑主循环。
- demo 主路径不直接调用 `picoui_app_create()`、`picoui_app_run()`、`picoui_app_destroy()`。
- 第一批只改 `picoui/demo/basic_widgets/main.c` 做样板；样板通过 review 后，再批量迁移其他 `picoui/demo/*/main.c`。
- 在 native runtime API 尚未全部落地前，允许文档先定义目标形态；实现阶段必须通过 P0/P1 明确 compatibility shim 或真实 API，不允许 demo 里手写假接口绕过构建。

### 4.6 证据分层必须重建

旧证据层包含 `ldgui backend mapping`。native-only 后必须替换为：

- public API leak gate
- native runtime smoke gate
- native widget contract gate
- native visible artifact gate
- native input/event gate
- demo boundary gate
- release capability matrix gate

## 5. 目标目录

建议新增或重组为：

```text
picoui/src/native/
  native_runtime.c
  native_widget.c
  native_layout.c
  native_style.c
  native_event.c
  native_input.c
  native_dirty.c
  native_render.c
  native_resource.c
  native_window.c
  native_label.c
  native_button.c
  ...

picoui/include/picoui/internal/
  可选：仅内部使用的 native 私有声明

tests/picoui/native/
  test_picoui_native_runtime.c
  test_picoui_native_screen.c
  test_picoui_native_widget_tree.c
  test_picoui_native_render_window_label_button.c
  test_picoui_native_input_button.c
  ...

tests/picoui/contract/
  check_picoui_no_ldgui_runtime_dependency.py
  check_picoui_native_release_matrix.py
```

保留但最终不进 PicoUI runtime target：

```text
picoui/src/backend/ldgui/
src/gui/ld*.c
```

`picoui/src/backend/ldgui/` 可在迁移期保留为旧 target 或对照 target，但 `v1.0.1` 默认 PicoUI runtime 不得链接它。

## 6. Native Runtime 核心模块

### 6.1 Runtime / Scheduler

职责：

- `picoui_init()` / `picoui_deinit()` 初始化和释放全局 runtime。
- 管理 display list、input device list、timer list、active screen。
- 泵 input、timer、layout、dirty、render、flush。
- 对接 `picoui_display_*`、`picoui_indev_*`、`picoui_tick_*`、`picoui_os_*` port API。

验收：

- `picoui_timer_handler()` 可以在 SDL port 下渲染一帧并输出 artifact。
- `picoui_app_run()` 不再是 v1.0+ 主路径；若保留，只能委托 native runtime compatibility layer。
- timer 行为保持 repeat、one-shot、stop/relink 合同。

### 6.2 Display / Indev / Screen

职责：

- `picoui_display_create()` 创建 display handle。
- `picoui_display_set_flush_cb()`、`picoui_display_set_buffers()`、`picoui_display_set_default()` 管理 display port。
- `picoui_indev_create()` 创建 input handle。
- `picoui_indev_set_type()`、`picoui_indev_set_read_cb()` 管理 input port。
- `picoui_screen_active()` 返回 active screen。
- `picoui_screen_create()` 创建独立 screen。
- `picoui_screen_load()` 切换 active screen。

验收：

- root 控件可以挂到 `picoui_screen_active()`。
- screen 切换不需要 public app handle。
- display/input/tick/os API 不再以 `struct picoui_app *` 作为必需参数。

### 6.3 Widget Tree

职责：

- PicoUI 自有 `struct picoui_widget` 成为唯一 runtime node。
- 管理 parent、first_child、next_sibling、owner display/screen、type、id、user_data。
- 管理 visible、enabled、selectable、focusable、dirty。

验收：

- tree query、parent query、type/name、geometry 读写全部由 PicoUI 自身状态返回。
- 不依赖 backend widget identity。

### 6.4 Layout

职责：

- 实现 absolute、flex、grid、ignore layout、min/max、grow、gap、padding、align。
- 迁移现有 PicoUI layout contract，不再委托 `ldWindowLayoutInternal`。

验收：

- `test_picoui_layout` 中原本依赖 real ld window 的断言改为 native layout 断言。
- flex/grid parity demo 可以通过 native layout 计算出稳定 geometry。

### 6.5 Style / Theme

职责：

- PicoUI theme colors/metrics 映射到 native render style。
- 支持 background、text、border、radius、padding、opacity、state style。
- 去掉 `backend_style_apply.c` 对 ld 控件字段的写入。

验收：

- window/label/button/checkbox/switch/slider/list/image 等控件的基础 style readback 稳定。
- style contract 不再读取 ld private field。

### 6.6 Input / Event / Focus

职责：

- 从 PicoUI input port 读取 pointer/key。
- 执行 hit-test、capture、press/release/click、value change、focus enter/leave。
- 支持 keyboard/key navigation。

验收：

- button 点击能触发 callback。
- enabled/visible/selectable/focus 状态影响 hit-test。
- keyboard、line_edit、scroll/select 类控件后续可复用同一事件核心。

### 6.7 Dirty / Render / Flush

职责：

- 管理 dirty region。
- 对 root tree 做 paint traversal。
- 调 ARM-2D 绘制 primitive、image、mask、font/text。
- 调 PicoUI display flush callback 输出像素。

验收：

- native visible artifact 不依赖 LingDongGUI frame chain。
- render 不调用 `ldGuiFrameStart()`、`ldGuiDraw()`、`ldGuiFrameComplete()`。

### 6.8 Resource

职责：

- 梳理 image、font、canvas、qrcode、animation resource 模型。
- 将当前 `void *tile` 兼容入口收敛成 PicoUI resource handle。
- 保留必要过渡 API，但文档标注底层资源语义。

验收：

- public API 不泄漏 `arm_2d_*` 类型名。
- native renderer 能消费 PicoUI image/font resource。

## 7. 控件迁移范围

`v1.0.1` 必须覆盖当前 PicoUI public 控件集合中已存在的用户可见控件。首批按阶段迁移，不允许把缺口放进 policy 规避。

### 7.1 P0 基线冻结

交付：

- 生成当前 PicoUI public header 控件清单。
- 生成当前 `picoui_app_*` public API 清单，并判定每个 API 是删除、compat wrapper、还是替换为 LVGL-like API。
- 生成当前 demo 清单。
- 生成当前 `picoui/demo/*/main.c` 入口形态清单，标记哪些仍使用 `picoui_app_*`。
- 生成当前 contract matrix 快照。
- 标记所有依赖 `picoui/src/backend/ldgui/*` 的 widget API。

输出：

- `docs/picoui-serial/v1.0-native/00-基线冻结.md`
- `tests/picoui/contract/picoui_native_migration_ledger.json`

### 7.2 P1 Native 垂直切片

范围：

- runtime init/deinit
- display
- indev
- screen
- window
- label
- button
- input pointer
- timer handler / render loop
- SDL port artifact
- `picoui/demo/basic_widgets/main.c` 作为 LVGL-like demo main 样板

成功标准：

- 新 native smoke demo 使用 `picoui_init()`、`picoui_screen_active()`、`picoui_timer_handler()`，不使用 `picoui_app_create()`。
- `picoui/demo/basic_widgets/main.c` 的结构接近 `third_party/lv_port_pc_vscode/main/src/main.c`：`picoui_init()`、`hal_init()`、`create_demo_ui()`、`while(1) picoui_timer_handler()`。
- `basic_widgets/main.c` 不在 demo 主路径直接调用 `picoui_app_create()`、`picoui_app_run()`、`picoui_app_destroy()`。
- `picoui/demo/hello_world` 或新 native smoke demo 可通过 native path 显示 window、label、button。
- button click callback 可验证。
- 构建该 target 时不链接 `picoui/src/backend/ldgui/*`。

### 7.3 P2 Core Widget / Layout / Theme

范围：

- widget tree
- geometry
- visible/enabled/selectable
- focus
- flex
- grid
- theme/style 基础状态

成功标准：

- 原 `test_picoui_layout` 改成 native truth 并通过。
- `layout_parity`、`grid_parity` 在 native path 下可渲染 artifact。

### 7.4 P3 基础控件迁移

范围：

- checkbox
- switch
- slider
- text
- image
- list
- background

成功标准：

- 每个控件都有 focused native unit test。
- 每个控件至少有一个 visible artifact 或 demo 覆盖。
- 原 backend mapping 断言被 native widget contract 替代。

### 7.5 P4 扩展控件迁移

范围：

- arc
- calendar
- clock
- combo_box
- date_time
- gauge
- graph
- icon_slider
- keyboard
- line_edit
- message_box
- progress_bar
- progress_wheel
- qrcode
- radial_menu
- scroll_selecter
- table
- animation
- canvas

成功标准：

- 所有控件 create/set/get/event/render 的核心 user-facing ability 进入 migration ledger。
- 不允许只迁 create 空壳。
- 复杂控件允许分子阶段，但 `v1.0.1` 前 ledger 不得有 user-facing missing。

### 7.6 P5 Resource / Font / Image / Text 完整化

范围：

- ARM-2D font bridge
- text measurement
- image tile/mask/source
- vres 兼容
- canvas primitive
- qrcode bitmap
- animation frame source

成功标准：

- 文本控件和 label/button/list/table 等复用同一 text renderer。
- image/mask 不再借用 ldImage 控件逻辑。
- resource API 文档明确 portable 层和 ARM-2D 兼容层。

### 7.7 P6 Demo / Port / Board 验证

范围：

- SDL native runtime
- manual artifact
- demo boundary
- legacy parity demos
- 将其余 `picoui/demo/*/main.c` 批量迁移到 `basic_widgets` 样板结构
- 后续 board port 准备

成功标准：

- PicoUI demo target 默认使用 native runtime。
- 所有 v1.0+ demo main 都采用 LVGL-like 结构：init、HAL、UI 创建、timer handler loop。
- 所有 v1.0+ demo main 不以 `picoui_app_*` 为主路径。
- `picoui/port/sdl/` 不承载控件 fake 逻辑。
- manual artifact 标注 native runtime，而不是 ldgui path。

### 7.8 P7 删除 PicoUI 对 ldgui backend 的构建依赖

范围：

- CMake target 清理。
- 测试 target 清理。
- runtime/mapping gate 替换。
- public docs 改口径。

成功标准：

- 默认 PicoUI library/runtime 不编译 `picoui/src/backend/ldgui/*.c`。
- `check_picoui_no_ldgui_runtime_dependency.py` 通过。
- `rg "ld[A-Z]|SIGNAL_|ldGui|ldBase|ldWindow|ldLabel|ldButton" picoui/src/native picoui/include/picoui picoui/demo` 不命中违规项。

### 7.9 P8 v1.0.1 Release Gate

范围：

- contract matrix 收口。
- native migration ledger 清零 user-facing missing。
- release notes。
- upgrade guide。
- closeout。

成功标准：

- `v1.0.1` 明确声明 PicoUI native-only。
- 发布说明区分“仓库仍保留 LingDongGUI 参考代码”和“PicoUI runtime 不再依赖 LingDongGUI”。
- 所有 gate 有 fresh verification 证据。

## 8. Contract 与测试要求

### 8.1 必须新增的 contract

- `check_picoui_no_ldgui_runtime_dependency.py`
- `check_picoui_native_public_api_leak.py`，可在现有 public API leak check 基础上扩展。
- `check_picoui_native_migration_ledger.py`
- `check_picoui_native_release_matrix.py`
- `check_picoui_demo_main_style.py`：检查 v1.0+ demo main 是否使用 LVGL-like 入口结构，且不把 `picoui_app_*` 作为主路径。

### 8.2 必须新增的 focused tests

- native runtime init/deinit
- native display/indev/screen
- native widget tree
- native geometry/layout
- native dirty/render
- native input hit-test
- native button event
- native text renderer
- native image renderer
- 每个控件一个 focused create/set/get/render test

### 8.3 必须替换的旧 gate

旧 gate：

- `check_picoui_backend_mapping`
- 依赖 ld field/readback 的 mapping/unit tests
- 只证明 LingDongGUI scene 存在的 runtime marker

替换为：

- native widget tree mapping
- native render artifact
- native input/event trace
- native release matrix

## 9. 文档要求

新增文档：

- `docs/picoui-serial/v1.0-native/00-基线冻结.md`
- `docs/picoui-serial/v1.0-native/01-native垂直切片.md`
- `docs/picoui-serial/v1.0-native/02-core-layout-theme.md`
- `docs/picoui-serial/v1.0-native/03-基础控件迁移.md`
- `docs/picoui-serial/v1.0-native/04-扩展控件迁移.md`
- `docs/picoui-serial/v1.0-native/05-resource-font-image.md`
- `docs/picoui-serial/v1.0-native/06-demo-port验证.md`
- `docs/picoui-serial/v1.0-native/07-ldgui构建解绑.md`
- `docs/picoui-serial/v1.0-native/08-v1.0.1发布门禁.md`
- `docs/picoui-serial/v1.0-native/线计划索引.md`

更新文档：

- `docs/ability/README.md`
- `docs/ability/*.md`
- `picoui/docs/quick_start.md`
- `picoui/docs/porting_rules.md`
- `picoui/docs/api_overview.md`
- release notes / upgrade guide

文档口径：

- 可以说 `PicoUI v1.0.1 native runtime 不依赖 LingDongGUI backend`。
- 不能说 `仓库已删除 LingDongGUI`。
- 不能把单个 vertical slice 写成全控件完成。
- 不能把 artifact existence 写成视觉验收通过。

## 10. 构建策略

迁移期允许同时存在两个 target：

- `picoui_legacy_ldgui`：旧 backend，对照用，可选构建。
- `picoui_native_arm2d`：新 runtime，v1.0.1 默认路径。

到 P7 后：

- 默认 `picoui` target 指向 native runtime。
- ldgui backend 不进默认 PicoUI runtime。
- legacy target 若保留，必须显式 opt-in。

建议 CMake 开关：

```cmake
PICOUI_RUNTIME=native_arm2d
PICOUI_ENABLE_LEGACY_LDGUI=OFF
```

`v1.0.1` release 构建必须使用：

```text
PICOUI_RUNTIME=native_arm2d
PICOUI_ENABLE_LEGACY_LDGUI=OFF
```

## 11. 风险与约束

### 11.1 最大风险

最大风险不是 ARM-2D 绘制，而是 LingDongGUI 多年积累的控件状态机、layout、输入、资源细节被低估。若只复制代码改名，很容易得到一套难维护的半成品。

### 11.2 控制方式

- 每个阶段都有 ledger 和 focused test。
- 每个控件按 user-facing ability 迁移，不按源文件复制进度宣称完成。
- 先 native 垂直切片，再扩展控件。
- 保留旧 LingDongGUI 作为 oracle，直到 v1.0.1 release gate 全绿。

### 11.3 不可接受的完成口径

- “编译通过，所以迁移完成。”
- “控件能创建，所以控件完成。”
- “demo 能弹窗，所以 runtime 完成。”
- “旧 ldgui mapping gate 通过，所以 native 完成。”
- “ledger allowlist 清爽，所以缺口不存在。”

## 12. v1.0.1 完成定义

`v1.0.1` 只能在以下条件全部满足后发布：

1. 默认 PicoUI runtime 不链接 `picoui/src/backend/ldgui/*`。
2. 当前 PicoUI public 控件 user-facing ability 在 native migration ledger 中全部 `covered` 或明确为非用户能力。
3. 所有 demo boundary 通过，demo 不泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
4. native runtime/layout/render/input/control focused tests 通过。
5. native visible artifact 可生成，并明确标注 native path。
6. release matrix 与 docs/ability 同步。
7. `picoui/docs` 已按 native-only 使用方式更新。
8. 发布说明明确保留 LingDongGUI 参考代码的边界。
9. v1.0+ quick start 和 demo 主路径不使用 `picoui_app_create()` / `picoui_app_run()`。
10. 所有 v1.0+ demo main 入口已按 `basic_widgets` 样板完成 LVGL-like 改写。

## 13. 下一步

本 spec 批准后，下一步拆 implementation plan。计划必须按 `P0 -> P8` 严格串行，每阶段可由 subagent 执行和 review，但写面不得重叠。

首个实现阶段只做 P0/P1：

1. 冻结当前 PicoUI API、demo、contract、ldgui dependency 基线。
2. 冻结并决策 `picoui_app_*` compatibility 策略。
3. 建 LVGL-like native runtime 最小垂直切片。
4. 用 `init + display + indev + screen + window + label + button + timer_handler` 证明 native path 真实可运行。
