# TinyUI src 系统性分析与精简优化意见

## 📑 相关文档索引

> 本文是**分析与优化意见**;可执行的 **Spec 与分阶段 Plan** 见下表(开发时从这里直接跳转)。
> 执行顺序:**Phase 0 → A → C**(Phase B 独立,可任意穿插);执行前置见 Spec §5(gitnexus 索引需先 `npx gitnexus analyze`)。

| 文档 | 链接 | 用途 |
|---|---|---|
| **权威 Spec** | [`docs/superpowers/specs/tinyui-src-simplification.md`](../../docs/superpowers/specs/tinyui-src-simplification.md) | 目标/非目标、锁定的折叠后结构与 6 个 core helper 命名、相位边界、验收、5 条高风险登记册、subagent 执行策略 |
| **Phase 0 Plan** | [`…/plans/2026-06-18-tinyui-phase0-deadcode.md`](../../docs/superpowers/plans/2026-06-18-tinyui-phase0-deadcode.md) | L0 零风险死代码清理(C 类记账字段 / list.c 死桩 / native.c 死表) |
| **Phase A Plan** | [`…-phaseA-appstate-fold.md`](../../docs/superpowers/plans/2026-06-18-tinyui-phaseA-appstate-fold.md) | `tinyui_backend_app_state` 折叠进 `tinyui_app` |
| **Phase B Plan** | [`…-phaseB-api-surface.md`](../../docs/superpowers/plans/2026-06-18-tinyui-phaseB-api-surface.md) | L3 公共 API 表面治理(删别名 / 双启动路径 / padding 收敛) |
| **Phase C Plan** | [`…-phaseC-collapse-backend.md`](../../docs/superpowers/plans/2026-06-18-tinyui-phaseC-collapse-backend.md) | 折叠中间层 + 样板收敛(C1 串行 core → C2 并行 29-widget 扇出 → C3 cleanup) |
| 历史审计 | [`src_adaptation_audit.md`](./src_adaptation_audit.md) | backend 边界审计(部分条目被"永不换 GUI"决策作废,见本文 §8) |
| 分层规则 | [`porting_rules.md`](./porting_rules.md) | `src` / `port` / `demo` 与 backend / port 边界定义 |

---

> 决策前提:**TinyUI 永不适配其他 GUI 后端**(底层永远是 LingDongGUI + ARM-2D)。
> 本文据此评估 `tinyui/src` 的代码膨胀来源,指出哪些"中间层"应当折叠删除,并给出分阶段优化路线。
> 本文为分析与方案文档,不含可直接合入的 patch;实际改动每个符号前须按项目规则跑 `gitnexus_impact`。

---

## 0. 结论先行(TL;DR)

1. **封装本身是必要且方向正确的**:LingDongGUI 是 ID-based、需手动传字体、命令式坐标的 API;LVGL 风格是指针式、声明式。二者有结构性语义鸿沟,必须有一层弥合。ARM-2D 只是纯 2D 图形原语(无 widget/事件/布局概念),所以 TinyUI 不是"第三套渲染器",而是把 LingDongGUI 提升为 LVGL 式 API——这是它的核心价值。

2. **但代码量被结构性放大了约 15–20%,与"封装是否必要"无关。** `tinyui/src` 共 26,118 行,其中 widgets 占 76%(19,784),而真正连接两套底层的绑定层只有 320 行。膨胀来自三个根因:**(A) 一个名为 `tinyui_backend_widget` 的中间镜像层;(B) backend 适配样板被复制进 29 个 widget;(C) 公共 API 表面过宽。**

3. **关键澄清:`tinyui_backend_widget` 这层从来不是一个"可换后端"的抽象。** 它没有 vtable/函数指针表,代码直接硬调 `ldBaseSetX`/`ldSliderSetPercent` 等具体符号。它的实际构成是:
   - **~30% 纯测试记账字段**(`data_model_epoch/identity`、`data_truth_policy`、`last_data_source`、`last_signal`、`last_native_*`、`dispatch_count`)——**生产代码 0 处读取,全部只被单测断言消费**;
   - **~50% 对 `tinyui_widget` / `ldBase_t` 的纯镜像**(自维护的一棵树、两份内嵌布局拷贝、几何/样式/text/font/theme 重复);
   - **~20% 真实 ld 绑定状态**(`ld_widget`、`ld_name_id`、事件桥接、value 归一化、list 项数、edit 结果)。

   因此在"永不换 GUI"前提下,**删除这层不会丢失任何能力**——真实绑定状态搬进 `tinyui_widget` 即可,镜像和记账直接删。

