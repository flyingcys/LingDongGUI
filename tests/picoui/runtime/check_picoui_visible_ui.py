import argparse
import math
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "picoui-runtime"
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
    "progress_bar_basic": "picoui_progress_bar_basic_demo",
    "arc_basic": "picoui_arc_basic_demo",
    "gauge_basic": "picoui_gauge_basic_demo",
    "icon_slider_basic": "picoui_icon_slider_basic_demo",
    "radial_menu_basic": "picoui_radial_menu_basic_demo",
    "progress_wheel_basic": "picoui_progress_wheel_basic_demo",
    "qrcode_basic": "picoui_qrcode_basic_demo",
    "message_box_basic": "picoui_message_box_basic_demo",
    "date_time_basic": "picoui_date_time_basic_demo",
    "clock_basic": "picoui_clock_basic_demo",
    "keyboard_basic": "picoui_keyboard_basic_demo",
    "line_edit_basic": "picoui_line_edit_basic_demo",
    "combo_box_basic": "picoui_combo_box_basic_demo",
    "scroll_selecter_basic": "picoui_scroll_selecter_basic_demo",
    "table_basic": "picoui_table_basic_demo",
    "graph_basic": "picoui_graph_basic_demo",
    "calendar_basic": "picoui_calendar_basic_demo",
    "animation_basic": "picoui_animation_basic_demo",
    "legacy_widget_parity": "picoui_legacy_widget_parity_demo",
    "layout_parity": "picoui_layout_parity_demo",
    "grid_parity": "picoui_grid_parity_demo",
}
THEME_BG = (0xF6, 0xF8, 0xFA)
WHITE_BG = (0xFF, 0xFF, 0xFF)
BACKGROUND_TOLERANCE = 20


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


def _is_background(color: tuple[int, int, int], bg: tuple[int, int, int]) -> bool:
    return (
        _color_distance(color, bg) <= BACKGROUND_TOLERANCE
        or _color_distance(color, THEME_BG) <= BACKGROUND_TOLERANCE
        or _color_distance(color, WHITE_BG) <= BACKGROUND_TOLERANCE
    )


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
            if not _is_background(_pixel(width, pixels, x, y), bg):
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
    cell_band_top = 0
    cell_band_bottom = height - 1

    for y in range(height):
        count = 0
        for x in range(width):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 4:
            active_rows.append(y)

    for x in range(width):
        count = 0
        for y in range(height):
            if not _is_background(_pixel(width, pixels, x, y), bg):
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
            if not _is_background(_pixel(width, pixels, x, y), bg):
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


def _assert_hello_world_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected hello_world capture size: {width}x{height}")

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    colors: set[tuple[int, int, int]] = set()

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "hello_world color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )
    if visible_width < 120 or visible_height < 28:
        failures.append(
            "hello_world structure coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 120x28"
        )

    for y in range(min_y, max_y + 1, 2):
        for x in range(min_x, max_x + 1, 2):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)

    if len(colors) < 1:
        failures.append(
            "hello_world content visibility failed: "
            "expected at least one non-background content color"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: hello_world capture is non-empty, but hello_world structure is not established.\n"
            f"  - {joined}"
        )


def _assert_layout_grid_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    failures: list[str] = []
    active_rows: list[int] = []
    active_columns: list[int] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected layout_grid capture size: {width}x{height}")

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "grid color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )

    for y in range(height):
        count = 0
        for x in range(width):
            if _color_distance(_pixel(width, pixels, x, y), bg) > BACKGROUND_TOLERANCE:
                count += 1
        if count >= 12:
            active_rows.append(y)

    row_runs = _runs(active_rows)
    title_rows = [run for run in row_runs if 20 <= (run[1] - run[0] + 1) <= 40]
    if len(row_runs) >= 2:
        cell_band_top = row_runs[1][0]
        cell_band_bottom = row_runs[1][1]
    elif title_rows:
        cell_band_top = title_rows[0][1] + 1
        cell_band_bottom = row_runs[-1][1] if row_runs else height - 1
    elif row_runs:
        cell_band_top = row_runs[0][0]
        cell_band_bottom = row_runs[-1][1]

    for x in range(width):
        count = 0
        for y in range(cell_band_top, cell_band_bottom + 1):
            if _color_distance(_pixel(width, pixels, x, y), bg) > BACKGROUND_TOLERANCE:
                count += 1
        if count >= 8:
            active_columns.append(x)

    column_runs = _runs(active_columns)
    cell_columns = [run for run in column_runs if (run[1] - run[0] + 1) >= 40]

    if len(title_rows) < 2:
        failures.append(
            "grid row separation failed: "
            f"row_runs={row_runs}, expected separate title and cell bands"
        )
    if len(cell_columns) < 2:
        failures.append(
            "grid column separation failed: "
            f"column_runs={column_runs}, expected at least two independent visible columns"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: layout_grid capture is non-empty, but grid layout structure is not established.\n"
            f"  - {joined}"
        )


