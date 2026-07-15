#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_BASELINE = ROOT / "tests" / "tinyui" / "perf" / "tinyui_perf_baseline.json"
SCHEMA_PATH = Path(__file__).with_name("v2.3-baseline.schema.json")
PROBE_TARGET = "test_tinyui_wrapper_struct_overhead"
REQUIRED_WRAPPERS = (
    "widget", "window", "background", "canvas", "label", "button", "keyboard",
    "checkbox", "switch", "slider", "progress_bar", "animation", "arc", "gauge",
    "icon_slider", "radial_menu", "qrcode", "progress_wheel", "date_time", "calendar",
    "clock", "list", "message_box", "text", "line_edit", "combo_box",
    "scroll_selecter", "image", "graph", "table",
)
REQUIRED_METRICS = tuple(
    f"{wrapper}_wrapper_struct_bytes" for wrapper in REQUIRED_WRAPPERS
) + (
    "switch_wrapper_struct_delta_bytes",
    "backend_widget_struct_bytes",
    "legacy_timer_node_bytes",
    "legacy_event_callback_storage_bytes",
    "runtime_bookkeeping_bytes",
)
EXACT_BASELINE_METRICS = (
    "legacy_timer_node_bytes",
    "legacy_event_callback_storage_bytes",
    "runtime_bookkeeping_bytes",
)
_MARKER_RE = re.compile(r"^TINYUI_SIZEOF_([A-Z0-9_]+)=(-?\d+)$")


def _metric_from_marker(marker: str) -> str:
    if marker.endswith("_WRAPPER"):
        return f"{marker[:-len('_WRAPPER')].lower()}_wrapper_struct_bytes"
    special_metrics = {
        "SWITCH_WRAPPER_DELTA": "switch_wrapper_struct_delta_bytes",
        "BACKEND_WIDGET_LD_WINDOW": "backend_widget_struct_bytes",
        "LEGACY_TIMER_NODE": "legacy_timer_node_bytes",
        "LEGACY_EVENT_CALLBACK_STORAGE": "legacy_event_callback_storage_bytes",
        "RUNTIME_BOOKKEEPING": "runtime_bookkeeping_bytes",
    }
    if marker not in special_metrics:
        raise ValueError(f"unexpected probe metric: {marker.lower()}")
    return special_metrics[marker]


def parse_probe(stdout: str) -> dict[str, int]:
    metrics: dict[str, int] = {}
    for line in stdout.splitlines():
        match = _MARKER_RE.fullmatch(line.strip())
        if match is None:
            continue
        metric_name = _metric_from_marker(match.group(1))
        if metric_name in metrics:
            raise ValueError(f"duplicate probe metric: {metric_name}")
        value = int(match.group(2))
        if value < 0:
            raise ValueError(f"negative probe metric: {metric_name}")
        metrics[metric_name] = value

    for wrapper in REQUIRED_WRAPPERS:
        metric_name = f"{wrapper}_wrapper_struct_bytes"
        if metric_name not in metrics:
            raise ValueError(f"missing wrapper metric: {metric_name}")
    for metric_name in REQUIRED_METRICS:
        if metric_name not in metrics:
            raise ValueError(f"missing probe metric: {metric_name}")
    extra_metrics = sorted(set(metrics) - set(REQUIRED_METRICS))
    if extra_metrics:
        raise ValueError("unexpected probe metrics: " + ", ".join(extra_metrics))
    return metrics


def _require_exact_keys(value: object, expected: set[str], path: str) -> dict[str, object]:
    if not isinstance(value, dict):
        raise ValueError(f"{path} must be an object")
    actual = set(value)
    extra = sorted(actual - expected)
    if extra:
        raise ValueError(f"{path} has unexpected key: {', '.join(extra)}")
    missing = sorted(expected - actual)
    if missing:
        raise ValueError(f"{path} missing key: {', '.join(missing)}")
    return value


