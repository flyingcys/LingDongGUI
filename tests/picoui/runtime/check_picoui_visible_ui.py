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
    "basic_widgets": "picoui_basic_widgets_demo",
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


def _non_background_bounds(
    width: int,
    height: int,
    pixels: bytes,
    bg: tuple[int, int, int],
) -> tuple[int, int, int, int] | None:
    xs: list[int] = []
    ys: list[int] = []
    for y in range(0, height, 2):
        for x in range(0, width, 2):
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


def _assert_basic_widgets_visible(path: Path) -> None:
    width, height, pixels = _read_ppm(path)
    if width != 480 or height != 320:
        raise AssertionError(f"VISIBLE FAIL: unexpected basic_widgets capture size: {width}x{height}")

    sampled_luma = _sample_luma(width, height, pixels)
    p90 = _percentile(sampled_luma, 0.90)
    p99 = _percentile(sampled_luma, 0.99)
    bg = _pixel(width, pixels, 8, 8)
    bounds = _non_background_bounds(width, height, pixels, bg)
    if bounds is None:
        raise AssertionError("SMOKE FAIL: capture has no non-background pixels")

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


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check PicoUI visible correctness evidence for selected demos."
    )
    parser.add_argument("--demo", choices=sorted(DEMOS), default="basic_widgets")
    args = parser.parse_args()
    target = DEMOS[args.demo]

    subprocess.run([RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"], check=True)
    subprocess.run([RTK, "cmake", "--build", str(BUILD), "--target", target], check=True)

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
        if args.demo == "basic_widgets":
            _assert_basic_widgets_visible(capture_path)


if __name__ == "__main__":
    main()