def _assert_layout_parity_visible(path: Path) -> None:
    width, height, bg, bounds, non_bg_area, non_bg_color_count, p90, p99 = _capture_visible_metrics(path)
    _, _, pixels = _read_ppm(path)
    active_rows: list[int] = []
    failures: list[str] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected layout_parity capture size: {width}x{height}")

    if bounds[2] - bounds[0] + 1 < 180 or bounds[3] - bounds[1] + 1 < 160:
        failures.append(
            "layout parity coverage failed: "
            f"content_bounds={bounds}, expected at least 180x160 visible area"
        )

    if non_bg_area < 18000 or non_bg_color_count < 2:
        failures.append(
            "layout parity readability failed: "
            f"non_bg_area={non_bg_area}, non_bg_color_count={non_bg_color_count}, "
            "expected >=18000 px and >=2 visible colors"
        )

    if p90 < 35.0 or p99 < 70.0:
        failures.append(
            "layout parity luma floor failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=35 and p99>=70"
        )

    for y in range(height):
        count = 0
        for x in range(width):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 40:
            active_rows.append(y)
    row_runs = [run for run in _runs(active_rows) if (run[1] - run[0] + 1) >= 12]

    if len(row_runs) < 2:
        failures.append(
            "layout section row separation failed: "
            f"row_runs={row_runs}, expected main content band plus guide/footer band"
        )

    footer_rows: list[int] = []
    for y in range(max(0, height - 80), height):
        count = 0
        for x in range(width):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 80:
            footer_rows.append(y)
    if not footer_rows:
        failures.append(
            "layout footer/guide visibility failed: expected a dedicated lower-band text or guide region"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: layout_parity capture is non-empty, but section parity structure is not established.\n"
            f"  - {joined}"
        )


def _assert_grid_parity_visible(path: Path) -> None:
    width, height, bg, bounds, non_bg_area, non_bg_color_count, p90, p99 = _capture_visible_metrics(path)
    _, _, pixels = _read_ppm(path)
    failures: list[str] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected grid_parity capture size: {width}x{height}")

    if non_bg_area < 16000 or non_bg_color_count < 2:
        failures.append(
            "grid parity readability failed: "
            f"non_bg_area={non_bg_area}, non_bg_color_count={non_bg_color_count}, "
            "expected >=16000 px and >=2 visible colors"
        )

    if p90 < 30.0 or p99 < 65.0:
        failures.append(
            "grid parity luma floor failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=30 and p99>=65"
        )

    content_rows: list[int] = []
    for y in range(height):
        count = 0
        for x in range(width):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 40:
            content_rows.append(y)
    content_row_runs = [run for run in _runs(content_rows) if (run[1] - run[0] + 1) >= 12]
    if len(content_row_runs) < 2:
        failures.append(
            "grid parity row-band separation failed: "
            f"row_runs={content_row_runs}, expected main canvas band plus lower guide/overlay band"
        )

    overlay_rows: list[int] = []
    for y in range(height):
        count = 0
        for x in range(max(0, width - 180), width):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 40:
            overlay_rows.append(y)
    overlay_runs = [run for run in _runs(overlay_rows) if (run[1] - run[0] + 1) >= 12]
    if not overlay_runs:
        failures.append(
            "grid parity overlay visibility failed: "
            "expected a dedicated visible band near the right-side overlay region"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: grid_parity capture is non-empty, but grid parity structure is not established.\n"
            f"  - {joined}"
        )


