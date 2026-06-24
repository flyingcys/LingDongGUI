# TinyUI 运行期控件生命周期 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 TinyUI 支持运行期单控件与子树的动态创建/销毁且无泄漏,做法是封装 LingDongGUI 已有的 `ptGuiFunc->depose` / `ldGuiDisposeNodeTree`,并把"ld 节点 → 宿主"反查从 LingDongGUI 拥有的 `pInfo` 迁到 TinyUI 自有的 `nameId` 注册表。

**Architecture:** 在 `tinyui_app` 上挂一条活宿主侵入式双向链表(注册表);`create_leaf` 注册宿主而非写 `pInfo`;事件桥与树 getter 通过 `nameId` 反查宿主;`tinyui_widget_destroy` 封装 `ptGuiFunc->depose` 真正释放 ld + 宿主;容器走暴露后的 `ldGuiDisposeNodeTree` 递归;每类可空 `host_cleanup` 钩子释放宿主侧堆。

**Tech Stack:** C(C99),CMake,CTest;LingDongGUI `src/gui`(ldBase/ldGui);现有 TinyUI core(`tinyui/src/core/`)、widgets(`tinyui/src/widgets/`)、单测(`tests/tinyui/unit/`)。

**权威设计:** `docs/superpowers/specs/2026-06-24-tinyui-widget-lifecycle-design.md`(方案 B2)。本计划不得自行更改其类型名/相位边界/验收。

**通用约定:**
- 计数/查残留用 `command grep`(代理会吞 grep 输出)。
- 每个相位结束跑全量回归:`cmake --build build -j8 && ctest --test-dir build`(基线 72/72 绿)。
- 编译用既有 `build/` 目录;若链接报 `ld*` 未定义,先 `cmake build/tinyui-runtime` 重配再 build。
- 提交信息末尾加:`Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`。

---

## 文件结构

| 文件 | 职责 | 改动 |
|---|---|---|
| `tinyui/src/core/internal.h` | 结构定义 | `tinyui_widget` 加 `reg_prev/reg_next`;`tinyui_app` 加 `host_list_head`(P1)、nameId 空闲表字段(P4);注册表/反查/destroy helper 原型 |
| `tinyui/src/core/widget_registry.c` | **新建**:注册表 + 反查 + nameId 分配/回收 | register/unregister/lookup/from_ld;`alloc_name_id`/`free_name_id`(P4) |
| `tinyui/src/core/widget.c` | core 生命周期 + 树 getter | `create_leaf` 改注册;树 getter 改反查;`tinyui_widget_destroy` 真销毁(P2);容器递归(P3) |
| `tinyui/src/core/runtime_bridge.c` | 事件桥 | 事件 slot + connect 改反查/注册;shutdown 回收宿主(P2) |
| `tinyui/src/widgets/{radial_menu,line_edit,message_box,icon_slider,table,combo_box,keyboard}.c` | 事件桥反查 | 7 处 `pInfo` 读 → `tinyui_widget_from_ld` |
| `tinyui/src/widgets/button.c` | name_id 反查 | 2 处 `pInfo` 读 → 反查;`host_cleanup`(P2) |
| `tinyui/src/widgets/{window,background}.c` | 自定义 create 路径 | pInfo 写 → 注册(P1) |
| `tinyui/src/widgets/keyboard.c` | 动态 layout | `host_cleanup` 释放 layout(P4) |
| `tinyui/src/core/app.c` | app 关机 | 遍历注册表回收宿主(P2) |
| `src/gui/ldGui.h` / `ldGui.c` | LingDongGUI | 把 `ldGuiDisposeNodeTree` 从 static 暴露成 public(P3,唯一一处 LingDongGUI 改动) |
| `tests/tinyui/unit/test_tinyui_lifecycle.c` | **新建** | 注册表/反查/真销毁/容器/churn/复合列表项/关机回收用例 |
| `tinyui/src/CMakeLists.txt`(或对应) | 构建 | 加 `widget_registry.c`、新测试目标 |

---

# Phase 1 — 注册表 + 反查迁移(行为不变,根治 #1 前置)

> 目标:把"ld→宿主"反查从 `pInfo` 迁到 `nameId` 注册表,`pInfo` 完全归还 LingDongGUI。本相位**不改变任何外部行为**,全量 ctest 仍 72/72。

### Task 1.1：注册表结构字段 + 原型

**Files:**
- Modify: `tinyui/src/core/internal.h`(`struct tinyui_widget` 末尾、`struct tinyui_app` 末尾、helper 原型区)

- [ ] **Step 1：给 `struct tinyui_widget` 增注册表链接字段**

在 `internal.h` `struct tinyui_widget` 末尾 `enum tinyui_edit_result edit_result_on_finish;` 之后加:
```c
    /* ── B2 运行期生命周期:活宿主注册表链接 ── */
    struct tinyui_widget *reg_prev;
    struct tinyui_widget *reg_next;
```

- [ ] **Step 2：给 `struct tinyui_app` 增注册表头**

在 `struct tinyui_app` 末尾 `struct tinyui_os_port_state os_port;` 之后加:
```c
    /* ── B2:活宿主侵入式双向链表头 ── */
    struct tinyui_widget *host_list_head;
```

- [ ] **Step 3：声明注册表/反查原型**

