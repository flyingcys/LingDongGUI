#!/usr/bin/env python3
"""Fail-closed aggregator for TinyUI v2.3 release gates.

CLI::

    python3 tests/tinyui/contract/check_tinyui_v23_release_gates.py \\
        --build-dir build/v2.3 \\
        --manifest tests/tinyui/contract/tinyui_v23_release_gate_manifest.json \\
        --artifact-root build/v2.3/release-evidence

Behavior:
  * inventory every required CTest via ``ctest --show-only=json-v1``
  * require each required test to exist and not be disabled
  * require closeout artifacts under ``--artifact-root`` (relative paths only)
  * reject empty evidence, ``fallback=true``, non-pass required statuses, unknown fields
  * verify design thresholds embedded in the manifest
  * never mark a missing/failed gate as passed

Task1 intentionally fails until fresh standard/ASan/UBSan/minimal/review/ABI
artifacts exist and every formal CTest name is registered.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = Path(__file__).resolve().parent
DEFAULT_MANIFEST = CONTRACT_DIR / "tinyui_v23_release_gate_manifest.json"

MANIFEST_TOP_LEVEL_KEYS = {
    "schema_version",
    "version",
    "port_completed",
    "abi_frozen",
    "manual_reviewed_passed",
    "required_ctests",
    "artifacts",
    "thresholds",
    "references",
    "implementation_notes",
}

REQUIRED_CTEST_KEYS = {"name", "category", "required"}
ARTIFACT_KEYS = {"path", "required_for_closeout", "description"}
ARTIFACT_PAYLOAD_ALLOWED_EXTRA = {
    "schema_version",
    "version",
    "status",
    "fallback",
    "profile",
    "evidence",
    "sha256",
    "commit",
    "build_dir",
    "summary",
    "gates",
    "results",
    "notes",
    "manual_reviewed_passed",
    "abi_frozen",
    "port_completed",
    "generated_by",
    "references",
}

DESIGN_THRESHOLDS: dict[str, Any] = {
    "wrapper_base_max_bytes": 192,
    "backend_widget_max_bytes": 512,
    "pool_bookkeeping_max_bytes_32bit": 1024,
    "image_source_max_bytes_32bit": 80,
    "font_max_bytes_32bit": 16,
    "wrapper_delta_max_percent": 5.0,
    "wrapper_delta_max_bytes": 8,
    "binary": {
        "__text": {"max_increase_percent": 5.0, "max_increase_bytes": 8192},
        "__data": {"max_increase_percent": 25.0, "max_increase_bytes": 256},
        "__bss": {"max_increase_percent": 10.0, "max_increase_bytes": 8192},
        "total": {"max_increase_percent": 5.0, "max_increase_bytes": 12288},
    },
    "allocation": {
        "process_implicit_heap_bytes": 0,
        "layout_implicit_heap_bytes": 0,
        "event_dispatch_implicit_heap_bytes": 0,
        "theme_apply_implicit_heap_bytes": 0,
        "ordinary_getter_implicit_heap_bytes": 0,
    },
    "time": {
        "warmup_runs": 5,
        "measured_runs": 30,
        "screen_object_create_ms_p95_max": 5.0,
        "capture_ready_ms_p95_max": 40.0,
    },
}

FORMAL_REQUIRED_CTEST_NAMES = (
    "check_tinyui_public_headers_c",
    "check_tinyui_public_headers_cpp",
    "check_tinyui_public_symbols_link",
    "check_ldgui_public_api_inventory",
    "check_tinyui_demo_boundary",
    "check_tinyui_docs_examples",
    "check_tinyui_install_consumer",
    "check_tinyui_runtime",
    "check_tinyui_backend_mapping",
    "check_tinyui_visible_ui",
    "check_tinyui_event_evidence",
    "test_tinyui_wrapper_struct_overhead",
    "test_tinyui_pool_abi",
    "test_tinyui_descriptor_abi",
    "test_tinyui_steady_state_allocation",
    "check_tinyui_binary_size",
    "check_tinyui_perf",
    "check_tinyui_minimal_profile",
    "check_tinyui_release_capability_matrix",
    "check_tinyui_public_api",
    "check_tinyui_v23_release_gates",
)

REFERENCE_EXISTENCE_OPTIONAL = {
    # Generated only at Task8 freeze; missing is an explicit closeout gap, not a
    # repo-tree hard requirement during Task1 scaffolding.
    "abi_manifest",
}


def error(code: str, **details: Any) -> dict[str, Any]:
    return {"code": code, **details}


def format_error(item: dict[str, Any]) -> str:
    code = item.get("code", "error")
    parts = [str(code)]
    for key, value in item.items():
        if key == "code":
            continue
        parts.append(f"{key}={value}")
    return " | ".join(parts)


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def is_disabled_property(properties: list[dict[str, Any]] | None) -> bool:
    if not properties:
        return False
    for prop in properties:
        name = str(prop.get("name", "")).upper()
        value = prop.get("value")
        if name == "DISABLED":
            if value is True:
                return True
            if isinstance(value, str) and value.strip().lower() in {"1", "true", "on", "yes"}:
                return True
            if value in (1, "1"):
                return True
    return False


def load_ctest_inventory(build_dir: Path) -> tuple[dict[str, dict[str, Any]], list[dict[str, Any]]]:
    errors: list[dict[str, Any]] = []
    if not build_dir.is_dir():
        errors.append(error("missing_build_dir", path=str(build_dir)))
        return {}, errors

    cmd = ["ctest", "--test-dir", str(build_dir), "--show-only=json-v1"]
    try:
        completed = subprocess.run(
            cmd,
            check=False,
            capture_output=True,
            text=True,
        )
    except OSError as exc:
        errors.append(error("ctest_invoke_failed", detail=str(exc), command=" ".join(cmd)))
        return {}, errors

    if completed.returncode != 0:
        errors.append(
            error(
                "ctest_show_only_failed",
                returncode=completed.returncode,
                stderr=(completed.stderr or "").strip()[:500],
            )
        )
        return {}, errors

    raw = (completed.stdout or "").strip()
    if not raw:
        errors.append(error("ctest_show_only_empty", build_dir=str(build_dir)))
        return {}, errors

    try:
        payload = json.loads(raw)
    except json.JSONDecodeError as exc:
        errors.append(error("ctest_show_only_invalid_json", detail=str(exc)))
        return {}, errors

    tests = payload.get("tests")
    if not isinstance(tests, list):
        errors.append(error("ctest_show_only_missing_tests_array"))
        return {}, errors

    inventory: dict[str, dict[str, Any]] = {}
    for entry in tests:
        if not isinstance(entry, dict):
            errors.append(error("ctest_entry_not_object", entry=repr(entry)[:120]))
            continue
        name = entry.get("name")
        if not isinstance(name, str) or not name:
            errors.append(error("ctest_entry_missing_name", entry=repr(entry)[:120]))
            continue
        inventory[name] = entry
    return inventory, errors


def _values_equal(expected: Any, actual: Any) -> bool:
    if isinstance(expected, float) or isinstance(actual, float):
        try:
            return abs(float(expected) - float(actual)) < 1e-9
        except (TypeError, ValueError):
            return False
    if isinstance(expected, dict) and isinstance(actual, dict):
        if set(expected) != set(actual):
            return False
        return all(_values_equal(expected[key], actual[key]) for key in expected)
    return expected == actual


def validate_thresholds(thresholds: Any) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    if not isinstance(thresholds, dict):
        return [error("thresholds_not_object")]

    unknown = sorted(set(thresholds) - set(DESIGN_THRESHOLDS))
    for key in unknown:
        errors.append(error("unknown_threshold_field", field=key))

    missing = sorted(set(DESIGN_THRESHOLDS) - set(thresholds))
    for key in missing:
        errors.append(error("missing_threshold_field", field=key))

    for key, expected in DESIGN_THRESHOLDS.items():
        if key not in thresholds:
            continue
        actual = thresholds[key]
        if not _values_equal(expected, actual):
            errors.append(
                error(
                    "threshold_mismatch",
                    field=key,
                    expected=json.dumps(expected, sort_keys=True),
                    actual=json.dumps(actual, sort_keys=True),
                )
            )
    return errors


def validate_manifest_shape(manifest: Any) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    if not isinstance(manifest, dict):
        return [error("manifest_not_object")]

    unknown = sorted(set(manifest) - MANIFEST_TOP_LEVEL_KEYS)
    for key in unknown:
        errors.append(error("unknown_manifest_field", field=key))

    for key in (
        "schema_version",
        "version",
        "port_completed",
        "abi_frozen",
        "manual_reviewed_passed",
        "required_ctests",
        "artifacts",
        "thresholds",
        "references",
    ):
        if key not in manifest:
            errors.append(error("missing_manifest_field", field=key))

    if manifest.get("schema_version") != 1:
        errors.append(
            error(
                "unsupported_schema_version",
                expected=1,
                actual=manifest.get("schema_version"),
            )
        )

    if manifest.get("version") != "2.3":
        errors.append(error("unexpected_version", expected="2.3", actual=manifest.get("version")))

    if manifest.get("port_completed") is not False:
        errors.append(
            error(
                "port_completed_must_be_false",
                actual=manifest.get("port_completed"),
            )
        )

    if not isinstance(manifest.get("abi_frozen"), bool):
        errors.append(error("abi_frozen_not_bool", actual=manifest.get("abi_frozen")))

    if not isinstance(manifest.get("manual_reviewed_passed"), bool):
        errors.append(
            error(
                "manual_reviewed_passed_not_bool",
                actual=manifest.get("manual_reviewed_passed"),
            )
        )

    ctests = manifest.get("required_ctests")
    if not isinstance(ctests, list) or not ctests:
        errors.append(error("required_ctests_missing_or_empty"))
        return errors + validate_thresholds(manifest.get("thresholds"))

    seen: set[str] = set()
    for index, entry in enumerate(ctests):
        if not isinstance(entry, dict):
            errors.append(error("required_ctest_not_object", index=index))
            continue
        unknown_entry = sorted(set(entry) - REQUIRED_CTEST_KEYS)
        for key in unknown_entry:
            errors.append(error("unknown_required_ctest_field", index=index, field=key))
        for key in REQUIRED_CTEST_KEYS:
            if key not in entry:
                errors.append(error("missing_required_ctest_field", index=index, field=key))
        name = entry.get("name")
        if not isinstance(name, str) or not name:
            errors.append(error("required_ctest_name_invalid", index=index, name=name))
            continue
        if name in seen:
            errors.append(error("duplicate_required_ctest", name=name))
        seen.add(name)
        if entry.get("required") is not True:
            # Aggregator is fail-closed: optional entries are not allowed in this
            # release manifest.
            errors.append(error("required_ctest_must_be_required", name=name))

    for formal in FORMAL_REQUIRED_CTEST_NAMES:
        if formal not in seen:
            errors.append(error("missing_formal_ctest_in_manifest", name=formal))

    extra = sorted(seen - set(FORMAL_REQUIRED_CTEST_NAMES))
    for name in extra:
        errors.append(error("unexpected_ctest_in_manifest", name=name))

    artifacts = manifest.get("artifacts")
    if not isinstance(artifacts, dict) or not artifacts:
        errors.append(error("artifacts_missing_or_empty"))
    else:
        for art_name, art in artifacts.items():
            if not isinstance(art, dict):
                errors.append(error("artifact_not_object", artifact=art_name))
                continue
            unknown_art = sorted(set(art) - ARTIFACT_KEYS)
            for key in unknown_art:
                errors.append(
                    error("unknown_artifact_field", artifact=art_name, field=key)
                )
            for key in ARTIFACT_KEYS:
                if key not in art:
                    errors.append(
                        error("missing_artifact_field", artifact=art_name, field=key)
                    )
            path = art.get("path")
            if not isinstance(path, str) or not path:
                errors.append(error("artifact_path_invalid", artifact=art_name, path=path))
            elif Path(path).is_absolute() or path.startswith("~"):
                errors.append(
                    error(
                        "artifact_path_must_be_relative",
                        artifact=art_name,
                        path=path,
                    )
                )
            if art.get("required_for_closeout") is not True:
                errors.append(
                    error(
                        "artifact_must_be_required_for_closeout",
                        artifact=art_name,
                    )
                )

    references = manifest.get("references")
    if not isinstance(references, dict) or not references:
        errors.append(error("references_missing_or_empty"))
    else:
        for ref_name, ref_path in references.items():
            if not isinstance(ref_path, str) or not ref_path:
                errors.append(error("reference_path_invalid", reference=ref_name))
                continue
            if Path(ref_path).is_absolute() or ref_path.startswith("~"):
                errors.append(
                    error(
                        "reference_path_must_be_relative",
                        reference=ref_name,
                        path=ref_path,
                    )
                )

    errors.extend(validate_thresholds(manifest.get("thresholds")))
    return errors


def validate_references(manifest: dict[str, Any], repo_root: Path) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    references = manifest.get("references")
    if not isinstance(references, dict):
        return errors

    for ref_name, ref_path in references.items():
        if not isinstance(ref_path, str):
            continue
        full = repo_root / ref_path
        if ref_name in REFERENCE_EXISTENCE_OPTIONAL:
            # Still record as closeout gap when absent so Task8 cannot forget it.
            if not full.is_file():
                errors.append(
                    error(
                        "optional_reference_missing",
                        reference=ref_name,
                        path=ref_path,
                        note="required before final abi freeze/closeout",
                    )
                )
            continue
        if not full.is_file():
            errors.append(error("reference_missing", reference=ref_name, path=ref_path))
    return errors


def _is_empty_evidence(value: Any) -> bool:
    if value is None:
        return True
    if isinstance(value, str) and not value.strip():
        return True
    if isinstance(value, (list, dict)) and len(value) == 0:
        return True
    return False


def validate_artifact_payload(
    artifact_name: str,
    payload: Any,
    path: Path,
    required_for_closeout: bool,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    if not isinstance(payload, dict):
        return [error("artifact_payload_not_object", artifact=artifact_name, path=str(path))]

    unknown = sorted(set(payload) - ARTIFACT_PAYLOAD_ALLOWED_EXTRA)
    for key in unknown:
        errors.append(
            error(
                "unknown_artifact_payload_field",
                artifact=artifact_name,
                field=key,
                path=str(path),
            )
        )

    if "schema_version" not in payload:
        errors.append(
            error("artifact_missing_schema_version", artifact=artifact_name, path=str(path))
        )
    if "version" not in payload and "profile" not in payload:
        # Accept either a release version stamp or an explicit profile label.
        errors.append(
            error(
                "artifact_missing_version_or_profile",
                artifact=artifact_name,
                path=str(path),
            )
        )

    if payload.get("fallback") is True:
        errors.append(error("artifact_fallback_true", artifact=artifact_name, path=str(path)))

    status = payload.get("status")
    if isinstance(status, str) and status.strip().lower() == "pending":
        errors.append(error("artifact_status_pending", artifact=artifact_name, path=str(path)))
    if required_for_closeout and (
        not isinstance(status, str) or status.strip().lower() != "pass"
    ):
        errors.append(
            error(
                "artifact_required_status_not_pass",
                artifact=artifact_name,
                status=status,
                path=str(path),
            )
        )

    if "evidence" in payload and _is_empty_evidence(payload.get("evidence")):
        errors.append(error("artifact_empty_evidence", artifact=artifact_name, path=str(path)))

    if "results" in payload and _is_empty_evidence(payload.get("results")):
        errors.append(error("artifact_empty_results", artifact=artifact_name, path=str(path)))

    if "gates" in payload and _is_empty_evidence(payload.get("gates")):
        errors.append(error("artifact_empty_gates", artifact=artifact_name, path=str(path)))

    if payload.get("status") == "pass" and payload.get("fallback") is True:
        errors.append(
            error(
                "artifact_pass_with_fallback",
                artifact=artifact_name,
                path=str(path),
            )
        )

    return errors


def validate_artifacts(
    manifest: dict[str, Any],
    artifact_root: Path,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    artifacts = manifest.get("artifacts")
    if not isinstance(artifacts, dict):
        return errors

    if not artifact_root.is_dir():
        errors.append(error("missing_artifact_root", path=str(artifact_root)))
        # Still enumerate each required artifact as missing for a clear checklist.
        for art_name, art in artifacts.items():
            if isinstance(art, dict) and art.get("required_for_closeout") is True:
                errors.append(
                    error(
                        "missing_artifact",
                        artifact=art_name,
                        path=str(artifact_root / str(art.get("path", ""))),
                    )
                )
        return errors

    for art_name, art in artifacts.items():
        if not isinstance(art, dict):
            continue
        rel = art.get("path")
        if not isinstance(rel, str):
            continue
        full = artifact_root / rel
        if not full.is_file():
            if art.get("required_for_closeout") is True:
                errors.append(
                    error(
                        "missing_artifact",
                        artifact=art_name,
                        path=str(full),
                    )
                )
            continue
        try:
            payload = load_json(full)
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(
                error(
                    "artifact_unreadable",
                    artifact=art_name,
                    path=str(full),
                    detail=str(exc),
                )
            )
            continue
        errors.extend(
            validate_artifact_payload(
                art_name,
                payload,
                full,
                art.get("required_for_closeout") is True,
            )
        )
    return errors


def validate_ctests(
    manifest: dict[str, Any],
    inventory: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    ctests = manifest.get("required_ctests")
    if not isinstance(ctests, list):
        return errors

    for entry in ctests:
        if not isinstance(entry, dict):
            continue
        name = entry.get("name")
        if not isinstance(name, str):
            continue
        if entry.get("required") is not True:
            continue
        found = inventory.get(name)
        if found is None:
            errors.append(error("missing_required_ctest", name=name))
            continue
        if is_disabled_property(found.get("properties")):
            errors.append(error("required_ctest_disabled", name=name))
    return errors


def validate_closeout_flags(
    manifest: dict[str, Any],
    artifact_errors: list[dict[str, Any]],
    repo_root: Path,
) -> list[dict[str, Any]]:
    """Flags may stay false in Task1; true only when matching evidence exists.

    ``port_completed`` must remain false for v2.3. ``abi_frozen`` /
    ``manual_reviewed_passed`` may flip true at Task8, but only if the matching
    closeout artifacts (and ABI manifest file) are present — the aggregator must
    never accept a bare claim.
    """
    errors: list[dict[str, Any]] = []
    missing_artifacts = {
        item.get("artifact")
        for item in artifact_errors
        if item.get("code") in {"missing_artifact", "missing_artifact_root"}
    }

    if manifest.get("manual_reviewed_passed") is True:
        if "manual_review" in missing_artifacts:
            errors.append(
                error(
                    "manual_reviewed_passed_without_artifact",
                    artifact="manual_review",
                )
            )

    if manifest.get("abi_frozen") is True:
        if "abi" in missing_artifacts:
            errors.append(error("abi_frozen_without_artifact", artifact="abi"))
        abi_manifest = manifest.get("references", {}).get("abi_manifest")
        if isinstance(abi_manifest, str):
            if not (repo_root / abi_manifest).is_file():
                errors.append(
                    error(
                        "abi_frozen_without_abi_manifest",
                        path=abi_manifest,
                    )
                )
    return errors


def check_release_gates(
    *,
    build_dir: Path,
    manifest_path: Path,
    artifact_root: Path,
    repo_root: Path = ROOT,
    inventory: dict[str, dict[str, Any]] | None = None,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []

    if not manifest_path.is_file():
        return [error("missing_manifest", path=str(manifest_path))]

    try:
        manifest = load_json(manifest_path)
    except (OSError, json.JSONDecodeError) as exc:
        return [error("manifest_unreadable", path=str(manifest_path), detail=str(exc))]

    errors.extend(validate_manifest_shape(manifest))
    # Continue collecting inventory/artifact issues even if shape has problems,
    # so operators get a full checklist rather than a single traceback-like stop.
    if not isinstance(manifest, dict):
        return errors

    errors.extend(validate_references(manifest, repo_root))

    if inventory is None:
        inventory, inv_errors = load_ctest_inventory(build_dir)
        errors.extend(inv_errors)
    errors.extend(validate_ctests(manifest, inventory))

    artifact_errors = validate_artifacts(manifest, artifact_root)
    errors.extend(artifact_errors)
    errors.extend(validate_closeout_flags(manifest, artifact_errors, repo_root))
    return errors


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="TinyUI v2.3 fail-closed release gate aggregator")
    parser.add_argument(
        "--build-dir",
        required=True,
        type=Path,
        help="CMake build directory used for ctest --show-only=json-v1",
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        default=DEFAULT_MANIFEST,
        help="Path to tinyui_v23_release_gate_manifest.json",
    )
    parser.add_argument(
        "--artifact-root",
        required=True,
        type=Path,
        help="Directory containing release-evidence JSON artifacts (relative paths in manifest)",
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=ROOT,
        help=argparse.SUPPRESS,
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    errors = check_release_gates(
        build_dir=args.build_dir.resolve(),
        manifest_path=args.manifest.resolve(),
        artifact_root=args.artifact_root.resolve(),
        repo_root=args.repo_root.resolve(),
    )
    if errors:
        print("FAIL: tinyui v2.3 release gates", file=sys.stderr)
        for item in errors:
            print(f"  - {format_error(item)}", file=sys.stderr)
        print(f"total_errors={len(errors)}", file=sys.stderr)
        return 1

    print("PASS: tinyui v2.3 release gates")
    return 0


if __name__ == "__main__":
    sys.exit(main())