def _assert_legacy_widget_parity_visible(path: Path) -> None:
    width, height, bg, bounds, non_bg_area, non_bg_color_count, p90, p99 = _capture_visible_metrics(path)
    _, _, pixels = _read_ppm(path)
    failures: list[str] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected legacy_widget_parity capture size: {width}x{height}")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    if visible_width < 430 or visible_height < 250:
        failures.append(
            "legacy widget coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 430x250"
        )

    if non_bg_area < 45000 or non_bg_color_count < 5:
        failures.append(
            "legacy widget readability failed: "
            f"non_bg_area={non_bg_area}, non_bg_color_count={non_bg_color_count}, "
            "expected >=45000 px and >=5 visible colors"
        )

    if p90 < 25.0 or p99 < 60.0:
        failures.append(
            "legacy widget luma floor failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=25 and p99>=60"
        )

    colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 6):
        for x in range(min_x, max_x + 1, 6):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)
    if len(colors) < 5:
        failures.append(
            "legacy widget contrast breadth failed: "
            f"colors={sorted(colors)}, expected at least 5 visible non-background shades"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: legacy_widget_parity capture is non-empty, but legacy mixed-widget structure is not established.\n"
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
            if not _is_background(color, bg):
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

    if visible_width < 220 or visible_height < 176:
        failures.append(
            "structure coverage check failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), "
            "expected readable UI to occupy at least 220x176 pixels"
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
            "VISIBLE FAIL: TinyUI v2.0 pilot startup proof 'basic_widgets' is non-empty, but visible correctness is not established.\n"
            f"  - {joined}"
        )


def _assert_list_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    failures: list[str] = []
    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected list_basic capture size: {width}x{height}")

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "list color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )

    if p90 < 55.0 or p99 < 95.0:
        failures.append(
            "list readability check failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=55 and p99>=95"
        )

    if visible_width < 180 or visible_height < 24:
        failures.append(
            "list structure check failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), "
            "expected visible list content to occupy at least 180x24 pixels"
        )

    active_rows: list[int] = []
    for y in range(min_y, max_y + 1):
        count = 0
        for x in range(min_x, max_x + 1):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 160:
            active_rows.append(y)

    row_runs = _runs(active_rows)
    wide_bands = [run for run in row_runs if (run[1] - run[0] + 1) >= 20]
    if not wide_bands:
        failures.append(
            "list band continuity check failed: "
            f"row_runs={row_runs}, expected at least one wide horizontal list band"
        )

    band_colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 2):
        for x in range(min_x, max_x + 1, 2):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                band_colors.add(color)

    if len(band_colors) < 3:
        failures.append(
            "list band contrast check failed: "
            f"band_colors={sorted(band_colors)}, expected at least 3 visible band shades"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: list_basic capture is non-empty, but visible correctness is not established.\n"
            f"  - {joined}"
        )


def _assert_progress_bar_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "progress_bar_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []
    horizontal_colors: set[tuple[int, int, int]] = set()
    vertical_colors: set[tuple[int, int, int]] = set()
    horizontal_runs: list[tuple[int, int]] = []
    vertical_runs: list[tuple[int, int]] = []
    horizontal_span: tuple[int, int] | None = None

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds

    candidate_rows: list[int] = []
    row_spans: dict[int, tuple[int, int]] = {}
    for y in range(64, min(height, 97)):
        xs: list[int] = []
        for x in range(112, min(width, 433)):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                xs.append(x)
        if len(xs) >= 180:
            candidate_rows.append(y)
            row_spans[y] = (xs[0], xs[-1])

    for row_start, row_end in _runs(candidate_rows):
        if row_end - row_start + 1 < 8 or row_end - row_start + 1 > 32:
            continue
        span_start = min(row_spans[y][0] for y in range(row_start, row_end + 1) if y in row_spans)
        span_end = max(row_spans[y][1] for y in range(row_start, row_end + 1) if y in row_spans)
        if span_end - span_start + 1 < 180:
            continue
        horizontal_runs = [(row_start, row_end)]
        horizontal_span = (span_start, span_end)
        for y in range(row_start, row_end + 1):
            for x in range(span_start, span_end + 1, 4):
                color = _pixel(width, pixels, x, y)
                if not _is_background(color, bg):
                    horizontal_colors.add(color)
        break

    candidate_columns: list[int] = []
    for x in range(min_x, max_x + 1):
        active = 0
        for y in range(min_y, max_y + 1):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                active += 1
        if active >= 72:
            candidate_columns.append(x)

    for col_start, col_end in _runs(candidate_columns):
        if col_end - col_start + 1 < 20 or col_end - col_start + 1 > 80:
            continue
        row_hits: list[int] = []
        for y in range(min_y, max_y + 1):
            active = 0
            for x in range(col_start, col_end + 1):
                if not _is_background(_pixel(width, pixels, x, y), bg):
                    active += 1
            if active >= 12:
                row_hits.append(y)
        tall_runs = [run for run in _runs(row_hits) if run[1] - run[0] + 1 >= 96]
        if not tall_runs:
            continue
        vertical_runs = tall_runs
        for y in range(tall_runs[0][0], tall_runs[0][1] + 1, 4):
            for x in range(col_start, col_end + 1, 2):
                color = _pixel(width, pixels, x, y)
                if not _is_background(color, bg):
                    vertical_colors.add(color)
        break

    if horizontal_span is None or horizontal_span[1] - horizontal_span[0] + 1 < 180:
        failures.append(
            "horizontal progress bar direction failed: "
            f"row_runs={horizontal_runs}, span={horizontal_span}, expected a thin row band with a long horizontal span"
        )
    if len(vertical_colors) < 2:
        failures.append(
            "vertical progress bar contrast failed: "
            f"colors={sorted(vertical_colors)}"
        )
    if not any((end - start + 1) >= 96 for start, end in vertical_runs):
        failures.append(
            "vertical progress bar direction failed: "
            f"row_runs={vertical_runs}, expected a tall vertical occupancy band"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: progress_bar_basic capture is non-empty, but progress bar structure is not established.\n"
            f"  - {joined}"
        )


