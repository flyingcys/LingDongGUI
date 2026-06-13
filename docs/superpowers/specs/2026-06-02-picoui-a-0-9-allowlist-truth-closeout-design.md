# TINYUI a-0.9 allowlist truth closeout 设计

## 1. 背景

`a-0.8` 已完成 LingDongGUI native API exhaustiveness 基线：

1. `tests/tinyui/contract/ldgui_public_api_inventory.json` 覆盖 `src/gui/ld*.h` public API，共 `611` 行。
2. `tests/tinyui/contract/tinyui_release_capability_matrix.json` 已升级为 `a-0.8-native-api-exhaustiveness-v1`，matrix rows 共 `611`。
3. `docs/ability/` 已按每个 LingDongGUI symbol 写入 TINYUI 覆盖状态、TINYUI API、backend proof、unit/gate、allowlist reason。
4. 当前 summary 为 `covered=404 / allowlisted=207 / missing_gap_total=0`。

这说明当前没有未建账的 LingDongGUI native API 行，但也不能写成 TINYUI direct public wrapper 100% 覆盖。`allowlisted=207` 的行全部是 `required=false`，大多是 lifecycle、render/show、runtime/internal helper、tree/resource/helper、weak hook 或 native action helper。

`a-0.9` 的职责是把这些 `allowlisted` 行从“粗粒度允许跳过”升级成“可机器校验、可解释、不会污染 release 结论的 policy truth”。它不是重做 a-0.8，也不是把 allowlist 直接改名成 covered。

## 2. 目标

`a-0.9` 必须完成：

1. 为 matrix row 增加明确 policy 分类，让 `allowlisted` 不再只有泛化 reason。
2. 将 lifecycle / show / internal helper 这类已决策内部能力标成 policy-complete，而不是继续让 widget 级状态误报 `parity_incomplete`。
3. 对 `keyboard weak hook`、`button action`、`background/root semantics` 做真实能力决策。
4. 对仍应暴露给 TINYUI 用户的能力补 public API、backend proof、unit/gate、matrix row。
5. 对不应暴露给 TINYUI 用户的能力写 strict rationale，并由 checker 验证。
6. 同步 `docs/ability/`、`docs/tinyui-serial/a-0.9-*` 和 matrix truth source。

## 3. 非目标

`a-0.9` 明确不做：

1. 不新增 LingDongGUI 原生 API。
2. 不重新设计 a-0.8 extractor / inventory 基线。
3. 不把 `allowlisted` 直接改名成 `covered`。
4. 不把 lifecycle / show / internal helper 强行包装成 TINYUI public API。
5. 不把 artifact existence 写成人工验收通过。
6. 不用截图、demo 存在或 markdown 文字替代 backend-field/unit/gate 证据。

## 4. 范围

### 4.1 输入真相源

1. `docs/ability/README.md`
2. `docs/tinyui-serial/a-0.9-未direct覆盖能力收口.md`
3. `tests/tinyui/contract/ldgui_public_api_inventory.json`
4. `tests/tinyui/contract/native_api_gap_ledger.json`
5. `tests/tinyui/contract/tinyui_release_capability_matrix.json`
6. `tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py`
7. `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`

### 4.2 待收口能力组

当前 `207` 个 allowlisted 行按 a-0.9 分为五组：

| 组 | 数量 | 说明 |
| --- | ---: | --- |
| lifecycle / show | 135 | 每个 widget 的 `depose / on_load / on_frame_start / on_frame_complete / show`。 |
| scene/runtime/internal groups | 35 | `gui / mem / switch_internal / window_layout_internal`。 |
| base tree/resource/helper | 30 | `ldBase` 的 tree、resource、date/time、drawing、layout helper。 |
| keyboard weak hook / button action | 5 | keyboard weak hook、draw callback、native action helper。 |
| background/root semantics | 1 concept | `widgetTypeBackground` 无独立 API group 的 root/background 语义。 |

## 5. 设计

### 5.1 新增 policy 分类

matrix row 增加 `policy_category` 字段，允许值：

