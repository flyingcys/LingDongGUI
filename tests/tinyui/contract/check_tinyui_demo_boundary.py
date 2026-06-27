from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "tinyui" / "demo"
DEMO_RUNNER_DIR = ROOT / "tinyui_demo"
DEMO_GUIDE = ROOT / "tinyui" / "docs" / "demo_guide.md"
SDL_CMAKE = ROOT / "examples" / "sdl" / "CMakeLists.txt"
FORBIDDEN_PATTERNS = {
    "LingDongGUI": re.compile(r"LingDongGUI"),
    "lingdonggui": re.compile(r"lingdonggui"),
    "Arm-2D": re.compile(r"Arm-2D"),
    "arm2d": re.compile(r"arm2d", re.I),
    "arm_2d": re.compile(r"arm_2d[A-Za-z0-9_]*"),
    "SIGNAL_*": re.compile(r"\bSIGNAL_[A-Za-z0-9_]+\b"),
    "ld*": re.compile(r"\bld(?:[A-Za-z0-9_]*|\*)\b|\bld\*"),
    "c_tile*": re.compile(r"\bc_tile[A-Za-z0-9_]*\b"),
    "IMAGE_*": re.compile(r"\bIMAGE_[A-Za-z0-9_]*\b"),
    ".img_tile": re.compile(r"\.img_tile\b"),
    ".mask_tile": re.compile(r"\.mask_tile\b"),
    "uiImages.h": re.compile(r"uiImages\.h"),
    "src/gui": re.compile(r"src/gui/"),
    "src/misc": re.compile(r"src/misc/"),
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
    "legacy_demo0_parity",
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
    "legacy_demo0_parity": (
        "tinyui_screen_create",
        "tinyui_screen_load",
        "tinyui_image_create",
        "tinyui_button_create",
        "tinyui_switch_create",
        "tinyui_table_create",
        "tinyui_line_edit_create",
        "tinyui_arc_create",
        "tinyui_gauge_create",
        "tinyui_button_set_on_released(button, on_button_released, runtime)",
        "tinyui_widget_set_opacity((struct tinyui_widget *)runtime->image, 128)",
        "tinyui_switch_set_on_toggled(sw, on_switch_toggled, switch_label)",
        "tinyui_table_set_keyboard_binding(table, 22)",
        "tinyui_line_edit_set_keyboard_binding(line_edit, 22)",
        "void tinyui_demo_legacy_demo0_parity_frame(unsigned int elapsed_ms)",
        "tinyui_demo_legacy_demo0_parity_frame",
        "LEGACY_DEMO0_FRAME_INTERVAL_MS",
        "runtime.frame_accumulated_ms",
        "tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle)",
        "tinyui_gauge_set_angle(runtime->gauge, runtime->angle)",
        "10, 10",
        "\\\"123\\\"",
        "100, 120",
        "200, 95",
        "220, 10",
        "300, 10",
        "\\\"123\\\\n12333\\\"",
        "300, 226",
        "356, 218",
        "\\\"OFF\\\"",
        "500, 10",
        "g_legacy_qrcode_truth",
        "{'l', 'd', 'g', 'u', 'i', '\\\\0'}",
        "850, 280",
        "\\\"title\\\"",
        "12345678abcdefg\\\\n99556",
        "\\\"yyyy - mm - dd\\\"",
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
    "legacy_demo0_parity": (
        "tinyui_widget_set_grid_cell",
        "tinyui_flex_set_flow",
        "tinyui_grid_set_columns",
        "tinyui_grid_set_rows",
        "tinyui_widget_set_ignore_layout",
        "tinyui_window_set_padding",
        "tinyui_label_create_with_props",
        "tinyui_text_create_with_props",
        "tinyui_image_create_with_props",
        "tinyui_button_create_with_props",
        "tinyui_checkbox_create_with_props",
        "tinyui_switch_create_with_props",
        "tinyui_slider_create_with_props",
        "tinyui_progress_bar_create_with_props",
        "tinyui_list_create_with_props",
        "tinyui_combo_box_create_with_props",
        "tinyui_calendar_create_with_props",
        "tinyui_clock_create_with_props",
        "tinyui_message_box_create_with_props",
        "tinyui_graph_create_with_props",
        "tinyui_table_create_with_props",
        "tinyui_radial_menu_create_with_props",
        "tinyui_icon_slider_create_with_props",
        "tinyui_qrcode_create_with_props",
        "tinyui_gauge_create_with_props",
        "tinyui_line_edit_create_with_props",
        "tinyui_keyboard_create_with_props",
        "tinyui_arc_create_with_props",
        "tinyui_scroll_selecter_create_with_props",
        "tinyui_date_time_create_with_props",
        "tinyui_animation_create_with_props",
        "tinyui_window_create_with_props",
        "tinyui_flex_set_align",
        "tinyui_flex_set_gap",
        "tick_runtime(&runtime)",
        "tinyui_label_set_align(label, TINYUI_ALIGN_END)",
        "tinyui_label_set_align(switch_label, TINYUI_ALIGN_CENTER)",
    ),
    "grid_parity": (
        'tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_g',
    ),
}


def main() -> int:
    cmake_text = SDL_CMAKE.read_text(encoding="utf-8")
    assert f"add_tinyui_demo({REQUIRED_RUNNER_TARGET}" in cmake_text, (
        f"missing single TinyUI demo runner target: {REQUIRED_RUNNER_TARGET}"
    )
    for demo_name in ("legacy_widget_parity", "layout_parity", "grid_parity", "legacy_demo0_parity"):
        assert f"{demo_name}/{demo_name}.c" in cmake_text, (
            f"single TinyUI demo runner does not compile parity demo: {demo_name}"
        )

    demos_h_text = (ROOT / "tinyui" / "demo" / "tinyui_demos.h").read_text(encoding="utf-8")
    assert "void tinyui_demo_legacy_demo0_parity(void);" in demos_h_text, (
        "tinyui_demos.h missing legacy_demo0_parity declaration"
    )
    assert "typedef void (*tinyui_demo_frame_cb_t)(unsigned int elapsed_ms);" in demos_h_text, (
        "tinyui_demos.h missing demo frame callback type"
    )
    assert "void tinyui_demos_frame(unsigned int elapsed_ms);" in demos_h_text, (
        "tinyui_demos.h missing per-frame dispatcher declaration"
    )

    demos_c_text = (ROOT / "tinyui" / "demo" / "tinyui_demos.c").read_text(encoding="utf-8")
    assert '"legacy_demo0_parity"' in demos_c_text, (
        "tinyui_demos.c missing legacy_demo0_parity registration name"
    )
    assert "tinyui_demo_legacy_demo0_parity" in demos_c_text, (
        "tinyui_demos.c missing legacy_demo0_parity entry point registration"
    )
    assert "tinyui_demo_legacy_demo0_parity_frame" in demos_c_text, (
        "tinyui_demos.c missing legacy_demo0_parity frame callback registration"
    )
    assert "tinyui_demo_frame_cb_t frame_cb;" in demos_c_text, (
        "tinyui_demos.c missing optional frame callback in demo registry"
    )
    assert "void tinyui_demos_frame(unsigned int elapsed_ms)" in demos_c_text, (
        "tinyui_demos.c missing per-frame dispatcher"
    )

    runner_text = (DEMO_RUNNER_DIR / "main.c").read_text(encoding="utf-8")
    assert "tinyui_demos_frame(elapsed_ms);" in runner_text, (
        "tinyui_demo/main.c must call demo frame hook every main-loop iteration"
    )

    demo_sources = sorted(DEMO_DIR.glob("**/*.c"))
    assert demo_sources, "expected TinyUI demo sources"

    found_demos = {source.parent.name for source in demo_sources}
    missing = sorted(REQUIRED_DEMOS - found_demos)
    assert not missing, f"missing TinyUI demos: {', '.join(missing)}"

    boundary_sources = sorted(DEMO_DIR.glob("**/*.[ch]"))
    boundary_sources.extend(sorted(DEMO_RUNNER_DIR.glob("**/*.[ch]")))
    boundary_sources.append(DEMO_GUIDE)
    assert boundary_sources, "expected TinyUI demo boundary sources"

    for source in boundary_sources:
        assert source.exists(), f"missing TinyUI demo boundary source: {source.relative_to(ROOT)}"
        text = source.read_text(encoding="utf-8")
        for label, pattern in FORBIDDEN_PATTERNS.items():
            match = pattern.search(text)
            rel = source.relative_to(ROOT)
            assert match is None, (
                f"{rel} leaks forbidden token: {match.group(0)} ({label})"
            )

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
