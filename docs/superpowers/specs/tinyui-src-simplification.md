# TinyUI src 精简重构 — Spec(权威基准)

> 本 spec 是 `tinyui/docs/tinyui_src_simplification.md`(已经 3 组对抗性核验,见其 §11)的可执行化基准。
> 4 个相位 plan(`docs/superpowers/plans/2026-06-18-tinyui-phase*.md`)必须以本 spec 为唯一权威:**类型名、helper 签名、结构字段、相位边界、验收标准以本文为准**,plan 不得自行更改。

---

## 1. 前提 / 目标 / 非目标

**决策前提**:TinyUI **永不适配其他 GUI 后端**(底层永远是 LingDongGUI + ARM-2D)。

**目标**:在不丢失任何用户控件能力的前提下,把 `tinyui/src` 的三层 widget 模型折叠为单层、消除冗余镜像与测试记账、收敛重复样板与公共 API 表面。保守目标 **削减 4,000–5,000 行(全 src ~15–20%)**,每 widget 省一次 `calloc` + 一个 496–512B 的 backend 结构。

**非目标(本次不做)**:
- 不删除 / 不弱化 **port 适配层**(`display/indev/tick/osal/drivers` + `tinyui/port/`)——它是跨**芯片**能力,与"换 GUI"无关。
- 不建立 `tinyui/src/backend/ldgui/` 私有可换后端层(决策前提下无意义)。
- 不追求"几何/样式影子字段彻底移除"这种深度精简——本次只把 ld 绑定状态并入,影子字段保留(列为可选后续)。
- 不改变任何**用户可见 API 行为**(L3 的删别名属破坏性 API 变更,但语义等价)。

---

## 2. 现状与目标架构

### 2.1 现状(三层并行)
```
tinyui_widget (用户层, internal.h:286-327)  →  .backend_widget (void*)
   tinyui_backend_widget (中间镜像层, runtime_internal.h:162-201)  →  .ld_widget (void*)
      ldBase_t/ldButton_t... (真实对象, src/gui/ld*.c) —— 本身已是一棵树、已存几何/布局/value
```
中间层实测构成:~30% 纯测试记账(生产 0 读)、~50% 对 widget/ld 的纯镜像、~20% 真实 ld 绑定状态。

### 2.2 目标(单层)
删除 `tinyui_backend_widget` / `tinyui_backend_app_state` 两个类型;真实绑定状态并入 `tinyui_widget` / `tinyui_app`;树形交还 `ldBase*` API。

---

## 3. 锁定的共享定义(plan 必须严格沿用,不得改名)

> 这些定义跨相位共享。Phase C 的 plan 负责落地它们;其余相位若引用须用这里的名字。
> 标注 `[执行时定稿]` 的签名允许在 C1 执行时微调形参,但**函数名/职责不变**。

### 3.1 折叠后的 `struct tinyui_widget`(Phase C1 落地)
在现有 `tinyui_widget`(internal.h:286-327,几何/样式/flex/grid/focus/edit 影子字段**保留**)基础上,**新增**以下从 backend 并入的字段,并**删除** `void *backend_widget`:
```c
struct tinyui_widget {
    /* —— 新增:原 tinyui_backend_widget 的真实绑定状态 —— */
    void              *ld_widget;            /* 原 backend.ld_widget */
    uint16_t           ld_name_id;           /* 原 backend.ld_name_id */
    enum tinyui_backend_widget_kind kind;    /* 类型分发 */
    struct tinyui_app *owner;                /* 原 backend.owner,直接持 app */
    struct ld_scene_t *ld_event_bridge_scene;
    void              *ld_event_bridge_sender;
    struct tinyui_widget *ld_event_bridge_next;
    int                value;                /* event 去重 guard(见高风险 #2)*/
    uint16_t           list_item_count;
    enum tinyui_edit_result edit_result_on_finish;
    /* —— 删除 —— void *backend_widget; —— */
    /* —— 既有影子字段 x/y/width/height/.../focus*/edit* 保留不动 —— */
};
```
**删除**(不并入):backend 的 `root/parent/first_child/next_sibling`(改走 ld 树)、`window_layout`/`child_layout`(child 直接删;window 的 padding 见 3.4)、`host_widget`、`id`(改读各子 struct 的 id)、`text/style_class/font/user_data/theme/image_source` 镜像(用 widget 既有或子 struct)、`list_item_ids[16]`(子 struct 已有镜像)、全部 C 类记账字段(Phase 0 已删)、`reserved_*`/`open`。

