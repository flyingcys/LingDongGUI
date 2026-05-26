import os
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD = ROOT / "build" / "picoui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 6
TARGETS = [
    "picoui_hello_world_demo",
    "picoui_theme_showcase_demo",
    "picoui_settings_panel_demo",
]
STATIC_TARGETS = [
    "picoui_hello_world_demo",
    "picoui_theme_showcase_demo",
]
INTERACTIVE_TARGETS = {
    "picoui_settings_panel_demo": {
        "real_ids": ["title", "apply"],
        "fallback_ids": ["wifi", "brightness"],
    }
}


def _parse_marker_ids(stdout: str, marker: str) -> set[str]:
    prefix = f"{marker}="
    for line in stdout.splitlines():
        if line.startswith(prefix):
            value = line[len(prefix) :].strip()
            if not value:
                return set()
            return {item.strip() for item in value.split(",") if item.strip()}
    raise AssertionError(f"Missing marker line '{marker}='.\nstdout:\n{stdout}")


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


subprocess.run(
    [RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"],
    check=True,
)
subprocess.run(
    [RTK, "cmake", "--build", str(BUILD), "--target", *TARGETS],
    check=True,
)

for target in TARGETS:
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = "1200"
    completed = subprocess.run(
        [str(_find_executable(target))],
        check=False,
        timeout=DEMO_TIMEOUT_SECONDS,
        capture_output=True,
        text=True,
        env=env,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"Demo '{target}' exited with {completed.returncode}.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )

    if "PICOUI_RUNTIME_READY" not in completed.stdout:
        raise AssertionError(
            f"Demo '{target}' missing runtime ready marker.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )

    if target in STATIC_TARGETS:
        if "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI" not in completed.stdout:
            raise AssertionError(
                f"Static demo '{target}' did not report REAL_LDGUI static mapping.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        if "PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK" in completed.stdout:
            raise AssertionError(
                f"Static demo '{target}' should not expose interactive fallback boundary.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        if "PICOUI_BACKEND_REAL_WIDGET_IDS=" not in completed.stdout:
            raise AssertionError(
                f"Static demo '{target}' missing real widget id evidence.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )

    if target in INTERACTIVE_TARGETS:
        expected = INTERACTIVE_TARGETS[target]
        if "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI" not in completed.stdout:
            raise AssertionError(
                f"Interactive demo '{target}' missing REAL_LDGUI static mapping evidence.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        if "PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK" not in completed.stdout:
            raise AssertionError(
                f"Interactive demo '{target}' should still expose A3 fallback boundary.\n"
                f"stdout:\n{completed.stdout}\n"
                f"stderr:\n{completed.stderr}"
            )
        real_ids = _parse_marker_ids(completed.stdout, "PICOUI_BACKEND_REAL_WIDGET_IDS")
        fallback_ids = _parse_marker_ids(completed.stdout, "PICOUI_BACKEND_FALLBACK_WIDGET_IDS")
        for widget_id in expected["real_ids"]:
            if widget_id not in real_ids:
                raise AssertionError(
                    f"Interactive demo '{target}' missing real-mapped widget id '{widget_id}' in REAL set {sorted(real_ids)}.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
        for widget_id in expected["fallback_ids"]:
            if widget_id not in fallback_ids:
                raise AssertionError(
                    f"Interactive demo '{target}' missing fallback widget id '{widget_id}' in FALLBACK set {sorted(fallback_ids)}.\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
