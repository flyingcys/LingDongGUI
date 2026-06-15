import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "tinyui" / "include"
DOC = ROOT / "docs" / "superpowers" / "specs" / "2026-05-29-picoui-d-line-widget-contract-matrix.md"
WIDGETS = ["window", "label", "button", "checkbox", "switch", "slider", "text", "image"]
ALL_WIDGETS = set(WIDGETS)
CHILD_WIDGETS = ALL_WIDGETS - {"window"}
TEXT_WIDGETS = {"label", "button", "checkbox", "text"}
STATUS_VALUES = {"support", "reject", "deferred"}
CAPABILITY_COLUMNS = [
    "create",
    "create_with_props",
    "text",
    "value",
    "checked",
    "range",
    "source",
    "user_data",
    "style_class",
    "style_value",
    "enabled",
    "visible",
    "focus",
    "dirty",
    "layout",
    "theme",
    "event",
]
STATUS_INDEX = {name: index for index, name in enumerate(CAPABILITY_COLUMNS)}

WIDGET_API_POLICY = {
    "tinyui_widget_set_pos": CHILD_WIDGETS,
    "tinyui_widget_set_size": CHILD_WIDGETS,
    "tinyui_widget_set_text": TEXT_WIDGETS,
    "tinyui_widget_set_style_class": ALL_WIDGETS,
    "tinyui_widget_set_user_data": ALL_WIDGETS,
    "tinyui_widget_set_bg_color": ALL_WIDGETS,
    "tinyui_widget_set_text_color": ALL_WIDGETS,
    "tinyui_widget_set_border_color": ALL_WIDGETS,
    "tinyui_widget_set_radius": ALL_WIDGETS,
    "tinyui_widget_set_padding": ALL_WIDGETS,
    "tinyui_widget_set_visible": ALL_WIDGETS,
    "tinyui_widget_set_enabled": ALL_WIDGETS,
    "tinyui_widget_set_flex_grow": CHILD_WIDGETS,
    "tinyui_widget_set_flex_new_track": CHILD_WIDGETS,
    "tinyui_widget_set_ignore_layout": CHILD_WIDGETS,
    "tinyui_widget_set_grid_cell": CHILD_WIDGETS,
    "tinyui_widget_remove_from_parent": CHILD_WIDGETS,
    "tinyui_widget_destroy": CHILD_WIDGETS,
}
LEGACY_OPTIONAL_WIDGET_APIS = {
    "tinyui_widget_find_by_name_id",
    "tinyui_widget_get_absolute_pos",
    "tinyui_widget_get_child_count",
    "tinyui_widget_get_corner",
    "tinyui_widget_get_first_child",
    "tinyui_widget_get_height",
    "tinyui_widget_get_name_id",
    "tinyui_widget_get_next_sibling",
    "tinyui_widget_get_opacity",
    "tinyui_widget_get_parent",
    "tinyui_widget_get_relative_pos",
    "tinyui_widget_get_root",
    "tinyui_widget_get_selectable",
    "tinyui_widget_get_selected",
    "tinyui_widget_get_type",
    "tinyui_widget_get_visible",
    "tinyui_widget_get_width",
    "tinyui_widget_get_x",
    "tinyui_widget_get_y",
    "tinyui_widget_is_hidden",
    "tinyui_widget_set_center",
    "tinyui_widget_set_opacity",
    "tinyui_widget_set_selectable",
    "tinyui_widget_set_selected",
    "tinyui_widget_set_corner",
    "tinyui_widget_set_flex_min_width",
    "tinyui_widget_set_flex_min_height",
    "tinyui_widget_set_flex_max_width",
    "tinyui_widget_set_flex_max_height",
}

WINDOW_LAYOUT_APIS = {
    "tinyui_flex_set_flow",
    "tinyui_flex_set_align",
    "tinyui_flex_set_gap",
    "tinyui_grid_set_columns",
    "tinyui_grid_set_rows",
    "tinyui_grid_set_gap",
    "tinyui_grid_set_align",
}
CHILD_LAYOUT_APIS = {
    "tinyui_widget_set_pos",
    "tinyui_widget_set_size",
    "tinyui_widget_set_flex_grow",
    "tinyui_widget_set_flex_new_track",
    "tinyui_widget_set_ignore_layout",
    "tinyui_widget_set_grid_cell",
    "tinyui_widget_set_padding",
}
THEME_API_POLICY = {
    "tinyui_theme_apply_to_widget": ALL_WIDGETS - {"image"},
}


INCLUDE_FORWARD_RE = re.compile(r'^\s*#include\s+"(?P<target>tinyui/[^"]+)"\s*$', re.M)


def _load_header_text(header: Path) -> str:
    text = header.read_text(encoding="utf-8")
    match = INCLUDE_FORWARD_RE.search(text)
    if match is not None:
        forwarded = ROOT / "tinyui" / "include" / match.group("target")
        return forwarded.read_text(encoding="utf-8")
    return text


def _public_functions(header: Path) -> set[str]:
    text = _load_header_text(header)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*", "", text)
    return set(re.findall(r"\b(tinyui_[A-Za-z0-9_]+)\s*\(", text))


def _extract_table_rows(text: str, marker: str) -> dict[str, list[str]]:
    if marker not in text:
        raise AssertionError(f"Missing matrix section marker: {marker}")
    start = text.index(marker)
    table_text = text[start:]
    rows: dict[str, list[str]] = {}

    for line in table_text.splitlines():
        if not line.startswith("| "):
            if rows:
                break
            continue
        columns = [column.strip() for column in line.strip().strip("|").split("|")]
        if not columns or columns[0] in {"控件", "---"}:
            continue
        widget = columns[0]
        if widget not in WIDGETS:
            continue
        rows[widget] = columns
    return rows


