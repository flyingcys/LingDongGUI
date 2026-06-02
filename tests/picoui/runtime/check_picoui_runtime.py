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
    "picoui_list_basic_demo",
    "picoui_progress_bar_basic_demo",
    "picoui_arc_basic_demo",
    "picoui_gauge_basic_demo",
    "picoui_icon_slider_basic_demo",
    "picoui_radial_menu_basic_demo",
    "picoui_progress_wheel_basic_demo",
    "picoui_qrcode_basic_demo",
    "picoui_message_box_basic_demo",
    "picoui_date_time_basic_demo",
    "picoui_clock_basic_demo",
    "picoui_keyboard_basic_demo",
    "picoui_line_edit_basic_demo",
    "picoui_combo_box_basic_demo",
    "picoui_scroll_selecter_basic_demo",
    "picoui_table_basic_demo",
    "picoui_graph_basic_demo",
    "picoui_calendar_basic_demo",
    "picoui_animation_basic_demo",
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


def _non_background_bounds(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
) -> tuple[int, int, int, int]:
    xs: list[int] = []
    ys: list[int] = []
    for y in range(height):
        for x in range(width):
            if _pixel(width, pixels, x, y) != bg:
                xs.append(x)
                ys.append(y)
    if not xs:
        raise AssertionError("basic_widgets capture has no non-background pixels")
    return min(xs), min(ys), max(xs), max(ys)


def _row_runs_with_content(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
    *,
    min_pixels: int,
) -> list[tuple[int, int]]:
    rows: list[int] = []
    runs: list[tuple[int, int]] = []

    for y in range(height):
        count = 0
        for x in range(width):
            if _pixel(width, pixels, x, y) != bg:
                count += 1
        if count >= min_pixels:
            rows.append(y)

    if not rows:
        return runs

    start = rows[0]
    previous = rows[0]
    for value in rows[1:]:
        if value == previous + 1:
            previous = value
            continue
        runs.append((start, previous))
        start = value
        previous = value
    runs.append((start, previous))
    return runs


def _band_signature(
    width: int,
    pixels: bytes,
    band: tuple[int, int],
    bg: tuple[int, int, int],
) -> tuple[int, int, int, int]:
    non_bg = 0
    blueish = 0
    greenish = 0
    height = band[1] - band[0] + 1

    for y in range(band[0], band[1] + 1):
        for x in range(width):
            color = _pixel(width, pixels, x, y)
            if color == bg:
                continue
            non_bg += 1
            if color[2] > color[1] and color[2] > color[0]:
                blueish += 1
            if color[1] > color[0] and color[1] > color[2]:
                greenish += 1

    return non_bg, blueish, greenish, height


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


def _assert_no_smoke_layout(target: str, stdout: str, stderr: str) -> None:
    marker = "PICOUI_SMOKE_LAYOUT_USED="
    for line in stdout.splitlines():
        if not line.startswith(marker):
            continue
        value = line[len(marker) :].strip()
        if value != "0":
            raise AssertionError(
                f"Formal native demo '{target}' used temporary smoke layout.\n"
                f"expected: PICOUI_SMOKE_LAYOUT_USED=0\n"
                f"actual: {line}\n"
                f"stdout:\n{stdout}\n"
                f"stderr:\n{stderr}"
            )
        return
    raise AssertionError(
        f"Formal native demo '{target}' did not report PICOUI_SMOKE_LAYOUT_USED.\n"
        f"stdout:\n{stdout}\n"
        f"stderr:\n{stderr}"
    )


def _assert_basic_widgets_capture(path: Path, stdout: str) -> None:
    width, height, pixels = _read_ppm(path)
    assert width == 480 and height == 320, f"unexpected basic widgets capture size: {width}x{height}"

    bg = _pixel(width, pixels, width - 8, height - 8)
    min_x, min_y, max_x, max_y = _non_background_bounds(width, height, pixels, bg)
    assert (max_x - min_x + 1) >= 180, (
        "basic_widgets should occupy a readable horizontal span, "
        f"bounds=({min_x},{min_y})-({max_x},{max_y})"
    )
    assert (max_y - min_y + 1) >= 180, (
        "basic_widgets should occupy a readable vertical span, "
        f"bounds=({min_x},{min_y})-({max_x},{max_y})"
    )

    content_runs = [
        run for run in _row_runs_with_content(width, height, pixels, bg, min_pixels=8)
        if (run[1] - run[0] + 1) >= 10
    ]
    assert len(content_runs) >= 5, f"basic_widgets should expose at least five visible content bands, row_runs={content_runs}"

    band_signatures = [(band, _band_signature(width, pixels, band, bg)) for band in content_runs]

    switch_band = max(band_signatures, key=lambda item: item[1][2])[0]
    checkbox_candidates = [
        item for item in band_signatures
        if item[0] != switch_band and item[1][3] <= 18 and item[1][1] > 0
    ]
    assert checkbox_candidates, f"basic_widgets should expose a checkbox-like band, signatures={band_signatures}"
    checkbox_band = min(checkbox_candidates, key=lambda item: item[1][0])[0]
    slider_candidates = [
        item for item in band_signatures
        if item[0] not in (switch_band, checkbox_band) and item[1][1] > 0 and item[1][2] > 0
    ]
    assert slider_candidates, f"basic_widgets should expose a slider-like band, signatures={band_signatures}"
    slider_band = max(slider_candidates, key=lambda item: item[1][0])[0]
    remaining_bands = [band for band in content_runs if band not in (switch_band, checkbox_band, slider_band)]
    assert len(remaining_bands) >= 2, f"basic_widgets should leave image/text bands after control detection, remaining={remaining_bands}"
    image_band = max(remaining_bands, key=lambda band: (_band_signature(width, pixels, band, bg)[1], _band_signature(width, pixels, band, bg)[0]))
    text_band = min((band for band in remaining_bands if band != image_band), key=lambda band: band[0])

    switch_colors = _region_colors(width, pixels, min_x, switch_band[0], max_x + 1, switch_band[1] + 1, bg)
    checkbox_colors = _region_colors(width, pixels, min_x, checkbox_band[0], max_x + 1, checkbox_band[1] + 1, bg)
    slider_colors = _region_colors(width, pixels, min_x, slider_band[0], max_x + 1, slider_band[1] + 1, bg)
    text_colors = _region_colors(width, pixels, min_x, text_band[0], max_x + 1, text_band[1] + 1, bg)
    image_colors = _region_colors(width, pixels, min_x, image_band[0], max_x + 1, image_band[1] + 1, bg)

    assert len(switch_colors) >= 2, f"switch should show real track/knob contrast, colors={sorted(switch_colors)}"
    assert len(checkbox_colors) >= 2, f"checkbox should show real edge/fill contrast, colors={sorted(checkbox_colors)}"
    assert len(slider_colors) >= 2, f"slider should show real active/idle contrast, colors={sorted(slider_colors)}"
    assert text_colors, "text band should contain visible non-background pixels"
    assert image_colors, "image band should contain visible non-background pixels"
    assert text_colors != image_colors, (
        "text/image bands should not collapse into the same visual treatment, "
        f"text_colors={sorted(text_colors)}, image_colors={sorted(image_colors)}"
    )
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
    _assert_no_smoke_layout(target, completed.stdout, completed.stderr)
