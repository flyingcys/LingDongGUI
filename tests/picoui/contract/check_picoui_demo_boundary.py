from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui" / "demo"
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
}


def main() -> int:
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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
