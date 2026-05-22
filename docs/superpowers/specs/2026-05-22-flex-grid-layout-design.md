# LingDongGUI Flex/Grid 布局系统设计文档

**日期**：2026-05-22  
**状态**：草稿  
**作者**：架构分析 & Claude Code

---

## 1. 背景与目标

### 1.1 背景

LingDongGUI 是专为 ARM Cortex-M 系列 MCU 设计的轻量 GUI 框架，基于 ARM-2D 硬件加速。当前版本（截至 2026-05-22）包含 26 个成熟控件，采用 OOC + 信号槽架构，具备脏矩阵优化和 PFB 分块渲染能力。

当前布局能力仅限于 `ldWindow` 的简单水平/垂直**等比均分**，缺乏现代 GUI 框架中常见的 Flex/Grid 响应式布局能力，这限制了复杂 UI 的开发效率。

### 1.2 目标

评估在 LingDongGUI 中引入类似 LVGL Flex/Grid 的布局能力的：

- **可行性**：技术路径是否通畅
- **复杂度**：代码量、算法难度、嵌入式适配难点
- **风险**：对现有架构的影响
- **推荐方案**：分阶段实施路径

---

## 2. 现状分析

### 2.1 LingDongGUI 当前布局能力

**核心数据结构** (`src/gui/ldBase.h:193-217`):

```c
typedef struct {
    implement(arm_2d_control_node_t);   // 树节点（含 tRegion: x, y, w, h）
    const ldBaseWidgetFunc_t *ptGuiFunc;
    ...
    bool isDirtyRegionUpdate:1;
    bool isHidden:1;
} ldBase_t;
```

**树节点的区域结构** (`arm_2d_region_t`):
```c
typedef struct {
    arm_2d_location_t tLocation;  // {iX, iY}：相对父节点的坐标
    arm_2d_size_t tSize;          // {iWidth, iHeight}：尺寸
} arm_2d_region_t;
```

**当前 Window 布局** (`src/gui/ldWindow.h:49-67`):

```c
typedef enum {
    layoutNone,          // 无自动布局（纯手动定位）
    layoutHorizontal,    // 水平均分
    layoutVertical,      // 垂直均分
} ldLayoutType_t;
```

**实现位置**：`src/gui/ldWindow.c:161-235`，`ldWindow_on_frame_start()` 函数

**布局能力对比表**：

| 特性 | 当前支持 | 说明 |
|------|:-------:|------|
| 绝对定位 (x, y, w, h) | ✅ | 所有控件默认支持 |
| 水平/垂直均分 | ✅ | ldWindow 支持 |
| Padding（per-slot） | ⚠️ | 通过 `pLayoutPaddingGroup` 支持，但限制多 |
| 百分比尺寸 | ❌ | 不支持 |
| Flex 主轴/交叉轴对齐 | ❌ | 不支持 |
| Flex grow/shrink | ❌ | 不支持 |
| Flex wrap 换行 | ❌ | 不支持 |
| Grid 行列定义 | ❌ | 不支持 |
| Grid fr 单位 | ❌ | 不支持 |
| Grid colspan/rowspan | ❌ | 不支持 |
| 内容自适应尺寸 | ❌ | 不支持 |

### 2.2 现有架构优势（支持 Flex/Grid 引入）

| 优势 | 具体说明 |
|------|---------|
| **树形结构已就位** | `arm_2d_control_node_t` 完整支持父子关系和兄弟链表 |
| **位置计算独立** | `ldBase_t.tRegion` 可被独立修改，不影响渲染管道 |
| **脏矩阵体系完善** | `isDirtyRegionUpdate = true` 即可触发重绘 |
| **帧驱动架构** | `frameStart` 回调是执行布局计算的天然时机 |
| **遍历枚举器** | `arm_ctrl_enum` 支持预序/后序，O(n) 遍历 |
| **内存抽象层** | `ldMalloc/ldFree` 已统一，可用于布局临时分配 |

---

## 3. LVGL Flex/Grid 参考分析

参考实现位置：`third_party/lvgl/src/layouts/`

### 3.1 Flex 实现分析

**文件**：`flex/lv_flex.c`（650 行），`flex/lv_flex.h`（100 行）

