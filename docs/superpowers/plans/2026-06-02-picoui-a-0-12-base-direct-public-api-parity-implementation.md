# TINYUI a-0.12 Base Direct Public API Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现 `base` 下 16 个 strict direct public API parity 缺口，让 TINYUI 对外 API 不再停留在 a-0.11 的缺口候选状态。

**Architecture:** 先新增 portable public API 与 unit proof，再同步 backend/native proof、contract JSON 和 ability 文档。所有实现都通过 TINYUI 类型表达，不泄漏 LingDongGUI native types。

**Tech Stack:** C、TINYUI public headers、LingDongGUI backend、Python contract checker、Markdown docs、GitNexus

---

## 0. 执行规则

- 当前按用户要求直接在主仓推进。
- 修改函数前必须跑 GitNexus impact；若 GitNexus 无法解析新 symbol，记录已对相邻入口或文件上下文做影响分析。
- 不实现 `policy_never_public=191`。
- 每个从 `allowlisted` 改为 `covered` 的 row 必须有 public API、backend proof、unit/gate、docs。

## 1. 文件写面

**Create:**
- `docs/tinyui-serial/a-0.12-线计划索引.md`
- `docs/superpowers/specs/2026-06-02-tinyui-a-0-12-base-direct-public-api-parity-design.md`
- `docs/superpowers/plans/2026-06-02-tinyui-a-0-12-base-direct-public-api-parity-implementation.md`

**Modify:**
- `tinyui/include/tinyui/widget.h`
- `tinyui/src/core/widget.c`
- `tinyui/src/backend/ldgui/backend_widget_tree.c`
- `tinyui/src/backend/ldgui/backend.h`
- `tests/tinyui/unit/test_tinyui_layout.c`
- `tests/tinyui/contract/ldgui_public_api_inventory.json`
- `tests/tinyui/contract/native_api_gap_ledger.json`
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`
- `docs/ability/README.md`
- `docs/ability/base.md`

## 2. Tasks

### Task T0: Spec Plan Index

- [x] **Step 1: Create a-0.12 serial index**

Create `docs/tinyui-serial/a-0.12-线计划索引.md`.

- [x] **Step 2: Create a-0.12 spec**

Create `docs/superpowers/specs/2026-06-02-tinyui-a-0-12-base-direct-public-api-parity-design.md`.

- [x] **Step 3: Create a-0.12 plan**

Create `docs/superpowers/plans/2026-06-02-tinyui-a-0-12-base-direct-public-api-parity-implementation.md`.

### Task T1: Public API Header

- [x] **Step 1: Add portable geometry structs and widget type enum**

Modify `tinyui/include/tinyui/widget.h`.

- [x] **Step 2: Add tree/name/type public declarations**

Add:

```c
struct tinyui_widget *tinyui_widget_get_parent(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_first_child(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_next_sibling(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_root(const struct tinyui_widget *widget);
int tinyui_widget_get_child_count(const struct tinyui_widget *widget);
int tinyui_widget_get_name_id(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_find_by_name_id(const struct tinyui_widget *root, int name_id);
enum tinyui_widget_type tinyui_widget_get_type(const struct tinyui_widget *widget);
```

- [x] **Step 3: Add geometry/focus public declarations**

Add:

```c
struct tinyui_point tinyui_widget_get_absolute_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point);
struct tinyui_point tinyui_widget_get_relative_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point);
struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent,
                                     struct tinyui_rect child,
                                     enum tinyui_align x_align,
                                     enum tinyui_align y_align);
struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent,
                                      struct tinyui_rect child);
