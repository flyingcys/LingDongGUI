# Grid 对齐 LVGL 主干 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 继续优化 LingDongGUI 的 `grid` 主干行为，使其在保持现有 LingDongGUI 命名习惯的前提下，尽量对齐 LVGL 常用 grid 语义；完成后审计 `switch / flex / grid` 三条线并细化任务板。

**Architecture:** 以 `src/gui/ldWindow.c` 的 descriptor-grid solver 为核心收口点，优先用 host-side 测试驱动 `FR / CONTENT / span / align / auto placement` 语义，再用 `USE_DEMO=5` 作为运行态证据。`switch` 与 `flex` 本轮不扩功能，只在 grid 收口完成后回写审计文档，明确已对齐、未对齐与延期项。

**Tech Stack:** C, CMake, Arm-2D, SDL2 dummy runtime, GitNexus impact analysis, `examples/sdl/tests/layout/test_layout_window.c`

---

## 文件结构

- Modify: `src/gui/ldWindow.c`
  - descriptor-grid solver 主实现：track kind、`CONTENT/FR` 分配、axis align、cell resolve、auto placement
- Modify: `src/gui/ldWindow.h`
  - 如需补最小注释或声明收口，只在现有接口范围内调整
- Modify: `src/gui/ldBase.c`
  - 如 grid child metadata 或 `ldBaseSetGridCell()` 需要最小行为修补时使用
- Modify: `src/gui/ldBase.h`
  - 如 grid child metadata 需要最小字段/声明补充时使用
- Modify: `examples/sdl/tests/layout/test_layout_window.c`
  - host-side grid 行为回归测试
- Modify: `examples/common/demo/layout/uiLayout.c`
  - `USE_DEMO=5` grid demo 运行态证据
- Modify: `examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md`
  - 回写三线审计结论
- Modify: `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md`
  - 回写 grid 收口后的最新真相

## Task 1: 先把 grid 剩余差距锁成 failing tests

**Files:**
- Modify: `examples/sdl/tests/layout/test_layout_window.c`
- Reference: `src/gui/ldWindow.c`

- [ ] **Step 1: 写 descriptor-grid 的 failing tests，覆盖本轮要继续收口的 4 类主干语义**

```c
static void test_grid_layout_content_track_uses_largest_visible_child(void)
{
    ldWindow_t root = {0};
    ldLabel_t small = {0};
    ldLabel_t large = {0};
    static const int16_t col_dsc[] = {LD_GRID_CONTENT, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 160, 40);
    set_widget_region((ldBase_t *)&small, 0, 0, 24, 8);
    set_widget_region((ldBase_t *)&large, 0, 0, 58, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&small);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&large);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldBaseSetGridCell((ldBase_t *)&small, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&large, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(large.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(root.gridColDsc == col_dsc);
}

static void test_grid_layout_auto_placement_skips_occupied_spans(void)
{
    ldWindow_t root = {0};
    ldLabel_t span = {0};
    ldLabel_t auto_a = {0};
    ldLabel_t auto_b = {0};
    static const int16_t col_dsc[] = {30, 30, 30, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {18, 18, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 120, 60);
    set_widget_region((ldBase_t *)&span, 0, 0, 12, 8);
    set_widget_region((ldBase_t *)&auto_a, 0, 0, 12, 8);
    set_widget_region((ldBase_t *)&auto_b, 0, 0, 12, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&span);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&auto_a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&auto_b);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 4, 6);
    ldBaseSetGridCell((ldBase_t *)&span, ldGridAlignStretch, 0, 2, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(auto_a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 72);
    assert(auto_a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(auto_b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(auto_b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
}

static void test_grid_layout_span_with_fr_tracks_preserves_expected_width(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};
    static const int16_t col_dsc[] = {40, LD_GRID_FR(1), LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 160, 40);
    set_widget_region((ldBase_t *)&child, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 4);
    ldBaseSetGridCell((ldBase_t *)&child, ldGridAlignStretch, 1, 2, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 44);
    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 116);
}

static void test_grid_layout_container_center_keeps_track_order_and_offsets(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 90, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 10);
    ldWindowSetGridAlign(&root, ldGridAlignCenter, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 10);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 40);
}
```

