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

当前摘要：

- `v2.2` 目前仍处 plan 刚建立状态
- `S0` 尚未执行；当前基线仍以源码扫描和 focused 读面为准

### S1 runtime 启动骨架收口

文件：`docs/v2.2/plans/stages/s1-runtime-startup-canonicalization-plan.md`

目标：

- 把 canonical 启动面固定到 `runtime.h`
- 让 `app.h/app.c` 退出主路径叙事
- 建立统一 runner 所需的最小 runtime API

当前摘要：

- `runtime.h` 已有 screen/timer 风格入口，但仍不是唯一 canonical usage
- `app.h/app.c` 仍保留旧 `picoui_app_*` 主路径，需要在本阶段退场

### S2 `backend.h` 退场

文件：`docs/v2.2/plans/stages/s2-backend-header-retirement-plan.md`

目标：

- 拆散 `backend.h`
- 删除 `tinyui/src/backend/ldgui/backend.h`

当前摘要：

- `backend.h` 当前仍存在
- 当前部分 widget/source 仍直接依赖该总头

### S3 demo LVGL-like 化

文件：`docs/v2.2/plans/stages/s3-demo-lvgl-like-runner-plan.md`

目标：

- demo `.c` 只保留 build API
- 统一 `main` 持有 runtime lifecycle
- 切换 demo 方式收口到“手工替换 build API”

当前摘要：

- 主线 demo 当前仍各自自带 `main()/run_demo()`
- 统一 runner 尚未建立
