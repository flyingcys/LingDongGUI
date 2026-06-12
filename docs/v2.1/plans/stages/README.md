# TinyUI v2.1 Stages

## 读法

先读：

1. `docs/v2.1/2026-06-10-tinyui-v2-1-design.md`
2. `docs/v2.1/plans/v2.1-orchestration-plan.md`
3. 本文件

然后严格按下面顺序执行阶段 plan。

## 阶段顺序

### V0 基线与 rename 守门

文件：`docs/v2.1/plans/stages/v0-baseline-and-rename-guards-plan.md`

目标：

- 冻结 `v2.0` 末态 inventory
- 给 `picoui -> tinyui` 目录/API 收口建立 machine guard
- 让后续阶段不在移动基线上作业

V0 closeout 要点：

- `v2.1` baseline inventory 已建立，后续阶段必须先回看 `docs/v2.1/2026-06-10-tinyui-v2-1-baseline-inventory.md`
- `rename/backend/api` transition guard 已作为 `v2.1` 机器真相源入口定义，不得绕开 inventory 口径各自记数
- 后续 `V1` 到 `V5` 统一服从 `v2.1` inventory，不再各自发明起跑状态

当前摘要：

- `check_tinyui_v21_transition_guards` 已注册到 CTest 并通过
- `tinyui_v21_transition_inventory.json` 已冻结当前实测基线：`backend_c_files=35`、`picoui_public_api_count=559`、`tinyui_public_api_count=34`
- `V0` 已完成；后续目录/API/backend 迁移必须先服从这组起跑真相
- 当前 `V2` 的 `backend_c_files=0` 守门已经实测通过；`check_tinyui_v21_transition_guards` 现已转为通过态，不再以 backend 目录残留作为 fail-first 预期

### V1 顶层目录与 public header 收口

文件：`docs/v2.1/plans/stages/v1-top-level-directory-and-public-header-plan.md`

目标：

- 让产品顶层目录只剩唯一 `tinyui/`
- 合并现有 `picoui/` 与试点 `tinyui/include/*`
- 收口 public include 树与入口头文件

当前摘要：

- 顶层 `picoui/` 目录已物理消失，产品层当前只剩唯一 `tinyui/`
- `tinyui/include/picoui/*` 已承接原 `picoui/include/picoui/*` public include 树，现有 `#include "picoui/..."` 在本阶段仍可编译
- `tinyui/include/*.h` 试点头已与统一产品根共存，`tinyui.h` 继续作为当前 canonical umbrella header
- `V1` focused compile proof 已通过；测试名、demo 名、target 名仍保留旧 `picoui` 命名，留待后续阶段迁移

### V2 backend 并回 widgets

文件：`docs/v2.1/plans/stages/v2-backend-to-widgets-merge-plan.md`

目标：

- 删除独立 `backend/` 目录
- 把 widget-specific backend 文件和 API 并回 `widgets/*`
- 保留真实 `LingDongGUI` binding，不保留 backend bridge

当前摘要：

