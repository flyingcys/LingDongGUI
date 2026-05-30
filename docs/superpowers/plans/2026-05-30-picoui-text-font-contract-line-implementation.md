# PicoUI Text Font 合同线 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `text | picoui_text_set_font()` 从 wrapper-only 存储提升为真实 backend 字体解析、runtime rebind、reflow 与 cleanup 合同，同时保持现有 `struct picoui_font { family, size }` public API 形状不变。

**Architecture:** 方案保持 `picoui_font` 为描述值，由 PicoUI backend 负责解析 `family/size` 到真实 `arm_2d_font_t` 资源，再通过稳定的 `ldText/text_box` 更新路径完成 runtime 字体切换。实现必须显式隔离调用方描述值与 backend 内部字体资源生命周期，避免继续依赖“覆写 `ptFont` 指针 + `ldText_depose()` 条件释放”的隐式 ownership。

**Tech Stack:** C, CMake, PicoUI abstraction layer, LingDongGUI `ldText`, Arm-2D `text_box`, GitNexus impact analysis, `ctest`

---

## 文件结构

- Modify: `picoui/src/widgets/text.c`
  - 保持 `picoui_text_set_font()` 的 public 入口，但把成功语义从 wrapper-only 存储升级为真实 backend update。
- Modify: `picoui/src/backend/ldgui/backend_text.c`
  - 为 `text` widget 的 backend create / set_font 路径提供真实字体解析与 runtime update 入口。
- Modify: `picoui/src/backend/ldgui/backend_label.c`
  - 若当前通用 `picoui_backend_widget_set_font()` 定义在此，需把 `text` 与 `label` 的字体路径边界拆清，避免复用错误语义。
- Modify: `picoui/src/backend/ldgui/backend_widget.h`
  - 若需要新增 backend text font helper、font 解析结果缓存或 ownership 标记，在此声明最小接口。
- Modify: `src/gui/ldText.h`
  - 声明最小、明确的 runtime font update helper，不暴露多余 public 语义。
- Modify: `src/gui/ldText.c`
  - 提供真实 `ldText/text_box` font rebind + reflow 路径，并修正 cleanup 行为，避免错误释放非 owning font。
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
  - 增补 PicoUI text font 的 fail-first 单测，覆盖 `NULL` fallback、runtime rebind、backend failure 不分裂等合同。
- Modify: `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`
  - 仅在实现、验证全部通过后，把 `text font` 从 `incomplete_contract` 提升为 `support`，同步证据层说明。
- Modify: `docs/picoui-serial/G-线计划索引.md`
  - 记录 `Text Font 合同线` 实现收口状态与下一剩余项入口。
- Modify: `docs/picoui-serial/G-线剩余缺口合同决策入口.md`
  - 标记 `Text Font 合同线` 已执行完毕，并收紧剩余候选线状态。

## Task 1: 冻结实现边界与 fail-first 测试

**Files:**
- Modify: `tests/picoui/unit/test_picoui_widgets.c`
- Reference: `docs/superpowers/specs/2026-05-30-picoui-text-font-contract-line-design.md`
- Reference: `picoui/src/widgets/text.c`
- Reference: `picoui/src/backend/ldgui/backend_text.c`
- Reference: `src/gui/ldText.c`

- [ ] **Step 1: 为将要修改的 symbol 跑 GitNexus impact**

Run:

```bash
printf '%s\n' \
  'impact target: picoui_text_set_font' \
  'impact target: picoui_backend_widget_set_font' \
  'impact target: ldText_init' \
  'impact target: ldText_depose'
```

Expected: 记录到任务日志中，后续若任一 symbol 为 `HIGH` 或 `CRITICAL`，先停下并重新审视计划边界。

- [ ] **Step 2: 写出 fail-first 单测骨架**

在 `tests/picoui/unit/test_picoui_widgets.c` 新增以下测试函数骨架，并把它们挂到主测试入口：

