#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import argparse
import hashlib
import re
import json
import subprocess
import sys
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "tinyui" / "include"
LEGACY_PUBLIC_DIR = ROOT / "tinyui" / "include" / "tinyui"
CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
INVENTORY_JSON = CONTRACT_DIR / "ldgui_public_api_inventory.json"
LEDGER_JSON = CONTRACT_DIR / "native_api_gap_ledger.json"
DEFAULT_ABI_MANIFEST = CONTRACT_DIR / "tinyui_v23_abi_manifest.json"
ABI_SCHEMA_VERSION = "tinyui-v2.3-abi-manifest-v1"
ABI_EXPORT_TARGETS = ("tinyui",)

LEGACY_RUNTIME_PROBE = """\
#include "core/runtime.h"
int main(void) { return tinyui_init() != 0 ? tinyui_init() : 0; }
"""
ALLOWED_FUNCTION_PREFIX = "tinyui_"
ALLOWED_MACRO_PREFIX = "TINYUI_"
ALLOWED_TYPE_PREFIX = "tinyui_"
ALLOWED_COMPAT_TINYUI_FUNCTIONS = {
    "tinyui_init",
    "tinyui_deinit",
    "tinyui_timer_handler",
    "tinyui_screen_create",
    "tinyui_screen_load",
    "tinyui_label_create",
    "tinyui_label_set_text",
    "tinyui_label_get_text",
    "tinyui_label_set_align",
    "tinyui_button_create",
    "tinyui_button_set_text",
    "tinyui_button_set_on_clicked",
    "tinyui_button_set_on_pressed",
    "tinyui_button_set_on_released",
    "tinyui_switch_create",
    "tinyui_switch_set_checked",
    "tinyui_switch_is_checked",
    "tinyui_switch_set_on_toggled",
}
ALLOWED_COMPAT_TINYUI_MACROS = {
}
ALLOWED_COMPAT_TINYUI_TYPES = {
    "tinyui_obj_t",
}
VALID_COVERAGE_KINDS = {
    "native_setter_parity",
    "native_getter_parity",
    "init_parameter_parity",
    "direct_field_parity",
    "macro_alias_parity",
    "lifecycle_internal_allowlisted",
    "non_widget_allowlisted",
    "shared_api_equivalence",
}
ALLOWLISTED_COVERAGE_KINDS = {
    "lifecycle_internal_allowlisted",
    "non_widget_allowlisted",
}
VALID_GAP_STATUSES = {
    "covered",
    "missing_tinyui_api",
    "missing_runtime_visible_evidence",
    "missing_unit",
    "missing_gate",
    "overwrapped",
    "allowlisted",
}

# M3 Task 8: misspelled / legacy public fragments (canonical only on public surface).
FORBIDDEN_PUBLIC_FRAGMENTS = (
    "scroll_selecter",
    "tabel",
    "q_r_code",
    "set_press(",
    "tinyui_app_create",
    "tinyui_app_destroy",
    "legacy_app",
)

