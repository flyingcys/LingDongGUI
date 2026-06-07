import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
MATRIX = ROOT / "tests/picoui/contract/picoui_native_release_matrix.json"
LEDGER = ROOT / "tests/picoui/contract/picoui_native_migration_ledger.json"
STYLE_INVENTORY = ROOT / "tests/picoui/contract/picoui_demo_main_style_inventory.json"
ARTIFACT_MANIFEST = ROOT / "tests/picoui/runtime/picoui_native_artifact_manifest.json"

REQUIRED_PHASES = {"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"}
ALLOWED_WIDGET_STATUS = {"covered", "non_user_capability"}
ALLOWED_STYLE_RESULT = {"covered"}
ALLOWED_RUNTIME_RESULT = {"visible", "runtime_only"}


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _assert(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def _run_checker(path: Path) -> None:
    subprocess.run([sys.executable, str(path)], check=True, cwd=ROOT)


def _validate_matrix_shape(matrix: dict) -> tuple[dict[str, dict], dict[str, dict], dict[str, dict], dict[str, dict]]:
    _assert(matrix.get("schema_version") == 1, "release matrix schema_version must be 1")
    _assert(matrix.get("route") == "v1.0-native", "release matrix route must be v1.0-native")
    _assert(matrix.get("release") == "v1.0.1", "release matrix release must be v1.0.1")

    phases = matrix.get("phases")
    widgets = matrix.get("widgets")
    demos = matrix.get("demos")
    dependencies = matrix.get("dependencies")
    _assert(isinstance(phases, list) and phases, "release matrix phases must be non-empty list")
    _assert(isinstance(widgets, list) and widgets, "release matrix widgets must be non-empty list")
    _assert(isinstance(demos, list) and demos, "release matrix demos must be non-empty list")
    _assert(isinstance(dependencies, list) and dependencies, "release matrix dependencies must be non-empty list")

    by_phase = {}
    for row in phases:
        phase = row.get("phase")
        _assert(isinstance(phase, str) and phase, f"invalid phase row: {row!r}")
        _assert(phase not in by_phase, f"duplicate phase row: {phase}")
        by_phase[phase] = row

    by_widget = {}
    for row in widgets:
        row_id = row.get("id")
        _assert(isinstance(row_id, str) and row_id, f"invalid widget row: {row!r}")
        _assert(row_id not in by_widget, f"duplicate widget row: {row_id}")
        _assert(row.get("status") in ALLOWED_WIDGET_STATUS, f"{row_id} invalid widget status")
        by_widget[row_id] = row

    by_demo = {}
    for row in demos:
        demo = row.get("demo")
        _assert(isinstance(demo, str) and demo, f"invalid demo row: {row!r}")
        _assert(demo not in by_demo, f"duplicate demo row: {demo}")
        _assert(row.get("style_result") in ALLOWED_STYLE_RESULT, f"{demo} invalid style_result")
        _assert(row.get("runtime_result") in ALLOWED_RUNTIME_RESULT, f"{demo} invalid runtime_result")
        by_demo[demo] = row

    by_dependency = {}
    for row in dependencies:
        dep_id = row.get("id")
        _assert(isinstance(dep_id, str) and dep_id, f"invalid dependency row: {row!r}")
        _assert(dep_id not in by_dependency, f"duplicate dependency row: {dep_id}")
        by_dependency[dep_id] = row

    return by_phase, by_widget, by_demo, by_dependency


def main() -> int:
    matrix = _load_json(MATRIX)
    ledger = _load_json(LEDGER)
    style_inventory = _load_json(STYLE_INVENTORY)
    artifact_manifest = _load_json(ARTIFACT_MANIFEST)
    by_phase, by_widget, by_demo, by_dependency = _validate_matrix_shape(matrix)

    _assert(set(by_phase) == REQUIRED_PHASES, f"phase rows must exactly cover {sorted(REQUIRED_PHASES)}")
    _assert(
        by_phase["P8"].get("status") in {"in_progress", "covered"},
        "P8 phase row must be in_progress during execution or covered after closeout",
    )

    ledger_entries = ledger.get("entries")
    _assert(isinstance(ledger_entries, list) and ledger_entries, "ledger entries missing")
    expected_widget_ids = []
    for entry in ledger_entries:
        row_id = entry.get("id")
        if row_id == "P8-release-gate":
            continue
        expected_widget_ids.append(row_id)
        _assert(row_id in by_widget, f"release matrix missing widget row for {row_id}")
        matrix_row = by_widget[row_id]
        _assert(matrix_row.get("status") == entry.get("status"), f"{row_id} status drift between ledger and matrix")
        _assert(matrix_row.get("phase") == entry.get("phase"), f"{row_id} phase drift between ledger and matrix")
        _assert(matrix_row.get("capability") == entry.get("capability"), f"{row_id} capability drift between ledger and matrix")
    _assert(set(by_widget) == set(expected_widget_ids), "release matrix widget rows must mirror ledger entries except P8-release-gate")

    inventory_rows = style_inventory.get("demos")
    manifest_rows = artifact_manifest.get("entries")
    _assert(isinstance(inventory_rows, list) and inventory_rows, "style inventory demos missing")
    _assert(isinstance(manifest_rows, list) and manifest_rows, "artifact manifest entries missing")
    inventory_by_name = {row["name"]: row for row in inventory_rows}
    manifest_by_demo = {row["demo"]: row for row in manifest_rows}
    _assert(set(inventory_by_name) == set(manifest_by_demo), "style inventory and artifact manifest demo sets must match")
    _assert(set(by_demo) == set(inventory_by_name), "release matrix demo rows must match style/runtime truth sources")

    for demo, inventory_row in inventory_by_name.items():
        manifest_row = manifest_by_demo[demo]
        matrix_row = by_demo[demo]
        _assert(matrix_row.get("main_style") == inventory_row.get("main_style"), f"{demo} main_style drift")
        _assert(matrix_row.get("uses_picoui_app") == inventory_row.get("uses_picoui_app"), f"{demo} uses_picoui_app drift")
        _assert(matrix_row.get("migration_phase") == inventory_row.get("migration_phase"), f"{demo} migration_phase drift")
        _assert(matrix_row.get("target") == manifest_row.get("target"), f"{demo} target drift")
        _assert(matrix_row.get("artifact_policy") == manifest_row.get("artifact_policy"), f"{demo} artifact_policy drift")
        _assert(matrix_row.get("runtime_result") == manifest_row.get("artifact_policy"), f"{demo} runtime_result must mirror artifact_policy")
        if manifest_row.get("artifact_policy") == "runtime_only":
            _assert(matrix_row.get("reason") == manifest_row.get("reason"), f"{demo} runtime_only reason drift")

    _assert(
        "default_no_ldgui_runtime_dependency" in by_dependency,
        "release matrix missing default_no_ldgui_runtime_dependency row",
    )
    dep_row = by_dependency["default_no_ldgui_runtime_dependency"]
    _assert(dep_row.get("status") == "covered", "default no-ldgui dependency row must be covered")
    _assert(dep_row.get("checker") == "check_picoui_no_ldgui_runtime_dependency", "dependency checker drift")

    _run_checker(ROOT / "tests/picoui/contract/check_picoui_native_migration_ledger.py")
    _run_checker(ROOT / "tests/picoui/contract/check_picoui_demo_main_style.py")
    _run_checker(ROOT / "tests/picoui/contract/check_picoui_no_ldgui_runtime_dependency.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
