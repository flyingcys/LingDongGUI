# SDL Demo6 Switch Gallery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增独立 `USE_DEMO == 6` 的 SDL 控件 gallery，保持 `demo2` 不变；gallery 采用鼠标水平滑动翻页、每页单控件展示，第 1 页固定为 `switch`。

**Architecture:** 先用纯逻辑 helper 锁定“水平滑动判定 + 页索引循环”语义，再把 SDL demo6 入口、gallery 公共 chrome、23 个单控件 page 分批接入。翻页手势优先走 page 背景区域，不下沉改动 `Virtual_TFT_Port` 或 `ldGui.c`，从而把风险控制在 SDL demo/widget 示例层。

**Tech Stack:** C, LingDongGUI, Arm-2D, SDL2, CMake, Python, pytest, assert-based host tests, GitNexus MCP

---

### Task 1: 先锁定 demo6 入口与滑动逻辑的 failing tests

**Files:**
- Create: `examples/common/demo/widget/uiWidgetSwipeLogic.h`
- Create: `examples/common/demo/widget/uiWidgetSwipeLogic.c`
- Create: `examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c`
- Create: `examples/sdl/tests/check_use_demo_6_widget_swipe.py`

- [ ] **Step 1: 先定义纯逻辑 helper 边界，只包含可独立测试的手势和索引语义**

```c
/* examples/common/demo/widget/uiWidgetSwipeLogic.h */
#ifndef __UI_WIDGET_SWIPE_LOGIC_H__
#define __UI_WIDGET_SWIPE_LOGIC_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t startX;
    int16_t startY;
    int16_t lastX;
    int16_t lastY;
    bool active;
} uiWidgetSwipeGestureState_t;

typedef enum {
    UI_WIDGET_SWIPE_NONE = 0,
    UI_WIDGET_SWIPE_PREV = -1,
    UI_WIDGET_SWIPE_NEXT = 1,
} uiWidgetSwipeDecision_t;

void uiWidgetSwipeGestureBegin(uiWidgetSwipeGestureState_t *ptState,
                               int16_t x,
                               int16_t y);

void uiWidgetSwipeGestureTrack(uiWidgetSwipeGestureState_t *ptState,
                               int16_t x,
                               int16_t y);

uiWidgetSwipeDecision_t uiWidgetSwipeGestureEnd(uiWidgetSwipeGestureState_t *ptState,
                                                int16_t x,
                                                int16_t y,
                                                int16_t threshold);

uint8_t uiWidgetSwipeWrapIndex(uint8_t currentIndex,
                               int8_t delta,
                               uint8_t pageCount);

bool uiWidgetSwipeIsHorizontalIntent(int16_t dx,
                                     int16_t dy,
                                     int16_t threshold);

#endif
```

- [ ] **Step 2: 先写 host-side failing test，覆盖阈值判定、方向判定、循环翻页**

```c
/* examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c */
#include <assert.h>
#include "uiWidgetSwipeLogic.h"

static void test_wrap_index_loops_from_first_to_last(void)
{
    assert(uiWidgetSwipeWrapIndex(0, -1, 23) == 22);
}

static void test_wrap_index_loops_from_last_to_first(void)
{
    assert(uiWidgetSwipeWrapIndex(22, 1, 23) == 0);
}

static void test_short_drag_does_not_flip_page(void)
{
    uiWidgetSwipeGestureState_t state = {0};
    uiWidgetSwipeGestureBegin(&state, 240, 120);
    uiWidgetSwipeGestureTrack(&state, 280, 126);
    assert(uiWidgetSwipeGestureEnd(&state, 280, 126, 96) == UI_WIDGET_SWIPE_NONE);
}

static void test_left_drag_enters_next_page(void)
{
    uiWidgetSwipeGestureState_t state = {0};
    uiWidgetSwipeGestureBegin(&state, 320, 120);
    uiWidgetSwipeGestureTrack(&state, 180, 122);
    assert(uiWidgetSwipeGestureEnd(&state, 180, 122, 96) == UI_WIDGET_SWIPE_NEXT);
}

static void test_vertical_drag_is_rejected(void)
{
    assert(uiWidgetSwipeIsHorizontalIntent(-120, 20, 96) == true);
    assert(uiWidgetSwipeIsHorizontalIntent(-40, 120, 96) == false);
}

int main(void)
{
    test_wrap_index_loops_from_first_to_last();
    test_wrap_index_loops_from_last_to_first();
    test_short_drag_does_not_flip_page();
    test_left_drag_enters_next_page();
    test_vertical_drag_is_rejected();
    return 0;
}
```

- [ ] **Step 3: 写 `USE_DEMO == 6` 的预处理契约测试，先让它在入口缺失处失败**

```python
from __future__ import annotations

import subprocess
from pathlib import Path

TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
CONFIG = SDL_ROOT / "user" / "ldConfig.h"

PREPROCESS_SOURCE = """
#include "ldConfig.h"
#define LDGUI_STR2(x) #x
#define LDGUI_STR(x) LDGUI_STR2(x)
const char *entry = LDGUI_STR(LD_DEMO_GUI_FUNC);
const char *width = LDGUI_STR(LD_CFG_SCREEN_WIDTH);
const char *height = LDGUI_STR(LD_CFG_SCREEN_HEIGHT);
""".strip()


def test_use_demo_6_widget_swipe_expands_to_new_entry() -> None:
    result = subprocess.run(
        [
            "cc",
            "-E",
            "-P",
            "-x",
            "c",
            "-DUSE_DEMO=6",
            f"-I{CONFIG.parent}",
            '-D__ARM_2D_USER_APP_CFG_H__="ldConfig.h"',
            "-",
        ],
        input=PREPROCESS_SOURCE,
        text=True,
        capture_output=True,
        check=True,
    )

    output = result.stdout
    assert 'const char *entry = "uiWidgetSwipeFunc";' in output
    assert 'const char *width = "(480)";' in output
    assert 'const char *height = "(272)";' in output
```

