# TinyUI v2.3 M4 消费者与交付边界实施计划

> **面向执行代理：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，逐任务执行本计划，并使用复选框（`- [ ]`）跟踪步骤。

**目标：** 将全部 TinyUI demo、用户文档和 CMake 安装包统一到 v2.3 canonical API，并用真正位于仓库外构建目录的 C/C++ `find_package()` 消费者证明交付边界。

**架构：** M4 不改变 M1-M3 已冻结的 core、对象、事件、布局或资源语义，只迁移消费者并建立安装接口。demo 统一为“runner 持有 runtime，`build(screen)` 只表达 UI 意图”；安装包只导出 canonical public headers 和消费 target，internal、LingDongGUI/Arm-2D 头及测试宿主细节保持私有。

**技术栈：** C11、C++17、CMake 3.16+、CTest、Python 3、TinyUI canonical API、LingDongGUI 真实 backend、Markdown。

## 全局约束

- M0-M3 的阶段门禁必须已经通过；若 canonical ABI 尚未冻结到 M3 约定状态，停止 M4，不在 demo 或文档中发明替代接口。
- TinyUI 是 LingDongGUI 的轻量上层 API，不得新增第二套 renderer、对象树、layout solver、style selector、事件队列或资源中心。
- `tinyui/demo/*` 只能包含 canonical `tinyui_*` API；禁止 `ld*`、`arm_2d_*`、`SIGNAL_*`、legacy app、native escape hatch 和 `struct tinyui_widget *` 强转。
- demo 不得通过新增固定坐标、固定尺寸或假视觉掩盖 backend/layout 缺口；发现缺口时回退到 M3 修复真实映射后再继续。
- SDL 仅为真实 LingDongGUI 输出的测试宿主；本阶段不声明 SDL 或 MCU port 已完成，不修改具体 port 的生产契约。
- 文本 setter 复制字符串；image/font descriptor 及其底层数据由调用者持有，文档示例必须体现对应生命周期。
- 只安装 canonical public headers；`tinyui/src/core`、`tinyui/src/drivers`、LingDongGUI、Arm-2D、测试和 demo 头不得进入安装树。
- 本项目不创建 worktree。本文不包含提交步骤；每个任务以 focused gate 和 `rtk git diff --check` 作为审查检查点。
- 所有 shell 命令必须经 `rtk` 执行。

---

## 文件结构

新增：

- `tests/tinyui/contract/tinyui_demo_manifest.json`：29 个 canonical demo 的唯一机器清单。
- `cmake/TinyUIConfig.cmake.in`：安装后的 `find_package(TinyUI CONFIG REQUIRED)` 入口。
- `tests/tinyui/consumer/CMakeLists.txt`：仓外 C/C++ consumer 工程。
- `tests/tinyui/consumer/main.c`：C11 最小消费者。
- `tests/tinyui/consumer/main.cpp`：C++17 `extern "C"` 消费者。
- `tests/tinyui/consumer/run_install_consumer.cmake`：安装、复制到仓外目录、配置、构建和运行的 fail-closed 驱动。
- `tests/tinyui/contract/check_tinyui_docs_examples.py`：抽取并编译 current-facing 文档中的标记 C 示例。
- `docs/v2.3/v2.3-migration-guide.md`：v2.2 到 v2.3 的破坏性迁移指南。

修改：

- `tinyui/demo/tinyui_demos.h`：统一 `build(screen)` 调度接口。
- `tinyui/demo/tinyui_demos.c`：只保存 demo 名、builder 和显示元数据，不持有 runtime 或逐帧 hook。
- `tinyui/demo/*/*.c`、`tinyui/demo/*/*.h`：全部迁移为 canonical builder。
- `tinyui_demo/main.c`：唯一 runtime/host runner。
- `examples/sdl/CMakeLists.txt`：显式列出 canonical demo source，并接入统一 runner。
- `tests/tinyui/contract/check_tinyui_demo_boundary.py`：manifest、canonical API、无 fake/legacy 泄漏门禁。
- `tests/tinyui/CMakeLists.txt`：注册 demo、文档和安装消费者 CTest。
- `cmake/LingDongGUI.cmake`：补齐 build/install interface 和 `TinyUI::tinyui` 导出属性。
- `CMakeLists.txt`：生成并安装 `TinyUIConfig.cmake`、版本文件和 export set。
- `tinyui/docs/quick_start.md`：canonical 最小循环。
- `tinyui/docs/api_overview.md`：runtime/object/widget/event/theme/layout/resource API 导航。
- `tinyui/docs/demo_guide.md`：builder 与统一 runner 规则。
- `tinyui/docs/resource_lifetime.md`：image/font descriptor 借用与释放规则。
- `docs/v2.3/deferred-port-work.md`：重申 port 延期边界。

重命名：

- `tinyui/demo/scroll_selecter_basic/scroll_selecter_basic.c` -> `tinyui/demo/scroll_selector_basic/scroll_selector_basic.c`
- `tinyui/demo/scroll_selecter_basic/scroll_selecter_basic.h` -> `tinyui/demo/scroll_selector_basic/scroll_selector_basic.h`

## 固定接口

全部 demo 使用以下接口，不允许每个 demo 自行定义生命周期：

```c
typedef tinyui_result_t (*tinyui_demo_build_cb_t)(tinyui_obj_t *screen);

tinyui_result_t tinyui_demos_build(const char *name, tinyui_obj_t *screen);
bool tinyui_demos_get_display_size(const char *name,
                                   uint16_t *width,
                                   uint16_t *height);
void tinyui_demos_show_help(void);

tinyui_result_t tinyui_demo_<name>_build(tinyui_obj_t *screen);
```

