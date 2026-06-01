# PicoUI a-0.7 LingDongGUI Native 100% Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `.worktree/a-0.7` 中把 PicoUI 从 `a-0.6 release-contract subset` 升级为 LingDongGUI 原生 `ld*` 控件能力 100% 覆盖。

**Architecture:** `a-0.7` 固定依赖为 `R0 -> R1 -> R2 -> parallel(R3A/R3B/R4A/R4B/R5A/R5B/R6A/R6B) -> R7 -> R8`。先串行建立 native inventory、shared native bridge、base/layout/theme/event，再并行补齐不重叠控件批次，最后串行升级 matrix/gate/manual artifact/docs。所有阶段必须以真实 LingDongGUI backend 字段、真实事件、真实资源、真实 readback 为证据，不接受 host-cache-only、fake visual、temporary smoke path。

**Tech Stack:** C、CMake、CTest、Python3、SDL2 host runtime、PicoUI、LingDongGUI、GitNexus、Markdown serial docs

---

## 0. 执行规则

- 主线 worktree 固定：`.worktree/a-0.7`
- 建议分支：`feat/picoui-a-0-7-native-100-parity`
- 创建或切换后必须执行：

```bash
git worktree add .worktree/a-0.7 -b feat/picoui-a-0-7-native-100-parity HEAD
cd .worktree/a-0.7
git submodule sync --recursive
git submodule update --init --recursive
```

- 严格串行段：`R0 -> R1 -> R2`，以及 `R7 -> R8`
- 可并行段：`R3A / R3B / R4A / R4B / R5A / R5B / R6A / R6B`
- 每个 Task 使用 fresh subagent 执行
- 每个 Task 结束后必须做独立只读 review；review 不通过时，由原执行 subagent 修复
- 涉及 C / Python 符号修改前必须先跑 GitNexus impact
- 每个 Task 结束前至少运行 `git diff --check`
- 每个 Task 结束后运行 `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`

### 并行执行规则

`R2` 合并并验证通过后，创建以下并行 worktree：

```bash
git worktree add .worktree/a-0.7-r3a-text-image -b feat/picoui-a-0-7-r3a-text-image HEAD
git worktree add .worktree/a-0.7-r3b-selection -b feat/picoui-a-0-7-r3b-selection HEAD
git worktree add .worktree/a-0.7-r4a-input-nav -b feat/picoui-a-0-7-r4a-input-nav HEAD
git worktree add .worktree/a-0.7-r4b-data -b feat/picoui-a-0-7-r4b-data HEAD
git worktree add .worktree/a-0.7-r5a-progress-qrcode -b feat/picoui-a-0-7-r5a-progress-qrcode HEAD
git worktree add .worktree/a-0.7-r5b-instrument-clock -b feat/picoui-a-0-7-r5b-instrument-clock HEAD
git worktree add .worktree/a-0.7-r6a-composite -b feat/picoui-a-0-7-r6a-composite HEAD
git worktree add .worktree/a-0.7-r6b-modal-animation -b feat/picoui-a-0-7-r6b-modal-animation HEAD
```

每个 worktree 创建或切换后必须执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

并行批次只允许最终提交：

1. 自己负责的 `picoui/include/picoui/<widget>.h`
2. 自己负责的 `picoui/src/widgets/<widget>.c`
3. 自己负责的 `picoui/src/backend/ldgui/backend_<widget>.c`
4. 自己负责的 `picoui/demo/<widget>_basic/main.c`
5. 自己负责的 `tests/picoui/unit/test_picoui_<widget>.c`
6. 自己负责的 `tests/picoui/contract/native_100_fragments/<batch>.json`

并行批次不得最终提交：

