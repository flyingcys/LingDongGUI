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
REPO_ROOT = TEST_FILE.parents[3]
EXECUTABLE_NAME = "ldgui_sdl_demo.exe" if sys.platform.startswith("win") else "ldgui_sdl_demo"


def _run(cmd: list[str], cwd: Path | None = None) -> None:
    subprocess.run(cmd, cwd=cwd, check=True)


def _read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if not data.startswith(b"P6\n"):
        raise AssertionError(f"{path} 不是 P6 PPM")
    header, pixels = data.split(b"\n255\n", 1)
    _, dims = header.split(b"\n", 1)
    width_token, height_token = dims.split()
    width = int(width_token)
    height = int(height_token)
    expected = width * height * 3
    if len(pixels) != expected:
        raise AssertionError(f"{path} 像素长度 {len(pixels)}，预期 {expected}")
    return width, height, pixels


def _avg_brightness(pixels: bytes) -> float:
    return sum(pixels) / float(len(pixels))


def _wait_for_capture(process: subprocess.Popen[str], capture_path: Path, timeout_seconds: float) -> tuple[int, int, bytes]:
    deadline = time.monotonic() + timeout_seconds
    last_size = -1
    stable_count = 0
    while time.monotonic() < deadline:
        if capture_path.exists():
            size = capture_path.stat().st_size
            if size > 32:
                if size == last_size:
                    stable_count += 1
                else:
                    stable_count = 0
                    last_size = size
                if stable_count >= 2:
                    return _read_ppm(capture_path)
        if process.poll() is not None:
            stdout, stderr = process.communicate(timeout=1)
            raise AssertionError(
                f"demo 在生成截图前退出，returncode={process.returncode}\n{stdout}{stderr}"
            )
        time.sleep(0.05)
    raise TimeoutError(f"等待截图超时: {capture_path}")


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
    parser = argparse.ArgumentParser(description="Build one USE_DEMO variant and assert first-frame capture exists.")
    parser.add_argument("--demo", choices=("1", "2", "3", "4", "5"), required=True)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=8.0)
    args = parser.parse_args()

    if args.demo == "5":
        settings_demo = REPO_ROOT / "picoui" / "demo" / "settings_panel" / "main.c"
        demo_source = settings_demo.read_text(encoding="utf-8")
        forbidden_markers = (
            ".width =",
            ".height =",
            "picoui_widget_set_size(",
            "picoui_widget_set_pos(",
        )
        if any(marker in demo_source for marker in forbidden_markers):
            raise AssertionError(
                "A4 RED: settings_panel 仍含 demo 侧硬编码尺寸/位置补丁，"
                f"禁止进入 GREEN。source={settings_demo}"
            )

    build_dir = args.build_dir.resolve()
    _run(["cmake", "-S", str(SDL_ROOT), "-B", str(build_dir), f"-DUSE_DEMO={args.demo}"])
    _run(["cmake", "--build", str(build_dir), "--target", "ldgui_sdl_demo"])

    demo_path = build_dir / EXECUTABLE_NAME
    if not demo_path.is_file():
        print(f"[FAIL] 缺少 demo 可执行文件: {demo_path}", file=sys.stderr)
        return 1

    with tempfile.TemporaryDirectory(prefix=f"use-demo-{args.demo}-") as tmpdir:
        capture_path = Path(tmpdir) / f"demo-{args.demo}.ppm"
        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = "dummy"
        env["LD_CAPTURE_FILE"] = str(capture_path)
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

        if width <= 0 or height <= 0:
            raise AssertionError(f"截图尺寸非法: {width}x{height}")
        if _avg_brightness(pixels) <= 4.0:
            raise AssertionError(f"截图平均亮度过低，疑似空白帧: {_avg_brightness(pixels):.2f}")

    print(f"[PASS] USE_DEMO={args.demo} first-frame capture exists and is non-blank")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
