# SDL 控件 Demo 多画布轮播技术方案

## 1. 背景与目标

当前 `examples/sdl` 中的控件展示 demo 通过 `USE_DEMO == 2` 加载 `uiWidgetFunc`，并把屏幕尺寸直接配置为 `1024 x 600`，所有控件一次性平铺在同一张大画布上展示。

本次目标是将其优化为更接近真实设备的展示方式：

- 屏幕固定为 `480 x 272`
- 每个画布只展示少量控件，避免一页塞满
- 整个 demo 由多个页面组成
- 每 `3s` 自动切换到下一页
- 切换动画使用滑动类效果
  - 例如 `1 -> 2` 左移
  - `2 -> 3` 上移
  - `3 -> 4` 右移
  - `4 -> 1` 下移
- 动画方向可以写死，不要求运行时配置

## 2. 当前代码现状

### 2.1 入口与 demo 选择

- `examples/sdl/user/main.c`
  - 通过 `ldGuiInit((ldPageFuncGroup_t *)&LD_DEMO_GUI_FUNC);` 启动当前 GUI demo
- `examples/sdl/user/ldConfig.h`
  - `USE_DEMO == 2` 时加载 `uiWidget.h`
  - 当前屏幕尺寸配置为：
    - `LD_CFG_SCREEN_WIDTH = 1024`
    - `LD_CFG_SCREEN_HEIGHT = 600`

### 2.2 当前控件页面

- `examples/common/demo/widget/uiWidget.c`
  - 当前只有一个 `uiWidgetFunc`
  - 在 `uiWidgetInit()` 中一次性创建所有控件
  - 坐标范围横向已经铺到 `x=850+`，纵向铺到 `y=500+`
  - 这本质上就是“超大画布 + 单页展示”

### 2.3 现有跳页与动画能力

- `src/gui/ldGui.h`
  - 已提供 `ldGuiJumpPage(page, mode, ms)`
- `src/gui/ldGui.c`
  - 已实现 `__ldGuiJumpPage(...)`
  - `USE_SCENE_SWITCHING == 2` 时支持 scene switching
- `common/Arm-2D/Helper/Source/arm_2d_helper_scene.c`
  - 已内置多种切换动画：
    - `ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT`
    - `ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT`
    - `ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP`
    - `ARM_2D_SCENE_SWITCH_MODE_SLIDE_DOWN`

结论：本需求不需要额外实现动画引擎，仓库已有能力足够支撑。

## 3. 方案选型

### 方案 A：继续保留单个 `uiWidget` 页面，在页面内部手写“子画布切换”

做法：

- 保留一个 page
- 自己维护当前子页面索引
- 控件按组显示/隐藏
- 自己处理位移动画

优点：

- 表面上改动集中在一个文件

缺点：

- 要自己处理控件组切换、显示隐藏、动画位移
- 需要额外管理每组控件生命周期
- 后续继续加页面时，维护成本会快速上升

不推荐。

### 方案 B：拆成多个独立 page，利用现有 `ldGuiJumpPage` 做自动轮播

做法：

- 将现有 `uiWidget.c` 拆分成多个 page
- 每个 page 只创建本页控件
- 在各自 `loop()` 中通过 `ldTimeOut(3000, false)` 自动跳到下一页
- 跳转时直接指定滑动动画模式

优点：

- 完全复用现有框架能力
- 页面边界清晰，维护简单
- 每页控件数量少，更接近真实产品页
- 动画切换逻辑非常直接

缺点：

- 需要新增 3 到 4 个 page 文件
- 需要整理控件分组

推荐采用本方案。

## 4. 推荐实现结构

建议把当前 widget demo 拆成 `4` 个页面。

### Page 1：基础控件页

建议放置：

- `Image`
- `Button`
- `Label`
- `CheckBox`
- `Window`

动画：

- `Page1 -> Page2` 使用 `ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT`

### Page 2：输入与滑动控件页

建议放置：

