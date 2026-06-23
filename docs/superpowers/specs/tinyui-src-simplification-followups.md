# TinyUI src 精简重构 — 遗留项处理方案

> 配套文档:`tinyui-src-simplification.md`(权威基准 + §9 完成核验)。
> 本文登记 src 精简重构评审中确认、但**不在本次重构范围**的遗留项,逐项给出处理建议、影响面与优先级,供后续排期。
> 状态基线:`b6ee68a` + 2026-06-23 评审修复轮(全量 ctest 72/72 绿)。
> 影响面核验约定:gitnexus 索引当前不含本仓库,改用 `command grep`/`rg`(规避代理吞 grep)+ 全量 ctest 核对,不跳过影响面比对。

---

## 0. 优先级与分类总览

| 优先级 | 含义 | 项 |
|---|---|---|
| **P1 — 尽快处理** | 真实潜在崩溃/越界/泄漏,触发条件虽窄但后果重 | #1 list 项 pInfo 冲突、#2 keyboard 动态布局泄漏 |
| **P2 — 计划内清理** | 设计契约需明确,或有发布质量影响 | #3 公共销毁生命周期契约、#4 产品码内故障注入桩 |
| **P3 — 可接受 / 仅文档化** | 单线程下安全或纯一致性优化,低收益 | #5~#10 |

所有项均为**既有问题或刻意设计**,非本次重构引入(逐项注明)。下文每项含:现象 / 风险 / 根因 / 建议处理 / 影响面与工作量 / 是否需 owner 决策。

---

## P1 — 尽快处理

### #1 list 项放复合控件时的 `pInfo` 类型混淆(越界读)

- **现象**:把 TinyUI 控件作为列表项(`tinyui_list_set_item_widget` → `ldListSetItemWidget`)后,该节点的 `pInfo` 被 `ldList.c:468` 覆写为 `arm_2d_location_t*`(每帧 `ldList_show` 在 `ldList.c:253` 读取);而 `create_leaf` 原本把 `pInfo` 设为 `tinyui_widget*`。
- **风险**:任何树 getter(`get_parent/first_child/...`)或 `tinyui_button_get_pressed_by_name_id`(button.c:774/812)若落到该项节点,会把 4 字节 `ldMalloc` 块当作 `tinyui_widget*` 解引用 → 越界读 / 误判。
- **根因**:`pInfo` 是 ld 原生复用字段,ld 内部(list)与 TinyUI 宿主反查共用同一槽位。**重构前同样存在**(那时 `pInfo` 存 `backend_widget*`),高风险登记册 #1 要求"确认无冲突"的部分未尽。
- **建议处理**(二选一,推荐先 a 后 b):
  - **a(短期,低成本)**:在 `ldListSetItemWidget` 复合项场景文档化"列表项不得为复合 TinyUI 控件",并在所有 `pInfo` 反查读取方加守卫——在 `tinyui_widget` 头部加一个魔数/`kind` 校验位,反查时先校验再 cast(`pInfo` 非空仍不足以判类型)。
  - **b(根治)**:列表项宿主指针不复用 `pInfo`,改用 `ldBase` 上的独立字段或 list 侧的 item→host 映射表;`pInfo` 仅留给 ld list 自身。
- **影响面 / 工作量**:a ≈ 半天(改 `internal.h` + 各反查点,约 3~5 处);b ≈ 1~2 天(动 list 绑定模型 + 回归 list/combo/scroll)。
- **owner 决策**:是否支持"复合控件作列表项"这一能力。若明确不支持 → 走 a;若要支持 → 必须走 b。

### #2 keyboard `set_buttons` 动态布局 teardown 泄漏

