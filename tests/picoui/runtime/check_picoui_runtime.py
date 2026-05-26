import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD = ROOT / "build" / "picoui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 8
TARGETS = [
    "picoui_hello_world_demo",
    "picoui_basic_widgets_demo",
    "picoui_layout_flex_demo",
    "picoui_layout_grid_demo",
    "picoui_theme_showcase_demo",
    "picoui_settings_panel_demo",
]

subprocess.run([
    RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"
], check=True)
subprocess.run([
    RTK, "cmake", "--build", str(BUILD), "--target", *TARGETS
], check=True)

for target in TARGETS:
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

    try:
        subprocess.run([str(executable)], check=True, timeout=DEMO_TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired as exc:
        raise TimeoutError(
            f"Demo '{target}' timed out after {DEMO_TIMEOUT_SECONDS} seconds"
        ) from exc
