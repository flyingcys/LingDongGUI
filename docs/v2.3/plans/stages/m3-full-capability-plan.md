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

- [ ] **步骤 1：重新生成并检查 LingDongGUI public inventory**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_native_100_inventory.py
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
```

预期： inventory 无漂移；若失败先修真相源，不得在 ledger 中隐藏新用户能力。

- [ ] **步骤 2：逐项分类全部控件能力**

对 animation、arc、background、button、calendar、canvas、checkbox、clock、combo_box、date_time、gauge、graph、icon_slider、image、keyboard、label、line_edit、list、message_box、progress_bar、progress_wheel、qrcode、radial_menu、scroll_selector、slider、switch、table、text、window 的每项 public LD 能力记录：canonical TinyUI API、direct/shared 等价关系、owner task、L4 test、是否可见、是否可操作。

只有 backend-private helper 可标 `policy_never_public`，且必须写具体理由与 inventory symbol。

- [ ] **步骤 3：验证 ledger schema 与唯一 owner**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk jq -e '[.capabilities[]?.owner_task] | all(. != null)' tests/tinyui/contract/native_api_gap_ledger.json
```

预期： 每项有且只有一个 owner；required 项没有 policy 规避；命令返回 0。

- [ ] **步骤 4：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：对本族所有被修改符号运行 impact**

至少调用：

```text
impact({target: "tinyui_switch_set_checked", direction: "upstream"})
impact({target: "tinyui_arc_create", direction: "upstream"})
impact({target: "tinyui_gauge_create", direction: "upstream"})
impact({target: "tinyui_progress_bar_set_percent", direction: "upstream"})
impact({target: "tinyui_progress_wheel_set_percent", direction: "upstream"})
```

预期：调用者仅对应控件 test/demo/shared common API；任何 HIGH 风险先报告。

- [ ] **步骤 2：为每个 ledger 项写失败测试**

每个现有 unit 文件逐项执行 canonical API，再读取对应 `ldSwitch_t`、`ldArc_t`、`ldGauge_t`、`ldProgressBar_t`、`ldProgressWheel_t` 字段或 getter。参数边界、错误 kind、props 中途失败回滚、真实 `VALUE_CHANGED` 事件必须覆盖；programmatic setter 不制造用户事件。

- [ ] **步骤 3：运行本族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_switch test_tinyui_arc test_tinyui_gauge test_tinyui_progress_bar test_tinyui_progress_wheel -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(switch|arc|gauge|progress_bar|progress_wheel)$' --output-on-failure
```

预期： 未迁移 setter、旧 callback 或 wrapper-only getter 使至少一项失败。

- [ ] **步骤 4：实现本族直接 LD 映射**

每个 setter 采用“全部预检 -> `ld*` -> 最小缓存提交”；专用 callback 转发统一 event pool；getter 有 LD getter/字段时直接读 backend。删除能由 backend 查询的镜像字段，不新增通用表或渲染路径。

- [ ] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_switch test_tinyui_arc test_tinyui_gauge test_tinyui_progress_bar test_tinyui_progress_wheel -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(switch|arc|gauge|progress_bar|progress_wheel)$' --output-on-failure
```

预期： 全部通过；每个 ledger required 项有真实 LD 状态断言。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：运行选择族影响分析**

```text
impact({target: "tinyui_list_set_selected_index", direction: "upstream"})
impact({target: "tinyui_combo_box_create", direction: "upstream"})
impact({target: "tinyui_scroll_selector_create", direction: "upstream"})
impact({target: "tinyui_icon_slider_create", direction: "upstream"})
impact({target: "tinyui_radial_menu_create", direction: "upstream"})
impact({target: "tinyui_calendar_create", direction: "upstream"})
```

预期：命名清理影响 demo/docs 留给 M4；本任务只实现 M1 已冻结 canonical symbol。

- [ ] **步骤 2：写容量、所有权与 L4 失败测试**

