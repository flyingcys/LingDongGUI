# Flex Layout Phase 0/1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在不开发 Grid 的前提下，为 LingDongGUI 落地 Phase 0/1：修正 `ldWindow` legacy 自动布局语义、建立 `layout dirty` 冒泡机制，并实现最小一维 Flex（`row/column + gap + padding + align`）。

**Architecture:** 保持布局实现留在 `src/gui/`，不引入 `src/layouts/`。通过新增内部布局辅助文件，把“直属子项收集 / legacy 排布 / Flex 排布 / layout dirty 冒泡”从 `ldWindow.c` 中拆出来，保证行为可测，再把 SDL 侧测试目标和示例 demo 接进现有 make 入口。

**Tech Stack:** C, LingDongGUI, Arm-2D, SDL, GNU make, assert-based host tests

---

### Task 1: 建立布局测试入口与内部布局辅助边界

**Files:**
- Create: `src/gui/ldWindowLayoutInternal.h`
- Create: `src/gui/ldWindowLayoutInternal.c`
- Create: `examples/sdl/tests/layout/test_layout_window.c`
- Modify: `examples/sdl/makefile`

- [ ] **Step 1: 先写出内部辅助边界和测试骨架**

本任务先不实现完整逻辑，只先定义后续要测试的最小边界：

```c
/* src/gui/ldWindowLayoutInternal.h */
uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow,
                                       ldBase_t **ppChildren,
                                       uint16_t maxCount,
                                       bool skipHidden);

void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget);

int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align,
                               int16_t innerMainSize,
                               int16_t contentMainSize,
                               uint16_t visibleCount,
                               int16_t gap,
                               int16_t *pResolvedGap);
```

```c
/* examples/sdl/tests/layout/test_layout_window.c */
static void test_collect_direct_children_ignores_grandchildren(void);
static void test_mark_parent_layout_dirty_marks_nearest_window(void);
static void test_flex_space_between_falls_back_to_start_when_no_space(void);

int main(void) {
    test_collect_direct_children_ignores_grandchildren();
    test_mark_parent_layout_dirty_marks_nearest_window();
    test_flex_space_between_falls_back_to_start_when_no_space();
    return 0;
}
```

- [ ] **Step 2: 先把 failing test 写出来**

第一批测试内容直接写成 `assert()`：

```c
static void test_collect_direct_children_ignores_grandchildren(void) {
    ldWindow_t parent = {0};
    ldWindow_t child_window = {0};
    ldLabel_t child_label = {0};
    ldLabel_t grandchild = {0};
    ldBase_t *items[4] = {0};

    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&child_window);
    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&child_label);
    ldBaseNodeAdd((arm_2d_control_node_t *)&child_window, (arm_2d_control_node_t *)&grandchild);

    assert(ldWindowCollectDirectChildren((ldBase_t *)&parent, items, 4, false) == 2);
    assert(items[0] == (ldBase_t *)&child_window);
    assert(items[1] == (ldBase_t *)&child_label);
}
```

```c
static void test_mark_parent_layout_dirty_marks_nearest_window(void) {
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t leaf = {0};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&leaf);

    root.isLayoutUpdate = false;
    container.isLayoutUpdate = false;

    ldBaseMarkParentLayoutDirty((ldBase_t *)&leaf);

    assert(container.isLayoutUpdate == true);
    assert(root.isLayoutUpdate == true);
}
```

- [ ] **Step 3: 在 makefile 中增加测试目标**

新增一个 host test 目标，避免每次都启动完整 SDL demo：

```make
TEST_LAYOUT_SOURCES = \
    examples/sdl/tests/layout/test_layout_window.c \
    ../../src/gui/ldWindowLayoutInternal.c \
    ../../src/gui/ldBase.c \
    ../../src/gui/ldWindow.c \
    ../../src/misc/*.c

test_layout: create_dir
	$(CC) $(CFLAGS) $(TEST_LAYOUT_SOURCES) -o $(BUILD_DIR)/test_layout $(LIBS)
	./$(BUILD_DIR)/test_layout
```

- [ ] **Step 4: 运行测试，确认当前基线失败**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
```

Expected:

- 编译失败，提示辅助函数未定义
- 或链接失败，提示 `ldWindowCollectDirectChildren` / `ldBaseMarkParentLayoutDirty` 缺失

- [ ] **Step 5: 用最小实现让测试能编译并失败在断言处**

先写最小桩实现：

```c
uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow,
                                       ldBase_t **ppChildren,
                                       uint16_t maxCount,
                                       bool skipHidden)
{
    (void)ptWindow;
    (void)ppChildren;
    (void)maxCount;
    (void)skipHidden;
    return 0;
}