4. **必须区分两种"中间层",只删其一:**
   | 中间层 | 为何存在 | 本次决策下 | 处置 |
   |---|---|---|---|
   | **backend 抽象**(`tinyui_backend_widget` / `tinyui_backend_app_state` / 规划中的 `src/internal bridge/`) | 把 LingDongGUI 当作可替换后端 | 永不换 GUI → **不需要** | **折叠删除** |
   | **port 适配层**(`display/indev/tick/osal/drivers` + `tinyui/port/`) | 同一套 LingDongGUI 跑在不同 MCU / SDL 宿主(跨**芯片**,非跨 GUI) | 仍然需要 | **保留** |

5. **保守估计可消除 4,000–5,000 行(全 src 的 ~15–20%),不损失任何用户控件能力**;另外每个 widget 省一次 `calloc` + 一个 **496–512B** 的 `backend_widget` 结构体(初版误记为 128–192B,实测内存收益更大),并删去约 380 处测试断言负担。

---

## 1. 现状画像

### 1.1 代码分布

| 目录 | 行数 | 占比 | 性质 |
|---|---|---|---|
| `widgets/`(29 文件) | 19,784 | 76% | 膨胀主体 |
| `core/` | 5,042 | 19% | widget 基类 + 运行时桥接 + 事件 |
| `theme/` | 561 | 2% | 真实主题映射,必要 |
| `drivers/`(ldgui/arm2d 真正的绑定层) | 320 | 1.2% | 必要 |
| `layout/` | 198 | 0.8% | 纯转发 facade,无实际逻辑 |
| `display/indev/osal/tick` | 213 | 0.8% | port 契约,必要 |
| **合计** | **26,118** | | |

### 1.2 三层 widget 模型(问题的核心)

```
struct tinyui_widget        (internal.h:286-327, ~40 字段)
   持有 x/y/w/h/text/colors/flex*/grid*/focus*/edit*  ← 用户层
        │ .backend_widget (void*)
        ▼
struct tinyui_backend_widget (runtime_internal.h:162-201, ~36 字段 + 2 份内嵌布局)
   自维护 root/parent/first_child/next_sibling 一棵树
   + 内嵌 window_layout / child_layout 两份完整布局拷贝
   + value/epoch/data_truth_policy/last_data_source 记账
   + ld_widget / ld_name_id / 事件桥接链表
        │ .ld_widget (void*)
        ▼
ldBase_t / ldButton_t ...   (src/gui/ld*.c)
   本身已经是一棵 parent/childList/sibling 树,也存几何/布局/样式/value  ← 真实对象
```

后果(经四个评审子任务交叉确认):
- **两棵结构等价的树并行维护**:`backend_widget` 的 `root/parent/first_child/next_sibling` 与 `ldBase_t` 已有的树重复(`ldBaseGetParent/ChildList/NextSibling` 现成可用)。
- **布局/几何属性存三份**:一次 `set_flex_grow` 同时写 `tinyui_widget`、`backend.child_layout`、`ld`(`widget.c:1207-1209`);grid cell 一次写 12 个字段(另加 1 次 ld 调用)。
- **9 处 getter 双真相回读**:`ld ? ldBaseGetX(ld) : host->x`(`widget.c:1509-1652`)。
- **每个 widget 双堆分配**:`calloc(host) + calloc(backend)`(如 `label.c:154` + `:159`)。

---

## 2. `tinyui_backend_widget` 字段三分类(删除依据)

普查确认每个字段的真实消费方,分三类:

### A 类 — 纯镜像,直接删(信息已在 `tinyui_widget` 或 `ldBase_t`)
| 字段 | 依据 |
|---|---|
| `owner` / `host_widget` | 折叠后 backend 即 widget 自身,反查指针消失 |
| `root/parent/first_child/next_sibling` | `ldBase_t` 即树,公共 getter 改走 `ldBase*` API |
| `window_layout`(内嵌 28B) | `tinyui_window`(internal.h:347-366)已持有 flex/grid/gap/align 全套(缺 padding 10 字段需补)。**注意:不止 padding 需迁移**——`window.c:602-603/626-627` 的 grid 列↔行交叉耦合还回读了 `grid_cols/rows/counts`,只重定向 padding 会改坏 grid 组合(数据已镜像在 `window->grid_*`,可迁但范围更大) |
| `child_layout`(内嵌 ~18B) | **纯写 0 回读**,`tinyui_widget` 已持 flex_grow/grid_col/span/align 全套 |
| `kind/text/style_class/font/user_data/theme/image_source` | 多数已在 `tinyui_widget`;`kind` 保留 1 个 enum 即可 |
| `id` | **不在基类 `tinyui_widget`,而在各子 struct**(`tinyui_label.id` 等);`backend->id` 在 `observe.c` 被读,折叠后改读子 struct 的 id 即可 |
| `reserved_*_padding`(3 个) | 0 写 0 读,纯占位 |
| `open` | 几乎无消费 |

### B 类 — 真实状态,必须搬家(迁入 `tinyui_widget` / `tinyui_app`)
| 字段 | 真实消费 | 搬家落点 |
|---|---|---|
| `ld_widget` | 真实 ld 对象指针,**630 处依赖**,唯一不可替代状态 | `tinyui_widget.ld_widget` |
| `ld_name_id` | name_id 分配 / `find_by_name_id` + 多处 getter 读取(初版"88 处"不可复现,实测 `find_by_name_id` 个位数、`ld_name_id` 数十处) | `tinyui_widget`,或全改 `ldBaseGetNameId` 后删 |
| `ld_event_bridge_scene/sender` | `event.c:117-137` 真实转发原生消息 | `tinyui_widget`(scene 可经 app 推导,sender 即 ld_widget) |
| `ld_event_bridge_next` | 事件桥接链表 next 指针,在 5 处 dispose snapshot 被读(progress_bar/progress_wheel/arc/gauge/table) | 随 scene/sender 一起搬入 `tinyui_widget`(初版漏分类,补此条) |
| `value` | `event.c` value 去重 guard(408/553/582/611) | 复用宿主已有 value(`slider->value`/`checkbox->checked`),逐个判断 |
| `list_item_count` | 边界判断真实读 | `tinyui_widget`(`list_item_ids[16]` 数组可删,宿主已有镜像) |
| `edit_result_on_finish` | `line_edit.c:87-98` / `table.c:253-268` 真实读写 | 复用宿主 `pending/last_edit_result` |

### C 类 — 过度设计,整套删(生产代码 0 读,仅测试消费)
| 字段 | 写/读分布 |
|---|---|
| `data_model_identity` | widget.c 写自增 / 生产仅 `widget.c:231` 1 处自指回绕 guard 读(非外部消费,仍可随字段一并删) / tests 23 读 |
| `data_model_epoch` | src 8 写 / 生产 0 读 / tests 10 读 |
| `data_truth_policy` | widget.c 写 / 生产 0 读 / tests 3 读 |
| `last_data_source` | src 8 写 / 生产 0 读 / tests 6 读 |
| `last_signal` | src 28 写 / 生产 0 读 / tests 28 读 |
| `last_native_signal` / `last_native_value` | src 3+3 写 / 生产 0 读 / tests 13+14 读 |
| `dispatch_count` | src 9 写 / 生产 0 读 / tests 35 读 |
| `runtime_evidence_flags` | **0 处写**,仅 `port/sdl/observe.c` 读(恒为 0,`EXCLUDE_FORMAL_MAPPING` 分支是死代码) |

> C 类是教科书式的"为测试存在的记账层"。配套的 `native.c:109-128` `tinyui_native_signal_to_ld` 翻译表也几乎无人调用(event.c 直接用 `SIGNAL_PRESS` 宏比较)。

