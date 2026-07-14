import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_tinyui_visible_ui as checker


def _write_grid_parity_capture(path: Path, *, include_panels: bool = True) -> None:
    width, height = 480, 320
    background = (240, 244, 248)
    pixels = bytearray(bytes(background) * (width * height))

    def fill(x0: int, y0: int, x1: int, y1: int, color: tuple[int, int, int]) -> None:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                offset = (y * width + x) * 3
                pixels[offset:offset + 3] = bytes(color)

    # The real capture has one continuous canvas; the grid structure is encoded
    # by separated panel rectangles inside that canvas.
    fill(12, 74, 467, 277, (232, 236, 240))
    if include_panels:
        fill(24, 86, 115, 139, (216, 224, 240))
        fill(128, 86, 239, 139, (216, 224, 240))
        fill(252, 86, 455, 139, (216, 224, 240))
        fill(24, 152, 239, 217, (216, 224, 240))
        fill(252, 152, 455, 265, (216, 224, 240))
        fill(24, 230, 115, 265, (216, 224, 240))
        fill(156, 234, 211, 261, (216, 224, 240))
        fill(332, 222, 447, 261, (24, 52, 80))

    path.write_bytes(f"P6\n{width} {height}\n255\n".encode() + pixels)


def _write_message_box_capture(path: Path, *, include_button: bool = True) -> None:
    width, height = 480, 320
    page_background = (240, 240, 240)
    pixels = bytearray(bytes(page_background) * (width * height))

    def fill(x0: int, y0: int, x1: int, y1: int, color: tuple[int, int, int]) -> None:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                offset = (y * width + x) * 3
                pixels[offset:offset + 3] = bytes(color)

    # The real capture uses a continuous dialog body with a single bottom button.
    fill(0, 0, 219, 27, (248, 252, 248))
    fill(110, 181, 369, 319, (248, 252, 248))
    if include_button:
        fill(122, 286, 357, 309, (216, 224, 240))

    path.write_bytes(f"P6\n{width} {height}\n255\n".encode() + pixels)


class GridParityVisibleCheckerTest(unittest.TestCase):
    def test_accepts_real_continuous_canvas_structure(self) -> None:
        with tempfile.TemporaryDirectory(prefix="grid-parity-visible-") as tmpdir:
            capture = Path(tmpdir) / "grid_parity.ppm"
            _write_grid_parity_capture(capture)
            checker._assert_grid_parity_visible(capture)

    def test_rejects_flat_continuous_canvas_without_grid_panels(self) -> None:
        with tempfile.TemporaryDirectory(prefix="grid-parity-flat-") as tmpdir:
            capture = Path(tmpdir) / "grid_parity.ppm"
            _write_grid_parity_capture(capture, include_panels=False)
            with self.assertRaisesRegex(AssertionError, "panel structure"):
                checker._assert_grid_parity_visible(capture)


class MessageBoxVisibleCheckerTest(unittest.TestCase):
    def test_accepts_real_continuous_dialog_with_bottom_button(self) -> None:
        with tempfile.TemporaryDirectory(prefix="message-box-visible-") as tmpdir:
            capture = Path(tmpdir) / "message_box.ppm"
            _write_message_box_capture(capture)
            checker._assert_message_box_basic_visible(capture)

    def test_rejects_dialog_without_bottom_button(self) -> None:
        with tempfile.TemporaryDirectory(prefix="message-box-no-button-") as tmpdir:
            capture = Path(tmpdir) / "message_box.ppm"
            _write_message_box_capture(capture, include_button=False)
            with self.assertRaisesRegex(AssertionError, "button"):
                checker._assert_message_box_basic_visible(capture)


if __name__ == "__main__":
    unittest.main()
