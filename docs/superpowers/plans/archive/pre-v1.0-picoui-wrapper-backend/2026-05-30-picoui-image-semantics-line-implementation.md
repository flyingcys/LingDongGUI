# PicoUI Image 语义线 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `image` 剩余 broad gap 收口成稳定、诚实、可验证的语义合同：明确 `style_class / user_data` 仍是 metadata-only 候选合同，`bg_color / text_color / border_color / radius / enabled` 继续 `reject`，`padding` 继续 `deferred`，并同步真相源与最小测试证据。

**Architecture:** 这条线不新增 `image` 真实视觉或 disabled backend；它只冻结 public contract 与测试口径，避免把不存在的 `ldImage` style/state 承接点误表述成“差一点 support”。实现方式是以现有 `picoui_widget_*` 与 backend style dispatch 的真实行为为锚点，补齐最小 contract/unit 证据，并把矩阵、索引、决策入口统一到同一设计结论上。

**Tech Stack:** C, CMake, PicoUI abstraction layer, LingDongGUI `ldImage`, GitNexus impact analysis, Python contract checks, `ctest`

---

## 文件结构

- Modify: `tests/picoui/unit/test_picoui_widgets.c`
  - 为 `image style_class / user_data / theme reject / enabled reject` 增补最小 fail-first -> green 合同测试。
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
  - 仅当测试证明当前 reject 路径与设计不一致时，收紧 `image` style apply 的显式拒绝行为；如果现状已符合合同，则不改实现。
- Modify: `picoui/src/core/widget.c`
  - 仅当测试证明 `image enabled` 现状没有被稳定拒绝时，补齐最小 reject 语义；如果现状已符合合同，则不改实现。
- Modify: `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`
  - 把 `image` 剩余项的状态、证据层和备注与 `Image 语义线` 设计统一。
- Modify: `docs/picoui-serial/G-线计划索引.md`
  - 记录 `Image 语义线` 的 implementation plan 与收口后剩余入口。
- Modify: `docs/picoui-serial/G-线剩余缺口合同决策入口.md`
  - 把 `Image 语义线` 从 design-only 状态推进到 design + implementation 入口，并更新后续推荐顺序。

## Task 1: 冻结最小测试边界并制造 fail-first

**Files:**
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Reference: `docs/superpowers/specs/2026-05-30-picoui-image-semantics-line-design.md`
- Reference: `picoui/src/backend/ldgui/backend_style_apply.c`
- Reference: `picoui/src/core/widget.c`

- [ ] **Step 1: 为将要修改的 symbol 跑 GitNexus impact**

Run:

```bash
printf '%s\n' \
  'impact target: picoui_backend_widget_apply_style' \
  'impact target: picoui_widget_set_enabled'
```

Expected: 记录 `image` 相关 style/state 入口的 blast radius；若任一 symbol 为 `HIGH` 或 `CRITICAL`，先停下并重新审视这条线是否还能维持“语义冻结而非功能扩张”的边界。

- [ ] **Step 2: 新增 image 语义合同测试骨架**

在 `tests/picoui/unit/test_picoui_widgets.c` 增加以下测试函数骨架，并挂到主测试入口：

```c
static void test_image_style_class_and_user_data_are_metadata_only_contract(void)
{
    assert(!"write me");
}

static void test_image_theme_style_parts_remain_explicitly_rejected(void)
{
    assert(!"write me");
}

static void test_image_enabled_remains_rejected_contract(void)
{
    assert(!"write me");
}
```

- [ ] **Step 3: 把 metadata-only 用例补成真实断言**

目标断言内容：

```c
static void test_image_style_class_and_user_data_are_metadata_only_contract(void)
{
    struct picoui_app *app = test_picoui_app_create();
    struct picoui_window *win = picoui_window_create(picoui_app_root(app), "root");
    struct picoui_image *image = picoui_image_create(win);
    const char *style_class = "hero-image";
    void *user_data = (void *)0x1234;

    assert(image != 0);
    assert(picoui_widget_set_style_class(&image->widget, style_class) == 0);
    assert(picoui_widget_set_user_data(&image->widget, user_data) == 0);
    assert(image->widget.style_class == style_class);
    assert(image->widget.user_data == user_data);
    assert(image->widget.backend_widget != 0);
    assert(image->widget.backend_widget->style_class == style_class);
    assert(image->widget.backend_widget->user_data == user_data);

    picoui_app_destroy(app);
}
```