def _require_number(value: object, path: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{path} must be numeric")
    if value < 0:
        raise ValueError(f"{path} must not be negative")
    return float(value)


def _validate_metric_rule(name: str, rule: object) -> dict[str, object]:
    payload = _require_exact_keys(
        rule,
        {"baseline_bytes", "max_increase_percent", "max_increase_bytes", "absolute_max_bytes"},
        f"wrapper metric {name}",
    )
    for key in payload:
        _require_number(payload[key], f"wrapper metric {name}.{key}")
    return payload


def _load_schema(schema_path: Path) -> dict[str, object]:
    if not schema_path.is_file():
        raise ValueError(f"missing schema file: {schema_path}")
    try:
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ValueError(f"invalid schema JSON: {schema_path}") from exc
    if not isinstance(schema, dict) or schema.get("$id") != "tinyui-v2.3-baseline-schema-v1":
        raise ValueError("invalid TinyUI v2.3 baseline schema")
    return schema


def _resolve_schema_ref(schema: dict[str, object], root_schema: dict[str, object]) -> dict[str, object]:
    reference = schema.get("$ref")
    if reference is None:
        return schema
    if not isinstance(reference, str) or not reference.startswith("#/"):
        raise ValueError(f"unsupported schema reference: {reference}")

    resolved: object = root_schema
    for part in reference[2:].split("/"):
        if not isinstance(resolved, dict) or part not in resolved:
            raise ValueError(f"unresolved schema reference: {reference}")
        resolved = resolved[part]
    if not isinstance(resolved, dict):
        raise ValueError(f"schema reference is not an object: {reference}")
    return resolved


def _validate_schema_value(value: object,
                           schema: dict[str, object],
                           root_schema: dict[str, object],
                           path: str) -> None:
    schema = _resolve_schema_ref(schema, root_schema)
    if "const" in schema and value != schema["const"]:
        raise ValueError(f"schema validation at {path}: expected constant {schema['const']!r}")
    if "enum" in schema and value not in schema["enum"]:
        raise ValueError(f"schema validation at {path}: value is not permitted")

    expected_type = schema.get("type")
    if expected_type == "object":
        if not isinstance(value, dict):
            raise ValueError(f"schema validation at {path}: expected object")
        required = schema.get("required", [])
        if not isinstance(required, list):
            raise ValueError(f"invalid schema required list at {path}")
        for name in required:
            if not isinstance(name, str):
                raise ValueError(f"invalid schema required name at {path}")
            if name not in value:
                raise ValueError(f"schema validation at {path}: missing required property {name}")
        properties = schema.get("properties", {})
        if not isinstance(properties, dict):
            raise ValueError(f"invalid schema properties at {path}")
        if schema.get("additionalProperties") is False:
            extra = sorted(set(value) - set(properties))
            if extra:
                raise ValueError(f"schema validation at {path}: unexpected property {extra[0]}")
        for name, child_schema in properties.items():
            if name in value:
                if not isinstance(child_schema, dict):
                    raise ValueError(f"invalid schema property at {path}.{name}")
                _validate_schema_value(value[name], child_schema, root_schema, f"{path}.{name}")
        return

    if expected_type == "array":
        if not isinstance(value, list):
            raise ValueError(f"schema validation at {path}: expected array")
        minimum = schema.get("minItems")
        if isinstance(minimum, int) and len(value) < minimum:
            raise ValueError(f"schema validation at {path}: expected at least {minimum} items")
        item_schema = schema.get("items")
        if item_schema is not None:
            if not isinstance(item_schema, dict):
                raise ValueError(f"invalid schema items at {path}")
            for index, item in enumerate(value):
                _validate_schema_value(item, item_schema, root_schema, f"{path}[{index}]")
        return

    if expected_type == "string":
        if not isinstance(value, str):
            raise ValueError(f"schema validation at {path}: expected string")
        minimum = schema.get("minLength")
        if isinstance(minimum, int) and len(value) < minimum:
            raise ValueError(f"schema validation at {path}: expected non-empty string")
        return

    if expected_type == "integer":
        if isinstance(value, bool) or not isinstance(value, int):
            raise ValueError(f"schema validation at {path}: expected integer")
    elif expected_type == "number":
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"schema validation at {path}: expected number")
    elif expected_type is not None:
        raise ValueError(f"unsupported schema type at {path}: {expected_type}")

    minimum = schema.get("minimum")
    if minimum is not None and value < minimum:
        raise ValueError(f"schema validation at {path}: value is below {minimum}")


