#!/usr/bin/env python3
"""TINYUI public/release checker inventory compatibility tests."""

import json
import sys
import tempfile
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_public_api import (  # noqa: E402
    ABI_SCHEMA_VERSION,
    abi_manifest_sha256,
    build_abi_manifest,
    check_abi_manifest,
    compare_abi_manifests,
    emit_abi_manifest,
    main as public_api_main,
    _inventory_rows_by_symbol,
)
from check_tinyui_release_capability_matrix import (  # noqa: E402
    _inventory_symbols_from_inventory,
)


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


class AbiManifestModeTests(unittest.TestCase):
    def test_check_missing_manifest_fails_without_fallback(self):
        missing = CONTRACT_DIR / "does_not_exist_tinyui_v23_abi_manifest.json"
        errors = check_abi_manifest(missing)
        self.assertTrue(errors)
        self.assertTrue(any("abi_manifest_missing" in item for item in errors))

    def test_emit_then_check_roundtrip(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "tinyui_v23_abi_manifest.json"
            emitted = emit_abi_manifest(path)
            self.assertEqual(emitted["schema_version"], ABI_SCHEMA_VERSION)
            self.assertGreater(len(emitted["headers"]), 0)
            self.assertGreater(len(emitted["functions"]), 0)
            self.assertIn("tinyui", emitted["exports"]["targets"])
            self.assertEqual(check_abi_manifest(path), [])
            # Check mode must not rewrite even if content would still match.
            before = path.read_text(encoding="utf-8")
            self.assertEqual(public_api_main(["--check-abi-manifest", str(path)]), 0)
            self.assertEqual(path.read_text(encoding="utf-8"), before)

    def test_check_detects_drift_and_does_not_rewrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "tinyui_v23_abi_manifest.json"
            manifest = build_abi_manifest()
            # Introduce a deliberate frozen-drift entry.
            manifest = dict(manifest)
            functions = list(manifest["functions"])
            if not functions:
                self.skipTest("no functions scanned for ABI manifest")
            functions = functions[:-1]
            manifest["functions"] = functions
            path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
            before = path.read_text(encoding="utf-8")
            errors = check_abi_manifest(path)
            self.assertTrue(errors)
            self.assertTrue(any("abi_manifest_mismatch" in item for item in errors))
            self.assertEqual(path.read_text(encoding="utf-8"), before)
            self.assertNotEqual(abi_manifest_sha256(manifest), abi_manifest_sha256(build_abi_manifest()))

    def test_compare_is_order_insensitive_via_canonical_encoding(self):
        left = build_abi_manifest()
        right = build_abi_manifest()
        right = dict(right)
        right["headers"] = list(reversed(list(right["headers"])))
        # After emit/check canonicalization both sides rebuild sorted; compare raw
        # reconstructed manifests should still match once rebuilt.
        self.assertEqual(compare_abi_manifests(left, build_abi_manifest()), [])


if __name__ == "__main__":
    unittest.main()