```c
static void test_text_font_null_falls_back_to_default_contract(void)
{
    assert(!"write me");
}

static void test_text_font_runtime_rebind_updates_real_ldtext(void)
{
    assert(!"write me");
}

static void test_text_font_backend_failure_does_not_split_state(void)
{
    assert(!"write me");
}
```

- [ ] **Step 3: 把第一个失败用例补成真实断言**

目标断言内容：

```c
static void test_text_font_null_falls_back_to_default_contract(void)
{
    struct picoui_app *app = test_picoui_app_create();
    struct picoui_window *win = picoui_window_create(picoui_app_root(app), "root");
    struct picoui_text *text = picoui_text_create(win, "body");
    struct picoui_backend_widget *backend = text->widget.backend_widget;
    ldText_t *ld_text = (ldText_t *)backend->ld_widget;

    assert(text != 0);
    assert(ld_text != 0);
    assert(picoui_text_set_font(text, NULL) == 0);
    assert(text->widget.font == NULL);
    assert(ld_text->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont != NULL);

    picoui_app_destroy(app);
}
```

说明：如果 `text_box` 内部字段名与当前代码不一致，按真实结构替换，但必须断言“真实渲染使用的 ptFont 非空且已稳定回退”，不能退化成只看 wrapper。

- [ ] **Step 4: 把第二个失败用例补成 runtime rebind 合同断言**

目标断言内容：

```c
static void test_text_font_runtime_rebind_updates_real_ldtext(void)
{
    struct picoui_app *app = test_picoui_app_create();
    struct picoui_window *win = picoui_window_create(picoui_app_root(app), "root");
    struct picoui_text *text = picoui_text_create(win, "body");
    struct picoui_font font_a = {"Sans", 14};
    struct picoui_font font_b = {"Sans", 18};
    struct picoui_backend_widget *backend = text->widget.backend_widget;
    ldText_t *ld_text = (ldText_t *)backend->ld_widget;
    arm_2d_font_t *font_after_a;
    arm_2d_font_t *font_after_b;

    assert(picoui_text_set_font(text, &font_a) == 0);
    font_after_a = ld_text->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont;
    assert(font_after_a != NULL);

    assert(picoui_text_set_font(text, &font_b) == 0);
    font_after_b = ld_text->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont;
    assert(font_after_b != NULL);
    assert(font_after_b != font_after_a || font_b.size == font_a.size);

    picoui_app_destroy(app);
}
```

说明：若 backend 使用缓存导致同一真实 font 指针被复用，必须把断言调整成“真实 ptFont 已按新描述重新解析或归一化，而不是 wrapper-only 不动”；不要写成固定要求“指针一定不同”。

- [ ] **Step 5: 把第三个失败用例补成 failure atomicity 合同断言**

建议做法：在测试里替换或注入一个 backend font 解析失败路径，确保 `picoui_text_set_font()` 返回失败时旧真实字体状态仍保持稳定。

```c
static void test_text_font_backend_failure_does_not_split_state(void)
{
    assert(picoui_text_set_font(text, &good_font) == 0);
    old_real_font = ld_text->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont;

    test_backend_force_next_text_font_failure(backend);
    assert(picoui_text_set_font(text, &bad_font) == -1);

    assert(text->widget.font == &good_font);
    assert(backend->font == &good_font);
    assert(ld_text->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont == old_real_font);
}
```

- [ ] **Step 6: 跑目标测试并确认确实失败**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: `test_picoui_widgets` 失败，且失败点来自新加的 `text font` 合同断言，而不是编译错误或无关测试。

- [ ] **Step 7: 提交 fail-first 测试**

```bash
git add tests/picoui/unit/test_picoui_widgets.c
git commit -m "test: add picoui text font contract cases"
```

## Task 2: 建立 backend font 解析与稳定 fallback

**Files:**
- Modify: `picoui/src/backend/ldgui/backend_text.c`
- Modify: `picoui/src/backend/ldgui/backend_widget.h`
- Modify: `picoui/src/backend/ldgui/backend_label.c`
- Test: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 写出 backend text font helper 接口**

在 `picoui/src/backend/ldgui/backend_widget.h` 增加最小接口声明，例如：

```c
int picoui_backend_text_apply_font(struct picoui_backend_widget *widget,
                                   const struct picoui_font *font,
                                   int update_widget_state);
```

要求：名字可按现有风格微调，但必须是 `text` 专属 helper，而不是继续让 `label` 与 `text` 共用同一 wrapper-only 路径。

- [ ] **Step 2: 先让测试通过最小 fallback 语义**

在 `picoui/src/backend/ldgui/backend_text.c` 添加最小解析函数框架：

```c
static arm_2d_font_t *picoui_backend_text_resolve_font(const struct picoui_font *font)
{
    (void)font;
    return ARM_2D_FONT_6x8;
}
```

说明：第一步只允许作为最小 green 过渡；后续必须继续收口 family / size 归一化与失败原子性，不能停在“永远 6x8”。

- [ ] **Step 3: 把 `font == NULL` 显式定义为默认字体 fallback**

在 `picoui_backend_text_apply_font(...)` 里最少做到：

```c
arm_2d_font_t *resolved = picoui_backend_text_resolve_font(font);
if (resolved == NULL) {
    return -1;
}
```

并让 `widget->font` 的更新只在真实 backend apply 成功后提交。

- [ ] **Step 4: 修改 `picoui_text_set_font()` 只在 backend 成功后提交 public 状态**

目标代码形状：

```c
int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_text_apply_font(text->widget.backend_widget, font, 1) != 0) {
        return -1;
    }

    text->widget.font = font;
    return 0;
}
```

要求：不能保留“先改 widget.font，再尝试 backend”的 split path。

- [ ] **Step 5: 跑目标测试，确认 fallback 用例转绿、其他新用例仍保持红**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: `font == NULL` 用例转绿；runtime rebind 或 failure atomicity 至少仍有一个失败，证明任务边界没被跳过。

- [ ] **Step 6: 提交 backend fallback 最小闭环**

```bash
git add picoui/src/backend/ldgui/backend_text.c picoui/src/backend/ldgui/backend_widget.h picoui/src/backend/ldgui/backend_label.c picoui/src/widgets/text.c tests/picoui/unit/test_picoui_widgets.c
git commit -m "feat: add text font backend fallback path"
```

## Task 3: 为 ldText/text_box 建立真实 runtime rebind 路径

**Files:**
- Modify: `src/gui/ldText.h`
- Modify: `src/gui/ldText.c`
- Modify: `picoui/src/backend/ldgui/backend_text.c`
- Test: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 为 `ldText` 增加最小 runtime font update helper 声明**

在 `src/gui/ldText.h` 增加类似接口：

```c
bool ldTextSetFont(ldText_t *ptWidget, arm_2d_font_t *ptFont, bool takeOwnership);
```

说明：签名可微调，但必须能表达“新真实 font”和“ownership 策略”，避免 cleanup 语义继续悬空。

- [ ] **Step 2: 让 helper 真实更新 `ldText` 和 `text_box`**

在 `src/gui/ldText.c` 最小实现应包含：

```c
bool ldTextSetFont(ldText_t *ptWidget, arm_2d_font_t *ptFont, bool takeOwnership)
{
    if (ptWidget == NULL || ptFont == NULL) {
        return false;
    }

    ptWidget->ptFont = ptFont;
    ptWidget->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.ptFont = ptFont;
    text_box_set_scale(&ptWidget->tTextPanel, ptWidget->tTextPanel.use_as__arm_2d_helper_pfb_tCFG.tCFG.fScale);
    return true;
}
```

说明：实际字段名按真实结构调整；关键是必须触发真实 line metrics / layout 重新计算，不能只改 `ptWidget->ptFont`。

- [ ] **Step 3: 让 backend 通过该 helper 完成 runtime font update**

在 `picoui/src/backend/ldgui/backend_text.c` 中，把 `resolved` font 通过 `ldTextSetFont(...)` 应用到真实 `ldText`，并只在成功后提交 backend wrapper 状态。

