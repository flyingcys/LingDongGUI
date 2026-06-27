## Task 3 报告

### 范围

- 修改 `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`
- 修改 `tests/tinyui/contract/check_tinyui_demo_boundary.py`
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

### 实现内容

- 以 `examples/common/demo/widget/uiWidgetLegacy.c` 为坐标和控件真值来源，补齐单页绝对坐标布局。
- 覆盖控件包括：image、button、panel label、label、checkbox/radio、switch、progress bar、text、slider、radial menu、date time、icon slider、qrcode、scroll selecter、gauge、combo box、graph、table、line edit、child window/nested button、keyboard、arc、list、message box、calendar。
- button 接线：`tinyui_button_set_on_released(button, on_button_released, runtime)`，回调中执行 `tinyui_widget_set_opacity((struct tinyui_widget *)runtime->image, 128)`。
- switch 接线：`tinyui_switch_set_on_toggled(sw, on_switch_toggled, switch_label)`，回调根据 value 更新 label 为 `ON` 或 `OFF`。
- table 接线：`tinyui_table_set_keyboard_binding(table, 22)`。
- line edit 接线：`tinyui_line_edit_set_keyboard_binding(line_edit, 22)`。
- runtime angle 接线：`tick_runtime()` 调用 `tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle)` 和 `tinyui_gauge_set_angle(runtime->gauge, runtime->angle)`。

### GitNexus

- `impact(target=tinyui_demo_legacy_demo0_parity, file_path=legacy_demo0_parity.c, upstream)`：索引未找到该新 demo 入口，风险为 UNKNOWN。
- `impact(target=main, file_path=check_tinyui_demo_boundary.py, upstream)`：LOW，1 个直接文件级影响，0 个 affected processes。
- `detect_changes(scope=all)`：LOW，0 affected processes；结果包含共享工作区中他人未提交的 widget/unit 改动，不全是本任务变更。

### 验证

- `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`：PASS
- `cmake --build build-tests --target tinyui_demo -j4`：PASS，`[100%] Built target tinyui_demo`
- `git diff --check -- tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c tests/tinyui/contract/check_tinyui_demo_boundary.py`：PASS

### 限制与 concern

- 当前 TinyUI canonical demo 入口通过 `tinyui_screen_create()` 创建 screen，但没有暴露 app 指针给 demo 注册 `tinyui_app_timer`；在不改 runtime/backend 的限制下，`tick_runtime()` 作为 demo 构建后的运行时接线 smoke 执行一次，未实现持续定时动画。
- 本环境没有可用的独立 subagent 执行接口；本任务为两个文件强顺序 TDD 改动，实际在主线程完成。
