# TinyUI src 适配边界审计

## 背景

本轮审计针对 `tinyui/src` 的跨芯片通用性和 port/backend 边界。当前 TinyUI 已完成一个阶段，但源码里仍有多处适配职责混入通用层的问题，典型例子是 SDL 宿主代码出现在 `tinyui/src/core/runtime_bridge.c` 和 `tinyui/src/core/runtime_host.c`。

审计基准来自现有规则：

- `tinyui/src/` 放 TinyUI 固定核心实现，不承载板级、OS、显示驱动、输入驱动差异。
- `tinyui/port/` 放 SDL host、板卡、RTOS、屏幕 flush、触摸采样、tick source、delay/lock 等平台适配。
- `tinyui/src/backend/ldgui/` 应是 TinyUI 到 LingDongGUI/ARM-2D 的唯一私有桥接层。
- demo 只能表达用户意图，不承担适配补丁职责。

## 审计范围

已检查范围：

- `tinyui/src/core`
- `tinyui/src/display`
- `tinyui/src/indev`
- `tinyui/src/osal`
- `tinyui/src/tick`
- `tinyui/src/drivers`
- `tinyui/src/layout`
- `tinyui/src/theme`
- `tinyui/src/widgets`
- 相关 public include：`tinyui/include/*`、`tinyui/include/port/*`
- 相关 port/build 边界：`tinyui/port/sdl/*`、`cmake/LingDongGUI.cmake`
- 相关规则文档：`tinyui/docs/porting_rules.md`

本轮 GitNexus MCP 当前未能读取 `LingDongGUI` 索引，只返回 `edgeio-js`、`cde` 两个仓库，因此本报告基于本地源码静态审计、关键词扫描和并行子任务复核。

## 总体结论

当前 `tinyui/src` 还不是纯通用层。实际结构更接近：

```text
tinyui public API
  + TinyUI 状态模型
  + LingDongGUI backend object
  + SDL host runtime
  + runtime smoke/test harness
```

这些职责目前在 `core`、`widgets`、`theme`、`window`、`drivers`、CMake target 之间交叉耦合。最需要先处理的是：

1. SDL runtime、窗口、事件循环、capture、benchmark、auto quit 必须移出 `tinyui/src/core`。
2. LingDongGUI/Arm-2D 对象和 `ld*` include 必须从通用 core/widgets/theme 收敛到 backend-private 层。
3. fake/smoke fallback 不能成为正式 runtime 路径。
4. 构建目标必须让 `tinyui_core` 不再 PUBLIC 暴露 backend driver include，也不能让 backend runtime 直接链接 SDL2。

## 严重度定义

- P0：破坏 TinyUI 跨芯片/跨后端边界，或会让正式 runtime 依赖错误宿主/假路径。
- P1：边界职责混乱，短期可能能工作，但会阻塞 backend/port 正式化或多平台扩展。
- P2：治理项或长期可维护性问题，需要纳入后续重构计划。

## P0 问题

### 1. core 直接内置 SDL 宿主窗口、tick 和 delay

证据：

- `tinyui/src/core/runtime_bridge.c:15` 声明 `SDL_Window`。
- `tinyui/src/core/runtime_bridge.c:18` 到 `tinyui/src/core/runtime_bridge.c:32` 直接声明 SDL API。
- `tinyui/src/core/runtime_bridge.c:90` 到 `tinyui/src/core/runtime_bridge.c:99` 用 `SDL_GetTicks()` 和 `SDL_Delay()` 作为默认 tick/delay。
- `tinyui/src/core/runtime_bridge.c:124` 到 `tinyui/src/core/runtime_bridge.c:157` 在 core 中 `SDL_Init()`、`SDL_CreateWindow()`、`SDL_CreateRenderer()`、`SDL_CreateTexture()`。

为什么不合理：

`runtime_bridge.c` 位于通用 core，但它直接管理桌面 SDL 宿主窗口生命周期。TinyUI 是通用跨芯片 API，core 不应知道 SDL 类型、SDL 常量或 SDL 错误处理。

建议归属：

- SDL 初始化、window/renderer/texture、SDL tick/delay 全部迁到 `tinyui/port/sdl/`。
- core 只保留抽象 runtime ops，例如 tick、delay、display flush、input push、backend step。
- 若需要默认 host runner，应作为 `tinyui/port/sdl` 或测试 runner，而不是 core 默认行为。

### 2. core 实现 SDL 事件循环和 host step

证据：

