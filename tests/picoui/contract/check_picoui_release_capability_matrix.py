import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
MATRIX_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_release_capability_matrix.json"

EXPECTED_WRAPPED_WIDGETS = {
    "window",
    "label",
    "button",
    "checkbox",
    "switch",
    "slider",
    "text",
    "image",
    "list",
    "line_edit",
    "progress_bar",
    "qrcode",
    "progress_wheel",
    "message_box",
    "date_time",
    "clock",
    "keyboard",
    "combo_box",
    "scroll_selecter",
    "graph",
    "table",
    "calendar",
}

EXPECTED_NOT_WRAPPED_WIDGETS = {
    "arc",
    "gauge",
    "icon_slider",
    "radial_menu",
}

EXPECTED_FULL_PARITY_COMPLETE = {
    "window",
    "label",
    "button",
    "slider",
}

EXPECTED_STABLE_CONTRACT = {
    "checkbox",
    "switch",
    "text",
    "image",
    "list",
    "line_edit",
    "keyboard",
    "combo_box",
    "scroll_selecter",
    "graph",
    "table",
    "calendar",
}

EXPECTED_MINIMAL_VERTICAL_SLICE = {
    "progress_bar",
    "qrcode",
    "progress_wheel",
    "message_box",
    "date_time",
    "clock",
}

EXPECTED_NON_SUPPORT_CAPABILITIES = {
    "image": {
        "theme": "reject",
        "style_class": "incomplete_contract",
        "user_data": "incomplete_contract",
        "bg_text_border_radius_colors": "reject",
        "padding": "deferred",
        "enabled": "reject",
    },
    "list": {
        "item_marker": "reject",
        "style_class": "incomplete_contract",
        "widget_level_user_data": "incomplete_contract",
    },
}

EXPECTED_J_PARITY_CAPABILITIES = {
    "window": {
        "create_and_props",
        "flex_layout",
        "grid_layout",
        "padding_gap_align",
        "bg_color",
        "background_image_and_mask",
        "padding_group_contract",
        "honest_minimal_readback",
    },
    "label": {
        "create_and_props",
        "text",
        "font",
        "bg_and_text_color",
        "transparent",
        "align",
        "background_image_and_mask",
        "text_color_bg_color_align_transparent_readback",
    },
    "button": {
        "create_and_props",
        "text",
        "clicked_pressed_released_callbacks",
        "bg_text_border_radius_padding_subset",
        "release_and_press_image",
        "transparent",
        "font",
        "checkable",
        "key_value",
        "pressed_state",
    },
    "slider": {
        "create_and_props",
        "value",
        "range",
        "on_value_changed",
        "horizontal",
        "background_and_indicator_image_mask",
        "indicator_width",
        "slim_size",
        "percent_and_orientation_readback_consistency",
    },
}


def _load_matrix() -> dict:
    return json.loads(MATRIX_JSON.read_text(encoding="utf-8"))


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


def _capabilities_by_name(widget: dict) -> dict[str, dict]:
    capabilities = widget.get("capabilities")
    if not isinstance(capabilities, list):
        raise AssertionError(f"{widget.get('name')} missing capabilities list")
    by_name: dict[str, dict] = {}
    for capability in capabilities:
        name = capability.get("name")
        if not isinstance(name, str) or not name:
            raise AssertionError(f"{widget.get('name')} has capability with invalid name: {capability!r}")
        if name in by_name:
            raise AssertionError(f"{widget.get('name')} has duplicate capability row: {name}")
        by_name[name] = capability
    return by_name


