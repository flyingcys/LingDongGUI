# TinyUI v2.1 V5 Closeout And Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 收口 `v2.1` 剩余 residue、完成 closeout/release 真相收口，并给出与当前仓库一致的最终完成态。

**Architecture:** V5 不再改路线，只做 residue cleanup、closeout 文档、release matrix、performance baseline 迁移、final gates 与 release-facing proof。完成态必须严格证明：canonical truth 已统一到 `tinyui`，独立 backend 已消失，`LingDongGUI` 目录未动；允许继续存在 release matrix 明确标注的 `picoui_*` 过渡态命中与历史资产入口。

**Tech Stack:** Markdown、Python checker、CTest、现有 runtime/visible/perf gates。

---

## 文件结构

新增：

- `docs/v2.1/v2.1-closeout.md`
- `docs/v2.1/v2.1-release-matrix.md`
- `docs/v2.1/v2.1-performance-baseline.md`

修改：

- `docs/v2.1/线计划索引.md`
- `docs/v2.1/plans/stages/README.md`
- `docs/v2.1/2026-06-10-tinyui-v2-1-design.md`
- renamed `tests/tinyui/*` checkers

---

### Task 1: 清零产品层 `picoui` 残留

**Files:**
- Modify or delete any final product-layer residue still matching `picoui`

- [x] **Step 1: 跑最终 residue scan**

Run:

```bash
rg -n "picoui" tinyui tests cmake docs/v2.1
```

Expected: either zero hits or only explicitly allowed historical references outside the product-layer truth surface.

当前执行口径补充：

- `zero hits` 当前不是 `v2.1` 的真实完成条件
- 允许继续存在的命中只包括：
  - `tinyui/include/picoui/*` compatibility subtree
  - `tests/picoui/{contract,runtime,perf}/*` 的历史兼容/历史资产说明
  - `docs/v2.1/*` 中对过渡态和 non-goal 的明确记录
  - 当前仍处过渡态的 public C API / demo / runtime 使用面中的 `picoui_*` 符号
- 不允许再把 scan 结果里这些“被设计允许的过渡态命中”误判成必须在 `v2.1` 里全部删除的 residue

fresh 证据：

- `rg -n "picoui" tinyui tests cmake docs/v2.1`
- 结果已确认：当前命中面收敛为 release matrix 明确允许的 `tinyui/include/picoui/*` compatibility subtree、`tests/picoui/*` 历史兼容/历史资产说明、`docs/v2.1/*` 中对过渡态和 non-goal 的明确记录，以及仍处过渡态的 public C API / demo / runtime `picoui_*` 符号；未再发现需要在 `v2.1` 中继续强制清零的 current-truth residue

- [x] **Step 2: 清理剩余命名残留**

For every remaining product-layer hit found in Step 1:

- rename it to `tinyui`, or
- delete it if it is obsolete migration residue

Do not touch:

- `LingDongGUI` directory names
- `ld*` engine API names
- historical `docs/v2.0/*` records that remain as prior-version truth

当前完成态：

- 当前 product-layer canonical truth 已统一到 `tinyui/`
- 仍保留的 `picoui_*` 命中已明确限定为 release matrix 允许的过渡态 public C API、compatibility subtree 与历史资产入口
- `LingDongGUI` 目录与 `ld*` engine API 全程未动

- [x] **Step 3: 退场历史兼容/旧真相源入口**

Explicitly decide and execute the final disposition of the remaining `tests/picoui/contract/*` residue:

- old `tinyui*_inventory.json` / `tinyui_release_capability_matrix.json` compatibility copies under `tests/picoui/contract/*`
- any remaining compatibility-shell script filenames under `tests/picoui/contract/*`

Expected: `V5` must either delete these historical compatibility assets, or leave only clearly documented non-canonical compatibility shells with no ambiguity about the canonical truth source.

当前真相：

