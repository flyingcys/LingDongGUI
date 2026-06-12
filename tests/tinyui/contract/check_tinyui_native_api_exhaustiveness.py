#!/usr/bin/env python3
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
INVENTORY_JSON = CONTRACT_DIR / "ldgui_public_api_inventory.json"
LEDGER_JSON = CONTRACT_DIR / "native_api_gap_ledger.json"
MATRIX_JSON = CONTRACT_DIR / "tinyui_release_capability_matrix.json"

VALID_COVERAGE_KINDS = {
    "native_setter_parity",
    "native_getter_parity",
    "init_parameter_parity",
    "direct_field_parity",
    "macro_alias_parity",
    "lifecycle_internal_allowlisted",
    "non_widget_allowlisted",
    "shared_api_equivalence",
}
ALLOWLISTED_COVERAGE_KINDS = {
    "lifecycle_internal_allowlisted",
    "non_widget_allowlisted",
}
VALID_GAP_STATUSES = {
    "covered",
    "missing_picoui_api",
    "missing_backend_proof",
    "missing_unit",
    "missing_gate",
    "overwrapped",
    "allowlisted",
}
VALID_GROUP_KINDS = {
    "widget",
    "shared_base",
    "runtime_host",
    "internal_helper",
    "enum_only",
}
VALID_POLICY_CATEGORIES = {
    "direct_covered",
    "lifecycle_internal",
    "render_pipeline_internal",
    "runtime_host_internal",
    "layout_solver_internal",
    "memory_internal",
    "base_tree_policy",
    "resource_time_helper_policy",
    "drawing_helper_policy",
    "backend_private_hook",
    "native_action_private",
    "enum_only_semantics",
}
VALID_DIRECT_100_CATEGORIES = {
    "policy_never_public",
    "optional_public_extension",
    "direct_100_required_if_user_demands",
}
REQUIRED_ROW_FIELDS = {
    "native_api",
    "coverage_kind",
    "picoui_api",
    "backend_proof",
    "unit_test",
    "gate_evidence",
    "gap_status",
    "capability_release_judgement",
    "policy_category",
}
LEDGER_ALIGNED_FIELDS = {
    "group_kind",
    "coverage_kind",
    "picoui_api",
    "backend_proof",
    "unit_test",
    "required",
    "planned_task",
    "rationale",
    "notes",
    "shared_policy",
    "shared_policy_evidence",
    "artifact_entry_exists",
    "manual_review_required",
    "manual_reviewed_passed",
    "policy_category",
}

VALID_SHARED_POLICIES = {
    "shared_api_equivalence",
    "backend_proof_only",
    "allowlisted_tree_helper",
    "missing_picoui_api",
    "missing_backend_proof",
}


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _inventory_rows(inventory: dict) -> dict[str, dict]:
    rows: dict[str, dict] = {}
    for widget in inventory.get("widgets", []):
        for row in widget.get("required_native_apis", []):
            symbol = row.get("ldgui_symbol")
            if not isinstance(symbol, str) or not symbol:
                raise AssertionError(f"inventory row has invalid ldgui_symbol: {row!r}")
            rows[symbol] = row
    return rows


def _ledger_rows(ledger: dict) -> dict[str, dict]:
    rows: dict[str, dict] = {}
    for row in ledger.get("rows", []):
        symbol = row.get("ldgui_symbol")
        if not isinstance(symbol, str) or not symbol:
            raise AssertionError(f"ledger row has invalid ldgui_symbol: {row!r}")
        if symbol in rows:
            raise AssertionError(f"duplicate ledger row for native API: {symbol}")
        group_kind = row.get("group_kind")
        if group_kind not in VALID_GROUP_KINDS:
            raise AssertionError(f"{symbol} ledger row has invalid group_kind: {group_kind!r}")
        rows[symbol] = row
    return rows


def _matrix_capability_rows(matrix: dict) -> dict[str, dict]:
    rows: dict[str, dict] = {}
    for widget in matrix.get("widgets", []):
        group_kind = widget.get("group_kind")
        if group_kind not in VALID_GROUP_KINDS:
            raise AssertionError(
                f"matrix widget has invalid group_kind: {widget.get('name')}.{group_kind!r}"
            )
        capabilities = widget.get("capabilities")
        if not isinstance(capabilities, list):
            raise AssertionError(f"matrix widget missing capabilities list: {widget.get('name')}")
        for row in capabilities:
            missing_fields = REQUIRED_ROW_FIELDS - set(row)
            if missing_fields:
                raise AssertionError(
                    "matrix capability row missing native API aligned fields: "
                    f"{widget.get('name')}.{row.get('name')} missing={sorted(missing_fields)}"
                )
            native_api = row.get("native_api")
            if not isinstance(native_api, str) or not native_api:
                raise AssertionError(f"matrix capability row has invalid native_api: {row!r}")
            if native_api in rows:
                raise AssertionError(f"duplicate matrix native API row: {native_api}")
            rows[native_api] = row
    return rows


