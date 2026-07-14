import importlib.util
import json
import os
import tempfile
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_binary_size.py")
SPEC = importlib.util.spec_from_file_location("tinyui_binary_size_checker", CHECKER_PATH)
assert SPEC is not None and SPEC.loader is not None
CHECKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CHECKER)


def _rule(value):
    return {
        "baseline_bytes": value,
        "max_increase_percent": 5.0,
        "max_increase_bytes": 8,
    }


def _profile(artifact, value):
    return {
        "artifact": artifact,
        "metrics": {
            "__text": _rule(value),
            "__data": _rule(value),
            "__bss": _rule(value),
            "total": _rule(value),
        },
    }


class TinyuiBinarySizeCheckerTests(unittest.TestCase):
    def test_resolve_relocates_full_artifact_to_closeout_build_dir(self):
        payload = {
            "binary": {
                "full": _profile("build/v2.3-m0-full/examples/sdl/tinyui_demo", 10),
                "minimal": _profile("build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer", 20),
            }
        }
        closeout_binary = CHECKER.ROOT / "build/v2.3-m0-closeout/examples/sdl/tinyui_demo"

        with tempfile.TemporaryDirectory() as temp_dir:
            old_cwd = Path.cwd()
            try:
                os.chdir(temp_dir)
                selected = CHECKER.resolve_baseline_for_binary(payload, closeout_binary)
            finally:
                os.chdir(old_cwd)

        self.assertIs(selected, payload["binary"]["full"])

    def test_load_and_resolve_selects_minimal_by_logical_artifact(self):
        payload = {
            "binary": {
                "full": _profile("build/v2.3-m0-full/examples/sdl/tinyui_demo", 10),
                "minimal": _profile("build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer", 20),
            }
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "baseline.json"
            path.write_text(json.dumps(payload), encoding="utf-8")
            loaded = CHECKER.load_baseline(path)

        closeout_binary = CHECKER.ROOT / "build/v2.3-m0-closeout/tests/tinyui/tinyui_minimal_consumer"
        self.assertIs(
            CHECKER.resolve_baseline_for_binary(loaded, closeout_binary),
            loaded["binary"]["minimal"],
        )

    def test_resolve_fails_closed_for_unknown_artifact(self):
        payload = {
            "binary": {
                "full": _profile("build/v2.3-m0-full/examples/sdl/tinyui_demo", 10),
                "minimal": _profile("build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer", 20),
            }
        }
        with self.assertRaisesRegex(ValueError, "artifact mismatch"):
            CHECKER.resolve_baseline_for_binary(
                payload,
                CHECKER.ROOT / "build/v2.3-m0-closeout/tests/tinyui/other",
            )

    def test_load_rejects_missing_binary_profile(self):
        payload = {"binary": {"full": _profile("build/full", 10)}}
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "baseline.json"
            path.write_text(json.dumps(payload), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "minimal"):
                CHECKER.load_baseline(path)


if __name__ == "__main__":
    unittest.main()
