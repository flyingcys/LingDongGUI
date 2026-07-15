#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BASELINE = ROOT / "tests" / "tinyui" / "perf" / "tinyui_perf_baseline.json"
REQUIRED_SYMBOLS = frozenset(
    {
        "tinyui_init",
        "tinyui_deinit",
        "tinyui_screen_create",
        "tinyui_screen_load",
        "tinyui_process",
        "tinyui_label_create",
        "tinyui_label_set_text",
        "tinyui_button_create",
        "tinyui_button_set_text",
    }
)
# Disabled under TINYUI_PROFILE=minimal. Keep prefixes sorted; cover every
# non-minimal public/module surface that nm can observe on the consumer:
# remaining widgets, theme, native interop wrap APIs, and the private v2.2
# demo bridge (tinyui_app_*/tinyui_widget_*/legacy spellings). Diagnostics
# has no dedicated public symbol prefix when off (compile-time string path).
FORBIDDEN_SYMBOL_PREFIXES = (
    "tinyui_animation_",
    "tinyui_app_",
    "tinyui_arc_",
    "tinyui_calendar_",
    "tinyui_canvas_",
    "tinyui_checkbox_",
    "tinyui_clock_",
    "tinyui_combo_box_",
    "tinyui_date_time_",
    "tinyui_gauge_",
    "tinyui_graph_",
    "tinyui_icon_slider_",
    "tinyui_image_",
    "tinyui_keyboard_",
    "tinyui_line_edit_",
    "tinyui_list_",
    "tinyui_message_box_",
    "tinyui_native_",
    "tinyui_progress_",
    "tinyui_qrcode_",
    "tinyui_radial_menu_",
    "tinyui_scroll_selecter_",
    "tinyui_scroll_selector_",
    "tinyui_slider_",
    "tinyui_switch_",
    "tinyui_table_",
    "tinyui_text_",
    "tinyui_theme_",
    "tinyui_timer_handler",
    "tinyui_widget_",
)
LOGICAL_ARTIFACT_MARKERS = (("examples", "sdl"), ("tests", "tinyui"))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check the TinyUI minimal consumer symbol set.")
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, default=DEFAULT_BASELINE)
    parser.add_argument("--mode", choices=("baseline", "enforce"), default="enforce")
    parser.add_argument("--enforce", action="store_true")
    parser.add_argument("--map", dest="map_path", type=Path)
    parser.add_argument("--build-dir", type=Path)
    return parser.parse_args()


def parse_nm_output(stdout: str) -> set[str]:
    symbols: set[str] = set()
    for line in stdout.splitlines():
        fields = line.split()
        if not fields:
            continue
        symbol_type = fields[-2].upper() if len(fields) >= 2 else ""
        symbol_name = fields[-1]
        if symbol_type == "U" and symbol_name.lstrip("_").startswith("tinyui_"):
            raise ValueError(f"undefined symbol in artifact: {symbol_name}")
        if len(fields) >= 3 and symbol_type not in {"A", "B", "C", "D", "R", "S", "T", "V", "W"}:
            continue
        symbols.add(symbol_name.lstrip("_"))
    return symbols


def enforced_policy(profile: dict[str, object]) -> tuple[set[str], tuple[str, ...]]:
    required = profile.get("required_symbols")
    forbidden = profile.get("forbidden_symbols")
    if not isinstance(required, list) or not all(isinstance(item, str) for item in required):
        raise ValueError("baseline minimal.required_symbols must be a string array")
    if not isinstance(forbidden, list) or not all(isinstance(item, str) for item in forbidden):
        raise ValueError("baseline minimal.forbidden_symbols must be a string array")
    return set(required), tuple(forbidden)


def check_symbols(
    symbols: set[str],
    required_symbols: set[str] | None = None,
    forbidden_symbols: tuple[str, ...] | None = None,
) -> None:
    required = REQUIRED_SYMBOLS if required_symbols is None else required_symbols
    forbidden = FORBIDDEN_SYMBOL_PREFIXES if forbidden_symbols is None else forbidden_symbols
    missing = sorted(required - symbols)
    if missing:
        raise AssertionError("missing required symbol(s): " + ", ".join(missing))
    unexpected = sorted(
        symbol
        for symbol in symbols
        if symbol.startswith(forbidden)
    )
    if unexpected:
        raise AssertionError("non-minimal symbol(s): " + ", ".join(unexpected))


def _artifact_key(path: str | Path) -> str:
    """Normalize relative baseline paths from the repository root."""
    candidate = Path(path)
    if not candidate.is_absolute():
        candidate = ROOT / candidate
    candidate = candidate.resolve()
    try:
        return candidate.relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def _logical_artifact_key(path: str | Path) -> str:
    """Use the stable artifact suffix, independent of build profile directory."""
    key = _artifact_key(path)
    parts = tuple(Path(key).parts)
    for marker in LOGICAL_ARTIFACT_MARKERS:
        for index in range(len(parts) - len(marker) + 1):
            if parts[index : index + len(marker)] == marker:
                return Path(*parts[index:]).as_posix()
    return key


