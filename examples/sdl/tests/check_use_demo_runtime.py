from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
EXECUTABLE_NAME = "ldgui_sdl_demo.exe" if sys.platform.startswith("win") else "ldgui_sdl_demo"


def _run(cmd: list[str], cwd: Path | None = None) -> None:
    subprocess.run(cmd, cwd=cwd, check=True)


def _terminate_after_smoke(process: subprocess.Popen[str], seconds: float) -> tuple[int | None, str]:
    try:
        time.sleep(seconds)
        return_code = process.poll()
        if return_code is not None:
            stdout, stderr = process.communicate(timeout=1)
            return return_code, stdout + stderr

        process.terminate()
        try:
            stdout, stderr = process.communicate(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            stdout, stderr = process.communicate(timeout=3)
        return None, stdout + stderr
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate(timeout=3)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build and smoke-run one USE_DEMO variant with SDL dummy video.")
    parser.add_argument("--demo", choices=("0", "6"), required=True)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--seconds", type=float, default=1.0)
    args = parser.parse_args()

    build_dir = args.build_dir.resolve()
    _run(["cmake", "-S", str(SDL_ROOT), "-B", str(build_dir), f"-DUSE_DEMO={args.demo}"])
    _run(["cmake", "--build", str(build_dir), "--target", "ldgui_sdl_demo"])

    demo_path = build_dir / EXECUTABLE_NAME
    if not demo_path.is_file():
        print(f"[FAIL] 缺少 demo 可执行文件: {demo_path}", file=sys.stderr)
        return 1

    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = "dummy"
    process = subprocess.Popen(
        [str(demo_path)],
        cwd=build_dir,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return_code, output = _terminate_after_smoke(process, args.seconds)
    if return_code is not None:
        print(f"[FAIL] USE_DEMO={args.demo} dummy run 立即退出，returncode={return_code}", file=sys.stderr)
        if output:
            print(output, file=sys.stderr)
        return 1

    print(f"[PASS] USE_DEMO={args.demo} dummy run 存活 {args.seconds:.1f}s 后正常终止")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
