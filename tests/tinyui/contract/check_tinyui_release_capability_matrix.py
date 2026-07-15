import json
import re
from pathlib import Path

from check_tinyui_public_api import _inventory_rows_by_symbol


ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
MATRIX_JSON = ROOT / "tests" / "tinyui" / "contract" / "tinyui_release_capability_matrix.json"
INVENTORY_JSON = CONTRACT_DIR / "ldgui_public_api_inventory.json"
LEDGER_JSON = CONTRACT_DIR / "native_api_gap_ledger.json"
TINYUI_INCLUDE_DIR = ROOT / "tinyui" / "include"

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
    "missing_tinyui_api",
    "missing_runtime_visible_evidence",
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
    "runtime_private_hook",
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
    "tinyui_api",
    "evidence_tag",
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
        "tinyui_hello_world_demo",
        "tinyui_basic_widgets_demo",
        "tinyui_layout_flex_demo",
        "tinyui_layout_grid_demo",
        "tinyui_theme_showcase_demo",
        "tinyui_settings_panel_demo",
        "tinyui_list_basic_demo",
        "tinyui_progress_bar_basic_demo",
        "tinyui_arc_basic_demo",
        "tinyui_gauge_basic_demo",
        "tinyui_icon_slider_basic_demo",
        "tinyui_radial_menu_basic_demo",
        "tinyui_progress_wheel_basic_demo",
        "tinyui_qrcode_basic_demo",
        "tinyui_message_box_basic_demo",
        "tinyui_date_time_basic_demo",
        "tinyui_clock_basic_demo",
        "tinyui_keyboard_basic_demo",
        "tinyui_line_edit_basic_demo",
        "tinyui_combo_box_basic_demo",
        "tinyui_scroll_selecter_basic_demo",
        "tinyui_table_basic_demo",
        "tinyui_graph_basic_demo",
        "tinyui_calendar_basic_demo",
        "tinyui_animation_basic_demo",
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

EVIDENCE_ALIAS_REWRITES = (
    ("test_tinyui_scroll_selector", "test_tinyui_scroll_selecter"),
)

# policy_never_public may only hide non-user surfaces.
POLICY_NEVER_PUBLIC_ALLOWED = {
    "lifecycle_internal",
    "render_pipeline_internal",
    "memory_internal",
    "runtime_host_internal",
    "layout_solver_internal",
    "runtime_private_hook",
    "native_action_private",
    "enum_only_semantics",
    "base_tree_policy",
    "resource_time_helper_policy",
    "drawing_helper_policy",
}

REQUIRED_EVIDENCE_KEYS = {
    "l1_header",
    "l2_link_test",
    "l3_contract_test",
    "l4_native_test",
    "l5_visual_test",
    "l5_event_test",
}

L5_SCENARIO_BASELINES = {
    "v23_core_vertical": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_core_vertical.ppm",
    "v23_value_instruments": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_value_instruments.ppm",
    "v23_selection_collection": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_selection_collection.ppm",
    "v23_input_data": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_input_data.ppm",
    "v23_media_composite": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_media_composite.ppm",
    "v23_theme_layout_resource": ROOT / "tests" / "tinyui" / "runtime" / "baselines" / "v23_theme_layout_resource.ppm",
}


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _normalize_ledger_aliases(value: object) -> object:
    if isinstance(value, str):
        normalized = value
        for src, dst in EVIDENCE_ALIAS_REWRITES:
            normalized = normalized.replace(src, dst)
        return normalized
    if isinstance(value, list):
        return [_normalize_ledger_aliases(item) for item in value]
    if isinstance(value, dict):
        return {
            key: _normalize_ledger_aliases(item)
            for key, item in value.items()
        }
    return value


