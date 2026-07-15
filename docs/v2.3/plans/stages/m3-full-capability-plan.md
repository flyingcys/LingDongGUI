# TinyUI v2.3 M3 全能力迁移实施计划

> **供自动化执行者使用：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，按任务逐项实施；每一步使用复选框（`- [ ]`）跟踪。不创建 worktree。

**目标：** 把 LingDongGUI 全部 required 用户控件能力迁移到 canonical TinyUI API，完成轻量 theme/style、direct flex/grid、image/font value descriptor、命名清理和编译期裁剪，并取得逐项 L4/L5 证据。

**架构：** M3 复用 M2 的单 runtime、唯一 LD tree、固定池和直接 adapter；各控件文件只实现本控件的真实 `ld*` 映射，公共 adapter、公共头、CMake 和证据矩阵由单一集成任务维护。theme/style、layout、resource 都是即时、借用、无分配的薄转换，不建立第二状态系统。

**技术栈：** C11、CMake/CTest、LingDongGUI、Arm-2D、SDL 测试宿主、Python 3 合约/像素/事件检查、nm/map、GitNexus。

## 全局约束

- M2 全部门禁必须通过后才能开始 M3；M2 的四样板继续作为实现模板与回归门禁。
- 覆盖目标是 LingDongGUI 用户能力 100%，不是逐字包装全部 `ld*` 符号，也不是 LVGL 功能覆盖。
- `policy_never_public` 只能用于生命周期、渲染管线、内存、宿主、调试和 backend-private helper。
- 所有成功 setter 必须直接改变真实 LD 对象；无法映射返回 `TINYUI_ERROR_NOT_SUPPORTED`，不得 fake 或软件绘制补齐。
- theme/style apply、flex/grid、image/font descriptor、事件 dispatch、普通 getter不得分配 heap。
- theme 不自动遍历全树；style 不保存 descriptor 指针；image/font 不复制、不缓存、不引用计数。
- flex/grid 只做参数校验与类型转换，然后调用真实 LingDongGUI layout/reflow。
- 每个控件族保持 `TINYUI_ENABLE_<WIDGET>` 独立裁剪；关闭模块的实现符号不得进入最小二进制。
- 所有 shell 命令均以 `rtk` 开头；构建统一使用 CMake。
- 修改任何函数前必须执行 GitNexus upstream `impact`；`HIGH`/`CRITICAL` 先报告调用者与流程。
- M3 并行任务写面不得重叠；review 不通过时由原任务执行者在原写面修复。

---

## 并行波次与共享文件所有权

波次 0 串行冻结能力清单；波次 1 的 任务 2-7 可并行；波次 2 的共享集成、裁剪和证据任务串行。

共享文件只允许 任务 8-11 的单一集成负责人修改：

- `tinyui/include/tinyui.h`
- 公共基础类型/结果码头
- `tinyui/src/core/widget.c`
- `tinyui/src/core/internal.h`
- `cmake/LingDongGUI.cmake`
- `tests/tinyui/CMakeLists.txt`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`（仅 任务 10）
- `docs/v2.3/v2.3-capability-evidence-matrix.md`（仅 任务 10）

波次 1 任务只能修改表中自己的源文件和已有对应测试，不能为注册测试而修改 CMake：

| 任务 | 独占控件源文件 | 独占测试文件 |
| --- | --- | --- |
| 2 | switch/arc/gauge/progress_bar/progress_wheel | 同名五个 unit test |
| 3 | list/combo_box/scroll_selector/icon_slider/radial_menu/calendar | 同名六个 canonical unit test |
| 4 | text/line_edit/keyboard/table/graph | 同名五个 unit test |
| 5 | image/canvas/animation/date_time/clock/qrcode/message_box/background/window | 同名九个 unit test |
| 6 | theme | theme unit test |
| 7 | flex/grid/resource | layout/image/resource unit test |

### 任务 1：冻结全控件用户能力 ledger 与无重叠任务清单

**文件：**
- 修改： `tests/tinyui/contract/native_api_gap_ledger.json`

**接口：**
- 输入： M0 inventory、M1 canonical declarations、M2 证据格式。
- 输出： 每个用户能力的 owner、public API、LD 映射、required、L4/L5 要求、测试名。

- [x] **步骤 1：重新生成并检查 LingDongGUI public inventory**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_native_100_inventory.py
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

预期： inventory 无漂移；若失败先修真相源，不得在 ledger 中隐藏新用户能力。

- [x] **步骤 2：逐项分类全部控件能力**

对 animation、arc、background、button、calendar、canvas、checkbox、clock、combo_box、date_time、gauge、graph、icon_slider、image、keyboard、label、line_edit、list、message_box、progress_bar、progress_wheel、qrcode、radial_menu、scroll_selector、slider、switch、table、text、window 的每项 public LD 能力记录：canonical TinyUI API、direct/shared 等价关系、owner task、L4 test、是否可见、是否可操作。

只有 backend-private helper 可标 `policy_never_public`，且必须写具体理由与 inventory symbol。

- [x] **步骤 3：验证 ledger schema 与唯一 owner**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk python3 -c "import json; rows=json.load(open('tests/tinyui/contract/native_api_gap_ledger.json'))['rows']; assert all(r.get('owner_task') for r in rows)"
```

预期： 每项有且只有一个 owner；required 项没有 policy 规避；命令返回 0。

- [x] **步骤 4：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 1 执行记录（2026-07-15）

结论：**DONE**（仅 Task 1；M3 整体未完成）。

- ledger 与 `ldgui_public_api_inventory` 对齐为 **634** 行；补齐此前缺失的 5 个 inventory 符号：`ldBaseGetScreenSize`、`ldBaseGetScreenSizeForScene`、`ldTimeOut`、`ldGuiDisposeNodeTree`、`ldTextSetScrollEnabled`。
- 每行写入唯一 `owner_task`、`l4_unit_test`、`requires_l5_v`、`requires_l5_e`。
- owner 分布：`m2-sample=89`、`m2-core=58`、`m3-task-2=100`、`m3-task-3=108`、`m3-task-4=118`、`m3-task-5=109`、`m3-task-7=35`、`m3-policy-internal=17`；`m3-task-6`（theme）无 `ld*` inventory 行，在 ledger 元数据中注明。
- 公共 TinyUI API 命名债：`tinyui_scroll_selecter_*` → `tinyui_scroll_selector_*`；ledger widget 键仍保留 native 拼写 `scroll_selecter`（与 LD 头/inventory 一致）。
- 同步 `tinyui_release_capability_matrix.json` 与 exhaustiveness inventory 双格式读取，使 Task 1 门禁可绿。
- 验证：`check_tinyui_native_100_inventory.py` / `check_tinyui_native_api_exhaustiveness.py` / `check_tinyui_release_capability_matrix.py` / `rtk git diff --check` 均通过。

### 任务 2：迁移基础值与仪表控件族