- `ProgressBar`
- `Text`
- `Slider`
- 纵向 `Slider`

动画：

- `Page2 -> Page3` 使用 `ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP`

### Page 3：复合展示控件页

建议放置：

- `RadialMenu`
- `DateTime`
- `IconSlider`
- `QRCode`

动画：

- `Page3 -> Page4` 使用 `ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT`

### Page 4：数据与表单控件页

建议放置：

- `ScrollSelecter`
- `Gauge`
- `ComboBox`
- `Graph`
- `Table`
- `LineEdit`
- `Arc`
- `List`
- `MessageBox`
- `Calendar`

动画：

- `Page4 -> Page1` 使用 `ARM_2D_SCENE_SWITCH_MODE_SLIDE_DOWN`

说明：

- 若 Page 4 控件仍偏多，可继续拆成 Page 4 / Page 5
- 先按 4 页组织更适合验证轮播机制

## 5. 页面尺寸与布局原则

统一约束：

- 页面尺寸固定为 `480 x 272`
- 每页控件数量控制在 `3` 到 `6` 个主控件之间
- 每页预留统一边距，建议：
  - 左右边距：`12 ~ 16 px`
  - 上下边距：`12 ~ 16 px`
  - 控件间距：`8 ~ 12 px`

布局建议：

- 尽量使用“2 列 + 多行”或“上大下小”的结构
- 不建议继续沿用当前 `1024x600` 坐标直接硬裁剪
- 应按 `480x272` 重新摆放控件坐标

建议增加统一布局宏，例如：

```c
#define DEMO_PAGE_W      480
#define DEMO_PAGE_H      272
#define DEMO_MARGIN      12
#define DEMO_GAP         10
```

这样后续微调布局时不会在多个页面里到处改 magic number。

## 6. 自动轮播机制

推荐在每个页面的 `loop()` 中处理自动翻页。

示意逻辑：

```c
void uiWidgetPage1Loop(ld_scene_t *ptScene)
{
    if (ldTimeOut(3000, false)) {
        ldGuiJumpPage(uiWidgetPage2Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT, 3000);
    }
}
```

这里需要注意两件事：

- `ldTimeOut(3000, false)` 用于只触发一次跳页
- `ldGuiJumpPage(page, mode, ms)` 的第三个参数在当前框架中会传给 scene switching period，和切换动作绑定

工程层面建议统一处理为：

- 停留时长：`3000 ms`
- 动画切换时长：先按框架现状沿用同一参数

如果后续发现“停留时长”和“动画时长”必须拆开，再单独扩展 `ldGui` 封装。

## 7. 文件改造建议

### 7.1 配置改造

修改文件：

- `examples/sdl/user/ldConfig.h`

建议调整：

- 将 `USE_DEMO == 2` 的尺寸由 `1024 x 600` 改为 `480 x 272`
- `LD_CFG_PFB_WIDTH` 可继续保留全屏宽度
- `LD_CFG_PFB_HEIGHT` 维持现有按高度分块的策略即可

### 7.2 demo 文件拆分

建议新增以下文件：

- `examples/common/demo/widget/uiWidgetPage1.c`
- `examples/common/demo/widget/uiWidgetPage1.h`
- `examples/common/demo/widget/uiWidgetPage2.c`
- `examples/common/demo/widget/uiWidgetPage2.h`
- `examples/common/demo/widget/uiWidgetPage3.c`
- `examples/common/demo/widget/uiWidgetPage3.h`
- `examples/common/demo/widget/uiWidgetPage4.c`
- `examples/common/demo/widget/uiWidgetPage4.h`

并保留：

- `examples/common/demo/widget/uiWidget.h`

其中 `uiWidget.h` 可以改为仅暴露首页入口：

```c
extern const ldPageFuncGroup_t uiWidgetPage1Func;
#define uiWidgetFunc uiWidgetPage1Func
```

这样可以尽量减少 `ldConfig.h` 和上层入口的改动面。