- `tinyui/src/core/runtime_host.c:22` 直接 include `<SDL.h>`。
- `tinyui/src/core/runtime_host.c:116` 到 `tinyui/src/core/runtime_host.c:138` 维护 SDL runtime state。
- `tinyui/src/core/runtime_host.c:727` 到 `tinyui/src/core/runtime_host.c:789` 直接 `SDL_PollEvent()` 并处理 mouse/window event。
- `tinyui/src/core/runtime_host.c:792` 到 `tinyui/src/core/runtime_host.c:830` 的 `tinyui_runtime_host_step_app()` 同时做 SDL event pump、render、delay、auto quit。

为什么不合理：

SDL 事件循环是 host port 职责，不是 TinyUI core 职责。当前 `tinyui_timer_handler()` 最终走到 SDL host step，导致通用 runtime API 被桌面宿主实现污染。

建议归属：

- `runtime_host.c` 拆出 core，迁到 `tinyui/port/sdl` 或 host runtime adapter。
- SDL port 将鼠标、键盘、窗口事件转换成 `tinyui_input_push_*()`。
- core 的 `tinyui_timer_handler()` 只调 backend-neutral runtime step。

### 3. core 混入 LingDongGUI/Arm-2D scene 生命周期

证据：

- `tinyui/src/core/runtime_bridge.c:3` 到 `tinyui/src/core/runtime_bridge.c:11` include `ldConfig`、`ldBase`、`ldGui`、`ldMsg`。
- `tinyui/src/core/runtime_bridge.c:398` 分配 `ld_scene_t`。
- `tinyui/src/core/runtime_bridge.c:477` 到 `tinyui/src/core/runtime_bridge.c:480` 调用 `ldGuiDespose()` 并释放 scene。
- `tinyui/src/core/runtime_host.c:23` 到 `tinyui/src/core/runtime_host.c:28` include `arm_2d`、`ldConfig`、`ldBase`、`ldGui`、display adapter。
- `tinyui/src/core/runtime_host.c:625` 到 `tinyui/src/core/runtime_host.c:629` 在 core 中执行 `ldGuiFrameStart()`、`ldGuiTouchProcess()`、`ldMsgProcess()`、`ldGuiDraw()`、`ldGuiFrameComplete()`。

为什么不合理：

LingDongGUI scene、Arm-2D tile、`ldGuiDraw()` 是具体 backend 细节。core 一旦直接管理 scene 和 draw pipeline，就无法替换 backend，也无法保持 TinyUI 的跨芯片通用层属性。

建议归属：

- `ld_scene_t`、frame start/complete、touch/msg/draw 全部迁到 `tinyui/src/backend/ldgui/`。
- core 只持有不透明 `backend_app`，通过 backend ops 初始化、step、shutdown。
- `runtime_internal.h` 不应出现 `ld_scene_t` 字段。

### 4. 正式 runtime 中存在 smoke/fake fallback 路径

证据：

- `tinyui/src/core/runtime_host.c:274` 到 `tinyui/src/core/runtime_host.c:280` 判断 fallback widget。
- `tinyui/src/core/runtime_host.c:311` 到 `tinyui/src/core/runtime_host.c:323` 判断 smoke layout 允许标记。
- `tinyui/src/core/runtime_host.c:382` 输出 `TINYUI_BACKEND_TEMPORARY_SMOKE_PATH=EXCLUDED_FORMAL_MAPPING`。
- `tinyui/src/core/runtime_host.c:388` 输出 `TINYUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`。
- `tinyui/src/core/runtime_host.c:582` 到 `tinyui/src/core/runtime_host.c:595` 用 smoke cursor layout 改写控件布局。

为什么不合理：

fake/smoke fallback 会把“能显示/能跑 smoke”误当成 backend 正式闭环，违背“UI 已完成必须基于真实 LingDongGUI 输出证据”的规则。正式 runtime 不能靠临时布局或 fake fallback 掩盖 backend/layout 缺口。

建议归属：

- 删除正式 runtime 中的 fake/smoke fallback。
- 若测试仍需要 smoke marker，迁到 `tests/tinyui/runtime/*` 或 SDL test runner。
- 文档中明确标注为 `temporary smoke path`，不得作为正式适配路径。

### 5. widgets/theme 层直接依赖 LingDongGUI 内部头

证据：

- `tinyui/src/theme/theme.c:22` 到 `tinyui/src/theme/theme.c:32` include 多个 `../../../src/gui/ld*.h`。
- `tinyui/src/widgets/gauge.c:22` 到 `tinyui/src/widgets/gauge.c:23` include `ldGauge.h` 等后端头。
- `tinyui/src/widgets/image.c:21` 到 `tinyui/src/widgets/image.c:22` include `ldBase.h`、`ldImage.h`。
- 全局扫描显示 `tinyui/src/widgets/*.c` 几乎全部直接调用 `ld*_init/Set/Get/depose` 或使用 Arm-2D 类型。