def _assert_expected_widgets(by_name: dict[str, dict]) -> None:
    missing_wrapped = sorted(EXPECTED_WRAPPED_WIDGETS - set(by_name))
    assert not missing_wrapped, f"release matrix missing wrapped widgets: {missing_wrapped}"

    missing_not_wrapped = sorted(EXPECTED_NOT_WRAPPED_WIDGETS - set(by_name))
    assert not missing_not_wrapped, f"release matrix missing not_wrapped widgets: {missing_not_wrapped}"

    assert "qr_code" not in by_name, "legacy widget name qr_code must be normalized to qrcode"

    for widget_name in sorted(EXPECTED_WRAPPED_WIDGETS):
        widget = by_name[widget_name]
        actual_status = widget.get("widget_status")
        assert actual_status == "wrapped", (
            f"{widget_name} must remain wrapped in release matrix, got {actual_status!r}"
        )

    for widget_name in sorted(EXPECTED_NOT_WRAPPED_WIDGETS):
        widget = by_name[widget_name]
        actual = widget.get("widget_status")
        assert actual == "not_wrapped", (
            f"{widget_name} must remain not_wrapped in release matrix, got {actual!r}"
        )
        parity_status = widget.get("parity_status")
        assert parity_status == "not_applicable", (
            f"{widget_name} must remain parity not_applicable in release matrix, got {parity_status!r}"
        )
        current_layer = widget.get("current_layer")
        assert current_layer == "not_wrapped", (
            f"{widget_name} must remain current_layer not_wrapped, got {current_layer!r}"
        )


def _assert_current_layers(by_name: dict[str, dict]) -> None:
    for widget_name in sorted(EXPECTED_FULL_PARITY_COMPLETE):
        widget = by_name[widget_name]
        assert widget.get("current_layer") == "full_parity_complete", (
            f"{widget_name} must be full_parity_complete, got {widget.get('current_layer')!r}"
        )
        assert widget.get("parity_status") == "parity_complete", (
            f"{widget_name} must remain parity_complete, got {widget.get('parity_status')!r}"
        )
        assert widget.get("widget_release_judgement") == "current_15_parity_complete", (
            f"{widget_name} must use current_15_parity_complete judgement"
        )

    for widget_name in sorted(EXPECTED_STABLE_CONTRACT):
        widget = by_name[widget_name]
        assert widget.get("current_layer") == "stable_contract_but_not_full_parity", (
            f"{widget_name} must be stable_contract_but_not_full_parity, got {widget.get('current_layer')!r}"
        )
        assert widget.get("parity_status") == "parity_incomplete", (
            f"{widget_name} must remain parity_incomplete, got {widget.get('parity_status')!r}"
        )
        assert widget.get("widget_release_judgement") == "current_15_stable_contract_gap", (
            f"{widget_name} must use current_15_stable_contract_gap judgement"
        )

    for widget_name in sorted(EXPECTED_MINIMAL_VERTICAL_SLICE):
        widget = by_name[widget_name]
        assert widget.get("current_layer") == "minimal_vertical_slice_only", (
            f"{widget_name} must be minimal_vertical_slice_only, got {widget.get('current_layer')!r}"
        )
        assert widget.get("parity_status") == "parity_incomplete", (
            f"{widget_name} must remain parity_incomplete, got {widget.get('parity_status')!r}"
        )
        assert widget.get("widget_release_judgement") == "current_15_minimal_slice_gap", (
            f"{widget_name} must use current_15_minimal_slice_gap judgement"
        )

    all_wrapped = (
        EXPECTED_FULL_PARITY_COMPLETE
        | EXPECTED_STABLE_CONTRACT
        | EXPECTED_MINIMAL_VERTICAL_SLICE
    )
    assert all_wrapped == EXPECTED_WRAPPED_WIDGETS, (
        "current-15 layer groups must partition all wrapped widgets"
    )


def _assert_expected_capabilities(by_name: dict[str, dict]) -> None:
    for widget_name, expected_capabilities in EXPECTED_J_PARITY_CAPABILITIES.items():
        widget = by_name[widget_name]
        capabilities = _capabilities_by_name(widget)
        actual_capabilities = set(capabilities)
        missing_capabilities = sorted(expected_capabilities - actual_capabilities)
        assert not missing_capabilities, (
            f"{widget_name} missing parity capability rows: {missing_capabilities}"
        )
        for capability_name in sorted(expected_capabilities):
            capability = capabilities[capability_name]
            assert capability.get("status") == "support", (
                f"{widget_name}.{capability_name} must be support, got {capability.get('status')!r}"
            )

    for widget_name, expected_capabilities in EXPECTED_NON_SUPPORT_CAPABILITIES.items():
        widget = by_name[widget_name]
        capabilities = _capabilities_by_name(widget)
        for capability_name, expected_status in expected_capabilities.items():
            capability = capabilities.get(capability_name)
            assert capability is not None, (
                f"{widget_name} missing capability row for {capability_name}"
            )
            actual_status = capability.get("status")
            assert actual_status == expected_status, (
                f"{widget_name}.{capability_name} must remain {expected_status}, got {actual_status!r}"
            )


