#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path

import collect_tinyui_v23_baseline as collector


THRESHOLDS = {"screen_object_create_ms": 5.0, "capture_ready_ms": 40.0}
SCHEMA_PATH = Path(__file__).with_name("v2.3-performance-baseline.schema.json")


def _resolve_schema_ref(schema: dict[str, object], root: dict[str, object]) -> dict[str, object]:
    reference = schema.get("$ref")
    if reference is None:
        return schema
    if not isinstance(reference, str) or not reference.startswith("#/"):
        raise ValueError(f"unsupported schema reference: {reference}")
    value: object = root
    for part in reference[2:].split("/"):
        if not isinstance(value, dict) or part not in value:
            raise ValueError(f"unresolved schema reference: {reference}")
        value = value[part]
    if not isinstance(value, dict):
        raise ValueError(f"schema reference is not an object: {reference}")
    return value


def _validate_schema_value(value: object, schema: dict[str, object], root: dict[str, object], path: str) -> None:
    schema = _resolve_schema_ref(schema, root)
    if "const" in schema and value != schema["const"]:
        raise ValueError(f"schema validation at {path}: expected {schema['const']!r}")
    expected_type = schema.get("type")
    if expected_type == "object":
        if not isinstance(value, dict):
            raise ValueError(f"schema validation at {path}: expected object")
        required = schema.get("required", [])
        for name in required:
            if name not in value:
                raise ValueError(f"schema validation at {path}: missing required property {name}")
        properties = schema.get("properties", {})
        pattern_properties = schema.get("patternProperties", {})
        if schema.get("additionalProperties") is False:
            for name in value:
                if name in properties:
                    continue
                if any(re.search(pattern, name) for pattern in pattern_properties):
                    continue
                raise ValueError(f"schema validation at {path}: unexpected property {name}")
        for name, child in properties.items():
            if name in value:
                _validate_schema_value(value[name], child, root, f"{path}.{name}")
        for pattern, child in pattern_properties.items():
            for name, child_value in value.items():
                if re.search(pattern, name):
                    _validate_schema_value(child_value, child, root, f"{path}.{name}")
        return
    if expected_type == "array":
        if not isinstance(value, list):
            raise ValueError(f"schema validation at {path}: expected array")
        if "minItems" in schema and len(value) < schema["minItems"]:
            raise ValueError(f"schema validation at {path}: expected at least {schema['minItems']} items")
        if "maxItems" in schema and len(value) > schema["maxItems"]:
            raise ValueError(f"schema validation at {path}: expected at most {schema['maxItems']} items")
        for index, item in enumerate(value):
            _validate_schema_value(item, schema["items"], root, f"{path}[{index}]")
        return
    if expected_type == "string":
        if not isinstance(value, str):
            raise ValueError(f"schema validation at {path}: expected string")
        if len(value) < schema.get("minLength", 0):
            raise ValueError(f"schema validation at {path}: expected non-empty string")
        if "pattern" in schema and re.fullmatch(schema["pattern"], value) is None:
            raise ValueError(f"schema validation at {path}: value does not match pattern")
        return
    if expected_type == "integer":
        if isinstance(value, bool) or not isinstance(value, int):
            raise ValueError(f"schema validation at {path}: expected integer")
    elif expected_type == "number":
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"schema validation at {path}: expected number")
    if "minimum" in schema and value < schema["minimum"]:
        raise ValueError(f"schema validation at {path}: value is below minimum")


def validate_schema(payload: object, schema_path: Path = SCHEMA_PATH) -> None:
    try:
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"invalid task8 schema: {schema_path}") from exc
    if not isinstance(schema, dict) or schema.get("$id") != "tinyui-v2.3-performance-baseline-schema-v1":
        raise ValueError("invalid task8 schema identity")
    _validate_schema_value(payload, schema, schema, "baseline")


def validate_metric_samples(samples: object, warmup_count: object) -> None:
    if not isinstance(samples, list) or len(samples) != 30:
        raise ValueError("samples must contain exactly 30 values")
    if warmup_count != 5:
        raise ValueError("warmup_count must be exactly 5")
    if any(isinstance(value, bool) or not isinstance(value, (int, float)) for value in samples):
        raise ValueError("samples must be numeric")