**核心数据结构**：

```c
typedef struct {
    lv_flex_align_t main_place;   // 主轴对齐
    lv_flex_align_t cross_place;  // 交叉轴对齐
    lv_flex_align_t track_place;  // 多轨道对齐
    uint8_t row : 1;              // 方向：行/列
    uint8_t wrap : 1;             // 是否换行
    uint8_t rev : 1;              // 是否反向
} flex_t;                         // 共 6 字节

typedef struct {
    lv_obj_t * item;
    int32_t min_size, max_size, final_size;
    uint32_t grow_value;
    uint32_t clamped : 1;
} grow_dsc_t;                     // 约 32 字节/项

typedef struct {
    int32_t track_cross_size;
    int32_t track_main_size;
    int32_t track_fix_main_size;
    int32_t track_grow_min_size;
    uint32_t item_cnt;
    grow_dsc_t * grow_dsc;        // 动态分配
    uint32_t grow_item_cnt;
    uint32_t grow_dsc_calc : 1;
} track_t;                        // 约 40 字节
```

**算法核心**：
1. `find_track_end()` — O(n)，确定一条轨道的子项范围
2. `children_repos()` — O(n + g²)，成长项迭代分配 + 对齐计算
3. `place_content()` — O(1)，计算主轴对齐偏移和间隙

**Flex 对齐选项**：START / END / CENTER / SPACE_EVENLY / SPACE_AROUND / SPACE_BETWEEN

**动态内存**：grow_dsc 数组按需 `lv_realloc`，函数返回前释放

### 3.2 Grid 实现分析

**文件**：`grid/lv_grid.c`（687 行），`grid/lv_grid.h`（90 行）

**轨道定义模式**（整数数组 + 终止符）：

```c
#define LV_GRID_FR(x)       (LV_COORD_MAX - 100 + x)  // 比例单位
#define LV_GRID_CONTENT     (LV_COORD_MAX - 101)        // 内容自适应
#define LV_GRID_TEMPLATE_LAST (LV_COORD_MAX)            // 数组终止符

// 使用示例：
int32_t col_dsc[] = {100, LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};
//                   固定  比例1份        固定  终止
```

**核心计算结构**：

```c
typedef struct {
    int32_t * x;    // 每列起始 x（动态分配）
    int32_t * y;    // 每行起始 y
    int32_t * w;    // 每列宽度
    int32_t * h;    // 每行高度
    uint32_t col_num, row_num;
    int32_t grid_w, grid_h;
} lv_grid_calc_t;
```

**算法核心**：
1. `calc_cols()` / `calc_rows()` — O(col + n)，两阶段（先扫 CONTENT，再分配 FR）
2. `item_repos()` — O(1)/项，按单元格坐标映射
3. `grid_align()` — O(track_num)，轨道级别对齐

**动态内存**：`lv_grid_calc_t` 的 x/y/w/h 数组，计算后释放（`calc_free()`）

### 3.3 LVGL 布局框架集成机制

```c
// 脏标记驱动：obj.layout_inv = 1 → 下一帧重算
void lv_obj_mark_layout_as_dirty(lv_obj_t * obj);

// 布局回调注册
typedef void (*lv_layout_update_cb_t)(lv_obj_t *, void * user_data);

// 计算在 frameStart 等价函数中触发（bottom-up DFS）
layout_update_core(scr);  // 子优先，再父
```

---

## 4. 可行性评估

### 4.1 总体结论

> **结论：在 LingDongGUI 中引入 Flex/Grid 布局完全可行，推荐分阶段实施。**

LingDongGUI 已有的树形结构、帧驱动架构和脏矩阵体系，是实现 Flex/Grid 的坚实基础。核心工作是在 `ldWindow_on_frame_start()` 中增加新的布局计算算法，并扩展相关数据结构。

### 4.2 技术难点分析

