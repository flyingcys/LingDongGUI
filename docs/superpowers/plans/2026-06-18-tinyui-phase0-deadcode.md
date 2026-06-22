# TinyUI Phase 0:零风险死代码清理 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (- [ ]) syntax.

**Goal:** 删除 `tinyui_backend_widget` 中 8 个 C 类纯测试记账字段、`runtime_evidence_flags` 及其 `observe.c` 死分支、`reserved_*`/`open`/`last_window_switch_*` 占位字段、`list.c` 9 个 `return -1` 死桩、`native.c` 的 `signal_to_ld`/`readback_policy_to_backend` 死抽象,以及它们在 `tests/` 的全部读点/断言,在不改变任何运行逻辑的前提下缩减 backend 结构与测试负担。

**Architecture:** 这是**纯删除/常量折叠**相位,不引入新类型、不改 public API、不动 `tinyui_backend_widget`/`tinyui_backend_app_state` 类型本身(仅删字段)。所有被删字段经普查确认**生产代码 0 读**(仅自增/写或仅死分支读),`runtime_evidence_flags` 字段从不写(恒 0),删除后受影响的 `observe.c` 两处分支等价于常量折叠为"恒不命中",其对外 marker(`TINYUI_SMOKE_LAYOUT_USED=0`、`TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI`)行为不变。以现有 `ctest`(unit/contract/runtime/perf)为安全网,逐 Task 先记录绿、删除、删过时断言、重跑到绿。

**Tech Stack:** C / CMake / CTest / tinyui 测试 harness(`tests/tinyui/{unit,contract,runtime,perf}` + `tests/support/tinyui_test_support.c`)/ gitnexus

---

## 关键事实(已逐条用 Read/grep 核实)

- **被删字段定义位置**:`tinyui/src/core/runtime_internal.h`。`struct tinyui_backend_widget` 字段区 162-201;`struct tinyui_backend_app_state` 203-210;三个相关 enum 100-116。
- **C 类 8 字段的 src 写点 / tests 读点**(已 grep 核实,见各 Task)。
- **`tinyui_widget_init_data_model`(widget.c:216-240)是纯 C 类函数**(只写 `data_truth_policy`/`data_model_identity`),其唯一生产调用在 `runtime_bridge.c:350`,声明在 `internal.h:240`,静态计数器 `g_tinyui_backend_next_data_model_identity` 在 `widget.c:116` → **整体删除**。
- **`tinyui_widget_update_value`(widget.c:274-293)是真实函数**(写 `backend->value` + 调 `tinyui_widget_sync_ld_value`),**只删其中 287/288 两行 C 类写**,函数与 `value` 写、sync 调用保留。声明 `internal.h:195` 保留。
- **`event.c` 各 dispatch 中 `backend->value` 守卫(408/553/582/611)与 `value=` 赋值、`tinyui_widget_sync_ld_value(...)` 调用一律保留**(B 类真实状态/真实副作用);只删同块内的 `data_model_epoch++` / `last_data_source=` / `last_signal=` / `dispatch_count++`。
- **`runtime_evidence_flags` 从不写**(全仓 0 写),`observe.c:118`(`excludes_formal_mapping`)与 `observe.c:226`(`allows_smoke_layout`)的读恒为 0;`tinyui_runtime_host_widget_excludes_formal_mapping` 仅在 observe.c 内部使用(115 定义、261/269 调用),`allows_smoke_layout` 在 `step.c:179` 调用且喂 `TINYUI_SMOKE_LAYOUT_USED` marker(删字段后该 marker 恒输出 `=0`,与现状一致)。`check_tinyui_backend_mapping.py` 中 `_assert_demo_excluded_from_formal_mapping`(286)是**定义但运行循环 322-350 从不调用**的死路径,不阻塞删除。
- **`native.c` 死抽象**:`tinyui_native_signal_to_ld`(102-128,含其上 enum 23-31)与 `tinyui_native_readback_policy_to_backend`(130-147)均**无生产调用**(event.c 直接用 `SIGNAL_PRESS` 等宏);`TINYUI_LD_NAV_*` enum(33-38)孤儿;声明在 `internal.h:145/154`。
- **公共 `native.h:33-47` 的 `TINYUI_NATIVE_SIGNAL_*`/`TINYUI_NATIVE_READBACK_*` enum 是 public API,本相位不删**(属 API 表面,Phase B 范畴;Phase 0 零风险不破坏 public API)。仅删私有翻译函数。
- **测试侧本地 helper `assert_source_lacks_function_definition`(test_tinyui_native_bridge.c:44-57)只 grep `static`-前缀本地定义**;被删函数是非 static,删除后 `test_internal_native_helpers_no_longer_use_tinyui_prefix`(120-134)继续通过,**保留不动**。
- **ctest 已配置**(`build/`),test target = 文件名(如 `test_tinyui_widgets`、`test_tinyui_list`、`test_tinyui_native_bridge`、`test_tinyui_app_lifecycle`、`test_tinyui_scroll_selecter`、`test_tinyui_button_events`、`test_tinyui_keyboard`、`test_tinyui_combo_box`、`test_tinyui_line_edit`、`test_tinyui_table`),python 测试 = `check_tinyui_object_overhead`/`check_tinyui_perf`/`check_tinyui_backend_mapping`/`check_tinyui_runtime`/`check_tinyui_visible_ui`。
- **所有 `rg` 命令在本地用 `grep -rn ... --include` 形式跑**(rtk 代理会吞掉 `rg --type`);读函数体一律用编辑器/Read,不要 `cat`/`sed`。

