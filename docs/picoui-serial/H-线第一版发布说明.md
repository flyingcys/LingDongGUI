# H-线第一版发布说明

## 文档定位

本文是 `H11` 的第一版发布说明真相源，用于整理当前 `PicoUI first release preparation` 的用户可读文档包。

当前口径已经改成：

- 这是 `J线` 之后的 `v0.1 parity` 发布准备说明，不是 `public v1.0` 宣布文档。
- `window / label / button / slider` 已完成当前首版对齐收口。
- `checkbox / switch / text / image / list` 仍是已 wrapped、待 `v0.2` 对齐的 backlog。
- 这仍不等于“PicoUI 已完整支持 LingDongGUI”，也不等于“人工窗口验收已通过”。

## 2026-05-30 当前验证状态

- 已实测通过：
  - `ctest --test-dir build --output-on-failure -L picoui`
  - `ctest --test-dir build --output-on-failure -L visible`
  - `ctest --test-dir build --output-on-failure -L mapping`
  - `python3 tests/picoui/contract/check_picoui_public_api.py`
  - `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
  - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets`
  - `python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel`
  - `git diff --check`
- 当前两个 manual artifact 脚本都能产出 `ARTIFACT_READY` 和对应 `frame.ppm`，且 `C-线人工窗口验收记录.md` 已补入基于 artifact 的人眼观察结论。
- 该结论只能认定 `artifact-based visual observation` 已补齐，不能上抬成 `live OS window acceptance passed`，也不能单靠这一层写成 `release ready`。
- 本轮 review blockers 已修复：
  - release matrix gate 现在会校验 `9` 个已覆盖控件保持 `wrapped`、`v0.1/v0.2` parity 分流一致，并校验 `window / label / button / slider` 的关键 parity capability rows 与 `summary` 对账一致。
  - `list` 合同单测已删除跨层指针同一性假合同，只保留当前发布口径需要的 identity 边界。
  - “Darwin + cocoa 自动桌面窗口截图” 不再写成仓库内当前固定证据，只保留为未来可选补强路径。
- 因此当前状态是：自动 gate 主路径已跑通，review blockers 已收口，且最小范围的 artifact-based visual observation 已补齐；但这仍不等于 live OS 窗口验收通过，也不等于发布完成。
- `H13` 的 latest closeout 前状态可见：
  - [H-线发布closeout前状态](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布closeout前状态.md)

## 当前支持范围摘要

当前 release discussion 仍围绕 `9` 个已覆盖控件，但发布口径已拆成两层：

### `v0.1 parity` 范围

- `window`
- `label`
- `button`
- `slider`

### `v0.2 parity backlog`

- `checkbox`
- `switch`
- `text`
- `image`
- `list`

支持范围总览与逐控件一行摘要见：

- [H-线已支持控件清单](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线已支持控件清单.md)
- [H-线当前9控件发布合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md)

机器可读能力矩阵与发布判断真相源见：

- [picoui_release_capability_matrix.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_release_capability_matrix.json)
- [H-线发布差距与LingDongGUI控件对比](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md)

## known limitations

`v0.1` 文案必须明确保留以下限制，不能弱化成“部分支持”或“基本等价”：

- `checkbox / switch / text / image / list`：当前只是 `wrapped but not yet parity-complete`，不能写成“首版已对齐控件”。
- `image`：当前只发布基础 `source/layout/visible` 子集；`theme`、`enabled`、颜色/边框/radius style 为 `reject`，`padding` 为 `deferred`，`style_class/user_data` 只是 metadata-only 合同。
- `list`：当前只发布基础列表能力；`item marker` 为 `reject`，`style_class` 与 widget-level `user_data` 为 metadata-only `incomplete_contract`，不得把 item id/marker 写成真实 backend widget support。
- `font`：`label/text` 只承认最小 font 映射/fallback，不是完整字体 family/size 解析或动态字体资源系统。
- `theme`：当前 theme 只表达颜色/metric 子集，不等于 `LingDongGUI` 的 image/mask/transparent skin 系统。
- `manual artifact`：当前只登记 `picoui_basic_widgets_demo` 与 `picoui_settings_panel_demo` 两个 demo 的 artifact 条目与 artifact-based visual observation 结论；release matrix 里的 `widget.manual_artifact.artifact_entry_exists` 只表达 widget-level 粒度，不直接表达 demo-level artifact 条目是否存在；`artifact-based visual observation != live OS window acceptance passed != release ready`。
- 未覆盖控件：其余 `17` 个 `LingDongGUI` 可封装控件仍是 `not_wrapped`；`line_edit`、`combo_box`、`progress_bar` 只是 `post-H candidate`，不属于当前发布面。

限制详情与发布判断口径见：

- [H-线当前9控件发布合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md)
- [H-线发布差距与LingDongGUI控件对比](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md)
- [C-线人工窗口验收记录](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/C-线人工窗口验收记录.md)

## build / test instructions

发布前构建、测试与互斥关系统一以 H10 测试矩阵为准：

- [H-线发布测试矩阵](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布测试矩阵.md)

