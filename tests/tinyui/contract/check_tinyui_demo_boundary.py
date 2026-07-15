#!/usr/bin/env python3
"""Fail-closed gate: TinyUI demos match the M4 manifest and use only canonical API.

Until Task 3+ migrate every demo to ``tinyui_demo_<name>_build(screen)`` and
rename ``scroll_selector_basic``, the checker is expected to FAIL and
enumerate current debt (legacy API, old void signatures, misspelled paths,
etc.). Task 2 additionally locks the registry/runner contract:
``tinyui_demo_build_cb_t`` only, no frame hook, no display-config backend type.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = Path(__file__).resolve().parent
DEFAULT_MANIFEST = CONTRACT_DIR / "tinyui_demo_manifest.json"
DEMO_DIR = ROOT / "tinyui" / "demo"
SDL_CMAKE = ROOT / "examples" / "sdl" / "CMakeLists.txt"
REGISTRY_SOURCES = (
    DEMO_DIR / "tinyui_demos.c",
    DEMO_DIR / "tinyui_demos.h",
)

BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//.*?$", re.MULTILINE)
STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"')
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.MULTILINE)
OLD_VOID_ENTRY_RE = re.compile(
    r"\bvoid\s+(tinyui_demo_[A-Za-z0-9_]+)\s*\(\s*void\s*\)"
)
SYMBOL_CALL_OR_DECL_RE_TEMPLATE = r"\b{symbol}\s*\("

# Forbidden tokens in all demo tree sources (including registry).
# Do NOT blanket-ban tinyui_window_ — window is a required canonical capability.
FORBIDDEN_PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    ("tinyui_app_", re.compile(r"\btinyui_app_[A-Za-z0-9_]*\b")),
    ("tinyui_widget_", re.compile(r"\btinyui_widget_[A-Za-z0-9_]*\b")),
    ("tinyui_native_", re.compile(r"\btinyui_native_[A-Za-z0-9_]*\b")),
    ("ld[A-Z]", re.compile(r"\bld[A-Z][A-Za-z0-9_]*\b")),
    ("arm_2d_", re.compile(r"\barm_2d_[A-Za-z0-9_]*\b")),
    ("SIGNAL_", re.compile(r"\bSIGNAL_[A-Za-z0-9_]*\b")),
    ("struct tinyui_widget", re.compile(r"\bstruct\s+tinyui_widget\b")),
    ("tinyui_demos_frame", re.compile(r"\btinyui_demos_frame\b")),
]

# Builder bodies must not own runtime lifecycle or pull platform headers.
BUILDER_LIFECYCLE_PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    ("tinyui_init", re.compile(r"\btinyui_init\s*\(")),
    ("tinyui_deinit", re.compile(r"\btinyui_deinit\s*\(")),
    ("tinyui_screen_create", re.compile(r"\btinyui_screen_create\s*\(")),
    ("tinyui_screen_load", re.compile(r"\btinyui_screen_load\s*\(")),
]

PLATFORM_HEADERS = frozenset(
    {
        "SDL.h",
        "SDL2/SDL.h",
        "SDL_main.h",
        "ldGui.h",
        "ldBase.h",
        "ldConfig.h",
        "arm_2d.h",
        "arm_2d_helper.h",
        "arm_2d_helper_pfb.h",
    }
)


def strip_c_noise(text: str) -> str:
    """Remove comments and string literal contents for token scanning."""
    text = BLOCK_COMMENT_RE.sub("", text)
    text = LINE_COMMENT_RE.sub("", text)
    text = STRING_RE.sub('""', text)
    return text


def error(code: str, **details: Any) -> dict[str, Any]:
    return {"code": code, **details}


def format_error(item: dict[str, Any]) -> str:
    code = item.get("code", "error")
    parts = [f"{code}"]
    for key, value in item.items():
        if key == "code":
            continue
        parts.append(f"{key}={value}")
    return " | ".join(parts)


def rel_to_root(path: Path, root: Path = ROOT) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def load_manifest(path: Path = DEFAULT_MANIFEST) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    errors: list[dict[str, Any]] = []
    if not path.is_file():
        return {}, [error("missing_manifest", path=str(path))]
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return {}, [error("invalid_manifest_json", path=str(path), detail=str(exc))]

    if not isinstance(data, dict):
        return {}, [error("invalid_manifest_root", path=str(path))]
    if data.get("schema_version") != 1:
        errors.append(
            error(
                "invalid_manifest_schema_version",
                path=str(path),
                value=data.get("schema_version"),
            )
        )

    demos = data.get("demos")
    if not isinstance(demos, list) or not demos:
        errors.append(error("invalid_manifest_demos", path=str(path)))
        return data, errors

    names: set[str] = set()
    for index, entry in enumerate(demos):
        if not isinstance(entry, dict):
            errors.append(error("invalid_manifest_entry", index=index))
            continue
        name = entry.get("name")
        source = entry.get("source")
        build_symbol = entry.get("build_symbol")
        if not isinstance(name, str) or not name:
            errors.append(error("invalid_manifest_entry_name", index=index))
            continue
        if name in names:
            errors.append(error("duplicate_manifest_name", name=name))
        names.add(name)
        if not isinstance(source, str) or not source.endswith(".c"):
            errors.append(error("invalid_manifest_entry_source", name=name, source=source))
        if not isinstance(build_symbol, str) or not build_symbol.endswith("_build"):
            errors.append(
                error(
                    "invalid_manifest_entry_build_symbol",
                    name=name,
                    build_symbol=build_symbol,
                )
            )
        expected_symbol = f"tinyui_demo_{name}_build"
        if isinstance(build_symbol, str) and build_symbol != expected_symbol:
            errors.append(
                error(
                    "manifest_build_symbol_mismatch",
                    name=name,
                    build_symbol=build_symbol,
                    expected=expected_symbol,
                )
            )
        expected_source = f"tinyui/demo/{name}/{name}.c"
        if isinstance(source, str) and source != expected_source:
            errors.append(
                error(
                    "manifest_source_path_mismatch",
                    name=name,
                    source=source,
                    expected=expected_source,
                )
            )

    ordered = [entry.get("name") for entry in demos if isinstance(entry, dict)]
    if ordered != sorted(x for x in ordered if isinstance(x, str)):
        errors.append(error("manifest_demos_not_sorted_by_name"))

    if len(demos) != 29:
        errors.append(error("manifest_demo_count_mismatch", count=len(demos), expected=29))

    return data, errors


# L5 evidence runners under tinyui/demo/ are separate CMake targets, not the
# 29-entry consumer demo manifest. They are still scanned for forbidden tokens
# when present on disk, but do not participate in source-set equality.
NON_MANIFEST_DEMO_PREFIXES = ("v23_",)


def is_manifest_demo_name(name: str) -> bool:
    return not any(name.startswith(prefix) for prefix in NON_MANIFEST_DEMO_PREFIXES)


def collect_actual_demo_sources(
    demo_dir: Path = DEMO_DIR,
    *,
    manifest_only: bool = True,
) -> dict[str, Path]:
    """Return ``{demo_name: absolute .c path}`` for each ``name/name.c`` subdir."""
    found: dict[str, Path] = {}
    if not demo_dir.is_dir():
        return found
    for child in sorted(demo_dir.iterdir()):
        if not child.is_dir():
            continue
        if manifest_only and not is_manifest_demo_name(child.name):
            continue
        source = child / f"{child.name}.c"
        if source.is_file():
            found[child.name] = source
    return found


def check_source_set(
    manifest: dict[str, Any],
    actual: dict[str, Path],
    root: Path = ROOT,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    demos = manifest.get("demos") or []
    manifest_names = {
        entry["name"] for entry in demos if isinstance(entry, dict) and "name" in entry
    }
    actual_names = set(actual)

    for name in sorted(manifest_names - actual_names):
        errors.append(error("missing_demo_source", name=name))
    for name in sorted(actual_names - manifest_names):
        rel = actual[name].relative_to(root).as_posix()
        errors.append(error("unexpected_demo_source", name=name, source=rel))

    for entry in demos:
        if not isinstance(entry, dict):
            continue
        name = entry.get("name")
        expected = entry.get("source")
        if not isinstance(name, str) or name not in actual:
            continue
        rel = actual[name].relative_to(root).as_posix()
        if rel != expected:
            errors.append(
                error(
                    "demo_source_path_mismatch",
                    name=name,
                    actual=rel,
                    expected=expected,
                )
            )
    return errors


def count_symbol_decls(text: str, symbol: str) -> int:
    cleaned = strip_c_noise(text)
    pattern = re.compile(SYMBOL_CALL_OR_DECL_RE_TEMPLATE.format(symbol=re.escape(symbol)))
    return len(pattern.findall(cleaned))


def check_build_symbols(
    manifest: dict[str, Any],
    actual: dict[str, Path],
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    for entry in manifest.get("demos") or []:
        if not isinstance(entry, dict):
            continue
        name = entry.get("name")
        symbol = entry.get("build_symbol")
        if not isinstance(name, str) or not isinstance(symbol, str):
            continue
        source = actual.get(name)
        if source is None:
            continue
        header = source.with_suffix(".h")
        c_text = source.read_text(encoding="utf-8", errors="replace")
        h_text = header.read_text(encoding="utf-8", errors="replace") if header.is_file() else ""
        c_count = count_symbol_decls(c_text, symbol)
        h_count = count_symbol_decls(h_text, symbol)
        if c_count == 0:
            errors.append(
                error(
                    "missing_build_symbol",
                    name=name,
                    build_symbol=symbol,
                    source=rel_to_root(source),
                )
            )
        elif c_count > 1:
            errors.append(
                error(
                    "duplicate_build_symbol",
                    name=name,
                    build_symbol=symbol,
                    count=c_count,
                    source=rel_to_root(source),
                )
            )
        if h_count > 1:
            errors.append(
                error(
                    "duplicate_build_symbol_header",
                    name=name,
                    build_symbol=symbol,
                    count=h_count,
                    header=rel_to_root(header),
                )
            )

        # Explicit debt signal: old void tinyui_demo_<name>(void) entry.
        old_hits = OLD_VOID_ENTRY_RE.findall(strip_c_noise(c_text + "\n" + h_text))
        for old_name in sorted(set(old_hits)):
            if old_name.endswith("_frame"):
                errors.append(
                    error(
                        "legacy_frame_entry",
                        name=name,
                        symbol=old_name,
                        source=rel_to_root(source),
                    )
                )
            elif old_name.startswith("tinyui_demo_"):
                errors.append(
                    error(
                        "legacy_void_entry",
                        name=name,
                        symbol=old_name,
                        expected=symbol,
                        source=rel_to_root(source),
                    )
                )
    return errors


def check_cmake_sources(
    manifest: dict[str, Any],
    cmake_text: str,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    if "add_tinyui_demo(tinyui_demo" not in cmake_text:
        errors.append(error("missing_tinyui_demo_target", path=str(SDL_CMAKE)))
    for entry in manifest.get("demos") or []:
        if not isinstance(entry, dict):
            continue
        name = entry.get("name")
        if not isinstance(name, str):
            continue
        fragment = f"{name}/{name}.c"
        if fragment not in cmake_text:
            errors.append(
                error(
                    "cmake_missing_demo_source",
                    name=name,
                    fragment=fragment,
                    path=str(SDL_CMAKE.relative_to(ROOT)),
                )
            )
    # Misspelled legacy path must not remain once migration completes; flag if present
    # while the canonical name is required by the manifest.
    if "scroll_selecter_basic/scroll_selecter_basic.c" in cmake_text:
        errors.append(
            error(
                "cmake_legacy_scroll_selecter_path",
                path=str(SDL_CMAKE.relative_to(ROOT)),
            )
        )
    return errors


def scan_forbidden_tokens(
    rel_path: str,
    text: str,
    patterns: list[tuple[str, re.Pattern[str]]] | None = None,
) -> list[dict[str, Any]]:
    patterns = patterns if patterns is not None else FORBIDDEN_PATTERNS
    errors: list[dict[str, Any]] = []
    cleaned = strip_c_noise(text)
    for label, pattern in patterns:
        for match in pattern.finditer(cleaned):
            errors.append(
                error(
                    "forbidden_token",
                    path=rel_path,
                    token=match.group(0),
                    rule=label,
                )
            )
    return errors


def scan_platform_includes(rel_path: str, text: str) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    # Includes are not inside strings usually; scan raw text line-oriented after comment strip.
    cleaned = BLOCK_COMMENT_RE.sub("", text)
    cleaned = LINE_COMMENT_RE.sub("", cleaned)
    for match in INCLUDE_RE.finditer(cleaned):
        header = match.group(2)
        base = header.split("/")[-1]
        if header in PLATFORM_HEADERS or base in PLATFORM_HEADERS:
            errors.append(
                error(
                    "forbidden_platform_include",
                    path=rel_path,
                    header=header,
                )
            )
    return errors


def check_demo_sources_canonical(
    manifest: dict[str, Any],
    actual: dict[str, Path],
    root: Path = ROOT,
) -> list[dict[str, Any]]:
    """Scan demo sources for non-canonical leaks.

    - Registry + every on-disk demo: forbidden legacy tokens.
    - Manifest consumer demos only: builder lifecycle bans + platform includes.
      ``v23_*`` L5 evidence runners keep their own host lifecycle for now and
      are not part of the 29-entry consumer builder contract.
    """
    errors: list[dict[str, Any]] = []
    # Registry always scanned for legacy/frame leaks.
    for path in REGISTRY_SOURCES:
        if not path.is_file():
            errors.append(error("missing_registry_source", path=rel_to_root(path, root)))
            continue
        rel = rel_to_root(path, root)
        text = path.read_text(encoding="utf-8", errors="replace")
        errors.extend(scan_forbidden_tokens(rel, text))

    for name, source in sorted(actual.items()):
        enforce_builder_rules = is_manifest_demo_name(name)
        for path in (source, source.with_suffix(".h")):
            if not path.is_file():
                continue
            rel = rel_to_root(path, root)
            text = path.read_text(encoding="utf-8", errors="replace")
            errors.extend(scan_forbidden_tokens(rel, text))
            if enforce_builder_rules:
                errors.extend(scan_forbidden_tokens(rel, text, BUILDER_LIFECYCLE_PATTERNS))
                errors.extend(scan_platform_includes(rel, text))
    return errors


# Task 2 registry/runner contract: build(screen) only, no frame hook, no
# display-config backend type in the public dispatcher API.
BUILD_CB_TYPEDEF_RE = re.compile(
    r"typedef\s+tinyui_result_t\s*\(\s*\*\s*tinyui_demo_build_cb_t\s*\)\s*"
    r"\(\s*tinyui_obj_t\s*\*\s*\w*\s*\)\s*;"
)
VOID_ENTRY_CB_TYPEDEF_RE = re.compile(
    r"typedef\s+void\s*\(\s*\*\s*(?:demo_method_cb|tinyui_demo_entry_cb_t|"
    r"tinyui_demo_void_cb_t)\s*\)\s*\(\s*void\s*\)\s*;"
)
FRAME_CB_TYPEDEF_RE = re.compile(
    r"typedef\s+void\s*\(\s*\*\s*tinyui_demo_frame_cb_t\s*\)\s*"
    r"\(\s*unsigned\s+int\s+\w*\s*\)\s*;"
)
REGISTRY_ENTRY_BUILD_FIELD_RE = re.compile(r"\btinyui_demo_build_cb_t\s+build\b")
REGISTRY_ENTRY_FRAME_FIELD_RE = re.compile(r"\btinyui_demo_frame_cb_t\s+frame_cb\b")
LEGACY_DISPATCH_SYMBOLS = (
    "tinyui_demos_create",
    "tinyui_demos_get_display_config",
    "tinyui_demos_frame",
    "tinyui_demo_frame_cb_t",
)
CANONICAL_DISPATCH_SYMBOLS = (
    "tinyui_demos_build",
    "tinyui_demos_get_display_size",
    "tinyui_demos_show_help",
    "tinyui_demo_build_cb_t",
)


def check_registry_contract(
    root: Path = ROOT,
    demos_h: Path | None = None,
    demos_c: Path | None = None,
    runner_main: Path | None = None,
) -> list[dict[str, Any]]:
    """Assert registry/runner expose only the Task 2 build(screen) contract."""
    errors: list[dict[str, Any]] = []
    header = demos_h if demos_h is not None else DEMO_DIR / "tinyui_demos.h"
    source = demos_c if demos_c is not None else DEMO_DIR / "tinyui_demos.c"
    main_c = runner_main if runner_main is not None else root / "tinyui_demo" / "main.c"

    if not header.is_file():
        errors.append(error("missing_registry_source", path=rel_to_root(header, root)))
        return errors
    if not source.is_file():
        errors.append(error("missing_registry_source", path=rel_to_root(source, root)))
        return errors

    h_text = header.read_text(encoding="utf-8", errors="replace")
    c_text = source.read_text(encoding="utf-8", errors="replace")
    h_clean = strip_c_noise(h_text)
    c_clean = strip_c_noise(c_text)
    combined = h_clean + "\n" + c_clean
    h_rel = rel_to_root(header, root)
    c_rel = rel_to_root(source, root)

    if not BUILD_CB_TYPEDEF_RE.search(h_clean):
        errors.append(
            error(
                "missing_build_cb_typedef",
                path=h_rel,
                expected="typedef tinyui_result_t (*tinyui_demo_build_cb_t)(tinyui_obj_t *screen);",
            )
        )

    if FRAME_CB_TYPEDEF_RE.search(combined) or "tinyui_demo_frame_cb_t" in combined:
        errors.append(
            error(
                "legacy_frame_cb_typedef",
                path=h_rel if "tinyui_demo_frame_cb_t" in h_clean else c_rel,
            )
        )

    if VOID_ENTRY_CB_TYPEDEF_RE.search(combined):
        errors.append(error("legacy_void_registry_callback", path=c_rel))

    if REGISTRY_ENTRY_FRAME_FIELD_RE.search(c_clean) or re.search(
        r"\bframe_cb\b", c_clean
    ):
        errors.append(error("registry_has_frame_cb_field", path=c_rel))

    if not REGISTRY_ENTRY_BUILD_FIELD_RE.search(c_clean):
        errors.append(
            error(
                "registry_missing_build_field",
                path=c_rel,
                expected="tinyui_demo_build_cb_t build",
            )
        )

    # Public dispatcher must not re-export backend display_config plumbing.
    if re.search(r"\btinyui_demos_get_display_config\b", combined):
        errors.append(error("legacy_display_config_dispatch", path=h_rel))
    if re.search(r"\bstruct\s+tinyui_display_config\b", h_clean):
        errors.append(error("registry_exposes_display_config_type", path=h_rel))
    if re.search(r"\btinyui_demos_get_display_size\b", h_clean) is None:
        errors.append(error("missing_display_size_dispatch", path=h_rel))
    if re.search(r"\btinyui_demos_build\b", h_clean) is None:
        errors.append(error("missing_demos_build_dispatch", path=h_rel))

    for symbol in LEGACY_DISPATCH_SYMBOLS:
        if re.search(rf"\b{re.escape(symbol)}\b", combined):
            errors.append(
                error(
                    "legacy_registry_dispatch_symbol",
                    path=h_rel if symbol in h_clean else c_rel,
                    symbol=symbol,
                )
            )

    for symbol in CANONICAL_DISPATCH_SYMBOLS:
        if re.search(rf"\b{re.escape(symbol)}\b", h_clean) is None:
            errors.append(
                error(
                    "missing_canonical_registry_symbol",
                    path=h_rel,
                    symbol=symbol,
                )
            )

    # Registry names must use corrected scroll_selector spelling.
    # Search raw source: strip_c_noise() blanks string literal contents.
    if re.search(r'"scroll_selecter_basic"', c_text):
        errors.append(error("registry_legacy_scroll_selecter_name", path=c_rel))
    if re.search(r'"scroll_selector_basic"', c_text) is None:
        errors.append(error("registry_missing_scroll_selector_name", path=c_rel))

    if main_c.is_file():
        main_clean = strip_c_noise(main_c.read_text(encoding="utf-8", errors="replace"))
        main_rel = rel_to_root(main_c, root)
        for symbol in (
            "tinyui_demos_create",
            "tinyui_demos_frame",
            "tinyui_demos_get_display_config",
            "tinyui_timer_handler",
        ):
            if re.search(rf"\b{re.escape(symbol)}\b", main_clean):
                errors.append(
                    error(
                        "runner_uses_legacy_dispatch",
                        path=main_rel,
                        symbol=symbol,
                    )
                )
        for symbol in (
            "tinyui_demos_build",
            "tinyui_demos_get_display_size",
            "tinyui_process",
            "tinyui_screen_create",
            "tinyui_screen_load",
        ):
            if re.search(rf"\b{re.escape(symbol)}\b", main_clean) is None:
                errors.append(
                    error(
                        "runner_missing_canonical_symbol",
                        path=main_rel,
                        symbol=symbol,
                    )
                )
    else:
        errors.append(error("missing_runner_main", path=rel_to_root(main_c, root)))

    return errors


def check_all(
    root: Path = ROOT,
    manifest_path: Path = DEFAULT_MANIFEST,
    cmake_path: Path = SDL_CMAKE,
    demo_dir: Path = DEMO_DIR,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    manifest, manifest_errors = load_manifest(manifest_path)
    errors.extend(manifest_errors)
    if not manifest or not isinstance(manifest.get("demos"), list):
        return errors

    actual = collect_actual_demo_sources(demo_dir, manifest_only=True)
    all_on_disk = collect_actual_demo_sources(demo_dir, manifest_only=False)
    errors.extend(check_source_set(manifest, actual, root=root))
    errors.extend(check_build_symbols(manifest, actual))

    if not cmake_path.is_file():
        errors.append(error("missing_cmake", path=str(cmake_path)))
    else:
        cmake_text = cmake_path.read_text(encoding="utf-8", errors="replace")
        errors.extend(check_cmake_sources(manifest, cmake_text))

    # Scan every on-disk demo (including non-manifest L5 runners) + registry.
    errors.extend(check_demo_sources_canonical(manifest, all_on_disk, root=root))
    errors.extend(check_registry_contract(root=root))
    return errors


def main(argv: list[str] | None = None) -> int:
    del argv  # reserved for future flags
    errors = check_all()
    if not errors:
        print("check_tinyui_demo_boundary: OK")
        return 0

    print(f"check_tinyui_demo_boundary: FAIL ({len(errors)} issue(s))", file=sys.stderr)
    # Stable, readable debt listing for Task 1 red gate.
    for item in errors:
        print(format_error(item), file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