在 `internal.h` 合适的原型区(`tinyui_widget_create_leaf` 原型附近)加:
```c
void tinyui_app_register_host(struct tinyui_app *app, struct tinyui_widget *w);
void tinyui_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w);
struct tinyui_widget *tinyui_app_lookup_host(const struct tinyui_app *app, uint16_t name_id);
struct tinyui_widget *tinyui_widget_from_ld(const void *ld_node);
```

- [ ] **Step 4：编译通过(结构改动不破坏现有)**

Run: `cmake --build build -j8`
Expected: 成功(仅加字段,未用)。

- [ ] **Step 5：提交**

```bash
git add tinyui/src/core/internal.h
git commit -m "feat(tinyui/core): add host-registry fields + prototypes (B2 P1)"
```

### Task 1.2：注册表实现 + 反查(TDD)

**Files:**
- Create: `tinyui/src/core/widget_registry.c`
- Modify: 构建脚本(把 `widget_registry.c` 加入 core 源)
- Test: `tests/tinyui/unit/test_tinyui_lifecycle.c`(新建)

- [ ] **Step 1：写失败测试 —— 注册/反查/注销**

新建 `tests/tinyui/unit/test_tinyui_lifecycle.c`:
```c
#include <assert.h>
#include "internal.h"

static void test_registry_register_lookup_unregister(void)
{
    struct tinyui_app app;
    struct tinyui_widget a, b;
    memset(&app, 0, sizeof(app));
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.ld_name_id = 1; b.ld_name_id = 2;

    tinyui_app_register_host(&app, &a);
    tinyui_app_register_host(&app, &b);
    assert(tinyui_app_lookup_host(&app, 1) == &a);
    assert(tinyui_app_lookup_host(&app, 2) == &b);
    assert(tinyui_app_lookup_host(&app, 99) == 0);

    tinyui_app_unregister_host(&app, &a);
    assert(tinyui_app_lookup_host(&app, 1) == 0);
    assert(tinyui_app_lookup_host(&app, 2) == &b);

    tinyui_app_unregister_host(&app, &b);
    assert(app.host_list_head == 0);
}

int main(void)
{
    test_registry_register_lookup_unregister();
    return 0;
}
```

- [ ] **Step 2：加测试目标,运行确认失败(链接错误:函数未定义)**

把 `test_tinyui_lifecycle` 加入测试 CMake(参照同目录 `test_tinyui_core_helpers` 的目标定义),然后:
Run: `cmake build/tinyui-runtime && cmake --build build --target test_tinyui_lifecycle -j8`
Expected: 链接失败,`undefined reference to tinyui_app_register_host`。

- [ ] **Step 3：实现注册表**

新建 `tinyui/src/core/widget_registry.c`:
```c
#include "internal.h"
#include "drivers/tinyui_ldgui_port.h"   /* ldgui_port_get_current_app */
#include "ldBase.h"                       /* ldBase_t */

void tinyui_app_register_host(struct tinyui_app *app, struct tinyui_widget *w)
{
    if (app == 0 || w == 0) {
        return;
    }
    w->reg_prev = 0;
    w->reg_next = app->host_list_head;
    if (app->host_list_head != 0) {
        app->host_list_head->reg_prev = w;
    }
    app->host_list_head = w;
}

void tinyui_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w)
{
    if (app == 0 || w == 0) {
        return;
    }
    if (w->reg_prev != 0) {
        w->reg_prev->reg_next = w->reg_next;
    } else if (app->host_list_head == w) {
        app->host_list_head = w->reg_next;
    }
    if (w->reg_next != 0) {
        w->reg_next->reg_prev = w->reg_prev;
    }
    w->reg_prev = 0;
    w->reg_next = 0;
}

struct tinyui_widget *tinyui_app_lookup_host(const struct tinyui_app *app, uint16_t name_id)
{
    struct tinyui_widget *cur;
    if (app == 0) {
        return 0;
    }
    for (cur = app->host_list_head; cur != 0; cur = cur->reg_next) {
        if (cur->ld_name_id == name_id) {
            return cur;
        }
    }
    return 0;
}

struct tinyui_widget *tinyui_widget_from_ld(const void *ld_node)
{
    struct tinyui_app *app;
    if (ld_node == 0) {
        return 0;
    }
    app = ldgui_port_get_current_app();
    return tinyui_app_lookup_host(app, ((const ldBase_t *)ld_node)->nameId);
}
```

- [ ] **Step 4：运行测试,确认通过**

Run: `cmake --build build --target test_tinyui_lifecycle -j8 && ctest --test-dir build -R test_tinyui_lifecycle --output-on-failure`
Expected: PASS。

- [ ] **Step 5：提交**

```bash
git add tinyui/src/core/widget_registry.c tests/tinyui/unit/test_tinyui_lifecycle.c <cmake files>
git commit -m "feat(tinyui/core): host registry + nameId reverse-lookup (B2 P1)"
```

### Task 1.3：`create_leaf` 改注册(替代 pInfo 写)

**Files:**
- Modify: `tinyui/src/core/widget.c`(`tinyui_widget_create_leaf` 第 8 步与回滚)

- [ ] **Step 1：把第 8 步的 pInfo 写改为注册**

`widget.c:1924-1925` 现为:
```c
    /* 8. Bind pInfo so ld events find this widget */
    ((ldBase_t *)ld_widget)->pInfo = w;
```
改为:
```c
    /* 8. 注册宿主(ld 事件经 nameId 反查到此 widget),不再占用 pInfo */
    tinyui_app_register_host(owner, w);
```

