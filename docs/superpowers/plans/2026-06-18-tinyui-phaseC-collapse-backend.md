# TinyUI Phase C：折叠中间层 + 样板收敛 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (- [ ]).

**Goal:** 删除 `struct tinyui_backend_widget` 中间镜像层，把真实 ld 绑定状态并入 `struct tinyui_widget`，29 个 widget 各自从「双 calloc + 双层 setter + 逐字复制的颜色/align/detach/dispose」收敛到「单对象 + 复用 core helper」，在**不丢失任何用户控件能力、SDL demo 真实输出逐页与改前一致**的前提下，净减约 4,000–5,000 行、每 widget 省 1 次 calloc + 一个 496–512B 结构。

**Architecture:** C1 串行 core prep → C2 并行 per-widget 扇出（写面=单 widget 的 `.c`/`.h`/单测）→ C3 串行 cleanup。

**Tech Stack:** C / CMake / CTest / LingDongGUI（`ld*`）/ ARM-2D / gitnexus。

---

## 起始状态核实（执行前 MUST 先读 —— 与 spec 假设有偏差）

权威 spec（`docs/superpowers/specs/tinyui-src-simplification.md` §4）规定 **Phase 0 → Phase A → Phase C** 顺序依赖，Phase C 的起点假设 Phase 0 + Phase A 已合入。**但当前 `dev-nanoui` 工作树经核实尚未合入 Phase 0 / Phase A**（核实证据如下），因此本 plan 设一个**硬前置门**，C1 第一个 Task 即校验前置；若未满足，必须先完成 Phase 0 + Phase A 对应 plan 再进入 C。

核实证据（当前树实测）：
- C 类记账字段仍在：`tinyui/src/core/runtime_internal.h:180-199`（`last_signal`/`dispatch_count`/`data_model_*`/`last_data_source`/`last_native_*`/`runtime_evidence_flags`）；写点仍在 `event.c:413-660`、`widget.c:116/230-238/287-288`。→ Phase 0 **未做**。
- `struct tinyui_backend_app_state` 仍存在（`runtime_internal.h:203-210`，含 `theme`/`last_window_switch_mode`/`_duration_ms`）；`tinyui_app.backend_app` 仍在（`internal.h:330`）；全 `tinyui/src` 有 **81 处** `backend_app` 引用（`grep -rn 'backend_app' tinyui/src | wc -l`），3 个访问器 + 3 处裸访问（`window.c:356/732`、`table.c:635`）均未折叠。→ Phase A **未做**。

> **rtk 陷阱（全程纪律）**：本环境 `rg`/`grep` 被 rtk 代理包裹，**会偶发吞掉匹配（真实有匹配却返回 0）**。凡 grep 关键计数（pInfo / backend_app / dispose_partial 等），**必须加 `dangerouslyDisableSandbox=true`**，或改用 Read 直接核对；任何「0 matches」都要交叉复核。本 plan 内所有计数均已用 `dangerouslyDisableSandbox=true` 复核过，与 spec 估值的偏差已在对应 Task 标注。

---

## 关键事实台账（plan 全程引用，已逐处核实）

**折叠后 `struct tinyui_widget` 新增字段（spec §3.1，名称锁定）**：`void *ld_widget; uint16_t ld_name_id; enum tinyui_backend_widget_kind kind; struct tinyui_app *owner; struct ld_scene_t *ld_event_bridge_scene; void *ld_event_bridge_sender; struct tinyui_widget *ld_event_bridge_next; int value; uint16_t list_item_count; enum tinyui_edit_result edit_result_on_finish;`；**删除** `void *backend_widget;`。既有几何/样式/flex/grid/focus/edit 影子字段（`internal.h:288-326`）保留不动。

**新增 core helper（spec §3.3，名称锁定，签名 `[执行时定稿]`）**：`tinyui_widget_create_leaf` / `tinyui_widget_destroy_common` / `tinyui_rgb_to_ld_color` / `tinyui_ld_color_to_rgb` / `tinyui_widget_detach_from_parent` / `tinyui_align_to_arm2d` / core item-list 模型。

**pInfo（高风险 #1）—— 实测 7 个 widget 读 pInfo，不是 spec 说的 6**：
- core 写：`runtime_bridge.c:51`（`sender->pInfo = backend`）、清：`runtime_bridge.c:390`（`unbind_host`）。读：`runtime_bridge.c:29`（事件 slot）。
- widget 读（全部 cast 成 `struct tinyui_backend_widget *`）：`combo_box.c:67`、`icon_slider.c:77`、`line_edit.c:77`、`radial_menu.c:78`、`table.c:232`、`message_box.c:62`（用 `ld_message_box` 而非 `msg.ptSender`）、**`keyboard.c:212`（spec 漏列，用 `ld_keyboard`）**。
- 另有 5 处 dispose 快照 guard 读 `pInfo`（test 用）：`table.c:139`、`arc.c`/`gauge.c`/`progress_bar.c`/`progress_wheel.c` 的 dispose snapshot。

**value 去重 guard（高风险 #2）—— `event.c` 的 `backend->value`**：`dispatch_signal:408`、`checkbox:553`、`switch:582`、`slider:611`、`list:642/654`。宿主已有等价镜像：`checkbox->checked` / `sw->checked` / `slider->value` / `list->selected_index`。spec §3.1 已为折叠后 widget 保留 `int value;` 字段 → **最低风险变换 = `backend->value` 改 `w->value`**，guard 形态不变。

**wrapper 结构（高风险 #5）—— 3 个 widget 内联 wrapper**：
- `button.c`：`struct tinyui_button_backend_host { struct tinyui_backend_widget widget; xBtnInfo_t action_info; }`；create 在 `_xBtnInit(name_id, ..., &host->action_info)`（`button.c:180`），dispose 在 `xBtnRemove(&host->action_info)` 后 `free(host)`（`button.c:94`）。
- `window.c`：`struct tinyui_window_backend_host { struct tinyui_backend_widget widget; ldPadding_t padding_group; int has_padding_group; }`（`window.c:169-173`）。
- `background.c`：`struct tinyui_background_backend_host { struct tinyui_backend_widget widget; }`（`background.c:27-29`，**无额外字段，纯包一层**，仍需去 wrapper 归一存储）。

**双 free / destroy（高风险 #4）**：`tinyui_widget_destroy`（`widget.c:1466-1500`）**本身不 free**（只 detach+unbind+置 `backend_widget=0`）；真正 `free` 仅在各 widget `*_dispose_partial`（create 失败回滚链）。

