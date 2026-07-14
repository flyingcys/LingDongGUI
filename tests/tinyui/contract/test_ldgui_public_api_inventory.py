#!/usr/bin/env python3
"""LingDongGUI public API inventory scanner contract tests."""

import sys
import json
import subprocess
import tempfile
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_ldgui_public_api_inventory import scan_header_text


class ScanHeaderTextTests(unittest.TestCase):
    def test_scan_text_records_object_like_macro_alias(self):
        text = "#define ldCheckBoxSetHidden ldBaseSetHidden\n"

        self.assertEqual(
            scan_header_text("src/gui/ldCheckBox.h", text),
            [
                {
                    "header": "src/gui/ldCheckBox.h",
                    "kind": "macro",
                    "symbol": "ldCheckBoxSetHidden",
                    "target": "ldBaseSetHidden",
                }
            ],
        )

    def test_scan_text_ignores_comments_and_normalizes_declarations(self):
        text = """\\
        /* int ldFake(void); */
        extern "C" {
        int ldLabelSetText(
            ldLabel_t *label,
            const char *text
        );
        }
        #define ldLabelGetText(obj) _ldLabelGetText(obj)
        // int ldCommentOnly(void);
        int ldLabelSetText(ldLabel_t *label, const char *text);
        """

        self.assertEqual(
            scan_header_text("src/gui/ldLabel.h", text),
            [
                {
                    "header": "src/gui/ldLabel.h",
                    "kind": "macro",
                    "symbol": "ldLabelGetText",
                    "target": "_ldLabelGetText",
                },
                {
                    "header": "src/gui/ldLabel.h",
                    "kind": "function",
                    "symbol": "ldLabelSetText",
                    "signature": "int ldLabelSetText(ldLabel_t *label, const char *text);",
                },
            ],
        )

    def test_scan_text_consumes_continuation_macro_before_next_function(self):
        text = """\\
        #define ldSampleGet(value) \\
            _ldSampleGet(value)
        int ldSampleSet(int value);
        """

        self.assertEqual(
            scan_header_text("src/gui/ldSample.h", text),
            [
                {
                    "header": "src/gui/ldSample.h",
                    "kind": "macro",
                    "symbol": "ldSampleGet",
                    "target": "_ldSampleGet",
                },
                {
                    "header": "src/gui/ldSample.h",
                    "kind": "function",
                    "symbol": "ldSampleSet",
                    "signature": "int ldSampleSet(int value);",
                },
            ],
        )

    def test_scan_text_records_statement_expression_macro_with_signature(self):
        text = """\\
        #define ldTimeOut(__ms, isReset, ...) ({static int64_t timestamp; \\
            __ldTimeOut(__ms, isReset, (&timestamp, ##__VA_ARGS__));})
        """

        self.assertEqual(
            scan_header_text("src/gui/ldBase.h", text),
            [
                {
                    "header": "src/gui/ldBase.h",
                    "kind": "macro",
                    "symbol": "ldTimeOut",
                    "signature": (
                        "#define ldTimeOut(__ms, isReset, ...) ({static int64_t timestamp; "
                        "__ldTimeOut(__ms, isReset, (&timestamp, ##__VA_ARGS__));})"
                    ),
                }
            ],
        )


