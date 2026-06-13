# TinyUI v2.0 P2 Pilot Widgets Flatten Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `window/label/button/switch` 上证明目标架构可行：移除它们对专用 `backend_*` wrapper 的依赖，改成经 shared core helpers 直接调用 `ld*`。

**Architecture:** 保留 public widget wrapper 的存在，但把 `backend_window/backend_label/backend_button/backend_switch` 的创建与 setter 胶水吸收到 widget-local 或 shared core helpers 中，并在不破坏现有 demo/tests 的前提下，把这组试点控件对应的 `backend` 压到零或接近零。

**Tech Stack:** C11、现有 `tinyui` widget structs、`LingDongGUI` `ldWindow/ldLabel/ldButton/ldSwitch`、当前 unit/runtime/visible gates。

---

## 文件结构

修改：

- `tinyui/src/widgets/window.c`
- `tinyui/src/widgets/label.c`
- `tinyui/src/widgets/button.c`
- `tinyui/src/widgets/switch.c`
- `tinyui/src/core/internal.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_window.c`
- `tinyui/src/backend/ldgui/backend_label.c`
- `tinyui/src/backend/ldgui/backend_button.c`
- `tinyui/src/backend/ldgui/backend_switch.c`
- `tests/tinyui/unit/test_tinyui_window.c`
- `tests/tinyui/unit/test_tinyui_label.c`
- `tests/tinyui/unit/test_tinyui_button_events.c`
- `tests/tinyui/unit/test_tinyui_switch.c`
- `tests/tinyui/runtime/check_tinyui_backend_mapping.py`

---

### Task 1: 把 `window/label/button` 改成 widget-local 直连

**Files:**
- Modify: `tinyui/src/widgets/window.c`
- Modify: `tinyui/src/widgets/label.c`
- Modify: `tinyui/src/widgets/button.c`
- Modify: `tinyui/src/core/internal.h`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tests/tinyui/unit/test_tinyui_window.c`
- Modify: `tests/tinyui/unit/test_tinyui_label.c`
- Modify: `tests/tinyui/unit/test_tinyui_button_events.c`

- [ ] **Step 1: 写 fail-first constructor truth tests**

Append to `tests/tinyui/unit/test_tinyui_window.c`:

```c
extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);

static void test_window_constructor_binds_ld_without_backend_wrapper(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");

    assert(app != NULL);
    assert(win != NULL);
    assert(tinyui_widget_has_ld_binding(&win->widget) == 1);
    tinyui_app_destroy(app);
}
```

- [ ] **Step 2: 运行 focused build，确认 helper 缺失**

Run:

```bash
rtk cmake --build build --target test_tinyui_window test_tinyui_label test_tinyui_button_events
```

Expected: FAIL，缺少 `tinyui_widget_has_ld_binding` 或现有实现不能满足新断言。

- [ ] **Step 3: 把 widget-local ld creation 写回 widget 文件**

Add to `tinyui/src/core/internal.h`:

```c
int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
uint16_t tinyui_runtime_next_name_id(struct tinyui_app *app);
```

Add to `tinyui/src/core/widget.c`:

```c
int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget)
{
    return widget != 0
        && widget->backend_widget != 0
        && ((struct tinyui_backend_widget *)widget->backend_widget)->ld_widget != 0;
}
```

Representative constructor rewrite in `tinyui/src/widgets/label.c`:

```c
backend = tinyui_widget_backend_alloc(PICOUI_BACKEND_WIDGET_LABEL, parent, id);
backend->ld_widget = ldLabel_init(state->ld_scene, NULL, name_id, parent_id, 0, 0, 120, 24);
backend->ld_name_id = name_id;
```

Representative constructor rewrite in `tinyui/src/widgets/button.c`:

```c
backend = tinyui_widget_backend_alloc(PICOUI_BACKEND_WIDGET_BUTTON, parent, id);
backend->ld_widget = ldButton_init(state->ld_scene, NULL, name_id, parent_id, 0, 0, 120, 36);
backend->ld_name_id = name_id;
```

Window/root constructor rewrite in `tinyui/src/widgets/window.c` should similarly own `ldWindow_init(...)` instead of delegating to `backend_window.c`.

- [ ] **Step 4: backend pilot files 降成 thin shim 或清空调用点**

In `backend_window.c/backend_label.c/backend_button.c`, replace constructor bodies with one of:

```c
/* thin shim kept temporarily for non-pilot callers; main path now lives in widgets/*.c */
return 0;
```

or remove the call sites completely and leave the file only for compatibility helpers still referenced elsewhere. Do not invent a new shared backend abstraction.

- [ ] **Step 5: 跑 focused tests**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_window|test_tinyui_label|test_tinyui_button_events)$' --output-on-failure
```

