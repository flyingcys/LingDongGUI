# TINYUI a-0.8 native API exhaustiveness 设计

## 1. 背景

`v0.7` 已把 LingDongGUI `27/27` widget-like 控件纳入 TINYUI 盘点，并把 release matrix 写成 `full_parity_complete=27`。

但 `docs/tinyui-serial/a-0.8-v0.7-native-100-review-gap-list.md` 已确认：这个结论不能等同于“100% 控件覆盖 + 对应控件 100% 能力覆盖”。根因不是单个控件漏几个 setter，而是 gate 结构仍不够硬：

1. `check_tinyui_native_100_inventory.py` 只解析 `ldBase.h` 的 widget enum，不解析所有 `ld*.h` public API。
2. `check_tinyui_release_capability_matrix.py` 主要校验 matrix 自填状态，不校验 `tinyui_api / backend_proof / unit_test` 是否真实存在。
3. `check_tinyui_public_api.py` 只做禁泄漏和命名前缀检查，不做 native API coverage。
4. `text / image / window-layout / line_edit / keyboard` 等控件存在可见 public 能力缺口。
5. `message_box / keyboard` 旧 manual evidence 仍保留 special-case。
6. `backend_app.c` 仍保留 temporary smoke cursor layout，不能参与 full parity 结论。

`a-0.8` 因此不是新控件线，而是 native-100 真实性修正线。

## 2. 目标

`a-0.8` 的目标：

1. 从 `src/gui/ld*.h` 自动抽取所有 public native API，生成机器真相源。
2. 用 native API inventory 反向约束 release matrix。
3. 让每个 native API row 都能追溯到 TINYUI public API、backend truth、unit test、gate evidence。
4. 生成全量 `native API gap ledger`，明确当前代码中所有 `missing_tinyui_api / missing_backend_proof / missing_unit / missing_gate / overwrapped` 项。
5. 修复 ledger 中所有真实缺口；review 中列出的控件只是已知样本，不是完整范围。
6. 清理旧 special-case 证据冲突。
7. 禁止 temporary smoke layout 参与 full parity 证据。
8. 最终重新建立可证明的 native-100 closeout。

## 3. 非目标

`a-0.8` 明确不做：

1. 不新增 LingDongGUI 原生控件。
2. 不改 LingDongGUI 原生 API 来适配 TINYUI。
3. 不把 fake renderer、temporary smoke path、demo local state、artifact existence 当能力完成证据。
4. 不用“上层 API 裁剪”继续跳过 native public API。
5. 不把未人工复核的 manual artifact 写成人工验收通过。
6. 不为 lifecycle-only 或内部 runtime API 伪造 TINYUI public API。
7. 不在现有 TINYUI API 已能等价表达 native 能力时重复新增 wrapper；必须先证明现有 API 不足。

## 4. 范围定义

### 4.1 控件范围

沿用 `v0.7` 的 `27` 个 widget-like 控件：

1. `window`
2. `label`
3. `button`
4. `checkbox`
5. `switch`
6. `slider`
7. `text`
8. `image`
9. `list`
10. `progress_bar`
11. `qrcode`
12. `progress_wheel`
13. `message_box`
14. `date_time`
15. `clock`
16. `line_edit`
17. `keyboard`
18. `combo_box`
19. `scroll_selecter`
20. `arc`
21. `gauge`
22. `graph`
23. `table`
24. `calendar`
25. `icon_slider`
26. `radial_menu`
27. `animation`

`widgetTypeBackground / ldBase / ldGui / internal solver / switch internal` 不作为独立控件，但其通用能力必须通过 shared TINYUI widget/layout/event/theme/resource API 证明。

### 4.2 native API 范围

extractor 必须识别以下类别：

1. `init`
2. `depose`
3. `on_load`
4. `on_frame_start`
5. `on_frame_complete`
6. `show`
7. public setter
8. public getter
9. callback / weak hook
10. update / navigate / action API
11. macro alias 到 `ldBase*` 的通用能力

默认所有 public API 都 in scope。

允许忽略的类别必须进入 allowlist，并带 `reason`：

1. lifecycle-only 且无 TINYUI public surface 的 `depose / on_load / on_frame_start / on_frame_complete / show`
2. internal helper header，例如 `ldSwitchInternal.h`
3. 非 widget public API
4. backend-private extension hook，例如无法稳定表达为跨平台 TINYUI public API 的 weak draw hook

allowlist 不能成为跳过控件能力的通道。

### 4.3 overwrap 判定

`a-0.8` 必须同时防止两类错误：

