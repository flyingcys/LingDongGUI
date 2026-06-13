# TINYUI List Metadata 合同线 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `list style_class` 与 widget-level `list user_data` 收口成稳定、诚实、可验证的 metadata-only 合同，同时把它们与 `on_selected(..., user_data)` 的 callback cookie 语义彻底拆开，并同步真相源与最小测试证据。

**Architecture:** 这条线不新增 `ldList` 的真实 metadata 消费行为；它只冻结 public contract 与测试口径，避免把 TINYUI / backend wrapper 的存储事实误表述成真实 `ldList` style/state 行为。实现方式是以现有 `tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()` 与 `tinyui_list_set_on_selected()` 的真实写入边界为锚点，补齐最小 contract/unit 证据，并把矩阵、索引、决策入口统一到同一设计结论上。

**Tech Stack:** C, CMake, TINYUI abstraction layer, LingDongGUI `ldList`, GitNexus impact analysis, Python contract checks, `ctest`

---

## 文件结构

- Modify: `tests/tinyui/unit/test_tinyui_list.c`
  - 为 `list style_class / user_data` 的 metadata-only 合同，以及 widget-level `user_data` 与 `on_selected(..., user_data)` callback cookie 分离关系，增补最小 fail-first -> green 测试。
- Modify: `tinyui/src/widgets/list.c`
  - 仅当测试证明 `create_with_props()` 没有稳定写入 metadata 或错误混入 callback cookie 时，补齐最小 metadata 提交顺序；若现状已符合合同，则不改实现。
- Modify: `tinyui/src/core/widget.c`
  - 仅当测试证明 `tinyui_widget_set_style_class()` / `tinyui_widget_set_user_data()` 在 list 上没有稳定把值写到 widget 与 backend wrapper 时，补齐最小 metadata 路径；若现状已符合合同，则不改实现。
- Modify: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
  - 把 `list style_class`、`list user_data` 与 `on_selected(..., user_data)` 的边界说明与 `List Metadata 合同线` 设计统一。
- Modify: `docs/tinyui-serial/G-线计划索引.md`
  - 记录 `List Metadata 合同线` 的 implementation plan 与后续剩余入口。
- Modify: `docs/tinyui-serial/G-线剩余缺口合同决策入口.md`
  - 把 `List Metadata 合同线` 从 design-only 状态推进到 design + implementation 入口，并更新后续推荐顺序。

## Task 1: 冻结最小测试边界并制造 fail-first

**Files:**
- Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Reference: `docs/superpowers/specs/2026-05-30-tinyui-list-metadata-contract-line-design.md`
- Reference: `tinyui/src/widgets/list.c`
- Reference: `tinyui/src/core/widget.c`

- [ ] **Step 1: 为将要修改的 symbol 跑 GitNexus impact**

Run:

```bash
printf '%s\n' \
  'impact target: tinyui_widget_set_style_class' \
  'impact target: tinyui_widget_set_user_data' \
  'impact target: tinyui_list_set_on_selected'
```

Expected: 记录 `list metadata` 相关入口的 blast radius；若任一 symbol 为 `HIGH` 或 `CRITICAL`，先停下并重新审视这条线是否还能维持“metadata-only 语义冻结”的边界。

- [ ] **Step 2: 新增 list metadata 合同测试骨架**

在 `tests/tinyui/unit/test_tinyui_list.c` 增加以下测试函数骨架，并挂到主测试入口：

```c
static void test_list_style_class_and_user_data_are_metadata_only_contract(void)
{
    assert(!"write me");
}

static void test_list_widget_user_data_is_distinct_from_on_selected_cookie(void)
{
    assert(!"write me");
}
```

- [ ] **Step 3: 把 metadata-only 用例补成真实断言**

目标断言内容：

```c
static void test_list_style_class_and_user_data_are_metadata_only_contract(void)
{
    struct tinyui_app *app = test_tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(tinyui_app_root(app), "root");
    struct tinyui_list *list = tinyui_list_create(win, "list_metadata_only");
    struct tinyui_backend_widget *backend;
    const char *style_class = "settings-list";
    int cookie = 41;

    assert(list != 0);
    backend = list->widget.backend_widget;
    assert(backend != 0);

    assert(tinyui_widget_set_style_class(&list->widget, style_class) == 0);
    assert(tinyui_widget_set_user_data(&list->widget, &cookie) == 0);
    assert(list->widget.style_class != 0);
    assert(strcmp(list->widget.style_class, style_class) == 0);
    assert(list->widget.user_data == &cookie);
    assert(backend->style_class != 0);
    assert(strcmp(backend->style_class, style_class) == 0);
    assert(backend->user_data == &cookie);

    tinyui_app_destroy(app);
}
```

