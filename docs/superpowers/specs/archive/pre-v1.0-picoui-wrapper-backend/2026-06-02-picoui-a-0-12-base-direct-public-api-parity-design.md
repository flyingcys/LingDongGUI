# PicoUI a-0.12 base direct public API parity 设计

## 1. 背景

`a-0.11` fresh audit 已确认 PicoUI 当前仍未达到 strict `100% direct public API parity`。剩余 direct public API 候选只在 `base`，共 `16` 个：

1. geometry/alignment helper：`5`
2. focus navigation：`2`
3. tree traversal：`5`
4. name/type lookup：`4`

这些能力在 a-0.11 中被标为 `optional_public_extension`，表示它们不是 a-0.10 release policy 必做项，但如果目标是“LingDongGUI 原生能力对外 100%”，它们必须被设计成 PicoUI public API 或被明确拒绝。用户已同意进入 `a-0.12`，因此本线进入实现阶段。

## 2. 目标

`a-0.12` 必须完成：

1. 为 `base` 的 16 个缺口提供 PicoUI public API。
2. API 必须 portable，不能泄漏 LingDongGUI native types。
3. 每个 covered row 必须具备：
   - `picoui/include` public declaration
   - `picoui/src` implementation
   - backend/native proof
   - unit test
   - contract JSON
   - `docs/ability` 更新
4. 完成后 `optional_public_extension=0`。

## 3. API 设计

### 3.1 Portable geometry types

新增到 `picoui/include/picoui/widget.h`：

```c
struct picoui_point {
    int x;
    int y;
};

struct picoui_size {
    int width;
    int height;
};

struct picoui_rect {
    int x;
    int y;
    int width;
    int height;
};
```

这些类型对应 LingDongGUI/Arm-2D region/location/size，但不暴露 `arm_2d_*`。

### 3.2 Widget type

新增：

```c
enum picoui_widget_type {
    PICOUI_WIDGET_TYPE_UNKNOWN = 0,
    PICOUI_WIDGET_TYPE_BACKGROUND,
    PICOUI_WIDGET_TYPE_WINDOW,
    PICOUI_WIDGET_TYPE_BUTTON,
    PICOUI_WIDGET_TYPE_IMAGE,
    PICOUI_WIDGET_TYPE_TEXT,
    PICOUI_WIDGET_TYPE_LINE_EDIT,
    PICOUI_WIDGET_TYPE_GRAPH,
    PICOUI_WIDGET_TYPE_CHECKBOX,
    PICOUI_WIDGET_TYPE_SLIDER,
    PICOUI_WIDGET_TYPE_SWITCH,
    PICOUI_WIDGET_TYPE_PROGRESS_BAR,
    PICOUI_WIDGET_TYPE_GAUGE,
    PICOUI_WIDGET_TYPE_QRCODE,
    PICOUI_WIDGET_TYPE_DATE_TIME,
    PICOUI_WIDGET_TYPE_ICON_SLIDER,
    PICOUI_WIDGET_TYPE_COMBO_BOX,
    PICOUI_WIDGET_TYPE_ARC,
    PICOUI_WIDGET_TYPE_RADIAL_MENU,
    PICOUI_WIDGET_TYPE_SCROLL_SELECTER,
    PICOUI_WIDGET_TYPE_LABEL,
    PICOUI_WIDGET_TYPE_TABLE,
    PICOUI_WIDGET_TYPE_KEYBOARD,
    PICOUI_WIDGET_TYPE_ANIMATION,
    PICOUI_WIDGET_TYPE_LIST,
    PICOUI_WIDGET_TYPE_MESSAGE_BOX,
    PICOUI_WIDGET_TYPE_CALENDAR,
    PICOUI_WIDGET_TYPE_PROGRESS_WHEEL,
    PICOUI_WIDGET_TYPE_CLOCK,
};
```

该 enum 映射 `ldWidgetType_t`，但不要求用户包含 `ldBase.h`。

### 3.3 Tree/name/type API

新增：

```c
struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget);
int picoui_widget_get_child_count(const struct picoui_widget *widget);
int picoui_widget_get_name_id(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_find_by_name_id(const struct picoui_widget *root, int name_id);
enum picoui_widget_type picoui_widget_get_type(const struct picoui_widget *widget);
```

覆盖：

1. `ldBaseGetParent`
2. `ldBaseGetChildList`
3. `ldBaseGetNextSibling`
4. `ldBaseGetRootNode`
5. `ldBaseGetChildCount`
6. `ldBaseGetNameId`
7. `ldBaseGetWidget`
8. `ldBaseGetWidgetById`
9. `ldBaseGetWidgetType`

### 3.4 Geometry helper API

新增：

```c
struct picoui_point picoui_widget_get_absolute_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);
struct picoui_point picoui_widget_get_relative_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);
struct picoui_rect picoui_rect_align(struct picoui_rect parent,
                                     struct picoui_rect child,
                                     enum picoui_align x_align,
                                     enum picoui_align y_align);
struct picoui_rect picoui_rect_center(struct picoui_rect parent,
                                      struct picoui_rect child);
int picoui_vertical_grid_align_offset(struct picoui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space);
```

覆盖：

1. `ldBaseGetAbsoluteLocation`
2. `ldBaseGetRelativeLocation`
3. `ldBaseGetAlignRegion`
4. `ldBaseAlignRegionCenter`
5. `ldBaseAutoVerticalGridAlign`

### 3.5 Focus navigation API

新增：

```c
int picoui_focus_reset(struct picoui_app *app);
int picoui_focus_navigate(struct picoui_app *app, enum picoui_native_nav_dir dir);
```

覆盖：

1. `ldBaseFocusNavigateInit`
2. `ldBaseFocusNavigate`

`picoui_focus_navigate` 使用 PicoUI app/focus model，不暴露 `ld_scene_t`。

## 4. 非目标

1. 不暴露 `ldBase_t`、`ld_scene_t`、`arm_2d_region_t`、`arm_2d_location_t`、`ldWidgetType_t`。
2. 不实现 `policy_never_public=191`。
3. 不修改 LingDongGUI native API。
4. 不用 demo/截图作为 public API proof。

## 5. 验收

1. unit test 覆盖 16 个 API 映射。
2. release matrix checker 能验证新 public API 均存在于 `picoui/include`。
3. matrix summary 更新为：
   - `covered_total=420`
   - `allowlisted_total=191`
   - `direct_public_covered_total=420`
   - `policy_allowlisted_total=191`
   - `direct_public_100_complete=true`
   - `direct_100_category_counts={"policy_never_public":191}`
4. `docs/ability/base.md` 不再列 strict 100% 缺口候选。
5. `docs/ability/README.md` 不再说 `optional_public_extension=16` 是剩余缺口候选。