def _assert_arc_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "arc_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    if visible_width < 100 or visible_height < 100:
        failures.append(
            "arc coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 100x100"
        )

    colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 3):
        for x in range(min_x, max_x + 1, 3):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)
    if len(colors) < 2:
        failures.append(
            "arc color contrast failed: "
            f"colors={sorted(colors)}, expected foreground/background arc contrast"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: arc_basic capture is non-empty, but arc structure is not established.\n"
            f"  - {joined}"
        )


def _assert_gauge_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "gauge_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    if visible_width < 100 or visible_height < 100:
        failures.append(
            "gauge coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 100x100"
        )

    colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 3):
        for x in range(min_x, max_x + 1, 3):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)
    if len(colors) < 2:
        failures.append(
            "gauge pointer contrast failed: "
            f"colors={sorted(colors)}, expected gauge pointer and dial contrast"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: gauge_basic capture is non-empty, but gauge structure is not established.\n"
            f"  - {joined}"
        )


def _assert_icon_slider_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "icon_slider_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    if visible_width < 150 or visible_height < 40:
        failures.append(
            "icon slider coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 150x70"
        )

    colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 3):
        for x in range(min_x, max_x + 1, 3):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)
    if len(colors) < 3:
        failures.append(
            "icon slider contrast failed: "
            f"colors={sorted(colors)}, expected multiple icon/text colors"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: icon_slider_basic capture is non-empty, but icon slider structure is not established.\n"
            f"  - {joined}"
        )


def _assert_radial_menu_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "radial_menu_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    if visible_width < 100 or visible_height < 70:
        failures.append(
            "radial menu coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 130x70"
        )

    colors: set[tuple[int, int, int]] = set()
    for y in range(min_y, max_y + 1, 3):
        for x in range(min_x, max_x + 1, 3):
            color = _pixel(width, pixels, x, y)
            if not _is_background(color, bg):
                colors.add(color)
    if len(colors) < 3:
        failures.append(
            "radial menu contrast failed: "
            f"colors={sorted(colors)}, expected multiple icon colors"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: radial_menu_basic capture is non-empty, but radial menu structure is not established.\n"
            f"  - {joined}"
        )