- 与 canonical 完全一致的 `tinyui_{transition_inventory,v21_transition_inventory,release_capability_matrix}.json` 兼容副本已退场
- canonical live checker 当前已不再直接读取 `tests/picoui/contract/{ldgui_public_api_inventory.json,ldgui_public_api_expected_symbols.json,native_api_gap_ledger.json}`；对应 `tests/tinyui/contract/{ldgui_public_api_inventory.json,native_api_gap_ledger.json,ldgui_public_api_expected_symbols.json}` 已建立 canonical 副本
- `tests/picoui/contract/check_ldgui_public_api_inventory.py` 当前也已切到 canonical `tests/tinyui/contract/*` 输入；旧 `tests/picoui/contract/{ldgui_public_api_inventory.json,ldgui_public_api_expected_symbols.json,native_api_gap_ledger.json}` 当前只剩历史资产职责
- `picoui_tinyui_transition_inventory.json` 当前已只剩历史文档引用，不在 live checker 执行链中
- `tests/tinyui/contract/check_tinyui_native_100_inventory.py` 与 `tests/tinyui/contract/tinyui_native_100_inventory.json` 当前已建立 canonical 副本；旧 `picoui_native_100_inventory.json` 当前只剩 legacy/docs 残留面
- canonical exhaustiveness/release-matrix truth 当前已落在 `tests/tinyui/contract/*`；`tinyui/docs/*` 与 `docs/ability/*` 当前口径也已切到 canonical matrix，旧 `picoui_release_capability_matrix.json` 当前只剩 legacy/docs 残留面
- `tinyui/include/{animation,arc,checkbox,clock,date_time,gauge,list,port,progress_bar,progress_wheel,qrcode,slider}.h` 当前已补齐顶层转发头，`tinyui/include/tinyui.h` 当前也已扩成可承接当前 demo umbrella 的 canonical 入口；`tinyui/demo/{list_basic,settings_panel,graph_basic,message_box_basic,combo_box_basic,line_edit_basic,layout_grid,layout_flex}/main.c` 已切到 `#include "tinyui.h"`
- `tests/tinyui/unit/{test_tinyui_clock,test_tinyui_date_time,test_tinyui_progress_bar,test_tinyui_qrcode}.c` 当前也已切到顶层 `{clock,date_time,progress_bar,qrcode}.h`，focused unit 与 target build proof 已通过
- `tests/support/picoui_test_support.h` 与剩余 `tests/tinyui/unit/*` umbrella consumer` 当前也已切到 `tinyui.h`；同时已补齐 `tinyui/include/port/sdl.h` 顶层转发头，`test_tinyui_port_sdl.c` 已切到 `tinyui.h + port/sdl.h`
- `tests/tinyui/unit/{test_tinyui_animation,test_tinyui_arc,test_tinyui_gauge,test_tinyui_list,test_tinyui_progress_wheel}.c` 当前也已从 `picoui/<widget>.h` 切到顶层 `{animation,arc,gauge,list,progress_wheel}.h`；对应 focused unit 与 target build proof 已通过
- `tinyui/demo/basic_widgets/main.c` 当前也已从 `picoui/*` include 集合切到顶层 `{button,checkbox,image,layout,runtime,slider,switch,text,widget,window}.h`；`tinyui/port/sdl/sdl.c` 当前也已切到 `port/sdl.h`，对应 demo/port focused build 与 `test_tinyui_port_sdl` 已通过
- `tinyui/src/widgets/{animation,background,button,calendar,checkbox,graph,image,label,progress_bar,text}.c` 当前也已从 `picoui/<widget>.h` 切到顶层同名头；对应 focused build 与相关 unit/demo proof 已通过
- `tinyui/src/widgets/{clock,date_time,list,qrcode,window}.c` 当前也已从 `picoui/*` include 集合切到顶层 `{clock,date_time,list,qrcode,widget,window}.h`；对应 focused build 与相关 unit/demo proof 已通过
- `tinyui/src/widgets/{arc,gauge,icon_slider,message_box,radial_menu}.c` 当前也已从 `picoui/*` include 集合切到顶层 `{arc,gauge,icon_slider,message_box,radial_menu,widget}.h`；对应 focused build 与相关 unit/demo proof 已通过
- `tinyui/src/widgets/{canvas,combo_box,keyboard,line_edit,progress_wheel,scroll_selecter,slider,switch,table}.c` 当前也已从 `picoui/*` include 集合切到顶层 `{canvas,combo_box,keyboard,line_edit,progress_wheel,scroll_selecter,slider,switch,table}.h`；对应 focused build、focused unit 与 `tinyui_basic_widgets_demo` proof 已通过
- `tests/tinyui/contract/{native_api_gap_ledger.json,tinyui_release_capability_matrix.json}` 与 `tests/picoui/contract/native_api_gap_ledger.json` 当前也已把 header evidence wording 从 `picoui/include/picoui/*.h` 同步到 `tinyui/include/*.h`；对应 canonical / legacy release-matrix 与 exhaustiveness gate 已重新通过
- `tests/tinyui/contract/tinyui_v21_transition_inventory.json` baseline 当前也已刷新到现状：`tinyui_public_header_count=44`，`compat_public_headers_require_followup` 当前只剩 `{picoui.h,sdl.h}`；current live checker 只走 `check_tinyui_v21_transition_guards.py`
- `tinyui/src/backend/ldgui/backend.h` 当前也已从 `picoui/{image,native,widget,window}.h` 切到顶层 `{image,native,widget,window}.h`；`check_tinyui_v21_transition_guards.py` baseline 当前新增 `backend_compat_includes=[]` 守卫，并已重新通过
- `check_tinyui_v21_transition_guards.py` baseline 当前也已新增 `top_level_wrapper_forward_count=43` 与 `top_level_wrapper_forward_names=*`，机器真相已明确记录哪些顶层头仍直接 `#include "picoui/..."`
- `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py` 当前也已切到 canonical `tinyui/include/*.h` 根路径，并通过单层 wrapper resolve 继续追到承载声明的 `picoui/*.h` 真头；legacy wrapper `check_picoui_widget_contract_matrix.py` 也已重新通过
- `tests/picoui/contract/check_picoui_widget_contract_matrix.py` 当前也已退场；canonical checker 只保留 `tests/tinyui/contract/check_tinyui_widget_contract_matrix.py`
- `tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py` 当前也已建立 canonical 入口，`tests/picoui/contract/check_picoui_native_api_exhaustiveness.py` 当前也已退场
- `tinyui/include/qrcode.h` 当前也已完成第一批真实迁出：不再 forward 到 `picoui/qrcode.h`，而是直接承载 canonical public 声明；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=43` 收窄到 `42`
- `tinyui/include/{app,display,indev,osal,tick,port}.h` 当前也已完成第二批真实迁出：不再 forward 到 `picoui/*`，而是直接承载 canonical public 声明或 canonical 聚合 include；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=42` 收窄到 `36`
- `tinyui/include/{background,arc,gauge,icon_slider,progress_wheel,radial_menu}.h` 当前也已完成第三批真实迁出：不再 forward 到 `picoui/*`，而是直接承载 canonical public 声明；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=36` 收窄到 `30`
- `tinyui/include/picoui/widget.h` 当前也已完成 compat include chain rewiring：内部 include 已从 `picoui/{layout,native,theme}.h` 切到顶层 `{layout,native,theme}.h`；这一步不减少 top-level wrapper 数量，但已为后续 `widget/layout/native/theme` 高耦合退场线清掉组合头回流路径
- `tinyui/include/layout.h` 当前也已完成第四批真实迁出第一步：不再 forward 到 `picoui/layout.h`，且 `tinyui/include/picoui/{widget,line_edit}.h` 当前也已改为显式走顶层 `../layout.h` / `../native.h` / `../theme.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=30` 收窄到 `29`
- `tinyui/include/{native,theme}.h` 当前也已完成第四批真实迁出第二步：不再 forward 到 `picoui/{native,theme}.h`，且 `tinyui/include/picoui/{canvas,table,widget}.h` 当前也已改为显式走顶层 `../native.h` / `../theme.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=29` 收窄到 `27`
- `tinyui/include/{progress_bar,clock}.h` 当前也已完成第四批真实迁出第三步：不再 forward 到 `picoui/{progress_bar,clock}.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=27` 收窄到 `25`
- `tinyui/include/{button,checkbox,label,text,graph}.h` 当前也已完成第四批真实迁出第四步：不再 forward 到对应 `picoui/*.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=25` 收窄到 `20`，`tinyui_public_api_count` 当前也已从 `26` 刷新到 `28`
- `tinyui/include/window.h` 当前也已完成第四批真实迁出第五步：不再 forward 到 `picoui/window.h`，且 `tinyui/include/picoui/{button,label,switch}.h` 当前也已改为显式走顶层 `../window.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=20` 收窄到 `19`
- `tinyui/include/picoui/{animation,button,calendar,checkbox,combo_box,date_time,graph,image,keyboard,label,line_edit,list,message_box,progress_bar,runtime,scroll_selecter,slider,switch,table,text}.h` 当前也已完成 compat widget include chain rewiring：内部 widget include 已从 `picoui/widget.h` 切到顶层 `../widget.h`；这一步不减少 top-level wrapper 数量，但已清掉 `widget/obj/runtime` 退场线的大部分 compat 回流面，同时 `tinyui/include/button.h` 当前也已补齐 `picoui_button_{set,get}_text_color` public 声明
- `tinyui/include/{widget,obj}.h` 当前也已完成第四批真实迁出第六步：`widget.h` 已不再 forward 到 `picoui/widget.h`，`obj.h` 当前也已改为复用顶层 `widget.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=19` 收窄到 `17`，当前 direct `picoui/widget.h` 代码依赖已只剩 legacy umbrella `tinyui/include/picoui/picoui.h`
- `tinyui/include/picoui/picoui.h` 当前也已完成 legacy umbrella 第一小步收口：已把 direct `widget/window` include 切到顶层 `../widget.h` 与 `../window.h`；当前主线代码面已无 direct `#include "picoui/widget.h"` / `#include "picoui/window.h"` 残留，下一步可直接转向 `runtime.h/screen.h`
- `tinyui/include/{runtime,screen,core}.h` 当前也已完成第四批真实迁出第七步：`runtime.h` 已不再 forward 到 `picoui/runtime.h`，`screen.h` 与 `core.h` 当前也已改为复用顶层 `runtime.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=17` 收窄到 `14`，`tinyui_public_api_count` 当前也已从 `28` 刷新到 `30`，当前 direct `picoui/runtime.h` 代码依赖已只剩 legacy umbrella `tinyui/include/picoui/picoui.h`
- `tinyui/include/picoui/picoui.h` 当前也已完成 legacy umbrella 第二小步收口：已把最后一条 direct `runtime` 入口切到顶层 `../runtime.h`；当前主线代码面已无 direct `#include "picoui/{widget,window,runtime}.h"` 残留，下一步可直接转向剩余 `14` 个 top-level wrapper
- `tinyui/include/animation.h` 当前也已完成剩余 wrapper 第一小批：不再 forward 到 `picoui/animation.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=14` 收窄到 `13`。同批摸底已确认：`image.h` 受 `picoui/canvas.h -> picoui/image.h` 组合面影响，需和 `canvas.h` 同批处理
- `tinyui/include/{combo_box,date_time}.h` 当前也已完成剩余 wrapper 第二小批：不再 forward 到 `picoui/{combo_box,date_time}.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=13` 收窄到 `11`，focused proof `test_tinyui_{combo_box,date_time,widgets}` 已通过；`image.h` 的剩余阻塞边界保持不变
- `tinyui/include/{keyboard,line_edit,list}.h` 当前也已完成剩余 wrapper 第三小批：不再 forward 到 `picoui/{keyboard,line_edit,list}.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=11` 收窄到 `8`，focused proof `test_tinyui_{keyboard,line_edit,list,widgets}` 已通过；`image.h` 的剩余阻塞边界保持不变
- `tinyui/include/{message_box,scroll_selecter,slider,table}.h` 当前也已完成剩余 wrapper 第四小批：不再 forward 到 `picoui/{message_box,scroll_selecter,slider,table}.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=8` 收窄到 `4`，focused proof `test_tinyui_{message_box,scroll_selecter,slider,table,widgets}` 已通过；当前剩余 wrapper 已收窄到 `calendar.h / canvas.h / image.h / switch.h`
- `tinyui/include/switch.h` 当前也已完成剩余 wrapper 第五小批：不再 forward 到 `picoui/switch.h`，并保留 `tinyui_switch_{create,set_checked,is_checked,set_on_toggled}` helper；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=4` 收窄到 `3`，`tinyui_public_api_count` 当前也已从 `30` 刷新到 `31`，focused proof `test_tinyui_{switch,widgets}` 已通过
- `canvas+image` 当前也已完成联批摸底：最小联动集合已收窄到 `tinyui/include/{canvas,image}.h + tinyui/include/picoui/canvas.h`，根因链路为 `tinyui.h -> canvas.h -> picoui/canvas.h -> picoui/image.h` 与 `tinyui.h -> image.h`
- `tinyui/include/{canvas,image}.h` 当前也已完成联批本体：不再 forward 到 `picoui/{canvas,image}.h`，且 `tinyui/include/picoui/canvas.h` 当前也已改为走顶层 `../image.h`；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=3` 收窄到 `1`，focused proof `test_tinyui_{canvas,image,animation,clock,widgets}` 已通过；当前剩余 wrapper 已只剩 `calendar.h`
- `tinyui/include/calendar.h` 当前也已完成最后一个 top-level wrapper：不再 forward 到 `picoui/calendar.h`，并改为直接承载 canonical public 声明；`check_tinyui_v21_transition_guards.py` baseline 当前已从 `top_level_wrapper_forward_count=1` 收窄到 `0`，focused proof `test_tinyui_{calendar,widgets}` 已通过；当前 top-level wrapper 已清零
- `tests/tinyui/contract/check_tinyui_public_api.py` 当前也已切到 canonical `tinyui/include/*.h` 根路径，并只扫描与 legacy compat surface 同名的顶层头，再通过单层 wrapper resolve 继续追到承载声明的 `picoui/*.h` 真头；旧 `check_picoui_public_api.py` 当前已退场
- `tests/picoui/contract/` 目录下与 canonical 完全一致的 `check_tinyui_{demo_boundary,public_api,release_capability_matrix,transition_guards,v21_transition_guards,widget_contract_matrix}.py` 重复兼容壳当前也已退场；CMake/live checker 当前只走 `tests/tinyui/contract/*`
- `tests/picoui/contract/` 目录下与 canonical 完全一致的 `check_picoui_{public_api,release_capability_matrix,native_100_inventory,native_api_exhaustiveness,tinyui_transition_guards,widget_contract_matrix}.py` thin wrapper 当前也已退场；legacy 目录剩余重点已进一步收敛到旧 JSON/ledger 资产
- compatibility subtree 当前也已进一步收窄：`tinyui/src/widgets/*` 与 `tinyui/src/backend/ldgui/backend.h` 都已不再直接 `#include "picoui/..."`；legacy widget-contract checker 也已不再自持 compat 头读取，当前主线代码面已无 direct `#include "picoui/{widget,window,runtime}.h"` 残留，剩余直接代码依赖主要只在 `13` 个 `tinyui/include/*.h` 顶层 wrapper 与 legacy umbrella 其余 compat include
- 当前又完成 demo-boundary residue 退场：`tests/picoui/contract/check_picoui_demo_boundary.py` 当前已退场；current live checker 只保留 canonical `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- 当前剩余重点已进一步收敛到两类：
  - `tests/picoui/contract/*` 旧 JSON/ledger 资产的最终定性与历史文档 wording 收口；其中 `tests/picoui/contract/README.md` 当前已明确这些旧资产只承担历史证据职责，不再属于 `v2.1` current truth，而 `check_ldgui_public_api_inventory.py` 当前也已只读取 canonical `tests/tinyui/contract/*`
  - `docs/v2.1`、`docs/ability/*`、历史文档里对 `picoui_*` 过渡态与 legacy asset 的 wording 是否与 `v2.1-release-matrix` 保持一致；其中 `docs/ability/README.md` 与各控件页总述当前已收口到 `TinyUI 当前覆盖` / `current public API` 口径，`docs/v2.0/*` 与 `docs/picoui-serial/H*` 已补齐“只对应历史阶段真相”的边界说明，`docs/picoui-serial/a-0.x*` 已把“当前真相源入口”降级为“本线历史真相源入口”，`a-0.3` 相关 spec/plan、部分 `superpowers` review/spec，以及 `a-0.3/a-0.4/a-0.5` 索引中的 `current truth-source / 当前 deep review / 当前 closeout review`、`a-0.6` 文档里的 `truth-source 入口 / release truth-source`、`H/J` 历史 `closeout 前状态` 里的“当前最终发布结论”也已限定回当时阶段语境；当前剩余重点主要转向 `docs/superpowers/*` 其他文件与其余历史文档残留
- `docs/superpowers/reviews/2026-05-31-picoui-a-0-3-closeout-review.md` 当前也已补完最后两处入口型措辞：`a-0.3` 只保留为当时 `a-0.4` 历史串行线输入，其“推荐的文档与执行入口”也已限定回当时语境

当前完成态：

- 与 canonical 完全一致的 `check_tinyui_*.py` / `check_picoui_*.py` compat shell 已退场
- `check_picoui_demo_boundary.py` 已退场
- `check_ldgui_public_api_inventory.py` 已切到 canonical `tests/tinyui/contract/*` 输入
- `tests/picoui/contract/README.md` 已明确旧 JSON/ledger 仅承担历史资产职责，不再属于 `v2.1` current truth

### Task 2: 建立 v2.1 closeout truth

**Files:**
- Create: `docs/v2.1/v2.1-closeout.md`
- Create: `docs/v2.1/v2.1-release-matrix.md`
- Create: `docs/v2.1/v2.1-performance-baseline.md`

- [x] **Step 1: 写 closeout 文档**

Document:

- what is completed
- what `v2.1` explicitly proves
- what is still outside scope
- the final gate commands

- [x] **Step 2: 写 release matrix**

Document at least:

- top-level directory unified to `tinyui/`
- product public API canonical truth unified to `tinyui` 口径
- independent backend removed
- shared layers retained intentionally
- tests/contracts/CMake renamed to `tinyui`
- `LingDongGUI` directories and `ld*` API left unchanged

- [x] **Step 3: 迁移 performance baseline 文档**

Move the `v2.0` performance truth into `v2.1` naming without weakening:

- binary size guard
- runtime perf guard
- wrapper struct overhead guard
- blocking policy

### Task 3: 跑 final gates 并更新索引

**Files:**
- Modify: `docs/v2.1/线计划索引.md`
- Modify: `docs/v2.1/plans/stages/README.md`

- [x] **Step 1: 跑 final broad gates**

Run:

```bash
rtk ctest --test-dir build -L 'tinyui' --output-on-failure
rtk ctest --test-dir build -L 'perf' --output-on-failure
rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure
git diff --check
```

Expected: PASS。

fresh 证据：

- `rtk ctest --test-dir build -L 'tinyui' --output-on-failure` -> `56/56 PASS`
- `rtk ctest --test-dir build -L 'perf' --output-on-failure` -> `3/3 PASS`
- `rtk ctest --test-dir build/tinyui-runtime -R 'check_tinyui_runtime|check_tinyui_visible_ui|check_tinyui_backend_mapping' --output-on-failure` -> `3/3 PASS`
- `git diff --check` -> PASS

- [x] **Step 2: 更新顶层索引**

Record in `docs/v2.1/线计划索引.md`:

- `V0-V5` completion
- final truth sources
- remaining non-goals

- [x] **Step 3: 更新阶段 README**

Record in `docs/v2.1/plans/stages/README.md`:

- `v2.1` 已完成 closeout / release truth
- completion means no current-truth residue is still被误记为 `picoui` 主线；不要求把 release matrix 已允许的过渡态 `picoui_*` / 历史资产命中机械清零
- completion still does not rename `LingDongGUI`

## 当前结论

- `V5` 当前已完成 closeout / release truth 收口
- `V0-V5` 当前已形成与仓库现状一致的完整阶段线
- 当前完成态仍明确保留以下 non-goal / allowed residue：
  - `LingDongGUI` 目录与 `ld*` public API 不改名
  - release matrix 已允许的 `picoui_*` 过渡态 public C API
  - `tinyui/include/picoui/*` compatibility subtree
  - `tests/picoui/*` 与历史文档所需的冻结资产入口
