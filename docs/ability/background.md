# `background`

本文记录 `widgetTypeBackground` 的控件类型边界和 PicoUI 覆盖边界。

## 覆盖摘要

- 控件类型来源：`src/gui/ldBase.h` 的 `ldWidgetType_t`。
- LingDongGUI 独立 header/API 分组：无。
- PicoUI 独立 matrix group：无。
- 覆盖结论：不能写成“PicoUI 独立 API 100% 覆盖 background”，因为 LingDongGUI 没有独立 `ldBackground.h` public API；只能通过 scene/root/window/tree 语义间接覆盖。

## 能力边界

| 能力项 | LingDongGUI 来源 | PicoUI 覆盖判断 | 说明 |
| --- | --- | --- | --- |
| 控件类型 | `widgetTypeBackground` | 无独立 PicoUI widget/API 行 | enum 中的独立类型。 |
| 根背景语义 | scene/window tree | 需通过 window/root/container gate 判断 | 作为场景根背景或背景边界参与遍历。 |
| 子层级语义 | `ldBase` tree | 见 `base.md` covered/allowlisted 行 | 可作为父级承载子控件，具体树能力以 `base.md` 为准。 |

## 审计边界

- `background` 算入真实 LingDongGUI 控件类型覆盖。
- `background` 不算一个独立 API 分组；不能虚构 setter/getter 能力。
- 若要证明 PicoUI 对 background 语义 100% 覆盖，必须追加专门的 root/background/container matrix 行和 runtime/backend/manual gate；当前文档不把它包装成已证明的 direct 100%。
