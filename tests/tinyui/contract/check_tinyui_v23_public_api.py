from __future__ import annotations
#!/usr/bin/env python3
"""Validate the TinyUI v2.3 canonical public API manifest."""

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_INCLUDE = ROOT / "tinyui" / "include"
CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
DEFAULT_MANIFEST = CONTRACT_DIR / "tinyui_v23_public_api.json"
SCHEMA_VERSION = "tinyui-v2.3-public-api-v1"
ENTRY_FIELDS = {"kind", "name", "header", "signature", "availability"}
ENTRY_KINDS = {"function", "typedef", "type", "macro"}
FEATURE_RE = re.compile(r"TINYUI_ENABLE_[A-Z0-9_]+\Z")
HASH_RE = re.compile(r"[0-9a-f]{64}\Z")
IDENTIFIER_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
TINYUI_IDENTIFIER_RE = re.compile(r"\btinyui_[A-Za-z0-9_]*\b")
TYPEDEF_CALLBACK_RE = re.compile(
    r"\(\s*\*\s*(?P<name>tinyui_[A-Za-z0-9_]*)\s*\)"
)
TYPEDEF_NAME_RE = re.compile(r"\b(?P<name>tinyui_[A-Za-z0-9_]*)\s*\Z")
OPAQUE_TYPE_RE = re.compile(r"^(?P<kind>struct|enum)\s+tinyui_[A-Za-z0-9_]+\s*;")

# These are intentionally checked from raw public headers, including headers
# that should disappear from the canonical install tree in later M1 tasks.
LEGACY_PREFIXES = (
    "tinyui_app_",
    "tinyui_timer_handler",
    "tinyui_widget_",
    "tinyui_theme_create",
    "tinyui_theme_destroy",
    "tinyui_theme_apply_to_widget",
    "tinyui_app_set_theme",
)
MISSPELLED_PREFIXES = (
    "tinyui_tabel_",
    "tinyui_scroll_selecter_",
    "tinyui_q_r_code_",
)
MISSPELLED_NAMES = {"tinyui_button_set_press", "tinyui_button_get_press"}
NATIVE_PREFIXES = (
    "tinyui_native_",
    "ld",
    "arm_2d_",
    "SIGNAL_",
)
FORBIDDEN_INIT_RE = re.compile(r"\btinyui_[A-Za-z0-9_]*_init\b")

CANONICAL_AGGREGATE_PREFIXES = (
    "tinyui_config.h",
    "core/result.h",
    "core/obj.h",
    "core/runtime.h",
    "core/event.h",
    "core/timer.h",
    "core/focus.h",
    "style/",
    "theme/",
    "layout/",
    "resource/",
    "widgets/",
)


def _m0_parser_helpers():
    """Load M0 normalization helpers without changing the M0 checker."""
    if str(CONTRACT_DIR) not in sys.path:
        sys.path.insert(0, str(CONTRACT_DIR))
    from check_ldgui_public_api_inventory import _normalize_declaration, _strip_comments

    return _normalize_declaration, _strip_comments


def _normalize_declaration(statement: str) -> str:
    normalize, _ = _m0_parser_helpers()
    return normalize(statement)


def _strip_comments(text: str) -> str:
    _, strip_comments = _m0_parser_helpers()
    return strip_comments(text)


def _normalize_signature(statement: str) -> str:
    """Collapse whitespace while preserving braces for enum/struct typedefs."""
    collapsed = " ".join(statement.replace("\n", " ").split())
    # C allows a trailing comma before '}' in enums/structs; treat it as noise.
    return re.sub(r",\s*}", " }", collapsed)


def _iter_top_level_statements(text: str) -> list[str]:
    """Split declarations on top-level semicolons, keeping braced bodies intact."""
    statements: list[str] = []
    buffer: list[str] = []
    depth = 0
    for char in text:
        if char == "{":
            depth += 1
            buffer.append(char)
            continue
        if char == "}":
            depth = max(0, depth - 1)
            buffer.append(char)
            continue
        if char == ";" and depth == 0:
            statement = "".join(buffer).strip()
            if statement:
                statements.append(statement)
            buffer = []
            continue
        buffer.append(char)
    trailing = "".join(buffer).strip()
    if trailing:
        statements.append(trailing)
    return statements


def _is_include_guard_macro(header: str, name: str) -> bool:
    if name == _include_guard(header):
        return True
    # Older/simple headers may still use the stem-only guard form.
    stem = Path(header).stem.upper().replace(".", "_")
    if name == f"TINYUI_{stem}_H":
        return True
    # Compatibility re-entry guards used by opaque typedef shims.
    return name.endswith("_DEFINED")


