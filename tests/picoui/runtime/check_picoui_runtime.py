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


def _region_colors(
    width: int,
    pixels: bytes,
    x0: int,
    y0: int,
    x1: int,
    y1: int,
    bg: tuple[int, int, int],
) -> set[tuple[int, int, int]]:
    colors: set[tuple[int, int, int]] = set()
    for y in range(y0, y1):
        for x in range(x0, x1):
            color = _pixel(width, pixels, x, y)
            if color != bg:
                colors.add(color)
    return colors


def _parse_marker_ids(stdout: str, marker: str, *, required: bool = True) -> set[str]:
    prefix = f"{marker}="
    for line in stdout.splitlines():
        if line.startswith(prefix):
            value = line[len(prefix) :].strip()
            if not value:
                return set()
            return {item.strip() for item in value.split(",") if item.strip()}
    if required:
        raise AssertionError(f"Missing marker line '{marker}='.\nstdout:\n{stdout}")
    return set()


def _assert_basic_widgets_capture(path: Path, stdout: str) -> None:
    width, height, pixels = _read_ppm(path)
    assert width == 480 and height == 320, f"unexpected basic widgets capture size: {width}x{height}"

    bg = _pixel(width, pixels, 8, 8)
    switch_colors = _region_colors(width, pixels, 16, 32, 120, 72, bg)
    checkbox_colors = _region_colors(width, pixels, 16, 80, 80, 118, bg)
    slider_colors = _region_colors(width, pixels, 24, 120, 160, 144, bg)
    button_fill = _pixel(width, pixels, 40, 170)
    text_fill = _pixel(width, pixels, 40, 220)

    assert len(switch_colors) >= 2, f"switch should show real track/knob contrast, colors={sorted(switch_colors)}"
    assert len(checkbox_colors) >= 2, f"checkbox should show real edge/fill contrast, colors={sorted(checkbox_colors)}"
    assert len(slider_colors) >= 2, f"slider should show real active/idle contrast, colors={sorted(slider_colors)}"
    assert button_fill != bg, f"button row should not match background: {button_fill} vs {bg}"
    assert text_fill != bg, f"text row should not match background: {text_fill} vs {bg}"
    assert button_fill != text_fill, f"button/text rows should differ: {button_fill} vs {text_fill}"
    real_ids = _parse_marker_ids(stdout, "PICOUI_BACKEND_REAL_WIDGET_IDS")
    fallback_ids = _parse_marker_ids(stdout, "PICOUI_BACKEND_FALLBACK_WIDGET_IDS", required=False)
    assert "logo" in real_ids, f"basic_widgets should keep logo in REAL widget ids: {sorted(real_ids)}"
    expected_interactive = {"wifi", "agree", "volume"}
    missing_interactive = expected_interactive - real_ids
    unexpected_fallback = expected_interactive & fallback_ids
    assert not missing_interactive, (
        "basic_widgets interactive widgets should move to REAL widget ids, "
        f"missing: {sorted(missing_interactive)}, real_ids={sorted(real_ids)}"
    )
    assert not unexpected_fallback, (
        "basic_widgets interactive widgets should stop using fallback ids, "
        f"fallback still contains: {sorted(unexpected_fallback)}, "
        f"fallback_ids={sorted(fallback_ids)}"
    )

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
            _assert_basic_widgets_capture(capture_path, completed.stdout)
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