- [ ] **Step 4: 运行目标测试，确认 runtime rebind 用例转绿**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: `test_text_font_runtime_rebind_updates_real_ldtext` 通过；failure atomicity 用例若仍失败，可留到下个任务。

- [ ] **Step 5: 提交 runtime rebind 闭环**

```bash
git add src/gui/ldText.h src/gui/ldText.c picoui/src/backend/ldgui/backend_text.c tests/picoui/unit/test_picoui_widgets.c
git commit -m "feat: add ldtext runtime font rebind"
```

## Task 4: 收口 ownership / cleanup 与 failure atomicity

**Files:**
- Modify: `src/gui/ldText.c`
- Modify: `src/gui/ldText.h`
- Modify: `picoui/src/backend/ldgui/backend_text.c`
- Test: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 明确 `ldText` 对当前真实 font 是否 owning**

在 `ldText_t` 结构中加入最小状态位，示例：

```c
bool ownsFont;
```

要求：状态位命名可调整，但必须能让 cleanup 路径区分“backend 内部持有资源”与“共享/默认字体”。

- [ ] **Step 2: 修正 `ldText_depose()` 的 font cleanup**

目标代码形状：

```c
if (ptWidget->ownsFont && ptWidget->ptFont != NULL) {
    ldFree(ptWidget->ptFont);
}
ptWidget->ptFont = NULL;
ptWidget->ownsFont = false;
```

说明：实际是否调用 `ldFree`、是否需要 backend custom destroy，按真实资源形态决定；但必须杜绝对共享默认字体或调用方描述值产生误释放。

- [ ] **Step 3: 让 `ldTextSetFont(...)` 先处理旧资源释放，再原子切换新状态**

示例流程：

```c
arm_2d_font_t *old_font = ptWidget->ptFont;
bool old_owns = ptWidget->ownsFont;

if (!text_font_apply_internal(ptWidget, new_font)) {
    return false;
}

ptWidget->ptFont = new_font;
ptWidget->ownsFont = takeOwnership;
text_font_cleanup(old_font, old_owns);
return true;
```

要求：失败时保留旧稳定状态，不能“先清旧状态，再发现新 apply 失败”。

- [ ] **Step 4: 在 backend 层补测试注入或失败开关**

在 `picoui/src/backend/ldgui/backend_text.c` 或测试辅助区加入仅测试可用的 failure 注入点，例如：

```c
static bool g_test_force_next_text_font_failure = false;
```

并提供测试 helper，使 `test_text_font_backend_failure_does_not_split_state` 能稳定触发失败路径。

- [ ] **Step 5: 跑目标测试，确认三个 text font 合同用例全部转绿**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
```

Expected: `test_picoui_widgets` 通过，且新增的三条 `text font` 合同用例全部为绿色。

- [ ] **Step 6: 提交 ownership / cleanup 收口**

```bash
git add src/gui/ldText.h src/gui/ldText.c picoui/src/backend/ldgui/backend_text.c tests/picoui/unit/test_picoui_widgets.c
git commit -m "fix: harden text font ownership contract"
```

## Task 5: 扩大验证并更新 G 线真相源

**Files:**
- Modify: `docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md`
- Modify: `docs/picoui-serial/G-线计划索引.md`
- Modify: `docs/picoui-serial/G-线剩余缺口合同决策入口.md`
- Test: `tests/picoui/unit/test_picoui_widgets.c`

- [ ] **Step 1: 跑实现完成后的 GitNexus impact 复核**

Run:

```bash
printf '%s\n' \
  'recheck impact target: picoui_text_set_font' \
  'recheck impact target: ldTextSetFont' \
  'recheck impact target: ldText_depose'
