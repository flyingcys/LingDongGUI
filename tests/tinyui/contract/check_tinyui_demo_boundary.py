from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "tinyui" / "demo"
SDL_CMAKE = ROOT / "examples" / "sdl" / "CMakeLists.txt"
FORBIDDEN_PATTERNS = {
    "ld*": re.compile(r"\bld[A-Za-z0-9_]+\b"),
    "arm_2d_*": re.compile(r"\barm_2d_[A-Za-z0-9_]+\b"),
    "SIGNAL_*": re.compile(r"\bSIGNAL_[A-Za-z0-9_]+\b"),
}
OPAQUE_ASSET_DECL_RE = re.compile(
    r"^\s*(typedef\s+struct\s+arm_2d_tile_t\s+arm_2d_tile_t;|"
    r"extern\s+const\s+arm_2d_tile_t\s+c_tile[A-Za-z0-9_]+;)\s*$",
    re.M,
)
REQUIRED_DEMOS = {
    "hello_world",
    "basic_widgets",
    "layout_flex",
    "layout_grid",
    "theme_showcase",
    "settings_panel",
    "list_basic",
    "progress_bar_basic",
    "progress_wheel_basic",
    "qrcode_basic",
    "message_box_basic",
    "date_time_basic",
    "clock_basic",
    "line_edit_basic",
    "combo_box_basic",
    "scroll_selecter_basic",
    "table_basic",
    "graph_basic",
    "calendar_basic",
    "animation_basic",
    "legacy_widget_parity",
    "layout_parity",
    "grid_parity",
}
REQUIRED_RUNNER_TARGET = "tinyui_demo"
DEMO_MARKERS = {
    "legacy_widget_parity": (
        "tinyui_image_create",
        "tinyui_switch_create",
        "tinyui_checkbox_create",
        "tinyui_slider_create",
        "tinyui_button_create",
        "tinyui_text_create",
        "tinyui_progress_bar_create",
        "tinyui_list_create",
        "tinyui_combo_box_create",
        "tinyui_calendar_create",
        "tinyui_scroll_selecter_create",
        "tinyui_date_time_create",
        "tinyui_message_box_create",
        "tinyui_graph_create",
        "tinyui_table_create",
        "tinyui_radial_menu_create",
        "tinyui_icon_slider_create",
        "tinyui_qrcode_create",
        "tinyui_gauge_create",
        "tinyui_line_edit_create",
        "tinyui_keyboard_create",
        "tinyui_arc_create",
        "tinyui_list_set_item_widget",
        "tinyui_window_create_child",
        "tinyui_image_set_source",
        "tinyui_button_set_image",
        "tinyui_progress_bar_set_image",
        "tinyui_slider_set_image",
        "tinyui_text_set_background_source",
        "tinyui_radial_menu_add_item_with_source",
        "tinyui_icon_slider_add_item_with_source",
        "tinyui_gauge_set_bg_source",
        "tinyui_gauge_set_pointer_source",
        "tinyui_arc_set_quarter_source",
    ),
    "layout_parity": (
        "tinyui_window_set_padding",
        "tinyui_flex_set_flow",
        "tinyui_text_set_text",
    ),
    "grid_parity": (
        "tinyui_window_create_child",
        "tinyui_grid_set_columns",
        "tinyui_grid_set_rows",
        "tinyui_widget_set_grid_cell",
        "tinyui_widget_set_ignore_layout",
        "{92, -2, -3, 0}",
        "{54, 66, -3, 0}",
    ),
}

DEMO_ABSENT_MARKERS = {
    "grid_parity": (
        'tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_g',
    ),
}


def main() -> int:
    cmake_text = SDL_CMAKE.read_text(encoding="utf-8")
    assert f"add_tinyui_demo({REQUIRED_RUNNER_TARGET}" in cmake_text, (
        f"missing single TinyUI demo runner target: {REQUIRED_RUNNER_TARGET}"
    )
    for demo_name in ("legacy_widget_parity", "layout_parity", "grid_parity"):
        assert f"{demo_name}/{demo_name}.c" in cmake_text, (
            f"single TinyUI demo runner does not compile parity demo: {demo_name}"
        )

    demo_sources = sorted(DEMO_DIR.glob("**/*.c"))
    assert demo_sources, "expected TinyUI demo sources"

    found_demos = {source.parent.name for source in demo_sources}
    missing = sorted(REQUIRED_DEMOS - found_demos)
    assert not missing, f"missing TinyUI demos: {', '.join(missing)}"

    for source in demo_sources:
        text = source.read_text(encoding="utf-8")
        text = OPAQUE_ASSET_DECL_RE.sub("", text)
        for label, pattern in FORBIDDEN_PATTERNS.items():
            match = pattern.search(text)
            assert match is None, f"{source.name} leaks forbidden token: {match.group(0)} ({label})"

    for demo_name, markers in DEMO_MARKERS.items():
        demo_source = DEMO_DIR / demo_name / f"{demo_name}.c"
        assert demo_source.exists(), f"missing parity demo source: {demo_source.relative_to(ROOT)}"
        demo_text = demo_source.read_text(encoding="utf-8")
        for marker in markers:
            assert marker in demo_text, f"{demo_name} missing marker: {marker}"
        for absent in DEMO_ABSENT_MARKERS.get(demo_name, ()):
            assert absent not in demo_text, f"{demo_name} should not contain marker: {absent}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