def _extract_api_matrix(text: str) -> dict[str, set[str]]:
    rows = _extract_table_rows(text, "## Public API 名称矩阵")
    return {
        widget: set(re.findall(r"`(tinyui_[A-Za-z0-9_]+)`", "|".join(columns)))
        for widget, columns in rows.items()
    }


def _extract_status_matrix(text: str) -> dict[str, list[str]]:
    rows = _extract_table_rows(text, "## 控件能力状态")
    return {widget: columns[1:] for widget, columns in rows.items()}


def _expected_header_functions(widget: str) -> set[str]:
    widget_header = PUBLIC_DIR / f"{widget}.h"
    expected = _public_functions(widget_header)
    expected |= _public_functions(PUBLIC_DIR / "widget.h")
    expected |= _public_functions(PUBLIC_DIR / "theme.h")
    if widget == "window":
        expected |= _public_functions(PUBLIC_DIR / "layout.h")
    return expected


def _expected_widget_setters(widget: str) -> set[str]:
    return {api for api, widgets in WIDGET_API_POLICY.items() if widget in widgets}


def _expected_layout_apis(widget: str) -> set[str]:
    if widget == "window":
        return WINDOW_LAYOUT_APIS | {"tinyui_widget_set_padding"}
    return CHILD_LAYOUT_APIS


def _expected_theme_apis(widget: str) -> set[str]:
    return {api for api, widgets in THEME_API_POLICY.items() if widget in widgets}


def main() -> int:
    text = DOC.read_text(encoding="utf-8")
    rows = _extract_api_matrix(text)
    missing_rows = sorted(set(WIDGETS) - set(rows))
    assert not missing_rows, f"matrix missing widget rows: {missing_rows}"

    widget_header_functions = {
        function
        for function in _public_functions(PUBLIC_DIR / "widget.h")
        if function.startswith("tinyui_widget_")
    }
    policy_functions = set(WIDGET_API_POLICY)
    unclassified_widget_functions = sorted(
        widget_header_functions - policy_functions - LEGACY_OPTIONAL_WIDGET_APIS
    )
    stale_policy_functions = sorted(policy_functions - widget_header_functions)
    assert not unclassified_widget_functions, (
        "widget.h exposes tinyui_widget_* API missing from matrix policy: "
        f"{unclassified_widget_functions}"
    )
    assert not stale_policy_functions, (
        "matrix policy references tinyui_widget_* API not found in widget.h: "
        f"{stale_policy_functions}"
    )

    status_rows = _extract_status_matrix(text)
    missing_status_rows = sorted(set(WIDGETS) - set(status_rows))
    assert not missing_status_rows, f"status matrix missing widget rows: {missing_status_rows}"

    for widget, statuses in status_rows.items():
        invalid = [status for status in statuses if status not in STATUS_VALUES]
        assert not invalid, f"{widget} status matrix has invalid values: {invalid}"

    for widget in WIDGETS:
        documented = rows[widget]
        expected = _expected_header_functions(widget)
        unknown = sorted(documented - expected)
        assert not unknown, f"{widget} documents API not found in public headers: {unknown}"

        direct_public = _public_functions(PUBLIC_DIR / f"{widget}.h")

        missing_widget_setters = sorted(_expected_widget_setters(widget) - documented)
        assert not missing_widget_setters, (
            f"{widget} missing applicable widget.h setters from matrix: {missing_widget_setters}"
        )

        documented_widget_setters = documented & widget_header_functions
        unexpected_widget_setters = sorted(documented_widget_setters - _expected_widget_setters(widget))
        assert not unexpected_widget_setters, (
            f"{widget} documents non-applicable widget.h setters: {unexpected_widget_setters}"
        )

        missing_layout_apis = sorted(_expected_layout_apis(widget) - documented)
        assert not missing_layout_apis, f"{widget} missing required layout API: {missing_layout_apis}"

        documented_layout_apis = documented & (WINDOW_LAYOUT_APIS | CHILD_LAYOUT_APIS)
        unexpected_layout_apis = sorted(documented_layout_apis - _expected_layout_apis(widget))
        assert not unexpected_layout_apis, (
            f"{widget} documents non-applicable layout API: {unexpected_layout_apis}"
        )

        missing_theme_apis = sorted(_expected_theme_apis(widget) - documented)
        assert not missing_theme_apis, f"{widget} missing required theme API: {missing_theme_apis}"

        documented_theme_apis = documented & set(THEME_API_POLICY)
        unexpected_theme_apis = sorted(documented_theme_apis - _expected_theme_apis(widget))
        assert not unexpected_theme_apis, (
            f"{widget} documents non-applicable theme API: {unexpected_theme_apis}"
        )

        create_with_props = f"tinyui_{widget}_create_with_props"
        assert create_with_props in direct_public, (
            f"{widget} header missing D3 create_with_props API: {create_with_props}"
        )
        assert create_with_props in documented, (
            f"{widget} matrix missing D3 create_with_props API: {create_with_props}"
        )
        create_with_props_status = status_rows[widget][STATUS_INDEX["create_with_props"]]
        assert create_with_props_status == "support", (
            f"{widget} create_with_props status must be support after D3: "
            f"{create_with_props_status}"
        )
        enabled_status = status_rows[widget][STATUS_INDEX["enabled"]]
        visible_status = status_rows[widget][STATUS_INDEX["visible"]]
        assert enabled_status == "support", (
            f"{widget} enabled status must be support after D4: {enabled_status}"
        )
        assert visible_status == "support", (
            f"{widget} visible status must be support after D4: {visible_status}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
