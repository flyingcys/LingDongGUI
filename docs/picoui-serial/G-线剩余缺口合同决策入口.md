# TINYUI G线剩余缺口合同决策入口

## 目的

- 本文不是新的实现计划，也不是直接进入 `G10` 的开发单。
- 它只整理 `G9` 之后剩余缺口为什么不能继续按“现成实现线”推进，以及后续若要继续推进，必须先做什么 line-level 合同决策。
- 使用场景：当 `G-线计划索引.md` 已把现有 capability gap 收紧到无法继续自然串行实现时，先从这里选下一条合同决策线，再单独开 spec / plan / implementation。

## 当前 authoritative 前提

- 当前真相源仍以：
  - `docs/tinyui-serial/G-线计划索引.md`
  - `docs/superpowers/specs/2026-05-29-tinyui-g-line-current-widget-capability-gap-matrix.md`
  为准。
- 当前剩余项已被收紧为：
  - `image style_class / user_data`：`incomplete_contract`
  - `image bg_color / text_color / border_color / radius`：`reject`
  - `image padding`：`deferred`
  - `image enabled`：`reject`
  - `list style_class`：`incomplete_contract`
  - `list user_data`：`incomplete_contract`

## 为什么不能直接进入实现

- `text font` 不是少一个 setter，而是缺从 `tinyui_font` 到 `ldText/text_box` 的真实 resource / ownership / runtime update 合同。
- `image style` 当前没有真实 backend 承接点；theme / backend style dispatch 还把 image 显式固定在 reject 路径。
- `image enabled` 没有 image-specific backend bridge，也没有 `ldImage` / `ldBase` 下自然的 disabled image 语义，不能把 `hidden` / `opacity` / `selectable` 偷换成 enabled。
- `list style_class` 与 `list user_data` 不是同一类缺口：前者是 wrapper-only metadata，后者还要与已收口的 `on_selected(..., user_data)` callback cookie 区分。

## 可选下一线

### 1. Image 语义线

- 目标：为 `image` 上仍未闭环的 style/state 项重新定合同，而不是直接实现。
- design truth：`docs/superpowers/specs/2026-05-30-tinyui-image-semantics-line-design.md`
- 这条线要先回答：
  - `image style_class / user_data` 是不是只承认 metadata 合同？
  - `image bg/text/border/radius` 是否长期 `reject`，还是未来允许进入 image-style backend？
  - `image padding` 到底代表 content inset、layout spacing，还是根本不应该成为 image public contract？
  - `image enabled` 是否长期 `reject`，还是未来要引入全新的 disabled image 语义？

### 2. List Metadata 合同线

- 目标：为 `list style_class` 与 widget-level `user_data` 重新定合同。
- design truth：`docs/superpowers/specs/2026-05-30-tinyui-list-metadata-contract-line-design.md`
- 这条线要先回答：
  - `style_class` 是否只承认 metadata 存储，不承诺真实行为？
  - widget-level `user_data` 是否只承认通用存储，不与 `on_selected(..., user_data)` callback cookie 混用？
  - 如果要升级为真实行为合同，最小真实消费链到底是什么？

## 推荐顺序

1. `List Metadata 合同线`

推荐理由：

- `list metadata` 的现实压力最低，因为 `on_selected` 主能力已收口，剩余项更多是 metadata / semantics cleanup。

## 当前设计决策状态

- `Image 语义线` 已完成 design truth 冻结：
  - `docs/superpowers/specs/2026-05-30-tinyui-image-semantics-line-design.md`
- `Image 语义线` 已进入 implementation 执行阶段：
  - `implementation plan`: `docs/superpowers/plans/2026-05-30-tinyui-image-semantics-line-implementation.md`
- 当前冻结结论：
  - `image style_class / user_data` 继续只承认 metadata-only 候选合同
  - `image bg_color / text_color / border_color / radius / enabled` 继续维持 `reject`
  - `image padding` 继续维持 `deferred`
- 当前实现进展：
  - `image style_class / user_data` 的 metadata-only 合同测试已补齐
  - `image enabled` 现已在 `tinyui_widget_set_enabled()` 上稳定维持 `reject`
  - `image` theme/style 的 reject 测试现已补到 `PICOUI_PART_TEXT`
- 当前阶段结论：
  - `Image 语义线` 已完成 design、implementation、review 与 verification 闭环。
  - 这条线没有新增 image 功能，而是把 `metadata-only / reject / deferred` 三类边界冻结成稳定合同。

- `List Metadata 合同线` 已完成 design truth 冻结：
  - `docs/superpowers/specs/2026-05-30-tinyui-list-metadata-contract-line-design.md`
- `List Metadata 合同线` 已进入 implementation 执行阶段：
  - `implementation plan`: `docs/superpowers/plans/2026-05-30-tinyui-list-metadata-contract-line-implementation.md`
- 当前冻结结论：
  - `list style_class` 继续只承认 metadata-only 候选合同
  - widget-level `list user_data` 继续只承认 metadata-only 候选合同
  - `on_selected(..., user_data)` 的 callback cookie 语义继续保持独立 support 合同，不并入 broad metadata 线
- 当前实现进展：
  - `test_list_widget_user_data_is_distinct_from_on_selected_cookie` 与 `test_list_style_class_and_user_data_are_metadata_only_contract` 已补入 `tests/tinyui/unit/test_tinyui_list.c`
  - `style_class` 的 metadata-only 测试边界已收紧为 widget/backend wrapper 存储事实，不再错误要求 copy/value 语义
  - 当前 authoritative 结论是：`tinyui_widget_set_style_class()`、`tinyui_widget_set_user_data()` 与 `tinyui_list_create_with_props()` 的 metadata 写入路径已经满足这条线的最小合同，不需要再为 list metadata 单独补实现
- 当前阶段结论：
  - `List Metadata 合同线` 已完成 design、implementation、spec re-review、code-quality review 与 verification 闭环。
  - 这条线没有把 `list style_class` / widget-level `user_data` 升级成真实 `ldList` 行为合同，而是把它们稳定冻结为 metadata-only + `incomplete_contract`。
  - 当前现状下不存在自然的下一条 `list metadata` 实现子线；若要继续推进，只能先重新定义更窄的 line-level 合同。

- `Text Font 合同线` 已完成设计、实现、review 与验证闭环：
  - `design truth`: `docs/superpowers/specs/2026-05-30-tinyui-text-font-contract-line-design.md`
- 当前冻结结论：
  - `struct tinyui_font { family, size }` 继续按描述值语义处理，不升级成 handle-only 或 token-only public contract。
  - 当前真实合同已收口为“描述值 font -> backend 最小内置映射/回退 -> 真实 `ldText/text_box` 更新”。
  - 对外不承诺完整 family/size 解析、动态字体资源加载或 public 指针身份保持；`NULL` fallback、runtime rebind、ownership/cleanup 与 approx failure atomicity 已闭环。

## 进入条件

- 当前已知后续合同线都已完成收口；若未来还要继续推进 G 线，只能先提出新的 line-level 合同边界，再重开 spec / plan / implementer 流程。
- 在没有新的合同线之前，不应把现有 `incomplete_contract / reject / deferred` 项直接当成 `G10` 现成实现任务硬做。
