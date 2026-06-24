# TinyUI 运行期控件生命周期 — 设计(方案 B2)

> 配套:`tinyui-src-simplification.md`(精简重构基准)、`tinyui-src-simplification-followups.md`(遗留项 #1/#2/#3/#9)。
> 本设计**取代** followups 中 #1(list 项 pInfo 冲突)、#2(keyboard teardown 泄漏)、#3(销毁生命周期契约)、#9(`s_*_depose_scene` 静态)的处理方案,统一为一套实现。
> 状态基线:`f00622a`(评审修复轮后,全量 ctest 72/72 绿)。

---

## 1. 目标 / 非目标

**目标**:让 TinyUI 支持运行期**单控件**与**子树(子页面/子窗口)**的动态创建与销毁,且反复 churn 与关机后均无泄漏。

**核心原则(决定一切)**:TinyUI 是 LingDongGUI 的**封装,不新造生命周期**。运行期销毁能力在 LingDongGUI `src/gui` 已存在,本设计只是把它**封装出来**,并修正 TinyUI 当前唯一"与 LingDongGUI 争用字段"的地方(`pInfo`)。

**非目标**:
- 不改 LingDongGUI 控件的渲染 / 事件 / 布局逻辑。对 `src/gui` 的唯一改动是**把已有的 `static ldGuiDisposeNodeTree` 暴露成 public**(纯暴露,不改逻辑)。
- 不引入多线程安全(维持现状单线程同步 create/destroy)。
- 不做控件影子字段的进一步精简。

---

## 2. LingDongGUI 已提供的生命周期能力(封装依据,实测 src/gui)

| 能力 | API | 语义 |
|---|---|---|
| 单控件销毁 | `widget->ptGuiFunc->depose(scene, widget)` | 断消息 `ldMsgDelConnect` + 摘树 `ldBaseNodeRemove` + `ldFree` 自有资源 + **`ldFree(widget)`**。func 表通用,任意控件可调 |
| 子树递归销毁 | `ldGuiDisposeNodeTree(scene, w)` | 深度优先销毁 w + 全部子孙。当前 `static`,需暴露 |
| 整场景销毁 | `ldGuiDespose(scene)` | 从 root 递归 + 消息队列 deinit |
| 按 id 查节点 | `ldBaseGetWidget(root, nameId)` | 每节点唯一 `nameId` |
| 节点反查通道 | `ldBase.pInfo`(唯一自由指针槽) | **LingDongGUI 自己用**:`ldList` 存列表项坐标(`ldList.c:468/253`) |

**结论**:`ptGuiFunc->depose` 就是运行期销毁;`ldGuiDisposeNodeTree` 就是子树销毁。TinyUI 要做的只是**封装它们 + 释放自己那段宿主内存**。

---

## 3. 根因:TinyUI 抢占了 LingDongGUI 的 `pInfo`

`ldBase_t` 只有一个自由指针 `pInfo`,**无独立 user-data 槽**。TinyUI 当前把 `pInfo` 当"ld 节点 → 宿主"反查指针(45 处 / 13 文件);而 LingDongGUI 自己也用 `pInfo` 存列表项坐标。复合控件作列表项时二者争用同一槽 → 越界读(followups #1)。

**修正**:宿主反查改走 TinyUI 自有的 **`nameId → host` 注册表**,`pInfo` 完全归还 LingDongGUI。

---

## 4. 架构(6 组件)

### 4.1 宿主注册表(挂 `struct tinyui_app`)
活宿主的**侵入式双向链表**。`struct tinyui_widget` 新增 `reg_prev` / `reg_next`;`tinyui_app` 新增 `host_list_head`。
- `register(app, w)`:头插。O(1)。create_leaf 调。
- `unregister(app, w)`:摘链。O(1)。destroy 调。
- `lookup(app, nameId) → host`:遍历链表匹配 `ld_name_id`。**O(n)**。事件桥 + 树 getter 用。
- `iterate(app)`:遍历(关机回收)。
- **选型理由**:只挂活宿主 → **注册表内存**随活控件数有界、无数组扩容。代价是 lookup O(n)。(注:这里"有界"指注册表内存;`nameId` **空间**的有界另需复用,见 §6。)
- **升级路径**:若 profiling 显示 lookup 成热点,可在不改调用方的前提下把内部换成 nameId 索引数组或开放寻址哈希(`lookup`/`register`/`unregister` 接口不变)。

### 4.2 反查 helper `tinyui_widget_from_ld(const ldBase_t *node) → struct tinyui_widget *`
`node → node->nameId → 注册表 lookup → host`。统一替换现有约 45 处 `((struct tinyui_widget*)node->pInfo)` cast。
- create_leaf / `bind_leaf_widget` **不再写 `pInfo`**。
- 涉及 7 个事件桥 widget(combo_box / icon_slider / line_edit / radial_menu / table / message_box / keyboard)+ core 树 getter(`get_parent/first_child/next_sibling/root/child_count`)+ `runtime_bridge` 事件 slot。

### 4.3 `tinyui_widget_create_leaf` — 注册而非写 pInfo
分配 host → ld_init → 挂树 → `register(owner, w)`(替代 `pInfo = w`)。
回滚(bind 失败):`unregister` → `ptGuiFunc->depose(scene, ld)` → `free(host)`(沿用本轮已修的回滚形态,把 pInfo 清理换成 unregister)。

### 4.4 单控件 `tinyui_widget_destroy`(叶子)— 封装 depose
1. 校验(非 window-root、ld 存在)。
2. 若持 focus/editing → 释放。
3. `host_cleanup(w)` 钩子(见 4.6,此时 ld 仍在)。
4. `unregister(owner, w)`。
5. `w->ptGuiFunc->depose(owner->ld_scene, w->ld_widget)`(LingDongGUI 断消息 + 摘树 + free ld)。
6. `free(w)`。

> host 与 ld 是两段独立内存,host free 不依赖 ld。步骤 5/6 顺序:先 depose ld 再 free host(host 在 depose 期间不被 LingDongGUI 触碰,反查已 unregister)。

### 4.5 容器 / 子树 `tinyui_widget_destroy`(window 等带子节点)— 封装 LingDongGUI 递归
把 `ldGuiDisposeNodeTree` 暴露为 public(`ldGui.h`)。流程:
1. **遍历 ld 子树**(用 ldBase getter):对每个节点 `lookup(nameId)`;查到宿主 → `host_cleanup` + `unregister` + `free(host)`(**此时 ld 仍在**);查不到(纯 ld 节点 / list 项坐标节点)→ 不碰。
2. `ldGuiDisposeNodeTree(scene, ld_subtree_root)` —— 一次性 free 整个 ld 子树(LingDongGUI 自己的深度优先递归)。
- 销毁子树时,内部任一控件若持 focus/editing,在步骤 1 遍历中一并释放。

### 4.6 每类 `host_cleanup` 钩子(可空 fn-ptr,挂 host,create 时设)
仅给**持宿主侧堆**或 **ld depose 未覆盖**的控件用:
- **keyboard**:释放动态 layout(`layout_entries` / 各 `text` / `native_layout`)→ 根治 #2。
- **button**:`xBtnRemove`(若 `ldButton_depose` 未含)。
- 其余控件:钩子为空。
- **副产物**:destroy 现在手里有 host → depose 所需 scene 直接取 `host->owner->ld_scene` → **删除全部 `s_*_depose_scene` 文件静态(#9)**,`tinyui_widget_destroy_common` 的 depose-cb 不再需要静态通道。

---

## 5. 数据流

- **创建**:`create_leaf` → ld_init → 挂树 → register。
- **事件**:native 事件 → `ld sender` → `tinyui_widget_from_ld` → host → 用户 cb。
- **单控件销毁**:见 4.4。
- **子树销毁**:见 4.5。
- **关机 `tinyui_app_destroy`**:遍历注册表逐个 `host_cleanup`(ld 仍在)→ `ldGuiDespose(scene)`(free 全部 ld)→ 遍历注册表 `free` 全部 host + 清空链表 → `free(scene)` → `free(app)`。**修掉当前"所有 host 在关机时泄漏"**。

---

## 6. 边界与风险

- **`uint16_t nameId` 空间**:`next_ld_name_id` 单调递增,长期高频 churn 累计 > 65535 次创建会回绕(与注册表结构无关)。**本期处理(已定)**:加 `nameId` 空闲表 —— destroy 时把已完全 depose 的 nameId 归还空闲表,`create_leaf` 优先从空闲表取、空了再 `++next_ld_name_id`。复用前提是该 nameId 对应 widget 已无任何引用(depose 已保证)。这与链表注册表正交。
- **复合列表项(#1 验证点)**:button/switch 作 list 项,其节点 `pInfo` 归 LingDongGUI 存坐标;TinyUI 反查走 nameId → 不再争用。需回归"列表项坐标在该控件存活期不被破坏"。
- **45 处 pInfo 迁移**:写面大但多为机械替换为 helper;7 个事件桥 widget 需逐个回归 native 事件可达。
- **销毁正在分发事件的控件**:维持现状单线程同步,不在事件回调里销毁自身(文档化约束);如需,加延迟销毁队列(本期非目标)。
- **window-root 不可单控件销毁**:维持现有 `tinyui_widget_destroy` 对 root window 返回 -1。

---

## 7. 落地分期(各期独立可编译、可回归)

- **P1 — 注册表 + 反查 helper(根治 #1 的前置,行为不变)**
  建注册表(链表 + `reg_prev/next` + `host_list_head`);`create_leaf` 改 register;45 处 `pInfo` 读 → `tinyui_widget_from_ld`;移除 pInfo 写。**验收**:全量 ctest 仍 72/72;事件 / 树 getter 行为不变;`command grep pInfo tinyui/src` 仅余(无)。

- **P2 — 真销毁(叶子)+ 关机回收 + 消除静态**
  `tinyui_widget_destroy` 包 depose + `host_cleanup` + free host;`tinyui_app_destroy` 遍历注册表回收;删 `s_*_depose_scene`(#9)。**验收**:单控件销毁后 ld 摘树 + host/ld 均 free + 焦点释放;反复 create/destroy 与关机 leak gate 干净。

- **P3 — 容器 / 子树递归销毁**
  暴露 `ldGuiDisposeNodeTree`(public);实现 4.5 子树宿主回收。**验收**:销毁带子控件的 window → 全部 host+ld 回收、无悬挂、无泄漏。

- **P4 — #2 keyboard + #1 复合列表项回归**
  keyboard `host_cleanup` 释放动态 layout;补复合列表项(button 作 list 项)增删回归。**验收**:keyboard `set_buttons` 后销毁无泄漏;复合列表项增删不破坏坐标、不越界。

---

## 8. 测试

| 用例 | 验证 |
|---|---|
| 销毁叶子控件 | ld 摘树、ld+host 双 free、focus/editing 释放、注册表移除 |
| 销毁容器(window+子控件) | 子树全部 host+ld 回收、无悬挂指针、无泄漏 |
| 复合列表项增删(#1) | list 项坐标(LingDongGUI pInfo)在控件存活期完好、销毁不越界 |
| 反复 churn | 无 nameId 无限增长 / 无回绕、leak gate 干净 |
| 事件迁移(pInfo→注册表) | 7 个事件桥 widget 的 native 事件仍达 host |
| keyboard set_buttons + destroy(#2) | 动态 layout 释放、无泄漏 |
| 关机回收 | 关机后全部 host 回收(leak gate),无残留 |

复用现有 `memory` / leak gate 与 `runtime` / `visible_ui` gate;新增 churn+teardown 泄漏用例。

---

## 9. 对 followups 的处置

| followup 项 | 本设计处置 |
|---|---|
| #1 list 项 pInfo 冲突 | **根治**(4.2 反查迁 nameId,pInfo 归还 LingDongGUI) |
| #2 keyboard teardown 泄漏 | **根治**(4.6 host_cleanup) |
| #3 销毁生命周期契约 | **落地方案 B**(运行期真销毁 + 关机回收) |
| #9 `s_*_depose_scene` 静态 | **消除**(4.6,destroy 持 host 直接取 scene) |

P3 其余项(#5 demo arm_2d 资产、#6 align 反向单源、#7 list 计数双源、#8 animation 双存、#10 直写 ld 字段)不在本设计范围,仍按 followups 登记。
