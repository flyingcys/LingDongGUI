# Task 1 报告

## 目标

锁定新的 TinyUI demo 边界与注册契约，新增 `legacy_demo0_parity`，并把它编入 SDL 示例构建。

## 变更内容

1. `tests/tinyui/contract/check_tinyui_demo_boundary.py`
   - 先加入 `legacy_demo0_parity` 的契约检查，作为 TDD 的红灯步骤。

2. `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.h`
   - 新增 demo 声明。

3. `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`
   - 新增最小 demo 骨架：创建 screen，失败则返回，成功则 load screen。
   - 未实现布局，符合任务要求的最小实现。

4. `tinyui/demo/tinyui_demos.h`
   - 增加 `tinyui_demo_legacy_demo0_parity(void);` 声明。

5. `tinyui/demo/tinyui_demos.c`
   - 在 runtime demo 注册表中加入 `legacy_demo0_parity`。

6. `examples/sdl/CMakeLists.txt`
   - 将新 demo 源文件编入 `tinyui_demo` 目标。

## TDD 记录

1. 先修改 contract 测试，让它失败。
2. 运行 `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`，确认失败原因为缺少 `legacy_demo0_parity`。
3. 实现最小代码。
4. 重新运行 contract 测试与 `cmake --build build --target tinyui_demo -j2`，确认通过。

## 验证

- Contract: 通过
- Build: 通过

## 备注

- 没有修改 `legacy_widget_parity`。
- 没有修改 runtime 或 widget 实现。
- 工作区内存在其他未相关修改，未触碰。

## Review 修复补记

1. 收到 review 后，补强 `tests/tinyui/contract/check_tinyui_demo_boundary.py`，让它显式断言 `legacy_demo0_parity` 也必须出现在 `examples/sdl/CMakeLists.txt` 的 `tinyui_demo` 编译列表中。
2. 先临时移除 `examples/sdl/CMakeLists.txt` 中的 `legacy_demo0_parity/legacy_demo0_parity.c`，运行 contract 测试确认断言失败。
3. 恢复该 CMake 条目后，重新运行 contract 测试与 `cmake --build build --target tinyui_demo -j2`，两者均通过。