- [ ] **Step 2：把回滚分支的 pInfo 清理改为注销**

`widget.c:1934-1940` 回滚分支(bind 失败)现把 `ld_base->pInfo = 0;`。改为先注销再清理:
```c
        ldBase_t *ld_base = (ldBase_t *)ld_widget;
        tinyui_app_unregister_host(owner, w);
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_widget);
        if (ld_base->ptGuiFunc != 0 && ld_base->ptGuiFunc->depose != 0) {
            ld_base->ptGuiFunc->depose(owner->ld_scene, ld_widget);
        }
        free(w);
        return 0;
```
(删去原 `ld_base->pInfo = 0;` 一行。)

- [ ] **Step 3：编译 + 全量回归(行为应不变,因读侧尚未迁移但写侧已无 pInfo——故本步会让事件暂时失效,Task 1.4 同相位内补齐;若 CI 要求每步绿,可与 1.4 合并为一次提交)**

Run: `cmake --build build -j8`
Expected: 编译成功。

> **执行提示:** Task 1.3 与 1.4/1.5 必须在**同一提交**前全部完成再跑 ctest(中间态 pInfo 写已移除但读未迁,事件会断)。把 1.3–1.5 视为一个原子改动。

### Task 1.4：迁移所有 `pInfo` 读 → 反查

**Files:**
- Modify(事件桥 slot,7 处,统一改法):
  - `radial_menu.c:123`、`line_edit.c:69`、`message_box.c:78`、`icon_slider.c:128`、`table.c:124`、`combo_box.c:69`、`keyboard.c:246`
- Modify(core 事件 slot):`runtime_bridge.c:27`
- Modify(name_id 反查):`button.c:774`、`button.c:812`(及其上方 `pInfo == 0` 守卫 `button.c:770/808`)
- Modify(树 getter):`widget.c:217`、`1302`、`1327`、`1352`、`1377`、`1447`

- [ ] **Step 1：事件桥 7 处统一替换**

每处把
```c
    w = (struct tinyui_widget *)((ldBase_t *)<sender>)->pInfo;
```
替换为
```c
    w = tinyui_widget_from_ld(<sender>);
```
其中 `<sender>` 为该处实参(`msg.ptSender` 或 `ld_message_box` / `ld_keyboard`)。删去相关 `C1-T7: pInfo ...` 注释。

- [ ] **Step 2：core 事件 slot(`runtime_bridge.c:27`)替换**
```c
    widget = tinyui_widget_from_ld(msg.ptSender);
```

- [ ] **Step 3：`button.c` name_id 反查(2 处)**

`button.c:770-774` 现为:
```c
    if (ld_found == 0 || ld_found->pInfo == 0) {
        return 0;   /* 或对应失败返回 */
    }
    widget = (struct tinyui_widget *)ld_found->pInfo;
```
改为:
```c
    widget = ld_found != 0 ? tinyui_widget_from_ld(ld_found) : 0;
    if (widget == 0) {
        return 0;   /* 保持原失败返回值 */
    }
```
`button.c:808-812` 同样处理。

- [ ] **Step 4：树 getter(`widget.c` 5 处 + 行 217)**

`tinyui_widget_get_parent`(行 1302)、`get_first_child`(1327)、`get_next_sibling`(1352)各把
```c
    return (struct tinyui_widget *)ld_xxx->pInfo;
```
改为
```c
    return tinyui_app_lookup_host(widget->owner, ((ldBase_t *)ld_xxx)->nameId);
```
(`widget` 为该 getter 入参;这些 getter 已持有 widget→owner。)

`get_root`(1371-1377)把"非空 pInfo"判断改为反查:
```c
        struct tinyui_widget *host = tinyui_app_lookup_host(widget->owner, ld_node->nameId);
        if (host != 0) {
            last_valid = host;
        }
```

`find_by_name_id`(1447)与行 217 的 `tinyui_widget_parent_internal`(若存在,行 203-217)同法改为 `tinyui_app_lookup_host(widget->owner, ld_xxx->nameId)`。

- [ ] **Step 5(确认无遗漏 pInfo 读)**

Run: `command grep -rn 'pInfo' tinyui/src`
Expected: 仅余 `window.c`/`background.c` 的写点(Task 1.5 处理)与注释;无 `= ...pInfo` 读。

### Task 1.5：自定义 create 路径(window/background)改注册

**Files:**
- Modify: `tinyui/src/widgets/window.c`(792 写、347/795 清)、`tinyui/src/widgets/background.c`(91 写、94 清)

- [ ] **Step 1：window 根创建处注册**

`window.c:792` `((ldBase_t *)ld_root)->pInfo = &window->widget;` 改为:
```c
    tinyui_app_register_host(window->widget.owner, &window->widget);
```
(确认此处 `window->widget.owner` 已赋值;若尚未,移到 owner 赋值之后。)

- [ ] **Step 2：window 失败/清理处注销**

`window.c:347` 与 `795` 的 `((ldBase_t *)...)->pInfo = 0;` 改为对应 `tinyui_app_unregister_host(window->widget.owner, &window->widget);`(按上下文判断是失败回滚还是销毁,owner 取该 window 的 owner)。

- [ ] **Step 3：background 同法**

`background.c:91` 写改注册 `tinyui_app_register_host(background->window.widget.owner, &background->window.widget);`;`background.c:94` 清改注销。

