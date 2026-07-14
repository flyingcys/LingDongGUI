import sys
import tempfile
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import check_tinyui_runtime as runtime


WIDTH = 480
HEIGHT = 320
BACKGROUND = (240, 240, 240)


def _offset(x, y):
    return (y * WIDTH + x) * 3


def _fill_rect(pixels, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            pixels[_offset(x, y) : _offset(x, y) + 3] = bytes(color)


def _synthetic_basic_widgets_capture():
    pixels = bytearray(bytes(BACKGROUND) * (WIDTH * HEIGHT))

    switch_color = (32, 148, 240)
    _fill_rect(pixels, 16, 24, 63, 47, (120, 120, 120))
    for y in range(24, 48):
        for x in range(16, 64):
            if (x, y) not in ((16, 24), (16, 47), (63, 24), (63, 47)):
                pixels[_offset(x, y) : _offset(x, y) + 3] = bytes(switch_color)

    _fill_rect(pixels, 16, 68, 29, 81, (0, 0, 0))
    _fill_rect(pixels, 17, 69, 28, 80, (248, 252, 248))
    _fill_rect(pixels, 32, 70, 38, 71, (0, 0, 0))
    _fill_rect(pixels, 42, 76, 48, 77, (0, 0, 0))

    _fill_rect(pixels, 16, 102, 235, 131, (160, 160, 160))
    _fill_rect(pixels, 110, 114, 119, 118, (160, 168, 160))
    _fill_rect(pixels, 16, 114, 100, 119, (0, 120, 216))
    _fill_rect(pixels, 16, 144, 175, 179, (216, 224, 240))
    _fill_rect(pixels, 16, 192, 235, 219, (248, 252, 248))
    _fill_rect(pixels, 32, 198, 44, 210, (0, 0, 0))

    header = f"P6\n{WIDTH} {HEIGHT}\n255\n".encode("ascii")
    return header + bytes(pixels)


class BasicWidgetsCaptureTest(unittest.TestCase):
    def test_neutral_checkbox_band_is_detected_without_blue_pixels(self):
        with tempfile.NamedTemporaryFile(suffix=".ppm") as capture:
            capture.write(_synthetic_basic_widgets_capture())
            capture.flush()
            runtime._assert_basic_widgets_capture(
                Path(capture.name),
                "\n".join(
                    (
                        "TINYUI_SMOKE_LAYOUT_USED=0",
                        "TINYUI_BACKEND_REAL_WIDGET_IDS=wifi,agree,volume,submit,title,logo",
                    )
                ),
            )


if __name__ == "__main__":
    unittest.main()