为什么不合理：

widgets/theme 层应表达 TinyUI public object、状态和用户能力，不应同时承担 LingDongGUI object 创建、setter/getter、生命周期和事件桥接。当前每个 widget 基本都是一个 `ld*` adapter，导致 public API 层和 backend 私有层没有切开。

建议归属：

- `ld*` include 与 `ld*_init/Set/Get/depose` 迁到 `tinyui/src/backend/ldgui/*`。
- `tinyui/src/widgets/*` 只维护 TinyUI 状态模型，并通过 backend interface/vtable 同步。
- theme 层只计算 resolved style，具体 `ld*` 颜色、尺寸、字段映射由 backend theme adapter 执行。

### 6. Public API 泄漏 native/Arm-2D 资源语义

证据：

- `tinyui/include/tinyui.h:44` 聚合 include `native.h`。
- `tinyui/include/native.h:4` 到 `tinyui/include/native.h:12` 暴露 `void *tile`、`void *font`。
- `tinyui/include/native.h:33` 到 `tinyui/include/native.h:41` 暴露 native signal。
- `tinyui/include/image.h:9` 到 `tinyui/include/image.h:14` 的 `tinyui_image_source` 直接包含 `img_tile`、`mask_tile`。
- `tinyui/src/core/resource.c:21` include `ldBase.h`，`tinyui/src/core/resource.c:40` 调用 `ldBaseGetVresImage()`，`tinyui/src/core/resource.c:82` 调用 `ldFree()`。

为什么不合理：

public API 不应要求用户理解 Arm-2D tile/font 或 LingDongGUI VRES/ldFree 生命周期。否则 TinyUI 用户能力被当前 backend 资源模型反向定义。

建议归属：

- public API 改成 backend-neutral resource/font/image handle 或 loader descriptor。
- `tinyui_native_image_wrap()`、VRES loader、tile/font wrapper 移到 `backend/ldgui` 或 compat adapter。
- `tinyui.h` 不默认 include backend/native escape hatch。

### 7. LingDongGUI display adapter 用 `flush_callback == NULL` 暗号识别 SDL/host

证据：

- `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:7` 到 `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:10` 注释写明 SDL/host 路径依赖 `flush_callback == NULL`。
- `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:79` 到 `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:82` 无 flush callback 时设置 `s_pfb_inited = -1`。
- `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:139` 到 `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c:140` 用 `if (s_pfb_inited)` 调用 `arm_2d_helper_pfb_task()`。

为什么不合理：

backend 不应通过 `flush_callback == NULL` 推断 SDL host。更严重的是 `s_pfb_inited = -1` 在 C 里为真，`tinyui_backend_step()` 仍可能调用 PFB task。这既是边界问题，也可能是实际运行风险。

建议归属：

- 用明确 render mode/backend ops 表达 PFB/host direct-present 差异。
- SDL host 显示能力在 `tinyui/port/sdl` 或 host runtime adapter 中处理。
- 避免 sentinel 复用布尔判断。

## P1 问题

### 8. runtime 中硬编码固定布局参数并改写后端控件位置

证据：

- `tinyui/src/core/runtime_host.c:33` 到 `tinyui/src/core/runtime_host.c:35` 定义固定 padding、row height、gap。
- `tinyui/src/core/runtime_host.c:549` 到 `tinyui/src/core/runtime_host.c:580` 用游标布局遍历 widget 并 `ldBaseSetRegion()`。
- `tinyui/src/core/runtime_host.c:597` 到 `tinyui/src/core/runtime_host.c:637` 在 render 时套用该布局。

为什么不合理：

固定坐标/固定行高布局属于临时 host/demo 补丁，不应进入 core。真实布局应由 TinyUI layout model 映射到 LingDongGUI layout，而不是 runtime 每帧改写 `ldBase` region。

建议归属：

- 移除 core runtime 的 cursor layout。
- layout model 由 `tinyui/src/layout` 保存，ldgui backend adapter 执行 apply。
- demo 不通过硬编码坐标掩盖 backend/layout 缺口。

### 9. demo/test 环境变量进入通用 runtime

证据：

