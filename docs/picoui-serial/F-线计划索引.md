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

- `F0`：worktree 准备和 baseline
- `F1`：list public contract 和 unit RED
- `F2`：list widget 和 backend mapping
- `F3`：list demo 和 runtime gate
- `F4`：docs 和 F线索引收口
- `F5`：F线 closeout review

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

