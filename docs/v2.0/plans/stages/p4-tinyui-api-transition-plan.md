# TinyUI v2.0 P4 TinyUI API Transition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 public surface 开始收口到 `tinyui_*`，同时为现有 `tinyui_*` 使用者保留受控兼容层。

**Architecture:** 先新增 `tinyui_*` headers 与 aliases，并接到已经拆平的 `widgets/core` 实现上；再更新 guard checker 强制 `tinyui_*` API count 非零；最后把试点 demo 与最小 tests 改到新命名。

**Tech Stack:** C11 headers、compatibility wrappers/macros、CMake、现有试点 widget/runtime 代码、当前 demo/test tree。

---

## 文件结构

新增：

- `tinyui/include/tinyui.h`
- `tinyui/include/core.h`
- `tinyui/include/obj.h`
- `tinyui/include/screen.h`
- `tinyui/include/label.h`
- `tinyui/include/button.h`
- `tinyui/include/switch.h`

修改：

- `tinyui/include/tinyui/tinyui.h`
- `tinyui/include/tinyui/runtime.h`
- `tinyui/include/tinyui/widget.h`
- `tinyui/include/tinyui/window.h`
- `tinyui/include/tinyui/label.h`
- `tinyui/include/tinyui/button.h`
- `tinyui/include/tinyui/switch.h`
- `tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py`
- `tests/tinyui/contract/tinyui_tinyui_transition_inventory.json`
- `tinyui/demo/basic_widgets/main.c`

---

### Task 1: 引入 `tinyui_*` public headers

**Files:**
- Create: `tinyui/include/tinyui.h`
- Create: `tinyui/include/core.h`
- Create: `tinyui/include/obj.h`
- Create: `tinyui/include/screen.h`
- Create: `tinyui/include/label.h`
- Create: `tinyui/include/button.h`
- Create: `tinyui/include/switch.h`
- Modify: `tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py`
- Modify: `tests/tinyui/contract/tinyui_tinyui_transition_inventory.json`

- [ ] **Step 1: 写 fail-first tinyui header contract**

Append to `tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py`:

```python
TINYUI_HEADERS = [
    ROOT / "tinyui" / "include" / "tinyui.h",
    ROOT / "tinyui" / "include" / "core.h",
    ROOT / "tinyui" / "include" / "obj.h",
    ROOT / "tinyui" / "include" / "screen.h",
]

for header in TINYUI_HEADERS:
    if not header.exists():
        print(f"missing tinyui header: {header}", file=sys.stderr)
        return 1
```