### 3.2 折叠后的 `struct tinyui_app`(Phase A 落地)
```c
struct tinyui_app {
    struct ld_scene_t *ld_scene;        /* 原 backend_app.ld_scene */
    uint16_t           next_ld_name_id; /* 原 backend_app.next_ld_name_id */
    void              *runtime_state;   /* 原 backend_app.runtime_state */
    /* —— 删除 —— void *backend_app; —— */
    /* theme/root_window/focus_owner/editing_owner/timers/各 port state 保留 —— */
};
```
删除 `tinyui_backend_app_state.theme`(与 app.theme 重复)、`last_window_switch_mode/_duration_ms`(0 读)。保留语义:scene 的 `bUserAllocated` 标记 + shutdown 释放序(`ldGuiDespose` 后 `free(ld_scene)`)。

### 3.3 新增 core 公共 helper(Phase C1 落地;名称锁定,签名 `[执行时定稿]`)
| helper | 职责 | 替换 |
|---|---|---|
| `tinyui_widget_create_leaf(parent, kind, ld_init_cb, ctx, host_size)` `[执行时定稿]` | 单对象分配 + 取 name_id + 调 per-widget `ld_init_cb` + attach 到 ld 树 + bind + 失败回滚 | 29 个 create 生命周期链(~1500 行) |
| `tinyui_widget_destroy_common(w, ld_depose_cb)` | detach + unbind + `ld_depose_cb(ld_widget)` + free 单对象 | 16 个 `*_dispose_partial`(~350 行) |
| `tinyui_rgb_to_ld_color(rgb888)` / `tinyui_ld_color_to_rgb(c)` | 唯一颜色换算 | 14 文件各写一遍(~120 行)+ 修不一致 |
| `tinyui_widget_detach_from_parent(w)` | 从父链表摘节点(改走 ld 树) | 7 个 `finish_detach_after_backend_failure`(~180 行) |
| `tinyui_align_to_arm2d(align)` | 唯一 align 映射 | 4+ 文件各写一遍(~80 行) |
| core 通用 item-list 模型 `[执行时定稿]` | items + 选中索引同步 | list/combo/scroll 三件套(~250 行) |

### 3.4 类型 / 文件搬迁(Phase C1+C3 落地)
- `enum tinyui_backend_widget_kind`、`enum tinyui_edit_result`、`enum tinyui_focus_event`:从 `runtime_internal.h` 迁入 `internal.h`(消除循环依赖后)。
- `tinyui_window` 新增 padding 字段(承接被删的 `window_layout` padding):`padding/padding_left/top/right/bottom/has_explicit_flex_padding/grid_padding_*×4/has_explicit_grid_padding`。
- `runtime_internal.h` 删空后移除;`runtime_bridge.c` 保留并简化(改名为 `ldgui_app_bridge.c` 为可选,不强制)。

### 3.5 backend 文件去向(见 `tinyui_src_simplification.md` §3.1)
`native.c`、`runtime_internal.h` 基本整删;`runtime_bridge.c`/`event.c`/`widget.c` 保留瘦身;`drivers/` 不动。**适配器逻辑(scene 生命周期/事件桥接/ld 绑定/name_id)必须保留——抽象可删,适配器不可删。**

---

## 4. 相位分解(每个相位独立可编译、可回归)

> 顺序依赖:**Phase 0 → Phase A → Phase C**;**Phase B 独立**,可与 0/A 并行或任意穿插。
> 每个相位 plan 的"起始状态"= 其前置相位已合入。

### Phase 0 — L0 零风险死代码清理(风险:极低)
- **范围**:删 C 类 8 字段(`data_model_identity/epoch`、`data_truth_policy`、`last_data_source`、`last_signal`、`last_native_signal/value`、`dispatch_count`)及全部 src 写点 + 全部 tests 读点;删 `runtime_evidence_flags` + `observe.c` 两处死分支;删 `reserved_*`/`open`/`last_window_switch_*`;删 `list.c:557-640` 9 个 `*_ld` 死桩 + 对应单测;删 `native.c:109-128` `signal_to_ld` 表(+ 仅服务 C 类的 readback policy)。
- **写面**:`runtime_internal.h`、`widget.c`、`event.c`、`native.c`、`list.c`、`port/sdl/observe.c` + 约 35 个 `tests/tinyui/unit/*`。`list.c`+其测试、`native.c` 与 C 类字段删除是**独立写面**,可并行子任务。
- **验收**:`rg 'data_model_|dispatch_count|last_signal|last_native_|last_data_source|runtime_evidence_flags' tinyui/src` 无残留;`list.c` 无 `return -1` 死桩;全量 `ctest` 绿(删掉对应断言后);SDL demo 行为与改前一致。