说明：该用例只能证明 widget 与 backend wrapper 两侧 metadata 被存储，不得附带任何 `ldList` 已消费、视觉已变化或 selection 行为已变化断言。

- [ ] **Step 4: 把 widget-level user_data 与 callback cookie 分离用例补成真实断言**

目标断言内容：

```c
static void test_list_widget_user_data_is_distinct_from_on_selected_cookie(void)
{
    struct tinyui_app *app = test_tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(tinyui_app_root(app), "root");
    struct tinyui_list *list = tinyui_list_create(win, "list_user_data_boundary");
    int widget_cookie = 11;
    int callback_cookie = 22;

    assert(list != 0);
    assert(tinyui_widget_set_user_data(&list->widget, &widget_cookie) == 0);
    assert(tinyui_list_set_on_selected(list, on_list_selected_probe, &callback_cookie) == 0);
    assert(list->widget.user_data == &widget_cookie);
    assert(list->user_data == &callback_cookie);
    assert(list->widget.user_data != list->user_data);

    tinyui_app_destroy(app);
}
```

说明：如果当前 callback 字段名不同，按真实结构替换；关键是锁住“widget-level `user_data` 不等于 callback cookie”这条合同。

- [ ] **Step 5: 跑目标测试并确认确实失败**

Run:

```bash
cmake --build build --target test_tinyui_list
ctest --test-dir build -R '^test_tinyui_list$' --output-on-failure
```

Expected: `test_tinyui_list` 失败，且失败点来自新加的 `list metadata` 合同断言，而不是编译错误或无关测试。

- [ ] **Step 6: 提交 fail-first 测试**

```bash
git add tests/tinyui/unit/test_tinyui_list.c
git commit -m "test: add tinyui list metadata cases"
```

## Task 2: 收紧 metadata-only 的最小实现或稳定现状

**Files:**
- Modify: `tinyui/src/widgets/list.c`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tests/tinyui/unit/test_tinyui_list.c`

- [ ] **Step 1: 只在测试证明现状不稳定时修改 style_class / user_data 写入路径**

若 `list` 当前在 `tinyui_widget_set_style_class()` / `tinyui_widget_set_user_data()` 后不能稳定让 widget 与 backend wrapper 两侧都持有 metadata，则在最小写面内收紧，例如：

```c
int tinyui_widget_set_style_class(struct tinyui_widget *widget, const char *style_class)
{
    ...
    widget->style_class = style_class;
    if (widget->backend_widget != 0) {
        return tinyui_backend_widget_set_style_class(widget->backend_widget, style_class);
    }
    return 0;
}
```

要求：不新增任何 `ldList` 消费链，不把 metadata-only 路径升级成真实 style/state 行为。

- [ ] **Step 2: 只在测试证明 create_with_props() 没有稳定提交 metadata 时补最小顺序**

若 `tinyui_list_create_with_props()` 当前没有稳定把 `style_class` / widget-level `user_data` 提交到 widget 与 backend wrapper，则在 `tinyui/src/widgets/list.c` 补最小提交顺序。例如：

```c
if (props->style_class != 0 && tinyui_widget_set_style_class(&list->widget, props->style_class) != 0) {
    tinyui_list_destroy(list);
    return 0;
}
if (tinyui_widget_set_user_data(&list->widget, props->user_data) != 0) {
    tinyui_list_destroy(list);
    return 0;
}
```

要求：不能借机改 callback cookie 语义，也不能新增 `ldList` metadata 消费行为。

- [ ] **Step 3: 确认 callback cookie 分离用例不要求改实现**

如果 `tinyui_list_set_on_selected()` 现状已经把 callback 私有 `user_data` 保存在独立字段，且不会覆盖 widget-level `user_data`，则不要额外修改 callback bridge；本任务的正确结果通常是“测试转绿，代码不动”。

- [ ] **Step 4: 跑目标测试并确认 list metadata 用例全部转绿**

Run:

```bash
cmake --build build --target test_tinyui_list
ctest --test-dir build -R '^test_tinyui_list$' --output-on-failure
```

Expected: 新增 `list metadata` 用例全部通过，且不引入其他 `tinyui list` 回归。

- [ ] **Step 5: 提交 list metadata 最小收口**

```bash
git add tinyui/src/widgets/list.c tinyui/src/core/widget.c tests/tinyui/unit/test_tinyui_list.c
git commit -m "feat: freeze tinyui list metadata contracts"
```

## Task 3: 同步矩阵、索引与剩余入口

**Files:**
- Modify: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- Modify: `docs/tinyui-serial/G-线计划索引.md`
- Modify: `docs/tinyui-serial/G-线剩余缺口合同决策入口.md`

- [ ] **Step 1: 更新能力矩阵中的 list metadata 表述**

在矩阵里确认或补齐以下状态与备注：

```md
| list | `tinyui_widget_set_style_class()` | 无真实 `ldList` 消费链；当前仅为 TINYUI / backend wrapper metadata 存储 | incomplete_contract | unit / contract | 当前只承认 metadata-only 存储语义 |
| list | `tinyui_widget_set_user_data()` | widget-level `user_data` 仅做 TINYUI / backend wrapper 通用存储；不等同于 `on_selected(..., user_data)` callback cookie | incomplete_contract | unit / contract | 当前只承认 metadata-only 存储语义，且必须与 callback cookie 分离 |
```

- [ ] **Step 2: 更新索引中的当前阶段结论**

在 `docs/tinyui-serial/G-线计划索引.md` 增加：

```md
- `List Metadata 合同线` implementation plan：`docs/superpowers/plans/2026-05-30-tinyui-list-metadata-contract-line-implementation.md`
```

并把当前摘要收紧成：

```md
- `List Metadata 合同线` 已完成 design/implementation/review/verification 闭环后，`list style_class` 与 widget-level `list user_data` 不再作为 broad metadata gap 保留；剩余入口再评估是否还有新的 line-level 合同线。
```

说明：如果此任务执行时还没完成双 review / 最终验证，则先把 wording 写成“implementation plan 已建立，当前下一条串行线仍是 `List Metadata 合同线`”。

- [ ] **Step 3: 更新剩余缺口入口文档**

在 `docs/tinyui-serial/G-线剩余缺口合同决策入口.md`：

```md
- `List Metadata 合同线` 已从 design truth 进入 implementation 计划阶段：
  - `design truth`: `docs/superpowers/specs/2026-05-30-tinyui-list-metadata-contract-line-design.md`
  - `implementation plan`: `docs/superpowers/plans/2026-05-30-tinyui-list-metadata-contract-line-implementation.md`
