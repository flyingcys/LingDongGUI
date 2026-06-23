# TinyUI Phase A:app_state 折叠 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (- [ ]).

**Goal:** 把 `tinyui_backend_app_state` 的三个真实字段(`ld_scene` / `next_ld_name_id` / `runtime_state`)并入 `struct tinyui_app`,删除该中间 struct 类型与 `void *backend_app` 指针、删除其 `calloc`/`free`,删除冗余字段(`theme` 与 `app.theme` 重复、`last_window_switch_mode/_duration_ms` 0 读)。3 个访问器 `tinyui_runtime_bridge_backend_state` / `_from_window` / `_from_parent` 退化为返回 `struct tinyui_app *`(对字段直读),修 3 处绕过访问器的裸 cast(`window.c:356`、`window.c:732`、`table.c:635`)。保留语义:scene 的 `bUserAllocated` 标记 + shutdown 释放序(`ldGuiDespose` 后再 `free(ld_scene)`)。**不丢失任何用户控件能力、不改任何用户可见 API 行为。**

**Architecture:** shared core,**串行单写者**。本相位写面集中在 `core/*` + 全部 `widgets/*.c` + `drivers/*` + `port/sdl/step.c` + ~20 个测试,且这些改动强耦合(删一个 struct 类型必然连锁触碰所有引用该类型的 translation unit),**不可并行**,所有 Task 必须按编号顺序由同一 writer 顺序执行。review 不通过在同一 subagent 内修复,不要新开 subagent、不要在主 agent 修复。

