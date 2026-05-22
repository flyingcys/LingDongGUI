# Widget Demo Multi-Page Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 SDL 的 widget demo 从单个 `1024x600` 大画布改造成 `480x272` 多页面自动轮播展示，并按固定方向执行滑动切换动画。

**Architecture:** 保持 `USE_DEMO == 2` 入口不变，只调整屏幕尺寸和 `uiWidget` 对外入口。将当前单文件 `uiWidget.c` 拆成多个独立 page，每个 page 只负责本页控件创建和 3 秒自动跳页，页面跳转统一走 `ldGuiJumpPage(...)` 与 Arm-2D 现有切换模式。

**Tech Stack:** C, LingDongGUI, Arm-2D, SDL, GNU make

---

### Task 1: 调整 demo 配置与入口映射

**Files:**
- Modify: `examples/sdl/user/ldConfig.h`
- Modify: `examples/common/demo/widget/uiWidget.h`

- [ ] **Step 1: 先写出最小可验证改动点**

目标：

- `USE_DEMO == 2` 的屏幕尺寸改为 `480 x 272`
- `uiWidget.h` 由暴露单页入口改为暴露首页入口别名

预期代码骨架：

```c
#if USE_DEMO == 2
#undef LD_CFG_SCREEN_WIDTH
#define LD_CFG_SCREEN_WIDTH (480)
#undef LD_CFG_SCREEN_HEIGHT
#define LD_CFG_SCREEN_HEIGHT (272)
...
#define LD_DEMO_GUI_FUNC uiWidgetFunc
#endif
```

```c
extern const ldPageFuncGroup_t uiWidgetPage1Func;
#define uiWidgetFunc uiWidgetPage1Func
```

- [ ] **Step 2: 编译一次，验证当前改动前的基线状态**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl build/demo
```

Expected:

- 当前代码应能完成编译，或至少给出与本任务无关的现存环境问题

- [ ] **Step 3: 实施最小配置修改**

改动内容：

- `examples/sdl/user/ldConfig.h`
  - 将 `USE_DEMO == 2` 的宽高改为 `480` 和 `272`
- `examples/common/demo/widget/uiWidget.h`
  - 改为暴露 `uiWidgetPage1Func`
  - 增加 `#define uiWidgetFunc uiWidgetPage1Func`

- [ ] **Step 4: 重新编译，确认入口层修改不引入新的头文件错误**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl build/demo
```

Expected:

- 如果后续 page 文件还未创建，可能在链接阶段报缺符号
- 不应出现明显语法错误或宏冲突

### Task 2: 新增 widget demo 公共定义与四个页面文件

**Files:**
- Create: `examples/common/demo/widget/uiWidgetCommon.h`
- Create: `examples/common/demo/widget/uiWidgetPage1.h`
- Create: `examples/common/demo/widget/uiWidgetPage1.c`
- Create: `examples/common/demo/widget/uiWidgetPage2.h`
- Create: `examples/common/demo/widget/uiWidgetPage2.c`
- Create: `examples/common/demo/widget/uiWidgetPage3.h`
- Create: `examples/common/demo/widget/uiWidgetPage3.c`
- Create: `examples/common/demo/widget/uiWidgetPage4.h`
- Create: `examples/common/demo/widget/uiWidgetPage4.c`

- [ ] **Step 1: 先定义公共常量与共享数据**

`uiWidgetCommon.h` 需要包含：

```c
#define DEMO_PAGE_W 480
#define DEMO_PAGE_H 272
#define DEMO_MARGIN 12
#define DEMO_GAP 10

#define ID_KB 22

