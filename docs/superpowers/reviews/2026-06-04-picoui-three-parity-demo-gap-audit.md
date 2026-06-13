# TINYUI 三个 Parity Demo 对老 SDL 页面差距审计

## 结论

当前 `tinyui/demo/legacy_widget_parity`、`tinyui/demo/layout_parity`、`tinyui/demo/grid_parity` 都已经具备“可对照 baseline”，但都还不能写成与老 SDL 页面完成 parity。

这轮重新按当前源码和独立 subagent 摸底结论收口后，可以把差距分成两类：

1. 真正缺少的 public capability
   - 只有一个高优先级硬缺口：`app 级 timer / tick / frame callback`，或等价的 public 定时器/loop hook。
2. 不缺 capability，但还缺 demo/资源/验证
   - `legacy_widget_parity` 主要缺老资源 exact-match 和内容细节。
   - `layout_parity` 静态结构不缺，只缺运行时 `1200ms` 宽度切换。
   - `grid_parity` descriptor 语义已补上，剩余主要是内容 fidelity 和 visible 对照。
   - 三页都还缺 dedicated visible 对照，不是 public API 缺口。

## 真相源

1. 老 SDL truth-source
   - `examples/common/demo/widget/uiWidgetLegacy.c`
   - `examples/common/demo/layout/uiLayout.c`
2. 当前 TINYUI parity 页面
   - `tinyui/demo/legacy_widget_parity/main.c`
   - `tinyui/demo/layout_parity/main.c`
   - `tinyui/demo/grid_parity/main.c`
3. 当前 TINYUI public API / backend 证据
   - `tinyui/include/tinyui/app.h`
   - `tinyui/src/core/app.c`
   - `tinyui/src/backend/ldgui/backend_app.c`
   - `tinyui/include/tinyui/layout.h`
   - `tinyui/include/tinyui/widget.h`
   - `tinyui/include/tinyui/image.h`
   - `tinyui/include/tinyui/button.h`
   - `tinyui/include/tinyui/progress_bar.h`
   - `tinyui/include/tinyui/text.h`
   - `tinyui/include/tinyui/slider.h`
   - `tinyui/include/tinyui/radial_menu.h`
   - `tinyui/include/tinyui/icon_slider.h`
   - `tinyui/include/tinyui/gauge.h`
4. 当前验证入口
   - `tests/tinyui/runtime/check_tinyui_visible_ui.py`
   - `tinyui/docs/demo_guide.md`

## 硬缺 capability

### 1. `app` 级 timer / tick / frame callback

这是当前唯一可以诚实标成“缺 public capability”的点。

证据：

1. 老 `layout` 页在 `uiLayoutLoop()` 里用 `ldTimeOut(1200, true, &s_layout_resize_timer)` 周期性切换 `flex row` 宽度，并直接改 `ldBaseSetWidth(...)`。
2. 老 `legacy widget` 页也用 `ldTimeOut(100, true)` 驱动 `arc/gauge` 动画。
3. TINYUI public app 面目前只有 `create/run/set_window/switch_window/destroy` 这一层，没有 callback/tick/timer 注册接口。
4. `tinyui_app_run()` 只是进入 backend loop。
5. backend runtime page 的 `.loop/.frameStart/.frameComplete` 也没有 public hook 暴露给 demo 使用。

判断：

1. `layout_parity` 缺的不是 layout/width API，而是合法表达老页运行时行为的 public loop capability。
2. 这一能力一旦补通，会同时解锁：
   - `layout_parity` 的 `1200ms` 宽度切换
   - `legacy_widget_parity` 里老 truth-source 的 `100ms` arc/gauge 动画

## 分页审计

### 1. `legacy_widget_parity`

#### 已对齐到的层级

1. 页面已包含老 `legacy-widget` 页的大部分核心控件样本。
2. `radial_menu / icon_slider / qrcode / gauge / line_edit / keyboard / arc / list item widget / child-window` 都已进入页面。
3. `image / button / progress bar / text / slider / radial menu / icon slider / gauge / arc` 都已走通 TINYUI public source 级 API。

#### 当前差距

1. `image/button/progress/text/slider/radial_menu/icon_slider/gauge/arc` 当前绑定的是共享占位 source，不是老页真实资源。
2. 老页使用的真实资源包括：
   - `IMAGE_LETTER_PAPER_BMP`
   - `IMAGE_KEYRELEASE_PNG / IMAGE_KEYPRESS_PNG`
   - `IMAGE_PROGRESSBARBG_BMP / IMAGE_PROGRESSBARFG_BMP`
   - `IMAGE_SLIDER_PNG / IMAGE_INDICATOR_PNG`
   - `IMAGE_WEATHER_PNG / IMAGE_NOTE_PNG / IMAGE_BOOK_PNG / IMAGE_CHART_PNG`
   - `IMAGE_GAUGE_PNG / IMAGE_GAUGEPOINTER_PNG_Mask`
   - `IMAGE_ARC_QUARTER_PNG_Mask / IMAGE_ARC_QUARTER_MASK_PNG_Mask`