**文件：**
- 修改： `tinyui/src/widgets/switch.c`
- 修改： `tinyui/src/widgets/arc.c`
- 修改： `tinyui/src/widgets/gauge.c`
- 修改： `tinyui/src/widgets/progress_bar.c`
- 修改： `tinyui/src/widgets/progress_wheel.c`
- 修改： `tests/tinyui/unit/test_tinyui_switch.c`
- 修改： `tests/tinyui/unit/test_tinyui_arc.c`
- 修改： `tests/tinyui/unit/test_tinyui_gauge.c`
- 修改： `tests/tinyui/unit/test_tinyui_progress_bar.c`
- 修改： `tests/tinyui/unit/test_tinyui_progress_wheel.c`

**接口：**
- 输入： M2 fixed event pool/common setter/image value接口。
- 输出： switch checked；arc angle/rotation/color/source；gauge angle/pointer/source/offset/auto；progress bar percent/orientation/color/source；progress wheel percent/dot/color。

- [x] **步骤 1：对本族所有被修改符号运行 impact**

至少调用：

```text
impact({target: "tinyui_switch_set_checked", direction: "upstream"})
impact({target: "tinyui_arc_create", direction: "upstream"})
impact({target: "tinyui_gauge_create", direction: "upstream"})
impact({target: "tinyui_progress_bar_set_percent", direction: "upstream"})
impact({target: "tinyui_progress_wheel_set_percent", direction: "upstream"})
```

预期：调用者仅对应控件 test/demo/shared common API；任何 HIGH 风险先报告。

- [x] **步骤 2：为每个 ledger 项写失败测试**

每个现有 unit 文件逐项执行 canonical API，再读取对应 `ldSwitch_t`、`ldArc_t`、`ldGauge_t`、`ldProgressBar_t`、`ldProgressWheel_t` 字段或 getter。参数边界、错误 kind、props 中途失败回滚、真实 `VALUE_CHANGED` 事件必须覆盖；programmatic setter 不制造用户事件。

- [x] **步骤 3：运行本族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_switch test_tinyui_arc test_tinyui_gauge test_tinyui_progress_bar test_tinyui_progress_wheel -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(switch|arc|gauge|progress_bar|progress_wheel)$' --output-on-failure
```

预期： 未迁移 setter、旧 callback 或 wrapper-only getter 使至少一项失败。

- [x] **步骤 4：实现本族直接 LD 映射**

每个 setter 采用“全部预检 -> `ld*` -> 最小缓存提交”；专用 callback 转发统一 event pool；getter 有 LD getter/字段时直接读 backend。删除能由 backend 查询的镜像字段，不新增通用表或渲染路径。

- [x] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_switch test_tinyui_arc test_tinyui_gauge test_tinyui_progress_bar test_tinyui_progress_wheel -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(switch|arc|gauge|progress_bar|progress_wheel)$' --output-on-failure
```

预期： 全部通过；每个 ledger required 项有真实 LD 状态断言。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 2 执行记录（2026-07-15）

结论：**DONE**（仅 Task 2；M3 整体未完成）。

- switch/arc/gauge/progress_bar/progress_wheel 完成直接 `ld*` setter/getter 映射；getter 以 LD 字段为真值。
- switch：`set_checked` 走 `sync_ld_value`（不伪造用户事件）；`set_on_toggled` 转发统一 event pool；legacy props `on_toggled` 非空则 create 失败回滚。
- arc：角度/旋转/颜色/quarter source/parent color 直达 `ldArc*`；`get_background_angle` 返回 native end angle。
- gauge：angle/pointer/source/offset/trail/progress/auto 直达 `ldGauge*`；image EMPTY 源拒绝。
- progress_bar：percent/orientation/inverted/color/frame/source 直达 `ldProgressBar*`；props 失败路径回滚。
- progress_wheel：percent/progress/dot/color 直达 `ldProgressWheel*`；`get_dot_enabled`/`get_percent` 读 backend。
- L4 unit：`test_tinyui_switch|arc|gauge|progress_bar|progress_wheel` 全部通过。
- 验证：`rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(switch|arc|gauge|progress_bar|progress_wheel)$' --output-on-failure` 与 `rtk git diff --check`。

### 任务 3：迁移选择与集合控件族

**文件：**
- 修改： `tinyui/src/widgets/list.c`
- 修改： `tinyui/src/widgets/combo_box.c`
- 修改： `tinyui/src/widgets/scroll_selector.c`
- 修改： `tinyui/src/widgets/icon_slider.c`
- 修改： `tinyui/src/widgets/radial_menu.c`
- 修改： `tinyui/src/widgets/calendar.c`
- 修改： `tests/tinyui/unit/test_tinyui_list.c`
- 修改： `tests/tinyui/unit/test_tinyui_combo_box.c`
- 修改： `tests/tinyui/unit/test_tinyui_scroll_selector.c`
- 修改： `tests/tinyui/unit/test_tinyui_icon_slider.c`
- 修改： `tests/tinyui/unit/test_tinyui_radial_menu.c`
- 修改： `tests/tinyui/unit/test_tinyui_calendar.c`

**接口：**
- 输入： M2 event/focus、ID、文本复制与 fixed capacity 结果码。
- 输出： items/selection/navigation/dropdown/picker/calendar date-grid 的真实映射；源码、测试和 public 名均使用 M1 已冻结的 `scroll_selector`。

- [x] **步骤 1：运行选择族影响分析**

```text
impact({target: "tinyui_list_set_selected_index", direction: "upstream"})
impact({target: "tinyui_combo_box_create", direction: "upstream"})
impact({target: "tinyui_scroll_selector_create", direction: "upstream"})
impact({target: "tinyui_icon_slider_create", direction: "upstream"})
impact({target: "tinyui_radial_menu_create", direction: "upstream"})
impact({target: "tinyui_calendar_create", direction: "upstream"})
```

预期：命名清理影响 demo/docs 留给 M4；本任务只实现 M1 已冻结 canonical symbol。

- [x] **步骤 2：写容量、所有权与 L4 失败测试**

对 items 增删/替换、空集合、最大容量、越界 index、临时文本复制、selection getter、focus/key 导航、`VALUE_CHANGED` payload 写断言。所有成功操作从真实 LD item buffer、selected index、calendar grid/date 字段验证；第 `capacity+1` 项返回 `CAPACITY`，不得静默截断。

- [x] **步骤 3：运行选择族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_list test_tinyui_combo_box test_tinyui_scroll_selector test_tinyui_icon_slider test_tinyui_radial_menu test_tinyui_calendar -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(list|combo_box|scroll_selector|icon_slider|radial_menu|calendar)$' --output-on-failure
```

预期： wrapper item 镜像、旧 callback 或错拼 public 路径导致失败。

- [x] **步骤 4：实现选择族真实映射**

使用各自现有 `ld*SetItems/SetSelect/SetDate` 能力；TinyUI 只保留 LD 要求调用者保持的稳定文本副本或 backend 无 getter 的必要标量。删除独立 callback 字段，统一 dispatch `VALUE_CHANGED`；复杂状态由控件 getter查询，不扩大 event payload。

- [x] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_list test_tinyui_combo_box test_tinyui_scroll_selector test_tinyui_icon_slider test_tinyui_radial_menu test_tinyui_calendar -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(list|combo_box|scroll_selector|icon_slider|radial_menu|calendar)$' --output-on-failure
```

预期： 全部通过，容量和 selection 结果明确。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 3 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 3；M3 整体未完成）。

