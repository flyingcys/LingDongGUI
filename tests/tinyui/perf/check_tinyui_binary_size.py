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
DEFAULT_BINARY = ROOT / "build" / "v2.3-m0-full" / "examples" / "sdl" / "tinyui_demo"
SUPPORTED_METRICS = ("__text", "__data", "__bss", "total")
LOGICAL_ARTIFACT_MARKERS = (("examples", "sdl"), ("tests", "tinyui"))


def _resolve_default_binary() -> Path:
    candidates = [
        ROOT / "build" / "v2.3-m3" / "examples" / "sdl" / "tinyui_demo",
        ROOT / "build" / "v2.3-m2" / "examples" / "sdl" / "tinyui_demo",
        ROOT / "build" / "v2.3-m0-full" / "examples" / "sdl" / "tinyui_demo",
        ROOT / "build" / "examples" / "sdl" / "tinyui_demo",
        ROOT / "build" / "tinyui-runtime" / "examples" / "sdl" / "tinyui_demo",
        DEFAULT_BINARY,
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return candidates[0]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check TinyUI binary size against the recorded baseline."
    )
    parser.add_argument("--baseline", type=Path, default=DEFAULT_BASELINE)
    parser.add_argument("--binary", type=Path, default=_resolve_default_binary())
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--enforce", action="store_true")
    return parser.parse_args()


def _validate_profile(profile: object, name: str) -> dict[str, object]:
    if not isinstance(profile, dict):
        raise ValueError(f"baseline binary missing {name} profile")
    artifact = profile.get("artifact")
    if not isinstance(artifact, str) or not artifact:
        raise ValueError(f"baseline binary.{name} missing artifact string")
    metrics = profile.get("metrics")
    if not isinstance(metrics, dict):
        raise ValueError(f"baseline binary.{name} missing metrics object")
    unexpected_metrics = sorted(metric for metric in metrics if metric not in SUPPORTED_METRICS)
    if unexpected_metrics:
        raise ValueError(
            f"baseline binary.{name} contains unsupported metrics: "
            + ", ".join(unexpected_metrics)
        )
    missing_metrics = [metric for metric in SUPPORTED_METRICS if metric not in metrics]
    if missing_metrics:
        raise ValueError(
            f"baseline binary.{name} missing metrics: " + ", ".join(missing_metrics)
        )
    for metric_name in SUPPORTED_METRICS:
        rule = metrics.get(metric_name)
        if not isinstance(rule, dict):
            raise ValueError(f"baseline binary.{name} metric '{metric_name}' must be an object")
        for key in ("baseline_bytes", "max_increase_percent", "max_increase_bytes"):
            if key not in rule:
                raise ValueError(
                    f"baseline binary.{name} metric '{metric_name}' missing field '{key}'"
                )
        try:
            int(rule["baseline_bytes"])
            float(rule["max_increase_percent"])
            int(rule["max_increase_bytes"])
        except (TypeError, ValueError) as exc:
            raise ValueError(
                f"baseline binary.{name} metric '{metric_name}' has non-numeric threshold fields"
            ) from exc
    return profile


def load_baseline(path: Path) -> dict[str, object]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ValueError(f"missing baseline file: {path}") from exc
    binary = payload.get("binary")
    if not isinstance(binary, dict):
        raise ValueError("baseline missing binary object")
    _validate_profile(binary.get("full"), "full")
    _validate_profile(binary.get("minimal"), "minimal")
    return payload


def _artifact_key(path: str | Path) -> str:
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


def _resolve_binary_argument(binary: Path, build_dir: Path | None) -> Path:
    if binary.is_absolute() or build_dir is None or binary.is_file():
        return binary
    relocated = build_dir / binary
    return relocated if relocated.is_file() else binary


def resolve_baseline_for_binary(payload: dict[str, object], binary: Path) -> dict[str, object]:
    profiles = payload.get("binary")
    if not isinstance(profiles, dict):
        raise ValueError("baseline missing binary object")
    actual = _logical_artifact_key(binary)
    for profile_name in ("full", "minimal"):
        profile = profiles.get(profile_name)
        if not isinstance(profile, dict):
            continue
        artifact = profile.get("artifact")
        if not isinstance(artifact, str):
            continue
        expected = _logical_artifact_key(artifact)
        if expected == actual:
            return profile
    expected_artifacts = ", ".join(
        _logical_artifact_key(profiles[name].get("artifact"))
        for name in ("full", "minimal")
        if isinstance(profiles.get(name), dict)
    )
    raise ValueError(f"artifact mismatch: baseline={expected_artifacts} actual={actual}")


def run_size(binary: Path) -> tuple[str, str]:
    candidates: list[tuple[str, list[str]]] = []
    if shutil.which("xcrun") is not None:
        candidates.append(("xcrun llvm-size --format=sysv", ["xcrun", "llvm-size", "--format=sysv", str(binary)]))
    if shutil.which("llvm-size") is not None:
        candidates.append(("llvm-size --format=sysv", ["llvm-size", "--format=sysv", str(binary)]))
    if shutil.which("size") is not None:
        candidates.append(("size", ["size", str(binary)]))
    if not candidates:
        raise RuntimeError("no usable size tools found in PATH: xcrun, llvm-size, size")

    errors: list[str] = []
    for label, command in candidates:
        result = subprocess.run(
            command,
            check=False,
            capture_output=True,
            text=True,
        )
        if result.returncode == 0:
            return label, result.stdout
        stderr = result.stderr.strip() or "<empty stderr>"
        stdout = result.stdout.strip() or "<empty stdout>"
        errors.append(
            f"{label} failed with exit code {result.returncode}; stderr: {stderr}; stdout: {stdout}"
        )
    raise RuntimeError("all size commands failed:\n" + "\n".join(errors))


