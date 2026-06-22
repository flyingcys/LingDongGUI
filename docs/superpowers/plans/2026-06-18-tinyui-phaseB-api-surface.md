# TinyUI Phase B：公共 API 表面治理 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (- [ ]).

**Goal:** 落地 spec(`docs/superpowers/specs/tinyui-src-simplification.md` §4 Phase B / §7)的「L3 公共 API 表面治理」——删除冗余的 `_init` 别名、`obj.h` inline 别名层、空转发头，收敛 window padding 入口，统一启动路径到 `runtime.h`,并把 `slider.h` 的 `has_*` 显式标志改哨兵值。属**破坏性 API 变更但语义等价**:每删一个公共符号都必须先全仓搜调用方并迁移到等价的保留 API,不留悬空引用。

**Architecture:** 本相位绝大多数写面是**独立头文件 + 其对应 widget `.c`**,互不重叠,可多 subagent 并行。唯一的串行耦合点是 **`app.h` 导出移除 + 两个 transition-guard 基线更新**(`tinyui_public_api_count`/header 计数会随任意 API 删除一起 drift,基线 JSON 必须最后统一重算)。`_init` 删除以 **widget 为写面单元**(同一 widget 的 `.h` + `.c` + 其单测 + 两个 contract JSON 的该 widget 行,归同一 subagent)。window padding 收敛单独成一个写面(`window.h` + `window.c` + demo/test 调用方),且与 Phase C 有 `backend->window_layout` 重叠,需注明。

**Tech Stack:** C / CMake / CTest / Python contract gate / gitnexus

---

## 0. 起始状态与全局事实(执行前必读)

> 本相位 **独立于 Phase 0 / A / C**(spec §4:"Phase B 独立,可与 0/A 并行或任意穿插")。起始状态 = 其前置相位(若有)已合入;若单独执行,直接基于 `dev-nanoui` 当前 `tinyui/src`。

**已核实的关键事实(改动前请勿推翻,均亲自 Read/grep 坐实):**

1. **`_init` 别名实测共 17 个,本相位只删其中 7 个。** 其余 10 个(animation/arc/clock/date_time/gauge/icon_slider/message_box/progress_wheel/qrcode/radial_menu)**入参类型是 `struct tinyui_widget *parent`,不是 `_create` 的纯同义转发**,不在本相位范围,**严禁删除**。本相位的 7 个(button/calendar/graph/line_edit/progress_bar/slider/table)入参均为 `struct tinyui_window *parent`,与各自 `_create` 签名**逐字相同**,确认为纯转发别名:
   - `button.c:220-223` → `tinyui_button_create`
   - `calendar.c:467-470` → `tinyui_calendar_create`
   - `graph.c:312-317` → `tinyui_graph_create`(带 `int series_max`)
   - `progress_bar.c:348-351` → `tinyui_progress_bar_create`
   - `slider.c:238-241` → `tinyui_slider_create`
   - `table.c:500-506` → `tinyui_table_create`(带 `int rows, int columns`)
   - `line_edit`:**只有 `line_edit.h:41` 的声明,`line_edit.c` 内无定义、全仓无调用方**(declaration-only / 近死代码)。删它只动头声明,无 `.c` 改动。

2. **`obj.h` 的 13 个 `tinyui_obj_*` inline 别名(`obj.h:29-92`)全仓零调用方**(除 `obj.h` 自身与 contract JSON)。删除只需删 inline 块。**`obj.h` 文件本身保留**——它的 `tinyui_obj_t` typedef(`obj.h:24`)被 contract 允许(`check_tinyui_public_api.py:46 ALLOWED_COMPAT_TINYUI_TYPES`)且与 `widget.h:11` 同义并存;只删 inline,不删文件。

3. **`app.h` 文件保留,只从 `tinyui.h:23` 的 umbrella 里移除。** `app.h` 仍被 ~25 个 `tests/tinyui/unit/*.c`、`tinyui/src/core/internal.h`、`tinyui/src/core/app.c` 直接 `#include "app.h"`;`app_header_exists` 基线为 `true`,删文件会破坏这些消费者与基线。**demo 已 0 处使用 `tinyui_app_*` 旧 API**(已 grep 确认),所以从 umbrella 移除不破坏任何 demo。

4. **`core.h`(`core.h:22` → `runtime.h`)与 `screen.h`(`screen.h:22-23` → `obj.h`+`runtime.h`)是空转发头,可整文件删。** 但二者被两处硬依赖,删前必须同步处理:
   - `tinyui.h:22`(`#include "core.h"`)、`tinyui.h:53`(`#include "screen.h"`)——改为直接 `#include "runtime.h"`(`tinyui.h` 已在 `:52` 有 `runtime.h`,且 `:45` 有 `obj.h`、`:63` 有 `widget.h`,故删两行即可,无需新增)。
   - `check_tinyui_v21_transition_guards.py:18-27` 的 `TINYUI_INCLUDE_PROBE` **硬编码 `#include "core.h"` 与 `#include "screen.h"`**(脚本第 20、21 行);删头后该探针 `check_tinyui_headers_are_composable()` 会编译失败 → **必须同步删探针里这两行**(这是改脚本代码,不只是改基线 JSON)。

