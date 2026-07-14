import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_object_overhead.py")
SPEC = importlib.util.spec_from_file_location("tinyui_object_overhead_checker", CHECKER_PATH)
assert SPEC is not None and SPEC.loader is not None
CHECKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CHECKER)


class TinyuiObjectOverheadCheckerTests(unittest.TestCase):
    def test_parse_probe_rejects_missing_required_wrapper_metric(self):
        lines = [
            f"TINYUI_SIZEOF_{wrapper.upper()}_WRAPPER=1"
            for wrapper in CHECKER.REQUIRED_WRAPPERS
            if wrapper != "table"
        ]

        with self.assertRaisesRegex(ValueError, "missing wrapper metric: table_wrapper_struct_bytes"):
            CHECKER.parse_probe("\n".join(lines))

    def test_parse_probe_rejects_duplicate_metric(self):
        with self.assertRaisesRegex(ValueError, "duplicate probe metric: widget_wrapper_struct_bytes"):
            CHECKER.parse_probe(
                "TINYUI_SIZEOF_WIDGET_WRAPPER=184\n"
                "TINYUI_SIZEOF_WIDGET_WRAPPER=184\n"
            )

    def test_parse_probe_rejects_negative_metric(self):
        with self.assertRaisesRegex(ValueError, "negative probe metric: widget_wrapper_struct_bytes"):
            CHECKER.parse_probe("TINYUI_SIZEOF_WIDGET_WRAPPER=-1\n")

    def test_schema_rejects_extra_metric_field(self):
        baseline = CHECKER.make_test_baseline()
        baseline["benchmark"]["wrapper_struct_overhead"]["metrics"][
            "widget_wrapper_struct_bytes"
        ]["unexpected"] = 1

        with self.assertRaisesRegex(ValueError, r"unexpected (?:property|key).*unexpected"):
            CHECKER.validate_baseline(baseline, CHECKER.SCHEMA_PATH)

    def test_validate_baseline_executes_nested_schema_requirements(self):
        baseline = CHECKER.make_test_baseline()
        schema = json.loads(CHECKER.SCHEMA_PATH.read_text(encoding="utf-8"))
        schema["$defs"]["binary_size"]["required"].append("schema_only_required")

        with tempfile.TemporaryDirectory() as temp_dir:
            schema_path = Path(temp_dir) / "schema.json"
            schema_path.write_text(json.dumps(schema), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "schema validation.*schema_only_required"):
                CHECKER.validate_baseline(baseline, schema_path)

    def test_schema_declares_strict_runtime_and_wrapper_metrics(self):
        schema = json.loads(CHECKER.SCHEMA_PATH.read_text(encoding="utf-8"))
        runtime_metrics = schema["$defs"]["runtime_perf"]["properties"]["metrics"]
        wrapper_metrics = schema["$defs"]["wrapper_metrics"]

        self.assertEqual(
            set(runtime_metrics["required"]),
            {"screen_object_create_ms", "capture_ready_ms"},
        )
        self.assertEqual(
            set(runtime_metrics["properties"]),
            {"screen_object_create_ms", "capture_ready_ms"},
        )
        self.assertIs(runtime_metrics["additionalProperties"], False)
        self.assertEqual(set(wrapper_metrics["required"]), set(CHECKER.REQUIRED_METRICS))
        self.assertEqual(set(wrapper_metrics["properties"]), set(CHECKER.REQUIRED_METRICS))
        self.assertIs(wrapper_metrics["additionalProperties"], False)

    def test_legacy_and_runtime_metrics_require_exact_baseline(self):
        baseline = CHECKER.make_test_baseline()
        rules = baseline["benchmark"]["wrapper_struct_overhead"]["metrics"]
        metrics = {name: int(rule["baseline_bytes"]) for name, rule in rules.items()}
        CHECKER.check_metrics(metrics, baseline)

        for name in CHECKER.EXACT_BASELINE_METRICS:
            with self.subTest(metric=name):
                drifted = dict(metrics)
                drifted[name] -= 1
                with self.assertRaisesRegex(AssertionError, rf"{name} drift"):
                    CHECKER.check_metrics(drifted, baseline)

    def test_threshold_requires_both_relative_and_absolute_growth(self):
        rule = {
            "baseline_bytes": 184,
            "max_increase_percent": 5.0,
            "max_increase_bytes": 8,
            "absolute_max_bytes": 192,
        }

        self.assertFalse(CHECKER.metric_exceeds_threshold(192, rule))
        self.assertTrue(CHECKER.metric_exceeds_threshold(193, rule))


if __name__ == "__main__":
    unittest.main()