对 items 增删/替换、空集合、最大容量、越界 index、临时文本复制、selection getter、focus/key 导航、`VALUE_CHANGED` payload 写断言。所有成功操作从真实 LD item buffer、selected index、calendar grid/date 字段验证；第 `capacity+1` 项返回 `CAPACITY`，不得静默截断。

- [ ] **步骤 3：运行选择族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_list test_tinyui_combo_box test_tinyui_scroll_selector test_tinyui_icon_slider test_tinyui_radial_menu test_tinyui_calendar -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(list|combo_box|scroll_selector|icon_slider|radial_menu|calendar)$' --output-on-failure
```

预期： wrapper item 镜像、旧 callback 或错拼 public 路径导致失败。

- [ ] **步骤 4：实现选择族真实映射**

使用各自现有 `ld*SetItems/SetSelect/SetDate` 能力；TinyUI 只保留 LD 要求调用者保持的稳定文本副本或 backend 无 getter 的必要标量。删除独立 callback 字段，统一 dispatch `VALUE_CHANGED`；复杂状态由控件 getter查询，不扩大 event payload。

- [ ] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_list test_tinyui_combo_box test_tinyui_scroll_selector test_tinyui_icon_slider test_tinyui_radial_menu test_tinyui_calendar -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(list|combo_box|scroll_selector|icon_slider|radial_menu|calendar)$' --output-on-failure
```

预期： 全部通过，容量和 selection 结果明确。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：运行输入数据族影响分析**

```text
impact({target: "tinyui_text_set_text", direction: "upstream"})
impact({target: "tinyui_line_edit_create", direction: "upstream"})
impact({target: "tinyui_keyboard_create", direction: "upstream"})
impact({target: "tinyui_table_create", direction: "upstream"})
impact({target: "tinyui_graph_create", direction: "upstream"})
```

预期：text common path 为 HIGH 时先列出调用者；不得通过保留旧 callback ABI绕过统一 event。

- [ ] **步骤 2：写真实状态、容量与交互失败测试**

覆盖临时 text/cell/label 数据所有权、keyboard layout 最大项、table row×column 溢出、graph series/point 最大项、edit commit/cancel、focus/key 与对应 event。每项从 `ldText_t`、`ldLineEdit_t`、`ldKeyboard_t`、`ldTable_t`、`ldGraph_t` 验证；复杂控件不得只检查 creator 非空。

- [ ] **步骤 3：运行测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_text test_tinyui_line_edit test_tinyui_keyboard test_tinyui_table test_tinyui_graph -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(text|line_edit|keyboard|table|graph)$' --output-on-failure
```

预期： smoke-only、host-only cache 或旧专用事件使至少一项失败。

- [ ] **步骤 4：实现输入数据族直接映射**

复用 M2 common text 提交；keyboard/table/graph 的固定上限必须在 public API 返回 `CAPACITY/OUT_OF_RANGE`，不静默截断。focus/key 同步交给 LD navigation，转换为统一 KEY/VALUE_CHANGED/FOCUSED/DEFOCUSED；不创建通用输入队列。

- [ ] **步骤 5：验证本族 L3/L4/L5-E 单元证据**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_text test_tinyui_line_edit test_tinyui_keyboard test_tinyui_table test_tinyui_graph -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(text|line_edit|keyboard|table|graph)$' --output-on-failure
```

预期： 全部通过，真实 key/edit/value 事件顺序确定。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：对九个 creator 与核心 setter执行 impact**

至少覆盖 `tinyui_image_create`、`tinyui_canvas_create`、`tinyui_animation_create`、`tinyui_date_time_create`、`tinyui_clock_create`、`tinyui_qrcode_create`、`tinyui_message_box_create`、`tinyui_background_create`、`tinyui_window_create`。

预期：screen/window 为 HIGH/CRITICAL 时先报告；不得修改 runtime/shared internal 写面。

- [ ] **步骤 2：写非 smoke 的 L3/L4 失败测试**