- `tinyui/src/core/runtime_host.c:72` 读取 `TINYUI_TOUCH_LOG`。
- `tinyui/src/core/runtime_host.c:86` 读取 `TINYUI_BENCHMARK_LOG`。
- `tinyui/src/core/runtime_host.c:417` 读取 `TINYUI_DEMO_AUTO_QUIT_MS`。
- `tinyui/src/core/runtime_host.c:463` 读取 `TINYUI_CAPTURE_FILE` 并写 PPM。
- `tinyui/src/core/runtime_bridge.c:540` 也读取 `TINYUI_TOUCH_LOG`。

为什么不合理：

auto quit、capture、benchmark、touch log 是 host/test harness 能力。通用 core 不应读环境变量，也不应写 capture 文件。

建议归属：

- 迁到 SDL port 或 runtime test runner。
- core 可提供 trace hook 或 runtime observer，但由 port/test 注入，不直接依赖环境变量。

### 10. `runtime_internal.h` 把 ldgui 字段做成通用 backend 数据结构成员

证据：

- `tinyui/src/core/runtime_internal.h:53` 声明 `struct ld_scene_t`。
- `tinyui/src/core/runtime_internal.h:175` 到 `tinyui/src/core/runtime_internal.h:177` 在 `tinyui_backend_widget` 中保存 ld event bridge 字段。
- `tinyui/src/core/runtime_internal.h:186` 到 `tinyui/src/core/runtime_internal.h:187` 保存 `ld_widget`、`ld_name_id`。
- `tinyui/src/core/runtime_internal.h:203` 到 `tinyui/src/core/runtime_internal.h:207` 在 `tinyui_backend_app_state` 中保存 `ld_scene`。

为什么不合理：

这些字段把 LingDongGUI 作为唯一 backend 写进 core internal 类型。后续即使引入其他 backend，也会被迫携带 `ld*` 概念。

建议归属：

- core 的 backend app/widget 只保留不透明 handle。
- `ld_widget`、`ld_name_id`、`ld_scene`、event bridge list 迁入 ldgui backend-private struct。
- 测试快照如果依赖这些字段，需要转成 backend 测试 helper。

### 11. 主题系统直接写 ld widget 字段和 setter

证据：

- `tinyui/src/theme/theme.c:145` 调用 `ldBaseSetHeight()`。
- `tinyui/src/theme/theme.c:168` 调用 `ldWindowSetColor()`。
- `tinyui/src/theme/theme.c:204` 调用 `ldButtonSetColor()`。
- `tinyui/src/theme/theme.c:260` 调用 `ldSwitchSetColor()`。
- `tinyui/src/theme/theme.c:293` 调用 `ldSliderSetColor()`。
- `tinyui/src/theme/theme.c:336` 附近直接处理 calendar 后端样式。

为什么不合理：

theme 层应负责 token、part、state 的通用解析，而不是直接操作 backend widget。当前实现会让 theme API 无法脱离 LingDongGUI。

建议归属：

- `theme.c` 只输出 resolved style。
- `backend/ldgui/theme_adapter.c` 负责把 resolved style apply 到 `ld*`。

### 12. 事件 bridge 分散在 core 和具体 widget 中

证据：

- `tinyui/src/core/event.c:21` 到 `tinyui/src/core/event.c:25` include `ldCheckBox`、`ldList`、`ldSlider`、`ldSwitch`、`ldMsg`。
- `tinyui/src/core/event.c:487` 起直接解释 `SIGNAL_PRESS`、`SIGNAL_HOLD_DOWN`、`SIGNAL_RELEASE`、`SIGNAL_VALUE_CHANGED`。
- `tinyui/src/widgets/icon_slider.c:64` 起有自己的 native slot。
- `tinyui/src/widgets/table.c:216` 起有自己的 `ldMsg_t` 事件 slot。
- `tinyui/src/widgets/line_edit.c:314` 起自行 `ldMsgConnect()` press/finished。

为什么不合理：

事件桥接策略分散后，不同控件容易产生不一致的 focus、value change、edit finish 语义，也会让 lifecycle 清理复杂化。

建议归属：

- 统一收敛到 ldgui backend event adapter。
- widgets 只接收 backend-neutral event/value-change/edit-result。
- backend adapter 统一维护 `ldMsgConnect()`、`pInfo`、native signal 到 TinyUI event 的转换。

### 13. 特殊控件重复实现 backend detach/unbind/depose 生命周期

证据：

- `tinyui/src/widgets/gauge.c:149` 到 `tinyui/src/widgets/gauge.c:179` 自行 detach、unbind、depose。
- `tinyui/src/widgets/image.c:91` 到 `tinyui/src/widgets/image.c:100` 自行 unbind/depose。
- `tinyui/src/widgets/table.c:117` 到 `tinyui/src/widgets/table.c:144` 自行 unbind/depose。
- `tinyui/src/widgets/progress_wheel.c:217` 附近也维护类似 dispose 快照和后端释放。

