# TinyUI v2.2 设计文档

> 面向后续 agent 执行：本设计文档锁定 `TinyUI v2.2` 的目标、边界、终态结构、阶段顺序与验收口径。`v2.2` 是在 `v2.1` 已完成基础上的后续收口线，聚焦启动模型、demo 组织方式与 `backend.h` 退场，不回退或改写 `v2.1` 完成结论。

## 1. 背景与设计结论

`TinyUI v2.1` 已完成产品层 canonical truth 收口：

- 顶层产品目录已统一到 `tinyui/`
- 独立 `backend` 目录已不再是 `v2.1` 的未完成项
- canonical contract / runtime / test / CMake 入口已统一到 `tinyui` 口径
- `v2.1` 完成态已经允许有限过渡残留，不再把“继续清零所有 `picoui_*` 公共 API”当作完成前提

但当前代码面仍保留一条不够理想的开发者主路径：

- `tinyui/src/core/app.c` 仍保留旧 `picoui_app_*` 主入口叙事
- `tinyui/include/runtime.h` 虽然已经提供 `screen + timer` 风格入口，但还不是唯一、绝对清晰的 canonical 启动真相
- `tinyui/src/backend/ldgui/backend.h` 仍像一个跨层总汇聚头，和“backend 已退场”这条架构结论不一致
- `tinyui/demo/*/main.c` 仍多为各 demo 自带启动/循环/切屏逻辑，不够接近 `LVGL` 常见 demo 组织方式

因此，`TinyUI v2.2` 的目标不是继续做 `v2.1` 式目录命名收口，而是：

**把 TinyUI 的开发者主路径收口到接近 LVGL 的方式：统一 runtime 启动骨架、让 demo `.c` 对外只提供 build API、删除 `backend.h` 总头，并把旧 `app.c` 主路径退出 canonical surface。**

一句话：

**`v2.2` 的完成态不是“再做一轮 rename”，而是“TinyUI 的启动方式、demo 组织方式和内部边界已经收口到更接近 LVGL 的开发模型”。**

## 2. 目标

`TinyUI v2.2` 的目标固定为：

- canonical 用户启动路径只保留 `runtime.h` 这一条主线
- `tinyui` demo 统一改成“demo `.c` 只导出 build API，统一 `main` 持有 init/load/loop/deinit” 的结构
- `tinyui/src/backend/ldgui/backend.h` 必须删除
- `tinyui/src/core/app.c` 不再承载 canonical 用户主入口模型
- 主线 demo 切换方式改成和 `LVGL` 常见做法接近：在统一 `main` 中手工替换 demo build API

## 3. 非目标

本路线明确不做以下事情：

- 不改写 `v2.1` 已完成结论
- 不重命名 `LingDongGUI` 的目录和 `ld*` public API
- 不做 demo 聚合轮播器
- 不做“自动发现 demo 并动态切换”的框架
- 不保留旧 `picoui_app_*` 主路径兼容作为本次设计约束
- 不把所有 shared 逻辑粗暴塞进 widgets
- 不借这条线顺手扩成 `TinyUI` 全量 public C API rename 工程

## 4. 核心设计结论

### 4.1 启动模型结论

`TinyUI v2.2` 的 canonical 启动模型固定为：

1. `tinyui_init()`
2. `tinyui_screen_create()`
3. demo build API 在 screen 上构建 UI
4. `tinyui_screen_load()`
5. `while (1) { tinyui_timer_handler(); }`
6. `tinyui_deinit()`

这条主路径由统一 runner 持有，不再由每个 demo 自己复制一份。

### 4.2 Demo 组织结论

每个 demo 目录下的 `.c` 文件对外只提供构建 API，不再提供：

- `main()`
- `run_demo()`
- `picoui_app_create()/picoui_app_run()` 式自持启动路径
- 自己的 runtime loop

推荐终态形态：

- `tinyui/demo/basic_widgets/basic_widgets.c`
- `tinyui/demo/settings_panel/settings_panel.c`
- `tinyui/demo/main.c`

其中：