| 难点 | 说明 | 应对 |
|------|------|------|
| **嵌入式动态内存** | Flex grow_dsc 数组需动态分配 | 用 `ldMalloc/ldFree`；或引入静态上限（如 max_grow_items=16） |
| **子项属性存储** | 子项需记录 flex-grow、grid-col/row 等属性 | 在 `ldBase_t` 增加位字段（约 +4 字节/控件） |
| **脏标记集成** | 布局变化需传播到父容器 | 沿用 `isDirtyRegionUpdate`，在 `frameStart` 检测 |
| **无样式系统** | LVGL 通过样式存储布局属性，LingDongGUI 无此机制 | 直接在结构体字段存储，或提供专用 setter API |
| **ARM-2D 坐标系** | 坐标是相对父节点的，需注意 | 现有 `ldBaseSetRegion` API 已处理相对坐标 |
| **PFB 脏矩阵联动** | 布局变化后需正确更新脏矩阵范围 | 调用 `ldGuiWidgetUpdate()` 或类似函数 |

### 4.3 嵌入式环境特殊考量

- **RAM 开销**：
  - 每个控件增加约 4 字节（flex/grid 属性位字段）
  - Flex 临时计算峰值：`grow_items × 32 bytes`（20 项约 640 B）
  - Grid 临时计算峰值：`(col + row) × 16 bytes`（10×10 约 320 B）
- **CPU 开销**：布局计算在 `frameStart` 执行，仅在 `isLayoutUpdate=true` 时触发，与 LVGL 的 `layout_inv` 机制相同，不影响正常帧率
- **代码尺寸**：完整实现约增加 **4-6 KB Flash**（Flex ~2.5K + Grid ~2.5K + 框架 ~1K）

---

## 5. 三种实现方案

### 方案 A：最小化增强（仅扩展现有 Window 布局）

**描述**：在 `ldLayoutType_t` 中增加有限的对齐选项，不引入独立布局框架。

**扩展内容**：
- 在 `layoutHorizontal/Vertical` 基础上支持 START/CENTER/END/SPACE_BETWEEN 对齐
- 在 `ldWindow_t` 增加 `gap` 字段
- 支持简单的 flex-grow（仅整数权重，无 min/max 约束）

**代码量估计**：约 400 行（修改现有文件）

**优点**：
- 改动最小，风险极低
- 不引入动态内存
- Flash 增量 < 2 KB

**缺点**：
- 不支持二维 Grid 布局
- 不支持 wrap 换行
- API 与 CSS Flex 规范差距大，学习成本不降低

**适用场景**：资源极度受限的 MCU（< 16 KB RAM），或只需简单对齐的场景

---

### 方案 B：独立 Flex 模块（参考 LVGL，轻量适配）

**描述**：将 LVGL Flex 实现移植到 LingDongGUI，适配数据结构和内存管理，不实现 Grid。

**新增文件**：
```
src/layouts/
├── ldLayout.h/.c    # 布局框架（注册机制）
└── flex/
    └── ldFlex.h/.c  # Flex 布局实现
```

**数据结构扩展**：

```c
// ldWindow_t 增加 Flex 配置（新增约 8 字节）
typedef struct {
    lv_flex_align_t main_place : 4;
    lv_flex_align_t cross_place : 4;
    lv_flex_align_t track_place : 4;
    uint8_t row : 1;        // 行/列方向
    uint8_t wrap : 1;       // 换行
    uint8_t rev : 1;        // 反向
} ldFlexConfig_t;

// ldBase_t 增加子项属性（新增约 4 字节，通过 uint32_t 位字段）
// flex_grow : 4     -- 成长权重 (0-15)
// flex_new_track : 1 -- 强制换新轨道
```

**API 设计**：
```c
// 容器配置
void ldWindowSetFlexFlow(ldWindow_t *ptWindow, ldFlexFlow_t flow);
void ldWindowSetFlexAlign(ldWindow_t *ptWindow,
                          ldFlexAlign_t main,
                          ldFlexAlign_t cross,
                          ldFlexAlign_t track);
void ldWindowSetGap(ldWindow_t *ptWindow, int16_t row_gap, int16_t col_gap);

// 子项配置
void ldBaseSetFlexGrow(ldBase_t *ptWidget, uint8_t grow);
void ldBaseSetFlexNewTrack(ldBase_t *ptWidget, bool isNew);
```

