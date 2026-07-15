# TinyUI API 总览

本文描述 TinyUI v2.3 **current-facing** public API。契约真相源以 `tests/tinyui/contract/tinyui_v23_public_api.json` 与 public 头为准。

## 定位

- TinyUI 是 LingDongGUI 上的轻量 canonical API，**不是**第二套对象树、renderer、layout solver 或事件传播系统。
- 参考 LVGL 的 **API 使用模式**（init / screen / obj / event / timer / style），但**不兼容 LVGL**，也不复制其内部机制。
- 用户不需要直接调用 `ld*` 或 Arm-2D；current-facing 文档与 demo 只推荐 `tinyui_*`。

统一入口：`#include <tinyui.h>`（按 feature 宏裁剪可选控件与 theme）。

通用约定：

| 主题 | 规则 |
| --- | --- |
| 所有权 | 父对象拥有子对象；`tinyui_obj_delete(root)` 释放子树 |
| 错误 | 多数 API 返回 `tinyui_result_t`；部分控件 setter 返回 `int`（0 成功） |
| 最近错误 | `tinyui_last_result()` / `tinyui_last_error_message()` |
| `NOT_SUPPORTED` | 能力在当前对象 kind 或裁剪配置下不可用时返回 `TINYUI_ERROR_NOT_SUPPORTED`，不伪造成功 |
| 空指针 | 非法参数返回 `TINYUI_ERROR_INVALID_ARG` 或 `INVALID_OBJECT` |

---

## 1. Runtime

**头文件：** `core/runtime.h`（经 `tinyui.h`）

| 符号 | 说明 |
| --- | --- |
| `tinyui_init` / `tinyui_deinit` | 进程内单实例 runtime 生命周期 |
| `tinyui_screen_create` / `tinyui_screen_create_with_props` | 创建 screen（底层 window 能力） |
| `tinyui_background_create` | 可选背景对象 |
| `tinyui_screen_load(screen, transition, duration_ms)` | 加载/切换 screen；无动画用 `TINYUI_SCREEN_TRANSITION_NONE` + `0` |
| `tinyui_screen_active` | 当前 active screen |
| `tinyui_process(uint32_t *next_ms)` | 推进一帧：定时器、输入消费、绘制调度；写出建议休眠毫秒 |

**所有权：** screen 由调用方创建；load 后仍由调用方在退出路径删除（或随 deinit 策略清理）。
**错误：** 未 init 时调用 process/load 返回 `INVALID_STATE`；参数非法返回 `INVALID_ARG`。
**集成：** 显示/输入/时钟由平台宿主提供；`next_ms` 的 sleep 是集成层职责。

---

## 2. Object

**头文件：** `core/obj.h`

`tinyui_obj_t` 为不透明对象句柄。核心能力：

- 树导航：`get_parent` / `get_first_child` / `get_next_sibling` / `get_root` / `get_child_count` / `find_by_id`
- 生命周期：`tinyui_obj_delete`
- 通用属性：`set_pos` / `set_size` / `set_text` / `set_bg_color` / `set_text_color` / `set_border_*` / `set_radius` / `set_padding` / `set_visible` / `set_enabled` / `set_opacity` / `set_selectable` / `set_selected`

**所有权：** 子对象挂在 parent 上；删除父节点释放子树。
**错误：** 对不支持某属性的 kind 返回 `NOT_SUPPORTED`（经 kind→adapter 映射，不暴露后端类型）。

---

## 3. Widgets

**头文件：** `widgets/*.h`（由 `tinyui.h` 按 `TINYUI_ENABLE_*` 条件包含）

创建模式统一为：

```text
tinyui_obj_t *tinyui_<widget>_create(tinyui_obj_t *parent);
tinyui_obj_t *tinyui_<widget>_create_with_props(parent, const props *);
```

