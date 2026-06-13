#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests" / "tinyui" / "contract" / "tinyui_v21_transition_inventory.json"

PICOUI_DIR = ROOT / "picoui"
TINYUI_DIR = ROOT / "tinyui"
BACKEND_DIR = ROOT / "tinyui" / "src" / "backend" / "ldgui"
PICOUI_HEADERS = sorted((ROOT / "tinyui" / "include" / "picoui").rglob("*.h"))
TINYUI_HEADERS = sorted((ROOT / "tinyui" / "include").rglob("*.h"))
TINYUI_TOP_HEADERS = sorted((ROOT / "tinyui" / "include").glob("*.h"))
TINYUI_INCLUDE_PROBE = """\
#include "tinyui.h"
#include "core.h"
#include "screen.h"
#include "obj.h"
#include "label.h"
#include "button.h"
#include "switch.h"
int main(void) { return 0; }
"""

REQUIRED_BASELINE_KEYS = (
    "picoui_dir_exists",
    "tinyui_dir_exists",
    "backend_c_files",
    "backend_compat_includes",
    "top_level_wrapper_forward_count",
    "top_level_wrapper_forward_names",
    "compat_public_header_count",
    "tinyui_public_header_count",
    "compat_public_headers_require_followup",
    "picoui_public_api_count",
    "tinyui_public_api_count",
)


def count_prefix(headers: list[Path], prefix: str) -> int:
    pattern = re.compile(r"\b" + re.escape(prefix) + r"[A-Za-z0-9_]*\s*\(")
    total = 0
    for header in headers:
        text = header.read_text(encoding="utf-8")
        total += len(pattern.findall(text))
    return total


def collect_actual() -> dict[str, object]:
    backend_h_path = BACKEND_DIR / "backend.h"
    if backend_h_path.exists():
        backend_text = backend_h_path.read_text(encoding="utf-8")
        backend_compat_includes = sorted(
            set(
                re.findall(
                    r'#include "picoui/([^"]+)"',
                    backend_text,
                )
            )
        )
    else:
        backend_compat_includes = []
    top_level_wrapper_forward_names = sorted(
        header.name
        for header in TINYUI_TOP_HEADERS
        if re.search(r'#include "picoui/[^"]+"', header.read_text(encoding="utf-8"))
    )
    compat_only = {header.name for header in PICOUI_HEADERS} - {header.name for header in TINYUI_TOP_HEADERS}
    return {
        "picoui_dir_exists": PICOUI_DIR.exists(),
        "tinyui_dir_exists": TINYUI_DIR.exists(),
        "backend_c_files": len(sorted(BACKEND_DIR.glob("*.c"))),
        "backend_compat_includes": backend_compat_includes,
        "top_level_wrapper_forward_count": len(top_level_wrapper_forward_names),
        "top_level_wrapper_forward_names": top_level_wrapper_forward_names,
        "compat_public_header_count": len(PICOUI_HEADERS),
        "tinyui_public_header_count": len(TINYUI_TOP_HEADERS),
        "compat_public_headers_require_followup": sorted(compat_only),
        "picoui_public_api_count": count_prefix(PICOUI_HEADERS, "picoui_"),
        "tinyui_public_api_count": count_prefix(TINYUI_HEADERS, "tinyui_"),
    }


def load_expected() -> dict[str, object]:
    if not INVENTORY.exists():
        raise FileNotFoundError(f"missing inventory: {INVENTORY}")
    payload = json.loads(INVENTORY.read_text(encoding="utf-8"))
    baseline = payload.get("baseline")
    if not isinstance(baseline, dict):
        raise ValueError("inventory missing baseline object")
    missing_keys = [key for key in REQUIRED_BASELINE_KEYS if key not in baseline]
    if missing_keys:
        raise ValueError(
            "inventory baseline missing required keys: " + ", ".join(missing_keys)
        )
    return baseline


def print_current() -> int:
    print(json.dumps(collect_actual(), ensure_ascii=False, sort_keys=True, indent=2))
    return 0


def check_tinyui_headers_are_composable() -> None:
    with tempfile.NamedTemporaryFile("w", suffix=".c", encoding="utf-8", delete=False) as probe:
        probe.write(TINYUI_INCLUDE_PROBE)
        probe_path = Path(probe.name)
    try:
        result = subprocess.run(
            [
                "cc",
                "-fsyntax-only",
                "-I",
                str(ROOT / "tinyui" / "include"),
                "-I",
                str(ROOT / "tinyui" / "include" / "picoui"),
                str(probe_path),
            ],
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
    finally:
        probe_path.unlink(missing_ok=True)
    if result.returncode != 0:
        raise RuntimeError(
            "tinyui canonical headers are not composable:\n"
            + result.stdout
            + result.stderr
        )


def main(argv: list[str]) -> int:
    if len(argv) > 2:
        print("usage: check_tinyui_v21_transition_guards.py [--print-current]", file=sys.stderr)
        return 1
    if len(argv) == 2:
        if argv[1] != "--print-current":
            print(f"unknown argument: {argv[1]}", file=sys.stderr)
            return 1
        return print_current()

    try:
        expected = load_expected()
        actual = collect_actual()
        check_tinyui_headers_are_composable()
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    drift = False
    for key in REQUIRED_BASELINE_KEYS:
        expected_value = expected.get(key)
        actual_value = actual.get(key)
        if actual_value != expected_value:
            print(
                f"baseline drift: {key} expected {expected_value} actual {actual_value}",
                file=sys.stderr,
            )
            drift = True

    if drift:
        print(json.dumps(actual, ensure_ascii=False, sort_keys=True, indent=2), file=sys.stderr)
        return 1

    print("tinyui v2.1 transition baseline guard OK")
    print(json.dumps(actual, ensure_ascii=False, sort_keys=True, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
