import os
import json
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BUILD = ROOT / "build" / "tinyui-runtime"
DEFAULT_BASELINE = ROOT / "tests" / "tinyui" / "perf" / "tinyui_perf_baseline.json"
RTK = shutil.which("rtk") or "rtk"
PROBE_TARGET = "test_tinyui_wrapper_struct_overhead"


def _load_baseline(path: Path) -> dict[str, object]:
    if not path.is_file():
        raise FileNotFoundError(f"Missing baseline file: {path}")
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError("baseline payload must be a JSON object")
    return payload


def _assert_wrapper_metric_within_gate(metric_name: str,
                                       actual_bytes: int,
                                       baseline_bytes: int,
                                       max_allowed_bytes: int) -> None:
    if actual_bytes > max_allowed_bytes:
        raise AssertionError(
            f"{metric_name} exceeded regression gate: "
            f"{actual_bytes} > {max_allowed_bytes} "
            f"(baseline {baseline_bytes})"
        )


def _resolve_limits(payload: dict[str, object]) -> tuple[dict[str, dict[str, int]], str]:
    benchmark = payload.get("benchmark")
    if not isinstance(benchmark, dict):
        raise ValueError("baseline missing benchmark object")
    wrapper_struct_overhead = benchmark.get("wrapper_struct_overhead")
    if not isinstance(wrapper_struct_overhead, dict):
        raise ValueError("baseline missing benchmark.wrapper_struct_overhead object")
    metrics = wrapper_struct_overhead.get("metrics")
    if not isinstance(metrics, dict):
        raise ValueError("baseline missing benchmark.wrapper_struct_overhead.metrics object")

    limits: dict[str, dict[str, int]] = {}
    for key in ("widget_wrapper_struct_bytes", "switch_wrapper_struct_delta_bytes"):
        rule = metrics.get(key)
        if not isinstance(rule, dict):
            raise ValueError(f"baseline missing benchmark.wrapper_struct_overhead.metrics.{key}")
        if "baseline_bytes" not in rule:
            raise ValueError(f"baseline missing {key}.baseline_bytes")
        if "max_allowed_bytes" not in rule:
            raise ValueError(f"baseline missing {key}.max_allowed_bytes")
        limits[key] = {
            "baseline_bytes": int(rule["baseline_bytes"]),
            "max_allowed_bytes": int(rule["max_allowed_bytes"]),
        }
    return limits, "baseline"


def _configure_build(build_dir: Path) -> None:
    subprocess.run(
        [RTK, "cmake", "-S", str(ROOT), "-B", str(build_dir), "-DUSE_DEMO=0"],
        check=True,
    )


def _find_probe_executable(build_dir: Path) -> Path:
    candidates = [
        build_dir / "tests" / "tinyui" / PROBE_TARGET,
        build_dir / PROBE_TARGET,
    ]
    executable = next((path for path in candidates if path.is_file()), None)
    if executable is None:
        raise FileNotFoundError(
            f"Could not find executable for target '{PROBE_TARGET}'. "
            f"Checked: {', '.join(str(path) for path in candidates)}"
        )
    return executable


def _run_probe(build_dir: Path) -> tuple[int, int]:
    subprocess.run(
        [RTK, "cmake", "--build", str(build_dir), "--target", PROBE_TARGET],
        check=True,
    )
    completed = subprocess.run(
        [str(_find_probe_executable(build_dir))],
        check=True,
        capture_output=True,
        text=True,
    )

    values: dict[str, int] = {}
    for line in completed.stdout.splitlines():
        if line.startswith("TINYUI_BENCHMARK_WIDGET_WRAPPER_STRUCT_BYTES="):
            values["widget_wrapper_struct_bytes"] = int(line.split("=", 1)[1].strip())
        if line.startswith("TINYUI_BENCHMARK_SWITCH_WRAPPER_STRUCT_DELTA_BYTES="):
            values["switch_wrapper_struct_delta_bytes"] = int(line.split("=", 1)[1].strip())

    required = (
        "widget_wrapper_struct_bytes",
        "switch_wrapper_struct_delta_bytes",
    )
    if any(key not in values for key in required):
        raise ValueError(f"probe did not emit expected markers.\nstdout:\n{completed.stdout}")
    return (
        values["widget_wrapper_struct_bytes"],
        values["switch_wrapper_struct_delta_bytes"],
    )


def _run_self_test() -> None:
    _assert_wrapper_metric_within_gate(
        "widget_wrapper_struct_bytes",
        actual_bytes=184,
        baseline_bytes=192,
        max_allowed_bytes=256,
    )
    print("SELFTEST_GREEN wrapper-regression-gate")


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

    widget_wrapper_struct_baseline = int(limits["widget_wrapper_struct_bytes"]["baseline_bytes"])
    widget_wrapper_struct_limit = int(limits["widget_wrapper_struct_bytes"]["max_allowed_bytes"])
    switch_wrapper_struct_delta_baseline = int(limits["switch_wrapper_struct_delta_bytes"]["baseline_bytes"])
    switch_wrapper_struct_delta_limit = int(limits["switch_wrapper_struct_delta_bytes"]["max_allowed_bytes"])
    try:
        _configure_build(DEFAULT_BUILD)
        (widget_wrapper_struct_bytes,
         switch_wrapper_struct_delta_bytes) = _run_probe(DEFAULT_BUILD)
    except (OSError, ValueError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)

    _assert_wrapper_metric_within_gate(
        "widget_wrapper_struct_bytes",
        actual_bytes=widget_wrapper_struct_bytes,
        baseline_bytes=widget_wrapper_struct_baseline,
        max_allowed_bytes=widget_wrapper_struct_limit,
    )
    _assert_wrapper_metric_within_gate(
        "switch_wrapper_struct_delta_bytes",
        actual_bytes=switch_wrapper_struct_delta_bytes,
        baseline_bytes=switch_wrapper_struct_delta_baseline,
        max_allowed_bytes=switch_wrapper_struct_delta_limit,
    )

    print(
        "TINYUI_WRAPPER_STRUCT_OVERHEAD_OK "
        f"baseline={DEFAULT_BASELINE} "
        f"limit_source={limit_source} "
        f"widget_wrapper_struct_bytes={widget_wrapper_struct_bytes}/"
        f"{widget_wrapper_struct_baseline}-{widget_wrapper_struct_limit} "
        f"switch_wrapper_struct_delta_bytes={switch_wrapper_struct_delta_bytes}/"
        f"{switch_wrapper_struct_delta_baseline}-{switch_wrapper_struct_delta_limit}"
    )


if __name__ == "__main__":
    main()