最小自动 gate 命令集：

```bash
ctest --test-dir build --output-on-failure -L picoui
ctest --test-dir build --output-on-failure -L visible
ctest --test-dir build --output-on-failure -L mapping
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
```

`manual artifact` 入口：

```bash
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

说明：

- `runtime / visible / mapping` 共享 `build/picoui-runtime`，必须串行执行，不能并行抢同一 build 目录。
- `manual artifact` 不接入强制 CTest，需要人工显式执行。
- `SDL_VIDEODRIVER=dummy` 不能支撑人工 OS 窗口验收结论。

## evidence explanation

第一版文档包必须把证据层分开理解：

- `unit`：单元级行为或边界测试存在。
- `contract`：public API、demo boundary、release matrix 等合同 gate 已存在。
- `mapping`：真实 backend tree / widget id / layout/event/theme 映射存在。
- `visible`：自动可见证据存在；当前仓库内可复现、已固定的主路径是 `SDL_VIDEODRIVER=dummy + PPM readback`。若后续补入真实桌面窗口截图脚本与 artifact 记录，它也只能作为 `visible` 层补强，不改变证据分层。
- `manual artifact`：在 release matrix 中只按 widget-level 粒度记录“该 widget 行是否声明自己的 artifact entry”；demo-level artifact 条目与 artifact-based visual observation 仍以 `C-线人工窗口验收记录.md` 为真相源。

边界约束：

- demo 可运行，不等于 visible 正确。
- automatic visible gate 通过，不等于 artifact-based visual observation 已补齐。
- artifact-based visual observation 已补齐，不等于 live OS 窗口现场验收通过。
- 若未来补入自动桌面窗口截图，它仍属于 `visible` 层自动证据，不等于 live OS 窗口验收通过。
- manual artifact 不能由 runtime、mapping 或 visible gate 代替。

可见 demo 与证据层样本见：

- [H-线demo-catalog](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线demo-catalog.md)
- [picoui/docs/demo_guide.md](/Users/cys/embedded/LingDongGUI/picoui/docs/demo_guide.md)

## public API

本节延续 `H9` 冻结结果，只记录当前公开头文件范围与命名边界，不把轻量 contract gate 写成完整 ABI 审计。

### 当前 public headers

| 头文件 | 当前公开范围 |
| --- | --- |
| `picoui/include/picoui/picoui.h` | 聚合入口，包含全部当前 public headers |
| `picoui/include/picoui/app.h` | 应用生命周期 |
| `picoui/include/picoui/window.h` | 窗口创建与基础属性 |
| `picoui/include/picoui/widget.h` | 通用 widget 属性、回调类型、`struct picoui_font` |
| `picoui/include/picoui/layout.h` | flex/grid 布局 API 与相关枚举 |
| `picoui/include/picoui/theme.h` | theme、part/state/color/metric API |
| `picoui/include/picoui/label.h` | label 控件 |
| `picoui/include/picoui/button.h` | button 控件 |
| `picoui/include/picoui/checkbox.h` | checkbox 控件 |
| `picoui/include/picoui/switch.h` | switch 控件 |
| `picoui/include/picoui/slider.h` | slider 控件 |
| `picoui/include/picoui/text.h` | text 控件 |
| `picoui/include/picoui/image.h` | image 控件 |
| `picoui/include/picoui/list.h` | list 控件 |

### 命名边界

| 类别 | 允许暴露形式 | 当前 gate 约束 |
| --- | --- | --- |
| 函数 | `picoui_*` | 轻量文本模式识别当前可见函数声明头，再断言函数名前缀 |
| 宏 | `PICOUI_*` | 头文件 guard 等公开宏必须使用 `PICOUI_` 前缀 |
| 结构体标签 | `struct picoui_*` | 公开 `struct` 标签不得越过 `picoui_` 边界 |
| 枚举标签 | `enum picoui_*` | 公开 `enum` 标签不得越过 `picoui_` 边界 |
| typedef 回调别名 | `picoui_*` | 公开回调 typedef 名称保持 `picoui_` 前缀 |

禁止泄漏的实现侧命名：

- `ld*`
- `arm_2d_*`
- `SIGNAL_*`

对应 contract gate：

- [check_picoui_public_api.py](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/check_picoui_public_api.py)

## 相关文档导航

- [H-线计划索引](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线计划索引.md)
- [H-线已支持控件清单](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线已支持控件清单.md)
- [H-线当前9控件发布合同](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线当前9控件发布合同.md)
- [H-线demo-catalog](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线demo-catalog.md)
- [H-线发布测试矩阵](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布测试矩阵.md)
- [H-线发布差距与LingDongGUI控件对比](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/H-线发布差距与LingDongGUI控件对比.md)
- [C-线人工窗口验收记录](/Users/cys/embedded/LingDongGUI/docs/picoui-serial/C-线人工窗口验收记录.md)
- [picoui_release_capability_matrix.json](/Users/cys/embedded/LingDongGUI/tests/picoui/contract/picoui_release_capability_matrix.json)