void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget)
{
    (void)ptWidget;
}
```

- [ ] **Step 6: 再跑测试，确认失败已经进入行为断言**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
```

Expected:

- 编译通过
- 断言失败，说明测试已经真正覆盖语义而不是缺符号

- [ ] **Step 7: 提交测试入口与辅助边界**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/tests/layout/test_layout_window.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/makefile
rtk git commit -m "test: add layout host test entrypoint"
```

### Task 2: 修正 legacy 布局为直属子项语义

**Files:**
- Modify: `src/gui/ldWindow.c`
- Modify: `src/gui/ldWindowLayoutInternal.c`
- Modify: `examples/sdl/tests/layout/test_layout_window.c`

- [ ] **Step 1: 补充 legacy 直属子项测试**

新增测试覆盖 hidden 兼容行为和直属子项顺序：

```c
static void test_collect_direct_children_keeps_hidden_for_legacy(void) {
    ldWindow_t parent = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldBase_t *items[4] = {0};

    a.use_as__ldBase_t.isHidden = false;
    b.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&b);

    assert(ldWindowCollectDirectChildren((ldBase_t *)&parent, items, 4, false) == 2);
}
```

- [ ] **Step 2: 实现直属子项收集逻辑**

用 `ptChildList -> ptNext` 只遍历一层，不再走 `PREORDER_TRAVERSAL`：

```c
uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow,
                                       ldBase_t **ppChildren,
                                       uint16_t maxCount,
                                       bool skipHidden)
{
    uint16_t count = 0;
    ldBase_t *ptChild = ldBaseGetChildList(ptWindow);

    while ((ptChild != NULL) && (count < maxCount)) {
        if ((!skipHidden) || (!ptChild->isHidden)) {
            ppChildren[count++] = ptChild;
        }
        ptChild = (ldBase_t *)ptChild->use_as__arm_2d_control_node_t.ptNext;
    }

    return count;
}
```

- [ ] **Step 3: 将 `ldWindow_on_frame_start()` 切换到直属子项数组**

把 legacy 布局里的子项枚举改成：

```c
ldBase_t *children[LD_LAYOUT_MAX_CHILDREN] = {0};
uint16_t childCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget,
                                                    children,
                                                    LD_LAYOUT_MAX_CHILDREN,
                                                    false);

for (uint16_t i = 0; i < childCount; i++) {
    ldBase_t *ptChild = children[i];
    ...
}
```

要求：

- `layoutHorizontal` / `layoutVertical` 只消费 `children[]`
- legacy 仍然保留 hidden 子项占位语义

- [ ] **Step 4: 运行测试，确认直属子项语义通过**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
```

Expected:

- `test_collect_direct_children_ignores_grandchildren` PASS
- `test_collect_direct_children_keeps_hidden_for_legacy` PASS

- [ ] **Step 5: 编译 SDL demo，确认链接与运行入口未被破坏**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl
```

Expected:

- `build/demo` 编译成功
- 不应出现 `ldWindow_on_frame_start` 相关编译错误

- [ ] **Step 6: 提交 legacy 直属子项修正**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindow.c \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/tests/layout/test_layout_window.c
rtk git commit -m "fix: limit window layout to direct children"
```

### Task 3: 落地 layout dirty 冒泡机制

**Files:**
- Modify: `src/gui/ldBase.h`
- Modify: `src/gui/ldBase.c`
- Modify: `src/gui/ldWindow.c`
- Modify: `src/gui/ldWindowLayoutInternal.h`
- Modify: `src/gui/ldWindowLayoutInternal.c`
- Modify: `examples/sdl/tests/layout/test_layout_window.c`

- [ ] **Step 1: 先补充 layout dirty 触发测试**

新增测试覆盖几何 setter 和隐藏切换：

```c
static void test_set_region_marks_parent_layout_dirty(void) {
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;

    ldBaseSetWidth((ldBase_t *)&child, 42);

    assert(root.isLayoutUpdate == true);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);
}
```

```c
static void test_set_hidden_marks_parent_layout_dirty(void) {
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;

    ldBaseSetHidden((ldBase_t *)&child, true);

    assert(root.isLayoutUpdate == true);
}
```

