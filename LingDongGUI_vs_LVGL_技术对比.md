# LingDongGUI vs LVGL 深度技术对比分析

> 分析时间：2026-05-22  
> LingDongGUI 版本：dev 分支  
> LVGL 版本：third_party/lvgl（src/lv_version.h）

---

## 一、框架概述

| 维度 | LingDongGUI | LVGL |
|------|------------|------|
| 定位 | 基于 ARM-2D 的轻量嵌入式 GUI | 通用嵌入式 GUI，广泛硬件覆盖 |
| 图形引擎 | ARM-2D（ARM 官方 2D 加速框架） | 自研软件渲染 + 多硬件加速后端 |
| 编程范式 | OOC（面向对象 C）+ 信号槽 | OOP C + 观察者事件模型 |
| 渲染策略 | 脏矩阵（Dirty Region）+ PFB | 脏矩形（Dirty Rectangle）+ 分层 |
| 设计目标 | 轻量、高帧率、MCU 优先 | 功能完整、可移植、生态丰富 |
| 社区成熟度 | 持续开发中（国内项目） | 全球活跃开源社区，持续迭代 |

---

## 二、控件数量与分类对比

### 2.1 LingDongGUI 控件清单（26 个）

#### 基础展示类（6 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| Window | `ldWindow.h/.c` | 容器/页面根节点 | 横纵向自动布局、多层嵌套 |
| Image | `ldImage.h/.c` | 图片显示 | PNG 格式、透明度、旋转、缩放 |
| Label | `ldLabel.h/.c` | 简单文本 | 轻量级纯文本显示 |
| Text | `ldText.h/.c` | 富文本 | 多行、对齐方式、颜色控制 |
| Arc | `ldArc.h/.c` | 圆弧绘制 | 精确角度控制、背景弧支持 |
| Animation | `ldAnimation.h/.c` | 序列帧动画 | 帧计时、周期配置、区域播放 |

#### 交互输入类（6 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| Button | `ldButton.h/.c` | 按钮 | 按压/释放/可检查状态、图片蒙板 |
| Slider | `ldSlider.h/.c` | 滑动条 | 横/纵向、百分比显示、蒙板支持 |
| CheckBox | `ldCheckBox.h/.c` | 复选框 | 自定义图片、单选/多选模式 |
| ComboBox | `ldComboBox.h/.c` | 下拉框 | 下拉动画、列表选择 |
| LineEdit | `ldLineEdit.h/.c` | 文本输入框 | 光标闪烁、与键盘联动 |
| Keyboard | `ldKeyboard.h/.c` | 虚拟键盘 | 自定义布局、多行输入 |

#### 数据展示类（5 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| ProgressBar | `ldProgressBar.h/.c` | 进度条 | 图片帧动画形成进度效果、双色显示 |
| ProgressWheel | `ldProgressWheel.h/.c` | 圆环进度 | ARM-2D 基础绘制、旋转动画 |
| Graph | `ldGraph.h/.c` | 波形图 | 多系列数据、动态追加、坐标缩放、网格 |
| Table | `ldTable.h/.c` | 表格 | 嵌套控件单元格、滚动、键盘联动、动态行高列宽 |
| List | `ldList.h/.c` | 列表 | 嵌套控件支持、垂直滚动 |

#### 仪表/时间类（4 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| Gauge | `ldGauge.h/.c` | 仪表盘 | 旋转指针、中心偏移、尾迹效果、进度条模式、角度精度 x10 |
| Clock | `ldClock.h/.c` | 模拟时钟 | 时分秒针、自动走时 |
| Calendar | `ldCalendar.h/.c` | 日历 | 日期选择、节假日标记 |
| DateTime | `ldDateTime.h/.c` | 日期时间选择 | 日期/时间独立模式、数字时钟 |

#### 高级交互类（4 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| IconSlider | `ldIconSlider.h/.c` | 图标网格滑块 | 行列网格、分页、图标+文本、水平/垂直滚动、自动滚动 |
| RadialMenu | `ldRadialMenu.h/.c` | 旋转菜单 | 圆形排列、缩放动画、角度精确定位 |
| ScrollSelecter | `ldScrollSelecter.h/.c` | 滚动选择器 | 循环滚动、物理阻力模拟 |
| QRCode | `ldQRCode.h/.c` | 二维码生成 | 纠错等级可配（7%-30%）、版本自适应、缩放因子 |