**代码量估计（仅模块代码）**：
- `ldLayout.h/.c`：约 100 行
- `ldFlex.h/.c`：约 600 行
- 修改 `ldBase_t`：+10 行
- 修改 `ldWindow_t`：+20 行
- 修改 `ldWindow.c` 集成点：+30 行
- **总计：约 760 行新增模块代码**（含测试/示例/文档的完整交付见第 8 节，约 1,380 行）

**优点**：
- Flex 功能完整（wrap / 6 种对齐 / grow）
- API 接近 CSS Flex 标准，降低学习成本
- 独立模块，可按需编译
- 完整解决一维响应式布局需求

**缺点**：
- 动态内存（grow_dsc 数组）需在嵌入式环境谨慎使用
- 不含 Grid
- Flash 增量约 3-4 KB

**适用场景**：主流目标（STM32F4/H7 等，RAM ≥ 64 KB），工具栏、导航栏、卡片列表等一维布局场景

---

### 方案 C：完整 Flex + Grid 布局系统

**描述**：同时引入 Flex 和 Grid，建立完整的布局框架，参考 LVGL 但适配嵌入式约束。

**新增文件**：
```
src/layouts/
├── ldLayout.h/.c         # 布局框架
├── flex/
│   └── ldFlex.h/.c       # Flex 布局
└── grid/
    └── ldGrid.h/.c       # Grid 布局
```

**Grid 特有数据结构**：

```c
// 轨道定义（存储在 ldWindow_t 中）
typedef struct {
    int32_t * col_dsc;    // 列描述符数组（用户提供，可为 ROM 数组）
    int32_t * row_dsc;    // 行描述符数组
} ldGridConfig_t;

// Grid 单元格属性（存储在 ldBase_t 中，约 +4 字节）
// grid_col_pos   : 4  -- 列位置 (0-15)
// grid_row_pos   : 4  -- 行位置 (0-15)
// grid_col_span  : 4  -- 列跨度 (1-15)
// grid_row_span  : 4  -- 行跨度 (1-15)
// grid_col_align : 3  -- 列内对齐
// grid_row_align : 3  -- 行内对齐
```

**Grid API 设计**：
```c
// 容器配置（col_dsc/row_dsc 可为 const 数组，存 ROM）
void ldWindowSetGridDsc(ldWindow_t *ptWindow,
                        const int32_t *col_dsc,
                        const int32_t *row_dsc);
void ldWindowSetGridAlign(ldWindow_t *ptWindow,
                          ldGridAlign_t col_align,
                          ldGridAlign_t row_align);

// 子项配置
void ldBaseSetGridCell(ldBase_t *ptWidget,
                       ldGridAlign_t col_align, uint8_t col_pos, uint8_t col_span,
                       ldGridAlign_t row_align, uint8_t row_pos, uint8_t row_span);

// 特殊值宏
#define LD_GRID_FR(x)           ((int32_t)(0x7FFF - 100 + (x)))
#define LD_GRID_CONTENT         ((int32_t)(0x7FFF - 101))
#define LD_GRID_TEMPLATE_LAST   ((int32_t)(0x7FFF))
```

**代码量估计**：
- Flex 部分（同方案 B）：约 760 行
- `ldGrid.h/.c`：约 650 行
- 修改 `ldBase_t`（Grid 字段）：+15 行
- 修改 `ldWindow_t`（Grid 配置）：+20 行
- 修改 `ldWindow.c` 集成：+20 行
- 单元测试（SDL 环境）：约 500 行
- **总计：约 1,965 行代码**

**优点**：
- 功能最完整，解决所有响应式布局场景
- 独立模块，Flex/Grid 可按 `#define` 分别开关
- Grid 轨道描述符可存 ROM，零 RAM 开销
- API 设计对齐 CSS Grid 标准

**缺点**：
- 工作量最大
- Grid 的 `lv_grid_calc_t` 需动态分配（可用静态缓冲区优化）
- Flash 增量约 5-7 KB
- 实现周期较长（建议分两期）

**适用场景**：功能仪表板、设置页面、网格图标布局等二维场景

---

## 6. 方案推荐

### 推荐：**分阶段实施，先方案 B，再方案 C**

**理由**：