- [ ] **Step 4: 先跑 python 预处理测试，确认当前会因为 `USE_DEMO == 6` 未接线而失败**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk python3 -m pytest examples/sdl/tests/check_use_demo_6_widget_swipe.py -q
```

Expected: FAIL，断言 `uiWidgetSwipeFunc` 不存在，或 `USE_DEMO == 6` 展开结果不正确。

- [ ] **Step 5: 再跑 host test，确认当前会因为 helper 文件尚不存在而失败**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk cc -std=gnu11 \
  -I./examples/common/demo/widget \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  -o /tmp/test_uiwidget_swipe_logic
```

Expected: 编译失败，提示 `uiWidgetSwipeLogic.c` 或头文件不存在。

- [ ] **Step 6: 给逻辑层补最小桩实现，把失败从“缺文件”推进到“断言失败”**

```c
/* examples/common/demo/widget/uiWidgetSwipeLogic.c */
#include "uiWidgetSwipeLogic.h"

void uiWidgetSwipeGestureBegin(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    (void)ptState;
    (void)x;
    (void)y;
}

void uiWidgetSwipeGestureTrack(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    (void)ptState;
    (void)x;
    (void)y;
}

uiWidgetSwipeDecision_t uiWidgetSwipeGestureEnd(uiWidgetSwipeGestureState_t *ptState,
                                                int16_t x,
                                                int16_t y,
                                                int16_t threshold)
{
    (void)ptState;
    (void)x;
    (void)y;
    (void)threshold;
    return UI_WIDGET_SWIPE_NONE;
}

uint8_t uiWidgetSwipeWrapIndex(uint8_t currentIndex, int8_t delta, uint8_t pageCount)
{
    (void)currentIndex;
    (void)delta;
    (void)pageCount;
    return 0;
}

bool uiWidgetSwipeIsHorizontalIntent(int16_t dx, int16_t dy, int16_t threshold)
{
    (void)dx;
    (void)dy;
    (void)threshold;
    return false;
}
```

- [ ] **Step 7: 再跑 host test，确认现在失败在行为断言而不是缺文件**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk cc -std=gnu11 \
  -I./examples/common/demo/widget \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  -o /tmp/test_uiwidget_swipe_logic && /tmp/test_uiwidget_swipe_logic
```

Expected: 编译通过，但进程在 `assert()` 处失败。

- [ ] **Step 8: 提交 failing tests 与 helper contract**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipeLogic.h \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/sdl/tests/check_use_demo_6_widget_swipe.py
rtk git commit -m "test: add demo6 swipe logic contracts"
```

### Task 2: 实现纯逻辑 helper，并把 `USE_DEMO == 6` 入口接进 SDL

**Files:**
- Modify: `examples/common/demo/widget/uiWidgetSwipeLogic.c`
- Modify: `examples/sdl/user/ldConfig.h`
- Modify: `examples/sdl/CMakeLists.txt`
- Create: `examples/common/demo/widget/uiWidgetSwipe.h`
- Create: `examples/common/demo/widget/uiWidgetSwipePages.h`

- [ ] **Step 1: 把手势判定 helper 实现到能让 host test 转绿**

```c
#include "uiWidgetSwipeLogic.h"

void uiWidgetSwipeGestureBegin(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    if (ptState == NULL) {
        return;
    }
    ptState->startX = x;
    ptState->startY = y;
    ptState->lastX = x;
    ptState->lastY = y;
    ptState->active = true;
}

void uiWidgetSwipeGestureTrack(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    if ((ptState == NULL) || !ptState->active) {
        return;
    }
    ptState->lastX = x;
    ptState->lastY = y;
}

bool uiWidgetSwipeIsHorizontalIntent(int16_t dx, int16_t dy, int16_t threshold)
{
    int16_t absDx = (dx < 0) ? (int16_t)-dx : dx;
    int16_t absDy = (dy < 0) ? (int16_t)-dy : dy;
    return (absDx >= threshold) && (absDx > absDy);
}

uiWidgetSwipeDecision_t uiWidgetSwipeGestureEnd(uiWidgetSwipeGestureState_t *ptState,
                                                int16_t x,
                                                int16_t y,
                                                int16_t threshold)
{
    int16_t dx;
    int16_t dy;

    if ((ptState == NULL) || !ptState->active) {
        return UI_WIDGET_SWIPE_NONE;
    }

    dx = (int16_t)(x - ptState->startX);
    dy = (int16_t)(y - ptState->startY);
    ptState->lastX = x;
    ptState->lastY = y;
    ptState->active = false;

    if (!uiWidgetSwipeIsHorizontalIntent(dx, dy, threshold)) {
        return UI_WIDGET_SWIPE_NONE;
    }

    return (dx < 0) ? UI_WIDGET_SWIPE_NEXT : UI_WIDGET_SWIPE_PREV;
}

uint8_t uiWidgetSwipeWrapIndex(uint8_t currentIndex, int8_t delta, uint8_t pageCount)
{
    int16_t nextIndex = (int16_t)currentIndex + (int16_t)delta;

    if (pageCount == 0U) {
        return 0U;
    }
    if (nextIndex < 0) {
        nextIndex = (int16_t)pageCount - 1;
    } else if (nextIndex >= pageCount) {
        nextIndex = 0;
    }
    return (uint8_t)nextIndex;
}
```

