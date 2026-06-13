# TINYUI a-0.6 icon_slider / radial_menu parity repair 记录

## 背景

`2026-06-01` 的 `a-0.6` review 先确认过一个真实问题：当前主仓虽然把 `icon_slider / radial_menu` 接进了 TINYUI public widget 集，也把 runtime / mapping / visible / manual artifact gate 接到了 final release catalog，但这两个控件当时还不能诚实地写成 `full_parity_complete`。

根因不是 gate 没跑，而是 public props 和真实 LingDongGUI backend 之间仍然脱节：

1. `icon_slider`
   - `tinyui_icon_slider_create_with_props()` 只把 `icon_width / icon_space / columns / rows / pages` 留在 host struct。
   - `tinyui_backend_create_icon_slider()` 仍用硬编码 `46 / 2 / 4 / 1 / 2` 调 `ldIconSlider_init(...)`。
2. `radial_menu`
   - `tinyui_radial_menu_create_with_props()` 只把 `x_axis / y_axis / item_max` 留在 host struct。
   - `tinyui_backend_create_radial_menu()` 仍用硬编码 `68 / 46 / 5` 调 `ldRadialMenu_init(...)`。

这会制造一种假完成：

1. host getter 看起来像已经支持 props。
2. demo / runtime / visible / mapping gate 也都能通过。
3. 但真实 backend 行为没有按 public props 生效。

## 修复目标

本轮只修最小确定 blocker，不扩 `a-0.6` 范围：

1. 让 `icon_slider` 的 `icon_width / icon_space / columns / rows / pages` 真实下传到 `ldIconSlider`。
2. 让 `radial_menu` 的 `x_axis / y_axis / item_max` 真实下传到 `ldRadialMenu`。
3. 用 unit test 直接断言 backend 实际字段，防止后续再次把 host cache 误写成 parity 完成。
4. 把 `a-0.6` 当时的 release truth-source 与 closeout 文档同步回和代码一致的口径。

## 非目标

本轮不做：

1. 新增 `icon_slider / radial_menu` 的更高阶 API。
2. 扩大 visible heuristic 范围。
3. 重写 final release matrix schema。
4. 顺手修别的控件 parity gap。

## 修复策略

### 1. fail-first unit test

先补两组失败测试：

1. `icon_slider_create_with_props`
   - 构造非默认 `width / height / icon_width / icon_space / columns / rows / pages / horizontal`
   - 直接检查 backend `ldIconSlider_t` 的实际字段
2. `radial_menu_create_with_props`
   - 构造非默认 `width / height / x_axis / y_axis / item_max / default_index`
   - 直接检查 backend `ldRadialMenu_t` 的实际字段

若测试在修复前不红，就说明测试还在看 host cache，必须先继续收紧。

### 2. 最小实现

实现只做一件事：把 public props 真实透传到 backend。

本轮实际落地方式：

1. `backend.h` 扩展 `tinyui_backend_create_icon_slider(...)` 与 `tinyui_backend_create_radial_menu(...)` 的 constructor 参数面。
2. `tinyui/src/widgets/icon_slider.c` / `tinyui/src/widgets/radial_menu.c` 新增 backend-config helper。
3. `create()` 继续走默认参数；`create_with_props()` 直接把 public props 传入 backend constructor。
4. `backend_icon_slider.c` / `backend_radial_menu.c` 去掉旧硬编码，改为把透传参数送入 `ldIconSlider_init(...)` / `ldRadialMenu_init(...)`。
5. 保持 demo 调用面不变，只修正 public props 不失真。

### 3. truth-source 同步

修完代码与测试后，再同步：

1. `tests/tinyui/contract/tinyui_release_capability_matrix.json`
2. `docs/tinyui-serial/a-0.6-final-release-closeout.md`
3. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
4. 必要时 `tinyui/docs/demo_guide.md`

同步原则：

1. 没有代码/测试支持的 parity 结论，不写成完成。
2. gate green 只证明 gate 自己的范围，不外推成更强结论。

## 修复结果

本轮结果：

1. `icon_slider` 的 `width / height / icon_width / icon_space / columns / rows / pages / horizontal` 已由 `create_with_props()` 落到真实 backend。
2. `radial_menu` 的 `width / height / x_axis / y_axis / item_max / default_index` 已由 `create_with_props()` 落到真实 backend 行为链路。
3. 新增两条 unit test 直接断言 `ldIconSlider_t` / `ldRadialMenu_t` 实际字段。
4. `a-0.6` closeout 与后续版本记录已同步回“代码与文档一致”的当前态。

## 验收结果

本轮已满足：

1. 新增 unit test 在修复前失败、修复后通过。
2. `icon_slider / radial_menu` 的关键 props 已进入真实 backend 字段。
3. 相关 unit / contract / runtime gate 通过。
4. truth-source 文档已与当前代码态重新对齐。

## 本轮涉及文件

代码：

1. `tinyui/src/widgets/icon_slider.c`
2. `tinyui/src/backend/ldgui/backend_icon_slider.c`
3. `tinyui/src/widgets/radial_menu.c`
4. `tinyui/src/backend/ldgui/backend_radial_menu.c`
5. `tinyui/src/backend/ldgui/backend.h`

测试：

1. `tests/tinyui/unit/test_tinyui_icon_slider.c`
2. `tests/tinyui/unit/test_tinyui_radial_menu.c`

文档 / truth-source：

1. `docs/tinyui-serial/a-0.6-final-release-closeout.md`
2. `docs/tinyui-serial/a-0.4-a-0.6-后续版本记录.md`
3. `tests/tinyui/contract/tinyui_release_capability_matrix.json`