- [ ] **Step 2: 运行 checker，确认 fail**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py
```

Expected: FAIL，缺少 `tinyui/include/*`。

- [ ] **Step 3: 新建 `tinyui` public 头**

Create `tinyui/include/tinyui.h`:

```c
#ifndef TINYUI_H
#define TINYUI_H

#include "core.h"
#include "obj.h"
#include "screen.h"
#include "label.h"
#include "button.h"
#include "switch.h"

#endif
```

Create `tinyui/include/obj.h`:

```c
#ifndef TINYUI_OBJ_H
#define TINYUI_OBJ_H

#include "tinyui/widget.h"

typedef struct tinyui_widget tinyui_obj_t;

#endif
```

Create `tinyui/include/screen.h`:

```c
#ifndef TINYUI_SCREEN_H
#define TINYUI_SCREEN_H

#include "obj.h"
#include "tinyui/window.h"

int tinyui_init(void);
void tinyui_deinit(void);
tinyui_obj_t *tinyui_screen_create(void);
int tinyui_screen_load(tinyui_obj_t *screen);
void tinyui_timer_handler(void);

#endif
```

`label.h/button.h/switch.h/core.h` 采用同样策略：对外暴露 `tinyui_*` 声明，底层类型/函数先映射到现有 flattened implementation。

- [ ] **Step 4: 更新 baseline inventory**

Update `tests/tinyui/contract/tinyui_tinyui_transition_inventory.json`:

```json
{
  "baseline": {
    "backend_c_files": 35,
    "app_header_exists": true,
    "app_source_exists": true,
    "tinyui_public_api_count": 548,
    "tinyui_public_api_count": 18
  }
}
```

若当前 checkout 实测 `tinyui_*` 计数与 `18` 不一致，以实测值为准，不要保留旧示例数。

- [ ] **Step 5: 跑 guard checker**

Run:

```bash
python3 tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py
rtk ctest --test-dir build -R '^check_tinyui_tinyui_transition_guards$' --output-on-failure
```

Expected: PASS。

### Task 2: 迁移试点 demo 和最小 tests 到 `tinyui_*`

**Files:**
- Modify: `tinyui/demo/basic_widgets/main.c`
- Modify: `tinyui/include/tinyui/tinyui.h`
- Modify: `tinyui/include/tinyui/runtime.h`
- Modify: `tinyui/include/tinyui/widget.h`
- Modify: `tinyui/include/tinyui/window.h`
- Modify: `tinyui/include/tinyui/label.h`
- Modify: `tinyui/include/tinyui/button.h`
- Modify: `tinyui/include/tinyui/switch.h`
- Modify: `tests/tinyui/unit/test_tinyui_runtime_model.c`
- Modify: `tests/tinyui/unit/test_tinyui_switch.c`

- [ ] **Step 1: 新旧 API 建立一对一 compatibility bridge**

In `tinyui/include/tinyui/runtime.h`, add:

```c
#define tinyui_init tinyui_init
#define tinyui_deinit tinyui_deinit
#define tinyui_timer_handler tinyui_timer_handler
```

In `tinyui/include/tinyui/window.h`, add:

```c
#define tinyui_screen_create tinyui_screen_create
#define tinyui_screen_load tinyui_screen_load
```

In `tinyui/include/tinyui/switch.h`, add:

```c
#define tinyui_switch_create(parent) ((tinyui_obj_t *)tinyui_switch_create((struct tinyui_window *)(parent), "switch"))
```

If macros become too messy, prefer `static inline` wrappers in the new `tinyui/include/*.h`.

- [ ] **Step 2: 把 `basic_widgets` demo 改到 `tinyui_*`**

In `tinyui/demo/basic_widgets/main.c`, change includes and calls:

```c
#include "tinyui.h"

...
if (tinyui_init() != 0) {
    return 1;
}
screen = tinyui_screen_create();
...
tinyui_screen_load(screen);
tinyui_timer_handler();
```

Within the widget section, use `tinyui_*` aliases for the pilot set wherever available.

- [ ] **Step 3: 把 focused test 改成认 `tinyui_*`**

Representative update in `tests/tinyui/unit/test_tinyui_runtime_model.c`:

```c
assert(tinyui_init() == 0);
screen = tinyui_screen_create();
assert(screen != NULL);
assert(tinyui_screen_load(screen) == 0);
tinyui_deinit();
```

- [ ] **Step 4: 跑 focused + visible gates**

Run:

```bash
rtk ctest --test-dir build -R '^(test_tinyui_runtime_model|test_tinyui_switch)$' --output-on-failure
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui' --output-on-failure
```

Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add \
  tinyui/include/tinyui.h \
  tinyui/include/core.h \
  tinyui/include/obj.h \
  tinyui/include/screen.h \
  tinyui/include/label.h \
  tinyui/include/button.h \
  tinyui/include/switch.h \
  tinyui/include/tinyui/tinyui.h \
  tinyui/include/tinyui/runtime.h \
  tinyui/include/tinyui/widget.h \
  tinyui/include/tinyui/window.h \
  tinyui/include/tinyui/label.h \
  tinyui/include/tinyui/button.h \
  tinyui/include/tinyui/switch.h \
  tinyui/demo/basic_widgets/main.c \
  tests/tinyui/unit/test_tinyui_runtime_model.c \
  tests/tinyui/unit/test_tinyui_switch.c \
  tests/tinyui/contract/check_tinyui_tinyui_transition_guards.py \
  tests/tinyui/contract/tinyui_tinyui_transition_inventory.json
git commit -m "feat: introduce tinyui pilot public api"
```
