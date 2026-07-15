#!/usr/bin/env python3
"""Unit tests for TinyUI per-widget feature option / source gating checker."""

from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_feature_options.py")


def _load_checker():
    if not CHECKER_PATH.is_file():
        raise FileNotFoundError(f"missing checker: {CHECKER_PATH}")
    spec = importlib.util.spec_from_file_location("check_tinyui_feature_options", CHECKER_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class FeatureOptionsCheckerTests(unittest.TestCase):
    def test_checker_module_exists(self):
        self.assertTrue(CHECKER_PATH.is_file(), f"missing checker: {CHECKER_PATH}")

    def test_manifest_has_exactly_one_option_per_widget_and_special_filenames(self):
        checker = _load_checker()
        manifest = checker.widget_manifest()
        names = sorted(entry["name"] for entry in manifest)
        self.assertEqual(len(names), 29)
        self.assertEqual(len(set(names)), 29)
        by_name = {entry["name"]: entry for entry in manifest}
        self.assertEqual(by_name["QRCODE"]["source"], "tinyui/src/widgets/qrcode.c")
        self.assertEqual(by_name["PROGRESS_WHEEL"]["source"], "tinyui/src/widgets/progress_wheel.c")
        self.assertEqual(by_name["SCROLL_SELECTOR"]["source"], "tinyui/src/widgets/scroll_selector.c")
        for entry in manifest:
            self.assertEqual(entry["option"], f"TINYUI_ENABLE_{entry['name']}")
            self.assertTrue(entry["source"].startswith("tinyui/src/widgets/"))
            self.assertTrue(entry["source"].endswith(".c"))

    def test_parse_cmake_cache_reads_bool_enable_options(self):
        checker = _load_checker()
        with tempfile.TemporaryDirectory() as temp_dir:
            cache = Path(temp_dir) / "CMakeCache.txt"
            cache.write_text(
                "\n".join(
                    [
                        "TINYUI_ENABLE_LABEL:BOOL=ON",
                        "TINYUI_ENABLE_SLIDER:BOOL=OFF",
                        "TINYUI_ENABLE_THEME:BOOL=ON",
                        "UNRELATED:STRING=1",
                        "",
                    ]
                ),
                encoding="utf-8",
            )
            values = checker.parse_cmake_cache(cache)
        self.assertEqual(values["TINYUI_ENABLE_LABEL"], 1)
        self.assertEqual(values["TINYUI_ENABLE_SLIDER"], 0)
        self.assertEqual(values["TINYUI_ENABLE_THEME"], 1)

    def test_check_sources_require_enabled_and_forbid_disabled(self):
        checker = _load_checker()
        manifest = [
            {
                "name": "LABEL",
                "option": "TINYUI_ENABLE_LABEL",
                "source": "tinyui/src/widgets/label.c",
            },
            {
                "name": "SLIDER",
                "option": "TINYUI_ENABLE_SLIDER",
                "source": "tinyui/src/widgets/slider.c",
            },
        ]
        options = {"TINYUI_ENABLE_LABEL": 1, "TINYUI_ENABLE_SLIDER": 0}
        ok_sources = [
            "/repo/tinyui/src/core/runtime.c",
            "/repo/tinyui/src/widgets/label.c",
        ]
        errors = checker.check_feature_source_consistency(manifest, options, ok_sources)
        self.assertEqual(errors, [])

        missing_enabled = checker.check_feature_source_consistency(
            manifest,
            options,
            ["/repo/tinyui/src/core/runtime.c"],
        )
        self.assertTrue(any("label.c" in item for item in missing_enabled))

        leak_disabled = checker.check_feature_source_consistency(
            manifest,
            options,
            ok_sources + ["/repo/tinyui/src/widgets/slider.c"],
        )
        self.assertTrue(any("slider.c" in item for item in leak_disabled))

    def test_check_requires_one_option_for_every_manifest_widget(self):
        checker = _load_checker()
        manifest = checker.widget_manifest()
        options = {entry["option"]: 1 for entry in manifest}
        del options["TINYUI_ENABLE_GAUGE"]
        sources = [f"/repo/{entry['source']}" for entry in manifest]
        errors = checker.check_feature_source_consistency(manifest, options, sources)
        self.assertTrue(any("TINYUI_ENABLE_GAUGE" in item for item in errors))

    def test_optional_modules_are_declared(self):
        checker = _load_checker()
        modules = checker.optional_module_options()
        self.assertEqual(
            set(modules),
            {
                "TINYUI_ENABLE_THEME",
                "TINYUI_ENABLE_DIAGNOSTICS",
                "TINYUI_ENABLE_NATIVE_INTEROP",
            },
        )


if __name__ == "__main__":
    unittest.main()