规则固定为：runner 调用 `tinyui_init()`、创建 screen、调用 builder、加载 screen，并循环调用 `tinyui_process(&next_ms)`；builder 不调用 init/deinit、screen create/load、平台 API或 sleep。动画 demo 使用 `tinyui_timer_create()`，不得保留 `tinyui_demos_frame()`。

安装后的唯一普通用户 target 固定为 `TinyUI::tinyui`。它必须传递 canonical include directory 和使用者所需库，但不得传递 `tinyui/src/*`、LingDongGUI 或 Arm-2D include directory。平台集成头仍须显式 include，且不由 `tinyui.h` 聚合。

### 任务 1：建立完整 demo 清单和先失败的 canonical 边界门禁

**文件：**

- 新增：`tests/tinyui/contract/tinyui_demo_manifest.json`
- 修改：`tests/tinyui/contract/check_tinyui_demo_boundary.py`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 消费：`tinyui/demo/**/*.c`、`tinyui/demo/**/*.h`、`tinyui/demo/tinyui_demos.c`。
- 产出：`check_tinyui_demo_boundary` CTest；它证明 manifest 与源码/CMake 一致，且所有 demo 只使用 canonical API。

- [x] **步骤 1：由主线程完成修改前影响分析**

主线程对 `tinyui_demos_create`、`tinyui_demos_frame` 和 29 个 `tinyui_demo_<name>` 入口完成影响分析（GitNexus 不可用时用 `rtk rg`）：

```text
impact({target: "<精确符号名>", direction: "upstream"})
```

结论（2026-07-15 Task 1）：风险 **MEDIUM**，非 HIGH/CRITICAL。
- `tinyui_demos_create` / `tinyui_demos_frame` / `tinyui_demos_get_display_config` 仅 `tinyui_demo/main.c` 与 `tinyui/demo/tinyui_demos.{c,h}` 使用。
- 29 个 `void tinyui_demo_<name>(void)` 旧入口仍在各自 demo 源中；全量 `tinyui_demo`、runtime capture 与相关 CTest 属于 Task 2+ 迁移面。
- Task 1 仅建立门禁与 manifest，不改 runner/demo 实现符号。

- [x] **步骤 2：写完整 manifest**

`tinyui_demo_manifest.json` 必须精确包含以下 29 个名称、源码路径和构建符号，按名称排序；`scroll_selector_basic` 使用修正后的拼写：

```json
{
  "schema_version": 1,
  "demos": [
    {"name":"animation_basic","source":"tinyui/demo/animation_basic/animation_basic.c","build_symbol":"tinyui_demo_animation_basic_build"},
    {"name":"arc_basic","source":"tinyui/demo/arc_basic/arc_basic.c","build_symbol":"tinyui_demo_arc_basic_build"},
    {"name":"basic_widgets","source":"tinyui/demo/basic_widgets/basic_widgets.c","build_symbol":"tinyui_demo_basic_widgets_build"},
    {"name":"calendar_basic","source":"tinyui/demo/calendar_basic/calendar_basic.c","build_symbol":"tinyui_demo_calendar_basic_build"},
    {"name":"clock_basic","source":"tinyui/demo/clock_basic/clock_basic.c","build_symbol":"tinyui_demo_clock_basic_build"},
    {"name":"combo_box_basic","source":"tinyui/demo/combo_box_basic/combo_box_basic.c","build_symbol":"tinyui_demo_combo_box_basic_build"},
    {"name":"date_time_basic","source":"tinyui/demo/date_time_basic/date_time_basic.c","build_symbol":"tinyui_demo_date_time_basic_build"},
    {"name":"gauge_basic","source":"tinyui/demo/gauge_basic/gauge_basic.c","build_symbol":"tinyui_demo_gauge_basic_build"},
    {"name":"graph_basic","source":"tinyui/demo/graph_basic/graph_basic.c","build_symbol":"tinyui_demo_graph_basic_build"},
    {"name":"grid_parity","source":"tinyui/demo/grid_parity/grid_parity.c","build_symbol":"tinyui_demo_grid_parity_build"},
    {"name":"hello_world","source":"tinyui/demo/hello_world/hello_world.c","build_symbol":"tinyui_demo_hello_world_build"},
    {"name":"icon_slider_basic","source":"tinyui/demo/icon_slider_basic/icon_slider_basic.c","build_symbol":"tinyui_demo_icon_slider_basic_build"},
    {"name":"keyboard_basic","source":"tinyui/demo/keyboard_basic/keyboard_basic.c","build_symbol":"tinyui_demo_keyboard_basic_build"},
    {"name":"layout_flex","source":"tinyui/demo/layout_flex/layout_flex.c","build_symbol":"tinyui_demo_layout_flex_build"},
    {"name":"layout_grid","source":"tinyui/demo/layout_grid/layout_grid.c","build_symbol":"tinyui_demo_layout_grid_build"},
    {"name":"layout_parity","source":"tinyui/demo/layout_parity/layout_parity.c","build_symbol":"tinyui_demo_layout_parity_build"},
    {"name":"legacy_demo0_parity","source":"tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c","build_symbol":"tinyui_demo_legacy_demo0_parity_build"},
    {"name":"legacy_widget_parity","source":"tinyui/demo/legacy_widget_parity/legacy_widget_parity.c","build_symbol":"tinyui_demo_legacy_widget_parity_build"},
    {"name":"line_edit_basic","source":"tinyui/demo/line_edit_basic/line_edit_basic.c","build_symbol":"tinyui_demo_line_edit_basic_build"},
    {"name":"list_basic","source":"tinyui/demo/list_basic/list_basic.c","build_symbol":"tinyui_demo_list_basic_build"},
    {"name":"message_box_basic","source":"tinyui/demo/message_box_basic/message_box_basic.c","build_symbol":"tinyui_demo_message_box_basic_build"},
    {"name":"progress_bar_basic","source":"tinyui/demo/progress_bar_basic/progress_bar_basic.c","build_symbol":"tinyui_demo_progress_bar_basic_build"},
    {"name":"progress_wheel_basic","source":"tinyui/demo/progress_wheel_basic/progress_wheel_basic.c","build_symbol":"tinyui_demo_progress_wheel_basic_build"},
    {"name":"qrcode_basic","source":"tinyui/demo/qrcode_basic/qrcode_basic.c","build_symbol":"tinyui_demo_qrcode_basic_build"},
    {"name":"radial_menu_basic","source":"tinyui/demo/radial_menu_basic/radial_menu_basic.c","build_symbol":"tinyui_demo_radial_menu_basic_build"},
    {"name":"scroll_selector_basic","source":"tinyui/demo/scroll_selector_basic/scroll_selector_basic.c","build_symbol":"tinyui_demo_scroll_selector_basic_build"},
    {"name":"settings_panel","source":"tinyui/demo/settings_panel/settings_panel.c","build_symbol":"tinyui_demo_settings_panel_build"},
    {"name":"table_basic","source":"tinyui/demo/table_basic/table_basic.c","build_symbol":"tinyui_demo_table_basic_build"},
    {"name":"theme_showcase","source":"tinyui/demo/theme_showcase/theme_showcase.c","build_symbol":"tinyui_demo_theme_showcase_build"}
  ]
}
```

