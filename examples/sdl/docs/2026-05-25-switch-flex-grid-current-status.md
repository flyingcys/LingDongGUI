# 2026-05-25 switch / flex / grid 当前状态

## 结论

- `switch`：主线已完成原生控件化，不再是 stub。当前已具备 `AUTO/横向/纵向` 方向、禁用态、首帧稳态、value change 事件、track/indicator/knob 几何与图片 fallback，并补齐了 `ldSwitchNavigate()`、`ldSwitchCanNavigate()`、通用焦点层“只在会改值时才消费方向键”的路由、indicator ring 与可见的 knob overhang 几何；本轮又补上了 `pressed / animation mid-frame / checked ring` 的截图矩阵证据，因此 switch 这条第一轮 parity 线已经从“主路径闭环”推进到“主路径 + 关键视觉证据闭环”。
- `flex`：Phase 0/1 主线已基本成型。当前已支持 8 种 flow、main/cross/track 对齐、item gap/track gap、`grow`、`new track`、`ignore layout`，更像“可用的一维布局骨架”，但离 LVGL 的完整 flex 体系还有 RTL、margin/percent/content-size 联动等差距。
- `grid`：已经从旧 `gridColumns` 顺排容器升级为显式二维网格。当前稳定支持 descriptor、`fixed/CONTENT/FR`、explicit cell、span、cell align、container align、descriptor-grid 下的 auto placement fallback，并保留 legacy `gridColumns` fallback。

## 本轮修复

这轮 review 发现的问题，不在 `switch` 本身，而在 flex/grid 收口 patch 还没合并干净：

1. `src/gui/ldWindow.h` 里 `gridColDsc` / `gridRowDsc` 重复声明，导致 `layout_window_test` 直接编译失败。
2. `src/gui/ldBase.c` 里 `ldBaseSetGridCell()` 重复定义，且两份实现语义不一致：
   - 一份沿用旧的 `uint8_t` clamp 思路
   - 一份改成 `int16_t` 坐标/跨度并设置 `isGridCellSet`
3. descriptor-grid 的关键行为缺少回归测试，尤其是：
   - container align 的 `SpaceBetween / SpaceAround / SpaceEvenly / Stretch`
   - 未显式设置 cell 时的自动落位兼容性

本轮已做的收口：

- 删除 `ldWindow_t` 里的重复 grid descriptor 字段与重复 API 原型。
- 删除旧版重复 `ldBaseSetGridCell()` 实现，统一保留 `int16_t + isGridCellSet` 这条新语义。
- 继续收口 `src/gui/ldWindow.c` 的 descriptor-grid solver，让 auto placement 会跳过已被 span 占用的格子。
- 在 `examples/sdl/tests/layout/test_layout_window.c` 新增回归测试，锁住：
  - `CONTENT` 轨道取最大可见 child
  - descriptor-grid 自动落位
  - span + `FR`
  - `SpaceBetween`
  - `SpaceAround`
  - `SpaceEvenly`
  - `Stretch`
- 在 `examples/common/demo/layout/uiLayout.c` 补了 auto placement panel，让 `USE_DEMO=5` 也能直观看到“显式 span 后的自动补位”。

## 代码真相源

### switch

- 控件与公开 API：`src/gui/ldSwitch.h`、`src/gui/ldSwitch.c`
- 内部几何与动画：`src/gui/ldSwitchInternal.h`、`src/gui/ldSwitchInternal.c`
- 焦点/输入接线：`src/gui/ldBase.c`、`examples/common/demo/widget/uiWidgetLegacy.c`
- demo 接线：`examples/common/demo/widget/uiWidgetLegacy.c`、`examples/common/demo/widget/uiWidgetSwipePage01.c`
- 测试：`examples/sdl/tests/switch/test_ldswitch_internal.c`、`examples/sdl/tests/switch/test_ldswitch_widget.c`、`examples/sdl/tests/layout/test_layout_window.c`
- 输入路由约束：`examples/sdl/tests/check_switch_focus_routing.py`
- 边角审计：`examples/sdl/docs/2026-05-25-switch-lvgl-edge-audit.md`

### flex

- 容器 API：`src/gui/ldWindow.h`、`src/gui/ldWindow.c`
- 子项元数据：`src/gui/ldBase.h`、`src/gui/ldBase.c`
- demo：`examples/common/demo/layout/uiLayout.c`
- 差距分析：`examples/sdl/docs/2026-05-24-flex-vs-lvgl-gap-analysis.md`

### grid

- descriptor / solver：`src/gui/ldWindow.h`、`src/gui/ldWindow.c`
- child cell metadata：`src/gui/ldBase.h`、`src/gui/ldBase.c`
- demo：`examples/common/demo/layout/uiLayout.c`
- 检查清单：`examples/sdl/docs/2026-05-24-grid-implementation-checklist.md`
- 差距分析：`examples/sdl/docs/2026-05-24-grid-vs-lvgl-gap-analysis.md`

## 当前边界

### switch

- 已进入“可维护主线”，后续不再是补控件存在性，而是补 LVGL 边角语义。
- 当前已证实的剩余差距主要有：
  - 当前这轮 contract 下，主路径和关键视觉证据已经补齐
  - 若继续追更高置信度 parity，后续更适合补整图截图比对或主题组合矩阵，而不是继续补基础存在性

### flex

- 当前适合继续补行为细节和与 LVGL 的语义差距。
- 还不适合宣称“LVGL flex parity 完成”。

### grid

- 当前实现已经具备 descriptor-grid 主干能力，并已补齐 auto placement fallback 的主路径证据。
- 仍明确不支持 `subgrid`、RTL、grid 下专门的 `ignore-layout` 等更大范围语义。
- `gridColumns` 仍是兼容入口，不应再把它当成 grid 主实现。

## 三控件对齐检查

| 线别 | 已对齐主路径 | 当前未闭环项 | 文档化后的下一步 |
| --- | --- | --- | --- |
| switch | 原生控件、方向/禁用/事件、indicator ring、knob overhang、值变更优先的通用焦点导航、pressed/mid-frame/ring 视觉证据 | 若继续深挖，只剩更高置信度像素比对 | 可选补主题矩阵或整图 diff |
| flex | flow/wrap/reverse、main/cross/track 对齐、gap/grow/new-track | RTL、margin、percent/content-size 联动 | 继续补第二阶段语义差距 |
| grid | descriptor、`CONTENT/FR`、span、cell/container align、auto placement | `subgrid`、RTL、grid ignore-layout | 继续补更大范围语义，不回退主实现 |

## 本轮验证

### 构建与测试

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-review -DUSE_DEMO=4
rtk cmake --build examples/sdl/build-review
rtk ./examples/sdl/build-review/layout_window_test
rtk ctest --test-dir examples/sdl/build-review --output-on-failure
```

结果：当前主线验证已扩到 `10/10`，其中 `check_switch_capture_matrix` 也已覆盖 `pressed / animation mid-frame / checked ring`。

### demo 启动 smoke

```bash
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-review/ldgui_sdl_demo
rtk cmake -S examples/sdl -B examples/sdl/build-grid-review -DUSE_DEMO=5
rtk cmake --build examples/sdl/build-grid-review --target ldgui_sdl_demo
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-grid-review/ldgui_sdl_demo
```

结果：`USE_DEMO=4`（layout/flex）与 `USE_DEMO=5`（grid）均可构建并启动。