### `tinyui_backend_app_state` 折叠
6 个字段:`theme`(与 `tinyui_app.theme` 重复,删)、`ld_scene`(留)、`next_ld_name_id`(留)、`runtime_state`(留,SDL host per-app)、`last_window_switch_mode`/`_duration_ms`(仅写 0 读,删)。
访问面全仓 162 处(此为引用次数),但实际只经 **3 个**访问器函数 `tinyui_runtime_bridge_backend_state` / `_from_window` / `_from_parent`,其中**仅 1 个**入参是 `tinyui_app*`、另两个经 `backend->owner` 推导 app;另有 **3 处裸直接访问绕过访问器**(`window.c:356`、`window.c:732`、`table.c:635`)。**折叠难度中等(非低)**:把 `ld_scene` + `next_ld_name_id` + `runtime_state` 三字段并入 `struct tinyui_app`,3 个访问器退化为字段直读,3 处裸访问需一并改,省一次 `calloc`。唯一须保留的语义是 scene 的 `bUserAllocated` 标记与 shutdown 释放序。

---

## 3. 目标架构:折叠为单层

```
struct tinyui_widget {            ← 用户层 = 唯一层
    void *ld_widget;              ← 直接持有 ld 对象(原 backend.ld_widget)
    uint16_t ld_name_id;          ← 原 backend.ld_name_id
    enum kind;                    ← 类型分发
    /* 事件/编辑/list 真实状态(B 类搬入) */
    void *ld_event_bridge_scene/sender;
    uint16_t list_item_count;
    enum edit_result ...;
    /* 用户可见属性:几何/样式/布局/focus 以 ld 为真相,
       未绑定 ld 前的影子缓存按需保留(可进一步精简) */
    ...
    /* 控件回调(正当的函数指针:on_clicked / on_value_changed ...) */
};

struct tinyui_app {
    struct ld_scene_t *ld_scene;  ← 原 backend_app.ld_scene
    uint16_t next_ld_name_id;     ← 原 backend_app.next_ld_name_id
    void *runtime_state;          ← SDL host per-app
    ... (theme/root_window/timers/port states 不变)
};
```

要点:
- **删除 `struct tinyui_backend_widget` 与 `tinyui_backend_app_state` 两个类型。**
- widget 树形交还给 ld:公共 `get_parent/first_child/next_sibling/root/child_count` 改走现成 `ldBase*` API。
- 各 widget 私有 `get_ld()` 由 `((backend*)w->backend_widget)->ld_widget` 改为 `w->ld_widget`(**232 处,模式一致,可批量改**)。
- widgets/core/theme **直接调用 `ld*`** 不再视为问题(无 backend 中立性约束)。`drivers/` 与 `port/` 的**适配职责**(display/indev/osal/tick/flush)不变,但 `port/sdl/observe.c`+`step.c` 目前直接消费 `tinyui_backend_widget` 类型(读 `kind/ld_widget`/遍历树),删类型时这两文件须同步迁移——即"port 的契约不变,但它作为 backend 类型的结构消费者要跟着改"。

### 3.1 backend 相关文件的逐个去向(澄清:折叠 ≠ 删文件)

> 常见误解:"折叠中间层 = 把 backend 相关文件都删掉"。**不是。** 折叠删的是冗余的**镜像 struct**(`tinyui_backend_widget`/`tinyui_backend_app_state`)+ 测试记账;但这些文件里还装着**真实不可省的适配器**——`ld_scene` 生命周期、事件桥接(`ldMsg` 的 `SIGNAL_*` → 回调)、`ld_widget` 绑定、`ld_name_id` 分配。删了这些就没有 UI。所以适配逻辑保留(变小/改名/合并),只有纯镜像和记账被删。**一句话:抽象可删,适配器不可删。**

| 文件 | 行数 | 折叠后去向 |
|---|---|---|
| `core/runtime_internal.h` | 237 | 两个 struct 删除;`kind` enum / layout cache / app 生命周期声明并入 `internal.h` → **文件本身可消失** |
| `core/native.c` | 147 | **基本整删**(`signal_to_ld` 近死代码、readback policy 仅服务 C 类记账) |
| `core/runtime_bridge.c` | 477 | **保留 + 简化**:真实 app↔`ld_scene` 桥、事件 connect、name_id 分配;访问器退化为字段直读 |
| `core/runtime_bridge.h` | 42 | 随之简化 / 合并 |
| `core/event.c` | 682 | **保留 + 简化**:`SIGNAL_*`→回调翻译是核心价值;删 C 类记账、合并 4 个重复 dispatch case |
| `core/widget.c` | 2101 | **保留 + 大幅瘦身**:setter/getter 是公共 API 本体;删并行树、三写、双真相 getter |
| `core/runtime.c` | 65 | 可并入 `app.c` |
| `core/app.c` | 392 | **保留**(timer 链表 + app 生命周期基础设施) |
| `core/resource.c` | 100 | **保留**(image/font 从 vres 加载的薄封装) |
| `drivers/`(`tinyui_ldgui_port.c` 等) | 320 | **原样保留**——跨芯片 port 绑定(显示/触摸/tick 符号路由),与"换 GUI"无关 |

