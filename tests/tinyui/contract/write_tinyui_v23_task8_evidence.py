#!/usr/bin/env python3
"""Write M5 Task 8 release-evidence projections (honest status, no silent pass)."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
DEFAULT_ARTIFACT_ROOT = ROOT / "build" / "v2.3" / "release-evidence"
DEFAULT_BUILD_DIR = ROOT / "build" / "v2.3"
ABI_MANIFEST = CONTRACT_DIR / "tinyui_v23_abi_manifest.json"


def _run(cmd: list[str], cwd: Path | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        cmd,
        check=False,
        capture_output=True,
        text=True,
        cwd=str(cwd) if cwd else None,
    )


def _git_commit() -> str:
    completed = _run(["git", "rev-parse", "HEAD"], cwd=ROOT)
    if completed.returncode != 0:
        return "unknown"
    return completed.stdout.strip()


def _sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")


def _now_iso() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec="seconds")


def write_minimal(artifact_root: Path, commit: str) -> dict[str, Any]:
    lightweight = artifact_root / "lightweight-gates.json"
    task4_log = artifact_root / "task4-logs" / "ctest_minimal_tree.log"
    light: dict[str, Any] = {}
    if lightweight.is_file():
        light = json.loads(lightweight.read_text(encoding="utf-8"))
    gate = {}
    if isinstance(light.get("gates"), dict):
        gate = light["gates"].get("check_tinyui_minimal_profile") or {}
    status = "pass" if str(gate.get("status", "")).upper() == "PASS" else "fail"
    if not gate and task4_log.is_file() and "100% tests passed" in task4_log.read_text(
        encoding="utf-8", errors="replace"
    ):
        status = "pass"
    log_sha = _sha256_file(task4_log) if task4_log.is_file() else ""
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "minimal",
        "status": status,
        "fallback": False,
        "commit": commit,
        "build_dir": "build/v2.3-minimal",
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_task8_evidence.py",
        "evidence": {
            "source": "M5 Task4 lightweight-gates + task4-logs/ctest_minimal_tree.log",
            "lightweight_gates": "build/v2.3/release-evidence/lightweight-gates.json",
            "ctest_minimal_tree_log": str(task4_log.relative_to(ROOT)) if task4_log.is_file() else None,
            "ctest_minimal_tree_log_sha256": log_sha or None,
            "gate": gate,
            "recorded_at": _now_iso(),
        },
        "results": {
            "check_tinyui_minimal_profile": status,
            "required_present": gate.get("required_present"),
            "forbidden_public_impl_zero_hits": gate.get("forbidden_public_impl_zero_hits"),
        },
        "summary": {
            "profile": "minimal",
            "status": status,
            "note": "Projection of Task4 minimal profile evidence; not a re-run claim of full-tree make all.",
        },
        "sha256": log_sha or _sha256_text(json.dumps(gate, sort_keys=True)),
        "notes": (
            "M5 Task8 minimal.json projection from Task4. "
            "Does not claim port complete or release closeout."
        ),
    }
    _write_json(artifact_root / "minimal.json", payload)
    return payload


def write_abi(artifact_root: Path, commit: str) -> dict[str, Any]:
    if str(CONTRACT_DIR) not in sys.path:
        sys.path.insert(0, str(CONTRACT_DIR))
    from check_tinyui_public_api import (  # type: ignore
        abi_manifest_sha256,
        build_abi_manifest,
        check_abi_manifest,
    )

    check_errors = check_abi_manifest(ABI_MANIFEST)
    status = "pass" if not check_errors else "fail"
    actual = build_abi_manifest()
    manifest_sha = abi_manifest_sha256(actual) if ABI_MANIFEST.is_file() else ""
    if ABI_MANIFEST.is_file():
        manifest_sha = _sha256_file(ABI_MANIFEST)
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "abi",
        "status": status,
        "fallback": False,
        "commit": commit,
        "build_dir": "build/v2.3",
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_task8_evidence.py",
        "abi_frozen": status == "pass",
        "port_completed": False,
        "evidence": {
            "manifest": "tests/tinyui/contract/tinyui_v23_abi_manifest.json",
            "manifest_sha256": manifest_sha,
            "check_command": (
                "python3 tests/tinyui/contract/check_tinyui_public_api.py "
                "--check-abi-manifest tests/tinyui/contract/tinyui_v23_abi_manifest.json"
            ),
            "check_errors": check_errors,
            "counts": {
                "headers": len(actual.get("headers", [])),
                "functions": len(actual.get("functions", [])),
                "enums": len(actual.get("enums", [])),
                "typedefs": len(actual.get("typedefs", [])),
                "opaques": len(actual.get("opaques", [])),
                "macros": len(actual.get("macros", [])),
                "export_symbols": len((actual.get("exports") or {}).get("symbols", [])),
            },
            "recorded_at": _now_iso(),
        },
        "results": {
            "abi_manifest_check": status,
            "emit_once": True,
            "auto_rewrite": False,
        },
        "summary": {
            "status": status,
            "manifest_sha256": manifest_sha,
            "functions": len(actual.get("functions", [])),
            "headers": len(actual.get("headers", [])),
        },
        "sha256": manifest_sha,
        "references": {
            "abi_manifest": "tests/tinyui/contract/tinyui_v23_abi_manifest.json",
            "public_api": "tests/tinyui/contract/tinyui_v23_public_api.json",
        },
        "notes": (
            "M5 Task8 ABI freeze evidence. Check mode never rewrites the frozen manifest. "
            "port_completed remains false."
        ),
    }
    _write_json(artifact_root / "abi.json", payload)
    return payload


def _run_ctest(build_dir: Path, regex: str) -> dict[str, Any]:
    log_path = build_dir / "release-evidence" / "task8-logs" / f"{regex.strip('^$')}.log"
    log_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        "ctest",
        "--test-dir",
        str(build_dir),
        "-R",
        regex,
        "--output-on-failure",
    ]
    completed = _run(cmd)
    log_text = (completed.stdout or "") + (completed.stderr or "")
    log_path.write_text(log_text, encoding="utf-8")
    passed = completed.returncode == 0
    return {
        "command": " ".join(cmd),
        "returncode": completed.returncode,
        "status": "pass" if passed else "fail",
        "log": str(log_path.relative_to(ROOT)),
        "log_sha256": _sha256_text(log_text),
        "stdout_tail": (completed.stdout or "")[-2000:],
        "stderr_tail": (completed.stderr or "")[-1000:],
    }


def write_install_consumer(artifact_root: Path, build_dir: Path, commit: str) -> dict[str, Any]:
    result = _run_ctest(build_dir, "^check_tinyui_install_consumer$")
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "install_consumer",
        "status": result["status"],
        "fallback": False,
        "commit": commit,
        "build_dir": str(build_dir.relative_to(ROOT)) if build_dir.is_relative_to(ROOT) else str(build_dir),
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_task8_evidence.py",
        "evidence": {
            "ctest": result,
            "recorded_at": _now_iso(),
        },
        "results": {
            "check_tinyui_install_consumer": result["status"],
            "returncode": result["returncode"],
        },
        "summary": {
            "status": result["status"],
            "ctest": "check_tinyui_install_consumer",
        },
        "sha256": result["log_sha256"],
        "notes": "M5 Task8 install consumer re-run evidence. Does not claim port complete.",
    }
    _write_json(artifact_root / "install_consumer.json", payload)
    return payload


def write_docs_examples(artifact_root: Path, build_dir: Path, commit: str) -> dict[str, Any]:
    result = _run_ctest(build_dir, "^check_tinyui_docs_examples$")
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "docs_examples",
        "status": result["status"],
        "fallback": False,
        "commit": commit,
        "build_dir": str(build_dir.relative_to(ROOT)) if build_dir.is_relative_to(ROOT) else str(build_dir),
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_task8_evidence.py",
        "evidence": {
            "ctest": result,
            "recorded_at": _now_iso(),
        },
        "results": {
            "check_tinyui_docs_examples": result["status"],
            "returncode": result["returncode"],
        },
        "summary": {
            "status": result["status"],
            "ctest": "check_tinyui_docs_examples",
        },
        "sha256": result["log_sha256"],
        "notes": "M5 Task8 docs examples re-run evidence.",
    }
    _write_json(artifact_root / "docs_examples.json", payload)
    return payload


def write_capability_matrix(artifact_root: Path, build_dir: Path, commit: str) -> dict[str, Any]:
    result = _run_ctest(build_dir, "^check_tinyui_release_capability_matrix$")
    matrix_path = CONTRACT_DIR / "tinyui_release_capability_matrix.json"
    matrix_sha = _sha256_file(matrix_path) if matrix_path.is_file() else ""
    payload = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "capability_matrix",
        "status": result["status"],
        "fallback": False,
        "commit": commit,
        "build_dir": str(build_dir.relative_to(ROOT)) if build_dir.is_relative_to(ROOT) else str(build_dir),
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_task8_evidence.py",
        "evidence": {
            "ctest": result,
            "matrix": "tests/tinyui/contract/tinyui_release_capability_matrix.json",
            "matrix_sha256": matrix_sha,
            "recorded_at": _now_iso(),
        },
        "results": {
            "check_tinyui_release_capability_matrix": result["status"],
            "returncode": result["returncode"],
        },
        "summary": {
            "status": result["status"],
            "ctest": "check_tinyui_release_capability_matrix",
            "matrix_sha256": matrix_sha,
        },
        "sha256": result["log_sha256"],
        "references": {
            "matrix": "tests/tinyui/contract/tinyui_release_capability_matrix.json",
            "checker": "tests/tinyui/contract/check_tinyui_release_capability_matrix.py",
        },
        "notes": (
            "M5 Task8 capability matrix re-run. Machine checker status is independent of "
            "manual_reviewed_passed (partial L5-E remains a manual residual)."
        ),
    }
    _write_json(artifact_root / "capability_matrix.json", payload)
    return payload


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--artifact-root", type=Path, default=DEFAULT_ARTIFACT_ROOT)
    parser.add_argument("--build-dir", type=Path, default=DEFAULT_BUILD_DIR)
    parser.add_argument(
        "--skip-ctest",
        action="store_true",
        help="Only write minimal/abi projections; skip install/docs/capability ctest runs",
    )
    args = parser.parse_args(argv)
    artifact_root = args.artifact_root.resolve()
    build_dir = args.build_dir.resolve()
    commit = _git_commit()

    written: dict[str, Any] = {}
    written["minimal"] = write_minimal(artifact_root, commit)
    written["abi"] = write_abi(artifact_root, commit)
    if not args.skip_ctest:
        written["install_consumer"] = write_install_consumer(artifact_root, build_dir, commit)
        written["docs_examples"] = write_docs_examples(artifact_root, build_dir, commit)
        written["capability_matrix"] = write_capability_matrix(artifact_root, build_dir, commit)

    print(json.dumps({k: v.get("status") for k, v in written.items()}, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