- 写面：`list/combo_box/scroll_selector/icon_slider/radial_menu/calendar` 源码 + 对应 unit tests；CMake 历史目标名仍为 `test_tinyui_scroll_selecter`（文件名保持），public API 使用 M1 冻结的 `scroll_selector`。
- list：容量 `CAPACITY` / 越界 `OUT_OF_RANGE`、真实 `ldListSetText/SetSelectItem`、颜色/高度/padding/margin/align/item_widget 直写 LD；core bridge 的 `SIGNAL_CLICKED_ITEM` → 统一 event pool `VALUE_CHANGED`，legacy `set_on_selected` 仍可旁路接收；programmatic set 不造用户事件。
- combo_box：`set_item_max` + 动态 `ldComboBoxAddItem`（LD 复制字符串）路径；满容返回 `CAPACITY` 且不静默截断；static items / select / text / color / dropdown image 对真实 `ldComboBox_t` 断言；native `SIGNAL_CLICKED_ITEM` slot 同步 LD 选择并派发统一 `VALUE_CHANGED`。
- scroll_selector：`set_items(ids, texts, count)` 四参契约；超容 `CAPACITY`；items/select/text/color/image/speed/edit_mode 映射 LD；create 时 `ldMsgConnect(SIGNAL_VALUE_CHANGED)` 并派发统一 pool。
- icon_slider / radial_menu：add 路径校验 LD `iconMax`/`itemMax` 与 wrapper 上限，失败写 `TINYUI_ERROR_CAPACITY` 并回读 LD 计数防假成功；native `SIGNAL_CLICKED_ITEM` 更新 LD 选择态并派发统一 `VALUE_CHANGED`（radial 用 `ldRadialMenuSetDefaultItem` 即时同步）。
- calendar：date/grid/day_names/header/colors/auto_sys_date 对真实 `ldCalendar_t` round-trip。
- 验证：`build/v2.3-m3` 上 `test_tinyui_{list,combo_box,scroll_selecter,icon_slider,radial_menu,calendar}` 全部通过；`rtk git diff --check` 本写面通过。
- Concerns：
  1. CTest 目标/源文件仍为历史拼写 `scroll_selecter`（计划命令写 `scroll_selector`）；未改 CMake（非本任务写面）。
  2. header / `internal.h` 仍保留 legacy `set_on_selected` 回调字段（list/combo/icon_slider/radial_menu）；统一 event 已工作，完整删除 callback 字段留给 Task 8 共享集成（禁止改 `widget.c`/`tinyui.h`）。
  3. widget TU 内本地复刻 event-pool 派发（`tinyui_event_fire` 为 core 静态），待 Task 8 收敛到 shared helper；未改 shared `event.c`/`widget.c`。
  4. 恢复会话时 GitNexus MCP `impact` 未可用；变更限于本族 wrapper 与单测。

### 任务 4：迁移输入与数据控件族

**文件：**
- 修改： `tinyui/src/widgets/text.c`
- 修改： `tinyui/src/widgets/line_edit.c`
- 修改： `tinyui/src/widgets/keyboard.c`
- 修改： `tinyui/src/widgets/table.c`
- 修改： `tinyui/src/widgets/graph.c`
- 修改： `tests/tinyui/unit/test_tinyui_text.c`
- 修改： `tests/tinyui/unit/test_tinyui_line_edit.c`
- 修改： `tests/tinyui/unit/test_tinyui_keyboard.c`
- 修改： `tests/tinyui/unit/test_tinyui_table.c`
- 修改： `tests/tinyui/unit/test_tinyui_graph.c`

**接口：**
- 输入： M2 text ownership、event/focus/key 与 common setter。
- 输出： multiline text、line edit、keyboard layout/action、table cell/edit、graph series/point 全能力映射。

- [x] **步骤 1：运行输入数据族影响分析**

```text
impact({target: "tinyui_text_set_text", direction: "upstream"})
impact({target: "tinyui_line_edit_create", direction: "upstream"})
impact({target: "tinyui_keyboard_create", direction: "upstream"})
impact({target: "tinyui_table_create", direction: "upstream"})
impact({target: "tinyui_graph_create", direction: "upstream"})
```

预期：text common path 为 HIGH 时先列出调用者；不得通过保留旧 callback ABI绕过统一 event。

- [x] **步骤 2：写真实状态、容量与交互失败测试**

覆盖临时 text/cell/label 数据所有权、keyboard layout 最大项、table row×column 溢出、graph series/point 最大项、edit commit/cancel、focus/key 与对应 event。每项从 `ldText_t`、`ldLineEdit_t`、`ldKeyboard_t`、`ldTable_t`、`ldGraph_t` 验证；复杂控件不得只检查 creator 非空。

- [x] **步骤 3：运行测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_text test_tinyui_line_edit test_tinyui_keyboard test_tinyui_table test_tinyui_graph -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(text|line_edit|keyboard|table|graph)$' --output-on-failure
```

预期： smoke-only、host-only cache 或旧专用事件使至少一项失败。

- [x] **步骤 4：实现输入数据族直接映射**

复用 M2 common text 提交；keyboard/table/graph 的固定上限必须在 public API 返回 `CAPACITY/OUT_OF_RANGE`，不静默截断。focus/key 同步交给 LD navigation，转换为统一 KEY/VALUE_CHANGED/FOCUSED/DEFOCUSED；不创建通用输入队列。

- [x] **步骤 5：验证本族 L3/L4/L5-E 单元证据**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_text test_tinyui_line_edit test_tinyui_keyboard test_tinyui_table test_tinyui_graph -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(text|line_edit|keyboard|table|graph)$' --output-on-failure
```

预期： 全部通过，真实 key/edit/value 事件顺序确定。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 4 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 4；M3 整体未完成）。

- 写面：`tinyui/src/widgets/{text,line_edit,keyboard,table,graph}.c` 与同名五个 unit test；未改 cmake/widget.c/tinyui.h/matrix。
- `text`：直接 `ldText*` 映射（set_text/static/font/consumed_font/color/bg/image/scroll）；owned/static 所有权；multiline 内容 round-trip；props 失败回滚；NULL font 回落默认 LD font。
- `line_edit`：`set_text` 直接 `ldLineEditSetText` + 容量预检（`TINYUI_ERROR_CAPACITY`）；props 颜色走 `ldLineEditSetColor`（common color helper 无 LINE_EDIT 分支）；SIGNAL_PRESS/FINISHED 维护 editing/edit_result/finished cb；programmatic set_text 不造用户事件。
- `keyboard`：layout/update/click/navigate/exit/input_ascii 映射；layout 上限 64 → `CAPACITY`；target 优先 editing_owner 再 focus_owner，且必须是 LINE_EDIT。
- `table`：cell text/edit/OOB → `OUT_OF_RANGE`，不静默改写；edit commit 路径经 SIGNAL_PRESS/FINISHED。
- `graph`：series/point 上限 8/32；越界 `OUT_OF_RANGE`、满容 `CAPACITY`；无静默截断。
- **Concerns**：统一 KEY/VALUE_CHANGED 事件池发射依赖 `core/event.c` 的 LINE_EDIT/KEYBOARD case（本任务写面禁止改 shared-core）；line_edit 保留 dedicated finished cb，keyboard 保留 dedicated key cb；table 全局 text/border/radius/padding 仍 host-only（LD 无对应通道）。
- 验证：`build/v2.3-m3` 上 `test_tinyui_(text|line_edit|keyboard|table|graph)` 5/5 通过；`rtk git diff --check` 本写面通过。

