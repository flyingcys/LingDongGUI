import os
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
RTK = shutil.which("rtk") or "rtk"
DEMO_SOURCE = ROOT / "picoui/demo/basic_widgets/main.c"
BUILD_DIR = ROOT / "build"
TARGET = "picoui_basic_widgets_demo"
DEMO_CANDIDATES = (
    BUILD_DIR / "examples" / "sdl" / TARGET,
    BUILD_DIR / TARGET,
    BUILD_DIR / "examples" / TARGET,
)
FORBIDDEN_MARKERS = (
    "picoui/port/sdl.h",
    "picoui_switch_create(",
    "picoui_checkbox_create(",
    "picoui_slider_create(",
    "picoui_text_create(",
    "picoui_image_create(",
    "picoui_grid_set_columns(",
    "picoui_grid_set_rows(",
    "picoui_grid_set_gap(",
    "picoui_widget_set_grid_cell(",
)
REQUIRED_MARKERS = (
    "picoui_window_create_root(",
    "picoui_label_create(",
    "picoui_button_create(",
    "picoui_button_set_on_clicked(",
    "picoui_timer_handler();",
)
TIMEOUT_SECONDS = 6


def _assert_v1_1_minimal_source() -> None:
    text = DEMO_SOURCE.read_text(encoding="utf-8")
    missing = [marker for marker in REQUIRED_MARKERS if marker not in text]
    forbidden = [marker for marker in FORBIDDEN_MARKERS if marker in text]

    assert not missing, f"basic_widgets 缺少 v1.1 最小链标记: {missing}"
    assert not forbidden, (
        "basic_widgets 仍包含非 v1.1 最小链控件/布局痕迹: "
        + ", ".join(forbidden)
    )


def _find_demo_executable() -> Path:
    for candidate in DEMO_CANDIDATES:
        if candidate.is_file():
            return candidate
    checked = ", ".join(str(path) for path in DEMO_CANDIDATES)
    raise AssertionError(f"未找到 {TARGET} 可执行文件，请先构建。已检查: {checked}")


def _assert_runtime_smoke(executable: Path) -> None:
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
    completed = subprocess.run(
        [str(executable)],
        check=False,
        capture_output=True,
        text=True,
        timeout=TIMEOUT_SECONDS,
        env=env,
    )

    assert completed.returncode == 0, (
        f"basic_widgets 运行失败 rc={completed.returncode}\n"
        f"stdout:\n{completed.stdout}\n"
        f"stderr:\n{completed.stderr}"
    )
    assert "PICOUI_RUNTIME_READY" in completed.stdout, (
        "basic_widgets 未进入 runtime ready 状态\n"
        f"stdout:\n{completed.stdout}\n"
        f"stderr:\n{completed.stderr}"
    )


def main() -> int:
    _assert_v1_1_minimal_source()
    subprocess.run([RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)], check=True)
    subprocess.run(
        [RTK, "cmake", "--build", str(BUILD_DIR), "--target", TARGET],
        check=True,
    )
    _assert_runtime_smoke(_find_demo_executable())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