#### 对话/提示类（1 个）

| 控件 | 文件 | 核心功能 | 特色能力 |
|------|------|---------|---------|
| MessageBox | `ldMessageBox.h/.c` | 消息对话框 | 模态对话、按钮组、消息文本 |

---

### 2.2 LVGL 控件清单（36 个）

#### 基础展示类（7 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| button | `widgets/button/` | 标准按钮，支持禁用/焦点状态 |
| label | `widgets/label/` | 文本标签，支持富文本、行间距 |
| image | `widgets/image/` | 图片显示，多格式解码 |
| arc | `widgets/arc/` | 圆弧，精确起止角度 |
| bar | `widgets/bar/` | 进度条，支持范围设置 |
| line | `widgets/line/` | 直线绘制 |
| led | `widgets/led/` | LED 指示灯（圆形光点效果） |

#### 交互输入类（10 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| slider | `widgets/slider/` | 滑块，支持范围/步长 |
| checkbox | `widgets/checkbox/` | 复选框 |
| switch | `widgets/switch/` | 拨动开关 |
| spinner | `widgets/spinner/` | 旋转等待指示器 |
| spinbox | `widgets/spinbox/` | 数字微调框 |
| roller | `widgets/roller/` | 滚轮选择器 |
| dropdown | `widgets/dropdown/` | 下拉列表 |
| textarea | `widgets/textarea/` | 多行文本输入区域 |
| keyboard | `widgets/keyboard/` | 虚拟键盘 |
| ime | `widgets/ime/` | 输入法（拼音/中文） |

#### 容器/布局类（8 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| buttonmatrix | `widgets/buttonmatrix/` | 按钮矩阵 |
| list | `widgets/list/` | 列表容器 |
| menu | `widgets/menu/` | 层级菜单系统 |
| msgbox | `widgets/msgbox/` | 消息对话框 |
| tabview | `widgets/tabview/` | 标签页切换 |
| tileview | `widgets/tileview/` | 平铺视图（可滑动切换） |
| win | `widgets/win/` | 窗口（带标题栏、工具栏） |
| canvas | `widgets/canvas/` | 自由绘图画布 |

#### 数据展示类（5 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| chart | `widgets/chart/` | 图表（折线/柱状/散点/阶梯） |
| table | `widgets/table/` | 表格 |
| calendar | `widgets/calendar/` | 日历（支持 arrow/dropdown 头部变体） |
| scale | `widgets/scale/` | 刻度尺 |
| span | `widgets/span/` | 行内富文本跨度 |

#### 媒体/动画类（5 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| gif | `widgets/gif/` | GIF 动图播放 |
| animimage | `widgets/animimage/` | 序列帧动画 |
| lottie | `widgets/lottie/` | Lottie（JSON 矢量动画） |
| imagebutton | `widgets/imagebutton/` | 图片按钮（多状态图片） |
| 3dtexture | `widgets/3dtexture/` | 3D 纹理贴图控件 |

#### 特殊功能类（1 个）

| 控件 | 目录 | 核心功能 |
|------|------|---------|
| arclabel | `widgets/arclabel/` | 沿圆弧路径排列文字 |

---

### 2.3 控件数量汇总

| 类别 | LingDongGUI | LVGL |
|------|:-----------:|:----:|
| 基础展示 | 6 | 7 |
| 交互输入 | 6 | 10 |
| 容器/布局 | 2（Window+MessageBox） | 8 |
| 数据展示 | 5 | 5 |
| 仪表/时间 | 4（特有） | 0 |
| 高级交互 | 4（特有） | 0 |
| 媒体/动画 | 1 | 5 |
| 特殊功能 | 1（QRCode） | 1（arclabel） |
| **合计** | **26** | **36** |

### 2.4 LingDongGUI 是否支持 switch