## 并行/串行说明

- **Task 1(C 类字段删除)= 串行单写者**:写面集中在 `runtime_internal.h` + `widget.c` + `event.c` + `icon_slider.c` + `radial_menu.c` + `runtime_bridge.c` + 8 个 unit 测试,跨文件强耦合(同一组字段/enum),必须一个 subagent 顺序完成。
- **Task 2(`observe.c` evidence_flags 死分支)**:写面 = `port/sdl/observe.c` + `step.c` + `runtime_internal.h` 的 enum/字段行。其中 `runtime_internal.h` 与 Task 1 重叠(同一文件),故 **Task 2 必须在 Task 1 之后串行**(或并入 Task 1 的同一 subagent 顺序步骤)。
- **Task 3(`list.c` 9 死桩 + 测试)= 独立写面**(`widgets/list.c` 仅删 557-640 + `tests/tinyui/unit/test_tinyui_list.c` 的桩声明/断言)。**可与 Task 4 并行**;但注意 `test_tinyui_list.c` 也被 Task 1 改(删 last_signal/last_native/dispatch_count 读点),故 **Task 3 与 Task 1 在 `test_tinyui_list.c` 上写面重叠 → Task 3 必须排在 Task 1 之后,或由同一 subagent 串行**。
- **Task 4(`native.c` 死抽象 + `test_tinyui_native_bridge.c`)= 独立写面**(`native.c` + `internal.h` 的两行声明 + `test_tinyui_native_bridge.c`)。`internal.h` 与 Task 1 不重叠行(Task 1 删 240,Task 4 删 145/154),但同文件;为零冲突起见 **Task 4 与 Task 1 串行**,可与 Task 3 并行。

> 推荐执行顺序:**Task 1 → (Task 2) → Task 3 ∥ Task 4**。Task 3/4 写面互不重叠、且都在 Task 1 之后,可真正并行。若用单 subagent 顺序执行则按 1→2→3→4。

---

## Task 1 — 删除 C 类 8 个记账字段及全部 src 写点 + tests 读点(串行单写者)

