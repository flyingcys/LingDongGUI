# LingDongGUI 胶囊绘制能力与 Switch LVGL 对齐开发文档

## 背景

TINYUI `basic_widgets` 当前已经解决了两个明显问题：

- `switch` 从 `(0,0)` 回到真实 grid padding 位置，目标 bbox 为 `(16,24)-(63,47)`。
- TINYUI SDL runtime 的 root/background/capture 已统一到 `480x320`，不再出现右侧和底部黑块。

但截图中 `switch` track 左右端仍偏方，圆角不够接近 LVGL。继续在 `ldSwitch.c` 内盲目调整 `draw_round_corner_box()` 不是正确方向，因为 LingDongGUI 当前缺少一个能表达 LVGL `LV_RADIUS_CIRCLE` 语义的通用胶囊绘制能力。

## LVGL 对照结论

LVGL default switch 的绘制路径在 `third_party/lvgl/src/widgets/switch/lv_switch.c`：

- `LV_PART_INDICATOR` 用 `lv_draw_rect()` 绘制。
- `LV_PART_KNOB` 用 `lv_draw_rect()` 绘制。
- default theme 在 `third_party/lvgl/src/themes/default/lv_theme_default.c` 给 switch 主体和 indicator 加 `circle` style，给 knob 加 `knob` style。
- `circle` 和 `knob` 的 radius 都是 `LV_RADIUS_CIRCLE`。

因此目标不是普通小圆角矩形，而是：

- 横向 track：左右端为半圆，中间为矩形。
- 纵向 track：上下端为半圆，中间为矩形。
- indicator 使用同一胶囊规则。
- knob 使用真圆。

## 当前能力缺口

LingDongGUI 已有能力：

- `draw_round_corner_box()`：基于固定圆角 mask 的通用圆角矩形。
- `ldArm2dDrawCircle()`：能画抗锯齿圆。
- `ldBaseColor()`：能填充矩形。

缺口：

- 没有 `radius = min(width, height) / 2` 的通用胶囊绘制 API。
- `ldSwitch` 只能复用普通圆角 box，导致 track 端部圆角不够。

## 设计

新增 LingDongGUI 基础绘制能力：

```c
void ldBaseDrawCapsule(arm_2d_tile_t *ptTile,
                       const arm_2d_region_t *ptRegion,
                       ldColor color,
                       uint8_t opacity);
```

行为：

- `ptRegion == NULL` 时使用默认 region，行为与常见 Arm-2D helper 保持一致。
- `width <= 0 || height <= 0` 时直接返回。
- 横向：`width >= height`，圆端使用当前端部 region 裁剪，半径为 `height / 2`，左端圆心在 `x + height / 2`，右端圆心在 `x + width - height / 2 - 1`，中间填充矩形；裁剪必须保证抗锯齿圆端不会污染 capsule region 外部像素。
- 纵向：`height > width`，圆端使用当前端部 region 裁剪，半径为 `width / 2`，上端圆心在 `y + width / 2`，下端圆心在 `y + height - width / 2 - 1`，中间填充矩形；裁剪必须保证抗锯齿圆端不会污染 capsule region 外部像素。
- 正方形退化为圆。
- 使用现有 `ldArm2dDrawCircle()` 绘制端部，使用 `ldBaseColor()` 或 `arm_2d_fill_colour_with_opacity()` 填充中段。

半径选择说明：

- 当前 `ldArm2dDrawCircle()` 内部绘制区不是简单的 `radius * 2`，而是会扩展到 `radius * 2 + 2`，并使用 `(radius + 1)^2` 做抗锯齿边界。
- 因此胶囊端部必须传入当前端部 region 做裁剪；去掉端部裁剪会让圆 primitive 污染 capsule region 外部像素。
- 在端部 region 裁剪存在时，偶数直径胶囊必须保留 `diameter / 2`。如果改成 `(diameter - 1) / 2`，当前 primitive 会让 `60x30`、`30x60` 这类偶数尺寸胶囊的端点中线缺 1px，破坏 LVGL-like 完整胶囊端点。
- 对应测试用端点中线采样防止半径退回 `(diameter - 1) / 2`，用 capsule 外圈像素采样防止移除端部 region 裁剪。

`ldSwitch` 改动：

- 圆角模式下，off track 使用 `ldBaseDrawCapsule()`。
- 圆角模式下，on indicator 使用 `ldBaseDrawCapsule()`。
- knob 继续使用当前真圆绘制。
- 图片模式保持原逻辑，不强行套 capsule。
- 非圆角模式保持矩形填充。

## 测试要求

必须先写失败测试，再实现。

### 1. LingDongGUI capsule 单元契约

在 switch widget 测试或新增基础绘制测试里验证：

