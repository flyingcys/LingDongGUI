import argparse
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "tinyui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 6
DEMO_TARGET = "tinyui_demo"
RUNTIME_SCREEN_DEFINES = {
    "LD_CFG_SCREEN_WIDTH": "480",
    "LD_CFG_SCREEN_HEIGHT": "320",
    "LD_CFG_PFB_WIDTH": "480",
}
ON_TRACK = (33, 150, 243)
BLACK = (0, 0, 0)
DEMOS = [
    "hello_world",
    "basic_widgets",
    "layout_flex",
    "layout_grid",
    "theme_showcase",
    "settings_panel",
    "list_basic",
    "progress_bar_basic",
    "arc_basic",
    "gauge_basic",
    "icon_slider_basic",
    "radial_menu_basic",
    "progress_wheel_basic",
    "qrcode_basic",
    "message_box_basic",
    "date_time_basic",
    "clock_basic",
    "keyboard_basic",
    "line_edit_basic",
    "combo_box_basic",
    "scroll_selecter_basic",
    "table_basic",
    "graph_basic",
    "calendar_basic",
    "animation_basic",
]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build and validate TinyUI SDL runtime demos.")
    parser.add_argument(
        "--demo",
        default="all",
        help="Demo short name such as 'basic_widgets', target name, or 'all'.",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        default=DEFAULT_BUILD,
        help="CMake build directory for the TinyUI runtime demos.",
    )
    return parser.parse_args()


def _demos_for_arg(demo: str) -> list[str]:
    if demo == "all":
        return DEMOS
    if demo in DEMOS:
        return [demo]
    if demo.startswith("tinyui_") and demo.endswith("_demo"):
        short_name = demo[len("tinyui_") : -len("_demo")]
        if short_name in DEMOS:
            return [short_name]
    raise AssertionError(f"Unknown TinyUI runtime demo '{demo}'. Known demos: {', '.join(DEMOS)}")


def _read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    header, pixels = data.split(b"\n255\n", 1)
    _, dims = header.split(b"\n", 1)
    width_token, height_token = dims.split()
    return int(width_token), int(height_token), pixels


def _pixel(width: int, pixels: bytes, x: int, y: int) -> tuple[int, int, int]:
    offset = (y * width + x) * 3
    return pixels[offset], pixels[offset + 1], pixels[offset + 2]

def _assert_not_black_bar(width: int, height: int, pixels: bytes) -> None:
    right_edge = [_pixel(width, pixels, width - 1, y) for y in range(height)]
    bottom_edge = [_pixel(width, pixels, x, height - 1) for x in range(width)]
    assert any(color != BLACK for color in right_edge), "basic_widgets right edge is a black bar"
    assert any(color != BLACK for color in bottom_edge), "basic_widgets bottom edge is a black bar"


def _is_on_track_blue(color: tuple[int, int, int]) -> bool:
    return (
        abs(color[0] - ON_TRACK[0]) <= 10
        and abs(color[1] - ON_TRACK[1]) <= 10
        and abs(color[2] - ON_TRACK[2]) <= 12
    )


def _color_bounds(
    width: int,
    height: int,
    pixels: bytes,
    predicate,
) -> tuple[int, int, int, int]:
    xs: list[int] = []
    ys: list[int] = []
    for y in range(height):
        for x in range(width):
            if predicate(_pixel(width, pixels, x, y)):
                xs.append(x)
                ys.append(y)
    if not xs:
        raise AssertionError("capture does not contain expected color")
    return min(xs), min(ys), max(xs), max(ys)


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


def _assert_optional_benchmark_markers(stdout: str) -> None:
    benchmark_markers = {
        "TINYUI_BENCHMARK_SCREEN_CREATE_MS": float,
        "TINYUI_BENCHMARK_FIRST_FRAME_MS": float,
    }

    for marker, parser in benchmark_markers.items():
        prefix = f"{marker}="
        for line in stdout.splitlines():
            if not line.startswith(prefix):
                continue
            value = line[len(prefix) :].strip()
            try:
                parsed = parser(value)
            except ValueError as exc:
                raise AssertionError(
                    f"benchmark marker '{marker}' is not parseable: {line}\nstdout:\n{stdout}"
                ) from exc
            if parsed < 0:
                raise AssertionError(
                    f"benchmark marker '{marker}' must be non-negative: {line}\nstdout:\n{stdout}"
                )
            break


def _assert_no_smoke_layout(target: str, stdout: str, stderr: str) -> None:
    marker = "TINYUI_SMOKE_LAYOUT_USED="
    for line in stdout.splitlines():
        if not line.startswith(marker):
            continue
        value = line[len(marker) :].strip()
        if value != "0":
            raise AssertionError(
                f"Formal native demo '{target}' used temporary smoke layout.\n"
                f"expected: TINYUI_SMOKE_LAYOUT_USED=0\n"
                f"actual: {line}\n"
                f"stdout:\n{stdout}\n"
                f"stderr:\n{stderr}"
            )
        return
    raise AssertionError(
        f"Formal native demo '{target}' did not report TINYUI_SMOKE_LAYOUT_USED.\n"
        f"stdout:\n{stdout}\n"
        f"stderr:\n{stderr}"
    )


