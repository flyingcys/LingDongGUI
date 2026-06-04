from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui" / "demo"
SDL_CMAKE = ROOT / "examples" / "sdl" / "CMakeLists.txt"
FORBIDDEN_PATTERNS = {
    "ld*": re.compile(r"\bld[A-Za-z0-9_]+\b"),
    "arm_2d_*": re.compile(r"\barm_2d_[A-Za-z0-9_]+\b"),
    "SIGNAL_*": re.compile(r"\bSIGNAL_[A-Za-z0-9_]+\b"),
}
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
REQUIRED_TARGETS = (
    "picoui_legacy_widget_parity_demo",
    "picoui_layout_parity_demo",
    "picoui_grid_parity_demo",
)
DEMO_MARKERS = {
    "legacy_widget_parity": (
        "picoui_image_create",
        "picoui_switch_create",
        "picoui_checkbox_create",
        "picoui_slider_create",
        "picoui_button_create",
        "picoui_text_create",
        "picoui_progress_bar_create",
        "picoui_list_create",
        "picoui_combo_box_create",
        "picoui_calendar_create",
        "picoui_scroll_selecter_create",
        "picoui_date_time_create",
        "picoui_message_box_create",
        "picoui_graph_create",
        "picoui_table_create",
        "picoui_radial_menu_create",
        "picoui_icon_slider_create",
        "picoui_qrcode_create",
        "picoui_gauge_create",
        "picoui_line_edit_create",
        "picoui_keyboard_create",
        "picoui_arc_create",
        "picoui_list_set_item_widget",
        "picoui_window_create_child",
        "picoui_image_set_source",
        "picoui_button_set_image",
        "picoui_progress_bar_set_image",
        "picoui_slider_set_image",
        "picoui_text_set_background_source",
        "picoui_radial_menu_add_item_with_source",
        "picoui_icon_slider_add_item_with_source",
        "picoui_gauge_set_bg_source",
        "picoui_gauge_set_pointer_source",
        "picoui_arc_set_quarter_source",
    ),
    "layout_parity": (
        "picoui_window_set_padding_group",
        "picoui_flex_set_flow",
        "picoui_text_set_text",
    ),
    "grid_parity": (
        "picoui_window_create_child",
        "picoui_grid_set_columns",
        "picoui_grid_set_rows",
        "picoui_widget_set_grid_cell",
        "picoui_widget_set_ignore_layout",
        "{92, -2, -3, 0}",
        "{54, 66, -3, 0}",
    ),
}

DEMO_ABSENT_MARKERS = {
    "grid_parity": (
        'picoui_widget_set_grid_cell((struct picoui_widget *)panel_g',
    ),
}


def main() -> int:
    cmake_text = SDL_CMAKE.read_text(encoding="utf-8")
    for target in REQUIRED_TARGETS:
        assert target in cmake_text, f"missing parity demo target: {target}"

    demo_sources = sorted(DEMO_DIR.glob("**/*.c"))
    assert demo_sources, "expected PicoUI demo sources"

    found_demos = {source.parent.name for source in demo_sources}
    missing = sorted(REQUIRED_DEMOS - found_demos)
    assert not missing, f"missing PicoUI demos: {', '.join(missing)}"

    for source in demo_sources:
        text = source.read_text(encoding="utf-8")
        for label, pattern in FORBIDDEN_PATTERNS.items():
            match = pattern.search(text)
            assert match is None, f"{source.name} leaks forbidden token: {match.group(0)} ({label})"

    for demo_name, markers in DEMO_MARKERS.items():
        demo_source = DEMO_DIR / demo_name / "main.c"
        assert demo_source.exists(), f"missing parity demo source: {demo_source.relative_to(ROOT)}"
        demo_text = demo_source.read_text(encoding="utf-8")
        for marker in markers:
            assert marker in demo_text, f"{demo_name} missing marker: {marker}"
        for absent in DEMO_ABSENT_MARKERS.get(demo_name, ()):
            assert absent not in demo_text, f"{demo_name} should not contain marker: {absent}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