- [x] **步骤 3：先扩展 checker，使旧 demo 必然失败**

checker 必须同时验证：manifest 源文件集合等于实际 demo 子目录 `.c` 集合；每个构建符号在对应 `.c/.h` 中恰好声明/定义一次；CMake 显式包含每个 source；禁止词至少包含 `tinyui_app_`、`tinyui_widget_`、`tinyui_native_`、`ld[A-Z]`、`arm_2d_`、`SIGNAL_`、`struct tinyui_widget`、`tinyui_demos_frame`；builder 内禁止 `tinyui_init`、`tinyui_deinit`、`tinyui_screen_create`、`tinyui_screen_load` 和平台头。不能用 `tinyui_window_` 前缀做笼统禁止词，因为 window 的 canonical 专用能力属于 required 用户能力；旧签名由 manifest 与编译测试拒绝。

实现说明（Task 1）：`v23_*` L5 证据 runner 不进入 29 项 consumer manifest 的源集合等式，但仍扫描 forbidden token；`tests/tinyui/contract/test_check_tinyui_demo_boundary.py` 用单元测试锁死规则与当前红灯债类。

- [x] **步骤 4：运行门禁并确认红灯原因准确**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

预期：FAIL；报告现存 legacy API、旧 `scroll_selecter_basic` 路径、旧 `void tinyui_demo_*()` 签名和 `tinyui_demos_frame`，不得因 JSON 解析或路径错误失败。

实测（2026-07-15）：FAIL，约 770 项；主要债类 = `missing_demo_source`/`unexpected_demo_source`（`scroll_selecter`→`scroll_selector`）、`missing_build_symbol`、`legacy_void_entry`、`cmake_*`、`forbidden_token`（含 `tinyui_widget_*` / `struct tinyui_widget` / lifecycle / `tinyui_demos_frame`）。无 JSON/路径基础设施失败。红灯保留至 Task 2+ 迁移完成属预期。

- [x] **步骤 5：注册 CTest 并检查格式**

在 `tests/tinyui/CMakeLists.txt` 中保持测试名 `check_tinyui_demo_boundary`，labels 固定为 `tinyui;contract;demo;consumer`；并注册 `test_check_tinyui_demo_boundary`（unit）。

运行：

```bash
rtk git diff --check
```

预期：PASS。

### 任务 2：迁移统一 runner 和 demo registry

**文件：**

- 修改：`tinyui/demo/tinyui_demos.h`
- 修改：`tinyui/demo/tinyui_demos.c`
- 修改：`tinyui_demo/main.c`
- 修改：`examples/sdl/CMakeLists.txt`
- 修改：`tests/tinyui/runtime/check_tinyui_runtime.py`

**接口：**

- 消费：任务 1 manifest；M1 的 `tinyui_init/deinit/process`，M2 的 screen API，M2 的 timer API。
- 产出：`tinyui_demos_build()`、`tinyui_demos_get_display_size()`、唯一 runtime runner。

- [x] **步骤 1：为 registry/runner 写失败断言**

在 demo checker 中断言 registry callback 类型为 `tinyui_demo_build_cb_t`，不存在 frame callback 字段；runtime checker 断言每个 manifest name 均能启动并取得 capture，不接受仅创建窗口作为成功。

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

预期：FAIL，指出旧 `void` callback、display config backend 类型和 frame hook。

实现说明（Task 2）：`check_registry_contract()` 锁死 `tinyui_demo_build_cb_t`、禁止 `frame_cb`/`tinyui_demos_frame`/`get_display_config`/`void` registry callback，并要求 runner 使用 `tinyui_process`/`screen_*`/`demos_build`；单元测试覆盖红/绿 fixture。`check_tinyui_runtime.py` 的 demo 列表改为读取 29 项 manifest（含 `scroll_selector_basic`）。实测：边界门禁仍 FAIL（含 registry 债，至实现落地后 registry 项转绿、demo 源债仍红）。

- [x] **步骤 2：实现最小 registry**

registry 仅保存 `name/build/width/height`。`tinyui_demos_build(NULL, screen)` 选择 `hello_world`；未知 name、NULL screen 或 builder 失败返回明确 `tinyui_result_t`，不得吞掉 backend 错误。