- 横向 `60x30` 胶囊有完整左右圆端和中段矩形填充，外圈像素不被污染。
- 纵向 `30x60` 胶囊有完整上下圆端和中段矩形填充，外圈像素不被污染。
- 正方形 `24x24` 退化为单个或等价圆形填充，不出现普通方角。
- 偶数尺寸端点中线必须非背景，锁定 `diameter / 2` 半径选择；外圈一圈像素必须保持背景，锁定端部 region 裁剪，避免 `ldArm2dDrawCircle()` 抗锯齿外溢。

### 2. Switch 绘制契约

`examples/sdl/tests/switch/test_ldswitch_widget.c` 增加断言：

- 圆角 switch 的 track/indicator 不再调用 `draw_round_corner_box()`。
- 圆角 switch 的 track/indicator 会走 `ldBaseDrawCapsule()` 或等价 circle-end path。
- knob 仍然只使用真圆绘制，pressed 不改变 knob 色。

### 3. SDL switch capture

`examples/sdl/tests/check_switch_capture_matrix.py` 继续验证：

- horizontal off/on。
- vertical off/on。
- disabled off/on。
- pressed。
- mid animation。

新增或强化端部采样：

- horizontal on 的左上角、左下角、右上角、右下角不能是 track 色，避免方角。
- horizontal on 的左右中点必须是 track 色，证明端部仍完整。
- vertical 同理检查上下端。

### 4. TINYUI runtime 截图

`tests/tinyui/runtime/check_tinyui_runtime.py` 必须明确断言 `tinyui_basic_widgets_demo`：

- capture size 为 `480x320`。
- 右侧和底部背景不是黑条。
- `wifi` switch track bbox 为 `(16,24)-(63,47)`。
- switch corner 像素不是 track 色，左右中点是 track 色，证明是胶囊而非方角。
- stdout 包含 `PICOUI_SMOKE_LAYOUT_USED=0`。

## CMake 尺寸一致性要求

当前 `LD_CFG_SCREEN_WIDTH=480` 等宏不能只加在 `tinyui_backend_ldgui` 上。TINYUI runtime 涉及以下 target：

- `tinyui_backend_ldgui`
- `longdonggui`
- `longdonggui_porting_default`
- demo executable

这些编译单元必须看到一致的 `LD_CFG_SCREEN_WIDTH=480`、`LD_CFG_SCREEN_HEIGHT=320`、`LD_CFG_PFB_WIDTH=480`，否则 root/window/capture/touch clamp 可能尺寸不一致。

实现时应优先用一个 CMake helper 或 INTERFACE target 集中定义 TINYUI runtime screen config，避免只在单个 target 上追加宏。

## 写面

允许修改：

- `src/gui/ldBase.h`
- `src/gui/ldBase.c`
- `src/gui/ldSwitch.c`
- `examples/sdl/tests/switch/test_ldswitch_widget.c`
- `examples/sdl/tests/check_switch_capture_matrix.py`
- `tests/tinyui/runtime/check_tinyui_runtime.py`
- `cmake/LingDongGUI.cmake`
- 必要时同步更新 `docs/2026-06-04-tinyui-switch-lvgl-visual-alignment-standard.md`

禁止：

- 修改 `tinyui/demo/basic_widgets/main.c` 来硬编码坐标或视觉。
- 在 `tinyui/src/backend/ldgui/backend_app.c` 添加 fake switch 绘制。
- 用更宽松的采样断言掩盖视觉问题。

## 验证命令

实现后至少运行：

```bash
cmake --build build --target test_tinyui_window test_tinyui_switch test_tinyui_theme test_tinyui_widgets
./build/tests/tinyui/test_tinyui_window
./build/tests/tinyui/test_tinyui_switch
./build/tests/tinyui/test_tinyui_theme
./build/tests/tinyui/test_tinyui_widgets
```

```bash
cmake --build build/sdl-switch-capture --target ldswitch_internal_test ldswitch_widget_test
./build/sdl-switch-capture/ldswitch_internal_test
./build/sdl-switch-capture/ldswitch_widget_test
python3 examples/sdl/tests/check_switch_capture_matrix.py --build-dir build/sdl-switch-capture
```

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py --demo basic_widgets --build-dir build/tinyui-runtime
```

并生成一次真实 PPM 采样，确认：

- switch bbox 为 `(16,24)-(63,47)`。
- track 左右角不是蓝色，左右中点是蓝色。
- 无右侧/底部黑边。

## Subagent 执行方式

使用一个 worker subagent 实现本任务，主线程负责：

- GitNexus impact。
- diff 审查。
- 最终验证。
- 处理 reviewer 反馈。

worker 必须：

- 遵守 TDD，先写失败测试。
- 不改 demo 规避问题。
- 不回退当前已有 switch/layout 修复。
- 只修改上述写面。