1. `picoui/include/picoui/picoui.h`
2. `tests/picoui/CMakeLists.txt`
3. `tests/picoui/contract/check_picoui_public_api.py`
4. `tests/picoui/contract/picoui_release_capability_matrix.json`
5. `tests/picoui/contract/check_picoui_release_capability_matrix.py`
6. `tests/picoui/runtime/check_picoui_runtime.py`
7. `tests/picoui/runtime/check_picoui_backend_mapping.py`
8. `tests/picoui/runtime/check_picoui_visible_ui.py`
9. `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
10. `picoui/docs/demo_guide.md`

这些聚合文件由 `R7` 串行合流。并行批次可在本地临时改它们做验证，但提交前必须把临时聚合改动整理成 fragment 或交接说明。

## 1. 文件结构与阶段边界

### R0 native inventory / truth-source

**Create:**
- `tests/picoui/contract/picoui_native_100_inventory.json`
- `tests/picoui/contract/check_picoui_native_100_inventory.py`
- `tests/picoui/contract/native_100_fragments/.gitkeep`

**Modify:**
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `docs/picoui-serial/a-0.7-线计划索引.md`

### R1 native bridge shared-core

**Create:**
- `picoui/include/picoui/native.h`
- `picoui/src/core/native.c`
- `tests/picoui/unit/test_picoui_native_bridge.c`

**Modify:**
- `picoui/include/picoui/picoui.h`
- `picoui/include/picoui/widget.h`
- `picoui/include/picoui/theme.h`
- `picoui/src/core/internal.h`
- `picoui/src/core/widget.c`
- `picoui/src/backend/ldgui/backend.h`
- `picoui/src/backend/ldgui/backend_widget.c`
- `tests/picoui/CMakeLists.txt`
- `tests/picoui/contract/check_picoui_public_api.py`

### R2 base / layout / theme / event 100%

**Modify:**
- `picoui/include/picoui/widget.h`
- `picoui/include/picoui/window.h`
- `picoui/include/picoui/layout.h`
- `picoui/include/picoui/theme.h`
- `picoui/src/core/widget.c`
- `picoui/src/core/event.c`
- `picoui/src/layout/flex.c`
- `picoui/src/layout/grid.c`
- `picoui/src/theme/theme.c`
- `picoui/src/backend/ldgui/backend_layout.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_theme.c`
- `picoui/src/backend/ldgui/backend_window.c`
- `tests/picoui/unit/test_picoui_layout.c`
- `tests/picoui/unit/test_picoui_theme.c`
- `tests/picoui/unit/test_picoui_button_events.c`

### R3 old-widget native parity

**Modify:**
- `picoui/include/picoui/label.h`
- `picoui/include/picoui/text.h`
- `picoui/include/picoui/image.h`
- `picoui/include/picoui/button.h`
- `picoui/include/picoui/checkbox.h`
- `picoui/include/picoui/switch.h`
- `picoui/include/picoui/slider.h`
- `picoui/include/picoui/list.h`
- `picoui/src/widgets/label.c`
- `picoui/src/widgets/text.c`
- `picoui/src/widgets/image.c`
- `picoui/src/widgets/button.c`
- `picoui/src/widgets/checkbox.c`
- `picoui/src/widgets/switch.c`
- `picoui/src/widgets/slider.c`
- `picoui/src/widgets/list.c`
- matching `picoui/src/backend/ldgui/backend_*.c`
- matching `tests/picoui/unit/test_picoui_*.c`

### R4 input / navigation / data-widget native parity

**Modify:**
- `picoui/include/picoui/line_edit.h`
- `picoui/include/picoui/keyboard.h`
- `picoui/include/picoui/combo_box.h`
- `picoui/include/picoui/scroll_selecter.h`
- `picoui/include/picoui/table.h`
- `picoui/include/picoui/graph.h`
- `picoui/include/picoui/calendar.h`
- matching widget/backend/unit/demo files
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`

### R5 visual / progress / instrument / clock / qrcode native parity

**Modify:**
- `picoui/include/picoui/progress_bar.h`
- `picoui/include/picoui/progress_wheel.h`
- `picoui/include/picoui/qrcode.h`
- `picoui/include/picoui/arc.h`
- `picoui/include/picoui/gauge.h`
- `picoui/include/picoui/date_time.h`
- `picoui/include/picoui/clock.h`
- matching widget/backend/unit/demo files

### R6 composite / modal / animation native parity