image 验证 tile/mask；canvas 每个公开 draw command 验证真实 LD command/state；animation 验证 source/frame/period；date_time/clock 验证显式值与 system-time 开关；QR 验证 text/color/ecc/version/zoom；message box 验证 title/message/buttons/colors/confirm event；window/background 验证真实 root/container/background状态与 props rollback。

- [ ] **步骤 3：运行媒体复合族测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_image test_tinyui_canvas test_tinyui_animation test_tinyui_date_time test_tinyui_clock test_tinyui_qrcode test_tinyui_message_box test_tinyui_background test_tinyui_window -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(image|canvas|animation|date_time|clock|qrcode|message_box|background|window)$' --output-on-failure
```

预期： complex controls 的 smoke-only 或 host command cache 测试不足，新增断言失败。

- [ ] **步骤 4：实现九控件直接映射**

所有公开能力落到现有 `ldImage/ldCanvas/ldAnimation/ldDateTime/ldClock/ldQRCode/ldMessageBox/ldWindow`；canvas 仅使用 LD 已有 command/render能力，不在 TinyUI/SDL 加绘制器。message box 事件进入 fixed callback pool；资源全部借用 任务 7 descriptor。

- [ ] **步骤 5：验证本族 L3/L4**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_image test_tinyui_canvas test_tinyui_animation test_tinyui_date_time test_tinyui_clock test_tinyui_qrcode test_tinyui_message_box test_tinyui_background test_tinyui_window -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(image|canvas|animation|date_time|clock|qrcode|message_box|background|window)$' --output-on-failure
```

预期： 全部通过，complex controls 每项都有状态断言而非仅非空/smoke。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

### 任务 6：实现轻量 theme/style 即时应用

**文件：**
- 修改： `tinyui/src/theme/theme.c`
- 修改： `tests/tinyui/unit/test_tinyui_theme.c`

**接口：**
- 输入： M1 `tinyui_style_t/tinyui_theme_t`，M2 common setter adapter。
- 输出： `tinyui_obj_apply_style`、`tinyui_theme_set/get/apply` 的无分配直接映射。

- [ ] **步骤 1：分析 theme 影响面**

```text
impact({target: "tinyui_theme_apply_to_widget", direction: "upstream"})
impact({target: "tinyui_theme_set", direction: "upstream"})
```

预期：theme apply 风险不高于 MEDIUM；若实际为 HIGH，先报告调用者。

- [ ] **步骤 2：写 descriptor 预检与即时应用测试**

覆盖全部 fields、固定 part/state 支持矩阵、无效 field bit、越界 opacity/metric、font 指针、栈上 style 调用后销毁、theme 借用生命周期、新对象自动应用、替换 theme 不遍历旧树、显式 apply。参数错误和 `NOT_SUPPORTED` 必须在任何字段修改前返回。

- [ ] **步骤 3：运行 theme 测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_theme -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_theme$' --output-on-failure
```

预期： 旧 dynamic theme/style class 或部分提交语义不符合，测试失败。

- [ ] **步骤 4：实现无状态 style 与借用 theme**

style 先完整校验 `fields/part/state`，然后按 bg、text、border、width、radius、padding、opacity、font 固定顺序调用真实 setter；不保存 style 指针。theme runtime 只保存一个借用指针；set 替换指针，apply 只应用目标对象，不遍历 tree、不构造 style object、不分配。

- [ ] **步骤 5：验证 theme/style L3/L4 与零分配**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_theme -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_theme$' --output-on-failure
```

预期： 测试通过，allocator delta 为 `0`，不支持的 state/part 不修改对象。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：分析 layout/resource 影响面**

```text
impact({target: "tinyui_flex_set_flow", direction: "upstream"})
impact({target: "tinyui_grid_set_columns", direction: "upstream"})
impact({target: "tinyui_image_source_from_vres", direction: "upstream"})
impact({target: "tinyui_resolve_ld_font", direction: "upstream"})
```

预期：font 影响多个控件；写出受影响控件清单后再修改。

- [ ] **步骤 2：写 flex/grid 参数转换失败测试**

