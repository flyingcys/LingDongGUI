# TinyUI v2.2 Stages

## 读法

先读：

1. `docs/v2.2/2026-06-13-tinyui-v2-2-lvgl-like-startup-spec.md`
2. `docs/v2.2/plans/v2.2-orchestration-plan.md`
3. 本文件

然后严格按下面顺序执行阶段 plan。

## 阶段顺序

### S0 启动残留面冻结

文件：`docs/v2.2/plans/stages/s0-startup-residue-baseline-plan.md`

目标：

- 冻结当前 canonical 启动面
- 冻结 `backend.h` 内容分布
- 冻结 demo `main/run_demo/app_run` 残留面

S0 closeout 要点：

- `v2.2` inventory 已建立，后续阶段必须先回看启动残留清单
- 后续 `S1` 到 `S3` 必须统一服从这组基线，不得边做边改起跑边界

S0 已完成（2026-06-13）：

- startup baseline inventory 已建立：`docs/v2.2/2026-06-13-tinyui-v2-2-startup-baseline-inventory.md`
- `backend.h` include 残留面已冻结：19 处直接 include，内容已按 shared/widget-local/layout 分层
- demo `main/run_demo/app_run` 残留面已冻结：28 个 demo 目录均有 `main.c` + `tinyui_app_*` 依赖

当前摘要：

- `v2.2` S0-S2 已完成
- `S0` 基线已锁定并执行完毕

### S1 runtime 启动骨架收口

文件：`docs/v2.2/plans/stages/s1-runtime-startup-canonicalization-plan.md`

目标：

- 把 canonical 启动面固定到 `runtime.h`
- 让 `app.h/app.c` 退出主路径叙事
- 建立统一 runner 所需的最小 runtime API

当前摘要：

- S1 已完成（2026-06-13）：
  - `runtime.h` 已固定为唯一 canonical 启动头：`tinyui_init()` / `tinyui_deinit()` / `tinyui_screen_create()` / `tinyui_screen_load()` / `tinyui_timer_handler()` 为原生声明
  - `app.h` 已标记为 INTERNAL / NON-CANONICAL，禁止新代码使用
  - `runtime.c` 实现已改为 `tinyui_*` 函数名，通过 `internal.h` 获取 app 内部声明
  - `tinyui/demo/main.c` unified runner 骨架已建立，待 S3 接入 demo build API
  - 向后兼容：`tinyui_*` static inline wrappers 保留，现有 demo 继续编译通过
  - contract tests 已更新：`check_tinyui_public_api.py` 和 `tinyui_v21_transition_inventory.json`

### S2 `backend.h` 退场

文件：`docs/v2.2/plans/stages/s2-backend-header-retirement-plan.md`

目标：

- 拆散 `backend.h`
- 删除 `tinyui/src/backend/ldgui/backend.h`

S2 已完成（2026-06-13）：

- `runtime_internal.h` 已创建，承载 shared enums、layout cache、widget tree 和 app state 结构体
- `internal.h` 已补充 missing forward declarations，改为 `#include "runtime_internal.h"`
- widget-specific backend bridge helpers 保留在各 widget `.c` 文件内（定义先于使用，自声明）
- 原 19 处 `#include "backend.h"` 已全部清理为零
- `tinyui/src/backend/ldgui/backend.h` 已删除
- 全量编译测试通过

当前摘要：

- `backend.h` 已删除
- 所有内容已按 shared/widget-local/layout 三层拆入 `runtime_internal.h`、`internal.h` 和 widget `.c` 文件

### S3 demo LVGL-like 化

文件：`docs/v2.2/plans/stages/s3-demo-lvgl-like-runner-plan.md`

目标：

- demo `.c` 只保留 build API
- 统一 `main` 持有 runtime lifecycle
- 切换 demo 方式收口到"手工替换 build API"

当前摘要：

- S3 部分完成（2026-06-13，未关闭）：
  - `tinyui/demo/basic_widgets/main.c` 已拆为 `basic_widgets.h` + `basic_widgets.c`，只暴露 `tinyui_demo_basic_widgets_build()` build API
  - `tinyui/demo/settings_panel/main.c` 已拆为 `settings_panel.h` + `settings_panel.c`，只暴露 `tinyui_demo_settings_panel_build()` build API
  - `tinyui/demo/main.c` unified runner 已接入 demo build API（默认 basic_widgets，支持编译宏切换 settings_panel）
  - CMake wiring 已更新：`add_tinyui_demo` 支持多源文件，各 demo target 使用 unified runner + demo build 文件
  - `basic_widgets/` 和 `settings_panel/` 子目录不再有 `main()` / `run_demo()` / `tinyui_app_*`
  - focused gate 已通过：`rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'`
  - 其余 26 个 demo 保持旧自定义 main/run_demo/tinyui_app_* 结构，待后续阶段迁移