1. 漏包：native public 能力没有 TINYUI 表达、backend proof、unit 或 gate。
2. 过包：为了追求字面 1:1，把 lifecycle-only、render-only、internal helper、已有 shared API 已覆盖的能力包装成新的 public API。

新增 TINYUI API 前必须满足：

1. 对应 native API 是用户可观察能力，而不是纯 lifecycle / render / internal helper。
2. 现有 `tinyui_widget_* / tinyui_layout_* / tinyui_theme_* / tinyui_native_*` API 不能等价表达。
3. backend proof 需要 public API 才能稳定触达，不能只靠已有 shared API。

如果现有 API 已等价覆盖，matrix row 必须使用 `coverage_kind = "shared_api_equivalence"`，并写明 `tinyui_api` 指向已有 API。

## 5. 方案

### 5.1 native API inventory

新增 `tests/tinyui/contract/ldgui_public_api_inventory.json`。

每条 row 至少包含：

```json
{
  "widget": "text",
  "header": "src/gui/ldText.h",
  "ldgui_symbol": "ldTextSetStaticText",
  "category": "setter",
  "signature": "void ldTextSetStaticText(ldText_t* ptWidget,const uint8_t *pStr)",
  "required": true,
  "tinyui_api": "tinyui_text_set_static_text",
  "backend_proof": "tinyui_backend_text_set_static_text",
  "unit_test": "test_text_static_text_uses_backend_static_storage",
  "gate_evidence": ["contract", "unit"],
  "gap_status": "missing_tinyui_api"
}
```

新增 `tests/tinyui/contract/check_ldgui_public_api_inventory.py`。

该 checker 必须：

1. 扫描 `src/gui/ld*.h`。
2. 抽取 `ld[A-Z][A-Za-z0-9_]*` function declarations。
3. 解析 `#define ldXxxSetHidden ldBaseSetHidden` 这类 macro alias。
4. 按 widget 归类。
5. 断言 inventory 中没有漏掉 required API。
6. 断言 allowlist 项带 reason。
7. 产出 `native_api_gap_ledger.json`，把每个 row 分类为 `covered / missing_tinyui_api / missing_backend_proof / missing_unit / missing_gate / overwrapped / allowlisted`。

### 5.2 exhaustiveness checker

新增 `tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py`。

该 checker 必须：

1. 读取 `ldgui_public_api_inventory.json`。
2. 读取 `tinyui_release_capability_matrix.json`。
3. 对每个 required native API 查找对应 matrix row。
4. 校验 `tinyui_api` 在 `tinyui/include/tinyui/*.h` 中存在。
5. 校验 `backend_proof` 在 `tinyui/src/backend/ldgui/*.c` 或 backend header 中存在。
6. 校验 `unit_test` 在 `tests/tinyui/unit/*.c` 中存在。
7. 校验 row status 是 `support`。
8. 校验 manual artifact 状态不被当作 manual pass。
9. 校验所有 `gap_status` 最终为 `covered` 或严格 allowlisted。
10. 校验 `overwrapped` 项被移除或改为 shared API equivalence。

### 5.3 matrix schema

release matrix capability row 必须从粗粒度能力改为 native API aligned row。

每条 row 至少包含：

```json
{
  "name": "ldTextSetStaticText",
  "status": "support",
  "native_api": "ldTextSetStaticText",
  "coverage_kind": "native_setter_parity",
  "tinyui_api": "tinyui_text_set_static_text",
  "backend_proof": "tinyui_backend_text_set_static_text",
  "unit_test": "test_text_static_text_uses_backend_static_storage",
  "gate_evidence": ["contract", "unit"],
  "capability_release_judgement": "final_release_ready"
}
```

`coverage_kind` 只能是：

1. `native_setter_parity`
2. `native_getter_parity`
3. `init_parameter_parity`
4. `direct_field_parity`
5. `macro_alias_parity`
6. `lifecycle_internal_allowlisted`
7. `non_widget_allowlisted`
8. `shared_api_equivalence`

`direct_field_parity` 必须带 rationale 和 unit proof。
`shared_api_equivalence` 必须带 `equivalence_proof`，说明现有 TINYUI API 如何覆盖 native 能力。

### 5.4 shared policy

`a-0.8` 必须统一以下 policy：

1. readback policy：getter 分为 public parity、backend proof、allowlist 三类；只有用户可观察且适合 public surface 的 getter 才补 TINYUI getter。
2. resource policy：image/mask/font/color/align 由 shared native bridge 表达，控件不得各自发明。
3. manual artifact policy：artifact exists、manual review required、manual reviewed passed 三者分离。
4. smoke path policy：temporary smoke path 可以保留给 runtime harness，但不能参与 native-100 formal mapping / visible / manual 结论。

