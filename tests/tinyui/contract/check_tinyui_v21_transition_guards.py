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

TINYUI_DIR = ROOT / "tinyui"
BACKEND_DIR = ROOT / "tinyui" / "src" / "backend" / "ldgui"
TINYUI_INCLUDE_DIR = ROOT / "tinyui" / "include"
TINYUI_COMPAT_DIR = TINYUI_INCLUDE_DIR / "tinyui"
TINYUI_HEADERS = sorted(
    header for header in TINYUI_INCLUDE_DIR.rglob("*.h")
    if "internal" not in header.relative_to(TINYUI_INCLUDE_DIR).parts
)
TINYUI_TOP_HEADERS = sorted(TINYUI_INCLUDE_DIR.glob("*.h"))
TINYUI_COMPAT_HEADERS = sorted(TINYUI_COMPAT_DIR.rglob("*.h")) if TINYUI_COMPAT_DIR.exists() else []
TINYUI_INCLUDE_PROBE = """\
#include "tinyui.h"
#include "core/obj.h"
#include "widgets/label.h"
#include "widgets/button.h"
#include "widgets/switch.h"
int main(void) { return 0; }
"""

REQUIRED_BASELINE_KEYS = (
    "tinyui_dir_exists",
    "backend_c_files",
    "backend_compat_includes",
    "top_level_wrapper_forward_count",
    "top_level_wrapper_forward_names",
    "compat_public_header_count",
    "tinyui_public_header_count",
    "compat_public_headers_require_followup",
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
                    r'#include "tinyui/([^"]+)"',
                    backend_text,
                )
            )
        )
    else:
        backend_compat_includes = []
    top_level_wrapper_forward_names = sorted(
        header.name
        for header in TINYUI_TOP_HEADERS
        if re.search(r'#include "tinyui/[^"]+"', header.read_text(encoding="utf-8"))
    )
    compat_only = sorted(
        str(header.relative_to(TINYUI_COMPAT_DIR))
        for header in TINYUI_COMPAT_HEADERS
    )
    return {
        "tinyui_dir_exists": TINYUI_DIR.exists(),
        "backend_c_files": len(sorted(BACKEND_DIR.glob("*.c"))),
        "backend_compat_includes": backend_compat_includes,
        "top_level_wrapper_forward_count": len(top_level_wrapper_forward_names),
        "top_level_wrapper_forward_names": top_level_wrapper_forward_names,
        "compat_public_header_count": len(TINYUI_COMPAT_HEADERS),
        "tinyui_public_header_count": len(TINYUI_TOP_HEADERS),
        "compat_public_headers_require_followup": compat_only,
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


def _generated_config_include_dirs() -> list[str]:
    """Locate generated tinyui_config.h for composability probes."""
    candidates = [
        ROOT / "build" / "v2.3" / "generated" / "tinyui",
        ROOT / "build" / "v2.3-m3" / "generated" / "tinyui",
        ROOT / "build" / "tinyui-runtime" / "generated" / "tinyui",
    ]
    dirs: list[str] = []
    for path in candidates:
        if (path / "tinyui_config.h").is_file():
            dirs.extend(["-I", str(path)])
    # Always allow a build/*/generated/tinyui discovery for local trees.
    build_root = ROOT / "build"
    if build_root.is_dir():
        for cfg in build_root.glob("*/generated/tinyui/tinyui_config.h"):
            include_dir = str(cfg.parent)
            if include_dir not in dirs:
                dirs.extend(["-I", include_dir])
            break
    return dirs


def check_tinyui_headers_are_composable() -> None:
    with tempfile.NamedTemporaryFile("w", suffix=".c", encoding="utf-8", delete=False) as probe:
        probe.write(TINYUI_INCLUDE_PROBE)
        probe_path = Path(probe.name)
    try:
        cmd = [
            "cc",
            "-fsyntax-only",
            "-I",
            str(ROOT / "tinyui" / "include"),
            "-I",
            str(ROOT / "tinyui" / "include" / "tinyui"),
        ]
        cmd.extend(_generated_config_include_dirs())
        cmd.append(str(probe_path))
        result = subprocess.run(
            cmd,
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