结论：**当前 LingDongGUI 没有原生 `switch` 控件实现**。源码里可直接对应的交互控件只有 `ldButton`、`ldCheckBox`、`ldSlider` 等；README 和教程也只公开了 `button`、`check box + radio button`、`slider`，没有 `switch` 条目。

#### 现状证据

| 观察点 | LingDongGUI | LVGL |
|--------|-------------|------|
| 控件入口 | `src/gui/ldButton.c`、`src/gui/ldCheckBox.c`、`src/gui/ldSlider.c` | `third_party/lvgl/src/widgets/switch/lv_switch.c` |
| 公开能力 | README / API 文档只列出 button、check box、slider | 公开 `lv_switch_create()` |
| 状态模型 | `ldCheckBox_t.isChecked`、`ldButton_t.isPressed` | `LV_STATE_CHECKED` |
| 值变化通知 | `ldCheckBox` 发 `SIGNAL_VALUE_CHANGED` | `LV_EVENT_STATE_CHANGED` |
| 专属视觉结构 | 无 track + knob 专用绘制对象 | `LV_PART_INDICATOR` + `LV_PART_KNOB` |
| 内置切换动画 | 无 | `lv_switch_trigger_anim()` 基于 `lv_anim_t` |

#### LingDongGUI 里最接近 switch 的现有能力

1. **`ldCheckBox` 语义最接近**
   - 内部有 `isChecked` 状态，点击后切换，并通过 `SIGNAL_VALUE_CHANGED` 对外通知。
   - 支持 `ldCheckBoxSetChecked()` / `ldCheckBoxIsChecked()`，已经具备“开/关值”的基本读写接口。
   - 支持 `ldCheckBoxSetImage()`，可以给“选中/未选中”两套图片，适合做一个**静态版 switch**。

2. **`ldButton` 可做自锁按钮，但语义偏弱**
   - `ldButtonSetCheckable()` 可以让按钮保持按下状态。
   - 状态字段是 `isPressed`，更偏“按键保持”而不是“开关值”，默认也没有 `SIGNAL_VALUE_CHANGED` 这一类语义化通知。
   - 更适合做按压态按钮，不适合直接当作 switch API。

3. **`ldSlider` 只有连续值，不是二值开关**
   - 适合拖动百分比，不适合作为 on/off 控件的直接替代。

#### LVGL switch 的实现拆解

LVGL 的 `switch` 不是简单“两个位图切换”，而是建立在通用对象、状态、样式、动画体系上的专用控件：

1. **对象模型**
   - `lv_switch_create()` 创建 `lv_switch_class` 实例。
   - 构造函数里给对象加上 `LV_OBJ_FLAG_CHECKABLE`，所以 switch 直接复用 LVGL 的 checked 状态机。

2. **绘制结构**
   - `LV_PART_INDICATOR` 负责底部轨道（track）。
   - `LV_PART_KNOB` 负责滑块圆钮（knob）。
   - `draw_main()` 根据横向/纵向方向、padding、RTL 设置，实时计算 knob 的位置。

3. **状态切换**
   - 监听 `LV_EVENT_STATE_CHANGED`。
   - 当 `LV_STATE_CHECKED` 发生变化时，触发 `lv_switch_trigger_anim()`。

4. **动画机制**
   - `anim_state` 保存 0~256 的过渡值。
   - `lv_anim_t` 驱动 knob 在 track 上滑动，不只是瞬时换图。
   - 动画时长直接取对象样式里的 `anim_duration`，说明它和样式系统深度耦合。

#### 如果在 LingDongGUI 中加入 switch，最现实的三条路线

| 路线 | 做法 | 优点 | 缺点 | 适合阶段 |
|------|------|------|------|---------|
| A. 皮肤复用 `ldCheckBox` | 用 `ldCheckBoxSetImage()` 提供开/关两张资源图，把视觉做成开关 | 改动最小，最快落地 | 只有离散两态，没有滑块动画，API 名字也不是 switch | 先验证业务需求 |
| B. 新增原生 `ldSwitch` 控件 | 参考 `ldCheckBox` 状态接口 + `ldButton` 交互接线，单独做 `ldSwitch.h/.c` | API 清晰，可补齐 track/knob/动画 | 需要新增绘制、资源、文档、示例 | 推荐正式方案 |
| C. 在 `ldButton` 上继续堆功能 | 把 checkable button 扩成 switch | 代码文件少 | 语义混乱，按钮/开关职责耦合，后续维护差 | 不推荐 |

