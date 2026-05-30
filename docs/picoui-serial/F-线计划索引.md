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
- `F3` demo 和 gate matrix 已接入；合并后的当前代码已经在 `backend_app.c` 释放并接入 `PICOUI_BACKEND_WIDGET_LIST` marker 分类，mapping/visible gate 当前通过。
- `F线` 当前完成的是 `picoui_list` 最小 vertical slice，不等于 100% 封装 `ldList` 的全部能力。

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
- `F3`：list demo 和 runtime gate - DONE
- `F4`：docs 和 F线索引收口 - DONE
- `F5`：F线 closeout review - REVIEWED

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

当前结论：F2 通过；backend 使用真实 `ldList_init`、`ldListSetText`、`ldListSetSelectItem`，并提供 `ldListGetSelectItem` backend helper。注意：当前 PicoUI public `picoui_list_get_selected_index()` 仍返回 PicoUI shadow state，不是实时 native `ldListGetSelectItem()` 读回。独立 review 已通过。

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

当前通过：

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo list_basic
```

当前结论：F3 已收口。`PICOUI_BACKEND_WIDGET_LIST` 使用真实 `ldList` backend，runtime marker 当前会把 `list` 归入真实 backend 映射，并把 `item_wifi/item_bluetooth/item_display` 作为 list item marker 一并输出。该证据证明的是 list vertical slice 的 backend mapping 与 automatic visible gate；其中 item marker 只证明 payload 已写入真实 `ldList`，不证明 item 是独立 backend widget，也不证明人工窗口验收，更不证明 `ldList` 全能力已由 PicoUI 100% 暴露。

### F4/F5 current-state review

已执行独立 review，当前态结论：

- Critical：`picoui_list_set_on_selected()` 公开 API 已暴露，但当前未接入 native selection event bridge，合同未兑现。
- Important：`PICOUI_BACKEND_REAL_WIDGET_IDS` 当前混入 list item marker，语义强于真实代码事实。
- Minor：无新增问题。
- `F线 closeout review with known gaps`。

review 覆盖：

- `picoui/demo/list_basic/main.c` 只使用 `picoui_*` API，未泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
- `picoui/include/picoui/list.h` 未泄漏底层类型/API。
- F 线实现期未抢占 `D线` shared backend 写面；合并后的主线代码已在 `backend_app.c` 接入 list marker 分类，用于解除旧 blocker。
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

当前通过项：`test_picoui_list`、demo boundary、public API、runtime smoke、backend mapping、list visible gate、`git diff --check`。

当前失败项：无已知 F 线 gate blocker；但 closeout review 发现公开 API 合同缺口和 mapping marker 语义缺口，不能按“已完整收口”理解。

当前 review 发现的能力缺口：

- `picoui_list_set_on_selected()` 当前只保存 callback/user_data，尚未接入 `ldList` native selection event bridge；该公开 API 当前合同未兑现，不能写成 list selection callback 已由真实输入事件触发。
- 当前 mapping marker 把 `item_wifi/item_bluetooth/item_display` 追加进 `PICOUI_BACKEND_REAL_WIDGET_IDS`，但这些 item 是 `ldListSetText()` 数据项，不是独立 LingDongGUI widget。当前只能把它们解释为 list item marker / payload marker；后续应拆成 `PICOUI_BACKEND_LIST_ITEM_IDS` 这类专用 marker，避免夸大证据。
- `picoui_list` 仍未封装 `ldList` 的 item height、text color、align、select color、item widget、padding、margin、selectable、corner 等能力。

## 当前控件完成态和能力边界

`PicoUI` 当前已完成的真实 backend 控件：

| PicoUI 控件 | LingDongGUI backend | 当前结论 |
| --- | --- | --- |
| `window` | `ldWindow` | 支持真实窗口、flex/grid 根布局、theme token v1 子集；不是完整 `ldWindow` 全 API 封装。 |
| `label` | `ldLabel` | 支持文本、字体、layout/theme 子集；不是完整 `ldLabel` 全 API 封装。 |
| `button` | `ldButton` | 支持文本、pressed/released/clicked 事件、layout/theme 子集；不是完整 `ldButton` 全 API 封装。 |
| `checkbox` | `ldCheckBox` | 支持文本、checked、toggled/native value changed、layout/theme 子集；不是完整 `ldCheckBox` 全 API 封装。 |
| `switch` | `ldSwitch` | 支持 checked、toggled/native value changed、disabled 同步、layout/theme 子集；不是完整 `ldSwitch` 全 API 封装。 |
| `slider` | `ldSlider` | 支持 value/range、native value changed、layout/theme 子集；不是完整 `ldSlider` 全 API 封装。 |
| `text` | `ldText` | 支持文本、字体、layout/theme 子集；不是完整 `ldText` 全 API 封装。 |
| `image` | `ldImage` | 支持 tile 指针 source 绑定/清空；不做资源加载，theme 明确拒绝。 |
| `list` | `ldList` | 支持 create、props、add item、selected index shadow state、真实 `ldListSetText`/`ldListSetSelectItem` backend 映射、mapping/visible gate；不支持 multi-select、virtualization、drag reorder、keyboard navigation，当前 `on_selected` 只保存 callback，未接入 native list selection event bridge。 |

因此当前不能写“PicoUI 已完成控件 100% 支持 LingDongGUI 对应控件能力”。更准确口径是：已完成控件都有真实 LingDongGUI backend 对象和可测的 PicoUI 合同子集；PicoUI 目前是稳定上层封装层，不是底层控件 API 的逐项全量镜像。

## 当前明确做什么

1. 先做 `picoui_list`。
2. 先完成最小 vertical slice，再考虑 `line_edit`。
3. 每个新增 demo 必须同步 runtime、mapping、visible matrix。
4. 每次汇报都区分 smoke、mapping、visible、manual artifact。
5. 下一步先补“能力差距矩阵”，逐项比较 PicoUI public API 与对应 `ld*` 控件公开能力，再决定哪些能力进入封装层、哪些明确 reject/deferred。
6. 优先把 list marker 语义拆清，再决定是否补 native selection event bridge。
7. 以 `G线` 为后续真相源，统一收口 capability gap、合同未兑现 API 和 gate 证据边界。

## 当前明确不做什么

1. 不实现 `line_edit`。
2. 不实现 multi-select、virtualization、drag reorder。
3. 不把 list demo 的 visible gate 通过写成所有 list 行为完成。
4. 不把 `picoui_list_basic_demo` 的 runtime smoke 通过写成 manual artifact 或人工窗口验收通过。
5. 不把 PicoUI 当前合同子集写成 LingDongGUI 对应控件的 100% 全能力封装。