5. **window padding 入口现有 8 个**(`window.h:51-86`):`set_padding_group`(l,t,r,b)、`set_padding`(l,t,r,b)、`set_grid_padding`(l,t,r,b)、`get_padding_left/top/right/bottom`(4 个)、`get_padding_group`(*l,*t,*r,*b)。语义已核实(`window.c:267/298/385`):
   - `set_padding` → `apply_explicit_padding`,置 `has_explicit_flex_padding`,调 `ldWindowSetPadding`。
   - `set_grid_padding` → `apply_explicit_grid_padding`,置 `has_explicit_grid_padding`,调 `ldWindowSetGridPadding`——**这是 grid 布局独立能力,spec §3.4 明确保留 `grid_padding_*`+`has_explicit_grid_padding`,不可并入 flex padding。**
   - `set_padding_group` → `set_padding_group_impl`(写 `host->padding_group`)→ `apply_padding_contract`(综合 flex+grid),是 demo 唯一在用的入口(28+ demo 调用 + 多处 test)。
   - **收敛目标(2 个):保留 1 个 flex 入口 `set_padding(l,t,r,b)` + 1 个 get `get_padding_group`;删 `set_padding_group`(迁调用方到 `set_padding`)+ 4 个单 getter(迁到 `get_padding_group`)。** grid padding 入口(`set_grid_padding`)按 spec 视为独立能力**保留**(详见 Task B5 的「收敛决策点」,执行前必须先核实 `set_padding` 与 `set_padding_group` 的 `has_*`/contract 语义等价性,否则迁移会改坏布局)。

