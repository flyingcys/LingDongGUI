# PicoUI a-0.11 direct 100% base extension gap 设计

## 1. 背景

`a-0.10` 已完成 PicoUI public API truth gate，并把 LingDongGUI native API 全量建账：

| 指标 | 当前值 | 结论 |
| --- | ---: | --- |
| LingDongGUI native API inventory | `611` | 已全量建账 |
| matrix row | `611` | 已全量建账 |
| `covered` | `404` | 有 PicoUI public API/backend/unit/gate 证据 |
| `allowlisted` | `207` | 非 direct public wrapper 覆盖 |
| `missing_gap_total` | `0` | 无未建账 native API |
| `direct_public_100_complete` | `false` | 当前不能写成 direct 100% |

2026-06-02 用户要求重新开 subagent 复查 PicoUI 控件及能力，对照 `docs/ability` 与 LingDongGUI 能力，目标为 100%。fresh subagent 只读审计后确认：

1. `covered=404` 行引用的 PicoUI API 均存在于 `picoui/include` public header。
2. `docs/ability/*` 没有发现直接宣称已达到 direct 100% 的错误表达。
3. 除 `base` 的 `optional_public_extension=16` 外，其他 allowlisted 行均为 `policy_never_public`，没有明显应改成 direct public API 缺口的行。
4. `docs/ability/README.md` 与 `docs/ability/base.md` 对 `optional_public_extension=16` 的描述偏弱，容易被误读为“只是未来增强，不影响 100% 目标”。

因此 `a-0.11` 的目标不是实现新 API，而是把这次 100% 口径审计结论固化为正式文档线，并把 `base` 的 16 个候选写成 direct 100% 目标下的剩余 public API 缺口候选。

## 2. 目标

`a-0.11` 必须完成：

1. 新增 serial index、spec、plan 三份文档。
2. 明确当前仍未达到 100% direct public API parity。
3. 在 `docs/ability/README.md` 中说明：
   - `optional_public_extension=16` 不是完成项；
   - 严格按 100% direct public API parity 目标，它们是剩余缺口候选；
   - 后续如果要真正 direct 100%，必须补 public API、backend proof、unit/gate、matrix/docs。
4. 在 `docs/ability/base.md` 中说明：
   - `base` 有 `16` 个候选；
   - 这些候选当前没有 PicoUI public API/backend/unit/gate；
   - `direct_100_gap=0` 只是 a-0.10 分类口径，不表示 strict 100% 目标完成。
5. 给后续实现拆出可执行阶段，但本线不直接实现 API。

## 3. 非目标

`a-0.11` 不做：

1. 不新增 `picoui/include` public API。
2. 不修改 `picoui/src` backend 或 widget 实现。
3. 不修改 LingDongGUI native headers。
4. 不把 `optional_public_extension` 直接改成 `covered`。
5. 不把 `policy_never_public=191` 重新纳入 direct public API 实现范围。
6. 不改变 a-0.10 的机器 truth-source 统计，除非后续另开实现线。

## 4. 缺口候选清单

本线确认的 strict 100% direct public API parity 剩余候选全部位于 `base`：

| LingDongGUI API | 当前分类 | strict 100% 口径 |
| --- | --- | --- |
| `ldBaseAlignRegionCenter` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseAutoVerticalGridAlign` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseFocusNavigate` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseFocusNavigateInit` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetAbsoluteLocation` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetAlignRegion` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetChildCount` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetChildList` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetNameId` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetNextSibling` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetParent` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetRelativeLocation` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetRootNode` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetWidget` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetWidgetById` | `optional_public_extension` | 缺 public API 候选 |
| `ldBaseGetWidgetType` | `optional_public_extension` | 缺 public API 候选 |

## 5. 后续实现方向

后续如果用户要求继续做到真实 direct 100%，不能直接把 native API 名照搬成 PicoUI API。必须先做 portable PicoUI abstraction 设计：

1. `geometry helpers`：决定是否暴露 align/absolute/relative location helper，或只保留 layout/widget getter 组合。
2. `focus navigation`：决定 PicoUI 是否需要 public focus navigation API，以及事件/scene 抽象如何表达。
3. `tree traversal`：决定 PicoUI 是否允许 user-facing parent/child/sibling/root traversal。
4. `nameId lookup`：决定是否引入 portable widget id/name lookup，而不是泄漏 LingDongGUI `nameId`/scene macro。
5. `widget type query`：决定是否暴露 PicoUI widget type enum/query，且不能泄漏 `ldWidgetType_t`。

每个被实现的候选都必须同步：

1. public header declaration。
2. widget/core/backend implementation。
3. unit test 或 contract test。
4. backend proof。
5. matrix row 从 `allowlisted` 改为 `covered`。
6. `docs/ability/base.md` 和 `docs/ability/README.md` 更新。

## 6. 验收

`a-0.11` 完成时必须满足：

1. `docs/ability/README.md` 明确写出 `optional_public_extension=16` 是 strict 100% 目标下的剩余缺口候选。
2. `docs/ability/base.md` 明确写出 `16` 个候选当前没有 PicoUI public API/backend/unit/gate。
3. serial index、spec、plan 均能让 fresh subagent 理解下一步不是“口头 100%”，而是需要设计并实现这些候选 API。
4. 以下命令全部通过：

```bash
python3 tests/picoui/contract/check_ldgui_public_api_inventory.py
python3 tests/picoui/contract/check_picoui_native_api_exhaustiveness.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

## 7. 当前执行记录

2026-06-02 已完成：

1. fresh subagent 审计确认：未达到 `100% direct public API parity`。
2. 已确认 covered 行 public API 存在，contract gate 通过。
3. 已确认需要更新的 ability 文档只有 `docs/ability/README.md` 与 `docs/ability/base.md`。
4. 已将 `optional_public_extension=16` 明确写成 strict 100% 目标下的剩余缺口候选。