为什么不合理：

backend tree/lifecycle 是基础设施，不应散落在各控件。重复实现容易造成 parent/child 链表、host binding、ld object 释放不一致。

建议归属：

- 提供统一 `tinyui_backend_widget_destroy()`、`tinyui_backend_widget_detach()`。
- widget destroy 只释放 TinyUI 数据和 public state。
- backend adapter 负责释放对应 `ld*` object。

### 14. widget 创建逻辑硬编码默认尺寸、字体和内置视觉资源

证据：

- `tinyui/src/widgets/button.c:154` 到 `tinyui/src/widgets/button.c:161` 默认按钮 `160x36`。
- `tinyui/src/widgets/button.c:33` 到 `tinyui/src/widgets/button.c:34` 直接引用 `ARM_2D_FONT_6x8`、`ARM_2D_FONT_16x24`。
- `tinyui/src/widgets/gauge.c:29` 到 `tinyui/src/widgets/gauge.c:32` 引用内置 tile/mask。
- `tinyui/src/widgets/graph.c:259` 到 `tinyui/src/widgets/graph.c:262` 设置 frame/grid/axis 默认值和 white dot mask。
- `tinyui/src/widgets/icon_slider.c:35` 到 `tinyui/src/widgets/icon_slider.c:46` 固定 tile/mask 数组。
- `tinyui/src/widgets/progress_wheel.c:321` 到 `tinyui/src/widgets/progress_wheel.c:322` 固定默认颜色。

为什么不合理：

默认视觉资源和尺寸策略属于 theme/default style 或 backend 资源策略。widget public API 不应被当前 demo/backend 的 tile、font、像素尺寸反向定义。

建议归属：

- 默认尺寸进入 theme metrics 或 widget default props。
- 默认资源进入 ldgui backend adapter 或 resource provider。
- public widget 只表达用户可配置能力，不直接引用 Arm-2D 全局资源。

### 15. layout 真实映射被塞进 window widget

证据：

- `tinyui/src/layout/flex.c:44`、`tinyui/src/layout/flex.c:50` 只是薄转发到 window apply。
- `tinyui/src/layout/grid.c:44`、`tinyui/src/layout/grid.c:50` 只是薄转发到 window apply。
- `tinyui/src/widgets/window.c:51` 到 `tinyui/src/widgets/window.c:147` 直接把 TinyUI flex/grid 映射成 `ldFlex*`、`LD_GRID_*`。
- `tinyui/src/widgets/window.c:243` 起 apply padding contract。
- `tinyui/src/widgets/window.c:410`、`tinyui/src/widgets/window.c:466` 附近继续执行真实 backend layout apply。

为什么不合理：

layout 层现在没有独立 model，真实逻辑挂在 `window.c`，并且直接绑定 `ldWindow`。这会让 layout 无法成为 TinyUI 通用能力。

建议归属：

- `tinyui/src/layout` 维护 backend-neutral layout model。
- `window.c` 只保存/暴露窗口状态。
- ldgui backend adapter 把 layout model apply 到 `ldWindow`。

### 16. keyboard/table/line_edit public API 透出 backend binding 和 native signal

证据：

- `tinyui/include/keyboard.h:20` 到 `tinyui/include/keyboard.h:22` 的 keyboard event callback 使用 `enum tinyui_native_signal`。
- `tinyui/include/table.h:20`、`tinyui/include/table.h:48` 暴露 `keyboard_binding`。
- `tinyui/include/line_edit.h:21`、`tinyui/include/line_edit.h:59` 暴露 keyboard binding。

为什么不合理：

用户 API 应表达输入/编辑会话语义，而不是 backend keyboard id 或 native signal。裸 `unsigned int` binding 很容易变成 `ld_name_id` 泄漏。

建议归属：

- public API 使用 TinyUI keyboard object handle、edit target、key event enum。
- `ld_name_id`、keyboard binding、native signal 留在 ldgui backend adapter。

### 17. `tinyui_core` 构建目标 PUBLIC 暴露 backend driver include

证据：

- `cmake/LingDongGUI.cmake:219` 到 `cmake/LingDongGUI.cmake:223` 中 `tinyui_core` PUBLIC include `tinyui/src/drivers`。
- `cmake/LingDongGUI.cmake:234` 到 `cmake/LingDongGUI.cmake:237` backend runtime target 也 PUBLIC include `tinyui/src/drivers`。

为什么不合理：

