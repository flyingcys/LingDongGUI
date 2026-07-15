#!/usr/bin/env python3
"""TDD fixtures for the TinyUI v2.3 canonical public API checker."""

import json
import hashlib
import sys
import tempfile
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_v23_public_api import check_contract


def _write_fixture(root: Path, files: dict[str, str], entries: list[dict]) -> Path:
    include_dir = root / "tinyui" / "include"
    for relative_path, text in files.items():
        path = include_dir / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    manifest = root / "manifest.json"
    manifest.write_text(
        json.dumps(
            {
                "schema_version": "tinyui-v2.3-public-api-v1",
                "entries": entries,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return manifest


def _entry(kind: str, name: str, header: str, signature: str, availability: str = "always") -> dict:
    return {
        "kind": kind,
        "name": name,
        "header": header,
        "signature": signature,
        "availability": availability,
    }


class TinyuiV23PublicApiCheckerTests(unittest.TestCase):
    def test_missing_symbol_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {"core/runtime.h": "typedef int tinyui_result_t;\n"},
                [
                    _entry(
                        "function",
                        "tinyui_process",
                        "core/runtime.h",
                        "tinyui_result_t tinyui_process(uint32_t *next_ms);",
                    )
                ],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(any(error["code"] == "missing_symbol" for error in errors))

    def test_signature_mismatch_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/event.h": (
                        "typedef struct tinyui_obj tinyui_obj_t;\n"
                        "typedef struct tinyui_event tinyui_event_t;\n"
                        "typedef void (*tinyui_event_cb_t)(tinyui_obj_t *obj, "
                        "const tinyui_event_t *event, void *user_data);\n"
                    )
                },
                [
                    _entry(
                        "typedef",
                        "tinyui_event_cb_t",
                        "core/event.h",
                        "typedef void (*tinyui_event_cb_t)(tinyui_obj_t *obj, void *user_data);",
                    )
                ],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(any(error["code"] == "signature_mismatch" for error in errors))

    def test_function_with_nested_function_pointer_parameter_is_scanned(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/event.h": (
                        "typedef struct tinyui_event tinyui_event_t;\n"
                        "int tinyui_register(void (*callback)(tinyui_event_t *));\n"
                    )
                },
                [
                    _entry(
                        "typedef",
                        "tinyui_event_t",
                        "core/event.h",
                        "typedef struct tinyui_event tinyui_event_t;",
                    ),
                    _entry(
                        "function",
                        "tinyui_register",
                        "core/event.h",
                        "int tinyui_register(void (*callback)(tinyui_event_t *));",
                    )
                ],
            )

            self.assertEqual(check_contract(root, manifest), [])

    def test_header_and_manifest_must_match_in_both_directions(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/runtime.h": (
                        "typedef int tinyui_result_t;\n"
                        "tinyui_result_t tinyui_init(void);\n"
                        "tinyui_result_t tinyui_process(uint32_t *next_ms);\n"
                    )
                },
                [
                    _entry(
                        "function",
                        "tinyui_init",
                        "core/runtime.h",
                        "tinyui_result_t tinyui_init(void);",
                    )
                ],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(any(error["code"] == "unregistered_symbol" for error in errors))

    def test_forbidden_aggregate_include_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "tinyui.h": '#include "core/app.h"\n',
                    "core/app.h": "typedef int tinyui_legacy_app_t;\n",
                },
                [],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(
                any(error["code"] == "forbidden_aggregate_include" for error in errors)
            )

    def test_legacy_native_and_misspelled_symbols_are_forbidden(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/legacy.h": (
                        "int tinyui_widget_set_text(void *widget, const char *text);\n"
                        "int tinyui_native_image(void);\n"
                        "int tinyui_tabel_show_keyboard(void *table);\n"
                        "int tinyui_scroll_selecter_create(void *parent);\n"
                        "int tinyui_q_r_code_init(void);\n"
                    )
                },
                [],
            )

            errors = check_contract(root, manifest)

            codes = {error["code"] for error in errors}
            self.assertIn("forbidden_legacy_symbol", codes)
            self.assertIn("forbidden_native_symbol", codes)
            self.assertIn("forbidden_misspelled_symbol", codes)

    def test_image_source_public_header_has_no_legacy_tile_fields(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "resource/image_source.h": (
                        "typedef struct tinyui_image_source {\n"
                        "    void *img_tile;\n"
                        "    void *mask_tile;\n"
                        "} tinyui_image_source_t;\n"
                    )
                },
                [],
            )

            errors = check_contract(root, manifest)

            codes = {error["code"] for error in errors}
            self.assertIn("forbidden_image_source_void_pointer", codes)
            self.assertIn("forbidden_image_source_legacy_field", codes)

    def test_pressed_spelling_is_not_classified_as_misspelled(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "widgets/button.h": (
                        "int tinyui_button_set_pressed(void *button, int pressed);\n"
                        "int tinyui_button_get_pressed(void *button, int *pressed);\n"
                    )
                },
                [
                    _entry(
                        "function",
                        "tinyui_button_set_pressed",
                        "widgets/button.h",
                        "int tinyui_button_set_pressed(void *button, int pressed);",
                    ),
                    _entry(
                        "function",
                        "tinyui_button_get_pressed",
                        "widgets/button.h",
                        "int tinyui_button_get_pressed(void *button, int *pressed);",
                    ),
                ],
            )

            errors = check_contract(root, manifest)

            self.assertFalse(
                any(error["code"] == "forbidden_misspelled_symbol" for error in errors)
            )

    def test_availability_must_be_always_or_one_feature_macro(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {"core/runtime.h": "int tinyui_init(void);\n"},
                [
                    _entry(
                        "function",
                        "tinyui_init",
                        "core/runtime.h",
                        "int tinyui_init(void);",
                        "TINYUI_ENABLE_RUNTIME && TINYUI_ENABLE_EXTRA",
                    )
                ],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(any(error["code"] == "invalid_availability" for error in errors))

    def test_duplicate_public_symbol_in_two_headers_is_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/one.h": "int tinyui_duplicate(void);\n",
                    "core/two.h": "int tinyui_duplicate(void);\n",
                },
                [
                    _entry(
                        "function",
                        "tinyui_duplicate",
                        "core/one.h",
                        "int tinyui_duplicate(void);",
                    )
                ],
            )

            errors = check_contract(root, manifest)

            self.assertTrue(any(error["code"] == "duplicate_public_symbol" for error in errors))

    def test_manifest_hash_is_stable_and_detects_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {"core/runtime.h": "int tinyui_init(void);\n"},
                [_entry("function", "tinyui_init", "core/runtime.h", "int tinyui_init(void);")],
            )
            value = json.loads(manifest.read_text(encoding="utf-8"))
            payload = json.dumps(
                {"schema_version": value["schema_version"], "entries": value["entries"]},
                ensure_ascii=True,
                sort_keys=True,
                separators=(",", ":"),
            ).encode("utf-8")
            value["manifest_sha256"] = hashlib.sha256(payload).hexdigest()
            manifest.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

            self.assertEqual(check_contract(root, manifest), [])

            value["entries"][0]["signature"] = "void tinyui_init(void);"
            manifest.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")
            errors = check_contract(root, manifest)
            self.assertTrue(any(error["code"] == "manifest_hash_mismatch" for error in errors))

    def test_braced_enum_and_struct_typedefs_are_scanned_intact(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = (
                "typedef enum tinyui_result {\n"
                "    TINYUI_OK = 0,\n"
                "    TINYUI_ERROR_INVALID_ARG,\n"
                "} tinyui_result_t;\n"
                "typedef struct tinyui_key_event_data {\n"
                "    uint16_t key;\n"
                "    bool pressed;\n"
                "} tinyui_key_event_data_t;\n"
                "typedef struct tinyui_event {\n"
                "    tinyui_result_t code;\n"
                "    void *user_data;\n"
                "    union {\n"
                "        int32_t value;\n"
                "        tinyui_key_event_data_t key;\n"
                "    } data;\n"
                "} tinyui_event_t;\n"
            )
            manifest = _write_fixture(
                root,
                {"core/event.h": header},
                [
                    _entry(
                        "typedef",
                        "tinyui_result_t",
                        "core/event.h",
                        "typedef enum tinyui_result { TINYUI_OK = 0, "
                        "TINYUI_ERROR_INVALID_ARG } tinyui_result_t;",
                    ),
                    _entry(
                        "typedef",
                        "tinyui_key_event_data_t",
                        "core/event.h",
                        "typedef struct tinyui_key_event_data { uint16_t key; "
                        "bool pressed; } tinyui_key_event_data_t;",
                    ),
                    _entry(
                        "typedef",
                        "tinyui_event_t",
                        "core/event.h",
                        "typedef struct tinyui_event { tinyui_result_t code; "
                        "void *user_data; union { int32_t value; "
                        "tinyui_key_event_data_t key; } data; } tinyui_event_t;",
                    ),
                ],
            )

            self.assertEqual(check_contract(root, manifest), [])

    def test_function_like_macros_are_scanned(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/event.h": (
                        "#define TINYUI_EVENT_MASK(code) (UINT32_C(1) << (code))\n"
                        "#define TINYUI_EVENT_MASK_ALL UINT32_MAX\n"
                    )
                },
                [
                    _entry(
                        "macro",
                        "TINYUI_EVENT_MASK",
                        "core/event.h",
                        "#define TINYUI_EVENT_MASK(code) (UINT32_C(1) << (code))",
                    ),
                    _entry(
                        "macro",
                        "TINYUI_EVENT_MASK_ALL",
                        "core/event.h",
                        "#define TINYUI_EVENT_MASK_ALL UINT32_MAX",
                    ),
                ],
            )

            self.assertEqual(check_contract(root, manifest), [])

    def test_integration_headers_are_scanned_but_not_required_in_aggregate(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "tinyui.h": '#include "core/runtime.h"\n',
                    "core/runtime.h": "int tinyui_init(void);\n",
                    "integration/input.h": (
                        "typedef enum tinyui_key { TINYUI_KEY_ENTER = 0 } tinyui_key_t;\n"
                        "int tinyui_input_send_key(tinyui_key_t key, bool pressed);\n"
                    ),
                },
                [
                    _entry(
                        "function",
                        "tinyui_init",
                        "core/runtime.h",
                        "int tinyui_init(void);",
                    ),
                    _entry(
                        "typedef",
                        "tinyui_key_t",
                        "integration/input.h",
                        "typedef enum tinyui_key { TINYUI_KEY_ENTER = 0 } tinyui_key_t;",
                    ),
                    _entry(
                        "function",
                        "tinyui_input_send_key",
                        "integration/input.h",
                        "int tinyui_input_send_key(tinyui_key_t key, bool pressed);",
                    ),
                ],
            )

            self.assertEqual(check_contract(root, manifest), [])

    def test_defined_guard_macros_are_not_registered_as_public_api(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = _write_fixture(
                root,
                {
                    "core/obj.h": (
                        "#ifndef TINYUI_OBJ_T_DEFINED\n"
                        "typedef struct tinyui_obj tinyui_obj_t;\n"
                        "#define TINYUI_OBJ_T_DEFINED\n"
                        "#endif\n"
                    )
                },
                [
                    _entry(
                        "typedef",
                        "tinyui_obj_t",
                        "core/obj.h",
                        "typedef struct tinyui_obj tinyui_obj_t;",
                    )
                ],
            )

            self.assertEqual(check_contract(root, manifest), [])


if __name__ == "__main__":
    unittest.main()