def parse_gnu_size(stdout: str) -> dict[str, int]:
    lines = [line.strip() for line in stdout.splitlines() if line.strip()]
    if len(lines) < 2:
        raise ValueError(f"unsupported size output: expected at least 2 lines\n{stdout}")
    header = lines[0].split()
    if header[:5] != ["text", "data", "bss", "dec", "hex"]:
        raise ValueError(
            "unsupported size output format: expected GNU columns "
            "'text data bss dec hex', got "
            f"'{lines[0]}'"
        )
    values = lines[1].split()
    if len(values) < 5:
        raise ValueError(f"unsupported size output row: '{lines[1]}'")
    return {
        "__text": int(values[0]),
        "__data": int(values[1]),
        "__bss": int(values[2]),
        "total": int(values[3]),
    }


def parse_llvm_size_sysv(stdout: str) -> dict[str, int]:
    aliases = {
        "__text": "__text",
        ".text": "__text",
        "__data": "__data",
        ".data": "__data",
        "__bss": "__bss",
        ".bss": "__bss",
        "Total": "total",
    }
    metrics: dict[str, int] = {}
    for line in stdout.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        name = parts[0]
        normalized = aliases.get(name)
        if normalized is None:
            continue
        try:
            metrics[normalized] = int(parts[1])
        except ValueError as exc:
            raise ValueError(
                f"unsupported llvm-size --format=sysv row for '{name}': '{line}'"
            ) from exc
    missing = [name for name in SUPPORTED_METRICS if name not in metrics]
    if missing:
        raise ValueError(
            "unsupported llvm-size --format=sysv output: missing metrics "
            + ", ".join(missing)
        )
    return metrics


def parse_size_output(stdout: str) -> dict[str, int]:
    lines = [line.strip() for line in stdout.splitlines() if line.strip()]
    if not lines:
        raise ValueError("empty size output")
    for line in lines:
        header = line.split()
        if header[:5] == ["text", "data", "bss", "dec", "hex"]:
            return parse_gnu_size("\n".join(lines[lines.index(line) :]))
        if "section" in header and "size" in header:
            return parse_llvm_size_sysv(stdout)
    raise ValueError(
        "unsupported size output format: expected GNU 'text data bss dec hex' "
        "or llvm-size sysv section table"
    )


def compare_metrics(actual: dict[str, int], baseline: dict[str, object]) -> list[str]:
    failures: list[str] = []
    metrics = baseline["metrics"]
    if not isinstance(metrics, dict):
        raise ValueError("baseline binary_size.metrics must be an object")
    unexpected_actual = sorted(name for name in actual if name not in SUPPORTED_METRICS)
    if unexpected_actual:
        raise ValueError("parser produced unsupported metrics: " + ", ".join(unexpected_actual))
    missing_actual = [name for name in SUPPORTED_METRICS if name not in actual]
    if missing_actual:
        raise ValueError("parser missing required metrics: " + ", ".join(missing_actual))
    for metric_name in SUPPORTED_METRICS:
        rule = metrics.get(metric_name)
        if not isinstance(rule, dict):
            raise ValueError(f"baseline metric '{metric_name}' must be an object")
        baseline_bytes = int(rule["baseline_bytes"])
        actual_bytes = actual[metric_name]
        if actual_bytes <= baseline_bytes:
            continue
        allowed_percent = float(rule["max_increase_percent"])
        allowed_bytes = int(rule["max_increase_bytes"])
        growth_bytes = actual_bytes - baseline_bytes
        growth_percent = (growth_bytes / baseline_bytes) * 100.0 if baseline_bytes else 0.0
        percent_limit_bytes = int(baseline_bytes * allowed_percent / 100.0)
        if growth_bytes > allowed_bytes and growth_bytes > percent_limit_bytes:
            failures.append(
                f"{metric_name}: baseline={baseline_bytes} actual={actual_bytes} "
                f"growth=+{growth_bytes}B ({growth_percent:.2f}%), "
                f"threshold=+{allowed_bytes}B and +{allowed_percent:.2f}%"
            )
    return failures


def main() -> int:
    args = parse_args()
    binary = _resolve_binary_argument(args.binary, args.build_dir)
    if not args.baseline.is_file():
        print(f"missing baseline file: {args.baseline}", file=sys.stderr)
        return 1
    if not binary.is_file():
        print(f"missing binary: {binary}", file=sys.stderr)
        return 1

    try:
        baseline_payload = load_baseline(args.baseline)
        baseline = resolve_baseline_for_binary(baseline_payload, binary) if args.enforce else None
        tool, stdout = run_size(binary)
        actual = parse_size_output(stdout)
    except (OSError, ValueError, RuntimeError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1

    if not args.enforce:
        print(f"binary size measured via {tool} artifact={binary}")
        print(json.dumps(actual, indent=2, sort_keys=True))
        return 0

    failures = compare_metrics(actual, baseline)
    if failures:
        print("binary size regression detected:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print(f"binary size baseline OK via {tool} artifact={baseline['artifact']}")
    print(json.dumps(actual, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
