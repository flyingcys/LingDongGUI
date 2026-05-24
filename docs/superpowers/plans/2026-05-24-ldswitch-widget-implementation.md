# LD Switch Widget Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 LingDongGUI 新增一个原生 `ldSwitch` 控件，对齐 LVGL switch 的核心视觉和交互特征，支持横竖方向、禁用态、程序设值和值变化事件，并接入 SDL widget demo 与文档链路。

**Architecture:** 保持公开控件实现位于 `src/gui/ldSwitch.h/.c`，把 knob 几何和动画推进拆到 `src/gui/ldSwitchInternal.h/.c`，先用 host-side assert 测试锁定纯逻辑，再把消息、渲染和 demo 集成串起来。现有 `ldButton`、`ldCheckBox`、`ldSlider` 不做语义重构，只复用消息、dirty region 和基础绘制能力。

**Tech Stack:** C, LingDongGUI, Arm-2D, SDL2, CMake, Python, assert-based host tests

---

### Task 1: 建立 switch 内部辅助边界与 failing host tests

**Files:**
- Create: `src/gui/ldSwitchInternal.h`
- Create: `src/gui/ldSwitchInternal.c`
- Create: `examples/sdl/tests/switch/test_ldswitch_internal.c`

- [ ] **Step 1: 先写内部辅助 API 边界，只定义本轮必须可测的纯逻辑**

```c
/* src/gui/ldSwitchInternal.h */
#ifndef __LD_SWITCH_INTERNAL_H__
#define __LD_SWITCH_INTERNAL_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t trackStart;
    int16_t trackLength;
    int16_t knobSize;
} ldSwitchAxisMetrics_t;

typedef struct {
    uint16_t start;
    uint16_t target;
    uint16_t current;
    uint16_t elapsedMs;
    uint16_t durationMs;
    bool running;
} ldSwitchAnimState_t;

ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal);

uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics,
                                   uint16_t animProgress);

bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim,
                              uint16_t deltaMs,
                              uint16_t *pOutProgress);

#endif
```

- [ ] **Step 2: 先把 failing test 写出来，覆盖横向、纵向、动画推进三个核心语义**

```c
/* examples/sdl/tests/switch/test_ldswitch_internal.c */
static void test_horizontal_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t m = ldSwitchResolveAxisMetrics(44, 24, 2, true);
    assert(m.knobSize == 20);
    assert(m.trackLength == 20);
}

static void test_vertical_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t m = ldSwitchResolveAxisMetrics(24, 44, 2, false);
    assert(m.knobSize == 20);
    assert(m.trackLength == 20);
}

static void test_anim_progress_reaches_target(void)
{
    ldSwitchAnimState_t anim = {
        .start = 0,
        .target = 1000,
        .current = 0,
        .elapsedMs = 0,
        .durationMs = 150,
        .running = true,
    };
    uint16_t progress = 0;

    assert(ldSwitchAdvanceAnimation(&anim, 75, &progress) == true);
    assert(progress > 0 && progress < 1000);
    assert(ldSwitchAdvanceAnimation(&anim, 75, &progress) == false);
    assert(progress == 1000);
}
```

- [ ] **Step 3: 给测试加最小 `main()`，保证它能作为独立 host test 运行**

```c
int main(void)
{
    test_horizontal_metrics_reserve_knob_padding();
    test_vertical_metrics_reserve_knob_padding();
    test_anim_progress_reaches_target();
    return 0;
}
```

- [ ] **Step 4: 先跑测试，确认当前基线必然失败**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
cc -std=gnu11 -I./src/gui \
  examples/sdl/tests/switch/test_ldswitch_internal.c \
  src/gui/ldSwitchInternal.c \
  -o /tmp/test_ldswitch_internal
```

Expected: 编译失败，提示 `src/gui/ldSwitchInternal.c` 或头文件尚不存在。

- [ ] **Step 5: 写最小桩实现，让测试从“缺文件”推进到“断言失败”**

```c
/* src/gui/ldSwitchInternal.c */
#include "ldSwitchInternal.h"

ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal)
{
    (void)width;
    (void)height;
    (void)knobPadding;
    (void)isHorizontal;
    return (ldSwitchAxisMetrics_t){0};
}

uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics,
                                   uint16_t animProgress)
{
    (void)ptMetrics;
    (void)animProgress;
    return 0;
}

bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim,
                              uint16_t deltaMs,
                              uint16_t *pOutProgress)
{
    (void)ptAnim;
    (void)deltaMs;
    *pOutProgress = 0;
    return false;
}
```

- [ ] **Step 6: 再跑测试，确认现在失败在行为断言而不是缺文件**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
cc -std=gnu11 -I./src/gui \
  examples/sdl/tests/switch/test_ldswitch_internal.c \
  src/gui/ldSwitchInternal.c \
  -o /tmp/test_ldswitch_internal && /tmp/test_ldswitch_internal
```

Expected: 编译通过，但进程在 `assert()` 处失败。

- [ ] **Step 7: 提交测试入口和内部边界**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  src/gui/ldSwitchInternal.h \
  src/gui/ldSwitchInternal.c \
  examples/sdl/tests/switch/test_ldswitch_internal.c
rtk git commit -m "test: add switch internal contract tests"
```

### Task 2: 实现内部几何与动画逻辑，让 host tests 转绿

**Files:**
- Modify: `src/gui/ldSwitchInternal.c`
- Modify: `examples/sdl/tests/switch/test_ldswitch_internal.c`

- [ ] **Step 1: 补齐横向 / 纵向几何计算最小实现**

```c
ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal)
{
    ldSwitchAxisMetrics_t metrics = {0};
    int16_t major = isHorizontal ? width : height;
    int16_t minor = isHorizontal ? height : width;
    int16_t knob = minor - (int16_t)(knobPadding * 2);

    if(knob < 0) {
        knob = 0;
    }

    metrics.trackStart = knobPadding;
    metrics.knobSize = knob;
    metrics.trackLength = major - knob - (int16_t)(knobPadding * 2);
    if(metrics.trackLength < 0) {
        metrics.trackLength = 0;
    }
    return metrics;
}
```

- [ ] **Step 2: 实现 knob offset 和动画推进逻辑**

```c
uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics,
                                   uint16_t animProgress)
{
    if(ptMetrics == NULL) {
        return 0;
    }
    if(animProgress > 1000) {
        animProgress = 1000;
    }
    return (uint16_t)((ptMetrics->trackLength * animProgress) / 1000);
}

bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim,
                              uint16_t deltaMs,
                              uint16_t *pOutProgress)
{
    uint32_t span;
    uint32_t value;

    if(ptAnim == NULL || pOutProgress == NULL) {
        return false;
    }

    if(!ptAnim->running || ptAnim->durationMs == 0) {
        ptAnim->current = ptAnim->target;
        *pOutProgress = ptAnim->current;
        ptAnim->running = false;
        return false;
    }

    ptAnim->elapsedMs = (uint16_t)(ptAnim->elapsedMs + deltaMs);
    if(ptAnim->elapsedMs >= ptAnim->durationMs) {
        ptAnim->elapsedMs = ptAnim->durationMs;
    }

    span = (ptAnim->target >= ptAnim->start)
        ? (uint32_t)(ptAnim->target - ptAnim->start)
        : (uint32_t)(ptAnim->start - ptAnim->target);

    value = ((uint32_t)span * ptAnim->elapsedMs) / ptAnim->durationMs;
    ptAnim->current = (ptAnim->target >= ptAnim->start)
        ? (uint16_t)(ptAnim->start + value)
        : (uint16_t)(ptAnim->start - value);

    if(ptAnim->elapsedMs >= ptAnim->durationMs) {
        ptAnim->current = ptAnim->target;
        ptAnim->running = false;
    }

    *pOutProgress = ptAnim->current;
    return ptAnim->running;
}
```

- [ ] **Step 3: 扩测试，补 knob offset 边界和反向动画场景**

```c
static void test_knob_offset_matches_progress(void)
{
    ldSwitchAxisMetrics_t m = {.trackStart = 2, .trackLength = 20, .knobSize = 20};
    assert(ldSwitchResolveKnobOffset(&m, 0) == 0);
    assert(ldSwitchResolveKnobOffset(&m, 500) == 10);
    assert(ldSwitchResolveKnobOffset(&m, 1000) == 20);
}

