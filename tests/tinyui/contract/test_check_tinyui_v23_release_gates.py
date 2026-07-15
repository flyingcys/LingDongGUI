#!/usr/bin/env python3
"""Unit locks for the fail-closed v2.3 release gate aggregator."""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_v23_release_gates import (  # noqa: E402
    DESIGN_THRESHOLDS,
    FORMAL_REQUIRED_CTEST_NAMES,
    check_release_gates,
    validate_manifest_shape,
    validate_thresholds,
)


def _formal_ctests() -> list[dict]:
    return [
        {"name": name, "category": "test", "required": True}
        for name in FORMAL_REQUIRED_CTEST_NAMES
    ]


def _base_manifest(**overrides) -> dict:
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "port_completed": False,
        "abi_frozen": False,
        "manual_reviewed_passed": False,
        "required_ctests": _formal_ctests(),
        "artifacts": {
            "standard": {
                "path": "standard.json",
                "required_for_closeout": True,
                "description": "standard",
            },
            "asan": {
                "path": "asan.json",
                "required_for_closeout": True,
                "description": "asan",
            },
            "manual_review": {
                "path": "manual_review.json",
                "required_for_closeout": True,
                "description": "review",
            },
            "abi": {
                "path": "abi.json",
                "required_for_closeout": True,
                "description": "abi",
            },
        },
        "thresholds": json.loads(json.dumps(DESIGN_THRESHOLDS)),
        "references": {
            "performance_baseline": "perf_baseline.json",
            "capability_matrix": "capability.json",
            "public_api": "public_api.json",
            "abi_manifest": "abi_manifest.json",
        },
    }
    payload.update(overrides)
    return payload


def _write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _codes(errors: list[dict]) -> set[str]:
    return {item["code"] for item in errors}


class ThresholdTests(unittest.TestCase):
    def test_design_thresholds_match(self):
        self.assertEqual(validate_thresholds(DESIGN_THRESHOLDS), [])

    def test_threshold_mismatch_is_reported(self):
        bad = json.loads(json.dumps(DESIGN_THRESHOLDS))
        bad["wrapper_base_max_bytes"] = 200
        errors = validate_thresholds(bad)
        self.assertIn("threshold_mismatch", _codes(errors))


class ManifestShapeTests(unittest.TestCase):
    def test_unknown_field_fails(self):
        manifest = _base_manifest()
        manifest["unexpected"] = True
        errors = validate_manifest_shape(manifest)
        self.assertIn("unknown_manifest_field", _codes(errors))

    def test_port_completed_true_fails(self):
        errors = validate_manifest_shape(_base_manifest(port_completed=True))
        self.assertIn("port_completed_must_be_false", _codes(errors))

    def test_absolute_artifact_path_fails(self):
        manifest = _base_manifest()
        manifest["artifacts"]["standard"]["path"] = "/tmp/standard.json"
        errors = validate_manifest_shape(manifest)
        self.assertIn("artifact_path_must_be_relative", _codes(errors))

    def test_missing_formal_ctest_fails(self):
        manifest = _base_manifest()
        manifest["required_ctests"] = manifest["required_ctests"][:-1]
        errors = validate_manifest_shape(manifest)
        self.assertIn("missing_formal_ctest_in_manifest", _codes(errors))


