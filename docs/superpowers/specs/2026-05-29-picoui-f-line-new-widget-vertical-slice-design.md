# PicoUI F线新控件 垂直切片 设计文档

> 日期：2026-05-29
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 建议 worktree：`.worktree/picoui-f-new-widgets`
> 入口索引：`docs/picoui-serial/F-线计划索引.md`

## 1. 背景

`D线` 负责当前控件完整性和 shared quality。为了快速推进 PicoUI 能力面，可以并行开 `F线`，但 `F线` 不能抢占 `D线` 的 shared backend/layout/event/theme 写面。

`LingDongGUI` 已有多个底层控件：`ldList`、`ldLineEdit`、`ldComboBox`、`ldTable`、`ldKeyboard`、`ldArc`。直接同时推进多个新控件会拉大 public API、输入、focus、layout、theme 和 visible gate 的耦合，因此 `F线` 先做一个低耦合新控件 vertical slice。

## 2. 控件选择

第一批只实现 `picoui_list`。

理由：

- `list` 能快速提升菜单、导航、设置项列表能力。
- 底层已有 `ldList`，比 `line_edit` 更少依赖输入法、键盘、focus 和文本编辑状态机。
- 可通过 mapping marker 和 visible gate 建立明确证据。

暂缓：

- `line_edit`：价值高，但输入、focus、keyboard、光标和编辑事件耦合高，适合作为 F2。
- `combo_box/table/keyboard/arc`：暂缓，避免复杂布局或绘制语义过早扩面。

## 3. 目标

`F线` 的目标是新增 `picoui_list` 的最小完整 vertical slice：

- public header：`picoui/include/picoui/list.h`
- widget implementation：`picoui/src/widgets/list.c`
- backend mapping：`picoui/src/backend/ldgui/backend_list.c`
- demo：`picoui/demo/list_basic/main.c`
- unit test：`tests/picoui/unit/test_picoui_list.c`
- runtime gates：smoke、mapping、visible matrix 增加 list demo
- docs：demo guide、F 线索引和测试矩阵说明

## 4. 非目标

- 不修改 `backend_app.c`、`backend_layout.c`、`backend_event.c`、`backend_style_apply.c`、`backend_theme.c`。
- 不实现 `line_edit`、`combo_box`、`table`、`keyboard`、`arc`。
- 不实现复杂 list virtualization、multi-select、drag reorder、filtering。
- 不通过修改现有 demo 绕过 D 线缺口。

## 5. `picoui_list` API 边界

最小 public API：

```c
struct picoui_list;

struct picoui_list_props {
    const char *id;
    const char *style_class;
    void *user_data;
};

struct picoui_list *picoui_list_create(struct picoui_widget *parent, const char *id);
struct picoui_list *picoui_list_create_with_props(struct picoui_widget *parent,
                                                  const struct picoui_list_props *props);
int picoui_list_add_item(struct picoui_list *list, const char *id, const char *text);
int picoui_list_set_selected_index(struct picoui_list *list, int index);
int picoui_list_get_selected_index(const struct picoui_list *list);
void picoui_list_set_on_selected(struct picoui_list *list,
                                 void (*callback)(struct picoui_list *list, int index, void *user_data),
                                 void *user_data);
```

拒绝或推迟：

- item icon
- multi-select
- item remove/reorder
- virtualized list
- keyboard navigation
- text editing

## 6. Gate 要求

新增 demo 必须同步：

- `tests/picoui/runtime/check_picoui_runtime.py`
- `tests/picoui/runtime/check_picoui_visible_ui.py`
- `tests/picoui/runtime/check_picoui_backend_mapping.py`
- `picoui/docs/demo_guide.md`
- `docs/picoui-serial/F-线计划索引.md`

`picoui_list_basic_demo` 必须输出：

- `PICOUI_RUNTIME_READY`
- `PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI`
- `PICOUI_BACKEND_REAL_WIDGET_IDS=list,title,item_wifi,item_bluetooth,item_display`
- 不得输出 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`

## 7. D/F 并行边界

`F线` 可以与 `D线` 并行，但只允许新增 list 独立文件和对应 demo/gate matrix 项。若需要修改共享聚合文件，例如 `picoui/include/picoui/picoui.h`、`tests/picoui/CMakeLists.txt`、`examples/sdl/CMakeLists.txt`，必须在 F 线计划中标注，并由主线程在合并阶段处理冲突。

## 8. 验收

最小验收命令：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
ctest --test-dir build -R test_picoui_list --output-on-failure
python3 tests/picoui/contract/check_picoui_public_api.py
python3 tests/picoui/contract/check_picoui_demo_boundary.py
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
ctest --test-dir build -L picoui --output-on-failure
git diff --check
```