**Create:**
- `picoui/include/picoui/animation.h`
- `picoui/src/widgets/animation.c`
- `picoui/src/backend/ldgui/backend_animation.c`
- `picoui/demo/animation_basic/main.c`
- `tests/picoui/unit/test_picoui_animation.c`

**Modify:**
- `picoui/include/picoui/icon_slider.h`
- `picoui/include/picoui/radial_menu.h`
- `picoui/include/picoui/message_box.h`
- `picoui/include/picoui/picoui.h`
- `picoui/src/widgets/icon_slider.c`
- `picoui/src/widgets/radial_menu.c`
- `picoui/src/widgets/message_box.c`
- matching backend/unit/demo/gate files

### R7 native-100 matrix / gate / manual artifact

**Modify:**
- `tests/picoui/contract/picoui_native_100_inventory.json`
- `tests/picoui/contract/check_picoui_native_100_inventory.py`
- `tests/picoui/contract/picoui_release_capability_matrix.json`
- `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- `tests/picoui/contract/check_picoui_public_api.py`
- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- `picoui/docs/demo_guide.md`

### R8 closeout / docs / release readiness audit

**Modify:**
- `docs/picoui-serial/a-0.7-线计划索引.md`
- `docs/picoui-serial/a-0.6-控件能力1对1对比审计.md`
- `docs/picoui-serial/a-0.6-final-release-closeout.md`
- `docs/superpowers/specs/2026-06-01-picoui-a-0-7-native-100-parity-design.md`
- `docs/superpowers/plans/2026-06-01-picoui-a-0-7-native-100-parity-implementation.md`
- `picoui/docs/api_overview.md`
- `picoui/docs/demo_guide.md`

## 2. Tasks

### Task R0: native inventory / truth-source

**Files:**
- Create: `tests/picoui/contract/picoui_native_100_inventory.json`
- Create: `tests/picoui/contract/check_picoui_native_100_inventory.py`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="check_picoui_release_capability_matrix.py", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 建立 native inventory**

`picoui_native_100_inventory.json` 必须至少包含：

```json
{
  "schema_version": "a-0.7-native-100-inventory-v1",
  "source": "src/gui/ld*.h",
  "widget_like_total": 27,
  "widgets": [
    {
      "name": "animation",
      "ldgui_type": "ldAnimation",
      "header": "src/gui/ldAnimation.h",
      "required_native_capabilities": [
        "init_image_period",
        "show_frame_tile",
        "frame_lifecycle"
      ],
      "picoui_widget": "picoui_animation",
      "status": "missing_implementation"
    }
  ]
}
```

- [ ] **Step 3: 写 inventory checker**

`check_picoui_native_100_inventory.py` 必须：

1. 解析 `src/gui/ldBase.h` 的 `ldWidgetType_t`。
2. 断言 widget-like 总数为 `27`。
3. 断言 inventory 覆盖 `animation`。
4. 断言每个 widget 至少有一个 native capability。
5. 断言没有 `reject / deferred / incomplete_contract` 字段。

- [ ] **Step 4: 运行 RED/GREEN**

Run:

```bash
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

Expected:

1. inventory checker 通过。
2. release matrix 可暂时允许 `missing_implementation`，但必须识别 `a-0.7-native-100-v1`。

- [ ] **Step 5: detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

### Task R1: native bridge shared-core

**Files:**
- Create: `picoui/include/picoui/native.h`
- Create: `picoui/src/core/native.c`
- Create: `tests/picoui/unit/test_picoui_native_bridge.c`
- Modify: `picoui/include/picoui/picoui.h`
- Modify: `picoui/include/picoui/widget.h`
- Modify: `picoui/src/core/internal.h`
- Modify: `picoui/src/backend/ldgui/backend.h`
- Modify: `tests/picoui/CMakeLists.txt`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_widget_set_bg_color", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_widget_init_data_model", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 定义 native bridge public API**

`picoui/include/picoui/native.h` 必须定义：

