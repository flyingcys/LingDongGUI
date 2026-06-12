import json
import re
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
TINYUI_CONTRACT_DIR = ROOT / "tests" / "tinyui" / "contract"
EXPECTED_JSON = TINYUI_CONTRACT_DIR / "ldgui_public_api_expected_symbols.json"
INVENTORY_JSON = TINYUI_CONTRACT_DIR / "ldgui_public_api_inventory.json"
LEDGER_JSON = TINYUI_CONTRACT_DIR / "native_api_gap_ledger.json"
HEADER_GLOB = "src/gui/ld*.h"
SCHEMA_VERSION = "a-0.8-ldgui-public-api-inventory-v1"
LEDGER_SOURCE_PATHS = {
    "tests/tinyui/contract/ldgui_public_api_inventory.json",
}

VALID_CATEGORIES = {
    "init",
    "lifecycle",
    "show",
    "setter",
    "getter",
    "callback_hook",
    "update_action",
    "capability",
    "macro_alias",
    "helper",
}
CAPABILITY_WORDS = (
    "Add",
    "Remove",
    "Clear",
    "Load",
    "Select",
    "Seek",
    "Focus",
    "Click",
    "Exit",
    "Insert",
    "Delete",
    "Scroll",
    "Start",
    "Stop",
    "Pause",
    "Resume",
    "Play",
)
LEDGER_REQUIRED_CATEGORIES = {
    "init",
    "setter",
    "getter",
    "callback_hook",
    "update_action",
    "capability",
    "macro_alias",
    "helper",
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
VALID_GROUP_KINDS = {
    "widget",
    "shared_base",
    "runtime_host",
    "internal_helper",
    "enum_only",
}
VALID_POLICY_CATEGORIES = {
    "direct_covered",
    "lifecycle_internal",
    "render_pipeline_internal",
    "runtime_host_internal",
    "layout_solver_internal",
    "memory_internal",
    "base_tree_policy",
    "resource_time_helper_policy",
    "drawing_helper_policy",
    "backend_private_hook",
    "native_action_private",
    "enum_only_semantics",
}
REQUIRED_SYMBOLS = [
    "ldTextSetStaticText",
    "ldImageSetMaskColor",
    "ldKeyboardBtnUserDraw",
    "ldTableGetItemRegion",
    "ldBaseFocusNavigate",
    "ldMalloc",
    "ldGuiJumpPage",
    "ldGuiJumpPage_0",
    "ldGuiJumpPage_1",
    "ldGuiJumpPage_2",
]
TASK_BY_WIDGET = {
    "base": "R2",
    "gui": "R2",
    "mem": "R2",
    "switch_internal": "R2",
    "window": "R2",
    "window_layout_internal": "R2",
    "label": "R3",
    "checkbox": "R3",
    "switch": "R3",
    "list": "R3",
    "text": "R3",
    "image": "R3",
    "line_edit": "R4",
    "keyboard": "R4",
    "combo_box": "R4",
    "scroll_selecter": "R4",
    "table": "R4",
    "graph": "R4",
    "calendar": "R4",
    "button": "R5",
    "slider": "R5",
    "qrcode": "R5",
    "arc": "R5",
    "gauge": "R5",
    "date_time": "R5",
    "clock": "R5",
    "progress_bar": "R5",
    "progress_wheel": "R5",
    "icon_slider": "R5",
    "radial_menu": "R5",
    "message_box": "R6",
    "animation": "R5",
}
GROUP_KIND_BY_WIDGET = {
    "base": "shared_base",
    "gui": "runtime_host",
    "mem": "internal_helper",
    "switch_internal": "internal_helper",
    "window_layout_internal": "internal_helper",
}


def _strip_comments(content: str) -> str:
    content = re.sub(r"/\*.*?\*/", "", content, flags=re.S)
    return re.sub(r"//.*", "", content)


def _clean_declaration_statement(statement: str) -> str:
    statement = statement.replace('extern "C" {', " ")
    statement = statement.replace("extern \"C\"{", " ")
    statement = statement.replace("extern \"C\"", " ")
    return " ".join(statement.split())


def _snake_widget(raw: str) -> str:
    raw = raw.removeprefix("ld")
    replacements = {
        "QRCode": "qrcode",
        "CheckBox": "checkbox",
        "LineEdit": "line_edit",
        "ComboBox": "combo_box",
        "ScrollSelecter": "scroll_selecter",
        "ProgressBar": "progress_bar",
        "ProgressWheel": "progress_wheel",
        "IconSlider": "icon_slider",
        "RadialMenu": "radial_menu",
        "MessageBox": "message_box",
        "DateTime": "date_time",
    }
    if raw in replacements:
        return replacements[raw]
    return re.sub(r"(?<!^)(?=[A-Z])", "_", raw).lower()


def _symbol_widget(symbol: str, header: Path) -> str:
    return _snake_widget(header.stem)


def _category(symbol: str, signature: str, target: str | None = None) -> str:
    if target:
        return "macro_alias"
    if symbol.endswith("_init") or symbol.endswith("Init"):
        return "init"
    if any(token in symbol for token in ("_depose", "_on_load", "_on_frame_start", "_on_frame_complete")):
        return "lifecycle"
    if symbol.endswith("_show") or symbol.endswith("Show"):
        return "show"
    if "Callback" in symbol or "UserDraw" in symbol or re.search(r"\bweak function\b", signature):
        return "callback_hook"
    if "Update" in symbol or "Navigate" in symbol or symbol.endswith("Move") or symbol.endswith("Resize"):
        return "update_action"
    if re.search(r"(^ld[A-Z][A-Za-z0-9]*Set)|(\bSet[A-Z])", symbol):
        return "setter"
    if re.search(r"(^ld[A-Z][A-Za-z0-9]*(Get|Is))|(\b(Get|Is)[A-Z])", symbol):
        return "getter"
    if any(word in symbol for word in CAPABILITY_WORDS):
        return "capability"
    return "helper"


def _required(category: str) -> bool:
    return category in LEDGER_REQUIRED_CATEGORIES


def _group_kind(widget: str) -> str:
    return GROUP_KIND_BY_WIDGET.get(widget, "widget")


def _policy_category(widget: str, symbol: str, category: str, gap_status: str) -> str:
    if gap_status == "covered":
        return "direct_covered"
    if symbol in {"ldButtonActionInit", "ldButtonActionIsPressById"}:
        return "native_action_private"
    if symbol in {"ldKeyboardGetTargetBtnList", "ldKeyboardCallback", "ldKeyboardBtnUserDraw"}:
        return "backend_private_hook"
    if widget == "gui":
        return "runtime_host_internal"
    if widget == "mem":
        return "memory_internal"
    if widget in {"switch_internal", "window_layout_internal"}:
        return "layout_solver_internal"
    if widget == "base":
        if symbol in {"ldBaseGetVresImage", "ldBaseGetVresFont", "ldBaseGetDate", "ldBaseGetTime", "ldBaseGetWeek"}:
            return "resource_time_helper_policy"
        if symbol in {"ldBaseDrawLine", "ldBaseImageScale"}:
            return "drawing_helper_policy"
        return "base_tree_policy"
    if category == "lifecycle" or any(
        symbol.endswith(suffix)
        for suffix in ("_depose", "_on_load", "_on_frame_start", "_on_frame_complete")
    ):
        return "lifecycle_internal"
    if category == "show" or symbol.endswith("_show") or symbol.endswith("Show"):
        return "render_pipeline_internal"
    if widget == "window":
        return "enum_only_semantics"
    return "backend_private_hook"


def _default_gap_status(category: str) -> str:
    if category in {"lifecycle", "show"}:
        return "allowlisted"
    if category == "macro_alias":
        return "missing_backend_proof"
    if category == "helper":
        return "allowlisted"
    return "missing_picoui_api"


def _default_allowlist_reason(category: str) -> str:
    if category == "lifecycle":
        return "LingDongGUI lifecycle hook; PicoUI must not expose a duplicate public wrapper."
    if category == "show":
        return "Render/show entry point owned by backend rendering pipeline."
    return "Internal/helper or host utility API; R0 keeps it in the ledger but does not propose a public wrapper."


def _default_picoui_api(symbol: str, category: str) -> str:
    if category in {"lifecycle", "show"}:
        return ""
    name = re.sub(r"(?<!^)(?=[A-Z])", "_", symbol.removeprefix("ld")).lower()
    return f"picoui_{name}"


def _default_backend_proof(symbol: str, category: str) -> str:
    if category in {"lifecycle", "show"}:
        return ""
    name = re.sub(r"(?<!^)(?=[A-Z])", "_", symbol.removeprefix("ld")).lower()
    return f"picoui_backend_{name}"


def _default_unit_test(symbol: str, category: str) -> str:
    if category in {"lifecycle", "show"}:
        return ""
    name = re.sub(r"(?<!^)(?=[A-Z])", "_", symbol.removeprefix("ld")).lower()
    return f"test_{name}_native_parity"


def scan_headers() -> list[dict]:
    rows: list[dict] = []
    for header in sorted((ROOT / "src" / "gui").glob("ld*.h")):
        rel_header = header.relative_to(ROOT).as_posix()
        raw = header.read_text(encoding="utf-8", errors="replace")
        content = _strip_comments(raw)
        declaration_lines = []
        skip_macro_continuation = False
        for line in content.splitlines():
            stripped = line.strip()
            if skip_macro_continuation:
                skip_macro_continuation = stripped.endswith("\\")
                continue
            if not stripped:
                continue
            if stripped.startswith("#"):
                skip_macro_continuation = stripped.startswith("#define") and stripped.endswith("\\")
                continue
            declaration_lines.append(line)
        for statement in "\n".join(declaration_lines).split(";"):
            statement = _clean_declaration_statement(statement)
            if not statement or "(" not in statement or ")" not in statement:
                continue
            if statement.startswith(("typedef ", "#", "struct ", "enum ", "static ")):
                continue
            if "{" in statement or "}" in statement:
                continue
            match = re.search(r"\b(?P<name>ld[A-Z][A-Za-z0-9_]*)\s*\([^()]*\)$", statement)
            if not match:
                continue
            symbol = match.group("name")
            if symbol.startswith("ld_"):
                continue
            signature = statement
            category = _category(symbol, signature)
            rows.append(
                {
                    "widget": _symbol_widget(symbol, header),
                    "header": rel_header,
                    "ldgui_symbol": symbol,
                    "category": category,
                    "signature": signature + ";",
                    "required": _required(category),
                }
            )
        macro_content = _strip_comments(raw.replace("\\\n", " "))
        for match in re.finditer(r"(?m)^[ \t]*#define[ \t]+(?P<name>ld[A-Z][A-Za-z0-9_]*)(?P<params>\([^)]*\))?[ \t]+(?P<body>.*)$", macro_content):
            symbol = match.group("name")
            body = match.group("body").strip()
            target_match = re.match(r"(?P<target>[A-Za-z_][A-Za-z0-9_]*)", body)
            if target_match:
                target = target_match.group("target")
            else:
                nested_target = re.search(r"\b(?P<target>_?ld[A-Za-z0-9_]*)\s*\(", body)
                if not nested_target:
                    continue
                target = nested_target.group("target")
            if not target.startswith(("ld", "_ld", "__ld", "xBtn", "ARM_CONNECT")):
                continue
            rows.append(
                {
                    "widget": _symbol_widget(symbol, header),
                    "header": rel_header,
                    "ldgui_symbol": symbol,
                    "category": "macro_alias",
                    "signature": " ".join(match.group(0).split()),
                    "macro_target": target,
                    "required": True,
                }
            )
    rows.sort(key=lambda row: (row["widget"], row["header"], row["ldgui_symbol"], row["signature"]))
    seen = set()
    unique_rows = []
    for row in rows:
        key = (row["header"], row["ldgui_symbol"], row["signature"])
        if key in seen:
            continue
        seen.add(key)
        unique_rows.append(row)
    return unique_rows


def build_inventory(rows: list[dict]) -> dict:
    widgets: dict[str, list[dict]] = defaultdict(list)
    for row in rows:
        category = row["category"]
        api_row = {
            **row,
            "group_kind": _group_kind(row["widget"]),
            "picoui_api": _default_picoui_api(row["ldgui_symbol"], category),
            "backend_proof": _default_backend_proof(row["ldgui_symbol"], category),
            "unit_test": _default_unit_test(row["ldgui_symbol"], category),
            "gate_evidence": [],
            "gap_status": _default_gap_status(category),
        }
        api_row["policy_category"] = _policy_category(
            row["widget"], row["ldgui_symbol"], category, api_row["gap_status"]
        )
        if api_row["gap_status"] == "allowlisted":
            api_row["allowlist_reason"] = _default_allowlist_reason(category)
        widgets[row["widget"]].append(api_row)
    return {
        "schema_version": SCHEMA_VERSION,
        "source": HEADER_GLOB,
        "public_api_total": len(rows),
        "widgets": [
            {"name": name, "required_native_apis": widget_rows}
            for name, widget_rows in sorted(widgets.items())
        ],
        "allowlist": [
            {
                "ldgui_symbol": row["ldgui_symbol"],
                "reason": row["allowlist_reason"],
            }
            for widget in sorted(widgets)
            for row in widgets[widget]
            if row.get("allowlist_reason")
        ],
    }


def build_ledger(rows: list[dict]) -> dict:
    ledger_rows = []
    for row in rows:
        if row["category"] not in LEDGER_REQUIRED_CATEGORIES and row["category"] not in {"lifecycle", "show"}:
            continue
        status = _default_gap_status(row["category"])
        ledger_row = {
            "widget": row["widget"],
            "group_kind": _group_kind(row["widget"]),
            "header": row["header"],
            "ldgui_symbol": row["ldgui_symbol"],
            "category": row["category"],
            "gap_status": status,
            "policy_category": _policy_category(row["widget"], row["ldgui_symbol"], row["category"], status),
            "planned_task": TASK_BY_WIDGET.get(row["widget"], "R5"),
            "overwrap_risk": False,
            "notes": "R0 inventory seed; later tasks must replace missing_* with concrete PicoUI/backend/unit/gate evidence.",
        }
        if status == "allowlisted":
            ledger_row["planned_task"] = "R2"
            ledger_row["allowlist_reason"] = _default_allowlist_reason(row["category"])
            ledger_row["notes"] = ledger_row["allowlist_reason"]
        if status == "covered":
            ledger_row["equivalence_proof"] = ""
        ledger_rows.append(ledger_row)
    return {
        "schema_version": "a-0.8-native-api-gap-ledger-v1",
        "source": "tests/tinyui/contract/ldgui_public_api_inventory.json",
        "rows": ledger_rows,
    }


def build_expected(rows: list[dict]) -> dict:
    symbols = sorted({row["ldgui_symbol"] for row in rows})
    return {
        "schema_version": "a-0.8-ldgui-public-api-expected-symbols-v1",
        "source": HEADER_GLOB,
        "minimum_symbol_count": len(symbols),
        "required_symbols": REQUIRED_SYMBOLS,
        "symbols": symbols,
    }


def _load_json(path: Path) -> dict:
    if not path.exists():
        raise AssertionError(f"missing required file: {path.relative_to(ROOT)}")
    return json.loads(path.read_text(encoding="utf-8"))


def _inventory_rows(inventory: dict) -> list[dict]:
    rows = []
    for widget in inventory.get("widgets", []):
        for row in widget.get("required_native_apis", []):
            rows.append(row)
    return rows


def validate_expected(rows: list[dict], expected: dict) -> None:
    symbols = {row["ldgui_symbol"] for row in rows}
    assert len(symbols) >= expected["minimum_symbol_count"], (
        f"symbol count below frozen minimum: {len(symbols)} < {expected['minimum_symbol_count']}"
    )
    frozen_symbols = set(expected.get("symbols", []))
    missing_frozen = sorted(frozen_symbols - symbols)
    extra_scanned = sorted(symbols - frozen_symbols)
    assert not missing_frozen and not extra_scanned, (
        "current src/gui/ld*.h public symbols drifted from frozen expected set:\n"
        f"missing={missing_frozen[:30]}\nextra={extra_scanned[:30]}"
    )
    missing = sorted(set(expected["required_symbols"]) - symbols)
    assert not missing, f"missing required ldgui symbols: {missing}"
    for symbol in REQUIRED_SYMBOLS:
        assert symbol in symbols, f"missing hard-required ldgui symbol: {symbol}"


def validate_inventory(scanned_rows: list[dict], inventory: dict) -> None:
    assert inventory.get("schema_version") == SCHEMA_VERSION
    assert inventory.get("source") == HEADER_GLOB
    inventory_rows = _inventory_rows(inventory)
    scanned_keys = {(row["header"], row["ldgui_symbol"], row["signature"]) for row in scanned_rows}
    inventory_keys = {(row.get("header"), row.get("ldgui_symbol"), row.get("signature")) for row in inventory_rows}
    assert inventory_keys == scanned_keys, (
        "inventory rows must exactly match current src/gui/ld*.h scan:\n"
        f"missing={sorted(scanned_keys - inventory_keys)[:20]}\n"
        f"extra={sorted(inventory_keys - scanned_keys)[:20]}"
    )
    allowlist_symbols = {row.get("ldgui_symbol") for row in inventory.get("allowlist", [])}
    for row in inventory_rows:
        for field in (
            "widget",
            "header",
            "ldgui_symbol",
            "category",
            "signature",
            "required",
            "group_kind",
            "picoui_api",
            "backend_proof",
            "unit_test",
            "gate_evidence",
            "gap_status",
            "policy_category",
        ):
            assert field in row, f"{row.get('ldgui_symbol')} missing inventory field {field}"
        assert row["category"] in VALID_CATEGORIES, f"{row['ldgui_symbol']} invalid category"
        assert row["gap_status"] in VALID_GAP_STATUSES, f"{row['ldgui_symbol']} invalid gap_status"
        assert row["group_kind"] in VALID_GROUP_KINDS, f"{row['ldgui_symbol']} invalid group_kind"
        assert row["policy_category"] in VALID_POLICY_CATEGORIES, (
            f"{row['ldgui_symbol']} invalid policy_category"
        )
        if row["gap_status"] == "covered":
            assert row["policy_category"] == "direct_covered", (
                f"{row['ldgui_symbol']} covered row must use direct_covered policy"
            )
        if row["gap_status"] == "allowlisted":
            assert row["required"] is False, f"{row['ldgui_symbol']} allowlisted row must be required=false"
            assert row["policy_category"] != "direct_covered", (
                f"{row['ldgui_symbol']} allowlisted row must not use direct_covered policy"
            )
            assert row.get("allowlist_reason"), f"{row['ldgui_symbol']} allowlisted without reason"
            assert row["ldgui_symbol"] in allowlist_symbols, (
                f"{row['ldgui_symbol']} allowlisted row missing top-level allowlist entry"
            )
    for row in inventory.get("allowlist", []):
        assert row.get("reason"), f"{row.get('ldgui_symbol')} allowlist entry missing reason"


def validate_ledger(scanned_rows: list[dict], ledger: dict) -> None:
    assert ledger.get("schema_version") == "a-0.8-native-api-gap-ledger-v1"
    assert ledger.get("source") in LEDGER_SOURCE_PATHS
    ledger_rows = ledger.get("rows")
    assert isinstance(ledger_rows, list) and ledger_rows, "ledger rows must be non-empty"
    ledger_by_symbol = defaultdict(list)
    for row in ledger_rows:
        symbol = row.get("ldgui_symbol")
        ledger_by_symbol[symbol].append(row)
        assert row.get("gap_status") in VALID_GAP_STATUSES, f"{symbol} invalid gap_status"
        assert row.get("group_kind") in VALID_GROUP_KINDS, f"{symbol} invalid group_kind"
        assert row.get("policy_category") in VALID_POLICY_CATEGORIES, f"{symbol} invalid policy_category"
        assert row.get("planned_task"), f"{symbol} missing planned_task"
        assert "overwrap_risk" in row, f"{symbol} missing overwrap_risk"
        if row.get("gap_status") == "covered":
            assert row.get("policy_category") == "direct_covered", (
                f"{symbol} covered row must use direct_covered policy"
            )
        if row.get("gap_status") == "allowlisted":
            assert row.get("required") is False, f"{symbol} allowlisted row must be required=false"
            assert row.get("policy_category") != "direct_covered", (
                f"{symbol} allowlisted row must not use direct_covered policy"
            )
            assert row.get("allowlist_reason"), f"{symbol} allowlisted without reason"
        if row.get("gap_status") == "covered" and row.get("coverage_kind") == "shared_api_equivalence":
            assert row.get("equivalence_proof"), f"{symbol} covered without equivalence_proof"
        if row.get("gap_status") == "overwrapped":
            assert row.get("notes"), f"{symbol} overwrapped without notes"
    for row in scanned_rows:
        if row["category"] in LEDGER_REQUIRED_CATEGORIES:
            assert ledger_by_symbol.get(row["ldgui_symbol"]), (
                f"{row['ldgui_symbol']} category {row['category']} missing from ledger"
            )
        if row["category"] in {"lifecycle", "show"}:
            matches = ledger_by_symbol.get(row["ldgui_symbol"], [])
            assert matches, f"{row['ldgui_symbol']} lifecycle/show row missing allowlist coverage"
            assert any(match.get("gap_status") == "allowlisted" and match.get("allowlist_reason") for match in matches), (
                f"{row['ldgui_symbol']} lifecycle/show row must be allowlisted with reason"
            )


def write_fixtures(rows: list[dict]) -> None:
    EXPECTED_JSON.write_text(json.dumps(build_expected(rows), indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    INVENTORY_JSON.write_text(json.dumps(build_inventory(rows), indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    LEDGER_JSON.write_text(json.dumps(build_ledger(rows), indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def main() -> int:
    rows = scan_headers()
    expected = _load_json(EXPECTED_JSON)
    inventory = _load_json(INVENTORY_JSON)
    ledger = _load_json(LEDGER_JSON)
    validate_expected(rows, expected)
    validate_inventory(rows, inventory)
    validate_ledger(rows, ledger)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