def _assert_progress_wheel_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    failures: list[str] = []
    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)
    wheel_pixels: list[tuple[int, int, tuple[int, int, int]]] = []
    white_pixels: set[tuple[int, int]] = set()

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected progress_wheel_basic capture size: {width}x{height}")

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "progress wheel color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )
    if p90 < 55.0 or p99 < 95.0:
        failures.append(
            "progress wheel readability check failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=55 and p99>=95"
        )

    for y in range(56, height):
        for x in range(width):
            color = _pixel(width, pixels, x, y)
            if _color_distance(color, WHITE_BG) <= BACKGROUND_TOLERANCE:
                white_pixels.add((x, y))
                continue
            if _is_background(color, bg):
                continue
            wheel_pixels.append((x, y, color))

    if not wheel_pixels:
        failures.append("progress wheel structure coverage failed: no non-background wheel pixels found below the title band")
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: progress_wheel_basic capture is non-empty, but progress wheel structure is not established.\n"
            f"  - {joined}"
        )

    xs = [x for x, _, _ in wheel_pixels]
    ys = [y for _, y, _ in wheel_pixels]
    min_x, min_y, max_x, max_y = min(xs), min(ys), max(xs), max(ys)
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    center_x = (min_x + max_x) // 2
    center_y = (min_y + max_y) // 2
    ring_colors: set[tuple[int, int, int]] = set()
    interior_colors: set[tuple[int, int, int]] = set()
    ring_samples = 0
    interior_samples = 0
    white_near_count = 0
    white_near_bounds: tuple[int, int, int, int] | None = None

    if visible_width < 24 or visible_height < 24:
        failures.append(
            "progress wheel structure coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 24x24 visible wheel pixels"
        )

    sample_radius = min(visible_width, visible_height) // 2
    outer_radius_sq = max(1, sample_radius * sample_radius)
    inner_radius_sq = max(1, (sample_radius // 3) * (sample_radius // 3))
    ring_pixel_coords: set[tuple[int, int]] = set()

    for y in range(min_y, max_y + 1, 2):
        for x in range(min_x, max_x + 1, 2):
            color = _pixel(width, pixels, x, y)
            if _is_background(color, bg) or _color_distance(color, WHITE_BG) <= BACKGROUND_TOLERANCE:
                continue
            dx = x - center_x
            dy = y - center_y
            distance_sq = dx * dx + dy * dy
            if distance_sq >= inner_radius_sq and distance_sq <= outer_radius_sq:
                ring_colors.add(color)
                ring_samples += 1
                ring_pixel_coords.add((x, y))
            elif distance_sq < inner_radius_sq:
                interior_colors.add(color)
                interior_samples += 1

    if ring_pixel_coords and white_pixels:
        white_xs: list[int] = []
        white_ys: list[int] = []
        for white_x, white_y in white_pixels:
            is_near_ring = False
            for dx in range(-6, 7):
                if is_near_ring:
                    break
                for dy in range(-6, 7):
                    if (white_x + dx, white_y + dy) in ring_pixel_coords:
                        is_near_ring = True
                        break
            if not is_near_ring:
                continue
            white_near_count += 1
            white_xs.append(white_x)
            white_ys.append(white_y)
        if white_xs and white_ys:
            white_near_bounds = (min(white_xs), min(white_ys), max(white_xs), max(white_ys))

    if ring_samples < 10:
        failures.append(
            "progress wheel ring occupancy failed: "
            f"ring_samples={ring_samples}, expected >=10 sampled ring pixels"
        )
    if len(ring_colors) < 1:
        failures.append(
            "progress wheel contrast failed: "
            f"ring_colors={sorted(ring_colors)}, expected at least one visible wheel color"
        )
    if interior_samples < 1:
        failures.append(
            "progress wheel interior visibility failed: "
            f"interior_samples={interior_samples}, expected at least one visible wheel interior sample"
        )
    if white_near_count < 4:
        failures.append(
            "progress wheel dot visibility failed: "
            f"white_near_count={white_near_count}, white_near_bounds={white_near_bounds}, "
            "expected nearby white dot pixels adjacent to the colored ring"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: progress_wheel_basic capture is non-empty, but progress wheel structure is not established.\n"
            f"  - {joined}"
        )


def _assert_qrcode_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []
    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    dark_pixels = 0
    light_pixels = 0
    module_x0 = max(0, min_x - 8)
    module_y0 = max(0, min_y - 8)
    module_x1 = min(width - 1, max_x + 8)
    module_y1 = min(height - 1, max_y + 8)

    if visible_width < 96 or visible_height < 96:
        failures.append(
            "qrcode structure coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 96x96 pixels"
        )
    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "qrcode color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )
    if p90 < 55.0 or p99 < 95.0:
        failures.append(
            "qrcode readability check failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=55 and p99>=95"
        )

    for y in range(module_y0, module_y1 + 1, 2):
        for x in range(module_x0, module_x1 + 1, 2):
            color = _pixel(width, pixels, x, y)
            if _luma(color) < 90:
                dark_pixels += 1
            else:
                light_pixels += 1

    if dark_pixels < 300:
        failures.append(
            "qrcode dark-module coverage failed: "
            f"dark_pixels={dark_pixels}, expected >=300 sampled dark pixels"
        )
    if light_pixels < 300:
        failures.append(
            "qrcode light-module coverage failed: "
            f"light_pixels={light_pixels}, expected >=300 sampled light pixels"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: qrcode_basic capture is non-empty, but qrcode structure is not established.\n"
            f"  - {joined}"
        )


def _assert_message_box_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "message_box_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    if max_y < height // 2:
        failures.append(
            "message box lower-half placement failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected visible lower-half structure"
        )

    active_rows: list[int] = []
    x0 = min_x
    x1 = max_x
    y0 = min_y
    y1 = min(max_y, height - 16)
    for y in range(y0, y1 + 1):
        count = 0
        for x in range(x0, x1 + 1):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 4:
            active_rows.append(y)

    band_runs = _runs(active_rows)
    thin_runs = [run for run in band_runs if run[1] - run[0] + 1 >= 4]
    thick_runs = [run for run in band_runs if run[1] - run[0] + 1 >= 12]
    if len(thin_runs) < 3 or len(thick_runs) < 1:
        failures.append(
            "message box vertical-band layering failed: "
            f"row_runs={band_runs}, expected two thin text bands plus one thicker button band"
        )

    if (max_x - min_x + 1) < 140 or (max_y - min_y + 1) < 90:
        failures.append(
            "message box rectangle coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 140x90"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: message_box_basic capture is non-empty, but message box structure is not established.\n"
            f"  - {joined}"
        )


def _assert_animation_basic_visible(path: Path) -> None:
    _assert_common_visible(path, "animation_basic")
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)

    if bounds is None:
        raise AssertionError("SMOKE FAIL: animation_basic capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    if (max_x - min_x + 1) < 24 or (max_y - min_y + 1) < 16:
        raise AssertionError(
            "VISIBLE FAIL: animation_basic visible region is too small for title + frame tile.\n"
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y})"
        )