- [ ] **Step 2: 跑 layout test，确认新增测试先红**

Run:

```bash
rtk cmake --build examples/sdl/build-review --target layout_window_test
rtk ./examples/sdl/build-review/layout_window_test
```

Expected:

- 至少 1 条新增 grid 测试失败
- 失败信息指向当前 solver 的落位、`CONTENT`、span 或 align 行为

- [ ] **Step 3: 提交 failing tests**

```bash
rtk git add examples/sdl/tests/layout/test_layout_window.c
rtk git commit -m "test: lock remaining grid lvgl gaps"
```

## Task 2: 最小修复 descriptor-grid solver，让新增测试转绿

**Files:**
- Modify: `src/gui/ldWindow.c`
- Modify: `src/gui/ldWindow.h`
- Modify: `src/gui/ldBase.c`
- Modify: `src/gui/ldBase.h`
- Test: `examples/sdl/tests/layout/test_layout_window.c`

- [ ] **Step 1: 对将修改的关键入口先做 GitNexus impact 分析**

Run:

```bash
rtk python - <<'PY'
print("impact targets: ldWindow_on_frame_start, ldWindowSetGridDscArray, ldBaseSetGridCell")
PY
```

然后分别调用 GitNexus：

- `impact(target_uid="Function:src/gui/ldWindow.c:ldWindow_on_frame_start", direction="upstream", repo="LingDongGUI", includeTests=true, minConfidence=0.8)`
- `impact(target_uid="Function:src/gui/ldWindow.c:ldWindowSetGridDscArray", direction="upstream", repo="LingDongGUI", includeTests=true, minConfidence=0.8)`（若索引无 uid，则先 `context()` 定位）
- `impact(target="ldBaseSetGridCell", file_path="src/gui/ldBase.c", kind="Function", direction="upstream", repo="LingDongGUI", includeTests=true, minConfidence=0.8)`

Expected:

- 风险为 `LOW` 或可接受 `MEDIUM`
- 若出现 `HIGH/CRITICAL`，暂停并先记录 blast radius

- [ ] **Step 2: 以最小改动修 `CONTENT`、span 占位和 auto placement**

在 `src/gui/ldWindow.c` 重点审查并修改：

- `ldWindowGridResolveCell()`
- `ldWindowGridResolveContentTracks()`
- `ldWindowGridGetCellSize()`
- descriptor-grid 主循环内的 cell 分配逻辑

目标代码方向：

```c
static bool ldWindowGridCellOccupiesTrack(const ldWindowGridCell_t *ptCell,
                                          uint16_t trackIndex,
                                          bool isColumnAxis)
{
    uint16_t start = isColumnAxis ? (uint16_t)ptCell->colPos : (uint16_t)ptCell->rowPos;
    uint16_t span = isColumnAxis ? (uint16_t)ptCell->colSpan : (uint16_t)ptCell->rowSpan;
    return (trackIndex >= start) && (trackIndex < (start + span));
}

static void ldWindowGridFindNextAutoCell(const bool *pOccupied,
                                         uint16_t colCount,
                                         uint16_t rowCount,
                                         uint16_t *pAutoCol,
                                         uint16_t *pAutoRow)
{
    uint16_t index;
    for (index = 0; index < (uint16_t)(colCount * rowCount); ++index)
    {
        if (pOccupied[index] == false)
        {
            *pAutoCol = (colCount > 0) ? (index % colCount) : 0;
            *pAutoRow = (colCount > 0) ? (index / colCount) : 0;
            return;
        }
    }
}
```

要求：

- 只有在确实需要时才引入 occupancy bookkeeping
- 不扩到 `subgrid / RTL / ignore-layout`
- 保持 `gridColumns` fallback 路径不变

- [ ] **Step 3: 跑单测，确认新增 grid 测试转绿**

Run:

```bash
rtk cmake --build examples/sdl/build-review --target layout_window_test
rtk ./examples/sdl/build-review/layout_window_test
```

Expected:

- `layout_window_test` 全绿

- [ ] **Step 4: 跑完整 layout/switch/swipe 测试，确认没有把别的主线打坏**

