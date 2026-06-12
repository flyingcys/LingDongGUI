#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests" / "picoui" / "contract" / "tinyui_transition_inventory.json"
BACKEND_DIR = ROOT / "tinyui" / "src" / "backend" / "ldgui"
APP_HEADER = ROOT / "tinyui" / "include" / "picoui" / "app.h"
APP_SOURCE = ROOT / "tinyui" / "src" / "core" / "app.c"
PICOUI_PUBLIC_HEADERS = sorted((ROOT / "tinyui" / "include" / "picoui").glob("*.h"))
TINYUI_INCLUDE_DIR = ROOT / "tinyui" / "include"
TINYUI_HEADERS = [
    TINYUI_INCLUDE_DIR / "tinyui.h",
    TINYUI_INCLUDE_DIR / "core.h",
    TINYUI_INCLUDE_DIR / "obj.h",
    TINYUI_INCLUDE_DIR / "screen.h",
    TINYUI_INCLUDE_DIR / "label.h",
    TINYUI_INCLUDE_DIR / "button.h",
    TINYUI_INCLUDE_DIR / "switch.h",
]
API_PATTERN_TEMPLATE = r"\b{prefix}[A-Za-z0-9_]*\s*\("
REQUIRED_BASELINE_KEYS = (
    "backend_c_files",
    "app_header_exists",
    "app_source_exists",
    "picoui_public_api_count",
    "tinyui_public_api_count",
)


def count_api(headers: list[Path], prefix: str) -> int:
    pattern = re.compile(API_PATTERN_TEMPLATE.format(prefix=re.escape(prefix)))
    total = 0
    for header in headers:
        text = header.read_text(encoding="utf-8")
        total += len(pattern.findall(text))
    return total


def load_expected() -> dict[str, object]:
    if not INVENTORY.exists():
        raise FileNotFoundError(f"missing inventory: {INVENTORY}")
    payload = json.loads(INVENTORY.read_text(encoding="utf-8"))
    baseline = payload.get("baseline")
    if not isinstance(baseline, dict):
        raise ValueError("inventory missing baseline object")
    missing_keys = [key for key in REQUIRED_BASELINE_KEYS if key not in baseline]
    if missing_keys:
        missing_text = ", ".join(missing_keys)
        raise ValueError(f"inventory baseline missing required keys: {missing_text}")
    return baseline


def collect_actual() -> dict[str, object]:
    return {
        "backend_c_files": len(sorted(BACKEND_DIR.glob("*.c"))),
        "app_header_exists": APP_HEADER.exists(),
        "app_source_exists": APP_SOURCE.exists(),
        "picoui_public_api_count": count_api(PICOUI_PUBLIC_HEADERS, "picoui_"),
        "tinyui_public_api_count": count_api(TINYUI_HEADERS, "tinyui_"),
    }


def main() -> int:
    try:
        for header in TINYUI_HEADERS:
            if not header.exists():
                print(f"missing tinyui header: {header}", file=sys.stderr)
                return 1
        expected = load_expected()
        actual = collect_actual()
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1

    drift = False
    for key, expected_value in expected.items():
        actual_value = actual.get(key)
        if actual_value != expected_value:
            print(
                f"baseline drift: {key} expected {expected_value} actual {actual_value}",
                file=sys.stderr,
            )
            drift = True

    if drift:
        print(json.dumps(actual, indent=2, sort_keys=True), file=sys.stderr)
        return 1

    print("tinyui transition baseline guard OK")
    print(json.dumps(actual, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
