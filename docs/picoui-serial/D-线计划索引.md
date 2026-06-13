# TINYUI D线计划索引

- `A线` 收口索引：`docs/tinyui-serial/A-线计划索引.md`
- `B线` 收口索引：`docs/tinyui-serial/B-线计划索引.md`
- `C线` 收口索引：`docs/tinyui-serial/C-线计划索引.md`
- `F线` 并行索引：`docs/tinyui-serial/F-线计划索引.md`
- `G线` 收口索引：`docs/tinyui-serial/G-线计划索引.md`
- `TINYUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-tinyui-abstraction-layer-design.md`
- `TINYUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-tinyui-lingdonggui-test-architecture-design.md`
- `D线` 总设计真相源：`docs/superpowers/specs/2026-05-29-tinyui-d-line-current-widget-completeness-design.md`
- `D线` 总实施口径：`docs/superpowers/plans/2026-05-29-tinyui-d-line-current-widget-completeness-implementation.md`
- `F线` 总设计真相源：`docs/superpowers/specs/2026-05-29-tinyui-f-line-new-widget-vertical-slice-design.md`
- `F线` 总实施口径：`docs/superpowers/plans/2026-05-29-tinyui-f-line-new-widget-vertical-slice-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `A线` 已完成真实 `TINYUI -> LingDongGUI` backend 主线。
- `B线` 已完成 6 个 demo 的 automatic visible gate。
- `C线` 已完成门禁工程化、CTest 接入、mapping matrix、可选 manual artifact gate 和长期新增规则。
- `D线` 已完成当前控件完整性主线：runtime layout、合同矩阵、props、state/event、image 边界、theme token v1 和 closeout gate 均已收口。
- `F线` 已完成 `tinyui_list` 最小 vertical slice，并在当前主线代码中通过 backend mapping 与 automatic visible gate。
- 当前完成态不等于“TINYUI 100% 支持 LingDongGUI 对应控件全部能力”；它表示已完成控件都有真实 `LingDongGUI` backend 对象和已记录、可测试的 TINYUI 合同子集。

## D线目标

**D线目标**：把 TINYUI 从“基础控件和门禁已闭环”推进到“当前控件合同完整、事件/状态/theme/layout 语义可持续扩展，并为后续新控件建立高质量并行推进模式”。

## 路线判断

- 第一优先级：修掉 runtime present 路径仍用 cursor 线性排布覆盖真实 flex/grid 结果的问题。
- 第二优先级：把当前控件做完整。
- 第三优先级：补齐当前控件矩阵之后，再快速推进新控件。
- 不建议先追求新控件数量，因为当前 `button/checkbox/switch/slider/label/text/image/window` 虽然已经有真实 backend 和 gate，但 runtime present 布局、props、状态、禁用态、错误语义、theme/style 覆盖、事件合同仍需要形成更完整的用户级 API 质量。
- 可以并行推进多个任务，但必须按写面隔离：
  - 同一个任务内部串行：先 spec/contract，再 test，再 backend，再 demo/gate，再 doc。
  - 多个任务之间可并行：只有当 public API、backend、demo/gate、docs 的写面不重叠时才并行。
  - review 后修复必须回到原 subagent，不另开新 subagent，也不在主线程顺手修。
- 可以同时开 `D线` 和 `F线` 两个 worktree 并行：
  - `D线` 负责当前控件完整性、runtime present layout、state/event/theme/image 等 shared quality 主线。
  - `F线` 负责新控件候选的 spec、原型或独立新控件 vertical slice。
  - `D线` 内部串行，`F线` 内部串行；`D/F` 之间只有写面不重叠时并行。

## 当前不变规则

- demo 只表达用户意图，只能使用 `tinyui_*` API。
- 禁止通过 demo 侧 `set_size()` / `set_pos()` 或硬编码补丁掩盖 backend/layout/theme 缺口。
- 禁止把 `runtime/capture` 非空、`ctest -L tinyui` 或 manual artifact 单独说成完整 UI 完成。
- 新增 demo/widget/layout/theme 必须同步 smoke、visible、mapping、manual artifact 的适用矩阵或豁免说明。
- 多 worktree 并行时，统一创建在项目根目录 `.worktree/`，创建或切换后必须同步并更新 submodule。

## D/F 双线并行边界

### 建议 worktree

- `D线`：`.worktree/tinyui-d-current-widgets`
- `F线`：`.worktree/tinyui-f-new-widgets`

### D线写面

- 当前控件完整性：
  - `tinyui/include/tinyui/{window,label,button,checkbox,switch,slider,text,image,widget,theme,layout}.h`
  - `tinyui/src/widgets/*`
  - `tinyui/src/core/widget.c`
- shared backend 质量：
  - `tinyui/src/backend/ldgui/backend_app.c`
  - `tinyui/src/backend/ldgui/backend_layout.c`
  - `tinyui/src/backend/ldgui/backend_event.c`
  - `tinyui/src/backend/ldgui/backend_style_apply.c`
  - `tinyui/src/backend/ldgui/backend_theme.c`
- 当前 gate：
  - `tests/tinyui/unit/*`
  - `tests/tinyui/runtime/check_tinyui_visible_ui.py`
  - `tests/tinyui/runtime/check_tinyui_backend_mapping.py`

### F线写面

- 新控件候选 spec：
  - `docs/superpowers/specs/*tinyui-f-line*`
  - `docs/superpowers/plans/*tinyui-f-line*`
- 若进入代码 vertical slice，只允许新增或修改该新控件独立文件：
  - `tinyui/include/tinyui/<new_widget>.h`
  - `tinyui/src/widgets/<new_widget>.c`
  - `tinyui/src/backend/ldgui/backend_<new_widget>.c`
  - `tinyui/demo/<new_widget>*/main.c`
  - `tests/tinyui/unit/test_tinyui_<new_widget>.c`
  - `tests/tinyui/runtime/*` 中该新 demo 的矩阵项

### 冲突规则

- `F线` 不得修改 `backend_app.c`、`backend_layout.c`、`backend_event.c`、`backend_style_apply.c`、`backend_theme.c`，除非 `D线` 已完成对应阶段并明确释放写面。
- `F线` 不得修改现有 demo 的用户意图来绕过 D 线缺口。
- `D线` 不得顺手实现 F 线新控件；最多在合同矩阵里预留状态。
- 如果两个 worktree 都需要改 `tinyui/include/tinyui/tinyui.h`、`tests/tinyui/CMakeLists.txt` 或 runtime matrix，必须由主线程在合并阶段串行整合。
- `D/F` 分别可以由不同 subagent 推进，但每条线内部仍按自己的任务顺序串行。

## 接下来最重要的 10 件事

### P0-1 runtime present 真实 layout 闭环

- 目标：让 runtime present 路径使用真实 `LingDongGUI` flex/grid 布局结果，而不是在 `backend_app.c` 里用固定 padding、row height、row gap 和 cursor 线性重排所有 widget；随后补齐 flex/grid 的用户语义边界。
- 当前状态：已完成。
- 完成证据：
  - `tinyui/src/backend/ldgui/backend_app.c` 已把全局 cursor fallback 收缩为 `temporary smoke path`，当 root 是真实 `layoutFlex` / `layoutGrid` 时不再覆盖 flex/grid 布局结果。
  - `tinyui_backend_render()` 仍保持 `ldGuiFrameStart()` / `ldMsgProcess()` / `ldGuiDraw()` / `ldGuiFrameComplete()` 真实绘制链。
  - `tests/tinyui/runtime/check_tinyui_visible_ui.py` 已为 `layout_flex` / `layout_grid` 增加结构断言；`layout_grid` 使用多 y 窗口扫描独立 column groups，不再只依赖非黑、bounds 或 marker。
  - `tests/tinyui/unit/test_tinyui_layout.c` 已覆盖 padding / gap / grid cell span / align / layout dirty / relayout；`tinyui_widget_set_padding()` 已同步到真实 backend，并覆盖 flex/grid layout type 切换后的 padding 保持。
  - D1 fresh 验证通过：`ctest --test-dir build -R test_tinyui_layout --output-on-failure`、`python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_flex`、`python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo layout_grid`、`ctest --test-dir build -L visible --output-on-failure`、`python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py`、`git diff --check`。
  - D1 spec review 和 code quality review 已通过。
  - GitNexus impact / detect_changes 当前未能映射该 linked worktree 的 static backend 写面；本阶段以源码 diff、独立 review 和 targeted tests 作为权威证据。
- 验收：
  - `layout_flex` 和 `layout_grid` 的可见结构断言不再只依赖非黑、bounds、marker。
  - runtime present 不再用一条全局线性 cursor 覆盖容器级 flex/grid 布局。
  - 若仍需 temporary fallback，必须只对无 layout 语义的 smoke/demo 明确局部启用，并在文档标注。
  - `tests/tinyui/unit/test_tinyui_layout.c` 覆盖 padding / gap / grid cell span / align / layout dirty 的支持或拒绝语义。
  - `layout_flex` / `layout_grid` demo 不靠固定坐标补视觉。
- 写面：
  - `tinyui/include/tinyui/layout.h`
  - `tinyui/src/backend/ldgui/backend_app.c`
  - `tinyui/src/backend/ldgui/backend_layout.c`
  - `tinyui/src/layout/*.c`
  - `tests/tinyui/runtime/check_tinyui_visible_ui.py`
  - `tests/tinyui/unit/test_tinyui_layout.c`
  - `docs/tinyui-serial/D-线计划索引.md`
- 并行性：必须先做，阻塞所有“layout 已完整可见闭环”的后续声明；任务内部串行。

### P0-2 当前控件合同矩阵

- 目标：为 `window/label/button/checkbox/switch/slider/text/image` 建立一张完整 contract matrix。
- 必须回答：
  - create / create_with_props 是否齐全。
  - text/value/checked/range/source/user_data/style_class 是否有 setter/getter 或明确拒绝。
  - enabled/visible/focus/dirty/layout/theme/event 的支持状态。
  - 哪些语义已由 unit、mapping、visible 或 manual artifact 覆盖。
- 写面：
  - `docs/tinyui-serial/D-线计划索引.md`
  - `docs/superpowers/specs/*tinyui-d-line*`
  - `tests/tinyui/contract/*`
- 并行性：可在 P0-1 设计稳定后并行做文档/contract 摸底；阻塞后续控件 API 实现任务。

### P0-3 当前控件 props 补齐

- 目标：先补当前控件的 `create_with_props` 一致性，而不是先加新控件。
- 当前状态：已完成。
- 完成证据：
  - `window/label/text/image` 已新增 props struct 与 `create_with_props`；`button/checkbox/switch/slider` props 字段已补齐。
  - `tests/tinyui/unit/test_tinyui_widgets.c` 覆盖 8 个当前控件 props 初值、direct style 字段、size/padding/user_data、交互控件 callback/user_data，以及 invalid props 不污染 backend tree。
  - public header 仍只暴露 `tinyui_*` API/类型，未泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
  - D3 spec review 和 code quality review 已通过。
- 优先顺序：
  - `label/text/image/window`
  - `checkbox/switch/slider`
  - `button`
- 验收：
  - public header 不泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
  - `tests/tinyui/unit/test_tinyui_widgets.c` 覆盖 props 初值。
  - `check_tinyui_backend_mapping.py` 仍通过。
- 写面：
  - `tinyui/include/tinyui/*.h`
  - `tinyui/src/widgets/*.c`
  - `tests/tinyui/unit/test_tinyui_widgets.c`
- 并行性：任务内部串行；不要和 P0-4 同时改同一批 widget 源文件。

### P0-4 enabled / visible / state 语义闭环

- 目标：把通用 widget state 从“字段存在”推进到可测、可见、可映射的行为合同。
- 当前状态：已完成。
- 完成证据：
  - `tinyui_widget_set_visible()` 同步真实 `ldBaseSetHidden()`，`tinyui_widget_set_enabled()` 保留通用 TINYUI state，并对 switch 同步真实 disabled 字段。
  - backend direct/native event dispatch 对 disabled/hidden 控件返回 no-op，不触发 callback、不推进 `dispatch_count`。
  - `tests/tinyui/unit/test_tinyui_button_events.c` 覆盖 disabled/hidden event gate、checkbox/switch/slider native callback user_data、setter-path 与 native-event-path 区分。
  - D4 spec review 首轮发现 hidden event gate 覆盖不足，已由原 subagent 修复；D4 spec/code quality review 已通过。
- 范围：
  - `tinyui_widget_set_enabled`
  - `tinyui_widget_set_visible`
  - state 到 theme/style apply 的同步
  - disabled 控件事件是否被拦截或明确暂不拦截
- 验收：
  - unit test 覆盖 state 改变。
  - theme test 覆盖 disabled/checked/value 相关视觉 token。
  - visible gate 至少有一个 demo 样本能读回差异，或文档写明不可由 readback 判定。
- 写面：
  - `tinyui/src/core/widget.c`
  - `tinyui/src/backend/ldgui/backend_style_apply.c`
  - `tinyui/src/backend/ldgui/backend_event.c`
  - `tests/tinyui/unit/test_tinyui_theme.c`
  - `tests/tinyui/unit/test_tinyui_button_events.c`
- 并行性：可与 P1 文档/探索并行，不可与 P0-3 同时改相同 widget state API。

### P0-5 事件合同扩展

- 目标：把事件合同从当前 button/widgets 样本扩展到所有交互控件。
- 当前状态：已完成。
- 完成证据：
  - `tests/tinyui/unit/test_tinyui_button_events.c` 覆盖 button pressed/released/clicked，以及 checkbox/switch/slider value changed 的 native event path 和 user_data。
  - setter-path 只同步状态，不触发 callback；native-event-path 触发 callback 并传递对应 user_data。
  - disabled/hidden 控件事件被 backend event bridge 拦截。
- 范围：
  - `checkbox` toggled
  - `switch` toggled
  - `slider` value_changed
  - callback 的 user_data 传递
  - setter-path 与 native-event-path 的区别
- 验收：
  - `tests/tinyui/unit/test_tinyui_button_events.c` 或新建事件测试覆盖所有交互控件。
  - backend event bridge 不引入 fake fallback。
- 写面：
  - `tinyui/src/backend/ldgui/backend_event.c`
  - `tinyui/src/widgets/checkbox.c`
  - `tinyui/src/widgets/switch.c`
  - `tinyui/src/widgets/slider.c`
  - `tests/tinyui/unit/*event*.c`
- 并行性：任务内部串行；可与 P0-2 之后的 P1-8 新控件 spec 并行，但不能与 P0-4 同时改 `backend_event.c`。

### P1-6 image 能力边界收口

- 目标：决定 `image` 是继续保持“source 绑定 + 占位可见”，还是进入真实图片资源加载线。
- 推荐：先收口当前边界，不直接做复杂资源系统。
- 当前状态：已完成，口径为 source 指针绑定边界，不进入资源加载系统。
- 完成证据：
  - `tinyui_image_create()` / `tinyui_image_create_with_props()` 均允许空 source 创建未绑定 image。
  - `tinyui_image_set_source(image, NULL)` 清空绑定；非空 source 必须提供 `img_tile`，`mask_tile == NULL` 表示无遮罩。
  - backend 同步真实 `ldImage` 的 `ptImgTile/ptMaskTile`；runtime render 不再为无 source image 注入 placeholder mask。
  - `tests/tinyui/unit/test_tinyui_widgets.c` 覆盖空 source、清空 source、无效 source、无遮罩 source、image theme reject 不污染 source/backend tile。
  - `check_tinyui_visible_ui.py --demo basic_widgets` 锁定 runtime 后 `logo:img=null,mask=null`，避免 visible gate 误把占位资源当作真实 source。
  - 文档明确 automatic visible gate 不证明占位资源绑定，也不证明真实图片加载完成。
- 必须明确：
  - 支持哪些 source 类型。
  - 不支持时返回什么错误或保持什么 no-op。
  - theme/style apply 当前拒绝是否继续保留。
- 验收：
  - unit test 锁定 source、空 source、无效 source 行为。
  - visible gate 中 `basic_widgets` 的 image 样本不产生误导。
  - 文档不把占位显示写成真实图片加载完成。
- 写面：
  - `tinyui/include/tinyui/image.h`
  - `tinyui/src/widgets/image.c`
  - `tinyui/src/backend/ldgui/backend_image.c`
  - `tests/tinyui/unit/test_tinyui_widgets.c`
  - `tinyui/docs/demo_guide.md`
- 并行性：可独立推进，但不要和 P0-3 同时改 `image` API。

### P1-7 theme token v1

- 目标：把 theme 从最小可测语义推进到稳定 token v1。
- 当前状态：已完成。
- 完成证据：
  - `tinyui_theme_apply_to_widget()` 在支持控件上应用 `text/bg/panel/border/accent/disabled` color token，以及 `padding/radius/control_height` metric token。
  - `PICOUI_METRIC_BORDER_WIDTH` 当前仅存储/校验，backend 同步 deferred，未写成真实边框宽度已闭环。
  - `tests/tinyui/unit/test_tinyui_theme.c` 读取真实 `ld*` 字段验证 window/label/text/button/checkbox/switch/slider 的 backend style/metric 同步，并锁定不支持 part/state 和 image theme reject。
  - `theme_showcase` demo 只表达 theme 用户意图，不承担 backend 补丁。
  - D6 spec/code quality review 已通过。
- 范围：
  - 文本/背景/边框/强调/禁用/错误色。
  - 默认间距和控件高度。
  - state/part 的支持矩阵。
- 验收：
  - `test_tinyui_theme.c` 覆盖每个支持 token。
  - 不支持的 part/state 明确拒绝。
  - `theme_showcase` demo 只表达用户意图，不承担 backend 补丁。
- 写面：
  - `tinyui/include/tinyui/theme.h`
  - `tinyui/src/theme/theme.c`
  - `tinyui/src/backend/ldgui/backend_theme.c`
  - `tinyui/src/backend/ldgui/backend_style_apply.c`
  - `tests/tinyui/unit/test_tinyui_theme.c`
- 并行性：可与 P1-8 的新控件 spec 并行；不能与 P0-4 同时改 style apply。

### P1-8 新控件候选 spec

- 目标：在不打断当前控件完整性的前提下，为下一批控件做并行只读设计；该任务可以升级为独立 `F线`。
- 候选顺序：
  - `line_edit`
  - `list`
  - `combo_box`
  - `table`
  - `keyboard`
  - `arc`
- 推荐第一批只选 1-2 个：`line_edit` 和 `list`。
- 选择理由：
  - `line_edit` 直接提升设置页/表单能力，但事件、输入法、focus 成本高。
  - `list` 能快速提升导航/菜单能力，显示和 mapping 成本较低。
  - `combo_box/table/keyboard/arc` 暂缓，避免输入、复杂布局或绘制语义过早扩面。
- 写面：
  - 只写 `docs/superpowers/specs/*tinyui-new-widgets*`
  - 不改 public header 或 backend。
- 并行性：可在 `.worktree/tinyui-f-new-widgets` 中并行给独立 subagent 做；只读 spec 阶段不抢 D 线写面。若 F 线进入代码 vertical slice，必须遵守上面的 D/F 写面隔离。

### P1-9 demo 质量矩阵

- 目标：把 demo 从“能证明 gate”提升到“能代表真实用户任务”。
- 范围：
  - `basic_widgets`：控件合同样板。
  - `settings_panel`：表单/设置页样板。
  - `layout_flex/layout_grid`：布局样板。
  - `theme_showcase`：theme token 样板。
- 验收：
  - 每个 demo 明确服务哪个 gate。
  - demo 不出现 `ld*`、`arm_2d_*`、`SIGNAL_*`。
  - demo 不靠固定坐标补 backend 缺口。
- 写面：
  - `tinyui/demo/*/main.c`
  - `tests/tinyui/runtime/check_tinyui_visible_ui.py`
  - `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
  - `tinyui/docs/demo_guide.md`
