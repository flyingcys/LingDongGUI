# `canvas`

本文由 `tests/picoui/contract/ldgui_public_api_inventory.json` 和 `tests/picoui/contract/picoui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 `canvas` API 能力和 PicoUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`canvas`
- group_kind：`widget`
- LingDongGUI API 条目数：`8`
- PicoUI 覆盖统计：`allowlisted`: 5, `covered`: 3
- 覆盖结论：a-0.13 item 8 已补齐 PicoUI 用户态 `canvas` 命令式绘制面，真实落到 native `ldCanvas` widget；生命周期和 `show` 仍是 backend-private。

## 用户态能力

| 能力 | LingDongGUI 来源 | PicoUI 覆盖 |
| --- | --- | --- |
| 创建真实 canvas widget | `ldCanvas_init` | `picoui_canvas_create()` |
| 清空命令列表 | `ldCanvasClear` | `picoui_canvas_clear()` |
| 推入绘制命令 | `ldCanvasPushCommand` | `picoui_canvas_fill_rect()`、`picoui_canvas_draw_line()`、`picoui_canvas_draw_image()`、`picoui_canvas_draw_image_scaled()`、`picoui_canvas_draw_text()` |

## 审计边界

- `covered`：必须有真实 PicoUI public API/backend/unit/gate 证据。
- `allowlisted`：`ldCanvas_depose/on_load/on_frame_start/on_frame_complete/show` 仍是 native lifecycle/render-pipeline 内部项，不暴露为 PicoUI public API。
- 当前 `canvas` 提供的是命令式绘制模型，不是用户自定义 native draw hook。
