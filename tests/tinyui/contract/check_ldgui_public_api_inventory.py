#!/usr/bin/env python3
"""Generate a normalized inventory of LingDongGUI public headers."""

import argparse
import json
import re
from pathlib import Path


HEADER_GLOB = "src/gui/ld*.h"
INVENTORY_SCHEMA_VERSION = "tinyui-v2.3-ldgui-public-api-inventory-v1"
EXPECTED_SCHEMA_VERSION = "tinyui-v2.3-ldgui-public-api-expected-symbols-v1"


def _strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", text)


def _normalize_declaration(statement: str) -> str:
    statement = statement.replace('extern "C" {', " ")
    statement = statement.replace('extern "C"{', " ")
    statement = statement.replace('extern "C"', " ")
    return " ".join(statement.replace("{", " ").replace("}", " ").split())


def scan_header_text(header: str, text: str) -> list[dict]:
    """Return normalized public function declarations and function-like macros."""
    comment_free = _strip_comments(text)
    rows: list[dict] = []
    declaration_lines: list[str] = []
    lines = comment_free.splitlines()
    index = 0
    while index < len(lines):
        line = lines[index]
        stripped = line.lstrip()
        if stripped == "\\":
            index += 1
            continue
        if not stripped.startswith("#"):
            declaration_lines.append(line)
            index += 1
            continue

        macro_lines = [line]
        while macro_lines[-1].rstrip().endswith("\\") and index + 1 < len(lines):
            index += 1
            macro_lines.append(lines[index])
        macro = " ".join(part.rstrip().removesuffix("\\") for part in macro_lines)
        match = re.match(
            r"^[ \t]*#define[ \t]+(?P<symbol>ld[A-Z][A-Za-z0-9_]*)\([^)]*\)[ \t]+(?P<body>.*)$",
            macro,
        )
        if match:
            target = re.match(r"(?P<target>[A-Za-z_][A-Za-z0-9_]*)", match.group("body").strip())
            if target:
                rows.append(
                    {
                        "header": header,
                        "kind": "macro",
                        "symbol": match.group("symbol"),
                        "target": target.group("target"),
                    }
                )
            else:
                rows.append(
                    {
                        "header": header,
                        "kind": "macro",
                        "symbol": match.group("symbol"),
                        "signature": " ".join(macro.split()),
                    }
                )
        else:
            alias = re.match(
                r"^[ \t]*#define[ \t]+(?P<symbol>ld[A-Z][A-Za-z0-9_]*)[ \t]+"
                r"(?P<target>ld[A-Z][A-Za-z0-9_]*)[ \t]*$",
                macro,
            )
            if alias:
                rows.append(
                    {
                        "header": header,
                        "kind": "macro",
                        "symbol": alias.group("symbol"),
                        "target": alias.group("target"),
                    }
                )
        index += 1

    declarations = "\n".join(declaration_lines)
    for statement in declarations.split(";"):
        normalized = _normalize_declaration(statement)
        match = re.search(r"\b(?P<symbol>ld[A-Z][A-Za-z0-9_]*)\s*\([^()]*\)$", normalized)
        if match:
            rows.append(
                {
                    "header": header,
                    "kind": "function",
                    "symbol": match.group("symbol"),
                    "signature": normalized + ";",
                }
            )

    unique = {(row["header"], row["kind"], row["symbol"]): row for row in rows}
    return sorted(unique.values(), key=lambda row: (row["header"], row["symbol"], row["kind"]))


def scan_headers(root: Path) -> list[dict]:
    rows: list[dict] = []
    header_directory = root / "src" / "gui"
    if not header_directory.is_dir():
        raise ValueError(f"missing header directory: {header_directory}")
    for header in sorted(header_directory.glob("ld*.h")):
        rows.extend(
            scan_header_text(
                header.relative_to(root).as_posix(),
                header.read_text(encoding="utf-8", errors="replace"),
            )
        )
    return sorted(rows, key=lambda row: (row["header"], row["symbol"], row["kind"]))


def build_inventory(rows: list[dict]) -> dict:
    return {
        "schema_version": INVENTORY_SCHEMA_VERSION,
        "source": HEADER_GLOB,
        "public_api_total": len(rows),
        "entries": rows,
    }


def build_expected(rows: list[dict]) -> dict:
    return {
        "schema_version": EXPECTED_SCHEMA_VERSION,
        "source": HEADER_GLOB,
        "symbols": sorted({row["symbol"] for row in rows}),
    }