static void test_anim_progress_supports_reverse_direction(void)
{
    ldSwitchAnimState_t anim = {
        .start = 1000,
        .target = 0,
        .current = 1000,
        .elapsedMs = 0,
        .durationMs = 150,
        .running = true,
    };
    uint16_t progress = 0;

    assert(ldSwitchAdvanceAnimation(&anim, 150, &progress) == false);
    assert(progress == 0);
}
```

- [ ] **Step 4: 跑 host tests，确认纯逻辑全部转绿**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
cc -std=gnu11 -I./src/gui \
  examples/sdl/tests/switch/test_ldswitch_internal.c \
  src/gui/ldSwitchInternal.c \
  -o /tmp/test_ldswitch_internal && /tmp/test_ldswitch_internal
```

Expected: 进程退出码为 0，无断言失败。

- [ ] **Step 5: 提交内部逻辑实现**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add src/gui/ldSwitchInternal.c examples/sdl/tests/switch/test_ldswitch_internal.c
rtk git commit -m "feat: implement switch internal metrics and animation"
```

### Task 3: 落地公开 `ldSwitch` 控件与事件 / 渲染链路

**Files:**
- Create: `src/gui/ldSwitch.h`
- Create: `src/gui/ldSwitch.c`
- Modify: `src/gui/ldBase.h`
- Modify: `src/gui/ldGui.h`

- [ ] **Step 1: 先在头文件和 widget type 中写出 failing public contract**

```c
/* src/gui/ldBase.h */
typedef enum{
    widgetTypeBackground,
    widgetTypeWindow,
    widgetTypeButton,
    widgetTypeImage,
    widgetTypeText,
    widgetTypeLineEdit,
    widgetTypeGraph,
    widgetTypeCheckBox,
    widgetTypeSlider,
    widgetTypeSwitch,
    widgetTypeProgressBar,
    widgetTypeGauge,
    widgetTypeQRCode,
    widgetTypeDateTime,
    widgetTypeIconSlider,
    widgetTypeComboBox,
    widgetTypeArc,
    widgetTypeRadialMenu,
    widgetTypeScrollSelecter,
    widgetTypeLabel,
    widgetTypeTable,
    widgetTypeKeyboard,
    widgetTypeAnimation,
    widgetTypeList,
    widgetTypeMessageBox,
    widgetTypeCalendar,
    widgetTypeProgressWheel,
    widgetTypeClock,
} ldWidgetType_t;
```

```c
/* src/gui/ldSwitch.h */
typedef struct ldSwitch_t ldSwitch_t;

struct ldSwitch_t {
    implement(ldBase_t);
    ldColor offTrackColor;
    ldColor onTrackColor;
    ldColor knobColor;
    ldColor borderColor;
    arm_2d_tile_t *ptOffImgTile;
    arm_2d_tile_t *ptOffMaskTile;
    arm_2d_tile_t *ptOnImgTile;
    arm_2d_tile_t *ptOnMaskTile;
    arm_2d_tile_t *ptKnobImgTile;
    arm_2d_tile_t *ptKnobMaskTile;
    uint16_t animProgress;
    uint16_t animStartProgress;
    uint16_t animTargetProgress;
    uint16_t animElapsedMs;
    uint16_t knobPadding;
    bool isChecked : 1;
    bool isHorizontal : 1;
    bool isDisabled : 1;
    bool isPressed : 1;
    bool isAnimating : 1;
    bool useImageStyle : 1;
};
```

- [ ] **Step 2: 把 `ldSwitch` 注册到 GUI 聚合头，先让集成点可编译**

```c
/* src/gui/ldGui.h */
#include "ldSlider.h"
#include "ldSwitch.h"
#include "ldText.h"
```

- [ ] **Step 3: 先写最小 `ldSwitch.c` 骨架，把 init / show / frameStart / setters 都声明出来**

```c
static bool slotSwitchProcess(ld_scene_t *ptScene, ldMsg_t msg);

const ldBaseWidgetFunc_t ldSwitchFunc = {
    .depose = (ldDeposeFunc_t)ldSwitch_depose,
    .load = (ldLoadFunc_t)ldSwitch_on_load,
    .frameStart = (ldFrameStartFunc_t)ldSwitch_on_frame_start,
    .frameComplete = (ldFrameCompleteFunc_t)ldSwitch_on_frame_complete,
    .show = (ldShowFunc_t)ldSwitch_show,
};