- [ ] **Step 2: 在 `ldConfig.h` 增加 demo6 宏分支，保持尺寸仍为 `480 x 272`**

```c
#ifndef USE_DEMO
// <o> choose demo to test
//     <0=> None
//     <1=> Startup
//     <2=> Show all widget
//     <3=> Printer
//     <4=> Layout
//     <5=> Grid
//     <6=> Widget swipe gallery
#define USE_DEMO                                  (0)
#endif

#if USE_DEMO == 6
#undef LD_CFG_COLOR_DEPTH
#define LD_CFG_COLOR_DEPTH                        (16)
#undef LD_CFG_SCREEN_WIDTH
#define LD_CFG_SCREEN_WIDTH                       (480)
#undef LD_CFG_SCREEN_HEIGHT
#define LD_CFG_SCREEN_HEIGHT                      (272)
#undef LD_CFG_PFB_WIDTH
#define LD_CFG_PFB_WIDTH                          (LD_CFG_SCREEN_WIDTH)
#undef LD_CFG_PFB_HEIGHT
#define LD_CFG_PFB_HEIGHT                         (LD_CFG_SCREEN_HEIGHT/10)
#define LD_DEMO_GUI_INCLUDE                       "uiWidgetSwipe.h"
#define LD_DEMO_GUI_FUNC                          uiWidgetSwipeFunc
#endif
```

- [ ] **Step 3: 在 `CMakeLists.txt` 接入 `USE_DEMO == 6`，复用 widget 资源目录**

```cmake
set(USE_DEMO "2" CACHE STRING "SDL demo id (0=legacy-widget, 1=startup, 2=widget, 3=printer, 4=layout, 5=grid, 6=widget-swipe)")
set_property(CACHE USE_DEMO PROPERTY STRINGS 0 1 2 3 4 5 6)

set(SUPPORTED_DEMOS 0 1 2 3 4 5 6)

elseif(USE_DEMO STREQUAL "6")
    file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS
        "${DEMO_DIR}/widget/*.c"
        "${DEMO_DIR}/widget/fonts/*.c"
        "${DEMO_DIR}/widget/images/*.c"
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/widget"
        "${DEMO_DIR}/widget/fonts"
        "${DEMO_DIR}/widget/images"
    )
endif()
```

- [ ] **Step 4: 创建 gallery 入口头与 page extern 头，先把 page01 作为固定入口符号**

```c
/* examples/common/demo/widget/uiWidgetSwipe.h */
#ifndef __UI_WIDGET_SWIPE_H__
#define __UI_WIDGET_SWIPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldBase.h"
#include "uiWidgetSwipePages.h"

#define uiWidgetSwipeFunc uiWidgetSwipePage01Func

#ifdef __cplusplus
}
#endif

#endif
```

```c
/* examples/common/demo/widget/uiWidgetSwipePages.h */
#ifndef __UI_WIDGET_SWIPE_PAGES_H__
#define __UI_WIDGET_SWIPE_PAGES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "uiWidgetCommon.h"

extern const ldPageFuncGroup_t uiWidgetSwipePage01Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage02Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage03Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage04Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage05Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage06Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage07Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage08Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage09Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage10Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage11Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage12Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage13Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage14Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage15Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage16Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage17Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage18Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage19Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage20Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage21Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage22Func;
extern const ldPageFuncGroup_t uiWidgetSwipePage23Func;

#ifdef __cplusplus
}
#endif

#endif
```

- [ ] **Step 5: 跑两组测试，确认逻辑 helper 与 demo6 宏入口都已转绿**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk python3 -m pytest examples/sdl/tests/check_use_demo_6_widget_swipe.py -q
rtk cc -std=gnu11 \
  -I./examples/common/demo/widget \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  -o /tmp/test_uiwidget_swipe_logic && /tmp/test_uiwidget_swipe_logic
```

Expected: 两条命令都 PASS。

- [ ] **Step 6: 提交 helper 实现和 demo6 入口**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  examples/common/demo/widget/uiWidgetSwipe.h \
  examples/common/demo/widget/uiWidgetSwipePages.h \
  examples/sdl/user/ldConfig.h \
  examples/sdl/CMakeLists.txt
rtk git commit -m "feat: add demo6 gallery entrypoint"
```

### Task 3: 搭好公共 chrome，并先跑通前 3 个页面闭环

**Files:**
- Create: `examples/common/demo/widget/uiWidgetSwipeCommon.h`
- Create: `examples/common/demo/widget/uiWidgetSwipeCommon.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage01.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage02.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage03.c`

- [ ] **Step 1: 先建立公共 chrome / registry / jump helper，避免每个页面各写一套页头和跳转**

