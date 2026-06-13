# TinyUI v2.2 S3 Demo LVGL-Like Runner Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 TinyUI demo 收口成 LVGL-like 结构：demo `.c` 只导出 build API，统一 `main` 负责 runtime lifecycle，切换 demo 方式固定为手工替换 build API。

**Architecture:** 先确定 unified runner 目标和首批主线 demo API 形态，再把 demo 自带 `main()/run_demo()` 迁出，最后更新 CMake wiring 和文档说明。`S3` 不做聚合轮播或动态 demo 发现，只做单 runner + 手工切 demo。

**Tech Stack:** C11、TinyUI runtime/widgets、CMake、CTest、SDL host demo targets。

---

## 文件结构

修改：

- `tinyui/demo/*`
- `tinyui/demo/main.c`
- `cmake/LingDongGUI.cmake`
- `examples/sdl/CMakeLists.txt`
- `docs/v2.2/线计划索引.md`
- `docs/v2.2/plans/stages/README.md`

可能新增：

- `tinyui/demo/<demo_name>/<demo_name>.h`
- `tinyui/demo/<demo_name>/<demo_name>.c`

---

### Task 1: 建立 unified demo runner

**Files:**
- Create or Modify: `tinyui/demo/main.c`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 写 unified runner**

Implement a single canonical runner shape:

```c
int main(void)
{
    tinyui_obj_t *screen;

    if (tinyui_init() != 0) {
        return 1;
    }
    screen = tinyui_screen_create();
    if (screen == NULL) {
        tinyui_deinit();
        return 1;
    }
    if (tinyui_demo_basic_widgets_build(screen) != 0) {
        tinyui_deinit();
        return 1;
    }
    if (tinyui_screen_load(screen) != 0) {
        tinyui_deinit();
        return 1;
    }
    while (1) {
        tinyui_timer_handler();
    }
}
```

The chosen demo call is intentionally manual and explicit. Do not add selector logic.

- [ ] **Step 2: 接 runner 到现有 demo target**

Update CMake wiring so the active demo target builds through the unified runner, not demo-local `main.c`.

- [ ] **Step 3: 跑 configure/build**

Run:

```bash
rtk cmake -S . -B build
rtk cmake --build build
```

Expected: PASS。

### Task 2: 把 demo 文件改成 build API

**Files:**
- Modify or Rename: `tinyui/demo/basic_widgets/*`
- Modify or Rename: `tinyui/demo/settings_panel/*`
- Modify later: remaining `tinyui/demo/*`

- [ ] **Step 1: 选定首批 canonical demo**

Start with at least:

- `tinyui/demo/basic_widgets`
- `tinyui/demo/settings_panel`

These become the first demos rewritten to:

- expose `tinyui_demo_<name>_build(tinyui_obj_t *screen)`
- remove demo-local `main()/run_demo()`

- [ ] **Step 2: 为首批 demo 暴露 build API**

For each selected demo, create the public build signature:

```c
int tinyui_demo_basic_widgets_build(tinyui_obj_t *screen);
int tinyui_demo_settings_panel_build(tinyui_obj_t *screen);
```

The implementation file should only build UI, layout, and callbacks on the provided screen.

- [ ] **Step 3: 移除 demo-local 启动路径**

Delete or rewrite demo-local:

- `main()`
- `run_demo()`
- `tinyui_app_create()/tinyui_app_run()` flow

End state: the demo file can only be used through the unified runner.

- [ ] **Step 4: 跑 focused demo build proof**

Run:

```bash
rg -n 'tinyui_app_create|tinyui_app_run|run_demo\(|int main\(' tinyui/demo
rtk cmake --build build
```

Expected: selected demos no longer contain those startup remnants; build PASS。

### Task 3: 收口 `v2.2` demo 开发方式真相

**Files:**
- Modify: `docs/v2.2/线计划索引.md`
- Modify: `docs/v2.2/plans/stages/README.md`
- Modify later if needed: `tinyui/docs/demo_guide.md`

- [ ] **Step 1: 更新索引和阶段 README**

Record that:

- demo `.c` now exports build APIs only
- unified `tinyui/demo/main.c` owns runtime lifecycle
- switching demo means editing the build API call in `main.c`

- [ ] **Step 2: 更新 demo guide（如当前仓库有 current-facing 入口）**

Document the new canonical usage:

1. include the demo build header
2. call `tinyui_demo_xxx_build(screen)` in unified `main`
3. switch demos by hand-editing the chosen build call

- [ ] **Step 3: 跑 S3 closeout gate**

Run:

```bash
rtk ctest --test-dir build --output-on-failure -R 'test_tinyui|check_tinyui'
rg -n 'tinyui_app_create|tinyui_app_run|run_demo\(|int main\(' tinyui/demo
git diff --check
```

Expected:

- focused canonical tests/checkers PASS
- active demo tree no longer shows per-demo startup skeletons except the unified runner
- format PASS