def _assert_date_time_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected date_time_basic capture size: {width}x{height}")

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "date_time color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )

    if visible_width < 100 or visible_height < 6:
        failures.append(
            "date_time coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 100x6"
        )

    active_rows: list[int] = []
    for y in range(min_y, max_y + 1):
        count = 0
        for x in range(min_x, max_x + 1):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                count += 1
        if count >= 8:
            active_rows.append(y)

    row_runs = _runs(active_rows)
    text_runs = [run for run in row_runs if 4 <= (run[1] - run[0] + 1) <= 12]
    if not text_runs:
        failures.append(
            "date_time text-band check failed: "
            f"row_runs={row_runs}, expected a thin readable datetime text band"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: date_time_basic capture is non-empty, but date_time structure is not established.\n"
            f"  - {joined}"
        )


def _assert_clock_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    failures: list[str] = []
    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)
    bounds = _non_background_bounds(width, height, pixels, bg)

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected clock_basic capture size: {width}x{height}")

    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "clock color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )
    if p90 < 55.0 or p99 < 95.0:
        failures.append(
            "clock readability check failed: "
            f"p90_luma={p90:.1f}, p99_luma={p99:.1f}, expected p90>=55 and p99>=95"
        )
    if visible_width < 60 or visible_height < 80:
        failures.append(
            "clock structure coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 60x80 visible clock pixels"
        )

    best_center_hits = 0
    best_center: tuple[int, int] | None = None
    ray_hits = 0
    direction_bins: dict[int, int] = {}

    for center_y in range(min_y + 8, max_y - 7, 2):
        for center_x in range(min_x + 8, max_x - 7, 2):
            center_hits = 0
            for y in range(center_y - 6, center_y + 7):
                for x in range(center_x - 6, center_x + 7):
                    if not _is_background(_pixel(width, pixels, x, y), bg):
                        center_hits += 1
            if center_hits > best_center_hits:
                best_center_hits = center_hits
                best_center = (center_x, center_y)

    if best_center is not None:
        center_x, center_y = best_center
        for y in range(min_y, max_y + 1):
            for x in range(min_x, max_x + 1):
                if _is_background(_pixel(width, pixels, x, y), bg):
                    continue
                dx = x - center_x
                dy = y - center_y
                if dx == 0 and dy == 0:
                    continue
                if dx * dx + dy * dy < 100:
                    continue
                ray_hits += 1
                angle = (math.degrees(math.atan2(-dy, dx)) + 360.0) % 360.0
                direction = int(angle // 30) * 30
                direction_bins[direction] = direction_bins.get(direction, 0) + 1

    strong_directions = sorted(direction for direction, count in direction_bins.items() if count >= 20)
    medium_directions = sorted(direction for direction, count in direction_bins.items() if count >= 8)

    if best_center_hits < 12:
        failures.append(
            "clock hub visibility failed: "
            f"center_hits={best_center_hits}, expected a visible center hub region"
        )
    if ray_hits < 80 or len(strong_directions) < 2 or len(medium_directions) < 3:
        failures.append(
            "clock pointer spread failed: "
            f"ray_hits={ray_hits}, strong_directions={strong_directions}, "
            f"medium_directions={medium_directions}, expected at least two strong and three medium pointer directions"
        )
    if (visible_width + visible_height) < 180:
        failures.append(
            "clock pointer span failed: "
            f"visible_width={visible_width}, visible_height={visible_height}, expected combined span >= 180"
        )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: clock_basic capture is non-empty, but clock structure is not established.\n"
            f"  - {joined}"
        )


