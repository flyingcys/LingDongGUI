#!/usr/bin/env python3
"""Fail-closed: current-facing TinyUI surface must not ship removed/legacy APIs.

Scans public headers (excluding include/internal/**), demos, current-facing docs,
consumer tests, and optional install trees. Rejects:

- tinyui_app_* / tinyui_widget_* (old app-handle / common-setter names)
- misspellings: tabel / scroll_selecter / q_r_code / set_press(
- removed resource destroy names (tinyui_theme_destroy, tinyui_image_destroy)
- backend leaks: ld[A-Z]*, arm_2d_*, SIGNAL_*
- v22_demo_bridge residual references
- docs that tell users to enable TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE

Migration guide (docs/v2.3/v2.3-migration-guide.md) intentionally shows old
names in Before tables/examples. For that file only:

- legacy prefixes / misspellings are allowed
- bridge residual / "enable bridge" teaching remains forbidden

CLI::

    python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
    python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py --prefix build/install
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]

SCAN_SUFFIXES = {".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".md", ".txt", ".cmake"}

# Current-facing docs under docs/v2.3 (plans/ is historical and excluded).
V23_CURRENT_FACING_DOCS = (
    "docs/v2.3/v2.3-migration-guide.md",
    "docs/v2.3/README.md",
    "docs/v2.3/deferred-port-work.md",
)

# User-facing TinyUI docs only (exclude internal adaptation audits/reviews).
TINYUI_USER_DOCS = (
    "tinyui/docs/quick_start.md",
    "tinyui/docs/api_overview.md",
    "tinyui/docs/demo_guide.md",
    "tinyui/docs/resource_lifetime.md",
    "tinyui/docs/porting_rules.md",
)

MIGRATION_GUIDE_REL = "docs/v2.3/v2.3-migration-guide.md"

# Exact removed public API symbols (must not appear outside private/internal).
DEPRECATED_API_SYMBOLS: dict[str, str] = {
    "tinyui_q_r_code_init": "tinyui_qrcode_create",
    "tinyui_q_r_code_set_text": "tinyui_qrcode_set_text",
    "tinyui_tabel_show_keyboard": "tinyui_table_show_keyboard",
    "tinyui_button_set_press(": "tinyui_button_set_pressed(",
    "tinyui_button_get_press(": "tinyui_button_get_pressed(",
    "tinyui_theme_destroy": "theme is not a public resource object",
    "tinyui_image_destroy": "image sources are not heap-owned public objects",
    "tinyui_theme_create": "tinyui_theme_set / theme tokens",
    "tinyui_theme_apply_to_widget": "theme applied via runtime tokens",
}

# Fragment / typo checks. set_press( does not match set_press_image(.
PUBLIC_FORBIDDEN_FRAGMENTS: dict[str, str] = {
    "scroll_selecter": "scroll_selector",
    "q_r_code": "qrcode",
    "tabel": "table",
    "set_press(": "set_pressed(",
    "legacy_app": "tinyui_init/runtime",
}

# Regex rules applied line-by-line with path:message output.
# (name, pattern, message, apply_to_migration_guide)
REGEX_RULES: list[tuple[str, re.Pattern[str], str, bool]] = [
    (
        "tinyui_app_",
        re.compile(r"\btinyui_app_[A-Za-z0-9_]*\b"),
        "uses removed app-handle API {match!r}; use tinyui_init/runtime/timer public API",
        False,
    ),
    (
        "tinyui_widget_",
        re.compile(r"\btinyui_widget_[A-Za-z0-9_]*\b"),
        "uses removed common-setter API {match!r}; use tinyui_obj_*",
        False,
    ),
    (
        "ld[A-Z]",
        re.compile(r"\bld[A-Z][A-Za-z0-9_]*\b"),
        "backend leak {match!r}; current-facing surface must not reference LingDongGUI APIs",
        False,
    ),
    (
        "arm_2d_",
        re.compile(r"\barm_2d_[A-Za-z0-9_]*\b"),
        "backend leak {match!r}; current-facing surface must not reference Arm-2D APIs",
        False,
    ),
    (
        "SIGNAL_",
        re.compile(r"\bSIGNAL_[A-Za-z0-9_]*\b"),
        "backend leak {match!r}; use tinyui_event_* public events",
        False,
    ),
    (
        "v22_demo_bridge",
        re.compile(r"\bv22_demo_bridge\b"),
        "v22_demo_bridge residual {match!r}; bridge is removed, do not reference it",
        False,  # migration guide may name it only while forbidding it
    ),
    (
        "enable_v22_bridge",
        re.compile(r"TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE"),
        "must not teach enabling TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE; bridge is removed",
        True,
    ),
    # Positive "please enable bridge" teaching only. Prohibition sentences in the
    # migration guide (e.g. 禁止/不要写“请启用…”) are handled separately.
    (
        "enable_bridge_phrase",
        re.compile(
            r"(?i)((请|需要|必须)\s*(enable|启用|打开)\s*"
            r"(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE|"
            r".{0,20}(v22[_\s-]?demo[_\s-]?bridge|internal[_\s-]?v22|demo[_\s-]?bridge))|"
            r"\b(recommended?|should enable|must enable)\b.{0,40}"
            r"(v22[_\s-]?demo[_\s-]?bridge|TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE))"
        ),
        "must not instruct users to enable/use the v22 demo bridge",
        True,
    ),
    # Old grid negative track sentinel patterns (reliable textual forms only).
    (
        "grid_neg_sentinel",
        re.compile(
            r"(?i)(grid[_\s-]?(track|col|row|span).{0,40}-1\b|-1\b.{0,40}(grid|track)\s*sentinel|"
            r"negative\s+sentinel.{0,20}grid|grid.{0,20}negative\s+sentinel)"
        ),
        "old grid negative sentinel pattern {match!r}; use explicit track units / TINYUI_GRID_*",
        False,
    ),
]


def rel_to_root(path: Path, root: Path = ROOT) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return str(path)


def _is_private_internal(path: Path, root: Path) -> bool:
    rel_parts = path.relative_to(root).parts
    return "include" in rel_parts and "internal" in rel_parts


def _is_plans_doc(path: Path, root: Path) -> bool:
    rel = rel_to_root(path, root)
    return rel.startswith("docs/v2.3/plans/")


def iter_files(roots: list[Path], root: Path):
    for base in roots:
        if not base.exists():
            continue
        if base.is_file():
            if base.suffix in SCAN_SUFFIXES and not _is_private_internal(base, root):
                if not _is_plans_doc(base, root):
                    yield base
            continue
        for path in sorted(base.rglob("*")):
            if not path.is_file() or path.suffix not in SCAN_SUFFIXES:
                continue
            if _is_private_internal(path, root):
                continue
            if _is_plans_doc(path, root):
                continue
            yield path


def collect_scan_roots(root: Path, prefix: Path | None) -> list[Path]:
    roots: list[Path] = [
        root / "tinyui" / "include",
        root / "tinyui" / "demo",
        root / "tinyui_demo",
        root / "tests" / "tinyui" / "consumer",
    ]
    for rel in TINYUI_USER_DOCS:
        roots.append(root / rel)
    for rel in V23_CURRENT_FACING_DOCS:
        roots.append(root / rel)
    if prefix is not None:
        roots.extend(
            [
                prefix / "include",
                prefix / "lib" / "cmake" / "TinyUI",
            ]
        )
    return roots


def is_migration_guide(path: Path, root: Path) -> bool:
    return rel_to_root(path, root) == MIGRATION_GUIDE_REL


def scan_file(path: Path, root: Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    rel = rel_to_root(path, root)
    migration = is_migration_guide(path, root)
    failures: list[str] = []

    # Exact symbol / fragment checks (skipped on migration guide for legacy before tables).
    if not migration:
        for old, new in DEPRECATED_API_SYMBOLS.items():
            if old in text:
                failures.append(f"{rel}: uses deprecated {old!r}; use {new}")
        for old, new in PUBLIC_FORBIDDEN_FRAGMENTS.items():
            if old in text:
                # Avoid false positive: set_press( is intentional; set_press_image is legal.
                # "tabel" can appear inside longer words; require word-ish boundary.
                if old == "tabel" and not re.search(r"(?i)(?<![A-Za-z0-9_])tabel(?![A-Za-z0-9_])", text):
                    continue
                failures.append(f"{rel}: public fragment {old!r}; use {new}")

    for _name, pattern, message, apply_to_migration in REGEX_RULES:
        if migration and not apply_to_migration:
            continue
        for line_no, line in enumerate(text.splitlines(), start=1):
            # Migration guide may quote forbidden phrases while prohibiting them.
            if migration and _name in ("enable_bridge_phrase", "enable_v22_bridge"):
                if re.search(r"(禁止|不得|不要|勿|removed|delete|删除)", line):
                    continue
            for match in pattern.finditer(line):
                token = match.group(0)
                failures.append(f"{rel}:{line_no}: {message.format(match=token)}")

    return failures


def run_checks(*, root: Path, prefix: Path | None) -> list[str]:
    failures: list[str] = []
    seen: set[Path] = set()
    for path in iter_files(collect_scan_roots(root, prefix), root):
        resolved = path.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        failures.extend(scan_file(path, root))
    return failures


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=ROOT,
        help="Repository root (default: inferred from script location)",
    )
    parser.add_argument(
        "--prefix",
        type=Path,
        default=None,
        help="Optional install prefix; scans include/ and lib/cmake/TinyUI/",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = args.root.resolve()
    prefix = args.prefix.resolve() if args.prefix else None
    failures = run_checks(root=root, prefix=prefix)
    if failures:
        # De-dupe while preserving order for stable CI output.
        ordered: list[str] = []
        seen_msg: set[str] = set()
        for item in failures:
            if item in seen_msg:
                continue
            seen_msg.add(item)
            ordered.append(item)
        sys.stderr.write("\n".join(ordered) + "\n")
        print(f"TINYUI_DEPRECATED_API_USAGE_FAIL count={len(ordered)}", file=sys.stderr)
        return 1
    print("TINYUI_DEPRECATED_API_USAGE_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