**Files:**
- Modify `tinyui/src/core/runtime_internal.h`(删字段 180/182/183/184/191/192/193/194;删 enum `tinyui_backend_data_truth_policy` 100-103、`tinyui_backend_data_value_source` 105-109;`reserved_signal_padding`:181 见 Task 1 末尾说明)
- Modify `tinyui/src/core/widget.c`(删 `g_..._identity` 116;删 `tinyui_widget_init_data_model` 216-240;删 `update_value` 内 287-288 两行)
- Modify `tinyui/src/core/event.c`(删各 dispatch 块内的 epoch/source/last_signal/dispatch_count 写:413/414/416/417、449/450、474/475、529-532 区的 531/532、558/559/561/562、587/588/590/591、616/617/619/620、657/658/659/660)
- Modify `tinyui/src/core/runtime_bridge.c`(删 `tinyui_widget_init_data_model(backend);` 调用 350)
- Modify `tinyui/src/core/internal.h`(删 `tinyui_widget_init_data_model` 声明 240)
- Modify `tinyui/src/widgets/icon_slider.c`(删 106 `data_model_epoch++`、107 `last_data_source=`、108 `last_signal=`、109 `dispatch_count++`)
- Modify `tinyui/src/widgets/radial_menu.c`(删 103 epoch、104 source、105 last_signal、106 dispatch_count)
- Modify `tinyui/src/widgets/checkbox.c`(删 175 last_signal、234 last_signal、235 dispatch_count)
- Modify `tinyui/src/widgets/combo_box.c`(删 105 last_signal、106 dispatch_count、167 last_signal)
- Modify `tinyui/src/widgets/switch.c`(删 246 last_signal、247 dispatch_count)
- Modify `tinyui/src/widgets/gauge.c`(删 359 last_signal)
- Modify `tinyui/src/widgets/arc.c`(删 327 last_signal)
- Modify `tinyui/src/widgets/slider.c`(删 209 last_signal)
- Modify `tinyui/src/widgets/list.c`(删 143 last_signal)
- Modify `tinyui/src/widgets/progress_bar.c`(删 313 last_signal)
- Modify `tinyui/src/widgets/progress_wheel.c`(删 337 last_signal)
- Modify `tinyui/src/widgets/scroll_selecter.c`(删 399 last_signal)
- Modify `tinyui/src/widgets/line_edit.c`(删 83 last_native_signal、84 last_native_value)
- Modify `tinyui/src/widgets/table.c`(删 243 last_native_signal、244 last_native_value)
- Test `tests/tinyui/unit/test_tinyui_widgets.c`(70 处读点)、`test_tinyui_button_events.c`(33 处)、`test_tinyui_scroll_selecter.c`(13 处)、`test_tinyui_list.c`(9 处)、`test_tinyui_line_edit.c`(6 处)、`test_tinyui_table.c`(6 处)、`test_tinyui_keyboard.c`(5 处)、`test_tinyui_combo_box.c`(2 处)

**Steps:**