```c
/* examples/common/demo/widget/uiWidgetSwipeCommon.h */
#ifndef __UI_WIDGET_SWIPE_COMMON_H__
#define __UI_WIDGET_SWIPE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "uiWidgetCommon.h"
#include "uiWidgetSwipeLogic.h"
#include "uiWidgetSwipePages.h"

#define UI_WIDGET_SWIPE_PAGE_COUNT      23
#define UI_WIDGET_SWIPE_GESTURE_PX      96
#define UI_WIDGET_SWIPE_HEADER_BG       __RGB(36, 78, 112)

typedef struct {
    uint16_t bgId;
    uint16_t headerId;
    uint16_t titleId;
    uint16_t hintId;
    uint16_t helpId;
} uiWidgetSwipeChromeIds_t;

void uiWidgetSwipeInitChrome(ld_scene_t *ptScene,
                             const uiWidgetSwipeChromeIds_t *ptIds,
                             uint8_t pageIndex,
                             ldColor bgColor);

bool uiWidgetSwipeHandleGesture(ld_scene_t *ptScene,
                                uiWidgetSwipeGestureState_t *ptGesture,
                                uint8_t pageIndex,
                                ldMsg_t msg);

void uiWidgetSwipeJumpRelative(uint8_t pageIndex, int8_t delta);

#ifdef __cplusplus
}
#endif

#endif
```

```c
/* examples/common/demo/widget/uiWidgetSwipeCommon.c */
static const ldPageFuncGroup_t *c_pages[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    &uiWidgetSwipePage01Func, &uiWidgetSwipePage02Func, &uiWidgetSwipePage03Func,
    &uiWidgetSwipePage04Func, &uiWidgetSwipePage05Func, &uiWidgetSwipePage06Func,
    &uiWidgetSwipePage07Func, &uiWidgetSwipePage08Func, &uiWidgetSwipePage09Func,
    &uiWidgetSwipePage10Func, &uiWidgetSwipePage11Func, &uiWidgetSwipePage12Func,
    &uiWidgetSwipePage13Func, &uiWidgetSwipePage14Func, &uiWidgetSwipePage15Func,
    &uiWidgetSwipePage16Func, &uiWidgetSwipePage17Func, &uiWidgetSwipePage18Func,
    &uiWidgetSwipePage19Func, &uiWidgetSwipePage20Func, &uiWidgetSwipePage21Func,
    &uiWidgetSwipePage22Func, &uiWidgetSwipePage23Func,
};

static const uint8_t *c_titles[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    (const uint8_t *)"Switch",      (const uint8_t *)"Button",
    (const uint8_t *)"CheckBox",    (const uint8_t *)"LineEdit",
    (const uint8_t *)"ComboBox",    (const uint8_t *)"Slider",
    (const uint8_t *)"ScrollSelecter", (const uint8_t *)"List",
    (const uint8_t *)"Table",       (const uint8_t *)"Label",
    (const uint8_t *)"Text",        (const uint8_t *)"Image",
    (const uint8_t *)"Window",      (const uint8_t *)"MessageBox",
    (const uint8_t *)"ProgressBar", (const uint8_t *)"Gauge",
    (const uint8_t *)"Arc",         (const uint8_t *)"Graph",
    (const uint8_t *)"DateTime",    (const uint8_t *)"Calendar",
    (const uint8_t *)"QRCode",      (const uint8_t *)"RadialMenu",
    (const uint8_t *)"IconSlider",
};

static const uint8_t *c_hints[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    (const uint8_t *)"1 / 23",  (const uint8_t *)"2 / 23",  (const uint8_t *)"3 / 23",
    (const uint8_t *)"4 / 23",  (const uint8_t *)"5 / 23",  (const uint8_t *)"6 / 23",
    (const uint8_t *)"7 / 23",  (const uint8_t *)"8 / 23",  (const uint8_t *)"9 / 23",
    (const uint8_t *)"10 / 23", (const uint8_t *)"11 / 23", (const uint8_t *)"12 / 23",
    (const uint8_t *)"13 / 23", (const uint8_t *)"14 / 23", (const uint8_t *)"15 / 23",
    (const uint8_t *)"16 / 23", (const uint8_t *)"17 / 23", (const uint8_t *)"18 / 23",
    (const uint8_t *)"19 / 23", (const uint8_t *)"20 / 23", (const uint8_t *)"21 / 23",
    (const uint8_t *)"22 / 23", (const uint8_t *)"23 / 23",
};
```

- [ ] **Step 2: 实现手势消息解码，只消费背景区域的 `PRESS / HOLD / RELEASE`**

```c
bool uiWidgetSwipeHandleGesture(ld_scene_t *ptScene,
                                uiWidgetSwipeGestureState_t *ptGesture,
                                uint8_t pageIndex,
                                ldMsg_t msg)
{
    int16_t x;
    int16_t y;
    uiWidgetSwipeDecision_t decision = UI_WIDGET_SWIPE_NONE;

    (void)ptScene;

    switch (msg.signal) {
    case SIGNAL_PRESS:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        uiWidgetSwipeGestureBegin(ptGesture, x, y);
        break;
    case SIGNAL_HOLD_DOWN:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        uiWidgetSwipeGestureTrack(ptGesture, x, y);
        break;
    case SIGNAL_RELEASE:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        decision = uiWidgetSwipeGestureEnd(ptGesture, x, y, UI_WIDGET_SWIPE_GESTURE_PX);
        if (decision == UI_WIDGET_SWIPE_NEXT) {
            uiWidgetSwipeJumpRelative(pageIndex, 1);
        } else if (decision == UI_WIDGET_SWIPE_PREV) {
            uiWidgetSwipeJumpRelative(pageIndex, -1);
        }
        break;
    default:
        break;
    }
    return false;
}

void uiWidgetSwipeJumpRelative(uint8_t pageIndex, int8_t delta)
{
    uint8_t nextIndex = uiWidgetSwipeWrapIndex(pageIndex, delta, UI_WIDGET_SWIPE_PAGE_COUNT);
    const ldPageFuncGroup_t *ptNext = c_pages[nextIndex];
    if (ptNext != NULL) {
        __ldGuiJumpPage((ldPageFuncGroup_t *)ptNext,
                        (delta > 0) ? &ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT
                                    : &ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT,
                        UI_WIDGET_PAGE_SWITCH_ANIM_MS);
    }
}
```

- [ ] **Step 3: 先实现 `Switch` 页，作为新 demo6 的固定首页**

```c
/* examples/common/demo/widget/uiWidgetSwipePage01.c */
#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE01_BG = 0,
    ID_PAGE01_HEADER = 1,
    ID_PAGE01_TITLE = 2,
    ID_PAGE01_HINT = 3,
    ID_PAGE01_HELP = 4,
    ID_PAGE01_SWITCH = 10,
};

static uiWidgetSwipeGestureState_t s_page01Gesture;

static bool uiWidgetSwipePage01Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page01Gesture, 0, msg);
}

static void uiWidgetSwipePage01Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE01_BG,
        .headerId = ID_PAGE01_HEADER,
        .titleId = ID_PAGE01_TITLE,
        .hintId = ID_PAGE01_HINT,
        .helpId = ID_PAGE01_HELP,
    };
    void *obj;

    uiWidgetSwipeInitChrome(ptScene, &ids, 0, __RGB(243, 246, 248));

    obj = ldSwitchInit(ID_PAGE01_SWITCH, ID_PAGE01_BG, 182, 112, 116, 56);
    ldSwitchSetSelectable(obj, true);
    ldSwitchSetChecked(obj, true);

    connect(ID_PAGE01_BG, SIGNAL_PRESS, uiWidgetSwipePage01Gesture);
    connect(ID_PAGE01_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage01Gesture);
    connect(ID_PAGE01_BG, SIGNAL_RELEASE, uiWidgetSwipePage01Gesture);
}

const ldPageFuncGroup_t uiWidgetSwipePage01Func = {
    .init = uiWidgetSwipePage01Init,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage01",
#endif
};
```

- [ ] **Step 4: 再实现 `Button` / `CheckBox` 两页，验证 page 模式可复制**

```c
/* examples/common/demo/widget/uiWidgetSwipePage02.c */
obj = ldButtonInit(ID_PAGE02_BUTTON, ID_PAGE02_BG, 160, 104, 160, 64);
ldButtonSetFont(obj, FONT_ARIAL_16_A8);
ldButtonSetText(obj, (uint8_t *)"Press");
ldButtonSetTextColor(obj, GLCD_COLOR_WHITE);
ldButtonSetImage(obj,
                 IMAGE_KEYRELEASE_PNG,
                 IMAGE_KEYRELEASE_PNG_Mask,
                 IMAGE_KEYPRESS_PNG,
                 IMAGE_KEYPRESS_PNG_Mask);
ldBaseSetSelectable(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage03.c */
obj = ldCheckBoxInit(ID_PAGE03_CHECK_A, ID_PAGE03_BG, 164, 94, 152, 24);
ldCheckBoxSetRadioButtonGroup(obj, 0);
ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option A");
ldCheckBoxSetSelectable(obj, true);

obj = ldCheckBoxInit(ID_PAGE03_CHECK_B, ID_PAGE03_BG, 164, 130, 152, 24);
ldCheckBoxSetRadioButtonGroup(obj, 0);
ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option B");
ldCheckBoxSetSelectable(obj, true);
```

- [ ] **Step 5: 首次构建 demo6，确认入口、page registry、公共 chrome 已经接通**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake -S . -B build/demo6 -DUSE_DEMO=6
rtk cmake --build build/demo6
```

Expected: `ldgui_sdl_demo` 构建成功。

- [ ] **Step 6: 用 dummy 驱动做第一次 smoke，确认日志进入 `uiWidgetSwipePage01`**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rm -f /tmp/demo6-smoke.log
SDL_VIDEODRIVER=dummy ./build/demo6/ldgui_sdl_demo >/tmp/demo6-smoke.log 2>&1 &
pid=$!
sleep 2
kill $pid || true
rtk rg -n "page uiWidgetSwipePage01 init" /tmp/demo6-smoke.log
```

Expected: 日志中至少出现一次 `page uiWidgetSwipePage01 init`。

- [ ] **Step 7: 提交公共层与前三页**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipeCommon.h \
  examples/common/demo/widget/uiWidgetSwipeCommon.c \
  examples/common/demo/widget/uiWidgetSwipePage01.c \
  examples/common/demo/widget/uiWidgetSwipePage02.c \
  examples/common/demo/widget/uiWidgetSwipePage03.c