**结论**:能整文件删掉的只有 `native.c`(基本)与 `runtime_internal.h`(作为独立文件,内容迁走);其余 backend 文件是真实适配器,**只会变小、不会消失**;`drivers/` 完全不动。TinyUI 永远需要一个对接 LingDongGUI 的适配器,折叠只是让它不再额外维护一棵镜像树。

---

## 4. 优化项分级清单

### L0 — 零风险即删(不改变任何运行逻辑)
| 项 | 位置 | 收益 |
|---|---|---|
| C 类 8 个记账字段 + 全部 src 写点 + 全部 tests 读点 | `runtime_internal.h` + widget.c/event.c + 35 个测试 | backend 结构缩 ~30%,删约 200+ 处测试断言 |
| `reserved_*` / `open` / `last_window_switch_*` 字段 | runtime_internal.h | 纯占位 |
| `observe.c` 中 `runtime_evidence_flags` 死分支 | `port/sdl/observe.c:118/226` | 删死代码 |
| `list.c` 9 个 `*_ld` 死桩(全 `return -1`) | `list.c:557-640` | 84 行死代码 + 对应测试 |
| `native.c` 未调用的 `signal_to_ld` 翻译表 | `native.c:109-128` | 死抽象 |

### L1 — widgets 样板收敛(抽 core 公共能力,约 3,500–4,000 行)
| 重复样板 | 范围 | 可消除 |
|---|---|---|
| create 生命周期链(calloc→`ldXxx_init`→attach→bind→回滚 free),仅 `ldXxx_init` 参数不同 | 29 文件 | **~1,500 行** → 抽 `tinyui_widget_create_leaf(parent, kind, ld_init_cb,...)` |
| backend/public 双层 setter(内层调 ld,外层只回写影子字段) | combo_box/scroll_selecter/arc/gauge/icon_slider/radial_menu/text/line_edit | **~900 行** → 合并直接调 ld |
| `_dispose_partial` 销毁链(与 core `tinyui_widget_destroy` 重叠,差一行 `ldXxx_depose`) | 16 文件 | **~350 行** → 带 depose 回调的通用析构 |
| 测试快照脚手架塞进产品文件 | arc/gauge/progress_wheel/table/progress_bar | **~250 行** → 移到测试侧 |
| list/combo/scroll item 三件套 | 3 文件 | **~250 行** → core 通用 item 模型 |
| `finish_detach_after_backend_failure` 逐字复制 | 7 文件 | **~180 行** |
| `rgb_to_ld_color`/`ld_color_to_rgb`(3 种不一致实现) | 14 文件 | **~120 行** + 修颜色不一致 bug |
| `tinyui_align`↔`arm_2d_align_t` 映射各写一遍 | 4+ 文件 | ~80 行 |

> L1 与 L2 应**合并为同一轮"逐 widget 重写"**:折叠中间层时本就要改每个 widget 的 create/dispose,顺手抽公共函数,避免每个文件被改两遍。

### L2 — 折叠中间层(单层模型,本次决策的核心目标)
- 删 `tinyui_backend_widget` / `tinyui_backend_app_state`,B 类状态搬入 `tinyui_widget` / `tinyui_app`(见 §3)。
- 删 backend 自维护树(~240 行),getter 改走 ld 树。
- 删 `window_layout`/`child_layout` 两份内嵌布局拷贝,window.c 改读 `tinyui_window` 自有字段。
- 25 个 widget 双 `calloc`/双 `free` 改单对象。