Expected: PASS。

### Task 2: 把 `switch` 直连 `ldSwitch`

**Files:**
- Modify: `tinyui/src/widgets/switch.c`
- Modify: `tinyui/src/backend/ldgui/backend_switch.c`
- Modify: `tests/tinyui/unit/test_tinyui_switch.c`
- Modify: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Read: `src/gui/ldSwitch.c`

- [ ] **Step 1: 写 fail-first switch binding test**

Append to `tests/tinyui/unit/test_tinyui_switch.c`:

```c
extern int tinyui_switch_uses_direct_ld_path(const struct tinyui_switch *sw);

static void test_switch_direct_ld_path_contract(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_switch *sw = tinyui_switch_create(win, "wifi");

    assert(app != NULL);
    assert(win != NULL);
    assert(sw != NULL);
    assert(tinyui_switch_uses_direct_ld_path(sw) == 1);
    tinyui_app_destroy(app);
}
```

- [ ] **Step 2: 运行 test，确认缺 helper**

Run:

```bash
rtk cmake --build build --target test_tinyui_switch
```

Expected: FAIL。

- [ ] **Step 3: 让 `switch.c` 直接创建和操作 `ldSwitch`**

In `tinyui/src/widgets/switch.c`, replace:

```c
sw->widget.backend_widget = tinyui_backend_create_switch(parent->widget.backend_widget, id);
```

with representative direct path:

```c
backend = tinyui_widget_backend_alloc(PICOUI_BACKEND_WIDGET_SWITCH, &parent->widget, id);
backend->ld_widget = ldSwitch_init(state->ld_scene, NULL, name_id, parent_id, 0, 0, 48, 24);
backend->ld_name_id = name_id;
sw->widget.backend_widget = backend;
ldSwitchSetColor((ldSwitch_t *)backend->ld_widget,
                 __RGB(224, 224, 224),
                 __RGB(33, 150, 243),
                 GLCD_COLOR_WHITE,
                 GLCD_COLOR_WHITE);
```

Add helper in `tinyui/src/widgets/switch.c`:

```c
int tinyui_switch_uses_direct_ld_path(const struct tinyui_switch *sw)
{
    return sw != 0 &&
        sw->widget.backend_widget != 0 &&
        ((struct tinyui_backend_widget *)sw->widget.backend_widget)->ld_widget != 0;
}
```

Setter/getter helpers should call `ldSwitchSetHorizontal`, `ldSwitchSetDirection`, `ldSwitchSetDisabled`, and direct state readback instead of delegating through `backend_switch.c`.

- [ ] **Step 4: 更新 mapping gate 的试点假设**

In `tests/tinyui/runtime/check_tinyui_backend_mapping.py`, replace wording that assumes every widget must transit a dedicated `backend_*` file with wording that accepts either:

```python
"pilot widget moved to widget-local ld binding"
```

for `window/label/button/switch`.

- [ ] **Step 5: 跑 focused + visible gates**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_switch|test_tinyui_window|test_tinyui_label|test_tinyui_button_events)$' --output-on-failure
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure
```

Expected: PASS。

- [ ] **Step 6: Commit**

```bash
git add \
  tinyui/src/core/internal.h \
  tinyui/src/core/widget.c \
  tinyui/src/widgets/window.c \
  tinyui/src/widgets/label.c \
  tinyui/src/widgets/button.c \
  tinyui/src/widgets/switch.c \
  tinyui/src/backend/ldgui/backend_window.c \
  tinyui/src/backend/ldgui/backend_label.c \
  tinyui/src/backend/ldgui/backend_button.c \
  tinyui/src/backend/ldgui/backend_switch.c \
  tests/tinyui/unit/test_tinyui_window.c \
  tests/tinyui/unit/test_tinyui_label.c \
  tests/tinyui/unit/test_tinyui_button_events.c \
  tests/tinyui/unit/test_tinyui_switch.c \
  tests/tinyui/runtime/check_tinyui_backend_mapping.py
git commit -m "refactor: flatten pilot tinyui widgets"
```