rtk git commit -m "feat: scaffold demo6 swipe gallery pages"
```

### Task 4: 补齐基础输入与滑动选择组页面（04-09）

**Files:**
- Create: `examples/common/demo/widget/uiWidgetSwipePage04.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage05.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage06.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage07.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage08.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage09.c`

- [ ] **Step 1: 先实现 `LineEdit` 页，并把键盘只作为这一页的辅助控件存在**

```c
/* examples/common/demo/widget/uiWidgetSwipePage04.c */
obj = ldLineEditInit(ID_PAGE04_EDIT, ID_PAGE04_BG, 110, 96, 260, 40, FONT_ARIAL_12, 16);
ldLineEditSetText(obj, (uint8_t *)"switch");
ldLineEditSetKeyboard(obj, UI_WIDGET_KEYBOARD_ID);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);

obj = ldKeyboardInit(UI_WIDGET_KEYBOARD_ID, ID_PAGE04_BG, FONT_ARIAL_12);
```

- [ ] **Step 2: 实现 `ComboBox` 和 `Slider` 两页，分别覆盖点击型和拖动型控件**

```c
/* examples/common/demo/widget/uiWidgetSwipePage05.c */
obj = ldComboBoxInit(ID_PAGE05_COMBO, ID_PAGE05_BG, 146, 106, 188, 32, FONT_ARIAL_12);
ldComboBoxSetStaticItems(obj, g_widget_combo_box_items, 3);
ldComboBoxSetSelectable(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage06.c */
obj = ldSliderInit(ID_PAGE06_SLIDER, ID_PAGE06_BG, 76, 120, 328, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iHeight);
ldSliderSetPercent(obj, 42);
ldSliderSetImage(obj,
                 IMAGE_SLIDER_PNG,
                 IMAGE_SLIDER_PNG_Mask,
                 IMAGE_INDICATOR_PNG,
                 IMAGE_INDICATOR_PNG_Mask);
ldSliderSetIndicatorWidth(obj, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iWidth);
ldSliderSetSelectable(obj, true);
```

- [ ] **Step 3: 实现 `ScrollSelecter`、`List`、`Table` 三页，保持“单页单主控件”**

```c
/* examples/common/demo/widget/uiWidgetSwipePage07.c */
obj = ldScrollSelecterInit(ID_PAGE07_SCROLL, ID_PAGE07_BG, 214, 84, 52, 120, FONT_ARIAL_12);
ldScrollSelecterSetItems(obj, g_widget_scroll_items, 5);
ldScrollSelecterSetBackgroundColor(obj, GLCD_COLOR_WHITE);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage08.c */
ptList = ldListInit(ID_PAGE08_LIST, ID_PAGE08_BG, 96, 78, 288, 122);
ldListSetItemHeight(ptList, 24);
ldListSetText(ptList, g_widget_scroll_items, 5, FONT_ARIAL_12);
ldListSetAlign(ptList, ARM_2D_ALIGN_LEFT);
ldBaseSetSelectable(ptList, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage09.c */
ptTable = ldTableInit(ID_PAGE09_TABLE, ID_PAGE09_BG, 56, 78, 368, 120, 4, 3, 1);
ldTableSetExcelType(ptTable, FONT_ARIAL_12);
ldTableSetKeyboard(ptTable, UI_WIDGET_KEYBOARD_ID);
ldTableSetItemText(ptTable, 1, 1, (uint8_t *)"id");
ldTableSetItemText(ptTable, 1, 2, (uint8_t *)"name");
ldTableSetItemText(ptTable, 1, 3, (uint8_t *)"size");
ldBaseSetSelectable(ptTable, true);
```

- [ ] **Step 4: 构建 demo6 并确认新增 page 符号没有打断 page registry**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake --build build/demo6
```

Expected: 增量构建成功，无 `uiWidgetSwipePage0XFunc` 未定义错误。

- [ ] **Step 5: 提交 04-09 页**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipePage04.c \
  examples/common/demo/widget/uiWidgetSwipePage05.c \
  examples/common/demo/widget/uiWidgetSwipePage06.c \
  examples/common/demo/widget/uiWidgetSwipePage07.c \
  examples/common/demo/widget/uiWidgetSwipePage08.c \
  examples/common/demo/widget/uiWidgetSwipePage09.c
rtk git commit -m "feat: add demo6 input and selection pages"
```

### Task 5: 补齐文本/内容/数据展示组页面（10-18）

**Files:**
- Create: `examples/common/demo/widget/uiWidgetSwipePage10.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage11.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage12.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage13.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage14.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage15.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage16.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage17.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage18.c`

- [ ] **Step 1: 先实现 `Label` / `Text` / `Image` 三页，保持排版简洁**

```c
/* examples/common/demo/widget/uiWidgetSwipePage10.c */
obj = ldLabelInit(ID_PAGE10_LABEL, ID_PAGE10_BG, 94, 92, 292, 72, FONT_ARIAL_16_A8);
ldLabelSetText(obj, (uint8_t *)"Single widget\nper page");
ldLabelSetBackgroundColor(obj, GLCD_COLOR_LIGHT_GREY);
ldLabelSetAlign(obj, ARM_2D_ALIGN_CENTER);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage11.c */
obj = ldTextInit(ID_PAGE11_TEXT, ID_PAGE11_BG, 88, 68, 304, 138, FONT_ARIAL_12, TEXT_BOX_LINE_ALIGN_LEFT, true);
ldTextSetBackgroundImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
ldTextSetText(obj, "Drag on empty space\nto change widget page.");
ldTextSetSelectable(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage12.c */
obj = ldImageInit(ID_PAGE12_IMAGE, ID_PAGE12_BG, 164, 60, 152, 152, NULL, NULL);
ldImageSetImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

- [ ] **Step 2: 再实现 `Window` / `MessageBox` / `ProgressBar` 三页**

```c
/* examples/common/demo/widget/uiWidgetSwipePage13.c */
obj = ldWindowInit(ID_PAGE13_WINDOW, ID_PAGE13_BG, 118, 82, 244, 108);
ldWindowSetColor(obj, __RGB(148, 214, 178));
ldBaseSetCorner(obj, true);
ldBaseSetSelectable(obj, true);

obj = ldLabelInit(ID_PAGE13_LABEL, ID_PAGE13_WINDOW, 20, 34, 180, 24, FONT_ARIAL_12);
ldLabelSetText(obj, (uint8_t *)"Nested window");
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage14.c */
obj = ldMessageBoxInit(ID_PAGE14_MSG, ID_PAGE14_BG, 140, 86, FONT_ARIAL_12);
ldMessageBoxSetTitle(obj, g_widget_message_title);
ldMessageBoxSetMsg(obj, g_widget_message_text);
ldMessageBoxSetBtn(obj, g_widget_message_buttons, 3);
ldMessageBoxSetBackgroundColor(obj, __RGB(255, 255, 255));
ldMessageBoxSetCorner(obj, true);
ldMessageBoxSetHidden(obj, false);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage15.c */
obj = ldProgressBarInit(ID_PAGE15_PROGRESS, ID_PAGE15_BG, 78, 120, 324, 28);
ldProgressBarSetPercent(obj, 45);
ldProgressBarSetImage(obj, IMAGE_PROGRESSBARBG_BMP, NULL, IMAGE_PROGRESSBARFG_BMP, NULL);
ldProgressBarSetSelectable(obj, true);
ldProgressBarSetCorner(obj, true);
```

- [ ] **Step 3: 完成 `Gauge` / `Arc` / `Graph` 三页，并保留最少动态或静态数据**

```c
/* examples/common/demo/widget/uiWidgetSwipePage16.c */
obj = ldGaugeInit(ID_PAGE16_GAUGE, ID_PAGE16_BG, 152, 72, 176, 140, IMAGE_GAUGE_PNG, IMAGE_GAUGE_PNG_Mask, 0, 10);
ldGaugeSetPointerImage(obj, NULL, IMAGE_GAUGEPOINTER_PNG_Mask, 5, 45);
ldGaugeSetPointerColor(obj, GLCD_COLOR_BLUE);
ldGaugeSetAngle(obj, 120.0f);
ldGaugeSetSelectable(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage17.c */
obj = ldArcInit(ID_PAGE17_ARC, ID_PAGE17_BG, 156, 72, 168, 168,
                IMAGE_ARC_QUARTER_PNG_Mask,
                IMAGE_ARC_QUARTER_MASK_PNG_Mask,
                __RGB(240, 240, 240));
ldArcSetBackgroundAngle(obj, 0, 350);
ldArcSetForegroundAngle(obj, 30);
ldArcSetColor(obj, __RGB(173, 216, 230), __RGB(144, 238, 144));
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage18.c */
ptGraph = ldGraphInit(ID_PAGE18_GRAPH, ID_PAGE18_BG, 86, 68, 308, 148, 2);
ldGraphSetAxis(ptGraph, 48, 56, 4);
ldGraphSetGridOffset(ptGraph, 4);
ldGraphAddSeries(ptGraph, GLCD_COLOR_RED, 2, 8);
ldGraphAddSeries(ptGraph, GLCD_COLOR_LIGHT_GREY, 2, 8);
ldGraphSetSelectable(ptGraph, true);
ldGraphSetCorner(ptGraph, true);
```

- [ ] **Step 4: 构建 demo6，并确认中段页面已全部连入**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake --build build/demo6
```

Expected: 构建成功，无 page registry 缺项。

- [ ] **Step 5: 提交 10-18 页**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipePage10.c \
  examples/common/demo/widget/uiWidgetSwipePage11.c \
  examples/common/demo/widget/uiWidgetSwipePage12.c \
  examples/common/demo/widget/uiWidgetSwipePage13.c \
  examples/common/demo/widget/uiWidgetSwipePage14.c \
  examples/common/demo/widget/uiWidgetSwipePage15.c \
  examples/common/demo/widget/uiWidgetSwipePage16.c \
  examples/common/demo/widget/uiWidgetSwipePage17.c \
  examples/common/demo/widget/uiWidgetSwipePage18.c
rtk git commit -m "feat: add demo6 content and data pages"
```

### Task 6: 补齐高级控件组页面（19-23）并完成总验收

**Files:**
- Create: `examples/common/demo/widget/uiWidgetSwipePage19.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage20.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage21.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage22.c`
- Create: `examples/common/demo/widget/uiWidgetSwipePage23.c`

- [ ] **Step 1: 先实现 `DateTime` / `Calendar` / `QRCode` 三页**

```c
/* examples/common/demo/widget/uiWidgetSwipePage19.c */
obj = ldDateTimeInit(ID_PAGE19_DATETIME, ID_PAGE19_BG, 110, 112, 260, 44, FONT_ARIAL_12);
ldBaseSetSelectable(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage20.c */
obj = ldCalendarInit(ID_PAGE20_CALENDAR, ID_PAGE20_BG, 72, 56, 336, 166, FONT_ARIAL_12, 2026, 1, 1);
ldCalendarSetDayNames(obj, g_widget_day_names);
ldCalendarSetHeader(obj, true);
ldCalendarSetHeaderFormat(obj, g_widget_header_format);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage21.c */
obj = ldQRCodeInit(ID_PAGE21_QRCODE, ID_PAGE21_BG, 170, 72, 140, 140,
                   "ldgui", GLCD_COLOR_BLUE, GLCD_COLOR_WHITE, QR_ECC_7, 2, 5);
ldQRCodeSetOpacity(obj, 100);
ldQRCodeSetSelectable(obj, true);
```

- [ ] **Step 2: 再实现 `RadialMenu` / `IconSlider` 两页，完成全部 23 页**

```c
/* examples/common/demo/widget/uiWidgetSwipePage22.c */
obj = ldRadialMenuInit(ID_PAGE22_RADIAL, ID_PAGE22_BG, 102, 102, 276, 108, 68, 46, 5);
ldRadialMenuAddItem(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask);
ldRadialMenuAddItem(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask);
ldRadialMenuAddItem(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask);
ldRadialMenuAddItem(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

```c
/* examples/common/demo/widget/uiWidgetSwipePage23.c */
obj = ldIconSliderInit(ID_PAGE23_ICON_SLIDER, ID_PAGE23_BG, 78, 120, 324, 86, 46, 2, 4, 1, 2, FONT_ARIAL_12);
ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[0]);
ldIconSliderAddIcon(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask, g_widget_icon_names[1]);
ldIconSliderAddIcon(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask, g_widget_icon_names[2]);
ldIconSliderAddIcon(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask, g_widget_icon_names[3]);
ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[4]);
ldBaseSetSelectable(obj, true);
ldBaseSetCorner(obj, true);
```

- [ ] **Step 3: 构建 demo6 最终版，并跑自动 smoke**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake --build build/demo6
rm -f /tmp/demo6-final.log
SDL_VIDEODRIVER=dummy ./build/demo6/ldgui_sdl_demo >/tmp/demo6-final.log 2>&1 &
pid=$!
sleep 2
kill $pid || true
rtk rg -n "page uiWidgetSwipePage01 init" /tmp/demo6-final.log
```

Expected: 构建成功，smoke 日志能进入 page01。

- [ ] **Step 4: 回归构建 `demo2`，证明旧入口未被破坏**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake -S . -B build/demo2-regression -DUSE_DEMO=2
rtk cmake --build build/demo2-regression
```

Expected: `demo2` 构建成功。

- [ ] **Step 5: 跑最终测试矩阵和静态检查**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk python3 -m pytest examples/sdl/tests/check_use_demo_0_legacy_widget.py -q
rtk python3 -m pytest examples/sdl/tests/check_use_demo_6_widget_swipe.py -q
rtk cc -std=gnu11 \
  -I./examples/common/demo/widget \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  -o /tmp/test_uiwidget_swipe_logic && /tmp/test_uiwidget_swipe_logic
rtk git diff --check
```

Expected: 两个 pytest、一个 host test、`git diff --check` 全部通过。

- [ ] **Step 6: 做手工验证并记录最关键的 5 个动作**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk ./build/demo6/ldgui_sdl_demo
```

Expected:

```text
1. 启动即进入 Switch 页
2. 在背景空白区左滑，进入 Button 页
3. 在背景空白区右滑，返回 Switch 页
4. 连续左滑可循环翻页至最后一页再回第一页
5. Switch/Button/CheckBox 至少可完成一次基本交互
```

- [ ] **Step 7: 提交高级页面与最终收口**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetSwipeCommon.h \
  examples/common/demo/widget/uiWidgetSwipeCommon.c \
  examples/common/demo/widget/uiWidgetSwipe.h \
  examples/common/demo/widget/uiWidgetSwipePages.h \
  examples/common/demo/widget/uiWidgetSwipePage*.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.c \
  examples/common/demo/widget/uiWidgetSwipeLogic.h \
  examples/sdl/tests/swipe/test_uiwidget_swipe_logic.c \
  examples/sdl/tests/check_use_demo_6_widget_swipe.py \
  examples/sdl/user/ldConfig.h \
  examples/sdl/CMakeLists.txt
rtk git commit -m "feat: add demo6 swipe widget gallery"
```

- [ ] **Step 8: 提交前用 GitNexus 检查实际影响面，再决定是否补充说明**

Run in MCP:

```text
gitnexus_detect_changes(repo="LingDongGUI", scope="all")
```

Expected: 影响面集中在 `examples/sdl` 与 `examples/common/demo/widget` 的 demo6 新文件及入口配置；若意外触及 `src/gui` 或旧 demo 逻辑，再回到对应任务修正。

---

## Self-Review

- **Spec coverage:** 已覆盖独立 `USE_DEMO == 6` 入口、手势翻页、首尾循环、23 个按类型重排的单控件页面、`demo2` 回归验证、host test / preprocessor test / SDL build smoke。
- **Placeholder scan:** 无 `TODO` / `TBD` / “类似上一步” 占位；每个任务都给了明确文件、代码和验证命令。
- **Type consistency:** 统一使用 `uiWidgetSwipeLogic.*` 负责纯逻辑，`uiWidgetSwipeCommon.*` 负责 GUI 公共层，`uiWidgetSwipePageXX.c` 负责单页内容，入口符号固定为 `uiWidgetSwipeFunc -> uiWidgetSwipePage01Func`。

