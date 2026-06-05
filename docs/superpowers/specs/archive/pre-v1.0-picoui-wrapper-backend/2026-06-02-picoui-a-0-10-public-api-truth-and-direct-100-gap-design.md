# PicoUI a-0.10 public API truth 与 direct-100 gap 设计

## 1. 背景

`a-0.9` 已把 LingDongGUI native public API 全量纳入 policy ledger：

1. `ldgui_public_api_inventory.json` 覆盖 `611` 行 native API。
2. `picoui_release_capability_matrix.json` 当前为 `covered=404 / allowlisted=207 / missing_gap_total=0`。
3. `policy_complete_not_direct_100` 表示 policy ledger 已闭环，但不是 PicoUI public API direct 100%。

2026-06-02 的 fresh subagent 复核发现一个更具体的问题：部分 `covered` 行的 `picoui_api` 字段不是当前 `picoui/include` 中真实存在的 public API 名。典型例子包括：

1. `picoui_gauge_move`，实际应为 `picoui_widget_set_pos`。
2. `picoui_progress_bar_set_frame_image`，实际应为 `picoui_progress_bar_set_frame_source`。
3. `picoui_q_r_code_set_select`，实际应为 `picoui_widget_set_selected`。
4. `picoui_slider_set_hidden`，实际应为 `picoui_widget_set_visible`。
5. `picoui_keyboard_btn_update`，实际应为 `picoui_keyboard_button_update`。
6. `picoui_radial_menu_set_offset_item`，实际应为 `picoui_radial_menu_offset_item`。

这类问题说明：matrix 不能只校验字段非空，还必须反查 public header。否则文档会把“想象中的 API”写成已覆盖能力。

## 2. 目标

`a-0.10` 必须完成：

1. 固化 public API truth gate：`covered` 行的 `picoui_api` 必须存在于 `picoui/include`。
2. 修正 `docs/ability/*` 与三份 contract JSON 中所有不存在的 public API 名。
3. 定义 direct public API 100% 的 denominator 和分层公式。
4. 对 `207` 个 `allowlisted` 行做 direct-100 再分类。
5. 更新 `docs/ability/*`，让每个控件能明确回答：
   - 哪些能力是 direct public API covered；
   - 哪些只是 policy allowlisted；
   - 当前是否达到 PicoUI 对外 direct 100%；
   - 若未达到，缺口是什么。
6. 为后续实现阶段拆分可交给 fresh subagent 的任务。

## 3. 非目标

`a-0.10` 不做：

1. 不把 `allowlisted` 行改成 `covered`。
2. 不把 `policy_complete_not_direct_100` 改写成 direct 100%。
3. 不强行暴露 lifecycle、show、render pipeline、runtime host、memory/layout solver helper。
4. 不暴露 native-only weak hook、raw Arm-2D draw callback、nameId action helper，除非先设计 portable PicoUI abstraction。
5. 不新增 LingDongGUI native API。
6. 不用截图、demo、artifact existence 证明 public API 覆盖。

## 4. 当前事实

当前事实必须在文档中保持 blunt：

| 指标 | 当前值 | 结论 |
| --- | ---: | --- |
| LingDongGUI native API inventory | `611` | 已全量建账 |
| matrix row | `611` | 已全量建账 |
| `covered` | `404` | 有 PicoUI public API/backend/unit/gate 证据 |
| `allowlisted` | `207` | 非 direct public wrapper 覆盖 |
| direct public API 100% | 否 | 当前不能写成 100% |
| policy ledger completeness | 是 | 不能等同 direct public API completeness |

## 5. Direct-100 分层

### 5.1 Denominator

Direct public API 100% 的 denominator 不是简单 `611`，必须分层：

1. `direct_denominator`：LingDongGUI native API 中应映射成 PicoUI user-facing public API 的能力。
2. `policy_denominator`：不应映射成 PicoUI public API、但必须说明 policy 的 native API。
3. `non_widget_denominator`：runtime/internal/helper 组，保留 inventory 审计，但不进入 widget direct public parity。

### 5.2 Row 分类

每个 row 增加或派生 direct-100 分类：