def _load(path: Path) -> dict[str, object]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    validate_schema(payload)
    if not isinstance(payload, dict) or payload.get("schema_version") != 1:
        raise ValueError("invalid TinyUI v2.3 baseline")
    required = {"fingerprint", "fingerprint_sha256", "scenarios", "probes", "artifacts"}
    if set(payload) - (required | {"schema_version", "measured_at", "$schema"}):
        raise ValueError("baseline has missing or unexpected fields")
    fingerprint = payload["fingerprint"]
    if not isinstance(fingerprint, dict):
        raise ValueError("baseline fingerprint must be an object")
    if collector.fingerprint_sha256(fingerprint) != payload["fingerprint_sha256"]:
        raise ValueError("baseline fingerprint digest mismatch")
    scenarios = payload["scenarios"]
    if not isinstance(scenarios, dict) or set(scenarios) != set(THRESHOLDS):
        raise ValueError("baseline scenarios are incomplete")
    for name, metric in scenarios.items():
        if not isinstance(metric, dict):
            raise ValueError(f"baseline scenario {name} must be an object")
        validate_metric_samples(metric.get("samples"), metric.get("warmup_count"))
        expected_median = __import__("statistics").median(metric["samples"])
        if metric.get("median") != expected_median:
            raise ValueError(f"baseline scenario {name} median mismatch")
        if metric.get("p95") != collector.percentile_nearest_rank(metric["samples"], 0.95):
            raise ValueError(f"baseline scenario {name} p95 mismatch")
        if metric["p95"] > THRESHOLDS[name]:
            raise ValueError(f"{name} p95 {metric['p95']} exceeds {THRESHOLDS[name]}")
    probes = payload["probes"]
    if not isinstance(probes, dict) or set(probes) != {
        "task5_wrapper_overhead", "task6_allocation", "task7_binary_size", "task7_minimal_symbols"
    } or any(not isinstance(value, str) or not value for value in probes.values()):
        raise ValueError("baseline task 5/6/7 probe output is incomplete")
    artifacts = payload["artifacts"]
    if not isinstance(artifacts, dict) or set(artifacts) != {
        "full_binary", "minimal_binary", "minimal_map", "wrapper_probe", "allocation_probe"
    }:
        raise ValueError("baseline artifact manifest is incomplete")
    for name, artifact in artifacts.items():
        if not isinstance(artifact, dict) or set(artifact) != {"path", "sha256"}:
            raise ValueError(f"baseline artifact {name} is malformed")
        if not isinstance(artifact["path"], str) or not isinstance(artifact["sha256"], str) or len(artifact["sha256"]) != 64:
            raise ValueError(f"baseline artifact {name} has invalid SHA-256")
    return payload


def _check_fingerprint(payload: dict[str, object], build_dir: Path) -> None:
    current = collector.collect_environment(build_dir)
    if current != payload["fingerprint"]:
        print("BASELINE_FINGERPRINT_MISMATCH", file=sys.stderr)
        raise ValueError("environment fingerprint differs from baseline")


def _artifact_path(recorded_path: str, build_dir: Path) -> Path:
    recorded = Path(recorded_path)
    relative = recorded.relative_to(collector.ROOT) if recorded.is_absolute() else recorded
    if len(relative.parts) >= 3 and relative.parts[0] == "build":
        return build_dir.resolve().joinpath(*relative.parts[2:])
    return (collector.ROOT / relative).resolve()


def _check_artifacts(payload: dict[str, object], build_dir: Path) -> None:
    artifacts = payload["artifacts"]
    assert isinstance(artifacts, dict)
    for name, entry in artifacts.items():
        assert isinstance(entry, dict)
        path = _artifact_path(str(entry["path"]), build_dir)
        if not path.is_file():
            raise ValueError(f"missing baseline artifact: {name}: {path}")
        digest = collector.artifact_sha256(path)
        if digest != entry["sha256"]:
            raise ValueError(f"artifact SHA-256 mismatch: {name}")


def resolve_baseline_path(baseline: Path | None, baseline_dir: Path | None) -> Path:
    if (baseline is None) == (baseline_dir is None):
        raise ValueError("exactly one of --baseline or --baseline-dir is required")
    if baseline is not None:
        return baseline
    assert baseline_dir is not None
    candidates = sorted(baseline_dir.glob("*.json"))
    if len(candidates) != 1:
        raise ValueError(f"--baseline-dir must contain exactly one JSON file, found {len(candidates)}")
    return candidates[0]


def check(baseline: Path, build_dir: Path) -> int:
    try:
        payload = _load(baseline)
        _check_fingerprint(payload, build_dir)
        _check_artifacts(payload, build_dir)
        print("TINYUI_V23_BASELINE_OK")
        for name, metric in payload["scenarios"].items():
            print(f"{name} median={metric['median']:.3f} p95={metric['p95']:.3f}")
    except (OSError, ValueError, json.JSONDecodeError, KeyError, TypeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    check_parser = subparsers.add_parser("check")
    baseline_group = check_parser.add_mutually_exclusive_group(required=True)
    baseline_group.add_argument("--baseline", type=Path)
    baseline_group.add_argument("--baseline-dir", type=Path)
    check_parser.add_argument("--build-dir", type=Path, required=True)
    args = parser.parse_args()
    return check(resolve_baseline_path(args.baseline, args.baseline_dir), args.build_dir)


if __name__ == "__main__":
    raise SystemExit(main())