Run:

```bash
rtk cmake --build examples/sdl/build-review
rtk ctest --test-dir examples/sdl/build-review --output-on-failure
```

Expected:

- `9/9` tests passed

- [ ] **Step 5: 提交 solver 收口**

```bash
rtk git add src/gui/ldWindow.c src/gui/ldWindow.h src/gui/ldBase.c src/gui/ldBase.h examples/sdl/tests/layout/test_layout_window.c
rtk git commit -m "fix: align grid solver with lvgl mainline"
```

## Task 3: 用 demo 证明 grid 主干语义真的可见

**Files:**
- Modify: `examples/common/demo/layout/uiLayout.c`
- Test: `examples/sdl/build-grid-review/ldgui_sdl_demo`

- [ ] **Step 1: 先给 grid demo 增一个最缺的主干展示点**

如果当前 demo 还没直接展示“auto placement 遇到 span 后自动跳格”或“`CONTENT + FR` 混合后的可见效果”，在 `examples/common/demo/layout/uiLayout.c` 新增最小展示块，方向如下：

```c
static const int16_t s_grid_col_dsc[] = {92, LD_GRID_CONTENT, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
static const int16_t s_grid_row_dsc[] = {54, 66, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};

/* 新增一组未显式 cell 的 panel，用于展示 auto placement fallback */
uiGridCreatePanel(ptScene,
                  ID_GRID_CELL_G,
                  ID_GRID_CELL_G_TITLE,
                  ID_GRID_CELL_G_HINT,
                  72,
                  UI_GRID_PANEL_HEIGHT_XS,
                  __RGB(120, 140, 168),
                  ldGridAlignStart,
                  0,
                  0,
                  ldGridAlignStart,
                  0,
                  0,
                  "Auto",
                  "no explicit cell\nauto placement fallback");
```

要求：

- 不重做 demo 结构
- 只补能证明本轮收口结果的最小场景

- [ ] **Step 2: 构建 `USE_DEMO=5`，确认 demo 编译通过**

Run:

```bash
rtk cmake -S examples/sdl -B examples/sdl/build-grid-review -DUSE_DEMO=5
rtk cmake --build examples/sdl/build-grid-review --target ldgui_sdl_demo
```

Expected:

- `ldgui_sdl_demo` build success

- [ ] **Step 3: 跑 dummy SDL smoke，确认 grid demo 启动**

Run:

```bash
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-grid-review/ldgui_sdl_demo >/tmp/ldgui-grid-mainline.log 2>&1 & pid=$!; sleep 3; kill "$pid"; wait "$pid" 2>/dev/null || true
rtk sed -n '1,60p' /tmp/ldgui-grid-mainline.log
```

Expected:

- 进程可启动
- 日志中出现 `init` 链路，无 crash / assert

- [ ] **Step 4: 提交 demo 证据收口**

```bash
rtk git add examples/common/demo/layout/uiLayout.c
rtk git commit -m "feat: strengthen grid demo parity evidence"
```

## Task 4: 回写 switch / flex / grid 三线审计与当前状态文档

**Files:**
- Modify: `examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md`
- Modify: `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md`
- Reference: `README.md`
- Reference: `examples/sdl/docs/2026-05-24-flex-vs-lvgl-gap-analysis.md`
- Reference: `examples/sdl/docs/2026-05-24-grid-vs-lvgl-gap-analysis.md`

- [ ] **Step 1: 先把本轮 grid 完成结果回写到 current-status**

将 `examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md` 中的 grid 结论更新为“本轮完成后的真相”，至少包含：

```md
- `grid`：descriptor-grid 主干继续收口后，当前已稳定支持 `fixed / CONTENT / FR`、explicit cell、span、cell align、container align、descriptor-grid 下 auto placement fallback，并保留 legacy `gridColumns` 兼容入口。
- 明确未做：`subgrid`、`RTL`、grid 下专门的 `ignore-layout`。
```

- [ ] **Step 2: 更新三线任务板，把 switch/flex/grid 重新分成已对齐/未对齐/延期**

任务板至少回写下面 3 组结论：