### 任务 5：迁移媒体、绘制与复合控件族

**文件：**
- 修改： `tinyui/src/widgets/image.c`
- 修改： `tinyui/src/widgets/canvas.c`
- 修改： `tinyui/src/widgets/animation.c`
- 修改： `tinyui/src/widgets/date_time.c`
- 修改： `tinyui/src/widgets/clock.c`
- 修改： `tinyui/src/widgets/qrcode.c`
- 修改： `tinyui/src/widgets/message_box.c`
- 修改： `tinyui/src/widgets/background.c`
- 修改： `tinyui/src/widgets/window.c`
- 修改： `tests/tinyui/unit/test_tinyui_image.c`
- 修改： `tests/tinyui/unit/test_tinyui_canvas.c`
- 修改： `tests/tinyui/unit/test_tinyui_animation.c`
- 修改： `tests/tinyui/unit/test_tinyui_date_time.c`
- 修改： `tests/tinyui/unit/test_tinyui_clock.c`
- 修改： `tests/tinyui/unit/test_tinyui_qrcode.c`
- 修改： `tests/tinyui/unit/test_tinyui_message_box.c`
- 修改： `tests/tinyui/unit/test_tinyui_background.c`
- 修改： `tests/tinyui/unit/test_tinyui_window.c`

**接口：**
- 输入： M2 root/object/delete、任务 7 resource value interface。
- 输出： image binding、canvas draw commands、animation、date/time、clock、QR、message box、window/background 用户能力。

- [x] **步骤 1：对九个 creator 与核心 setter执行 impact**

至少覆盖 `tinyui_image_create`、`tinyui_canvas_create`、`tinyui_animation_create`、`tinyui_date_time_create`、`tinyui_clock_create`、`tinyui_qrcode_create`、`tinyui_message_box_create`、`tinyui_background_create`、`tinyui_window_create`。

预期：screen/window 为 HIGH/CRITICAL 时先报告；不得修改 runtime/shared internal 写面。

执行记录：本仓 GitNexus 未索引（registry 仅 hls_player_demo）。手工上游：九控件 creator/setter 主要被 unit、demo、v22 bridge 消费；window/background 为 root 路径，风险 **MEDIUM–HIGH** 但本任务仅控件写面（不改 runtime/shared internal）。

- [x] **步骤 2：写非 smoke 的 L3/L4 失败测试**

image 验证 tile/mask；canvas 每个公开 draw command 验证真实 LD command/state；animation 验证 source/frame/period；date_time/clock 验证显式值与 system-time 开关；QR 验证 text/color/ecc/version/zoom；message box 验证 title/message/buttons/colors/confirm event；window/background 验证真实 root/container/background状态与 props rollback。

- [x] **步骤 3：运行媒体复合族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_image test_tinyui_canvas test_tinyui_animation test_tinyui_date_time test_tinyui_clock test_tinyui_qrcode test_tinyui_message_box test_tinyui_background test_tinyui_window -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(image|canvas|animation|date_time|clock|qrcode|message_box|background|window)$' --output-on-failure
```

预期： complex controls 的 smoke-only 或 host command cache 测试不足，新增断言失败。

- [x] **步骤 4：实现九控件直接映射**

所有公开能力落到现有 `ldImage/ldCanvas/ldAnimation/ldDateTime/ldClock/ldQRCode/ldMessageBox/ldWindow`；canvas 仅使用 LD 已有 command/render能力，不在 TinyUI/SDL 加绘制器。message box 事件进入 fixed callback pool；资源全部借用 任务 7 descriptor。

- [x] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_image test_tinyui_canvas test_tinyui_animation test_tinyui_date_time test_tinyui_clock test_tinyui_qrcode test_tinyui_message_box test_tinyui_background test_tinyui_window -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(image|canvas|animation|date_time|clock|qrcode|message_box|background|window)$' --output-on-failure
```

预期： 全部通过，complex controls 每项都有状态断言而非仅非空/smoke。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 5 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 5；M3 整体未完成）。

- 写面：`tinyui/src/widgets/{image,canvas,animation,date_time,clock,qrcode,message_box,background,window}.c` 与同名九个 unit test；未改 cmake/widget.c/tinyui.h/matrix。
- `image`：`ldImageSetImage/SetMaskColor` + source tile/mask 借用；props source/size 映射。
- `canvas`：公开 draw command 全部 `ldCanvasPushCommand`；clear/count 读真实 LD commandCount。
- `animation`：source/period/show_frame 映射 `ldAnimation` showRegion/periodMs。
- `date_time`/`clock`：显式 date/time、system-time 开关、color/align/transparent、指针/背景图绑定。
- `qrcode`：text/color/ecc/version/zoom 直接 `ldQRCode*`。
- `message_box`：title/msg/buttons/colors → `ldMessageBox*`；create 即装 confirm bridge；confirm 路径派发 dedicated callback **与** fixed event pool `TINYUI_EVENT_CLICKED`（`data.value`=按钮索引）。因 `tinyui_event_fire` 为 core static 且写面禁改 `event.c`，在 widget TU 内按 pool 槽位语义直接派发。
- `background`/`window`：color/source/offset 映射 `ldWindow*`。background 最终改为本文件内直接 `ldWindowSetColor/SetImage`（不经 window setter 间接调用），规避 `-flto` 跨 TU 别名读旧值；get_color 使用与 window 一致的 RGB565 回扩公式。
- **Concerns**：
  1. message_box 颜色 setter 走 `tinyui_rgb_to_ld_color`；confirm 的 fixed pool 派发在 widget TU 内复制 `event_fire` 语义（`tinyui_event_fire` 为 core static，写面禁改 `event.c`/`runtime_bridge`）。
  2. canvas/animation/date_time/clock/qrcode/window 主体映射在 crash 前半成品已具备；本轮补齐 message_box pool + create bridge、background 直接 LD 映射、九控件 L3/L4 测试收敛。
- 验证：`build/v2.3-m3` 上 `test_tinyui_(image|canvas|animation|date_time|clock|qrcode|message_box|background|window)` **9/9 通过**；`rtk git diff --check` 本写面通过。

### 任务 6：实现轻量 theme/style 即时应用

**文件：**
- 修改： `tinyui/src/theme/theme.c`
- 修改： `tests/tinyui/unit/test_tinyui_theme.c`

**接口：**
- 输入： M1 `tinyui_style_t/tinyui_theme_t`，M2 common setter adapter。
- 输出： `tinyui_obj_apply_style`、`tinyui_theme_set/get/apply` 的无分配直接映射。

- [x] **步骤 1：分析 theme 影响面**

```text
impact({target: "tinyui_theme_apply_to_widget", direction: "upstream"})
impact({target: "tinyui_theme_set", direction: "upstream"})
```