def _assert_basic_widgets_capture(path: Path, stdout: str) -> None:
    width, height, pixels = _read_ppm(path)
    assert width == 480 and height == 320, f"unexpected basic widgets capture size: {width}x{height}"
    _assert_not_black_bar(width, height, pixels)
    assert "TINYUI_SMOKE_LAYOUT_USED=0" in stdout, (
        "basic_widgets must report formal layout usage.\n"
        f"stdout:\n{stdout}"
    )

    switch_bbox = _color_bounds(width, height, pixels, _is_on_track_blue)
    assert switch_bbox == (16, 24, 63, 47), f"unexpected wifi switch bbox: {switch_bbox}"
    for point in ((16, 24), (16, 47), (63, 24), (63, 47)):
        assert not _is_on_track_blue(_pixel(width, pixels, point[0], point[1])), (
            f"wifi switch corner should not be blue at {point}"
        )
    assert _is_on_track_blue(_pixel(width, pixels, 16, 35)), "wifi switch left midpoint should be blue"
    assert _is_on_track_blue(_pixel(width, pixels, 63, 35)), "wifi switch right midpoint should be blue"

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
    real_ids = _parse_marker_ids(stdout, "TINYUI_BACKEND_REAL_WIDGET_IDS")
    fallback_ids = _parse_marker_ids(stdout, "TINYUI_BACKEND_FALLBACK_WIDGET_IDS", required=False)
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


def _compile_commands_for_source(
    compile_commands: list[dict[str, str]],
    source_suffix: str,
) -> list[str]:
    matches = [
        entry.get("command", "")
        for entry in compile_commands
        if entry.get("file", "").endswith(source_suffix)
    ]
    if not matches:
        raise AssertionError(f"missing compile command for {source_suffix}")
    return matches


def _assert_compile_unit_has_screen_defines(
    compile_commands: list[dict[str, str]],
    source_suffix: str,
) -> None:
    matches = _compile_commands_for_source(compile_commands, source_suffix)
    for name, value in RUNTIME_SCREEN_DEFINES.items():
        define = f"-D{name}={value}"
        if not any(define in command for command in matches):
            raise AssertionError(f"{source_suffix} missing {define}")


def _assert_compile_unit_lacks_screen_defines(
    compile_commands: list[dict[str, str]],
    source_suffix: str,
) -> None:
    matches = _compile_commands_for_source(compile_commands, source_suffix)
    for command in matches:
        for name, value in RUNTIME_SCREEN_DEFINES.items():
            define = f"-D{name}={value}"
            if define in command:
                raise AssertionError(f"{source_suffix} should not inherit {define}")


def _assert_tinyui_runtime_screen_defines(build_dir: Path) -> None:
    import json

    compile_db_path = build_dir / "compile_commands.json"
    if not compile_db_path.is_file():
        raise AssertionError(f"missing compile_commands.json: {compile_db_path}")
    compile_commands = json.loads(compile_db_path.read_text())
    for source_suffix in (
        "tinyui/port/sdl/runtime_host.c",
        "tinyui/demo/tinyui_demos.c",
    ):
        _assert_compile_unit_has_screen_defines(compile_commands, source_suffix)
    for source_suffix in (
        "src/gui/ldSwitch.c",
        "src/porting/ldConfig.c",
        "src/porting/arm_2d_disp_adapter_0.c",
        "tinyui/src/core/app.c",
        "tinyui/src/core/runtime_bridge.c",
    ):
        _assert_compile_unit_lacks_screen_defines(compile_commands, source_suffix)

def main() -> None:
    args = _parse_args()
    build_dir = args.build_dir
    demos = _demos_for_arg(args.demo)

    subprocess.run([
        RTK, "cmake", "-S", str(ROOT), "-B", str(build_dir), "-DUSE_DEMO=0", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ], check=True)
    _assert_tinyui_runtime_screen_defines(build_dir)
    subprocess.run([
        RTK, "cmake", "--build", str(build_dir), "--target", DEMO_TARGET
    ], check=True)

    candidates = [
        build_dir / "examples" / "sdl" / DEMO_TARGET,
        build_dir / DEMO_TARGET,
        build_dir / "examples" / DEMO_TARGET,
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        candidate_paths = ", ".join(str(path) for path in candidates)
        raise FileNotFoundError(
            f"Could not find executable for target '{DEMO_TARGET}'. Checked: {candidate_paths}"
        )

    for demo in demos:
        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
        env["TINYUI_DEMO_AUTO_QUIT_MS"] = "1200"
        with tempfile.TemporaryDirectory(prefix=f"{demo}-") as tmpdir:
            capture_path = Path(tmpdir) / "frame.ppm"
            env["TINYUI_CAPTURE_FILE"] = str(capture_path)
            completed = subprocess.run(
                [str(executable), demo],
                check=False,
                timeout=DEMO_TIMEOUT_SECONDS,
                capture_output=True,
                text=True,
                env=env,
            )
            if not capture_path.is_file() or capture_path.stat().st_size <= 32:
                raise AssertionError(
                    f"Demo '{demo}' did not produce a capture frame.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
            if demo == "basic_widgets":
                _assert_basic_widgets_capture(capture_path, completed.stdout)
        if completed.returncode != 0:
            raise RuntimeError(
                f"Demo '{demo}' exited with {completed.returncode}.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        if "TINYUI_RUNTIME_READY" not in completed.stdout:
            raise AssertionError(
                f"Demo '{demo}' did not report entering a visible runtime loop.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        _assert_optional_benchmark_markers(completed.stdout)
        if demo == "basic_widgets" and "TINYUI_FOCUS_RUNTIME_READY=1" not in completed.stdout:
            raise AssertionError(
                "basic_widgets still does not prove the app-free runtime main path.\n"
                "expected marker: TINYUI_FOCUS_RUNTIME_READY=1\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        _assert_no_smoke_layout(demo, completed.stdout, completed.stderr)


if __name__ == "__main__":
    main()