extern const uint8_t *g_widget_page_str_group[5];
extern const uint8_t *g_widget_icon_names[5];
extern const uint8_t *g_widget_combo_box_group[3];
extern const uint8_t g_widget_title_str[];
extern const uint8_t g_widget_msg_str[];
extern const uint8_t *g_widget_btn_str[3];
extern const char *g_widget_day_names[7];
extern const char g_widget_header_format[];
```

- [ ] **Step 2: 新建 Page 1，承接基础控件**

Page 1 需要：

- 创建全屏 window
- 放置 `Image / Button / Label / CheckBox / Window`
- 保留原按钮点击示例逻辑
- `loop()` 中 3 秒后跳转到 Page 2

跳转代码：

```c
if (ldTimeOut(3000, false)) {
    ldGuiJumpPage(uiWidgetPage2Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT, 3000);
}
```

- [ ] **Step 3: 新建 Page 2，承接输入与滑动控件**

Page 2 需要：

- 放置 `ProgressBar / Text / Horizontal Slider / Vertical Slider`
- 保留上下左右方向键 focus 导航
- `loop()` 中 3 秒后跳转到 Page 3

跳转代码：

```c
if (ldTimeOut(3000, false)) {
    ldGuiJumpPage(uiWidgetPage3Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP, 3000);
}
```

- [ ] **Step 4: 新建 Page 3，承接复合展示控件**

Page 3 需要：

- 放置 `RadialMenu / DateTime / IconSlider / QRCode`
- 保留本页控件的 selectable 设置
- `loop()` 中 3 秒后跳转到 Page 4

跳转代码：

```c
if (ldTimeOut(3000, false)) {
    ldGuiJumpPage(uiWidgetPage4Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT, 3000);
}
```

- [ ] **Step 5: 新建 Page 4，承接数据与表单控件**

Page 4 需要：

- 放置 `ScrollSelecter / Gauge / ComboBox / Graph / Table / LineEdit / Arc / List / MessageBox / Calendar`
- 保留 `Gauge` 与 `Arc` 的动态更新逻辑
- `loop()` 中 3 秒后跳转回 Page 1

跳转代码：

```c
if (ldTimeOut(3000, false)) {
    ldGuiJumpPage(uiWidgetPage1Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_DOWN, 3000);
}
```

- [ ] **Step 6: 每页使用 `480x272` 重新排布控件**

布局要求：

- 不再沿用 `1024x600` 原坐标
- 每页控件不超过当前视口边界
- 优先使用 `DEMO_MARGIN` 和 `DEMO_GAP` 控制间距

- [ ] **Step 7: 编译验证新增文件已被 makefile 和 qmake 通配自动收集**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl build/demo
```

Expected:

- 新增 page 文件参与编译
- 没有缺失声明、重复定义或未解析符号

### Task 3: 收口旧入口文件并清理单页实现

**Files:**
- Modify: `examples/common/demo/widget/uiWidget.c`

- [ ] **Step 1: 将旧 `uiWidget.c` 改为仅承载共享常量**

目标：

- 删除旧的单页 `init/loop/quit`
- 仅保留共享静态数据定义
- 不再导出 `uiWidgetFunc`

建议保留内容：

```c
#include "uiWidgetCommon.h"

const uint8_t *g_widget_page_str_group[5] = {...};
const uint8_t *g_widget_icon_names[5] = {...};
const uint8_t *g_widget_combo_box_group[3] = {...};
...
```

- [ ] **Step 2: 编译验证旧入口已被新 page 入口完全替换**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl build/demo
```

Expected:

- 不再依赖旧 `uiWidgetFunc`
- `USE_DEMO == 2` 能链接到 `uiWidgetPage1Func`

### Task 4: 本地运行验证并修正分页轮播问题

**Files:**
- Modify as needed: `examples/common/demo/widget/uiWidgetPage1.c`
- Modify as needed: `examples/common/demo/widget/uiWidgetPage2.c`
- Modify as needed: `examples/common/demo/widget/uiWidgetPage3.c`
- Modify as needed: `examples/common/demo/widget/uiWidgetPage4.c`

- [ ] **Step 1: 编译产出 SDL demo**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl
```

Expected:

- 生成 `examples/sdl/build/demo`

- [ ] **Step 2: 运行 demo，观察 4 页轮播闭环**

Run:

```bash
rtk /home/share/samba/flyingcys/LingDongGUI/examples/sdl/build/demo
```

Expected:

- 窗口尺寸为 `480x272`
- 初始进入 Page 1
- 每 3 秒自动跳到下一页
- 动画顺序为 左移、上移、右移、下移

- [ ] **Step 3: 若出现页面叠影、切换不触发或控件越界，进行最小修正**

排查优先级：

1. 页面 `loop()` 是否重复使用共享计时逻辑
2. `ldGuiJumpPage(...)` 的 page 目标是否声明完整
3. 本页控件坐标是否越过 `480x272`
4. Page 4 动态控件逻辑是否引用了不存在的控件 ID

- [ ] **Step 4: 再次编译并运行，确认最终行为稳定**

Run:

```bash
rtk make -C /home/share/samba/flyingcys/LingDongGUI/examples/sdl
rtk /home/share/samba/flyingcys/LingDongGUI/examples/sdl/build/demo
```

Expected:

- 编译通过
- 运行后轮播稳定
- 无明显语法告警、链接错误或首屏崩溃