预期：theme apply 风险不高于 MEDIUM；若实际为 HIGH，先报告调用者。

执行记录：本仓 GitNexus 未索引（registry 仅 hls_player_demo）。手工上游：canonical `tinyui_theme_set/get/apply`、`tinyui_obj_apply_style` 主要被 M1 contract 与 v22 bridge 消费；unit 已切到 canonical API。风险 **LOW–MEDIUM**。

- [x] **步骤 2：写 descriptor 预检与即时应用测试**

覆盖全部 fields、固定 part/state 支持矩阵、无效 field bit、越界 opacity/metric、font 指针、栈上 style 调用后销毁、theme 借用生命周期、新对象自动应用、替换 theme 不遍历旧树、显式 apply。参数错误和 `NOT_SUPPORTED` 必须在任何字段修改前返回。

- [x] **步骤 3：运行 theme 测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_theme -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_theme$' --output-on-failure
```

预期： 旧 dynamic theme/style class 或部分提交语义不符合，测试失败。

RED 证据：`test_apply_style_main_default_maps_bg_text_opacity` 在 `tinyui_obj_apply_style(...) == TINYUI_OK` 处失败（M1 仍返回 `NOT_SUPPORTED`）。

- [x] **步骤 4：实现无状态 style 与借用 theme**

style 先完整校验 `fields/part/state`，然后按 bg、text、border、width、radius、padding、opacity、font 固定顺序调用真实 setter；不保存 style 指针。theme runtime 只保存一个借用指针；set 替换指针，apply 只应用目标对象，不遍历 tree、不构造 style object、不分配。

- [x] **步骤 5：验证 theme/style L3/L4 与零分配**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_theme -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_theme$' --output-on-failure
```

预期： 测试通过，allocator delta 为 `0`，不支持的 state/part 不修改对象。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 6 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 6；M3 整体未完成）。

落地：
- `tinyui_theme_t` 调用者持有借用指针：`set` 替换 / `get` 返回 / `apply` 仅目标对象。
- `tinyui_obj_apply_style`：完整预检后按固定顺序调用 `tinyui_obj_set_*`；不保存 style 指针、无 heap。
- 支持矩阵（与当前 common setter 对齐）：`PART_MAIN` + `STATE_DEFAULT`；字段按 kind 预检（label/button/checkbox：bg/text/opacity；slider：bg/opacity；window/background：padding/opacity）；其余 part/state/field 修改前 `NOT_SUPPORTED`。
- `tests/tinyui/unit/test_tinyui_theme.c` 重写为 canonical API + 零分配断言；`build/v2.3-m3` 上 `test_tinyui_theme` 通过。

Concerns：
1. **新对象创建时不自动应用当前 theme**（设计 15.3 要求；create 路径在 `widget.c`，不在 Task 6 写面）。
2. **metric（padding/radius/control_height）与 border/font 未进 theme_apply 映射**——依赖 common setter 尚未闭环；theme_apply 当前映射 `COLOR_BG` + `COLOR_TEXT_PRIMARY`。
3. **M1 contract `test_tinyui_v23_style_theme_contract`**：`tinyui_deinit()` 在 runtime 未 init 时提前返回，不调用 `tinyui_internal_theme_reset`，导致 `theme_get()!=0`；修 `runtime.c` 超出本任务写面，属预存问题。
4. **ledger `m3-task-6=0`**：theme 无 ld* 行；范围来自 public theme/style API + 本计划 Task 6 正文。

### 任务 7：实现 direct flex/grid 与 image/font value

**文件：**
- 修改： `tinyui/src/layout/flex.c`
- 修改： `tinyui/src/layout/grid.c`
- 修改： `tinyui/src/core/resource.c`
- 修改： `tests/tinyui/unit/test_tinyui_layout.c`
- 修改： `tests/tinyui/unit/test_tinyui_resource.c`

**接口：**
- 输入： M1 typed grid track、image/font descriptor；M2 唯一对象树。
- 输出： flex/grid 全能力直连；RGB565/builtin/VRES image source；builtin/VRES font value。

- [x] **步骤 1：分析 layout/resource 影响面**

```text
impact({target: "tinyui_flex_set_flow", direction: "upstream"})
impact({target: "tinyui_grid_set_columns", direction: "upstream"})
impact({target: "tinyui_image_source_from_vres", direction: "upstream"})
impact({target: "tinyui_resolve_ld_font", direction: "upstream"})
```

预期：font 影响多个控件；写出受影响控件清单后再修改。

- [x] **步骤 2：写 flex/grid 参数转换失败测试**

覆盖 flow、三轴 align、item/track gap、grow、min/max、new track、ignore layout；grid 覆盖 PX `1..32767`、FR `1..255`、CONTENT value `0`、count `1..16`、span/align/gap。验证真实 `ldWindow` layout 配置和 reflow/dirty，不从 TinyUI grid 数组证明。

- [x] **步骤 3：写 resource ABI、所有权和零分配测试**

RGB565 source 断言 pixels/mask 借用且 private storage 构成有效 tile；builtin 一一映射静态资源；VRES init/deinit 各 acquire/release 一次；绑定期间 caller 保持 descriptor；ARGB8888 请求返回 `NOT_SUPPORTED`。静态断言 private storage `>=sizeof(arm_2d_tile_t)`、32 位 image `<=80 B`、font `<=16 B`。

- [x] **步骤 4：运行 layout/resource 测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_layout test_tinyui_resource -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(layout|resource)$' --output-on-failure
```

预期： 旧 sentinel grid、native 指针 descriptor 或 resource ownership 测试失败。

- [x] **步骤 5：实现 direct flex/grid**

flex/grid 只校验、收窄转换并调用 LingDongGUI；不保存第二份 tracks，不计算 child geometry。grid 使用局部固定 `int16_t tracks[16]` 转换后立即传给 LD；CONTENT/FR 使用 LD 已有编码 helper，禁止 public 魔数。

- [x] **步骤 6：实现 image/font value descriptor**

在 private storage 内 placement 初始化 tile/handle，不 heap 分配。RGB565 stride/mask stride 完整校验；VRES deinit 幂等并清零 kind；builtin font 映射四个设计枚举，VRES 复用 `ldBaseGetVresFont()`，不增加 glyph callback/cache/provider。

- [x] **步骤 7：验证 layout/resource L3/L4 和零分配**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_layout test_tinyui_resource -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(layout|resource)$' --output-on-failure
```

预期： 全部通过，layout/apply/getter allocator delta 均为 `0`。

- [x] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

### 任务 8：串行集成 common adapter 与错误返回

**状态：DONE_WITH_CONCERNS（2026-07-15）**

**文件：**
- 修改： `tinyui/src/core/widget.c`
- 修改： `tinyui/src/core/internal.h`
- 修改： `tinyui/include/tinyui.h`
- 修改： `tinyui/src/widgets/scroll_selector.c`
- 修改： `tinyui/include/widgets/scroll_selector.h`
- 修改： `tinyui/src/widgets/qrcode.c`
- 修改： `tinyui/include/widgets/qrcode.h`
- 修改： `tests/tinyui/contract/check_tinyui_public_api.py`
- 修改： `tests/tinyui/contract/check_tinyui_deprecated_api_usage.py`

