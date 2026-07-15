#!/usr/bin/env python3
"""Validate TinyUI per-widget feature options against tinyui_core sources."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]

# Explicit special filenames stay in the manifest rather than relying on
# automatic underscore conversion edge cases.
_WIDGET_SOURCE_OVERRIDES: dict[str, str] = {
    "QRCODE": "qrcode.c",
    "PROGRESS_WHEEL": "progress_wheel.c",
    "SCROLL_SELECTOR": "scroll_selector.c",
}

_WIDGET_NAMES: tuple[str, ...] = (
    "WINDOW",
    "BACKGROUND",
    "LABEL",
    "BUTTON",
    "CHECKBOX",
    "SWITCH",
    "SLIDER",
    "TEXT",
    "IMAGE",
    "LINE_EDIT",
    "KEYBOARD",
    "CANVAS",
    "COMBO_BOX",
    "SCROLL_SELECTOR",
    "TABLE",
    "GRAPH",
    "CALENDAR",
    "ARC",
    "GAUGE",
    "ICON_SLIDER",
    "RADIAL_MENU",
    "PROGRESS_BAR",
    "PROGRESS_WHEEL",
    "QRCODE",
    "ANIMATION",
    "DATE_TIME",
    "CLOCK",
    "LIST",
    "MESSAGE_BOX",
)

_OPTIONAL_MODULES: tuple[str, ...] = (
    "TINYUI_ENABLE_THEME",
    "TINYUI_ENABLE_DIAGNOSTICS",
    "TINYUI_ENABLE_NATIVE_INTEROP",
)

_CACHE_BOOL_RE = re.compile(
    r"^(?P<name>TINYUI_ENABLE_[A-Z0-9_]+):BOOL=(?P<value>ON|OFF|TRUE|FALSE|1|0)\s*$",
    re.IGNORECASE,
)


def _source_filename(name: str) -> str:
    if name in _WIDGET_SOURCE_OVERRIDES:
        return _WIDGET_SOURCE_OVERRIDES[name]
    return f"{name.lower()}.c"


def widget_manifest() -> list[dict[str, str]]:
    entries: list[dict[str, str]] = []
    for name in _WIDGET_NAMES:
        source = f"tinyui/src/widgets/{_source_filename(name)}"
        entries.append(
            {
                "name": name,
                "option": f"TINYUI_ENABLE_{name}",
                "source": source,
            }
        )
    return entries


def optional_module_options() -> list[str]:
    return list(_OPTIONAL_MODULES)


def parse_cmake_cache(path: Path) -> dict[str, int]:
    text = path.read_text(encoding="utf-8")
    values: dict[str, int] = {}
    for line in text.splitlines():
        match = _CACHE_BOOL_RE.match(line.strip())
        if match is None:
            continue
        raw = match.group("value").upper()
        values[match.group("name")] = 0 if raw in {"OFF", "FALSE", "0"} else 1
    return values


def _normalize_source_path(path: str) -> str:
    candidate = Path(path)
    parts = candidate.as_posix().split("/")
    for index, part in enumerate(parts):
        if part == "tinyui" and index + 1 < len(parts) and parts[index + 1] == "src":
            return "/".join(parts[index:])
    return candidate.as_posix()


def check_feature_source_consistency(
    manifest: list[dict[str, str]],
    options: dict[str, int],
    sources: list[str],
) -> list[str]:
    errors: list[str] = []
    normalized = {_normalize_source_path(item) for item in sources}
    for entry in manifest:
        option = entry["option"]
        source = entry["source"]
        if option not in options:
            errors.append(f"missing cmake option {option} for widget {entry['name']}")
            continue
        enabled = int(options[option]) != 0
        present = source in normalized
        if enabled and not present:
            errors.append(f"enabled option {option}=1 requires source {source}")
        if (not enabled) and present:
            errors.append(f"disabled option {option}=0 forbids source {source}")
    return errors


def load_target_sources_from_cmake_files(build_dir: Path, target: str = "tinyui_core") -> list[str]:
    depend = build_dir / "CMakeFiles" / f"{target}.dir" / "DependInfo.cmake"
    if not depend.is_file():
        raise FileNotFoundError(f"missing DependInfo for target {target}: {depend}")
    text = depend.read_text(encoding="utf-8")
    sources: list[str] = []
    # DependInfo lists absolute paths quoted in CMAKE_DEPENDS_CHECK_C / similar blocks.
    for match in re.finditer(r'"([^"]+\.c)"', text):
        sources.append(match.group(1))
    if not sources:
        # Fallback: ninja/make build.make may still list source paths.
        build_make = build_dir / "CMakeFiles" / f"{target}.dir" / "build.make"
        if build_make.is_file():
            for match in re.finditer(r"(/[^\s:]+\.c)\b", build_make.read_text(encoding="utf-8")):
                sources.append(match.group(1))
    return sources


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--target", default="tinyui_core")
    args = parser.parse_args(argv)

    build_dir = args.build_dir.resolve()
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        print(f"missing CMakeCache.txt: {cache}", file=sys.stderr)
        return 1

    try:
        options = parse_cmake_cache(cache)
        sources = load_target_sources_from_cmake_files(build_dir, args.target)
    except (OSError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 1

    manifest = widget_manifest()
    errors = check_feature_source_consistency(manifest, options, sources)
    if errors:
        for item in errors:
            print(item, file=sys.stderr)
        return 1

    enabled = sum(1 for entry in manifest if options.get(entry["option"], 0))
    print(
        f"feature options OK target={args.target} widgets={len(manifest)} "
        f"enabled={enabled} sources={len(sources)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
