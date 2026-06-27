# `canvas`

本文由 `tests/tinyui/contract/ldgui_public_api_inventory.json` 和 `tests/tinyui/contract/tinyui_release_capability_matrix.json` 对齐生成。
本页同时记录 LingDongGUI 原生 `canvas` API 能力和 TinyUI 对应覆盖状态。

## 覆盖摘要

- 能力分组：`canvas`
- group_kind：`widget`
- LingDongGUI API 条目数：`8`
- TinyUI 当前覆盖统计：`allowlisted`: 5, `covered`: 3
- 覆盖结论：a-0.13 item 8 已补齐 TinyUI 用户态 `canvas` 命令式绘制面，真实落到 native `ldCanvas` widget；生命周期和 `show` 仍是 runtime-private。

## 用户态能力

| 能力 | LingDongGUI 来源 | TinyUI 覆盖 |
| --- | --- | --- |
| 创建真实 canvas widget | `ldCanvas_init` | `tinyui_canvas_create()` |
| 清空命令列表 | `ldCanvasClear` | `tinyui_canvas_clear()` |
| 推入绘制命令 | `ldCanvasPushCommand` | `tinyui_canvas_fill_rect()`、`tinyui_canvas_draw_line()`、`tinyui_canvas_draw_image()`、`tinyui_canvas_draw_image_scaled()`、`tinyui_canvas_draw_text()` |

## 审计边界

- `covered`：必须有真实 current public API/unit/runtime/visible gate 证据。
- `allowlisted`：`ldCanvas_depose/on_load/on_frame_start/on_frame_complete/show` 仍是 native lifecycle/render-pipeline 内部项，不暴露为当前 public API。
- 当前 `canvas` 提供的是命令式绘制模型，不是用户自定义 native draw hook。