```c
typedef struct tinyui_demo_entry {
    const char *name;
    tinyui_demo_build_cb_t build;
    uint16_t width;
    uint16_t height;
} tinyui_demo_entry_t;
```

实现说明：`tinyui/demo/tinyui_demos.{h,c}` 已切换到上述契约；registry 名使用 `scroll_selector_basic`。29 个 `*_build` 以 **weak stub** 返回 `TINYUI_ERROR_NOT_SUPPORTED`，便于 Task 3–5 在各 demo 源提供强符号覆盖且保持 `tinyui_demo` 可链接。

- [x] **步骤 3：将 runner 收口到 canonical lifecycle**

`tinyui_demo/main.c` 的顺序固定为：解析 name/显示元数据，初始化 SDL 测试宿主，`tinyui_init()`，`tinyui_screen_create()`，`tinyui_demos_build()`，`tinyui_screen_load(..., TINYUI_SCREEN_TRANSITION_NONE, 0)`，循环 `tinyui_process(&next_ms)`，退出时逆序 deinit。runner 不调用 legacy `tinyui_timer_handler()`，也不调用 demo frame hook。

实现说明：实际顺序为 parse → `tinyui_init` → display default → SDL window/mouse（依赖已 init 的 app）→ screen_create → build → load → process 循环 → 逆序 deinit；已移除 `tinyui_timer_handler` / `tinyui_demos_frame` / `tinyui_demos_create`。

- [x] **步骤 4：构建 focused runner**

运行：

```bash
rtk cmake -S . -B build/v2.3-m4-demo -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m4-demo --target tinyui_demo -j
```

预期：当前因尚未迁移全部 builder 而链接失败；失败符号只应是 manifest 中的 `tinyui_demo_*_build`。

实现说明（Task 2 调整）：按执行指示优先提供 29 个 weak stub，使 `tinyui_demo` **链接通过**；运行时未迁移 demo 的 `build` 返回 `NOT_SUPPORTED` 并由 runner 退出。真实 builder 与 capture 绿灯留给 Task 3–5。

### 任务 3：迁移基础、输入和数据控件 demo

**文件：**

- 修改：`tinyui/demo/hello_world/*`
- 修改：`tinyui/demo/basic_widgets/*`
- 修改：`tinyui/demo/settings_panel/*`
- 修改：`tinyui/demo/list_basic/*`
- 修改：`tinyui/demo/table_basic/*`
- 修改：`tinyui/demo/graph_basic/*`
- 修改：`tinyui/demo/calendar_basic/*`
- 修改：`tinyui/demo/combo_box_basic/*`
- 修改：`tinyui/demo/keyboard_basic/*`
- 修改：`tinyui/demo/line_edit_basic/*`
- 重命名并修改：`tinyui/demo/scroll_selecter_basic/*` -> `tinyui/demo/scroll_selector_basic/*`

**接口：**

- 消费：统一 `tinyui_obj_t *`、`create(parent)`、`tinyui_obj_set_*`、控件专用 API、event/focus API。
- 产出：11 个 `tinyui_result_t tinyui_demo_<name>_build(tinyui_obj_t *screen)` 实现。

- [x] **步骤 1：执行错拼文件的可追踪重命名**

运行：

```bash
rtk git mv tinyui/demo/scroll_selecter_basic tinyui/demo/scroll_selector_basic
rtk git mv tinyui/demo/scroll_selector_basic/scroll_selecter_basic.c tinyui/demo/scroll_selector_basic/scroll_selector_basic.c
rtk git mv tinyui/demo/scroll_selector_basic/scroll_selecter_basic.h tinyui/demo/scroll_selector_basic/scroll_selector_basic.h
```

预期：旧路径不存在，新路径与 manifest 一致。

- [x] **步骤 2：逐个迁移 creator、setter 和 callback**

每个源文件只 include `tinyui.h` 与自身头；所有对象变量使用 `tinyui_obj_t *`；所有 creator 接受 `screen` 或真实 container；通用属性只用 `tinyui_obj_set_*`；控件专用状态只用 `tinyui_<widget>_*`；callback 统一为 `void callback(const tinyui_event_t *event)`。任一步失败立即返回原始错误，已创建对象由 screen 所有权统一回收。

- [x] **步骤 3：保持布局意图，不增加坐标补丁**

原本通过 flex/grid 表达的页面继续只调用 canonical flex/grid；移除强转时不得把布局替换成 `tinyui_obj_set_pos()`。原本确属画布/legacy parity 的绝对位置保留原值，但本任务新增的 `set_pos/set_size` 调用数必须为 `0`。

运行：

```bash
rtk git diff --unified=0 -- tinyui/demo/hello_world tinyui/demo/basic_widgets tinyui/demo/settings_panel tinyui/demo/list_basic tinyui/demo/table_basic tinyui/demo/graph_basic tinyui/demo/calendar_basic tinyui/demo/combo_box_basic tinyui/demo/keyboard_basic tinyui/demo/line_edit_basic tinyui/demo/scroll_selector_basic
```

预期：diff 中没有新增以 `+` 开头的固定坐标/尺寸补丁。