覆盖 flow、三轴 align、item/track gap、grow、min/max、new track、ignore layout；grid 覆盖 PX `1..32767`、FR `1..255`、CONTENT value `0`、count `1..16`、span/align/gap。验证真实 `ldWindow` layout 配置和 reflow/dirty，不从 TinyUI grid 数组证明。

- [ ] **步骤 3：写 resource ABI、所有权和零分配测试**

RGB565 source 断言 pixels/mask 借用且 private storage 构成有效 tile；builtin 一一映射静态资源；VRES init/deinit 各 acquire/release 一次；绑定期间 caller 保持 descriptor；ARGB8888 请求返回 `NOT_SUPPORTED`。静态断言 private storage `>=sizeof(arm_2d_tile_t)`、32 位 image `<=80 B`、font `<=16 B`。

- [ ] **步骤 4：运行 layout/resource 测试确认失败**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_layout test_tinyui_resource -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(layout|resource)$' --output-on-failure
```

预期： 旧 sentinel grid、native 指针 descriptor 或 resource ownership 测试失败。

- [ ] **步骤 5：实现 direct flex/grid**

flex/grid 只校验、收窄转换并调用 LingDongGUI；不保存第二份 tracks，不计算 child geometry。grid 使用局部固定 `int16_t tracks[16]` 转换后立即传给 LD；CONTENT/FR 使用 LD 已有编码 helper，禁止 public 魔数。

- [ ] **步骤 6：实现 image/font value descriptor**

在 private storage 内 placement 初始化 tile/handle，不 heap 分配。RGB565 stride/mask stride 完整校验；VRES deinit 幂等并清零 kind；builtin font 映射四个设计枚举，VRES 复用 `ldBaseGetVresFont()`，不增加 glyph callback/cache/provider。

- [ ] **步骤 7：验证 layout/resource L3/L4 和零分配**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_layout test_tinyui_resource -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_(layout|resource)$' --output-on-failure
```

预期： 全部通过，layout/apply/getter allocator delta 均为 `0`。

- [ ] **步骤 8：运行格式检查**

```sh
rtk git diff --check
```

### 任务 8：串行集成 common adapter 与错误返回

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

- [ ] **步骤 1：对 shared adapter 与 rename 运行 impact**

```text
impact({target: "tinyui_widget_set_text", direction: "upstream"})
impact({target: "tinyui_widget_set_bg_color", direction: "upstream"})
impact({target: "tinyui_scroll_selector_create", direction: "upstream"})
impact({target: "tinyui_q_r_code_init", direction: "upstream"})
```

预期：common setter 为 HIGH；先核对 任务 1 ledger 中每个控件支持矩阵。

- [ ] **步骤 2：写全控件 adapter 与禁用旧名合约测试**

`check_tinyui_public_api.py` 扫描 `tinyui.h` 和聚合 public headers，断言不出现 `scroll_selecter`、`tabel`、`q_r_code`、`set_press(`、legacy app、`ld*`、`arm_2d_*`、`SIGNAL_*`。unit 合约对每个 kind 的 common 属性断言 supported 或 `NOT_SUPPORTED`，不得返回笼统 `BACKEND` 掩盖能力缺失。