**逐字复制样板（核实计数，部分与 spec 偏差）**：
- `*_dispose_partial`：**16 文件**（与 spec 一致）：checkbox/button/label/slider/text/table/line_edit/window/image/calendar/combo_box/arc/progress_bar/progress_wheel/qrcode/gauge。命名有 `_impl` 变体（image/arc/progress_bar/progress_wheel/qrcode/gauge）。
- `finish_detach_after_backend_failure`：**8 处**（spec 说 7）：progress_bar/progress_wheel(主+test 变体)/qrcode/arc/table/image/gauge；体完全同构（摘父链表节点）。
- 私有 `get_ld`：**27 widget** 有（`background`/`canvas`/`image` 无）；两变体：A 入参 `struct tinyui_xxx*`、B 入参 `void *backend_widget`（line_edit/radial_menu/icon_slider/scroll_selecter/calendar/combo_box）。
- 颜色换算：**14 文件** 各写 `*_rgb_to_ld_color`（实现完全一致：`__RGB((rgb>>16)&0xFF, (rgb>>8)&0xFF, rgb&0xFF)`）；其中 4 文件（arc/label/window/gauge）另有 `*_ld_color_to_rgb`（实现一致）。
- align 映射：**7 文件**写通用 `tinyui_align→arm_2d_align_t`（canvas/line_edit/label/date_time/list/table + label 反向）；**window.c 另有 4 个 flex/grid 专属映射**（`ldFlexMainAlign_t`/`ldFlexCrossAlign_t`/`ldFlexTrackAlign_t`/`ldGridAlign_t`）→ 这 4 个是 window 专属、**不并入** `tinyui_align_to_arm2d`，保留在 window。

**native-event slot 绑定的 widget（C2 特殊点）**：`ldMsgConnect`/`pInfo=` 出现于 progress_wheel/progress_bar/arc/radial_menu/line_edit/icon_slider/table/combo_box/gauge + message_box(`confirm_bridge`) + keyboard(读 pInfo)。

**带 inline 数组 / native slot 的 widget（C2 特殊点）**：list/combo_box/scroll_selecter（`items[16]` + `backend_item_ids[16]` + `backend_item_texts[16]`）、icon_slider/radial_menu（`items[16]` + `item_sources[16]`）、keyboard（`native_layout` + 动态 `layout_entries`）。

**C3 backend 类型消费者（核实计数，均高于 spec 估值）**：
- `port/sdl/observe.c`：**~42 处**（spec 估 23）—— 6 处函数签名 `const struct tinyui_backend_widget *`、`->kind` 11、`->ld_widget` 6、`->id` 3、`first_child` 8、`next_sibling` 5、`runtime_evidence_flags` 2。
- `port/sdl/step.c`：**~27 处**（spec 估 14）—— 签名/变量 9、`->kind` 4、`->ld_widget` 3、`first_child` 7、`next_sibling` 1、bridge 调用 5、cast 2。
- `port/sdl/host_internal.h`：**4 处**函数原型声明 `const struct tinyui_backend_widget *`（`:52-56`，observe/step 的 helper 原型，须同步改签名）。
- `port/sdl/hal.c`：**0 处**（clean，不动）。
- `src/theme/theme.c`：**~59 处**（最重耦合）—— `tinyui_theme_apply_widget_style(void *backend_widget)` 入口（`:392`），cast `backend_widget->ld_widget` 成各 `ld*_t`（window/label/text/button/checkbox/switch/slider/list/image/calendar），`switch(kind)` 校验，读 `backend_widget->theme->colors[]`。
- `src/layout/flex.c` / `grid.c`：**各 1 处**（纯 facade，经 `window->widget.backend_widget` 取 backend 做 validity 检查 → 折叠后改 `&window->widget` / `widget` 直读）。

**perf 基线 / 探针处置**：
- `tests/tinyui/perf/tinyui_perf_baseline.json`：`backend_widget_struct_bytes`（baseline 496 / max 512，`:78-80`）→ **删除该 metric**（结构消失）；`widget_wrapper_struct_bytes`（184/192，`:70-72`）→ 折叠后 `sizeof(struct tinyui_widget)` 增大（只增 B 类 ~7 个真实字段，A/C 类删除）→ **重测后写新 baseline**（实测值为准，不得拍脑袋）；`switch_wrapper_struct_delta_bytes`（32/32）通常不变，重测确认。
- 探针 `tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c:11`：`sizeof(struct tinyui_backend_widget)` 行 → **删除**；保留 `sizeof(struct tinyui_widget)` 与 switch delta 两行。
- `tests/tinyui/perf/check_tinyui_object_overhead.py`：`:49` 的 key 列表删 `backend_widget_struct_bytes`；`:103-104/152-153/175-179` 探针解析与 gate 断言同步删；`_run_self_test()`（`:120-133`）的 128/192 是**自测桩、非真实 baseline**，可一并清理但不影响 gate。

**构建 / 回归命令（已核实，全程沿用）**：
- 配置：`rtk cmake -S . -B build/tinyui-runtime -DUSE_DEMO=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`（runtime gate 还需 `-DLD_BUILD_RUNTIME_TESTS=ON`，默认 ON）。
- 编译：`rtk cmake --build build/tinyui-runtime -j`。
- 单测 + contract + perf：`ctest --test-dir build/tinyui-runtime -L tinyui --output-on-failure`。
- 单 widget 单测：`ctest --test-dir build/tinyui-runtime -R '^test_tinyui_<widget>$' --output-on-failure`。
- runtime/截图 gate：`ctest --test-dir build/tinyui-runtime -L 'tinyui;runtime' --output-on-failure`（内部走 `tinyui_demo` + `SDL_VIDEODRIVER=dummy TINYUI_DEMO_AUTO_QUIT_MS=1200 TINYUI_CAPTURE_FILE=<tmp>/frame.ppm`，demo 含 `hello_world`/`basic_widgets`/`layout_flex` 等）。
- MAIN_LIB = `tinyui_backend_ldgui`；单测 41 个 + 探针 1 + contract 9 + perf 3 + runtime 3。

---

# C1 — shared core prep（串行，高风险，单写者）

> **执行模式：串行单写者**（spec §7）。写面集中在 `core/*` + `event.c` + 7 个用 pInfo 的 widget 的 pInfo slot；**不可并行**。review 不通过在**同一 subagent 内**修复，不另开。
> **纪律**：每改一个公共符号前 `gitnexus_impact({target, direction:"upstream"})`，HIGH/CRITICAL 须报告主线程；本节每步以现有 ctest + SDL 截图为安全网（行为保持）；新增 helper **先写其单测（红）→ 实现 → 绿**。
> **C1 是整轮最高风险**：它同时改写「树 getter 的真相源」「事件 pInfo 反查」「value guard」「存储模型」四件互相耦合的东西。C2/C3 完全依赖 C1 完成后的稳定 core 接口。

## C1-T0：前置门 + impact 基线 + 安全网快照

**Files:** `tinyui/src/core/runtime_internal.h`、`tinyui/src/core/internal.h`、`tinyui/src/core/runtime_bridge.c`（只读核实）

