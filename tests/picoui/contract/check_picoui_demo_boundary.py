from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui" / "demo"
FORBIDDEN = ["ld", "arm_2d_", "SIGNAL_"]
REQUIRED_DEMOS = {
    "hello_world",
    "basic_widgets",
    "layout_flex",
    "layout_grid",
    "theme_showcase",
    "settings_panel",
    "list_basic",
}


def main() -> int:
    demo_sources = sorted(DEMO_DIR.glob("**/*.c"))
    assert demo_sources, "expected PicoUI demo sources"

    found_demos = {source.parent.name for source in demo_sources}
    missing = sorted(REQUIRED_DEMOS - found_demos)
    assert not missing, f"missing PicoUI demos: {', '.join(missing)}"

    for source in demo_sources:
        text = source.read_text(encoding="utf-8")
        for needle in FORBIDDEN:
            assert needle not in text, f"{source.name} leaks forbidden token: {needle}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
