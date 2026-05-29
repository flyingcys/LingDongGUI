import os
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD = ROOT / "build" / "picoui-runtime"
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 6
static_mapping_targets = {
    "picoui_hello_world_demo": {
        "real_ids": ["title", "ok"],
        "reason": "static label/button demo; runtime marker can prove named widgets use real LingDongGUI backend objects.",
    },
    "picoui_theme_showcase_demo": {
        "real_ids": ["title", "body", "accent"],
        "reason": "theme showcase has stable named widgets; marker proves real backend objects, not full theme semantics.",
    },
}
interactive_mapping_targets = {
    "picoui_basic_widgets_demo": {
        "real_ids": ["wifi", "agree", "volume", "submit", "title", "logo"],
        "reason": "basic widget demo has stable static and interactive widget ids.",
    },
    "picoui_settings_panel_demo": {
        "real_ids": ["title", "wifi", "brightness", "apply"],
        "reason": "settings panel has stable interactive widget ids that previously risked fallback behavior.",
    },
    "picoui_list_basic_demo": {
        "real_ids": ["list", "item_wifi", "item_bluetooth", "item_display"],
        "reason": "list demo proves the list and item ids are backed by real LingDongGUI list mapping.",
    },
}
layout_mapping_targets = {
    "picoui_layout_flex_demo": {
        "real_ids": ["first", "second", "third"],
        "reason": "layout ids prove child widgets enter the real backend tree; layout solver semantics stay in layout tests and visible gate.",
    },
    "picoui_layout_grid_demo": {
        "real_ids": ["title", "left", "right"],
        "reason": "grid ids prove named cells enter the real backend tree; placement correctness stays in layout tests and visible gate.",
    },
}
theme_mapping_targets = {
    "picoui_theme_showcase_demo": {
        "real_ids": ["title", "body", "accent"],
        "reason": "theme ids prove themed sample widgets are real backend objects; style color/readback stays in theme tests and visible gate.",
    },
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


def _merge_target_matrix() -> dict[str, dict[str, object]]:
    merged: dict[str, dict[str, object]] = {}
    for category, targets in (
        ("static_mapping_targets", static_mapping_targets),
        ("interactive_mapping_targets", interactive_mapping_targets),
        ("layout_mapping_targets", layout_mapping_targets),
        ("theme_mapping_targets", theme_mapping_targets),
    ):
        for target, config in targets.items():
            entry = merged.setdefault(
                target,
                {
                    "real_ids": [],
                    "categories": [],
                    "reasons": [],
                    "requires_real_widget_ids": True,
                },
            )
            entry["categories"].append(category)
            entry["reasons"].append(config["reason"])
            for widget_id in config["real_ids"]:
                if widget_id not in entry["real_ids"]:
                    entry["real_ids"].append(widget_id)
    return merged


def _assert_target_matrix_complete(target_matrix: dict[str, dict[str, object]]) -> None:
    expected_targets = {
        "picoui_hello_world_demo",
        "picoui_basic_widgets_demo",
        "picoui_layout_flex_demo",
        "picoui_layout_grid_demo",
        "picoui_theme_showcase_demo",
        "picoui_settings_panel_demo",
        "picoui_list_basic_demo",
    }
    missing_targets = sorted(expected_targets - set(target_matrix))
    unexpected_targets = sorted(set(target_matrix) - expected_targets)
    if missing_targets or unexpected_targets:
        raise AssertionError(
            "Backend mapping matrix must explicitly cover the seven visible-gate demos.\n"
            f"missing demos: {missing_targets}\n"
            f"unexpected demos: {unexpected_targets}"
        )

    for target in sorted(expected_targets):
        expected = target_matrix[target]
        if expected["requires_real_widget_ids"] and not expected["real_ids"]:
            raise AssertionError(
                f"Demo '{target}' requires PICOUI_BACKEND_REAL_WIDGET_IDS but has no expected ids.\n"
                f"categories: {expected['categories']}\n"
                f"reasons: {expected['reasons']}"
            )


def _assert_no_fallback(target: str, stdout: str, stderr: str) -> None:
    if "PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK" not in stdout:
        return
    fallback_ids = _parse_marker_ids(stdout, "PICOUI_BACKEND_FALLBACK_WIDGET_IDS")
    raise AssertionError(
        f"Demo '{target}' should not expose fallback backend boundary.\n"
        f"fallback ids: {sorted(fallback_ids)}\n"
        f"stdout:\n{stdout}\n"
        f"stderr:\n{stderr}"
    )


def _assert_real_mapping(target: str, expected: dict[str, object], stdout: str, stderr: str) -> None:
    if "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI" not in stdout:
        raise AssertionError(
            f"Demo '{target}' missing marker PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI.\n"
            f"categories: {expected['categories']}\n"
            f"reasons: {expected['reasons']}\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )

    if not expected["requires_real_widget_ids"]:
        return

    real_ids = _parse_marker_ids(stdout, "PICOUI_BACKEND_REAL_WIDGET_IDS")
    missing_ids = [widget_id for widget_id in expected["real_ids"] if widget_id not in real_ids]
    if missing_ids:
        raise AssertionError(
            f"Demo '{target}' missing real-mapped widget ids: {missing_ids}.\n"
            f"expected ids: {expected['real_ids']}\n"
            f"actual ids: {sorted(real_ids)}\n"
            f"categories: {expected['categories']}\n"
            f"reasons: {expected['reasons']}\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )


target_matrix = _merge_target_matrix()
_assert_target_matrix_complete(target_matrix)
TARGETS = sorted(target_matrix)


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

    _assert_no_fallback(target, completed.stdout, completed.stderr)
    _assert_real_mapping(target, target_matrix[target], completed.stdout, completed.stderr)