- [ ] 校验 Phase 0 + Phase A 已合入：`grep -rn 'data_model_\|dispatch_count\|last_signal\|runtime_evidence_flags' tinyui/src`（加 `dangerouslyDisableSandbox=true`）→ **预期 0 行**；`grep -rn 'backend_app\|tinyui_backend_app_state' tinyui/src` → **预期 0 行**。若非 0，**停止 C1**，先执行 Phase 0 / Phase A plan。
- [ ] 建安全网基线：`rtk cmake -S . -B build/tinyui-runtime -DUSE_DEMO=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && rtk cmake --build build/tinyui-runtime -j` → 预期成功；`ctest --test-dir build/tinyui-runtime -L tinyui --output-on-failure` → **预期全绿**（记录通过数作基线）。
- [ ] 截图基线：跑 runtime gate `ctest --test-dir build/tinyui-runtime -L 'tinyui;runtime' --output-on-failure` → 预期绿；归档 `hello_world`/`basic_widgets`/`layout_flex` 三张 `frame.ppm`（C1/C2/C3 末尾逐页比对的「改前」基准）。
- [ ] `gitnexus_impact` 预扫 6 个 C1 核心符号：`tinyui_widget_get_parent`、`tinyui_widget_destroy`、`tinyui_runtime_bridge_ld_event_bridge_slot`、`tinyui_widget_dispatch_signal`、`tinyui_widget_attach_child`、`tinyui_runtime_bridge_bind_host`；把 blast radius（直接 caller、受影响 process、风险级）记入本 Task 笔记，HIGH/CRITICAL 报告主线程。

## C1-T1：folded `struct tinyui_widget` 字段并入 + enum/padding 搬迁（§3.1/§3.4）

**Files:** `tinyui/src/core/internal.h:274-327`（enum + struct）、`tinyui/src/core/runtime_internal.h:60-91`（enum 来源）、`tinyui/src/core/internal.h:347-366`（`tinyui_window` 补 padding）

- [ ] 把 `enum tinyui_backend_widget_kind`（`runtime_internal.h:62-91`，28 项）、`enum tinyui_focus_event`、`enum tinyui_edit_result`（已在 `internal.h:274-284`）整理到 `internal.h`，确认消除 `internal.h ↔ runtime_internal.h` 循环依赖（`internal.h:22` 现 `#include "runtime_internal.h"`）。`[执行时定稿]` 具体搬迁顺序。
- [ ] 在 `struct tinyui_widget`（`internal.h:286`）新增 spec §3.1 锁定的 10 个字段，**删除** `void *backend_widget;`（`internal.h:287`）。字段名严格照抄关键事实台账，不得改名。
- [ ] `struct tinyui_window`（`internal.h:347`）新增承接 `window_layout` 的 padding 字段：`padding/padding_left/padding_top/padding_right/padding_bottom/has_explicit_flex_padding/grid_padding_left/top/right/bottom/has_explicit_grid_padding`（对照 `runtime_internal.h:120-146` 的 `tinyui_backend_layout_window_state`，搬 padding 子集；grid_cols/rows/counts/gap/align 已在 `tinyui_window` 既有字段，勿重复）。
- [ ] 本步**只加字段、不改逻辑**，故源码暂不可编译通过（getter/setter 仍引用 `backend_widget`）—— 这是预期；编译验证留到 C1-T6 收口。先 `gitnexus_impact({target:"tinyui_widget"})` 确认结构改动 blast radius。

## C1-T2：core helper —— 颜色 / align（先写单测，红→绿）

**Files:** 新增 `tinyui/src/core/widget.c` 内（或 `internal.h` 声明）`tinyui_rgb_to_ld_color`/`tinyui_ld_color_to_rgb`/`tinyui_align_to_arm2d`；新增单测 `tests/tinyui/unit/test_tinyui_core_helpers.c`（并在 `tests/tinyui/CMakeLists.txt:1-44` 注册）

- [ ] **先写单测**：`test_tinyui_core_helpers.c` 断言 `tinyui_rgb_to_ld_color(0x112233)` == `__RGB(0x11,0x22,0x33)`、round-trip `ld_color_to_rgb(rgb_to_ld_color(x))` 在 565 精度内一致、`tinyui_align_to_arm2d(TINYUI_ALIGN_START/CENTER/END)` == `ARM_2D_ALIGN_LEFT/CENTRE/RIGHT`。注册后 `ctest -R test_tinyui_core_helpers` → **预期红（链接失败/未定义）**。
- [ ] 实现三 helper（签名 `[执行时定稿]`，名称锁定）：`tinyui_rgb_to_ld_color(unsigned int rgb888)` 体 = `__RGB((rgb>>16)&0xFF,(rgb>>8)&0xFF,rgb&0xFF)`（照搬 14 文件的统一实现）；`tinyui_ld_color_to_rgb(...)` 照搬 arc/label/window/gauge 的 565→888 还原；`tinyui_align_to_arm2d(enum tinyui_align)` 照搬通用 7 文件实现。→ `ctest -R test_tinyui_core_helpers` **预期绿**。
- [ ] 显式记录：window 的 4 个 flex/grid 专属映射（`ldFlexMainAlign_t` 等）**不并入**本 helper，保留在 `window.c`（它们的目标 enum 不是 `arm_2d_align_t`）。

## C1-T3：core helper —— detach / destroy_common（先写单测，红→绿）

**Files:** `tinyui/src/core/widget.c`（新增 `tinyui_widget_detach_from_parent`/`tinyui_widget_destroy_common`）、`tinyui/src/core/runtime_bridge.c:399-412`（`detach_from_parent` 改走 ld 树）、单测追加到 `test_tinyui_core_helpers.c`

- [ ] **先写单测**：构造 parent+2 child（经公共 create API），断言 `tinyui_widget_detach_from_parent(child)` 后 `ldBaseGetChildCount(parent_ld)` 减 1、child 不再出现在 `ldBaseGetChildList` 遍历；断言 `tinyui_widget_destroy_common(child, depose_cb)` 调用 depose_cb 一次且 free 单对象（用计数 stub）。→ **预期红**。
- [ ] 实现 `tinyui_widget_detach_from_parent(struct tinyui_widget *w)`：替代 8 处 `finish_detach_after_backend_failure` —— 改用 `ldBaseNodeRemove((arm_2d_control_node_t*)w->ld_widget)`（现 `runtime_bridge.c:408` 已用）摘 ld 树节点，不再维护 backend 父链表。
- [ ] 实现 `tinyui_widget_destroy_common(struct tinyui_widget *w, void (*ld_depose_cb)(...))`（签名 `[执行时定稿]`）：`detach + unbind（清 pInfo、清 bridge 字段）+ ld_depose_cb(w->ld_widget) + free(w)` 单次。替代 16 个 `*_dispose_partial`。
- [ ] 改 `runtime_bridge.c` 的 `detach_from_parent`/`unbind_host`/`backend_detach`：入参 `void *backend_widget` 改 `struct tinyui_widget *`，内部 `widget->parent/next_sibling/host_widget/ld_event_bridge_*` 等改为新折叠字段或改走 ld 树。→ `ctest -R test_tinyui_core_helpers` **预期绿**。