def _assert_manual_artifact_fields(by_name: dict[str, dict]) -> None:
    matrix = _load_matrix()
    policy = matrix.get("manual_artifact_policy")
    if not isinstance(policy, dict):
        raise AssertionError("release matrix missing manual_artifact_policy object")
    for field_name in (
        "scope",
        "artifact_entry_exists",
        "manual_review_required",
        "note",
        "demo_artifact_truth_source",
    ):
        assert field_name in policy, f"manual_artifact_policy missing field: {field_name}"
    assert policy["scope"] == "widget_level_only", (
        "manual_artifact_policy.scope must remain widget_level_only"
    )
    assert policy["manual_review_required"] is True, (
        "manual_artifact_policy.manual_review_required must remain true"
    )
    assert policy["demo_artifact_truth_source"] == (
        "docs/picoui-serial/C-线人工窗口验收记录.md"
    ), "manual_artifact_policy.demo_artifact_truth_source must point to C-line manual record"
    note = policy["note"]
    assert isinstance(note, str) and note, "manual_artifact_policy.note must be a non-empty string"
    assert "widget-level only" in note and "C-线人工窗口验收记录.md" in note, (
        "manual_artifact_policy.note must explain widget-level-only scope and point to the demo-level truth source"
    )

    for widget_name in sorted(EXPECTED_WRAPPED_WIDGETS):
        widget = by_name[widget_name]
        evidence_layers = widget.get("evidence_layers")
        assert isinstance(evidence_layers, dict), f"{widget_name} missing evidence_layers object"
        assert evidence_layers.get("manual_artifact") == "manual_review_required", (
            f"{widget_name} evidence_layers.manual_artifact must remain manual_review_required"
        )
        manual_artifact = widget.get("manual_artifact")
        assert isinstance(manual_artifact, dict), f"{widget_name} missing manual_artifact object"
        for field_name in ("scope", "artifact_entry_exists", "manual_review_required", "note"):
            assert field_name in manual_artifact, (
                f"{widget_name} manual_artifact missing field: {field_name}"
            )
        assert manual_artifact["scope"] == "widget_level_only", (
            f"{widget_name} manual_artifact.scope must remain widget_level_only"
        )
        assert manual_artifact["manual_review_required"] is True, (
            f"{widget_name} manual_artifact.manual_review_required must remain true"
        )


