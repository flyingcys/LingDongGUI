import importlib.util
import json
import os
import tempfile
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_minimal_symbols.py")
SPEC = importlib.util.spec_from_file_location("tinyui_minimal_symbols_checker", CHECKER_PATH)
assert SPEC is not None and SPEC.loader is not None
CHECKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CHECKER)


class TinyuiMinimalSymbolsCheckerTests(unittest.TestCase):
    def test_enforce_policy_comes_from_baseline_profile(self):
        profile = {
            "required_symbols": ["tinyui_init"],
            "forbidden_symbols": ["tinyui_slider_"],
        }

        required, forbidden = CHECKER.enforced_policy(profile)

        self.assertEqual(required, {"tinyui_init"})
        self.assertEqual(forbidden, ("tinyui_slider_",))

    def test_enforce_requires_map_artifact_to_exist(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            profile = {
                "artifact": "binary",
                "map_artifact": "missing.map",
                "required_symbols": sorted(CHECKER.REQUIRED_SYMBOLS),
                "forbidden_symbols": list(CHECKER.FORBIDDEN_SYMBOL_PREFIXES),
            }
            with self.assertRaisesRegex(ValueError, "missing map"):
                CHECKER.check_map_artifact(profile, Path(temp_dir) / "binary")

    def test_artifact_key_resolves_relative_baseline_paths_from_repo_root(self):
        relative = "build/../build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer"
        absolute = CHECKER.ROOT / "build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer"

        with tempfile.TemporaryDirectory() as temp_dir:
            old_cwd = Path.cwd()
            try:
                os.chdir(temp_dir)
                self.assertEqual(CHECKER._artifact_key(relative), CHECKER._artifact_key(absolute))
            finally:
                os.chdir(old_cwd)

    def test_load_baseline_relocates_canonical_artifact_to_closeout_build_dir(self):
        baseline = {
            "minimal": {
                "artifact": "build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer",
                "required_symbols": sorted(CHECKER.REQUIRED_SYMBOLS),
                "forbidden_symbols": list(CHECKER.FORBIDDEN_SYMBOL_PREFIXES),
            }
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "baseline.json"
            path.write_text(json.dumps(baseline), encoding="utf-8")
            closeout_binary = CHECKER.ROOT / "build/v2.3-m0-closeout/tests/tinyui/tinyui_minimal_consumer"
            profile = CHECKER.load_baseline(path, closeout_binary)

        self.assertEqual(profile["artifact"], baseline["minimal"]["artifact"])

    def test_check_map_artifact_relocates_canonical_map_to_build_dir(self):
        profile = {
            "artifact": "build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer",
            "map_artifact": "build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer.map",
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            build_dir = Path(temp_dir) / "v2.3-m0-closeout"
            map_path = build_dir / "tests/tinyui/tinyui_minimal_consumer.map"
            map_path.parent.mkdir(parents=True)
            map_path.write_text("map", encoding="utf-8")
            binary = build_dir / "tests/tinyui/tinyui_minimal_consumer"

            selected = CHECKER.check_map_artifact(profile, binary, build_dir)

        self.assertEqual(selected, map_path.resolve())

    def test_parse_nm_accepts_defined_symbols_and_rejects_undefined_artifact(self):
        output = """
0000000100001000 T _tinyui_init
                 U _tinyui_not_in_minimal
0000000100001010 T _tinyui_screen_create
"""

        with self.assertRaisesRegex(ValueError, "undefined symbol"):
            CHECKER.parse_nm_output(output)

    def test_check_symbols_requires_minimal_lifecycle_and_widget_symbols(self):
        symbols = {
            "tinyui_init",
            "tinyui_deinit",
            "tinyui_screen_create",
            "tinyui_screen_load",
            "tinyui_timer_handler",
            "tinyui_label_create",
            "tinyui_label_set_text",
            "tinyui_button_create",
            "tinyui_button_set_text",
        }

        CHECKER.check_symbols(symbols)

        with self.assertRaisesRegex(AssertionError, "tinyui_button_set_text"):
            CHECKER.check_symbols(symbols - {"tinyui_button_set_text"})

    def test_check_symbols_rejects_non_minimal_widget_symbols(self):
        symbols = set(CHECKER.REQUIRED_SYMBOLS)
        symbols.add("tinyui_slider_create")

        with self.assertRaisesRegex(AssertionError, "non-minimal symbol"):
            CHECKER.check_symbols(symbols)

    def test_load_baseline_fails_closed_for_artifact_mismatch(self):
        baseline = {"minimal": {"artifact": "expected/minimal"}}

        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "baseline.json"
            path.write_text('{"minimal": {"artifact": "expected/minimal"}}', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "artifact mismatch"):
                CHECKER.load_baseline(path, "actual/minimal")


if __name__ == "__main__":
    unittest.main()