- [ ] **步骤 3：运行合约确认失败**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk cmake --build build/v2.3-m3 --target test_tinyui_core_helpers -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_core_helpers$' --output-on-failure
```

预期： 旧拼写或未接入 common adapter 的 kind 使检查失败。

- [ ] **步骤 4：集成全部 kind 的静态 adapter**

在单一 `widget.c` switch 中接入 任务 2-7 已实现的真实能力；不创建动态 registry。更新 `internal.h` kind 拼写与声明，删除 wrapper 中已无用途的 callback/style/resource镜像字段。

- [ ] **步骤 5：复核 M1 已冻结的 canonical 命名**

确认 scroll selector、QR 和 pressed symbol 与 M1 manifest 完全一致；M3 不再次移动或重命名这些文件。任何旧拼写只能暂存于 M1 定义的私有 `temporary migration bridge`，不得进入 canonical 头、能力矩阵或最小 profile；迁移映射留给 M4 migration guide。

- [ ] **步骤 6：验证公共边界和全控件 unit**

运行：

```sh
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk ctest --test-dir build/v2.3-m3 -L tinyui -L unit --output-on-failure
```

预期： 旧名/legacy/native 泄漏为 0；全部 TinyUI unit 通过。

- [ ] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

### 任务 9：串行复核编译期裁剪和最小 profile

**文件：**
- 修改： `cmake/LingDongGUI.cmake`
- 修改： `tests/tinyui/CMakeLists.txt`
- 修改： `tests/tinyui/perf/check_tinyui_minimal_symbols.py`
- 修改： `tests/tinyui/minimal/minimal_consumer.c`

**接口：**
- 输入： M1 `TINYUI_ENABLE_<WIDGET>`、theme/diagnostics/native 选项和 任务 2-8 全部源文件。
- 输出：M1 已建立的条件 source list 在全控件迁移后仍成立，唯一 CTest 名为 `check_tinyui_minimal_profile`，底层只调用 `check_tinyui_minimal_symbols.py`。

- [ ] **步骤 1：写最小 profile 失败检查**

沿用 M1 的 consumer，只调用 runtime、screen/window、label、button。扩展现有 checker 的禁用符号集合，覆盖 M3 新接入的全部控件、theme、diagnostics、native interop 和 private v2.2 demo bridge；不得新增第二个 minimal checker 或 consumer。

- [ ] **步骤 2：运行最小 profile 捕获 M3 新增静态引用**

运行：

```sh
rtk cmake -S . -B build/v2.3-minimal -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none -DTINYUI_PROFILE=minimal
rtk cmake --build build/v2.3-minimal --target tinyui_minimal_consumer -j
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期：若 波次 1 新增了跨模块静态引用，gate 精确报告对应 disabled symbol；若直接通过，则保存通过结果，不伪造红灯。

- [ ] **步骤 3：按 option 条件加入源文件**

`tinyui_core` 固定只含 runtime/object/event/timer/window/label/button 必需源；每个其他 widget、theme、diagnostics、native interop 使用独立 CMake option 条件加入。关闭模块不生成空 registry/table，不在链接产物保留 public 实现。

- [ ] **步骤 4：注册最小 profile CTest**

保持 M1 注册的 `check_tinyui_minimal_profile`，label 为 `tinyui;contract;size;minimal`。该 CTest 只驱动 `check_tinyui_minimal_symbols.py`；配置失败、map 缺失、nm 不可用、预期符号缺失均 fail-closed。

- [ ] **步骤 5：验证最小与默认 profile**

运行：