`tinyui_core` 是通用层，不应让 consumer 看到 `tinyui_ldgui_port.h`、`tinyui_ldgui_port_config.h` 等 backend-private 头。PUBLIC include 会扩大误依赖面。

建议归属：

- `tinyui/src/drivers` 或迁移后的 `backend/ldgui` include 只能 PRIVATE 给 backend target。
- `tinyui_core` PUBLIC include 只保留 `tinyui/include`。

### 18. backend runtime target 直接链接 SDL2，绕过专门 SDL port

证据：

- `cmake/LingDongGUI.cmake:232` 到 `cmake/LingDongGUI.cmake:240` 创建 `tinyui_backend_ldgui` 和 `tinyui_backend_ldgui_runtime`。
- `cmake/LingDongGUI.cmake:250` 到 `cmake/LingDongGUI.cmake:260` 在 backend target 上直接添加 SDL2 include/lib。
- `cmake/LingDongGUI.cmake:267` 到 `cmake/LingDongGUI.cmake:295` 另有独立 `tinyui_port_sdl` target。

为什么不合理：

SDL host 依赖应该集中在 `tinyui_port_sdl` 或 demo/runtime executable。当前 backend runtime 自己链接 SDL2，和专门 port 重叠，边界混乱。

建议归属：

- `tinyui_backend_ldgui` 只链接 backend/core/longdonggui。
- `tinyui_port_sdl` 独立链接 SDL2。
- runtime demo 或测试可显式组合 `tinyui_backend_ldgui + tinyui_port_sdl`。

### 19. `tinyui_port_sdl_attach()` 固定显示配置，port 不可配置

证据：

- `tinyui/port/sdl/sdl.c:21` 到 `tinyui/port/sdl/sdl.c:46` 是唯一 attach API。
- `tinyui/port/sdl/sdl.c:23` 到 `tinyui/port/sdl/sdl.c:29` 固定 `480x320`、`RGB565`。
- `tinyui/include/port/sdl.h:6` 只有 `tinyui_port_sdl_attach(struct tinyui_app *app)`。

为什么不合理：

SDL port 是专门 port，但它无法让调用方配置窗口尺寸、颜色格式、buffer height、输入/显示策略。固定值会进入所有 host 使用场景。

建议归属：

- 增加 `struct tinyui_port_sdl_config`。
- `tinyui_port_sdl_attach()` 可作为默认 helper，但正式入口应允许传入 display config 和 host 行为选项。

## P2 问题

### 20. `src/drivers` 与文档声明的 `src/backend/ldgui` 不一致

证据：

- `tinyui/docs/porting_rules.md:12` 声明 `tinyui/src/backend/ldgui/` 是唯一私有桥接层。
- `tinyui/docs/porting_rules.md:92` 将 `tinyui/src/backend/ldgui/*` 列入默认编译。
- `cmake/LingDongGUI.cmake:146` 实际设置 `LD_TINYUI_BACKEND_LDGUI_DIR` 为 `tinyui/src/drivers`。
- 当前 checkout 没有 `tinyui/src/backend/ldgui/` 目录。

为什么不合理：

文档和实现命名不一致，会让后续开发者不知道 backend 私有代码应该放在 `drivers` 还是 `backend/ldgui`，也会混淆“芯片 driver”和“GUI backend adapter”。

建议归属：

- 建立 `tinyui/src/backend/ldgui/`。
- 将 `tinyui_ldgui_port.*`、`tinyui_ldgui_disp_adapter.c` 和后续 ldgui adapter 迁入。
- `drivers` 若保留，应只表示真实设备 driver，不表示 GUI backend。

### 21. public 聚合头把用户 API 和 port 作者 API 混在一起

证据：

- `tinyui/include/tinyui.h:33` include `display.h`。
- `tinyui/include/tinyui.h:38` include `indev.h`。
- `tinyui/include/tinyui.h:46` include `osal.h`。
- `tinyui/include/tinyui.h:47` include `port.h`。
- `tinyui/include/tinyui.h:62` include `tick.h`。

为什么不合理：

普通应用用户 include `tinyui.h` 时会获得 port contract。display/indev/osal/tick 是 port 作者接口，不应默认混入应用层 API。

建议归属：

- `tinyui.h` 聚合用户控件、layout、theme、runtime 应用 API。
- 新增或整理 `tinyui_port.h` 聚合 display/indev/osal/tick/port contract。
- 旧头保留兼容 wrapper，逐步迁移。

### 22. include 目录仍是扁平结构，难表达 runtime/port/display/indev 边界

证据：