def _assert_calendar_basic_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    bg = _background_color(width, height, pixels)
    bounds = _non_background_bounds(width, height, pixels, bg)
    failures: list[str] = []
    active_rows: list[int] = []
    dense_rows: list[int] = []

    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected calendar_basic capture size: {width}x{height}")
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

    min_x, min_y, max_x, max_y = bounds
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1

    if _color_distance(bg, THEME_BG) > 24:
        failures.append(
            "calendar color/readback check failed: "
            f"background={bg}, expected near theme bg={THEME_BG}"
        )
    if visible_width < 220 or visible_height < 140:
        failures.append(
            "calendar coverage failed: "
            f"content_bounds=({min_x},{min_y})-({max_x},{max_y}), expected at least 220x140"
        )

    for y in range(min_y, max_y + 1):
        active = 0
        for x in range(min_x, max_x + 1):
            if not _is_background(_pixel(width, pixels, x, y), bg):
                active += 1
        if active >= 14:
            active_rows.append(y)
        if active >= 70:
            dense_rows.append(y)

    row_runs = _runs(active_rows)
    dense_row_runs = _runs(dense_rows)
    if not dense_row_runs:
        failures.append(
            "calendar header text-band failed: "
            f"dense_row_runs={dense_row_runs}, expected a visible header text band"
        )
    if len(row_runs) < 6:
        failures.append(
            "calendar row-band structure failed: "
            f"row_runs={row_runs}, expected at least six visible row bands"
        )

    if len(row_runs) >= 2:
        weekday_y0, weekday_y1 = row_runs[1]
        date_y0 = row_runs[2][0] if len(row_runs) >= 3 else min(max_y, weekday_y1 + 8)
        date_y1 = row_runs[-2][1] if len(row_runs) >= 4 else min(max_y, date_y0 + 96)
        span = max_x - min_x + 1
        weekday_bins = []
        date_bins = []
        for index in range(7):
            x0 = min_x + (span * index) // 7
            x1 = min_x + (span * (index + 1)) // 7 - 1
            weekday_count = 0
            date_count = 0
            for y in range(weekday_y0, weekday_y1 + 1):
                for x in range(x0, x1 + 1):
                    if not _is_background(_pixel(width, pixels, x, y), bg):
                        weekday_count += 1
            for y in range(date_y0, date_y1 + 1):
                for x in range(x0, x1 + 1):
                    if not _is_background(_pixel(width, pixels, x, y), bg):
                        date_count += 1
            weekday_bins.append((x0, x1, weekday_count))
            date_bins.append((x0, x1, date_count))
        if sum(1 for _, _, count in weekday_bins if count >= 8) < 7:
            failures.append(
                "calendar weekday-grid failed: "
                f"weekday_bins={weekday_bins}, expected seven weekday buckets with visible text"
            )
        if sum(1 for _, _, count in date_bins if count >= 24) < 7:
            failures.append(
                "calendar date-grid failed: "
                f"date_bins={date_bins}, expected seven date buckets with visible content"
            )

    if failures:
        joined = "\n  - ".join(failures)
        raise AssertionError(
            "VISIBLE FAIL: calendar_basic capture is non-empty, but calendar header/date grid structure is not established.\n"
            f"  - {joined}"
        )


