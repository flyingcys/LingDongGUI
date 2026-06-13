import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "tinyui-runtime"
DEMO_TIMEOUT_SECONDS = 8
DEMOS = {
    "hello_world": "tinyui_hello_world_demo",
    "basic_widgets": "tinyui_basic_widgets_demo",
    "layout_flex": "tinyui_layout_flex_demo",
    "layout_grid": "tinyui_layout_grid_demo",
    "theme_showcase": "tinyui_theme_showcase_demo",
    "settings_panel": "tinyui_settings_panel_demo",
    "list_basic": "tinyui_list_basic_demo",
    "progress_bar_basic": "tinyui_progress_bar_basic_demo",
    "arc_basic": "tinyui_arc_basic_demo",
    "gauge_basic": "tinyui_gauge_basic_demo",
    "icon_slider_basic": "tinyui_icon_slider_basic_demo",
    "radial_menu_basic": "tinyui_radial_menu_basic_demo",
    "progress_wheel_basic": "tinyui_progress_wheel_basic_demo",
    "qrcode_basic": "tinyui_qrcode_basic_demo",
    "message_box_basic": "tinyui_message_box_basic_demo",
    "date_time_basic": "tinyui_date_time_basic_demo",
    "clock_basic": "tinyui_clock_basic_demo",
    "keyboard_basic": "tinyui_keyboard_basic_demo",
    "line_edit_basic": "tinyui_line_edit_basic_demo",
    "combo_box_basic": "tinyui_combo_box_basic_demo",
    "scroll_selecter_basic": "tinyui_scroll_selecter_basic_demo",
    "table_basic": "tinyui_table_basic_demo",
    "graph_basic": "tinyui_graph_basic_demo",
    "calendar_basic": "tinyui_calendar_basic_demo",
    "animation_basic": "tinyui_animation_basic_demo",
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
        ROOT / "build" / "tinyui-runtime" / "examples" / "sdl" / f"{target}{suffix}",
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
    print("TINYUI_MANUAL_WINDOW_ARTIFACT=SKIP")
    print("ARTIFACT_ENTRY_EXISTS=0")
    print("MANUAL_REVIEW_REQUIRED=1")
    print("MANUAL_REVIEWED_PASSED=0")
    for key, value in metadata.items():
        print(f"{key}={value}")
    print(f"SKIP_REASON={message}")
    return 0


def _print_metadata(status: str, metadata: dict[str, str]) -> None:
    print(f"TINYUI_MANUAL_WINDOW_ARTIFACT={status}")
    print(f"ARTIFACT_ENTRY_EXISTS={1 if status == 'ARTIFACT_READY' else 0}")
    print("MANUAL_REVIEW_REQUIRED=1")
    print("MANUAL_REVIEWED_PASSED=0")
    for key, value in metadata.items():
        print(f"{key}={value}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="生成 TinyUI v2.1 demo-level 人工窗口验收 artifact。"
    )
    parser.add_argument("--demo", choices=sorted(DEMOS), default="basic_widgets")
    parser.add_argument("--all", action="store_true", help="为全部 native-100 demo 生成 artifact")
    parser.add_argument("--build-dir", default=str(DEFAULT_BUILD), help="包含 demo target 的 CMake build 目录")
    parser.add_argument(
        "--artifact-root",
        default=str(ROOT / "artifacts" / "tinyui" / "manual-window"),
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
            print("TINYUI_MANUAL_WINDOW_ARTIFACT=FAIL")
            for key, value in metadata.items():
                print(f"{key}={value}")
            print(f"ERROR={exc}")
            return 2

        artifact_path.parent.mkdir(parents=True, exist_ok=True)
        if artifact_path.exists():
            artifact_path.unlink()

        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = driver
        env["TINYUI_DEMO_AUTO_QUIT_MS"] = "1200"
        env["TINYUI_CAPTURE_FILE"] = str(artifact_path)
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

        if "TINYUI_RUNTIME_READY" not in completed.stdout:
            _print_metadata("FAIL", metadata)
            print("ERROR=demo 未输出 TINYUI_RUNTIME_READY，不能作为可追溯 artifact 记录。")
            return 2

        if not artifact_path.is_file() or artifact_path.stat().st_size <= 32:
            _print_metadata("FAIL", metadata)
            print("ERROR=未生成有效 frame.ppm artifact。")
            return 2

        _print_metadata("ARTIFACT_READY", metadata)
        print("MANUAL_CONCLUSION=脚本只证明 artifact_entry_exists；manual_review_required=true 不能当作 manual pass，人工窗口验收通过必须另有 manual_reviewed_passed=true 记录。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