- [ ] **Step 2: 实现向上冒泡到布局容器的逻辑**

内部实现建议：

```c
void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget)
{
    arm_2d_control_node_t *ptNode = ptWidget->use_as__arm_2d_control_node_t.ptParent;

    while (ptNode != NULL) {
        ldBase_t *ptBase = (ldBase_t *)ptNode;
        if (ptBase->widgetType == widgetTypeWindow) {
            ldWindow_t *ptWindow = (ldWindow_t *)ptBase;
            if (ptWindow->layoutTpye != layoutNone) {
                ptWindow->isLayoutUpdate = true;
            }
        }
        ptNode = ptNode->ptParent;
    }
}
```

- [ ] **Step 3: 在几何 setter 和节点管理点接入冒泡**

对以下位置补一行调用：

```c
ldBaseMarkParentLayoutDirty(ptWidget);
```

接入点：

- `ldBaseSetRegion`
- `ldBaseResize`
- `ldBaseSetX`
- `ldBaseSetY`
- `ldBaseSetWidth`
- `ldBaseSetHeight`
- `ldBaseSetHidden`

对节点增删采用父节点触发方式：

```c
if (parent != NULL) {
    ldBaseMarkParentLayoutDirty((ldBase_t *)parent);
}
```

- [ ] **Step 4: 跑测试验证 dirty 冒泡**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
```

Expected:

- 新增 layout dirty 测试全部 PASS

- [ ] **Step 5: 手工运行 demo，确认无明显重影**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl
```

Expected:

- demo 启动成功
- 页面切换和控件更新时无明显残影或旧区域残留

- [ ] **Step 6: 提交 layout dirty 基础设施**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldBase.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldBase.c \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindow.c \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/tests/layout/test_layout_window.c
rtk git commit -m "feat: add layout dirty bubbling for windows"
```

### Task 4: 新增最小一维 Flex 配置与排布实现

**Files:**
- Modify: `src/gui/ldBase.h`
- Modify: `src/gui/ldWindow.h`
- Modify: `src/gui/ldWindow.c`
- Modify: `src/gui/ldWindowLayoutInternal.h`
- Modify: `src/gui/ldWindowLayoutInternal.c`
- Modify: `examples/sdl/tests/layout/test_layout_window.c`

- [ ] **Step 1: 先写最小 Flex 计算测试**

新增测试：

```c
static void test_flex_row_center_applies_gap_and_padding(void) {
    int16_t gap = 10;
    int16_t resolvedGap = gap;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignCenter, 180, 110, 3, gap, &resolvedGap);

    assert(start == 35);
    assert(resolvedGap == 10);
}
```

```c
static void test_flex_space_between_resolves_gap_from_remaining_space(void) {
    int16_t resolvedGap = 0;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 180, 90, 3, 0, &resolvedGap);

    assert(start == 0);
    assert(resolvedGap == 45);
}
```

- [ ] **Step 2: 在 `ldWindow.h` 中加入最小 Flex 类型和 API**

新增类型：

```c
typedef enum {
    layoutNone,
    layoutHorizontal,
    layoutVertical,
    layoutFlex,
} ldLayoutType_t;

typedef enum {
    ldFlexFlowRow = 0,
    ldFlexFlowColumn,
} ldFlexFlow_t;

typedef enum {
    ldFlexMainAlignStart = 0,
    ldFlexMainAlignCenter,
    ldFlexMainAlignEnd,
    ldFlexMainAlignSpaceBetween,
} ldFlexMainAlign_t;

typedef enum {
    ldFlexCrossAlignStart = 0,
    ldFlexCrossAlignCenter,
    ldFlexCrossAlignEnd,
} ldFlexCrossAlign_t;
```

新增 API：

```c
void ldWindowSetFlexFlow(ldWindow_t *ptWidget, ldFlexFlow_t flow);
void ldWindowSetFlexAlign(ldWindow_t *ptWidget,
                          ldFlexMainAlign_t mainAlign,
                          ldFlexCrossAlign_t crossAlign);
