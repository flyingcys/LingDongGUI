import os
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "tinyui-runtime"
DEFAULT_BASELINE = ROOT / "tests" / "tinyui" / "perf" / "tinyui_perf_baseline.json"
RTK = shutil.which("rtk") or "rtk"
TARGET = "tinyui_demo"
DEMO = "basic_widgets"
DEMO_TIMEOUT_SECONDS = 8
CMAKE_BENCHMARK_OPTIONS = ("-DUSE_DEMO=0", "-DENABLE_TEST=ON")


def _find_executable(build_dir: Path) -> Path:
    candidates = [
        build_dir / "examples" / "sdl" / TARGET,
        build_dir / TARGET,
        build_dir / "examples" / TARGET,
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        raise FileNotFoundError(
            f"Could not find executable for target '{TARGET}'. Checked: {', '.join(str(path) for path in candidates)}"
        )
    return executable


def _parse_marker(stdout: str, marker: str) -> float:
    prefix = f"{marker}="
    for line in stdout.splitlines():
        if line.startswith(prefix):
            return float(line[len(prefix) :].strip())
    raise AssertionError(f"Missing benchmark marker '{marker}'.\nstdout:\n{stdout}")


def _assert_runtime_metric_within_gate(metric_name: str,
                                       actual_ms: float,
                                       baseline_ms: float,
                                       max_allowed_ms: float) -> None:
    if actual_ms > max_allowed_ms:
        raise AssertionError(
            f"{metric_name} exceeded regression gate: "
            f"{actual_ms:.3f} > {max_allowed_ms:.3f} "
            f"(baseline {baseline_ms:.3f})"
        )


def _load_baseline(path: Path) -> dict[str, object]:
    if not path.is_file():
        raise FileNotFoundError(f"Missing baseline file: {path}")
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError("baseline payload must be a JSON object")
    return payload


def _resolve_limits(payload: dict[str, object]) -> tuple[dict[str, dict[str, float]], str]:
    benchmark = payload.get("benchmark")
    if not isinstance(benchmark, dict):
        raise ValueError("baseline missing benchmark object")
    runtime_perf = benchmark.get("runtime_perf")
    if not isinstance(runtime_perf, dict):
        raise ValueError("baseline missing benchmark.runtime_perf object")
    metrics = runtime_perf.get("metrics")
    if not isinstance(metrics, dict):
        raise ValueError("baseline missing benchmark.runtime_perf.metrics object")

    limits: dict[str, dict[str, float]] = {}
    for key in ("screen_object_create_ms", "capture_ready_ms"):
        rule = metrics.get(key)
        if not isinstance(rule, dict):
            raise ValueError(f"baseline missing benchmark.runtime_perf.metrics.{key}")
        if "baseline_ms" not in rule:
            raise ValueError(f"baseline missing {key}.baseline_ms")
        if "max_allowed_ms" not in rule:
            raise ValueError(f"baseline missing {key}.max_allowed_ms")
        limits[key] = {
            "baseline_ms": float(rule["baseline_ms"]),
            "max_allowed_ms": float(rule["max_allowed_ms"]),
        }
    return limits, "baseline"


def prepare_build(build_dir: Path, targets: tuple[str, ...] = (TARGET,)) -> Path:
    subprocess.run(
        [RTK, "cmake", "-S", str(ROOT), "-B", str(build_dir), *CMAKE_BENCHMARK_OPTIONS],
        check=True,
    )
    subprocess.run(
        [RTK, "cmake", "--build", str(build_dir), "--target", *targets],
        check=True,
    )
    return _find_executable(build_dir)


def run_demo_once(executable: Path, demo: str = DEMO) -> subprocess.CompletedProcess[str]:

    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")
    env["TINYUI_DEMO_AUTO_QUIT_MS"] = "1200"
    env["TINYUI_BENCHMARK_LOG"] = "1"
    with tempfile.TemporaryDirectory(prefix="tinyui-benchmark-") as tmpdir:
        capture_path = Path(tmpdir) / "frame.ppm"
        env["TINYUI_CAPTURE_FILE"] = str(capture_path)
        completed = subprocess.run(
            [str(executable), demo],
            check=False,
            timeout=DEMO_TIMEOUT_SECONDS,
            capture_output=True,
            text=True,
            env=env,
        )
    if completed.returncode != 0:
        raise RuntimeError(
            f"Demo '{DEMO}' exited with {completed.returncode}.\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )
    return completed


def _run_benchmark(build_dir: Path) -> subprocess.CompletedProcess[str]:
    return run_demo_once(prepare_build(build_dir))


def _run_self_test() -> None:
    _assert_runtime_metric_within_gate(
        "screen_object_create_ms",
        actual_ms=1.0,
        baseline_ms=2.0,
        max_allowed_ms=5.0,
    )
    _assert_runtime_metric_within_gate(
        "capture_ready_ms",
        actual_ms=8.0,
        baseline_ms=12.0,
        max_allowed_ms=16.0,
    )
    print("SELFTEST_GREEN runtime-regression-gate")


def main() -> None:
    if len(sys.argv) == 2 and sys.argv[1] == "--self-test":
        _run_self_test()
        return

    try:
        payload = _load_baseline(DEFAULT_BASELINE)
        limits, limit_source = _resolve_limits(payload)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)

    screen_object_create_baseline = float(limits["screen_object_create_ms"]["baseline_ms"])
    screen_object_create_limit = float(limits["screen_object_create_ms"]["max_allowed_ms"])
    capture_ready_baseline = float(limits["capture_ready_ms"]["baseline_ms"])
    capture_ready_limit = float(limits["capture_ready_ms"]["max_allowed_ms"])
    completed = _run_benchmark(DEFAULT_BUILD)
    screen_object_create_ms = _parse_marker(completed.stdout, "TINYUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS")
    capture_ready_ms = _parse_marker(completed.stdout, "TINYUI_BENCHMARK_CAPTURE_READY_MS")

    try:
        _assert_runtime_metric_within_gate(
            "screen_object_create_ms",
            actual_ms=screen_object_create_ms,
            baseline_ms=screen_object_create_baseline,
            max_allowed_ms=screen_object_create_limit,
        )
        _assert_runtime_metric_within_gate(
            "capture_ready_ms",
            actual_ms=capture_ready_ms,
            baseline_ms=capture_ready_baseline,
            max_allowed_ms=capture_ready_limit,
        )
    except AssertionError as exc:
        raise AssertionError(f"{exc}\nstdout:\n{completed.stdout}") from exc

    print(
        "TINYUI_PERF_OK "
        f"baseline={DEFAULT_BASELINE} "
        f"limit_source={limit_source} "
        f"screen_object_create_ms={screen_object_create_ms:.3f}/"
        f"{screen_object_create_baseline:.3f}-{screen_object_create_limit:.3f} "
        f"capture_ready_ms={capture_ready_ms:.3f}/"
        f"{capture_ready_baseline:.3f}-{capture_ready_limit:.3f}"
    )


if __name__ == "__main__":
    main()
