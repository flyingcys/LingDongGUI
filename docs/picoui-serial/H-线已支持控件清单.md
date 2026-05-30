# H-线已支持控件清单

## 文档定位

本文是 `H11` 的 supported controls 真相源，只回答当前 first release discussion 范围内，哪些 PicoUI public 控件可以被列入支持清单，以及每个控件当前承认的支持子集。

当前口径仍然是 `internal v0.1 candidate`，不是 `public v1.0`。

完整合同与限制仍以以下文档为准：

- [H-线当前9控件发布合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md)
- [H-线发布差距与LingDongGUI控件对比](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md)
- [H-线第一版发布说明](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线第一版发布说明.md)

## 当前 9 个已支持控件

| 控件 | 当前支持子集摘要 | 详细合同 |
| --- | --- | --- |
| `window` | 容器、flex/grid、padding/gap/align、背景色 | [window 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `label` | 文本、最小 font 映射、背景色、文字色 | [label 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `button` | 文本、clicked/pressed/released、基础样式子集 | [button 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `checkbox` | 文本、checked state、toggle callback、基础样式子集 | [checkbox 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `switch` | checked state、toggle callback、enabled、基础样式子集 | [switch 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `slider` | value、range、value changed callback、基础样式子集 | [slider 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `text` | 文本、最小 font fallback、背景色、文字色 | [text 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `image` | 基础 source 绑定、布局显示、真实 `ldImage` backend 路径；仅基础子集，`style_class/user_data` 仅 metadata-only，详见限制 | [image 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |
| `list` | add item、selected index、真实 `ldList` 文本/选择映射、`on_selected` callback cookie；高风险控件，非 support 项与 metadata-only 限制见合同 | [list 合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md) |

## 当前不在支持清单里的内容

- `image` 的 `theme`、`enabled`、颜色/边框/radius style 与 `padding` 扩展能力不在当前支持清单中；其中 `style_class` 与 `user_data` 只承认 metadata-only 合同，不是真实 backend support。
- `list` 的 `item marker`、更细粒度 per-item widget 语义、advanced style 不在当前支持清单中；其中 `style_class` 与 widget-level `user_data` 都是 metadata-only `incomplete_contract`。
- `list` 的 `on_selected(..., user_data)` 中 `user_data` 只是 callback cookie；它与 widget-level `user_data` 分离，不能混写成“list 已支持 user_data”。
- 完整字体系统、完整 theme/skin 系统、人工窗口验收通过结论。
- 其余 `17` 个未覆盖的 `LingDongGUI` 可封装控件。

## 跳转

- [H-线第一版发布说明](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线第一版发布说明.md)
- [H-线demo-catalog](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线demo-catalog.md)
- [H-线发布测试矩阵](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布测试矩阵.md)
- [picoui_release_capability_matrix.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_release_capability_matrix.json)