**Tech Stack:** C(C11,ID-based LingDongGUI + ARM-2D 底层)/ CMake(out-of-tree `build/`,lib target `tinyui_core`)/ CTest(72 个用例,关键 `test_tinyui_app_lifecycle` #32)/ gitnexus(impact / detect_changes)。

---

## 0.4 当前执行快照（2026-06-23，持续更新）

> 只记录已经由当前代码与 focused verification 坐实的事实；未闭环项不写成完成。

### 已完成并验证

- **`struct tinyui_app` 已持有 `ld_scene / next_ld_name_id / runtime_state`。**
  - 当前真相：`tinyui/src/core/internal.h` 的 `struct tinyui_app` 已直接定义这 3 个字段，`void *backend_app` 已不存在。

- **`tinyui_runtime_bridge_backend_state*` 访问器已退化为返回 `struct tinyui_app *`。**
  - 当前真相：
    - `tinyui/src/core/runtime_bridge.h` 中
      - `tinyui_runtime_bridge_backend_state`
      - `tinyui_runtime_bridge_backend_state_from_window`
      - `tinyui_runtime_bridge_backend_state_from_parent`
      当前都返回 `struct tinyui_app *`
    - 当前源码侧使用点已直接按 `app_state->ld_scene` / `app_state->runtime_state` 读取

- **Phase A 目标里的 `backend_app` 代码路径已基本从 `tinyui/include/src/port` 消失。**
  - 当前真相：
    - `rtk rg -n "backend_app\\b|tinyui_backend_app_state" tinyui/include tinyui/src tinyui/port`
      当前只剩 0 命中或文档外影子清零
    - `tinyui/include/tinyui.h` 里也不再透传 `app.h`

- **Phase A 相关 transition 基线当前为已通过状态。**
  - 当前真相：
    - `tests/tinyui/contract/tinyui_transition_inventory.json`
      - `backend_c_files: 0`
      - `app_header_exists: true`
      - `app_source_exists: true`
      - `tinyui_public_api_count: 546`
  - focused verification：
    - `rtk ctest --test-dir build/tinyui-runtime -R '^(check_tinyui_transition_guards|check_tinyui_v21_transition_guards)$' --output-on-failure`

- **app 生命周期最小回归当前为已通过状态。**
  - focused verification：
    - `rtk ctest --test-dir build/tinyui-runtime -R '^(check_tinyui_transition_guards|check_tinyui_v21_transition_guards|test_tinyui_app_lifecycle)$' --output-on-failure`

- **`next_ld_name_id` 已确认仍归属 `struct tinyui_app`，但“唯一自增点在 runtime_bridge.c”这一旧计划假设已不再成立。**
  - 当前真相：
    - `tinyui/src/core/runtime_bridge.c` 在 `tinyui_runtime_bridge_init_app` 中执行 `app->next_ld_name_id = 0`
    - `tinyui/src/core/widget.c` 在通用创建路径中执行 `name_id = owner->next_ld_name_id++`
    - 多个 widget 源文件当前直接执行 `name_id = ++app_state->next_ld_name_id`
  - 结论：
    - Phase A 的“字段已折叠进 `tinyui_app`”这一目标已实现
    - 但旧计划里关于“唯一自增点 / 唯一分配路径”的描述已经过时，后续 closeout 不能再沿用该假设

- **当前 `name_id` 分配模型更接近“并存双轨”，不是单一 helper 收敛。**
  - 当前真相：
    - `tinyui/src/core/widget.c` 的 `tinyui_widget_create_leaf` 会：
      - 读取 `parent->widget.owner`
      - 执行 `name_id = owner->next_ld_name_id++`
      - 再调用传入的 `ld_init_cb(...)`
    - 但多个 widget 仍各自直接分配：
      - 例如 `tinyui/src/widgets/button.c` 的 `tinyui_button_alloc`
      - 例如 `tinyui/src/widgets/list.c` 的创建路径
      - 都是在本地先执行 `++app_state->next_ld_name_id`，再直接调用对应 `ld*init`
  - 当前判断：
    - 这是“通用 leaf helper 路径”和“widget 自建路径”并存
    - 目前不能把它描述成“所有 widget 已统一走 `tinyui_widget_create_leaf`”
    - 更准确地说：`tinyui_widget_create_leaf` 已实现、已声明、已有测试覆盖，但当前还不是生产 create 主路径的统一入口
    - helper 当前使用**后置自增**，而多数生产 widget 路径使用**前置自增**；若未来要统一收敛，这个首值语义差异必须单独处理

- **现有 `find_by_name_id` 语义当前未见回退证据。**
  - 当前真相：
    - `tinyui/src/core/widget.c` 的 `tinyui_widget_find_by_name_id` 仍按 `ld_name_id` 从 ld tree 查询
    - 只要 widget 最终把分配出的 `name_id` 写回 `widget.ld_name_id`，查找语义就保持一致
  - focused verification：
    - `rtk ctest --test-dir build/tinyui-runtime -R '^(test_tinyui_(button_events|list|window|table|combo_box|calendar))$' --output-on-failure`
  - 当前结论：
    - 至少在本轮覆盖到的 create/find 相关子集上，没有看到因为多路径分配而导致的 `name_id` 查找回退

### 本轮补核差异（2026-06-23）

- **旧文档中关于 `name_id` 的若干表述已与当前树不一致。**
  - 当前 `rtk rg -n "next_ld_name_id|tinyui_runtime_bridge_backend_state|tinyui_runtime_bridge_app_from|backend_app\\b|tinyui_backend_app_state" tinyui/include tinyui/src tinyui/port` 结果表明：
    - `backend_app` / `tinyui_backend_app_state` 旧路径已清空
    - `next_ld_name_id` 仍有多处真实使用点，分布在 `core/widget.c` 与多个 `widgets/*.c`
  - 因此，本文后续 Task 中凡是写着“唯一自增点在 `runtime_bridge.c`”或“仅 1 处自增”的段落，应视为**历史计划假设**，不是当前代码真相

- **Phase A closeout 现阶段更适合核对“字段归属与行为未回退”，而不是继续追求旧版单点分配模型。**
  - 本轮 focused verification 只证明：
    - `tinyui_app` 持有状态字段
    - transition guard 通过
    - `test_tinyui_app_lifecycle` 通过
    - `button/list/window/table/combo_box/calendar` 子集在当前 `name_id` 并存分配模型下仍通过
    - `tinyui_widget_find_by_name_id` 当前语义仍建立在 ld tree 的 `name_id + pInfo` 绑定之上，而不是建立在“唯一自增 helper”之上
  - 本轮**未**证明：
    - 所有 widget 的 `name_id` 分配路径已统一收敛到单一 helper
    - 历史计划 Task 6 中描述的“唯一自增点”模型仍成立

### 当前 closeout 判定

- **Phase A 的代码目标现在可以判定为已完成。**
  - 当前依据：
    - `tinyui_app` 已直接持有 `ld_scene / next_ld_name_id / runtime_state`
    - `backend_app` / `tinyui_backend_app_state` 旧代码路径已从 `tinyui/include`、`tinyui/src`、`tinyui/port` 清空
    - 访问器已退化为返回 `struct tinyui_app *`
    - `name_id` 相关 create/find 行为在当前模型下未见回退
  - focused verification：
    - `rtk ctest --test-dir build/tinyui-runtime -R '^(test_tinyui_(core_helpers|native_bridge|theme|widgets|button_events|list|app_lifecycle|app_timer|runtime_model|port_display|port_input|port_tick_os|app_window_switch|event|combo_box|table|window|calendar))$' --output-on-failure`
    - `rtk ctest --test-dir build/tinyui-runtime -R '^(check_tinyui_public_api|check_tinyui_transition_guards|check_tinyui_v21_transition_guards|check_tinyui_widget_contract_matrix|check_tinyui_release_capability_matrix)$' --output-on-failure`

- **不再把“统一到单一 `tinyui_widget_create_leaf` helper”当成 Phase A closeout blocker。**
  - 当前判断：
    - 这属于后续收敛/简化方向，不影响 Phase A 的 app_state 折叠目标是否完成
    - Phase A 真正要求的是字段归属、访问器退化、旧 app_state 路径删除，以及相关行为不回退；这些当前都已坐实

- **仍有两类失败/漂移存在，但不应继续挂在 Phase A 名下。**
  - `test_tinyui_layout` 的 grid 布局断言失败：
    - 表现为 `test_grid_layout_positions_basic_widgets_like_demo` 中 `text/image` 的纵向相对位置断言失败
    - 更像 layout 行为漂移，不是 app_state 折叠问题
  - `check_tinyui_runtime` 的 demo 编译失败：
    - `tinyui/demo/keyboard_basic/keyboard_basic.c` 仍在使用 `line_edit_props.has_keyboard_binding`
    - 这是 Phase B sentinel API 收口后的 demo 跟进缺失，不是 Phase A blocker

### 0.5 closeout 补记（2026-06-23）

- **此前挂在 closeout 上的 `test_tinyui_layout` 已完成复核，当前不再构成 Phase A blocker。**
  - 本轮处理方式：
    - 先单独重建并运行 `test_tinyui_layout`
    - 对 `test_grid_layout_positions_basic_widgets_like_demo` 做一次临时调试打印，确认 `switch/checkbox/slider/button/text/image` 的 Y 坐标严格递增
    - 临时调试代码已在确认后移除
  - 当前结论：
    - 当前代码树下，`test_tinyui_layout` 通过
    - 先前失败更接近旧二进制/旧构建状态残留，不是 app_state 折叠带来的现行回退

- **Phase A closeout 现在不再保留额外未解 blocker。**
  - 说明：
    - `line_edit` sentinel 相关 demo/runtime 问题已在 Phase B 侧收口
    - layout focused 复核当前已通过，且未发现与 `tinyui_app` 持有 `ld_scene / next_ld_name_id / runtime_state` 的折叠相关的新回退

- **最终验证已覆盖到当前完整 runtime 测试树。**
  - focused / full verification：
    - `rtk ctest --test-dir build/tinyui-runtime -R '^test_tinyui_layout$' --output-on-failure`
    - `rtk ctest --test-dir build/tinyui-runtime --output-on-failure`
  - 当前结果：
    - `build/tinyui-runtime` 共 72/72 通过

## 0. 起始状态核实结论(写 plan 时已逐项 Read 核实,执行前请复核)

> **历史说明（2026-06-23 回看）:** 本节是 2026-06-18 写 plan 时的起始态快照，**不是当前代码真相**。其中凡是声称 `struct tinyui_backend_app_state` / `void *backend_app` / 旧访问器返回类型仍存在的表述，当前都应以本文 `0.4 当前执行快照` 为准。

> **重要纠偏:Phase 0 尚未合入(与任务书"前置假设 Phase 0 已完成"不符)。** 执行本相位前必须确认 Phase 0 已合入;若未合入,见下方两条处置。

逐项核实(2026-06-18,当前 `dev-nanoui` 分支):

- **C 类记账字段仍在** `runtime_internal.h:180-194`(`last_signal`/`last_native_signal`/`dispatch_count`/`data_model_identity`/`data_model_epoch`/`data_truth_policy`/`last_data_source` 等)——Phase 0 应删的字段**全部还在**。这些字段都在 `struct tinyui_backend_widget`(L162-201),**不属于 Phase A 写面**(Phase A 只动 `tinyui_backend_app_state`,L203-210),所以即使 Phase 0 未先做,Phase A 仍可独立完成,但需注意:
- **`last_window_switch_mode` / `last_window_switch_duration_ms` 仍在** `runtime_internal.h:208-209`,**属于 `tinyui_backend_app_state`**,本就是 Phase A 要删的字段(任务书第 2 条)。任务书括注"Phase 0 可能已删"——核实:**未删**,因此本相位 **Task 5 负责删除**它们及其两个 setter 的字段写点。
- `struct tinyui_backend_app_state` 当前 6 字段(`runtime_internal.h:203-210`):`theme`(删,与 `app.theme` 重复)、`ld_scene`(留)、`next_ld_name_id`(留)、`runtime_state`(留)、`last_window_switch_mode`(删)、`last_window_switch_duration_ms`(删)。
- `struct tinyui_app` 当前(`internal.h:329-340`):首字段 `void *backend_app;`,其后 `theme/root_window/focus_owner/editing_owner/timers/display_port/input_port/tick_port/os_port`。
- 3 个访问器签名(`runtime_bridge.h:12/13/16`)当前**返回 `struct tinyui_backend_app_state *`**:
  - `tinyui_runtime_bridge_backend_state(struct tinyui_app *app)`
  - `tinyui_runtime_bridge_backend_state_from_window(struct tinyui_window *window)`
  - `tinyui_runtime_bridge_backend_state_from_parent(void *backend_widget)`
- 另有 2 个相关访问器(`runtime_bridge.h:17/18`):`tinyui_runtime_bridge_scene_from_parent(void *)`(返回 `ld_scene`,**保留,改为内部走新 `_from_parent`**)、`tinyui_runtime_bridge_next_name_id(void *)`(返回 `++next_ld_name_id`,**保留**)。

**处置(二选一,执行前由主线程决策):**
- **(A)推荐**:确保 Phase 0 已合入后再起 Phase A(spec §4 顺序依赖 Phase 0 → A)。
- **(B)若必须先做 A**:本 plan 各 Task 已设计为不依赖 Phase 0 的字段删除即可独立编译通过(Phase A 只动 app_state 那 6 字段;widget 的 C 类字段照旧存在,不影响)。Task 5 主动删 `last_window_switch_*`(本就是 A 的职责)。

### 0.1 gitnexus 索引现状(执行前必读)

写 plan 时 `mcp__gitnexus__list_repos` 仅返回 `edgeio-js` / `cde`,**`LingDongGUI` 未在 MCP 索引内**(尽管 CLAUDE.md 声称已索引)。因此:
- 每个 Task 的 `gitnexus_impact` / `gitnexus_detect_changes` 步骤,执行前**先在终端跑 `npx gitnexus analyze`** 把本仓库索引出来;调用 MCP 时带 `repo: "LingDongGUI"`。
- **若索引仍不可用**:用本 plan 每个 Task 内已给出的 `rg` 命令作为 impact 的等价人工核验(grep 出全部直接引用点即为 blast radius),并在报告里注明"gitnexus 索引缺失,已用 grep 等价核验"。

### 0.2 经核实的全量 blast radius(本相位真实写面,远大于"3 访问器 + 3 裸访问")

> **历史说明（2026-06-23 回看）:** 本节的 blast radius 是基于 2026-06-18 的迁移前状态估算。当前仓库已经跨过这一步，不能再把这里的“待改文件数 / 旧类型命中数”直接当成现状。

> **关键认知:删除 `struct tinyui_backend_app_state` 类型会连锁触碰所有引用该类型名的 translation unit。** 任务书第 3/4 条描述的是"逻辑改动点"(访问器退化 + 3 处裸访问),但 spec §3.2/§4 + 任务书第 2 条要求**删除类型本身**,而几乎每个 widget `.c` 都把局部变量声明为 `struct tinyui_backend_app_state *app_state;`,删类型后这些声明全部要改。实测:

| 写面 | 文件数 | 触碰点 | 说明 |
|---|---|---|---|
| `src/core/runtime_bridge.c` | 1 | ~20 处 | 3 访问器 + `init/shutdown/bind_theme/window_switch` 等,struct 的 `calloc/free`/字段读写都在此 |
| `src/core/runtime_bridge.h` | 1 | 5 行 | fwd-decl `struct tinyui_backend_app_state;`(L9)+ 3 访问器签名(L12/13/16) |
| `src/core/runtime_internal.h` | 1 | L203-210 | 删 `struct tinyui_backend_app_state` 定义 |
| `src/core/internal.h` | 1 | L330 | 删 `void *backend_app;`,新增 3 字段到 `struct tinyui_app` |
| `src/widgets/*.c` | ~31 | ~54 处局部声明 `struct tinyui_backend_app_state *app_state;` + 3 处裸 cast(`window.c:356/732`、`table.c:635`)+ ~389 处 `app_state->ld_scene` | 主体改面 |
| `src/drivers/tinyui_ldgui_disp_adapter.c` | 1 | L38、L132 | 2 处 `struct tinyui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(...)` |
| `port/sdl/step.c` | 1 | L126/135/192/230/269/271/291/360 | 8 处声明 + `app_state->runtime_state`(L132/245/253/368/391)+ `app_state->ld_scene`(L204/212-216/278) |
| `tests/tinyui/unit/*.c` | ~20 | ~60 处 | 见 Task 7 清单 |

**字段访问全量普查(决定迁移可行性,已核实):** 通过 `app_state` 变量触碰的字段**仅 5 种**:
`app_state->ld_scene`(389)、`app_state->runtime_state`(6,全在 step.c)、`app_state->theme`(5:runtime_bridge.c 2 写 + test_tinyui_theme.c 3 读)、`app_state->next_ld_name_id`(2,全在 runtime_bridge.c)、`app_state->last_window_switch_mode/_duration_ms`(各 2,全在 runtime_bridge.c)。
→ 结论:迁移是**纯机械**的——`app_state->ld_scene` ⇒ `app->ld_scene`、`->runtime_state` ⇒ `app->runtime_state`、`->next_ld_name_id` ⇒ `app->next_ld_name_id`;`theme`/`last_window_switch_*` 是被删字段,其访问点一并删除/改写。

### 0.3 迁移策略决策(锁定,执行不得偏离)

> **历史说明（2026-06-23 回看）:** 本节里“访问器退化”与“批量类型替换”的大方向仍可作为历史背景参考，但其中关于 `name_id` 的单点分配假设已经失效，不能直接沿用到当前 closeout 判断。

**访问器返回类型从 `struct tinyui_backend_app_state *` 改为 `struct tinyui_app *`。** 理由:`tinyui_app` 折叠后即持有 `ld_scene/next_ld_name_id/runtime_state`,访问器只需返回 app 自身(对 `_from_parent`/`_from_window` 仍做"经 backend->owner / window->widget.backend_widget 推导出 app"的工作,推导逻辑不变,只是返回 app 而非 app_state)。所有调用方把局部变量类型由 `struct tinyui_backend_app_state *app_state` 改为 `struct tinyui_app *app_state`(**变量名 `app_state` 保留不改**,以缩小 diff;仅类型变),字段访问 `app_state->ld_scene` 原样可用(因为现在 `app_state` 是 `tinyui_app*`,而 `ld_scene` 已是 `tinyui_app` 的字段)。

> 这一策略让 ~389 处 `app_state->ld_scene` **无需逐处改写**(只要把局部变量的*声明类型*换掉 + 把 3 处裸 cast 改成调访问器),极大降低出错面。`runtime_state` 同理。

**name_id 分配时序保护(高风险 #3，历史假设已失效):** 本段原先假设 `++app_state->next_ld_name_id` 是**唯一**自增分配点；该假设与 2026-06-23 当前代码真相不符。当前应以 `0.4 当前执行快照` 为准：`next_ld_name_id` 仍是 app 级单一计数器，但分配路径是 `core/widget.c` helper 与多个 `widgets/*.c` 生产路径并存，而不是单点自增。

---

## Task 1 — 折叠后字段并入 `struct tinyui_app`,删 `void *backend_app`(写面:`internal.h`)

**Files:**
- `tinyui/src/core/internal.h:329-340`(`struct tinyui_app` 定义)

- [ ] 跑 impact:`gitnexus_impact({repo:"LingDongGUI", target:"tinyui_app", direction:"upstream", relationTypes:["ACCESSES"]})`;若索引缺失,改跑 `rg -n 'app->backend_app|->backend_app' tinyui tests` 记录全部直接读写点(预期命中 `runtime_bridge.c` 与 3 处裸 cast)。HIGH/CRITICAL 须先报主线程。
- [ ] 基线绿:`cd build && cmake --build . --target tinyui_core 2>&1 | tail -3`(预期 `[100%] Built target tinyui_core`)。**注意:本 Task 单独改完会编译失败**(`backend_app` 仍被 `runtime_bridge.c` 引用),所以本 Task 不单独跑全量 build,与 Task 2 连续完成后再编译(见 Task 2)。
- [ ] 改 `struct tinyui_app`:删 `void *backend_app;`(L330),在结构体顶部按 spec §3.2 顺序新增三字段:
  ```c
  struct tinyui_app {
      struct ld_scene_t *ld_scene;        /* 原 backend_app.ld_scene */
      uint16_t           next_ld_name_id; /* 原 backend_app.next_ld_name_id */
      void              *runtime_state;   /* 原 backend_app.runtime_state(SDL host per-app)*/
      struct tinyui_theme *theme;
      struct tinyui_window *root_window;
      struct tinyui_widget *focus_owner;
      struct tinyui_widget *editing_owner;
      struct tinyui_app_timer *timers;
      struct tinyui_display_port_state display_port;
      struct tinyui_input_port_state input_port;
      struct tinyui_tick_port_state tick_port;
      struct tinyui_os_port_state os_port;
  };
  ```
- [ ] 确认 `struct ld_scene_t` 在此可见:`internal.h:22` 已 `#include "runtime_internal.h"`,后者 L53 `struct ld_scene_t;` 前向声明 —— 指针字段用前向声明即可,无需完整定义。**核实命令**:`rg -n 'struct ld_scene_t' tinyui/src/core/internal.h tinyui/src/core/runtime_internal.h`(预期至少命中 runtime_internal.h 的 fwd-decl)。
- [ ] (本 Task 不 commit,与 Task 2 合并为一次可编译提交。)

---

## Task 2 — 改写 `runtime_bridge.c`:3 访问器退化 + 删 struct calloc/free + 删 theme/window_switch 字段写点(写面:`runtime_bridge.c` / `runtime_bridge.h`)

**Files:**
- `tinyui/src/core/runtime_bridge.h:9,12-13,16`(fwd-decl + 3 访问器签名)
- `tinyui/src/core/runtime_bridge.c:117-126`(`_from_parent`)、`128-138`(`scene_from_parent`)、`140-150`(`next_name_id`)、`152-163`(`bind_theme`)、`165-193`(`init_app`,含 calloc)、`223-243`(`shutdown_app`,含 free)、`337-360`(`bind_host`)、`414-417`(`has_scene`)、`419-426`(`backend_state`)、`428-438`(`backend_state_from_window`)、`453-477`(`reset/set_window_switch`)

- [ ] 跑 impact(三访问器):`gitnexus_impact` 逐个 `tinyui_runtime_bridge_backend_state` / `_from_parent` / `_from_window`(`direction:"upstream"`);索引缺失则用 0.2 表的 `rg` 清单。预期 d=1 命中 ~31 widget + drivers + step.c + tests —— **HIGH 风险,报主线程后继续**(本相位本就承诺串行单写者承接此面)。
- [ ] **改 `runtime_bridge.h`**:
  - 删 L9 `struct tinyui_backend_app_state;` 前向声明;补 `struct tinyui_app;`(若未声明)。
  - 三访问器返回类型 `struct tinyui_backend_app_state *` → `struct tinyui_app *`:
    - `struct tinyui_app *tinyui_runtime_bridge_backend_state(struct tinyui_app *app);`
    - `struct tinyui_app *tinyui_runtime_bridge_backend_state_from_window(struct tinyui_window *window);`
    - `struct tinyui_app *tinyui_runtime_bridge_backend_state_from_parent(void *backend_widget);`
  - `scene_from_parent` / `next_name_id` 签名**不变**。
- [ ] **改 `runtime_bridge.c` 访问器实现**(函数名/职责不变,函数体 `[执行时定稿]` 方向如下):
  - `backend_state(app)`:`return (app == 0 || app->backend_app == 0) ? 0 : app->backend_app;` ⇒ 改为 **直接返回 app**:`return app;`(L419-426)。判空交给调用方现有 `if (app_state == 0 ...)`(语义等价:原来 `backend_app==NULL` 返回 NULL;现在 app 自身非空就返回,**调用方对 `app_state->ld_scene==0` 的判空仍然兜底**;若担心 `app==NULL` 也要返回 0,则写 `return app;`,调用方传入非 NULL,与原逻辑等价)。`[执行时定稿]`:保留 `if (app == 0) return 0;` 更稳妥。
  - `backend_state_from_window(window)`:逻辑不变(读 `window->widget.backend_widget` → `backend->owner`),末尾 `return tinyui_runtime_bridge_backend_state(backend->owner);` 现返回 `tinyui_app*`(L428-438)。把局部 `const struct tinyui_backend_widget *backend` 保留不动(`backend_widget` 是 widget 层,Phase A 不折叠)。
  - `backend_state_from_parent(backend_widget)`:逻辑不变,`return tinyui_runtime_bridge_backend_state(parent_widget->owner);`(L117-126)。
  - `scene_from_parent`:`app_state = _from_parent(...)` 现为 `tinyui_app*`,`return app_state->ld_scene;` ⇒ `return app_state == 0 ? 0 : app_state->ld_scene;`(字段现在在 app 上,直读)(L128-138)。
  - `next_name_id`:`return ++app_state->next_ld_name_id;` ⇒ 局部 `app_state` 改 `tinyui_app*`,`return ++app_state->next_ld_name_id;`(**自增点不变**)(L140-150)。
- [ ] **改 `init_app`(L165-193)—— 删 struct calloc,字段挪到 app**:
  - 删 `struct tinyui_backend_app_state *app_state;` 与其 `calloc`。
  - 现为 `app_state->ld_scene = calloc(1, sizeof(*app_state->ld_scene));` ⇒ `app->ld_scene = calloc(1, sizeof(*app->ld_scene));`。
  - guard `if (app->backend_app != NULL) return 0;`(L173)⇒ 改幂等判据为 `if (app->ld_scene != NULL) return 0;`(已初始化则跳过)。
  - 删 `app_state->theme = app->theme;`(theme 字段已删,app->theme 本就是真相)。
  - `app_state->next_ld_name_id = 0;` ⇒ `app->next_ld_name_id = 0;`(也可省略,calloc app 时已 0;但 app 是在 `app.c` calloc 的,这里仍显式置 0 保险)。
  - `app_state->ld_scene->bUserAllocated = true;` ⇒ `app->ld_scene->bUserAllocated = true;`(**保留语义**)。
  - 删末尾 `app->backend_app = app_state;`。
  - 失败回滚:原 `free(app_state)` ⇒ 改为 `free(app->ld_scene); app->ld_scene = NULL;`(无独立 app_state 可 free 了)。**核对**:scene calloc 失败时无需 free 任何东西(app 由上层 `app.c` 负责)。
- [ ] **改 `shutdown_app`(L223-243)—— 删 struct free,保留释放序**:
  - 删 `struct tinyui_backend_app_state *app_state;` 取用;`if (app_state == NULL) return;` ⇒ 改为 `if (app->ld_scene == NULL) { tinyui_runtime_host_shutdown_app(app); return; }`(或先 host shutdown 再判)。`[执行时定稿]`:**保持原顺序**——先 `tinyui_runtime_host_shutdown_app(app);` 再处理 scene。
  - **释放序必须为**:`if (app->ld_scene != NULL) { ldGuiDespose(app->ld_scene); } free(app->ld_scene); app->ld_scene = NULL;`(对应原 L237-240 的 `ldGuiDespose` 后 `free`,**顺序不变**)。
  - 删 `free(app_state);` 与 `app->backend_app = NULL;`。
- [ ] **改 `bind_theme`(L152-163)**:删 `app_state` 取用与 `app_state->theme = theme;`;保留 `app->theme = theme;`;判空由 `if (app == 0 || theme == 0)` 承接(原还判 `app_state == 0`,可改判 `app->ld_scene == 0` 或直接去掉该判,因 bind_theme 不依赖 scene —— `[执行时定稿]`:保留 `app == 0 || theme == 0` 即可)。
- [ ] **改 `bind_host`(L337-360)**:局部 `struct tinyui_backend_app_state *app_state = NULL;` ⇒ `struct tinyui_app *app_state = NULL;`;`app_state = tinyui_runtime_bridge_backend_state(backend->owner);` 不变;`app_state->ld_scene` 直读不变。
- [ ] **改 `has_scene`(L414-417)**:`return app != 0 && app->backend_app != 0;` ⇒ `return app != 0 && app->ld_scene != 0;`(判据从"有 backend_app"变为"有 scene",语义等价:原 backend_app 一旦 init 即带 scene)。
- [ ] **改 `reset/set_window_switch`(L453-477)**:这两个函数体仅写 `app_state->last_window_switch_*`(被删字段)。**整体改为空实现或保留壳**:Task 5 决定 `last_window_switch_*` 的最终去向。本 Task 先把局部 `app_state` 类型改为 `tinyui_app*` 并把函数体内对被删字段的写**先注释/暂留**——**实际删除在 Task 5**(避免本 Task 因字段尚在 struct 里而编译错)。`[执行时定稿]`:若 Task 顺序保证 Task 5 紧随,可在本 Task 直接清空函数体(见 Task 5 决策),则 reset/set 变为 `(void)app; (void)mode; ...` 空操作。
- [ ] **编译**(Task 1+2 合并验证):`cd build && cmake --build . --target tinyui_core 2>&1 | tail -8`。**预期此时仍有大量编译错**(widgets/drivers/step.c 仍声明旧类型)—— 这是预期的,Task 3-4 修完才整体绿。**本步只确认 `runtime_bridge.c`/`internal.h` 自身无语法错**:`cd build && cmake --build . --target tinyui_core 2>&1 | rg 'runtime_bridge\.c|internal\.h' | head`(预期这两文件**不**出现在错误里;错误应集中在 widgets/*.c)。
- [ ] (不 commit,继续 Task 3。)

---

## Task 3 — 批量迁移 widgets 局部声明类型 + 3 处裸 cast(写面:`src/widgets/*.c`)

**Files:**（~31 文件,~54 处声明 + 3 处裸 cast;清单见 0.2 表与下方核实命令）
- 3 处裸 cast(必须改成调访问器):`tinyui/src/widgets/window.c:356`、`tinyui/src/widgets/window.c:732`、`tinyui/src/widgets/table.c:635`
- ~54 处局部声明:`tinyui/src/widgets/{animation,arc,background,button,calendar,canvas,checkbox,clock,combo_box,date_time,gauge,graph,icon_slider,image,keyboard,label,line_edit,list,message_box,progress_bar,progress_wheel,qrcode,radial_menu,scroll_selecter,slider,switch,table,text,window}.c`

- [ ] 生成精确改点清单(impact 等价核验):
  ```
  rg -n 'struct tinyui_backend_app_state \*app_state' tinyui/src/widgets
  rg -n '\(struct tinyui_backend_app_state \*\)' tinyui/src/widgets   # 预期仅 window.c:356/732, table.c:635
  ```
- [ ] **批量改局部声明**:把 widgets 内所有 `struct tinyui_backend_app_state *app_state;` 改为 `struct tinyui_app *app_state;`(变量名不变)。**逐文件 Edit**(禁用全仓 sed find-replace —— 项目规则);每改一个文件后该文件内 `app_state->ld_scene` 等访问无需改(类型已是 `tinyui_app*`,`ld_scene` 是其字段)。
  - **核实无遗漏的 `app_state->FIELD` 越界**:改完跑 `rg -n 'app_state->(theme|last_window_switch)' tinyui/src/widgets`(**预期 0 命中** —— widgets 从不碰这俩被删字段;若命中说明有意外耦合,停下报主线程)。
- [ ] **改 `window.c:356`**(在 `tinyui_window_dispose_partial`):
  - 原:`app_state = backend->owner != 0 ? (struct tinyui_backend_app_state *)backend->owner->backend_app : 0;`
  - 改:`app_state = backend->owner != 0 ? tinyui_runtime_bridge_backend_state(backend->owner) : 0;`(局部声明已在上一步改为 `tinyui_app*`)。
- [ ] **改 `window.c:732`**(在 `tinyui_window_create`):
  - 原:`app_state = (struct tinyui_backend_app_state *)app->backend_app;`
  - 改:`app_state = tinyui_runtime_bridge_backend_state(app);`
- [ ] **改 `table.c:635`**(在 `tinyui_tabel_show_keyboard`):
  - 原:`app_state = (struct tinyui_backend_app_state *)backend->owner->backend_app;`
  - 改:`app_state = tinyui_runtime_bridge_backend_state(backend->owner);`
- [ ] **确认头可见性**:用到 `tinyui_runtime_bridge_backend_state*` 的 widget 均已 `#include "runtime_bridge.h"`(经核实,这些访问器原本就是这么调的,头已包含)。逐文件无需新增 include。
- [ ] 编译:`cd build && cmake --build . --target tinyui_core 2>&1 | tail -8`。预期 widgets 错误清零;**残余错误应只在 `drivers/`(Task 4 处理),core 已绿**。命令核验:`cd build && cmake --build . --target tinyui_core 2>&1 | rg 'src/widgets' | head`(预期 0 命中)。
- [ ] (不 commit,继续 Task 4。)

---

## Task 4 — 迁移 drivers(写面:`src/drivers/tinyui_ldgui_disp_adapter.c`)

**Files:**
- `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:38`、`:132`

- [ ] 读这两处上下文确认只读 `ld_scene`:`rg -n -A3 'tinyui_backend_app_state \*app_state' tinyui/src/drivers/tinyui_ldgui_disp_adapter.c`(预期紧接着是 `tinyui_runtime_bridge_backend_state(...)` 调用 + `app_state->ld_scene` 读)。
- [ ] 把两处 `struct tinyui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(...)` 的声明类型改为 `struct tinyui_app *app_state = tinyui_runtime_bridge_backend_state(...)`;`app_state->ld_scene` 直读不变。
- [ ] 编译核心库:`cd build && cmake --build . --target tinyui_core 2>&1 | tail -5`(**预期 `[100%] Built target tinyui_core`** —— core 库整体绿)。
- [ ] (不 commit,继续 Task 5。)

---

## Task 5 — 删被删字段 `theme` / `last_window_switch_*` + 删 struct 定义(写面:`runtime_internal.h` / `runtime_bridge.c`)

**Files:**
- `tinyui/src/core/runtime_internal.h:203-210`(删整个 `struct tinyui_backend_app_state` 定义)
- `tinyui/src/core/runtime_bridge.c:453-477`(`reset/set_window_switch` 函数体最终化)

- [ ] 跑 impact:`gitnexus_impact({repo:"LingDongGUI", target:"tinyui_backend_app_state", direction:"upstream"})`;索引缺失则 `rg -n 'tinyui_backend_app_state' tinyui/src tinyui/port tinyui/include`(**预期此时仅剩 `runtime_internal.h:203` 定义本身 + `port/sdl/step.c` 的 8 处 + 任何尚未迁移项** —— step.c 在 Task 6 处理;若 widgets/drivers 仍命中说明 Task 3/4 漏改,回到对应 Task 修复)。
- [ ] **最终化 `reset_window_switch` / `set_window_switch`**:这两个函数仅写已删字段,删除字段后函数体内对 `last_window_switch_*` 的写必须清除。决策:**保留函数符号(签名不变),函数体改为空操作**(因 `tinyui_app_switch_window` 等仍调用它们,见 `app.c:199/235`):
  ```c
  void tinyui_runtime_bridge_reset_window_switch(struct tinyui_app *app) { (void)app; }
  void tinyui_runtime_bridge_set_window_switch(struct tinyui_app *app, int mode, unsigned int duration_ms)
  { (void)app; (void)mode; (void)duration_ms; }
  ```
  > 这两个 setter 的字段是 0 读死状态(spec §3.2 / 任务书第 2 条:删 `last_window_switch_mode/_duration_ms`)。保留空壳避免触碰 `app.c` 调用点(`app.c` 不在本相位写面收益内,且空壳零行为变更)。若后续 Phase B 要删这两个 bridge 函数与 `app.c` 调用,属 B 的范围。
- [ ] **删 `struct tinyui_backend_app_state` 定义**(`runtime_internal.h:203-210`)整块删除。同时更新该文件顶部注释(L25 "Backend widget tree and app state structs" → 去掉 app state 提及)。
- [ ] 删 `runtime_internal.h` 中可能因此变孤儿的 fwd-decl:核实 `struct ld_scene_t;`(L53)是否仍被 `tinyui_backend_widget` 用到(`ld_event_bridge_scene` 字段,**仍用** —— 保留)。`runtime_state`/`next_ld_name_id` 已搬走,无新增孤儿。
- [ ] 编译核心库:`cd build && cmake --build . --target tinyui_core 2>&1 | tail -5`(预期仍 `[100%] Built`;core 不再引用该类型)。
- [ ] (不 commit,继续 Task 6 处理 port,然后整体跑测。)

---

## Task 6 — 迁移 port/sdl/step.c + name_id 时序验证(写面:`port/sdl/step.c`)

**Files:**
- `tinyui/port/sdl/step.c:126,132,135-137,192,230,245,253,269-271,291,360,368,391`

> **历史说明（2026-06-23 回看）:** 本 Task 内关于“唯一自增点”“`tinyui_runtime_bridge_next_name_id` 调用次序”的若干验收项，已经不适配当前代码真相。若后续继续补文档或 closeout，应改成验证“多路径分配下 `ld_name_id` / `find_by_name_id` 语义未回退”，而不是继续验证旧版单点模型。

- [ ] 读 step.c 这些点确认字段用法:`rg -n 'tinyui_backend_app_state|app_state->(ld_scene|runtime_state)' tinyui/port/sdl/step.c`。确认只触碰 `ld_scene` 与 `runtime_state`(0.2 普查已证实)。
- [ ] **改局部声明**:8 处 `struct tinyui_backend_app_state *app_state;`(含 `static ... *tinyui_runtime_host_app_state_from_window(...)` 返回类型 L135 与 `prepare_runtime_scene(..., struct tinyui_backend_app_state **app_state_out)` 形参 L269/271)全部改为 `struct tinyui_app *`。
  - `tinyui_runtime_host_app_state_from_window`(L135)返回类型 → `struct tinyui_app *`,函数体 `return tinyui_runtime_bridge_backend_state_from_window(window);` 不变(现返回 app)。
  - `tinyui_runtime_host_prepare_runtime_scene` 的 `struct tinyui_backend_app_state **app_state_out` → `struct tinyui_app **app_state_out`;内部 `*app_state_out = app_state;` 不变。
- [ ] **字段访问保持**:`app_state->ld_scene`(L204/212/213/214/215/216/278 等)、`app_state->runtime_state`(L132/245/253/368/391)直读不变(现 `app_state` 是 `tinyui_app*`,字段已在 app 上)。
- [ ] **编译 port + 全库**:`cd build && cmake --build . 2>&1 | tail -8`(预期全部 target 绿,含 `libtinyui_port_sdl.a`)。
- [ ] **【高风险 #3 — name_id 分配时序验证,本段大半已过时】**:
  - **已过时的旧假设**：把 `next_ld_name_id` 当作“仅 1 处自增”的单点模型。
  - **当前更合适的核验方向**：
    - `rtk rg -n 'next_ld_name_id' tinyui/src tinyui/port`，确认当前是“单一计数器、多处分配路径”
    - 抽查 `core/widget.c` 与 `widgets/button.c|window.c|list.c`，确认 helper 路径与生产路径并存
    - `ctest -R 'test_tinyui_(button_events|list|window|table|combo_box|calendar)$' --output-on-failure`，确认当前多路径模型下 `ld_name_id` / `find_by_name_id` 相关子集未回退
  - **当前报告口径**应改为：`name_id` 仍由 `tinyui_app` 持有的单一计数器提供，但当前并未统一收敛为单一 helper；closeout 应验证行为未回退，而不是继续声称“唯一自增点已成立”。
- [ ] (不 commit,继续 Task 7 修测试,再整体回归。)

---

## Task 7 — 迁移测试到新类型 + 调整 app_state 断言(写面:`tests/tinyui/unit/*.c`)

**Files:**(~20 文件,~60 处;凡 `(struct tinyui_backend_app_state *)app->backend_app` 形态均需改)
- `test_tinyui_app_window_switch.c:63,71`、`test_tinyui_button_events.c:386,422`、`test_tinyui_calendar.c:329`、`test_tinyui_clock.c:267,269`、`test_tinyui_combo_box.c:107,124,169,189`、`test_tinyui_date_time.c:122,132,231,244`、`test_tinyui_layout.c:752,753`、`test_tinyui_line_edit.c:78,155,170,201,210,233,250`、`test_tinyui_list.c:183,191,192,270,288,439,460,543,1086`、`test_tinyui_message_box.c:208,224,354,367,439,457`、`test_tinyui_table.c:129,207,224,260,272,307,325`、`test_tinyui_theme.c:462,467,470,474,479,489,495,496,501`、`test_tinyui_widgets.c:2044,2781,2814`、`test_tinyui_window.c:346,349`

- [ ] 生成清单:`rg -n 'tinyui_backend_app_state|->backend_app' tests/tinyui/unit`。
- [ ] **统一改写模式**:测试里 `struct tinyui_backend_app_state *app_state = (struct tinyui_backend_app_state *)app->backend_app;` ⇒ `struct tinyui_app *app_state = app;`(直接用 app);经 `backend->owner->backend_app` 的形态 ⇒ `backend->owner`。`app_state->ld_scene` / `->runtime_state` / `->next_ld_name_id` 直读不变。**变量名 `app_state` 保留**。
  - `ensure_line_edit_msg_queue(struct tinyui_backend_app_state *app_state)`(line_edit.c:78)、`ensure_table_msg_queue(...)`(table.c:129)、`list_app_state(...)`(list.c:183)等辅助函数形参/返回类型同步改为 `struct tinyui_app *`。
- [ ] **`test_tinyui_theme.c` 的 theme 断言改写(L470/474/479)**:这 3 处断言 `app_state->theme == ...`,但 `theme` 字段已删。改为断言 `app->theme == ...`(`tinyui_app.theme` 是唯一真相,`bind_theme` 写的就是它)。核实 `bind_theme` 行为不变:绑定后 `app->theme == theme`。
  - L489/495/496/501 的 `void *saved_backend_app; saved_backend_app = app->backend_app; app->backend_app = NULL; ... app->backend_app = saved_backend_app;` 形态(临时清空以测无 scene 分支):`backend_app` 已删。改为保存/清空/还原 `app->ld_scene`:`struct ld_scene_t *saved = app->ld_scene; app->ld_scene = NULL; ...; app->ld_scene = saved;`(等价:原意是制造"无 backend_app/无 scene"态;现 `has_scene` 判据是 `app->ld_scene != 0`,清空 `ld_scene` 即复现该态)。
- [ ] **`test_tinyui_app_lifecycle.c` 核对**:该文件无 `tinyui_backend_app_state` cast(grep 已确认),其断言是"自二进制无 `tinyui_backend_app_*` 符号"(L257-262/364-366)与 host helper 存在性 —— **与本相位无冲突,无需改**。确认:`rg -n 'tinyui_backend_app_state|->backend_app' tests/tinyui/unit/test_tinyui_app_lifecycle.c`(预期 0 命中)。
- [ ] **`test_tinyui_layout.c:752-753`** `const struct tinyui_backend_app_state *app_state = (const ...)app->backend_app;` ⇒ `const struct tinyui_app *app_state = app;`;后续 `app_state->ld_scene` 直读。
- [ ] 重建测试并跑受影响子集:`cd build && cmake --build . 2>&1 | tail -5 && ctest -R 'test_tinyui_(theme|window|table|layout|list|combo_box|line_edit|message_box|date_time|clock|calendar|button_events|app_window_switch|widgets)$' --output-on-failure`(预期全绿)。
- [ ] (不 commit,进入相位整体验收。)

---

## Task 8 — 相位整体回归 + 提交

**Files:** 无（验证 + commit）

> **历史说明（2026-06-23 回看）:** 本 Task 是原始执行计划的验收尾段，不代表当前仓库仍需要按同一顺序重跑一遍。当前若继续 closeout，应以 `0.4 当前执行快照` 中尚未补强的证据缺口为准。

- [ ] **全量重建**:`cd build && cmake --build . 2>&1 | tail -5`(预期所有 target 绿)。
- [ ] **`test_tinyui_app_lifecycle` 单跑**(spec §4 指定验收):`cd build && ctest -R '^test_tinyui_app_lifecycle$' --output-on-failure`(预期 `Passed`)。
- [ ] **全量 ctest**:`cd build && ctest --output-on-failure 2>&1 | tail -15`(预期 `100% tests passed, 0 tests failed out of 72`)。若有 contract/runtime/perf gate(如 `check_tinyui_backend_mapping` #72)失败,排查是否误删 marker —— 本相位不应触碰 widget backend 类型,gate 应不受影响。
- [ ] **gitnexus_detect_changes**:`gitnexus_detect_changes({repo:"LingDongGUI"})`(索引缺失则跳过并在报告注明);确认变更只命中 `core/{internal.h,runtime_internal.h,runtime_bridge.c,runtime_bridge.h}` + `widgets/*` + `drivers/*` + `port/sdl/step.c` + tests,无意外符号。
- [ ] **SDL demo 一致性**(spec §5 强制:UI 结论须基于真实 LingDongGUI 输出):按仓库现有 SDL demo 运行方式启动至少一个 demo(如 `build/tinyui-runtime/` 下产物),确认能正常起窗、无崩溃,行为与改前一致。**核实命令**:`ls build/tinyui-runtime/ build/examples/ 2>/dev/null`;运行其中 demo 可执行并观察(若有 `TINYUI_AUTO_QUIT_MS`/capture 机制则用之做无人值守截图比对)。在报告记录证据。
- [ ] **commit**(用户/主线程批准后):先确认在 `dev-nanoui`(非 master);`gitnexus_detect_changes` 通过后:
  ```
  git add -A && git commit  # message 见下
  ```
  commit message:
  ```
  refactor(tinyui/core): fold tinyui_backend_app_state into tinyui_app (Phase A)

  - merge ld_scene/next_ld_name_id/runtime_state into struct tinyui_app
  - drop void *backend_app; delete struct tinyui_backend_app_state + its calloc/free
  - delete dup theme field and 0-read last_window_switch_mode/_duration_ms
  - degrade 3 accessors to return tinyui_app* (field direct read)
  - fix 3 bare casts: window.c:356/732, table.c:635
  - preserve name_id alloc order and scene bUserAllocated + ldGuiDespose/free order

  Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
  ```

---

## 相位验收 Checklist（全绿方可收口）

- [ ] **`backend_app` 符号消失**:`rg -n 'backend_app' tinyui/src tinyui/port tinyui/include`(**预期 0 命中**;`tinyui_backend_app_init/run/shutdown` 这类历史符号名只在 `test_tinyui_app_lifecycle.c` 字符串里作"已移除"断言,不在 src —— 单独核 `rg 'backend_app' tinyui/src` 必须为 0)。
- [ ] **`tinyui_backend_app_state` 类型消失**:`rg -n 'tinyui_backend_app_state' tinyui tests`(**预期仅命中 docs/*.md 历史文档**;`tinyui/src`、`tinyui/port`、`tinyui/include`、`tests/tinyui` 全 0)。
- [ ] **`struct tinyui_app` 已含三字段**:`rg -n 'ld_scene|next_ld_name_id|runtime_state' tinyui/src/core/internal.h`(命中 `struct tinyui_app` 内三字段)。
- [ ] **name_id 序不变(高风险 #3)**:唯一自增点 `++app->next_ld_name_id` 在 `runtime_bridge.c`;`rg -n 'next_ld_name_id' tinyui/src` 仅 1 自增 + 1 初始化;Task 6 列出的 name_id 相关 ctest 子集全绿。
- [ ] **scene 释放序保留**:`rg -n -A2 'ldGuiDespose' tinyui/src/core/runtime_bridge.c` 显示 `ldGuiDespose(app->ld_scene)` **后**紧跟 `free(app->ld_scene)`;`bUserAllocated = true` 仍在 `init_app`。
- [ ] **`test_tinyui_app_lifecycle` 绿** + **全量 ctest 绿**:`ctest --output-on-failure` → `100% tests passed ... out of 72`。
- [ ] **SDL demo 一致**:真实 LingDongGUI 输出与改前一致(运行证据已记录,非 smoke/fake 冒充)。
- [ ] **gitnexus_detect_changes** 仅命中预期写面(或注明索引缺失 + grep 等价核验)。

---

## 附:本相位不做（边界，留给后续相位）

- 不动 `struct tinyui_backend_widget`(含 `owner`/`backend_widget` 指针、C 类记账字段)—— 属 Phase 0(C 类清理)/ Phase C（widget 折叠）。本相位 `backend->owner`、`window->widget.backend_widget` 原样保留。
- 不删 `tinyui_runtime_bridge_reset/set_window_switch` 这两个 bridge 函数本身与 `app.c` 调用点（只清空函数体）—— 函数删除属 Phase B(API 表面治理)。
- 不动 `port/` 适配契约本身（display/indev/tick/osal），只改其对 app_state 类型的消费。
- 不重写 perf baseline（`backend_widget_struct_bytes` 等）—— 属 Phase C；本相位删的是 app_state（不在 perf 探针内,核实:`rg -n 'app_state|backend_app_state' tests/tinyui` 的 perf 文件无该指标）。