1. **Flex 覆盖 80% 的布局需求**：工具栏、按钮组、卡片列表、导航栏，这些高频场景一维 Flex 足够
2. **风险可控**：先验证布局框架集成的正确性，再扩展 Grid
3. **嵌入式友好**：Flex 的内存峰值可控；Grid 的 `lv_grid_calc_t` 可在 Phase 2 用静态缓冲区优化
4. **API 可向前兼容**：Phase 1 设计的 `ldLayout` 框架直接复用于 Phase 2

**实施阶段规划**：

| 阶段 | 内容 | 预估代码量 | 优先级 |
|------|------|----------|--------|
| Phase 1 | `ldLayout` 框架 + `ldFlex` 完整实现 | ~760 行 | P0 |
| Phase 2 | `ldGrid` 实现 + 静态缓冲区优化 | ~700 行 | P1 |
| Phase 3 | 百分比尺寸（`LD_PCT()`）、内容自适应尺寸 | ~200 行 | P2 |

---

## 7. 详细设计（Phase 1：Flex 布局）

### 7.1 数据结构变更

#### `ldBase_t` 扩展（`src/gui/ldBase.h`）

在 `ldBase_t` 尾部增加布局字段，通过位字段压缩：

```c
typedef struct {
    // ... 现有字段（保持不变）...

    // 布局相关（Phase 1 新增约 +2 字节；Phase 2 全部字段约 +4 字节）
    uint8_t flexGrow;           // Flex grow 权重 (0=不成长)   [Phase 1]
    uint8_t flexNewTrack : 1;   // 强制换新轨道                [Phase 1]
#if LD_CFG_LAYOUT_GRID          // 以下字段 Phase 2 才编译进来
    uint8_t gridColPos : 4;     // Grid 列起始位置 (0-15)
    uint8_t gridRowPos : 4;     // Grid 行起始位置 (0-15)
    uint8_t gridColSpan : 4;    // Grid 列跨度 (1-15)
    uint8_t gridRowSpan : 4;    // Grid 行跨度 (1-15)
    uint8_t gridColAlign : 3;   // Grid 列内对齐
    uint8_t gridRowAlign : 3;   // Grid 行内对齐
#endif
} ldBase_t;
```

#### `ldWindow_t` 扩展（`src/gui/ldWindow.h`）

```c
typedef enum {
    layoutNone,
    layoutHorizontal,       // 保留（向后兼容）
    layoutVertical,         // 保留（向后兼容）
    layoutFlex,             // 新增
    layoutGrid,             // 新增（Phase 2）
} ldLayoutType_t;

typedef enum {
    ldFlexAlignStart = 0,
    ldFlexAlignEnd,
    ldFlexAlignCenter,
    ldFlexAlignSpaceEvenly,
    ldFlexAlignSpaceAround,
    ldFlexAlignSpaceBetween,
} ldFlexAlign_t;

typedef struct {
    uint8_t row : 1;            // 0=列方向, 1=行方向
    uint8_t wrap : 1;           // 是否换行
    uint8_t rev : 1;            // 是否反向
    ldFlexAlign_t mainPlace : 4;
    ldFlexAlign_t crossPlace : 4;
    ldFlexAlign_t trackPlace : 4;
    int16_t rowGap;             // 行间距（像素）
    int16_t colGap;             // 列间距（像素）
} ldFlexConfig_t;               // 约 8 字节

struct ldWindow_t {
    implement(ldBase_t);
    ldColor bgColor;
    arm_2d_tile_t *ptImgTile;
    arm_2d_tile_t *ptMaskTile;
    bool isTransparent : 1;
    ldLayoutType_t layoutType : 3;  // 原字段名为 layoutTpye（含拼写错误）；改名是破坏性变更，可通过宏别名保持向后兼容：#define layoutTpye layoutType
    bool isLayoutUpdate : 1;
    ldPadding_t padding;            // 容器内边距（新增，替代 pLayoutPaddingGroup）
    ldFlexConfig_t *ptFlexConfig;   // Flex 配置指针（NULL 时无 Flex）
    ldPadding_t *pLayoutPaddingGroup; // 保留向后兼容
};
```

### 7.2 新增文件结构