STRUCT_RE = re.compile(r"\bstruct\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)")
ENUM_RE = re.compile(r"\benum\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)")
TYPEDEF_CB_RE = re.compile(r"typedef\s+.+\(\*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\)\s*\(")
LD_IDENTIFIER_RE = re.compile(r"\bld[A-Za-z0-9_]*\b")
ARM_IDENTIFIER_RE = re.compile(r"\barm_2d_[A-Za-z0-9_]*\b")
SIGNAL_IDENTIFIER_RE = re.compile(r"\bSIGNAL_[A-Za-z0-9_]*\b")
IDENTIFIER_AT_END_RE = re.compile(r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*$")
OPAQUE_ARM_TYPEDEF_RE = re.compile(
    r"typedef\s+struct\s+arm_2d_[A-Za-z0-9_]*\s+arm_2d_[A-Za-z0-9_]*\s*;"
)
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//.*?$", re.MULTILINE)
INCLUDE_FORWARD_RE = re.compile(r'^\s*#include\s+"(?P<target>tinyui/[^"]+)"\s*$', re.M)


def _allowed_include_guard(header: Path) -> str:
    stem = header.stem.upper().replace(".", "_")
    return f"TINYUI_{stem}_H"


def strip_c_comments(text: str) -> str:
    text = BLOCK_COMMENT_RE.sub("", text)
    return LINE_COMMENT_RE.sub("", text)


def resolve_public_header_text(header: Path) -> str:
    text = header.read_text(encoding="utf-8")
    match = INCLUDE_FORWARD_RE.search(text)
    if match is not None:
        forwarded = ROOT / "tinyui" / "include" / match.group("target")
        return forwarded.read_text(encoding="utf-8")
    return text


def assert_allowed_prefix(name: str, *, header: Path, kind: str, prefix: str) -> None:
    assert name.startswith(prefix), (
        f"{header.name} exports {kind} '{name}' outside allowed prefix '{prefix}'"
    )


def assert_allowed_public_symbol(name: str, *, header: Path, kind: str) -> None:
    if kind == "function" and name in ALLOWED_COMPAT_TINYUI_FUNCTIONS:
        return
    if kind == "macro" and name in ALLOWED_COMPAT_TINYUI_MACROS:
        return
    if kind == "macro" and name == _allowed_include_guard(header):
        return
    if kind in {"struct tag", "enum tag", "typedef callback"} and name in ALLOWED_COMPAT_TINYUI_TYPES:
        return
    prefix = ALLOWED_MACRO_PREFIX if kind == "macro" else (
        ALLOWED_TYPE_PREFIX if kind in {"struct tag", "enum tag", "typedef callback"} else ALLOWED_FUNCTION_PREFIX
    )
    assert_allowed_prefix(name, header=header, kind=kind, prefix=prefix)


def check_macro_prefixes(header: Path, text: str) -> None:
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped.startswith("#"):
            continue
        parts = stripped.split()
        if len(parts) < 2 or parts[0] not in {"#define", "#ifndef"}:
            continue
        assert_allowed_public_symbol(parts[1], header=header, kind="macro")


def check_type_prefixes(header: Path, text: str) -> None:
    for match in STRUCT_RE.finditer(text):
        assert_allowed_public_symbol(match.group("name"), header=header, kind="struct tag")
    for match in ENUM_RE.finditer(text):
        assert_allowed_public_symbol(match.group("name"), header=header, kind="enum tag")
    for match in TYPEDEF_CB_RE.finditer(text):
        assert_allowed_public_symbol(match.group("name"), header=header, kind="typedef callback")


def check_function_prefixes(header: Path, text: str) -> None:
    for statement in strip_c_comments(text).split(";"):
        normalized = " ".join(statement.split())
        if not normalized or normalized.startswith("#") or normalized.startswith("typedef"):
            continue
        if "(" not in normalized:
            continue
        declaration_head = normalized.split("(", 1)[0].strip()
        if declaration_head.endswith(")") or "(*" in declaration_head:
            continue
        match = IDENTIFIER_AT_END_RE.search(declaration_head)
        assert match is not None, f"{header.name} has unrecognized function declaration: {normalized}"
        assert_allowed_public_symbol(match.group("name"), header=header, kind="function")


def check_forbidden_identifiers(header: Path, text: str) -> None:
    sanitized = OPAQUE_ARM_TYPEDEF_RE.sub("", strip_c_comments(text))
    forbidden_patterns = (
        ("ld*", LD_IDENTIFIER_RE),
        ("arm_2d_*", ARM_IDENTIFIER_RE),
        ("SIGNAL_*", SIGNAL_IDENTIFIER_RE),
    )
    for label, pattern in forbidden_patterns:
        match = pattern.search(sanitized)
        assert match is None, f"{header.name} leaks forbidden identifier '{match.group(0)}' ({label})"


def check_forbidden_fragments(header: Path, text: str) -> None:
    sanitized = strip_c_comments(text)
    for fragment in FORBIDDEN_PUBLIC_FRAGMENTS:
        assert fragment not in sanitized, (
            f"{header.relative_to(ROOT)} contains forbidden public fragment '{fragment}'"
        )


def iter_public_headers() -> list[Path]:
    """Canonical public surface: umbrella + core/widgets/layout/style/theme/resource.

    Private/internal headers (include/internal/**) are migration-only and excluded.
    """
    headers: list[Path] = []
    for path in sorted(PUBLIC_DIR.rglob("*.h")):
        rel_parts = path.relative_to(PUBLIC_DIR).parts
        if rel_parts and rel_parts[0] == "internal":
            continue
        headers.append(path)
    return headers


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _inventory_rows_by_symbol(inventory: dict) -> dict[str, dict]:
    """Normalize the current entries inventory and the historical widgets shape."""
    if "entries" in inventory:
        entries = inventory.get("entries")
        assert isinstance(entries, list), "inventory entries must be a list"
        rows: dict[str, dict] = {}
        for row in entries:
            assert isinstance(row, dict), f"inventory entry must be an object: {row!r}"
            symbol = row.get("symbol")
            assert isinstance(symbol, str) and symbol, f"inventory entry missing symbol: {row!r}"
            assert symbol not in rows, f"duplicate inventory symbol: {symbol}"
            rows[symbol] = row
        return rows

    rows = {}
    for widget in inventory.get("widgets", []):
        required_native_apis = widget.get("required_native_apis")
        assert isinstance(required_native_apis, list), (
            f"inventory widget missing rows: {widget.get('name')}"
        )
        for row in required_native_apis:
            assert isinstance(row, dict), f"inventory row must be an object: {row!r}"
            symbol = row.get("ldgui_symbol")
            assert isinstance(symbol, str) and symbol, f"inventory row missing ldgui_symbol: {row!r}"
            assert symbol not in rows, f"duplicate inventory symbol: {symbol}"
            rows[symbol] = row
    return rows


def _ledger_rows_by_symbol() -> dict[str, dict]:
    ledger = _load_json(LEDGER_JSON)
    rows: dict[str, dict] = {}
    for row in ledger.get("rows", []):
        symbol = row.get("ldgui_symbol")
        assert isinstance(symbol, str) and symbol, f"ledger row has invalid ldgui_symbol: {row!r}"
        assert symbol not in rows, f"duplicate ledger row: {symbol}"
        rows[symbol] = row
    return rows


def _assert_inventory_contract_rows() -> None:
    inventory = _load_json(INVENTORY_JSON)
    ledger_by_symbol = _ledger_rows_by_symbol()
    inventory_by_symbol = _inventory_rows_by_symbol(inventory)
    for symbol, row in inventory_by_symbol.items():
        if symbol not in ledger_by_symbol:
            continue
        ledger_row = ledger_by_symbol[symbol]
        required = ledger_row.get("required", row.get("required"))
        coverage_kind = ledger_row.get("coverage_kind")
        gap_status = ledger_row.get("gap_status")
        assert isinstance(required, bool), f"{symbol} missing boolean required"
        assert coverage_kind in VALID_COVERAGE_KINDS, f"{symbol} invalid coverage_kind"
        assert gap_status in VALID_GAP_STATUSES, f"{symbol} invalid gap_status"
        assert ledger_row.get("rationale"), f"{symbol} missing rationale"
        if required:
            assert ledger_row.get("tinyui_api") or coverage_kind == "shared_api_equivalence", (
                f"{symbol} required row must name planned tinyui_api or shared_api_equivalence"
            )
        else:
            assert ledger_row.get("allowlist_reason"), (
                f"{symbol} required=false row must have allowlist_reason"
            )
            assert coverage_kind in ALLOWLISTED_COVERAGE_KINDS, (
                f"{symbol} required=false row must use allowlisted coverage_kind"
            )
    assert set(ledger_by_symbol) <= set(inventory_by_symbol), (
        "native_api_gap_ledger rows must match ldgui_public_api_inventory rows:\n"
        f"missing_from_inventory={sorted(set(ledger_by_symbol) - set(inventory_by_symbol))[:25]}\n"
        f"inventory_only={sorted(set(inventory_by_symbol) - set(ledger_by_symbol))[:25]}"
    )


def check_legacy_runtime_header_compiles() -> None:
    """Verify the current public runtime header compiles."""
    with tempfile.NamedTemporaryFile("w", suffix=".c", encoding="utf-8", delete=False) as probe:
        probe.write(LEGACY_RUNTIME_PROBE)
        probe_path = Path(probe.name)
    try:
        result = subprocess.run(
            [
                "cc",
                "-fsyntax-only",
                "-I", str(PUBLIC_DIR),
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
    assert result.returncode == 0, (
        f"public header `#include \"runtime.h\"` failed to compile:\n"
        + result.stdout + result.stderr
    )


def _load_abi_scan_helpers():
    """Reuse v2.3 public-API declaration parsers without reimplementing them."""
    if str(CONTRACT_DIR) not in sys.path:
        sys.path.insert(0, str(CONTRACT_DIR))
    from check_ldgui_public_api_inventory import _normalize_declaration
    from check_tinyui_v23_public_api import (
        OPAQUE_TYPE_RE,
        TYPEDEF_CALLBACK_RE,
        TYPEDEF_NAME_RE,
        _function_name,
        _is_include_guard_macro,
        _iter_top_level_statements,
        _normalize_signature,
        _strip_comments,
    )

    return {
        "normalize_declaration": _normalize_declaration,
        "opaque_type_re": OPAQUE_TYPE_RE,
        "typedef_callback_re": TYPEDEF_CALLBACK_RE,
        "typedef_name_re": TYPEDEF_NAME_RE,
        "function_name": _function_name,
        "is_include_guard_macro": _is_include_guard_macro,
        "iter_top_level_statements": _iter_top_level_statements,
        "normalize_signature": _normalize_signature,
        "strip_comments": _strip_comments,
    }


def _parse_parameter_types(param_blob: str) -> list[str]:
    text = param_blob.strip()
    if not text or text == "void":
        return []
    parts: list[str] = []
    depth = 0
    current: list[str] = []
    for char in text:
        if char == "(":
            depth += 1
            current.append(char)
        elif char == ")":
            depth = max(0, depth - 1)
            current.append(char)
        elif char == "," and depth == 0:
            parts.append("".join(current).strip())
            current = []
        else:
            current.append(char)
    if current:
        parts.append("".join(current).strip())
    return parts


def _parse_enum_values(signature: str) -> list[dict[str, str]]:
    match = re.search(r"\{(.*)\}", signature)
    if match is None:
        return []
    values: list[dict[str, str]] = []
    auto = 0
    for item in match.group(1).split(","):
        item = item.strip()
        if not item:
            continue
        if "=" in item:
            name, raw_value = item.split("=", 1)
            name = name.strip()
            value = raw_value.strip()
            values.append({"name": name, "value": value})
            try:
                auto = int(value, 0) + 1
            except ValueError:
                auto += 1
        else:
            values.append({"name": item, "value": str(auto)})
            auto += 1
    return values


def _function_return_type(head: str, function_name: str) -> str:
    match = re.search(r"(?P<ret>.+?)\b" + re.escape(function_name) + r"\s*$", head)
    if match is None:
        return head.strip()
    return match.group("ret").strip()


def _abi_sort_key(row: dict[str, Any]) -> tuple[str, str, str]:
    return (
        str(row.get("header", "")),
        str(row.get("name", "")),
        str(row.get("signature", "")),
    )


def build_abi_manifest(*, root: Path = ROOT) -> dict[str, Any]:
    """Build a deterministic ABI snapshot of the installed public include tree."""
    helpers = _load_abi_scan_helpers()
    normalize_declaration = helpers["normalize_declaration"]
    normalize_signature = helpers["normalize_signature"]
    strip_comments = helpers["strip_comments"]
    iter_top_level_statements = helpers["iter_top_level_statements"]
    is_include_guard_macro = helpers["is_include_guard_macro"]
    typedef_callback_re = helpers["typedef_callback_re"]
    typedef_name_re = helpers["typedef_name_re"]
    opaque_type_re = helpers["opaque_type_re"]
    function_name = helpers["function_name"]

    public_dir = root / "tinyui" / "include"
    headers: list[str] = []
    typedefs: list[dict[str, Any]] = []
    opaques: list[dict[str, Any]] = []
    functions: list[dict[str, Any]] = []
    enums: list[dict[str, Any]] = []
    macros: list[dict[str, Any]] = []

    for header in sorted(public_dir.rglob("*.h")):
        relative_parts = header.relative_to(public_dir).parts
        if relative_parts and relative_parts[0] == "internal":
            continue
        rel = header.relative_to(public_dir).as_posix()
        headers.append(rel)
        text = header.read_text(encoding="utf-8")
        # Follow include-forward shims the same way the surface checker does.
        forwarded = INCLUDE_FORWARD_RE.search(text)
        if forwarded is not None:
            forwarded_path = public_dir / forwarded.group("target")
            if forwarded_path.is_file():
                text = forwarded_path.read_text(encoding="utf-8")
        comment_free = strip_comments(text)
        lines = comment_free.splitlines()
        index = 0
        declaration_lines: list[str] = []
        while index < len(lines):
            line = lines[index]
            if line.lstrip().startswith("#"):
                macro_lines = [line]
                while macro_lines[-1].rstrip().endswith("\\") and index + 1 < len(lines):
                    index += 1
                    macro_lines.append(lines[index])
                macro = " ".join(part.rstrip().removesuffix("\\") for part in macro_lines)
                match = re.match(
                    r"^\s*#define\s+(?P<name>TINYUI_[A-Za-z0-9_]*)(?:\(|\s+|$)(?P<body>.*)$",
                    macro,
                )
                if match and not is_include_guard_macro(rel, match.group("name")):
                    macros.append(
                        {
                            "name": match.group("name"),
                            "header": rel,
                            "value": match.group("body").strip(),
                            "signature": normalize_signature(macro),
                        }
                    )
                index += 1
                continue
            declaration_lines.append(line)
            index += 1

        for statement in iter_top_level_statements("\n".join(declaration_lines)):
            signature = normalize_signature(statement)
            if not signature:
                continue
            signature_sc = signature + ";"
            brace_free = normalize_declaration(statement)
            if signature.startswith("typedef"):
                callback = typedef_callback_re.search(brace_free)
                if callback is not None:
                    typedefs.append(
                        {
                            "name": callback.group("name"),
                            "header": rel,
                            "kind": "callback",
                            "signature": signature_sc,
                        }
                    )
                    continue
                if re.search(r"\btypedef\s+enum\b", signature):
                    name_match = typedef_name_re.search(brace_free)
                    if name_match is not None:
                        enums.append(
                            {
                                "name": name_match.group("name"),
                                "header": rel,
                                "values": _parse_enum_values(signature),
                                "signature": signature_sc,
                            }
                        )
                    continue
                name_match = typedef_name_re.search(brace_free)
                if name_match is not None:
                    if "struct" in signature and "{" not in signature:
                        opaques.append(
                            {
                                "name": name_match.group("name"),
                                "header": rel,
                                "signature": signature_sc,
                            }
                        )
                    else:
                        typedefs.append(
                            {
                                "name": name_match.group("name"),
                                "header": rel,
                                "kind": "typedef",
                                "signature": signature_sc,
                            }
                        )
                continue

            opaque = opaque_type_re.match(brace_free)
            if opaque is not None:
                opaques.append(
                    {
                        "name": brace_free.split()[1],
                        "header": rel,
                        "signature": signature_sc,
                    }
                )
                continue
            if "(" not in brace_free:
                continue
            fname = function_name(brace_free)
            if not fname:
                continue
            end = len(brace_free) - 1
            while end >= 0 and brace_free[end].isspace():
                end -= 1
            depth = 0
            opening = -1
            for idx in range(end, -1, -1):
                if brace_free[idx] == ")":
                    depth += 1
                elif brace_free[idx] == "(":
                    depth -= 1
                    if depth == 0:
                        opening = idx
                        break
            if opening < 1:
                continue
            head = brace_free[:opening].strip()
            params = _parse_parameter_types(brace_free[opening + 1 : end])
            functions.append(
                {
                    "name": fname,
                    "header": rel,
                    "return_type": _function_return_type(head, fname),
                    "parameters": params,
                    "signature": signature_sc,
                }
            )

    return {
        "schema_version": ABI_SCHEMA_VERSION,
        "headers": sorted(set(headers)),
        "typedefs": sorted(typedefs, key=_abi_sort_key),
        "opaques": sorted(opaques, key=_abi_sort_key),
        "functions": sorted(functions, key=_abi_sort_key),
        "enums": sorted(enums, key=_abi_sort_key),
        "macros": sorted(macros, key=_abi_sort_key),
        "exports": {
            "targets": list(ABI_EXPORT_TARGETS),
            "symbols": sorted({row["name"] for row in functions}),
        },
        "forbidden_tokens": sorted(FORBIDDEN_PUBLIC_FRAGMENTS),
    }


def abi_manifest_to_json(manifest: dict[str, Any]) -> str:
    """Canonical JSON encoding: sorted keys, no timestamps/absolute paths."""
    return json.dumps(
        manifest,
        ensure_ascii=True,
        sort_keys=True,
        indent=2,
        separators=(",", ": "),
    ) + "\n"


def abi_manifest_sha256(manifest: dict[str, Any]) -> str:
    return hashlib.sha256(abi_manifest_to_json(manifest).encode("utf-8")).hexdigest()


def emit_abi_manifest(path: Path, *, root: Path = ROOT) -> dict[str, Any]:
    manifest = build_abi_manifest(root=root)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(abi_manifest_to_json(manifest), encoding="utf-8")
    return manifest


def load_abi_manifest(path: Path) -> dict[str, Any]:
    if not path.is_file():
        raise FileNotFoundError(f"ABI manifest does not exist: {path}")
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"ABI manifest root must be an object: {path}")
    return payload


def compare_abi_manifests(
    expected: dict[str, Any],
    actual: dict[str, Any],
) -> list[str]:
    """Return human-readable diffs. Check mode never rewrites the expected file."""
    errors: list[str] = []
    expected_json = abi_manifest_to_json(expected)
    actual_json = abi_manifest_to_json(actual)
    if expected_json == actual_json:
        return errors

    for key in (
        "schema_version",
        "headers",
        "typedefs",
        "opaques",
        "functions",
        "enums",
        "macros",
        "exports",
        "forbidden_tokens",
    ):
        if expected.get(key) != actual.get(key):
            errors.append(f"abi_manifest_mismatch field={key}")

    if not errors:
        errors.append("abi_manifest_mismatch field=<serialized>")
    errors.append(
        "expected_sha256="
        + hashlib.sha256(expected_json.encode("utf-8")).hexdigest()
    )
    errors.append(
        "actual_sha256="
        + hashlib.sha256(actual_json.encode("utf-8")).hexdigest()
    )
    return errors


def check_abi_manifest(path: Path, *, root: Path = ROOT) -> list[str]:
    if not path.is_file():
        return [f"abi_manifest_missing path={path.as_posix()}"]
    try:
        expected = load_abi_manifest(path)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        return [f"abi_manifest_unreadable path={path.as_posix()} detail={exc}"]
    actual = build_abi_manifest(root=root)
    return compare_abi_manifests(expected, actual)


def run_surface_contract_checks() -> None:
    headers = iter_public_headers()
    assert headers, "expected TINYUI public headers to exist"
    for header in headers:
        text = resolve_public_header_text(header)
        check_forbidden_identifiers(header, text)
        check_forbidden_fragments(header, text)
        check_macro_prefixes(header, text)
        check_type_prefixes(header, text)
        check_function_prefixes(header, text)
    _assert_inventory_contract_rows()
    check_legacy_runtime_header_compiles()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="TINYUI public surface and ABI freeze checker"
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument(
        "--emit-abi-manifest",
        type=Path,
        help="Write the deterministic v2.3 ABI manifest to the given path",
    )
    mode.add_argument(
        "--check-abi-manifest",
        type=Path,
        help="Compare the public ABI against a frozen manifest (never rewrites it)",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=ROOT,
        help=argparse.SUPPRESS,
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = args.root.resolve()

    if args.emit_abi_manifest is not None:
        path = args.emit_abi_manifest
        if not path.is_absolute():
            path = (Path.cwd() / path).resolve()
        manifest = emit_abi_manifest(path, root=root)
        print(
            "TINYUI_ABI_MANIFEST_EMITTED "
            f"{path} sha256={abi_manifest_sha256(manifest)} "
            f"functions={len(manifest['functions'])} "
            f"headers={len(manifest['headers'])}"
        )
        return 0

    if args.check_abi_manifest is not None:
        path = args.check_abi_manifest
        if not path.is_absolute():
            path = (Path.cwd() / path).resolve()
        errors = check_abi_manifest(path, root=root)
        if errors:
            print("FAIL: tinyui ABI manifest check", file=sys.stderr)
            for item in errors:
                print(f"  - {item}", file=sys.stderr)
            return 1
        print(f"TINYUI_ABI_MANIFEST_OK {path}")
        return 0

    # Default CTest path: public surface + inventory contract.
    run_surface_contract_checks()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