- demo 文件负责 UI 创建、布局、事件绑定
- `main.c` 负责 runtime 初始化、screen 创建、load、loop、deinit
- 切换 demo 方式固定为：手工替换 `main.c` 中调用的 build API

### 4.3 `backend.h` 结论

`tinyui/src/backend/ldgui/backend.h` 在 `v2.2` 中必须彻底退场。

这里的“退场”不是：

- 改个文件名继续保留一份 backend 总头
- 把大部分声明原样搬去另一份 shared mega-header

而是：

- widget-specific helper 声明继续下沉到对应 widget 文件或 widget-private header
- shared enum / backend tree / runtime state / event bridge / data truth 之类 shared internal 定义，按职责拆入 `core`、`runtime`、`layout`、`native` 等专属 internal 头
- window/layout cache 这类仅服务特定子系统的结构，回到对应子系统自己的 private 头或实现文件

### 4.4 `app.c` 结论

`tinyui/src/core/app.c` 不再作为 canonical 用户入口真相源。

允许的终态只有两种：

1. 文件保留，但只承担 timer/lifecycle/shared state 的 internal 薄职责
2. 文件进一步被拆平，用户入口能力被完全并入 `runtime.*` 与其他 shared 层

无论采用哪种落地形式，都不再允许 `picoui_app_create()`、`picoui_app_run()`、`picoui_app_switch_window()` 这类旧模型继续作为 TinyUI 的当前主叙事。

## 5. 模块边界与文件落点

### 5.1 `runtime.h`

`tinyui/include/runtime.h` 成为唯一 canonical 启动头。

对外应承接：

- `tinyui_init()`
- `tinyui_deinit()`
- `tinyui_screen_create()`
- `tinyui_screen_load()`
- `tinyui_timer_handler()`

若当前 API 还不足以表达统一 runner 的最小骨架，可以在这里补最小增量；但不得把主入口再分散回 `app.h`。

### 5.2 `app.h` / `app.c`

`tinyui/include/app.h` 与 `tinyui/src/core/app.c` 不再面向 demo / 用户入口承担主路径职责。

`v2.2` 的清理方向固定为：

- 旧 `picoui_app_*` 主路径退出 canonical surface
- 剩余必要能力要么并入 `runtime`，要么降级为 internal helper
- demo 与后续示例不得再依赖 `app_create -> window_create -> app_run` 这条链

### 5.3 `backend.h` 拆散落点

`backend.h` 当前内容拆分时，按下列原则落位：

- `backend widget kind / signal / truth policy / value source`：
  进入 shared `core` internal 头
- app/runtime backend state：
  进入 `runtime` internal 头
- window/layout cache：
  进入 `window` / `layout` private 头或实现内局部结构
- widget-specific LDGUI bridge/helper：
  保留在各 widget `.c` 或 widget-private 头
- 仅单文件使用的 helper：
  优先改成 `static`

硬约束：

- 不允许新建另一份“总代码中心头”取代 `backend.h`
- 不允许把本该 widget-local 的 bridge/helper 再提升回 shared 层

### 5.4 Demo 文件布局

`v2.2` 推荐把主线 demo 收口成：

- `tinyui/demo/<demo_name>/<demo_name>.c`
- `tinyui/demo/<demo_name>/<demo_name>.h`
- `tinyui/demo/main.c`

其中 demo 头文件只暴露 build API，例如：

```c
int tinyui_demo_basic_widgets_build(tinyui_obj_t *screen);
```

若某个 demo 需要少量静态资源或局部 helper，这些内容仍留在 demo 自己目录，不上升到 runtime 层。

## 6. 参考 LVGL 的方式

`v2.2` 参考 `LVGL` 的地方固定为：

- 统一 `main`
- 手工替换 demo API 切换样例
- demo 本体只负责 build UI
- runtime loop 与 demo 内容解耦

`v2.2` 明确不参考 `LVGL` 的地方：

- 不复制整套 `LVGL` demo 管理框架
- 不做复杂 demo selector
- 不为了表面相似而引入额外 subsystem

## 7. 串行阶段拆分

`v2.2` 固定拆成 4 个串行阶段。

### S0 启动残留面冻结