def _find_executable(build_dir: Path, target: str) -> Path:
    candidates = [
        build_dir / "examples" / "sdl" / target,
        build_dir / target,
        build_dir / "examples" / target,
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        candidate_paths = ", ".join(str(path) for path in candidates)
        raise FileNotFoundError(
            f"Could not find executable for target '{target}'. Checked: {candidate_paths}"
        )
    return executable


def _run_demo(build_dir: Path, target: str, capture_path: Path) -> subprocess.CompletedProcess[str]:
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
    env["PICOUI_CAPTURE_FILE"] = str(capture_path)
    return subprocess.run(
        [str(_find_executable(build_dir, target))],
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


def _assert_real_mapping_honesty(demo: str, stdout: str) -> None:
    expected = "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI"
    if expected in stdout:
        return
    raise AssertionError(
        f"VISIBLE FAIL: {demo} no longer reports its real-mapping honesty marker.\n"
        f"expected marker: {expected}\n"
        f"stdout:\n{stdout}"
    )


def _assert_basic_widgets_image_source_boundary(stdout: str) -> None:
    expected = "PICOUI_BACKEND_IMAGE_SOURCE=logo:img=null,mask=null"

    if expected not in stdout:
        raise AssertionError(
            "VISIBLE FAIL: TinyUI v2.0 pilot startup proof 'basic_widgets' changed its image source boundary during runtime render.\n"
            f"expected marker: {expected}\n"
            f"stdout:\n{stdout}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check TinyUI v2.0/PicoUI visible correctness evidence for selected demos."
    )
    parser.add_argument("--demo", choices=sorted(DEMOS), default="basic_widgets")
    parser.add_argument(
        "--all",
        action="store_true",
        help="check every TinyUI v2.0/PicoUI demo visible gate",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="reuse an existing build directory instead of allocating an isolated one",
    )
    args = parser.parse_args()
    selected = sorted(DEMOS) if args.all else [args.demo]

    build_dir = args.build_dir
    if build_dir is None:
        build_dir = Path(tempfile.mkdtemp(prefix="picoui-visible-build-"))
    else:
        build_dir = build_dir.resolve()
        build_dir.mkdir(parents=True, exist_ok=True)

    try:
        subprocess.run([RTK, "cmake", "-S", str(ROOT), "-B", str(build_dir), "-DUSE_DEMO=0"], check=True)
        subprocess.run(
            [RTK, "cmake", "--build", str(build_dir), "--target", *(DEMOS[demo] for demo in selected)],
            check=True,
        )

        for demo in selected:
            target = DEMOS[demo]
            with tempfile.TemporaryDirectory(prefix=f"{target}-visible-") as tmpdir:
                capture_path = Path(tmpdir) / "frame.ppm"
                completed = _run_demo(build_dir, target, capture_path)
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
                elif demo == "hello_world":
                    _assert_hello_world_visible(capture_path)
                elif demo == "list_basic":
                    _assert_list_basic_visible(capture_path)
                elif demo == "progress_bar_basic":
                    _assert_progress_bar_basic_visible(capture_path)
                elif demo == "arc_basic":
                    _assert_arc_basic_visible(capture_path)
                elif demo == "gauge_basic":
                    _assert_gauge_basic_visible(capture_path)
                elif demo == "icon_slider_basic":
                    _assert_icon_slider_basic_visible(capture_path)
                elif demo == "radial_menu_basic":
                    _assert_radial_menu_basic_visible(capture_path)
                elif demo == "progress_wheel_basic":
                    _assert_progress_wheel_basic_visible(capture_path)
                elif demo == "qrcode_basic":
                    _assert_qrcode_basic_visible(capture_path)
                elif demo == "message_box_basic":
                    _assert_message_box_basic_visible(capture_path)
                    _assert_real_mapping_honesty(demo, completed.stdout)
                elif demo == "animation_basic":
                    _assert_animation_basic_visible(capture_path)
                elif demo == "date_time_basic":
                    _assert_date_time_basic_visible(capture_path)
                elif demo == "clock_basic":
                    _assert_clock_basic_visible(capture_path)
                elif demo == "calendar_basic":
                    _assert_calendar_basic_visible(capture_path)
                elif demo == "legacy_widget_parity":
                    _assert_legacy_widget_parity_visible(capture_path)
                elif demo == "layout_parity":
                    _assert_layout_parity_visible(capture_path)
                elif demo == "grid_parity":
                    _assert_grid_parity_visible(capture_path)
                elif demo == "line_edit_basic":
                    _assert_common_visible(capture_path, demo)
                elif demo == "layout_flex":
                    _assert_layout_flex_visible(capture_path)
                elif demo == "layout_grid":
                    _assert_layout_grid_visible(capture_path)
                else:
                    _assert_common_visible(capture_path, demo)
                _assert_no_unexpected_fallback(demo, completed.stdout)
    finally:
        if args.build_dir is None:
            shutil.rmtree(build_dir, ignore_errors=True)


if __name__ == "__main__":
    main()