- [ ] 记录基线绿:`rtk ctest --test-dir build -R 'test_tinyui_widgets|test_tinyui_button_events|test_tinyui_scroll_selecter|test_tinyui_list|test_tinyui_line_edit|test_tinyui_table|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_event|test_tinyui_app_lifecycle' -V` → 预期全部 `Passed`(记下通过条目数,作为删断言后对照)。
- [ ] 改前 impact:对每个将删的命名符号跑 `gitnexus_impact({target, direction:"upstream"})`:`tinyui_widget_init_data_model`、`g_tinyui_backend_next_data_model_identity`、`tinyui_backend_data_truth_policy`、`tinyui_backend_data_value_source`。把各自直接调用方/风险等级记入 PR 描述;若任一返回 HIGH/CRITICAL 先停并报告(预期为 LOW:`init_data_model` 仅 1 生产调用 + 1 声明;两个 enum 仅 widget.c/native.c/tests 引用)。
- [ ] 删 `runtime_internal.h` 的 8 个 C 类字段行:`int16_t last_signal;`(180)、`uint32_t last_native_signal;`(182)、`uint64_t last_native_value;`(183)、`int dispatch_count;`(184)、`unsigned int data_model_identity;`(191)、`unsigned int data_model_epoch;`(192)、`int16_t data_truth_policy;`(193)、`int16_t last_data_source;`(194)。**保留** `int value;`(179)、`edit_result_on_finish`(195)、`list_item_count`(189)等 B 类字段。
- [ ] 删 `runtime_internal.h` 现已无消费者的 enum:`enum tinyui_backend_data_truth_policy { ... }`(100-103)与 `enum tinyui_backend_data_value_source { ... }`(105-109)。**保留** `enum tinyui_backend_signal`(93-98,dispatch 在用)。
- [ ] 删 `widget.c`:静态计数器 `static unsigned int g_tinyui_backend_next_data_model_identity = 1;`(116);整函数 `tinyui_widget_init_data_model`(216-240,含 switch 与自指回绕 guard 230-233)。
- [ ] 删 `internal.h:240` 声明 `void tinyui_widget_init_data_model(struct tinyui_backend_widget *backend);`。
- [ ] 删 `runtime_bridge.c:350` 的孤立调用行 `tinyui_widget_init_data_model(backend);`(确认上下文该行可独立删,不影响同函数其余 bind/attach 逻辑)。
- [ ] 改 `widget.c` 的 `tinyui_widget_update_value`(274-293):仅删 287 `backend->data_model_epoch++;` 和 288 `backend->last_data_source = TINYUI_BACKEND_DATA_SOURCE_SETTER;` 两行;**保留** 286 `backend->value = value;` 与 289 `tinyui_widget_sync_ld_value(...)`。
- [ ] 改 `event.c` 的 `VALUE_CHANGED` 块(407-423):删 413(epoch)、414(source)、416(last_signal)、417(dispatch_count);**保留** 408 守卫、412 value 写、415 sync、418 bridge、419-421 回调。
- [ ] 改 `event.c` 的 `dispatch_event`(428-456):删 449(last_signal)、450(dispatch_count);保留 446 focus、451 emit。
- [ ] 改 `event.c` 的 `dispatch_native_signal`(458-475):删 474(last_native_signal)、475(last_native_value);保留其后 enabled/visible 判断与 switch。
- [ ] 改 `event.c` keyboard 块(520-538):删 529-531(last_signal 三行赋值)、532(dispatch_count);保留 522-528 focus 与 533-535 回调。
- [ ] 改 `event.c` checkbox 块(540-568):删 558(epoch)、559(source)、561(last_signal)、562(dispatch_count);保留 553 守卫、556 checked、557 value、560 sync、563-566 回调。
- [ ] 改 `event.c` switch 块(569-597):删 587、588、590、591;保留 582 守卫、585 checked、586 value、589 sync、592-595 回调。
- [ ] 改 `event.c` slider 块(598-626):删 616、617、619、620;保留 611 守卫、614 value、615 backend value、618 sync、621-624 回调。
- [ ] 改 `event.c` list 块(627-665):删 657(epoch)、658(source)、659(last_signal)、660(dispatch_count);保留 638 边界、648 `tinyui_list_set_selected_index_ld`、651 sync、654 守卫、661-663 回调。
- [ ] 改 `icon_slider.c`(删 106/107/108/109)、`radial_menu.c`(删 103/104/105/106):删该处 epoch/source/last_signal/dispatch_count 四连写,保留同函数的 value 同步与回调。
- [ ] 删各 widget 单点 `last_signal` 写:`checkbox.c:175,234`、`combo_box.c:105,167`、`switch.c:246`、`gauge.c:359`、`arc.c:327`、`slider.c:209`、`list.c:143`、`progress_bar.c:313`、`progress_wheel.c:337`、`scroll_selecter.c:399`;删 `checkbox.c:235`、`combo_box.c:106`、`switch.c:247` 的 `dispatch_count++`;删 `line_edit.c:83/84`、`table.c:243/244` 的 `last_native_signal/value` 写。**保留**这些函数的实际副作用语句。
- [ ] 编译验证:`rtk cmake --build build` → 预期无 `error:`(若出现"未声明 enum/字段"说明仍有遗漏读点,定位后删除)。
- [ ] 删过时单测断言(8 文件)。逐文件 grep 定位后删除断言行:
  - `grep -n 'data_model_identity\|data_model_epoch\|data_truth_policy\|last_data_source\|last_signal\|last_native_signal\|last_native_value\|dispatch_count\|TINYUI_BACKEND_DATA_TRUTH_\|TINYUI_BACKEND_DATA_SOURCE_' tests/tinyui/unit/test_tinyui_widgets.c` → 删命中的 `assert(...)` 行(含 629-631、671-691 的 data_truth/last_data_source 断言);若某测试函数删空仅剩声明,同时删该测试函数及其在 `main()` 的调用。
  - 对 `test_tinyui_button_events.c`、`test_tinyui_scroll_selecter.c`(含 82/118)、`test_tinyui_list.c`、`test_tinyui_line_edit.c`、`test_tinyui_table.c`、`test_tinyui_keyboard.c`、`test_tinyui_combo_box.c` 重复同一 grep+删除流程。
  - **只删断言/取值行,不删被测控件的创建与真实能力断言**(value/checked/selected_index 读回、host 绑定恒等保留)。
- [ ] 处理 `reserved_signal_padding`(181):它是紧跟 `last_signal` 的对齐占位,随 Task 1 一并删(见 Task 2 同批删 `reserved_list_padding`/`reserved_edit_padding`/`open`;若希望集中在 Task 2 处理也可,但 181 与本 Task 删的 180 相邻,**就近在本 Task 删 181**)。删后确认结构无悬空注释。
- [ ] 重编译 + 重跑:`rtk cmake --build build && rtk ctest --test-dir build -R 'test_tinyui_widgets|test_tinyui_button_events|test_tinyui_scroll_selecter|test_tinyui_list|test_tinyui_line_edit|test_tinyui_table|test_tinyui_keyboard|test_tinyui_combo_box|test_tinyui_event|test_tinyui_app_lifecycle' -V` → 预期全部 `Passed`。
- [ ] 残留核验:`grep -rn 'data_model_\|dispatch_count\|last_signal\|last_native_\|last_data_source\|data_truth_policy' tinyui/src tinyui/port` → 预期**无输出**;`grep -rn 'data_model_\|dispatch_count\|last_signal\|last_native_\|last_data_source\|data_truth_policy\|TINYUI_BACKEND_DATA_TRUTH_\|TINYUI_BACKEND_DATA_SOURCE_' tests/tinyui` → 预期**无输出**。
- [ ] `gitnexus_detect_changes()` → 确认受影响符号集与本 Task 预期一致(仅上列文件/符号),无意外波及。
- [ ] commit(消息含 `Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`)。