3. `table` 仍是 `3x3` 样例，不是老页的 `10x6` Excel 风格表。
4. `graph` 仍是简化静态数据，不是老页两组 16 点数据。
5. `message_box` 按钮数量和文本未对齐老页。
6. `qrcode / calendar / date_time / combo_box / list` 的内容文本与老页仍有差异。
7. 老页 `arc/gauge` 还有 `100ms` 动画驱动，这部分目前也卡在 app 级 timer/tick capability 缺口。

#### 能力判断

1. exact-match 图片资源绑定不是 capability 缺口。
2. 相关 public source API 已经齐，backend 也是真落到底层 `ld*SetImage`。
3. 当前缺的是 demo 真实资源接入和内容 fidelity，不是图片型 public API 缺失。

#### 判断

`legacy_widget_parity` 当前更接近“控件覆盖 + source 通路闭环”，还不是“资源和页面细节 parity”。

### 2. `layout_parity`

#### 已对齐到的层级

1. `flex row / flex column / legacy row / legacy column` 四组结构都已拆成独立 child-window section。
2. `new track / min-width / grow / ignore-layout / hidden middle gap / badge` 等关键结构都已进入页面。
3. 这页已经不再是“根窗口上硬摆一堆控件”，而是有真实 section 容器层级。
4. 静态布局所需 public API 已经存在，backend 也是真映射到 `ldWindowSetFlex*` 路径。

#### 当前差距

1. 老页 `uiLayoutLoop()` 每 `1200ms` 会在 `170` 和 `220` 之间切换 `flex row` 区域宽度。
2. TINYUI parity 页面目前没有对应 loop / timer 行为，只是静态页面。
3. 当前阻塞不是 demo 忘了写，而是 TINYUI public app/demo 层还没有 timer / tick / frame callback。
4. 老页页面标题和 hint 文案更贴近行为描述；TINYUI 版文案更偏 section 结构解释。
5. TINYUI 版为了增强结构可读性，引入了 section shell，这使结构更清楚，但不再是对老页外壳的逐像素复刻。

#### 能力判断

1. `layout_parity` 的静态布局能力本身不缺。
2. 真缺口只有运行时 `1200ms` 驱动能力。

#### 判断

`layout_parity` 当前是三页里结构最接近老页的，但仍缺一个真实运行时行为闭环。

### 3. `grid_parity`

#### 已对齐到的层级

1. 页面已经保留 `A-G panel`、`grid canvas`、`overlay` 三类核心元素。
2. 当前已改成 child-window `canvas` 作为真实 grid 容器。
3. 当前已使用真实 tracks：
   - columns: `[92, content, 1fr]`
   - rows: `[54, 66, 1fr]`
4. `G` 当前已回到 auto placement，不再显式塞 cell。
5. `overlay` 继续走 `ignore-layout`。
6. 页面已经足够用于 backend/grid visible diff 起点。

#### 当前差距

1. 当前还没有 TINYUI / 老 SDL 双开截图或 visible diff 证据。
2. panel 内容目前仍是 `A-G` 简化文案，不是老页那种 `window + title + hint` 内容层。
3. panel 尺寸、文案、颜色仍是近似，不是逐像素复刻。
4. root 外层仍是 TINYUI 页面壳，不是老页原始外围排布的逐像素拷贝。

#### 能力判断

1. `grid_parity` 所需的 grid descriptor / span / auto placement / ignore-layout public API 已经存在。
2. child-window 和 text public API 也已经足够表达老页 panel 内 title/hint 内容层。
3. 当前缺的是页面内容 fidelity 和 visible 对照，不是新的 public capability。

#### 判断

`grid_parity` 当前已经不再是 capability gap，剩余主要是内容和验证层差距。

## 共同缺口

### 1. dedicated visible 对照未接入

当前 `tests/tinyui/runtime/check_tinyui_visible_ui.py` 的 `DEMOS` 只覆盖 `layout_flex/layout_grid/basic_widgets/...`，还没有把：

1. `legacy_widget_parity`
2. `layout_parity`
3. `grid_parity`

接成 dedicated visible 对照入口。

这说明当前缺的是对照验证接入，不是 public API 缺失。

## 优先顺序

1. 先补最小 public `app` timer / tick / frame callback 能力
   - 这是唯一硬 capability gap
   - 独立设计与计划已拆到：
     - `docs/superpowers/specs/2026-06-04-tinyui-app-timer-tick-capability-design.md`
     - `docs/superpowers/plans/2026-06-04-tinyui-app-timer-tick-capability-implementation.md`
2. 再给 `legacy_widget_parity` 接老 demo 真实资源
   - 这是资源 fidelity 收敛
3. 再做三页 dedicated visible 对照和内容细化
   - 这是验证层和页面 fidelity 收敛

## 当前不能说的话

1. 不能说这三个 parity demo 已与老 SDL 页面一致。
2. 不能说 `layout_parity` 已包含老页所有运行时行为。
3. 不能说 `grid_parity` 已有 visible parity 证据。
4. 不能说 `legacy_widget_parity` 已达到图片资源 parity。
5. 不能把 `legacy_widget_parity` 和 `grid_parity` 的剩余问题写成“还缺一批 public API”；当前源码不支持这个说法。