- 当前实际头是 `tinyui/include/runtime.h`、`tinyui/include/port.h`、`tinyui/include/display.h`、`tinyui/include/indev.h`。
- 只有 `tinyui/include/port/sdl.h` 已进入二级目录。

为什么不合理：

扁平 include 容易让应用 API、port contract、backend escape hatch 混用。目录结构本身无法表达哪些是用户可用能力，哪些是 port 作者接口。

建议归属：

- 逐步建立分层 include，例如：

```text
tinyui/include/tinyui/*.h
tinyui/include/tinyui/runtime/*.h
tinyui/include/tinyui/port/*.h
tinyui/include/tinyui/backend/ldgui/*.h
```

- 保留旧路径转发头，避免一次性破坏兼容。

### 23. LingDongGUI 外部符号桥接依赖全局 current app

证据：

- `tinyui/src/drivers/tinyui_ldgui_port.c:26` 定义 `s_ldgui_current_app`。
- `tinyui/src/drivers/tinyui_ldgui_port.c:29` 到 `tinyui/src/drivers/tinyui_ldgui_port.c:36` 通过 setter/getter 管理当前 app。
- `tinyui/src/drivers/tinyui_ldgui_port.c:40` 到 `tinyui/src/drivers/tinyui_ldgui_port.c:55` 的 `Disp0_DrawBitmap()` 使用该全局 app。
- `tinyui/src/drivers/tinyui_ldgui_port.c:77` 到 `tinyui/src/drivers/tinyui_ldgui_port.c:83` 的 timestamp 使用该全局 app。
- `tinyui/src/drivers/tinyui_ldgui_port.c:87` 到 `tinyui/src/drivers/tinyui_ldgui_port.c:123` 的触摸读写使用该全局 app。

为什么不合理：

这可能是适配 LingDongGUI legacy 外部符号的现实约束，但作为正式 TinyUI backend bridge，它限制多 app、多屏、多实例和并发测试。

建议归属：

- 短期在 backend 文档中明确 single-app 限制。
- 中期把上下文接入 backend app state 或 display adapter instance。
- 避免把 `current_app` 暴露为 public port contract。

### 24. runtime public helper 使用全局单例

证据：

- `tinyui/src/core/runtime.c:8` 定义 `g_tinyui_runtime_app`。
- `tinyui/src/core/runtime.c:10` 到 `tinyui/src/core/runtime.c:18` 的 `tinyui_init()` 创建单例 app。
- `tinyui/src/core/runtime.c:30` 到 `tinyui/src/core/runtime.c:38` 的 `tinyui_screen_create()` 依赖单例。
- `tinyui/src/core/runtime.c:49` 到 `tinyui/src/core/runtime.c:65` 的 `tinyui_timer_handler()` 依赖单例。

为什么不合理：

LVGL-like 默认实例可以保留，但底层通用层不应被全局单例绑死。否则多 display、多 app、测试隔离会受限。

建议归属：

- 保留便捷单例 API，但底层实现基于显式 `tinyui_app` context。
- port/backend 初始化以 app context 为主，默认实例只是薄 wrapper。

### 25. ARM-2D 配置宏固定在 backend-private config header

证据：

- `tinyui/src/drivers/tinyui_ldgui_port_config.h` 承载 ARM-2D feature、颜色深度、screen placeholder、PFB lines 等配置。
- `cmake/LingDongGUI.cmake:160` 到 `cmake/LingDongGUI.cmake:162` 将其固定注入为 `__ARM_2D_USER_APP_CFG_H__`。

为什么不合理：

板卡、display、PFB、颜色深度策略不完全属于 ldgui backend。把它们固定在 backend-private config 里，会让 board/host port 难以独立配置。

建议归属：

- backend 保留最小 ARM-2D glue。
- board/host port 或 target toolchain config 提供屏幕、PFB、颜色深度配置。

### 26. progress_wheel 使用 shadow struct 修改后端私有布局

证据：

- `tinyui/src/widgets/progress_wheel.c:33` 到 `tinyui/src/widgets/progress_wheel.c:36` 定义本地桥接结构。
- `tinyui/src/widgets/progress_wheel.c:202` 到 `tinyui/src/widgets/progress_wheel.c:214` cast 并修改后端内部字段。

为什么不合理：

本地 shadow struct 假定后端私有结构布局，后端结构一变就会破坏 TinyUI widget。这个属于 backend private capability，不应在 widget 层用结构体覆盖。

建议归属：

- 补 LingDongGUI 正式 API 或 ldgui backend adapter capability。
- widget 层不要直接假定后端私有结构布局。

