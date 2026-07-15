#!/usr/bin/env python3
"""Static anti-heavyweight gate for TinyUI public ABI and hot runtime paths.

Layers:
1. public identifier scan — reject LVGL-like heavyweight concepts
2. runtime allocation scan — reject heap use in process / dispatch / theme /
   layout setters / ordinary getters

Comments and string literals never count as identifier hits.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]

# Forbidden concept fragments matched as underscore-delimited identifier segments.
# Multi-segment fragments (style_class, resource_cache, ...) require consecutive
# segments; single-segment tokens use whole-segment equality.
FORBIDDEN_PUBLIC_FRAGMENTS: tuple[str, ...] = (
    "style_class",
    "selector",
    "cascade",
    "bubble",
    "capture",
    "property_registry",
    "property_database",
    "resource_manager",
    "resource_cache",
    "renderer_plugin",
    "draw_task",
)

# Known good identifiers that contain a forbidden single-token fragment as a
# substring of a longer legitimate name (scroll_selector embeds "selector").
PUBLIC_IDENTIFIER_ALLOWLIST_PREFIXES: tuple[str, ...] = (
    "scroll_selector",
    "scroll_selecter",
    "SCROLL_SELECTOR",
    "SCROLL_SELECTER",
    "tinyui_scroll_selector",
    "tinyui_scroll_selecter",
    "TINYUI_ENABLE_SCROLL_SELECTOR",
)

FORBIDDEN_ALLOC_CALLEES: tuple[str, ...] = (
    "malloc",
    "calloc",
    "realloc",
    "free",
    "ldMalloc",
    "ldCalloc",
    "ldFree",
)

# Hot paths that must remain allocation-free in steady-state runtime.
RUNTIME_SCAN_FUNCTIONS: tuple[str, ...] = (
    "tinyui_process",
    "tinyui_theme_apply",
    "tinyui_runtime_internal_app_pump_timers",
    "tinyui_runtime_internal_widget_dispatch_signal",
    "tinyui_runtime_internal_widget_dispatch_event",
    "tinyui_runtime_internal_widget_dispatch_native_signal",
    "tinyui_obj_add_event_cb",
    "tinyui_obj_remove_event_cb",
)

RUNTIME_FUNCTION_NAME_RE = re.compile(
    r"\b("
    r"tinyui_process|"
    r"tinyui_theme_apply|"
    r"tinyui_runtime_internal_app_pump_timers|"
    r"tinyui_runtime_internal_widget_dispatch_[A-Za-z0-9_]+|"
    r"tinyui_obj_add_event_cb|"
    r"tinyui_obj_remove_event_cb|"
    r"tinyui_obj_get_[A-Za-z0-9_]+|"
    r"tinyui_obj_set_[A-Za-z0-9_]+|"
    r"tinyui_layout_[A-Za-z0-9_]+|"
    r"tinyui_[a-z0-9_]+_set_(?:flex|grid)_[A-Za-z0-9_]+"
    r")\b"
)

IDENTIFIER_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
ALLOC_CALL_RE = re.compile(
    r"\b(?P<name>" + "|".join(re.escape(n) for n in FORBIDDEN_ALLOC_CALLEES) + r")\s*\("
)

PUBLIC_HEADER_GLOBS: tuple[str, ...] = (
    "tinyui/include/tinyui.h",
    "tinyui/include/core/*.h",
    "tinyui/include/style/*.h",
    "tinyui/include/theme/*.h",
    "tinyui/include/layout/*.h",
    "tinyui/include/resource/*.h",
    "tinyui/include/widgets/*.h",
    "tinyui/include/integration/*.h",
)

RUNTIME_SOURCE_GLOBS: tuple[str, ...] = (
    "tinyui/src/core/*.c",
    "tinyui/src/theme/*.c",
    "tinyui/src/layout/*.c",
)


def strip_comments_and_strings(text: str) -> str:
    """Remove C comments and string/char literals so docs never false-hit."""
    out: list[str] = []
    i = 0
    n = len(text)
    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if ch == "/" and nxt == "/":
            i += 2
            while i < n and text[i] not in "\n\r":
                i += 1
            continue
        if ch == "/" and nxt == "*":
            i += 2
            while i + 1 < n and not (text[i] == "*" and text[i + 1] == "/"):
                # Preserve newlines to keep approximate line mapping for diagnostics.
                if text[i] in "\n\r":
                    out.append(text[i])
                i += 1
            i = min(n, i + 2)
            continue
        if ch in "\"'":
            quote = ch
            out.append(" ")
            i += 1
            while i < n:
                if text[i] == "\\":
                    i = min(n, i + 2)
                    continue
                if text[i] == quote:
                    i += 1
                    break
                if text[i] in "\n\r":
                    out.append(text[i])
                i += 1
            continue
        out.append(ch)
        i += 1
    return "".join(out)


def _segments(identifier: str) -> list[str]:
    return [part for part in identifier.split("_") if part]


def _has_consecutive_segments(segments: list[str], fragment_segments: list[str]) -> bool:
    if not fragment_segments or len(fragment_segments) > len(segments):
        return False
    width = len(fragment_segments)
    for index in range(0, len(segments) - width + 1):
        if segments[index : index + width] == fragment_segments:
            return True
    return False


def _is_allowlisted_identifier(identifier: str) -> bool:
    lowered = identifier.lower()
    # scroll_selector family embeds the forbidden single token "selector".
    if "scroll_selector" in lowered or "scroll_selecter" in lowered:
        return True
    if identifier.startswith("TINYUI_ENABLE_SCROLL_SELECTOR"):
        return True
    for prefix in PUBLIC_IDENTIFIER_ALLOWLIST_PREFIXES:
        if identifier == prefix:
            return True
    return False


def _style_class_is_residual_field(segments: list[str], match_start: int, match_width: int) -> bool:
    """Allow residual prop field `style_class` / `*_FIELD_STYLE_CLASS`.

    Heavyweight compounds such as tinyui_style_class_registry keep trailing
    segments after the style_class match and remain forbidden.
    """
    if match_start + match_width != len(segments):
        return False
    # exact style_class, or presence-bit macros ending in FIELD_STYLE_CLASS
    if len(segments) == match_width:
        return True
    if match_start >= 1 and segments[match_start - 1].lower() == "field":
        return True
    # bare trailing field name: ..._style_class on a props struct
    return match_width == 2 and segments[-2].lower() == "style" and segments[-1].lower() == "class"


def identifier_has_forbidden_fragment(identifier: str) -> str | None:
    """Return the matched forbidden fragment, or None if the identifier is clean."""
    if _is_allowlisted_identifier(identifier):
        return None
    segments = _segments(identifier)
    lowered = [s.lower() for s in segments]
    for fragment in FORBIDDEN_PUBLIC_FRAGMENTS:
        frag_parts = [p.lower() for p in fragment.split("_")]
        width = len(frag_parts)
        if width == 0 or width > len(lowered):
            continue
        for index in range(0, len(lowered) - width + 1):
            if lowered[index : index + width] != frag_parts:
                continue
            if fragment == "style_class" and _style_class_is_residual_field(segments, index, width):
                continue
            return fragment
    return None


def scan_public_identifiers_text(header: str, text: str) -> list[str]:
    cleaned = strip_comments_and_strings(text)
    errors: list[str] = []
    seen: set[str] = set()
    for match in IDENTIFIER_RE.finditer(cleaned):
        identifier = match.group(0)
        fragment = identifier_has_forbidden_fragment(identifier)
        if fragment is None:
            continue
        key = f"{header}:{identifier}:{fragment}"
        if key in seen:
            continue
        seen.add(key)
        errors.append(
            f"{header}: forbidden public identifier '{identifier}' "
            f"(matches heavyweight fragment '{fragment}')"
        )
    return errors


def _iter_public_headers(root: Path) -> list[Path]:
    headers: list[Path] = []
    for pattern in PUBLIC_HEADER_GLOBS:
        headers.extend(sorted(root.glob(pattern)))
    # De-duplicate while preserving order.
    unique: list[Path] = []
    seen: set[Path] = set()
    for path in headers:
        if path in seen:
            continue
        seen.add(path)
        unique.append(path)
    return unique


def scan_public_identifiers(root: Path) -> list[str]:
    errors: list[str] = []
    for path in _iter_public_headers(root):
        rel = path.relative_to(root).as_posix()
        text = path.read_text(encoding="utf-8", errors="replace")
        errors.extend(scan_public_identifiers_text(rel, text))
    return errors


def _skip_ws(text: str, index: int) -> int:
    n = len(text)
    while index < n and text[index].isspace():
        index += 1
    return index


def extract_function_bodies(text: str, functions: list[str] | None = None) -> dict[str, list[str]]:
    """Extract top-level-ish C function bodies by brace matching.

    Returns mapping function_name -> list of body texts (multiple defs allowed).
    """
    cleaned = strip_comments_and_strings(text)
    wanted = set(functions) if functions is not None else None
    bodies: dict[str, list[str]] = {}
    n = len(cleaned)
    i = 0
    while i < n:
        match = IDENTIFIER_RE.match(cleaned, i)
        if match is None:
            i += 1
            continue
        name = match.group(0)
        j = match.end()
        j = _skip_ws(cleaned, j)
        if j >= n or cleaned[j] != "(":
            i = match.end()
            continue
        # Walk parameter list.
        depth = 0
        k = j
        while k < n:
            ch = cleaned[k]
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    k += 1
                    break
            k += 1
        else:
            i = match.end()
            continue
        k = _skip_ws(cleaned, k)
        # Skip attributes / qualifiers between ) and {
        while k < n and cleaned[k] not in "{;":
            # stop early on next identifier start of a new declaration line
            if cleaned.startswith("\n", k):
                # allow multi-line signatures
                k += 1
                k = _skip_ws(cleaned, k)
                continue
            k += 1
        if k >= n or cleaned[k] != "{":
            i = match.end()
            continue
        if wanted is not None and name not in wanted:
            i = match.end()
            continue
        # Brace-match body.
        body_start = k + 1
        depth = 1
        p = body_start
        while p < n and depth:
            ch = cleaned[p]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
            p += 1
        body = cleaned[body_start : p - 1]
        bodies.setdefault(name, []).append(body)
        i = p
    return bodies


def scan_forbidden_allocations(
    text: str, functions: list[str] | None = None
) -> list[str]:
    """Reject heap APIs inside selected function bodies."""
    errors: list[str] = []
    bodies = extract_function_bodies(text, functions)
    targets = functions if functions is not None else sorted(bodies)
    for name in targets:
        for body in bodies.get(name, []):
            for match in ALLOC_CALL_RE.finditer(body):
                callee = match.group("name")
                errors.append(
                    f"forbidden allocation '{callee}' inside function '{name}'"
                )
    return errors


def _discover_runtime_functions(text: str) -> list[str]:
    return sorted(set(RUNTIME_FUNCTION_NAME_RE.findall(text)))


def scan_runtime_sources(root: Path) -> list[str]:
    errors: list[str] = []
    for pattern in RUNTIME_SOURCE_GLOBS:
        for path in sorted(root.glob(pattern)):
            rel = path.relative_to(root).as_posix()
            text = path.read_text(encoding="utf-8", errors="replace")
            names = _discover_runtime_functions(text)
            # Always include the fixed hot set when present in this file.
            for required in RUNTIME_SCAN_FUNCTIONS:
                if re.search(rf"\b{re.escape(required)}\b", strip_comments_and_strings(text)):
                    if required not in names:
                        names.append(required)
            if not names:
                continue
            for item in scan_forbidden_allocations(text, names):
                errors.append(f"{rel}: {item}")
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args(argv)
    root = args.root.resolve()

    errors: list[str] = []
    errors.extend(scan_public_identifiers(root))
    errors.extend(scan_runtime_sources(root))

    if errors:
        print("TinyUI lightweight architecture violations:", file=sys.stderr)
        for item in errors:
            print(item, file=sys.stderr)
        return 1

    print("tinyui lightweight architecture OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