### L3 — 公共 API 表面治理(可独立进行,低风险但属破坏性变更)
| 项 | 位置 |
|---|---|
| 删 `tinyui_*_init` 别名系列(纯转发 `_create`,实测仅 7 个 widget 有:button/calendar/graph/line_edit/progress_bar/slider/table) | 对应 widget 头 |
| 删 `obj.h` 13 个 `tinyui_obj_*` inline 别名层(与 `widget_*` 重复) | `obj.h:29-92` |
| `app.h` 移出公共导出(自标 NON-CANONICAL 却仍被 `tinyui.h:23` 导出),统一到 `runtime.h` | `app.h` / `tinyui.h` |
| window padding 入口(`window.h:51-86` 内约 8 个)收敛为 2 个 | `window.h:51-86` |
| `layout/flex.c`+`grid.c`(198 行纯转发)合并进 window 或明确标为 facade | `layout/` |
| 删空转发头 `core.h` / `screen.h` | include/ |
| `slider_props` 暴露的 `has_*` 内部标志改哨兵值 | `slider.h:32-36` |

---

## 5. 高风险点(L2 必须逐个判断,按风险排序)

1. **事件桥接经 `ldBase_t::pInfo` 反查**:`runtime_bridge.c:29/51` 把 `((ldBase_t*)sender)->pInfo` 指向 backend;折叠后必须改指向 `tinyui_widget`,错改会断掉 native 事件。**注意**:不止 core 一处——`combo_box/icon_slider/line_edit/radial_menu/table/message_box` 共 6 个 widget 也各自用 `msg.ptSender->pInfo` 接消息(与审计 #12 一致),都要同步改;且 `pInfo` 是 LingDongGUI 原生字段(`ldBase.h:225`)、`ldList.c` 内部也用它,改写指向前须确认不与 ld 自身用途冲突。
2. **event.c 的 value 去重 guard**:`backend->value` 是与宿主字段并行的第二真值。删它需逐 widget 决定改用宿主字段做 guard,错改导致重复/丢失 `value_changed` 回调。
3. **name_id 分配时序**:折叠 app_state 时须保证 name_id 在 ld_widget 创建前后的分配次序不变,否则 `find_by_name_id` / `ldBaseGetNameId` 不一致。
4. **双 free → 单 free(位置在 create 回滚链,非销毁)**:运行期销毁 `tinyui_widget_destroy`(`widget.c:1466`)本身**不 free**(只 detach+unbind+置空);真正分别 `free(backend)+free(host)` 的是各 widget 的 `*_dispose_partial`,且仅在 **create 失败回滚路径**被调用。折叠为单对象后这些回滚函数大多**变简单**(一次 free)。**漏评成本**:button/background/window 的外层 wrapper struct 内联了附加字段(button 的 `action_info`、window 的 `padding_group`),单对象化需把这些字段一并合并。
5. **存储模式不统一(措辞澄清)**:并非"内嵌 vs 独立 calloc"的割裂——**所有 widget 都是 host + backend 两次独立 `calloc`**;真实差异仅是 button/background/window 把 `tinyui_backend_widget` 作为成员嵌进一个外层 wrapper struct(为携带附加字段),其余 22 个是裸 backend。折叠前需把这两种存储形态归一。

---

## 6. 测试与契约的同步改动

- **单测**:约 803 处 backend 内部字段断言。其中 **~380 处随中间层一起删**(dispatch_count/epoch/identity/last_signal/dispose snapshot/树结构断言);**~20–30 处改写保留**(测真实用户能力的:`_ld` 错误边界返回 -1、value/checked 读回、host 绑定恒等)。
- **perf baseline 必改**:`tinyui_perf_baseline.json` 锁定了 `backend_widget_struct_bytes`(**baseline 496 / max 512B**)与独立的 `widget_wrapper_struct_bytes`(184/192B);`check_tinyui_object_overhead.py` 里的 128/192 只是 `_run_self_test()` 桩值,**不是真实 baseline**(初版把二者混了)。探针 `test_tinyui_wrapper_struct_overhead.c:11` 须随结构删除重写——删掉 496B 的 backend 结构是实打实的内存收益。
- **契约脚本**:9 个 contract 脚本对 runtime **历史字段符号**(dispatch_count 等)引用=0,这一面不受影响。但 **`check_tinyui_transition_guards.py` / `check_tinyui_v21_transition_guards.py` 对 `app.h`/runtime 目录有结构依赖**——§4 L3"移除 `app.h` 公共导出"会触发这两个脚本的 baseline drift 失败,需同步更新其基线。另有 3 个 runtime marker 校验需微调(`runtime/visible_ui`):`EXCLUDE_FORMAL_MAPPING` marker 源于死字段可直接删,`SMOKE_LAYOUT_USED` 源于 `host_internal.h` 与 backend 无关、可独立保留。

---

## 7. 实施路线与验收

| 阶段 | 内容 | 风险 | 验收 |
|---|---|---|---|
| **第 1 步** | L0 全部 + L3 中无依赖项(删别名/空头) | 极低 | 编译通过;`grep data_model_/dispatch_count/last_signal tinyui/src` 仅余必要项;现有 runtime/contract 测试绿 |
| **第 2 步** | app_state 折叠进 `tinyui_app` | 低 | `backend_app` 间接层消失;`test_tinyui_app_lifecycle` 绿 |
| **第 3 步** | L1+L2 合并,**逐 widget 重写**(每改一个先跑 `gitnexus_impact`,改完跑该 widget 单测) | 高 | 每个 widget:单层模型、单 calloc/free、复用 core 公共 create/dispose;SDL demo 真实输出与改前一致 |
| **第 4 步** | 删 `tinyui_backend_widget` 类型定义 + perf baseline 重写 + 树 getter 改走 ld。**前置不止 29 个 widget,还含 `theme/theme.c`、`layout/flex.c`+`grid.c`,以及 `port/sdl/observe.c`(23 处)+`step.c`(14 处)**——后两者直接读 backend 的 `kind/ld_widget`/树遍历,必须随类型删除一并迁移 | 中 | `runtime_internal.h` 不再出现 backend_widget;全量测试绿;SDL demo 截图比对通过 |
| **第 5 步** | L3 剩余 API 治理(padding/layout facade/启动路径统一) | 中(破坏性) | public API contract 检查通过;demo 全迁 `runtime.h` |

每步独立可合入、可回归,避免一次性大爆炸。

---

## 8. 与现有 `src_adaptation_audit.md` 的关系(重要)

现有审计是以"保持 backend 可替换 / 跨芯片中立"为基准写的,共 26 条。**"永不换 GUI"的决策会改变其中相当一部分条目的性质**:

| 审计项 | 原意图 | 本决策下 |
|---|---|---|
| #3 core 混入 ld scene 生命周期 | 边界违规 | **作废**:core/app 直接持有 `ld_scene` 反而是简化目标(§2 app 折叠) |
| #5 widgets 是 ld adapter / #10 runtime_internal.h 含 ld 字段 / #11 theme 写 ld | backend 中立性违规 | **边界顾虑作废**;但 #5 的**重复样板**问题仍成立(→ L1) |
| #12 事件桥接分散 | 边界 + 一致性 | 边界作废;**一致性顾虑仍成立**(统一到 core,非另建 backend 层) |
| #20 `drivers` vs `internal bridge` 命名不一致 / Wave 2 建立 `src/internal bridge/` | 建私有可换后端层 | **可换后端动机作废**,直接折叠更省(L2)。但 `porting_rules.md:12` 把 `internal bridge` 定义为"唯一私有桥接层",其"把 ld glue 收敛到单一目录"的**组织性价值独立于可换性仍成立**(审计 #5/#10/#11 想要的"ld 代码不散落");可保留为可选治理项 |
| #17/#18/#19 cmake/port 边界、SDL 链接、port 可配置 | 跨**芯片**/宿主边界 | **仍然成立**(属 port 层,非 backend 抽象) |
| Wave 1(SDL host 移出 core) | 跨芯片边界 | **已完成且正确,保留** |

**净效果:本决策让重构比团队现有计划更简单**——不必投入精力去建立一个"干净的可换后端私有层"(Wave 2/3 的 internal bridge),而是直接把那层折叠掉。需要继续坚持的只剩"跨芯片 port 边界"(display/indev/tick/osal/drivers + port/)和"public API 不泄漏 arm_2d/ld 类型"这两条。

---

## 9. 量化收益汇总

| 维度 | 收益 |
|---|---|
| 代码行数 | 保守可消除 **4,000–5,000 行(~15–20%)**,不损失任何用户控件能力 |
| 每对象内存 | 省一次 `calloc` + 一个 **496–512B** `backend_widget` 结构 / 每 widget;app 省一次 `calloc` |
| backend 结构 | 字段从 ~36 个 + 2 份内嵌布局 缩到 ~7 个真实字段(其余并入 widget) |
| 测试负担 | 删约 380 处 backend 内部断言,测试聚焦真实用户能力 |
| 架构 | 三层模型 → 单层;消除两棵并行树、三份属性、9 处双真相 getter |
| 认知 | API 表面收敛(去三套创建命名 / 双启动路径 / 别名层) |

---

## 10. 一句话回答你的问题

> **"是否不需要 backend 相关的中间层?"**

是。`tinyui_backend_widget` / `tinyui_backend_app_state` 这层 backend **抽象**应整体折叠删除——它本就不是真正的可换后端机制,而是镜像 + 测试记账,删它零能力损失。
但要**保留** `display/indev/tick/osal/drivers + port/` 这套 **port 适配层**:它解决的是"同一套 LingDongGUI 跑在不同芯片/宿主",与"换 GUI"无关,是 TinyUI 跨芯片能力的根基。

---

## 11. 评审核验记录

本文结论经 3 个独立对抗性子任务 + 主线程抽样,逐条对照真实源码复核。

**核心结论全部通过核验,可据此排期**:
- 封装必要性论证(`ldButton_init` 为 ID 入参、`ldLabel_init` 需传 `arm_2d_font_t*`、ARM-2D 头文件无 widget/event/layout 概念、core 无 vtable 直调 ld*)——4/4 源码坐实。
- 字段三分类**安全性成立**:C 类 9 字段生产代码 0 读(`runtime_evidence_flags` 甚至 0 写、双死分支),B 类 6 字段确有真实生产读取,A 类镜像信息确在 `tinyui_widget`/`ldBase_t`/`tinyui_app` 中有等价物且 ld 树 API(`ldBaseGetParent/ChildList/NextSibling/ChildCount`)现成存在。**无任何被误判可删却实际生产在用的字段。**
- 与审计的关系("backend 可换"前提被"永不换 GUI"推翻)成立;`backend 抽象删 / port 层留` 的区分与 `porting_rules.md` 分层一致。
- L0 死代码清单(C 类字段、`list.c` 9 个 `return -1` 死桩、observe.c evidence_flags 死分支、`signal_to_ld` 零调用)全部坐实。

**初版已修正的硬伤(本文已订正)**:
1. perf 基线 `backend_widget` 实测 **496/512B**(非 128/192B;128/192 是 `.py` 自测桩 / `widget_wrapper` 指标)——内存收益被低估。
2. `find_by_name_id "88 处"` 不可复现,已去除假数字。
3. §5「双 free」实际在 create 回滚链(`*_dispose_partial`),`tinyui_widget_destroy` 本身不 free;补 wrapper 内联字段合并成本。
4. §5「存储模式」措辞夸大:所有 widget 均双 calloc,仅 3 个多套一层 wrapper。
5. 「port 保留不变」与「删 backend 类型」矛盾:`observe.c`+`step.c` 是 backend 类型的结构消费者,§7 第 4 步前置已补 theme/layout/port。
6. `app_state` 折叠难度由「低」修正为「中」(3 访问器 + 3 处裸访问);`window_layout` 迁移不止 padding(含 grid 交叉耦合回读);补漏 `ld_event_bridge_next`(B 类);`id` 不在基类。
7. off-by-N 已订正:`obj.h` 13、padding 入口 8、grid cell 写 12 字段、`_init` 仅 7 widget、backend 非布局字段 ~36。

**口径说明**:正文符号计数(`ld_widget`≈630、`get_ld(`≈232、`backend_app`=162 等)统计范围为 `tinyui/src + tinyui/port + tinyui/include + tests/tinyui`;若只算 `tinyui/src`,`ld_widget` 引用约 283。L1 各项可消除行数为估算,最终以实改后 `wc` 为准。