```
src/layouts/
├── ldLayout.h          # 布局类型枚举、回调注册接口
├── ldLayout.c          # 框架实现（约 100 行）
└── flex/
    ├── ldFlex.h        # Flex 公开 API
    └── ldFlex.c        # Flex 算法实现（约 600 行）
```

### 7.3 布局触发机制

**集成点**：`ldWindow_on_frame_start()` 函数

```c
void ldWindow_on_frame_start(ld_scene_t *ptScene, ldWindow_t *ptWidget) {
    if (ptWidget->isLayoutUpdate) {
        ptWidget->isLayoutUpdate = false;
        switch (ptWidget->layoutType) {
            case layoutHorizontal:
            case layoutVertical:
                ldWindowLayoutLegacy(ptScene, ptWidget);  // 现有实现
                break;
            case layoutFlex:
                ldFlexUpdate((ldBase_t*)ptWidget);        // 新实现
                break;
            case layoutGrid:
                ldGridUpdate((ldBase_t*)ptWidget);        // Phase 2
                break;
            default:
                break;
        }
    }
}
```

**脏标记传播**：
- 任何子项的 `isLayoutUpdate` 置 true 时，需同时置父容器的 `isLayoutUpdate = true`
- `ldBaseSetFlexGrow()` 等配置函数自动触发此标记

### 7.4 Flex 算法实现要点

**函数清单**（`ldFlex.c`）：

```c
// 主入口（替换现有 Window 布局逻辑）
void ldFlexUpdate(ldBase_t *ptCont);

// 内部：确定一条轨道的起止子项
static int32_t ldFlexFindTrackEnd(
    ldBase_t *ptCont, ldFlexConfig_t *pFlex,
    int32_t itemStartId, int32_t maxMainSize,
    int32_t itemGap, ldFlexTrack_t *ptTrack);

// 内部：重排轨道内子项坐标
static void ldFlexChildrenRepos(
    ldBase_t *ptCont, ldFlexConfig_t *pFlex,
    int32_t itemFirstId, int32_t itemLastId,
    int32_t absX, int32_t absY,
    int32_t maxMainSize, int32_t itemGap,
    ldFlexTrack_t *ptTrack);

// 内部：计算对齐偏移和间隙
static void ldFlexPlaceContent(
    ldFlexAlign_t place, int32_t maxSize,
    int32_t contentSize, int32_t itemCnt,
    int32_t *startPos, int32_t *gap);
```

**嵌入式优化策略**：

1. **静态 grow_dsc 缓冲区**：定义编译期上限，避免运行时 `ldRealloc`
   ```c
   #define LD_FLEX_MAX_GROW_ITEMS  16  // 可在 ldConfig.h 配置
   static ldFlexGrowDsc_t growDscBuf[LD_FLEX_MAX_GROW_ITEMS];  // 栈上分配
   ```

2. **仅在布局变化时计算**：`isLayoutUpdate` 标志确保非变化帧零开销

3. **可选 wrap 支持**：通过 `LD_CFG_LAYOUT_FLEX_WRAP` 宏控制换行功能编译

### 7.5 公开 API 设计

```c
// ------ 容器配置 ------

// 设置 Flex 流方向（自动切换到 layoutFlex 模式）
void ldWindowSetFlexFlow(ldWindow_t *ptWindow, ldFlexFlow_t flow);
// flow 枚举：LD_FLEX_FLOW_ROW / COLUMN / ROW_WRAP / COLUMN_WRAP
// ROW_REVERSE / COLUMN_REVERSE / ROW_WRAP_REVERSE / COLUMN_WRAP_REVERSE

// 设置三轴对齐方式
void ldWindowSetFlexAlign(ldWindow_t *ptWindow,
                          ldFlexAlign_t main,
                          ldFlexAlign_t cross,
                          ldFlexAlign_t track);

// 设置容器内边距
void ldWindowSetPadding(ldWindow_t *ptWindow, ldPadding_t padding);

// 设置子项间距
void ldWindowSetGap(ldWindow_t *ptWindow, int16_t rowGap, int16_t colGap);

// ------ 子项配置 ------

// 设置 flex-grow 权重（0 = 不参与成长分配）
void ldBaseSetFlexGrow(ldBase_t *ptWidget, uint8_t grow);

// 强制此子项开始新轨道（仅 wrap 模式有效）
void ldBaseSetFlexNewTrack(ldBase_t *ptWidget, bool isNew);

// ------ 通用 ------

// 手动触发布局重算（通常不需要，结构修改时自动触发）
void ldWindowMarkLayoutDirty(ldWindow_t *ptWindow);
```