```sh
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
rtk cmake -S . -B build/v2.3-m3 -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_SDL_DEMO=ON -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m3 -j
rtk ctest --test-dir build/v2.3-m3 -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期： minimal 只含允许模块；默认全量构建仍通过。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：为五个家族场景写失败检查**

每个场景使用真实 LD screen 和 direct flex/grid 排列；不得用 fixed `set_pos/set_size` 掩盖 layout 缺口。可见能力用确定区域像素差、颜色/边缘/尺寸特征断言；可操作能力用真实 backend signal/key 注入和精确 event trace。

- [ ] **步骤 2：运行无 baseline/trace 检查确认失败**

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

- [ ] **步骤 3：生成真实像素和事件证据**

通过 SDL 测试宿主驱动 LingDongGUI/Arm-2D capture；逐场景人工确认真实控件、状态差异和布局均可见，再保存五个 PPM。事件 trace 必须区分 pressed/released/clicked/value/key/focus，不用截图替代事件。

- [ ] **步骤 4：更新唯一证据矩阵**

每个 required row 填 public API、L2 consumer、L3 test、L4 test 名、L5-V scenario+region/assertion、L5-E trace；不适用的 L5 维度写 `not_applicable` 与用户能力理由。复杂控件不能只绑定 smoke 或 create test。

- [ ] **步骤 5：运行全部 L4/L5 gate**

运行：

```sh
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py --all-v23
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
```

预期： required 全部至少 L4；可见能力全有 L5-V；可操作能力全有 L5-E；policy 无用户能力规避。

- [ ] **步骤 6：运行格式检查**

```sh
rtk git diff --check
```

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

- [ ] **步骤 1：扩展全 wrapper 与 descriptor probe**

逐控件打印 `sizeof` 并相对 M0 检查；增加 image source `<=80 B`、font `<=16 B`、tile private storage 静态断言；保留 timer+event+runtime 32 位静态 RAM `<=1024 B`。任一缺 baseline/schema 立即失败。

- [ ] **步骤 2：运行性能与尺寸检查**

运行：

```sh
rtk cmake --build build/v2.3-m3 --target test_tinyui_wrapper_struct_overhead -j
rtk ctest --test-dir build/v2.3-m3 -R '^test_tinyui_wrapper_struct_overhead$' --output-on-failure
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py
rtk python3 tests/tinyui/perf/check_tinyui_perf.py
```

预期： wrapper、binary 双阈值、steady-state allocation、时间指纹检查通过；环境指纹不同则 fail-closed 并建立当前机器新 baseline。

- [ ] **步骤 3：运行标准 M3 全量 gate**

运行：

```sh
rtk cmake -S . -B build/v2.3-m3 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m3 -j
rtk ctest --test-dir build/v2.3-m3 --output-on-failure
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期： 配置、全量构建、全部 CTest 和 minimal profile 零失败。

- [ ] **步骤 4：检查禁用的重型概念和 native 泄漏**

运行：

```sh
rtk rg -n 'style_class|selector|event_queue|resource_registry|refcount|renderer_plugin' tinyui/src tinyui/include
rtk rg -n 'ld[A-Z]|arm_2d_|SIGNAL_' tinyui/include/tinyui.h tinyui/include/core tinyui/include/widgets tinyui/include/layout tinyui/include/theme
```

预期： 第一条只允许已明确删除计划中的旧测试/注释命中，生产实现无动态 selector/queue/registry；第二条 canonical public headers 零命中。发现命中必须分类修复后重跑。

- [ ] **步骤 5：运行 GitNexus 影响审计**

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：变化覆盖 core adapter、全控件、theme/layout/resource、裁剪、测试与证据；不应出现 SDL/MCU 生产 port、PFB/DMA/flush/tick provider 工作。若出现第二 renderer/layout/style/resource runtime，M3 不得关闭。

- [ ] **步骤 6：记录 M3 closeout**

在 `docs/v2.3/v2.3-performance-baseline.md` 记录完整机器指纹、git commit、全 wrapper/descriptor/RAM/binary/分配数、执行命令与结果。结论只能写“v2.3 M3 内部里程碑完成”，L6 与 port production 明确延期。

- [ ] **步骤 7：运行格式检查**

```sh
rtk git diff --check
```

## M3 完成判定

- [ ] inventory 中全部 required 用户能力有 canonical public API 且至少达到 L4。
- [ ] 全部可见能力有真实 LingDongGUI L5-V；全部可操作能力有真实 L5-E。
- [ ] theme/style 是即时 descriptor 映射，无 selector、级联或全树自动 apply。
- [ ] flex/grid 直接调用 LingDongGUI，没有第二 layout solver 或 TinyUI geometry 真值。
- [ ] image/font 是调用者持有的固定 value descriptor，无 heap、cache、refcount 或 provider。
- [ ] `scroll_selector`、`qrcode`、`set_pressed` 命名唯一，public headers 无 legacy/native/backend 泄漏。
- [ ] 每个 widget/theme/diagnostics/native 模块可裁剪，最小 profile 的 disabled symbols 不进二进制。
- [ ] 全 wrapper、静态 RAM、descriptor、binary size 和 steady-state allocation 未越过硬预算。
- [ ] 全量 CMake/CTest 与 minimal profile 零失败，GitNexus 影响范围符合 M3。