```c
struct picoui_native_image {
    void *tile;
    void *mask;
    unsigned int mask_color;
};

struct picoui_native_font {
    void *font;
};

enum picoui_native_align {
    PICOUI_NATIVE_ALIGN_START,
    PICOUI_NATIVE_ALIGN_CENTER,
    PICOUI_NATIVE_ALIGN_END,
    PICOUI_NATIVE_ALIGN_STRETCH,
    PICOUI_NATIVE_ALIGN_SPACE_EVENLY,
    PICOUI_NATIVE_ALIGN_SPACE_AROUND,
    PICOUI_NATIVE_ALIGN_SPACE_BETWEEN
};

enum picoui_native_nav_dir {
    PICOUI_NATIVE_NAV_LEFT,
    PICOUI_NATIVE_NAV_RIGHT,
    PICOUI_NATIVE_NAV_UP,
    PICOUI_NATIVE_NAV_DOWN
};
```

- [ ] **Step 3: 写 native bridge unit**

新增测试必须覆盖：

```c
static void test_native_image_preserves_tile_and_mask_pointers(void);
static void test_native_font_preserves_font_pointer(void);
static void test_native_align_maps_all_ldgrid_align_values(void);
static void test_native_nav_dir_maps_all_ld_nav_values(void);
```

- [ ] **Step 4: 实现最小 bridge**

实现要求：

1. 所有 resource wrapper 都只包装，不复制。
2. backend 收到的 tile/mask/font 指针必须与 public API 输入一致。
3. enum 映射必须 exhaustively tested。

- [ ] **Step 5: 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R '^test_picoui_native_bridge$' --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

### Task R2: base / layout / theme / event 100%

**Files:**
- Modify: `picoui/include/picoui/widget.h`
- Modify: `picoui/include/picoui/window.h`
- Modify: `picoui/include/picoui/layout.h`
- Modify: `picoui/include/picoui/theme.h`
- Modify: `picoui/src/core/widget.c`
- Modify: `picoui/src/core/event.c`
- Modify: `picoui/src/backend/ldgui/backend_layout.c`
- Modify: `picoui/src/backend/ldgui/backend_event.c`
- Modify: `tests/picoui/unit/test_picoui_layout.c`
- Modify: `tests/picoui/unit/test_picoui_theme.c`
- Modify: `tests/picoui/unit/test_picoui_button_events.c`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_widget_set_enabled", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_window_set_grid_align", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_backend_widget_dispatch_signal", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 补 base/layout/theme/event API**

必须覆盖：

1. `ldBaseSetHidden / Opacity / Selectable / Select / Corner / Center`
2. `ldBaseSetFlexGrow / FlexNewTrack / FlexMinWidth / FlexMinHeight / FlexMaxWidth / FlexMaxHeight / IgnoreLayout / GridCell`
3. `ldWindowSetLayout / FlexFlow / FlexAlign / FlexTrackAlign / Padding / FlexGap / Gap / GridColumns / GridDscArray / GridAlign / GridGap / GridPadding / PaddingGroup`
4. `SIGNAL_PRESS / HOLD_DOWN / RELEASE / CLICKED_ITEM / FINISHED / VALUE_CHANGED`
5. theme state/part/color/metric 对真实 backend style 字段的映射

- [ ] **Step 3: 写 fail-first tests**

至少新增：

```c
static void test_widget_native_base_flags_round_trip_to_ldbase(void);
static void test_widget_native_flex_min_max_round_trip_to_ldbase(void);
static void test_window_native_grid_descriptors_round_trip_to_ldwindow(void);
static void test_native_event_bridge_preserves_press_hold_release_payload(void);
static void test_theme_native_parts_apply_to_real_backend_fields(void);
```