class InventoryCliTests(unittest.TestCase):
    def test_check_reports_known_extra_drift_from_legacy_snapshot(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            header_directory = root / "src" / "gui"
            header_directory.mkdir(parents=True)
            (header_directory / "ldBase.h").write_text(
                "void ldBaseGetScreenSize(int16_t *width, int16_t *height);\n"
                "void ldBaseGetScreenSizeForScene(void *scene, int16_t *width, int16_t *height);\n",
                encoding="utf-8",
            )
            (header_directory / "ldGui.h").write_text(
                "void ldGuiDisposeNodeTree(void *scene, void *widget);\n",
                encoding="utf-8",
            )
            (header_directory / "ldText.h").write_text(
                "void ldTextSetScrollEnabled(void *widget, bool enabled);\n"
                "int ldExisting(int value);\n",
                encoding="utf-8",
            )
            output = root / "inventory.json"
            expected_output = root / "expected.json"
            output.write_text(
                json.dumps(
                    {
                        "schema_version": "a-0.8-ldgui-public-api-inventory-v1",
                        "source": "src/gui/ld*.h",
                        "public_api_total": 2,
                        "widgets": [
                            {
                                "name": "legacy",
                                "required_native_apis": [
                                    {
                                        "header": "src/gui/ldText.h",
                                        "ldgui_symbol": "ldExisting",
                                        "signature": "int ldExisting(void);",
                                    },
                                    {
                                        "header": "src/gui/ldText.h",
                                        "ldgui_symbol": "ldRemoved",
                                        "signature": "int ldRemoved(void);",
                                    },
                                ],
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            expected_output.write_text(
                json.dumps(
                    {
                        "schema_version": "a-0.8-ldgui-public-api-expected-symbols-v1",
                        "source": "src/gui/ld*.h",
                        "symbols": ["ldExisting", "ldRemoved"],
                    }
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [
                    sys.executable,
                    str(CONTRACT_DIR / "check_ldgui_public_api_inventory.py"),
                    "--root",
                    str(root),
                    "--output",
                    str(output),
                    "--expected-output",
                    str(expected_output),
                    "--check",
                ],
                text=True,
                capture_output=True,
                check=False,
            )

        self.assertNotEqual(0, result.returncode)
        report = json.loads(result.stdout)
        self.assertEqual(
            [
                "ldBaseGetScreenSize",
                "ldBaseGetScreenSizeForScene",
                "ldGuiDisposeNodeTree",
                "ldTextSetScrollEnabled",
            ],
            [entry["symbol"] for entry in report["extra"]],
        )
        self.assertEqual("ldRemoved", report["missing"][0]["symbol"])
        self.assertEqual("ldExisting", report["changed_signature"][0]["current"]["symbol"])
        self.assertTrue(report["schema_errors"])

    def test_check_reports_missing_extra_and_changed_signature(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            header = root / "src" / "gui" / "ldSample.h"
            header.parent.mkdir(parents=True)
            header.write_text(
                "int ldCurrent(void);\nint ldChanged(int value);\n",
                encoding="utf-8",
            )
            output = root / "inventory.json"
            expected_output = root / "expected.json"
            output.write_text(
                json.dumps(
                    {
                        "schema_version": "tinyui-v2.3-ldgui-public-api-inventory-v1",
                        "source": "src/gui/ld*.h",
                        "public_api_total": 2,
                        "entries": [
                            {
                                "header": "src/gui/ldSample.h",
                                "kind": "function",
                                "symbol": "ldChanged",
                                "signature": "int ldChanged(void);",
                            },
                            {
                                "header": "src/gui/ldSample.h",
                                "kind": "function",
                                "symbol": "ldRemoved",
                                "signature": "int ldRemoved(void);",
                            },
                        ],
                    }
                ),
                encoding="utf-8",
            )
            expected_output.write_text(
                json.dumps(
                    {
                        "schema_version": "tinyui-v2.3-ldgui-public-api-expected-symbols-v1",
                        "source": "src/gui/ld*.h",
                        "symbols": ["ldChanged", "ldCurrent"],
                    }
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [
                    sys.executable,
                    str(CONTRACT_DIR / "check_ldgui_public_api_inventory.py"),
                    "--root",
                    str(root),
                    "--output",
                    str(output),
                    "--expected-output",
                    str(expected_output),
                    "--check",
                ],
                text=True,
                capture_output=True,
                check=False,
            )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing", result.stdout)
        self.assertIn("extra", result.stdout)
        self.assertIn("changed_signature", result.stdout)

    def test_write_refreshes_only_inventory_outputs(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            header = root / "src" / "gui" / "ldSample.h"
            header.parent.mkdir(parents=True)
            header.write_text("#define ldSampleGet() _ldSampleGet()\n", encoding="utf-8")
            output = root / "inventory.json"
            expected_output = root / "expected.json"
            ledger = root / "tests" / "tinyui" / "contract" / "native_api_gap_ledger.json"
            ledger.parent.mkdir(parents=True)
            ledger.write_text('{"manual":"preserve"}\n', encoding="utf-8")

            result = subprocess.run(
                [
                    sys.executable,
                    str(CONTRACT_DIR / "check_ldgui_public_api_inventory.py"),
                    "--root",
                    str(root),
                    "--output",
                    str(output),
                    "--expected-output",
                    str(expected_output),
                    "--write",
                ],
                text=True,
                capture_output=True,
                check=False,
            )

            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual('{"manual":"preserve"}\n', ledger.read_text(encoding="utf-8"))
            self.assertEqual(
                ["ldSampleGet"],
                json.loads(expected_output.read_text(encoding="utf-8"))["symbols"],
            )


if __name__ == "__main__":
    unittest.main()