说明：该用例只能证明 metadata 存储，不得附带任何“视觉已生效”或“ldImage 已消费”断言。

- [ ] **Step 4: 把 image theme/style reject 用例补成显式断言**

目标断言内容：

```c
static void test_image_theme_style_parts_remain_explicitly_rejected(void)
{
    struct picoui_app *app = test_picoui_app_create();
    struct picoui_window *win = picoui_window_create(picoui_app_root(app), "root");
    struct picoui_image *image = picoui_image_create(win);
    struct picoui_theme_token token = {
        .bg = 0xff112233u,
        .fg = 0xff445566u,
        .border = 0xff778899u,
        .radius = 6,
    };

    assert(image != 0);
    assert(picoui_theme_apply_to_widget(&image->widget, PICOUI_PART_MAIN, &token) == -1);
    assert(picoui_theme_apply_to_widget(&image->widget, PICOUI_PART_TEXT, &token) == -1);

    picoui_app_destroy(app);
}
```

说明：如果当前 theme token 结构名或字段名不同，按真实头文件替换；关键是锁住 “`image` style apply 显式拒绝” 这条合同。

- [ ] **Step 5: 把 image enabled reject 用例补成显式断言**

目标断言内容：

```c
static void test_image_enabled_remains_rejected_contract(void)
{
    struct picoui_app *app = test_picoui_app_create();
    struct picoui_window *win = picoui_window_create(picoui_app_root(app), "root");
    struct picoui_image *image = picoui_image_create(win);

    assert(image != 0);
    assert(picoui_widget_set_enabled(&image->widget, 0) == -1);
    assert(image->widget.enabled == 1);

    picoui_app_destroy(app);
}
```

说明：这条线不允许把 `enabled` 偷换成 `visible`、`opacity` 或其他近似语义；测试目标就是把当前 `reject` 冻结成稳定合同。

- [ ] **Step 6: 跑目标测试并确认确实失败**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: `test_picoui_widgets` 失败，且失败点来自新加的 `image` 语义合同断言，而不是编译错误或无关测试。

- [ ] **Step 7: 提交 fail-first 测试**

```bash
git add tests/picoui/unit/test_picoui_widgets.c
git commit -m "test: add picoui image semantics cases"
```

## Task 2: 收紧 image reject / metadata-only 的最小实现或稳定现状

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_style_apply.c`
- Modify: `picoui/src/core/widget.c`
- Modify: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 只在测试证明现状不稳定时修改 style reject 路径**

若 `image` 当前某些 part 没有稳定返回 `-1`，在 `picoui/src/backend/ldgui/backend_style_apply.c` 把 `PICOUI_BACKEND_WIDGET_IMAGE` 的 reject 逻辑显式收紧成类似：

```c
case PICOUI_BACKEND_WIDGET_IMAGE:
    (void)part;
    (void)token;
    return -1;
