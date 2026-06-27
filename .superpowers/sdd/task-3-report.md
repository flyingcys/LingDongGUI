## Task 3 报告

### 范围

- 修改 `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`
- 修改 `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- 修改 `tinyui/demo/tinyui_demos.h`
- 修改 `tinyui/demo/tinyui_demos.c`
- 修改 `tinyui_demo/main.c`
- 按要求补写本报告 `.superpowers/sdd/task-3-report.md`
- 未修改 backend/runtime/widget 实现，未删除 `legacy_demo0_parity.c`，保留 Task 2 的 truth 字段契约。

### TDD 记录

1. RED：先在 `check_tinyui_demo_boundary.py` 为 `legacy_demo0_parity` 增加 Task 3 关键 marker：
   - 全控件布局关键创建：image/button/switch/table/line_edit/arc/gauge
   - 关键交互接线：button release -> image opacity、switch toggled -> OFF/ON、table/line_edit keyboard=22、arc/gauge angle update
   - 关键坐标：`100, 120`、`200, 95`、`220, 10`
2. RED 命令：`python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
3. RED 结果：失败，`legacy_demo0_parity missing marker: tinyui_image_create`
4. GREEN：实现 `legacy_demo0_parity.c` 的单页绝对坐标控件树，并只移除 contract 中与 Task 3 真实实现直接冲突的旧 absent marker。
5. GREEN 命令：`python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
6. GREEN 结果：通过。
7. 修复轮 RED：补强 contract，要求 demo registry 暴露 `tinyui_demo_frame_cb_t`、`tinyui_demos_frame(unsigned int elapsed_ms)`，runner 每轮调用 `tinyui_demos_frame(elapsed_ms)`，`legacy_demo0_parity` 暴露 `tinyui_demo_legacy_demo0_parity_frame(unsigned int elapsed_ms)`，并禁止旧的 `tick_runtime(&runtime)` 一次性调用。
8. 修复轮 RED 命令：`python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
9. 修复轮 RED 结果：失败，`tinyui_demos.h missing demo frame callback type`
10. 修复轮 GREEN：增加 runner per-frame hook，`legacy_demo0_parity` 通过 frame callback 按 100ms 节流持续更新 arc/gauge，并修正两个明显不等价的 label align。
11. 修复轮 GREEN 命令：`python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
12. 修复轮 GREEN 结果：通过。

### 实现内容

- 以 `examples/common/demo/widget/uiWidgetLegacy.c` 为坐标和控件真值来源，补齐单页绝对坐标布局。
- 覆盖控件包括：image、button、panel label、label、checkbox/radio、switch、progress bar、text、slider、radial menu、date time、icon slider、qrcode、scroll selecter、gauge、combo box、graph、table、line edit、child window/nested button、keyboard、arc、list、message box、calendar。
- button 接线：`tinyui_button_set_on_released(button, on_button_released, runtime)`，回调中执行 `tinyui_widget_set_opacity((struct tinyui_widget *)runtime->image, 128)`。
- switch 接线：`tinyui_switch_set_on_toggled(sw, on_switch_toggled, switch_label)`，回调根据 value 更新 label 为 `ON` 或 `OFF`。
- table 接线：`tinyui_table_set_keyboard_binding(table, 22)`。
- line edit 接线：`tinyui_line_edit_set_keyboard_binding(line_edit, 22)`。
- runner hook：`tinyui_demos.h/c` 增加可选 `tinyui_demo_frame_cb_t`，demo registry 保存当前 demo 的 frame callback；`tinyui_demo/main.c` 每轮主循环计算 `elapsed_ms` 并调用 `tinyui_demos_frame(elapsed_ms)`。
- runtime angle 接线：`tinyui_demo_legacy_demo0_parity_frame(unsigned int elapsed_ms)` 累积 frame delta，达到 `LEGACY_DEMO0_FRAME_INTERVAL_MS = 100` 后调用 `update_runtime_angle()`，持续驱动 `tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle)` 和 `tinyui_gauge_set_angle(runtime->gauge, runtime->angle)`。
- 对齐修正：主 label 和 switch_label 均改为 `TINYUI_ALIGN_START`，对应 legacy 的 left 方向；TinyUI 现有 public `tinyui_label_set_align()` 只能表达 START/CENTER/END 的水平语义，不能完整表达 `BOTTOM_LEFT` 或 `MIDDLE_LEFT` 的垂直细分。

### GitNexus

- `impact(target=tinyui_demo_legacy_demo0_parity, file_path=legacy_demo0_parity.c, upstream)`：索引未找到该新 demo 入口，风险为 UNKNOWN。
- `impact(target=main, file_path=check_tinyui_demo_boundary.py, upstream)`：LOW，1 个直接文件级影响，0 个 affected processes。
- `impact(target=tinyui_demos_create, file_path=tinyui/demo/tinyui_demos.c, upstream)`：LOW，1 个 direct caller，直接影响 `tinyui_demo/main.c`；affected processes 为 7 条 `Main -> ...` runner 启动流程。
- `impact(target=main, file_path=tinyui_demo/main.c, upstream)`：LOW，0 direct caller，0 affected processes。
- `detect_changes(scope=unstaged)`：HIGH，包含共享工作区中他人未提交的 widget/unit 改动，不全是本任务变更。
- `detect_changes(scope=staged)`：HIGH，6 个 staged 文件，7 个 affected processes；风险来自 `tinyui_demo/main.c:main` 参与 7 条 runner 启动流程（`Main -> LdBaseGetChildList`、`Main -> XQueueDestroy`、`Main -> Free`、`Main -> Ldgui_port_get_current_app`、`Main -> Ldgui_port_set_current_app`、`Main -> Ldgui_port_unregister_scene_app`、`Main -> Tinyui_app_create`）。本次改动为 main loop 增加 demo frame callback 调度，已用 contract、build、自动退出 runner smoke 覆盖。

### 验证

- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`：PASS
- `cmake --build build-tests --target tinyui_demo -j4`：PASS，`[100%] Built target tinyui_demo`
- `env SDL_VIDEODRIVER=dummy TINYUI_DEMO_AUTO_QUIT_MS=250 ./build-tests/examples/sdl/tinyui_demo legacy_demo0_parity`：PASS，输出 `TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI`，`TINYUI_SMOKE_LAYOUT_USED=0`，并列出 `demo0_arc`、`demo0_gauge` 等真实 widget id。
- `git diff --check -- tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c tests/tinyui/contract/check_tinyui_demo_boundary.py tinyui/demo/tinyui_demos.h tinyui/demo/tinyui_demos.c tinyui_demo/main.c .superpowers/sdd/task-3-report.md`：PASS

### 限制与 concern

- TinyUI public label align 目前不能完整表达 legacy `BOTTOM_LEFT` / `MIDDLE_LEFT` 的垂直细分；本次采用最接近的 public API：`TINYUI_ALIGN_START`。
- 本环境没有可用的独立 subagent 执行接口；本轮 review 修复实际在主线程完成，并通过 contract/build/runner smoke/GitNexus 收敛风险。