ldSwitch_t *ldSwitch_init(ld_scene_t *ptScene,
                          ldSwitch_t *ptWidget,
                          uint16_t nameId,
                          uint16_t parentNameId,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height)
{
    ldBase_t *ptParent;

    if(ptWidget == NULL) {
        ptWidget = ldCalloc(1, sizeof(ldSwitch_t));
        if(ptWidget == NULL) {
            return NULL;
        }
    }
    else {
        memset(ptWidget, 0, sizeof(ldSwitch_t));
    }

    ptParent = ldBaseGetWidget(ptScene->ptNodeRoot, parentNameId);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptParent, (arm_2d_control_node_t *)ptWidget);

    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX = x;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY = y;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = width;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
    ptWidget->use_as__ldBase_t.nameId = nameId;
    ptWidget->use_as__ldBase_t.widgetType = widgetTypeSwitch;
    ptWidget->use_as__ldBase_t.ptGuiFunc = &ldSwitchFunc;
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->use_as__ldBase_t.isDirtyRegionAutoReset = true;
    ptWidget->use_as__ldBase_t.opacity = 255;
    ptWidget->use_as__ldBase_t.tTempRegion =
        ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;
    ptWidget->isHorizontal = true;
    ptWidget->knobPadding = 2;
    ptWidget->animProgress = 0;
    ptWidget->animStartProgress = 0;
    ptWidget->animTargetProgress = 0;
    ptWidget->offTrackColor = __RGB(190, 190, 190);
    ptWidget->onTrackColor = __RGB(72, 184, 120);
    ptWidget->knobColor = GLCD_COLOR_WHITE;
    ptWidget->borderColor = __RGB(120, 120, 120);
    ldMsgConnect(ptWidget, SIGNAL_PRESS, slotSwitchProcess);
    ldMsgConnect(ptWidget, SIGNAL_RELEASE, slotSwitchProcess);
    return ptWidget;
}
```

- [ ] **Step 4: 实现事件与 setter 语义，只先保证行为正确，不追求画面精细**

```c
static void ldSwitchApplyValue(ld_scene_t *ptScene, ldSwitch_t *ptWidget, bool isChecked)
{
    uint16_t target = isChecked ? 1000 : 0;

    if(ptWidget->isChecked == isChecked && ptWidget->animProgress == target) {
        return;
    }

    ptWidget->isChecked = isChecked;
    ptWidget->animStartProgress = ptWidget->animProgress;
    ptWidget->animTargetProgress = target;
    ptWidget->animElapsedMs = 0;
    ptWidget->isAnimating = (ptWidget->animProgress != target);
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ldMsgEmit(ptScene->ptMsgQueue, ptWidget, SIGNAL_VALUE_CHANGED, isChecked ? 1 : 0);
}