def _load_json(path: Path) -> dict:
    if not path.is_file():
        raise ValueError(f"missing required file: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        raise ValueError(f"invalid JSON: {path}: {error}") from error
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


def _validate_inventory(value: dict) -> list[dict]:
    if value.get("schema_version") != INVENTORY_SCHEMA_VERSION:
        raise ValueError("inventory schema_version is invalid")
    if value.get("source") != HEADER_GLOB:
        raise ValueError("inventory source is invalid")
    entries = value.get("entries")
    if not isinstance(entries, list) or value.get("public_api_total") != len(entries):
        raise ValueError("inventory entries or public_api_total is invalid")
    for entry in entries:
        if not isinstance(entry, dict):
            raise ValueError("inventory entry must be an object")
        if set(entry) not in ({"header", "kind", "symbol", "signature"}, {"header", "kind", "symbol", "target"}):
            raise ValueError("inventory entry fields are invalid")
        if entry.get("kind") == "function" and not isinstance(entry.get("signature"), str):
            raise ValueError("function inventory entry requires signature")
        if entry.get("kind") == "macro" and not isinstance(entry.get("target"), str) and not isinstance(entry.get("signature"), str):
            raise ValueError("macro inventory entry requires target or signature")
    return entries


def _legacy_inventory_entries(value: dict) -> list[dict]:
    if value.get("source") != HEADER_GLOB or not isinstance(value.get("widgets"), list):
        raise ValueError("inventory schema_version is invalid")
    entries: list[dict] = []
    for widget in value["widgets"]:
        if not isinstance(widget, dict) or not isinstance(widget.get("required_native_apis"), list):
            raise ValueError("legacy inventory widgets are invalid")
        for row in widget["required_native_apis"]:
            if not isinstance(row, dict):
                raise ValueError("legacy inventory entry must be an object")
            symbol = row.get("ldgui_symbol")
            header = row.get("header")
            if not isinstance(symbol, str) or not isinstance(header, str):
                raise ValueError("legacy inventory entry fields are invalid")
            if isinstance(row.get("macro_target"), str):
                entries.append(
                    {"header": header, "kind": "macro", "symbol": symbol, "target": row["macro_target"]}
                )
            elif isinstance(row.get("signature"), str):
                entries.append(
                    {"header": header, "kind": "function", "symbol": symbol, "signature": row["signature"]}
                )
            else:
                raise ValueError("legacy inventory entry requires signature or macro_target")
    return entries


def _read_inventory(value: dict) -> tuple[list[dict], list[str]]:
    if value.get("schema_version") == INVENTORY_SCHEMA_VERSION:
        return _validate_inventory(value), []
    if isinstance(value.get("schema_version"), str):
        return _legacy_inventory_entries(value), ["inventory schema_version is legacy"]
    raise ValueError("inventory schema_version is invalid")


def _validate_expected(value: dict) -> list[str]:
    if value.get("schema_version") != EXPECTED_SCHEMA_VERSION:
        raise ValueError("expected schema_version is invalid")
    if value.get("source") != HEADER_GLOB:
        raise ValueError("expected source is invalid")
    symbols = value.get("symbols")
    if not isinstance(symbols, list) or any(not isinstance(symbol, str) for symbol in symbols):
        raise ValueError("expected symbols are invalid")
    if symbols != sorted(set(symbols)):
        raise ValueError("expected symbols must be unique and sorted")
    return symbols


def _read_expected(value: dict) -> tuple[list[str], list[str]]:
    if value.get("schema_version") == EXPECTED_SCHEMA_VERSION:
        return _validate_expected(value), []
    if value.get("source") != HEADER_GLOB:
        raise ValueError("expected schema_version is invalid")
    symbols = value.get("symbols")
    if not isinstance(symbols, list) or any(not isinstance(symbol, str) for symbol in symbols):
        raise ValueError("expected symbols are invalid")
    return sorted(set(symbols)), ["expected schema_version is legacy"]


def _compare_entries(scanned: list[dict], stored: list[dict]) -> dict:
    key = lambda row: (row["header"], row["kind"], row["symbol"])
    scanned_by_key = {key(row): row for row in scanned}
    stored_by_key = {key(row): row for row in stored}
    common = sorted(scanned_by_key.keys() & stored_by_key.keys())
    return {
        "missing": [stored_by_key[item] for item in sorted(stored_by_key.keys() - scanned_by_key.keys())],
        "extra": [scanned_by_key[item] for item in sorted(scanned_by_key.keys() - stored_by_key.keys())],
        "changed_signature": [
            {"current": scanned_by_key[item], "stored": stored_by_key[item]}
            for item in common
            if scanned_by_key[item] != stored_by_key[item]
        ],
    }


def check(root: Path, output: Path, expected_output: Path) -> dict:
    scanned = scan_headers(root)
    stored, inventory_errors = _read_inventory(_load_json(output))
    expected_symbols, expected_errors = _read_expected(_load_json(expected_output))
    result = _compare_entries(scanned, stored)
    schema_errors = inventory_errors + expected_errors
    if schema_errors:
        result["schema_errors"] = schema_errors
    scanned_symbols = sorted({row["symbol"] for row in scanned})
    if scanned_symbols != expected_symbols:
        result["expected_missing"] = sorted(set(scanned_symbols) - set(expected_symbols))
        result["expected_extra"] = sorted(set(expected_symbols) - set(scanned_symbols))
    return result


def write(root: Path, output: Path, expected_output: Path) -> None:
    rows = scan_headers(root)
    output.write_text(json.dumps(build_inventory(rows), indent=2) + "\n", encoding="utf-8")
    expected_output.write_text(json.dumps(build_expected(rows), indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--expected-output", type=Path, required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    try:
        if arguments.write:
            write(arguments.root, arguments.output, arguments.expected_output)
            return 0
        result = check(arguments.root, arguments.output, arguments.expected_output)
    except ValueError as error:
        print(f"LDGUI_PUBLIC_API_INVENTORY_ERROR: {error}")
        return 1
    if any(result.values()):
        print(json.dumps(result, indent=2))
        return 1
    print("LDGUI_PUBLIC_API_INVENTORY_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