## 6. 全量 gap ledger 与已知缺口修复要求

`a-0.8` 的修复范围由 `native_api_gap_ledger.json` 决定，不由人工列举决定。

ledger 必须至少覆盖全部 `27` 个 widget-like 控件和 `ldBase / ldWindow` shared 能力。以下小节只是当前 review 已知样本，不能被解释为完整修复范围。

### 6.1 text

必须补齐或证明已有 shared API 等价：

1. `tinyui_text_set_transparent`
2. `tinyui_text_set_static_text`
3. `tinyui_text_set_text_color`
4. `tinyui_text_set_bg_color`
5. `tinyui_text_set_background_source`
6. `tinyui_text_set_consumed_font`
7. `tinyui_text_scroll_seek`
8. `tinyui_text_scroll_move`

### 6.2 image

必须补齐或证明已有 shared API 等价：

1. `tinyui_image_set_mask_color`
2. backend 写入 `ldImage_t.maskColor`
3. unit proof

### 6.3 window/layout

必须补齐或证明已有 shared layout/widget API 等价；不得在现有 `tinyui_widget_set_padding`、`tinyui_flex_set_*`、`tinyui_grid_set_*` 已能完整覆盖时重复包装：

1. `ldWindowSetLayout`
2. `ldWindowSetPadding`
3. `ldWindowSetGridPadding`
4. `ldWindowSetGap`
5. `ldWindow_t.layoutTpye / flexPadding / gridPadding / flexGap / gridRowGap / gridColumnGap` unit proof

### 6.4 line_edit

必须补齐或证明已有 widget/theme API 等价：

1. `tinyui_line_edit_set_align`
2. `tinyui_line_edit_set_color`
3. native `SIGNAL_FINISHED` parity
4. TINYUI 内部 commit/cancel edit_result 证据，但不包装成 native public parity reason
5. backend proof for align/color/finished signal

### 6.5 keyboard

必须决策并补齐或 allowlist：

1. target button list hook 决策：public abstraction、backend-private extension 或 allowlist
2. callback hook 决策：public abstraction、backend-private extension 或 allowlist
3. user draw hook 默认 backend-private/allowlist；不得把 Arm-2D draw context 伪装成跨平台 TINYUI public API
4. update / button update API 或 allowlist reason
5. dedicated mapping / visible / manual evidence

### 6.6 slider / button / qrcode

必须补齐：

1. `slider` 的三色语义 readback / proof。
2. `button` 的 getter parity 决策和补齐。
3. `qrcode` 的 init-only field / direct field parity 标注。

### 6.7 all remaining widgets

必须由 ledger 覆盖并修复或证明等价：

1. `label / checkbox / switch / list`
2. `progress_bar / progress_wheel / qrcode`
3. `date_time / clock / arc / gauge`
4. `combo_box / scroll_selecter`
5. `graph / table / calendar`
6. `icon_slider / radial_menu / message_box / animation`

每个 widget 的所有 public setter/getter/init 参数/callback/update/action/macro alias 必须处于 `covered` 或严格 allowlisted 状态。

### 6.8 message_box / keyboard evidence

必须修复：

1. `message_box_basic` 旧 formal mapping exclusion。
2. `keyboard_basic` 旧 fake fallback 特例。
3. manual artifact 状态混用。

### 6.9 backend_app smoke path

必须修复：

1. full parity demo 触发 smoke cursor layout 时 gate fail。
2. smoke path 只能在明确 temporary smoke target 中 opt-in。
3. closeout 文档必须明确 smoke path 不参与 native-100 结论。

## 7. 完成定义

`a-0.8` 完成必须同时满足：

1. `ldgui_public_api_inventory.json` 覆盖所有 required `src/gui/ld*.h` public API。
2. `check_ldgui_public_api_inventory.py` 通过。
3. `check_tinyui_native_api_exhaustiveness.py` 通过。
4. `tinyui_release_capability_matrix.json` capability rows 与 native API inventory 完全对齐。
5. `native_api_gap_ledger.json` 中所有真实缺口全部关闭。
6. 旧 special-case evidence 已替换或从 full parity 结论剥离。
7. temporary smoke layout 不参与 full parity demo 结论。
8. `ctest --test-dir build -L tinyui --output-on-failure` 通过。
9. `python3 tests/tinyui/contract/check_tinyui_public_api.py` 通过。
10. `python3 tests/tinyui/contract/check_tinyui_native_100_inventory.py` 通过。
11. `python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py` 通过。
12. `python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py` 通过。
13. `python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py` 通过。
14. runtime / mapping / visible / manual artifact gate 全部通过。
15. closeout 文档不把 artifact existence 写成人工验收通过。
