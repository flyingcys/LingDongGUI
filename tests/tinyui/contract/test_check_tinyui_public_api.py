#!/usr/bin/env python3
"""TINYUI public/release checker inventory compatibility tests."""

import sys
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_public_api import _inventory_rows_by_symbol
from check_tinyui_release_capability_matrix import _inventory_symbols_from_inventory


class InventoryCompatibilityTests(unittest.TestCase):
    def test_entries_inventory_is_consumed_by_both_checkers(self):
        inventory = {
            "schema_version": "tinyui-v2.3-ldgui-public-api-inventory-v1",
            "entries": [
                {
                    "header": "src/gui/ldBase.h",
                    "kind": "function",
                    "symbol": "ldBaseGetScreenSize",
                    "signature": "void ldBaseGetScreenSize(int16_t *width, int16_t *height);",
                },
                {
                    "header": "src/gui/ldCheckBox.h",
                    "kind": "macro",
                    "symbol": "ldCheckBoxSetHidden",
                    "target": "ldBaseSetHidden",
                },
            ],
        }

        rows = _inventory_rows_by_symbol(inventory)

        self.assertEqual(
            {"ldBaseGetScreenSize", "ldCheckBoxSetHidden"},
            set(rows),
        )
        self.assertEqual(
            {"ldBaseGetScreenSize", "ldCheckBoxSetHidden"},
            _inventory_symbols_from_inventory(inventory),
        )

    def test_legacy_widgets_inventory_remains_supported(self):
        inventory = {
            "schema_version": "a-0.8-ldgui-public-api-inventory-v1",
            "widgets": [
                {
                    "name": "legacy",
                    "required_native_apis": [
                        {
                            "ldgui_symbol": "ldExisting",
                            "required": True,
                        }
                    ],
                }
            ],
        }

        self.assertEqual({"ldExisting"}, set(_inventory_rows_by_symbol(inventory)))
        self.assertEqual({"ldExisting"}, _inventory_symbols_from_inventory(inventory))


if __name__ == "__main__":
    unittest.main()