- 并行性：必须等对应能力任务完成后串行更新，不要提前美化 demo。

### P2-10 CI 与人工验收策略

- 目标：把本地 gate 进一步接入 CI 或形成稳定人工验收节奏。
- 推荐顺序：
  - 先只记录 CI 设计，不改现有 pack/release workflow。
  - 再决定是否新增测试 workflow。
  - manual artifact 继续保持可选，不进入无窗口 CI。
- 验收：
  - CI 文档复用 C 线 gate matrix。
  - 不把 manual artifact 变成无窗口环境的硬 blocker。
  - 每次 release 前能明确跑哪些 gate。
- 写面：
  - `docs/superpowers/specs/*tinyui-ci-gates*`
  - 可选 `.github/workflows/*`
- 并行性：可在 P0/P1 后期并行设计；不建议先实现。

## 推荐推进顺序

1. 串行做 `P0-1`，先解决 runtime present 线性布局覆盖真实 flex/grid 结果的问题。
2. 做 `P0-2`，冻结当前控件合同矩阵。
3. 根据矩阵拆 `P0-3`、`P0-4`、`P0-5`、`P1-6`，但同一批源文件不得并行写。
4. `P1-8` 已通过 F线落地 `tinyui_list` 最小 vertical slice；后续新控件继续按独立线推进，不能抢 D 线主线。
5. 当前控件合同稳定后，再进入 `P1-7` 和 `P1-9`。
6. `P2-10` 放到 D 线后段，避免 CI 先行导致门禁和能力矩阵反复改。