## C1-T4：core helper —— create_leaf + item-list 模型（先写单测，红→绿）

**Files:** `tinyui/src/core/widget.c`（新增 `tinyui_widget_create_leaf`、core item-list 模型）、单测追加到 `test_tinyui_core_helpers.c`

- [ ] **先写单测**：用一个最简 kind（如 LABEL）经 `tinyui_widget_create_leaf` 构造，断言：单次 calloc（host stub 计数）、`ld_widget != NULL`、`ld_name_id` 在父之后分配、attach 后父 child_count +1、bind 后 `((ldBase_t*)w->ld_widget)->pInfo == w`；再断言 ld_init_cb 返回失败时**回滚链单 free、无泄漏**（depose 调一次）。→ **预期红**。
- [ ] 实现 `tinyui_widget_create_leaf(parent, kind, ld_init_cb, ctx, host_size)`（签名 `[执行时定稿]`，名称锁定）：① `calloc(1, host_size)` 单对象；② `next_name_id`（保持「name_id 在 ld 创建前取、传入 ld_init」次序 —— 高风险 #3，照 `message_box.c:118→125` 现序）；③ 调 per-widget `ld_init_cb(ctx, scene, name_id, parent_name_id...)` 拿 `ld_widget`；④ 写 `w->ld_widget/ld_name_id/kind/owner`；⑤ attach 到 ld 树（child 挂 parent 的 ld 节点）；⑥ `bind_host`（写 `pInfo=w`、connect native events、init value guard）；⑦ **任一步失败按逆序回滚：已 init 的 ld 调 depose、已 calloc 的 free，单次**（高风险 #4）。
- [ ] 实现 core item-list 模型（签名 `[执行时定稿]`）：把 list/combo_box/scroll_selecter 的「`items[]` + selected_index 同步」抽到 core（icon_slider/radial_menu 的 `item_sources[]` 因带 image 指针，C2 时按需复用或保留私有，本步只做文本三件套）。→ `ctest -R test_tinyui_core_helpers` **预期绿**。

## C1-T5：事件 pInfo 反查改指向 widget（高风险 #1，先隔离验证）

**Files:** `tinyui/src/core/runtime_bridge.c:29/51/390`、`tinyui/src/core/event.c:458-671`（`dispatch_native_signal` 经 pInfo→host）、`tinyui/src/core/widget.c:368-382`（`backend_host`/`get_host` 反查）

- [ ] **先隔离验证（高风险 #1）**：确认 `pInfo` 是 ld 原生字段（`src/gui/ldBase.h:225`）且 `ldList.c` 内部也用它 —— 读 `ldBase.h` 确认 `pInfo` 仅作「用户挂载指针」语义、ld 自身不写它做内部用途（若 ld 内部写，则不能借 pInfo）。`gitnexus_impact({target:"tinyui_runtime_bridge_ld_event_bridge_slot"})` 报告。
- [ ] 改写 `runtime_bridge.c:51` `sender->pInfo = backend` → `sender->pInfo = widget`（指向 `tinyui_widget`）；`:29` 事件 slot 的 `(struct tinyui_backend_widget *)...->pInfo` → `(struct tinyui_widget *)...->pInfo`；`:390` 清 pInfo 不变。
- [ ] `dispatch_native_signal`（`event.c:458`）入参与内部 cast 从 backend 改 widget；`tinyui_widget_backend_host`/`get_host`（`widget.c:368-382`）的「backend→host」反查**改为「ld→widget」经 pInfo**（或直接传 widget），保证 `tinyui_widget_dispatch_native_signal` 仍能从 ld sender 拿到正确 `tinyui_widget`。
- [ ] **此步先不改 7 个 widget 的 pInfo slot**（留 C1-T7 一次性改），但 core 侧改完后这 7 个 widget 暂时编译/行为不一致是预期 —— 用 `test_tinyui_button_events`/`test_tinyui_checkbox`/`test_tinyui_slider`/`test_tinyui_switch`/`test_tinyui_list` 这 5 个**纯 core 路径**的事件单测验证 core 改对：`ctest -R '^test_tinyui_(button_events|checkbox|slider|switch|list)$'` → **预期绿**（这些 widget 不在 7 个私有 pInfo reader 里，走 core slot）。

## C1-T6：value guard 改宿主字段 + 树 getter 改走 ld + 收口编译（高风险 #2）

**Files:** `tinyui/src/core/event.c:407-660`、`tinyui/src/core/widget.c:1191-1432`（三写 setter）、`:1509-1769`（双真相 getter + 树 getter）、`:444-468`（`find_by_name_id`）

- [ ] **value guard（高风险 #2）**：`event.c` 的 `backend->value` 全部改 `w->value`（folded 字段）。逐处核对：`dispatch_signal:408`、`checkbox:553`、`switch:582`、`slider:611`、`list:642/654` —— guard 形态（compare-then-update）**保持不变**，只换字段持有者。删 C 类记账写点（`data_model_epoch++`/`last_data_source`/`last_signal`/`dispatch_count`）—— **注意**：若 Phase 0 已删 C 类字段，此处应已无残留；若发现残留即 Phase 0 未净，回 C1-T0 门。
- [ ] **三写 setter 改单写**：`set_flex_grow`(`widget.c:1207-1209`)、`set_flex_new_track`(`:1239-1241`)、`set_ignore_layout`(`:1367-1369`)、`set_grid_cell`(`:1412-1430`) —— 删 `backend_widget->child_layout.*` 中间写，保留 `ldBaseSet*` + `widget->*` 影子。
- [ ] **双真相 getter 去 backend 分支**：`get_x/y/width/height/visible/opacity/selectable/selected/corner`（`widget.c:1509-1652`）的 `ld_base = tinyui_widget_get_ld_base(...)` 改为 `(ldBase_t*)widget->ld_widget`（`get_ld_base` 内部 `:118-131` 改读 `widget->ld_widget`），逻辑 `ld ? ldBaseGetX(ld) : widget->x` 保留。
- [ ] **树 getter 改走 ld 树 + 反查**：`get_parent`(`:1662`)/`first_child`(`:1683`)/`next_sibling`(`:1704`)/`get_root`(`:1725`) 从 `backend->parent/first_child/next_sibling/root` 改为 `ldBaseGetParent/ChildList/NextSibling` + `ldBaseGetRootNode`，再经 `pInfo` 反查回 `tinyui_widget`；`get_child_count`(`:1746`) 删 backend fallback、只留 `ldBaseGetChildCount`；`find_by_name_id`(`:444/1804`) 改用 ld 树遍历 + `ldBaseGetNameId`（或保留 folded `ld_name_id` 字段遍历 ld 树）。
- [ ] 删 backend 树维护残骸：`clear_owner_and_root`(`:314`)、`bind_subtree_owner_and_root`(`:331`)、`can_attach_child`(`:295`) 改为只在 ld 树 + folded 字段上工作；`attach_child`(`:470`)/`backend_detach`(`:504`) 改走 ld 树 + `tinyui_widget_detach_from_parent`。
- [ ] **收口编译**：此时 core 应自洽。`rtk cmake --build build/tinyui-runtime -j` —— **预期**：core 编译通过，但 29 个 widget `.c` 仍引用 `backend_widget`/旧 helper → **预期大量 widget 编译错**（C2 修）。先确保 `core/*` + `event.c` + `runtime_bridge.c` 这几个 TU 单独编过（可临时用 `--target` 单编 core 库目标核对）。