---

## Task 2 — 删除 `runtime_evidence_flags` + `observe.c` 两处死分支 + 占位字段(串行,接 Task 1)

> 写面含 `runtime_internal.h`(与 Task 1 同文件)→ 必须在 Task 1 之后。

**Files:**
- Modify `tinyui/src/core/runtime_internal.h`(删 `uint16_t runtime_evidence_flags;` 199、`uint16_t open;` 200、`uint16_t reserved_list_padding;` 190、`uint16_t reserved_edit_padding;` 196;删 `enum tinyui_backend_runtime_evidence_flags { ... }` 113-116;删 `tinyui_backend_app_state` 的 `int last_window_switch_mode;` 208 与 `unsigned int last_window_switch_duration_ms;` 209)
- Modify `tinyui/port/sdl/observe.c`(简化 `tinyui_runtime_host_widget_excludes_formal_mapping` 115-127 及其调用 260-274;简化 `tinyui_runtime_host_widget_allows_smoke_layout` 223-235)
- Modify `tinyui/port/sdl/step.c`(`apply_smoke_cursor_layout` 173-186 处理 `allows_smoke_layout` 调用)
- Modify `tinyui/port/sdl/host_internal.h`(若删除 `allows_smoke_layout` 函数则同步删其声明 53)
- Modify `tinyui/src/core/runtime_bridge.c`(删 `tinyui_runtime_bridge_reset_window_switch` 453-463 / `_set_window_switch` 465-477 内对已删字段的写;见步骤)

**Steps:**

- [ ] 记录基线绿:`rtk ctest --test-dir build -R 'check_tinyui_runtime|check_tinyui_backend_mapping|check_tinyui_visible_ui|test_tinyui_app_lifecycle|test_tinyui_app_window_switch' -V` → 预期 `Passed`(runtime/visible 需 `-DLD_BUILD_RUNTIME_TESTS=ON`;若该构建未开则记录 unit 部分并在最终全量门补跑)。
- [ ] 改前 impact:`gitnexus_impact` on `tinyui_runtime_host_widget_excludes_formal_mapping`、`tinyui_runtime_host_widget_allows_smoke_layout`、`tinyui_runtime_bridge_set_window_switch`、`tinyui_runtime_bridge_reset_window_switch`。预期 LOW(局部 + 仅 step.c/observe.c 调用)。
- [ ] `observe.c` `tinyui_runtime_host_widget_excludes_formal_mapping`(115-127):因 `runtime_evidence_flags` 恒 0,函数恒返回 0 → **删除整函数**,并改其两处调用:
  - 260-267 块:把条件 `real_used > 0 && !tinyui_runtime_host_widget_excludes_formal_mapping(root->first_child) && !state->static_mapping_logged` 简化为 `real_used > 0 && !state->static_mapping_logged`(去掉恒真的 `!excludes` 子句),`STATIC_MAPPING=REAL_LDGUI` 行为不变。
  - 269-274 块:`if (excludes... && !temporary_smoke_logged) { 打印 TEMPORARY_SMOKE_PATH }` 整块**删除**(恒不命中的死分支)。
- [ ] `observe.c` `tinyui_runtime_host_widget_allows_smoke_layout`(223-235):因字段恒 0,函数恒返回 0。为保持 `step.c` 调用点与 `TINYUI_SMOKE_LAYOUT_USED` marker 行为,选择**删除整函数**并在 `step.c:178-179` 的守卫里去掉 `!tinyui_runtime_host_widget_allows_smoke_layout(root->first_child)` 子句的等价处理:把 `apply_smoke_cursor_layout`(173-186)简化为恒早返回(因 smoke 永不开启)——即删除函数体中 `state->smoke_layout_used = 1; tinyui_runtime_host_apply_real_widget_layout(...);`(184-185),保留早返回 `return;`。这样 `smoke_layout_used` 恒 0,`tinyui_runtime_host_log_smoke_layout_marker`(384-392)仍打印 `TINYUI_SMOKE_LAYOUT_USED=0`,与现状一致。
  - 同步删 `host_internal.h:53` 的 `tinyui_runtime_host_widget_allows_smoke_layout` 声明。
