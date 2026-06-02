import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
MATRIX_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_release_capability_matrix.json"
INVENTORY_JSON = ROOT / "tests" / "picoui" / "contract" / "ldgui_public_api_inventory.json"
LEDGER_JSON = ROOT / "tests" / "picoui" / "contract" / "native_api_gap_ledger.json"

A08_SCHEMA_VERSION = "a-0.8-native-api-exhaustiveness-v1"
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
LEDGER_ALIGNED_FIELDS = {
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
        by_name[name] = widget
    return by_name


def _assert_matrix_header(matrix: dict) -> None:
    assert matrix.get("schema_version") == A08_SCHEMA_VERSION
    assert matrix.get("line") == "a-0.8"
    assert matrix.get("stage") == "native-api-exhaustiveness-r1"
    assert matrix.get("purpose") == (
        "PicoUI native API exhaustiveness truth source aligned to LingDongGUI public API "
        "inventory and gap ledger"
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
        rows[symbol] = row
    return rows


def _assert_native_api_rows(by_name: dict[str, dict], ledger_by_symbol: dict[str, dict]) -> int:
    seen: set[str] = set()
    for widget_name, widget in sorted(by_name.items()):
        assert widget.get("widget_status") == "native_api_gap_tracked", (
            f"{widget_name} must remain native_api_gap_tracked in R1"
        )
        assert widget.get("widget_release_judgement") != "final_release_ready", (
            f"{widget_name} must not be final_release_ready during R1"
        )
        capabilities = widget.get("capabilities")
        assert isinstance(capabilities, list) and capabilities, f"{widget_name} missing capabilities"
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
            assert isinstance(capability.get("gate_evidence"), list), (
                f"{native_api} gate_evidence must be a list"
            )
            if capability.get("gap_status") == "covered":
                for field in ("picoui_api", "backend_proof", "unit_test"):
                    assert capability.get(field), f"{native_api} covered row missing {field}"
                assert capability.get("gate_evidence"), (
                    f"{native_api} covered row missing gate_evidence"
                )
            if capability.get("gap_status") == "allowlisted":
                assert capability.get("coverage_kind") in ALLOWLISTED_COVERAGE_KINDS, (
                    f"{native_api} allowlisted row has invalid coverage_kind"
                )
                assert capability.get("allowlist_reason"), (
                    f"{native_api} allowlisted row missing allowlist_reason"
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
    for row in ledger_by_symbol.values():
        coverage_kind = row["coverage_kind"]
        gap_status = row["gap_status"]
        expected_coverage_counts[coverage_kind] = expected_coverage_counts.get(coverage_kind, 0) + 1
        expected_gap_counts[gap_status] = expected_gap_counts.get(gap_status, 0) + 1

    assert summary.get("ldgui_public_api_total") == len(ledger_by_symbol)
    assert summary.get("capability_entry_total") == capability_total
    assert summary.get("native_api_matrix_row_total") == capability_total
    assert summary.get("capability_entry_total") == len(matrix_rows)
    assert summary.get("native_api_matrix_row_total") == len(matrix_rows)
    assert summary.get("gap_status_counts") == dict(sorted(expected_gap_counts.items()))
    assert summary.get("coverage_kind_counts") == dict(sorted(expected_coverage_counts.items()))
    assert summary.get("missing_gap_total") == sum(
        count for status, count in expected_gap_counts.items() if status not in {"covered", "allowlisted"}
    )
    assert summary.get("allowlisted_total") == expected_gap_counts.get("allowlisted", 0)
    assert summary.get("covered_total") == expected_gap_counts.get("covered", 0)
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
    assert set(ledger_by_symbol).issubset(inventory_symbols), "ledger must be backed by inventory"
    capability_total = _assert_native_api_rows(by_name, ledger_by_symbol)
    _assert_summary(matrix, capability_total, ledger_by_symbol)
    _assert_gate_catalog(matrix)
    _assert_manual_artifact_policy(matrix)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