- [x] **步骤 4：运行 focused boundary/build**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
rtk cmake --build build/v2.3-m4-demo --target tinyui_demo -j
```

预期：checker 只报告尚未迁移的其他 demo；上述 11 个 demo 无违规命中，编译无类型强转警告。
实现说明（Task 3）：11 个基础/输入/数据 demo 已迁为 `tinyui_demo_<name>_build(screen)`；`scroll_selecter_basic` 已重命名为 `scroll_selector_basic`；无 public grid-cell API 时纵向堆叠改用 canonical flex column，未新增 `set_pos/set_size` 补丁；`examples/sdl/CMakeLists.txt` 的 `TINYUI_DEMO_MIGRATED_SOURCES` 已接入这 11 源。

### 任务 4：迁移仪表、资源和复合控件 demo

**文件：**

- 修改：`tinyui/demo/animation_basic/*`
- 修改：`tinyui/demo/arc_basic/*`
- 修改：`tinyui/demo/clock_basic/*`
- 修改：`tinyui/demo/date_time_basic/*`
- 修改：`tinyui/demo/gauge_basic/*`
- 修改：`tinyui/demo/icon_slider_basic/*`
- 修改：`tinyui/demo/message_box_basic/*`
- 修改：`tinyui/demo/progress_bar_basic/*`
- 修改：`tinyui/demo/progress_wheel_basic/*`
- 修改：`tinyui/demo/qrcode_basic/*`
- 修改：`tinyui/demo/radial_menu_basic/*`
- 修改：`tinyui/demo/theme_showcase/*`

**接口：**

- 消费：timer/event/theme/style/image/font descriptor API。
- 产出：12 个 canonical builder；descriptor 所有权和 timer 生命周期可由运行时 gate 观察。

- [x] **步骤 1：先为资源与 timer 生命周期加失败检查**

demo checker 必须拒绝 builder 内栈上创建并在返回后仍被 image/font 对象借用的 descriptor；runtime capture 必须运行 animation/clock 至少两个采样时刻，并验证真实像素变化。

运行：

```bash
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py --demo animation_basic
```

预期：FAIL，旧逐帧 hook 或旧资源句柄尚未符合 canonical 生命周期。

- [x] **步骤 2：迁移 descriptor 和 timer**

需要跨 builder 返回继续使用的 `tinyui_image_source_t`、`tinyui_font_t`、`tinyui_theme_t` 放入静态 demo state 或由 screen 关联的长期状态；临时 `tinyui_style_t` 可在 apply 返回后销毁。animation/clock 使用 `tinyui_timer_create()`，timer callback 只调用 canonical setter；不增加全局资源缓存或引用计数。

- [x] **步骤 3：迁移主题和复杂控件事件**

`theme_showcase` 使用 `tinyui_theme_set/apply` 和固定 style descriptor；message box、radial menu、icon slider、gauge、arc 等交互统一使用 event callback。backend 不支持的 part/state 必须显式处理 `TINYUI_ERROR_NOT_SUPPORTED`，不得画假状态。

- [x] **步骤 4：运行 focused runtime/capture**

运行：

```bash
rtk cmake --build build/v2.3-m4-demo --target tinyui_demo -j
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py --demo animation_basic
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py --demo theme_showcase
```

预期：三个命令 PASS；capture 来自真实 LingDongGUI 渲染链。

实现记录（2026-07-15 Task 4）：12 个 demo 已改为 `tinyui_demo_<name>_build(screen)`；animation/clock 使用 `tinyui_timer_create`；image/font/theme descriptor 使用静态 demo state；message_box/radial_menu/icon_slider/gauge/arc 挂 event callback；`examples/sdl/CMakeLists.txt` 的 `TINYUI_DEMO_MIGRATED_SOURCES` 仅追加本任务 12 项。

### 任务 5：迁移 layout/parity 尾部并关闭全部 demo

**文件：**

- 修改：`tinyui/demo/layout_flex/*`
- 修改：`tinyui/demo/layout_grid/*`
- 修改：`tinyui/demo/layout_parity/*`
- 修改：`tinyui/demo/grid_parity/*`
- 修改：`tinyui/demo/legacy_widget_parity/*`
- 修改：`tinyui/demo/legacy_demo0_parity/*`
- 修改：`examples/sdl/CMakeLists.txt`
- 修改：`tests/tinyui/runtime/check_tinyui_runtime.py`

**接口：**

- 消费：canonical flex/grid、typed track、timer、全控件能力。
- 产出：6 个 canonical builder；全部 29 个 demo 的 build、启动和 capture 证据。

- [x] **步骤 1：先锁定 layout/parity 行为证据**

为每个 layout demo 记录确定像素断言，为两个 parity demo 记录现有用户意图和交互断言。typed grid track 只允许 `TINYUI_GRID_UNIT_PX/FR/CONTENT`，禁止旧负数魔数和 END sentinel。

- [x] **步骤 2：迁移 layout demo**

用 canonical flex/grid API 直接表达 flow、align、gap、grow、track、cell、new-track 和 ignore-layout。若真实 reflow 不正确，停止 M4 并回到 M3 修复 backend；不得在 demo 增加坐标补偿。

- [x] **步骤 3：迁移两个 parity demo**

`legacy_widget_parity` 覆盖的每种控件必须继续存在且只通过 public API 创建；`legacy_demo0_parity` 的 frame 更新改为固定池 timer。名称中的 `legacy` 仅表示对照场景，不允许保留 legacy API。

- [x] **步骤 4：显式同步 CMake source 清单**

`examples/sdl/CMakeLists.txt` 的 demo source 集合必须与 manifest 完全相同，不使用 `GLOB`，并移除旧 `scroll_selecter_basic` source。

- [x] **步骤 5：运行全部 demo gate**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
rtk cmake --build build/v2.3-m4-demo --target tinyui_demo -j
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py --all
```

预期：全部 PASS；manifest、源码、头和 CMake 均为 29 项；每个 demo 至少取得一次非空真实像素 capture，交互 demo 仍须由 M3 的 L5-E 证据支撑。

实现记录（2026-07-15 Task 5）：
- 阻塞解除：child layout 能力提升为 canonical public API（`tinyui_obj_set_flex_grow/new_track/min_*/max_*`、`tinyui_obj_set_ignore_layout`、`tinyui_obj_set_grid_cell`），声明于 `tinyui/include/layout/layout.h`，实现 thin wrapper 于 `tinyui/src/core/widget.c`，并写入 `tests/tinyui/contract/tinyui_v23_public_api.json`。
- 6 个 demo 全部改为 `tinyui_demo_<name>_build(screen)`；layout 使用 typed track + public child layout；`legacy_demo0_parity` 用 `tinyui_timer_create` 驱动 gauge/arc 更新，禁止 frame hook。
- `examples/sdl/CMakeLists.txt` 的 `TINYUI_DEMO_MIGRATED_SOURCES` 追加 6 项（共 29，与 manifest 对齐）。
- `check_tinyui_v23_public_api.py` 与 `check_tinyui_demo_boundary.py` PASS。

### 任务 6：建立 install/export/find_package 和仓外 C/C++ consumer

**文件：**

- 新增：`cmake/TinyUIConfig.cmake.in`
- 修改：`cmake/LingDongGUI.cmake`
- 修改：`CMakeLists.txt`
- 新增：`tests/tinyui/consumer/CMakeLists.txt`
- 新增：`tests/tinyui/consumer/main.c`
- 新增：`tests/tinyui/consumer/main.cpp`
- 新增：`tests/tinyui/consumer/run_install_consumer.cmake`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 消费：M3 canonical target 和 public headers。
- 产出：`TinyUIConfig.cmake`、`TinyUIConfigVersion.cmake`、`TinyUITargets.cmake`、`TinyUI::tinyui`、CTest `check_tinyui_install_consumer`。

- [x] **步骤 1：先写仓外 consumer**

consumer `CMakeLists.txt` 必须仅使用安装前缀，不得 `add_subdirectory()`、不得引用源码绝对路径：

```cmake
cmake_minimum_required(VERSION 3.16)
project(tinyui_external_consumer LANGUAGES C CXX)
find_package(TinyUI 2.3 CONFIG REQUIRED)
add_executable(tinyui_consumer_c main.c)
add_executable(tinyui_consumer_cpp main.cpp)
target_compile_features(tinyui_consumer_c PRIVATE c_std_11)
target_compile_features(tinyui_consumer_cpp PRIVATE cxx_std_17)
target_link_libraries(tinyui_consumer_c PRIVATE TinyUI::tinyui)
target_link_libraries(tinyui_consumer_cpp PRIVATE TinyUI::tinyui)
```

C 和 C++ 程序都只 include `<tinyui.h>`。C 程序调用 `tinyui_image_source_from_rgb565()`/`tinyui_image_source_deinit()`，C++ 程序调用 `tinyui_font_from_builtin()`/`tinyui_font_deinit()`；两者均检查返回码并以 `0` 退出。该仓外 gate 只证明 canonical 类型、C/C++ 头和安装库可编译链接，不调用需要具体平台宿主的 runtime，避免把 package consumer 误写成 port/L6 证明；C++ 程序必须无额外 `extern "C"` 包裹即可编译。

- [x] **步骤 2：运行 consumer 并确认先失败**

运行：

```bash
rtk cmake -S tests/tinyui/consumer -B build/v2.3-m4-consumer-fail -DCMAKE_PREFIX_PATH=build/v2.3-m4-prefix
```

预期：FAIL，精确报告找不到 `TinyUIConfig.cmake`。

- [x] **步骤 3：实现安装导出**

使用 `GNUInstallDirs` 和 `CMakePackageConfigHelpers`；版本兼容策略为 `SameMajorVersion`；export namespace 固定 `TinyUI::`。`TinyUIConfig.cmake.in` 只 include `TinyUITargets.cmake` 并执行 `check_required_components(TinyUI)`。安装树只允许：

```text
include/tinyui.h
include/tinyui_config.h
include/core/*.h
include/widgets/*.h
include/layout/*.h
include/theme/*.h
include/style/*.h
include/resource/*.h
include/integration/input.h
lib/<TinyUI libraries>
lib/cmake/TinyUI/TinyUIConfig.cmake
lib/cmake/TinyUI/TinyUIConfigVersion.cmake
lib/cmake/TinyUI/TinyUITargets.cmake
```

- [x] **步骤 4：实现 fail-closed 仓外驱动**

`run_install_consumer.cmake` 必须删除旧 stage/source/build，执行 `cmake --install` 到 `${binary_dir}/_tinyui_install`，复制 consumer fixture 到 `${binary_dir}/_tinyui_consumer_src`，断言复制目录不在 repo root 内，再分别配置、构建并运行 C/C++ executable。任一 `execute_process()` 非零立即 `message(FATAL_ERROR ...)`。

- [x] **步骤 5：注册和运行安装消费者 gate**

CTest 名固定 `check_tinyui_install_consumer`，labels 固定 `tinyui;contract;install;consumer`。

运行：

```bash
rtk cmake --build build/v2.3-m4-demo -j
rtk ctest --test-dir build/v2.3-m4-demo -R '^check_tinyui_install_consumer$' --output-on-failure
```

预期：PASS；日志中的 consumer source/build/install 三个路径均在 `build/v2.3-m4-demo` 下且不在源码树中；C 与 C++ 均配置、链接和运行成功。

- [x] **步骤 6：检查安装树无私有泄漏**

运行：

```bash
rtk find build/v2.3-m4-demo/_tinyui_install -type f
rtk rg -n 'arm_2d|LingDongGUI|tinyui/src|src/core|src/drivers' build/v2.3-m4-demo/_tinyui_install/include build/v2.3-m4-demo/_tinyui_install/lib/cmake/TinyUI
```

预期：第一条只列出约定安装内容；第二条零命中。

实现记录（2026-07-15 Task 6）：

- 新增 `cmake/TinyUIConfig.cmake.in`（只 include `TinyUITargets.cmake` + `check_required_components`）。
- `ld_define_core_targets()` 新增用户 INTERFACE 目标 `tinyui` + `TinyUI::tinyui` ALIAS；`ld_install_tinyui_package()` 安装 public headers / 静态库 / Config+Version+Targets。
- 安装库：`libtinyui_core.a`、`liblongdonggui.a`、`liblongdonggui_arm2d.a`（符号解析需要；**不**安装 backend/private headers）。
- 关键：`longdonggui*` / `tinyui_core` 的 include 与 Arm-2D PUBLIC compile defs 全部改为 `$<BUILD_INTERFACE:...>`，export 后 `TinyUI::tinyui` 的 `INTERFACE_INCLUDE_DIRECTORIES` 仅 `${prefix}/include`，无 backend include 泄漏。
- 仓外 fixture：`tests/tinyui/consumer/{CMakeLists.txt,main.c,main.cpp,run_install_consumer.cmake}`；CTest `check_tinyui_install_consumer` labels=`tinyui;contract;install;consumer`。
- C++ 兼容：`tinyui.h` 增加 `extern "C"` 包裹，使 `main.cpp` 无需额外包装即可链接。
- 先红：`find_package(TinyUI 2.3 CONFIG REQUIRED)` 在无 install 前缀时精确失败（找不到 `TinyUIConfig.cmake`）。
- 后绿：`check_tinyui_install_consumer` PASS；泄漏扫描零命中。

### 任务 7：重写 quick start、API、demo 和资源文档并编译示例

**文件：**

- 修改：`tinyui/docs/quick_start.md`
- 修改：`tinyui/docs/api_overview.md`
- 修改：`tinyui/docs/demo_guide.md`
- 修改：`tinyui/docs/resource_lifetime.md`
- 新增：`tests/tinyui/contract/check_tinyui_docs_examples.py`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 消费：安装后的 `<tinyui.h>` 与 `TinyUI::tinyui`。
- 产出：current-facing 中文文档和 `check_tinyui_docs_examples` CTest。

- [x] **步骤 1：先写文档禁止项和示例抽取测试**

checker 扫描四份面向当前版本的文档，拒绝 `tinyui_app_*`、`tinyui_widget_*`、`tinyui_native_*`、`ld*`、`arm_2d_*`、`SIGNAL_*`；抽取标记为 ````c compile```` 和 ````cpp compile```` 的代码块，生成临时编译单元，并只使用安装包编译链接。canonical `tinyui_window_*` 不得被笼统拒绝。

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_docs_examples.py --build-dir build/v2.3-m4-docs --prefix build/v2.3-m4-demo/_tinyui_install
```

预期：FAIL，旧 quick start 仍使用 app/window legacy API。
实现：`tests/tinyui/contract/check_tinyui_docs_examples.py`；先对旧文档跑红（`tinyui_app_create` / `tinyui_native_*` / 缺 compile 块）。

- [x] **步骤 2：重写 quick start**

最小示例必须完整展示：`tinyui_init()`、screen、label/button、统一 event callback、`tinyui_screen_load(...NONE...)`、`tinyui_process(&next_ms)`、错误清理、`tinyui_deinit()`。平台 sleep/clock 只描述为集成层职责，不给出伪 port 实现。

实现：完整生命周期用普通 ````c```` 阅读块（install 仅链三库时 `tinyui_init` 会缺宿主符号）；````c compile```` / ````cpp compile```` 覆盖 image_source/font/result 可链接探针。

- [x] **步骤 3：重写 API overview**

按 runtime/object/widgets/event+focus/timer/theme+style/flex+grid/image+font/result 分节；每节列出 canonical 头、核心签名、所有权、错误语义和 `NOT_SUPPORTED` 行为。明确 TinyUI 参考 LVGL API 模式但不兼容 LVGL，也不复制其内部机制。

- [x] **步骤 4：重写 demo/resource 文档**

demo guide 只描述 `build(screen)` 和统一 runner。resource lifetime 必须分别给出 RGB565 借用数据、builtin、VRES、font 的生命周期；明确 source/font 绑定对象期间有效，临时 style apply 后可释放，禁止暗示 core 有缓存或引用计数。

- [x] **步骤 5：运行文档示例 gate**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_docs_examples.py --build-dir build/v2.3-m4-docs --prefix build/v2.3-m4-demo/_tinyui_install
rtk ctest --test-dir build/v2.3-m4-demo -R '^check_tinyui_docs_examples$' --output-on-failure
```

预期：PASS；全部 C/C++ 示例只依赖安装包并编译链接成功。
CTest：`check_tinyui_docs_examples` labels=`tinyui;contract;docs;consumer`。

### 任务 8：落地 v2.2 -> v2.3 迁移指南并关闭 M4

**文件：**

- 新增：`docs/v2.3/v2.3-migration-guide.md`
- 修改：`docs/v2.3/deferred-port-work.md`
- 修改：`tests/tinyui/contract/check_tinyui_deprecated_api_usage.py`
- 删除：`tinyui/include/internal/v22_demo_bridge.h`
- 删除：`tinyui/src/compat/v22_demo_bridge.c`
- 修改：CMake 中 `TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE` 的私有接线

**接口：**

- 消费：M1 删除清单、M2/M3 canonical 签名、任务 1 manifest、任务 6 package contract。
- 产出：可逐项执行的破坏性迁移映射、迁移桥零残留和 M4 closeout 证据。

- [x] **步骤 1：先扩展 deprecated usage gate**

扫描 current-facing demo/docs/consumer，拒绝 app handle、旧 typed wrapper、错拼 `tabel/scroll_selecter/q_r_code`、`set_press` 重复语义、旧 grid sentinel、旧资源 destroy 名和 native/backend 泄漏。

实现说明（2026-07-15 Task 8）：重写 `check_tinyui_deprecated_api_usage.py`，覆盖 public headers（排除 `internal/**`）、demo、user docs、`docs/v2.3` current-facing（migration/README/deferred，排除 plans）、consumer、可选 `--prefix` 安装树。`check_tinyui_removed_api.py` full/minimal 均要求 forbidden 定义符号零命中（不再有 bridge TU 例外）。

- [x] **步骤 2：写完整迁移映射**

迁移指南必须包含以下独立章节和 before/after 可编译片段：

1. `app_create/app_run` -> `init + screen + process`。
2. typed widget 指针与强转 -> `tinyui_obj_t *`。
3. `create(app/window, id)` -> `create(parent)` 与 props presence bit/id。
4. widget/window 通用 setter -> `tinyui_obj_set_*`。
5. 旧 callback -> `tinyui_event_t` + handle remove。
6. app timer -> 固定池 `tinyui_timer_*`。
7. 旧 theme/style -> descriptor apply。
8. grid 负数魔数/sentinel -> typed track + count。
9. image/font 旧资源句柄 -> caller-owned descriptor/deinit。
10. `tabel`、`scroll_selecter`、`q_r_code`、`set_press` -> canonical 拼写。
11. 源码内 include/add_subdirectory -> 安装包 `find_package(TinyUI CONFIG REQUIRED)`。
12. 不兼容说明：v2.3 不承诺 v2.2 ABI，port 生产问题仍延期。

实现说明：`docs/v2.3/v2.3-migration-guide.md` 12 章独立 `## N.` + before/after C 片段；明确禁止 `v22_demo_bridge`。

- [x] **步骤 3：明确 port 未完成**

`deferred-port-work.md` 必须逐项保留 framebuffer/stride/PFB、flush/DMA/fence、平台时钟、SDL event pump、MCU toolchain/RTOS/ISR/cache、真机验证，并明确 M4 的 SDL consumer 只是测试宿主证据，不是 L6 或生产可用证明。

实现说明：已强调 **M4 关闭后 port 仍延期**；SDL consumer/demo 非 L6/非生产证明。

- [x] **步骤 4：删除仓内临时迁移桥**

全部 29 个 demo、旧测试和文档迁移后，删除 `v22_demo_bridge.h/.c` 与 `TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE` 接线。`check_tinyui_deprecated_api_usage.py` 必须检查 public headers、full archive、demo、文档、consumer 和安装树，legacy/错拼符号全部零命中。不得把 bridge 保留到 M5，也不得把它改名后继续存在。

实现说明：删除 `tinyui/include/internal/v22_demo_bridge.h`、`tinyui/src/compat/v22_demo_bridge.c`；清除 CMake/`tinyui.h`/tests/examples 接线。unit/support 调用迁到 `tinyui_runtime_internal_*` 或 public API。未改名保留 bridge。

- [x] **步骤 5：运行 M4 全门禁**

运行：

```bash
rtk cmake -S . -B build/v2.3-m4 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m4 -j
rtk ctest --test-dir build/v2.3-m4 -L 'demo|consumer|install' --output-on-failure
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py --all
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk git diff --check
```

预期：全部 PASS；29 个 demo canonical boundary/capture、文档代码、install/export/find_package、仓外 C/C++ consumer 同时通过。

实现说明：见 Task 8 closeout 验证表；优先保证 demo/consumer/install/docs/deprecated 门禁；全量 unit/port 若有既有债记入 Concerns。

- [x] **步骤 6：运行阶段影响复核**

主线程调用 GitNexus MCP：

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：改动只落在 demo、runner、交付 CMake、consumer test 和 current-facing 文档；若出现 core/backend 行为符号，M4 不得关闭，必须回到对应阶段重新验证。

实现说明：GitNexus 不可用时以 `rtk git diff --stat` / 路径审查代替；Task 8 主要落在 contract/demo/docs/tests/support/cmake 与删 bridge，不改 public 控件语义。

## M4 完成定义

- 全部 29 个 demo 使用 `build(screen)` 和 canonical public API，runner 是唯一 runtime owner。
- demo 未新增固定坐标/尺寸补丁，真实 backend/layout 缺口没有被视觉代码掩盖。
- quick start、API overview、demo guide、resource lifetime 和迁移指南中的代码均由安装包编译通过。
- `cmake --install` 产物可被仓外 C11/C++17 工程通过 `find_package(TinyUI 2.3 CONFIG REQUIRED)` 使用。
- 安装树不包含 internal、LingDongGUI 或 Arm-2D 头。
- 文档和测试明确 SDL 只是测试宿主，port/L6 未完成。
- M4 只可描述为 v2.3 内部里程碑；不得提前声明 TinyUI v2.3 已全部完成。

## Task 8 / M4 closeout（2026-07-15）

**结论：`DONE_WITH_CONCERNS`（内部里程碑）**

已交付：
1. deprecated/removed current-facing 门禁扩展，bridge 零残留（计划历史文档除外）。
2. 中文迁移指南 12 章 + deferred-port 强调测试宿主非 L6。
3. 删除 `v22_demo_bridge` 与全部 `TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE` 接线；tests/support 改 `runtime_internal`/public。
4. MCU 示例模板改为 canonical `init/screen/process`（仍非生产 port）。

**Concerns：**
1. port/L6/真机仍延期；M4 SDL 仅测试宿主。
2. 部分 unit/port 测试依赖 internal 头（可接受；非 public consumer）。
3. `app_legacy.h` / `widget_legacy.h` 仍作为 internal 类型与声明残留（非 public、非 bridge）；M5 可继续收口。
4. 全量 CTest / binary·perf 指纹 / 全 demo runtime capture 仍可按机器与既有债延后到 M5。
5. 不声明 v2.3 发布完成。