- [ ] **Step 4: 实现并验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(layout|theme|button_events|widgets)' --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
git diff --check
```

### Task R3A: `label / text / image`

**Files:** matching header/widget/backend/unit/demo files for `label / text / image`.
- Create: `tests/picoui/contract/native_100_fragments/r3a-text-image.json`

- [ ] **Step 1: 跑 impact**

Run:

```text
gitnexus_impact(target="picoui_label_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_text_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_image_set_source", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 补 native capabilities**

必须覆盖：

1. `ldLabelSetTransparent / Text / TextColor / Align / BackgroundImage / BackgroundColor / Font`
2. `ldTextSetTransparent / Text / StaticText / TextColor / Font / ConsumedFont / BackgroundImage / BackgroundColor / ScrollSeek / ScrollMove`
3. `ldImageSetImage / MaskColor`

- [ ] **Step 3: 写 backend-field tests**

至少新增：

```c
static void test_label_native_background_image_and_font_round_trip(void);
static void test_text_native_static_text_and_scroll_round_trip(void);
static void test_image_native_mask_color_round_trip(void);
static void test_image_theme_and_enabled_are_support_not_reject(void);
```

- [ ] **Step 4: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(widgets|theme)' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R3B: `button / checkbox / switch / slider / list`

**Files:** matching header/widget/backend/unit/demo files for `button / checkbox / switch / slider / list`.
- Create: `tests/picoui/contract/native_100_fragments/r3b-selection.json`

- [ ] **Step 1: 跑 impact**

Run impact for each changed public setter before edit:

```text
gitnexus_impact(target="picoui_button_set_text", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_checkbox_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_switch_set_checked", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_slider_set_value", direction="upstream", repo="LingDongGUI")
gitnexus_impact(target="picoui_list_set_selected_index", direction="upstream", repo="LingDongGUI")
```

- [ ] **Step 2: 补 native capabilities**

必须覆盖：

1. button：color/image/transparent/font/text/textColor/checkable/keyValue/press/readback。
2. checkbox：color/image/text/radioGroup/textColor/stringLeftSpace/checked。
3. switch：color/image/canNavigate/navigate/horizontal/direction/disabled/checked/readback。
4. slider：percent/horizontal/image/color/indicatorWidth/slimSize。
5. list：itemHeight/text/textColor/align/bgColor/selectColor/itemWidget/selectItem/padding/margin。

- [ ] **Step 3: 写 backend-field tests**

至少新增：

```c
static void test_checkbox_native_radio_group_and_image_mode_round_trip(void);
static void test_switch_native_direction_navigation_and_image_skin_round_trip(void);
static void test_list_native_item_widget_padding_margin_round_trip(void);
static void test_list_item_marker_is_support_not_reject(void);
```

- [ ] **Step 4: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(widgets|list)' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R4A: `line_edit / keyboard / combo_box / scroll_selecter`

**Files:** matching header/widget/backend/unit/demo/gate files.
- Create: `tests/picoui/contract/native_100_fragments/r4a-input-nav.json`

- [ ] **Step 1: 补 native capabilities**

必须覆盖：

1. line_edit：text/keyboard/type/align/color/textMax/finished reason。
2. keyboard：navigate/update/btnUpdate/click/exit/key map/user draw/target ownership。
3. combo_box：textColor/bgColor/frameColor/selectColor/selectItem/staticItems/itemMax/addItem/dropdownImage/readback。
4. scroll_selecter：items/textColor/bgColor/bgImage/indicatorColor/indicatorImage/transparent/speed/selectItem/selectText/editMode/readback。

- [ ] **Step 2: 清除特殊边界**

要求：

1. `line_edit.editing_and_finished_boundary_without_reason` 转 support。
2. `line_edit.submit_cancel_reason` 转 support。
3. `keyboard.mapping` 和 `keyboard.visible` 从 `not_present` 转 `present`。

- [ ] **Step 3: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(line_edit|keyboard|combo_box|scroll_selecter)' --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
git diff --check
```

### Task R4B: `table / graph / calendar`

**Files:** matching header/widget/backend/unit/demo/gate files.
- Create: `tests/picoui/contract/native_100_fragments/r4b-data.json`

- [ ] **Step 1: 补 native capabilities**

必须覆盖：

1. table：itemWidth/itemHeight/itemText/staticText/itemColor/itemFont/bgColor/itemAlign/itemImage/itemButton/keyboard/itemEditable/excelType/alignGrid/navigate/itemSelect/getItem/getAlign/getEditable/getRegion。
2. graph：axis/axisOffset/frameSpace/gridOffset/pointImageMask/setValue/moveAdd。
3. calendar：dayNames/header/headerFormat/date/getDate/color/theme/grid。

- [ ] **Step 2: 写 backend-field tests**

至少新增：

```c
static void test_table_native_item_image_button_and_excel_type_round_trip(void);
static void test_graph_native_axis_grid_and_point_mask_round_trip(void);
static void test_calendar_native_day_names_and_colors_round_trip(void);
```

- [ ] **Step 3: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(table|graph|calendar)' --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
git diff --check
```

### Task R5A: `progress_bar / progress_wheel / qrcode`

**Files:** matching header/widget/backend/unit/demo/gate files.
- Create: `tests/picoui/contract/native_100_fragments/r5a-progress-qrcode.json`

- [ ] **Step 1: 补 native capabilities**

必须覆盖：

1. progress_bar：percent/image/frameImage/color/frameColor/horizontal/inverted。
2. progress_wheel：progress/wheelColor/dotColor/dotEnable。
3. qrcode：text/qrColor/bgColor/ecc/maxVersion/zoom。

- [ ] **Step 2: 清除 reject**

要求：

1. `progress_bar.advanced_skin_and_theme` 转 support。
2. `progress_wheel.advanced_animation_and_theme` 转 support。
3. `qrcode.advanced_qrcode_configuration` 转 support。

- [ ] **Step 3: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(progress_bar|progress_wheel|qrcode)' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R5B: `arc / gauge / date_time / clock`

**Files:** matching header/widget/backend/unit/demo/gate files.
- Create: `tests/picoui/contract/native_100_fragments/r5b-instrument-clock.json`

- [ ] **Step 1: 补 native capabilities**

必须覆盖：

1. arc：quarterImage/quarterMask/parentColor/backgroundAngle/foregroundAngle/rotationAngle/color/readback。
2. gauge：bgImage/bgMask/centreOffset/pointerImage/pointerColor/angle/trail/progressBar/autoMove。
3. date_time：transparent/format/textColor/align/bgColor/date/time。
4. clock：backgroundImage/hourPointer/minutePointer/secondPointer/maskColor/anchor/stepSecond。

- [ ] **Step 2: 清除 reject**

要求：

1. `date_time.advanced_datetime_modes` 转 support。
2. `clock.advanced_clock_configuration` 转 support。

- [ ] **Step 3: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(arc|gauge|date_time|clock)' --output-on-failure
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
git diff --check
```

### Task R6A: `icon_slider / radial_menu`

**Files:** matching header/widget/backend/unit/demo/gate files.
- Create: `tests/picoui/contract/native_100_fragments/r6a-composite.json`

- [ ] **Step 1: 补 native capabilities**

必须覆盖：

1. icon_slider：icon image/mask/name、horizontalScroll、speed、iconWidth/iconSpace/columns/rows/pages。
2. radial_menu：item image/mask、clickItem、offsetItem、defaultItem、xAxis/yAxis/itemMax。

- [ ] **Step 2: 写 backend-field tests**

至少新增：

```c
static void test_icon_slider_native_icon_images_and_speed_round_trip(void);
static void test_radial_menu_native_click_default_offset_round_trip(void);
```

- [ ] **Step 3: 验证**

Run:

```bash
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(icon_slider|radial_menu)' --output-on-failure
git diff --check
```

### Task R6B: `message_box / animation`

**Files:**
- Create: `picoui/include/picoui/animation.h`
- Create: `picoui/src/widgets/animation.c`
- Create: `picoui/src/backend/ldgui/backend_animation.c`
- Create: `picoui/demo/animation_basic/main.c`
- Create: `tests/picoui/unit/test_picoui_animation.c`
- Create: `tests/picoui/contract/native_100_fragments/r6b-modal-animation.json`
- Modify: `picoui/include/picoui/message_box.h`
- Modify: `picoui/src/widgets/message_box.c`
- Modify: `picoui/src/backend/ldgui/backend_message_box.c`
- Modify: `tests/picoui/unit/test_picoui_message_box.c`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`

- [ ] **Step 1: 补 message_box native capabilities**

必须覆盖：

1. title/msg/buttons。
2. multi-button callback。
3. string colors。
4. button colors。
5. background color。
6. modal behavior。
7. formal mapping marker，不能再 exclusion。

- [ ] **Step 2: 新增 animation widget**

必须覆盖：

1. `picoui_animation_create_with_props`。
2. image tile / periodMs。
3. `picoui_animation_show_frame`。
4. frame lifecycle proof。
5. runtime/mapping/visible/manual artifact demo。

- [ ] **Step 3: 清除 reject / not wrapped**

要求：

1. `message_box.multi_button_and_modal_behavior` 转 support。
2. `animation` 进入 `27/27` widget rows。
3. `picoui/include/picoui/picoui.h` include `animation.h`。

- [ ] **Step 4: 验证**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -R 'test_picoui_(message_box|animation)' --output-on-failure
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

### Task R7: native-100 matrix / gate / manual artifact

**Files:**
- Modify: `tests/picoui/contract/picoui_native_100_inventory.json`
- Modify: `tests/picoui/contract/check_picoui_native_100_inventory.py`
- Modify: `tests/picoui/contract/picoui_release_capability_matrix.json`
- Modify: `tests/picoui/contract/check_picoui_release_capability_matrix.py`
- Modify: `tests/picoui/runtime/check_picoui_runtime.py`
- Modify: `tests/picoui/runtime/check_picoui_backend_mapping.py`
- Modify: `tests/picoui/runtime/check_picoui_visible_ui.py`
- Modify: `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: matrix closeout**

要求：

1. `schema_version = a-0.7-native-100-v1`
2. `ldgui_widget_like_total = 27`
3. `picoui_native_wrapped_total = 27`
4. `not_wrapped = 0`
5. `reject = 0`
6. `deferred = 0`
7. `incomplete_contract = 0`
8. `missing_implementation = 0`
9. 合并所有 `tests/picoui/contract/native_100_fragments/*.json`

- [ ] **Step 2: gate closeout**

要求：

1. runtime catalog 覆盖全部 native-100 demos。
2. mapping catalog 不允许 exclusion。
3. visible catalog 覆盖 `animation / keyboard / message_box`。
4. manual artifact 支持 `--all` 并覆盖全部 native-100 demos。

- [ ] **Step 3: 验证**

Run:

```bash
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
git diff --check
```

### Task R8: closeout / docs / release readiness audit

**Files:**
- Modify: `docs/picoui-serial/a-0.7-线计划索引.md`
- Modify: `docs/picoui-serial/a-0.6-控件能力1对1对比审计.md`
- Modify: `docs/picoui-serial/a-0.6-final-release-closeout.md`
- Modify: `picoui/docs/api_overview.md`
- Modify: `picoui/docs/demo_guide.md`

- [ ] **Step 1: docs closeout**

要求：

1. `a-0.7-线计划索引.md` 增加当前 closeout 状态。
2. `a-0.6-控件能力1对1对比审计.md` 保留历史审计，新增 a-0.7 后继关系。
3. `a-0.6-final-release-closeout.md` 保持 a-0.6 口径，不回写成 native-100。
4. `api_overview.md` 和 `demo_guide.md` 进入 native-100 口径。

- [ ] **Step 2: final verification**

Run:

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
python3 tests/picoui/contract/check_picoui_native_100_inventory.py
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/contract/check_picoui_release_capability_matrix.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --all
git diff --check
```

- [ ] **Step 3: GitNexus detect changes**

Run:

```text
gitnexus_detect_changes(scope="all", repo="LingDongGUI")
```

Expected:

1. 受影响流程与 PicoUI/backend/gate/docs 一致。
2. 不出现无关 LingDongGUI core 渲染改动。
3. 不出现 demo-only fake renderer 路径。

## 3. 最终交付物

`a-0.7` 完成后必须交付：

1. native-100 public API。
2. `animation` widget 全链路。
3. 27 控件全部 backend mapping。
4. 27 控件全部 unit proof。
5. 27 控件全部 runtime/mapping/visible/manual artifact proof。
6. `a-0.7-native-100-v1` matrix。
7. native inventory checker。
8. 中文 closeout。