### Phase A — app_state 折叠进 tinyui_app(风险:中)
- **范围**:把 `ld_scene`+`next_ld_name_id`+`runtime_state` 并入 `struct tinyui_app`(§3.2);删 `tinyui_backend_app_state` + 其 `calloc`(runtime_bridge.c:177-191 附近);3 个访问器(`tinyui_runtime_bridge_backend_state`/`_from_window`/`_from_parent`)退化为字段直读;修 3 处裸访问(`window.c:356`、`window.c:732`、`table.c:635`)。
- **写面**:`runtime_internal.h`、`runtime_bridge.c/.h`、`internal.h`(tinyui_app)、`app.c`、`window.c`、`table.c`,外加 **~54 处 widget 局部 `struct tinyui_backend_app_state *` 声明**(实测,远大于初版"3 访问器 + 3 裸访问"估计)。机械策略:访问器返回类型改 `tinyui_app*` + 局部变量只换声明类型保留变量名,使 ~389 处 `app_state->...` 访问无需逐个改写。shared core,**串行单写者**。
- **验收**:`backend_app` 符号消失;name_id 分配序不变(高风险 #3);`test_tinyui_app_lifecycle` + 全量绿。

### Phase B — L3 公共 API 表面治理(风险:中,破坏性 API)
- **范围**:删 7 个 widget 的 `tinyui_*_init` 别名(button/calendar/graph/line_edit/progress_bar/slider/table);删 `obj.h:29-92` 13 个 inline 别名;`app.h` **文件保留**但移出 `tinyui.h:23` 公共导出、统一到 `runtime.h`;window **flex** padding 入口收敛为 2(`set_padding`+1 个 get),**`set_grid_padding` 是独立能力、保留不动**(实测纠正:不是 8→2 全塌);`layout/flex.c`+`grid.c` 标为 facade 或并入 window;删空头 `core.h`/`screen.h`;`slider.h:32-36` `has_*` 改哨兵;**同步更新 `check_tinyui_transition_guards.py` / `check_tinyui_v21_transition_guards.py` 基线**(app.h 导出变更会触发 drift)。
- **写面**:`include/*`(各 widget 头 + obj/core/screen/tinyui/app/window/slider)、对应 widget `.c`(删 _init 定义)、`layout/`、2 个 transition-guard 脚本 + 基线。**多为独立头文件,可并行子任务**(但 _init 删除需同时改 .h + .c 同一 widget,该 widget 为一个写面单元)。
- **验收**:`public API contract` 检查通过;demo 全用 `runtime.h`;transition-guard 绿;`rg 'tinyui_\w+_init\b' tinyui/include` 仅余非别名项。

### Phase C — L1+L2 折叠中间层 + 样板收敛(风险:高)
分三子阶段,**C1 串行 → C2 并行扇出 → C3 串行**:

- **C1(shared core prep,串行,高风险)**:落地 §3.1/§3.3/§3.4——新增 core helper;`tinyui_widget` 并入 ld 绑定字段、删 `backend_widget` 指针;树 getter(`get_parent/first_child/next_sibling/root/child_count`)改走 `ldBase*` API;事件 `pInfo` 改指向 `tinyui_widget`(core + 6 个用 `pInfo` 的 widget,见高风险 #1);value 去重 guard 改用宿主字段(高风险 #2)。写面:`core/*` + `event.c` + 6 个 widget 的 pInfo slot。
- **C2(per-widget 迁移,并行扇出,1 subagent / widget,写面=单 widget 的 .c/.h)**:29 个 widget 各自改为单对象模型、复用 C1 helper、删 `*_dispose_partial`/双层 setter/各自的颜色与 align 与 detach 拷贝、测试快照脚手架移到测试侧。**统一 per-widget 模板见 plan**。各 widget 写面互不重叠 → 真正可并行。
- **C3(final cleanup,串行)**:删 `struct tinyui_backend_widget` 类型 + `runtime_internal.h` 内容搬迁;迁移 `port/sdl/observe.c`(23 处)+`step.c`(14 处)+`theme/theme.c`+`layout/`(它们直接消费 backend 类型);重写 perf 基线(`backend_widget_struct_bytes` 496/512 → 删除或新值);全量回归 + SDL demo 截图比对。
- **验收**:`runtime_internal.h` 不再有 `tinyui_backend_widget`;每 widget 单 calloc/free + 复用 core helper;全量 `ctest` 绿;**SDL demo 真实 LingDongGUI 输出与改前逐页一致(截图比对)**。

---

## 5. 约束与质量门(所有相位强制)

- **每个符号改动前跑 `gitnexus_impact({target, direction:"upstream"})`**,HIGH/CRITICAL 风险须先报告;提交前跑 `gitnexus_detect_changes()`。
- **"UI 完成"必须基于真实 LingDongGUI 输出证据**(SDL demo 运行 + 截图),不得用 smoke/fake 通过冒充。
- `tinyui/demo/*` 只能用 `tinyui_*` API,禁止泄漏 `ld*`/`arm_2d_*`/`SIGNAL_*`。
- 不得用改 demo / 硬编码坐标 / 补假视觉来掩盖 backend/layout 缺口。
- 每相位结束跑:全量 `ctest --test-dir build`、contract gate、runtime gate、perf gate。
- cmake 编译;worktree 合并可忽略主仓库文档自动变更。
- **执行前置(重要)**:gitnexus 索引当前**不含 `LingDongGUI`**(MCP 仅 `edgeio-js`/`cde`),`gitnexus_impact` 暂不可用——执行前先在终端 `npx gitnexus analyze`;否则改用各 plan 内置的 `rg` 等价核验,且不得跳过影响面核对。
- **计数注意**:rtk 代理会吞 grep 匹配,核对计数/扫残留时用 `dangerouslyDisableSandbox=true`(或 `command grep`)。
- **相位顺序**:当前 dev-nanoui 树 Phase 0/A 尚未合入,各 plan 已设硬前置门;**必须按 0 → A → C 顺序执行**(B 独立,可任意穿插)。

---

## 6. 高风险登记册(Phase C 必须逐个判断;详见 `tinyui_src_simplification.md` §5)

1. **事件 `pInfo` 反查**:`runtime_bridge.c:29/51` + **7 个** widget(combo_box/icon_slider/line_edit/radial_menu/table/message_box/**keyboard**——实测比初版多 keyboard)用 `msg.ptSender->pInfo`;`pInfo` 是 ld 原生字段(`ldBase.h:225`)、`ldList.c` 内部也用——改指向前确认不冲突。错改断掉所有 native 事件。
2. **value 去重 guard**:`event.c:408/553/582/611` 的 `backend->value` 改用宿主字段,逐 widget 判断,错改导致重复/丢失 `value_changed`。
3. **name_id 分配时序**:折叠 app_state 时保持 name_id 在 ld_widget 创建前后的分配次序。
4. **双 free**(create 回滚链,非销毁):真正 free 在 `*_dispose_partial`(仅 create 失败路径);折叠后变单 free。button/background/window wrapper 内联字段(`action_info`/`padding_group`)需一并合并。
5. **存储模式归一**:所有 widget 均双 calloc;button/background/window 多套一层 wrapper,先归一再单对象化。

---

## 7. Subagent 执行策略

- **串行 shared-core**(Phase 0 / A / C1 / C3):写面集中在 `core/*`,**不可并行**,单写者顺序执行;review 不通过在同一 subagent 内修复。
- **并行扇出**(Phase B 头文件、Phase C2 的 29 widget):写面互不重叠,可多 subagent 并行;**严禁两个 subagent 写同一文件**。
- 主线程负责:方案决策、`gitnexus_impact`、结果收敛、每相位最终验证(全量回归 + SDL 截图)。
- 每个 widget(C2)或每个独立头(B)= 一个 subagent 任务单元;一个 widget 的 `.c` + `.h` + 其单测视为同一写面,归同一 subagent。

---

## 8. 全局成功度量

| 维度 | 目标 |
|---|---|
| 代码行数 | `tinyui/src` 净减 4,000–5,000 行(改后 `wc -l` 为准) |
| 结构 | `tinyui_backend_widget` / `tinyui_backend_app_state` 类型消失;三层 → 单层 |
| 内存 | 每 widget 省 1 次 calloc;删 496–512B 的 `backend_widget`,但 `tinyui_widget`(wrapper)因并入 B 类字段会从 ~184B 升到 ~240–256B → **净省 ~430B/widget**(perf 基线据实重测,不拍脑袋);app 省 1 次 calloc |
| 测试 | 删约 380 处 backend 内部断言;全量 ctest / contract / runtime / perf gate 绿 |
| 行为 | SDL demo 真实输出逐页与改前一致(截图比对) |
| 能力 | 不丢失任何用户控件能力(27/27 widget-like 覆盖不变) |