## C1-T7：7 个 pInfo-reader widget 的 slot 同步（与 core 同写面，串行收尾）

**Files:** `combo_box.c:67`、`icon_slider.c:77`、`line_edit.c:77`、`radial_menu.c:78`、`table.c:232`、`message_box.c:62`、`keyboard.c:212`（**仅改这 7 处 pInfo cast，不做 C2 的全量 widget 迁移**）

- [ ] 把这 7 处 `(struct tinyui_backend_widget *)((ldBase_t*)sender)->pInfo` 统一改成 `(struct tinyui_widget *)...->pInfo`，并把后续对 `backend->host_widget`/`backend->kind`/`backend->ld_widget` 的取用改为直接用 `tinyui_widget*`（或 `w->ld_widget` 等 folded 字段）。**keyboard 与 message_box 用 `ld_keyboard`/`ld_message_box` 而非 `msg.ptSender`**，按各自局部变量改。
- [ ] 这 7 个 widget 的其余部分（create/dispose/setter）留 C2 全量迁移；本步只让 pInfo 事件回调编过且语义对。
- [ ] **C1 行为保持验证**：单 widget 事件单测 `ctest -R '^test_tinyui_(combo_box|icon_slider|line_edit|radial_menu|table|message_box|keyboard)$'`（若这些 widget 尚未在 C2 迁移完成，可能因其他部分编译错而无法跑 —— 此时把验证推迟到 C2 该 widget 完成时，但 pInfo slot 的改动本身必须 review 通过）。

## C1 验收（进入 C2 前 MUST 全绿）

- [ ] `tinyui_widget` 已含 10 个 folded 字段、无 `backend_widget` 成员；6 个 core helper + item-list 模型已实现且 `test_tinyui_core_helpers` 绿。
- [ ] core 侧 `event.c`/`runtime_bridge.c`/`widget.c` 编译通过；pInfo 指向 widget；value guard 用 `w->value`；树 getter 走 ld 树。
- [ ] 不依赖 widget 全量迁移即可绿的 core 路径单测全绿：`ctest -R '^test_tinyui_(button_events|checkbox|slider|switch|list|app_lifecycle|event|layout|theme)$'`。
- [ ] `gitnexus_detect_changes()` 确认改动范围 = `core/*` + `event.c` + 7 widget 的 pInfo slot，无意外外溢。

---

# C2 — per-widget 迁移（并行扇出，1 subagent / widget，写面=单 widget）

> **执行模式：并行扇出**（spec §7）。**每个 widget = 一个 subagent 任务单元**；该 widget 的 `.c` + `.h`（若有）+ 其单测 `tests/tinyui/unit/test_tinyui_<widget>.c` 视为**同一写面，归同一 subagent**。**严禁两个 subagent 写同一文件**——29 个 widget 写面互不重叠，故真正可并行。
> **安全网**：每个 widget 改完，subagent 跑该 widget 单测 + 与 C1 末尾归档的截图比对相关 demo 页；review 不通过**在同一 subagent 内修复**。
> **共享只读依赖**：所有 widget 复用 C1 的 core helper（`tinyui_widget_create_leaf`/`destroy_common`/`rgb_to_ld_color`/`ld_color_to_rgb`/`detach_from_parent`/`align_to_arm2d`/item-list）—— **只读、不改 core**；若发现 helper 缺口，**回报主线程**统一在 C1 写面补（不得各 widget 私改 core）。
> 主线程职责：派发/收敛、跨 widget 的 helper 缺口决策、`gitnexus_impact`（对仍被跨文件引用的 widget 公共符号）、C2 末尾全量回归 + 截图。

## C2 统一 per-widget 迁移模板（transformation recipe —— 对每个 widget 的标准变换）

每个 widget 的 subagent **按此 5 步**改造（逐行 diff 标 `[执行时定稿]`，但步骤本身写实）：

- **① 双 calloc → 单对象**：删 `calloc(backend)`（及 wrapper widget 的 `calloc(host_wrapper)`）；只保留 `calloc(1, sizeof(struct tinyui_<widget>))`。原 backend 字段全部改读 folded `widget->ld_widget/ld_name_id/kind/owner/...`。
- **② create 改调 `tinyui_widget_create_leaf`**：把现有「`calloc`→`ld<Xxx>_init`→`init_child`→`attach_child`→`bind_host`→回滚」整链替换为一次 `tinyui_widget_create_leaf(parent, KIND, <widget>_ld_init_cb, &ctx, sizeof(struct tinyui_<widget>))`；per-widget 仅保留一个 `ld_init_cb`（封装该 widget 专属的 `ld<Xxx>_init(scene, NULL, name_id, parent_name_id, x, y, w, h, ...)` 调用与参数）。post-bind 初始化（如 progress_wheel 的 `set_percent(0)/set_dot_enabled(1)`、table/combo_box 的 native slot bind）保留在 create 尾部。
- **③ dispose 改调 `tinyui_widget_destroy_common`**：删本地 `*_dispose_partial` + `finish_detach_after_backend_failure`；改调 `tinyui_widget_destroy_common(&w->widget, <widget>_ld_depose_cb)`（depose_cb 封装 `ld<Xxx>_depose`）。
- **④ 删本地 `get_ld`/`rgb_to_ld_color`/`ld_color_to_rgb`/`align_to_ld`/`finish_detach`，改用 core**：私有 `tinyui_<widget>_get_ld` → 直接 `(ld<Xxx>_t*)w->widget.ld_widget`（或保留一个 1 行 inline 取 folded 字段）；私有颜色/align helper 删除，全部改调 `tinyui_rgb_to_ld_color`/`tinyui_ld_color_to_rgb`/`tinyui_align_to_arm2d`。
- **⑤ 双层 setter 合并**：把 `tinyui_backend_<widget>_set_*`（内层调 ld）与 `tinyui_<widget>_set_*`（外层写影子）合并为单层：直接 `ld<Xxx>Set*(...)` + 写 `w->widget.<shadow>`，删 backend 内层函数。

**模板验收（每 widget）**：单 calloc / 单 free；无私有 `get_ld`/颜色/align/detach 拷贝；无 `*_dispose_partial`/`*_backend_*` 双层 setter；`ctest -R '^test_tinyui_<widget>$'` 绿；相关 demo 页截图与 C1 基准一致。