#### 推荐的 `ldSwitch` 设计草案

如果要做长期可维护实现，建议单独新增 `ldSwitch`，不要继续挤进 `ldButton` 或 `ldCheckBox`：

```c
typedef struct ldSwitch_t {
    implement(ldBase_t);
    ldColor trackOffColor;
    ldColor trackOnColor;
    ldColor knobColor;
    uint16_t knobPadding;
    uint16_t animProgress;   // 0..1000
    bool isChecked : 1;
    bool isAnimating : 1;
} ldSwitch_t;
```

建议接口：

```c
ldSwitch_t *ldSwitch_init(...);
void ldSwitchSetChecked(ldSwitch_t *ptWidget, bool isChecked);
bool ldSwitchIsChecked(ldSwitch_t *ptWidget);
void ldSwitchSetColor(ldSwitch_t *ptWidget, ldColor offColor, ldColor onColor, ldColor knobColor);
void ldSwitchSetImage(ldSwitch_t *ptWidget, arm_2d_tile_t *ptOffImg, arm_2d_tile_t *ptOnImg, arm_2d_tile_t *ptKnobImg);
```

#### LingDongGUI 已有基础设施，哪些能复用

- **事件层**：现成 `SIGNAL_PRESS` / `SIGNAL_RELEASE` / `SIGNAL_VALUE_CHANGED`，不用重做输入系统。
- **状态更新**：现成 `isDirtyRegionUpdate` 脏区刷新机制，适合 switch 小面积局部重绘。
- **绘制能力**：`ldCheckBox_show()` 已经示范了圆角框、遮罩图、文本绘制；可复用为 track / knob 绘制。
- **资源模式**：现有控件支持颜色绘制和图片绘制两条路径，switch 也可以保持同样双模式。

#### LingDongGUI 还缺什么

1. **缺少专用 track + knob 几何计算**
   - 需要像 LVGL `draw_main()` 那样，根据控件宽高和 padding 计算 knob 位置。

2. **缺少通用属性动画**
   - 当前更像“控件内自带动画”，没有 `lv_anim_t` 这种统一插值层。
   - 如果要做顺滑拨动，需要在 `ldSwitch_on_frame_start()` 或定时器里推进 `animProgress`。

3. **缺少样式分部机制**
   - LVGL 可以分别配置 `MAIN / INDICATOR / KNOB`。
   - LingDongGUI 目前更多是控件私有字段，做 switch 时要自己定义颜色、圆角、padding、资源接口。

#### 实现优先级建议

1. **第一阶段：静态二态版**
   - 先基于 `ldCheckBox` 资源双态验证视觉和业务交互。

2. **第二阶段：原生 `ldSwitch`**
   - 抽出单独控件，补 `isChecked`、`SIGNAL_VALUE_CHANGED`、track/knob 绘制。

3. **第三阶段：动画增强**
   - 增加 knob 滑动动画、按下高亮、禁用态、可选纵向模式。

这意味着：**LingDongGUI 不是“完全做不了 switch”，而是“已有 70% 的二值状态和绘制基础，但还没有 LVGL 那种独立、可动画、可样式化的 switch 控件封装”。**

---

## 三、动画与视觉效果对比

### 3.1 LingDongGUI 动画系统

| 能力 | 实现方式 | 文件 |
|------|---------|------|
| 序列帧动画 | `ldAnimation` 控件，帧计时器驱动 | `ldAnimation.h/.c` |
| 指针旋转动画 | `ldGauge` 内置旋转，支持尾迹 | `ldGauge.h/.c` |
| 仪表圆环动画 | `ldProgressWheel` ARM-2D 旋转绘制 | `ldProgressWheel.h/.c` |
| 图片平移动画 | `ldProgressBar` 图片滑动模拟进度 | `ldProgressBar.h/.c` |
| 图标自动滚动 | `ldIconSlider` 内置自动滑动 | `ldIconSlider.h/.c` |
| 物理阻力滚动 | `ldScrollSelecter` 减速模拟 | `ldScrollSelecter.h/.c` |
| 旋转菜单缩放 | `ldRadialMenu` 缩放 + 角度动画 | `ldRadialMenu.h/.c` |

