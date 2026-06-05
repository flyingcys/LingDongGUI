# PicoUI 三个 Parity Demo 差距修复计划

**Goal:** 把 `legacy_widget_parity / layout_parity / grid_parity` 从“可对照 baseline”继续推进到更接近老 SDL truth-source 的真实行为、资源和验证闭环。

**Current Truth:** 这轮重新摸底后，真正的 public capability 缺口只有一个：`app` 级 `timer / tick / frame callback`。其它两类工作不是补 API，而是补 demo 资源、页面内容和 dedicated visible 对照。

**Strategy:** 后续顺序要改成 capability-first，而不是继续围绕 `grid_parity` 做局部页面修整。

**Capability Line Docs:**

- `timer capability design`: `docs/superpowers/specs/2026-06-04-picoui-app-timer-tick-capability-design.md`
- `timer capability implementation`: `docs/superpowers/plans/2026-06-04-picoui-app-timer-tick-capability-implementation.md`

1. 先补最小 public `app` timer/tick/frame callback 能力。
2. 再用它补 `layout_parity` 的 `1200ms` 宽度切换，并回收 `legacy_widget_parity` 的 `100ms` arc/gauge 动画。
3. 再给 `legacy_widget_parity` 接老 demo 真实资源。
4. 最后把三页都接进 dedicated visible 对照并细化内容 fidelity。

---

## Task 1: 补最小 public `app` timer/tick capability

**Goal:** 给 demo 层一个合法表达周期行为的 public hook，不把页面行为偷偷写进 backend。

**Files:**
- Modify: `picoui/include/picoui/app.h`
- Modify: `picoui/src/core/app.c`
- Modify: `picoui/src/backend/ldgui/backend_app.c`
- Reference: `examples/common/demo/layout/uiLayout.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`

- [ ] 设计最小 public 接口
  - 可选方向：frame callback、tick callback、timer register
- [ ] 保证该接口是通用 capability，不是只为某个 parity demo 定制
- [ ] 保证 demo 只通过 `picoui_*` public API 使用该能力
- [ ] 为新能力补最小 contract / unit / demo 级验证

约束：

- 不要把 `layout_parity` 的宽度切换硬编码到 backend。
- 不要发明 `layout` 专用或 `legacy widget` 专用 API。
- 若需要改函数/方法实现，下一轮开工前必须先重新跑 GitNexus impact。

## Task 2: 用新 capability 补 `layout_parity` 的运行时行为

**Files:**
- Modify: `picoui/demo/layout_parity/main.c`
- Reference: `examples/common/demo/layout/uiLayout.c`

- [ ] 复刻老页 `1200ms` 周期切换 `flex row` 区域宽度的行为
- [ ] 维持现有 section child-window 结构，不回退成根窗口硬摆放
- [ ] 不把行为写死到 backend；只允许在 demo 层表达页面行为
- [ ] 重新构建并验证

## Task 3: 回收 `legacy_widget_parity` 的剩余真实差距

**Files:**
- Modify: `picoui/demo/legacy_widget_parity/main.c`
- Reference: `examples/common/demo/widget/uiWidgetLegacy.c`
- Reference: `examples/common/demo/resource/uiImages.h`

- [ ] 把共享占位 source 替换成老页对应真实资源
- [ ] 优先接入：
  - `image`
  - `button press/release`
  - `progress bar bg/fg`
  - `text background`
  - `slider bg/indicator`
  - `radial menu / icon slider item`
  - `gauge bg/pointer`
  - `arc quarter source`
- [ ] 用新 timer/tick capability 回收老 truth-source 的 `100ms` arc/gauge 动画
- [ ] 继续补 `table/graph/message_box` 等内容 fidelity

说明：

- 这一步不是为了补 public source API，相关 API 已经存在。
- 真差距是资源 exact-match 和内容细节。

## Task 4: 给三页接 dedicated visible 对照

**Files:**
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Reference: `picoui/demo/legacy_widget_parity/main.c`
- Reference: `picoui/demo/layout_parity/main.c`
- Reference: `picoui/demo/grid_parity/main.c`

- [ ] 把 `legacy_widget_parity`
- [ ] 把 `layout_parity`
- [ ] 把 `grid_parity`
- [ ] 接进 dedicated visible 对照入口
- [ ] 为每页定义最小但有效的 visible 断言
- [ ] 明确 visible gate 是“对照验证”，不是“资源已 exact-match”的替代说法

## Task 5: 细化 `grid_parity` 内容 fidelity

**Files:**
- Modify: `picoui/demo/grid_parity/main.c`
- Reference: `examples/common/demo/layout/uiLayout.c`

- [ ] 保留当前 `[92, content, 1fr] x [54, 66, 1fr]` descriptor 语义
- [ ] 把 `A-G` 简化文案逐步推进到更接近老页的 `window + title + hint` 内容层
- [ ] 不要再把这页当 capability gap；这里只做页面 fidelity 和 visible 收敛

## Task 6: 同步文档

**Files:**
- Modify: `picoui/docs/demo_guide.md`
- Modify: `docs/superpowers/reviews/2026-06-04-picoui-three-parity-demo-gap-audit.md`
- Modify: `docs/superpowers/plans/2026-06-04-picoui-three-parity-demo-gap-repair.md`

- [x] 写清唯一硬 capability gap 是 `app` 级 timer/tick/frame callback
- [x] 写清 `legacy_widget_parity` 和 `grid_parity` 当前不是 API 缺口
- [x] 写清三页都还缺 dedicated visible 对照

## 当前已完成项

- [x] `grid_parity` 的 descriptor 语义和 auto placement 已收紧到更接近老页
- [x] `layout_parity` 的 capability 阻塞已经澄清
- [x] 文档已按当前真相源改写

## 本轮最小验证

- [x] `python3 tests/picoui/contract/check_picoui_demo_boundary.py`
- [x] `cmake --build build --target picoui_grid_parity_demo picoui_layout_parity_demo`
- [x] `git diff --check`
