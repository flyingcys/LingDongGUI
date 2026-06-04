from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
EXECUTABLE_NAME = "ldgui_sdl_demo.exe" if sys.platform.startswith("win") else "ldgui_sdl_demo"

OFF_TRACK = (224, 224, 224)
ON_TRACK = (33, 150, 243)
KNOB = (255, 255, 255)
DISABLED_OFF_TRACK = (112, 112, 112)
DISABLED_ON_TRACK = (64, 126, 168)

SAMPLES = {
    "h_off": ((114, 92), OFF_TRACK),
    "h_off_knob": ((92, 92), KNOB),
    "h_on": ((90, 162), ON_TRACK),
    "h_on_knob": ((116, 162), KNOB),
    "v_off": ((262, 64), OFF_TRACK),
    "v_off_knob": ((262, 100), KNOB),
    "v_on": ((262, 250), ON_TRACK),
    "v_on_knob": ((262, 222), KNOB),
    "disabled_off": ((126, 301), DISABLED_OFF_TRACK),
    "disabled_on": ((90, 370), DISABLED_ON_TRACK),
    "pressed_knob": ((92, 442), KNOB),
    "mid_left": ((256, 450), ON_TRACK),
    "mid_knob": ((270, 442), KNOB),
    "mid_right": ((288, 440), OFF_TRACK),
}

CAPSULE_SAMPLES = {
    "h_on_left_top_corner": ((80, 150), ON_TRACK),
    "h_on_left_bottom_corner": ((80, 173), ON_TRACK),
    "h_on_right_top_corner": ((127, 150), ON_TRACK),
    "h_on_right_bottom_corner": ((127, 173), ON_TRACK),
    "h_on_left_mid": ((80, 162), ON_TRACK),
    "h_on_right_mid": ((127, 162), ON_TRACK),
    "v_on_top_left_corner": ((250, 210), ON_TRACK),
    "v_on_top_right_corner": ((273, 210), ON_TRACK),
    "v_on_bottom_left_corner": ((250, 257), ON_TRACK),
    "v_on_bottom_right_corner": ((273, 257), ON_TRACK),
    "v_on_top_mid": ((262, 210), ON_TRACK),
    "v_on_bottom_mid": ((262, 257), ON_TRACK),
}


def _run(cmd: list[str], cwd: Path | None = None) -> None:
    subprocess.run(cmd, cwd=cwd, check=True)


def _read_token(data: bytes, index: int) -> tuple[bytes, int]:
    size = len(data)
    while index < size and data[index] in b" \t\r\n":
        index += 1
    if index < size and data[index] == ord("#"):
        while index < size and data[index] not in b"\r\n":
            index += 1
        return _read_token(data, index)
    start = index
    while index < size and data[index] not in b" \t\r\n":
        index += 1
    return data[start:index], index


def _read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    magic, index = _read_token(data, 0)
    width_token, index = _read_token(data, index)
    height_token, index = _read_token(data, index)
    max_token, index = _read_token(data, index)
    if magic != b"P6":
        raise AssertionError(f"{path} 不是 P6 PPM: {magic!r}")
    width = int(width_token)
    height = int(height_token)
    max_value = int(max_token)
    if max_value != 255:
        raise AssertionError(f"{path} max value 应为 255，实际 {max_value}")
    while index < len(data) and data[index] in b" \t\r\n":
        index += 1
    pixels = data[index:]
    expected_size = width * height * 3
    if len(pixels) != expected_size:
        raise AssertionError(f"{path} 像素数据长度 {len(pixels)}，预期 {expected_size}")
    return width, height, pixels


def _pixel(width: int, height: int, pixels: bytes, x: int, y: int) -> tuple[int, int, int]:
    if not (0 <= x < width and 0 <= y < height):
        raise AssertionError(f"采样坐标越界: ({x}, {y}) not in {width}x{height}")
    offset = (y * width + x) * 3
    return pixels[offset], pixels[offset + 1], pixels[offset + 2]


def _assert_close(name: str, actual: tuple[int, int, int], expected: tuple[int, int, int], tolerance: int) -> None:
    deltas = tuple(abs(a - e) for a, e in zip(actual, expected))
    if any(delta > tolerance for delta in deltas):
        raise AssertionError(f"{name} 采样 {actual}，预期接近 {expected}，容差 {tolerance}")