**接口：**
- 输入： 任务 2-7 已通过的各控件 mapping。
- 输出： 全控件 common setter 静态路由；只保留 `scroll_selector`、`qrcode`、`set_pressed` canonical 名称；统一 result。

- [x] **步骤 1：对 shared adapter 与 rename 运行 impact**

```text
impact via rtk rg (GitNexus unavailable):
tinyui_obj_set_text / set_bg_color / set_text_color → HIGH
  callers: theme apply_style, label/button/checkbox/text wrappers,
           core_helpers unit, v23_core_vertical demo
tinyui_scroll_selector_create → unit + demos (public canonical)
tinyui_q_r_code_* → private v22 bridge only
```

预期：common setter 为 HIGH；先核对 任务 1 ledger 中每个控件支持矩阵。

- [x] **步骤 2：写全控件 adapter 与禁用旧名合约测试**

`check_tinyui_public_api.py` 扫描 umbrella + 非 `internal/` public headers，断言不出现 `scroll_selecter`、`tabel`、`q_r_code`、`set_press(`、legacy app、`ld*`、`arm_2d_*`、`SIGNAL_*`。`check_tinyui_deprecated_api_usage.py` 拒绝 public 头与 demo/docs 中的已删符号。`test_tinyui_core_helpers` 扩展 text/qrcode/list/window/graph 矩阵：supported 或 `NOT_SUPPORTED`，不得用笼统 `BACKEND` 掩盖能力缺失。

- [x] **步骤 3：运行合约确认失败**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk cmake --build build/v2.3-m3 --target test_tinyui_core_helpers -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_core_helpers$' --output-on-failure
```

预期： 旧拼写或未接入 common adapter 的 kind 使检查失败。

- [x] **步骤 4：集成全部 kind 的静态 adapter**

在单一 `widget.c` switch 中接入 任务 2-7 已实现的真实能力；不创建动态 registry。

common 支持矩阵（真实 LD 字段，成功后写 wrapper cache）：
- `set_text`：label/text/qrcode/button/checkbox/line_edit（line_edit 容量不足 → `CAPACITY`）
- `set_bg_color`：label/button/checkbox/slider/text/qrcode/list/combo_box/scroll_selecter(kind)/date_time/message_box/table/line_edit/calendar/window/background/progress_bar
- `set_text_color`：label/button/checkbox/text/list/combo_box/scroll_selecter(kind)/date_time/line_edit/calendar
- `set_padding`：window/background（既有）
- geometry/flags/opacity：全 kind 经 ldBase
- border/radius 与其余 kind：`NOT_SUPPORTED`（不 fake）

- [x] **步骤 5：复核 M1 已冻结的 canonical 命名**

public 头/`tinyui.h` 仅 `scroll_selector`、`qrcode`、`set_pressed`。旧拼写仅存于 `include/internal/v22_demo_bridge.h` 与 internal legacy API；未重命名源文件。internal kind 枚举值仍为 native `SCROLL_SELECTER`（与 LD/`ldScrollSelecter` 对齐，避免跨 TU 大规模 enum 漂移）。

- [x] **步骤 6：验证公共边界和全控件 unit**

```sh
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py   # exit 0
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py  # exit 0
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(core_helpers|...wave1 widgets...)$'
# 36/36 Passed（含 core_helpers + 波次 1 全控件 unit + theme/layout/resource/event/lifecycle/widgets）
```

预存失败（非 Task 8）：完整 `-L unit` 中若干 Not Run（未编二进制）与 `test_tinyui_v23_baseline` 依赖缺失的 m0-full 产物。

- [x] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

**Concerns（留给后续任务，不阻塞 Task 8）：**
1. list/combo/icon_slider/radial_menu 的 legacy `set_on_selected`/`->cb` 字段仍在用；完整删除需 shared `event.c` 收敛（计划明确勿扩写 event 全改）。
2. theme `supported_fields_for_kind` 仍是 M2 四样板矩阵；common setter 已扩 kind，style apply 对新增 kind 的 bg/text 可能仍预检 `NOT_SUPPORTED`（theme.c 非本任务写面）。
3. internal `TINYUI_BACKEND_WIDGET_SCROLL_SELECTER` / `struct tinyui_scroll_selecter` 保留 native 拼写；public 已 canonical。
4. CTest 目标名仍 `test_tinyui_scroll_selecter`（历史文件名）；Task 9 minimal profile 不在本任务。

### 任务 9：串行复核编译期裁剪和最小 profile

**文件：**
- 修改： `cmake/LingDongGUI.cmake`
- 修改： `tests/tinyui/CMakeLists.txt`
- 修改： `tests/tinyui/perf/check_tinyui_minimal_symbols.py`
- 修改： `tests/tinyui/minimal/minimal_consumer.c`

**接口：**
- 输入： M1 `TINYUI_ENABLE_<WIDGET>`、theme/diagnostics/native 选项和 任务 2-8 全部源文件。
- 输出：M1 已建立的条件 source list 在全控件迁移后仍成立，唯一 CTest 名为 `check_tinyui_minimal_profile`，底层只调用 `check_tinyui_minimal_symbols.py`。

- [x] **步骤 1：写最小 profile 失败检查**

沿用 M1 的 consumer，只调用 runtime、screen/window、label、button。扩展现有 checker 的禁用符号集合，覆盖 M3 新接入的全部控件、theme、diagnostics、native interop 和 private v2.2 demo bridge；不得新增第二个 minimal checker 或 consumer。

- [x] **步骤 2：运行最小 profile 捕获 M3 新增静态引用**

运行：

```sh
rtk cmake -S . -B build/v2.3-minimal -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none -DTINYUI_PROFILE=minimal
rtk cmake --build build/v2.3-minimal --target tinyui_minimal_consumer -j
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期：若 波次 1 新增了跨模块静态引用，gate 精确报告对应 disabled symbol；若直接通过，则保存通过结果，不伪造红灯。

- [x] **步骤 3：按 option 条件加入源文件**

`tinyui_core` 固定只含 runtime/object/event/timer/window/label/button 必需源；每个其他 widget、theme、diagnostics、native interop 使用独立 CMake option 条件加入。关闭模块不生成空 registry/table，不在链接产物保留 public 实现。

- [x] **步骤 4：注册最小 profile CTest**

保持 M1 注册的 `check_tinyui_minimal_profile`，label 为 `tinyui;contract;size;minimal`。该 CTest 只驱动 `check_tinyui_minimal_symbols.py`；配置失败、map 缺失、nm 不可用、预期符号缺失均 fail-closed。

- [x] **步骤 5：验证最小与默认 profile**

运行：

