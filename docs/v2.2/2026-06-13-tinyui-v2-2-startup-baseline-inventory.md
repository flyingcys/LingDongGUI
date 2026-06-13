# TinyUI v2.2 Startup Baseline Inventory

> 生成日期：2026-06-13
> 版本：v2.2 S0 启动残留面冻结
> 用途：锁定 `S1-S3` 的起跑边界，后续阶段不得再重新定义起跑边界

## 1. 当前 canonical 启动相关头/源

### 1.1 runtime.h — 当前 canonical 入口（部分）

文件：`tinyui/include/runtime.h`

当前状态：
- 已提供 `tinyui_init/deinit/screen_create/screen_load/timer_handler` 函数声明
- 但声明仍使用 `tinyui_*` 前缀命名，不是 `tinyui_*`
- 通过 `#define` alias 和 `static inline` wrapper 提供 `tinyui_*` 入口
- 不是唯一、绝对清晰的 canonical 启动真相

### 1.2 app.h — 旧主路径（仍活跃）

文件：`tinyui/include/app.h`

当前状态：
- 包含完整的 `tinyui_app_*` 旧主路径声明
- 头部注释已标记为 "Compatibility API retained during TinyUI transition"
- 但所有 demo 仍依赖此路径
- 声明：`tinyui_app_create/run/run_background/set_window/set_background/switch_window/switch_background/timer_*`

### 1.3 app.c — 旧主路径实现

文件：`tinyui/src/core/app.c`

当前状态：
- 实现完整的 `tinyui_app_*` 函数：`create/run/run_background/set_window/set_background/switch_window/switch_background/timer_create/start/stop/is_running/destroy/destroy`
- 包含 `tinyui_app_pump_timers` 内部 helper
- 被 `runtime.c` 通过 `tinyui_app_create()` 间接依赖

### 1.4 runtime.c — 当前 runtime 实现

文件：`tinyui/src/core/runtime.c`

当前状态：
- 实现 `tinyui_init/deinit/screen_create/screen_load/timer_handler`
- `tinyui_init` 内部调用 `tinyui_app_create()` — 仍依赖旧 app 路径
- `tinyui_screen_load` 内部调用 `tinyui_app_set_window()` — 仍依赖旧 app 路径

### 1.5 runtime_host.c — SDL host 渲染

文件：`tinyui/src/core/runtime_host.c`

当前状态：
- 实现 `tinyui_runtime_host_step_app` — runtime loop 主步进函数
- 包含完整的 SDL 窗口/渲染/事件循环
- include `backend.h` — S2 需要清理

### 1.6 runtime_bridge.c — bridge 层

文件：`tinyui/src/core/runtime_bridge.c`

当前状态：
- 实现 `tinyui_runtime_bridge_init_app/run_app/step_app/shutdown_app`
- 实现 `tinyui_runtime_bridge_bind_host/unbind_host`
- 实现 `tinyui_runtime_bridge_backend_state` 等 backend state 访问
- 不直接 include `backend.h`，但通过 `internal.h` 间接依赖

## 2. 当前 demo 启动真相

### 2.1 所有 28 个 demo 目录均有 `main.c`

```
tinyui/demo/animation_basic/main.c
tinyui/demo/arc_basic/main.c
tinyui/demo/basic_widgets/main.c
tinyui/demo/calendar_basic/main.c
tinyui/demo/clock_basic/main.c
tinyui/demo/combo_box_basic/main.c
tinyui/demo/date_time_basic/main.c
tinyui/demo/gauge_basic/main.c
tinyui/demo/graph_basic/main.c
tinyui/demo/grid_parity/main.c
tinyui/demo/hello_world/main.c
tinyui/demo/icon_slider_basic/main.c
tinyui/demo/keyboard_basic/main.c
tinyui/demo/layout_flex/main.c
tinyui/demo/layout_grid/main.c
tinyui/demo/layout_parity/main.c
tinyui/demo/legacy_widget_parity/main.c
tinyui/demo/line_edit_basic/main.c
tinyui/demo/list_basic/main.c
tinyui/demo/message_box_basic/main.c
tinyui/demo/progress_bar_basic/main.c
tinyui/demo/progress_wheel_basic/main.c
tinyui/demo/qrcode_basic/main.c
tinyui/demo/radial_menu_basic/main.c
tinyui/demo/scroll_selecter_basic/main.c
tinyui/demo/settings_panel/main.c
tinyui/demo/table_basic/main.c
tinyui/demo/theme_showcase/main.c
```

### 2.2 demo 启动模式分类

**模式 A：`main() -> run_demo() -> tinyui_app_create() + tinyui_app_run()`**
（25 个 demo 采用此模式）

- `run_demo()` 内创建 `tinyui_app_create()`，构建 UI，调用 `tinyui_app_run()`
- `main()` 仅调用 `return run_demo();`

具体：arc_basic, progress_bar_basic, list_basic, settings_panel, clock_basic, layout_grid, line_edit_basic, theme_showcase, layout_flex, layout_parity, message_box_basic, hello_world, qrcode_basic, gauge_basic, icon_slider_basic, animation_basic, progress_wheel_basic, radial_menu_basic, date_time_basic, grid_parity, legacy_widget_parity

**模式 B：`main() -> tinyui_app_create() + tinyui_app_run()`（直接）**
（6 个 demo 采用此模式）

- `main()` 内直接调用 `tinyui_app_create()` 和 `tinyui_app_run()`
- 无 `run_demo()` 函数