## C2 widget 清单（29 个，各 1 subagent；标注特殊点 + 该 widget 单测）

> 写面 = 列「文件」+ 对应 `tests/tinyui/unit/test_tinyui_<name>.c`（清单见 `tests/tinyui/CMakeLists.txt:1-44`）。逐行 diff 标 `[执行时定稿]`。

**A. 简单叶子（仅模板 ①–⑤，无 native slot / 无 inline 数组）**：
- [ ] `label.c`（create `:135-215`，dispose_partial `:102`，get_ld `:75`，rgb `:32`，align map/unmap `:49/63`，ld→rgb `:37`）
- [ ] `text.c`（dispose_partial `:78`，get_ld `:37` `_get_ld_text`，rgb `:32`）
- [ ] `image.c`（dispose_partial `_impl :77`，finish_detach `:46`，rgb `:30`，**无 get_ld**——直接用 folded；observe/step/theme 对 IMAGE 有特判，注意公共行为）
- [ ] `qrcode.c`（dispose_partial `_impl :83`，finish_detach `:52`，get_ld `:36`）
- [ ] `animation.c`（get_ld `:26`）
- [ ] `date_time.c`（get_ld `:55`，rgb `:29`，align map `:34` 输出参数变体）
- [ ] `clock.c`（get_ld `:36`）
- [ ] `calendar.c`（dispose_partial `:407`，get_ld `:54` void* 变体；theme 对 CALENDAR 有 cast `:332`）
- [ ] `graph.c`（get_ld `:58`/`_get_ld_const :69`，经 `tinyui_graph_backend` 间接）
- [ ] `canvas.c`（**无 get_ld**，align `:33`；canvas 自维护 `commands[]`——非 ld item-list，保留私有）

**B. value 类（走 core slot，value guard 已在 C1 改；模板 ①–⑤）**：
- [ ] `checkbox.c`（dispose_partial `:72`，get_ld `:36`，rgb `:31`；value=`checked`）
- [ ] `switch.c`（get_ld `:42`；value=`checked`；**switch_wrapper_struct_delta_bytes** perf metric 测的就是它，C3 重测注意）
- [ ] `slider.c`（dispose_partial `:74`，get_ld `:49`；value=`value`，bind_host 失败走 dispose_partial `:224`）
- [ ] `progress_bar.c`（dispose_partial `_impl :119` 含 test 快照，finish_detach `:88`，get_ld `:77`，rgb `:56`；**测试快照脚手架移到测试侧**）
- [ ] `progress_wheel.c`（dispose_partial `_impl :217` 含 test 快照，finish_detach 主 `:99`+test `:191`，get_ld `:88`，rgb `:47`；post-bind `set_percent/set_dot_enabled`；**测试快照移测试侧**）

**C. native-event slot widget（特殊点：保留 native slot 绑定，pInfo slot 已在 C1-T7 改）**：
- [ ] `button.c`（**wrapper 高风险 #5**：`tinyui_button_backend_host{widget; action_info}` `:28`；create `_alloc :117-197`，`_xBtnInit(&host->action_info) :180`，dispose `xBtnRemove + free(host) :94`；单对象化须把 `action_info` 并入 `struct tinyui_button` 内联字段，create_leaf 后单独 `_xBtnInit`）
- [ ] `table.c`（pInfo `:232`，dispose_partial `:98`，finish_detach `:68`，get_ld `_get_ld_widget :163`，align `:191` 输出参数变体，native slot `tinyui_table_bind_native_slot :433`/`:216-270`；`edit_result_on_finish` folded 字段；**测试快照移测试侧**）
- [ ] `combo_box.c`（pInfo `:67`，dispose_partial `:409`，get_ld `:42` void* 变体，rgb `:33`，双层 setter `set_text_color_ld :218`/`set_text_color :612`，native slot `bind_host :462`/`:53-112`，**inline item 三件套**→复用 core item-list）
- [ ] `icon_slider.c`（pInfo `:77`，get_ld `:51` void* 变体，native slot；**inline `items[]`+`item_sources[]`**——image 指针保留私有或扩展 item-list）
- [ ] `radial_menu.c`（pInfo `:78`，get_ld `:52` void* 变体，native slot；**inline `items[]`+`item_sources[]`**）
- [ ] `line_edit.c`（pInfo `:77`，dispose_partial `:335`，get_ld `:35` void* 变体，rgb `:59`，align `:46`，native slot；`edit_result_on_finish` folded）
- [ ] `arc.c`（dispose_partial `_impl :128` 含 test 快照，finish_detach `:97`，get_ld `:86`，rgb `:53`/ld→rgb `:58`，双层 setter `backend_arc_set_color :461`/`set_color :475`，native slot；**测试快照移测试侧**）
- [ ] `gauge.c`（dispose_partial `_impl :130` 含 test 快照，finish_detach `:99`，get_ld `:88`，rgb `:55`/ld→rgb `:60`，native slot；**测试快照移测试侧**）
- [ ] `message_box.c`（pInfo `:62` 用 `ld_message_box`，create `:90-171` 直写 `backend->ld_widget` 非标准——改用 create_leaf 或 leaf 变体，get_ld `:35`，`confirm_bridge :51`）
- [ ] `keyboard.c`（pInfo `:212` 用 `ld_keyboard`，get_ld `_get_ld_widget :93`，**`native_layout` + 动态 `layout_entries`**——native slot + 动态内存，单对象化注意 layout_entries 的 free 在 destroy_common 之外另管）
- [ ] `scroll_selecter.c`（get_ld `:66` void* 变体，rgb `:39`，**inline item 三件套**→复用 core item-list，native-event 相关）
- [ ] `list.c`（dispose_partial 见 16 文件清单，get_ld `:57`，rgb `:29`，align `:34`，**inline item 三件套 + `list_item_count` folded**→复用 core item-list；走 core slot 的 LIST 事件已在 C1 改）

**D. 容器（wrapper 高风险 #5；window/background 非可销毁子节点，特殊处理）**：
- [ ] `window.c`（**wrapper**：`tinyui_window_backend_host{widget; padding_group; has_padding_group}` `:169-173`；get_ld `_get_ld_window :207`，rgb `:34`/ld→rgb `:39`，4 个 flex/grid 专属 align 映射 `:74/94/111/131` **保留 window 私有**；承接 C1-T1 搬入的 padding 字段，把 `padding_group`/`has_padding_group` 并入 `struct tinyui_window` 内联；window 是 root、不走 child detach/destroy）
- [ ] `background.c`（**wrapper**：`tinyui_background_backend_host{widget}` `:27-29` 无额外字段，纯去 wrapper；**无 get_ld**，rgb 经 window；background 包 window，特殊点同 window）

## C2 验收（进入 C3 前 MUST 全绿）

