import argparse
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD = ROOT / "build" / "picoui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 6
DEMOS = {
    "hello_world": "picoui_hello_world_demo",
    "basic_widgets": "picoui_basic_widgets_demo",
    "layout_flex": "picoui_layout_flex_demo",
    "layout_grid": "picoui_layout_grid_demo",
    "theme_showcase": "picoui_theme_showcase_demo",
    "settings_panel": "picoui_settings_panel_demo",
    "list_basic": "picoui_list_basic_demo",
}
THEME_BG = (0xF6, 0xF8, 0xFA)


def _read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    header, pixels = data.split(b"\n255\n", 1)
    _, dims = header.split(b"\n", 1)
    width_token, height_token = dims.split()
    return int(width_token), int(height_token), pixels


def _pixel(width: int, pixels: bytes, x: int, y: int) -> tuple[int, int, int]:
    offset = (y * width + x) * 3
    return pixels[offset], pixels[offset + 1], pixels[offset + 2]


def _luma(color: tuple[int, int, int]) -> float:
    red, green, blue = color
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue


def _sample_luma(width: int, height: int, pixels: bytes, *, step: int = 4) -> list[float]:
    values: list[float] = []
    for y in range(0, height, step):
        for x in range(0, width, step):
            values.append(_luma(_pixel(width, pixels, x, y)))
    return values


def _percentile(values: list[float], percent: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, int((len(ordered) - 1) * percent))
    return ordered[index]


def _color_distance(lhs: tuple[int, int, int], rhs: tuple[int, int, int]) -> int:
    return sum(abs(a - b) for a, b in zip(lhs, rhs))