def _non_empty_string_set(values: object, field_name: str) -> set[str]:
    assert isinstance(values, list) and values, f"{field_name} must be a non-empty list"
    allowed = {
        value
        for value in values
        if isinstance(value, str) and value
    }
    assert len(allowed) == len(values), f"{field_name} must contain only non-empty unique strings"
    return allowed


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
    for key in ("schema_version", "line", "stage", "purpose"):
        value = matrix.get(key)
        assert isinstance(value, str) and value.strip(), f"release matrix missing {key}"
    truth_source_note = matrix.get("truth_source_note")
    assert isinstance(truth_source_note, str) and truth_source_note.strip(), (
        "release matrix missing truth_source_note"
    )
    status_enums = matrix.get("status_enums")
    assert isinstance(status_enums, dict), "release matrix missing status_enums"
    for key in ("gap_status", "coverage_kind", "capability_release_judgement", "group_kind", "policy_category"):
        _non_empty_string_set(status_enums.get(key), f"status_enums.{key}")
    evidence_enums = matrix.get("evidence_enums")
    _non_empty_string_set(evidence_enums, "evidence_enums")


def _inventory_symbols_from_inventory(inventory: dict) -> set[str]:
    return set(_inventory_rows_by_symbol(inventory))


def _inventory_symbols() -> set[str]:
    inventory = _load_json(INVENTORY_JSON)
    return _inventory_symbols_from_inventory(inventory)


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


def _public_tinyui_api_symbols() -> set[str]:
    symbols: set[str] = set()
    pattern = re.compile(r"\b(tinyui_[A-Za-z0-9_]+)\s*\(")
    for header in TINYUI_INCLUDE_DIR.rglob("*.h"):
        text = header.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            symbols.add(match.group(1))
    return symbols


def _assert_public_tinyui_api(native_api: str, tinyui_api: str, public_tinyui_symbols: set[str]) -> None:
    api_symbols = [part.strip() for part in tinyui_api.split("+")]
    assert api_symbols and all(api_symbols), f"{native_api} has invalid tinyui_api field"
    missing = [symbol for symbol in api_symbols if symbol not in public_tinyui_symbols]
    assert not missing, (
        f"{native_api} covered row references non-public TINYUI API: "
        f"{', '.join(missing)}"
    )