def _infer_build_dir(binary: Path) -> Path | None:
    parts = binary.resolve().parts
    for marker in LOGICAL_ARTIFACT_MARKERS:
        for index in range(len(parts) - len(marker) + 1):
            if parts[index : index + len(marker)] == marker:
                return Path(*parts[:index])
    return None


def _resolve_binary_argument(binary: Path, build_dir: Path | None) -> Path:
    if binary.is_absolute() or build_dir is None or binary.is_file():
        return binary
    relocated = build_dir / binary
    return relocated if relocated.is_file() else binary


def load_baseline(path: Path, artifact: str | Path) -> dict[str, object]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ValueError(f"missing baseline file: {path}") from exc
    minimal = payload.get("minimal")
    if not isinstance(minimal, dict):
        raise ValueError("baseline missing minimal object")
    expected = minimal.get("artifact")
    if not isinstance(expected, str) or not expected:
        raise ValueError("baseline missing minimal.artifact string")
    if _logical_artifact_key(expected) != _logical_artifact_key(artifact):
        raise ValueError(
            f"artifact mismatch: baseline={_logical_artifact_key(expected)} "
            f"actual={_logical_artifact_key(artifact)}"
        )
    enforced_policy(minimal)
    return minimal


def check_map_artifact(
    profile: dict[str, object],
    binary: Path,
    build_dir: Path | None = None,
) -> Path:
    map_artifact = profile.get("map_artifact")
    if not isinstance(map_artifact, str) or not map_artifact:
        raise ValueError("baseline minimal.map_artifact string is required")
    map_path = Path(map_artifact)
    logical_map = _logical_artifact_key(map_artifact)
    resolved_build_dir = build_dir.resolve() if build_dir is not None else _infer_build_dir(binary)
    if resolved_build_dir is not None and logical_map != _artifact_key(map_artifact):
        map_path = resolved_build_dir / logical_map
    elif not map_path.is_absolute():
        map_path = ROOT / map_path
    map_path = map_path.resolve()
    if not map_path.is_file():
        raise ValueError(f"missing map artifact: {map_path}")
    return map_path


def write_baseline(path: Path, binary: Path, symbols: set[str], map_path: Path) -> None:
    payload = json.loads(path.read_text(encoding="utf-8"))
    minimal = payload.setdefault("minimal", {})
    if not isinstance(minimal, dict):
        raise ValueError("baseline minimal must be an object")
    minimal["artifact"] = _artifact_key(binary)
    minimal["map_artifact"] = _artifact_key(map_path)
    minimal["symbols"] = sorted(symbols)
    minimal["map_summary"] = {"exists": True, "size_bytes": map_path.stat().st_size}
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def run_nm(binary: Path) -> tuple[str, str]:
    tools = []
    if shutil.which("llvm-nm"):
        tools.append(("llvm-nm", ["llvm-nm", "--defined-only", "--extern-only", str(binary)]))
    if shutil.which("nm"):
        tools.append(("nm", ["nm", "-g", str(binary)]))
    if not tools:
        raise RuntimeError("no usable nm tool found in PATH")
    errors = []
    for label, command in tools:
        result = subprocess.run(command, check=False, capture_output=True, text=True)
        if result.returncode == 0:
            return label, result.stdout
        errors.append(f"{label} failed with exit code {result.returncode}: {result.stderr.strip()}")
    raise RuntimeError("all nm commands failed:\n" + "\n".join(errors))


def main() -> int:
    args = parse_args()
    mode = "enforce" if args.enforce else args.mode
    binary = _resolve_binary_argument(args.binary, args.build_dir)
    if not binary.is_file():
        print(f"missing binary: {binary}", file=sys.stderr)
        return 1
    try:
        tool, output = run_nm(binary)
        symbols = parse_nm_output(output)
        map_path = args.map_path or Path(str(binary) + ".map")
        if mode == "baseline":
            if not map_path.is_file():
                raise ValueError(f"missing map artifact: {map_path}")
            write_baseline(args.baseline, binary, symbols, map_path)
        else:
            profile = load_baseline(args.baseline, binary)
            required, forbidden = enforced_policy(profile)
            check_symbols(symbols, required, forbidden)
            check_map_artifact(profile, binary, args.build_dir)
    except (OSError, ValueError, AssertionError, RuntimeError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    print(f"minimal symbols {mode} OK via {tool} artifact={_artifact_key(binary)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