## 建议整改顺序

### 2026-06-15 当前进展

- Wave 1 的第一步已经落地：`tinyui/src/core/runtime_host.c` 已迁到 `tinyui/port/sdl/runtime_host.c`。
- `tinyui/src/core/runtime_bridge.c` 已移除 SDL window/tick/delay 默认实现，不再直接声明或调用 SDL API。
- `tinyui_port_sdl` 承担 SDL runtime host 构建职责。
- 新增 `tests/tinyui/contract/check_tinyui_core_no_sdl.py`，用于防止 SDL 符号和旧 runtime host 文件回到 `tinyui/src/core`。
- 当前验证：`rtk ctest --test-dir build --output-on-failure -R 'test_tinyui_app_lifecycle|check_tinyui_core_no_sdl'` 通过。

说明：本进展只关闭 SDL host 位于 core 的一部分问题；`runtime_bridge.c` 仍保留 LingDongGUI scene 生命周期，widgets/theme 仍直接依赖 `ld*` 的问题继续按后续 wave 处理。

### Wave 1：先切 SDL/host 出 core

目标：

- `tinyui/src/core/runtime_bridge.c` 不再包含 SDL 类型/API。
- `tinyui/src/core/runtime_host.c` 迁出 core 或拆成 SDL port runtime。
- `tinyui_backend_ldgui_runtime` 不再直接链接 SDL2。
- `tinyui_port_sdl` 承担 SDL event pump、present、capture、auto quit、benchmark。

验收：

- `rg -n "SDL_|#include <SDL" tinyui/src/core tinyui/src/widgets tinyui/src/theme` 无结果。
- SDL demo 仍可通过显式组合 `tinyui_backend_ldgui + tinyui_port_sdl` 运行。

### Wave 2：建立 backend-private ldgui 层

目标：

- 新建或落实 `tinyui/src/backend/ldgui/`。
- 迁移 `tinyui/src/drivers/tinyui_ldgui_*`。
- 迁移 core 中 `ld_scene`、`ld_widget`、`ld_name_id`、event bridge 字段。
- `tinyui_core` 不 PUBLIC include backend-private 目录。

验收：

- `runtime_internal.h` 不出现 `ld_scene_t`、`ld_widget`、`ld_name_id`。
- `tinyui_core` target 不 include `tinyui/src/drivers` 或 `tinyui/src/backend/ldgui`。

### Wave 3：widgets/theme/layout 变成 TinyUI 状态层

目标：

- widgets 不直接 include `../../../src/gui/ld*.h`。
- theme 不直接调用 `ld*Set*` 或写 ld struct 字段。
- layout model 从 `window.c` 抽出为 backend-neutral 状态。
- 统一 backend lifecycle 和 event adapter。

验收：

- `rg -n "../../../src/gui|ld[A-Z].*\\(|SIGNAL_|arm_2d" tinyui/src/widgets tinyui/src/theme tinyui/src/layout` 只允许 backend-neutral 兼容清单中的过渡项。
- 每个过渡项必须有明确迁移 owner 和 deadline。

### Wave 4：public API 与 port API 分层

目标：

- `tinyui.h` 不默认聚合 `native.h`、`display.h`、`indev.h`、`osal.h`、`port.h`、`tick.h`。
- 建立 port 作者使用的聚合头。
- image/font/resource API 改成 backend-neutral descriptor。
- keyboard/table/line_edit 不再暴露 backend binding/native signal。

验收：

- 应用 demo 只 include 用户 API。
- port 示例只 include port contract。
- public API contract 检查覆盖 include 分层。

## 后续审计命令

建议在每个整改 wave 后运行这些检查：

```bash
rg -n "SDL_|#include <SDL" tinyui/src
rg -n "../../../src/gui|../../../src/misc|#include \"ld[A-Z]|SIGNAL_|arm_2d|ARM_2D_|COLOUR_INT" tinyui/src/core tinyui/src/widgets tinyui/src/theme tinyui/src/layout
rg -n "TINYUI_.*SMOKE|FAKE_FALLBACK|TINYUI_DEMO|TINYUI_CAPTURE|TINYUI_BENCHMARK|getenv|fopen" tinyui/src
rg -n "tinyui/src/drivers|tinyui/src/backend/ldgui|SDL2" cmake/LingDongGUI.cmake tests/tinyui/CMakeLists.txt examples/sdl/CMakeLists.txt
```

这些命令不是完成标准，只是快速发现边界回归。最终完成仍要以真实 LingDongGUI 输出、SDL demo 运行、CMake target 边界和 public API contract 检查为准。
