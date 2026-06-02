# `background`

本文记录 `widgetTypeBackground` 的控件类型边界和 PicoUI 覆盖边界。

## 覆盖摘要

- 控件类型来源：`src/gui/ldBase.h` 的 `ldWidgetType_t`。
- LingDongGUI 独立 header/API 分组：无。
- PicoUI 独立 matrix group：无；a-0.9 通过 `window` group 的 `enum_only_semantics` policy 行记录 root/background 语义。
- 覆盖结论：不能写成“PicoUI 独立 API 100% 覆盖 background”，因为 LingDongGUI 没有独立 `ldBackground.h` public API；当前正式定义为 `window/tree derived enum-only` policy。

## 能力边界

| 能力项 | LingDongGUI 来源 | PicoUI 覆盖判断 | 说明 |
| --- | --- | --- | --- |
| 控件类型 | `widgetTypeBackground` | `enum_only_semantics` policy | enum 中的独立类型，但无独立 public API group。 |
| 根背景语义 | scene/window tree | `picoui_window_create` + window background/color/layout API | `picoui_backend_create_window` 创建真实 `ldWindow` root，window API 覆盖用户态 root/background 意图。 |
| 子层级语义 | `ldBase` tree | 见 `base.md` covered/allowlisted 行 | 可作为父级承载子控件，具体树能力以 `base.md` 为准。 |

## 审计边界

- `background` 算入真实 LingDongGUI 控件类型覆盖。
- `background` 不算一个独立 API 分组；不能虚构 setter/getter 能力。
- a-0.9 R5 决策：不新增独立 PicoUI background public abstraction；root/background semantics 由 window/tree 派生 policy 覆盖。
- `manual_artifact` 存在不等于人工验收通过；background/root 结论以 matrix policy 与 window/backend/unit gate 为准。