- [ ] **Step 4：移除所有剩余 pInfo 写,确认归零**

Run: `command grep -rn 'pInfo' tinyui/src`
Expected: tinyui/src 内 `pInfo` 仅余文档注释(无读写)。

- [ ] **Step 5：全量回归(行为不变验证)**

Run: `cmake build/tinyui-runtime && cmake --build build -j8 && ctest --test-dir build`
Expected: **72/72 + 新 test_tinyui_lifecycle 通过**;事件桥 7 widget、树 getter、button name_id 反查行为与改前一致。

- [ ] **Step 6：提交(1.3–1.5 原子)**

```bash
git add tinyui/src
git commit -m "refactor(tinyui): move host reverse-lookup off pInfo to nameId registry (B2 P1)"
```

---

# Phase 2 — 叶子真销毁 + 关机回收 + 消除静态

> 目标:`tinyui_widget_destroy` 真正 depose ld + free 宿主;关机遍历注册表回收;删除 `s_*_depose_scene` 文件静态(#9)。

### Task 2.1：`host_cleanup` 钩子字段 + 原型

**Files:**
- Modify: `tinyui/src/core/internal.h`

- [ ] **Step 1：给 `struct tinyui_widget` 加可空钩子**

在 `reg_next` 之后加:
```c
    /* ── B2:宿主侧额外清理(仅 keyboard/button 等用,可空)。在 ld depose 前调用,ld 仍存活 ── */
    void (*host_cleanup)(struct tinyui_widget *w);
```

- [ ] **Step 2：编译通过 + 提交**

Run: `cmake --build build -j8`
```bash
git add tinyui/src/core/internal.h
git commit -m "feat(tinyui/core): add host_cleanup hook field (B2 P2)"
```

### Task 2.2：叶子真销毁(TDD)

**Files:**
- Modify: `tinyui/src/core/widget.c`(`tinyui_widget_destroy` 行 1093-1122)
- Test: `tests/tinyui/unit/test_tinyui_lifecycle.c`

- [ ] **Step 1：写失败测试 —— 销毁叶子后从父树消失**

在 `test_tinyui_lifecycle.c` 加(用真实 app/window/label;参照 `tests/support/tinyui_test_support.c` 的 `test_window_create`):
```c
static void test_destroy_leaf_removes_from_tree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = test_window_create(&app);
    struct tinyui_label *label = tinyui_label_create(win, "leaf");
    struct tinyui_widget *w = &label->widget;
    uint16_t id = w->ld_name_id;

    assert(tinyui_app_lookup_host(app, id) == w);
    assert(tinyui_widget_destroy(w) == 0);
    /* 销毁后注册表查不到、父子树无此节点 */
    assert(tinyui_app_lookup_host(app, id) == 0);
    tinyui_app_destroy(app);
}
```
(把 `main()` 调用补上。)

- [ ] **Step 2：运行,确认失败**

Run: `cmake --build build --target test_tinyui_lifecycle -j8 && ctest --test-dir build -R test_tinyui_lifecycle --output-on-failure`
Expected: FAIL —— 现 `tinyui_widget_destroy` 不注销,`lookup` 仍返回 w(或断言在 free 后行为不符)。

- [ ] **Step 3：改写 `tinyui_widget_destroy` 为真销毁**

`widget.c:1093-1122` 整体改为:
```c
int tinyui_widget_destroy(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }
    if (widget->kind == TINYUI_BACKEND_WIDGET_WINDOW || widget->ld_widget == 0) {
        return -1;
    }

    owner = widget->owner;
    if (owner != 0) {
        if (owner->focus_owner == widget) {
            (void)tinyui_widget_release_focus(widget);
        }
        if (owner->editing_owner == widget) {
            (void)tinyui_widget_release_editing(widget);
        }
    }

    /* 宿主侧额外清理(ld 仍存活) */
    if (widget->host_cleanup != 0) {
        widget->host_cleanup(widget);
    }

    /* 从注册表摘除,再 depose ld(LingDongGUI 断消息+摘树+free ld),最后 free 宿主 */
    tinyui_app_unregister_host(owner, widget);
    ld_base = (ldBase_t *)widget->ld_widget;
    if (ld_base->ptGuiFunc != 0 && ld_base->ptGuiFunc->depose != 0) {
        ld_base->ptGuiFunc->depose(owner != 0 ? owner->ld_scene : 0, widget->ld_widget);
    } else {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
    }
    free(widget);
    return 0;
}
```

- [ ] **Step 4：运行测试,确认通过**

Run: `cmake --build build --target test_tinyui_lifecycle -j8 && ctest --test-dir build -R test_tinyui_lifecycle --output-on-failure`
Expected: PASS。

- [ ] **Step 5：迁移现有 `destroy_common` 调用点为新销毁路径(或保留 destroy_common 仅供 create 回滚)**

确认各 widget 的 create 回滚仍可用(它们调 `tinyui_widget_destroy_common`)。`destroy_common` 保留用于 create 回滚;运行期销毁统一走 `tinyui_widget_destroy`。无需逐 widget 改。

- [ ] **Step 6：全量回归 + 提交**

Run: `ctest --test-dir build`
Expected: 全绿。
```bash
git add tinyui/src/core/widget.c tests/tinyui/unit/test_tinyui_lifecycle.c
git commit -m "feat(tinyui): real leaf widget destroy via wrapped ld depose (B2 P2)"
```

### Task 2.3：关机遍历注册表回收宿主

**Files:**
- Modify: `tinyui/src/core/app.c`(`tinyui_app_destroy` 行 ~ shutdown 序)、`tinyui/src/core/runtime_bridge.c`(`shutdown_app` 若负责 scene depose)

- [ ] **Step 1：写失败测试 —— 关机后无残留(用 instrumented 计数或既有 leak gate)**

在 `test_tinyui_lifecycle.c` 加:建 app + 多个未手动销毁的控件 → `tinyui_app_destroy` → 断言不崩溃(泄漏由 leak gate / valgrind 验证)。同时新增"churn"用例:循环 create+destroy 同类控件 N 次,断言每轮 `lookup` 行为正确、`host_list_head` 最终随 app 释放。

- [ ] **Step 2：在 `tinyui_app_destroy` 加宿主回收**

在 `app.c` `tinyui_app_destroy` 内,**在 `tinyui_runtime_bridge_shutdown_app(app)`(深 depose ld)之前**先对每个宿主做 host_cleanup(ld 仍在),**之后**遍历注册表 free 宿主:
```c
    /* B2:关机前对剩余宿主做 host_cleanup(ld 仍存活) */
    {
        struct tinyui_widget *w;
        for (w = app->host_list_head; w != 0; w = w->reg_next) {
            if (w->host_cleanup != 0) {
                w->host_cleanup(w);
            }
        }
    }

    tinyui_runtime_bridge_shutdown_app(app);   /* ldGuiDespose: free 全部 ld */
    xBtnDestroy();

    /* B2:ld 已 depose,回收所有宿主结构 */
    {
        struct tinyui_widget *w = app->host_list_head;
        while (w != 0) {
            struct tinyui_widget *next = w->reg_next;
            free(w);
            w = next;
        }
        app->host_list_head = 0;
    }

    free(app);
```
(注意:window/background 宿主是内嵌在 `tinyui_window`/`tinyui_background` 里的 `widget` 成员,`free(w)` 即释放整个外层结构——确认 `&window->widget` 是外层结构首成员或用 `container_of` 还原首地址;若 `widget` 非首成员,需存外层指针。**见 Step 3 校验**。)

- [ ] **Step 3：校验宿主可被 `free(&...->widget)` 直接释放**

Run: `command grep -n 'struct tinyui_widget widget;' tinyui/src/widgets/*.c tinyui/src/core/internal.h`
确认各 widget 外层结构(如 `struct tinyui_label { struct tinyui_widget widget; ... }`)的 `widget` 是**首成员**(则 `&w->widget == w` 地址相同,`free(w)` 正确)。window/background 内嵌 `tinyui_window` → 其 `widget` 亦应为首成员。**若有非首成员者,改为在注册表存外层首地址**(本计划默认首成员,create_leaf 的 `host_size`+`calloc(1,host_size)` 返回的就是外层首地址,`&w->widget==w` 成立)。

- [ ] **Step 4：全量回归 + 泄漏门 + 提交**

Run: `ctest --test-dir build`(含 memory/leak gate)
Expected: 全绿,关机回收后 leak gate 干净。
```bash
git add tinyui/src/core/app.c tests/tinyui/unit/test_tinyui_lifecycle.c
git commit -m "feat(tinyui): reclaim all host structs at app teardown (B2 P2)"
```

### Task 2.4：消除 `s_*_depose_scene` 文件静态(#9)

**Files:**
- Modify: 含 `s_*_depose_scene` 的各 widget(button/switch/checkbox/slider/line_edit/arc/gauge/progress_bar/progress_wheel/image/qrcode/animation/date_time/clock/calendar/keyboard 等,以 `command grep -rln 's_.*_depose_scene' tinyui/src/widgets` 为准)

- [ ] **Step 1：列出所有静态通道**

Run: `command grep -rn 's_.*depose_scene' tinyui/src/widgets`
记录每处 widget 与其 `ld_depose_cb`。

- [ ] **Step 2：把 per-widget `ld_depose_cb` 改为从 host 取 scene**

由于运行期销毁现走 `tinyui_widget_destroy`(直接调 `ptGuiFunc->depose(owner->ld_scene, ld)`),per-widget 的 `*_ld_depose_cb` + `s_*_depose_scene` 仅剩 **create 回滚** 路径在用(`destroy_common(w, cb)`)。改造 `destroy_common` 让其自身从 `w->owner->ld_scene` 取 scene 并调 `((ldBase_t*)w->ld_widget)->ptGuiFunc->depose(scene, w->ld_widget)`,从而 `destroy_common` 不再需要 `ld_depose_cb` 参数与静态通道。

`widget.c:1815` `tinyui_widget_destroy_common` 改为(去掉 cb 参数):
```c
void tinyui_widget_destroy_common(struct tinyui_widget *w)
{
    ldBase_t *ld_base;
    struct ld_scene_t *scene;
    if (w == 0) {
        return;
    }
    if (w->host_cleanup != 0) {
        w->host_cleanup(w);
    }
    scene = (w->owner != 0) ? w->owner->ld_scene : 0;
    ld_base = (ldBase_t *)w->ld_widget;
    if (ld_base != 0 && ld_base->ptGuiFunc != 0 && ld_base->ptGuiFunc->depose != 0) {
        ld_base->ptGuiFunc->depose(scene, w->ld_widget);
    } else if (ld_base != 0) {
        ldBaseNodeRemove((arm_2d_control_node_t *)w->ld_widget);
    }
    free(w);
}
```
更新原型(`internal.h`)与全部调用点:把 `tinyui_widget_destroy_common(&x->widget, x_ld_depose_cb)` 改为 `tinyui_widget_destroy_common(&x->widget)`;删除各 `s_*_depose_scene` 静态与 `*_ld_depose_cb`(若 cb 仅做 `ldXxx_depose`——而 `ptGuiFunc->depose` 已是同一函数,可直接删)。

**同时去重 `tinyui_widget_destroy`(避免重复 depose / 双重 host_cleanup):** Task 2.2 里 `tinyui_widget_destroy` 内联了 host_cleanup + depose + free;现 `destroy_common` 已统一承担这三步。把 `tinyui_widget_destroy` 收敛为只做"前置 + 委托":
```c
    /* focus/editing 释放(同 Task 2.2)... */
    tinyui_app_unregister_host(owner, widget);
    tinyui_widget_destroy_common(widget);   /* host_cleanup + depose + free 都在这里 */
    return 0;
```
即从 `tinyui_widget_destroy` 中删去内联的 host_cleanup / depose / free,只保留校验、focus/editing 释放、unregister,然后委托 `destroy_common`。**确保 host_cleanup 只被调用一次。**

> **校验:** 确认每个 widget 的 `ptGuiFunc->depose` 与其原 `*_ld_depose_cb` 调用的 `ldXxx_depose` 是同一函数(func 表注册的就是它)。`command grep -n '.depose =' src/gui/ld<Widget>.c` 核对。

- [ ] **Step 3：确认静态归零**

Run: `command grep -rn 's_.*depose_scene\|_ld_depose_cb' tinyui/src/widgets`
Expected: 无残留。

- [ ] **Step 4：全量回归 + 提交**

Run: `ctest --test-dir build`
Expected: 全绿。
```bash
git add tinyui/src
git commit -m "refactor(tinyui): drop s_*_depose_scene statics; destroy_common derives scene from host (B2 P2, #9)"
```

---

# Phase 3 — 容器 / 子树递归销毁

> 目标:支持销毁带子控件的容器(子窗口/子页面),封装 LingDongGUI 的 `ldGuiDisposeNodeTree`。

### Task 3.1：暴露 `ldGuiDisposeNodeTree`(LingDongGUI 唯一改动)

**Files:**
- Modify: `src/gui/ldGui.c`(去掉 `static`)、`src/gui/ldGui.h`(加声明)

- [ ] **Step 1：去掉 static**

`ldGui.c:26` `static void ldGuiDisposeNodeTree(...)` → `void ldGuiDisposeNodeTree(...)`(纯暴露,函数体不动)。

- [ ] **Step 2：加 public 声明**

`ldGui.h` 在 `void ldGuiDespose(ld_scene_t *ptScene);` 附近加:
```c
/* 深度优先 depose 一个控件及其全部子孙(供上层做子树销毁) */
void ldGuiDisposeNodeTree(ld_scene_t *ptScene, ldBase_t *ptWidget);
```

- [ ] **Step 3：编译 + 提交**

Run: `cmake --build build -j8`
```bash
git add src/gui/ldGui.c src/gui/ldGui.h
git commit -m "feat(ldgui): expose ldGuiDisposeNodeTree for subtree teardown (B2 P3)"
```

### Task 3.2：容器递归销毁(TDD)

**Files:**
- Modify: `tinyui/src/core/widget.c`(`tinyui_widget_destroy` 支持容器)
- Test: `tests/tinyui/unit/test_tinyui_lifecycle.c`

- [ ] **Step 1：写失败测试 —— 销毁子窗口连带子控件**

```c
static void test_destroy_container_reclaims_subtree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = test_window_create(&app);
    struct tinyui_window *child = tinyui_window_create_child(root, "child_panel");
    struct tinyui_label *l1 = tinyui_label_create(child, "c1");
    struct tinyui_label *l2 = tinyui_label_create(child, "c2");
    uint16_t cid = child->widget.ld_name_id, id1 = l1->widget.ld_name_id, id2 = l2->widget.ld_name_id;

    assert(tinyui_widget_destroy(&child->widget) == 0);
    assert(tinyui_app_lookup_host(app, cid) == 0);
    assert(tinyui_app_lookup_host(app, id1) == 0);
    assert(tinyui_app_lookup_host(app, id2) == 0);
    tinyui_app_destroy(app);
}
```
(注意:子 window 的 `kind == TINYUI_BACKEND_WIDGET_WINDOW`,当前 `tinyui_widget_destroy` 对 WINDOW 返回 -1;容器销毁需放开"非 root window"的销毁。)

- [ ] **Step 2：运行,确认失败**

Run: `ctest --test-dir build -R test_tinyui_lifecycle --output-on-failure`
Expected: FAIL(子 window 被拒或子控件残留)。

- [ ] **Step 3：实现容器分支**

在 `tinyui_widget_destroy` 中区分叶子/容器:若 `ld_widget` 有子节点(`ldBaseGetChildList != 0`)或 `kind==WINDOW`(非 root),走子树销毁:
把开头"禁止销毁"的条件从"任何 WINDOW"(`kind == TINYUI_BACKEND_WIDGET_WINDOW`)收窄为"root window":
```c
    /* 仅 root window 不可销毁;其余 window/容器可走子树销毁 */
    if (owner != 0 && owner->root_window != 0 &&
        &owner->root_window->widget == widget) {
        return -1;
    }
```
然后在叶子销毁逻辑之前插入容器分支(有子节点即走递归):
```c
    if (ldBaseGetChildList((ldBase_t *)widget->ld_widget) != 0) {
        /* 容器:先回收子树内每个宿主(ld 仍在),再让 LingDongGUI 递归 depose 整棵 ld 子树 */
        tinyui_destroy_reclaim_hosts_in_subtree(owner, (ldBase_t *)widget->ld_widget);
        ldGuiDisposeNodeTree(owner->ld_scene, (ldBase_t *)widget->ld_widget);
        return 0;
    }
```

- [ ] **Step 4：实现 `tinyui_destroy_reclaim_hosts_in_subtree`(深度优先,宿主回收但不碰 ld)**

新增 static helper(`widget.c`):
```c
static void tinyui_destroy_reclaim_hosts_in_subtree(struct tinyui_app *app, ldBase_t *node)
{
    ldBase_t *child;
    if (node == 0) {
        return;
    }
    for (child = ldBaseGetChildList(node); child != 0; child = (ldBase_t *)child->use_as__arm_2d_control_node_t.ptNext) {
        tinyui_destroy_reclaim_hosts_in_subtree(app, child);
    }
    {
        struct tinyui_widget *host = tinyui_app_lookup_host(app, node->nameId);
        if (host != 0) {
            if (host->owner != 0) {
                if (host->owner->focus_owner == host) {
                    (void)tinyui_widget_release_focus(host);
                }
                if (host->owner->editing_owner == host) {
                    (void)tinyui_widget_release_editing(host);
                }
            }
            if (host->host_cleanup != 0) {
                host->host_cleanup(host);
            }
            tinyui_app_unregister_host(app, host);
            free(host);   /* 仅 free 宿主;ld 由随后的 ldGuiDisposeNodeTree 统一 free */
        }
    }
}
```
(确认 `ldBaseGetChildList` 与兄弟遍历字段名:用现有树 getter 的同款访问;若 core 已有 `tinyui_widget_get_first_child/next_sibling` 的底层 ld 遍历,复用之。)

- [ ] **Step 5：运行测试,确认通过 + 全量回归**

Run: `ctest --test-dir build`
Expected: 新容器用例 PASS,72/72 仍绿。

- [ ] **Step 6：提交**

```bash
git add tinyui/src/core/widget.c tests/tinyui/unit/test_tinyui_lifecycle.c
git commit -m "feat(tinyui): recursive container/subtree destroy via ldGuiDisposeNodeTree (B2 P3)"
```

---

# Phase 4 — keyboard 泄漏(#2)+ 复合列表项回归(#1)+ nameId 防回绕

### Task 4.1：keyboard `host_cleanup` 释放动态 layout

**Files:**
- Modify: `tinyui/src/widgets/keyboard.c`
- Test: `tests/tinyui/unit/test_tinyui_keyboard.c` 或 `test_tinyui_lifecycle.c`

- [ ] **Step 1：写失败测试 —— set_buttons 后销毁无泄漏**

```c
static void test_keyboard_set_buttons_then_destroy_no_leak(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = test_window_create(&app);
    struct tinyui_keyboard *kbd = tinyui_keyboard_create(win, "kbd");
    static const struct tinyui_keyboard_button buttons[] = { /* 若干自定义键 */ };
    assert(tinyui_keyboard_set_buttons(kbd, buttons, <count>) == 0);
    assert(tinyui_widget_destroy(&kbd->widget) == 0);   /* host_cleanup 释放动态 layout */
    tinyui_app_destroy(app);
}
```
(泄漏由 leak gate / valgrind 验证。)

- [ ] **Step 2：实现 keyboard `host_cleanup` 并在 create 时挂上**

新增:
```c
static void tinyui_keyboard_host_cleanup(struct tinyui_widget *w)
{
    struct tinyui_keyboard *kbd = (struct tinyui_keyboard *)w;
    /* 释放 set_buttons 分配的动态布局:layout_entries / 各 entry text / native_layout */
    tinyui_keyboard_free_dynamic_layout(kbd);   /* 复用 keyboard.c 现有释放逻辑(原仅回滚/复用时调) */
}
```
在 `tinyui_keyboard_create` 成功后设 `keyboard->widget.host_cleanup = tinyui_keyboard_host_cleanup;`。把 keyboard.c 现有"回滚/复用时释放动态布局"的代码抽成 `tinyui_keyboard_free_dynamic_layout(kbd)` 并复用(幂等:释放后置空,可重复调)。

- [ ] **Step 3：运行 + 全量回归 + 提交**

Run: `ctest --test-dir build`(含 leak gate + test_tinyui_keyboard)
Expected: 全绿,无泄漏。
```bash
git add tinyui/src/widgets/keyboard.c tests/tinyui/unit/test_tinyui_keyboard.c
git commit -m "fix(tinyui/keyboard): free dynamic layout on destroy via host_cleanup (B2 P4, #2)"
```

### Task 4.2：复合列表项增删回归(#1 验证)

**Files:**
- Test: `tests/tinyui/unit/test_tinyui_lifecycle.c`(或 `test_tinyui_list.c`)

- [ ] **Step 1：写测试 —— button 作 list 项,坐标不被破坏、销毁不越界**

```c
static void test_composite_list_item_no_pinfo_clash(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = test_window_create(&app);
    struct tinyui_list *list = tinyui_list_create(win, "list");
    struct tinyui_button *item = tinyui_button_create(win, "item_btn");
    assert(tinyui_list_set_item_widget(list, 0, &item->widget) == 0);  /* 复合项 */
    /* 该 item 节点 pInfo 归 LingDongGUI 存坐标;TinyUI 反查走 nameId,二者不争用 */
    assert(tinyui_widget_from_ld(item->widget.ld_widget) == &item->widget);   /* 反查仍命中 */
    /* 销毁 list 连带项,不崩溃、不越界 */
    assert(tinyui_widget_destroy(&list->widget) == 0);
    tinyui_app_destroy(app);
}
```
(按 `tinyui_list` 实际 set-item API 调整;若复合项 API 形态不同,用最接近的真实路径。)

- [ ] **Step 2：运行,确认通过(P1 已根治 → 应直接绿)**

Run: `ctest --test-dir build -R test_tinyui_lifecycle --output-on-failure`
Expected: PASS(若失败,说明 list 项节点的 nameId 反查或坐标 pInfo 有残余冲突,在本 Task 内排查修复)。

- [ ] **Step 3：提交**

```bash
git add tests/tinyui/unit/test_tinyui_lifecycle.c
git commit -m "test(tinyui): composite widget as list item — no pInfo clash (B2 P4, #1)"
```

### Task 4.3：nameId 空闲表(防 uint16 回绕)

**Files:**
- Modify: `tinyui/src/core/internal.h`(app 加空闲表字段)、`tinyui/src/core/widget_registry.c`(alloc/free name_id)、`widget.c` create_leaf 取 id、destroy/destroy_common 归还

- [ ] **Step 1：写失败测试 —— 反复 churn 后 nameId 被复用(不单调增长到回绕)**

```c
static void test_name_id_reused_after_destroy(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = test_window_create(&app);
    struct tinyui_label *a = tinyui_label_create(win, "a");
    uint16_t id_a = a->widget.ld_name_id;
    assert(tinyui_widget_destroy(&a->widget) == 0);
    struct tinyui_label *b = tinyui_label_create(win, "b");
    assert(b->widget.ld_name_id == id_a);   /* 复用了 a 的 id */
    tinyui_app_destroy(app);
}
```

- [ ] **Step 2：app 加空闲表 + alloc/free**

`internal.h` `struct tinyui_app` 加:
```c
    uint16_t *free_name_ids;
    uint16_t  free_name_id_count;
    uint16_t  free_name_id_cap;
```
`widget_registry.c` 加:
```c
uint16_t tinyui_app_alloc_name_id(struct tinyui_app *app)
{
    if (app->free_name_id_count > 0) {
        return app->free_name_ids[--app->free_name_id_count];
    }
    return ++app->next_ld_name_id;
}

void tinyui_app_free_name_id(struct tinyui_app *app, uint16_t name_id)
{
    if (app->free_name_id_count == app->free_name_id_cap) {
        uint16_t new_cap = app->free_name_id_cap == 0 ? 16 : app->free_name_id_cap * 2;
        uint16_t *grown = (uint16_t *)realloc(app->free_name_ids, new_cap * sizeof(uint16_t));
        if (grown == 0) {
            return;   /* OOM:放弃复用该 id(仅退化为单调增长,不破坏正确性) */
        }
        app->free_name_ids = grown;
        app->free_name_id_cap = new_cap;
    }
    app->free_name_ids[app->free_name_id_count++] = name_id;
}
```
原型加入 `internal.h`。

- [ ] **Step 3：create_leaf 取 id 改用 alloc;destroy/destroy_common/window/background 归还 id**

`widget.c:1892` `name_id = ++owner->next_ld_name_id;` → `name_id = tinyui_app_alloc_name_id(owner);`
在 `tinyui_widget_destroy` 与 `tinyui_destroy_reclaim_hosts_in_subtree` 与 `destroy_common` 的 free 宿主前,调 `tinyui_app_free_name_id(owner, widget->ld_name_id);`(window/background 自定义路径同理)。
关机回收(`app.c`)末尾 free 空闲表:`free(app->free_name_ids);`。

- [ ] **Step 4：运行测试 + 全量回归 + 提交**

Run: `ctest --test-dir build`
Expected: 新 churn/复用用例 PASS,72/72 + 新增用例全绿,leak gate 干净。
```bash
git add tinyui/src
git commit -m "feat(tinyui): nameId free-list to bound id space under churn (B2 P4)"
```

---

## 最终验收(全相位后)

- [ ] 全量 `ctest --test-dir build` 绿(原 72 + 新增 lifecycle 用例);contract / runtime / perf / `visible_ui` 门绿。
- [ ] `command grep -rn 'pInfo' tinyui/src` 仅余注释(无读写);`command grep -rn 's_.*depose_scene' tinyui/src/widgets` 归零。
- [ ] 运行期销毁叶子 / 容器 / 复合列表项均无崩溃、无越界、无泄漏(leak gate / valgrind)。
- [ ] 反复 churn 不致 nameId 回绕、不泄漏;关机后全部宿主回收。
- [ ] SDL demo 行为与改前一致(`visible_ui` 截图门)。
- [ ] 更新 followups 文档:#1/#2/#3/#9 标记为"已由 2026-06-24 生命周期方案解决"。