常用控件包括（非穷尽）：`label`、`button`、`checkbox`、`switch`、`slider`、`text`、`image`、`list`、`progress_bar`、`progress_wheel`、`qrcode`、`message_box`、`date_time`、`clock`、`line_edit`、`keyboard`、`combo_box`、`scroll_selector`、`arc`、`gauge`、`graph`、`table`、`calendar`、`icon_slider`、`radial_menu`、`animation`、`canvas`、`window`、`background`。

**窗口控件：** `tinyui_window_*` 是合法的 canonical 控件 API（与 runtime 的 `tinyui_screen_*` 分工不同：screen 是页面根，window 是可嵌套容器能力）。

**所有权：** 返回的 `tinyui_obj_t *` 由 parent 树拥有；创建失败返回 `NULL`（可查 `tinyui_last_result()`）。
**props：** `fields` 位掩码标明哪些成员有效；未置位字段保持默认。

示例（button 点击）：

```c
tinyui_obj_t *btn = tinyui_button_create(parent);
tinyui_button_set_text(btn, "OK");
tinyui_button_set_on_clicked(btn, on_clicked, user_data);
```

---

## 4. Event + Focus

**头文件：** `core/event.h`、`core/focus.h`

### 事件

- 事件码：`PRESSED` / `RELEASED` / `CLICKED` / `VALUE_CHANGED` / `FOCUSED` / `DEFOCUSED` / `KEY` / `DELETE`
- 回调：`typedef void (*tinyui_event_cb_t)(const tinyui_event_t *event);`
- 注册：`tinyui_obj_add_event_cb(obj, event_mask, cb, user_data, &handle)`
- 移除：`tinyui_obj_remove_event_cb(obj, handle)`
- 掩码：`TINYUI_EVENT_MASK(code)` / `TINYUI_EVENT_MASK_ALL`
- 容量：固定池 `TINYUI_EVENT_CB_CAPACITY`（默认 16）；满则 `CAPACITY`

部分控件提供窄转发（如 `tinyui_button_set_on_clicked`），语义为替换式写入统一 event 池。

### 焦点

- `tinyui_focus_set` / `tinyui_focus_clear` / `tinyui_focus_move` / `tinyui_focus_current`
- 方向：`NEXT` / `PREVIOUS` / `LEFT` / `RIGHT` / `UP` / `DOWN`

**错误：** 非法对象或当前状态不支持时返回对应 `tinyui_result_t`；焦点在无可聚焦目标时可能 `NOT_SUPPORTED` 或 `INVALID_STATE`。

---

## 5. Timer

**头文件：** `core/timer.h`

```c
tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms,
                                    bool repeat,
                                    tinyui_timer_cb_t cb,
                                    void *user_data);
tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer, uint32_t interval_ms);
void tinyui_timer_delete(tinyui_timer_t *timer);
```

**所有权：** 调用方持有 `tinyui_timer_t *`，不用时 `tinyui_timer_delete`。
**调度：** 由 `tinyui_process` 推进；不在回调中假设可重入创建无限定时器。
**容量：** `TINYUI_TIMER_CAPACITY`（默认 16）；满则创建失败 / `CAPACITY`。

---

## 6. Theme + Style

**头文件：** `theme/theme.h`（`TINYUI_ENABLE_THEME`）、`style/style.h`

### Style

- `tinyui_style_t`：按 `fields` 位掩码携带 bg/text/border/radius/padding/opacity/font
- `tinyui_obj_apply_style(obj, part, state, &style)` 立即应用到对象
- part：`MAIN` / `TEXT` / `INDICATOR` / `KNOB` / `TRACK`
- state：`DEFAULT` / `DISABLED` / `PRESSED` / `CHECKED` / `FOCUSED`

**生命周期：** style 结构在 `apply` 调用期间被读取；调用返回后调用方可释放临时 style。core **不**缓存 style，也**不**对 font 指针做引用计数。若 style 携带 font 指针，则 font 在对象仍使用该字体期间必须保持有效。

