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
        "real_ids": ["list"],
        "reason": "list demo proves the list object itself is a real LingDongGUI widget; item ids are intentionally excluded from the real widget marker contract.",
    },
    "picoui_progress_bar_basic_demo": {
        "real_ids": ["primary", "secondary", "title"],
        "reason": "progress bar demo proves both horizontal and vertical progress bars are real LingDongGUI widgets.",
    },
    "picoui_arc_basic_demo": {
        "real_ids": ["title", "arc"],
        "reason": "arc demo proves the arc widget is a real LingDongGUI widget with named backend ids.",
    },
    "picoui_gauge_basic_demo": {
        "real_ids": ["title", "gauge"],
        "reason": "gauge demo proves the gauge widget is a real LingDongGUI widget with named backend ids.",
    },
    "picoui_icon_slider_basic_demo": {
        "real_ids": ["title", "icon_slider"],
        "reason": "icon slider demo proves the composite icon slider widget is a real LingDongGUI widget.",
    },
    "picoui_radial_menu_basic_demo": {
        "real_ids": ["title", "radial_menu"],
        "reason": "radial menu demo proves the composite radial menu widget is a real LingDongGUI widget.",
    },
    "picoui_progress_wheel_basic_demo": {
        "real_ids": ["title", "wheel"],
        "reason": "progress wheel demo proves the progress wheel widget is a real LingDongGUI widget.",
    },
    "picoui_qrcode_basic_demo": {
        "real_ids": ["title", "qrcode"],
        "reason": "qrcode demo proves the QR code widget is a real LingDongGUI widget.",
    },
    "picoui_message_box_basic_demo": {
        "real_ids": ["message_box"],
        "reason": "message_box demo should prove the dialog widget is a real LingDongGUI widget and no longer depend on temporary smoke-path exclusion.",
    },
    "picoui_date_time_basic_demo": {
        "real_ids": ["title", "date_time"],
        "reason": "date_time demo proves the date-time widget is a real LingDongGUI widget.",
    },
    "picoui_clock_basic_demo": {
        "real_ids": ["clock"],
        "reason": "clock demo proves the clock widget itself is a real LingDongGUI widget.",
    },
    "picoui_keyboard_basic_demo": {
        "real_ids": ["keyboard_demo_input", "keyboard_demo_keyboard"],
        "reason": "keyboard demo proves both the edit target and keyboard widget are real LingDongGUI widgets.",
    },
    "picoui_line_edit_basic_demo": {
        "real_ids": ["title", "line_edit"],
        "reason": "line_edit demo proves the editable text widget is a real LingDongGUI widget.",
    },
    "picoui_combo_box_basic_demo": {
        "real_ids": ["title", "combo_box"],
        "reason": "combo_box demo proves the dropdown widget is a real LingDongGUI widget.",
    },
    "picoui_scroll_selecter_basic_demo": {
        "real_ids": ["title", "scroll_selecter"],
        "reason": "scroll_selecter demo proves the scroll selecter widget is a real LingDongGUI widget.",
    },
    "picoui_table_basic_demo": {
        "real_ids": ["table"],
        "reason": "table demo proves the table widget itself is a real LingDongGUI widget.",
    },
    "picoui_graph_basic_demo": {
        "real_ids": ["title", "graph"],
        "reason": "graph demo proves the graph widget itself is a real LingDongGUI widget.",
    },
    "picoui_calendar_basic_demo": {
        "real_ids": ["title", "calendar"],
        "reason": "calendar demo proves the calendar widget itself is a real LingDongGUI widget.",
    },
    "picoui_animation_basic_demo": {
        "real_ids": ["title", "animation"],
        "reason": "animation demo proves the animation widget itself is a real LingDongGUI widget with a frame tile source.",
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


def _parse_smoke_layout_marker(stdout: str) -> int:
    prefix = "PICOUI_SMOKE_LAYOUT_USED="
    for line in stdout.splitlines():
        if not line.startswith(prefix):
            continue
        value = line[len(prefix) :].strip()
        if value in {"0", "1"}:
            return int(value)
        raise AssertionError(f"Invalid smoke layout marker: {line}\nstdout:\n{stdout}")
    raise AssertionError(f"Missing marker line '{prefix}'.\nstdout:\n{stdout}")


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
        "picoui_progress_bar_basic_demo",
        "picoui_arc_basic_demo",
        "picoui_gauge_basic_demo",
        "picoui_icon_slider_basic_demo",
        "picoui_radial_menu_basic_demo",
        "picoui_progress_wheel_basic_demo",
        "picoui_qrcode_basic_demo",
        "picoui_message_box_basic_demo",
        "picoui_date_time_basic_demo",
        "picoui_clock_basic_demo",
        "picoui_keyboard_basic_demo",
        "picoui_line_edit_basic_demo",
        "picoui_combo_box_basic_demo",
        "picoui_scroll_selecter_basic_demo",
        "picoui_table_basic_demo",
        "picoui_graph_basic_demo",
        "picoui_calendar_basic_demo",
        "picoui_animation_basic_demo",
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
    if _parse_smoke_layout_marker(stdout) != 0:
        raise AssertionError(
            f"Demo '{target}' used temporary smoke layout and cannot be counted as formal mapping evidence.\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )

    if "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI" not in stdout:
        raise AssertionError(
            f"Demo '{target}' missing formal real-mapping marker PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI.\n"
            "This checker only accepts explicit runtime evidence that the demo entered the real LingDongGUI mapping path.\n"
            "It does not require a dedicated backend_*.c file layout, but it does require the marker contract to remain truthful.\n"
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
            "Formal mapping proof is the marker plus the named real backend ids for this demo, not any specific dedicated backend source-file split.\n"
            f"expected ids: {expected['real_ids']}\n"
            f"actual ids: {sorted(real_ids)}\n"
            f"categories: {expected['categories']}\n"
            f"reasons: {expected['reasons']}\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )


def _assert_demo_excluded_from_formal_mapping(target: str, stdout: str, stderr: str) -> None:
    if "PICOUI_RUNTIME_READY" not in stdout:
        raise AssertionError(
            f"Demo '{target}' no longer reports runtime-ready state.\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )
    if "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI" in stdout:
        raise AssertionError(
            f"Demo '{target}' still emits formal REAL_LDGUI mapping markers.\n"
            "R0 honesty requires this demo to stay outside the formal mapping conclusion until the temporary smoke path is isolated.\n"
            f"stdout:\n{stdout}\n"
            f"stderr:\n{stderr}"
        )
    if "PICOUI_BACKEND_TEMPORARY_SMOKE_PATH=EXCLUDED_FORMAL_MAPPING" not in stdout:
        raise AssertionError(
            f"Demo '{target}' must explicitly report temporary smoke-path evidence.\n"
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