def validate_baseline(payload: object, schema_path: Path = SCHEMA_PATH) -> dict[str, object]:
    schema = _load_schema(schema_path)
    _validate_schema_value(payload, schema, schema, "baseline")
    baseline = _require_exact_keys(
        payload,
        {"schema_version", "binary_size", "benchmark", "minimal", "binary"}
        & set(payload),
        "baseline",
    )
    if baseline["schema_version"] != 3:
        raise ValueError("baseline.schema_version must equal 3")

    binary_size = _require_exact_keys(
        baseline["binary_size"],
        {"artifact", "measured_at", "command", "format", "metric_mapping", "metrics"},
        "baseline.binary_size",
    )
    if not all(isinstance(binary_size[key], str) and binary_size[key] for key in ("artifact", "measured_at", "command", "format")):
        raise ValueError("baseline.binary_size string fields must be non-empty")
    binary_metrics = _require_exact_keys(binary_size["metrics"], {"__text", "__data", "__bss", "total"}, "baseline.binary_size.metrics")
    for name, rule in binary_metrics.items():
        _require_exact_keys(rule, {"baseline_bytes", "max_increase_percent", "max_increase_bytes"}, f"binary metric {name}")

    benchmark = _require_exact_keys(
        baseline["benchmark"],
        {"runtime_perf", "wrapper_struct_overhead", "steady_state_allocation"}
        & set(baseline["benchmark"]),
        "baseline.benchmark",
    )
    _require_exact_keys(benchmark["runtime_perf"], {"artifact", "measured_at", "command", "metrics"}, "baseline.benchmark.runtime_perf")
    wrapper = _require_exact_keys(
        benchmark["wrapper_struct_overhead"],
        {"measured_at", "command", "probe", "metrics", "runtime_static_ram"},
        "baseline.benchmark.wrapper_struct_overhead",
    )
    metrics = _require_exact_keys(wrapper["metrics"], set(REQUIRED_METRICS), "baseline.benchmark.wrapper_struct_overhead.metrics")
    for name, rule in metrics.items():
        _validate_metric_rule(name, rule)

    widget_rule = metrics["widget_wrapper_struct_bytes"]
    if widget_rule["absolute_max_bytes"] != 192:
        raise ValueError("widget_wrapper_struct_bytes.absolute_max_bytes must equal 192")
    switch_rule = metrics["switch_wrapper_struct_delta_bytes"]
    if switch_rule["absolute_max_bytes"] != 32:
        raise ValueError("switch_wrapper_struct_delta_bytes.absolute_max_bytes must equal 32")
    backend_rule = metrics["backend_widget_struct_bytes"]
    if backend_rule["baseline_bytes"] != 496 or backend_rule["absolute_max_bytes"] != 512:
        raise ValueError("backend_widget_struct_bytes requires baseline 496 and absolute max 512")
    for wrapper_name in REQUIRED_WRAPPERS:
        if wrapper_name == "widget":
            continue
        rule = metrics[f"{wrapper_name}_wrapper_struct_bytes"]
        if rule["absolute_max_bytes"] != rule["baseline_bytes"] + 8:
            raise ValueError(f"{wrapper_name}_wrapper_struct_bytes.absolute_max_bytes must equal baseline_bytes + 8")

    runtime_static_ram = _require_exact_keys(
        wrapper["runtime_static_ram"],
        {"implementation", "legacy_timer_node_bytes", "legacy_event_callback_storage_bytes", "runtime_bookkeeping_bytes", "canonical_32bit_hard_limit_bytes"},
        "baseline.benchmark.wrapper_struct_overhead.runtime_static_ram",
    )
    if runtime_static_ram["implementation"] != "legacy":
        raise ValueError("runtime static RAM implementation must be legacy during M0")
    if runtime_static_ram["canonical_32bit_hard_limit_bytes"] != 1024:
        raise ValueError("runtime static RAM canonical 32-bit hard limit must equal 1024")
    for name in ("legacy_timer_node_bytes", "legacy_event_callback_storage_bytes", "runtime_bookkeeping_bytes"):
        _require_number(runtime_static_ram[name], f"runtime static RAM {name}")
        if runtime_static_ram[name] != metrics[name]["baseline_bytes"]:
            raise ValueError(f"runtime static RAM {name} must match its metric baseline")
    return baseline


def metric_exceeds_threshold(actual_bytes: int, rule: dict[str, object]) -> bool:
    baseline_bytes = int(rule["baseline_bytes"])
    max_increase_percent = float(rule["max_increase_percent"])
    max_increase_bytes = int(rule["max_increase_bytes"])
    absolute_max_bytes = int(rule["absolute_max_bytes"])
    return actual_bytes > absolute_max_bytes or (
        actual_bytes > baseline_bytes * (1.0 + max_increase_percent / 100.0)
        and actual_bytes > baseline_bytes + max_increase_bytes
    )


def check_metrics(metrics: dict[str, int], baseline: dict[str, object]) -> None:
    rules = baseline["benchmark"]["wrapper_struct_overhead"]["metrics"]
    assert isinstance(rules, dict)
    for name in REQUIRED_METRICS:
        rule = rules[name]
        assert isinstance(rule, dict)
        if name in EXACT_BASELINE_METRICS and metrics[name] != int(rule["baseline_bytes"]):
            raise AssertionError(
                f"{name} drift: probe {metrics[name]} != baseline {rule['baseline_bytes']}"
            )
        if metric_exceeds_threshold(metrics[name], rule):
            raise AssertionError(
                f"{name} exceeded regression gate: {metrics[name]} > "
                f"baseline {rule['baseline_bytes']}, absolute max {rule['absolute_max_bytes']}"
            )