static bool slotSwitchProcess(ld_scene_t *ptScene, ldMsg_t msg)
{
    ldSwitch_t *ptWidget = msg.ptSender;

    if(ptWidget->isDisabled) {
        return false;
    }

    if(msg.signal == SIGNAL_PRESS) {
        ptWidget->isPressed = true;
        ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    }
    else if(msg.signal == SIGNAL_RELEASE) {
        ptWidget->isPressed = false;
        ldSwitchApplyValue(ptScene, ptWidget, !ptWidget->isChecked);
    }
    return false;
}
```

- [ ] **Step 5: 实现颜色模式绘制和 `frameStart` 动画推进**

```c
void ldSwitch_on_frame_start(ld_scene_t *ptScene, ldSwitch_t *ptWidget)
{
    (void)ptScene;
    ldSwitchAnimState_t anim = {
        .start = ptWidget->animStartProgress,
        .target = ptWidget->animTargetProgress,
        .current = ptWidget->animProgress,
        .elapsedMs = ptWidget->animElapsedMs,
        .durationMs = 150,
        .running = ptWidget->isAnimating,
    };

    uint16_t progress = ptWidget->animProgress;
    if(ptWidget->isAnimating) {
        ptWidget->isAnimating = ldSwitchAdvanceAnimation(&anim, SYS_TICK_CYCLE_MS, &progress);
        ptWidget->animElapsedMs = anim.elapsedMs;
        ptWidget->animProgress = progress;
        ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    }
}
```

```c
void ldSwitch_show(ld_scene_t *ptScene, ldSwitch_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame)
{
    (void)ptScene;
    ldSwitchAxisMetrics_t metrics;
    uint16_t knobOffset;
    arm_2d_region_t globalRegion;
    arm_2d_region_t trackRegion;
    arm_2d_region_t knobRegion;
    int16_t width = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
    int16_t height = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
    ldColor trackColor = ptWidget->isChecked ? ptWidget->onTrackColor : ptWidget->offTrackColor;

    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t *)ptWidget, &globalRegion, true);
    if(!arm_2d_helper_pfb_is_region_active(ptTile, &globalRegion, true)) {
        return;
    }

    metrics = ldSwitchResolveAxisMetrics(width, height, ptWidget->knobPadding, ptWidget->isHorizontal);
    knobOffset = ldSwitchResolveKnobOffset(&metrics, ptWidget->animProgress);

    trackRegion = globalRegion;

    if(ptWidget->isHorizontal) {
        knobRegion = (arm_2d_region_t){
            .tLocation = {
                .iX = globalRegion.tLocation.iX + metrics.trackStart + (int16_t)knobOffset,
                .iY = globalRegion.tLocation.iY + ptWidget->knobPadding,
            },
            .tSize = {
                .iWidth = metrics.knobSize,
                .iHeight = metrics.knobSize,
            },
        };
    }
    else {
        knobRegion = (arm_2d_region_t){
            .tLocation = {
                .iX = globalRegion.tLocation.iX + ptWidget->knobPadding,
                .iY = globalRegion.tLocation.iY + metrics.trackStart + (metrics.trackLength - (int16_t)knobOffset),
            },
            .tSize = {
                .iWidth = metrics.knobSize,
                .iHeight = metrics.knobSize,
            },
        };
    }

    draw_round_corner_box((arm_2d_tile_t *)ptTile, &trackRegion, trackColor,
                          ptWidget->isDisabled ? (ptWidget->use_as__ldBase_t.opacity / 2) : ptWidget->use_as__ldBase_t.opacity,
                          bIsNewFrame);
    arm_2d_draw_box((arm_2d_tile_t *)ptTile, &trackRegion, 1, ptWidget->borderColor, ptWidget->use_as__ldBase_t.opacity);
    draw_round_corner_box((arm_2d_tile_t *)ptTile, &knobRegion,
                          ptWidget->isPressed ? ptWidget->borderColor : ptWidget->knobColor,
                          ptWidget->isDisabled ? (ptWidget->use_as__ldBase_t.opacity / 2) : ptWidget->use_as__ldBase_t.opacity,
                          bIsNewFrame);
}
```

- [ ] **Step 6: 跑最小编译检查，确认 `ldSwitch` 已接入主源码树**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
cmake -S . -B build/switch-dev -DUSE_DEMO=2
cmake --build build/switch-dev
```

Expected: `ldgui_sdl_demo` 编译通过，不再出现 `widgetTypeSwitch`、`ldSwitchFunc`、`ldSwitch_init` 等缺符号错误。

- [ ] **Step 7: 提交公开控件实现骨架**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add src/gui/ldBase.h src/gui/ldGui.h src/gui/ldSwitch.h src/gui/ldSwitch.c
rtk git commit -m "feat: add native switch widget"
```

### Task 4: 把 switch 接进 SDL widget demo，补禁用态、文档与最终验证

**Files:**
- Modify: `examples/common/demo/widget/uiWidgetPage2.c`
- Modify: `README.md`
- Modify: `README.en.md`
- Modify: `src/gui/ldSwitch.c`
- Modify: `docs/tutorial/04 api.md`
- Modify: `LingDongGUI_vs_LVGL_技术对比.md`

- [ ] **Step 1: 先在 widget demo 第 2 页加入三个 switch 示例位**

```c
/* examples/common/demo/widget/uiWidgetPage2.c */
enum {
    ID_PAGE2_BG = 0,
    ID_PAGE2_HEADER = 1,
    ID_PAGE2_TITLE = 2,
    ID_PAGE2_HINT = 3,
    ID_PAGE2_PROGRESS = 10,
    ID_PAGE2_PROGRESS_LABEL = 11,
    ID_PAGE2_TEXT = 12,
    ID_PAGE2_SLIDER_H = 13,
    ID_PAGE2_SLIDER_V = 14,
    ID_PAGE2_SWITCH_H = 15,
    ID_PAGE2_SWITCH_V = 16,
    ID_PAGE2_SWITCH_DISABLED = 17,
};
```

```c
obj = ldSwitchInit(ID_PAGE2_SWITCH_H, ID_PAGE2_BG, 300, 70, 58, 28);
ldSwitchSetSelectable(obj, true);
ldSwitchSetChecked(obj, true);

