#!/usr/bin/env python3
"""Full-profile wrapper for formal CTest name check_tinyui_minimal_profile.

Symbol enforce is only meaningful on a non-LTO minimal profile artifact.
When the current tree is not TINYUI_PROFILE=minimal, this wrapper looks up a
sibling/prebuilt minimal build and runs check_tinyui_minimal_symbols.py against
that artifact (fail-closed if missing).
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CHECKER = Path(__file__).with_name("check_tinyui_minimal_symbols.py")
DEFAULT_BASELINE = Path(__file__).with_name("tinyui_perf_baseline.json")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Enforce minimal profile symbols via dedicated minimal artifact."
    )
    parser.add_argument(
        "--current-profile",
        default="full",
        help="Current TINYUI_PROFILE of the invoking CMake tree.",
    )
    parser.add_argument(
        "--binary",
        type=Path,
        help="Direct path to tinyui_minimal_consumer (minimal profile only).",
    )
    parser.add_argument(
        "--map",
        dest="map_path",
        type=Path,
        help="Direct path to the link map (minimal profile only).",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="Build dir owning the binary/map (optional for checker).",
    )
    parser.add_argument(
        "--minimal-build-dir",
        type=Path,
        help="Sibling/prebuilt minimal build directory for full-profile trees.",
    )
    parser.add_argument(
        "--baseline",
        type=Path,
        default=DEFAULT_BASELINE,
    )
    return parser.parse_args()


def _candidate_minimal_dirs(explicit: Path | None, current_build: Path | None) -> list[Path]:
    candidates: list[Path] = []
    env = os.environ.get("BUILD_V23_MINIMAL_DIR") or os.environ.get("TINYUI_MINIMAL_BUILD_DIR")
    if env:
        candidates.append(Path(env))
    if explicit is not None:
        candidates.append(explicit)
    if current_build is not None:
        candidates.append(current_build.parent / "v2.3-minimal")
        candidates.append(current_build / "v2.3-minimal")
    candidates.append(ROOT / "build" / "v2.3-minimal")
    # De-dupe while preserving order.
    seen: set[str] = set()
    ordered: list[Path] = []
    for path in candidates:
        key = str(path.resolve()) if path.exists() else str(path)
        if key in seen:
            continue
        seen.add(key)
        ordered.append(path)
    return ordered


def _find_minimal_artifacts(minimal_dirs: list[Path]) -> tuple[Path, Path, Path]:
    binary_names = (
        Path("tests/tinyui/tinyui_minimal_consumer"),
        Path("tinyui_minimal_consumer"),
    )
    for build_dir in minimal_dirs:
        for rel in binary_names:
            binary = build_dir / rel
            map_path = Path(str(binary) + ".map")
            if binary.is_file() and map_path.is_file():
                return binary, map_path, build_dir
    searched = ", ".join(str(path) for path in minimal_dirs)
    raise FileNotFoundError(
        "minimal profile artifact not found (need tinyui_minimal_consumer + .map). "
        f"Searched: {searched}. Build build/v2.3-minimal with "
        "-DTINYUI_PROFILE=minimal first."
    )


def main() -> int:
    args = parse_args()
    if not CHECKER.is_file():
        print(f"missing checker: {CHECKER}", file=sys.stderr)
        return 1

    profile = (args.current_profile or "").strip().lower()
    try:
        if profile == "minimal":
            if args.binary is None or args.map_path is None:
                raise ValueError(
                    "minimal profile requires --binary and --map for local artifact"
                )
            binary = args.binary
            map_path = args.map_path
            build_dir = args.build_dir
            if not binary.is_file():
                raise FileNotFoundError(f"missing binary: {binary}")
            if not map_path.is_file():
                raise FileNotFoundError(f"missing map: {map_path}")
        else:
            binary, map_path, build_dir = _find_minimal_artifacts(
                _candidate_minimal_dirs(args.minimal_build_dir, args.build_dir)
            )
    except (FileNotFoundError, ValueError) as exc:
        print(f"check_tinyui_minimal_profile FAIL: {exc}", file=sys.stderr)
        return 1

    cmd = [
        sys.executable,
        str(CHECKER),
        "--binary",
        str(binary),
        "--map",
        str(map_path),
        "--baseline",
        str(args.baseline),
        "--mode",
        "enforce",
    ]
    if build_dir is not None:
        cmd.extend(["--build-dir", str(build_dir)])

    print(
        "check_tinyui_minimal_profile via "
        f"profile={profile} binary={binary} map={map_path}",
        flush=True,
    )
    completed = subprocess.run(cmd, check=False, cwd=str(ROOT))
    if completed.returncode != 0:
        print(
            f"check_tinyui_minimal_profile FAIL: checker exit={completed.returncode}",
            file=sys.stderr,
        )
        return completed.returncode
    print("check_tinyui_minimal_profile OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