**特点：** 动画深度集成在控件内部，调用简单，但不提供独立的通用动画引擎，无缓动曲线配置。

### 3.2 LVGL 动画系统

| 能力 | 实现方式 | 文件 |
|------|---------|------|
| 通用属性动画 | `lv_anim_t` 任意属性插值 | `misc/lv_anim.h/.c` |
| 时间轴动画 | `lv_anim_timeline_t` 序列编排 | `misc/lv_anim_timeline.h/.c` |
| 缓动曲线 | linear / ease_in / ease_out / overshoot 等 | `misc/lv_anim.h` |
| GIF 播放 | `lv_gif` 控件 | `widgets/gif/` |
| Lottie 动画 | `lv_lottie` 控件，JSON 矢量动画 | `widgets/lottie/` |
| 序列帧 | `lv_animimage` 控件 | `widgets/animimage/` |
| 样式过渡 | 属性动画绑定到样式系统 | `core/lv_obj_style.h` |

**特点：** 独立通用动画引擎，支持任意属性插值和缓动曲线配置，Lottie 支持是重大差异。

### 3.3 动画效果对比总结

| 能力 | LingDongGUI | LVGL |
|------|:-----------:|:----:|
| 序列帧动画 | ✅ | ✅ |
| GIF 播放 | ❌ | ✅ |
| Lottie 矢量动画 | ❌ | ✅ |
| 通用属性动画引擎 | ❌ | ✅ |
| 时间轴动画编排 | ❌ | ✅ |
| 缓动曲线配置 | ❌ | ✅ |
| 仪表指针动效 | ✅（专用，高质量） | ❌ |
| 旋转菜单动效 | ✅（专用） | ❌ |
| 物理阻力滚动 | ✅（专用） | ⚠️（部分） |
| 图片变换动画 | ✅（旋转/缩放/透明） | ✅ |

---

## 四、渲染架构对比

### 4.1 LingDongGUI 渲染架构

```
用户控件层 (ldBase_t 继承体系)
        ↓
ARM-2D 渲染层（硬件加速 2D 操作）
        ↓
脏矩阵（Dirty Region）计算
        ↓
PFB（Pixel Frame Buffer）分块传输
        ↓
DMA / 显示驱动层
```

**关键配置（`arm_2d_disp_adapter_0.h`）：**
- PFB 块宽高可配置
- 支持 3FB（三缓冲）消除撕裂
- DMA 复制加速传输
- 颜色深度：8 / 16 / 32 位
- 屏幕旋转：0° / 90° / 180° / 270°
- 子窗口脏矩阵传递优化

### 4.2 LVGL 渲染架构

```
用户控件层 (lv_obj_t 继承体系)
        ↓
样式系统解析（CSS 风格属性）
        ↓
脏矩形（Dirty Rect）合并计算
        ↓
分层渲染（lv_layer_t）
        ↓
绘制后端选择（软件 / DMA2D / VG-Lite / OpenGL ES …）
        ↓
显示驱动抽象层
```

**绘制后端（`src/draw/`）：**

| 后端 | 目录 | 适用平台 |
|------|------|---------|
| 软件渲染（SW） | `sw/` | 通用（无硬件要求） |
| DMA2D | `dma2d/` | STM32 系列 |
| Renesas | `renesas/` | 瑞萨 RA/RZ 系列 |
| NXP | `nxp/` | NXP i.MX RT 系列 |
| Espressif | `espressif/` | ESP32-S3 等 |
| OpenGL ES | `opengles/` | 带 GPU 的 Linux/Android |
| NanoVG | `nanovg/` | 矢量图形 GPU 加速 |
| VG-Lite | `vg_lite/` | NXP 矢量引擎 |
| Eve | `eve/` | Bridgetek Eve 芯片 |
| SDL | `sdl/` | PC 模拟器开发 |
| Nema GFX | `nema_gfx/` | Think Silicon Nema |