- [ ] 删 `runtime_internal.h`:`uint16_t runtime_evidence_flags;`(199)、`uint16_t open;`(200);并删 `enum tinyui_backend_runtime_evidence_flags { EXCLUDE_FORMAL_MAPPING ... ALLOW_SMOKE_LAYOUT ... }`(113-116)。
- [ ] 删 `runtime_internal.h` 占位:`uint16_t reserved_list_padding;`(190)、`uint16_t reserved_edit_padding;`(196)(`reserved_signal_padding` 已在 Task 1 删)。
- [ ] 删 `runtime_internal.h` `tinyui_backend_app_state` 的 `int last_window_switch_mode;`(208)与 `unsigned int last_window_switch_duration_ms;`(209)。**保留** `theme`/`ld_scene`/`next_ld_name_id`/`runtime_state`(Phase A 处理)。
- [ ] 改 `runtime_bridge.c`:`tinyui_runtime_bridge_reset_window_switch`(453-463)删 461/462 两行写;`tinyui_runtime_bridge_set_window_switch`(465-477)删 475/476 两行写。删后两函数体只剩空指针检查 → 函数变为 no-op。先 `gitnexus_impact` 确认这两个函数的调用方是否需要保留为 no-op:
  - `grep -rn 'tinyui_runtime_bridge_set_window_switch\|tinyui_runtime_bridge_reset_window_switch' tinyui tests` → 若仅声明 + 这两处定义 + 调用点,**保留空函数体**(不改签名,避免触动调用点与 transition-guard),仅删字段写。若有测试断言它写入字段则一并删该断言。
- [ ] 编译:`rtk cmake --build build` → 预期无 `error:`。
- [ ] 残留核验:`grep -rn 'runtime_evidence_flags\|EXCLUDE_FORMAL_MAPPING\|ALLOW_SMOKE_LAYOUT\|last_window_switch_\|reserved_.*_padding\|\bopen\b' tinyui/src/core/runtime_internal.h` → 预期**无输出**;`grep -rn 'runtime_evidence_flags\|allows_smoke_layout\|excludes_formal_mapping' tinyui/port tinyui/src` → 预期**无输出**。
- [ ] 重跑 runtime/visible/lifecycle:同基线命令 → 预期 `Passed`;特别确认 `test_tinyui_app_lifecycle` 中 `TINYUI_SMOKE_LAYOUT_USED=0` 与 `TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI` 两条断言(220-221)仍通过。
- [ ] `gitnexus_detect_changes()` → 确认范围。
- [ ] commit。

---

## Task 3 — 删除 `list.c` 9 个 `*_ld` 死桩 + 对应单测(独立写面,接 Task 1 后可与 Task 4 并行)

> `test_tinyui_list.c` 的 C 类读点已在 Task 1 删除;本 Task 只动 9 个死桩相关行 → 排在 Task 1 之后即无写面冲突。

**Files:**
- Modify `tinyui/src/widgets/list.c`(删 557-640 共 9 个 `return -1` 死桩)
- Test `tests/tinyui/unit/test_tinyui_list.c`(删前向声明 18-37 中对应 9 行;删断言 803-814)

**Steps:**

- [ ] 记录基线绿:`rtk ctest --test-dir build -R '^test_tinyui_list$' -V` → 预期 `Passed`。
- [ ] 改前 impact:对 9 个桩名各跑一次 `gitnexus_impact`(或一次性确认):`tinyui_list_set_items_ld`、`tinyui_list_set_item_height_ld`、`tinyui_list_set_padding_group_ld`、`tinyui_list_set_margin_group_ld`、`tinyui_list_set_text_color_ld`、`tinyui_list_set_bg_color_ld`、`tinyui_list_set_select_color_ld`、`tinyui_list_set_align_ld`、`tinyui_list_set_item_widget_ld`。预期 LOW(各仅 1 定义 + 1 测试声明 + 1 测试断言,无 src 调用、无头声明)。
- [ ] 删 `list.c` 557-640 这 9 个函数定义(从 `tinyui_list_set_items_ld`(557)起到 `tinyui_list_set_item_widget_ld` 结尾 `return -1; }`(640)止)。**严禁误删** 642 起的 `tinyui_list_set_selected_index_ld`(有真实逻辑,event.c:648 在用)。
- [ ] 删 `test_tinyui_list.c` 前向声明 18-37 中这 9 个桩的声明行(保留非桩声明)。
- [ ] 删 `test_tinyui_list.c` 断言 803-814:
  - 803-806 `tinyui_list_set_items_ld(...) == -1`(多行)、807 `tinyui_list_set_item_height_ld(...) == -1`、808 `..._padding_group_ld`、809 `..._margin_group_ld`、810 `..._text_color_ld`、811 `..._bg_color_ld`、812 `..._select_color_ld`、813 `..._align_ld`、814 `..._item_widget_ld`。若删后所在测试函数空了,连同其 `main()` 调用一并删。