---

## 8. 复杂度总结

### 8.1 实现复杂度

| 组件 | 算法复杂度 | 代码复杂度 | 嵌入式适配难度 |
|------|-----------|----------|--------------|
| 布局框架 (`ldLayout`) | 低 | 低 | 低 |
| Flex 基础 (无 wrap) | 中 | 中 | 低 |
| Flex wrap/grow | 中 | 中-高 | 中（内存管理）|
| Grid 基础 (固定轨道) | 中 | 中 | 低 |
| Grid FR 单位 | 中 | 中 | 中 |
| Grid colspan/rowspan | 高 | 高 | 中 |
| 内容自适应尺寸 | 高 | 高 | 高 |

### 8.2 工作量估算（Phase 1）

| 工作项 | 估计行数 | 说明 |
|--------|---------|------|
| `ldLayout.h/.c` | ~100 | 注册框架 |
| `ldFlex.h/.c` | ~600 | 算法实现 |
| `ldBase_t` / `ldWindow_t` 结构扩展 | ~50 | 数据结构 |
| `ldWindow.c` 集成 | ~30 | 框架接入 |
| 单元测试（SDL 环境） | ~300 | 基础用例 |
| 示例代码 | ~200 | demo/widget 演示 |
| 文档 | ~100 | API 文档更新 |
| **总计** | **~1,380** | |

### 8.3 风险评估

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| 动态内存在 MCU 碎片化 | 中 | 中 | 使用静态 grow_dsc 缓冲区 |
| 脏矩阵传播遗漏 | 低 | 高 | 充分测试动态布局变化场景 |
| 新字段使 `ldBase_t` 过大 | 低 | 低 | 位字段打包，+4 字节可接受 |
| 布局计算阻塞帧渲染 | 低 | 中 | `isLayoutUpdate` 保证仅变化帧执行 |
| 与现有 `layoutHorizontal/Vertical` 冲突 | 低 | 低 | 保留原实现，新类型走新路径 |
| `ldBase_t` 向后兼容性 | 低 | 高 | 新字段位于尾部，零初始化不影响现有控件 |

---

## 9. 配置开关建议

在 `ldConfig.h` 增加以下编译选项（默认开启，按需关闭节省 Flash）：

```c
/* 布局系统开关 */
#ifndef LD_CFG_LAYOUT_FLEX
#define LD_CFG_LAYOUT_FLEX      1   // 0=关闭 Flex，节省约 2.5KB Flash
#endif

#ifndef LD_CFG_LAYOUT_GRID
#define LD_CFG_LAYOUT_GRID      0   // Phase 2 默认关闭
#endif

#ifndef LD_CFG_LAYOUT_FLEX_WRAP
#define LD_CFG_LAYOUT_FLEX_WRAP 1   // 0=关闭换行支持，节省约 0.5KB
#endif

#ifndef LD_FLEX_MAX_GROW_ITEMS
#define LD_FLEX_MAX_GROW_ITEMS  16  // 每个 Flex 容器最多可成长子项数
#endif
```

---

## 10. 参考资料

- LVGL Flex 实现：`third_party/lvgl/src/layouts/flex/lv_flex.c`（650 行）
- LVGL Grid 实现：`third_party/lvgl/src/layouts/grid/lv_grid.c`（687 行）
- LVGL 布局框架：`third_party/lvgl/src/layouts/lv_layout.c`（103 行）
- 当前 Window 布局：`src/gui/ldWindow.c:161-235`
- 基础结构定义：`src/gui/ldBase.h:193-217`
- CSS Flexbox 规范：https://www.w3.org/TR/css-flexbox-1/
- CSS Grid 规范：https://www.w3.org/TR/css-grid-1/
