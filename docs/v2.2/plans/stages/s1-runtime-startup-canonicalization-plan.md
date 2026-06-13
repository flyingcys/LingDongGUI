# TinyUI v2.2 S1 Runtime Startup Canonicalization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 TinyUI 的 canonical 启动路径固定到 `runtime.h`，让 `app.h/app.c` 退出用户主路径叙事，并为统一 demo runner 建立最小 runtime 骨架。

**Architecture:** 先收 runtime public surface，再缩退或拆平 `app.h/app.c` 旧主路径，最后建立至少一个 demo 可用的 unified runtime runner 前置骨架。`S1` 不做 `backend.h` 拆散和全量 demo 迁移，只做启动主线 canonicalization。

**Tech Stack:** C11、TinyUI runtime/core、CMake、CTest。

---

## 文件结构

修改：

- `tinyui/include/runtime.h`
- `tinyui/include/app.h`
- `tinyui/src/core/app.c`
- `tinyui/src/core/runtime_host.c`
- `tinyui/src/core/runtime_bridge.c`
- `tinyui/src/core/internal.h`
- `cmake/LingDongGUI.cmake`

可能新增：

- `tinyui/demo/main.c`（若先以最小 runner 形式引入）

---

### Task 1: 收口 canonical runtime public surface

**Files:**
- Modify: `tinyui/include/runtime.h`
- Modify: `tinyui/src/core/runtime_host.c`
- Modify: `tinyui/src/core/runtime_bridge.c`
- Modify: `tinyui/src/core/internal.h`

- [ ] **Step 1: 先写 fail-first API 使用面扫描**

Run:

```bash
rg -n 'tinyui_app_create|tinyui_app_run|tinyui_app_switch_window' tinyui/demo tinyui/include tinyui/src
```

Expected: 当前仍有旧 app 主路径命中。

- [ ] **Step 2: 在 `runtime.h` 固定 canonical 启动集合**

Ensure `tinyui/include/runtime.h` exposes only the startup path needed by unified runner:

```c
int tinyui_init(void);
void tinyui_deinit(void);
tinyui_obj_t *tinyui_screen_create(void);
int tinyui_screen_load(tinyui_obj_t *screen);
void tinyui_timer_handler(void);
```

If current implementation still expresses these through `tinyui_*` aliases or mixed naming, rewrite the public face so the header itself reads as TinyUI canonical truth.

- [ ] **Step 3: 编译最小 runtime public surface**

Run:

```bash
rtk cmake --build build --target test_tinyui_runtime_model
```

Expected: PASS。

### Task 2: 让 `app.h/app.c` 退出主路径叙事

**Files:**
- Modify: `tinyui/include/app.h`
- Modify: `tinyui/src/core/app.c`
- Modify: `tinyui/src/core/runtime_host.c`
- Modify: `tinyui/src/core/runtime_bridge.c`

- [ ] **Step 1: 写 fail-first 主路径残留扫描**

Run:

```bash
rg -n 'tinyui_app_create|tinyui_app_run|tinyui_app_set_window|tinyui_app_switch_window' tinyui/include tinyui/src tinyui/demo
```

Expected: 输出旧主路径残留。

- [ ] **Step 2: 缩退 `app.h` 到非-canonical 角色**

Refactor `tinyui/include/app.h` so it no longer represents the recommended user startup surface.

Required end state:

- demo / runtime runner 不再需要 include `app.h`
- 若保留少量 timer/lifecycle API，也应明确降级为 internal or non-canonical support role

- [ ] **Step 3: 缩退 `app.c` 到 internal/lifecycle 薄职责**

Move or eliminate user-entry responsibilities from `tinyui/src/core/app.c`.

Allowed end state:

- timer pump / lifecycle small helper remains
- user startup / run / switch-window orchestration no longer lives here as canonical path

- [ ] **Step 4: 跑 focused lifecycle proof**

Run:

```bash
rtk ctest --test-dir build --output-on-failure -R '^(test_tinyui_runtime_model|test_tinyui_app_lifecycle)$'
```

Expected: PASS。

### Task 3: 建立 unified runner 最小前提

**Files:**
- Modify: `tinyui/include/runtime.h`
- Modify: `cmake/LingDongGUI.cmake`
- Modify: `examples/sdl/CMakeLists.txt`
- Create or Modify: `tinyui/demo/main.c`

- [ ] **Step 1: 建 runner 最小骨架**

Add or wire a single canonical runner shape:

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
    if (tinyui_screen_load(screen) != 0) {
        tinyui_deinit();
        return 1;
    }
    while (1) {
        tinyui_timer_handler();
    }
}
```

At `S1`, the demo build call may still be a placeholder hook point, but the runtime lifecycle path itself must be canonical and runnable.

- [ ] **Step 2: 跑 configure/build**

Run:

```bash
rtk cmake -S . -B build
rtk cmake --build build
```

Expected: PASS。

- [ ] **Step 3: 更新 S1 文档真相**

Update:

- `docs/v2.2/线计划索引.md`
- `docs/v2.2/plans/stages/README.md`

Record that:

- runtime.h is now the only canonical startup path
- `app.h/app.c` no longer define the main user-entry story
- unified runner prerequisites are in place for `S3`
