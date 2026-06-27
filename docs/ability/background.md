# `background`

本文记录 `widgetTypeBackground` 的控件类型边界和 TinyUI 覆盖边界。

## 覆盖摘要

- 控件类型来源：`src/gui/ldBase.h` 的 `ldWidgetType_t`。
- LingDongGUI 独立 header/API 分组：无。
- TinyUI 独立 matrix group：仍无独立 native API group，但 a-0.14 已补独立 `tinyui_background_*` public widget 面。
- 覆盖结论：LingDongGUI 虽然没有独立 `ldBackground.h` public API，但 TinyUI 现在已经提供独立 `background` 用户态控件能力，并继续复用真实 `ldWindow` root/background runtime 语义。
- native/user-facing 100%：当前代码面已闭环；`widgetTypeBackground` 已有独立 TinyUI public widget、runtime 映射与 focused unit 证据。
- direct public API 100%：不适用；`background` 没有独立 native API group，不进入 widget public parity denominator。
- direct_public_covered：`0`
- policy_allowlisted：`0`
- direct_100_gap：`0`
- direct_100_category：`enum_only_semantics` policy，见 `window.md`。

## 能力边界

| 能力项 | LingDongGUI 来源 | TinyUI 覆盖判断 | 说明 |
| --- | --- | --- | --- |
| 控件类型 | `widgetTypeBackground` | `tinyui_background_create` | a-0.14 已补独立 background public widget，runtime 真实复用 native root `ldWindow` 并保持 `widgetTypeBackground`。 |
| 根背景语义 | scene/window tree | `tinyui_background_create` + `tinyui_app_run_background` / `set_background` / `switch_background` | `tinyui_runtime_evidence_create_background` 创建真实 root/background，并可作为 app active root 运行。 |
| 子层级语义 | `ldBase` tree | 见 `base.md` covered/allowlisted 行 | 可作为父级承载子控件，具体树能力以 `base.md` 为准。 |
| 背景图片/颜色 | `ldWindowSetImage`、`ldWindowSetColor` | `tinyui_background_set_source()` / `set_color()` / `get_color()` | 继续复用真实 native root/background 图像与颜色路径。 |
| 背景移动/平移 | `ldBaseBgMove` | `tinyui_background_set_offset()` / `get_offset()` | 真实驱动 native scene root background move。 |

## 控件能力等价已补齐

| 能力 | LingDongGUI 来源 | 当前补齐状态 | 边界 |
| --- | --- | --- | --- |
| 独立创建 background root | `widgetTypeBackground` | a-0.14 已新增 `tinyui_background_create()` | native 仍复用 `ldWindow_init(nameId=0)`，不虚构新的 native background struct |
| 独立运行/切换 background root | scene root/window switching | a-0.14 已新增 `tinyui_app_run_background()` / `set_background()` / `switch_background()` | 本质是 app root 切换的 background 专用 public contract |
| 背景图/颜色/偏移 | `ldWindowSetImage`、`ldWindowSetColor`、`ldBaseBgMove` | a-0.14 已新增 `tinyui_background_set_source()` / `set_color()` / `get_color()` / `set_offset()` / `get_offset()` | 继续走真实 LingDongGUI root/background runtime 行为，不引入 fake renderer |

## 审计边界

- `background` 算入真实 LingDongGUI 控件类型覆盖。
- `background` 不算一个独立 API 分组；不能虚构 setter/getter 能力。
- a-0.14 当前实现没有新增独立 native `ldBackground.h`，而是以 TinyUI public widget 形式复用真实 `ldWindow` root/background 语义。
- 旧的 a-0.9 / a-0.10 “不新增 `tinyui_background_*`” 决策已不再代表当前代码事实；若 contract 三件套仍保留该口径，需要继续同步更新。
- `manual_artifact` 存在不等于人工验收通过；background/root 结论以 matrix policy 与 window/runtime/unit gate 为准。