### 4.3 渲染能力对比

| 能力 | LingDongGUI | LVGL |
|------|:-----------:|:----:|
| 脏区域渲染 | ✅（ARM-2D 驱动） | ✅ |
| 三缓冲（3FB） | ✅ | ⚠️（依赖驱动） |
| DMA 加速 | ✅（ARM-2D 内置） | ✅（DMA2D 后端） |
| 多硬件后端 | ❌（仅 ARM-2D） | ✅（10+ 种后端） |
| 软件纯渲染 | ⚠️（依赖 ARM-2D） | ✅ |
| GPU 加速 | ⚠️（ARM-2D 硬件加速） | ✅（OpenGL ES/VG-Lite） |
| 分层渲染 | ⚠️（基础层级） | ✅（完整层系统） |
| 图像混合模式 | ⚠️（ARM-2D 混合） | ✅（多种混合模式） |

---

## 五、布局系统对比

### 5.1 LingDongGUI 布局

- **手动定位**：`(x, y, width, height)` 绝对坐标
- **Window 自动布局**：可配置横向/纵向自动排列子控件
- **锚点对齐**：支持相对锚点定位

### 5.2 LVGL 布局

- **绝对定位**：`lv_obj_set_pos()`
- **Flex 布局**（`layouts/flex/`）：类似 CSS Flexbox，主轴/交叉轴对齐
- **Grid 布局**（`layouts/grid/`）：类似 CSS Grid，行列定义
- **样式驱动**：padding / margin / border 等 CSS 风格属性

| 布局能力 | LingDongGUI | LVGL |
|---------|:-----------:|:----:|
| 绝对坐标 | ✅ | ✅ |
| Flex 弹性布局 | ❌ | ✅ |
| Grid 网格布局 | ❌ | ✅ |
| 容器自动排列 | ⚠️（Window 简单版） | ✅ |
| padding / margin | ❌ | ✅ |

---

## 六、媒体与图像格式对比

### 6.1 LingDongGUI

| 格式 | 支持 | 说明 |
|------|------|------|
| PNG | ✅ | 工具预处理转为 arm_2d_tile_t |
| 旋转/缩放变换 | ✅ | ARM-2D 硬件加速 |
| 图片蒙板 | ✅ | 独立蒙板图层 |
| GIF | ❌ | — |
| JPEG | ❌ | — |
| SVG | ❌ | — |
| 视频 | ❌ | — |

### 6.2 LVGL

| 格式 | 支持 | 库 |
|------|------|----|
| PNG | ✅ | libpng / lodepng |
| JPEG | ✅ | libjpeg_turbo / tjpgd |
| WebP | ✅ | libwebp |
| BMP | ✅ | 内置 |
| GIF | ✅ | 内置 gif 库 |
| SVG | ✅ | thorvg / svg 库 |
| Lottie | ✅ | rlottie |
| GLTF（3D） | ✅ | gltf 库 |
| 视频流 | ✅ | FFmpeg / GStreamer |

---

## 七、字体系统对比

### 7.1 LingDongGUI

- 字体格式：`arm_2d_font_t`（预编译点阵）
- 多字体：通过指针绑定到控件
- 矢量字体：不支持运行时渲染

### 7.2 LVGL

| 能力 | 说明 | 目录 |
|------|------|------|
| 点阵字体 | 预编译二进制格式 | `font/` |
| FreeType | 运行时 TTF/OTF 渲染 | `libs/freetype/` |
| TinyTTF | 轻量 TTF 渲染 | `libs/tiny_ttf/` |
| 图形字体（Icon） | FontAwesome 等图标字体 | `font/` |
| 字体管理器 | 动态加载、多字体切换 | `font/` |

---

## 八、操作系统与平台支持

### 8.1 LingDongGUI

| 目标 | 支持 |
|------|------|
| Cortex-M MCU | ✅（主力平台，基于 ARM-2D） |
| FreeRTOS | ✅ |
| 裸机（无 OS） | ✅ |
| Linux / PC | ⚠️（SDL example 支持） |
| Windows | ❌ |

### 8.2 LVGL OSAL