def _assert_not_close(name: str, actual: tuple[int, int, int], unexpected: tuple[int, int, int], tolerance: int) -> None:
    deltas = tuple(abs(a - e) for a, e in zip(actual, unexpected))
    if all(delta <= tolerance for delta in deltas):
        raise AssertionError(f"{name} 采样 {actual}，不应接近 {unexpected}，容差 {tolerance}")


def _brightness(color: tuple[int, int, int]) -> float:
    return sum(color) / 3.0


def _wait_for_capture(
    process: subprocess.Popen[str], capture_path: Path, timeout_seconds: float
) -> tuple[int, int, bytes]:
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        return_code = process.poll()
        if return_code is not None:
            stdout, stderr = process.communicate(timeout=1)
            raise AssertionError(
                f"demo 在生成截图前退出，returncode={return_code}\n{stdout}{stderr}"
            )
        if capture_path.exists() and capture_path.stat().st_size > 0:
            try:
                return _read_ppm(capture_path)
            except AssertionError:
                pass
        time.sleep(0.05)
    stdout, stderr = process.communicate(timeout=1) if process.poll() is not None else ("", "")
    raise AssertionError(f"等待截图超时: {capture_path}\n{stdout}{stderr}")


def _terminate(process: subprocess.Popen[str]) -> None:
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.communicate(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()
        process.communicate(timeout=3)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build demo0 and verify the switch screenshot matrix.")
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=8.0)
    args = parser.parse_args()

    build_dir = args.build_dir.resolve()
    _run(["cmake", "-S", str(SDL_ROOT), "-B", str(build_dir), "-DUSE_DEMO=0"])
    _run(["cmake", "--build", str(build_dir), "--target", "ldgui_sdl_demo"])

    demo_path = build_dir / EXECUTABLE_NAME
    if not demo_path.is_file():
        print(f"[FAIL] 缺少 demo 可执行文件: {demo_path}", file=sys.stderr)
        return 1

    with tempfile.TemporaryDirectory(prefix="ldswitch-capture-") as tmp:
        capture_path = Path(tmp) / "switch_matrix.ppm"
        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = "dummy"
        env["LD_SWITCH_CAPTURE_MATRIX"] = "1"
        env["LD_SWITCH_CAPTURE_FILE"] = str(capture_path)

        process = subprocess.Popen(
            [str(demo_path)],
            cwd=build_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            width, height, pixels = _wait_for_capture(process, capture_path, args.timeout)
        finally:
            _terminate(process)
        sampled: dict[str, tuple[int, int, int]] = {}
        for name, (point, expected) in SAMPLES.items():
            sampled[name] = _pixel(width, height, pixels, point[0], point[1])
            _assert_close(name, sampled[name], expected, 18)
        for name, (point, expected) in CAPSULE_SAMPLES.items():
            sampled[name] = _pixel(width, height, pixels, point[0], point[1])
            if name.endswith("_corner"):
                _assert_not_close(name, sampled[name], expected, 18)
            else:
                _assert_close(name, sampled[name], expected, 18)

        if not (_brightness(sampled["h_off_knob"]) > _brightness(sampled["h_off"]) + 12):
            raise AssertionError(f"h_off knob 没有在灰色 track 内形成白色圆点: {sampled}")
        if not (_brightness(sampled["h_on_knob"]) > _brightness(sampled["h_on"]) + 60):
            raise AssertionError(f"h_on knob 没有在蓝色 track 内形成白色圆点: {sampled}")
        if not (_brightness(sampled["v_off_knob"]) > _brightness(sampled["v_off"]) + 12):
            raise AssertionError(f"v_off knob 没有在灰色 track 内形成白色圆点: {sampled}")
        if not (_brightness(sampled["v_on_knob"]) > _brightness(sampled["v_on"]) + 60):
            raise AssertionError(f"v_on knob 没有在蓝色 track 内形成白色圆点: {sampled}")

        if not (_brightness(sampled["disabled_off"]) < _brightness(sampled["h_off"]) - 40):
            raise AssertionError(f"disabled_off 亮度没有明显低于 h_off: {sampled}")
        if not (_brightness(sampled["disabled_on"]) < _brightness(sampled["h_on"]) - 15):
            raise AssertionError(f"disabled_on 亮度没有明显低于 h_on: {sampled}")

    print("[PASS] switch capture matrix pixels match off/on/vertical/disabled/pressed/mid expectations")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