1. `direct_covered`
2. `lifecycle_internal`
3. `render_pipeline_internal`
4. `runtime_host_internal`
5. `layout_solver_internal`
6. `memory_internal`
7. `base_tree_policy`
8. `resource_time_helper_policy`
9. `drawing_helper_policy`
10. `backend_private_hook`
11. `native_action_private`
12. `enum_only_semantics`

规则：

1. `gap_status=covered` 的行必须使用 `policy_category=direct_covered`。
2. `gap_status=allowlisted` 的行必须使用除 `direct_covered` 外的 policy。
3. `allowlisted` 行必须 `required=false`。
4. `allowlisted` 行必须有非空 `allowlist_reason`。
5. `allowlisted` 行如果没有明确 `policy_category`，checker 必须 fail。

### 5.2 Widget 级状态

当前很多 widget 因 lifecycle/show 行保持 `parity_incomplete`。a-0.9 改成三层状态：

1. `direct_parity_complete`：所有 native API row 都是 direct `covered`。
2. `policy_complete`：所有 row 要么 direct `covered`，要么 strict allowlisted policy，且无 open missing/overwrapped。
3. `parity_incomplete`：仍存在 missing、overwrapped、证据缺口，或需要决策的 allowlist。

`a-0.9` 完成后，普通 widget 应进入 `policy_complete`，除非 R3/R4/R5 发现确实需要新增 TINYUI API。

### 5.3 Group kind

matrix widget/group 增加 `group_kind`：

1. `widget`
2. `shared_base`
3. `runtime_host`
4. `internal_helper`
5. `enum_only`

规则：

1. `window/button/text/...` 这类控件为 `widget`。
2. `base` 为 `shared_base`。
3. `gui` 为 `runtime_host`。
4. `mem/switch_internal/window_layout_internal` 为 `internal_helper`。
5. `background` 不在当前 matrix 中；若新增 policy row，应为 `enum_only`。

### 5.4 重点决策

#### Base tree/resource/helper

`ldBaseGetParent / GetChildCount / GetChildList / GetNextSibling / NodeAdd / NodeRemove` 可能是用户可见结构能力。a-0.9 必须做二选一：

1. 暴露 TINYUI tree introspection API：补 `tinyui_widget_get_parent`、`tinyui_widget_child_count`、`tinyui_widget_get_child`、`tinyui_widget_get_next_sibling` 等 API、backend proof、unit、matrix。
2. 保持内部：说明 TINYUI 用户模型不暴露 native tree traversal，所有 tree mutation 由 `tinyui_widget_append_child` 等现有 API 闭环；checker 验证这些 row 是 `base_tree_policy`。

#### Keyboard weak hook / button action

这 5 行不能只靠泛化 allowlist：

1. `ldKeyboardGetTargetBtnList`
2. `ldKeyboardCallback`
3. `ldKeyboardBtnUserDraw`
4. `ldButtonActionInit`
5. `ldButtonActionIsPressById`

必须逐项决策：

1. 如果 TINYUI 需要 key layout、callback、custom draw 或 action readback，补 TINYUI abstraction。
2. 如果不暴露 native weak hook / nameId action helper，写明跨平台替代能力和不可移植原因。

#### Background/root semantics

`background` 是 `ldWidgetType_t` 类型，但无独立 header/API group。a-0.9 必须：

1. 要么新增 enum-only policy rows，明确 root/background semantics 由 window/tree 派生覆盖；
2. 要么新增 TINYUI root/background public abstraction 和 gate。

不能继续只保留文字边界。

## 6. 验收

`a-0.9` 完成时必须满足：

1. `611` 行 native API 仍全部在 inventory / matrix / ability docs 中。
2. `covered` 行仍有 TINYUI API/backend/unit/gate 证据。
3. `allowlisted` 行全部 `required=false`，有非空 reason 和 `policy_category`。
4. `group_kind` 覆盖所有 matrix group。
5. 普通 widget 不再因为 lifecycle/show/internal policy 行误报 `parity_incomplete`。
6. `keyboard weak hook / button action / background root semantics` 已明确开发或拒绝决策。
7. `docs/ability/` 同步显示 direct covered、policy-covered internal、仍需开发。
8. 所有 contract checker 和 runtime/mapping/visible/manual artifact gate fresh 通过。