- [ ] 29 个 widget 全部单 calloc / 单 free，复用 6 个 core helper；无残留私有 `get_ld`/`rgb_to_ld_color`/`ld_color_to_rgb`/`align_to_ld`（window 4 个 flex/grid 映射除外）/`finish_detach_after_backend_failure`/`*_dispose_partial`/双层 `*_backend_*` setter。
- [ ] `grep -rn '_dispose_partial\|finish_detach_after_backend_failure' tinyui/src/widgets`（`dangerouslyDisableSandbox=true`）→ **预期 0 行**。
- [ ] 全量单测 `ctest --test-dir build/tinyui-runtime -L 'tinyui;unit' --output-on-failure` → **预期全绿**（41 个 widget/core 单测）。
- [ ] 测试快照脚手架（arc/gauge/progress_bar/progress_wheel/table）已移到测试侧，产品 `.c` 不再含 `*_test_*_dispose_snapshot` 产品代码。
- [ ] 每个 widget subagent 回报：该 widget 相关 demo 页截图与 C1 基准一致（或说明该 widget 无独立 demo 页，靠 `basic_widgets` 综合页覆盖）。

---

# C3 — final cleanup（串行，单写者）

> **执行模式：串行单写者**。写面 = `runtime_internal.h` 删类型、`port/sdl/{observe.c,step.c,host_internal.h}`、`theme/theme.c`、`layout/{flex.c,grid.c}`、perf 基线 + 探针 + py gate。**不可并行**（多文件强耦合于「类型删除」这一原子动作）。
> **纪律**：删类型前 `gitnexus_impact({target:"tinyui_backend_widget", direction:"upstream"})` 确认 C1/C2 后只剩 C3 这批消费者；删后 `gitnexus_detect_changes()`。

## C3-T1：迁移 theme.c（~59 处，最重耦合）

**Files:** `tinyui/src/theme/theme.c`（`tinyui_theme_apply_widget_style :392`、`backend_widget->ld_widget` 各 cast `:168/175/185/196/214/233/272/305/323/332`、`switch(kind)` `:70-97/407`、`backend_widget->theme->colors[]`）、`tinyui/src/core/internal.h:267-272`（`tinyui_theme_apply_widget_style` 声明）

- [ ] `gitnexus_impact({target:"tinyui_theme_apply_widget_style"})` 报告 blast radius。
- [ ] `tinyui_theme_apply_widget_style` 入参 `void *backend_widget` → `struct tinyui_widget *widget`；内部 `backend_widget->ld_widget` 全改 `widget->ld_widget`、`backend_widget->kind` → `widget->kind`、`backend_widget->theme` → 经 `widget->owner->theme`（folded）。各 `ld*_t` cast 不变。
- [ ] `rtk cmake --build build/tinyui-runtime -j` → 预期 theme.c 编译通过；`ctest -R '^test_tinyui_theme$'` → **预期绿**。

## C3-T2：迁移 port/sdl/observe.c + step.c + host_internal.h（~42 + ~27 + 4 处）

**Files:** `tinyui/port/sdl/observe.c`（签名 `:26/66/107/115/129/130/208/223/238/284`、`->kind`/`->ld_widget`/`->id`/`first_child`/`next_sibling`/`runtime_evidence_flags`）、`tinyui/port/sdl/step.c`（签名/`->kind`/`->ld_widget`/`first_child`/`next_sibling`/bridge 调用/cast）、`tinyui/port/sdl/host_internal.h:52-56`（4 个原型）

- [ ] `host_internal.h:52-56` 的 4 个原型入参 `const struct tinyui_backend_widget *` → `const struct tinyui_widget *`（`window_has_real_layout`/`widget_allows_smoke_layout`/`log_mapping_markers`/`log_image_source_marker`）。
- [ ] `observe.c`：所有 helper 签名同步改 `const struct tinyui_widget *`；`->kind`/`->ld_widget`/`->id` 改 folded 字段（`->id` 改读各子 struct 的 `id` —— observe 主要在 IMAGE 分支 `:287/291` 读 id，改 `((struct tinyui_image*)w)->id` 或公共 id getter）；树遍历 `first_child`/`next_sibling` 改 `ldBaseGetChildList`/`ldBaseGetNextSibling` + pInfo 反查；**`runtime_evidence_flags` 两处（`:118/226`）**：Phase 0 已删该字段 → 这两个分支应已是死分支，C3 直接删（spec §4 L0 已规划，若 Phase 0 未净则回 C1-T0 门）。
- [ ] `step.c`：同 observe.c —— 签名改 widget、树遍历改 ld、`bridge_backend_state*` 调用因 Phase A 已折叠应已是字段直读（核实）；cast `window->widget.backend_widget`（`:199/201`）→ `&window->widget` 或 `window->widget` 指针。
- [ ] `hal.c` 确认仍 0 引用、不动。
- [ ] `rtk cmake --build build/tinyui-runtime -j && ctest -R '^test_tinyui_port_' ` → **预期绿**；runtime gate `ctest -L 'tinyui;runtime'` → **预期绿**（observe/step 是 capture/layout 日志的来源，必须截图通过）。

## C3-T3：迁移 layout/flex.c + grid.c（各 1 处）

**Files:** `tinyui/src/layout/flex.c:22-28`、`tinyui/src/layout/grid.c:21-33`

- [ ] flex.c/grid.c 的 helper 从 `window->widget.backend_widget` 取 backend 做 validity 检查，改为直接判 `window->widget.ld_widget != NULL`（或 `&window->widget`）；转发到 `tinyui_window_apply_*` 不变（纯 facade）。
- [ ] `ctest -R '^test_tinyui_layout$'` → **预期绿**。

## C3-T4：删 `struct tinyui_backend_widget` 类型 + `runtime_internal.h` 内容搬迁

**Files:** `tinyui/src/core/runtime_internal.h`（整文件评估删除）、`tinyui/src/core/internal.h`（承接残留声明）、`tinyui/src/core/native.c`（基本整删，spec §3.5）

- [ ] `gitnexus_impact({target:"tinyui_backend_widget"})` —— 确认 upstream 只剩本 Task 范围；HIGH/CRITICAL 报告。
- [ ] 删 `struct tinyui_backend_widget`（`runtime_internal.h:162-201`）、`struct tinyui_backend_app_state`（`:203-210`，若 Phase A 已删则确认无残留）、两个 layout cache struct（`:120-158`）、evidence flags enum（`:113-116`，Phase 0 域）、test snapshot 前置声明与 test helper 声明（`:212-236`，已随 C2 移测试侧）。
- [ ] `enum tinyui_backend_widget_kind` 已在 C1-T1 搬到 `internal.h` —— 确认 `runtime_internal.h` 不再定义它；保留的 app 生命周期声明（`tinyui_backend_app_init/run/shutdown :219-221`）若仍需则并入 `internal.h`。
- [ ] `runtime_internal.h` 删空后从 `internal.h:22` 的 `#include` 移除并删文件（spec §3.5：「文件本身可消失」）。`native.c`（`signal_to_ld` 表 + 仅服务 C 类的 readback policy）按 spec §3.5「基本整删」处理（Phase 0 域，核实是否已删）。
- [ ] `runtime_bridge.c`/`runtime_bridge.h` 保留瘦身（真实 app↔scene 桥 / 事件 connect / name_id 分配 / pointer bridge）—— 不改名（spec §3.4 改名 `ldgui_app_bridge.c` 为可选，本 plan 不做）。
- [ ] `rtk cmake --build build/tinyui-runtime -j` → **预期全工程编译通过**。