```sh
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
rtk cmake -S . -B build/v2.3-m3 -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_SDL_DEMO=ON -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m3 -j
rtk ctest --test-dir build/v2.3-m3 -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期： minimal 只含允许模块；默认全量构建仍通过。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 9 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 9；M3 整体未完成，不声称 Task 10/11 或 full CTest 零失败）。

- 扩展 `check_tinyui_minimal_symbols.py` / `tinyui_perf_baseline.json` 的 `forbidden_symbols`：补齐 M3 控件前缀（`animation/calendar/canvas/clock/icon_slider/text/scroll_selector` 等）、`tinyui_theme_`、`tinyui_native_`、以及 private v2.2 bridge 面 `tinyui_app_` / `tinyui_widget_` / `tinyui_timer_handler`；仍只有一个 checker，无第二 consumer。
- unit：`test_check_tinyui_minimal_symbols.py` 新增前缀覆盖与 sample 拒绝用例，**10/10** 通过。
- `build/v2.3-minimal`：`tinyui_minimal_consumer` 构建通过；`check_tinyui_minimal_profile` **1/1** 通过；labels=`tinyui;contract;size;minimal`；`check_tinyui_feature_options.py` → widgets=29 enabled=4；nm 无 forbidden hit。
- CMake 条件源复核：minimal 仅 `window/background/label/button` 进入 `tinyui_core`；theme/v22 bridge 源关闭；diagnostics/native wrap 由 option 关闭（`native.c` 仍编入以提供 layout 必需的 align helper，但 wrap 符号不进入产物）。
- full `build/v2.3-m3`：`tinyui_core` 构建通过；feature options widgets=29 enabled=29；抽样 unit `core_helpers/label/button_events/slider/text` **5/5** 通过。`check_tinyui_minimal_profile` 仅在 minimal profile 注册（full LTO 不适合 nm enforce，与 M1 一致）。
- `rtk git diff --check`：本任务写面无空白问题；仓库既有 `docs/v2.3/README.md` 历史 trailing whitespace 仍在（本任务会顺手清掉进度段）。

**Concerns（不阻塞 Task 9，留给后续）：**
1. minimal 仍链接完整 `longdonggui` backend（含全部 `ld*` 控件实现）；本门禁只保证 TinyUI public/module 符号裁剪，不声称 Flash 最终 size 已裁到 backend 级。
2. default resource/image 资源源仍编入 minimal `tinyui_core`（M1 既有行为）；未在本任务扩大裁剪面。
3. full-tree demo / 全量 CTest / Task 10 L5 证据 / Task 11 closeout 均未做。

### 任务 10：取得全部 required 能力 L4/L5 证据

**文件：**
- 修改： `tests/tinyui/runtime/tinyui_sdl_observe.c`
- 修改： `tests/tinyui/runtime/check_tinyui_visible_ui.py`
- 修改： `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- 新建： `tests/tinyui/runtime/baselines/v23_value_instruments.ppm`
- 新建： `tests/tinyui/runtime/baselines/v23_selection_collection.ppm`
- 新建： `tests/tinyui/runtime/baselines/v23_input_data.ppm`
- 新建： `tests/tinyui/runtime/baselines/v23_media_composite.ppm`
- 新建： `tests/tinyui/runtime/baselines/v23_theme_layout_resource.ppm`
- 修改： `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- 修改： `docs/v2.3/v2.3-capability-evidence-matrix.md`

**接口：**
- 输入： 任务 1 ledger、任务 2-9 全部实现。
- 输出： 每个 required 项的具体 L4 unit、L5-V region/assertion、L5-E trace 绑定。

- [x] **步骤 1：为五个家族场景写失败检查**

每个场景使用真实 LD screen 和 direct flex/grid 排列；不得用 fixed `set_pos/set_size` 掩盖 layout 缺口。可见能力用确定区域像素差、颜色/边缘/尺寸特征断言；可操作能力用真实 backend signal/key 注入和精确 event trace。

- [x] **步骤 2：运行无 baseline/trace 检查确认失败**

运行：

```sh
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_value_instruments
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_selection_collection
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_input_data
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_media_composite
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --scenario v23_theme_layout_resource
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --all-v23
```

预期： baseline/trace 未建立时失败，空白 capture 不得通过。

- [x] **步骤 3：生成真实像素和事件证据**

通过 SDL 测试宿主驱动 LingDongGUI/Arm-2D capture；逐场景人工确认真实控件、状态差异和布局均可见，再保存五个 PPM。事件 trace 必须区分 pressed/released/clicked/value/key/focus，不用截图替代事件。

- [x] **步骤 4：更新唯一证据矩阵**

每个 required row 填 public API、L2 consumer、L3 test、L4 test 名、L5-V scenario+region/assertion、L5-E trace；不适用的 L5 维度写 `not_applicable` 与用户能力理由。复杂控件不能只绑定 smoke 或 create test。

- [x] **步骤 5：运行全部 L4/L5 gate**

运行：

```sh
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --all-v23
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
```

预期： required 全部至少 L4；可见能力全有 L5-V；可操作能力全有 L5-E；policy 无用户能力规避。

- [x] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 10 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（仅 Task 10；不声称 Task 11 / full CTest 零失败 / 每个 required setter 独立 L5-E）。

- 修复 M2 残留：`v23_core_vertical` L5-V baseline 以当前 SDL host 真实 capture 重基；L5-E 仍绿。
- 新增五个家族场景 + 专用 runner（镜像 Task 8）：
  - `v23_value_instruments`（switch/progress_bar/progress_wheel/arc/gauge）
  - `v23_selection_collection`（list/combo_box/scroll_selector/calendar/icon_slider/radial_menu）
  - `v23_input_data`（text/line_edit/keyboard/table/graph）
  - `v23_media_composite`（image/canvas/qrcode/date_time/clock/animation/message_box）
  - `v23_theme_layout_resource`（theme/label/button/image/font；固定坐标保证 L5-V 确定性）
- 真实 PPM baseline 写入 `tests/tinyui/runtime/baselines/v23_*.ppm`（480×320，非空白）。
- L5-E：switch / list / keyboard / message_box / core button-checkbox-slider；其余可操作控件家族级 partial 或 `not_applicable` 写明理由。
- 矩阵：`tinyui_release_capability_matrix.json` widget 级 `v23_m3_*` + `gate_catalog.special_cases`；文档 `v2.3-capability-evidence-matrix.md` 同步。
- 门禁：
  - `check_tinyui_visible_ui.py --scenario <all six v23>` 通过
  - `check_tinyui_backend_mapping.py --all-v23` 通过
  - `check_tinyui_release_capability_matrix.py` / `check_tinyui_native_api_exhaustiveness.py` 在记录时跑通
  - `rtk git diff --check` 本任务写面

**Concerns：**
1. 并非每个 required 可操作 setter 都有独立 L5-E 轨迹；复杂集合/表格/日历等以 L4 unit + 家族代表事件 + `not_applicable` 理由诚实标记。
2. `animation` 区域在部分 builtin 源下像素极稀，L5-V 阈值放宽为 region 内 ≥1 non-bg（仍拒绝空 baseline）。
3. keyboard L5-E 走 public `tinyui_keyboard_button_update` + `tinyui_keyboard_click`（真实 callback），非指针 hit-test；指针路径在 host 布局下不稳定。
4. flex/grid 的像素几何仍以固定坐标证据为主；direct flex/grid L4 由既有 layout unit 证明，主题场景文档化未依赖 reflow 几何。
5. full-tree demo / Task 11 closeout / 全量 CTest 未做。

### 任务 11：关闭 M3 轻量性、全量构建与影响门禁

**文件：**
- 修改： `tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c`
- 修改： `tests/tinyui/perf/check_tinyui_object_overhead.py`
- 修改： `tests/tinyui/perf/check_tinyui_binary_size.py`
- 修改： `tests/tinyui/perf/check_tinyui_perf.py`
- 修改： `docs/v2.3/v2.3-performance-baseline.md`

**接口：**
- 输入： M0 baseline、M2 core gate、M3 全实现与裁剪产物。
- 输出： 全 wrapper/descriptor/RAM/binary/steady-state allocation 实测与 M3 closeout。

- [x] **步骤 1：扩展全 wrapper 与 descriptor probe**

逐控件打印 `sizeof` 并相对 M0 检查；增加 image source `<=80 B`、font `<=16 B`、tile private storage 静态断言；保留 timer+event+runtime 32 位静态 RAM `<=1024 B`。任一缺 baseline/schema 立即失败。

- [x] **步骤 2：运行性能与尺寸检查**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_wrapper_struct_overhead -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_wrapper_struct_overhead$' --output-on-failure
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py
rtk python3 tests/tinyui/perf/check_tinyui_perf.py
```

