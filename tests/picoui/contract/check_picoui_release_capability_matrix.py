import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
MATRIX_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_release_capability_matrix.json"
INVENTORY_JSON = ROOT / "tests" / "picoui" / "contract" / "ldgui_public_api_inventory.json"
LEDGER_JSON = ROOT / "tests" / "picoui" / "contract" / "native_api_gap_ledger.json"
PICOUI_INCLUDE_DIR = ROOT / "picoui" / "include"

A09_SCHEMA_VERSION = "a-0.9-allowlist-policy-v1"
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
VALID_PARITY_STATUSES = {
    "direct_parity_complete",
    "policy_complete",
    "parity_incomplete",
    "non_widget_policy_complete",
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
EXPECTED_GATE_CATALOG = {
    "runtime": {
        "picoui_hello_world_demo",
        "picoui_basic_widgets_demo",
        "picoui_layout_flex_demo",
        "picoui_layout_grid_demo",
        "picoui_theme_showcase_demo",
        "picoui_settings_panel_demo",
        "picoui_list_basic_demo",
        "picoui_progress_bar_basic_demo",
        "picoui_arc_basic_demo",
        "picoui_gauge_basic_demo",
        "picoui_icon_slider_basic_demo",
        "picoui_radial_menu_basic_demo",
        "picoui_progress_wheel_basic_demo",
        "picoui_qrcode_basic_demo",
        "picoui_message_box_basic_demo",
        "picoui_date_time_basic_demo",
        "picoui_clock_basic_demo",
        "picoui_keyboard_basic_demo",
        "picoui_line_edit_basic_demo",
        "picoui_combo_box_basic_demo",
        "picoui_scroll_selecter_basic_demo",
        "picoui_table_basic_demo",
        "picoui_graph_basic_demo",
        "picoui_calendar_basic_demo",
        "picoui_animation_basic_demo",
    },
    "mapping": {
        "picoui_hello_world_demo",
        "picoui_basic_widgets_demo",
        "picoui_layout_flex_demo",
        "picoui_layout_grid_demo",
        "picoui_theme_showcase_demo",
        "picoui_settings_panel_demo",
        "picoui_list_basic_demo",
        "picoui_progress_bar_basic_demo",
        "picoui_arc_basic_demo",
        "picoui_gauge_basic_demo",
        "picoui_icon_slider_basic_demo",
        "picoui_radial_menu_basic_demo",
        "picoui_progress_wheel_basic_demo",
        "picoui_qrcode_basic_demo",
        "picoui_message_box_basic_demo",
        "picoui_date_time_basic_demo",
        "picoui_clock_basic_demo",
        "picoui_keyboard_basic_demo",
        "picoui_line_edit_basic_demo",
        "picoui_combo_box_basic_demo",
        "picoui_scroll_selecter_basic_demo",
        "picoui_table_basic_demo",
        "picoui_graph_basic_demo",
        "picoui_calendar_basic_demo",
        "picoui_animation_basic_demo",
    },
    "visible": {
        "hello_world",
        "basic_widgets",
        "layout_flex",
        "layout_grid",
        "theme_showcase",
        "settings_panel",
        "list_basic",
        "progress_bar_basic",
        "arc_basic",
        "gauge_basic",
        "icon_slider_basic",
        "radial_menu_basic",
        "progress_wheel_basic",
        "qrcode_basic",
        "message_box_basic",
        "date_time_basic",
        "clock_basic",
        "keyboard_basic",
        "line_edit_basic",
        "combo_box_basic",
        "scroll_selecter_basic",
        "table_basic",
        "graph_basic",
        "calendar_basic",
        "animation_basic",
    },
    "manual_artifact": {
        "hello_world",
        "basic_widgets",
        "layout_flex",
        "layout_grid",
        "theme_showcase",
        "settings_panel",
        "list_basic",
        "progress_bar_basic",
        "arc_basic",
        "gauge_basic",
        "icon_slider_basic",
        "radial_menu_basic",
        "progress_wheel_basic",
        "qrcode_basic",
        "message_box_basic",
        "date_time_basic",
        "clock_basic",
        "keyboard_basic",
        "line_edit_basic",
        "combo_box_basic",
        "scroll_selecter_basic",
        "table_basic",
        "graph_basic",
        "calendar_basic",
        "animation_basic",
    },
}


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _widgets_by_name(matrix: dict) -> dict[str, dict]:
    widgets = matrix.get("widgets")
    if not isinstance(widgets, list):
        raise AssertionError("release matrix missing widgets list")
    by_name: dict[str, dict] = {}
    for widget in widgets:
        name = widget.get("name")
        if not isinstance(name, str) or not name:
            raise AssertionError(f"widget row has invalid name: {widget!r}")
        if name in by_name:
            raise AssertionError(f"duplicate widget row in release matrix: {name}")
        group_kind = widget.get("group_kind")
        if group_kind not in VALID_GROUP_KINDS:
            raise AssertionError(f"{name} has invalid group_kind: {group_kind!r}")
        parity_status = widget.get("parity_status")
        if parity_status not in VALID_PARITY_STATUSES:
            raise AssertionError(f"{name} has invalid parity_status: {parity_status!r}")
        if group_kind in {"runtime_host", "internal_helper"}:
            assert parity_status == "non_widget_policy_complete", (
                f"{name} {group_kind} group must be non_widget_policy_complete"
            )
        if group_kind == "shared_base" and parity_status == "policy_complete":
            non_covered_policy_categories = {
                capability.get("policy_category")
                for capability in widget.get("capabilities", [])
                if capability.get("gap_status") != "covered"
            }
            assert non_covered_policy_categories <= {
                "base_tree_policy",
                "resource_time_helper_policy",
                "drawing_helper_policy",
            }, f"{name} shared_base policy_complete has invalid policy categories"
        by_name[name] = widget
    return by_name


def _assert_matrix_header(matrix: dict) -> None:
    assert matrix.get("schema_version") == A09_SCHEMA_VERSION
    assert matrix.get("line") == "a-0.9"
    assert matrix.get("stage") == "allowlist-truth-r0-policy-schema"
    assert matrix.get("purpose") == (
        "PicoUI allowlist policy truth source aligned to LingDongGUI public API inventory, "
        "gap ledger, and a-0.9 policy categories"
    )


def _inventory_symbols() -> set[str]:
    inventory = _load_json(INVENTORY_JSON)
    symbols: set[str] = set()
    for widget in inventory.get("widgets", []):
        for row in widget.get("required_native_apis", []):
            symbols.add(row["ldgui_symbol"])
    return symbols


def _ledger_rows() -> dict[str, dict]:
    ledger = _load_json(LEDGER_JSON)
    rows: dict[str, dict] = {}
    for row in ledger.get("rows", []):
        symbol = row.get("ldgui_symbol")
        assert isinstance(symbol, str) and symbol, f"invalid ledger row: {row!r}"
        assert symbol not in rows, f"duplicate ledger row: {symbol}"
        group_kind = row.get("group_kind")
        assert group_kind in VALID_GROUP_KINDS, f"{symbol} invalid ledger group_kind: {group_kind!r}"
        rows[symbol] = row
    return rows


def _public_picoui_api_symbols() -> set[str]:
    symbols: set[str] = set()
    pattern = re.compile(r"\b(picoui_[A-Za-z0-9_]+)\s*\(")
    for header in PICOUI_INCLUDE_DIR.rglob("*.h"):
        text = header.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            symbols.add(match.group(1))
    return symbols


def _assert_public_picoui_api(native_api: str, picoui_api: str, public_picoui_symbols: set[str]) -> None:
    api_symbols = [part.strip() for part in picoui_api.split("+")]
    assert api_symbols and all(api_symbols), f"{native_api} has invalid picoui_api field"
    missing = [symbol for symbol in api_symbols if symbol not in public_picoui_symbols]
    assert not missing, (
        f"{native_api} covered row references non-public PicoUI API: "
        f"{', '.join(missing)}"
    )


def _assert_native_api_rows(
    by_name: dict[str, dict],
    ledger_by_symbol: dict[str, dict],
    public_picoui_symbols: set[str],
) -> int:
    seen: set[str] = set()
    for widget_name, widget in sorted(by_name.items()):
        assert widget.get("widget_status") == "native_api_gap_tracked", (
            f"{widget_name} must remain native_api_gap_tracked in R1"
        )
        assert widget.get("widget_release_judgement") != "final_release_ready", (
            f"{widget_name} must not be final_release_ready during R1"
        )
        group_kind = widget.get("group_kind")
        parity_status = widget.get("parity_status")
        capabilities = widget.get("capabilities")
        assert isinstance(capabilities, list) and capabilities, f"{widget_name} missing capabilities"
        non_covered_policy_categories = {
            capability.get("policy_category")
            for capability in capabilities
            if capability.get("gap_status") != "covered"
        }
        if group_kind == "widget" and non_covered_policy_categories <= {
            "lifecycle_internal",
            "render_pipeline_internal",
            "backend_private_hook",
            "native_action_private",
            "enum_only_semantics",
        }:
            assert parity_status == "policy_complete", (
                f"{widget_name} policy-only widget must be policy_complete"
            )
        if group_kind in {"runtime_host", "internal_helper"}:
            assert parity_status == "non_widget_policy_complete", (
                f"{widget_name} internal/runtime group must be non_widget_policy_complete"
            )
        if group_kind == "shared_base" and non_covered_policy_categories <= {
            "base_tree_policy",
            "resource_time_helper_policy",
            "drawing_helper_policy",
        }:
            assert parity_status == "policy_complete", (
                f"{widget_name} shared_base policy rows must be policy_complete"
            )
        for capability in capabilities:
            native_api = capability.get("native_api")
            assert isinstance(native_api, str) and native_api, (
                f"{widget_name} capability missing native_api: {capability!r}"
            )
            assert native_api not in seen, f"duplicate matrix native API row: {native_api}"
            seen.add(native_api)
            assert native_api in ledger_by_symbol, f"{native_api} missing from ledger"
            ledger_row = ledger_by_symbol[native_api]
            assert capability.get("name") == native_api, f"{native_api} matrix name must equal native_api"
            assert capability.get("status") == ledger_row.get("gap_status"), (
                f"{native_api} matrix status drifted from ledger gap_status"
            )
            assert capability.get("gap_status") == ledger_row.get("gap_status"), (
                f"{native_api} matrix gap_status drifted from ledger"
            )
            for field in sorted(LEDGER_ALIGNED_FIELDS):
                assert capability.get(field) == ledger_row.get(field), (
                    f"{native_api} matrix {field} drifted from ledger"
                )
            assert capability.get("coverage_kind") in VALID_COVERAGE_KINDS, (
                f"{native_api} invalid coverage_kind"
            )
            assert capability.get("gap_status") in VALID_GAP_STATUSES, (
                f"{native_api} invalid gap_status"
            )
            policy_category = capability.get("policy_category")
            assert policy_category in VALID_POLICY_CATEGORIES, (
                f"{native_api} invalid policy_category: {policy_category!r}"
            )
            assert isinstance(capability.get("gate_evidence"), list), (
                f"{native_api} gate_evidence must be a list"
            )
            if capability.get("gap_status") == "covered":
                assert policy_category == "direct_covered", (
                    f"{native_api} covered row must use policy_category=direct_covered"
                )
                for field in ("picoui_api", "backend_proof", "unit_test"):
                    assert capability.get(field), f"{native_api} covered row missing {field}"
                _assert_public_picoui_api(native_api, capability.get("picoui_api"), public_picoui_symbols)
                assert capability.get("gate_evidence"), (
                    f"{native_api} covered row missing gate_evidence"
                )
            if capability.get("gap_status") == "allowlisted":
                assert capability.get("required") is False, (
                    f"{native_api} allowlisted row must be required=false"
                )
                assert policy_category != "direct_covered", (
                    f"{native_api} allowlisted row must not use policy_category=direct_covered"
                )
                assert capability.get("coverage_kind") in ALLOWLISTED_COVERAGE_KINDS, (
                    f"{native_api} allowlisted row has invalid coverage_kind"
                )
                assert capability.get("allowlist_reason"), (
                    f"{native_api} allowlisted row missing allowlist_reason"
                )
                direct_100_category = capability.get("direct_100_category")
                assert direct_100_category in VALID_DIRECT_100_CATEGORIES, (
                    f"{native_api} allowlisted row has invalid direct_100_category: "
                    f"{direct_100_category!r}"
                )
                assert capability.get("allowlist_reason") == ledger_row.get("allowlist_reason"), (
                    f"{native_api} matrix allowlist_reason drifted from ledger"
                )
                assert capability.get("capability_release_judgement") == "allowlisted", (
                    f"{native_api} allowlisted row must be judged allowlisted"
                )
            else:
                assert "allowlist_reason" not in capability, (
                    f"{native_api} non-allowlisted row must not carry allowlist_reason"
                )
                assert capability.get("capability_release_judgement") != "final_release_ready", (
                    f"{native_api} missing/non-allowlisted R1 row must not be final_release_ready"
                )
            if capability.get("gap_status") == "overwrapped":
                assert capability.get("notes"), f"{native_api} overwrapped row missing notes"
    assert seen == set(ledger_by_symbol), (
        "matrix native API rows must match native_api_gap_ledger rows:\n"
        f"missing={sorted(set(ledger_by_symbol) - seen)[:25]}\n"
        f"extra={sorted(seen - set(ledger_by_symbol))[:25]}"
    )
    return len(seen)


def _assert_summary(matrix: dict, capability_total: int, ledger_by_symbol: dict[str, dict]) -> None:
    summary = matrix.get("summary")
    assert isinstance(summary, dict), "release matrix missing summary"
    matrix_rows = [
        capability
        for widget in matrix.get("widgets", [])
        for capability in widget.get("capabilities", [])
    ]
    expected_coverage_counts: dict[str, int] = {}
    expected_gap_counts: dict[str, int] = {}
    expected_policy_counts: dict[str, int] = {}
    expected_direct_100_category_counts: dict[str, int] = {}
    expected_group_kind_counts: dict[str, int] = {}
    for row in ledger_by_symbol.values():
        coverage_kind = row["coverage_kind"]
        gap_status = row["gap_status"]
        policy_category = row["policy_category"]
        group_kind = row["group_kind"]
        expected_coverage_counts[coverage_kind] = expected_coverage_counts.get(coverage_kind, 0) + 1
        expected_gap_counts[gap_status] = expected_gap_counts.get(gap_status, 0) + 1
        expected_policy_counts[policy_category] = expected_policy_counts.get(policy_category, 0) + 1
        expected_group_kind_counts[group_kind] = expected_group_kind_counts.get(group_kind, 0) + 1
        if gap_status == "allowlisted":
            direct_100_category = row["direct_100_category"]
            expected_direct_100_category_counts[direct_100_category] = (
                expected_direct_100_category_counts.get(direct_100_category, 0) + 1
            )

    assert summary.get("ldgui_public_api_total") == len(ledger_by_symbol)
    assert summary.get("capability_entry_total") == capability_total
    assert summary.get("native_api_matrix_row_total") == capability_total
    assert summary.get("capability_entry_total") == len(matrix_rows)
    assert summary.get("native_api_matrix_row_total") == len(matrix_rows)
    assert summary.get("gap_status_counts") == dict(sorted(expected_gap_counts.items()))
    assert summary.get("coverage_kind_counts") == dict(sorted(expected_coverage_counts.items()))
    assert summary.get("policy_category_counts") == dict(sorted(expected_policy_counts.items()))
    assert summary.get("direct_100_category_counts") == dict(
        sorted(expected_direct_100_category_counts.items())
    )
    assert summary.get("group_kind_counts") == dict(sorted(expected_group_kind_counts.items()))
    assert summary.get("missing_gap_total") == sum(
        count for status, count in expected_gap_counts.items() if status not in {"covered", "allowlisted"}
    )
    assert summary.get("allowlisted_total") == expected_gap_counts.get("allowlisted", 0)
    assert summary.get("covered_total") == expected_gap_counts.get("covered", 0)
    assert summary["direct_public_covered_total"] == summary["covered_total"]
    assert summary["policy_allowlisted_total"] == summary["allowlisted_total"]
    assert summary["direct_public_100_complete"] is False
    assert summary.get("widget_row_total") == len(matrix.get("widgets", []))


def _assert_gate_catalog(matrix: dict) -> None:
    gate_catalog = matrix.get("gate_catalog")
    assert isinstance(gate_catalog, dict), "release matrix missing gate_catalog"
    for gate_name, expected_targets in EXPECTED_GATE_CATALOG.items():
        actual_targets = gate_catalog.get(gate_name)
        assert isinstance(actual_targets, list), f"gate_catalog.{gate_name} must be a list"
        assert set(actual_targets) == expected_targets, (
            f"gate_catalog.{gate_name} drifted:\n"
            f"expected={sorted(expected_targets)}\nactual={sorted(actual_targets)}"
        )
    assert isinstance(gate_catalog.get("special_cases"), dict), (
        "gate_catalog.special_cases must stay machine-readable"
    )


def _assert_manual_artifact_policy(matrix: dict) -> None:
    policy = matrix.get("manual_artifact_policy")
    assert isinstance(policy, dict), "release matrix missing manual_artifact_policy"
    for key in ("artifact_entry_exists", "manual_review_required", "manual_reviewed_passed"):
        assert isinstance(policy.get(key), bool), f"manual_artifact_policy.{key} must be boolean"
    assert policy["artifact_entry_exists"] is True
    assert policy["manual_review_required"] is True
    assert policy["manual_reviewed_passed"] is False, (
        "manual_review_required=true must not be counted as manual pass"
    )
    assert policy.get("manual_pass_evidence") in ("not_reviewed", "reviewed_passed")
    if policy["manual_review_required"] and not policy["manual_reviewed_passed"]:
        assert policy.get("manual_pass_evidence") == "not_reviewed"


def main() -> int:
    matrix = _load_json(MATRIX_JSON)
    _assert_matrix_header(matrix)
    by_name = _widgets_by_name(matrix)
    inventory_symbols = _inventory_symbols()
    ledger_by_symbol = _ledger_rows()
    public_picoui_symbols = _public_picoui_api_symbols()
    assert set(ledger_by_symbol).issubset(inventory_symbols), "ledger must be backed by inventory"
    capability_total = _assert_native_api_rows(by_name, ledger_by_symbol, public_picoui_symbols)
    _assert_summary(matrix, capability_total, ledger_by_symbol)
    _assert_gate_catalog(matrix)
    _assert_manual_artifact_policy(matrix)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