def _assert_native_api_rows(
    by_name: dict[str, dict],
    ledger_by_symbol: dict[str, dict],
    public_tinyui_symbols: set[str],
) -> int:
    status_enums = _load_json(MATRIX_JSON).get("status_enums", {})
    allowed_gap_statuses = _non_empty_string_set(status_enums.get("gap_status"), "status_enums.gap_status")
    declared_capability_release_judgements = _non_empty_string_set(
        status_enums.get("capability_release_judgement"),
        "status_enums.capability_release_judgement",
    )
    allowed_capability_release_judgements = (
        declared_capability_release_judgements
        | allowed_gap_statuses
        | set(VALID_PARITY_STATUSES)
    )
    allowed_widget_release_judgements = set(VALID_PARITY_STATUSES)
    allowed_widget_release_judgements.update(
        f"{status}_not_direct_100"
        for status in VALID_PARITY_STATUSES
        if status != "parity_incomplete"
    )
    seen_widget_statuses: set[str] = set()
    seen: set[str] = set()
    for widget_name, widget in sorted(by_name.items()):
        widget_status = widget.get("widget_status")
        assert isinstance(widget_status, str) and widget_status, f"{widget_name} missing widget_status"
        assert re.fullmatch(r"[a-z0-9_]+", widget_status), (
            f"{widget_name} widget_status must be a machine token: {widget_status!r}"
        )
        seen_widget_statuses.add(widget_status)
        widget_release_judgement = widget.get("widget_release_judgement")
        assert widget_release_judgement in allowed_widget_release_judgements, (
            f"{widget_name} invalid widget_release_judgement: {widget_release_judgement!r}"
        )
        group_kind = widget.get("group_kind")
        parity_status = widget.get("parity_status")
        capabilities = widget.get("capabilities")
        assert isinstance(capabilities, list) and capabilities, f"{widget_name} missing capabilities"
        non_direct_policy_categories = {
            capability.get("policy_category")
            for capability in capabilities
            if capability.get("policy_category") != "direct_covered"
        }
        if group_kind == "widget" and non_direct_policy_categories <= {
            "lifecycle_internal",
            "render_pipeline_internal",
                "runtime_private_hook",
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
        if group_kind == "shared_base" and non_direct_policy_categories <= {
            "base_tree_policy",
            "resource_time_helper_policy",
            "drawing_helper_policy",
        }:
            expected_shared_base_status = (
                "direct_parity_complete"
                if not non_direct_policy_categories
                else "policy_complete"
            )
            assert parity_status == expected_shared_base_status, (
                f"{widget_name} shared_base rows must be {expected_shared_base_status}"
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
            capability_status = capability.get("status")
            assert capability_status in allowed_gap_statuses, (
                f"{native_api} matrix status must stay inside status_enums.gap_status"
            )
            assert capability.get("gap_status") == ledger_row.get("gap_status"), (
                f"{native_api} matrix gap_status drifted from ledger"
            )
            for field in sorted(LEDGER_ALIGNED_FIELDS):
                assert _normalize_ledger_aliases(capability.get(field)) == ledger_row.get(field), (
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
            gate_evidence = capability.get("gate_evidence")
            assert isinstance(gate_evidence, list), (
                f"{native_api} gate_evidence must be a list"
            )
            capability_release_judgement = capability.get("capability_release_judgement")
            assert capability_release_judgement in allowed_capability_release_judgements, (
                f"{native_api} invalid capability_release_judgement: {capability_release_judgement!r}"
            )
            if policy_category == "direct_covered":
                assert policy_category == "direct_covered", (
                    f"{native_api} covered row must use policy_category=direct_covered"
                )
                for field in ("tinyui_api", "evidence_tag", "unit_test"):
                    assert capability.get(field), f"{native_api} covered row missing {field}"
                _assert_public_tinyui_api(native_api, capability.get("tinyui_api"), public_tinyui_symbols)
                assert gate_evidence, (
                    f"{native_api} covered row missing gate_evidence"
                )
            if capability.get("required") is False:
                assert capability.get("required") is False, (
                    f"{native_api} allowlisted row must be required=false"
                )
                assert policy_category != "direct_covered", (
                    f"{native_api} allowlisted row must not use policy_category=direct_covered"
                )
                assert capability.get("coverage_kind") in ALLOWLISTED_COVERAGE_KINDS, (
                    f"{native_api} optional-public/policy row has invalid coverage_kind"
                )
                allowlist_reason = capability.get("allowlist_reason")
                assert isinstance(allowlist_reason, str) and allowlist_reason, (
                    f"{native_api} required=false row missing allowlist_reason"
                )
                direct_100_category = capability.get("direct_100_category")
                assert direct_100_category in VALID_DIRECT_100_CATEGORIES, (
                    f"{native_api} policy row has invalid direct_100_category: "
                    f"{direct_100_category!r}"
                )
                assert allowlist_reason == ledger_row.get("allowlist_reason"), (
                    f"{native_api} matrix allowlist_reason drifted from ledger"
                )
                assert ledger_row.get("required") is False, (
                    f"{native_api} ledger required flag drifted from required=false policy row"
                )
            else:
                assert capability.get("allowlist_reason") in (None, ""), (
                    f"{native_api} required row must not carry allowlist_reason"
                )
                assert capability.get("direct_100_category") in (None, ""), (
                    f"{native_api} required row must not carry direct_100_category"
                )
            if capability.get("gap_status") == "overwrapped":
                assert capability.get("notes"), f"{native_api} overwrapped row missing notes"
    assert seen == set(ledger_by_symbol), (
        "matrix native API rows must match native_api_gap_ledger rows:\n"
        f"missing={sorted(set(ledger_by_symbol) - seen)[:25]}\n"
        f"extra={sorted(seen - set(ledger_by_symbol))[:25]}"
    )
    assert len(seen_widget_statuses) == 1, (
        f"widget_status must stay release-matrix consistent, got: {sorted(seen_widget_statuses)}"
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
    assert summary.get("direct_public_covered_total") == summary.get("covered_total")
    assert summary.get("policy_allowlisted_total") == summary.get("allowlisted_total")
    expected_direct_public_100_complete = (
        expected_direct_100_category_counts == {"policy_never_public": expected_gap_counts.get("allowlisted", 0)}
    )
    assert summary.get("direct_public_100_complete") is expected_direct_public_100_complete
    assert summary.get("widget_row_total") == len(matrix.get("widgets", []))
    release_closeout_note = summary.get("release_closeout_note")
    if release_closeout_note is not None:
        assert isinstance(release_closeout_note, str) and release_closeout_note.strip(), (
            "summary.release_closeout_note must be a non-empty string when present"
        )


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


def _path_exists(path_value: object, *, field: str, native_api: str) -> Path:
    assert isinstance(path_value, str) and path_value.strip(), (
        f"{native_api} {field} must be a non-empty path string"
    )
    path = Path(path_value)
    if not path.is_absolute():
        path = ROOT / path
    assert path.exists(), f"{native_api} {field} path does not exist: {path_value}"
    return path


def _assert_ppm_artifact(path: Path, *, native_api: str, field: str) -> None:
    assert path.suffix.lower() == ".ppm", (
        f"{native_api} {field} must be a .ppm pixel artifact, got {path.name}"
    )
    size = path.stat().st_size
    assert size > 64, f"{native_api} {field} PPM is empty/too small: {path} ({size} B)"
    header = path.read_bytes()[:64]
    assert header.startswith(b"P3") or header.startswith(b"P6"), (
        f"{native_api} {field} is not a PPM image: {path}"
    )


def _assert_not_cross_substituted(
    evidence: dict,
    *,
    native_api: str,
    requires_l5_v: bool,
    requires_l5_e: bool,
) -> None:
    """Reject event-as-pixel or screenshot-as-event substitutions."""
    visual_test = evidence.get("l5_visual_test")
    event_test = evidence.get("l5_event_test")
    visual_artifact = evidence.get("l5_visual_artifact")
    event_artifact = evidence.get("l5_event_artifact")

    if requires_l5_v:
        assert visual_test not in (None, "", "not_applicable"), (
            f"{native_api} requires L5-V but l5_visual_test is missing/not_applicable"
        )
        assert isinstance(visual_artifact, str) and visual_artifact, (
            f"{native_api} requires L5-V but l5_visual_artifact is missing"
        )
        # Event traces / backend mapping tests cannot substitute for pixels.
        assert "backend_mapping" not in str(visual_test), (
            f"{native_api} L5-V must not use backend_mapping/event evidence as pixels"
        )
        assert "event" not in Path(str(visual_artifact)).name.lower() or str(
            visual_artifact
        ).endswith(".ppm"), (
            f"{native_api} L5-V artifact must be pixel evidence, not an event log"
        )
        ppm = _path_exists(visual_artifact, field="l5_visual_artifact", native_api=native_api)
        _assert_ppm_artifact(ppm, native_api=native_api, field="l5_visual_artifact")

    if requires_l5_e:
        assert event_test not in (None, ""), (
            f"{native_api} requires L5-E but l5_event_test is missing"
        )
        if event_test == "not_applicable":
            reason = evidence.get("l5_event_reason")
            assert isinstance(reason, str) and reason.strip(), (
                f"{native_api} L5-E not_applicable requires l5_event_reason"
            )
        else:
            assert event_artifact is not None, (
                f"{native_api} requires L5-E but l5_event_artifact is missing"
            )
            # Screenshots cannot substitute for events.
            if isinstance(event_artifact, str):
                assert not event_artifact.lower().endswith(".ppm"), (
                    f"{native_api} L5-E must not use PPM screenshot as event evidence"
                )
                assert "visible_ui" not in str(event_test), (
                    f"{native_api} L5-E must not use visible_ui/pixel test as event evidence"
                )
            elif isinstance(event_artifact, dict):
                trace = event_artifact.get("trace")
                assert trace not in (None, "", [], {}), (
                    f"{native_api} L5-E artifact missing event trace"
                )
                assert event_artifact.get("kind") in {
                    "widget_event_trace",
                    "family_representative_event",
                    "unit_event_state",
                }, f"{native_api} L5-E artifact has invalid kind: {event_artifact.get('kind')!r}"
            else:
                raise AssertionError(
                    f"{native_api} l5_event_artifact must be path string or trace object"
                )


def _assert_required_release_evidence(matrix: dict, ledger_by_symbol: dict[str, dict]) -> None:
    """M5 Task5: every required covered row must bind real L1-L5 evidence paths."""
    seen_ids: set[str] = set()
    required_covered = 0

    for widget in matrix.get("widgets", []):
        widget_name = widget.get("name")
        assert isinstance(widget_name, str) and widget_name, "widget missing name"
        group_kind = widget.get("group_kind")

        for capability in widget.get("capabilities", []):
            native_api = capability.get("native_api")
            assert isinstance(native_api, str) and native_api, (
                f"{widget_name} capability missing native_api"
            )
            ledger_row = ledger_by_symbol[native_api]
            required = capability.get("required")
            gap_status = capability.get("gap_status")
            policy_category = capability.get("policy_category")
            direct_100_category = capability.get("direct_100_category")

            # policy_never_public may not hide widget user capabilities.
            if required is False and direct_100_category == "policy_never_public":
                assert policy_category in POLICY_NEVER_PUBLIC_ALLOWED, (
                    f"{native_api} policy_never_public uses disallowed policy_category="
                    f"{policy_category!r}"
                )
                # Hard fail: a required=false row still classified as direct user surface.
                if group_kind == "widget" and policy_category == "direct_covered":
                    raise AssertionError(
                        f"{native_api} widget user capability cannot be policy_never_public"
                    )
                # Heuristic: public setter/getter style symbols under widget group with
                # non-internal policy are already blocked above; additionally reject
                # allowlist_reason that admits user-facing capability hiding.
                allowlist_reason = (capability.get("allowlist_reason") or "").lower()
                # Only fail on explicit admission of hiding user capability.
                forbidden_phrases = (
                    "hide user capability",
                    "规避用户能力",
                    "隐藏用户能力",
                    "to avoid implementing",
                    "skip user-facing",
                )
                for phrase in forbidden_phrases:
                    assert phrase not in allowlist_reason, (
                        f"{native_api} policy_never_public appears to hide user capability: "
                        f"{allowlist_reason!r}"
                    )

            if required is not True or gap_status != "covered":
                continue

            required_covered += 1
            capability_id = capability.get("capability_id")
            assert isinstance(capability_id, str) and capability_id.strip(), (
                f"{native_api} required covered row missing capability_id"
            )
            assert capability_id not in seen_ids, (
                f"duplicate capability_id: {capability_id}"
            )
            seen_ids.add(capability_id)

            public_symbol = capability.get("public_symbol")
            public_header = capability.get("public_header")
            assert isinstance(public_symbol, str) and public_symbol.startswith("tinyui_"), (
                f"{native_api} missing public_symbol"
            )
            assert isinstance(public_header, str) and public_header.endswith(".h"), (
                f"{native_api} missing public_header"
            )
            header_path = _path_exists(public_header, field="public_header", native_api=native_api)
            header_text = header_path.read_text(encoding="utf-8", errors="replace")
            assert re.search(rf"\b{re.escape(public_symbol)}\s*\(", header_text), (
                f"{native_api} public_symbol {public_symbol} not declared in {public_header}"
            )

            evidence = capability.get("evidence")
            assert isinstance(evidence, dict), f"{native_api} missing evidence object"
            missing_keys = sorted(REQUIRED_EVIDENCE_KEYS - set(evidence))
            assert not missing_keys, (
                f"{native_api} evidence missing keys: {missing_keys}"
            )

            l1 = evidence.get("l1_header")
            assert isinstance(l1, str) and ":" in l1, (
                f"{native_api} evidence.l1_header must be path:symbol"
            )
            l1_path, l1_symbol = l1.split(":", 1)
            assert l1_path == public_header, (
                f"{native_api} l1_header path drifted from public_header"
            )
            assert l1_symbol == public_symbol, (
                f"{native_api} l1_header symbol drifted from public_symbol"
            )

            for field in ("l2_link_test", "l3_contract_test", "l4_native_test"):
                value = evidence.get(field)
                assert isinstance(value, str) and value.strip(), (
                    f"{native_api} evidence.{field} must be non-empty"
                )
                assert value not in {"pending", "todo", "missing", "smoke"}, (
                    f"{native_api} evidence.{field} rejects placeholder {value!r}"
                )

            # L4 unit source must exist (real backend-state harness, not smoke-only claim).
            l4_unit_source = evidence.get("l4_unit_source")
            if isinstance(l4_unit_source, str) and l4_unit_source:
                unit_path = _path_exists(
                    l4_unit_source, field="l4_unit_source", native_api=native_api
                )
                assert unit_path.suffix == ".c", (
                    f"{native_api} l4_unit_source must be a C unit file: {l4_unit_source}"
                )
                unit_text = unit_path.read_text(encoding="utf-8", errors="replace")
                # Reject empty/smoke-only files.
                assert "tinyui_" in unit_text and (
                    "ld" in unit_text.lower() or "backend" in unit_text.lower() or "assert" in unit_text
                ), f"{native_api} l4_unit_source does not look like a real L4 harness: {l4_unit_source}"

            requires_l5_v = bool(
                capability.get("requires_l5_v", ledger_row.get("requires_l5_v"))
            )
            requires_l5_e = bool(
                capability.get("requires_l5_e", ledger_row.get("requires_l5_e"))
            )
            assert capability.get("requires_l5_v") is requires_l5_v or capability.get(
                "requires_l5_v"
            ) is None, f"{native_api} requires_l5_v drifted from ledger"
            # Prefer explicit matrix flags when present; otherwise ledger.
            if "requires_l5_v" in capability:
                assert capability.get("requires_l5_v") is bool(ledger_row.get("requires_l5_v")), (
                    f"{native_api} requires_l5_v must match ledger"
                )
            if "requires_l5_e" in capability:
                assert capability.get("requires_l5_e") is bool(ledger_row.get("requires_l5_e")), (
                    f"{native_api} requires_l5_e must match ledger"
                )
            requires_l5_v = bool(ledger_row.get("requires_l5_v"))
            requires_l5_e = bool(ledger_row.get("requires_l5_e"))

            _assert_not_cross_substituted(
                evidence,
                native_api=native_api,
                requires_l5_v=requires_l5_v,
                requires_l5_e=requires_l5_e,
            )

            # Widget-level L5 scenario baseline must exist when widget binds v23 scenario.
            scenario = widget.get("v23_m3_scenario")
            if isinstance(scenario, str) and scenario in L5_SCENARIO_BASELINES:
                baseline = L5_SCENARIO_BASELINES[scenario]
                assert baseline.exists(), (
                    f"{widget_name} scenario {scenario} missing baseline {baseline}"
                )

    assert required_covered > 0, "no required covered capabilities found"
    assert len(seen_ids) == required_covered, "capability_id count mismatch"


def main() -> int:
    matrix = _load_json(MATRIX_JSON)
    _assert_matrix_header(matrix)
    by_name = _widgets_by_name(matrix)
    inventory_symbols = _inventory_symbols()
    ledger_by_symbol = _ledger_rows()
    public_tinyui_symbols = _public_tinyui_api_symbols()
    assert set(ledger_by_symbol).issubset(inventory_symbols), "ledger must be backed by inventory"
    capability_total = _assert_native_api_rows(by_name, ledger_by_symbol, public_tinyui_symbols)
    _assert_summary(matrix, capability_total, ledger_by_symbol)
    _assert_gate_catalog(matrix)
    _assert_manual_artifact_policy(matrix)
    _assert_required_release_evidence(matrix, ledger_by_symbol)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