### 7.3 公共资源抽取

建议新增一个公共头文件，例如：

- `examples/common/demo/widget/uiWidgetCommon.h`

用于统一放置：

- 通用字符串
- icon 名称数组
- 公共布局宏
- 可复用的辅助函数

避免 4 个页面里重复定义相同资源。

## 8. 动画方向策略

本需求明确允许“写死切换方向”，因此不建议引入配置表或运行时策略。

推荐直接在各页面写死：

- `Page1 -> Page2`: `SLIDE_LEFT`
- `Page2 -> Page3`: `SLIDE_UP`
- `Page3 -> Page4`: `SLIDE_RIGHT`
- `Page4 -> Page1`: `SLIDE_DOWN`

这样有几个好处：

- 实现最简单
- 视觉效果已经足够明显
- 调试时定位最直接

## 9. 交互与行为边界

建议本阶段先明确以下边界，避免第一次改造范围过大：

- 自动轮播优先，先不做手动翻页按键
- 页面切换时不保留上一页控件状态
- 每页只承担“展示控件能力”的职责，不做复杂业务联动
- 保留当前已有的局部动态效果
  - 例如 `Gauge` 角度变化
  - `Arc` 旋转

也就是说，这次改造核心是：

- 从“大画布陈列”切到“多页轮播展示”

而不是把 demo 演进成完整产品应用。

## 10. 风险与注意事项

### 10.1 `ldTimeOut` 的静态计时器作用域

`ldTimeOut(...)` 宏内部带静态计时变量。

这意味着：

- 每个页面各自使用自己的 `loop()` 没问题
- 但不要把多个页面的跳页逻辑都塞到同一个公共函数里复用，否则容易共享计时状态

### 10.2 页面切换时的资源释放

当前 `ldGui` 在切页时会走页面 `quit` 和控件 `depose` 流程。

因此：

- 页面内创建的控件应继续按现有方式初始化
- 不建议跨页面持有控件实例指针
- 跨页只共享常量资源和配置即可

### 10.3 Page 4 可能过满

数据类控件较多，`480x272` 下可能仍显拥挤。

处理顺序建议：

1. 先按 4 页实现
2. 如果 Page 4 明显拥挤，再拆为第 5 页

不要一开始就过度设计分页数量。

## 11. 实施步骤建议

建议按下面顺序落地：

1. 修改 `USE_DEMO == 2` 的屏幕尺寸到 `480 x 272`
2. 新增 `uiWidgetPage1 ~ uiWidgetPage4`
3. 将现有 `uiWidget.c` 的控件按页面拆分
4. 在每个页面 `loop()` 中加入 `3s` 自动跳转
5. 为每个跳转写死一个滑动方向
6. 编译并运行 SDL demo，确认轮播链路闭环
7. 观察页面布局是否拥挤，再微调坐标和分页

## 12. 验证口径

改造完成后，至少验证以下内容：

- SDL 窗口分辨率确认为 `480 x 272`
- 每页只显示该页控件，不再出现整张大画布
- 页面能每 `3s` 自动跳到下一页
- 每次切换都带滑动动画
- 四个方向按预期循环出现
- 页面切换后无明显残影、重影或控件叠加
- `Gauge / Arc` 等动态控件在其所属页面可正常刷新

## 13. 最终建议

推荐直接采用“多独立 page + `ldGuiJumpPage` 自动轮播”的实现方式。

这是当前工程里改动最小、可维护性最好、最符合现有框架设计的做法。核心原因有三点：

- 当前仓库已具备 scene switching 与 slide 动画能力
- `uiWidget` 本身就是单 page，天然适合拆分为多个小 page
- 需求重点是“展示优化”，不是底层动画框架开发

如果进入实现阶段，建议第一版先完成：

- `480 x 272`
- `4` 页
- `3s` 自动轮播
- 固定四方向滑动切换

先把主链路跑通，再做布局美化和分页微调。