```md
## switch
- 已对齐：基础交互、方向、禁用、首帧稳态、demo、测试
- 待复核：是否仍有视觉和易用性边角差距

## flex
- 已对齐：主干 flow/align/wrap/grow/new-track/ignore-layout
- 未对齐：RTL、margin、percent/content-size、完整 min/max

## grid
- 已对齐：descriptor-grid 主干、fixed/CONTENT/FR、explicit cell、span、cell align、container align、auto placement fallback
- 延期：subgrid、RTL、grid ignore-layout
```

- [ ] **Step 3: 自检两份文档的一致性**

Run:

```bash
rtk rg -n "subgrid|RTL|ignore-layout|auto placement|CONTENT|FR|span" \
  examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md \
  examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md
```

Expected:

- 两份文档对“已做 / 未做 / 延期”的口径一致

- [ ] **Step 4: 提交文档回写**

```bash
rtk git add examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md
rtk git commit -m "docs: update lvgl parity audit status"
```

## Task 5: 最终验证与提交前影响检查

**Files:**
- Verify only: 当前任务涉及的全部文件

- [ ] **Step 1: 重新跑 GitNexus detect_changes，确认影响范围仍符合预期**

Run GitNexus:

- `detect_changes(repo="LingDongGUI", scope="unstaged")`（若都已 staged/committed，则用 compare 或当前 diff 范围等价方式）

Expected:

- 风险维持 `LOW` 或可接受 `MEDIUM`
- 影响面集中在 layout/grid/docs/demo/test

- [ ] **Step 2: 跑最终完整验证链**

Run:

```bash
rtk cmake --build examples/sdl/build-review
rtk ctest --test-dir examples/sdl/build-review --output-on-failure
rtk cmake --build examples/sdl/build-grid-review --target ldgui_sdl_demo
SDL_VIDEODRIVER=dummy rtk ./examples/sdl/build-grid-review/ldgui_sdl_demo >/tmp/ldgui-grid-final.log 2>&1 & pid=$!; sleep 3; kill "$pid"; wait "$pid" 2>/dev/null || true
rtk sed -n '1,40p' /tmp/ldgui-grid-final.log
```

Expected:

- build 通过
- `ctest` 全绿
- grid demo smoke 正常

- [ ] **Step 3: 检查最终变更集，确认没有把 scope 扩到非本轮目标**

Run:

```bash
rtk git status --short
rtk git diff --stat HEAD~3..HEAD
```

Expected:

- 改动集中在 grid solver、layout test、grid demo、审计文档
- 未顺手引入 `subgrid / RTL / grid ignore-layout`

- [ ] **Step 4: 汇总并提交最终收口 commit**

```bash
rtk git add src/gui/ldWindow.c src/gui/ldWindow.h src/gui/ldBase.c src/gui/ldBase.h \
  examples/sdl/tests/layout/test_layout_window.c \
  examples/common/demo/layout/uiLayout.c \
  examples/sdl/docs/2026-05-25-switch-flex-grid-current-status.md \
  examples/sdl/docs/2026-05-25-switch-flex-grid-lvgl-audit-taskboard.md
rtk git commit -m "feat: finish grid lvgl mainline pass"
```

## Plan Self-Review

- **Spec coverage:** 本计划覆盖了设计文档中的 3 个主块：`grid` 主干收口、验证面补强、三线审计回写。
- **Placeholder scan:** 没有保留 `TODO/TBD/implement later` 之类占位；每个 task 都给出文件、命令和示例代码。
- **Type consistency:** 计划内使用的接口名保持当前 LingDongGUI 习惯：`ldWindowSetGridDscArray`、`ldWindowSetGridAlign`、`ldBaseSetGridCell`，没有混入 LVGL 命名。

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-05-25-grid-lvgl-mainline-implementation.md`。

由于仓库规则要求开发必须使用 subagent，实际可执行选项以 **Subagent-Driven** 为准：

**1. Subagent-Driven（推荐，且符合仓库规则）** - 我按 task 派发 fresh subagent，逐任务 review 后再推进

**2. Inline Execution（仅当你明确要覆盖仓库约束时才考虑）** - 在当前 session 里批量执行 task

你要我按哪个方式继续？