| 操作系统 | 文件 |
|---------|------|
| FreeRTOS | `osal/lv_freertos.h/.c` |
| CMSIS-RTOS2 | `osal/lv_cmsis_rtos2.h/.c` |
| RT-Thread | `osal/lv_rtthread.h/.c` |
| Linux (POSIX) | `osal/lv_linux.h/.c` |
| Windows | `osal/lv_windows.h/.c` |
| SDL2（跨平台） | `osal/lv_sdl2.h/.c` |
| MQX | `osal/lv_mqx.h/.c` |
| 无 OS（裸机） | `osal/lv_os_none.h/.c` |

**驱动支持（`src/drivers/`）：**

| 平台 | 目录 |
|------|------|
| SDL（PC 模拟） | `sdl/` |
| Linux evdev | `evdev/` |
| Linux libinput | `libinput/` |
| Wayland | `wayland/` |
| X11 | `x11/` |
| Windows | `windows/` |
| QNX | `qnx/` |
| NuttX | `nuttx/` |
| UEFI | `uefi/` |
| OpenGL ES | `opengles/` |

---

## 九、输入事件系统对比

### 9.1 LingDongGUI

```c
// 事件类型
SIGNAL_NO_OPERATION
SIGNAL_PRESS       // 按下 (x, y)
SIGNAL_HOLD_DOWN   // 拖拽 (dx, dy, x, y)
SIGNAL_RELEASE     // 释放 (vx, vy, x, y)
```

- **模型**：信号槽（Signal-Slot），多对多触发
- **实现**：`xBtnAction.h/.c`、`ldMsg.h`
- **队列**：`xQueue.h/.c` 消息队列
- **支持**：物理按键 + 触摸屏

### 9.2 LVGL

- **模型**：观察者模式，事件冒泡/捕获
- **事件类型**：30+ 种（点击、滚动、聚焦、拖拽、值变化…）
- **输入设备**：触摸、鼠标、键盘、编码器
- **indev 抽象层**：`src/indev/`，统一多设备管理
- **手势识别**：内置滑动/甩动检测

---

## 十、辅助工具与生态

### 10.1 LingDongGUI 工具链

| 工具 | 说明 |
|------|------|
| `uiPageCreate.py` | UI 页面代码模板生成 |
| `widgetCreate.py` | 控件代码模板生成 |
| 图片转换工具 | PNG → arm_2d_tile_t |
| TLSF 内存分配器 | `src/misc/tlsf.c` |
| 轻量内存池 | `src/misc/lwmem.c` |

### 10.2 LVGL 工具链与生态

| 工具/库 | 说明 |
|---------|------|
| LVGL UI Editor | 在线设计器 |
| SquareLine Studio | 可视化 UI 设计工具 |
| lv_port_* | 大量官方移植包 |
| Micropython 绑定 | Python 脚本控制 UI |
| QML/JavaScript 实验支持 | — |
| barcode / QRCode | 内置条码库（`libs/barcode/`, `libs/qrcode/`） |
| FFmpeg / GStreamer | 视频播放支持 |
| 文件浏览器 | `others/file_explorer/` |
| Fragment | 类似 Android Fragment 的页面管理 |
| 翻译（i18n） | `others/translation/` |
| 调试工具 | `debugging/`（事件监听、内存监控） |

---

## 十一、资源占用估算

### 11.1 LingDongGUI

- **代码体积**：轻量，仅依赖 ARM-2D
- **RAM 消耗**：低，TLSF + lwmem 精细管理
- **最小运行内存**：约 32KB RAM + Flash
- **适合**：Cortex-M3/M4/M7 MCU（内存 64KB+）

### 11.2 LVGL

| 配置 | 最小 RAM | 推荐 RAM |
|------|---------|---------|
| 最小配置（lv_conf 精简） | ~8KB | — |
| 标准配置 | ~32KB | 64KB+ |
| 全功能（含字体/图像） | 100KB+ | 256KB+ |

---

## 十二、专项特性对比总览