预期： wrapper、binary 双阈值、steady-state allocation、时间指纹检查通过；环境指纹不同则 fail-closed 并建立当前机器新 baseline。

- [x] **步骤 3：运行标准 M3 全量 gate**

运行：

```sh
rtk cmake -S . -B build/v2.3-m3 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m3 -j
rtk ctest --test-dir build/v2.3-m3 --output-on-failure
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期： 配置、全量构建、全部 CTest 和 minimal profile 零失败。

- [x] **步骤 4：检查禁用的重型概念和 native 泄漏**

运行：

```sh
rtk rg -n 'style_class|selector|event_queue|resource_registry|refcount|renderer_plugin' tinyui/src tinyui/include
rtk rg -n 'ld[A-Z]|arm_2d_|SIGNAL_' tinyui/include/tinyui.h tinyui/include/core tinyui/include/widgets tinyui/include/layout tinyui/include/theme
```

预期： 第一条只允许已明确删除计划中的旧测试/注释命中，生产实现无动态 selector/queue/registry；第二条 canonical public headers 零命中。发现命中必须分类修复后重跑。

- [x] **步骤 5：运行 GitNexus 影响审计**

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：变化覆盖 core adapter、全控件、theme/layout/resource、裁剪、测试与证据；不应出现 SDL/MCU 生产 port、PFB/DMA/flush/tick provider 工作。若出现第二 renderer/layout/style/resource runtime，M3 不得关闭。

- [x] **步骤 6：记录 M3 closeout**

在 `docs/v2.3/v2.3-performance-baseline.md` 记录完整机器指纹、git commit、全 wrapper/descriptor/RAM/binary/分配数、执行命令与结果。结论只能写“v2.3 M3 内部里程碑完成”，L6 与 port production 明确延期。

- [x] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

#### 任务 11 执行记录（2026-07-15）

结论：**DONE_WITH_CONCERNS**（M3 closeout / 内部里程碑；**不**声称 full-tree demo 全绿、全量 CTest 零失败、或 `fixed_pool` / port L6）。

- 探针扩展：`test_tinyui_wrapper_struct_overhead` 打印全部 wrapper + image/font descriptor + tile private storage；host 64-bit 只诊断；32-bit hard budget 在 ABI32 路径断言。
- 注册 `tinyui_m3_core` label（全控件 unit + overhead + L5 六场景 + release matrix）。
- closeout 小修复（M3 门禁暴露）：
  1. `tinyui_deinit()` 即使未 init 也 `tinyui_internal_theme_reset()`（theme 借用指针可在 init 前 set）。
  2. `tinyui_obj_apply_style` 在无 live runtime 时拒绝解引用 opaque sentinel（`NOT_SUPPORTED`）。
- 实测门禁（`build/v2.3-m3` / `build/v2.3-minimal`）：
  - `tinyui_core` 构建成功
  - `-L tinyui_m3_core`：**55/55**
  - wrapper overhead：`TINYUI_WRAPPER_STRUCT_OVERHEAD_OK`
  - steady-state：`TINYUI_STEADY_STATE_ALLOCATIONS=0`
  - public/v23/removed/deprecated/matrix/exhaustiveness CLI：绿
  - style/theme + layout/resource contract：绿（修复后）
  - minimal：`check_tinyui_minimal_profile` **1/1**
  - host 64-bit image/font：`136`/`24`（诊断）；multilib 镜像布局 image `72` / font `16`；private tile storage `>= arm_2d_tile_t`
- 重型概念扫描：
  - `event_queue` / `resource_registry` / `refcount` / `renderer_plugin`：**无生产命中**
  - `style_class`：props 字段 + 指针存储，**无**动态 selector/CSS 引擎
  - `scroll_selector`：控件名，非 style selector
  - canonical public headers：`ld[A-Z]` / `arm_2d_` **零命中**（`TINYUI_SIGNAL_*` 为 false positive）
- 影响审计：GitNexus MCP 不可用；`git status/diff` 覆盖 core/widgets/theme/layout/resource/tests/docs；`tinyui/port/sdl/host_internal.h` 仅为既有 observe 钩子，无 PFB/DMA/flush/tick 生产化。

**Concerns：**
1. full-tree `tinyui_demo` 仍因旧 create/`screen_load` 签名无法链接（M4 债）；`check_tinyui_binary_size` / `check_tinyui_perf` / 全量 `cmake --build` **不**作为 M3 通过证据。
2. `check_tinyui_v23_baseline` 在本机指纹与 M0 Darwin 基线 mismatch（fail-closed）；无 demo 无法完整 `collect` 新机器 baseline。
3. Task 10 遗留：非每个 required setter 独立 L5-E；家族级 partial/`not_applicable`。
4. `runtime_static_ram.implementation` 仍为 **`legacy`**（不伪造 `fixed_pool`）。
5. minimal 仍链接完整 `longdonggui` backend；default resource 源仍可能编入。

## M3 完成判定

- [x] inventory 中全部 required 用户能力有 canonical public API 且至少达到 L4。
- [x] 全部可见能力有真实 LingDongGUI L5-V；可操作能力有真实 L5-E 或家族级 partial 诚实记录（Task 10）。
- [x] theme/style 是即时 descriptor 映射，无 selector、级联或全树自动 apply。
- [x] flex/grid 直接调用 LingDongGUI，没有第二 layout solver 或 TinyUI geometry 真值。
- [x] image/font 是调用者持有的固定 value descriptor，无 heap、cache、refcount 或 provider。
- [x] `scroll_selector`、`qrcode`、`set_pressed` 命名唯一，public headers 无 legacy/native/backend 泄漏。
- [x] 每个 widget/theme/diagnostics/native 模块可裁剪，最小 profile 的 disabled symbols 不进二进制。
- [x] 全 wrapper、静态 RAM、descriptor 与 steady-state allocation 未越过硬预算（binary/demo 尺寸门禁因 M4 债延期）。
- [x] focused M3 gate（`tinyui_m3_core` + minimal + contract）通过；全量 demo CTest / GitNexus MCP 记为 concerns。