- [ ] 编译 + 重跑:`rtk cmake --build build && rtk ctest --test-dir build -R '^test_tinyui_list$' -V` → 预期 `Passed`。
- [ ] 残留核验:`grep -rn 'tinyui_list_set_items_ld\|tinyui_list_set_item_height_ld\|tinyui_list_set_padding_group_ld\|tinyui_list_set_margin_group_ld\|tinyui_list_set_text_color_ld\|tinyui_list_set_bg_color_ld\|tinyui_list_set_select_color_ld\|tinyui_list_set_align_ld\|tinyui_list_set_item_widget_ld' tinyui tests` → 预期**无输出**;`grep -n 'return -1' tinyui/src/widgets/list.c` → 预期仅余 `set_selected_index_ld` 等真实函数的合法 `return -1`(不是 9 死桩)。
- [ ] `gitnexus_detect_changes()` → 确认范围。
- [ ] commit。

---

## Task 4 — 删除 `native.c` `signal_to_ld`/`readback_policy_to_backend` 死抽象 + 对应单测(独立写面,接 Task 1 后可与 Task 3 并行)

**Files:**
- Modify `tinyui/src/core/native.c`(删 `tinyui_native_signal_to_ld` 102-128 含其上 `enum { TINYUI_LD_SIGNAL_* }` 23-31;删孤儿 `enum { TINYUI_LD_NAV_* }` 33-38;删 `tinyui_native_readback_policy_to_backend` 130-147)
- Modify `tinyui/src/core/internal.h`(删声明 145 及其注释块 138-145;删声明 154 及注释块 147-154)
- Test `tests/tinyui/unit/test_tinyui_native_bridge.c`(删测试函数 `test_native_signal_maps_all_ld_signal_values` 99-108、`test_native_readback_policy_maps_backend_truth_modes` 110-118;删 `main()` 调用 236、237)

**Steps:**

- [ ] 记录基线绿:`rtk ctest --test-dir build -R '^test_tinyui_native_bridge$' -V` → 预期 `Passed`。
- [ ] 改前 impact:`gitnexus_impact({target:"tinyui_native_signal_to_ld", direction:"upstream"})` 与 `gitnexus_impact({target:"tinyui_native_readback_policy_to_backend", direction:"upstream"})`。预期 LOW(无生产调用方;仅 test 引用)。若返回非预期调用方则停并报告。
- [ ] 删 `native.c`:`tinyui_native_signal_to_ld` 整函数(含其上注释块 102-108 与函数体 109-128)+ 其上 `enum { TINYUI_LD_SIGNAL_NO_OPERATION ... VALUE_CHANGED = 14 };`(23-31)。
- [ ] 删 `native.c` 孤儿 `enum { TINYUI_LD_NAV_UP ... RIGHT = 3 };`(33-38,全仓无消费者)。
- [ ] 删 `native.c`:`tinyui_native_readback_policy_to_backend` 整函数(注释块 130-136 + 函数体 137-147)。删后 `native.c` 应只剩 `tinyui_native_image_wrap`、`tinyui_native_font_wrap`、`tinyui_native_align_to_ld_grid` 三个真实函数。
- [ ] 删 `internal.h` 两处声明:145 `int tinyui_native_signal_to_ld(...)`(连同注释 138-144)、154 `int tinyui_native_readback_policy_to_backend(...)`(连同注释 147-153)。**保留** `tinyui_native_nav_dir_to_ld`(136 区)与 `tinyui_widget_update_value`(195)声明。
- [ ] 删 `test_tinyui_native_bridge.c`:测试函数 `test_native_signal_maps_all_ld_signal_values`(99-108)、`test_native_readback_policy_maps_backend_truth_modes`(110-118),并删 `main()` 中调用 `test_native_signal_maps_all_ld_signal_values();`(236)、`test_native_readback_policy_maps_backend_truth_modes();`(237)。
- [ ] **保留**(不动):`test_internal_native_helpers_no_longer_use_tinyui_prefix`(120-134)及其 `main()` 调用(238)——其本地 `assert_source_lacks_function_definition`(44-57)只 grep `static`-前缀本地定义,删除非 static 函数后该测试继续通过且语义更强(确认这两个 helper 不再以任何形式存在于 native.c)。无需改公共 `native.h`(`TINYUI_NATIVE_SIGNAL_*`/`TINYUI_NATIVE_READBACK_*` 是 public enum,本相位保留)。
- [ ] 编译 + 重跑:`rtk cmake --build build && rtk ctest --test-dir build -R '^test_tinyui_native_bridge$' -V` → 预期 `Passed`。
- [ ] 残留核验:`grep -rn 'tinyui_native_signal_to_ld\|tinyui_native_readback_policy_to_backend\|TINYUI_LD_SIGNAL_\|TINYUI_LD_NAV_' tinyui tests` → 预期**无输出**(public `TINYUI_NATIVE_*` 不在此列,正常保留)。
- [ ] `gitnexus_detect_changes()` → 确认范围。
- [ ] commit。