```

Expected: 风险仍在可接受范围；若出现意外扩散，先在 review 中处理，再更新文档结论。

- [ ] **Step 2: 跑最小验证矩阵**

Run:

```bash
cmake --build build --target test_picoui_widgets
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
git diff --check
```

Expected: 全部通过。

- [ ] **Step 3: 如有现成可用的 visible / mapping 证据，补跑一次；若没有，不虚构更强证据**

Run:

```bash
ctest --test-dir build -L picoui --output-on-failure
```

Expected: 不要求 `text font` 由整组标签测试单独证明可见视觉效果，但要确认此次改动未破坏既有 PicoUI 测试面。

- [ ] **Step 4: 更新 G 线 capability gap matrix**

把 `text | picoui_text_set_font()` 行更新为类似结论：

```md
| text | `picoui_text_set_font()` | backend text font resolve + `ldText` runtime rebind + reflow | support | unit / contract | `Text Font 合同线` 已收口为描述值 font -> backend 解析 -> 真实 `ldText/text_box` 更新合同；不承诺 public 指针身份保持 |
```

- [ ] **Step 5: 更新索引与剩余缺口入口**

在 `docs/picoui-serial/G-线计划索引.md` 和 `docs/picoui-serial/G-线剩余缺口合同决策入口.md` 中：

```md
- `Text Font 合同线` 已完成实现与验证收口。
- 当前剩余项回到 `image 语义线` 与 `list metadata 合同线`。
```

- [ ] **Step 6: 提交文档与验证收口**

```bash
git add docs/superpowers/specs/2026-05-29-picoui-g-line-current-widget-capability-gap-matrix.md docs/picoui-serial/G-线计划索引.md docs/picoui-serial/G-线剩余缺口合同决策入口.md
git commit -m "docs: close text font contract line"
```

## Task 6: review 收口与最终验证

**Files:**
- Review only: 本线全部改动

- [ ] **Step 1: 派 spec reviewer**

检查点：

```text
1. 是否严格实现了“描述值语义”，没有把 public API 偷改成 handle/token。
2. 是否真实实现了 backend 解析、runtime rebind、reflow、failure atomicity。
3. 是否没有越界顺手改 image/list 其他剩余项。
```

- [ ] **Step 2: 若 spec review 不通过，只回同一个 implementer 修复**

Run:

```bash
printf '%s\n' 'spec review failed -> send back to same implementer agent'
```

Expected: 不在主线程代修。

- [ ] **Step 3: 派 code quality reviewer**

检查点：

```text
1. ownership / cleanup 是否会误释放共享字体。
2. 测试是否真的走到真实 `ldText/text_box`，而不是 wrapper-only。
3. 是否引入新的状态分裂或隐藏耦合。
```

- [ ] **Step 4: 若 code quality review 不通过，只回同一个 implementer 修复**

Run:

```bash
printf '%s\n' 'code quality review failed -> send back to same implementer agent'
```

- [ ] **Step 5: 主线程做 fresh 最终验证**

Run:

```bash
cmake --build build
ctest --test-dir build -R '^test_picoui_widgets$' --output-on-failure
ctest --test-dir build -L picoui --output-on-failure
git diff --check
```

Expected: 全部通过。

- [ ] **Step 6: 主线程跑 `gitnexus_detect_changes()` 并核对变更范围**

Run:

```bash
printf '%s\n' 'run gitnexus_detect_changes(scope=\"all\") before closeout'
```

Expected: 只影响 `Text Font 合同线` 相关 symbol / process；若有额外扩散，先解释并收敛。

## Self-Review

- Spec coverage:
  - `描述值语义` 由 Task 2 与 Task 5 收口。
  - `runtime rebind + reflow` 由 Task 3 收口。
  - `ownership / cleanup` 与 `failure atomicity` 由 Task 4 收口。
  - `更新 G 线真相源` 由 Task 5 收口。
  - `subagent review + final verification` 由 Task 6 收口。
- Placeholder scan:
  - 计划中没有 `TBD`、`TODO`、`implement later` 一类占位语。
  - 个别代码块使用“示例流程/目标形状”而非逐行最终代码，是因为实际结构字段需对齐仓库现状；但每个任务都已经限定了必须满足的真实合同与验证命令。
- Type consistency:
  - 统一使用 `struct picoui_font { family, size }`、`picoui_text_set_font()`、`ldTextSetFont(...)`、`ptFont`、`ownsFont` 这些同一语义标识；若实现中最终命名微调，后续 review 必须同步检查所有文档与测试。