def _assert_summary_counts(matrix: dict, by_name: dict[str, dict]) -> None:
    summary = matrix.get("summary")
    if not isinstance(summary, dict):
        raise AssertionError("release matrix missing summary object")

    capability_status_counts = summary.get("capability_status_counts")
    if not isinstance(capability_status_counts, dict):
        raise AssertionError("release matrix summary missing capability_status_counts object")

    current_layer_counts = summary.get("current_layer_counts")
    if not isinstance(current_layer_counts, dict):
        raise AssertionError("release matrix summary missing current_layer_counts object")

    derived_widget_total = len(by_name)
    derived_wrapped_total = sum(
        1 for widget in by_name.values() if widget.get("widget_status") == "wrapped"
    )
    derived_not_wrapped_total = sum(
        1 for widget in by_name.values() if widget.get("widget_status") == "not_wrapped"
    )
    derived_capability_entry_total = 0
    derived_capability_status_counts = {
        "support": 0,
        "reject": 0,
        "incomplete_contract": 0,
        "deferred": 0,
    }
    derived_current_layer_counts = {
        "full_parity_complete": 0,
        "stable_contract_but_not_full_parity": 0,
        "minimal_vertical_slice_only": 0,
        "not_wrapped": 0,
    }
    derived_parity_complete_total = 0
    derived_parity_incomplete_total = 0

    for widget in by_name.values():
        current_layer = widget.get("current_layer")
        if current_layer not in derived_current_layer_counts:
            raise AssertionError(
                f"{widget.get('name')} has unexpected current_layer: {current_layer!r}"
            )
        derived_current_layer_counts[current_layer] += 1

        parity_status = widget.get("parity_status")
        if parity_status == "parity_complete":
            derived_parity_complete_total += 1
        elif parity_status == "parity_incomplete":
            derived_parity_incomplete_total += 1
        elif parity_status != "not_applicable":
            raise AssertionError(
                f"{widget.get('name')} has unexpected parity_status: {parity_status!r}"
            )

        for capability in widget.get("capabilities", []):
            status = capability.get("status")
            if status not in derived_capability_status_counts:
                raise AssertionError(
                    f"{widget.get('name')} has capability with unexpected status: {status!r}"
                )
            derived_capability_entry_total += 1
            derived_capability_status_counts[status] += 1

    assert summary.get("ldgui_wrappable_widget_total") == derived_widget_total, (
        "summary.ldgui_wrappable_widget_total must match actual widget row count, "
        f"got {summary.get('ldgui_wrappable_widget_total')!r} vs {derived_widget_total}"
    )
    assert summary.get("picoui_wrapped_widget_total") == derived_wrapped_total, (
        "summary.picoui_wrapped_widget_total must match wrapped widget row count, "
        f"got {summary.get('picoui_wrapped_widget_total')!r} vs {derived_wrapped_total}"
    )
    assert summary.get("picoui_not_wrapped_widget_total") == derived_not_wrapped_total, (
        "summary.picoui_not_wrapped_widget_total must match not_wrapped widget row count, "
        f"got {summary.get('picoui_not_wrapped_widget_total')!r} vs {derived_not_wrapped_total}"
    )
    assert summary.get("full_parity_complete_total") == derived_current_layer_counts["full_parity_complete"], (
        "summary.full_parity_complete_total must match actual full_parity_complete count"
    )
    assert summary.get("stable_contract_but_not_full_parity_total") == derived_current_layer_counts["stable_contract_but_not_full_parity"], (
        "summary.stable_contract_but_not_full_parity_total must match actual stable_contract count"
    )
    assert summary.get("minimal_vertical_slice_only_total") == derived_current_layer_counts["minimal_vertical_slice_only"], (
        "summary.minimal_vertical_slice_only_total must match actual minimal slice count"
    )
    assert summary.get("parity_complete_total") == derived_parity_complete_total, (
        "summary.parity_complete_total must match actual parity_complete count"
    )
    assert summary.get("parity_incomplete_total") == derived_parity_incomplete_total, (
        "summary.parity_incomplete_total must match actual parity_incomplete count"
    )
    assert summary.get("capability_entry_total") == derived_capability_entry_total, (
        "summary.capability_entry_total must match actual capability row count, "
        f"got {summary.get('capability_entry_total')!r} vs {derived_capability_entry_total}"
    )
    assert capability_status_counts == derived_capability_status_counts, (
        "summary.capability_status_counts must match actual capability status counts, "
        f"got {capability_status_counts!r} vs {derived_capability_status_counts!r}"
    )
    assert current_layer_counts == derived_current_layer_counts, (
        "summary.current_layer_counts must match actual current layer counts, "
        f"got {current_layer_counts!r} vs {derived_current_layer_counts!r}"
    )


def main() -> int:
    matrix = _load_matrix()
    by_name = _widgets_by_name(matrix)
    _assert_expected_widgets(by_name)
    _assert_current_layers(by_name)
    _assert_expected_capabilities(by_name)
    _assert_manual_artifact_fields(by_name)
    _assert_summary_counts(matrix, by_name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
