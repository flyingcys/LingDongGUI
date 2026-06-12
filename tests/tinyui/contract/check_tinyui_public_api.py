#!/usr/bin/env python3
from pathlib import Path
import re
import json


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "tinyui" / "include" / "picoui"
INVENTORY_JSON = ROOT / "tests" / "picoui" / "contract" / "ldgui_public_api_inventory.json"
LEDGER_JSON = ROOT / "tests" / "picoui" / "contract" / "native_api_gap_ledger.json"
ALLOWED_FUNCTION_PREFIX = "picoui_"
ALLOWED_MACRO_PREFIX = "PICOUI_"
ALLOWED_TYPE_PREFIX = "picoui_"
ALLOWED_COMPAT_TINYUI_FUNCTIONS = {
    "tinyui_screen_create",
    "tinyui_screen_load",
    "tinyui_label_create",
    "tinyui_button_create",
    "tinyui_switch_create",
}
ALLOWED_COMPAT_TINYUI_MACROS = {
    "tinyui_init",
    "tinyui_deinit",
    "tinyui_timer_handler",
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
    "missing_picoui_api",
    "missing_backend_proof",
    "missing_unit",
    "missing_gate",
    "overwrapped",
    "allowlisted",
}

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


def strip_c_comments(text: str) -> str:
    text = BLOCK_COMMENT_RE.sub("", text)
    return LINE_COMMENT_RE.sub("", text)


def assert_allowed_prefix(name: str, *, header: Path, kind: str, prefix: str) -> None:
    assert name.startswith(prefix), (
        f"{header.name} exports {kind} '{name}' outside allowed prefix '{prefix}'"
    )


def assert_allowed_public_symbol(name: str, *, header: Path, kind: str) -> None:
    if kind == "function" and name in ALLOWED_COMPAT_TINYUI_FUNCTIONS:
        return
    if kind == "macro" and name in ALLOWED_COMPAT_TINYUI_MACROS:
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
    sanitized = OPAQUE_ARM_TYPEDEF_RE.sub("", text)
    forbidden_patterns = (
        ("ld*", LD_IDENTIFIER_RE),
        ("arm_2d_*", ARM_IDENTIFIER_RE),
        ("SIGNAL_*", SIGNAL_IDENTIFIER_RE),
    )
    for label, pattern in forbidden_patterns:
        match = pattern.search(sanitized)
        assert match is None, f"{header.name} leaks forbidden identifier '{match.group(0)}' ({label})"


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


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
    inventory_symbols: set[str] = set()
    for widget in inventory.get("widgets", []):
        rows = widget.get("required_native_apis")
        assert isinstance(rows, list), f"inventory widget missing rows: {widget.get('name')}"
        for row in rows:
            symbol = row.get("ldgui_symbol")
            assert isinstance(symbol, str) and symbol, f"inventory row missing ldgui_symbol: {row!r}"
            inventory_symbols.add(symbol)
            assert symbol in ledger_by_symbol, f"{symbol} missing from native_api_gap_ledger.json"
            ledger_row = ledger_by_symbol[symbol]
            required = ledger_row.get("required", row.get("required"))
            coverage_kind = ledger_row.get("coverage_kind")
            gap_status = ledger_row.get("gap_status")
            assert isinstance(required, bool), f"{symbol} missing boolean required"
            assert coverage_kind in VALID_COVERAGE_KINDS, f"{symbol} invalid coverage_kind"
            assert gap_status in VALID_GAP_STATUSES, f"{symbol} invalid gap_status"
            assert ledger_row.get("rationale"), f"{symbol} missing rationale"
            if required:
                assert ledger_row.get("picoui_api") or coverage_kind == "shared_api_equivalence", (
                    f"{symbol} required row must name planned picoui_api or shared_api_equivalence"
                )
            else:
                assert ledger_row.get("allowlist_reason"), (
                    f"{symbol} required=false row must have allowlist_reason"
                )
                assert coverage_kind in ALLOWLISTED_COVERAGE_KINDS, (
                    f"{symbol} required=false row must use allowlisted coverage_kind"
                )
    assert set(ledger_by_symbol) == inventory_symbols, (
        "native_api_gap_ledger rows must match ldgui_public_api_inventory rows:\n"
        f"missing_from_inventory={sorted(set(ledger_by_symbol) - inventory_symbols)[:25]}\n"
        f"missing_from_ledger={sorted(inventory_symbols - set(ledger_by_symbol))[:25]}"
    )


def main() -> int:
    headers = sorted(PUBLIC_DIR.rglob("*.h"))
    assert headers, "expected PicoUI public headers to exist"
    for header in headers:
        text = header.read_text(encoding="utf-8")
        check_forbidden_identifiers(header, text)
        check_macro_prefixes(header, text)
        check_type_prefixes(header, text)
        check_function_prefixes(header, text)
    _assert_inventory_contract_rows()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