void ldWindowSetPadding(ldWindow_t *ptWidget, ldPadding_t padding);
void ldWindowSetGap(ldWindow_t *ptWidget, int16_t gap);
```

- [ ] **Step 3: 为 `ldWindow_t` 增加最小 Flex 配置字段**

最小字段建议：

```c
ldPadding_t flexPadding;
int16_t flexGap;
ldFlexFlow_t flexFlow : 1;
ldFlexMainAlign_t flexMainAlign : 2;
ldFlexCrossAlign_t flexCrossAlign : 2;
```

要求：

- 不新增 grow / wrap / grid 字段
- `pLayoutPaddingGroup` 继续只服务 legacy

- [ ] **Step 4: 实现主轴起点与 gap 解析函数**

实现 `ldFlexResolveMainStart()`：

```c
int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align,
                               int16_t innerMainSize,
                               int16_t contentMainSize,
                               uint16_t visibleCount,
                               int16_t gap,
                               int16_t *pResolvedGap)
{
    int16_t remain = innerMainSize - contentMainSize;
    if (remain <= 0) {
        *pResolvedGap = gap;
        return 0;
    }

    switch (align) {
    case ldFlexMainAlignCenter:
        *pResolvedGap = gap;
        return remain / 2;
    case ldFlexMainAlignEnd:
        *pResolvedGap = gap;
        return remain;
    case ldFlexMainAlignSpaceBetween:
        if (visibleCount >= 2) {
            *pResolvedGap = remain / (visibleCount - 1);
            return 0;
        }
        *pResolvedGap = gap;
        return 0;
    default:
        *pResolvedGap = gap;
        return 0;
    }
}
```

- [ ] **Step 5: 在 `ldWindow_on_frame_start()` 中分发 `layoutFlex`**

切分布局入口：

```c
switch (ptWidget->layoutTpye) {
case layoutHorizontal:
case layoutVertical:
    ldWindowLayoutLegacy(ptWidget);
    break;
case layoutFlex:
    ldWindowLayoutFlex(ptWidget);
    break;
default:
    break;
}
```

Flex 规则：

- 只收集直属可见子项
- 只改位置，不改尺寸
- 支持 `row / column`
- 支持 `padding / gap / mainAlign / crossAlign`

- [ ] **Step 6: 运行测试，确认最小 Flex 计算通过**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
```

Expected:

- Flex 起点与 `space-between` 测试全部 PASS

- [ ] **Step 7: 提交最小 Flex 实现**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldBase.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindow.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindow.c \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.h \
  /home/share/samba/flyingcys/LingDongGUI/src/gui/ldWindowLayoutInternal.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/tests/layout/test_layout_window.c
rtk git commit -m "feat: add minimal one-dimensional flex layout"
```

### Task 5: 增加专用 layout demo 并完成手工验证

**Files:**
- Create: `examples/common/demo/layout/uiLayout.h`
- Create: `examples/common/demo/layout/uiLayout.c`
- Modify: `examples/sdl/user/ldConfig.h`
- Modify: `examples/sdl/makefile`

- [ ] **Step 1: 先写 demo 页面目标和骨架**

页面要覆盖：

- legacy horizontal
- legacy vertical
- flex row
- flex column
- gap / padding / align
- hidden item 不同语义

页面骨架：

```c
enum {
    ID_LAYOUT_BG = 0,
    ID_LAYOUT_LEGACY = 10,
    ID_LAYOUT_FLEX_ROW = 20,
    ID_LAYOUT_FLEX_COLUMN = 30,
};

