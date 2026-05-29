# PicoUI F线计划索引

- `D线` 并行边界：`docs/picoui-serial/D-线计划索引.md`
- `F线` 总设计真相源：`docs/superpowers/specs/2026-05-29-picoui-f-line-new-widget-vertical-slice-design.md`
- `F线` 总实施口径：`docs/superpowers/plans/2026-05-29-picoui-f-line-new-widget-vertical-slice-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `F线` 可以与 `D线` 并行。
- `F线` 内部串行推进。
- 第一批新控件只做 `picoui_list` vertical slice。
- `line_edit`、`combo_box`、`table`、`keyboard`、`arc` 暂缓。
- `F0` baseline 已通过；`F1/F2` 已完成并通过独立 review。
- `F3` demo 和 gate matrix 已接入，但 mapping/visible gate 当前为 `BLOCKED`，原因是 runtime marker 分类位于 `D线` 禁止文件 `picoui/src/backend/ldgui/backend_app.c`。

## F线目标

**F线唯一目标**：在不抢占 D 线 shared backend/layout/event/theme 写面的前提下，新增一个低耦合新控件 `picoui_list`，并同步 public API、backend mapping、demo、unit test、runtime/mapping/visible gate 和文档。

## F线不是什么

- 不是当前控件完整性主线。
- 不是 shared backend 重构线。
- 不是一次性补齐所有新控件。
- 不是通过修改现有 demo 绕过 D 线缺口。

## worktree

- 建议路径：`.worktree/picoui-f-new-widgets`
- 创建或切换后必须执行：

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

## 写面边界

允许：

- `picoui/include/picoui/list.h`
- `picoui/src/widgets/list.c`
- `picoui/src/backend/ldgui/backend_list.c`
- `picoui/demo/list_basic/main.c`
- `tests/picoui/unit/test_picoui_list.c`
- `tests/picoui/runtime/*` 中 list demo 的矩阵项
- `picoui/docs/demo_guide.md`

谨慎修改，合并阶段由主线程串行整合：

- `picoui/include/picoui/picoui.h`
- `tests/picoui/CMakeLists.txt`
- `examples/sdl/CMakeLists.txt`
- PicoUI source list CMake 文件

禁止，除非 D 线完成对应阶段并释放写面：

- `picoui/src/backend/ldgui/backend_app.c`
- `picoui/src/backend/ldgui/backend_layout.c`
- `picoui/src/backend/ldgui/backend_event.c`
- `picoui/src/backend/ldgui/backend_style_apply.c`
- `picoui/src/backend/ldgui/backend_theme.c`

## 阶段导航

- `F0`：worktree 准备和 baseline - DONE
- `F1`：list public contract 和 unit RED - DONE
- `F2`：list widget 和 backend mapping - DONE
- `F3`：list demo 和 runtime gate - BLOCKED
- `F4`：docs 和 F线索引收口 - DONE_FOR_CURRENT_BLOCKED_STATE
- `F5`：F线 closeout review - REVIEWED_WITH_KNOWN_BLOCKER

## 当前进度证据

### F0 baseline

已执行：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

当前结论：baseline 通过。注意：runtime/visible/mapping 脚本共用 `build/picoui-runtime`，不要并行跑这些标签，否则 CMake configure 可能互相抢同一 build 目录。

### F1 public contract

已新增：

- `picoui/include/picoui/list.h`
- `tests/picoui/unit/test_picoui_list.c`

当前结论：RED 已确认，随后进入 F2。独立 review 已通过。

### F2 widget/backend mapping

已新增或接入：

- `picoui/src/widgets/list.c`
- `picoui/src/backend/ldgui/backend_list.c`
- `PICOUI_BACKEND_WIDGET_LIST`
- `picoui_backend_create_list`
- `picoui_backend_list_set_items`
- `picoui_backend_list_set_selected_index`
- `picoui_backend_list_get_selected_index`

已执行：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target test_picoui_list
ctest --test-dir build -R test_picoui_list --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
git diff --check
```

当前结论：F2 通过；backend 使用真实 `ldList_init`、`ldListSetText`、`ldListSetSelectItem`、`ldListGetSelectItem`。独立 review 已通过。

### F3 demo/gate

已新增或接入：

- `picoui/demo/list_basic/main.c`
- `picoui_list_basic_demo`
- runtime smoke matrix
- backend mapping matrix
- visible gate matrix
- demo boundary matrix

已通过：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target picoui_list_basic_demo
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
git diff --check
```

当前失败：

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
```

失败输出锚点：

```text
PICOUI_BACKEND_REAL_WIDGET_IDS=title
PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK
PICOUI_BACKEND_FALLBACK_WIDGET_IDS=list
```

当前结论：F3 写面和 gate 设计已通过独立 review，但 F3 不能收口。`PICOUI_BACKEND_WIDGET_LIST` 已有真实 `ldList` backend，但 runtime marker 的 supported-real 分类在 `picoui/src/backend/ldgui/backend_app.c`，该文件属于 `D线` 禁止写面。`F线` 当前不得修改该文件，也不得弱化 mapping/visible gate 或伪造 `item_*` marker。

### F4/F5 current-state review

已执行独立 review，当前态结论：

- Critical：无新增问题。
- Important：无新增问题。
- Minor：无新增问题。
- `F线 current-state review pass with known blocker`。

review 覆盖：

- `picoui/demo/list_basic/main.c` 只使用 `picoui_*` API，未泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
- `picoui/include/picoui/list.h` 未泄漏底层类型/API。
- 未修改 `D线` 禁止文件：`backend_app.c`、`backend_layout.c`、`backend_event.c`、`backend_style_apply.c`、`backend_theme.c`。
- runtime/mapping/visible matrix 已同步，且没有弱化 gate。
- F 线索引和 demo guide 已区分 smoke、mapping、visible、manual artifact，没有把 blocked 状态写成完成。
- `picoui_list` 未扩到 multi-select、virtualization、drag reorder、keyboard navigation、`line_edit` 等非目标。

review 复核命令：

```bash
ctest --test-dir build -R test_picoui_list --output-on-failure
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
git diff --check
```

当前通过项：`test_picoui_list`、demo boundary、public API、runtime smoke、`git diff --check`。

当前失败项：backend mapping、visible UI；失败原因仍是 `picoui_list_basic_demo` 输出 `PICOUI_BACKEND_FALLBACK_WIDGET_IDS=list`。

## 当前明确做什么

1. 先做 `picoui_list`。
2. 先完成最小 vertical slice，再考虑 `line_edit`。
3. 每个新增 demo 必须同步 runtime、mapping、visible matrix。
4. 每次汇报都区分 smoke、mapping、visible、manual artifact。

## 当前明确不做什么

1. 不实现 `line_edit`。
2. 不实现 multi-select、virtualization、drag reorder。
3. 不改 D 线 shared backend 文件。
4. 不把 list demo 的 visible gate 通过写成所有 list 行为完成。
5. 不把 `picoui_list_basic_demo` 的 runtime smoke 通过写成 mapping/visible 已完成。