## 并行模型

- 主线程职责：
  - GitNexus impact / detect_changes。
  - 阶段决策。
  - 文档和结果收敛。
  - 最终验证。
- subagent 职责：
  - 每个 P0/P1 任务一个 fresh subagent。
  - 代码 review、文档 review、评审后修复都用独立 subagent 或回原 subagent。
  - 多个 subagent 并行时，写面必须不重叠。
- 任务内部顺序：
  - spec
  - contract test
  - implementation
  - runtime/mapping/visible gate
  - docs
  - review
  - final verification

## 退出口径

`D线` 只有在下面条件满足后才可收口：

- runtime present 不再用全局 cursor 线性排布覆盖真实 flex/grid 布局结果，或剩余 fallback 已被明确限定为 temporary smoke path。
- 当前控件合同矩阵已完成，并且每个支持/拒绝项都有测试或文档证据。
- 当前控件 props、state、event、image、layout、theme 的 P0/P1 项完成或明确移出范围。
- 新控件候选已有 spec，但未抢跑实现主线。
- 6 个 demo 的 smoke、mapping、visible gate 仍全部通过。
- 文档继续区分 smoke、backend mapping、automatic visible、manual artifact。

## D/F 后当前总口径

当前已完成 TINYUI 控件：`window`、`label`、`button`、`checkbox`、`switch`、`slider`、`text`、`image`、`list`。