def make_test_baseline() -> dict[str, object]:
    def rule(baseline_bytes: int, absolute_max_bytes: int | None = None) -> dict[str, object]:
        return {
            "baseline_bytes": baseline_bytes,
            "max_increase_percent": 5.0,
            "max_increase_bytes": 8,
            "absolute_max_bytes": baseline_bytes + 8 if absolute_max_bytes is None else absolute_max_bytes,
        }

    metrics = {f"{name}_wrapper_struct_bytes": rule(184, 192 if name == "widget" else None) for name in REQUIRED_WRAPPERS}
    metrics["switch_wrapper_struct_delta_bytes"] = rule(24, 32)
    metrics["backend_widget_struct_bytes"] = rule(496, 512)
    metrics["legacy_timer_node_bytes"] = rule(48)
    metrics["legacy_event_callback_storage_bytes"] = rule(32)
    metrics["runtime_bookkeeping_bytes"] = rule(240)
    return {
        "schema_version": 3,
        "binary_size": {
            "artifact": "build/tinyui-runtime/examples/sdl/tinyui_demo",
            "measured_at": "2026-07-13",
            "command": "size build/tinyui-runtime/examples/sdl/tinyui_demo",
            "format": "gnu-size",
            "metric_mapping": {
                "__text": ["__text", ".text"],
                "__data": ["__data", ".data"],
                "__bss": ["__bss", ".bss"],
                "total": ["Total", "dec"],
            },
            "metrics": {name: {"baseline_bytes": 1, "max_increase_percent": 5.0, "max_increase_bytes": 1} for name in ("__text", "__data", "__bss", "total")},
        },
        "benchmark": {
            "runtime_perf": {
                "artifact": "build/tinyui-runtime/examples/sdl/tinyui_demo",
                "measured_at": "2026-07-13",
                "command": "true",
                "metrics": {
                    "screen_object_create_ms": {"baseline_ms": 0.0, "max_allowed_ms": 5.0},
                    "capture_ready_ms": {"baseline_ms": 49.0, "max_allowed_ms": 65.0},
                },
            },
            "wrapper_struct_overhead": {
                "measured_at": "2026-07-13",
                "command": "true",
                "probe": "test_tinyui_wrapper_struct_overhead",
                "metrics": metrics,
                "runtime_static_ram": {
                    "implementation": "legacy",
                    "legacy_timer_node_bytes": 48,
                    "legacy_event_callback_storage_bytes": 32,
                    "runtime_bookkeeping_bytes": 240,
                    "canonical_32bit_hard_limit_bytes": 1024,
                },
            },
        },
    }


def _resolve_default_probe() -> Path | None:
    candidates = [
        ROOT / "build" / "v2.3-m3" / "tests" / "tinyui" / PROBE_TARGET,
        ROOT / "build" / "v2.3-m2" / "tests" / "tinyui" / PROBE_TARGET,
        ROOT / "build" / "v2.3-m0-full" / "tests" / "tinyui" / PROBE_TARGET,
        ROOT / "build" / "tests" / "tinyui" / PROBE_TARGET,
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check TinyUI wrapper and runtime memory baselines.")
    parser.add_argument("--baseline", type=Path, default=DEFAULT_BASELINE)
    parser.add_argument("--probe", type=Path, default=_resolve_default_probe())
    parser.add_argument("--build-dir", type=Path)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    try:
        payload = json.loads(args.baseline.read_text(encoding="utf-8"))
        baseline = validate_baseline(payload)
        probe = args.probe
        if args.build_dir is not None:
            relocated = args.build_dir / "tests" / "tinyui" / PROBE_TARGET
            if relocated.is_file():
                probe = relocated
        if probe is None:
            raise ValueError("--probe is required (or provide an existing build/v2.3-m3 probe)")
        if not probe.is_file():
            raise ValueError(f"missing probe executable: {probe}")
        completed = subprocess.run([str(probe)], check=True, capture_output=True, text=True)
        metrics = parse_probe(completed.stdout)
        check_metrics(metrics, baseline)
        # Descriptor budgets are enforced by the C probe (32-bit ABI only).
        # Surface host diagnostics without expanding the schema-required metric set.
        for line in completed.stdout.splitlines():
            if line.startswith("TINYUI_DESC_") or line.startswith("TINYUI_ABI32_DESC_"):
                print(line)
    except (OSError, ValueError, json.JSONDecodeError, subprocess.CalledProcessError, AssertionError) as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)
    print("TINYUI_WRAPPER_STRUCT_OVERHEAD_OK " + " ".join(f"{name}={metrics[name]}" for name in REQUIRED_METRICS))


if __name__ == "__main__":
    main()