class AggregatorBehaviorTests(unittest.TestCase):
    def test_missing_ctest_and_artifacts_fail_without_traceback(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "manifest.json"
            _write_json(manifest_path, _base_manifest())
            # references present so they do not dominate the assertion
            for rel in (
                "perf_baseline.json",
                "capability.json",
                "public_api.json",
            ):
                _write_json(root / rel, {"schema_version": 1})

            errors = check_release_gates(
                build_dir=root / "build",
                manifest_path=manifest_path,
                artifact_root=root / "release-evidence",
                repo_root=root,
                inventory={},  # empty inventory => all required ctests missing
            )

            codes = _codes(errors)
            self.assertIn("missing_required_ctest", codes)
            self.assertIn("missing_artifact_root", codes)
            self.assertIn("missing_artifact", codes)
            self.assertTrue(any(e.get("name") == "check_tinyui_event_evidence" for e in errors))

    def test_disabled_ctest_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "manifest.json"
            _write_json(manifest_path, _base_manifest())
            for rel in (
                "perf_baseline.json",
                "capability.json",
                "public_api.json",
                "abi_manifest.json",
            ):
                _write_json(root / rel, {"schema_version": 1})

            inventory = {
                name: {
                    "name": name,
                    "properties": [{"name": "DISABLED", "value": True}]
                    if name == "check_tinyui_demo_boundary"
                    else [],
                }
                for name in FORMAL_REQUIRED_CTEST_NAMES
            }
            artifact_root = root / "release-evidence"
            for name in ("standard", "asan", "manual_review", "abi"):
                _write_json(
                    artifact_root / f"{name}.json",
                    {
                        "schema_version": 1,
                        "version": "2.3",
                        "status": "pass",
                        "evidence": {"ok": True},
                    },
                )

            errors = check_release_gates(
                build_dir=root / "build",
                manifest_path=manifest_path,
                artifact_root=artifact_root,
                repo_root=root,
                inventory=inventory,
            )
            self.assertIn("required_ctest_disabled", _codes(errors))
            self.assertTrue(
                any(
                    e.get("code") == "required_ctest_disabled"
                    and e.get("name") == "check_tinyui_demo_boundary"
                    for e in errors
                )
            )

    def test_fallback_and_pending_artifact_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "manifest.json"
            _write_json(manifest_path, _base_manifest())
            for rel in (
                "perf_baseline.json",
                "capability.json",
                "public_api.json",
                "abi_manifest.json",
            ):
                _write_json(root / rel, {"schema_version": 1})

            inventory = {
                name: {"name": name, "properties": []}
                for name in FORMAL_REQUIRED_CTEST_NAMES
            }
            artifact_root = root / "release-evidence"
            _write_json(
                artifact_root / "standard.json",
                {
                    "schema_version": 1,
                    "version": "2.3",
                    "status": "pass",
                    "fallback": True,
                    "evidence": {"ok": True},
                },
            )
            _write_json(
                artifact_root / "asan.json",
                {
                    "schema_version": 1,
                    "version": "2.3",
                    "status": "pending",
                    "evidence": {"ok": True},
                },
            )
            _write_json(
                artifact_root / "manual_review.json",
                {
                    "schema_version": 1,
                    "version": "2.3",
                    "status": "pass",
                    "evidence": {},
                },
            )
            _write_json(
                artifact_root / "abi.json",
                {
                    "schema_version": 1,
                    "version": "2.3",
                    "status": "pass",
                    "evidence": {"ok": True},
                    "unexpected_field": 1,
                },
            )

            errors = check_release_gates(
                build_dir=root / "build",
                manifest_path=manifest_path,
                artifact_root=artifact_root,
                repo_root=root,
                inventory=inventory,
            )
            codes = _codes(errors)
            self.assertIn("artifact_fallback_true", codes)
            self.assertIn("artifact_status_pending", codes)
            self.assertIn("artifact_empty_evidence", codes)
            self.assertIn("unknown_artifact_payload_field", codes)

    def test_required_artifact_status_must_pass(self):
        for artifact_name, status in (
            ("standard", "fail"),
            ("asan", "fail"),
            ("manual_review", "fail"),
            ("abi", "inconclusive"),
        ):
            with self.subTest(artifact=artifact_name, status=status):
                with tempfile.TemporaryDirectory() as directory:
                    root = Path(directory)
                    manifest_path = root / "manifest.json"
                    _write_json(
                        manifest_path,
                        _base_manifest(abi_frozen=True, manual_reviewed_passed=True),
                    )
                    for rel in (
                        "perf_baseline.json",
                        "capability.json",
                        "public_api.json",
                        "abi_manifest.json",
                    ):
                        _write_json(root / rel, {"schema_version": 1})

                    inventory = {
                        name: {"name": name, "properties": []}
                        for name in FORMAL_REQUIRED_CTEST_NAMES
                    }
                    artifact_root = root / "release-evidence"
                    for name in ("standard", "asan", "manual_review", "abi"):
                        _write_json(
                            artifact_root / f"{name}.json",
                            {
                                "schema_version": 1,
                                "version": "2.3",
                                "status": status if name == artifact_name else "pass",
                                "evidence": {"ok": True, "profile": name},
                            },
                        )

                    errors = check_release_gates(
                        build_dir=root / "build",
                        manifest_path=manifest_path,
                        artifact_root=artifact_root,
                        repo_root=root,
                        inventory=inventory,
                    )
                    self.assertIn("artifact_required_status_not_pass", _codes(errors))
                    self.assertTrue(
                        any(
                            error.get("code") == "artifact_required_status_not_pass"
                            and error.get("artifact") == artifact_name
                            and error.get("status") == status
                            for error in errors
                        )
                    )

    def test_happy_path_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "manifest.json"
            _write_json(
                manifest_path,
                _base_manifest(abi_frozen=True, manual_reviewed_passed=True),
            )
            for rel in (
                "perf_baseline.json",
                "capability.json",
                "public_api.json",
                "abi_manifest.json",
            ):
                _write_json(root / rel, {"schema_version": 1})

            inventory = {
                name: {"name": name, "properties": []}
                for name in FORMAL_REQUIRED_CTEST_NAMES
            }
            artifact_root = root / "release-evidence"
            for name in ("standard", "asan", "manual_review", "abi"):
                _write_json(
                    artifact_root / f"{name}.json",
                    {
                        "schema_version": 1,
                        "version": "2.3",
                        "status": "pass",
                        "evidence": {"ok": True, "profile": name},
                    },
                )

            errors = check_release_gates(
                build_dir=root / "build",
                manifest_path=manifest_path,
                artifact_root=artifact_root,
                repo_root=root,
                inventory=inventory,
            )
            self.assertEqual(errors, [])


if __name__ == "__main__":
    unittest.main()