具体：table_basic, graph_basic, scroll_selecter_basic, combo_box_basic, calendar_basic, keyboard_basic

**模式 C：`main() -> run_demo()`（但 `run_demo` 不调用 `tinyui_app_create`）**
（1 个 demo）

- `basic_widgets/main.c`：`run_demo()` 不直接调用 `tinyui_app_create()`，但 `main()` 调用 `run_demo()`

### 2.3 所有 demo 均依赖 `tinyui_app_*` 主路径

无一例外，所有 28 个 demo 均通过 `tinyui_app_create()/tinyui_app_run()` 或其包装函数启动。

### 2.4 当前无统一 `tinyui/demo/main.c`

当前不存在统一 runner。

## 3. `backend.h` 依赖面

### 3.1 文件位置

`tinyui/src/backend/ldgui/backend.h` — 1502 行

### 3.2 直接 include 来源（19 处）

| 文件 | include 方式 |
|------|-------------|
| `tinyui/src/core/internal.h` | `#include "backend.h"` |
| `tinyui/src/core/runtime_host.c` | `#include "backend.h"` |
| `tinyui/src/core/native.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/button.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/checkbox.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/slider.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/arc.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/gauge.c` | `#include "backend.h"` |
| `tinyui/src/widgets/icon_slider.c` | `#include "backend.h"` |
| `tinyui/src/widgets/radial_menu.c` | `#include "backend.h"` |
| `tinyui/src/widgets/progress_bar.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/qrcode.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/progress_wheel.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/image.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/line_edit.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/combo_box.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/calendar.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/table.c` | `#include "../backend/ldgui/backend.h"` |
| `tinyui/src/widgets/message_box.c` | `#include "backend.h"` |

### 3.3 `backend.h` 内容分层

#### 类别 A：Shared enum / truth policy / signal（应迁入 `core/internal.h`）

- `enum tinyui_backend_widget_kind` (26 个 widget kind)
- `enum tinyui_backend_signal` (4 个信号类型)
- `enum tinyui_backend_data_truth_policy` (3 个策略)
- `enum tinyui_backend_data_value_source` (3 个数据源)
- `enum tinyui_backend_runtime_evidence_flags` (2 个 flags)

#### 类别 B：Backend widget tree / runtime state（应迁入 `core/runtime_internal.h`）

- `struct tinyui_backend_widget` (核心 backend widget 结构体，含 tree、layout、data model 等)
- `struct tinyui_backend_app_state` (app backend state，含 theme、scene、runtime_state 等)

#### 类别 C：Layout cache（应迁入 `window` / `layout` private）

- `struct tinyui_backend_layout_window_state` (flex/grid layout 参数)
- `struct tinyui_backend_layout_child_state` (child layout 参数)

#### 类别 D：Widget-specific LDGUI bridge/helper（应保留在各 widget `.c` 或 widget-private 头）

- `tinyui_backend_arc_*` (arc 相关 backend 函数)
- `tinyui_backend_gauge_*` (gauge 相关 backend 函数)
- `tinyui_backend_progress_bar_*` (progress bar 相关 backend 函数)
- `tinyui_backend_qrcode_*` (qrcode 相关 backend 函数)
- `tinyui_backend_line_edit_*` (line edit 相关 backend 函数)
- `tinyui_backend_combo_box_*` (combo box 相关 backend 函数)
- `tinyui_backend_scroll_selecter_*` (scroll selecter 相关 backend 函数)
- `tinyui_backend_switch_*` (switch 相关 backend 函数)
- `tinyui_backend_image_*` (image 相关 backend 函数)
- `tinyui_backend_window_*` (window 相关 backend 函数)
- `tinyui_list_*` (list 相关 backend 函数)
- `tinyui_widget_emit_*` (emit helper 函数)
- `tinyui_widget_*_backend_focus` (focus 函数)
- `tinyui_widget_init_data_model` (data model 函数)
- `tinyui_backend_set_image_source` (image source setter)
- `tinyui_backend_progress_wheel_test_*` (progress wheel 测试 hook)
- `tinyui_backend_table_test_*` (table 测试 hook)

#### 类别 E：仅单文件使用的 helper

- `tinyui_widget_set_backend_text` — 检查实际使用面，可能可改成 `static`
- `tinyui_widget_emit_value_changed/event/clicked` — 多 widget 使用，应放入 shared internal 头

## 4. 目标终态

### 4.1 启动模型

```
tinyui_init() -> tinyui_screen_create() -> demo_build(screen) -> tinyui_screen_load() -> while(1) { tinyui_timer_handler(); } -> tinyui_deinit()
```

### 4.2 Demo 组织

- demo `.c` 只导出 `tinyui_demo_<name>_build(tinyui_obj_t *screen)` 构建 API
- `tinyui/demo/main.c` 作为统一 runner
- 切换 demo 靠手工替换 `main.c` 中调用的 build API

### 4.3 `backend.h` 退场

- 按 shared / widget-local 真边界拆散
- 不允许新建替代性 mega-header
- 不允许把 widget-local helper 提升为 shared 总入口

### 4.4 `app.c/app.h` 退出主路径

- 不再作为 canonical 用户入口
- 保留 timer/lifecycle 薄职责或完全并入 runtime

## 5. 禁止回退边界

- 不允许新建另一份 mega-header 取代 `backend.h`
- 不允许把 widget-local helper 再提升成 shared 总入口
- 不允许 demo 迁移时顺手把业务逻辑上推到 runtime 层
- 不允许保留 `tinyui_app_*` 作为 canonical 主路径
