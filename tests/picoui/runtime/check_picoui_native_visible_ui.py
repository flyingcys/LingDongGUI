import argparse
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "picoui-native-artifact"
MANIFEST_PATH = Path(__file__).with_name("picoui_native_artifact_manifest.json")
RTK = shutil.which("rtk") or "rtk"
DEMO_TIMEOUT_SECONDS = 8
AUTO_QUIT_MS = "1200"


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run PicoUI native demo artifact gate from manifest."
    )
    parser.add_argument(
        "--demo",
        help="Single demo short name such as 'basic_widgets'. Ignored when --all-demos is set.",
    )
    parser.add_argument(
        "--all-demos",
        action="store_true",
        help="Run all demos listed in picoui_native_artifact_manifest.json.",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        default=DEFAULT_BUILD,
        help="CMake build directory for the PicoUI native demos.",
    )
    args = parser.parse_args()
    if not args.all_demos and not args.demo:
        args.demo = "basic_widgets"
    return args


def _load_manifest() -> list[dict[str, str]]:
    payload = json.loads(MANIFEST_PATH.read_text())
    entries = payload.get("entries")
    if not isinstance(entries, list) or not entries:
        raise AssertionError(f"manifest entries missing or empty: {MANIFEST_PATH}")

    demos: set[str] = set()
    normalized: list[dict[str, str]] = []
    for entry in entries:
        if not isinstance(entry, dict):
            raise AssertionError(f"manifest entry must be object: {entry!r}")
        demo = str(entry.get("demo", "")).strip()
        target = str(entry.get("target", "")).strip()
        policy = str(entry.get("artifact_policy", "")).strip()
        reason = str(entry.get("reason", "")).strip()
        if not demo or not target:
            raise AssertionError(f"manifest entry missing demo/target: {entry!r}")
        if demo in demos:
            raise AssertionError(f"duplicate manifest demo entry: {demo}")
        if policy not in {"visible", "runtime_only"}:
            raise AssertionError(f"unsupported artifact_policy for {demo}: {policy}")
        if policy == "runtime_only" and not reason:
            raise AssertionError(f"runtime_only demo must provide reason: {demo}")
        demos.add(demo)
        normalized.append(
            {
                "demo": demo,
                "target": target,
                "artifact_policy": policy,
                "reason": reason,
            }
        )
    return normalized


def _select_entries(
    entries: list[dict[str, str]],
    *,
    demo: str | None,
    all_demos: bool,
) -> list[dict[str, str]]:
    if all_demos:
        return entries
    assert demo is not None
    for entry in entries:
        if entry["demo"] == demo or entry["target"] == demo:
            return [entry]
        if f"picoui_{demo}_demo" == entry["target"]:
            return [entry]
    known = ", ".join(entry["demo"] for entry in entries)
    raise AssertionError(f"unknown demo '{demo}'. known demos: {known}")