### Theme

- `tinyui_theme_set` / `tinyui_theme_get`：进程级主题快照（颜色 + 度量）
- `tinyui_theme_apply(obj)`：把当前主题应用到对象树/节点

**错误：** 未启用 theme 时相关 API 不可用；非法参数返回 `INVALID_ARG`。

---

## 7. Flex + Grid

**头文件：** `layout/layout.h`

### 容器

- Flex：`tinyui_flex_set_flow` / `set_align` / `set_gap`
- Grid：`tinyui_grid_set_columns` / `set_rows` / `set_gap` / `set_align`
- 轨道：`tinyui_grid_track_t`（`PX` / `FR` / `CONTENT`），最多 `TINYUI_GRID_MAX_TRACKS`

### 子项（public child 属性）

- Flex 子项：`tinyui_obj_set_flex_grow` / `set_flex_new_track` / `set_flex_min_width` / `set_flex_min_height` / `set_flex_max_width` / `set_flex_max_height`
- Grid 子项：`tinyui_obj_set_grid_cell(obj, col, row, col_span, row_span, x_align, y_align)`
- 通用：`tinyui_obj_set_ignore_layout`

**错误：** 对象不参与布局或 kind 不支持时 `NOT_SUPPORTED`；轨道数超限 `OUT_OF_RANGE` / `INVALID_ARG`。

---

## 8. Image + Font

**头文件：** `resource/image_source.h`、`resource/font.h`

### Image source

| Factory | 含义 |
| --- | --- |
| `tinyui_image_source_from_rgb565` | 借用调用方像素缓冲（不拷贝） |
| `tinyui_image_source_from_builtin` | 内置资源枚举 |
| `tinyui_image_source_from_vres` | 外部 vres 地址 |
| `tinyui_image_source_deinit` | 清空句柄；不释放调用方缓冲或静态 builtin |

### Font

| Factory | 含义 |
| --- | --- |
| `tinyui_font_from_builtin` | 内置字体枚举 |
| `tinyui_font_from_vres` | vres 字体 |
| `tinyui_font_deinit` | 清空句柄 |

**绑定规则：** 传给 `tinyui_image_set_source` / label/button font setter 的 descriptor，在对象仍引用期间必须存活。core **没有**资源中心缓存或引用计数。详见 [资源生命周期](./resource_lifetime.md)。

---

## 9. Result

**头文件：** `core/result.h`

```c
typedef enum tinyui_result {
    TINYUI_OK = 0,
    TINYUI_ERROR_INVALID_ARG,
    TINYUI_ERROR_INVALID_OBJECT,
    TINYUI_ERROR_INVALID_STATE,
    TINYUI_ERROR_NOT_SUPPORTED,
    TINYUI_ERROR_OUT_OF_RANGE,
    TINYUI_ERROR_NO_MEMORY,
    TINYUI_ERROR_CAPACITY,
    TINYUI_ERROR_BACKEND,
} tinyui_result_t;

tinyui_result_t tinyui_last_result(void);
const char *tinyui_last_error_message(void);
```

语义摘要：

| 码 | 典型场景 |
| --- | --- |
| `INVALID_ARG` | NULL 指针、非法枚举 |
| `INVALID_OBJECT` | 句柄已删除或不是预期对象 |
| `INVALID_STATE` | 未 init、重复 load 等 |
| `NOT_SUPPORTED` | 当前 kind/配置无此能力 |
| `OUT_OF_RANGE` | 尺寸/索引/轨道越界 |
| `NO_MEMORY` | 分配失败 |
| `CAPACITY` | timer/event 固定池满 |
| `BACKEND` | 后端操作失败（不向用户暴露后端类型） |

---

## 相关文档

- [快速开始](./quick_start.md)
- [资源生命周期](./resource_lifetime.md)
- [Demo 运行指南](./demo_guide.md)
