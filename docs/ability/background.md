# `background`

本文记录 `widgetTypeBackground` 的控件类型边界和 PicoUI 覆盖边界。

## 覆盖摘要

- 控件类型来源：`src/gui/ldBase.h` 的 `ldWidgetType_t`。
- LingDongGUI 独立 header/API 分组：无。
- PicoUI 独立 matrix group：无；a-0.9 通过 `window` group 的 `enum_only_semantics` policy 行记录 root/background 语义。
- 覆盖结论：不能写成“PicoUI 独立 API 100% 覆盖 background”，因为 LingDongGUI 没有独立 `ldBackground.h` public API；当前正式定义为 `window/tree derived enum-only` policy。
- native/user-facing 100%：未闭环；当前由 window/root/background source/color 语义承载。如果目标是每个 `ldWidgetType_t` 都有独立 PicoUI public widget，`background` 仍是缺口。
- direct public API 100%：不适用；`background` 没有独立 native API group，不进入 widget public parity denominator。
- direct_public_covered：`0`
- policy_allowlisted：`0`
- direct_100_gap：`0`
- direct_100_category：`enum_only_semantics` policy，见 `window.md`。

## 能力边界

| 能力项 | LingDongGUI 来源 | PicoUI 覆盖判断 | 说明 |
| --- | --- | --- | --- |
| 控件类型 | `widgetTypeBackground` | `enum_only_semantics` policy | enum 中的独立类型，但无独立 public API group。 |
| 根背景语义 | scene/window tree | `picoui_window_create` + window background/color/layout API | `picoui_backend_create_window` 创建真实 `ldWindow` root，window API 覆盖用户态 root/background 意图。 |
| 子层级语义 | `ldBase` tree | 见 `base.md` covered/allowlisted 行 | 可作为父级承载子控件，具体树能力以 `base.md` 为准。 |
| 背景移动/平移 | `ldBaseBgMove` | 未闭环 | 当前作为 `base_tree_policy` allowlist，缺少 PicoUI 用户态 window/background pan/move 能力。 |

## 仍需补齐

- 若 100% 目标要求每个 `ldWidgetType_t` 都能由 PicoUI 独立创建和操作，需要新增 `picoui_background_*` 控件能力、backend group、unit/gate。
- 若继续保持 background 由 window/root 派生承载，也必须补齐背景移动/平移能力，不能只把 `ldBaseBgMove` 作为 backend helper allowlist。

## 审计边界

- `background` 算入真实 LingDongGUI 控件类型覆盖。
- `background` 不算一个独立 API 分组；不能虚构 setter/getter 能力。
- 当前没有独立 `picoui_background_*` public API、backend group、unit/gate；这不是代码遗漏，而是 policy 决策。若目标升级为每个 native widget type 都有独立对外控件，需要另开实现线。
- a-0.9 R5 决策：不新增独立 PicoUI background public abstraction；root/background semantics 由 window/tree 派生 policy 覆盖。
- a-0.10 R4 决策：`background` 保持 enum-only/root-window-tree derived policy；不新增 PicoUI background public API。
- `manual_artifact` 存在不等于人工验收通过；background/root 结论以 matrix policy 与 window/backend/unit gate 为准。
