#!/usr/bin/env python3
"""TDD fixtures for the TinyUI removed-API gate."""

import sys
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
ROOT = CONTRACT_DIR.parents[2]
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_removed_api import (
    PUBLIC_INCLUDE,
    check_preprocessed_aggregate,
    is_forbidden_symbol,
)


class TinyuiRemovedApiCheckerTests(unittest.TestCase):
    def test_public_legacy_and_misspelled_symbols_are_forbidden(self):
        self.assertTrue(is_forbidden_symbol("tinyui_app_create"))
        self.assertTrue(is_forbidden_symbol("tinyui_widget_set_text"))
        self.assertTrue(is_forbidden_symbol("tinyui_tabel_show_keyboard"))
        self.assertTrue(is_forbidden_symbol("tinyui_scroll_selecter_create"))
        self.assertTrue(is_forbidden_symbol("tinyui_q_r_code_init"))
        self.assertTrue(is_forbidden_symbol("tinyui_button_set_press"))
        self.assertTrue(is_forbidden_symbol("tinyui_theme_create"))
        self.assertTrue(is_forbidden_symbol("tinyui_animation_init"))
        self.assertFalse(is_forbidden_symbol("tinyui_init"))
        self.assertFalse(is_forbidden_symbol("tinyui_button_set_pressed"))

    def test_private_runtime_internal_init_helpers_are_not_forbidden(self):
        # Full archives keep private helpers such as tinyui_runtime_internal_*_init.
        # The removed-API gate only polices legacy public ABI, not internal symbols.
        self.assertFalse(is_forbidden_symbol("tinyui_runtime_internal_animation_init"))
        self.assertFalse(is_forbidden_symbol("tinyui_runtime_internal_q_r_code_init"))
        self.assertFalse(is_forbidden_symbol("tinyui_runtime_internal_widget_is_kind"))

    def test_preprocessed_aggregate_resolves_generated_config_without_build_cwd(self):
        """Aggregate preprocess must not fail solely because tinyui_config.h is generated."""
        self.assertTrue((ROOT / "tinyui" / "include" / "tinyui.h").is_file())
        errors = check_preprocessed_aggregate(PUBLIC_INCLUDE)
        codes = {item.get("code") for item in errors}
        self.assertNotIn(
            "preprocess_failed",
            codes,
            msg=f"preprocess should resolve tinyui_config.h; got {errors}",
        )


if __name__ == "__main__":
    unittest.main()