这些控件均应按“TINYUI 上层合同子集 + 真实 LingDongGUI backend 映射”理解，不应按“逐项完整复制底层 `ld*` 控件 API”理解。下一阶段若目标是“完整当前控件”，必须先建立逐控件能力差距矩阵：

- 对每个 `tinyui_*` 控件列出对应 `ld*` 控件已有能力。
- 标记 TINYUI 已封装、明确拒绝、暂缓、缺测试的能力。
- 对缺口按用户价值排序，优先补事件、输入、focus、动态 item、资源加载、theme/style 这类会影响真实应用的能力。
- 每新增一个能力，都必须同步 public API、backend 映射、unit/contract、mapping/visible gate 和文档，不能只用 demo 外观证明完成。

已知优先缺口：

- `list`：拆分 list item marker 语义，补或明确拒绝 native selection event bridge。
- `slider`：补 `tinyui_slider_set_range()` clamp 后同步底层 `ldSlider` percent 的实现和测试，或在合同矩阵中降级为 deferred。
- `style_value`：继续保持“TINYUI 字段合同”口径；若要变成实时 backend style setter，必须补真实 `ld*` 字段同步和 visible/theme 证据。
- `button/checkbox/switch/slider/label/text/image/list`：基于对应 `ld*` header 建 capability gap matrix，避免把当前合同子集写成 100% 全量封装。
- 上述缺口进入 `G线` 统一收口；`G线` 真相源负责回答“哪些能力已封装、哪些只是子集、哪些公开 API 仍未兑现合同”。
