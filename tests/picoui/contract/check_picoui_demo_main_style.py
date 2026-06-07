from pathlib import Path
import json


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui/demo"
INVENTORY = ROOT / "tests/picoui/contract/picoui_demo_main_style_inventory.json"


def _assert_lvgl_like_demo(name: str, text: str, main_text: str) -> None:
    assert "picoui_init();" in text, f"{name} main missing marker: picoui_init();"
    assert (
        "picoui_sdl_hal_init(320, 480)" in text or "hal_init(320, 480);" in text
    ), f"{name} main missing HAL init marker"
    assert "create_demo_ui(" in text, f"{name} main missing marker: create_demo_ui("
    assert "while (1)" in main_text, f"{name} main missing marker: while (1)"
    assert "picoui_timer_handler();" in text, f"{name} main missing marker: picoui_timer_handler();"

    forbidden_main_path = [
        "picoui_app_create()",
        "picoui_app_run(",
        "picoui_app_destroy(",
    ]
    for marker in forbidden_main_path:
        assert marker not in main_text, f"{name} main path still uses {marker}"


def _assert_pre_v1_app_run_demo(name: str, text: str) -> None:
    required_file_markers = [
        "picoui_app_create()",
        "picoui_app_run(",
    ]
    for marker in required_file_markers:
        assert marker in text, f"{name} pre_v1 demo should still contain {marker}"


def main() -> int:
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    rows = data.get("demos")
    assert isinstance(rows, list), "demos must be a list"

    by_name = {row.get("name"): row for row in rows}
    actual = sorted(path.parent.name for path in DEMO_DIR.glob("*/main.c"))
    missing = [name for name in actual if name not in by_name]
    assert not missing, f"missing demo inventory rows: {missing}"

    pending = []
    for name in actual:
        row = by_name[name]
        text = (DEMO_DIR / name / "main.c").read_text(encoding="utf-8")
        main_start = text.index("int main(")
        main_text = text[main_start:]
        style = row.get("main_style")
        uses = row.get("uses_picoui_app")

        if style == "v1_lvgl_like":
            assert uses is False, f"{name} v1_lvgl_like inventory must set uses_picoui_app=false"
            _assert_lvgl_like_demo(name, text, main_text)
        elif style == "pre_v1_app_run":
            assert uses is True, f"{name} pre_v1_app_run inventory must set uses_picoui_app=true"
            _assert_pre_v1_app_run_demo(name, text)
            pending.append(name)
        else:
            raise AssertionError(f"{name} invalid main_style={style!r}")

    assert not pending, (
        "demo mains still pending P6 LVGL-like migration: "
        + ", ".join(pending)
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
