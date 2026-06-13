# TINYUI a-0.7 LingDongGUI 原生 100% 能力对齐设计

## 1. 背景

`a-0.6` 已完成 `26/26 release-target` 控件的 final release contract，但这个结论不是 LingDongGUI 原生能力 100% 覆盖。

当前 1 对 1 审计已经确认：

1. LingDongGUI `ldBase` 枚举里存在 `27` 个 widget-like 类型，`ldAnimation` 未进入 TINYUI。
2. `a-0.6` matrix 仍有 `13` 条 `reject-with-rationale`。
3. 多个 `ld*` public setter/getter/event/resource 能力未进入 TINYUI public contract。
4. gate 主要证明 release contract，不证明每个原生能力逐项 parity。

`a-0.7` 因此必须重定义为 native-100 线，而不是 `a-0.6` 的小修补。

## 2. 目标

`a-0.7` 的唯一目标：

1. 以 `src/gui/ld*.h` 为原生事实源，覆盖全部 widget-like 控件。
2. TINYUI public API 能表达每个 LingDongGUI 原生控件的全部 public 能力。
3. 每个 TINYUI API 都落到真实 LingDongGUI backend 对象和字段，不依赖 fake renderer、demo 私有状态或 host-cache 假读回。
4. `tests/tinyui/contract/tinyui_release_capability_matrix.json` 升级为 `a-0.7-native-100-v1`。
5. matrix 中 `not_wrapped / reject / deferred / incomplete_contract` 全部清零。
6. runtime / mapping / visible / manual artifact gate 都以 native-100 口径重建。

## 3. 非目标

`a-0.7` 明确不做：

1. 不改 LingDongGUI 原生 API。
2. 不以“上层 API 裁剪”为理由保留 reject。
3. 不用 demo 硬编码、固定坐标、fake visual 证明能力。
4. 不把 artifact existence 写成人工验收通过。
5. 不把 `a-0.6 full_parity_complete` 继续当 native-100 完成依据。

## 4. 方案比较

### 方案 A：只把 13 条 reject 转 support

优点：

1. 范围最小。

缺点：

1. `ldAnimation` 仍未覆盖。
2. 未入 matrix 的原生高级能力仍遗漏。
3. gate 仍不证明所有 `ld*` public 能力。

结论：拒绝。

### 方案 B：按控件逐个补齐

优点：

1. 容易派 subagent。
2. 每个控件边界直观。

缺点：

1. image/mask/font/color/align/event/navigation 等 shared bridge 会被重复实现。
2. 容易出现每个控件各自定义 native resource 表达。
3. gate 和 matrix 会反复返工。

结论：不推荐作为主架构。

### 方案 C：truth-source + shared bridge 先行，再按能力组串行补齐

做法：

1. 先从 `src/gui/ld*.h` 建 native inventory。
2. 再建 shared native bridge。
3. 然后按控件能力组串行补齐。
4. 最后统一升级 matrix/gate/docs。

优点：

1. 最接近真实 100%。
2. 能防止每个控件重复造资源/事件/读回模型。
3. 适合长任务 subagent 串行推进。
4. gate 可以从第一天绑定 native inventory，不再靠人工口径。

缺点：

1. 前期文档和 shared-core 工作重。
2. 计划较长，必须严格阶段化。

结论：推荐。

### 方案 D：truth-source + shared bridge 串行，控件批次并行

做法：

1. `R0 / R1 / R2` 串行冻结 native inventory、shared bridge、base/layout/theme/event。
2. `R3A / R3B / R4A / R4B / R5A / R5B / R6A / R6B` 基于同一 frozen `R2` baseline 并行开发。
3. 每个并行批次只负责自己的 header/widget/backend/unit/demo 文件。
4. 所有聚合入口、matrix、gate、manual artifact、docs 由 `R7 / R8` 串行合流。

优点：

1. 能显著缩短控件实现时间。
2. 不牺牲 `R0~R2` 的 shared-core 质量。
3. 共享文件冲突被集中到 `R7`，不让八个 subagent 互相覆盖。

缺点：

1. `R7` 合流压力更大。
2. 每个并行批次必须严格提交 capability fragment，否则 matrix 合流会漏项。

结论：作为实际执行策略推荐。`a-0.7` 的设计仍按依赖链理解，但执行允许 `R3~R6` 并行。

## 5. native-100 范围定义

### 5.1 控件范围

`a-0.7` 覆盖以下 `27` 个 widget-like 控件：