- `switch` 已完成首批 widget-specific backend 行为并回；`backend_switch.c` 已被识别为纯空编译单元，不再承载任何 production shared helper，这轮只把它从编译面退场并删除
- `background` 已切到 direct-create；当前 `V2` 真相以 public create 路径为准，不再把 legacy create 当成合法入口
- `window/layout` 相关 focused unit proof 当前为绿：`test_picoui_background`、`test_picoui_window`、`test_picoui_layout`、`test_picoui_app_window_switch`、`test_picoui_switch`
- `layout` 已补上 explicit padding persistence 合同：`picoui_window_set_padding()` / `picoui_window_set_grid_padding()` 设置后的四边 padding，后续 `flex/grid` 配置更新不得回退成 uniform padding
- `label` 已补上 `create_with_props()` 失败回滚合同，确保 widget-specific setter 失败时不会把半创建 child 残留在 backend tree
- `button` 当前已补上的失败路径证据仍是有限口径：`font` setter 失败时，`test_picoui_button_events` 只证明 parent child 链不会遗留半挂载 child；`xBtnAction/action_info` 清理尚未在本批测试内形成 fresh proof，因此这里不再把它写成已证实合同
- `button` 的 widget-specific `font/image/transparent/checkable/key_value/pressed` 行为已并回 `tinyui/src/widgets/button.c`；对应纯 disabled-stub `backend_button.c` 已从 CMake 退编并删除
- `window` 的 widget-specific `background_source/background_offset/bg_color/padding_group/getters` 行为已并回 `tinyui/src/widgets/window.c`；对应纯 disabled-stub `backend_window.c` 已从 CMake 退编并删除
- `window` 已补上 root `create_with_props()` 失败回滚合同：`bg_color` setter 失败时，不得在 `ld_scene->ptNodeRoot` 残留 root node
- `checkbox` 的 create 路径与 widget-specific `check_color/text_color/unchecked_source/checked_source/radio_group/string_left_space` 行为现已全部并回 `tinyui/src/widgets/checkbox.c`；对应 `backend_checkbox.c` 已不再承载 production shared helper，可从编译面退场并删除
- `checkbox` 当前 focused proof 已补到 native helper 行为：除 direct-create / props happy path / checked-text round-trip 外，还直接断言 `fgColor/textColor/image tiles/radioButtonGroup/isRadioButton/boxWidth` 与非法参数拒绝；但 `create_with_props()` 失败回滚尚未在 fresh test 中坐实，因此这里不再把它写成已证实合同
- `checkbox` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_checkbox`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `slider` 的 create 路径与 widget-specific `horizontal/background_source/indicator_source/image/color/indicator_width/slim_size/get_percent` 行为已并回 `tinyui/src/widgets/slider.c`；对应纯 disabled-stub `backend_slider.c` 已从 CMake 退编并删除
- `slider` 已补上更强的 `create_with_props()` 失败回滚合同：按 `id` 注入 `indicator_width` 失败后，不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `slider` 专项 direct-create tree/binding proof 已补齐：`owner/root/parent/name_id/ld_widget/host_widget` 与 LD event bridge 当前均有专门测试护栏
- `slider` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_slider`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `progress_bar` 的 create 路径与 widget-specific `percent/horizontal/image/bg_source/fg_source/frame_source/color/frame_color/inverted` 行为已并回 `tinyui/src/widgets/progress_bar.c`
- `progress_bar` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `progress_bar` 专项 direct-create tree/binding proof 已补齐：`kind/owner/root/parent/ld_name_id/host_widget/ld_event_bridge/ldBase.pInfo` 当前均有专门测试护栏
- `progress_bar` 这批低风险声明面收口已完成：`backend.h` 里的 legacy `picoui_backend_create_progress_bar()` 声明已删除；对应 unit test 不再把 disabled stub 当成当前 `V2` 合法入口，继续只守 direct-create tree/binding 与 rollback 真相
- `progress_bar` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_progress_bar`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `progress_wheel` 的 widget-local create 路径与 widget-specific `percent/progress alias/wheel_color/dot_color/dot_enabled/get_percent` 行为已稳定落在 `tinyui/src/widgets/progress_wheel.c`；本轮进一步把 `dirty-region` helper 与 rollback test seam 也并回 widget 文件
- `progress_wheel` 已补上 direct-create tree/binding proof：`kind/owner/root/parent/ld_name_id/host_widget/ld_event_bridge/ldBase.pInfo` 当前均有专门测试护栏
- `progress_wheel` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `progress_wheel` 这批已完成 `V2` 当前收口：`backend_progress_wheel.c` 不再需要保留，helper-only 真实边界已消失；对应 compile-surface cleanup 以 widget 文件内 test seam + public-path rollback proof 为准
- `progress_wheel` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_progress_wheel`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `arc` 的 create 路径与 widget-specific `background/foreground/rotation angle`、`quarter_source`、`parent_color`、`bg/fg color` 行为已并回 `tinyui/src/widgets/arc.c`；对应纯 disabled-stub `backend_arc.c` 已从 CMake 退编并删除
- `arc` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `arc` 专项 direct-create tree/binding proof 已补齐：`kind/owner/root/parent/ld_name_id/host_widget/ld_event_bridge/ldBase.pInfo` 当前均有专门测试护栏
- `arc` 颜色读回合同已锁定为 production `rgb565 -> rgb888` round-trip 真相，不允许回退到自等式或弱断言
- `arc` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_arc`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `gauge` 的 create 路径与 widget-specific `angle/bg_source/pointer_source/centre_offset/pointer_color/auto_move/trail/progress_bar` 行为已并回 `tinyui/src/widgets/gauge.c`；对应纯 disabled-stub `backend_gauge.c` 已从 CMake 退编并删除
- `gauge` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `gauge` 专项 direct-create tree/binding proof 已补齐：`kind/owner/root/parent/ld_name_id/host_widget/ld_event_bridge/ldBase.pInfo` 当前均有专门测试护栏
- `gauge` 的 `pointer_color` 读回合同已锁定为 production `rgb565 -> rgb888` round-trip 真相，不允许回退到 `!= 0` 之类弱断言
- `gauge` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_gauge`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `window/label/button/checkbox/slider/arc/gauge` 这批 low-risk 声明面收口与 compile-surface cleanup 已完成：`backend.h` 中 `picoui_backend_create_window`、`picoui_backend_create_child_window`、`picoui_backend_create_background`、`picoui_backend_create_label`、`picoui_backend_create_button`、`picoui_backend_create_checkbox`、`picoui_backend_create_slider`、`picoui_backend_create_arc`、`picoui_backend_create_gauge` 声明已删除
- 对应 unit test 不再把 disabled stub 当成当前 `V2` 合法入口，统一只守 public/direct-create tree/binding、props-stage rollback 与 widget-local 真相
- 本批 compile-surface cleanup 已完成：`backend_window.c`、`backend_button.c`、`backend_slider.c`、`backend_arc.c`、`backend_gauge.c` 这 5 个纯 disabled-stub backend 文件已从 `cmake/LingDongGUI.cmake` 退编并删除
- `backend_label.c` 仍保留在编译面，因为它当前仍承载 shared helper；`backend_checkbox.c` 则已完成进一步收口：legacy `picoui_backend_create_checkbox()` 与 widget-specific helper 实现均已迁离 backend 路径，因此 `backend_checkbox.c` 已从 CMake 退编并删除
- `image` 的 create 路径与 widget-specific `source/mask_color` 行为已并回 `tinyui/src/widgets/image.c`
- `image` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `image` 的 rollback test seam 已从 production 主实现剥离到 `tests/support/picoui_test_support.c`，不再把 fail switch 和 snapshot 状态编进 `widgets/image.c`
- `image` 这批低风险声明面收口已完成：`backend.h` 里的 legacy `picoui_backend_create_image()` 声明已删除；对应 unit test 不再把 disabled stub 当成当前 `V2` 合法入口，继续只守 direct-create tree/binding 与 rollback 真相
- `image` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_image`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `qrcode` 的 create 路径与 widget-specific `text/qr_color/bg_color/ecc/max_version/zoom` 行为已并回 `tinyui/src/widgets/qrcode.c`
- `qrcode` 已补上 props-stage `create_with_props()` 失败回滚合同：失败后不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `qrcode` 的 rollback test seam 已从 production 主实现剥离到 `tests/support/picoui_test_support.c`，不再把 fail switch 和 snapshot 状态编进 `widgets/qrcode.c`
- `qrcode` 这批低风险声明面收口已完成：`backend.h` 里的 legacy `picoui_backend_create_qrcode()` 声明已删除；对应 unit test 不再把 disabled stub 当成当前 `V2` 合法入口，继续只守 direct-create tree/binding 与 rollback 真相
- `qrcode` 当前 focused/runtime proof 已 fresh 通过：`test_picoui_qrcode`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- 在当前 `cmake/LingDongGUI.cmake` 已迁到 `tinyui/*` 路径的工作树真相下，`image + qrcode + progress_bar` 这一批 compile-surface cleanup 已进一步完成：`LD_PICOUI_BACKEND_LDGUI_SOURCES` 已移除 `backend_image.c`、`backend_qrcode.c`、`backend_progress_bar.c` 这 3 个纯 disabled-stub backend 条目，且对应文件已删除
- `progress_wheel` 当前不再保留 helper-only backend 编译面；`backend_progress_wheel.c` 已退出 `V2` 当前真相，focused proof 继续只守 widget-local helper、public create tree/binding 与 rollback 合同
- `test_picoui_widgets` 已改为通过 public create 验证 root/child widget 的 direct-create tree/binding 真相，不再依赖任何已移除的 legacy create 声明
- `calendar/combo_box/line_edit` 这一个低风险批次已完成当前收口：`backend_calendar.c`、`backend_combo_box.c`、`backend_line_edit.c` 不再由 CMake 编译，`backend.h` 里的对应 legacy constructor 声明也已删除
- `test_picoui_calendar`、`test_picoui_widgets`、`test_picoui_line_edit` 已切到 `V2` 当前真相：通过 public widget create 验证 widget-local backend tree/binding，而不是继续把 legacy constructor 当成合法入口
- `text/canvas/date_time` 这一批已完成 public create 收口：`tinyui/src/widgets/{text,canvas,date_time}.c` 现在直接完成 backend child 建树与 `host_widget` 绑定，不再经由 `picoui_backend_create_text/canvas/date_time()`
- 其中 `canvas` 已进一步把 `picoui_backend_canvas_sync()` 的 native push/clear 逻辑并回 `tinyui/src/widgets/canvas.c`，`backend.h` 中对应 helper 声明已删除，`backend_canvas.c` 也已从 CMake 编译面退场并删除；`date_time` 这轮也已把 `format/date/time/text_color/bg_color/align/transparent/use_system_time/get_*` 真相并回 `tinyui/src/widgets/date_time.c`，`backend.h` 中对应 helper 声明已删除，`backend_date_time.c` 已从 CMake 编译面退场并删除；本轮 `text` 也已进一步完成 widget-specific helper 收口：`font/static_text/transparent/text_color/bg_color/background_source/scroll_*` 真相已并回 `tinyui/src/widgets/text.c`，`backend.h` 中对应 helper 声明已删除，`backend_text.c` 已从 CMake 编译面退场并删除
- 本批 focused unit proof 应以 `test_picoui_text`、`test_picoui_canvas`、`test_picoui_date_time` 为准，重点守住 `parent/root/owner/host_widget/ld_widget` 的 direct-create tree/binding 合同；其中 `canvas` 还继续锁定 native command push/clear 与 text snapshot 回归点，`date_time` 这轮则补强了 public/native readback、transparent/bg_color 交互，以及 corrupted backend binding 负向合同
- `list/scroll_selecter` 这一批已完成 public create 收口：`tinyui/src/widgets/{list,scroll_selecter}.c` 现在直接完成 backend child 建树、attach 与 `host_widget` 绑定，不再经由独立 legacy create 路径
- `backend.h` 里 `picoui_backend_create_list()`、`picoui_backend_create_scroll_selecter()` 声明已删除；对应 unit test 已切到 `V2` 当前真相：通过 public widget create 验证 direct-create tree/binding，同时显式断言 legacy constructor 符号已不存在
- `backend_list.c` 已完成当前收口：原先剩余的 `selected_index/get_selected_index/sync_selected_index` 兼容 helper 现已并入 `tinyui/src/widgets/list.c`，不再继续挂在 `backend/` 路径；对应 `backend_list.c` 已从 CMake 退编并删除
- `backend_scroll_selecter.c` 则已完成进一步收口：在确认 legacy create 符号 impact 为 `LOW`、`direct callers = 0` 后，这个只剩 disabled create 的空壳文件已从 CMake 退编并删除；scroll_selecter 的 setter/getter/sync helper 真相现已全部并入 `tinyui/src/widgets/scroll_selecter.c`
- 本批 focused unit proof 应以 `test_picoui_list`、`test_picoui_scroll_selecter` 为准，重点守住 `parent/root/owner/host_widget/ld_widget/ldBase.pInfo` 的 direct-create tree/binding 合同；其中 `list` 继续锁定 migrated helper 漏网调用必须 fail-closed、selected-index readback/sync 继续遵守 widget-local/native 真相，且 legacy `picoui_backend_list_*` selected-index 符号已从测试二进制中消失，`scroll_selecter` 则继续锁定 `set_items()` 后 native/public selected state 同步重置
- `animation/graph/clock` 这一批已完成 public create 收口：`tinyui/src/widgets/{animation,graph,clock}.c` 现在直接完成 backend child 建树、attach 与 `host_widget` 绑定，不再经由 `picoui_backend_create_animation/graph/clock()`
- `animation` 当前也已进一步把 `source/period_ms/show_frame` helper 真相并回 `tinyui/src/widgets/animation.c`，`backend.h` 中这 3 个 helper 声明已删除，`backend_animation.c` 已从 CMake 编译面退场删除；`graph` 这轮也已把 `axis/axis_offset/frame_space/grid_offset/point_mask/add_series/set_value/move_add/get_*` 真相并回 `tinyui/src/widgets/graph.c`，`backend.h` 中对应 helper 声明已删除，`backend_graph.c` 已从 CMake 编译面退场删除；`clock` 则更早已把 `background/pointer/time` 等 helper 全部并回 widget，并完成 `backend_clock.c` 退编删除
- 本批 focused unit proof 应以 `test_picoui_animation`、`test_picoui_graph`、`test_picoui_clock` 为准，重点守住 `parent/root/owner/host_widget/ld_widget/ldBase.pInfo` 的 direct-create tree/binding 合同；其中 `animation` 继续保留轻量 `create/init` 语义，native attach 已并回 `create_with_props()`，这轮又补强了 init 顺序与跨行 sprite sheet frame 定位合同；`graph` 这轮则补强了 corrupted backend binding 下 host/native 双侧不漂移，以及 `point_mask width > frame_space` 的合法 native 自适应合同；`clock` 则已补强 `use_system_time/step_second/background/pointer/mask/anchor` 的 native 状态断言
- `message_box/keyboard/table` 这一批已完成 public create 收口：`tinyui/src/widgets/{message_box,keyboard,table}.c` 现在直接完成 backend child 建树、attach 与 `host_widget` 绑定，不再经由 `picoui_backend_create_message_box/keyboard/table()`
- `message_box` 这轮已进一步完成 widget-specific helper 收口：`title/message/confirm_text/buttons/string_colors/button_colors/bg_color/callback bridge` 真相已并回 `tinyui/src/widgets/message_box.c`，`backend.h` 中对应 helper 声明已删除，`backend_message_box.c` 已从 CMake 编译面退场并删除
- `keyboard/table` 这一批当前都已完成 compile-surface cleanup：`backend_keyboard.c` 更早已退场；本轮 `backend_table.c` 也已从 CMake 退编并删除，`table` 不再保留独立 helper-only backend 文件
- 本批 focused unit proof 应以 `test_picoui_message_box`、`test_picoui_keyboard`、`test_picoui_table` 为准；其中 `message_box` 重点守住 `parent/root/owner/host_widget/ld_widget/ldBase.pInfo` 的 direct-create tree/binding 合同，以及 widget 文件直接驱动 native title/message/buttons/colors/callback bridge 的真相；`table` 当前 focused proof 已覆盖 `current_cell sync / editable commit-cancel / image-button / static_text / keyboard binding / align-grid / region getter` 等真实合同，且新增锁定 `picoui_backend_table_bind_host()` 不再以 backend 符号形式暴露
- `table` 这轮已完成 compile-surface cleanup：原先 `backend_table.c` 中承载过的 `picoui_backend_table_bind_host()`、`native_slot`、`set/get_keyboard_binding()`、`set_selected_cell()`、`set_current_cell()`、`set/get_cell_text()`、`set_cell_editable()`、`set/get_item_align()`、`get_item_editable()`、`get_item_region()`、`set_excel_type()`、`set_background_color()`、`set_item_static_text()`、`set_item_font()`、`set_item_width()`、`set_item_height()`、`set_item_color()`、`set_item_image()`、`set_item_button()`、`navigate()` 与 `sync_current_cell()` 已全部并回 `tinyui/src/widgets/table.c` 的 widget-local/native 路径；`backend_table.c` 已不再保留任何 production retained seam
- `table` 这轮补上了 `create_with_props()` 的 props-stage 失败回滚合同：若 `keyboard_binding` 在 props 应用阶段失败，必须回滚已 attach 的 backend child，不得残留 `parent/owner/root/host_widget`、event bridge、`ldBase.pInfo` 或 sibling 链脏状态
- `radial_menu/icon_slider/switch` 这一批已完成 public create 收口：`tinyui/src/widgets/{radial_menu,icon_slider,switch}.c` 现在直接完成 backend child 建树、attach 与 `host_widget` 绑定；其中 `switch` 也已收掉 widget 文件内自留的 legacy `picoui_backend_create_switch()` 边界
- `backend_switch.c` 已确认只是空编译单元，不再承载 shared helper，因此已从 `cmake/LingDongGUI.cmake` 退编并删除；`icon_slider/radial_menu` 这轮也已进一步完成 widget-specific helper 收口：`item add/select/click/native bind`、`horizontal/speed` 等逻辑已全部并回 `tinyui/src/widgets/{icon_slider,radial_menu}.c`，`backend.h` 中对应 helper 声明已删除，`backend_icon_slider.c`、`backend_radial_menu.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除
- 本批 focused unit proof 应以 `test_picoui_radial_menu`、`test_picoui_icon_slider`、`test_picoui_switch` 为准，重点守住 `parent/root/owner/host_widget/ld_widget/ldBase.pInfo` 的 direct-create tree/binding 合同
- `label/text` 这轮已进一步完成 shared helper 收口：原先挂在 `backend_label.c` 的 `set_text/style_class/user_data` shared bridge 现已并回 `tinyui/src/core/widget.c`，`label` 仅剩的 `font` bridge 也已并回 `tinyui/src/widgets/label.c`；`backend_text.c` 与 `backend_label.c` 均已从 `cmake/LingDongGUI.cmake` 退编并删除
- 本批 focused unit proof 当前应以 `test_picoui_label`、`test_picoui_text`、`test_picoui_widgets` 为准；其中 `label/text` 都已锁定 `create_with_props()` 失败回滚不得在 parent child 链遗留半挂载 backend child，`text` 还继续锁定 runtime font rebind 与 fail-next seam 的原子性合同
- `progress_wheel` 这轮也已进一步完成 widget-specific helper 收口：`percent/get_percent/wheel_color/dot_color/dot_enabled/dirty-region disable` 与 rollback test seam 真相现已并回 `tinyui/src/widgets/progress_wheel.c`；`backend_progress_wheel.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除，`backend.h` 中对应 production helper 声明已删除
- 本批 focused unit proof 当前应以 `test_picoui_progress_wheel` 为准；重点锁定 `create_with_props()` 失败回滚不得在 parent child 链遗留半挂载 backend child，同时保持 native `iProgress`、wheel/dot color、dot enabled 和 dirty-region 合同不回退
- `layout` 当前 focused/runtime proof 已恢复并 fresh 通过：`test_picoui_layout`、`test_picoui_widgets`、`check_picoui_visible_ui`、`check_picoui_backend_mapping`
- `layout` 已锁定新的持久化与失败语义合同：
  - 显式 `flex/grid` padding 后续不得被 layout 更新覆盖
  - `padding_group` 后续 `flex/grid` 更新后仍必须保持生效
  - grid template 缩短时，`ldWindow` descriptor tail 必须正确清尾
  - native binding 缺失时，layout setter 必须拒绝且不得污染 public state
- `layout` 这轮又把 `window padding trio` 从 `backend_layout.c` 的 production surface 收回 `widgets/core` 路径：`picoui_widget_set_padding()` 里的 window/background 分支现在直接走 `tinyui/src/widgets/window.c` 的 uniform-padding apply，`picoui_window_set_padding()` / `picoui_window_set_grid_padding()` 也直接完成 native apply 与 backend cache 同步；`test_picoui_layout` 当前直接锁定 `picoui_backend_widget_set_padding()`、`picoui_backend_window_set_padding()`、`picoui_backend_window_set_grid_padding()` 不得再出现在 `libpicoui_backend_ldgui.a`
- `layout` 这轮又把 `flex quartet` 从 `backend_layout.c` 的 production surface 收回 `tinyui/src/widgets/window.c`：`layout_type/flex_flow/flex_align/flex_gap` 现已直接完成 native apply、public state 与 backend cache 同步，`test_picoui_layout` 当前直接锁定 `picoui_backend_window_set_layout_type()`、`picoui_backend_window_set_flex_flow()`、`picoui_backend_window_set_flex_align()`、`picoui_backend_window_set_flex_gap()` 不得再出现在 `libpicoui_backend_ldgui.a`
- `layout` 这轮又把 `grid quartet` 从 `backend_layout.c` 的 production surface 收回 `tinyui/src/widgets/window.c`：`grid_columns/grid_rows/grid_gap/grid_align` 现已直接完成 public state、backend cache、grid descriptor mapping 与 native `ldWindow` 同步，`test_picoui_layout` 当前直接锁定 `picoui_backend_window_set_grid_columns()`、`picoui_backend_window_set_grid_rows()`、`picoui_backend_window_set_grid_gap()`、`picoui_backend_window_set_grid_align()` 不得再出现在 `libpicoui_backend_ldgui.a`
- `layout` 这轮又把最后的 `generic gap tail` 和残余 shared helper 真相从 `backend_layout.c` 收掉：`picoui_window_set_gap()` 现已直接走 `tinyui/src/widgets/window.c` 的 layout-neutral gap apply，保持 `layoutTpye` 不漂移，同时同步 public state、backend cache 与 native `flexItemGap/flexTrackGap`；`picoui_native_align_to_ld_grid()` 已迁到 `tinyui/src/core/native.c`；`test_picoui_layout` 当前新增锁定 `picoui_backend_window_set_gap()` 与 `backend_layout.c.o` 都不得再出现在 `libpicoui_backend_ldgui.a`
- `layout` 这轮已把 4 个 child-layout helper 从 `backend_layout.c` 的 production surface 收回 `tinyui/src/core/widget.c`：`picoui_widget_set_flex_grow()`、`picoui_widget_set_flex_new_track()`、`picoui_widget_set_ignore_layout()`、`picoui_widget_set_grid_cell()` 现在直接完成 native apply 与 child-layout cache 同步；`test_picoui_layout` 当前直接锁定这 4 个 legacy backend 符号不得再出现在 `libpicoui_backend_ldgui.a` 或测试二进制中
- `layout` 新增 corrupted-binding 负向合同：child layout setter 在 backend `ld_widget` 非空但 native `widgetType` 与 backend `kind` 不一致时，必须 `fail-closed` 返回 `-1`，不得污染既有 public `ignore_layout` 或 native `ignoreLayout` 状态
- `layout` 这轮又补上 window padding/grid padding 的 corrupted-binding 负向合同：`picoui_window_set_padding()` / `picoui_window_set_grid_padding()` 在 backend `ld_widget` 非空但 native `widgetType` 损坏时，必须 `fail-closed` 返回 `-1`，且不得提前污染 backend cache 或 native `flexPadding/gridPadding` 状态
- `layout` 这轮又补上 grid columns/rows/gap/align 的 corrupted-binding 负向合同：`picoui_grid_set_columns()` / `picoui_grid_set_rows()` / `picoui_grid_set_gap()` / `picoui_grid_set_align()` 在 backend `ld_widget` 非空但 native `widgetType` 损坏时，必须 `fail-closed` 返回 `-1`，且不得污染 backend cache 或 native `ldWindow` grid descriptor/gap/align 状态
- `list` 这一小步已进一步完成 compile-surface cleanup：原先残留在 `backend_list.c` 的 `selected_index/get_selected_index/sync_selected_index` 兼容 helper 现已迁到 `tinyui/src/widgets/list.c` 的 widget-local hidden helper，不再保留独立 backend 文件
- `backend_label.c` 这批 shared helper 已完成迁离 `backend/` 路径并清零退场；`layout/keyboard/table` 仍承载实逻辑，当前仍不是低风险 compile-surface cleanup 候选
- `list` 这轮 focused proof 已把 selected-index 兼容链真相改写为 widget-local/native readback：`backend_event.c` 的 list selection restore/native dispatch 现已改走 `tinyui/src/widgets/list.c` 内部 helper，不再依赖独立 `backend_list.c`
- `list` 新增 closeout 证据：`test_picoui_list` 当前直接扫描测试二进制符号表，锁定 legacy `picoui_backend_list_set_selected_index/get_selected_index/sync_selected_index` 不得再以 backend 符号形式出现；同时 corrupted-binding 负向合同继续要求 selected-index sync 在 `kind/ld_widget` 绑定损坏时返回 `-1`，不得污染 `list->selected_index` / `backend->value` 缓存
- `keyboard` 新增 corrupted-binding 负向合同：`picoui_backend_keyboard_button_update()` 在 `kind/ld_widget` 绑定损坏时必须返回 `-1`，不得把损坏绑定误判成合法 keyboard，也不得污染 native `keyCode/isKeySelect` 或 callback 计数
- `keyboard` 这轮又进一步完成最小 shared/widget 边界收口：`picoui_backend_keyboard_click()` 与 `picoui_backend_keyboard_exit()` 已并回 `tinyui/src/widgets/keyboard.c` 的 widget-local retained seam；`test_picoui_keyboard` 当前直接锁定这两个 legacy backend 符号不再以 public/backend 形式暴露
- `keyboard` 这轮又进一步完成最小 shared/widget 边界收口：`picoui_backend_keyboard_update()` 与 `picoui_backend_keyboard_button_update()` 也已并回 `tinyui/src/widgets/keyboard.c` 的 widget-local retained seam；`test_picoui_keyboard` 当前继续直接锁定这两个 legacy backend 符号不再以 public/backend 形式暴露，同时保持 native `pBtnList/isWaitInit/keyCode/isKeySelect` 与 callback 合同不回退
- `keyboard` 这轮已完成 compile-surface cleanup：剩余的 `input_ascii/navigate` 也已并回 `tinyui/src/widgets/keyboard.c` 的 widget-local/native 路径，`backend_keyboard.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除；`test_picoui_keyboard` 当前继续直接锁定 legacy `picoui_backend_keyboard_input_ascii()` / `picoui_backend_keyboard_navigate()` 不得再以 public/backend 形式暴露
- `table` 的 corrupted-binding、current-cell readback、navigate、rollback 与 archive symbol absence 合同当前都已继续保留在 `test_picoui_table`，且新增锁定 `libpicoui_backend_ldgui.a` 不得再包含 `backend_table.c.o`
- `table` 当前已不再是 `V2` 的 retained-boundary 候选；它的 widget-specific backend seam 已清零退场，`V2` 最后一批已落到 SDL host runtime compile-surface cleanup
- `backend_layout.c` 这轮已完成 compile-surface cleanup：`padding trio`、`flex quartet`、`grid quartet`、`generic gap tail` 与 child-layout quartet 都已迁离 production surface，残余 shared helper `picoui_native_align_to_ld_grid()` 也已迁到 `tinyui/src/core/native.c`；`cmake/LingDongGUI.cmake` 已将 `backend_layout.c` 退编并删除文件本体，`test_picoui_layout` 当前继续锁定 `backend_layout.c.o` 不得再出现在 `libpicoui_backend_ldgui.a`
- `backend_event.c` 当前已继续完成 shared-helper 外层剥离：`picoui_backend_emit_value_changed()`、`picoui_backend_emit_event()`、`picoui_backend_emit_clicked()` 已迁到 `tinyui/src/core/widget.c`，`picoui_widget_dispatch_signal()` / `picoui_widget_dispatch_event()`、`picoui_widget_sync_ld_value()`、`picoui_widget_emit_ld_event_bridge()` 已迁到 `tinyui/src/core/event.c`；legacy `picoui_backend_sync_ld_value()` / `picoui_backend_emit_ld_event_bridge()` 不再继续以 backend 符号形式暴露
- `backend_event.c` 这轮又进一步完成 shared setter-helper 收口：`picoui_backend_widget_update_value()` 已迁到 `tinyui/src/core/widget.c` 的 shared/internal 入口 `picoui_widget_update_value()`；`checkbox/switch/slider` 的 setter/data-model 路径现已不再依赖 backend 符号，`test_picoui_widgets` 继续直接锁定 legacy `picoui_backend_widget_update_value()` 不得再出现在测试二进制符号表
- `backend_event.c` 这轮又进一步完成 shared binding 收口：`picoui_backend_widget_bind_host()` 与 `picoui_backend_widget_bind_ld_event_bridge()` 已迁到 `tinyui/src/core/runtime_bridge.c`，不再继续挂在 `backend/` 路径
- `backend_event.c` 这轮又完成 native dispatch 主链收口：`picoui_backend_widget_dispatch_native_signal()` 与 rejected-list-selection restore 已迁到 `tinyui/src/core/event.c`，shared/internal 入口统一命名为 `picoui_widget_dispatch_native_signal()`；`tinyui/src/core/runtime_bridge.c` 中的 `ldMsg` bridge slot 现已直接调 shared/core 路径，不再依赖 backend event router
- `test_picoui_widgets` / `test_picoui_keyboard` 当前继续直接锁定 legacy `picoui_backend_widget_dispatch_native_signal()` 与 `backend_event.c.o` 不得再出现在测试二进制或 `libpicoui_backend_ldgui.a`；同时新增 list hidden/disabled reject 路径合同，要求 native dispatch 回退 selected-index 时不得污染 native/current cache，也不得误发业务 callback
- 本批 focused proof 继续以 `test_picoui_widgets`、`test_picoui_button_events` 为准；新增合同锁定 `picoui_backend_widget_bind_ld_event_bridge()` 在 native `ld_widget` 缺失时必须 `fail-closed` 返回 `-1`，且不得脏写 `ld_event_bridge_scene/sender`；同时测试二进制符号表继续锁定 legacy `picoui_backend_sync_ld_value()` / `picoui_backend_emit_ld_event_bridge()` 不得再出现
- `test_picoui_button_events` 当前还直接扫描测试二进制符号表，锁定 legacy `picoui_backend_widget_dispatch_signal()` / `picoui_backend_widget_dispatch_event()` 不得再以 backend 符号形式出现；同时保持 pressed/released/value-changed 的 callback、focus claim、dispatch_count 与 invisible/disabled fail-closed 合同不回退
- `backend_event.c` 当前已完成 compile-surface cleanup：shared dispatch/setter/binding/native-dispatch 真相已分别并回 `tinyui/src/core/widget.c`、`tinyui/src/core/event.c`、`tinyui/src/core/runtime_bridge.c`，`cmake/LingDongGUI.cmake` 已将 `backend_event.c` 退编并删除文件本体
- `backend_app.c` 相关 shared seam 已在前序批次全部收口：timer 迁到 `tinyui/src/core/app.c`，pointer bridge/outer wrapper/init/ensure-window 迁到 `tinyui/src/core/runtime_bridge.c`，legacy `picoui_backend_runtime_step()` 也已清零
- `backend_app.c` 文件本体已在本批从 `tinyui/src/backend/ldgui/backend_app.c` 迁到 `tinyui/src/core/runtime_host.c`，继续承载 SDL host runtime、visible smoke marker、capture 写图与 internal `picoui_backend_step_app()` 实现，但它已不再占用 `backend/*.c` compile surface
- `picoui_backend_ldgui` 与 `picoui_backend_ldgui_runtime` 当前都编译 `tinyui/src/core/runtime_host.c`；`tests/picoui/runtime/check_picoui_runtime.py` 的 screen-define 真相源也已同步到 `runtime_host.c`
- `test_picoui_app_lifecycle`、`test_picoui_native_bridge` 当前已改守 `runtime_host.c` / `runtime_bridge.c` 的新边界；`check_tinyui_v21_transition_guards.py` 现已通过，实测真相是 `backend_c_files=0`
- `backend_widget.c` 这批 shared helper 已清零退场：原先残留的 `picoui_native_nav_dir_to_ld()`、`picoui_backend_widget_init_data_model()`、`picoui_backend_widget_claim_focus()`、`picoui_backend_widget_release_focus()` 现已统一迁到 `tinyui/src/core/widget.c`；`cmake/LingDongGUI.cmake` 已将 `backend_widget.c` 退编并删除文件本体
- `theme` 这一步已完成 shared bridge 收口：`picoui_app_set_theme()` 不再经由 `backend_theme.c` 薄包装，而是直接调用 `tinyui/src/core/runtime_bridge.c` 中的 `picoui_runtime_bridge_bind_theme()`；对应空壳 `backend_theme.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除
- `theme` 这一步也已完成 shared style apply 收口：`picoui_theme_apply_to_widget()` 现已在 `tinyui/src/theme/theme.c` 内直接完成 `window/label/text/button/checkbox/switch/slider/list/image/calendar` 的 native style dispatch，不再经由 `backend_style_apply.c`
- `backend_style_apply.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除；对应 retained/shared style 边界现已收敛到 `theme.c`，不再保留独立 backend style apply 层
- `backend_widget_tree.c` 这一步也已完成 shared helper 收口：`picoui_backend_widget_is_kind()` 已并入 `tinyui/src/core/widget.c`，`picoui_backend_widget_unbind_host()` / `picoui_backend_widget_detach_from_parent()` 已并入 `tinyui/src/core/runtime_bridge.c`；对应 `backend_widget_tree.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除
- 本批 focused proof 当前以 `test_picoui_widgets`、`test_picoui_image`、`test_picoui_qrcode` 为准：继续锁定 `backend_widget_tree` 这 3 个 shared helper 在 null 输入下必须 fail-closed，且 image/qrcode rollback 路径继续通过 shared helper 完成 detach/unbind，不回退成 backend 路径私有实现
- 当前 backend 目录下独立编译面已清零：`backend_c_files=0`
- 但独立 `backend_*.c` 文件总数仍未清零，所以 `V2` 仍在进行中，未 closeout

### V3 shared layer 命名与 include path 收口

文件：`docs/v2.1/plans/stages/v3-shared-layer-rename-plan.md`

目标：

- 统一 `core/display/indev/layout/theme/tick/osal` 的产品层命名
- 收口内部 include path、target、路径引用
- 保持共享层薄化，而不是把共享逻辑塞进 widgets

当前摘要：

- `V3` 已启动，当前先完成了最小安全第一批：shared 源码层的 include path 收口
- `tinyui/src/{core,display,indev,layout,theme,tick,osal}` 现在已不再直接 `#include "picoui/..."`；对应 `V3 Step 1` 的 fail-first path scan 已从命中转为空
- 为避免把 `V3` 和 `V4` 混做，这一批只新增了 `tinyui/include/*.h` 顶层 wrapper 入口，并把 shared 源码改走这些统一入口；尚未改 public API 前缀、CTest 名称、`tests/picoui/*` 路径或 `check_picoui_*` / `test_picoui_*` 命名
- `V3 Step 4` 的第一轮 focused compile proof 当前已 fresh 通过：`test_picoui_runtime_model`、`test_picoui_layout`、`test_picoui_theme`、`test_picoui_native_bridge`
- `V3 Step 3` 当前又完成了第一批 shared symbol naming：`tinyui/src/core/runtime_bridge.{c,h}` 及其 shared/core 直系调用点中的 `picoui_runtime_bridge_*` internal/shared 接口现已统一收口为 `tinyui_runtime_bridge_*`
- 这批只改 internal/shared 接口及其直系源码/测试符号引用，不改 public `picoui_*` API，不改 `tests/picoui/*` 路径、`test_picoui_*` / `check_picoui_*` 文件名或 CTest label，明确不提前混入 `V4`
- `V3 Step 3` 当前又完成了第二批 internal-only helper naming：`window_apply_*` 这组 shared layout helper 现已从 `picoui_window_apply_*` 统一收口为 `tinyui_window_apply_*`，范围只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c)、[tinyui/src/layout/flex.c](/Users/cys/embedded/LingDongGUI/tinyui/src/layout/flex.c)、[tinyui/src/layout/grid.c](/Users/cys/embedded/LingDongGUI/tinyui/src/layout/grid.c)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c) 与 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h) 之间的 internal/shared 调用链
- `V3 Step 3` 当前又完成了第三批 internal-only helper naming：`native_*` 这组 shared conversion helper 已从 `picoui_native_align_to_ld_grid()`、`picoui_native_nav_dir_to_ld()`、`picoui_native_signal_to_ld()`、`picoui_native_readback_policy_to_backend()` 统一收口为 `tinyui_native_*`，范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/native.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/native.c)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c)、[tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c)、[tinyui/src/widgets/switch.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/switch.c)、[tinyui/src/widgets/table.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/table.c) 与 [tests/picoui/unit/test_picoui_native_bridge.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_native_bridge.c) 的 internal/shared 调用链；这一步仍未触碰 public `picoui_*` API、`tests/picoui/*` 路径、`test_picoui_*` / `check_picoui_*` 文件名或 CTest label
- `V3 Step 3` 当前又完成了第四批 internal-only helper naming：`display/indev` 的文件内静态 helper 已从 `g_picoui_default_display_config`、`picoui_display_config_is_valid()`、`picoui_display_resolve_config()`、`picoui_input_key_is_valid()` 收口到 `tinyui_*` 命名；范围只覆盖 [tinyui/src/display/display.c](/Users/cys/embedded/LingDongGUI/tinyui/src/display/display.c)、[tinyui/src/indev/indev.c](/Users/cys/embedded/LingDongGUI/tinyui/src/indev/indev.c) 及对应 focused proof [tests/picoui/unit/test_picoui_port_display.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_port_display.c)、[tests/picoui/unit/test_picoui_port_input.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_port_input.c)；public `picoui_display_*` / `picoui_input_*` API 保持不变
- `V3 Step 3` 当前又完成了第五批 internal-only bridge naming：`runtime_bridge` 里的 4 个 shared/internal seam 已从 `picoui_backend_widget_bind_host()`、`picoui_backend_widget_bind_ld_event_bridge()`、`picoui_backend_widget_unbind_host()`、`picoui_backend_widget_detach_from_parent()` 统一收口为 `tinyui_runtime_bridge_*`；范围覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/runtime_bridge.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_bridge.c)、`widgets/*` 直系调用点，以及 [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)、[tests/picoui/unit/test_picoui_image.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_image.c)、[tests/picoui/unit/test_picoui_qrcode.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_qrcode.c) 的 internal/shared 符号引用；这一步仍未触碰 public widget API、`tests/picoui/*` 路径、`test/check` 文件名或 CTest label
- `V3 Step 3` 当前又完成了第六批 internal-only helper naming：`theme.c` 里的 pure-static helper 已从 `picoui_theme_rgb_to_ld_color()`、`picoui_theme_state_is_valid()`、`picoui_theme_part_is_valid()`、`picoui_theme_part_supported()`、`picoui_theme_map_widget_colors()`、`picoui_theme_apply_widget_metrics()`、`picoui_theme_backend_can_apply_style()`、`picoui_theme_apply_{window,label,text,button,checkbox,switch,slider,list,image,calendar}_style()` 与 `picoui_theme_apply_native_widget_style()` 统一收口为 `tinyui_theme_*`；这一步只改动 [tinyui/src/theme/theme.c](/Users/cys/embedded/LingDongGUI/tinyui/src/theme/theme.c) 与对应 focused proof [tests/picoui/unit/test_picoui_theme.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_theme.c)，明确不触碰 `picoui_theme_apply_widget_style()`、public `picoui_theme_*` API 或 `picoui_app_set_theme()`
- `V3 Step 3` 当前又完成了第七批 internal-only helper naming：`layout/flex.c` 与 `layout/grid.c` 里的 pure-static helper 已从 `picoui_window_get_backend()`、`picoui_window_is_valid()` 统一收口为 `tinyui_window_get_backend()`、`tinyui_window_is_valid()`；这一步只改动 [tinyui/src/layout/flex.c](/Users/cys/embedded/LingDongGUI/tinyui/src/layout/flex.c)、[tinyui/src/layout/grid.c](/Users/cys/embedded/LingDongGUI/tinyui/src/layout/grid.c) 与对应 focused proof [tests/picoui/unit/test_picoui_layout.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_layout.c)，明确不触碰 public `picoui_flex_*` / `picoui_grid_*` API、contract/demo 命名或 `tests/picoui/*` 路径
- `V3 Step 3` 当前又完成了第八批 internal-only helper naming：`runtime_host.c` 里最前面的 pure-static/internal runtime helper 已从 `picoui_backend_touch_log_enabled()`、`picoui_backend_runtime_bootstrap()`、`picoui_backend_runtime_page_init()`、`picoui_backend_runtime_page_quit()`、`struct picoui_backend_runtime_state` 与 `g_picoui_backend_runtime_page` 收口为 `tinyui_runtime_host_*` / `struct tinyui_runtime_host_state` / `g_tinyui_runtime_host_page`；这一步只改动 [tinyui/src/core/runtime_host.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c) 与对应 focused proof [tests/picoui/unit/test_picoui_app_lifecycle.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_app_lifecycle.c)，明确不触碰 `picoui_backend_step_app()`、`prepare/render` helper、public `picoui_*` API 或 runtime/test 资产命名
- `V3 Step 3` 当前又完成了第九批 internal-only helper naming：`runtime_host.c` 里支撑 mapping/logging 判定的 pure-static helper 已从 `picoui_backend_widget_is_supported_real()`、`picoui_backend_widget_is_real_mapped()`、`picoui_backend_append_id()`、`picoui_backend_widget_needs_fallback()`、`picoui_backend_window_has_real_layout()`、`picoui_backend_widget_excludes_formal_mapping()`、`picoui_backend_widget_allows_smoke_layout()`、`picoui_backend_append_widget_ids()` 收口为 `tinyui_runtime_host_*`；这一步只改动 [tinyui/src/core/runtime_host.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c) 与对应 focused proof [tests/picoui/unit/test_picoui_app_lifecycle.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_app_lifecycle.c)，明确不触碰 runtime marker 字符串、`prepare/render/step` helper、public `picoui_*` API 或 runtime/test 资产命名
- `V3 Step 3` 当前又完成了第十批 internal-only helper naming：`runtime_host.c` 里支撑 capture/render 路径但尚未被现有 focused proof 锁名的 pure-static helper 已从 `picoui_backend_log_image_source_marker()`、`picoui_backend_parse_auto_quit_ms()`、`picoui_backend_pixel_to_rgb888()`、`picoui_backend_pixel_to_argb8888()`、`picoui_backend_runtime_state_from_app()`、`picoui_backend_app_state_from_window()`、`picoui_backend_present_real_frame()`、`picoui_backend_apply_real_widget_layout()` 收口为 `tinyui_runtime_host_*`；这一步只改动 [tinyui/src/core/runtime_host.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c) 与对应 focused proof [tests/picoui/unit/test_picoui_app_lifecycle.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_app_lifecycle.c)，明确不触碰 runtime marker 字符串、`write_capture` / `apply_smoke_cursor_layout` / `render` / `prepare` / `step` helper、public `picoui_*` API 或 runtime/test 资产命名
- `V3 Step 3` 当前又完成了第十一批 internal-only helper naming：`runtime_host.c` 里剩余的 internal runtime 主链 helper 已从 `picoui_backend_prepare_runtime_state()`、`picoui_backend_prepare_runtime_scene()`、`picoui_backend_log_runtime_ready()`、`picoui_backend_prepare_runtime()`、`picoui_backend_pump_sdl_events()`、`picoui_backend_step_app()` 统一收口为 `tinyui_runtime_host_*`；这一步同步改动了 [tinyui/src/core/runtime_host.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c)、[tinyui/src/core/runtime_bridge.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_bridge.c)、[tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h) 与对应 focused proof [tests/picoui/unit/test_picoui_app_lifecycle.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_app_lifecycle.c)，明确不触碰 runtime marker 字符串、public `picoui_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 contract/demo 命名
- `runtime_host.c` 当前已不再保留任何 `picoui_backend_*` internal helper 前缀；`test_picoui_app_lifecycle` 现已改守 `tinyui_runtime_host_prepare_runtime_state()`、`tinyui_runtime_host_prepare_runtime_scene()`、`tinyui_runtime_host_log_runtime_ready()`、`tinyui_runtime_host_prepare_runtime()`、`tinyui_runtime_host_pump_sdl_events()` 与 `tinyui_runtime_host_step_app()` 这条 runtime 主链
- `V3 Step 3` 当前又完成了第十二批 internal-only helper naming：`core/widget.c` 里的 pure-static/internal helper 已从 `picoui_widget_is_valid()`、`picoui_widget_get_ld_base()`、`picoui_widget_get_backend()`、`picoui_widget_expected_native_type()`、`picoui_widget_validate_native_binding()`、`picoui_backend_widget_can_attach_child()`、`picoui_backend_widget_clear_owner_and_root()`、`picoui_backend_widget_bind_subtree_owner_and_root()`、`picoui_backend_widget_get_host()` 与 `g_picoui_backend_next_data_model_identity` 收口为 `tinyui_*` / `g_tinyui_*`；这一步只改动 [tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c) 与对应 focused proof [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)，明确不触碰 `picoui_backend_widget_init_root()`、`picoui_backend_widget_init_child()`、`picoui_backend_widget_attach_child()`、`picoui_backend_widget_claim_focus()`、`picoui_backend_widget_release_focus()`、`picoui_backend_widget_is_kind()`、`picoui_widget_update_value()` 这些当前仍被 widgets/tests 当 internal entry 使用的非静态接口
- `core/widget.c` 当前仍保留的 `picoui_*` 名称已经收敛到 public widget API 与现有 internal/shared entry seam；这一批 focused proof 现已直接锁定上述 pure-static helper 和 data-model identity 计数器不再以 `picoui_*` 形式出现在 source 中
- `V3 Task 2 Step 1` 当前已完成最小安全第一批 target naming：`cmake/LingDongGUI.cmake` 中的 shared/product-layer target 已建立 `tinyui_*` 实名，当前包括 `tinyui_core`、`tinyui_backend_ldgui`、`tinyui_backend_ldgui_runtime`、`tinyui_port_sdl`；`tests/support/CMakeLists.txt` 中的 support 库也已建立 `tinyui_test_support` 实名
- 为避免提前撞进 `V4` 的 test/demo/contract/archive 命名面，这一批采用的是“`tinyui_*` 实名 + `picoui_*` CMake alias 兼容”口径：`picoui_core`、`picoui_backend_ldgui`、`picoui_backend_ldgui_runtime`、`picoui_port_sdl`、`picoui_test_support` 当前都仍可被现有测试图引用，但底层真实 target 名已经收口到 `tinyui_*`
- `V3 Task 2 Step 1` 这轮又继续完成了测试图内部 target 引用收口：`tests/picoui/CMakeLists.txt` 里的 `SUPPORT_LIB` / `MAIN_LIB` / `target_link_libraries(test_picoui_port_sdl ...)` 现已直接使用 `tinyui_test_support`、`tinyui_backend_ldgui`、`tinyui_port_sdl` 实名；外部测试可执行文件名、`check_picoui_*` / `test_picoui_*` 脚本与 label 保持不变，仍通过 `picoui_*` alias 与现有 gate 兼容
- 这一批 focused compile proof 已 fresh 通过：`test_picoui_runtime_model`、`test_picoui_layout`、`test_picoui_theme`、`test_picoui_native_bridge`、`test_picoui_port_sdl`；当前仍未触碰 `libpicoui_backend_ldgui.a` 这类 archive 名、`test_picoui_*` / `check_picoui_*` 文件名、`tests/picoui/*` 路径、CTest label 或 contract/demo 命名
- 这一批仍不触碰 public `picoui_window_set_*` / `picoui_grid_*` / `picoui_flex_*` API，也不触碰任何 `V4` 的测试、路径、label 或 contract/demo 命名迁移
- `V3 Task 2` 当前已补齐一批 gate 真相修复，但这不是新阶段切换：`tests/picoui/CMakeLists.txt` 中的 `test_picoui_wrapper_struct_overhead` 已从 `perf` label 退回 `probe` label，避免 `ctest -L perf` 在未预构建裸 probe 时出现假红；真正的 perf gate 继续由 `check_picoui_tinyui_object_overhead.py` 承担
- 同一批里，[tinyui/src/backend/ldgui/backend.h](/Users/cys/embedded/LingDongGUI/tinyui/src/backend/ldgui/backend.h) 已对 `struct picoui_backend_layout_window_state`、`struct picoui_backend_layout_child_state` 和 `struct picoui_backend_widget` 的小标量缓存做宽度收紧，`backend_widget_struct_bytes` 已从回归态 `536` 压回 `464`，重新低于当前 baseline gate `<=512`
- `V3 Step 3` 这轮又完成了一批 shared core internal event/callback seam 命名收口：`picoui_backend_emit_value_changed()`、`picoui_backend_emit_event()`、`picoui_backend_emit_clicked()` 已统一收口为 `tinyui_widget_emit_value_changed()`、`tinyui_widget_emit_event()`、`tinyui_widget_emit_clicked()`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c)、[tinyui/src/core/event.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/event.c)、[tinyui/src/backend/ldgui/backend.h](/Users/cys/embedded/LingDongGUI/tinyui/src/backend/ldgui/backend.h) 与 focused proof [tests/picoui/unit/test_picoui_button_events.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_button_events.c)
- 同一小批里，`theme.c` 的 backend-facing internal helper `picoui_theme_apply_widget_style()` 已收口为 `tinyui_theme_apply_widget_style()`，对应 focused proof 已同步到 [tests/picoui/unit/test_picoui_theme.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_theme.c)；这一步明确不触碰 public `picoui_theme_*` API、`picoui_app_set_theme()`、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 demo/contract 命名
- `V3 Step 3` 这轮又继续完成了 shared widget-tree lifecycle seam 的第一小批：`picoui_backend_widget_is_kind()` 已收口为 `tinyui_widget_is_kind()`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c) 与 focused proof [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)、[tests/picoui/unit/test_picoui_image.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_image.c)、[tests/picoui/unit/test_picoui_qrcode.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_qrcode.c)
- `V3 Step 3` 这轮又继续完成了 shared widget-tree lifecycle seam 的第二小批：`picoui_backend_widget_init_root()`、`picoui_backend_widget_init_child()`、`picoui_backend_widget_attach_child()` 已统一收口为 `tinyui_widget_init_root()`、`tinyui_widget_init_child()`、`tinyui_widget_attach_child()`，改动范围覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c)、`tinyui/src/widgets/*.c` 的直系 create/attach caller，以及 focused proof [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)、[tests/picoui/unit/test_picoui_image.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_image.c)、[tests/picoui/unit/test_picoui_qrcode.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_qrcode.c)
- `V3 Step 3` 这轮又继续完成了 `list selected-index seam`：`picoui_list_backend_set_selected_index()`、`picoui_list_backend_get_selected_index()`、`picoui_list_backend_sync_selected_index()` 已统一收口为 `tinyui_list_set_selected_index()`、`tinyui_list_get_selected_index()`、`tinyui_list_sync_selected_index()`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/event.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/event.c)、[tinyui/src/widgets/list.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/list.c) 与 focused proof [tests/picoui/unit/test_picoui_list.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_list.c)
- 这一步继续保持边界：`list` 这轮只收 `selected-index` 这 3 个 internal/shared entry，不顺手扩到 `picoui_backend_list_set_items()`、`set_item_height()`、`set_padding_group()`、`set_margin_group()`、`set_text_color()`、`set_bg_color()`、`set_select_color()`、`set_align()`、`set_item_widget()` 这组 widget-local moved helper
- `V3 Step 3` 这轮又继续完成了 `combo_box` 的 internal/widget-local helper seam：`picoui_backend_combo_box_rgb_to_ld_color()`、`picoui_backend_combo_box_get_ld()`、`picoui_backend_combo_box_native_slot()`、`picoui_combo_box_create_backend_local()`，以及 `picoui_backend_combo_box_{set_items,set_text_color,set_bg_color,set_frame_color,set_select_color,set_item_max,set_dropdown_source,set_selected_index,get_selected_index,get_text,sync_selected_index,bind_host,get_open}()` 已统一收口为 `tinyui_combo_box_*`，改动范围只覆盖 [tinyui/src/widgets/combo_box.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/combo_box.c) 与 focused proof [tests/picoui/unit/test_picoui_combo_box.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_combo_box.c)
- 这一步继续保持边界：`combo_box` 这轮只收 widget-local/internal helper 名称，不触碰 public `picoui_combo_box_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `line_edit` 的 internal/widget-local helper seam：`picoui_backend_line_edit_get_ld()`、`picoui_backend_line_edit_align_to_ld()`、`picoui_backend_line_edit_rgb_to_ld_color()`、`picoui_backend_line_edit_native_slot()`、`picoui_line_edit_create_backend_local()`，以及 `picoui_backend_line_edit_{set_text,set_align,set_color,get_text,set_type,get_type,set_keyboard_binding,get_keyboard_binding,bind_host,get_editing}()` 已统一收口为 `tinyui_line_edit_*`，改动范围只覆盖 [tinyui/src/widgets/line_edit.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/line_edit.c) 与 focused proof [tests/picoui/unit/test_picoui_line_edit.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_line_edit.c)
- 这一步继续保持边界：`line_edit` 这轮只收 widget-local/internal helper 名称，不触碰 public `picoui_line_edit_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `core/event` shared helper seam：`picoui_widget_accepts_event()`、`picoui_widget_slider_value_to_percent()`、`picoui_widget_slider_percent_to_value()`、`picoui_widget_sync_ld_value()`、`picoui_widget_emit_ld_event_bridge()`、`picoui_widget_claim_focus_for_signal()`、`picoui_widget_restore_rejected_list_selection()`、`picoui_widget_get_owner_app()`、`picoui_widget_note_focus_event()` 已统一收口为 `tinyui_widget_*`，改动范围只覆盖 [tinyui/src/core/event.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/event.c)、[tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c) 与 focused proof [tests/picoui/unit/test_picoui_button_events.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_button_events.c)
- 这一步继续保持边界：`core/event` 这轮只收 shared/internal helper 名称，不触碰 `picoui_widget_update_value()`、`picoui_widget_dispatch_signal()`、`picoui_widget_dispatch_event()`、`picoui_widget_dispatch_native_signal()` 这些 shared/public entry，也不触碰 public `picoui_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `text` 的 widget-local/internal seam：`picoui_backend_text_rgb_to_ld_color()`、`picoui_backend_text_get_ld_text()`、`picoui_backend_text_apply_consumed_font()`、`picoui_backend_text_default_font()`、`picoui_backend_text_resolve_font()`、`picoui_backend_text_test_fail_next_set_font()`，以及 `picoui_backend_text_{set_font,set_static_text,set_transparent,set_text_color,set_bg_color,set_background_source,scroll_seek,scroll_move}()` 已统一收口为 `tinyui_text_*`，改动范围覆盖 [tinyui/src/widgets/text.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/text.c) 与 focused proof [tests/picoui/unit/test_picoui_text.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_text.c)，以及一个直系测试调用点 [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)
- 这一步继续保持边界：`text` 这轮只收 widget-local/internal helper 与 test seam 名称，不触碰 public `picoui_text_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `list` 的剩余 widget-local/internal seam：`picoui_backend_list_{set_items,set_item_height,set_padding_group,set_margin_group,set_text_color,set_bg_color,set_select_color,set_align,set_item_widget}()` 已统一收口为 `tinyui_list_*`，改动范围覆盖 [tinyui/src/widgets/list.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/list.c)、[tinyui/src/backend/ldgui/backend.h](/Users/cys/embedded/LingDongGUI/tinyui/src/backend/ldgui/backend.h) 与 focused proof [tests/picoui/unit/test_picoui_list.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_list.c)；此前已完成的 `tinyui_list_set/get/sync_selected_index()` 保持不变
- 这一步继续保持边界：`list` 这轮只收 widget-local/internal helper 名称与旧声明清理，不触碰 public `picoui_list_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `calendar` 的 widget-local/internal seam：旧的 day-name globals、native readback/helper、widget-local create helper，以及整组 date/header/color/grid/system-date retained helper 已统一收口为 `g_tinyui_calendar_*` 与 `tinyui_calendar_*`，改动范围覆盖 [tinyui/src/widgets/calendar.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/calendar.c)、[tinyui/src/backend/ldgui/backend.h](/Users/cys/embedded/LingDongGUI/tinyui/src/backend/ldgui/backend.h) 与 focused proof [tests/picoui/unit/test_picoui_calendar.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_calendar.c)
- 这一步继续保持边界：`calendar` 这轮只收 widget-local/internal helper 名称与旧声明清理，不触碰 public `picoui_calendar_*` API、`tests/picoui/*` 路径、`test/check` 文件名、CTest label 或 `V4` 资产命名
- `V3 Step 3` 这轮又继续完成了 `window` 的最小 `color test seam`：旧的 bg-color fail flag、RGB/native color conversion helper 与对应 test seam 已统一收口为 `tinyui_window_*`，改动范围只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 color helper 与 test seam 名称，不触碰 public `picoui_window_*` API，也不把 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `layout mapper seam`：旧的 flex/grid align-flow-track mapping helper 与 grid-track copy helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 pure-static mapper/copy helper 名称，不触碰 public `picoui_window_*` API，也不把 `get_ld_window/get_root_size/init_defaults/dispose_partial/apply_*` 或 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `native access seam`：旧的 backend/native access helper 与 layout-type mapping helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `get_backend/get_backend_host/get_ld_window/map_layout_type` 这组 static helper 名称，不触碰 public `picoui_window_*` API，也不把 `get_root_size/init_defaults/dispose_partial/apply_*` 或 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `validation seam`：旧的 `is_valid/props_are_valid` 这组 static 校验 helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `is_valid/props_are_valid` 这组 static 校验 helper 名称，不触碰 public `picoui_window_*` API，也不把 `get_root_size/init_defaults/dispose_partial/apply_*` 或 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `init-defaults seam`：旧的 `init_defaults` static helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `init_defaults` 这一项 static 初始化 helper 名称，不触碰 public `picoui_window_*` API，也不把 `get_root_size/dispose_partial` 或 `apply_*` 与 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `root-size seam`：旧的 `get_root_size` static helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `get_root_size` 这一项 static root-size helper 名称，不触碰 public `picoui_window_*` API，也不把 `dispose_partial` 或 `apply_*` 与 `root/layout/background` shared 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `dispose-partial seam`：旧的 `dispose_partial` static lifecycle helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `dispose_partial` 这一项 static lifecycle helper 名称，不触碰 public `picoui_window_*` API，也不把 `set_padding_group_impl/apply_padding_contract/apply_layout_type_impl/apply_flex_contract_impl/apply_generic_gap_impl` 那条 layout-contract 主链混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `generic-gap seam`：旧的 `apply_generic_gap_impl` static gap helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `apply_generic_gap_impl` 这一项 static gap helper 名称，不触碰 public `picoui_window_*` API，也不把 `apply_flex_contract_impl`、`set_padding_group_impl`、`apply_layout_type_impl` 与共享核心 `apply_padding_contract` 混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `padding-contract seam` 第一批：旧的 `set_padding_group_impl` 与共享核心 `apply_padding_contract` 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `set_padding_group_impl + apply_padding_contract` 这一组 internal padding-contract helper 名称，不触碰 public `picoui_window_*` API，也不把 `apply_layout_type_impl` 或死 helper `apply_flex_contract_impl` 混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的 `layout-type seam`：旧的 `apply_layout_type_impl` static helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 这一步继续保持边界：`window` 这轮只收 `apply_layout_type_impl` 这一项 internal helper 名称，不触碰 public `picoui_window_*` API，也不把最后的死 helper `apply_flex_contract_impl` 混进来
- `V3 Step 3` 这轮又继续完成了 `window` 的最后一个 `flex-contract dead-helper seam`：旧的 `apply_flex_contract_impl` static dead helper 已统一收口为 `tinyui_window_*`，改动范围仍只覆盖 [tinyui/src/widgets/window.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/window.c) 与 focused proof [tests/picoui/unit/test_picoui_window.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_window.c)
- 到当前真相，`window.c` 这条 `V3 Step 3` internal rename 线已经收口：`color test / layout mapper / native access / validation / init-defaults / root-size / dispose-partial / generic-gap / padding-contract / layout-type / dead-helper` 这些 widget-local/internal seam 都已统一切到 `tinyui_window_*`
- 后续若继续动 `window`，边界不再是同类 internal rename，而应转向 public API / shared truth / `V4` 资产面；这里不再把 `window.c` 写成仍有同类 `V3 Step 3` internal helper 待办
- `V3 Step 3` 这轮又继续完成了 shared/core `host-binding seam`：`picoui_widget_bind_backend_host()`、`picoui_widget_backend_host()`、`picoui_widget_backend_detach()`、`picoui_widget_owner_app()`、`picoui_widget_has_ld_binding()` 已统一收口为 `tinyui_widget_*`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c)、[tinyui/src/core/runtime_bridge.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_bridge.c)、[tinyui/src/widgets/list.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/list.c) 与 focused proof [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)、[tests/picoui/unit/test_picoui_button_events.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_button_events.c) 及一组只读 extern 调用测试
- 这一步继续保持 internal-only 边界：本批只收 host-binding helper cluster，不触碰 `picoui_widget_update_value()`、`picoui_widget_dispatch_signal()`、`picoui_widget_dispatch_event()`、`picoui_widget_dispatch_native_signal()` 或 `picoui_backend_set_text()`；因此 `V3 Step 3` 当前仍是进行态，不应写成代码面整体 closeout
- `V3 Step 3` 这轮又继续完成了 shared/core `update-value seam`：`picoui_widget_update_value()` 已统一收口为 `tinyui_widget_update_value()`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/widget.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/widget.c)、[tinyui/src/widgets/checkbox.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/checkbox.c)、[tinyui/src/widgets/switch.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/switch.c)、[tinyui/src/widgets/slider.c](/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/slider.c) 与 focused proof [tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)
- 这一步继续保持 internal-only 边界：本批只收 `update_value` 这一条 shared setter-helper，不触碰 `picoui_widget_dispatch_signal()`、`picoui_widget_dispatch_event()`、`picoui_widget_dispatch_native_signal()` 或 `picoui_backend_set_text()`
- `V3 Step 3` 这轮又继续完成了 shared/core `dispatch seam`：`picoui_widget_dispatch_signal()`、`picoui_widget_dispatch_event()`、`picoui_widget_dispatch_native_signal()` 已统一收口为 `tinyui_widget_dispatch_signal()`、`tinyui_widget_dispatch_event()`、`tinyui_widget_dispatch_native_signal()`，改动范围只覆盖 [tinyui/src/core/internal.h](/Users/cys/embedded/LingDongGUI/tinyui/src/core/internal.h)、[tinyui/src/core/event.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/event.c)、[tinyui/src/core/runtime_bridge.c](/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_bridge.c) 与 focused proof [tests/picoui/unit/test_picoui_button_events.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_button_events.c)、[tests/picoui/unit/test_picoui_widgets.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_widgets.c)、[tests/picoui/unit/test_picoui_keyboard.c](/Users/cys/embedded/LingDongGUI/tests/picoui/unit/test_picoui_keyboard.c)
- 这一步继续保持 internal-only 边界：本批只收 `dispatch_*` 这一组 shared dispatch helper，不触碰 `picoui_backend_set_text()`；到当前真相，下一自然顺序已前移到 `backend_set_text()`
- `V3 Task 2 Step 3` 当前也已完成最小 closeout 文档收口：[docs/v2.1/v2.1-closeout.md](/Users/cys/embedded/LingDongGUI/docs/v2.1/v2.1-closeout.md) 已建立 `V3` 阶段 closeout truth，明确 shared layer 已按产品层 `tinyui` 子系统收口、shared layer retention 是有意设计、backend removal 没有把 shared responsibilities 粗暴塌缩进 widgets；同时也明确这还不是 `v2.1` 全阶段最终 release closeout

### V4 demo/test/contract/CMake 全量迁移

文件：`docs/v2.1/plans/stages/v4-demo-test-contract-cmake-migration-plan.md`

目标：

- demo、tests、contracts、CMake/CTest、perf artifact 全量迁移到 `tinyui`
- 删除产品层对 `picoui` 的公开命名依赖

### V5 Closeout 与 release 收口

文件：`docs/v2.1/plans/stages/v5-closeout-and-release-plan.md`

目标：

- 清零剩余 `picoui` 产品层痕迹
- 收口 `v2.1` closeout/release truth
- 用 fresh gates 给出最终完成态

## 执行纪律

- 任何阶段未完成 focused tests + broad gates + docs update，不得进入下一阶段
- 每阶段建议独立 fresh subagent
- 跨阶段共享文件冲突时，由主线程先重新裁边界
- `v2.1` 不得破坏 `v2.0` 已闭环的 perf/runtime/visible 证据线
- `v2.1` 不得把 rename 范围扩到 `LingDongGUI` 目录和 `ld*` public API

## 当前阶段真相

- `V3` 已完成：
  - 2026-06-12 fresh focused proof 已覆盖 shared/internal 最后一批 seam：`test_picoui_{widgets,app_timer,combo_box,icon_slider,line_edit,radial_menu,table,runtime_model}`
  - 2026-06-12 fresh broad gates 已通过：`rtk ctest --test-dir build/picoui-runtime -R 'check_picoui_runtime|check_picoui_visible_ui|check_picoui_backend_mapping' --output-on-failure` 与 `rtk ctest --test-dir build -L 'perf' --output-on-failure`
  - `docs/v2.1/v2.1-closeout.md`、`docs/v2.1/线计划索引.md` 与 `docs/v2.1/plans/stages/v3-shared-layer-rename-plan.md` 已同步到完成态
- `V4` 已启动但尚未完成：
  - 当前第一批最小收口已落地：`tests/picoui/CMakeLists.txt` 里的 contract/runtime/perf 注册已切到 `check_tinyui_*` 与 `tinyui` label；`examples/sdl/CMakeLists.txt` 已改用 `add_tinyui_demo()`；`cmake/LingDongGUI.cmake` 已建立 `ld_apply_tinyui_runtime_screen_config()` 并保留旧 helper 转发兼容
  - 当前第二批收口也已落地：`tests/tinyui/{contract,runtime,perf}` 已成为 canonical 路径；`tests/picoui/{contract,runtime,perf}` 当前只保留兼容薄壳/兼容副本；`tests/picoui/CMakeLists.txt` 里的 Python checker 注册已切到 `tests/tinyui/*`
  - 当前第三批已在文档层收口：`docs/v2.1/plans/stages/v4-demo-test-contract-cmake-migration-plan.md` 已把已完成项同步为完成态，`V4` 剩余工作已明确收敛到 `tests/picoui/unit/*` 物理迁移、测试源码内自引用路径/固定二进制路径清理、以及最终 broad gate
  - 当前第四批也已落地：`tests/tinyui/unit/*` 已成为 canonical unit 路径；原 `tests/picoui/unit/test_picoui_*.c` 已物理迁到 `tests/tinyui/unit/test_tinyui_*.c`；`tests/picoui/unit/` 当前已清空；`tests/picoui/CMakeLists.txt` 已切到 `../tinyui/unit/test_tinyui_*.c`
  - 当前第五批也已落地：`tests/tinyui/contract/tinyui_release_capability_matrix.json` 的 canonical 证据串已切到 `test_tinyui_*` / `check_tinyui_*` / `tests/tinyui/runtime/*`；`tests/picoui/{contract,perf,runtime}` 的剩余 `check_picoui_*` / `check_picoui_tinyui_*` 已进一步收紧为 thin wrapper
  - 当前第六批也已落地：`tests/picoui/contract/*` 旧真相源的证据串已进一步统一到 TinyUI 口径；同时 `check_picoui_native_100_inventory.py` 与 `picoui_native_100_inventory.json` 已补齐 `canvas`，native-100 inventory gate 已重新通过
  - 当前第七批已完成 include 消费面第一小批 safe move：`tinyui/port/sdl/sdl.c` 已切到顶层 `display.h`/`osal.h`/`tick.h`，`tinyui/demo/animation_basic/main.c` 已切到顶层 `image.h`
  - 当前 fresh 证据：`rtk cmake -S . -B build`、`rtk cmake --build build --target tinyui_hello_world_demo`、`python3 tests/tinyui/contract/check_tinyui_public_api.py`、`python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py --print-current`、`python3 tests/tinyui/perf/check_tinyui_perf.py --self-test`、`python3 tests/tinyui/perf/check_tinyui_object_overhead.py --self-test`、`python3 tests/tinyui/runtime/check_tinyui_runtime.py --help` 均已通过
  - 当前又新增 fresh focused proof：`rtk ctest --test-dir build --output-on-failure -R '^(test_tinyui_app_timer|test_tinyui_app_lifecycle|test_tinyui_layout|test_tinyui_keyboard|test_tinyui_wrapper_struct_overhead)$'` 已通过
  - 当前又新增兼容壳验证：`python3 tests/picoui/runtime/check_picoui_runtime.py --help`、`python3 tests/picoui/runtime/check_picoui_visible_ui.py --help`、`python3 tests/picoui/perf/check_picoui_tinyui_perf.py --self-test` 已通过
  - 当前又新增 contract 旧真相源验证：`python3 tests/picoui/contract/check_picoui_release_capability_matrix.py`、`python3 tests/picoui/contract/check_picoui_public_api.py`、`python3 tests/picoui/contract/check_picoui_native_100_inventory.py` 已通过
  - 当前又新增 include-first 验证：`rtk cmake -S . -B build` 与 `rtk cmake --build build --target tinyui_animation_basic_demo test_tinyui_port_sdl test_tinyui_port_display test_tinyui_port_input test_tinyui_port_tick_os` 已通过
  - 当前又完成 canonical 头可组合性修复：`tinyui/include/{core,screen,label,button,switch}.h` 不再重复声明 `tinyui_init` / `tinyui_screen_create` / `tinyui_label_create` / `tinyui_button_create` / `tinyui_switch_create` 等已由兼容头提供的 inline/宏；`check_tinyui_v21_transition_guards.py` 已新增多头组合 `cc -fsyntax-only` probe；去重后 `tinyui_v21_transition_inventory.json` 的 `tinyui_public_api_count` 当前实测为 `26`，`tinyui_transition_inventory.json` 当前实测为 `21`
  - 当前仍未完成：`tests/picoui/contract/*` 旧入口文件名与 legacy loader、以及 docs 历史记录里的 `test_picoui_*` / `check_picoui_*` 残留清理
  - `tinyui/include/picoui/*` 当前不能直接删：仍被 `tinyui/include/*.h`、`tinyui/src/*`、`tinyui/demo/*`、`tests/tinyui/unit/*` 与 canonical contract checker 直接依赖；最小安全顺序是先迁 consumer/header/checker 面，再在 `V5` 退场
  - consumer include 面此前暴露的 canonical 头多头组合 `tinyui_*` inline redefinition blocker 已完成最小修复；下一步仍不能直接删兼容头，应继续小批迁移 consumer/header/checker 面

## Closeout Truth

- `v2.1` 完成态以以下文档为准：
  - `docs/v2.1/v2.1-closeout.md`
  - `docs/v2.1/v2.1-release-matrix.md`
  - `docs/v2.1/v2.1-performance-baseline.md`
- 若阶段 plan checkbox、历史 summary 与上述文档 wording 不一致，优先以后者和 fresh gate 输出为准。
- `v2.1` 的完成态必须证明：
  - 产品层公开痕迹不再出现 `picoui`
  - 独立 `backend/` 目录已消失
  - `LingDongGUI` 目录与 `ld*` public API 保持不变