def _assert_row_shape(native_api: str, row: dict) -> None:
    coverage_kind = row.get("coverage_kind")
    assert coverage_kind in VALID_COVERAGE_KINDS, (
        f"{native_api} has invalid coverage_kind: {coverage_kind!r}"
    )
    gap_status = row.get("gap_status")
    assert gap_status in VALID_GAP_STATUSES, f"{native_api} has invalid gap_status: {gap_status!r}"
    assert isinstance(row.get("gate_evidence"), list), f"{native_api} gate_evidence must be a list"
    policy_category = row.get("policy_category")
    assert policy_category in VALID_POLICY_CATEGORIES, (
        f"{native_api} has invalid policy_category: {policy_category!r}"
    )

    if gap_status == "covered":
        assert policy_category == "direct_covered", (
            f"{native_api} covered row must use policy_category=direct_covered"
        )
        for field in ("picoui_api", "backend_proof", "unit_test"):
            assert isinstance(row.get(field), str) and row[field], (
                f"{native_api} covered row must have concrete {field}"
            )
        assert row["gate_evidence"], f"{native_api} covered row must have gate_evidence"

    if gap_status == "allowlisted":
        assert row.get("required") is False, f"{native_api} allowlisted row must be required=false"
        assert policy_category != "direct_covered", (
            f"{native_api} allowlisted row must not use policy_category=direct_covered"
        )
        assert coverage_kind in ALLOWLISTED_COVERAGE_KINDS, (
            f"{native_api} allowlisted row must use allowlisted coverage_kind"
        )
        assert row.get("allowlist_reason"), f"{native_api} allowlisted row missing allowlist_reason"
        direct_100_category = row.get("direct_100_category")
        assert direct_100_category in VALID_DIRECT_100_CATEGORIES, (
            f"{native_api} allowlisted row has invalid direct_100_category: "
            f"{direct_100_category!r}"
        )

    if gap_status == "overwrapped":
        assert row.get("notes"), f"{native_api} overwrapped row missing notes"

    shared_policy = row.get("shared_policy")
    if shared_policy is not None:
        assert shared_policy in VALID_SHARED_POLICIES, (
            f"{native_api} has invalid shared_policy: {shared_policy!r}"
        )
        evidence = row.get("shared_policy_evidence")
        assert isinstance(evidence, list) and evidence, (
            f"{native_api} shared_policy row must carry shared_policy_evidence"
        )

    for field in ("artifact_entry_exists", "manual_review_required", "manual_reviewed_passed"):
        if field in row:
            assert isinstance(row[field], bool), f"{native_api} {field} must be boolean"
    if row.get("manual_review_required") and not row.get("manual_reviewed_passed", False):
        assert row.get("gap_status") != "covered", (
            f"{native_api} cannot be covered by manual-review-required artifact only"
        )


def _assert_row_matches_ledger(native_api: str, matrix_row: dict, ledger_row: dict) -> None:
    assert matrix_row.get("name") == native_api, f"{native_api} matrix name must equal native_api"
    assert matrix_row.get("status") == ledger_row.get("gap_status"), (
        f"{native_api} matrix status drifted from ledger gap_status"
    )
    assert matrix_row.get("gap_status") == ledger_row.get("gap_status"), (
        f"{native_api} matrix gap_status drifted from ledger: "
        f"matrix={matrix_row.get('gap_status')} ledger={ledger_row.get('gap_status')}"
    )
    for field in sorted(LEDGER_ALIGNED_FIELDS):
        assert matrix_row.get(field) == ledger_row.get(field), (
            f"{native_api} matrix {field} drifted from ledger: "
            f"matrix={matrix_row.get(field)!r} ledger={ledger_row.get(field)!r}"
        )
    if ledger_row.get("gap_status") == "allowlisted":
        assert matrix_row.get("allowlist_reason") == ledger_row.get("allowlist_reason"), (
            f"{native_api} matrix allowlist_reason drifted from ledger"
        )
    else:
        assert "allowlist_reason" not in matrix_row, (
            f"{native_api} non-allowlisted matrix row must not carry allowlist_reason"
        )


def main() -> int:
    inventory = _load_json(INVENTORY_JSON)
    ledger = _load_json(LEDGER_JSON)
    matrix = _load_json(MATRIX_JSON)

    inventory_by_symbol = _inventory_rows(inventory)
    ledger_by_symbol = _ledger_rows(ledger)
    matrix_by_symbol = _matrix_capability_rows(matrix)

    assert set(matrix_by_symbol) == set(ledger_by_symbol), (
        "matrix native API rows must match native_api_gap_ledger rows:\n"
        f"missing_from_matrix={sorted(set(ledger_by_symbol) - set(matrix_by_symbol))[:25]}\n"
        f"extra_in_matrix={sorted(set(matrix_by_symbol) - set(ledger_by_symbol))[:25]}"
    )
    assert set(ledger_by_symbol).issubset(set(inventory_by_symbol)), (
        "ledger rows must be backed by ldgui_public_api_inventory rows:\n"
        f"missing_from_inventory={sorted(set(ledger_by_symbol) - set(inventory_by_symbol))[:25]}"
    )

    for native_api, matrix_row in sorted(matrix_by_symbol.items()):
        ledger_row = ledger_by_symbol[native_api]
        _assert_row_matches_ledger(native_api, matrix_row, ledger_row)
        _assert_row_shape(native_api, matrix_row)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