| 特性 | LingDongGUI | LVGL |
|------|:-----------:|:----:|
| **控件总数** | 26 | 36 |
| 仪表盘控件 | ✅（高精度，尾迹效果） | ❌ |
| 旋转菜单 | ✅ | ❌ |
| 图标网格分页 | ✅ | ❌ |
| 物理滚动模拟 | ✅ | ⚠️ |
| 模拟时钟 | ✅ | ❌ |
| 二维码生成 | ✅ | ✅（libs/qrcode） |
| 波形图 | ✅ | ✅（chart） |
| 表格 | ✅ | ✅ |
| 日历 | ✅ | ✅ |
| Lottie 动画 | ❌ | ✅ |
| GIF 播放 | ❌ | ✅ |
| SVG 渲染 | ❌ | ✅ |
| 视频播放 | ❌ | ✅ |
| 3D 纹理 | ❌ | ✅ |
| Flex/Grid 布局 | ❌ | ✅ |
| 通用动画引擎 | ❌ | ✅ |
| 多绘制后端 | ❌ | ✅（10+ 种） |
| 多 OS 支持 | ⚠️（3 种） | ✅（8 种） |
| 主题/样式系统 | ❌ | ✅（CSS 风格） |
| 输入法（IME） | ❌ | ✅ |
| 可视化设计工具 | ❌ | ✅（SquareLine Studio） |
| 调试工具 | ⚠️（xLog） | ✅（完整调试模块） |
| i18n 翻译 | ❌ | ✅ |
| ARM-2D 硬件加速 | ✅（原生） | ❌ |

---

## 十三、综合评估与选型建议

### LingDongGUI 的优势

1. **ARM 生态深度优化**：原生基于 ARM-2D，天然利用 Cortex-M 2D 加速，无需额外适配
2. **仪表类控件领先**：Gauge（带尾迹）、ProgressWheel、Clock、Arc 等仪表控件质量高，专为工业/IoT 设备优化
3. **独特高级交互**：RadialMenu（旋转菜单）、IconSlider（图标网格分页）、ScrollSelecter（物理阻力滚动）是 LVGL 没有的
4. **脏矩阵优化内置**：PFB + 脏矩阵开箱即用，帧率表现稳定
5. **代码简洁**：信号槽模型调用简单，适合快速开发嵌入式 UI
6. **QRCode 内置**：26 个基础控件中直接包含 QR 码生成能力

### LVGL 的优势

1. **控件数量更多**：36 个控件，覆盖更多 UI 场景
2. **媒体能力全面**：Lottie / GIF / SVG / 视频，现代 UI 所需媒体格式全支持
3. **布局系统现代**：Flex + Grid 响应式布局，适配不同屏幕尺寸
4. **动画引擎完整**：通用属性动画 + 时间轴 + 缓动曲线，动画表达力强
5. **平台移植性强**：10+ 绘制后端，8 种 OS，几乎所有嵌入式平台均可运行
6. **生态最成熟**：SquareLine Studio、大量移植包、Python 绑定、活跃社区
7. **样式/主题系统**：CSS 风格的样式系统，支持主题切换

### 选型矩阵

| 场景 | 推荐 | 原因 |
|------|------|------|
| Cortex-M 仪表/工控面板 | **LingDongGUI** | ARM-2D 加速 + 仪表控件完善 |
| 需要旋转菜单/图标网格 | **LingDongGUI** | 独有 RadialMenu / IconSlider |
| 资源受限 MCU（<128KB RAM） | **LingDongGUI** | 更轻量，ARM-2D 优化 |
| 需要 Lottie / GIF 动画 | **LVGL** | 原生支持 |
| 需要视频播放 | **LVGL** | FFmpeg/GStreamer 支持 |
| 跨平台（MCU + Linux + PC） | **LVGL** | 多后端、多 OS |
| 现代响应式 UI | **LVGL** | Flex/Grid 布局 |
| 多语言/i18n 应用 | **LVGL** | 内置翻译支持 |
| 可视化拖拽设计 | **LVGL** | SquareLine Studio |
| 快速嵌入式原型验证 | **LingDongGUI** | 模板生成工具 + 信号槽简单 |

---

*文档基于代码源码静态分析生成，如有控件功能描述偏差，请以实际运行效果为准。*