```

要求：不新增任何 image style backend，不把 `MAIN` 或 `TEXT` 偷放进“部分成功”路径。

- [ ] **Step 2: 只在测试证明现状不稳定时修改 enabled reject 路径**

若 `picoui_widget_set_enabled(&image->widget, ...)` 当前不是稳定 `-1`，在 `picoui/src/core/widget.c` 把 `image` 保持在 reject 路径。例如：

```c
if (widget->type == PICOUI_WIDGET_IMAGE) {
    return -1;
}
```

要求：不能把 `image` 接到 `list` 的 selectable 路径，也不能引入视觉灰态伪语义。

- [ ] **Step 3: 确认 metadata-only 用例不要求改实现**

如果 `style_class / user_data` 测试已经能证明 PicoUI 与 backend wrapper 存储一致，则不要额外修改 `ldImage` 或 backend create 路径；本任务的正确结果通常是“测试转绿，代码不动”。

- [ ] **Step 4: 跑目标测试并确认三条 image 语义用例全部转绿**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: 新增 `image` 语义用例全部通过，且不引入其他 `picoui widgets` 回归。

- [ ] **Step 5: 提交 image 语义最小收口**

```bash
git add picoui/src/backend/ldgui/backend_style_apply.c picoui/src/core/widget.c tests/picoui/unit/test_picoui_widgets.c
git commit -m "feat: freeze picoui image semantics contracts"
```

## Task 3: 同步矩阵、索引与剩余入口

**Files:**
- Modify: `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`
- Modify: `docs/picoui-serial/G-线计划索引.md`
- Modify: `docs/picoui-serial/G-线剩余缺口合同决策入口.md`

- [ ] **Step 1: 更新能力矩阵中的 image 剩余项表述**

在矩阵里确认或补齐以下状态与备注：

```md
| image | `picoui_widget_set_style_class()`、`picoui_widget_set_user_data()` | 无真实 `ldImage` 消费链；当前只更新 PicoUI `widget` 与 backend wrapper 字段 | incomplete_contract | unit / contract | 当前只承认 metadata-only 存储语义 |
| image | `picoui_widget_set_bg_color()`、`picoui_widget_set_text_color()`、`picoui_widget_set_border_color()`、`picoui_widget_set_radius()` | `backend_style_apply` 对 `PICOUI_BACKEND_WIDGET_IMAGE` 显式拒绝 | reject | unit / contract | 不存在 image-style backend 承接点 |
| image | `picoui_widget_set_padding()` | 无稳定 image content box / layout spacing 合同 | deferred | contract | 语义未冻结，暂不承诺 |
| image | `picoui_widget_set_enabled()` | 无 image-specific disabled backend 语义 | reject | unit / contract | 不偷换成 `visible` / `opacity` / `selectable` |
```

- [ ] **Step 2: 更新索引中的当前阶段结论**

在 `docs/picoui-serial/G-线计划索引.md` 增加：

```md
- `Image 语义线` implementation plan：`docs/superpowers/plans/2026-05-30-picoui-image-semantics-line-implementation.md`
```

并把当前摘要收紧成：

```md
- `Image 语义线` 已完成 design/implementation/review/verification 闭环后，`image` broad gap 不再作为实现候选保留；剩余入口只回到 `List Metadata 合同线`。
```

说明：如果此任务执行时还没完成双 review / 最终验证，则先把 wording 写成“implementation plan 已建立，当前下一条串行线仍是 `Image 语义线`”。

- [ ] **Step 3: 更新剩余缺口入口文档**

在 `docs/picoui-serial/G-线剩余缺口合同决策入口.md`：

```md
- `Image 语义线` 已从 design truth 进入 implementation 计划阶段：
  - `design truth`: `docs/superpowers/specs/2026-05-30-picoui-image-semantics-line-design.md`
  - `implementation plan`: `docs/superpowers/plans/2026-05-30-picoui-image-semantics-line-implementation.md`
```

并在真正收口后把推荐顺序更新成只剩：

```md
1. `List Metadata 合同线`
```

- [ ] **Step 4: 跑文档自检**

Run:

```bash
git diff --check -- \
  docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md \
  docs/picoui-serial/G-线计划索引.md \
  docs/picoui-serial/G-线剩余缺口合同决策入口.md
```

Expected: 无 whitespace / conflict 标记 / 行尾错误。

- [ ] **Step 5: 提交文档同步**

```bash
git add docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md docs/picoui-serial/G-线计划索引.md docs/picoui-serial/G-线剩余缺口合同决策入口.md
git commit -m "docs: record picoui image semantics line"
```

## Task 4: 最终验证与收口

**Files:**
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Modify: `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`
- Modify: `docs/picoui-serial/G-线计划索引.md`
- Modify: `docs/picoui-serial/G-线剩余缺口合同决策入口.md`

- [ ] **Step 1: 跑本线目标验证**

Run:

```bash
cmake --build build
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
ctest --test-dir build -L picoui --output-on-failure
```

Expected: `image` 语义合同测试与现有 `picoui` 测试全集全部通过。

- [ ] **Step 2: 跑最终文档与 diff 自检**

Run:

```bash
git diff --check
```

Expected: 通过；没有新的 whitespace 或 conflict 标记。

- [ ] **Step 3: 在 review 通过后把索引收成下一剩余入口**

目标 wording：

```md
- `Image 语义线` 已完成 design、implementation、review 与 verification 闭环；它没有新增 image 功能，而是把 `metadata-only / reject / deferred` 三类边界冻结成稳定合同。
- 当前剩余入口只剩 `List Metadata 合同线`。
```

- [ ] **Step 4: 提交最终收口**

```bash
git add tests/picoui/unit/test_picoui_widgets.c picoui/src/backend/ldgui/backend_style_apply.c picoui/src/core/widget.c docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md docs/picoui-serial/G-线计划索引.md docs/picoui-serial/G-线剩余缺口合同决策入口.md
git commit -m "feat: close picoui image semantics line"
```

