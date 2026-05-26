import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD = ROOT / "build" / "picoui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 6
TARGETS = [
    "picoui_hello_world_demo",
    "picoui_basic_widgets_demo",
    "picoui_layout_flex_demo",
    "picoui_layout_grid_demo",
    "picoui_theme_showcase_demo",
    "picoui_settings_panel_demo",
]


def _read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    header, pixels = data.split(b"\n255\n", 1)
    _, dims = header.split(b"\n", 1)
    width_token, height_token = dims.split()
    return int(width_token), int(height_token), pixels


def _pixel(width: int, pixels: bytes, x: int, y: int) -> tuple[int, int, int]:
    offset = (y * width + x) * 3
    return pixels[offset], pixels[offset + 1], pixels[offset + 2]


def _assert_basic_widgets_capture(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    assert width == 480 and height == 320, f"unexpected basic widgets capture size: {width}x{height}"

    bg = _pixel(width, pixels, 8, 8)
    button_fill = _pixel(width, pixels, 40, 170)
    text_fill = _pixel(width, pixels, 40, 210)
    image_fill = _pixel(width, pixels, 40, 260)
    image_edge = _pixel(width, pixels, 16, 260)

    assert button_fill != bg, f"button row should not match background: {button_fill} vs {bg}"
    assert text_fill != bg, f"text row should not match background: {text_fill} vs {bg}"
    assert image_fill != bg, f"image row should not match background: {image_fill} vs {bg}"
    assert button_fill != text_fill, f"button/text rows should differ: {button_fill} vs {text_fill}"
    assert image_edge != image_fill, f"image edge/fill should differ: {image_edge} vs {image_fill}"

subprocess.run([
    RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"
], check=True)
subprocess.run([
    RTK, "cmake", "--build", str(BUILD), "--target", *TARGETS
], check=True)

for target in TARGETS:
    candidates = [
        BUILD / "examples" / "sdl" / target,
        BUILD / target,
        BUILD / "examples" / target,
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        candidate_paths = ", ".join(str(path) for path in candidates)
        raise FileNotFoundError(
            f"Could not find executable for target '{target}'. Checked: {candidate_paths}"
        )

    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
    with tempfile.TemporaryDirectory(prefix=f"{target}-") as tmpdir:
        capture_path = Path(tmpdir) / "frame.ppm"
        env["PICOUI_CAPTURE_FILE"] = str(capture_path)
        completed = subprocess.run(
            [str(executable)],
            check=False,
            timeout=DEMO_TIMEOUT_SECONDS,
            capture_output=True,
            text=True,
            env=env,
        )
        if not capture_path.is_file() or capture_path.stat().st_size <= 32:
            raise AssertionError(
                f"Demo '{target}' did not produce a capture frame.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        if target == "picoui_basic_widgets_demo":
            _assert_basic_widgets_capture(capture_path)
    if completed.returncode != 0:
        raise RuntimeError(
            f"Demo '{target}' exited with {completed.returncode}.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )
    if "PICOUI_RUNTIME_READY" not in completed.stdout:
        raise AssertionError(
            f"Demo '{target}' did not report entering a visible runtime loop.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )
