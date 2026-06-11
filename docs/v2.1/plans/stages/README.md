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
- 当前 `V2` 已把 guard 收紧为 `backend_c_files=0`；在 backend 真正清零前，`check_tinyui_v21_transition_guards` 应保持 fail-first

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
- `keyboard/table` 当前保留的 helper-only backend 文件已缩到 `backend_table.c`：`backend_keyboard.c` 已退场，`backend_table.c` 当前继续承载 `cell/edit/navigation/sync_current_cell`；因此这一轮 keyboard 已完成 compile-surface cleanup，table 仍不是当前 cleanup 候选
- 本批 focused unit proof 应以 `test_picoui_message_box`、`test_picoui_keyboard`、`test_picoui_table` 为准；其中 `message_box` 重点守住 `parent/root/owner/host_widget/ld_widget/ldBase.pInfo` 的 direct-create tree/binding 合同，以及 widget 文件直接驱动 native title/message/buttons/colors/callback bridge 的真相；`table` 当前 focused proof 已覆盖 `current_cell sync / editable commit-cancel / image-button / static_text / keyboard binding / align-grid / region getter` 等真实合同，且新增锁定 `picoui_backend_table_bind_host()` 不再以 backend 符号形式暴露
- `table` 这轮又进一步完成最小 shared/widget 边界收口：原先 `backend_table.c` 中的 `picoui_backend_table_bind_host()` 与 `native_slot` 已并回 `tinyui/src/widgets/table.c` 的 widget-local 绑定路径；`backend_table.c` 当前继续保留的边界缩到 `cell/edit/navigation/sync_current_cell`
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
- `table` 新增 corrupted-binding 负向合同：`picoui_backend_table_sync_current_cell()` 在 `kind/ld_widget` 绑定损坏时必须返回 `-1`，不得把损坏绑定误判成合法 current-cell，也不得污染 `table->current_row` / `table->current_column` 缓存
- `table` 这轮又进一步完成最小 shared/widget 边界收口：`picoui_backend_table_navigate()` 与 `picoui_backend_table_sync_current_cell()` 已并回 `tinyui/src/widgets/table.c` 的 widget-local retained seam；`test_picoui_table` 当前继续直接锁定这两个 legacy backend 符号不再以 public/backend 形式暴露，同时保持 current-cell readback、navigate 和 corrupted-binding 合同不回退
- `table` 当前不再是“待摸清边界”的优先项：focused test 已经把 `backend_table.c` 的真实保留面基本坐实，因此后续若继续推进 `V2`，更应优先选择 `keyboard` 或继续评估 `layout` 是否还有新的 contract gap，而不是把 `table` 当成 compile-surface cleanup 候选
- `backend_layout.c` 当前继续保留，真实保留边界仍是 `window/layout` 实逻辑；这一步只收紧 retained-boundary 合同，不把它写成 compile-surface cleanup 候选
- `backend_event.c` 当前已继续完成 shared-helper 外层剥离：`picoui_backend_emit_value_changed()`、`picoui_backend_emit_event()`、`picoui_backend_emit_clicked()` 已迁到 `tinyui/src/core/widget.c`，`picoui_widget_dispatch_signal()` / `picoui_widget_dispatch_event()`、`picoui_widget_sync_ld_value()`、`picoui_widget_emit_ld_event_bridge()` 已迁到 `tinyui/src/core/event.c`；legacy `picoui_backend_sync_ld_value()` / `picoui_backend_emit_ld_event_bridge()` 不再继续以 backend 符号形式暴露
- `backend_event.c` 这轮又进一步完成 shared setter-helper 收口：`picoui_backend_widget_update_value()` 已迁到 `tinyui/src/core/widget.c` 的 shared/internal 入口 `picoui_widget_update_value()`；`checkbox/switch/slider` 的 setter/data-model 路径现已不再依赖 backend 符号，`test_picoui_widgets` 继续直接锁定 legacy `picoui_backend_widget_update_value()` 不得再出现在测试二进制符号表
- `backend_event.c` 这轮又进一步完成 shared binding 收口：`picoui_backend_widget_bind_host()` 与 `picoui_backend_widget_bind_ld_event_bridge()` 已迁到 `tinyui/src/core/runtime_bridge.c`，不再继续挂在 `backend/` 路径
- 本批 focused proof 继续以 `test_picoui_widgets`、`test_picoui_button_events` 为准；新增合同锁定 `picoui_backend_widget_bind_ld_event_bridge()` 在 native `ld_widget` 缺失时必须 `fail-closed` 返回 `-1`，且不得脏写 `ld_event_bridge_scene/sender`；同时测试二进制符号表继续锁定 legacy `picoui_backend_sync_ld_value()` / `picoui_backend_emit_ld_event_bridge()` 不得再出现
- `test_picoui_button_events` 当前还直接扫描测试二进制符号表，锁定 legacy `picoui_backend_widget_dispatch_signal()` / `picoui_backend_widget_dispatch_event()` 不得再以 backend 符号形式出现；同时保持 pressed/released/value-changed 的 callback、focus claim、dispatch_count 与 invisible/disabled fail-closed 合同不回退
- `backend_event.c` 这一步之后仍保留的责任是真实 retained/shared event router 边界：`ldMsg` bridge connect/slot、`widget_connect_native_events()`、`widget_dispatch_native_signal()`、slider/list 原生事件归一化、focus/editing 协调与 rejected-list-selection restore；本批不宣称这些已经迁完
- `backend_widget.c` 这批 shared helper 已清零退场：原先残留的 `picoui_native_nav_dir_to_ld()`、`picoui_backend_widget_init_data_model()`、`picoui_backend_widget_claim_focus()`、`picoui_backend_widget_release_focus()` 现已统一迁到 `tinyui/src/core/widget.c`；`cmake/LingDongGUI.cmake` 已将 `backend_widget.c` 退编并删除文件本体
- `theme` 这一步已完成 shared bridge 收口：`picoui_app_set_theme()` 不再经由 `backend_theme.c` 薄包装，而是直接调用 `tinyui/src/core/runtime_bridge.c` 中的 `picoui_runtime_bridge_bind_theme()`；对应空壳 `backend_theme.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除
- `theme` 这一步也已完成 shared style apply 收口：`picoui_theme_apply_to_widget()` 现已在 `tinyui/src/theme/theme.c` 内直接完成 `window/label/text/button/checkbox/switch/slider/list/image/calendar` 的 native style dispatch，不再经由 `backend_style_apply.c`
- `backend_style_apply.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除；对应 retained/shared style 边界现已收敛到 `theme.c`，不再保留独立 backend style apply 层
- `backend_widget_tree.c` 这一步也已完成 shared helper 收口：`picoui_backend_widget_is_kind()` 已并入 `tinyui/src/core/widget.c`，`picoui_backend_widget_unbind_host()` / `picoui_backend_widget_detach_from_parent()` 已并入 `tinyui/src/core/runtime_bridge.c`；对应 `backend_widget_tree.c` 已从 `cmake/LingDongGUI.cmake` 退编并删除
- 本批 focused proof 当前以 `test_picoui_widgets`、`test_picoui_image`、`test_picoui_qrcode` 为准：继续锁定 `backend_widget_tree` 这 3 个 shared helper 在 null 输入下必须 fail-closed，且 image/qrcode rollback 路径继续通过 shared helper 完成 detach/unbind，不回退成 backend 路径私有实现
- 当前 backend 编译面已进一步收缩到 4 个：`backend_app.c`、`backend_layout.c`、`backend_event.c`、`backend_table.c`
- 但 `backend_layout.c` 仍承载实逻辑，且独立 `backend_*.c` 文件总数仍未清零，所以 `V2` 仍在进行中，未 closeout

### V3 shared layer 命名与 include path 收口

文件：`docs/v2.1/plans/stages/v3-shared-layer-rename-plan.md`

目标：

- 统一 `core/display/indev/layout/theme/tick/osal` 的产品层命名
- 收口内部 include path、target、路径引用
- 保持共享层薄化，而不是把共享逻辑塞进 widgets

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