- **现象**:`tinyui_keyboard_set_buttons` 分配的 `layout_entries`、各 `entries[i].text`、`native_layout`(keyboard.c:49-95)仅在 **create 回滚** 与 **再次 set_buttons 复用** 时释放;正常存活到关机时不释放。
- **风险**:调用过 `set_buttons` 且存活到 app 关闭的 keyboard 会泄漏这部分堆。keyboard 是唯一持有额外动态堆的控件。
- **根因**:沿用"宿主对象生命周期 = 整场景、公共 API 不单独 free 单控件"的既有模型(见 #3),`tinyui_widget_destroy` / app 关机都不触发 keyboard 专属 depose。**非本次回归**。
- **建议处理**:
  - **短期**:文档化契约"销毁/重置前调用 `tinyui_keyboard_set_buttons(kbd, NULL, 0)` 归还动态布局"。
  - **根治**:与 #3 一并解决——若引入真正的整场景关机宿主回收,则在该回收点调用 keyboard 的动态布局释放。
- **影响面 / 工作量**:短期文档 ≈ 0.5h;根治随 #3。
- **owner 决策**:并入 #3 的生命周期决策。

---

## P2 — 计划内清理

### #3 公共销毁路径不 free 宿主、不 depose ld(生命周期契约需明确)

- **现象**:`tinyui_widget_destroy` 仅 `detach_from_parent` + `unbind_host`,**不 free 宿主、也不 depose ld 对象**;真正的单 free 只在 create 失败回滚(`destroy_common`,高风险登记册 #4)。
- **现状判定**:**与基线 `52f06fa` 行为一致**,非回归;且本次重构把"宿主+backend 双结构泄漏"降为"单结构",严格更优。所有控件宿主与 ld 对象按"整场景生命周期"存活,仅在 `ldGuiDespose` 关场景时由 ld 树回收 ld 对象(宿主 `tinyui_widget` 仍不单独 free)。
- **风险**:若上层在运行期频繁 create/destroy 控件,宿主结构会持续泄漏(ld 对象因仍挂树可在关机回收,宿主不会)。
- **建议处理**(需 owner 拍板生命周期契约):
  - **方案 A(确认现状为契约)**:正式文档化"控件宿主 = 整场景生命周期,运行期不支持单控件销毁回收";`tinyui_widget_destroy` 命名/注释明确其为"从树摘除+解绑",不含释放。**零代码改动**。
  - **方案 B(支持运行期销毁)**:让 `tinyui_widget_destroy` 走 `destroy_common` 路径(detach + 泛型 `ptGuiFunc->depose` + free 宿主),并在 app 关机时遍历回收所有宿主。需逐控件确认 depose 幂等、并解决 keyboard(#2)的动态堆。
- **影响面 / 工作量**:A ≈ 文档;B ≈ 2~3 天(core + 全控件回归 + 泄漏门)。
- **owner 决策**:**核心决策点** —— 是否需要运行期单控件销毁能力。本次重构按"非目标"保留现状(方案 A 的事实状态)。

### #4 产品 `.c` 内的故障注入桩进入发布二进制

- **现象**:`button/text/table/slider/progress_wheel/window` 等保留 `*_test_fail_next_*` / `s_fail_test_id` 文件静态 + 热路径上的判定(如 `slider.c:set_indicator_width` 每次跑 `strcmp`/标志检查)。
- **现状判定**:**刻意保留** —— 单测断言这些符号在 widget 源码内未命名空间地存在(如 `test_tinyui_arc.c:418`、`test_tinyui_progress_bar.c:83`),是约定的故障注入缝,与已正确移到测试侧的"快照脚手架"不同。
- **风险**:发布二进制携带测试专用静态 + 每调用一次的额外分支/`strcmp`,体积与极小热路径开销。
- **建议处理**:用编译期开关包裹 —— `#ifdef TINYUI_TEST_FAULT_INJECTION` 包住桩定义与注入点,单测构建打开该宏,发布构建关闭;桩缺省编译为空。需同步改相关单测的"符号存在"断言为"在测试构建下存在"。
- **影响面 / 工作量**:中等,≈ 1 天(动 6~8 个 widget + 对应单测 + cmake 测试目标加宏)。
- **owner 决策**:是否接受为隔离测试缝引入条件编译(轻微增加构建矩阵复杂度)。

---

## P3 — 可接受 / 仅文档化(低收益清理)

> 以下均单线程下安全或纯一致性优化,建议顺手清理或仅登记,不单独排期。

### #5 demo `animation_basic` 引用 `arm_2d_tile_t` 资产符号
- **现象**:`tinyui/demo/animation_basic/animation_basic.c:22-26` 前置声明并 `extern const arm_2d_tile_t c_tileQuaterArcGRAY8;`,仅作 `(void*)` 图像资产指针。
- **判定**:非渲染 API 调用,`check_tinyui_demo_boundary` 通过;按字面 demo 边界规则属临界。
- **建议**:登记为"外部图像资产引用,可接受";若要彻底闭合边界,提供 `tinyui` 图像资产 typedef/句柄完全隐藏 `arm_2d_*`。低优先。

### #6 `tinyui_align_to_arm2d` 单源不对称
- **现象**:新 helper 仅映射水平轴;垂直轴仍走既有 `tinyui_align_to_ld_vertical`;`label.c:56-66`、`table.c:1038` 各有一份反向(arm_2d→tinyui)映射。
- **建议**:补 `tinyui_align_from_arm2d` + 垂直版,收敛为真正单源。低优先。

### #7 list 项计数双真相源
- **现象**:list/combo/scroll 各自保留 `item_count` 字段,同时又写折叠的 `widget.list_item_count`;core LIST 事件读 `list->item_count`,combo 读 `widget.list_item_count`。
- **风险**:两字段表达同一概念,后续维护有漂移风险(当前同步)。
- **建议**:统一收敛到折叠字段 `widget.list_item_count`,删除各控件镜像。低优先。

### #8 animation 宽高双存储漂移
- **现象**:`animation->width/height` 与 `widget.width/height` 双写;用户后续 `tinyui_widget_set_size` 只改 `widget.*`,致 `animation->*`(被 `show_frame` 读)变陈旧。既有问题,折叠后暴露。
- **建议**:`show_frame` 只读 `widget.*`,删除 animation 镜像。低优先。

### #9 `s_*_depose_scene` 文件静态通道非可重入
- **现象**:各控件在同步 `destroy_common` 前把 scene 塞进共享文件静态(因 `ld_depose_cb` 签名 `void(*)(void*)` 无 ctx)。
- **判定**:当前单线程同步 create/destroy 下安全。
- **建议**:给 core 增 `destroy_common` 的带 `void *ctx` depose-cb 变体,消除全部 `s_*_depose_scene` 静态。中等工作量(动 core + 各控件),低优先。

### #10 个别控件直写 ld 字段绕过 API
- **现象**:`qrcode.c:213-294` 直写 color/ecc 到 ld 结构 +宿主影子;`progress_wheel.c:39-74` 用 `may_alias` 结构清 `bUseDirtyRegions`。无对应 ld setter。
- **风险**:对 ld 字段重排脆弱。既有问题。
- **建议**:若 ld 提供/补充 setter 则改用;否则保留并加注释说明绕过原因。低优先。

---

## 附:本轮已修复项(见主 spec §9.2,此处仅索引)

`ldKeyboardClick` NULL 守卫(Critical)、`create_leaf` 回滚泄漏/幽灵节点、`tinyui_widget_destroy` pInfo 漏清、`combo_box` 重复 `value_changed`、`clock` 跨分配器 malloc/ldFree、`port/sdl` 2 个死函数删除。

## 建议落地顺序

1. **先决策 #3 生命周期契约**(影响 #1/#2 的处理深度)。
2. 按决策落地 **#1**(列表项 pInfo)与 **#2**(keyboard 泄漏)。
3. 视发布要求处理 **#4**(故障注入桩条件编译)。
4. **#5~#10** 作为后续清理,可在相关文件下次改动时顺手收敛。