---

## 相位验收 checklist(对齐 spec §4 Phase 0 验收 + §5 质量门)

- [ ] **残留为零**(spec §4 Phase 0 验收):
  - `grep -rn 'data_model_\|dispatch_count\|last_signal\|last_native_\|last_data_source\|runtime_evidence_flags' tinyui/src` → 无输出。
  - `grep -rn 'data_truth_policy\|TINYUI_BACKEND_DATA_TRUTH_\|TINYUI_BACKEND_DATA_SOURCE_\|EXCLUDE_FORMAL_MAPPING\|ALLOW_SMOKE_LAYOUT\|last_window_switch_\|reserved_.*_padding' tinyui/src tinyui/port` → 无输出。
  - `grep -n 'return -1' tinyui/src/widgets/list.c` → 无 9 死桩(仅余真实函数合法返回)。
  - `grep -rn 'tinyui_native_signal_to_ld\|tinyui_native_readback_policy_to_backend' tinyui tests` → 无输出。
- [ ] **全量 ctest 绿**(spec §5):配置 `rtk cmake -S . -B build -DLD_BUILD_RUNTIME_TESTS=ON`(开 runtime/visible 门),`rtk cmake --build build`,然后:
  - `rtk ctest --test-dir build -L tinyui -V` → 预期全部 `Passed`,`0 tests failed`。
  - 单独确认门:`rtk ctest --test-dir build -R 'check_tinyui_object_overhead|check_tinyui_perf|check_tinyui_binary_size' -V`(perf gate)→ 预期 `Passed`(本相位删字段会缩小 `backend_widget` 结构;若 `check_tinyui_object_overhead`/perf baseline 锁定旧 struct 字节数而失败,属预期内的 baseline 漂移——记录实测新值并在该 Task 的 commit 中同步更新对应 baseline,而非跳过;若无失败则无需改 baseline)。
  - `rtk ctest --test-dir build -R 'check_tinyui_transition_guards|check_tinyui_v21_transition_guards|check_tinyui_public_api|check_tinyui_widget_contract_matrix' -V`(contract gate)→ 预期 `Passed`(Phase 0 不改 public API/`app.h` 导出,transition-guard 不应 drift;若 drift 则说明误删了 public 符号,回退排查)。
- [ ] **SDL demo 行为一致**(spec §5「真实 LingDongGUI 输出证据」):`rtk ctest --test-dir build -R 'check_tinyui_runtime|check_tinyui_backend_mapping|check_tinyui_visible_ui' -V` → 预期 `Passed`;确认 `TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI`、`TINYUI_SMOKE_LAYOUT_USED=0`、各 demo `TINYUI_RUNTIME_READY` marker 与改前一致(这三项 runtime 门即"逐页行为一致"的自动化证据)。
- [ ] **gitnexus 终检**:对全部 4 个 Task 的改动跑 `gitnexus_detect_changes()`,确认受影响符号/执行流仅落在本相位预期集合内,无意外破坏 native 事件流(`dispatch_native_signal` → 回调)或 list 选中流。
- [ ] **行数核对**(spec §8 度量参考,非门):记录改后 `wc -l tinyui/src/core/native.c tinyui/src/core/runtime_internal.h tinyui/src/widgets/list.c` 与改前对照(预期 native.c 减约 50 行、runtime_internal.h 减约 25 行、list.c 减 84 行、event.c 减约 25 行、widget.c 减约 30 行,合计 src 侧约削 200+ 行;tests 侧删约 200+ 处断言)。