6. **三个 contract gate + 基线文件(已核实路径与机制):**
   - `tests/tinyui/contract/check_tinyui_public_api.py` → 校验 `tinyui/include/*.h` 前缀/禁词,并 `_assert_inventory_contract_rows()` 交叉校验 `ldgui_public_api_inventory.json` ↔ `native_api_gap_ledger.json`(行按 `ldgui_symbol` 配对,要求 `set` 相等;每 required 行须有 `tinyui_api`)。**删 `_init` 后,这两个 JSON 里 `"tinyui_api": "tinyui_<w>_init"` 必须改成 `_create`**(详见 Task B1)。
   - `tests/tinyui/contract/check_tinyui_transition_guards.py` → 基线 `tinyui_transition_inventory.json`,关键键 `tinyui_public_api_count`(当前 **584**,统计 `tinyui/include/*.h` 内 `\btinyui_\w*\s*\(` 计数)、`app_header_exists`(true,保持)。
   - `tests/tinyui/contract/check_tinyui_v21_transition_guards.py` → 基线 `tinyui_v21_transition_inventory.json`,关键键 `tinyui_public_api_count`(**584**)、`compat_public_header_count`/`tinyui_public_header_count`(均 **44**,删 core.h+screen.h 后变 42)。含 §0.4 的编译探针。
   - **CTest 名**(`ctest --test-dir build -N` 已确认):`check_tinyui_public_api`(#59)、`check_tinyui_demo_boundary`(#60)、`check_tinyui_transition_guards`(#61)、`check_tinyui_v21_transition_guards`(#63)。picoui 目录通过 `tests/picoui/CMakeLists.txt:81-89` **复用同一份 tinyui 脚本**(`../tinyui/contract/...`),读同一份基线 JSON,故只需改 tinyui 侧基线,两边 ctest 同时变绿。

7. **`check_tinyui_demo_boundary.py:82`** 在 `layout_parity` demo 的期望 API 白名单里列了 `tinyui_window_set_padding_group`。**Task B5 删/迁 `set_padding_group` 时,该白名单条目必须同步更新**(改成迁移后的 `set_padding`,且 `layout_parity.c` 的 5 处 `set_padding_group` 调用要改),否则 `check_tinyui_demo_boundary`(#60)失败。

8. **下游 contract 矩阵(范围边界,先做发现步、按需同步):** `_init` 名亦出现在 `check_tinyui_widget_contract_matrix.py` / `check_tinyui_release_capability_matrix.py` 消费的矩阵 JSON,以及 `tests/picoui/contract/` 的独立副本(`ldgui_public_api_inventory.json` / `native_api_gap_ledger.json` / `picoui_release_capability_matrix.json` 等,由 picoui 的 `check_ldgui_public_api_inventory.py` 校验)。这些不在 spec 列名的 8 项内,但属真实 drift 面;Task B7 用一个**发现步**枚举所有引用 `_init`/`set_padding_group` 的 contract 资产并按需同步,不遗漏。

**全局纪律(spec §5/§7):**
- 每删一个公共符号前,先 `gitnexus_impact({target:"<symbol>", direction:"upstream"})`,HIGH/CRITICAL 必须先报告主线程。
- 不用 bash `cat`/`head`/`sed`/`echo` 读内容(rtk 压缩失真);读真实内容用 Read;`rg`/`grep`/`wc` 可用(注意环境会把 `rg`/`git` 重写为 rtk,精确匹配时用 `/usr/bin/rg`、`/usr/bin/grep`)。
- 多 subagent 并行时写面**绝不重叠**;`app.h`/transition-guard 基线为串行收敛点。
- 每个 Task 收尾跑对应 contract/ctest;相位末跑全量回归(全量 `ctest` + 三个 gate + SDL demo 行为)。
- 提交前 `gitnexus_detect_changes()` 核对影响面只落在预期符号。

**并行/串行总览:**
- **可并行批(各 1 subagent,写面互不重叠):** B1-a..B1-g(7 个 widget 各自,但共享 2 个 contract JSON → 见 B1 的写面协调)、B2(`obj.h` inline)、B4(`slider.h` has_*)、B5(window padding)、B6(`core.h`/`screen.h` + `tinyui.h` + v21 探针)。
- **串行收敛(必须在以上全部合入后,单写者):** B3(`app.h` 从 umbrella 移除)+ B8(两个 transition-guard 基线统一重算)。**B8 必须最后做**,因为它依赖前面所有 API 删除后的最终计数。
- **B1 的 contract JSON 协调:** 7 个 widget 的 `.h`/`.c`/单测可并行,但 `ldgui_public_api_inventory.json` + `native_api_gap_ledger.json` 是**两个共享文件**(7 个 widget 的行都在里面)→ 这两个 JSON 的编辑**收归一个 subagent(或主线程)串行批改**,不要让 7 个 widget subagent 各自写同一 JSON。

---

## Task B1 — 删 7 个 widget 的 `_init` 别名(按 widget 为写面单元)

**Files:**
- 头声明:`tinyui/include/button.h:67`、`calendar.h:25`、`graph.h:24`、`line_edit.h:41`、`progress_bar.h:24`、`slider.h:45`、`table.h:43`
- `.c` 定义:`tinyui/src/widgets/button.c:220-223`、`calendar.c:467-470`、`graph.c:312-317`、`progress_bar.c:348-351`、`slider.c:238-241`、`table.c:500-506`(**`line_edit.c` 无定义,跳过 .c**)
- 单测调用方(仅 4 处):`tests/tinyui/unit/test_tinyui_slider.c:214-215`、`test_tinyui_progress_bar.c:371`、`test_tinyui_table.c:871`(button/calendar/graph/line_edit 单测无 `_init` 调用)
- 共享 contract JSON(串行批改):`tests/tinyui/contract/ldgui_public_api_inventory.json`、`tests/tinyui/contract/native_api_gap_ledger.json`(`"tinyui_api"` 字段:button 行 1860/2046 & ledger 2927/3214、calendar 2266 & ledger 3557、graph 4689 & ledger 7067、progress_bar 7507/7767 & ledger 10945/11342、slider 8799/9042 & ledger 12912/13283、table 10001/10306 & ledger 14507/14976;line_edit 在这两 JSON 中无行)

**每个 widget 子任务步骤(`.h`+`.c`+单测 视为同一写面;以 button 为模板,calendar/graph/progress_bar/slider/table 同构,line_edit 仅删头声明):**
- [ ] `gitnexus_impact({target:"tinyui_button_init", direction:"upstream"})`,记录 blast radius;若 HIGH/CRITICAL 先报告。
- [ ] 全仓搜调用方:`/usr/bin/rg -n 'tinyui_button_init\b' tinyui tests`,确认调用方集合(预期只在 .h/.c/contract JSON;含单测的 widget 另含单测行)。
- [ ] 删头声明(`button.h:67` 整行)。
- [ ] 删 `.c` 定义(`button.c:220-223` 整个函数 + 其上方 doc 注释块)。
- [ ] 迁单测调用方(若该 widget 有):把 `tinyui_<w>_init(...)` 原地改名为 `tinyui_<w>_create(...)`(签名逐字相同,纯改名)。例:`test_tinyui_slider.c:214-215` 的 `tinyui_slider_init` → `tinyui_slider_create`;`test_tinyui_progress_bar.c:371`、`test_tinyui_table.c:871` 同理。
- [ ] line_edit 专属:仅删 `line_edit.h:41`;`/usr/bin/grep -rn 'tinyui_line_edit_init' tinyui tests` 必须只剩 0 命中(它本就无定义无调用)。
- [ ] **共享 JSON 串行批改(由统一 subagent/主线程一次性做,不在 7 个并行 subagent 内各写):** 在 `ldgui_public_api_inventory.json` + `native_api_gap_ledger.json` 中,把上列各行 `"tinyui_api": "tinyui_<w>_init"` 改为 `"tinyui_<w>_create"`;`equivalence_proof`/`unit_test` 文本里出现的 `<w>_init` 措辞按需改成 `<w>_create`(保持两文件一致,行的 `ldgui_symbol` 主键不变)。
- [ ] 跑 contract:`ctest --test-dir build -R check_tinyui_public_api --output-on-failure`,预期 `Passed`(`_assert_inventory_contract_rows` 不再因 `_init` 失配报错)。

**Run(整个 B1 完成后):**
- [ ] `/usr/bin/rg -n 'tinyui_(button|calendar|graph|line_edit|progress_bar|slider|table)_init\b' tinyui tests` → **0 命中**(JSON 也已迁移)。
- [ ] `cmake --build build -j` → 链接通过(无未定义符号引用)。
- [ ] `ctest --test-dir build -R 'test_tinyui_(slider|progress_bar|table|button|calendar|graph|line_edit)' --output-on-failure` → 全 `Passed`。
- [ ] `gitnexus_detect_changes()` → 影响面只含上述 7 widget 符号 + contract 行。
- [ ] commit:`refactor(tinyui/api): drop 7 redundant _init aliases, migrate callers to _create`(尾部带 Co-Authored-By)。

---

## Task B2 — 删 `obj.h` 的 13 个 `tinyui_obj_*` inline 别名(并行,独立写面)

**Files:** `tinyui/include/obj.h:29-92`(13 个 `static inline`:set_pos/set_size/set_text/set_style_class/set_user_data/set_bg_color/set_text_color/set_border_color/set_radius/set_padding/set_center/set_visible/destroy)。**保留 `obj.h:1-28`(license + `#include "widget.h"` + `obj.h:24-27` 的 4 个 typedef)与 `obj.h:94 #endif`。**

**Steps:**
- [ ] `gitnexus_impact({target:"tinyui_obj_set_pos", direction:"upstream"})`(抽样代表),确认别名层无生产消费方。
- [ ] 全仓搜调用方:`/usr/bin/rg -n 'tinyui_obj_(set_pos|set_size|set_text|set_style_class|set_user_data|set_bg_color|set_text_color|set_border_color|set_radius|set_padding|set_center|set_visible|destroy)\b' tinyui tests` 排除 `obj.h`/`*.json` → 预期 **0 命中**(已核实)。无调用方 → 无迁移步。
- [ ] 删 `obj.h:29-92`(整 13 个 inline 块),保留 typedef 段与 `#endif`。
- [ ] 确认 `tinyui_obj_t` typedef 仍在(`obj.h:24` 保留;`widget.h:11`/`runtime.h` 亦提供),`check_tinyui_public_api.py` 的 `ALLOWED_COMPAT_TINYUI_TYPES={"tinyui_obj_t"}` 不受影响。
- [ ] 若 contract JSON 中存在 `tinyui_obj_*` 行(`/usr/bin/grep -rln 'tinyui_obj_set\|tinyui_obj_destroy' tests/tinyui/contract/*.json`),同步处理(预期无;有则删行并保持 inventory/ledger 配对)。

**Run:**
- [ ] `cmake --build build -j` → 通过(obj 别名无人调用,删除不破坏链接)。
- [ ] `ctest --test-dir build -R check_tinyui_public_api --output-on-failure` → `Passed`。
- [ ] commit:`refactor(tinyui/api): remove obj.h tinyui_obj_* inline alias layer`。

> 注:本 Task 改的 `tinyui_public_api_count` 由 B8 统一重算基线;此处不单独改 transition 基线,避免与 B8 冲突。

---

## Task B4 — `slider.h` 的 `has_*` 显式标志改哨兵值(并行,独立写面)

**Files:** `tinyui/include/slider.h:32-36`(`has_horizontal`/`has_background_source`/`has_indicator_source`/`has_indicator_width`/`has_slim_size` 5 个 int 标志);消费方仅 `tinyui/src/widgets/slider.c:122-130`、`slider.c:271-279`(已核实,`props->has_* == 0` / `!= 0` 判断)。

**设计(哨兵值替代显式 has_ 标志):** `slider_props` 把"用户是否设置了该可选字段"从外挂的 `has_*` int,改为给对应业务字段一个**不可能的哨兵默认值**(如 `horizontal`/`indicator_width`/`slim_size` 用 `-1`/`INT_MIN` 之类 out-of-range 默认;`background_source`/`indicator_source` 本是指针,`NULL` 即天然哨兵)。消费侧判断从 `props->has_X` 改为 `props->X != <哨兵>`。

**Steps:**
- [ ] `gitnexus_impact({target:"tinyui_slider_create_with_props", direction:"upstream"})`(`has_*` 的真实读点在 create_with_props 路径)。
- [ ] 全仓搜显式标志读写:`/usr/bin/rg -n 'has_horizontal|has_background_source|has_indicator_source|has_indicator_width|has_slim_size' tinyui tests`,登记所有读/写点(含测试是否构造 `props.has_* = 1`)。
- [ ] 选定每个可选字段的哨兵值(指针字段用 `NULL`;`indicator_width`/`slim_size`/`horizontal` 选 `<0` 哨兵,与合法值域核对——`horizontal` 是布尔,合法 0/1,哨兵取 `-1`)。在 `slider.h` 注释里写明哨兵约定。
- [ ] 删 `slider.h:32-36` 的 5 个 `has_*` 字段。
- [ ] 改 `slider.c:122-130`、`271-279`:把 `props->has_X` 判断替换为 `props->X != <哨兵>`(逐字段核对真值表,保持原"未设置则用默认/不下发"行为不变,**这是行为等价的核心**)。
- [ ] 改测试构造点:凡构造 `slider_props` 时显式 `has_X=1`/`=0` 的,改为设/不设对应字段值(或显式置哨兵)。
- [ ] 若 `slider_props` 在 contract JSON 有 `direct_field_parity` 行涉及 `has_*`,同步更新(`/usr/bin/grep -n 'has_horizontal\|has_slim_size' tests/tinyui/contract/*.json`)。

**Run:**
- [ ] `cmake --build build -j` → 通过。
- [ ] `ctest --test-dir build -R test_tinyui_slider --output-on-failure` → `Passed`(value/range/percent/image/color/horizontal 行为不变)。
- [ ] commit:`refactor(tinyui/slider): replace slider_props has_* flags with sentinel defaults`。

> 风险提示:`has_*` 是"用户显式设置"语义的唯一来源;哨兵化后必须保证"用户显式传 0/默认值"与"未传"在哨兵下可区分。`horizontal` 布尔字段尤其要核对——若用户合法地想设 `horizontal=0`(纵向)且哨兵取 0 会冲突,故哨兵必须取**值域外**值(如 `-1`)。这是本 Task 的判断点,改前在 subagent 内核对真值表;review 不过在同一 subagent 内修复。

---

## Task B5 — window padding 入口 8 → 2(并行,独立写面;与 Phase C 有重叠提示)

**Files:**
- 头:`tinyui/include/window.h:51-86`(8 个 padding 入口)
- 实现:`tinyui/src/widgets/window.c:1102`(`set_padding_group`)、`:1151`(`set_padding`)、`:1179`(`set_grid_padding`)、`:1221/1243/1265/1287`(4 个单 getter)、`get_padding_group`;底层 `:267 apply_explicit_padding`、`:298 apply_explicit_grid_padding`、`:385 set_padding_group_impl`、`:410 apply_padding_contract`
- demo 调用方(28+ 处 `set_padding_group`,全仓):`tinyui/demo/*/.c`(arc_basic/calendar_basic/clock_basic/combo_box_basic/date_time_basic/gauge_basic/graph_basic/grid_parity(×2)/icon_slider_basic/layout_parity(×5)/line_edit_basic/progress_bar_basic/progress_wheel_basic/qrcode_basic/radial_menu_basic/scroll_selecter_basic/table_basic/basic_widgets — 逐一以 grep 结果为准)
- test 调用方:`tests/tinyui/unit/test_tinyui_layout.c`(多处 `set_padding_group`/`set_padding`/`set_grid_padding`/4 getter)、`test_tinyui_window.c:252/302`、`test_tinyui_widgets.c:2515-2518`(4 getter)
- contract 白名单:`check_tinyui_demo_boundary.py:82`(`layout_parity` 期望含 `tinyui_window_set_padding_group`)

**收敛决策点(执行前必须先核实,再选方案):**
- [ ] **先核实 `set_padding`(`apply_explicit_padding`,置 `has_explicit_flex_padding`)与 `set_padding_group`(`set_padding_group_impl`→`apply_padding_contract`,写 `host->padding_group`)是否对最终 ld 输出语义等价。** 读 `window.c:410 apply_padding_contract` 完整体,确认 `set_padding_group` 是否只是"flex+grid 综合下发"而 `set_padding` 是"仅 flex 显式"。
  - 若**等价(同样下发到 `ldWindowSetPadding`,grid 维度不受影响)**:按方案 A 收敛。
  - 若**不等价**(`set_padding_group` 还联动 grid 维度):保留 `set_padding_group` 语义到保留的那个入口里,或在迁移时补齐——**不得用改 demo 坐标/补假视觉掩盖差异**(CLAUDE.md 红线)。把判断结论写进 commit message。

**方案 A(默认,2 个入口):保留 `set_padding(l,t,r,b)` + `get_padding_group`;`set_grid_padding` 按 spec §3.4 作为 grid 独立能力保留(不计入"被收敛的 8 个 flex padding 入口",视为 padding 域的第 3 个但属 grid 维度)。** 删:`set_padding_group`(迁→`set_padding`)、`get_padding_left/top/right/bottom`(迁→`get_padding_group`)。

> 说明:spec 把 grid padding(`grid_padding_*` + `has_explicit_grid_padding`)列为**保留字段**,故 `set_grid_padding` 不可删。"8→2"的字面收敛落在 **flex padding 那一组**(set_padding_group + set_padding 合一 + 4 单 getter 合一 = 6 个塌成 2 个);grid padding 入口独立保留。这一点在 Task header 与 commit 里写清,避免被误读为"漏删一个"。

**Steps:**
- [ ] `gitnexus_impact({target:"tinyui_window_set_padding_group", direction:"upstream"})`——这是 demo 广用 API,预期 blast radius 大(MEDIUM/HIGH),**先报告主线程**再动。
- [ ] 全仓搜调用方(已 grep,以执行时结果为准):`/usr/bin/rg -n 'tinyui_window_(set_padding_group|get_padding_left|get_padding_top|get_padding_right|get_padding_bottom)\b' tinyui tests`。
- [ ] 迁 demo:把所有 `tinyui_window_set_padding_group(win, a,b,c,d)` 改为 `tinyui_window_set_padding(win, a,b,c,d)`(签名同形)。逐文件改,grep 复核 0 残留。
- [ ] 迁 test:`test_tinyui_layout.c`/`test_tinyui_window.c` 的 `set_padding_group` 调用 → `set_padding`;4 个单 getter 断言(`test_tinyui_layout.c` 多处、`test_tinyui_widgets.c:2515-2518`)改为通过 `get_padding_group(win,&l,&t,&r,&b)` 取值后断言 `l/t/r/b`。
- [ ] 删头声明:`window.h` 的 `set_padding_group`、4 个单 getter(`get_padding_left/top/right/bottom`),保留 `set_padding`、`set_grid_padding`、`get_padding_group`、`set_layout_type`/`set_gap`/`get_layout_type`/`get_gap` 等非 padding 入口。
- [ ] 删 `window.c` 对应定义(`set_padding_group:1102`、4 单 getter:1221/1243/1265/1287);内部 `set_padding_group_impl`/`apply_padding_contract` 若仅被 `set_padding_group` 用则一并删,若 `create_with_props`/`set_padding` 仍依赖则保留(grep `set_padding_group_impl`/`apply_padding_contract` 调用方后定)。
  - **注意 `window.c:892`** 有 `&& tinyui_window_set_padding_group(window, ...)` 的内部调用(props 路径),迁到 `set_padding` 或直接调内部 impl,不能留悬空。
- [ ] 更新 contract:`check_tinyui_demo_boundary.py:82` 把 `"tinyui_window_set_padding_group"` 改为 `"tinyui_window_set_padding"`。
- [ ] **Phase C 重叠提示(写进 commit + 给主线程):** `apply_explicit_padding`/`apply_explicit_grid_padding`/`apply_padding_contract`/`set_padding_group_impl` 直接读写 `backend->window_layout.*` 与 `host->padding_group`。Phase C(C1/C3)会删 `tinyui_backend_widget` 并把 padding 字段迁入 `tinyui_window`。**若 B5 在 C 之前合入,按当前 backend 模型改;C 阶段会再次重写这些函数体。** B5 只做"入口收敛 + 调用方迁移",不预先做 backend 字段迁移(那是 C 的写面)。两阶段写面在 `window.c` 有重叠 → B 与 C 不要同时并行改 `window.c`,由主线程串接顺序。

**Run:**
- [ ] `/usr/bin/rg -n 'tinyui_window_set_padding_group\b|tinyui_window_get_padding_(left|top|right|bottom)\b' tinyui tests` → **0 命中**。
- [ ] `cmake --build build -j` → 通过(含全部 demo target)。
- [ ] `ctest --test-dir build -R 'test_tinyui_(layout|window|widgets)' --output-on-failure` → `Passed`。
- [ ] `ctest --test-dir build -R check_tinyui_demo_boundary --output-on-failure` → `Passed`。
- [ ] **SDL demo 行为核对**(spec §5 红线:UI 完成须基于真实 LingDongGUI 输出):跑受影响 demo(如 `layout_parity`/`grid_parity`/`basic_widgets`)对比改前 padding 视觉一致。
- [ ] commit:`refactor(tinyui/window): collapse flex padding entries to set_padding + get_padding_group`。

---

## Task B6 — 删空转发头 `core.h`/`screen.h` + 修 umbrella + 修 v21 探针(并行,独立写面)

**Files:**
- 删:`tinyui/include/core.h`(整文件)、`tinyui/include/screen.h`(整文件)
- 改 umbrella:`tinyui/include/tinyui.h:22`(删 `#include "core.h"`)、`tinyui.h:53`(删 `#include "screen.h"`)——`runtime.h`(`:52`)、`obj.h`(`:45`)、`widget.h`(`:63`)已在 umbrella 内,无需补
- 改探针:`tests/tinyui/contract/check_tinyui_v21_transition_guards.py:18-27` 的 `TINYUI_INCLUDE_PROBE`,删第 20 行 `#include "core.h"`、第 21 行 `#include "screen.h"`

**Steps:**
- [ ] `gitnexus_impact({target:"core.h", direction:"upstream"})` 与 `screen.h`(若 gitnexus 不索引头文件,改用 `/usr/bin/rg -rn '#include\s+"(core|screen)\.h"' tinyui tests`)。
- [ ] 全仓搜直接 includer:`/usr/bin/rg -rn '#include\s+"(core|screen)\.h"' tinyui tests`。已核实结果:仅 `tinyui.h:22/53` 与 v21 探针(`check_tinyui_v21_transition_guards.py:21/22`)。**demo 与 src 均不直接 include 这两个头**(已确认)。
- [ ] 删 `tinyui.h:22` 与 `tinyui.h:53` 两行。
- [ ] 删探针的两行 include(`check_tinyui_v21_transition_guards.py` 内 `TINYUI_INCLUDE_PROBE` 字符串里的 `core.h`/`screen.h`)。
- [ ] `git rm tinyui/include/core.h tinyui/include/screen.h`(或 Write 工具无法删文件 → 用 Bash `rm`;删后用 `/usr/bin/grep -rln '"core.h"\|"screen.h"'` 复核无残留 include)。
- [ ] 复核 `runtime.h` 仍可独立编译(`check_tinyui_public_api.py::check_legacy_runtime_header_compiles` 编译 `#include "runtime.h"`,而 `runtime.h` 经 `widget.h` 拿到 `tinyui_obj_t`,不依赖 core.h/screen.h → 不受影响)。

**Run:**
- [ ] `cmake --build build -j` → 通过(`tinyui.h` 仍提供等价符号集)。
- [ ] `ctest --test-dir build -R 'check_tinyui_public_api|check_tinyui_v21_transition_guards' --output-on-failure`:
   - `check_tinyui_public_api` → `Passed`(headers glob 少 2 个文件不影响前缀校验)。
   - `check_tinyui_v21_transition_guards` → **此处可能因 header 计数 drift 报 baseline drift**(`compat_public_header_count`/`tinyui_public_header_count` 44→42)。这是**预期**的,由 Task B8 统一重算基线后转绿;本 Task 只需确认 `check_tinyui_headers_are_composable()` **不再因缺 core.h/screen.h 编译失败**(即失败信息是 baseline drift 而非 probe 编译错误)。
- [ ] commit:`refactor(tinyui/include): delete empty core.h/screen.h forwarders, fix umbrella + v21 probe`。

---

## Task B3 — `app.h` 移出 `tinyui.h` 公共导出(串行收敛点之一)

**Files:** `tinyui/include/tinyui.h:23`(删 `#include "app.h"`);**不删 `app.h` 文件**(仍被 ~25 个单测 + `internal.h` + `app.c` 直接 include)。

**Steps:**
- [ ] `gitnexus_impact({target:"tinyui_app_create", direction:"upstream"})`(代表性旧 API),确认旧启动路径消费方分布。
- [ ] 全仓确认无 demo 依赖旧 API:`/usr/bin/rg -n 'tinyui_app_(create|run|run_background|set_window|set_background|switch_window|switch_background|timer_create|timer_start|timer_stop|destroy)\b' tinyui/demo` → 预期 **0**(已核实)。**若有残留(意外),先迁 demo 到 `runtime.h` 的 `tinyui_init/screen_create/screen_load/timer_handler` 链**,再移除导出。
- [ ] 确认直接 includer 仍能编译:单测/`internal.h`/`app.c` 用 `#include "app.h"`,文件保留 → 不受 umbrella 改动影响。
- [ ] 删 `tinyui.h:23` 的 `#include "app.h"`。
- [ ] 复核 `tinyui.h` 不再传递导出 `tinyui_app_*`:`/usr/bin/grep -n 'app.h' tinyui/include/tinyui.h` → 0;`cc -fsyntax-only -I tinyui/include` 编译一个仅 `#include "tinyui.h"` 的探针,确认仍 OK 且不暴露旧 API(可选)。

**Run:**
- [ ] `cmake --build build -j` → 通过(demo 不依赖旧 API;单测仍直接 include app.h)。
- [ ] `ctest --test-dir build -R 'test_tinyui_app_' --output-on-failure` → `Passed`(`test_tinyui_app_lifecycle`/`app_timer`/`app_window_switch` 直接 include app.h,行为不变)。
- [ ] `gitnexus_detect_changes()` 核对只动 `tinyui.h`。
- [ ] commit:`refactor(tinyui/api): remove app.h from public umbrella, route users to runtime.h`。

> 串行原因:本 Task 与 B8 一起影响 `tinyui_public_api_count`/transition 基线;且应在 B1/B2/B5/B6 全部合入后做,使 B8 一次性重算最终基线。`app_header_exists` 保持 `true`(文件未删),该基线键不变。

---

## Task B7 — 下游 contract 资产同步发现步(串行,紧接 B1/B5 后)

**Files(发现后按需):** `tests/tinyui/contract/*.json` 与 `*.py`(widget_contract_matrix / release_capability_matrix);`tests/picoui/contract/*.json`(独立副本)与 `check_ldgui_public_api_inventory.py`。

**Steps:**
- [ ] 枚举所有引用被删/迁符号的 contract 资产:
   - `/usr/bin/rg -rln 'tinyui_(button|calendar|graph|line_edit|progress_bar|slider|table)_init\b' tests`
   - `/usr/bin/rg -rln 'tinyui_window_set_padding_group\b|tinyui_obj_set' tests`
- [ ] 对每个命中文件,判断它是否被某个 ctest gate 校验(查 `tests/*/CMakeLists.txt` 的 `ld_add_python_test`);被校验的才需改。
- [ ] 跑全部 contract gate 找出真实 drift:`ctest --test-dir build -L contract --output-on-failure`,逐个失败项定位是否源于 B1/B5 的符号变更。
- [ ] 对真实失败的矩阵 JSON / picoui 副本,按其校验规则同步(`_init`→`_create`、`set_padding_group`→`set_padding`),保持各自 inventory/ledger 行配对一致。**不动与本相位无关的行。**

**Run:**
- [ ] `ctest --test-dir build -L contract --output-on-failure` → 除 transition-guard(留给 B8)外全 `Passed`。
- [ ] commit:`test(contract): sync downstream matrices/picoui inventories after Phase B api changes`。

> 范围边界:本 Task 只处理"因 B1/B5 删/迁符号而真实失败"的 contract 资产;不重写无关矩阵。若发现 picoui 侧需要大改且超出 spec 列名 8 项,**先报告主线程裁决**是否纳入本相位。

---

## Task B8 — 两个 transition-guard 基线统一重算(串行收敛点,**最后做**)

**Files:** `tests/tinyui/contract/tinyui_transition_inventory.json`(baseline)、`tests/tinyui/contract/tinyui_v21_transition_inventory.json`(baseline)。脚本本身不改(B6 已改 v21 探针)。

**前置:** B1/B2/B3/B5/B6/B7 全部已合入(任何 `tinyui/include/*.h` 的 API 增删都会改 `tinyui_public_api_count`,故必须等所有头改完再重算,避免反复改基线)。

**Steps:**
- [ ] `gitnexus_impact` 不适用(纯数据基线);跳过。
- [ ] 用脚本自带的"当前值"打印能力生成新基线快照:
   - v21:`python3 tests/tinyui/contract/check_tinyui_v21_transition_guards.py --print-current`(脚本支持 `--print-current`,见 `:139-143`),取输出 JSON。
   - 普通 guard:无 `--print-current`,直接运行让其报 `baseline drift: <key> expected X actual Y`,记录 stderr 里打印的 `actual` JSON(脚本在 drift 时会 `print(json.dumps(actual...))`,见 `:80`)。
- [ ] 把两个 inventory JSON 的 `baseline` 对象按新 `actual` 值更新:
   - `tinyui_transition_inventory.json`:更新 `tinyui_public_api_count`(从 584 → 重算值:删 7 `_init`、13 `obj` inline、padding 收敛净减、移 app.h 不计入因 app.h 本就在 umbrella 外不被该脚本单独统计……以脚本实算为准),`app_header_exists` 仍 `true`、`app_source_exists` 仍 `true`、`backend_c_files` 不变(0)。
   - `tinyui_v21_transition_inventory.json`:更新 `tinyui_public_api_count`、`compat_public_header_count`(44→42)、`tinyui_public_header_count`(44→42)、`compat_public_headers_require_followup`(若 core.h/screen.h 曾在其中则移除)、`top_level_wrapper_forward_*`(预期仍 0);`tinyui_dir_exists`/`backend_c_files`/`backend_compat_includes` 不变。
- [ ] **不手算计数**,完全以脚本 `--print-current` / drift 输出的 `actual` 为准填回 baseline(避免 off-by-N)。

**Run:**
- [ ] `ctest --test-dir build -R 'check_tinyui_transition_guards|check_tinyui_v21_transition_guards' --output-on-failure` → 两者均 `Passed`(打印 `tinyui transition baseline guard OK` / `tinyui v2.1 transition baseline guard OK`)。
- [ ] picoui 侧同名 ctest(复用同脚本同基线)亦绿:`ctest --test-dir build -R 'check_tinyui_v21_transition_guards' --output-on-failure` 覆盖两处注册。
- [ ] `gitnexus_detect_changes()` 核对只动两个 inventory JSON。
- [ ] commit:`test(contract): rebaseline transition guards after Phase B api-surface changes`。

---

## 相位验收 Checklist(全部满足才算 Phase B 完成)

- [ ] `/usr/bin/rg -n 'tinyui_\w+_init\b' tinyui/include` → **仅余非别名项**:即只剩那 10 个 `struct tinyui_widget *parent` 入参的 widget `_init`(animation/arc/clock/date_time/gauge/icon_slider/message_box/progress_wheel/qrcode/radial_menu),**7 个纯转发别名全部消失**;`tinyui_line_edit_init` 0 命中。
- [ ] `/usr/bin/rg -n 'tinyui_obj_(set|destroy)\w*\s*\(' tinyui/include/obj.h` → 0(inline 别名层已删,typedef 段保留)。
- [ ] `/usr/bin/grep -n 'app.h' tinyui/include/tinyui.h` → 0;`app.h` 文件仍存在(`ls tinyui/include/app.h`)。
- [ ] `ls tinyui/include/core.h tinyui/include/screen.h` → 两者均不存在;`/usr/bin/rg -rn '#include\s+"(core|screen)\.h"' tinyui tests` → 0。
- [ ] `/usr/bin/rg -n 'tinyui_window_set_padding_group\b' tinyui tests` → 0;window padding 入口收敛为 `set_padding`+`get_padding_group`(+ grid 独立保留的 `set_grid_padding`)。
- [ ] `/usr/bin/rg -n 'has_horizontal|has_slim_size' tinyui/include/slider.h` → 0(已改哨兵)。
- [ ] **demo 全用 canonical 路径**:`/usr/bin/rg -n 'tinyui_app_(create|run|set_window|switch_)' tinyui/demo` → 0;demo 仍仅 `#include "tinyui.h"`/`runtime.h`,无 `ld*`/`arm_2d_*`/`SIGNAL_*` 新增泄漏(`check_tinyui_demo_boundary` 绿)。
- [ ] **三个 gate 全绿**:`ctest --test-dir build -R 'check_tinyui_public_api|check_tinyui_transition_guards|check_tinyui_v21_transition_guards|check_tinyui_demo_boundary' --output-on-failure` → 全 `Passed`(transition-guard 打印 `...baseline guard OK`)。
- [ ] **全量回归**:`ctest --test-dir build --output-on-failure` → 全 `Passed`。
- [ ] **真实 UI 证据**(spec §5):受 padding 收敛影响的 SDL demo(layout_parity / grid_parity / basic_widgets)运行输出与改前逐页一致(非 smoke/fake)。
- [ ] `gitnexus_detect_changes()` 总核对:影响面只落在本相位预期符号与 contract 行,无意外溢出。