1. `direct_public_covered`：当前 `gap_status=covered`，且 `picoui_api` 是真实 public API。
2. `policy_never_public`：lifecycle/show/runtime/internal/private hook 等，不应暴露。
3. `optional_public_extension`：可以设计成 PicoUI public API，但不是当前 portable user model 必需。
4. `direct_100_required_if_user_demands`：若目标是“PicoUI 对外能力完全镜像 native 能力”，必须新增 public API。

### 5.3 Ability 文档表达

每个 `docs/ability/*.md` 必须显示：

1. `direct_public_covered` 数量。
2. `policy_allowlisted` 数量。
3. `direct_100_gap` 数量。
4. 当前结论：
   - `direct public 100% complete`
   - `policy complete, not direct public 100%`
   - `direct public gap remains`

## 6. Public API Truth Gate

`check_picoui_release_capability_matrix.py` 必须：

1. 扫描 `picoui/include/**/*.h`。
2. 抽取 `picoui_*(` 函数声明名。
3. 对每个 `gap_status=covered` 行校验 `picoui_api`。
4. 支持 `picoui_widget_get_x + picoui_widget_get_y` 这种组合 public API 表达。
5. 发现不存在 API 时 fail，并输出 native symbol 与错误 API 名。

## 7. 文档输出

`a-0.10` 必须新增：

1. `docs/picoui-serial/a-0.10-线计划索引.md`
2. `docs/superpowers/specs/2026-06-02-picoui-a-0-10-public-api-truth-and-direct-100-gap-design.md`
3. `docs/superpowers/plans/2026-06-02-picoui-a-0-10-public-api-truth-and-direct-100-gap-implementation.md`

并更新：

1. `docs/ability/README.md`
2. 所有直接受 public API 名修正影响的 ability 页面。
3. 若 R2/R3 发现新的 direct-100 缺口，更新对应 ability 页面。

## 8. 验收

`a-0.10` 完成时必须满足：

1. `covered` 行全部引用真实 public API。
2. `docs/ability` 不存在虚构 `picoui_*` API。
3. `docs/ability` 对 direct 100% 状态表达一致，不再只靠 policy complete。
4. `207` 个 allowlisted 行已有 direct-100 再分类。
5. checker 能防止非 public API 再次进入 `covered` 行。
6. fresh subagent review 不再发现 code/docs/matrix 对齐问题。
7. 以下命令全部通过：

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

## 9. R1-R3 决策记录

2026-06-02 按本 spec 推进后的当前决策：

1. Direct public API denominator 已冻结：`covered=404` 才是当前 direct public covered 行，`allowlisted=207` 不是 direct public wrapper。
2. Matrix summary 已增加 `direct_public_covered_total=404`、`policy_allowlisted_total=207`、`direct_public_100_complete=false`。
3. `207` 个 allowlisted 行已增加 `direct_100_category`：
   - `policy_never_public=191`
   - `optional_public_extension=16`
   - `direct_100_required_if_user_demands=0`
4. 因为没有 `direct_100_required_if_user_demands`，`a-0.10` 不新增 PicoUI public API。`optional_public_extension` 只表示未来如果产品要扩展 tree/focus/nameId 等能力，可以单独开线设计；本线不得把它当缺失必做接口。

## 10. R4-R5 收口记录

2026-06-02 收口后的当前事实：

1. `docs/ability/*` 已全量同步 direct public API 100% 表达。matrix-backed 页面均包含 `direct_public_covered`、`policy_allowlisted`、`direct_100_gap` 与 `direct_100_category`。
2. `docs/ability/background.md` 按 enum-only/window-tree policy 单独写明：不进入 widget public parity denominator，不新增 background public API。
3. fresh reviewer 已独立复核 public header、matrix、ledger、ability 文档和本 spec/plan/index，未发现 code/docs/matrix 对齐问题。
4. 最终 contract、backend mapping、visible、manual artifact、`git diff --check` 均通过。
5. manual artifact gate 只证明 artifact entry ready；当前仍为 `MANUAL_REVIEW_REQUIRED=1`、`MANUAL_REVIEWED_PASSED=0`，不能作为人工验收通过结论。
