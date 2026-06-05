# PicoUI v1.0 Native P1 垂直切片

> 状态：`P1` closeout 完成，serial index 已推进到 `P2-A`

## 当前状态

- `picoui_init()` / `picoui_deinit()` / `picoui_timer_handler()` 最小 lifecycle API 已接入 `picoui_core`。
- `picoui_display_create()` / `picoui_display_set_flush_cb()` / `picoui_display_set_default()` / `picoui_display_get_default()` 最小 display contract 已落地：
  - 保存 `width` / `height`
  - 保存 `flush_cb` / `flush_user_data`
  - 支持 default display 指针
- `picoui_indev_create()` / `picoui_indev_set_type()` / `picoui_indev_set_read_cb()` 最小 indev contract 已落地：
  - 保存 `type`
  - 保存 `read_cb` / `read_user_data`
- `picoui_screen_active()` / `picoui_screen_create()` / `picoui_screen_load()` 最小 screen contract 已落地：
  - 进程内默认 active screen 始终可取
  - `picoui_screen_load()` 会切换 active screen，并标记 `loaded`
- 当前 runtime 维护 `initialized` 与最小 auto-quit 状态：
  - `picoui_init()` 返回 `0`
  - `picoui_deinit()` 清零状态
  - `picoui_timer_handler()` 未初始化时返回 `-1`
  - 正常运行返回 `0`
  - 当环境变量 `PICOUI_DEMO_AUTO_QUIT_MS` 达到阈值时返回正数，用于 runtime automation 正常退出
  - P1 native 路径会输出 `PICOUI_RUNTIME_READY`
- 当前阶段只证明 P1-H 最小 runtime loop 已闭环，不代表完整 native runtime、渲染链路、输入泵、capture/export 或 screen tree 已完成。
- `P1-I` 最小 render smoke 已接入：
  - `test_picoui_native_render_window_label_button` 会建立 `display/screen/root window/label/button`
  - 当前只要求 `picoui_timer_handler()` 在存在 root tree 时返回 `0` 或 `1`，不允许返回 `-1`
  - `native_render.c` 当前只做 backend widget tree 的最小 traversal smoke，不调用 `ldGuiFrameStart()` / `ldGuiDraw()` / `ldGuiFrameComplete()`
  - 这不等于 native visible artifact 已完成；像素输出与 formal render evidence 留到后续阶段

## basic_widgets main 样板

状态：已迁移为 LVGL-like 入口结构。

入口形态：

- `picoui_init()`
- `hal_init(320, 480)`
- `create_demo_ui()`
- `while (1) picoui_timer_handler()`

当前 loop 语义：

- `rc < 0`：失败退出
- `rc == 0`：继续运行
- `rc > 0`：视为 auto-quit 正常退出，执行 `picoui_deinit()` 后返回 `0`

P1-H 边界：

- `basic_widgets` runtime automation 以 `PICOUI_DEMO_AUTO_QUIT_MS` 为唯一退出语义。
- 本阶段只要求 demo 在 automation 下输出 `PICOUI_RUNTIME_READY` 并在超时前正常退出。
- 若当前 native render 尚未就绪，`basic_widgets` 可记录为 documented known limitation：
  - runtime checker 对 `picoui_basic_widgets_demo` 在 P1 只保留 `PICOUI_RUNTIME_READY`、auto-quit、进程正常退出为硬门
  - 如果 capture 文件已经存在，则只校验尺寸 `480x320`
  - 颜色 bbox、REAL/FALLBACK widget ids、formal layout marker 等更强断言留到后续阶段恢复

## P1-I Native label/button render smoke

状态：已完成最小 native render smoke。

当前闭环：

- 新增 `test_picoui_native_render_window_label_button`
- 覆盖顺序：
  - `picoui_init()`
  - `picoui_display_create()` / `picoui_display_set_default()`
  - `picoui_screen_active()`
  - `picoui_window_create_root()`
  - `picoui_label_create()` / `picoui_label_set_text()`
  - `picoui_button_create()` / `picoui_button_set_text()`
  - `picoui_timer_handler()`