obj = ldSwitchInit(ID_PAGE2_SWITCH_V, ID_PAGE2_BG, 388, 62, 28, 58);
ldSwitchSetHorizontal(obj, false);
ldSwitchSetSelectable(obj, true);

obj = ldSwitchInit(ID_PAGE2_SWITCH_DISABLED, ID_PAGE2_BG, 300, 126, 58, 28);
ldSwitchSetChecked(obj, true);
ldSwitchSetDisabled(obj, true);
```

- [ ] **Step 2: 给 `ldSwitch.c` 补齐文档注释，并重新生成 API 文档**

```c
/**
 * @file ldSwitch.h
 * @brief switch widget 原生拨动开关，支持横向/纵向、禁用态、程序设值和动画切换
 * @signal SIGNAL_VALUE_CHANGED value=0关闭,1打开
 */
```

```bash
cd /Users/cys/embedded/LingDongGUI
python3 docs/tutorial/generate_api.py
```

- [ ] **Step 3: 给 README 中英文列表补 switch 条目，并更新技术对比文档口径**

```markdown
| ✅ | switch | 原生拨动开关，支持横向/纵向、禁用态和动画切换 |
| ✅ | switch | Native switch with orientation, disabled state, and animated toggle |

- LingDongGUI 已新增原生 `ldSwitch`
- 当前已覆盖：checked 状态、轨道/滑钮绘制、横竖方向、禁用态、值变化事件、切换动画
- 当前仍未覆盖：LVGL 的通用样式分部系统、统一动画引擎、RTL 语义
```

- [ ] **Step 4: 构建并运行 SDL smoke，验证 switch 在真实 demo 中可见可交互**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
cmake -S . -B build/switch-dev -DUSE_DEMO=2
cmake --build build/switch-dev
SDL_VIDEODRIVER=dummy ./build/switch-dev/ldgui_sdl_demo >/tmp/ldswitch-demo.log 2>&1 & pid=$!; sleep 3; kill "$pid"; wait "$pid" 2>/dev/null || true
sed -n '1,40p' /tmp/ldswitch-demo.log
```

Expected:

- 构建成功
- demo 能启动，不因 `ldSwitch` 崩溃退出
- 日志中不出现明显的空指针、未注册控件或资源缺失致命错误

- [ ] **Step 5: 跑 host tests 和最终构建，确认逻辑与集成都转绿**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
cc -std=gnu11 -I./src/gui \
  examples/sdl/tests/switch/test_ldswitch_internal.c \
  src/gui/ldSwitchInternal.c \
  -o /tmp/test_ldswitch_internal && /tmp/test_ldswitch_internal

cd /Users/cys/embedded/LingDongGUI/examples/sdl
cmake -S . -B build/switch-dev -DUSE_DEMO=2
cmake --build build/switch-dev
```

Expected:

- host test 退出码为 0
- SDL demo 构建通过

- [ ] **Step 6: 检查最终变更集，确认只包含 switch 相关文件**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git status --short
```

Expected: 只出现 `ldSwitch` 新文件、demo 页面、README/API/对比文档和 switch 测试文件。

- [ ] **Step 7: 提交 demo、文档和最终收口**

```bash
cd /Users/cys/embedded/LingDongGUI
rtk git add \
  examples/common/demo/widget/uiWidgetPage2.c \
  README.md \
  README.en.md \
  docs/tutorial/04\ api.md \
  LingDongGUI_vs_LVGL_技术对比.md
rtk git commit -m "feat: wire switch into widget demo and docs"
```
