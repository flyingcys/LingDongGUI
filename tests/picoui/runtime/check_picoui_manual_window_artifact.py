import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "picoui-runtime"
DEMO_TIMEOUT_SECONDS = 8
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
}


def _default_sdl_driver() -> str:
    requested = os.environ.get("SDL_VIDEODRIVER", "").strip()
    if requested:
        return requested
    if sys.platform == "darwin":
        return "cocoa"
    if sys.platform.startswith("linux"):
        if os.environ.get("WAYLAND_DISPLAY"):
            return "wayland"
        if os.environ.get("DISPLAY"):
            return "x11"
        return ""
    return ""


def _find_executable(build_dir: Path, target: str) -> Path:
    suffix = ".exe" if os.name == "nt" else ""
    candidates = [
        build_dir / "examples" / "sdl" / f"{target}{suffix}",
        build_dir / target / f"{target}{suffix}",
        build_dir / f"{target}{suffix}",
        build_dir / "examples" / f"{target}{suffix}",
        ROOT / "build" / "picoui-runtime" / "examples" / "sdl" / f"{target}{suffix}",
        ROOT / "build" / "examples" / "sdl" / f"{target}{suffix}",
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        checked = "\n  - ".join(str(path) for path in candidates)
        raise FileNotFoundError(
            f"找不到 demo target '{target}' 的可执行文件。已检查：\n  - {checked}"
        )
    return executable


def _skip(message: str, metadata: dict[str, str]) -> int:
    print("PICOUI_MANUAL_WINDOW_ARTIFACT=SKIP")
    for key, value in metadata.items():
        print(f"{key}={value}")
    print(f"SKIP_REASON={message}")
    return 0


def _print_metadata(status: str, metadata: dict[str, str]) -> None:
    print(f"PICOUI_MANUAL_WINDOW_ARTIFACT={status}")
    for key, value in metadata.items():
        print(f"{key}={value}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="生成 PicoUI a-0.7 native-100 demo-level 人工窗口验收 artifact。"
    )
    parser.add_argument("--demo", choices=sorted(DEMOS), default="basic_widgets")
    parser.add_argument("--all", action="store_true", help="为全部 native-100 demo 生成 artifact")
    parser.add_argument("--build-dir", default=str(DEFAULT_BUILD), help="包含 demo target 的 CMake build 目录")
    parser.add_argument(
        "--artifact-root",
        default=str(ROOT / "artifacts" / "picoui" / "manual-window"),
        help="artifact 输出根目录",
    )
    args = parser.parse_args()

    selected_demos = sorted(DEMOS) if args.all else [args.demo]
    build_dir = Path(args.build_dir).resolve()
    driver = _default_sdl_driver()
    platform_text = f"{platform.system()} {platform.release()} ({platform.machine()})"

    if not driver:
        return _skip(
            "当前环境未设置 SDL_VIDEODRIVER，且未检测到可用桌面窗口环境；C6 可选门禁不应让无窗口 CI 失败。",
            {
                "PLATFORM": platform_text,
                "SDL_VIDEO_DRIVER": driver or "<unset>",
                "BUILD_DIR": str(build_dir),
            },
        )

    if driver == "dummy":
        return _skip(
            "SDL_VIDEODRIVER=dummy 只能生成 readback artifact，不能支撑人工 OS 窗口验收结论。",
            {
                "PLATFORM": platform_text,
                "SDL_VIDEO_DRIVER": driver or "<unset>",
                "BUILD_DIR": str(build_dir),
            },
        )

    for demo in selected_demos:
        target = DEMOS[demo]
        artifact_path = Path(args.artifact_root).resolve() / demo / "frame.ppm"
        metadata = {
            "PLATFORM": platform_text,
            "SDL_VIDEO_DRIVER": driver or "<unset>",
            "DEMO_TARGET": target,
            "BUILD_DIR": str(build_dir),
            "ARTIFACT_PATH": str(artifact_path),
        }

        try:
            executable = _find_executable(build_dir, target)
        except FileNotFoundError as exc:
            print("PICOUI_MANUAL_WINDOW_ARTIFACT=FAIL")
            for key, value in metadata.items():
                print(f"{key}={value}")
            print(f"ERROR={exc}")
            return 2

        artifact_path.parent.mkdir(parents=True, exist_ok=True)
        if artifact_path.exists():
            artifact_path.unlink()

        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = driver
        env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
        env["PICOUI_CAPTURE_FILE"] = str(artifact_path)
        command = [str(executable)]
        metadata["RUN_COMMAND"] = " ".join(command)

        try:
            completed = subprocess.run(
                command,
                check=False,
                timeout=DEMO_TIMEOUT_SECONDS,
                capture_output=True,
                text=True,
                env=env,
            )
        except subprocess.TimeoutExpired:
            _print_metadata("FAIL", metadata)
            print(f"ERROR=demo target '{target}' 超时，未能稳定生成人工窗口 artifact。")
            return 2

        if completed.stdout:
            print(completed.stdout, end="")
        if completed.stderr:
            print(completed.stderr, end="", file=sys.stderr)

        if completed.returncode != 0:
            _print_metadata("FAIL", metadata)
            print(f"ERROR=demo target '{target}' 退出码为 {completed.returncode}。")
            return 2

        if "PICOUI_RUNTIME_READY" not in completed.stdout:
            _print_metadata("FAIL", metadata)
            print("ERROR=demo 未输出 PICOUI_RUNTIME_READY，不能作为可追溯 artifact 记录。")
            return 2

        if not artifact_path.is_file() or artifact_path.stat().st_size <= 32:
            _print_metadata("FAIL", metadata)
            print("ERROR=未生成有效 frame.ppm artifact。")
            return 2

        _print_metadata("ARTIFACT_READY", metadata)
        print("MANUAL_CONCLUSION=脚本只证明 artifact 已生成；人工窗口验收结论必须写入 C-线人工窗口验收记录。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