def _background_color(width: int, height: int, pixels: bytes) -> tuple[int, int, int]:
    samples = [
        _pixel(width, pixels, width - 8, 8),
        _pixel(width, pixels, 8, height - 8),
        _pixel(width, pixels, width - 8, height - 8),
        _pixel(width, pixels, width // 2, height - 8),
    ]
    return min(samples, key=lambda color: _color_distance(color, THEME_BG))


def _non_background_bounds(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
) -> tuple[int, int, int, int] | None:
    xs: list[int] = []
    ys: list[int] = []
    for y in range(height):
        for x in range(width):
            if _pixel(width, pixels, x, y) != bg:
                xs.append(x)
                ys.append(y)
    if not xs:
        return None
    return min(xs), min(ys), max(xs), max(ys)


def _column_signature(width: int, height: int, pixels: bytes, x0: int, x1: int) -> tuple[int, ...]:
    signature: list[int] = []
    for y in range(0, height, 8):
        total = 0
        count = 0
        for x in range(x0, x1, 4):
            total += int(_luma(_pixel(width, pixels, x, y)))
            count += 1
        signature.append(total // max(1, count))
    return tuple(signature)


def _active_rows_and_columns(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
) -> tuple[list[int], list[int]]:
    active_rows: list[int] = []
    active_columns: list[int] = []

    for y in range(height):
        count = 0
        for x in range(width):
            if _pixel(width, pixels, x, y) != bg:
                count += 1
        if count >= 4:
            active_rows.append(y)

    for x in range(width):
        count = 0
        for y in range(height):
            if _pixel(width, pixels, x, y) != bg:
                count += 1
        if count >= 4:
            active_columns.append(x)

    return active_rows, active_columns


def _runs(values: list[int]) -> list[tuple[int, int]]:
    if not values:
        return []

    runs: list[tuple[int, int]] = []
    start = values[0]
    previous = values[0]
    for value in values[1:]:
        if value == previous + 1:
            previous = value
            continue
        runs.append((start, previous))
        start = value
        previous = value
    runs.append((start, previous))
    return runs


def _column_runs_in_band(
    width: int,
    pixels: bytes,
    bg: tuple[int, int, int],
    y0: int,
    y1: int,
) -> list[tuple[int, int]]:
    columns: list[int] = []
    sample_height = max(1, y1 - y0 + 1)
    threshold = max(4, min(18, sample_height // 3))

    for x in range(width):
        count = 0
        for y in range(y0, y1 + 1):
            if _pixel(width, pixels, x, y) != bg:
                count += 1
        if count >= threshold:
            columns.append(x)

    return _runs(columns)


def _find_grid_column_groups(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
) -> list[tuple[int, int]]:
    best_runs: list[tuple[int, int]] = []
    window_height = 44

    for y0 in range(0, max(1, height - window_height + 1), 8):
        y1 = min(height - 1, y0 + window_height - 1)
        runs = [
            run
            for run in _column_runs_in_band(width, pixels, bg, y0, y1)
            if run[1] - run[0] + 1 >= 45
        ]
        if len(runs) > len(best_runs):
            best_runs = runs
        if len(runs) >= 2:
            return runs

    return best_runs


def _assert_layout_flex_visible(path: Path) -> None:
    _assert_common_visible(path, "layout_flex")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    active_rows, active_columns = _active_rows_and_columns(width, height, pixels, bg)
    row_runs = _runs(active_rows)
    column_runs = _runs(active_columns)
    failures: list[str] = []

    wide_columns = [run for run in column_runs if run[1] - run[0] + 1 >= 120]
    if len(wide_columns) < 2:
        failures.append(
            "flex main-axis distribution failed: "
            f"wide_column_runs={column_runs}, expected at least two separated horizontal groups"
        )

    if not any((end - start + 1) >= 32 for start, end in row_runs):
        failures.append(
            "flex wrapped-track visibility failed: "
            f"row_runs={row_runs}, expected a visibly occupied row track"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: layout_flex capture is non-empty, but flex layout structure is not established.\n"
            f"  - {joined}"
        )


def _assert_layout_grid_visible(path: Path) -> None:
    _assert_common_visible(path, "layout_grid")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    active_rows, active_columns = _active_rows_and_columns(width, height, pixels, bg)
    row_runs = _runs(active_rows)
    column_runs = _find_grid_column_groups(width, height, pixels, bg)
    failures: list[str] = []

    cell_rows = [run for run in row_runs if run[1] - run[0] + 1 >= 24]
    if len(cell_rows) < 2:
        failures.append(
            "grid row separation failed: "
            f"row_runs={row_runs}, expected title and cell rows to occupy independent visible rows"
        )
    if len(column_runs) < 2:
        failures.append(
            "grid column separation failed: "
            f"best_column_runs={column_runs}, expected at least two independent visible columns"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: layout_grid capture is non-empty, but grid layout structure is not established.\n"
            f"  - {joined}"
        )


def _capture_visible_metrics(path: Path) -> tuple[int, int, tuple[int, int, int], tuple[int, int, int, int], int, int, float, float]:
    width, height, pixels = _read_ppm(path)
    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    non_bg_colors: set[tuple[int, int, int]] = set()
    for y in range(0, height, 4):
        for x in range(0, width, 4):
            color = _pixel(width, pixels, x, y)
            if color != bg:
                non_bg_colors.add(color)

    min_x, min_y, max_x, max_y = bounds
    non_bg_area = (max_x - min_x + 1) * (max_y - min_y + 1)
    return width, height, bg, bounds, non_bg_area, len(non_bg_colors), p90, p99


def _assert_common_visible(path: Path, demo: str) -> None:
    width, height, bg, bounds, non_bg_area, non_bg_color_count, p90, p99 = _capture_visible_metrics(path)
    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected {demo} capture size: {width}x{height}")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    failures: list[str] = []

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )

    if p90 < 55.0 or p99 < 95.0:
        failures.append(
            "near-black/readability check failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=55 and p99>=95"
        )

    if visible_width < 40 or visible_height < 20:
        failures.append(
            "structure coverage check failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), "
            "expected visible content to occupy at least 40x20 pixels"
        )

    if non_bg_color_count < 2 and non_bg_area < 20000:
        failures.append(
            "readability check failed: "
            f"non_bg_color_count={non_bg_color_count}, non_bg_area={non_bg_area}, "
            "expected >=2 colors or >=20000 px content area"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            f"VISIBLE FAIL: {demo} capture is non-empty, but visible correctness is not established.\n"
            f"  - {joined}"
        )


def _assert_basic_widgets_visible(path: Path) -> None:
    _assert_common_visible(path, "basic_widgets")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    failures: list[str] = []

    if visible_width < 220 or visible_height < 180:
        failures.append(
            "structure coverage check failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), "
            "expected readable UI to occupy at least 220x180 pixels"
        )

    left = _column_signature(width, height, pixels, 0, width // 2)
    right = _column_signature(width, height, pixels, width // 2, width)
    identical_rows = sum(1 for lhs, rhs in zip(left, right) if abs(lhs - rhs) <= 1)
    if identical_rows >= len(left) * 0.70:
        failures.append(
            "duplicate-column/structure check failed: "
            f"{identical_rows}/{len(left)} sampled rows have near-identical left/right signatures"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: basic_widgets capture is non-empty, but visible correctness is not established.\n"
            f"  - {joined}"
        )


def _assert_list_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "list_basic")
    width, height, pixels = _read_ppm(path)
    bg = _pixel(width, pixels, 8, 8)
    bounds = _non_background_bounds(width, height, pixels, bg)
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    failures: list[str] = []

    if visible_width < 180 or visible_height < 80:
        failures.append(
            "list structure check failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), "
            "expected readable list content to occupy at least 180x80 pixels"
        )

    row_bands = [
        (min_y + 32, min_y + 56),
        (min_y + 56, min_y + 80),
        (min_y + 80, min_y + 104),
    ]
    readable_rows = 0
    for y0, y1 in row_bands:
        colors: set[tuple[int, int, int]] = set()
        for y in range(max(0, y0), min(height, y1), 4):
            for x in range(max(0, min_x), min(width, min_x + 220), 4):
                color = _pixel(width, pixels, x, y)
                if color != bg:
                    colors.add(color)
        if len(colors) >= 2:
            readable_rows += 1

    if readable_rows < 2:
        failures.append(
            "list row contrast check failed: "
            f"readable_rows={readable_rows}, expected at least 2 rows with visible contrast"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: list_basic capture is non-empty, but visible correctness is not established.\n"
            f"  - {joined}"
        )


def _find_executable(target: str) -> Path:
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
    return executable


def _run_demo(target: str, capture_path: Path) -> subprocess.CompletedProcess[str]:
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
    env["PICOUI_CAPTURE_FILE"] = str(capture_path)
    return subprocess.run(
        [str(_find_executable(target))],
        check=False,
        timeout=DEMO_TIMEOUT_SECONDS,
        capture_output=True,
        text=True,
        env=env,
    )


def _assert_no_unexpected_fallback(demo: str, stdout: str) -> None:
    if "PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK" not in stdout:
        return
    raise AssertionError(
        f"VISIBLE FAIL: {demo} still uses backend fallback widgets.\n"
        f"stdout:\n{stdout}"
    )


def _assert_basic_widgets_image_source_boundary(stdout: str) -> None:
    expected = "PICOUI_BACKEND_IMAGE_SOURCE=logo:img=null,mask=null"

    if expected not in stdout:
        raise AssertionError(
            "VISIBLE FAIL: basic_widgets image source boundary changed during runtime render.\n"
            f"expected marker: {expected}\n"
            f"stdout:\n{stdout}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check PicoUI visible correctness evidence for selected demos."
    )
    parser.add_argument("--demo", choices=sorted(DEMOS), default="basic_widgets")
    parser.add_argument("--all", action="store_true", help="check every PicoUI demo visible gate")
    args = parser.parse_args()
    selected = sorted(DEMOS) if args.all else [args.demo]

    subprocess.run([RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"], check=True)
    subprocess.run([RTK, "cmake", "--build", str(BUILD), "--target", *(DEMOS[demo] for demo in selected)], check=True)

    for demo in selected:
        target = DEMOS[demo]
        with tempfile.TemporaryDirectory(prefix=f"{target}-visible-") as tmpdir:
            capture_path = Path(tmpdir) / "frame.ppm"
            completed = _run_demo(target, capture_path)
            if completed.returncode != 0:
                raise RuntimeError(
                    f"SMOKE FAIL: demo '{target}' exited with {completed.returncode}.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
            if "PICOUI_RUNTIME_READY" not in completed.stdout:
                raise AssertionError(
                    f"SMOKE FAIL: demo '{target}' did not report entering a runtime loop.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
            if not capture_path.is_file() or capture_path.stat().st_size <= 32:
                raise AssertionError(
                    f"SMOKE FAIL: demo '{target}' did not produce a capture frame.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
            if demo == "basic_widgets":
                _assert_basic_widgets_visible(capture_path)
                _assert_basic_widgets_image_source_boundary(completed.stdout)
            elif demo == "list_basic":
                _assert_list_basic_visible(capture_path)
            elif demo == "layout_flex":
                _assert_layout_flex_visible(capture_path)
            elif demo == "layout_grid":
                _assert_layout_grid_visible(capture_path)
            else:
                _assert_common_visible(capture_path, demo)
            _assert_no_unexpected_fallback(demo, completed.stdout)


if __name__ == "__main__":
    main()
