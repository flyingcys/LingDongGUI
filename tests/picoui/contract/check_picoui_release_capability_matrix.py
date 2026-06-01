import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
MATRIX_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_release_capability_matrix.json"
INVENTORY_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_native_100_inventory.json"

A07_SCHEMA_VERSION = "a-0.7-native-100-v1"
EXPECTED_EVIDENCE_LAYERS = {
    "unit": "present",
    "contract": "present",
    "mapping": "present",
    "visible": "present",
    "manual_artifact": "manual_review_required",
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
    assert matrix.get("schema_version") == A07_SCHEMA_VERSION
    assert matrix.get("line") == "a-0.7"
    assert matrix.get("stage") == "native-100-closeout"
    assert matrix.get("purpose") == "PicoUI native-100 truth source and gate catalog"


def _assert_widgets_match_inventory(matrix: dict, by_name: dict[str, dict]) -> None:
    inventory = _load_json(INVENTORY_JSON)
    inventory_names = {widget["name"] for widget in inventory.get("widgets", [])}
    assert set(by_name) == inventory_names, (
        "matrix widget rows must match native inventory rows:\n"
        f"matrix={sorted(by_name)}\ninventory={sorted(inventory_names)}"
    )
    assert len(by_name) == 27, f"matrix must contain 27 widget rows, got {len(by_name)}"


def _assert_widget_rows(by_name: dict[str, dict]) -> int:
    capability_total = 0
    for widget_name, widget in sorted(by_name.items()):
        assert widget.get("widget_status") == "wrapped", f"{widget_name} must be wrapped"
        assert widget.get("current_layer") == "full_parity_complete", (
            f"{widget_name} must be full_parity_complete"
        )
        assert widget.get("parity_status") == "parity_complete", (
            f"{widget_name} must be parity_complete"
        )
        assert widget.get("widget_release_judgement") == "final_release_ready", (
            f"{widget_name} must be final_release_ready"
        )
        evidence_layers = widget.get("evidence_layers")
        assert evidence_layers == EXPECTED_EVIDENCE_LAYERS, (
            f"{widget_name} evidence_layers drifted: {evidence_layers!r}"
        )
        manual_artifact = widget.get("manual_artifact")
        assert isinstance(manual_artifact, dict), f"{widget_name} missing manual_artifact object"
        assert manual_artifact.get("scope") == "demo_level_final_release"
        assert manual_artifact.get("artifact_entry_exists") is True
        assert manual_artifact.get("manual_review_required") is True
        capabilities = widget.get("capabilities")
        assert isinstance(capabilities, list) and capabilities, f"{widget_name} missing capabilities"
        for capability in capabilities:
            capability_total += 1
            assert capability.get("status") == "support", (
                f"{widget_name}.{capability.get('name')} must be support"
            )
            assert capability.get("capability_release_judgement") == "final_release_ready", (
                f"{widget_name}.{capability.get('name')} must be final_release_ready"
            )
    return capability_total


def _assert_summary(matrix: dict, capability_total: int) -> None:
    summary = matrix.get("summary")
    assert isinstance(summary, dict), "release matrix missing summary"
    assert summary.get("ldgui_widget_like_total") == 27
    assert summary.get("picoui_native_wrapped_total") == 27
    assert summary.get("ldgui_wrappable_widget_total") == 27
    assert summary.get("picoui_wrapped_widget_total") == 27
    assert summary.get("picoui_not_wrapped_widget_total") == 0
    assert summary.get("missing_implementation_total") == 0
    assert summary.get("full_parity_complete_total") == 27
    assert summary.get("stable_contract_but_not_full_parity_total") == 0
    assert summary.get("minimal_vertical_slice_only_total") == 0
    assert summary.get("parity_complete_total") == 27
    assert summary.get("parity_incomplete_total") == 0
    assert summary.get("capability_entry_total") == capability_total
    assert summary.get("capability_status_counts") == {"support": capability_total}
    assert summary.get("current_layer_counts") == {"full_parity_complete": 27}


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
    assert gate_catalog.get("special_cases") == {}, "a-0.7 must not keep formal gate special_cases"


def main() -> int:
    matrix = _load_json(MATRIX_JSON)
    _assert_matrix_header(matrix)
    by_name = _widgets_by_name(matrix)
    _assert_widgets_match_inventory(matrix, by_name)
    capability_total = _assert_widget_rows(by_name)
    _assert_summary(matrix, capability_total)
    _assert_gate_catalog(matrix)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
