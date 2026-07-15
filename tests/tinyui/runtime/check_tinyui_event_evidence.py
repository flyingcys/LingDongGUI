#!/usr/bin/env python3
"""Aggregate L5-E event evidence under the formal CTest name.

Wraps the existing backend-mapping v23 scenario suite so the release
manifest can require an exact CTest name without deleting per-scenario
gates.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CHECKER = Path(__file__).with_name("check_tinyui_backend_mapping.py")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run TinyUI L5-E event evidence scenarios (formal gate)."
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        required=True,
        help="Existing CMake build directory containing v23 scenario runners.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    build_dir = args.build_dir.resolve()
    if not build_dir.is_dir():
        print(f"missing build dir: {build_dir}", file=sys.stderr)
        return 1
    if not CHECKER.is_file():
        print(f"missing checker: {CHECKER}", file=sys.stderr)
        return 1

    cmd = [
        sys.executable,
        str(CHECKER),
        "--all-v23",
        "--build-dir",
        str(build_dir),
    ]
    print("running event evidence via:", " ".join(cmd), flush=True)
    completed = subprocess.run(cmd, check=False, cwd=str(ROOT))
    if completed.returncode != 0:
        print(
            f"check_tinyui_event_evidence FAIL: backend mapping exit={completed.returncode}",
            file=sys.stderr,
        )
        return completed.returncode
    print("check_tinyui_event_evidence OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