1. `window`
2. `label`
3. `button`
4. `checkbox`
5. `switch`
6. `slider`
7. `text`
8. `image`
9. `list`
10. `progress_bar`
11. `qrcode`
12. `progress_wheel`
13. `message_box`
14. `date_time`
15. `clock`
16. `line_edit`
17. `keyboard`
18. `combo_box`
19. `scroll_selecter`
20. `arc`
21. `gauge`
22. `graph`
23. `table`
24. `calendar`
25. `icon_slider`
26. `radial_menu`
27. `animation`

`widgetTypeBackground / ldBase / ldGui / internal solver / switch internal` 不作为独立控件，但其通用能力必须通过 `tinyui_widget_*`、layout、event、theme、resource API 覆盖。

### 5.2 能力范围

每个控件至少覆盖以下维度：

1. init 参数等价表达。
2. public setter。
3. public getter 或 backend-field proof。
4. 事件和 callback。
5. resource：image tile、mask tile、font、color、align。
6. layout：position、size、grid cell、flex child、ignore layout、window flex/grid。
7. state：hidden、opacity、selectable、selected、corner、enabled/disabled。
8. data model：item、series、cell、calendar grid、date/time、qrcode text、keyboard target。
9. visible proof：不只证明显示，还要证明关键 native 参数影响真实输出或 backend 字段。
10. manual artifact：每个 demo 都有可追踪 artifact，且边界明确。

## 6. 推荐架构

### 6.1 native inventory 先行

新增或升级 matrix schema：

```text
schema_version = a-0.7-native-100-v1
ldgui_widget_like_total = 27
tinyui_native_wrapped_total = 27
native_capability_total = derived from src/gui/ld*.h inventory
native_capability_status_counts.support = native_capability_total
native_capability_status_counts.reject = 0
native_capability_status_counts.deferred = 0
```

matrix 每行必须记录：

1. `widget`
2. `ldgui_symbol`
3. `tinyui_api`
4. `backend_proof`
5. `unit_test`
6. `runtime_gate`
7. `mapping_gate`
8. `visible_gate`
9. `manual_artifact`
10. `status = support`

### 6.2 shared native bridge

`a-0.7` 必须先统一以下类型和桥：

1. `tinyui_native_color`
2. `tinyui_native_align`
3. `tinyui_native_font`
4. `tinyui_native_image_source`
5. `tinyui_native_mask_source`
6. `tinyui_native_resource_pair`
7. `tinyui_native_nav_dir`
8. `tinyui_native_signal`
9. `tinyui_native_callback`
10. `tinyui_native_readback_policy`

这些桥必须在 shared-core 中统一实现，控件 subagent 不得各自发明重复类型。

### 6.3 backend truth policy

每个能力只能属于以下一种：

1. backend field truth：直接读真实 `ld*` 对象字段或 getter。
2. backend committed truth：TINYUI cache 是提交到 backend 的同一份真值，并有 unit proof 证明提交。
3. resource pointer truth：TINYUI resource wrapper 与 backend tile/mask/font 指针一致。

不允许：

1. host cache only。
2. demo local state only。
3. visible only。
4. artifact only。

### 6.4 gate 架构

gate 分四层：

1. `native inventory gate`
   - 从 `src/gui/ld*.h` 抽取控件/API。
   - 校验 matrix 无遗漏。
2. `backend field gate`
   - unit test 逐能力断言真实 backend 字段。
3. `runtime/mapping/visible gate`
   - 每个控件至少一个 native-100 demo。
   - 每个 demo 必须输出 real mapping marker。
4. `manual artifact gate`
   - 每个 native-100 demo 产物有记录。
   - live OS 现场验收若未做，必须明确未做。

## 7. 阶段设计

### R0：native inventory / truth-source

目标：

1. 建立 `a-0.7-native-100` matrix schema。
2. 将 `27` 个 widget-like 控件写入 truth-source。
3. 将 `src/gui/ld*.h` public setter/getter/init 能力逐项写入 native capability rows。
4. 初始状态可为 `missing_implementation`，但最终 closeout 前必须全部转 support。

### R1：native bridge shared-core

目标：

1. 统一 resource/font/color/align/nav/signal/callback/readback 类型。
2. 统一 backend-field proof helper。
3. 统一 visible/mapping marker 输出。
4. 移除后续控件各自造 bridge 的空间。

### R2：base / layout / theme / event 100%

目标：

1. `ldBase` 通用能力通过 `tinyui_widget_*` 覆盖。
2. `ldWindow` flex/grid/padding/gap/grid descriptor 全量覆盖。
3. theme 不再是 image/list 等控件 reject 边界。
4. press/hold/release/clicked_item/finished/value_changed 都有统一 event bridge。

