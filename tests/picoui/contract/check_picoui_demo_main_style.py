from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BASIC_WIDGETS = ROOT / "picoui/demo/basic_widgets/main.c"


def main() -> int:
    text = BASIC_WIDGETS.read_text(encoding="utf-8")
    required = [
        "static void create_demo_ui(",
        "static struct picoui_display *hal_init(",
        "picoui_init();",
        "hal_init(320, 480);",
        "create_demo_ui();",
        "while (1)",
        "picoui_timer_handler();",
    ]
    for marker in required:
        assert marker in text, f"basic_widgets main missing marker: {marker}"

    forbidden_main_path = [
        "picoui_app_create()",
        "picoui_app_run(",
        "picoui_app_destroy(",
    ]
    main_start = text.index("int main(")
    main_text = text[main_start:]
    for marker in forbidden_main_path:
        assert marker not in main_text, f"basic_widgets main path still uses {marker}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