```

并在真正收口后把推荐顺序更新成“先重新评估剩余 line-level gap”，而不是继续保留旧 broad 入口。

- [ ] **Step 4: 跑文档自检**

Run:

```bash
git diff --check -- \
  docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md \
  docs/tinyui-serial/G-线计划索引.md \
  docs/tinyui-serial/G-线剩余缺口合同决策入口.md
```

Expected: 无 whitespace / conflict 标记 / 行尾错误。

- [ ] **Step 5: 提交文档同步**

```bash
git add docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md docs/tinyui-serial/G-线计划索引.md docs/tinyui-serial/G-线剩余缺口合同决策入口.md
git commit -m "docs: record tinyui list metadata line"
```

## Task 4: 最终验证与收口

**Files:**
- Modify: `tests/tinyui/unit/test_tinyui_list.c`
- Modify: `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
- Modify: `docs/tinyui-serial/G-线计划索引.md`
- Modify: `docs/tinyui-serial/G-线剩余缺口合同决策入口.md`

- [ ] **Step 1: 跑本线目标验证**

Run:

```bash
cmake --build build
ctest --test-dir build -R '^test_tinyui_list$' --output-on-failure
ctest --test-dir build -L tinyui --output-on-failure
```

Expected: `list metadata` 合同测试与现有 `tinyui` 测试全集全部通过。

- [ ] **Step 2: 跑最终文档与 diff 自检**

Run:

```bash
git diff --check
```

Expected: 通过；没有新的 whitespace 或 conflict 标记。

- [ ] **Step 3: 在 review 通过后把索引收成新的剩余入口**

目标 wording：

```md
- `List Metadata 合同线` 已完成 design、implementation、review 与 verification 闭环；它没有新增真实 `ldList` metadata 行为，而是把 widget/backend wrapper metadata 与 callback cookie 语义边界冻结成稳定合同。
- 当前剩余入口应回到新的 line-level gap 盘点，而不是继续保留旧 broad metadata gap。
```

- [ ] **Step 4: 提交最终收口**

```bash
git add tests/tinyui/unit/test_tinyui_list.c tinyui/src/widgets/list.c tinyui/src/core/widget.c docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md docs/tinyui-serial/G-线计划索引.md docs/tinyui-serial/G-线剩余缺口合同决策入口.md
git commit -m "feat: close tinyui list metadata line"
```
