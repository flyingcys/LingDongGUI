import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
INVENTORY_JSON = ROOT / "tests" / "picoui" / "contract" / "picoui_native_100_inventory.json"
LD_BASE_HEADER = ROOT / "src" / "gui" / "ldBase.h"

EXPECTED_WIDGET_NAMES = {
    "window",
    "button",
    "image",
    "text",
    "line_edit",
    "graph",
    "checkbox",
    "slider",
    "switch",
    "progress_bar",
    "gauge",
    "qrcode",
    "date_time",
    "icon_slider",
    "combo_box",
    "arc",
    "radial_menu",
    "scroll_selecter",
    "label",
    "table",
    "keyboard",
    "animation",
    "list",
    "message_box",
    "calendar",
    "progress_wheel",
    "clock",
    "canvas",
}

DISALLOWED_STATUS_FIELDS = {"reject", "deferred", "incomplete_contract"}
ENUM_NAME_NORMALIZATION = {
    "check_box": "checkbox",
    "q_r_code": "qrcode",
}


def _load_inventory() -> dict:
    return json.loads(INVENTORY_JSON.read_text(encoding="utf-8"))


def _parse_widget_types() -> set[str]:
    content = LD_BASE_HEADER.read_text(encoding="utf-8")
    match = re.search(r"typedef enum\{(?P<body>.*?)\}ldWidgetType_t;", content, re.S)
    if not match:
        raise AssertionError("cannot locate ldWidgetType_t enum in src/gui/ldBase.h")
    body = match.group("body")
    raw_names = re.findall(r"widgetType([A-Za-z0-9_]+)", body)
    if not raw_names:
        raise AssertionError("ldWidgetType_t enum has no widgetType entries")
    widget_names = set()
    for raw_name in raw_names:
        if raw_name == "Background":
            continue
        snake = re.sub(r"(?<!^)(?=[A-Z])", "_", raw_name).lower()
        snake = ENUM_NAME_NORMALIZATION.get(snake, snake)
        widget_names.add(snake)
    return widget_names


def main() -> int:
    inventory = _load_inventory()
    assert inventory.get("schema_version") == "a-0.7-native-100-inventory-v1", (
        "inventory schema_version must be a-0.7-native-100-inventory-v1"
    )
    assert inventory.get("source") == "src/gui/ld*.h", (
        "inventory source must point at src/gui/ld*.h"
    )

    header_widgets = _parse_widget_types()
    assert len(header_widgets) == len(EXPECTED_WIDGET_NAMES), (
        f"ldWidgetType_t widget-like total must be {len(EXPECTED_WIDGET_NAMES)}, got {len(header_widgets)}"
    )
    assert header_widgets == EXPECTED_WIDGET_NAMES, (
        "ldWidgetType_t widget-like names drifted from native-100 inventory expectation:\n"
        f"expected={sorted(EXPECTED_WIDGET_NAMES)}\nactual={sorted(header_widgets)}"
    )

    widgets = inventory.get("widgets")
    assert isinstance(widgets, list), "inventory widgets must be a list"
    assert inventory.get("widget_like_total") == len(EXPECTED_WIDGET_NAMES), (
        f"inventory widget_like_total must be {len(EXPECTED_WIDGET_NAMES)}"
    )
    assert len(widgets) == len(EXPECTED_WIDGET_NAMES), (
        f"inventory widgets must contain {len(EXPECTED_WIDGET_NAMES)} rows, got {len(widgets)}"
    )

    seen = set()
    for widget in widgets:
        name = widget.get("name")
        assert isinstance(name, str) and name, f"invalid widget row name: {widget!r}"
        assert name not in seen, f"duplicate inventory widget row: {name}"
        seen.add(name)
        assert name in EXPECTED_WIDGET_NAMES, f"unexpected inventory widget row: {name}"
        required_native_capabilities = widget.get("required_native_capabilities")
        assert isinstance(required_native_capabilities, list) and required_native_capabilities, (
            f"{name} must declare at least one native capability"
        )
        for field_name in DISALLOWED_STATUS_FIELDS:
            assert field_name not in widget, (
                f"{name} inventory row must not expose legacy status field {field_name!r}"
            )

    assert seen == EXPECTED_WIDGET_NAMES, (
        "inventory widget set must match native widget set:\n"
        f"expected={sorted(EXPECTED_WIDGET_NAMES)}\nactual={sorted(seen)}"
    )
    assert any(widget.get("name") == "animation" for widget in widgets), (
        "inventory must cover animation"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