const ldPageFuncGroup_t uiLayoutFunc = {
    .init = uiLayoutInit,
    .loop = uiLayoutLoop,
    .quit = uiLayoutQuit,
};
```

- [ ] **Step 2: 先让 demo 作为独立 `USE_DEMO` 入口接入**

在 `examples/sdl/user/ldConfig.h` 增加一个新 demo 入口：

```c
#if USE_DEMO == 5
#define LD_DEMO_GUI_INCLUDE "uiLayout.h"
#define LD_DEMO_GUI_FUNC uiLayoutFunc
#endif
```

并在 `examples/sdl/makefile` 中加入新 demo 源目录：

```make
$(wildcard ../common/demo/layout/*.c)
```

- [ ] **Step 3: 实现最小 layout demo**

demo 至少要包含：

```c
obj = ldWindowInit(ID_LAYOUT_FLEX_ROW, ID_LAYOUT_BG, 12, 40, 220, 80);
ldWindowSetColor(obj, __RGB(240, 240, 240));
ldWindowSetFlexFlow(obj, ldFlexFlowRow);
ldWindowSetPadding(obj, (ldPadding_t){ .left = 8, .top = 8, .right = 8, .bottom = 8 });
ldWindowSetGap(obj, 10);
ldWindowSetFlexAlign(obj, ldFlexMainAlignCenter, ldFlexCrossAlignCenter);
```

- [ ] **Step 4: 编译并运行 layout demo**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl USE_DEMO=5
```

Expected:

- `build/demo` 编译成功
- 启动后能看到 legacy 与 flex 布局示例

- [ ] **Step 5: 完成手工验收记录**

手工核对项：

- legacy 只影响直属子项
- flex row / column 位置正确
- hidden item 在 flex 中不占位
- 改容器尺寸后下一帧自动重排
- 无明显残影

- [ ] **Step 6: 提交 layout demo**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/examples/common/demo/layout/uiLayout.h \
  /home/share/samba/flyingcys/LingDongGUI/examples/common/demo/layout/uiLayout.c \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/user/ldConfig.h \
  /home/share/samba/flyingcys/LingDongGUI/examples/sdl/makefile
rtk git commit -m "feat: add layout demo for flex phase 0 and 1"
```

### Task 6: 同步 API 与开发文档

**Files:**
- Modify: `docs/tutorial/04 api.md`
- Modify: `docs/tutorial/05 development.md`
- Modify: `docs/superpowers/specs/2026-05-22-flex-layout-phase0-1-development-spec.md` (如实施中有必要的小范围回写)

- [ ] **Step 1: 更新 Window API 文档**

新增或修正以下接口说明：

```c
void ldWindowSetLayout(ldWindow_t *ptWidget, ldLayoutType_t type);
void ldWindowSetPaddingGroup(ldWindow_t *ptWidget, ldPadding_t *pPaddingGroup);
void ldWindowSetFlexFlow(ldWindow_t *ptWidget, ldFlexFlow_t flow);
void ldWindowSetFlexAlign(ldWindow_t *ptWidget,
                          ldFlexMainAlign_t mainAlign,
                          ldFlexCrossAlign_t crossAlign);
void ldWindowSetPadding(ldWindow_t *ptWidget, ldPadding_t padding);
void ldWindowSetGap(ldWindow_t *ptWidget, int16_t gap);
```

- [ ] **Step 2: 在开发文档中加入最小 Flex 使用示例**

示例代码：

```c
void uiDemoInit(ld_scene_t* ptScene)
{
    void *obj;

    obj = ldWindowInit(ID_BG, ID_BG, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldWindowSetColor(obj, __RGB(255, 255, 255));

    obj = ldWindowInit(ID_ROW, ID_BG, 10, 10, 220, 60);
    ldWindowSetLayout(obj, layoutFlex);
    ldWindowSetFlexFlow(obj, ldFlexFlowRow);
    ldWindowSetGap(obj, 8);
}
```

- [ ] **Step 3: 再跑一次 host tests 和 SDL 编译，确保文档描述与真实 API 一致**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl test_layout
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl
```

Expected:

- host tests PASS
- SDL demo 编译成功

- [ ] **Step 4: 提交文档同步**

```bash
rtk git add \
  /home/share/samba/flyingcys/LingDongGUI/docs/tutorial/04\ api.md \
  /home/share/samba/flyingcys/LingDongGUI/docs/tutorial/05\ development.md \
  /home/share/samba/flyingcys/LingDongGUI/docs/superpowers/specs/2026-05-22-flex-layout-phase0-1-development-spec.md
rtk git commit -m "docs: document flex phase 0 and 1 api"
```

---

## Self-Review

### Spec coverage

- legacy 直属子项语义：Task 2
- layout dirty 机制：Task 3
- 最小一维 Flex：Task 4
- SDL 验证和 layout demo：Task 5
- 文档同步：Task 6
- Grid 仅协商不开发：全计划未引入 Grid 代码任务，符合 spec

### Placeholder scan

- 未使用 `TODO` / `TBD`
- 每个任务都给出精确文件、命令和最小代码骨架
- 未引用未在计划中定义的 Grid API

### Type consistency

- 布局类型统一使用 `layoutFlex`
- flow 统一使用 `ldFlexFlowRow / ldFlexFlowColumn`
- 对齐统一使用 `ldFlexMainAlign*` 与 `ldFlexCrossAlign*`
- internal helper 统一使用 `ldWindowLayoutInternal.*`

---

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-05-22-flex-layout-phase0-1-implementation.md`. Two execution options:

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
