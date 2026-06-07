from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
TEXT = (ROOT / "cmake/LingDongGUI.cmake").read_text(encoding="utf-8")


def main() -> int:
    assert "add_library(picoui_core_v1_1 STATIC" in TEXT
    assert "add_library(picoui_port_v1_1 INTERFACE" in TEXT
    assert 'OUTPUT "${LD_PICOUI_V1_1_PLACEHOLDER_DIR}/picoui_widgets_v1_1_placeholder.c"' in TEXT

    widget_block_match = re.search(
        r"add_library\(picoui_widgets_v1_1 STATIC(?P<body>.*?)\n\s*\)\n"
        r"(?P<tail>.*?target_link_libraries\(picoui_widgets_v1_1 PUBLIC picoui_core_v1_1\))",
        TEXT,
        re.DOTALL,
    )
    assert widget_block_match is not None
    widget_block = widget_block_match.group("body")
    widget_tail = widget_block_match.group("tail")

    assert "picoui_widgets_v1_1_placeholder.c" in widget_block
    assert "picoui/src/widgets/" not in widget_block
    assert "target_link_libraries(picoui_widgets_v1_1 PUBLIC picoui_core_v1_1)" in widget_tail

    port_block_match = re.search(
        r"add_library\(picoui_port_v1_1 INTERFACE\)(?P<body>.*?)(?:\n\s*add_library\(|\n\s*if\(|\n\s*endfunction\()",
        TEXT,
        re.DOTALL,
    )
    assert port_block_match is not None
    port_block = port_block_match.group("body")

    assert "picoui_port_sdl" not in port_block
    assert "picoui_core" not in port_block
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
