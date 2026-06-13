# tests/tinyui/contract 历史资产说明

本目录已不再承载当前 TinyUI `v2.1` 的 canonical contract truth。

当前 canonical truth source 统一位于：

- `tests/tinyui/contract/`

本目录当前保留的文件只承担以下历史职责：

- 为 `docs/tinyui-serial/*`、`docs/superpowers/*`、`docs/v2.0/*` 等历史文档保留可追溯证据路径
- 保留 TINYUI 阶段性审计、review、spec、plan 中引用过的冻结 JSON/ledger 快照
- 保留仍带独立逻辑、但当前已改读 canonical `tests/tinyui/contract/*` 输入的 `check_ldgui_public_api_inventory.py`

约束：

- 不要把本目录中的 JSON/ledger 当作 current live checker 输入
- 不要把本目录中的路径当作 `docs/v2.1`、`docs/ability/*`、`tinyui/docs/*` 的 canonical truth
- 若需更新当前 contract truth，应修改 `tests/tinyui/contract/*`

当前目录中主要历史资产包括：

- `ldgui_public_api_inventory.json`
- `ldgui_public_api_expected_symbols.json`
- `native_api_gap_ledger.json`
- `tinyui_release_capability_matrix.json`
- `tinyui_native_100_inventory.json`
- `native_100_fragments/*.json`
- `tinyui_tinyui_transition_inventory.json`