def _error(code: str, **details: object) -> dict:
    return {"code": code, **details}


def _manifest_digest(value: dict) -> str:
    payload = {
        "schema_version": value["schema_version"],
        "entries": value["entries"],
    }
    encoded = json.dumps(
        payload,
        ensure_ascii=True,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def _include_guard(header: str) -> str:
    if header == "tinyui.h":
        return "TINYUI_H"
    # Nested headers may use either the stem-only or full-relative guard form.
    # Prefer the relative-path form for matching actual public headers.
    relative = header.replace("\\", "/").removesuffix(".h")
    parts = [part for part in relative.split("/") if part]
    return "TINYUI_" + "_".join(part.upper().replace(".", "_") for part in parts) + "_H"


def _scan_header(header: str, text: str) -> list[dict]:
    """Scan direct declarations for the v2.3 public API surface."""
    comment_free = _strip_comments(text)
    declaration_lines: list[str] = []
    rows: list[dict] = []
    lines = comment_free.splitlines()
    index = 0
    while index < len(lines):
        line = lines[index]
        stripped = line.lstrip()
        if stripped.startswith("#"):
            macro_lines = [line]
            while macro_lines[-1].rstrip().endswith("\\") and index + 1 < len(lines):
                index += 1
                macro_lines.append(lines[index])
            macro = " ".join(part.rstrip().removesuffix("\\") for part in macro_lines)
            match = re.match(
                r"^\s*#define\s+(?P<name>TINYUI_[A-Za-z0-9_]*)(?:\(|\s+|$)(?P<body>.*)$",
                macro,
            )
            if match and not _is_include_guard_macro(header, match.group("name")):
                rows.append(
                    {
                        "kind": "macro",
                        "name": match.group("name"),
                        "header": header,
                        "signature": _normalize_signature(macro),
                    }
                )
            index += 1
            continue
        declaration_lines.append(line)
        index += 1

    for statement in _iter_top_level_statements("\n".join(declaration_lines)):
        signature = _normalize_signature(statement)
        if not signature:
            continue
        signature_with_semicolon = signature + ";"
        # Function-pointer typedefs need the brace-free form for name extraction.
        brace_free = _normalize_declaration(statement)
        callback = TYPEDEF_CALLBACK_RE.search(brace_free)
        if signature.startswith("typedef") and callback:
            rows.append(
                {
                    "kind": "typedef",
                    "name": callback.group("name"),
                    "header": header,
                    "signature": signature_with_semicolon,
                }
            )
            continue
        if signature.startswith("typedef"):
            match = TYPEDEF_NAME_RE.search(brace_free)
            if match:
                rows.append(
                    {
                        "kind": "typedef",
                        "name": match.group("name"),
                        "header": header,
                        "signature": signature_with_semicolon,
                    }
                )
            continue
        opaque_type = OPAQUE_TYPE_RE.match(brace_free)
        if opaque_type:
            name = brace_free.split()[1]
            rows.append(
                {
                    "kind": "type",
                    "name": name,
                    "header": header,
                    "signature": signature_with_semicolon,
                }
            )
            continue
        if "(" not in brace_free:
            continue
        function_name = _function_name(brace_free)
        if function_name:
            rows.append(
                {
                    "kind": "function",
                    "name": function_name,
                    "header": header,
                    "signature": signature_with_semicolon,
                }
            )

    unique = {(row["kind"], row["name"], row["header"]): row for row in rows}
    return sorted(unique.values(), key=lambda row: (row["header"], row["kind"], row["name"]))


def _function_name(normalized: str) -> str | None:
    """Extract a declaration name after M0 normalization, including nested params."""
    end = len(normalized) - 1
    while end >= 0 and normalized[end].isspace():
        end -= 1
    if end < 0 or normalized[end] != ")":
        return None
    depth = 0
    opening = -1
    for index in range(end, -1, -1):
        if normalized[index] == ")":
            depth += 1
        elif normalized[index] == "(":
            depth -= 1
            if depth == 0:
                opening = index
                break
    if opening < 1:
        return None
    match = re.search(r"(?P<name>tinyui_[A-Za-z0-9_]*)\s*$", normalized[:opening])
    return match.group("name") if match else None


def scan_public_headers(root: Path) -> tuple[list[dict], list[dict]]:
    include_dir = root / "tinyui" / "include"
    if not include_dir.is_dir():
        return [], [_error("missing_public_header_root", path="tinyui/include")]
    rows: list[dict] = []
    errors: list[dict] = []
    # integration/ is opt-in and not aggregated by tinyui.h, but Task 3 registered
    # its explicit key surface in the M1 manifest, so it must still be scanned.
    skip_prefixes = (
        "internal/",
        "extensions/",
        "display/",
        "indev/",
        "tick/",
        "osal/",
        "port/",
    )
    for header in sorted(include_dir.rglob("*.h")):
        relative = header.relative_to(include_dir).as_posix()
        if any(relative.startswith(prefix) for prefix in skip_prefixes):
            continue
        text = header.read_text(encoding="utf-8", errors="replace")
        # Widget headers: still scan for forbidden legacy/typos, but registration
        # against the core-only M1 manifest is deferred to later tasks.
        if relative.startswith("widgets/") or relative == "core/nav.h":
            errors.extend(_scan_forbidden_symbols(relative, text))
            continue
        rows.extend(_scan_header(relative, text))
        errors.extend(_scan_forbidden_symbols(relative, text))
        if relative == "tinyui.h":
            errors.extend(_scan_aggregate_includes(text))
    return rows, errors


def _scan_forbidden_symbols(header: str, text: str) -> list[dict]:
    errors: list[dict] = []
    if header == "resource/image_source.h":
        if re.search(r"\bvoid\s*\*", _strip_comments(text)):
            errors.append(_error("forbidden_image_source_void_pointer", header=header))
        for name in ("img_tile", "mask_tile"):
            if re.search(rf"\b{re.escape(name)}\b", _strip_comments(text)):
                errors.append(
                    _error("forbidden_image_source_legacy_field", header=header, name=name)
                )
    for identifier in sorted(set(TINYUI_IDENTIFIER_RE.findall(_strip_comments(text)))):
        if identifier == "tinyui_init":
            continue
        if identifier in MISSPELLED_NAMES or any(
            identifier.startswith(prefix) for prefix in MISSPELLED_PREFIXES
        ):
            errors.append(_error("forbidden_misspelled_symbol", header=header, name=identifier))
            continue
        if any(identifier.startswith(prefix) for prefix in NATIVE_PREFIXES):
            errors.append(_error("forbidden_native_symbol", header=header, name=identifier))
            continue
        if any(identifier.startswith(prefix) for prefix in LEGACY_PREFIXES) or FORBIDDEN_INIT_RE.fullmatch(identifier):
            errors.append(_error("forbidden_legacy_symbol", header=header, name=identifier))
    # Native identifiers do not all have the tinyui_ prefix.
    for identifier in sorted(set(IDENTIFIER_RE.findall(_strip_comments(text)))):
        if identifier.startswith(("ld", "arm_2d_", "SIGNAL_")):
            errors.append(_error("forbidden_native_symbol", header=header, name=identifier))
    return errors


def _scan_aggregate_includes(text: str) -> list[dict]:
    errors: list[dict] = []
    for match in re.finditer(r"^\s*#include\s+[\"<]([^\">]+)[\">]", text, re.MULTILINE):
        include = match.group(1)
        if not any(include.startswith(prefix) for prefix in CANONICAL_AGGREGATE_PREFIXES):
            errors.append(_error("forbidden_aggregate_include", header="tinyui.h", include=include))
    return errors


def _load_manifest(path: Path, *, validate_hash: bool = True) -> tuple[dict | None, list[dict]]:
    if not path.is_file():
        return None, [_error("missing_manifest", path=str(path))]
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return None, [_error("invalid_manifest_json", path=str(path), message=str(exc))]
    if not isinstance(value, dict):
        return None, [_error("invalid_manifest_root", path=str(path))]
    if value.get("schema_version") != SCHEMA_VERSION:
        return None, [_error("invalid_manifest_schema", expected=SCHEMA_VERSION)]
    if set(value) - {"schema_version", "entries", "manifest_sha256"} or not isinstance(
        value.get("entries"), list
    ):
        return None, [_error("invalid_manifest_schema", message="expected schema_version and entries")]

    errors: list[dict] = []
    manifest_sha256 = value.get("manifest_sha256")
    if manifest_sha256 is not None and validate_hash:
        if not isinstance(manifest_sha256, str) or HASH_RE.fullmatch(manifest_sha256) is None:
            errors.append(_error("invalid_manifest_hash"))
        elif manifest_sha256 != _manifest_digest(value):
            errors.append(
                _error(
                    "manifest_hash_mismatch",
                    expected=_manifest_digest(value),
                    actual=manifest_sha256,
                )
            )
    seen: set[tuple[str, str]] = set()
    for index, entry in enumerate(value["entries"]):
        if not isinstance(entry, dict) or set(entry) != ENTRY_FIELDS:
            errors.append(_error("invalid_manifest_entry", index=index))
            continue
        kind = entry.get("kind")
        name = entry.get("name")
        header = entry.get("header")
        signature = entry.get("signature")
        availability = entry.get("availability")
        if kind not in ENTRY_KINDS or not all(
            isinstance(value, str) and value for value in (name, header, signature, availability)
        ):
            errors.append(_error("invalid_manifest_entry", index=index))
            continue
        if availability != "always" and FEATURE_RE.fullmatch(availability) is None:
            errors.append(_error("invalid_availability", index=index, value=availability))
        key = (kind, name)
        if key in seen:
            errors.append(_error("duplicate_manifest_symbol", kind=kind, name=name))
        seen.add(key)
    return value, errors


def _compare_entries(manifest: dict, scanned: list[dict], root: Path) -> list[dict]:
    errors: list[dict] = []
    entries = manifest.get("entries", [])
    manifest_rows = {(entry["kind"], entry["name"]): entry for entry in entries if isinstance(entry, dict)}
    scanned_rows = {(row["kind"], row["name"]): row for row in scanned}
    scanned_by_symbol: dict[tuple[str, str], list[dict]] = {}
    for row in scanned:
        scanned_by_symbol.setdefault((row["kind"], row["name"]), []).append(row)
    for key, rows in sorted(scanned_by_symbol.items()):
        if len(rows) > 1:
            errors.append(
                _error(
                    "duplicate_public_symbol",
                    kind=key[0],
                    name=key[1],
                    headers=sorted(row["header"] for row in rows),
                )
            )
    for entry in entries:
        if not isinstance(entry, dict) or not ENTRY_FIELDS <= set(entry):
            continue
        header_path = root / "tinyui" / "include" / entry["header"]
        if not header_path.is_file():
            errors.append(_error("missing_header", header=entry["header"], name=entry["name"]))
    for key in sorted(manifest_rows.keys() - scanned_rows.keys()):
        entry = manifest_rows[key]
        errors.append(_error("missing_symbol", kind=key[0], name=key[1], header=entry["header"]))
    for key in sorted(scanned_rows.keys() - manifest_rows.keys()):
        row = scanned_rows[key]
        errors.append(_error("unregistered_symbol", kind=key[0], name=key[1], header=row["header"]))
    for key in sorted(manifest_rows.keys() & scanned_rows.keys()):
        expected = manifest_rows[key]
        actual = scanned_rows[key]
        if expected["header"] != actual["header"]:
            errors.append(
                _error(
                    "header_mismatch",
                    kind=key[0],
                    name=key[1],
                    expected=expected["header"],
                    actual=actual["header"],
                )
            )
        if _normalize_signature(expected["signature"]) != _normalize_signature(actual["signature"]):
            errors.append(
                _error(
                    "signature_mismatch",
                    kind=key[0],
                    name=key[1],
                    expected=expected["signature"],
                    actual=actual["signature"],
                )
            )
    return errors


def check_contract(root: Path, manifest_path: Path) -> list[dict]:
    manifest, errors = _load_manifest(manifest_path)
    if manifest is None:
        return errors
    scanned, scan_errors = scan_public_headers(root)
    errors.extend(scan_errors)
    errors.extend(_compare_entries(manifest, scanned, root))
    return sorted(errors, key=lambda error: (error["code"], str(error)))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--check", type=Path, dest="check_manifest")
    parser.add_argument("--write-hash", type=Path, dest="write_hash")
    arguments = parser.parse_args(argv)
    if arguments.write_hash is not None:
        path = arguments.write_hash.resolve()
        manifest, errors = _load_manifest(path, validate_hash=False)
        if manifest is None or errors:
            for error in errors:
                print(json.dumps(error, sort_keys=True))
            return 1
        manifest["manifest_sha256"] = _manifest_digest(manifest)
        path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        print(f"TINYUI_V23_PUBLIC_API_HASH {manifest['manifest_sha256']} {path}")
        return 0
    manifest = arguments.check_manifest or arguments.manifest
    errors = check_contract(arguments.root.resolve(), manifest.resolve())
    if errors:
        for error in errors:
            print(json.dumps(error, sort_keys=True))
        return 1
    print(f"TINYUI_V23_PUBLIC_API_OK {manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