int tinyui_vertical_grid_align_offset(struct tinyui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space);
int tinyui_focus_reset(struct tinyui_app *app);
int tinyui_focus_navigate(struct tinyui_app *app, enum tinyui_native_nav_dir dir);
```

### Task T2: Core Backend Implementation

- [x] **Step 1: Implement backend tree helpers**

Add backend helpers for parent/child/sibling/root/name lookup/type.

- [x] **Step 2: Implement widget tree/name/type functions**

Use backend widget tree state and `ld_name_id`; return `struct tinyui_widget *`.

- [x] **Step 3: Implement geometry helpers**

Use TINYUI structs and map align enums to equivalent behavior.

- [x] **Step 4: Implement focus helpers**

Use `struct tinyui_app` focus model and `enum tinyui_native_nav_dir`.

### Task T3: Unit Proof

- [x] **Step 1: Add tree/name/type tests**

Extend `tests/tinyui/unit/test_tinyui_layout.c`.

- [x] **Step 2: Add geometry tests**

Cover absolute/relative/align/center/vertical-grid results.

- [x] **Step 3: Add focus tests**

Cover reset and directional navigation without exposing `ld_scene_t`.

### Task T4: Contract Matrix Docs

- [x] **Step 1: Update three JSON truth-sources**

Move the 16 symbols from `allowlisted` to `covered`.

- [x] **Step 2: Update `docs/ability/README.md` and `docs/ability/base.md`**

Remove strict 100% leftover wording and show `base` as direct covered.

- [x] **Step 3: Verify generated counts**

Expected:

```text
covered_total=420
allowlisted_total=191
direct_public_covered_total=420
policy_allowlisted_total=191
direct_public_100_complete=true
optional_public_extension=0
```

### Task T5: Gate Review Closeout

- [x] **Step 1: Run unit and contract gates**

```bash
ctest --test-dir build -R 'test_tinyui_layout|check_tinyui_release_capability_matrix' --output-on-failure
python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py
python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
git diff --check
```

- [x] **Step 2: Fresh review**

Use a fresh subagent to review a-0.12 API/docs/matrix consistency.

- [x] **Step 3: Final GitNexus detect_changes**

Run `gitnexus_detect_changes(scope="all", repo="LingDongGUI")`.

## 3. Self-review checklist

- [x] a-0.12 scope stays limited to base 16 candidates.
- [x] public API does not expose LingDongGUI native pointer types.
- [x] plan requires public header/source/unit/matrix/docs for every covered row.
- [x] implementation and verification evidence recorded after T1-T5.


## 4. 执行记录

- 2026-06-02 T1：已在 `tinyui/include/tinyui/widget.h` 增加 portable geometry structs、`tinyui_widget_type`、tree/name/type、geometry/focus public API declaration。
- 2026-06-02 T2：已在 `tinyui/src/backend/ldgui/backend_widget_tree.c` / `backend.h` 增加 parent/first_child/next_sibling/nameId lookup helper；已在 `tinyui/src/core/widget.c` 实现 tree/name/type、geometry、focus public API；`tinyui/src/widgets/window.c` 绑定 root host widget，保证 root/tree 查询返回 TINYUI wrapper。
- 2026-06-02 T3：已在 `tests/tinyui/unit/test_tinyui_layout.c` 增加 tree/name/type、geometry、focus public API round-trip proof。
- 2026-06-02 T4：已同步 `ldgui_public_api_inventory.json`、`native_api_gap_ledger.json`、`tinyui_release_capability_matrix.json`，并更新 `docs/ability/README.md` 与 `docs/ability/base.md`。当前 matrix summary 为 `covered_total=420`、`allowlisted_total=191`、`direct_public_covered_total=420`、`policy_allowlisted_total=191`、`direct_public_100_complete=true`、`direct_100_category_counts={policy_never_public:191}`。
- 2026-06-02 T5：完整 gate 已通过：`python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py`、`python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py`、`python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py`、`ctest --test-dir build -R 'test_tinyui_layout|check_tinyui_release_capability_matrix|test_tinyui_native_bridge|ldswitch_internal_test|ldswitch_widget_test|check_switch_focus_routing' --output-on-failure`、`git diff --check`。额外构建验证：`cmake --build build --target test_tinyui_layout test_tinyui_native_bridge`。fresh reviewer 首轮发现 `ldBaseFocusNavigate` 只覆盖四向 sibling；已在同一个 subagent 内补 `PICOUI_NATIVE_NAV_ENTER/BACK`、backend nav 映射、focus ENTER/BACK 行为和 unit proof 后复验通过。最终 GitNexus `detect_changes(scope=all)`：medium risk，14 files，48 changed symbols，1 affected process (`Picoui_window_create_with_props -> Picoui_backend_create_window`)。
