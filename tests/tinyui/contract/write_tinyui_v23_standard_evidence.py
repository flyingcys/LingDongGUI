#!/usr/bin/env python3
"""Write M5 Task 2 standard.json release evidence from a completed CTest run.

Required non-null fields (plan Task 2 step 5):
  - git commit
  - TinyUI-related CMake cache options
  - compiler ID / version / target
  - test totals / pass count / failures
  - start / end ISO timestamps
  - ctest log SHA-256
Optional:
  - exclusions (must include reasons when present)
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[3]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write TinyUI v2.3 standard.json evidence.")
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument(
        "--output",
        type=Path,
        help="Default: <build-dir>/release-evidence/standard.json",
    )
    parser.add_argument(
        "--ctest-log",
        type=Path,
        help="Full ctest console log used for SHA-256 (required for non-null hash).",
    )
    parser.add_argument("--start-iso", required=True, help="ISO-8601 start time")
    parser.add_argument("--end-iso", required=True, help="ISO-8601 end time")
    parser.add_argument(
        "--exclusions-json",
        type=Path,
        help="Optional JSON list of {name, reason} exclusions.",
    )
    parser.add_argument(
        "--status",
        choices=("pass", "fail"),
        help="Override status; default derived from ctest totals.",
    )
    return parser.parse_args()


def _run(cmd: list[str], cwd: Path | None = None) -> str:
    completed = subprocess.run(
        cmd,
        check=False,
        capture_output=True,
        text=True,
        cwd=str(cwd) if cwd else None,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"command failed ({completed.returncode}): {' '.join(cmd)}\n"
            f"stderr: {completed.stderr.strip()}"
        )
    return completed.stdout


def _git_commit() -> str:
    return _run(["git", "rev-parse", "HEAD"], cwd=ROOT).strip()


def _read_cmake_cache(build_dir: Path) -> dict[str, str]:
    cache_path = build_dir / "CMakeCache.txt"
    if not cache_path.is_file():
        raise FileNotFoundError(f"missing CMakeCache.txt: {cache_path}")
    values: dict[str, str] = {}
    for line in cache_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line or line.startswith("//") or line.startswith("#") or "=" not in line:
            continue
        key_type, value = line.split("=", 1)
        key = key_type.split(":", 1)[0]
        values[key] = value
    return values


def _tinyui_related_options(cache: dict[str, str]) -> dict[str, str]:
    keys = sorted(
        key
        for key in cache
        if key.startswith("TINYUI_")
        or key.startswith("LD_")
        or key in {
            "ENABLE_TEST",
            "USE_DEMO",
            "CMAKE_BUILD_TYPE",
            "CMAKE_C_COMPILER",
            "CMAKE_C_COMPILER_ID",
            "CMAKE_C_COMPILER_VERSION",
            "CMAKE_SYSTEM_NAME",
            "CMAKE_SYSTEM_PROCESSOR",
            "CMAKE_C_COMPILER_TARGET",
        }
    )
    return {key: cache[key] for key in keys}


def _compiler_info(cache: dict[str, str]) -> dict[str, str | None]:
    compiler_path = cache.get("CMAKE_C_COMPILER") or "cc"
    compiler_id = cache.get("CMAKE_C_COMPILER_ID")
    compiler_version = cache.get("CMAKE_C_COMPILER_VERSION")
    target = (
        cache.get("CMAKE_C_COMPILER_TARGET")
        or cache.get("CMAKE_SYSTEM_PROCESSOR")
        or cache.get("CMAKE_HOST_SYSTEM_PROCESSOR")
    )
    system = cache.get("CMAKE_SYSTEM_NAME") or cache.get("CMAKE_HOST_SYSTEM_NAME")

    # CMakeCache does not always persist compiler ID/version as ordinary entries.
    # Probe the compiler when those fields are absent so standard.json stays non-null.
    if not compiler_id or not compiler_version:
        try:
            version_line = _run([compiler_path, "--version"]).splitlines()[0]
        except RuntimeError:
            version_line = ""
        lower = version_line.lower()
        if "clang" in lower:
            compiler_id = compiler_id or "Clang"
        elif "gcc" in lower or "free software foundation" in lower:
            compiler_id = compiler_id or "GNU"
        else:
            # Ubuntu often reports "cc (Ubuntu 11.4.0-...) 11.4.0" for gcc via /usr/bin/cc.
            try:
                dump = _run([compiler_path, "-dumpversion"]).strip()
            except RuntimeError:
                dump = ""
            if dump:
                compiler_id = compiler_id or "GNU"
                compiler_version = compiler_version or dump
            else:
                compiler_id = compiler_id or Path(compiler_path).name
        compiler_version = compiler_version or (version_line or "unknown")

    if not target:
        try:
            target = _run([compiler_path, "-dumpmachine"]).strip() or "unknown"
        except RuntimeError:
            target = "unknown"
    if not system:
        try:
            import platform

            system = platform.system()
        except Exception:  # noqa: BLE001
            system = "unknown"

    return {
        "id": compiler_id,
        "version": compiler_version,
        "path": compiler_path,
        "target": target,
        "system": system,
    }


def _parse_ctest_summary(log_text: str) -> dict[str, Any]:
    # Examples:
    #   100% tests passed, 0 tests failed out of 123
    #   98% tests passed, 2 tests failed out of 123
    #   No tests were found!!!
    total = None
    failed = None
    passed = None
    m = re.search(
        r"(\d+)% tests passed,\s*(\d+) tests failed out of\s*(\d+)",
        log_text,
    )
    if m:
        failed = int(m.group(2))
        total = int(m.group(3))
        passed = total - failed
    else:
        m2 = re.search(r"(\d+)\s+tests passed.*?(\d+)\s+tests failed", log_text, re.I)
        if m2:
            passed = int(m2.group(1))
            failed = int(m2.group(2))
            total = passed + failed

    failures: list[str] = []
    # The following tests FAILED:
    # \t 12 - test_name (Failed)
    in_failed = False
    for line in log_text.splitlines():
        if "The following tests FAILED" in line:
            in_failed = True
            continue
        if in_failed:
            if not line.strip():
                if failures:
                    break
                continue
            m_fail = re.search(r"-\s+([^\s(]+)\s+\(", line)
            if m_fail:
                failures.append(m_fail.group(1))
            elif re.match(r"^\s*\d+%", line) or line.startswith("Errors while"):
                break

    if total is None:
        # Fallback: ctest -N inventory count not available; keep explicit null check later.
        raise ValueError("could not parse ctest totals from log")

    if failed is None:
        failed = len(failures)
    if passed is None:
        passed = total - failed

    return {
        "total": total,
        "passed": passed,
        "failed": failed,
        "failures": failures,
    }


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def _load_exclusions(path: Path | None) -> list[dict[str, str]]:
    if path is None:
        return []
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, list):
        raise ValueError("exclusions JSON must be a list")
    out: list[dict[str, str]] = []
    for item in data:
        if not isinstance(item, dict) or "name" not in item or "reason" not in item:
            raise ValueError("each exclusion needs name and reason")
        out.append({"name": str(item["name"]), "reason": str(item["reason"])})
    return out


def main() -> int:
    args = parse_args()
    build_dir = args.build_dir.resolve()
    output = (
        args.output.resolve()
        if args.output
        else build_dir / "release-evidence" / "standard.json"
    )
    ctest_log = args.ctest_log.resolve() if args.ctest_log else None
    if ctest_log is None:
        # Prefer LastTest.log when present.
        candidate = build_dir / "Testing" / "Temporary" / "LastTest.log"
        if candidate.is_file():
            ctest_log = candidate
        else:
            print("missing --ctest-log and no Testing/Temporary/LastTest.log", file=sys.stderr)
            return 1

    try:
        cache = _read_cmake_cache(build_dir)
        log_text = ctest_log.read_text(encoding="utf-8", errors="replace")
        summary = _parse_ctest_summary(log_text)
        exclusions = _load_exclusions(args.exclusions_json)
        commit = _git_commit()
        compiler = _compiler_info(cache)
        tinyui_options = _tinyui_related_options(cache)
        log_sha = _sha256_file(ctest_log)
    except (OSError, ValueError, RuntimeError, json.JSONDecodeError) as exc:
        print(f"write_tinyui_v23_standard_evidence FAIL: {exc}", file=sys.stderr)
        return 1

    # Validate non-null required fields.
    required_non_null = {
        "commit": commit,
        "tinyui_options": tinyui_options,
        "compiler.id": compiler.get("id"),
        "compiler.version": compiler.get("version"),
        "compiler.target": compiler.get("target"),
        "tests.total": summary.get("total"),
        "tests.passed": summary.get("passed"),
        "tests.failed": summary.get("failed"),
        "start_iso": args.start_iso,
        "end_iso": args.end_iso,
        "log_sha256": log_sha,
    }
    missing = [key for key, value in required_non_null.items() if value in (None, "", {})]
    if missing:
        print(f"required fields null/empty: {', '.join(missing)}", file=sys.stderr)
        return 1

    status = args.status
    if status is None:
        status = "pass" if summary["failed"] == 0 else "fail"

    payload: dict[str, Any] = {
        "schema_version": 1,
        "version": "2.3",
        "profile": "standard",
        "status": status,
        "fallback": False,
        "commit": commit,
        "build_dir": str(build_dir),
        "generated_by": "tests/tinyui/contract/write_tinyui_v23_standard_evidence.py",
        "evidence": {
            "start_iso": args.start_iso,
            "end_iso": args.end_iso,
            "git_commit": commit,
            "cmake_tinyui_options": tinyui_options,
            "compiler": compiler,
            "tests": {
                "total": summary["total"],
                "passed": summary["passed"],
                "failed": summary["failed"],
                "failures": summary["failures"],
            },
            "ctest_log": str(ctest_log),
            "ctest_log_sha256": log_sha,
            "exclusions": exclusions,
        },
        "results": {
            "total": summary["total"],
            "passed": summary["passed"],
            "failed": summary["failed"],
            "failures": summary["failures"],
        },
        "summary": {
            "profile": "standard",
            "status": status,
            "total": summary["total"],
            "passed": summary["passed"],
            "failed": summary["failed"],
        },
        "sha256": log_sha,
        "notes": (
            "M5 Task 2 clean standard CMake/CTest evidence. "
            "Does not claim port complete, ABI freeze, or release closeout."
        ),
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"wrote {output}")
    print(
        f"status={status} total={summary['total']} "
        f"passed={summary['passed']} failed={summary['failed']}"
    )
    return 0 if status == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