- 断言边界仍然保持在 smoke 强度：
  - `picoui_timer_handler()` 只能返回 `0` 或正数
  - 不做像素断言
  - 不要求 capture/native visible artifact

native render 当前真实语义：

- `native_render.c` 只提供最小 private render smoke path：
  - 绑定 active screen 与 root window
  - 从 root backend tree 做一次 traversal
  - 统计访问到的 widget 数量，以及 label/button 节点命中
- `picoui_timer_handler()` 现在会在 runtime lifecycle 通过后，附带执行一次上述 traversal smoke
- 本阶段明确不调用：
  - `ldGuiFrameStart()`
  - `ldGuiDraw()`
  - `ldGuiFrameComplete()`

P1-I limitation：

- 这只证明 native root tree 已可被最小 render smoke 路径消费。
- 这不证明：
  - ldgui real frame chain 已接入
  - native visible render 已完成
  - label/button 像素输出、布局结果、capture/export 已完成
- 上述更强证据留给后续阶段，不在 P1-I 宣称完成。

## P1-J Native input button smoke

状态：已完成 button-only 最小 native input smoke。

当前闭环：

- 新增 `test_picoui_native_input_button`
- 覆盖顺序：
  - `picoui_init()`
  - `picoui_display_create()` / `picoui_display_set_default()`
  - `picoui_screen_active()`
  - `picoui_window_create_root()`
  - `picoui_button_create_with_props()` / `picoui_button_set_on_clicked()`
  - `picoui_input_push_pointer(..., pressed=1)` + `picoui_timer_handler()`
  - `picoui_input_push_pointer(..., pressed=0)` + `picoui_timer_handler()`
- 预期边界：
  - pointer down 不触发 click
  - pointer up 在同一 button 内部时只触发一次 click

native event 当前真实语义：

- `native_event.c` 接管当前对外 `picoui_timer_handler()` / `picoui_deinit()`
- 顺序是：
  - 先走现有 P1-I render/runtime 路径
  - 再从 root window 的 owner app 读取 pointer 状态
  - 只在 `pressed 0->1` 与 `1->0` 转换时做 button-only 命中与分发
- 当前只桥接：
  - `SIGNAL_PRESS`
  - `SIGNAL_RELEASE`
- 当前故意不扩到：
  - keyboard
  - navigation
  - focus 重构
  - hold/repeat/drag/cancel 语义

P1-J limitation：

- 这只证明 button native click path 已有最小 pointer->hit-test->dispatch smoke。
- 这不证明：
  - 通用 input pump 已完成
  - 非 button widget 命中链已完成
  - keyboard/navigation/focus 统一输入模型已完成
  - visible/native input artifact 已完成

## P1 Closeout

P1 gate 已完成，当前证据对应关系如下：

- `P1-runtime-init`
  - `test_picoui_native_runtime`
  - `python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets`
- `P1-display-indev-screen`
  - `test_picoui_native_display_indev`
  - `test_picoui_native_screen`
- `P1-window-label-button`
  - `test_picoui_native_screen_window`
  - `test_picoui_native_render_window_label_button`
  - `test_picoui_native_input_button`
- `P1-basic-widgets-main-style`
  - `check_picoui_demo_main_style`
  - `check_picoui_demo_main_style_inventory`
  - `cmake --build build --target picoui_basic_widgets_demo`

P1 关闭后的真实边界：

- 已证明：
  - LVGL-like lifecycle / display / indev / screen public contract 已建立
  - `basic_widgets` main 已切到 LVGL-like 结构
  - window/label/button vertical slice 已有 render smoke
  - button-only pointer click path 已有最小 smoke
- 未证明：
  - native visible artifact 完成
  - 通用 input/event 核心完成
  - layout/theme/widget tree native foundation 完成
  - default build 已移除 ldgui runtime 依赖

下一阶段：`P2-A` native widget tree foundation。
