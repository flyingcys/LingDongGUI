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
}

EXPECTED_V0_1_PARITY_TARGETS = {
    "window",
    "label",
    "button",
    "slider",
}

EXPECTED_V0_2_PARITY_BACKLOG = {
    "checkbox",
    "switch",
    "text",
    "image",
    "list",
}

EXPECTED_NOT_WRAPPED_WIDGETS = {
    "line_edit",
    "keyboard",
    "combo_box",
    "scroll_selecter",
    "progress_bar",
    "progress_wheel",
    "arc",
    "gauge",
    "graph",
    "table",
    "calendar",
    "date_time",
    "clock",
    "qr_code",
    "icon_slider",
    "radial_menu",
    "message_box",
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

EXPECTED_V0_1_PARITY_CAPABILITIES = {
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
        "value_and_get_value",
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

    for widget_name in sorted(EXPECTED_WRAPPED_WIDGETS):
        widget = by_name[widget_name]
        actual_status = widget.get("widget_status")
        assert actual_status == "wrapped", (
            f"{widget_name} must remain wrapped in release matrix, got {actual_status!r}"
        )
        actual_judgement = widget.get("widget_release_judgement")
        assert actual_judgement == "internal_v0_1_known_limitation", (
            f"{widget_name} must remain internal_v0_1_known_limitation, got {actual_judgement!r}"
        )

    for widget_name in sorted(EXPECTED_NOT_WRAPPED_WIDGETS):
        actual = by_name[widget_name].get("widget_status")
        assert actual == "not_wrapped", (
            f"{widget_name} must remain not_wrapped in release matrix, got {actual!r}"
        )
        parity_bucket = by_name[widget_name].get("parity_bucket")
        assert parity_bucket == "post_v0_2_candidate", (
            f"{widget_name} must remain post_v0_2_candidate in release matrix, got {parity_bucket!r}"
        )
        parity_status = by_name[widget_name].get("parity_status")
        assert parity_status == "not_applicable", (
            f"{widget_name} must remain parity not_applicable in release matrix, got {parity_status!r}"
        )


def _assert_parity_layering(by_name: dict[str, dict]) -> None:
    for widget_name in sorted(EXPECTED_V0_1_PARITY_TARGETS):
        widget = by_name[widget_name]
        assert widget.get("widget_status") == "wrapped", (
            f"{widget_name} must stay wrapped before parity layering is evaluated"
        )
        assert widget.get("parity_bucket") == "v0_1_parity_target", (
            f"{widget_name} must be tagged as v0_1_parity_target, got {widget.get('parity_bucket')!r}"
        )
        assert widget.get("parity_status") == "parity_complete", (
            f"{widget_name} must be parity_complete, got {widget.get('parity_status')!r}"
        )

    for widget_name in sorted(EXPECTED_V0_2_PARITY_BACKLOG):
        widget = by_name[widget_name]
        assert widget.get("widget_status") == "wrapped", (
            f"{widget_name} must stay wrapped while still in parity backlog"
        )
        assert widget.get("parity_bucket") == "v0_2_parity_backlog", (
            f"{widget_name} must be tagged as v0_2_parity_backlog, got {widget.get('parity_bucket')!r}"
        )
        assert widget.get("parity_status") == "parity_incomplete", (
            f"{widget_name} must remain parity_incomplete, got {widget.get('parity_status')!r}"
        )

    wrapped_widgets = EXPECTED_V0_1_PARITY_TARGETS | EXPECTED_V0_2_PARITY_BACKLOG
    assert wrapped_widgets == EXPECTED_WRAPPED_WIDGETS, (
        "parity layering must partition all wrapped widgets into v0_1 targets and v0_2 backlog"
    )


def _assert_v0_1_parity_capabilities(by_name: dict[str, dict]) -> None:
    for widget_name, expected_capabilities in EXPECTED_V0_1_PARITY_CAPABILITIES.items():
        widget = by_name[widget_name]
        capabilities = _capabilities_by_name(widget)
        actual_capabilities = set(capabilities)
        missing_capabilities = sorted(expected_capabilities - actual_capabilities)
        assert not missing_capabilities, (
            f"{widget_name} missing v0_1 parity capability rows: {missing_capabilities}"
        )
        for capability_name in sorted(expected_capabilities):
            capability = capabilities[capability_name]
            assert capability.get("status") == "support", (
                f"{widget_name}.{capability_name} must be support for parity_complete, "
                f"got {capability.get('status')!r}"
            )


def _assert_known_limitations(by_name: dict[str, dict]) -> None:
    for widget_name, expected_capabilities in EXPECTED_NON_SUPPORT_CAPABILITIES.items():
        widget = by_name.get(widget_name)
        assert widget is not None, f"release matrix missing widget row for {widget_name}"
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
        assert "manual_artifact" in evidence_layers, (
            f"{widget_name} evidence_layers missing manual_artifact field"
        )
        assert evidence_layers["manual_artifact"] == "manual_review_required", (
            f"{widget_name} evidence_layers.manual_artifact must remain manual_review_required, "
            f"got {evidence_layers['manual_artifact']!r}"
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
        widget_note = manual_artifact["note"]
        assert isinstance(widget_note, str) and widget_note, (
            f"{widget_name} manual_artifact.note must be a non-empty string"
        )
        assert "does not deny" in widget_note and "artifact entry" in widget_note, (
            f"{widget_name} manual_artifact.note must explain that widget-level false does not deny demo-level artifact entries"
        )


def _assert_summary_counts(matrix: dict, by_name: dict[str, dict]) -> None:
    summary = matrix.get("summary")
    if not isinstance(summary, dict):
        raise AssertionError("release matrix missing summary object")

    capability_status_counts = summary.get("capability_status_counts")
    if not isinstance(capability_status_counts, dict):
        raise AssertionError("release matrix summary missing capability_status_counts object")

    parity_bucket_counts = summary.get("parity_bucket_counts")
    if not isinstance(parity_bucket_counts, dict):
        raise AssertionError("release matrix summary missing parity_bucket_counts object")

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
    derived_parity_bucket_counts = {
        "v0_1_parity_target": 0,
        "v0_2_parity_backlog": 0,
        "post_v0_2_candidate": 0,
    }
    derived_parity_complete_total = 0
    derived_parity_incomplete_total = 0

    for widget in by_name.values():
        parity_bucket = widget.get("parity_bucket")
        if parity_bucket not in derived_parity_bucket_counts:
            raise AssertionError(
                f"{widget.get('name')} has unexpected parity_bucket: {parity_bucket!r}"
            )
        derived_parity_bucket_counts[parity_bucket] += 1

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
    assert summary.get("capability_entry_total") == derived_capability_entry_total, (
        "summary.capability_entry_total must match actual capability row count, "
        f"got {summary.get('capability_entry_total')!r} vs {derived_capability_entry_total}"
    )
    assert summary.get("v0_1_parity_target_total") == derived_parity_bucket_counts["v0_1_parity_target"], (
        "summary.v0_1_parity_target_total must match actual v0_1 parity target count, "
        f"got {summary.get('v0_1_parity_target_total')!r} vs {derived_parity_bucket_counts['v0_1_parity_target']}"
    )
    assert summary.get("v0_2_parity_backlog_total") == derived_parity_bucket_counts["v0_2_parity_backlog"], (
        "summary.v0_2_parity_backlog_total must match actual v0_2 parity backlog count, "
        f"got {summary.get('v0_2_parity_backlog_total')!r} vs {derived_parity_bucket_counts['v0_2_parity_backlog']}"
    )
    assert summary.get("parity_complete_total") == derived_parity_complete_total, (
        "summary.parity_complete_total must match actual parity_complete count, "
        f"got {summary.get('parity_complete_total')!r} vs {derived_parity_complete_total}"
    )
    assert summary.get("parity_incomplete_total") == derived_parity_incomplete_total, (
        "summary.parity_incomplete_total must match actual parity_incomplete count, "
        f"got {summary.get('parity_incomplete_total')!r} vs {derived_parity_incomplete_total}"
    )
    assert capability_status_counts == derived_capability_status_counts, (
        "summary.capability_status_counts must match actual capability status counts, "
        f"got {capability_status_counts!r} vs {derived_capability_status_counts!r}"
    )
    assert parity_bucket_counts == derived_parity_bucket_counts, (
        "summary.parity_bucket_counts must match actual parity bucket counts, "
        f"got {parity_bucket_counts!r} vs {derived_parity_bucket_counts!r}"
    )


def main() -> int:
    matrix = _load_matrix()
    by_name = _widgets_by_name(matrix)
    _assert_expected_widgets(by_name)
    _assert_parity_layering(by_name)
    _assert_v0_1_parity_capabilities(by_name)
    _assert_known_limitations(by_name)
    _assert_manual_artifact_fields(by_name)
    _assert_summary_counts(matrix, by_name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