def _find_executable(build_dir: Path, target: str) -> Path:
    suffix = ".exe" if os.name == "nt" else ""
    candidates = [
        build_dir / "examples" / "sdl" / f"{target}{suffix}",
        build_dir / target / f"{target}{suffix}",
        build_dir / "examples" / f"{target}{suffix}",
        build_dir / f"{target}{suffix}",
        ROOT / "build" / "examples" / "sdl" / f"{target}{suffix}",
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        checked = "\n  - ".join(str(path) for path in candidates)
        raise FileNotFoundError(
            f"找不到 demo target '{target}' 的可执行文件。已检查：\n  - {checked}"
        )
    return executable


def _configure_and_build(build_dir: Path, targets: list[str]) -> None:
    subprocess.run(
        [
            RTK,
            "cmake",
            "-S",
            str(ROOT),
            "-B",
            str(build_dir),
            "-DUSE_DEMO=0",
        ],
        check=True,
    )
    subprocess.run(
        [
            RTK,
            "cmake",
            "--build",
            str(build_dir),
            "--target",
            *targets,
        ],
        check=True,
    )


def _run_demo(executable: Path) -> dict[str, object]:
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["PICOUI_DEMO_AUTO_QUIT_MS"] = AUTO_QUIT_MS

    with tempfile.TemporaryDirectory(prefix=f"{executable.stem}-") as tmpdir:
        capture_path = Path(tmpdir) / "frame.ppm"
        env["PICOUI_CAPTURE_FILE"] = str(capture_path)
        completed = subprocess.run(
            [str(executable)],
            check=False,
            timeout=DEMO_TIMEOUT_SECONDS,
            capture_output=True,
            text=True,
            env=env,
        )
        capture_ready = capture_path.is_file() and capture_path.stat().st_size > 32
        capture_size = capture_path.stat().st_size if capture_path.is_file() else 0
    return {
        "returncode": completed.returncode,
        "stdout": completed.stdout,
        "stderr": completed.stderr,
        "runtime_ready": "PICOUI_RUNTIME_READY" in completed.stdout,
        "capture_ready": capture_ready,
        "capture_size": capture_size,
    }


def _format_observation(entry: dict[str, str], result: dict[str, object], status: str) -> str:
    return (
        f"{status} demo={entry['demo']} target={entry['target']} policy={entry['artifact_policy']} "
        f"rc={result['returncode']} ready={1 if result['runtime_ready'] else 0} "
        f"capture={1 if result['capture_ready'] else 0} capture_size={result['capture_size']}"
    )


def _validate_visible(entry: dict[str, str], result: dict[str, object]) -> list[str]:
    failures: list[str] = []
    if result["returncode"] != 0:
        failures.append(f"expected rc=0, actual rc={result['returncode']}")
    if not result["runtime_ready"]:
        failures.append("expected PICOUI_RUNTIME_READY in stdout")
    if not result["capture_ready"]:
        failures.append("expected non-empty capture artifact")
    return failures


def _validate_runtime_only(entry: dict[str, str], result: dict[str, object]) -> list[str]:
    failures: list[str] = []
    if result["returncode"] < 0:
        failures.append(f"process terminated by signal, rc={result['returncode']}")
    if (
        result["returncode"] == 0
        and not result["runtime_ready"]
        and not result["capture_ready"]
    ):
        failures.append(
            "runtime_only demo exited 0 but reported neither runtime-ready nor capture artifact"
        )
    return failures


def main() -> int:
    args = _parse_args()
    entries = _load_manifest()
    selected = _select_entries(entries, demo=args.demo, all_demos=args.all_demos)
    build_dir = args.build_dir.resolve()

    _configure_and_build(build_dir, [entry["target"] for entry in selected])

    failures: list[str] = []
    visible_count = 0
    runtime_only_count = 0

    for entry in selected:
        executable = _find_executable(build_dir, entry["target"])
        try:
            result = _run_demo(executable)
        except subprocess.TimeoutExpired:
            failures.append(f"FAIL demo={entry['demo']} target={entry['target']} timed out")
            continue

        policy = entry["artifact_policy"]
        if policy == "visible":
            visible_count += 1
            validation_failures = _validate_visible(entry, result)
            if validation_failures:
                joined = "; ".join(validation_failures)
                failures.append(
                    _format_observation(entry, result, "FAIL")
                    + f" reason={joined}\nstdout:\n{result['stdout']}\nstderr:\n{result['stderr']}"
                )
            else:
                print(_format_observation(entry, result, "PASS"))
            continue

        runtime_only_count += 1
        validation_failures = _validate_runtime_only(entry, result)
        if validation_failures:
            joined = "; ".join(validation_failures)
            failures.append(
                _format_observation(entry, result, "FAIL")
                + f" manifest_reason={entry['reason']} reason={joined}\nstdout:\n{result['stdout']}\nstderr:\n{result['stderr']}"
            )
            continue

        if (
            result["returncode"] == 0
            and result["runtime_ready"]
            and result["capture_ready"]
        ):
            print(
                _format_observation(entry, result, "PASS-UPGRADE")
                + f" manifest_reason={entry['reason']}"
            )
        else:
            print(
                _format_observation(entry, result, "PASS-RUNTIME-ONLY")
                + f" manifest_reason={entry['reason']}"
            )

    print(
        f"SUMMARY visible={visible_count} runtime_only={runtime_only_count} total={len(selected)} failures={len(failures)}"
    )
    if failures:
        raise AssertionError("\n\n".join(failures))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