目标：

- 冻结当前 canonical 启动面
- 冻结 `backend.h` 内容分布
- 冻结 demo `main/run_demo/app_run` 残留面

必须产出：

- 启动入口 inventory
- `backend.h` 拆分 inventory
- demo 迁移清单

完成标准：

- 后续阶段不再边做边重新定义起跑边界

### S1 Runtime 启动骨架收口

目标：

- 把 canonical 启动面固定到 `runtime.h`
- 让 `app.c/app.h` 退出主路径叙事
- 建立统一 runner 所需的最小 runtime API

必须完成：

- 明确唯一 `tinyui_init/screen_create/screen_load/timer_handler/deinit` 主链
- demo 不再依赖 `picoui_app_*` 主路径

完成标准：

- 至少一个主线 demo 已能通过 unified runtime runner 启动
- `app.c` 不再是 canonical usage 示例的前提

### S2 `backend.h` 退场

目标：

- 拆散 `backend.h`
- 删除 `tinyui/src/backend/ldgui/backend.h`

必须完成：

- 按 shared 与 widget-local 真边界完成落位
- 清理所有直接 `#include "../backend/ldgui/backend.h"` 依赖

完成标准：

- 仓库主线代码面已无 `backend.h`
- 不存在替代性的 mega-header 回退

### S3 Demo LVGL-like 化

目标：

- demo `.c` 只保留 build API
- 统一 `main` 持有 runtime lifecycle
- 切换 demo 方式收口到“手工替换 build API”

必须完成：

- 主线 demo 去掉各自 `main()/run_demo()`
- 统一 runner 建立并用于当前主线 demo
- docs 明确新 demo 开发方式

完成标准：

- demo 组织方式与 `LVGL` 常见写法接近
- “如何切 demo” 的答案固定为修改统一 `main` 中调用的 demo API

## 8. 验收与证据

`v2.2` 的完成证据至少包含：

1. configure / build 通过
2. 统一 runner 可启动目标 demo
3. focused unit/runtime proof 通过
4. `backend.h` 已删除且无残留 include
5. 主线 demo 已无各自 `main/run_demo/picoui_app_run` 主路径
6. `git diff --check` 通过

建议固定验证集合：

```bash
rtk cmake -S . -B build
rtk cmake --build build
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
rg -n 'backend\\.h|picoui_app_create|picoui_app_run|run_demo\\(|int main\\(' tinyui/demo tinyui/src tinyui/include
git diff --check
```

说明：

- 具体 `ctest` 子集可在 implementation plan 再细化
- 但 `backend.h` 残留扫描、demo 主路径残留扫描、统一 runner 启动 proof 必须进入 mandatory evidence

## 9. 风险与约束

`v2.2` 的主要风险固定为：

- `backend.h` 当前承载内容较多，拆分不当容易变成新的 shared 污染点
- `app.c` 与 runtime 可能存在历史交织，若边界不先锁清，会出现职责漂移
- demo 迁移若一次性全推，容易把“统一 runner 骨架”与“单个 demo 业务问题”混在一起

对应约束：

- 必须先做 `S0` inventory，再做结构改动
- 统一 runner 先跑通，再批量迁移 demo
- 任何 demo 迁移不得顺手把业务逻辑上推到 runtime 层

## 10. 完成定义

`TinyUI v2.2` 只有在以下条件同时满足时才算完成：

- TinyUI 的 canonical 用户启动方式已经统一到 `runtime.h`
- `backend.h` 已彻底消失
- demo `.c` 已转为 build API 文件
- 统一 `main` 已成为主线 demo 唯一启动入口
- 手工替换 build API 已成为当前标准 demo 切换方式
- `app.c` 已不再代表 TinyUI 的主路径开发模型

若仍存在以下任一情况，则 `v2.2` 不能判完成：

- `backend.h` 仍存在
- 主线 demo 仍各自持有 `main()/run_demo()`
- 仍把 `picoui_app_*` 作为当前 TinyUI canonical 用法
- 统一 runner 还未建立
- demo 切换方式仍需依赖各 demo 自己的启动骨架
