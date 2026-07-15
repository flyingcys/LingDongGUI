# TinyUI Demo 运行指南

本文描述 TinyUI v2.3 **current-facing** demo 模型：demo 只表达 UI 意图，**不**拥有 runtime 生命周期。

## 契约

每个 demo 只暴露：

```c
tinyui_result_t tinyui_demo_<name>_build(tinyui_obj_t *screen);
```

统一 runner（`tinyui_demo`）负责：

1. `tinyui_init`
2. 平台宿主接入（显示 / 输入 / 时钟）
3. `tinyui_screen_create`
4. 调用 `tinyui_demo_<name>_build(screen)`
5. `tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0)`（或 runner 选定的过渡）
6. 循环 `tinyui_process(&next_ms)`
7. 清理与 `tinyui_deinit`

### Demo 内禁止

- 调用 `tinyui_init` / `tinyui_deinit` / `tinyui_screen_create` / `tinyui_screen_load` / `tinyui_process`
- 使用历史兼容 API（app / widget / native 前缀族，已移出 canonical）
- 泄漏后端符号或头（LingDongGUI 原生前缀、Arm-2D、旧 signal 宏）
- 引入平台头（如 SDL）到 `tinyui/demo/*` builder

`tinyui_window_*` 是合法 canonical 控件 API，可用于 demo。

## 清单与注册

- 清单：`tests/tinyui/contract/tinyui_demo_manifest.json`（权威名字与数量）
- 注册表：`tinyui/demo/tinyui_demos.h` / `tinyui_demos.c`
- 构建回调类型：`tinyui_demo_build_cb_t`
- 边界门禁：`tests/tinyui/contract/check_tinyui_demo_boundary.py`

当前 runner 以**单一可执行文件** `tinyui_demo` 聚合全部 demo，运行时按名字选择：

```bash
./build/v2.3-m4-demo/examples/sdl/tinyui_demo hello_world
./build/v2.3-m4-demo/examples/sdl/tinyui_demo basic_widgets
```

（具体构建目录随本机 cmake `-B` 而定。）

## 构建

从仓库根目录：

```bash
rtk cmake -S . -B build/v2.3-m4-demo \
  -DENABLE_TEST=ON \
  -DLD_BUILD_SDL_DEMO=ON \
  -DLD_BUILD_RUNTIME_TESTS=ON \
  -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m4-demo -j --target tinyui_demo
```

帮助与列表：

```bash
./build/v2.3-m4-demo/examples/sdl/tinyui_demo --help
```

## 编写新 demo 的最小模板

```c
#include "tinyui.h"

tinyui_result_t tinyui_demo_example_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *label;
    tinyui_obj_t *button;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    if (tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN) != TINYUI_OK) {
        return tinyui_last_result();
    }

    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    if (label == NULL || button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(label, "Hello") != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_button_set_text(button, "OK") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
```

要点：

1. 只接收 runner 提供的 `screen`，在其上创建子树。
2. 资源 descriptor 使用静态/页面级存储（见 [资源生命周期](./resource_lifetime.md)）。
3. 交互使用统一 `tinyui_obj_add_event_cb` 或控件 `set_on_*`。
4. 动态刷新用 `tinyui_timer_create`，不要在 demo 内写帧钩子。

## 自动门禁（摘录）

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk python3 tests/tinyui/contract/check_tinyui_docs_examples.py \
  --build-dir build/v2.3-m4-docs \
  --prefix build/v2.3-m4-demo/_tinyui_install
```

## 常见问题

### 为什么 demo 不能自己 init / process？

生命周期与宿主集成属于 runner。demo 只描述“页面长什么样、如何响应”，才能在 MCU/host 多种宿主间复用同一 builder。

### port 是否已经生产闭环？

否。SDL 路径是开发与可见性验证宿主；MCU port 与生产级 port 仍属延期项，见 `docs/v2.3/deferred-port-work.md`。

### 旧的独立 `tinyui_*_demo` target？

v2.3 以统一 `tinyui_demo` + `build(screen)` 为准；历史多可执行文件布局不再作为 current-facing 真相。