## C3-T5：perf 基线 + 探针 + py gate 重写

**Files:** `tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c:10-11`、`tests/tinyui/perf/tinyui_perf_baseline.json:65-83`、`tests/tinyui/perf/check_tinyui_object_overhead.py:49/103-104/120-133/152-153/175-179`

- [ ] 探针 `test_tinyui_wrapper_struct_overhead.c`：删第 10-11 行 `sizeof(struct tinyui_backend_widget)` 输出；保留 `sizeof(struct tinyui_widget)` 与 switch delta 两行。
- [ ] `check_tinyui_object_overhead.py`：`:49` 的 key 元组删 `"backend_widget_struct_bytes"`；`:103-104` 探针解析行、`:152-153` 读取行、`:175-179` gate 断言行同步删；`_run_self_test()`（`:120-133`）的 backend 桩断言一并清理。
- [ ] **重测后改 baseline**：先 `rtk cmake --build build/tinyui-runtime --target test_tinyui_wrapper_struct_overhead && build/tinyui-runtime/tests/tinyui/test_tinyui_wrapper_struct_overhead` 读出新的 `WIDGET_WRAPPER_STRUCT_BYTES` 与 `SWITCH_WRAPPER_STRUCT_DELTA_BYTES`（实测值，预期 widget 从 184 升到 ~240–256B 区间——只增 B 类真实字段）；据实测改 `tinyui_perf_baseline.json:70-77` 的 `widget_wrapper_struct_bytes.baseline_bytes/max_allowed_bytes` 与 `switch_wrapper_struct_delta_bytes`；**删除** `:78-80` 的 `backend_widget_struct_bytes` 整块。
- [ ] `ctest -R '^check_tinyui_object_overhead$'` → **预期绿**（新 baseline）；`check_tinyui_binary_size`/`check_tinyui_perf` → 预期绿（结构合并对运行时 perf 影响低，binary size 可能小幅下降，超限则据实调 baseline 并说明）。

## C3-T6：contract / transition guard 基线同步 + 全量回归 + SDL 截图比对

**Files:** `tests/tinyui/contract/check_tinyui_transition_guards.py`、`check_tinyui_v21_transition_guards.py`、`tests/tinyui/runtime/check_tinyui_backend_mapping.py`/`check_tinyui_runtime.py`/`check_tinyui_visible_ui.py`（按 spec §6 微调）

- [ ] 跑 9 个 contract gate `ctest -L 'tinyui;contract' --output-on-failure`；transition-guard 若因结构/头文件变更触发 baseline drift（spec §6），据实更新其基线（**注意**：本 plan 不动 `app.h` 公共导出——那是 Phase B 域；C 仅在结构删除触发 drift 时同步）。
- [ ] `check_tinyui_backend_mapping`/`runtime`/`visible_ui` 的 runtime marker 微调（`EXCLUDE_FORMAL_MAPPING` marker 源于已删死字段，可删；`SMOKE_LAYOUT_USED` 源于 `host_internal.h`、与 backend 无关、保留）。
- [ ] **全量回归**：`ctest --test-dir build/tinyui-runtime --output-on-failure`（unit + contract + perf + runtime 全部）→ **预期全绿**。
- [ ] **SDL demo 真实输出逐页截图比对（硬验收）**：对 `hello_world`/`basic_widgets`/`layout_flex`（及各 widget demo 页）逐页跑 `SDL_VIDEODRIVER=dummy TINYUI_DEMO_AUTO_QUIT_MS=1200 TINYUI_CAPTURE_FILE=<tmp>/frame.ppm build/tinyui-runtime/examples/sdl/tinyui_demo <demo>`，与 C1-T0 归档基准**逐页逐像素比对**（PPM 同尺寸、非背景像素结构一致）→ **预期一致**。任何页不一致即视为行为破坏，回到对应 C2 widget 或 C3 消费者 subagent 修复。
- [ ] `gitnexus_detect_changes()` 终检：确认全部改动落在 C1+C2+C3 预期符号/流程内。

---

## 相位验收 checklist（Phase C 完成判据）

- [ ] **类型消失**：`struct tinyui_backend_widget` / `struct tinyui_backend_app_state` / `runtime_internal.h` 在生产代码中已消失；`tinyui/src` / `tinyui/port` 不再存在对这两个 struct 或 `backend_app` 的结构依赖。允许保留 `enum tinyui_backend_widget_kind` 这一 folded kind 名称。
- [ ] **单层模型**：`struct tinyui_widget` 持 folded ld 绑定字段，无 `backend_widget` 成员；29 个 widget 每个**单 calloc / 单 free**；绝大多数 leaf/child create 走 `tinyui_widget_create_leaf`、dispose 走 `tinyui_widget_destroy_common`。`background` 与 root `window` 保留真实 `ldWindow_init` 专门路径，作为 root/container 边界的有意例外，不再视为 C3 缺口。
- [ ] **样板收敛**：`grep -rn '_dispose_partial\|finish_detach_after_backend_failure\|_rgb_to_ld_color\|_ld_color_to_rgb' tinyui/src/widgets` → 仅余 window 的 4 个 flex/grid 专属映射（不属上述模式），其余清零；颜色/align/detach/create/destroy 全部复用 6 个 core helper。
- [ ] **5 条高风险落地确认**：#1 pInfo 指向 `tinyui_widget`（core + 7 widget，比 spec 多 keyboard）、#2 value guard 用 `w->value`、#3 name_id 在 ld 创建前取序不变、#4 单 free 在 create_leaf 回滚链、#5 button/window/background 三 wrapper 内联字段（`action_info`/`padding_group`）已并入单对象。
- [ ] **perf 处置**：`backend_widget_struct_bytes` metric 已删、探针 `sizeof(backend_widget)` 行已删；`widget_wrapper_struct_bytes` 已据实重测改 baseline；`check_tinyui_object_overhead` 绿。
- [ ] **全量门绿**：`ctest --test-dir build/tinyui-runtime --output-on-failure` 全绿（unit 41 + 探针 + contract 9 + perf 3 + runtime 3）；contract/transition/runtime/perf gate 全过。
- [ ] **行为保持（最终硬验收）**：SDL demo 真实 LingDongGUI 输出**逐页与改前一致（截图比对通过）**——「能弹窗」不算，必须逐页像素结构一致。
- [ ] **能力不丢**：27/27 widget-like 用户控件能力覆盖不变（widget contract matrix gate 绿）。
- [ ] `gitnexus_detect_changes()` 确认无意外外溢。