### R3~R6：并行控件批次

`R3~R6` 可以在 `R2` frozen baseline 之后并行执行：

1. `R3A`: `label / text / image`
2. `R3B`: `button / checkbox / switch / slider / list`
3. `R4A`: `line_edit / keyboard / combo_box / scroll_selecter`
4. `R4B`: `table / graph / calendar`
5. `R5A`: `progress_bar / progress_wheel / qrcode`
6. `R5B`: `arc / gauge / date_time / clock`
7. `R6A`: `icon_slider / radial_menu`
8. `R6B`: `message_box / animation`

并行约束：

1. 每个批次一个独立 worktree。
2. 每个批次只能最终提交自己的控件文件和 unit/demo 文件。
3. 每个批次必须产出 `tests/tinyui/contract/native_100_fragments/<batch>.json`。
4. 每个批次可临时改聚合文件做本地验证，但最终合流由 `R7` 统一处理。
5. 任一批次发现 shared bridge 缺口，必须停下并回到主线程调整 `R1/R2`，不能在批次内私改 shared bridge。

### R3：text / image / button / selection old-widget 100%

目标：

1. `label / text / image` 完整文本、字体、颜色、背景图、mask、scroll、static text。
2. `button / checkbox / switch / slider / list` 完整状态、图片 skin、radio group、navigation、item widget、padding/margin。

### R4：input / navigation / data-widget 100%

目标：

1. `line_edit / keyboard / combo_box / scroll_selecter` 完整输入、导航、key map、dropdown/indicator image、finished reason。
2. `table / graph / calendar` 完整单元格、series、axis、grid、calendar day names、颜色/字体/图片/编辑模型。

### R5：visual / progress / instrument / clock / qrcode 100%

目标：

1. `progress_bar / progress_wheel / qrcode` 完整 skin/theme/image/color/ECC/zoom/version/dot 配置。
2. `arc / gauge / date_time / clock` 完整 resource、pointer、trail/progressBar、time/date、align/color/transparent、clock dial/pointer。

### R6：composite / modal / animation 100%

目标：

1. `icon_slider / radial_menu` 完整 icon image/mask、speed、selection/click/default/offset。
2. `message_box` 完整多按钮、callback、string/button/background color、modal behavior，且不再 formal mapping exclusion。
3. `animation` 新增 TINYUI public widget，覆盖 frame image、period、show/update 证据。

### R7：native-100 matrix / gate / manual artifact

目标：

1. matrix 所有 native capability rows 变为 support。
2. `reject / deferred / incomplete_contract / not_wrapped` 全部清零。
3. `keyboard` 不再是 runtime-first 特例。
4. `message_box` 不再是 mapping exclusion 特例。
5. 所有 demo 进入 runtime/mapping/visible/manual artifact catalog。

### R8：closeout / docs / release readiness audit

目标：

1. 中文 docs 全部改为 native-100 口径。
2. closeout 明确列出 proof set。
3. 独立 review 复核代码、matrix、gate、docs 是否一致。

## 8. 完成定义

`a-0.7` 完成必须同时满足：

1. `27/27` 控件覆盖，包含 `animation`。
2. `src/gui/ld*.h` public native API 能力无遗漏。
3. matrix 无 `reject / deferred / incomplete_contract / not_wrapped / missing_implementation`。
4. 每个 capability row 至少有 unit proof。
5. 每个 widget 至少有 runtime/mapping/visible/manual artifact proof。
6. `check_tinyui_release_capability_matrix.py` 能从 native inventory 校验 matrix 完整性。
7. `check_tinyui_public_api.py` 校验所有 native-100 public API。
8. `check_tinyui_backend_mapping.py` 不允许 fake fallback 和 formal mapping exclusion。
9. `check_tinyui_visible_ui.py --all` 覆盖全部 native-100 demos。
10. `check_tinyui_manual_window_artifact.py --all` 覆盖全部 native-100 demos。
11. `git diff --check` 通过。
12. `gitnexus_detect_changes(scope="all", repo="LingDongGUI")` 风险符合预期。

## 9. 风险

1. `a-0.7` 实际上会把 TINYUI 从上层 API 推近 native mirror。风险是 public API 暴涨。
2. image/mask/font 资源桥如果设计不稳，所有视觉控件都会返工。
3. native inventory gate 如果只靠人工维护，很容易漏 API。
4. `message_box / keyboard / animation` 是最容易破坏当前 gate 假设的三类控件。
5. `temporary smoke path` 必须退出 native-100 结论，否则会继续制造假完成。
