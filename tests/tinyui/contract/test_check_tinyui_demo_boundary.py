#!/usr/bin/env python3
"""Unit tests for the TinyUI demo boundary fail-closed checker (M4 Task 1)."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_demo_boundary.py")
MANIFEST_PATH = Path(__file__).with_name("tinyui_demo_manifest.json")


def _load_checker():
    if not CHECKER_PATH.is_file():
        raise FileNotFoundError(f"missing checker: {CHECKER_PATH}")
    spec = importlib.util.spec_from_file_location("check_tinyui_demo_boundary", CHECKER_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TinyuiDemoBoundaryCheckerTests(unittest.TestCase):
    def test_checker_and_manifest_exist(self):
        self.assertTrue(CHECKER_PATH.is_file(), f"missing checker: {CHECKER_PATH}")
        self.assertTrue(MANIFEST_PATH.is_file(), f"missing manifest: {MANIFEST_PATH}")

    def test_manifest_lists_exactly_29_sorted_demos_with_scroll_selector_spelling(self):
        data = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
        demos = data["demos"]
        self.assertEqual(data["schema_version"], 1)
        self.assertEqual(len(demos), 29)
        names = [item["name"] for item in demos]
        self.assertEqual(names, sorted(names))
        self.assertIn("scroll_selector_basic", names)
        self.assertNotIn("scroll_selecter_basic", names)
        selector = next(item for item in demos if item["name"] == "scroll_selector_basic")
        self.assertEqual(
            selector["source"],
            "tinyui/demo/scroll_selector_basic/scroll_selector_basic.c",
        )
        self.assertEqual(
            selector["build_symbol"],
            "tinyui_demo_scroll_selector_basic_build",
        )

    def test_forbidden_tokens_are_detected_and_window_prefix_is_not(self):
        checker = _load_checker()
        dirty = """
        void demo(void) {
            tinyui_app_create();
            tinyui_widget_set_pos(w, 0, 0);
            tinyui_native_escape();
            ldButton_t *b;
            arm_2d_tile_t tile;
            SIGNAL_CLICK(x);
            struct tinyui_widget *legacy = 0;
            tinyui_demos_frame(16);
            tinyui_window_create(parent, "ok");
        }
        """
        errors = checker.scan_forbidden_tokens("demo.c", dirty)
        rules = {item["rule"] for item in errors}
        self.assertIn("tinyui_app_", rules)
        self.assertIn("tinyui_widget_", rules)
        self.assertIn("tinyui_native_", rules)
        self.assertIn("ld[A-Z]", rules)
        self.assertIn("arm_2d_", rules)
        self.assertIn("SIGNAL_", rules)
        self.assertIn("struct tinyui_widget", rules)
        self.assertIn("tinyui_demos_frame", rules)
        # window is required canonical capability — not a blanket ban.
        self.assertTrue(all(item.get("token", "").startswith("tinyui_window_") is False
                            or item.get("rule") != "tinyui_window_"
                            for item in errors))
        self.assertFalse(any("tinyui_window_create" in str(item) for item in errors))

    def test_comments_and_strings_do_not_count_as_forbidden_hits(self):
        checker = _load_checker()
        text = """
        /* tinyui_widget_set_pos in comment */
        // tinyui_app_create
        const char *doc = "struct tinyui_widget and tinyui_demos_frame";
        void ok(void) { tinyui_label_create(parent, "x"); }
        """
        errors = checker.scan_forbidden_tokens("demo.c", text)
        self.assertEqual(errors, [])

    def test_builder_lifecycle_and_platform_includes_are_rejected(self):
        checker = _load_checker()
        text = """
        #include <SDL.h>
        #include "ldGui.h"
        tinyui_result_t tinyui_demo_hello_world_build(tinyui_obj_t *screen)
        {
            tinyui_init();
            tinyui_obj_t *s = tinyui_screen_create();
            tinyui_screen_load(s);
            tinyui_deinit();
            return 0;
        }
        """
        life = checker.scan_forbidden_tokens("hello.c", text, checker.BUILDER_LIFECYCLE_PATTERNS)
        rules = {item["rule"] for item in life}
        self.assertIn("tinyui_init", rules)
        self.assertIn("tinyui_deinit", rules)
        self.assertIn("tinyui_screen_create", rules)
        self.assertIn("tinyui_screen_load", rules)
        includes = checker.scan_platform_includes("hello.c", text)
        headers = {item["header"] for item in includes}
        self.assertIn("SDL.h", headers)
        self.assertIn("ldGui.h", headers)

    def test_source_set_equality_reports_missing_and_unexpected(self):
        checker = _load_checker()
        manifest = {
            "demos": [
                {
                    "name": "hello_world",
                    "source": "tinyui/demo/hello_world/hello_world.c",
                    "build_symbol": "tinyui_demo_hello_world_build",
                }
            ]
        }
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            demo_dir = root / "tinyui" / "demo"
            extra = demo_dir / "extra_demo"
            extra.mkdir(parents=True)
            (extra / "extra_demo.c").write_text("void x(void) {}\n", encoding="utf-8")
            actual = checker.collect_actual_demo_sources(demo_dir)
            # remap relative root for path checks by monkeypatching via absolute paths
            # check_source_set uses root only for relative_to of unexpected
            errors = checker.check_source_set(manifest, actual, root=root)
            codes = {item["code"] for item in errors}
            self.assertIn("missing_demo_source", codes)
            self.assertIn("unexpected_demo_source", codes)

    def test_build_symbol_and_legacy_void_detection(self):
        checker = _load_checker()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            demo_dir = root / "tinyui" / "demo" / "hello_world"
            demo_dir.mkdir(parents=True)
            source = demo_dir / "hello_world.c"
            header = demo_dir / "hello_world.h"
            source.write_text(
                "void tinyui_demo_hello_world(void)\n"
                "{\n"
                "    tinyui_screen_create();\n"
                "}\n",
                encoding="utf-8",
            )
            header.write_text(
                "void tinyui_demo_hello_world(void);\n",
                encoding="utf-8",
            )
            manifest = {
                "demos": [
                    {
                        "name": "hello_world",
                        "source": "tinyui/demo/hello_world/hello_world.c",
                        "build_symbol": "tinyui_demo_hello_world_build",
                    }
                ]
            }
            actual = {"hello_world": source}
            # Temporarily point ROOT-relative reporting; function uses module ROOT for rel paths
            # so we only assert codes that don't depend on ROOT-relative success.
            errors = checker.check_build_symbols(manifest, actual)
            codes = {item["code"] for item in errors}
            self.assertIn("missing_build_symbol", codes)
            self.assertIn("legacy_void_entry", codes)

    def test_cmake_requires_each_manifest_source_and_flags_legacy_spelling(self):
        checker = _load_checker()
        manifest = {
            "demos": [
                {
                    "name": "hello_world",
                    "source": "tinyui/demo/hello_world/hello_world.c",
                    "build_symbol": "tinyui_demo_hello_world_build",
                },
                {
                    "name": "scroll_selector_basic",
                    "source": "tinyui/demo/scroll_selector_basic/scroll_selector_basic.c",
                    "build_symbol": "tinyui_demo_scroll_selector_basic_build",
                },
            ]
        }
        cmake = """
        add_tinyui_demo(tinyui_demo
            "${TINYUI_DEMO_DIR}/hello_world/hello_world.c"
            "${TINYUI_DEMO_DIR}/scroll_selecter_basic/scroll_selecter_basic.c"
        )
        """
        errors = checker.check_cmake_sources(manifest, cmake)
        codes = {item["code"] for item in errors}
        self.assertIn("cmake_missing_demo_source", codes)
        self.assertIn("cmake_legacy_scroll_selecter_path", codes)

    def test_repo_gate_is_green_after_m4_demo_migration(self):
        """M4 closeout: real tree must pass the canonical boundary gate."""
        checker = _load_checker()
        errors = checker.check_all()
        self.assertEqual(
            errors,
            [],
            msg="expected zero demo boundary errors after M4 migration:\n"
            + "\n".join(checker.format_error(item) for item in errors),
        )

    def test_registry_contract_rejects_void_and_frame_callback(self):
        """Task 2: registry must be build(screen) only — fail on old void/frame forms."""
        checker = _load_checker()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            demo_dir = root / "tinyui" / "demo"
            demo_dir.mkdir(parents=True)
            header = demo_dir / "tinyui_demos.h"
            source = demo_dir / "tinyui_demos.c"
            main_c = root / "tinyui_demo" / "main.c"
            main_c.parent.mkdir(parents=True)
            header.write_text(
                "typedef void (*tinyui_demo_frame_cb_t)(unsigned int elapsed_ms);\n"
                "typedef void (*demo_method_cb)(void);\n"
                "bool tinyui_demos_create(char *info[], int size);\n"
                "bool tinyui_demos_get_display_config(char *info[], int size,\n"
                "                                     struct tinyui_display_config *out);\n"
                "void tinyui_demos_frame(unsigned int elapsed_ms);\n"
                "void tinyui_demos_show_help(void);\n",
                encoding="utf-8",
            )
            source.write_text(
                "typedef void (*demo_method_cb)(void);\n"
                "typedef struct {\n"
                "    const char *name;\n"
                "    demo_method_cb entry_cb;\n"
                "    tinyui_demo_frame_cb_t frame_cb;\n"
                "    int width;\n"
                "    int height;\n"
                "} demo_entry_info_t;\n"
                'static const demo_entry_info_t demos_entry_info[] = {\n'
                '    { "scroll_selecter_basic", tinyui_demo_scroll_selecter_basic, NULL, 480, 320 },\n'
                "};\n"
                "bool tinyui_demos_create(char *info[], int size) { (void)info; (void)size; return 0; }\n"
                "void tinyui_demos_frame(unsigned int elapsed_ms) { (void)elapsed_ms; }\n",
                encoding="utf-8",
            )
            main_c.write_text(
                "int main(void) {\n"
                "    tinyui_demos_create(0, 0);\n"
                "    tinyui_demos_frame(16);\n"
                "    return tinyui_timer_handler();\n"
                "}\n",
                encoding="utf-8",
            )
            errors = checker.check_registry_contract(
                root=root,
                demos_h=header,
                demos_c=source,
                runner_main=main_c,
            )
            codes = {item["code"] for item in errors}
            self.assertIn("missing_build_cb_typedef", codes)
            self.assertIn("legacy_frame_cb_typedef", codes)
            self.assertIn("legacy_void_registry_callback", codes)
            self.assertIn("registry_has_frame_cb_field", codes)
            self.assertIn("legacy_display_config_dispatch", codes)
            self.assertIn("registry_exposes_display_config_type", codes)
            self.assertIn("legacy_registry_dispatch_symbol", codes)
            self.assertIn("registry_legacy_scroll_selecter_name", codes)
            self.assertIn("runner_uses_legacy_dispatch", codes)
            self.assertIn("runner_missing_canonical_symbol", codes)

    def test_registry_contract_accepts_build_only_shape(self):
        checker = _load_checker()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            demo_dir = root / "tinyui" / "demo"
            demo_dir.mkdir(parents=True)
            header = demo_dir / "tinyui_demos.h"
            source = demo_dir / "tinyui_demos.c"
            main_c = root / "tinyui_demo" / "main.c"
            main_c.parent.mkdir(parents=True)
            header.write_text(
                "#include <stdint.h>\n"
                "#include <stdbool.h>\n"
                "#include \"tinyui.h\"\n"
                "typedef tinyui_result_t (*tinyui_demo_build_cb_t)(tinyui_obj_t *screen);\n"
                "tinyui_result_t tinyui_demos_build(const char *name, tinyui_obj_t *screen);\n"
                "bool tinyui_demos_get_display_size(const char *name, uint16_t *width, uint16_t *height);\n"
                "void tinyui_demos_show_help(void);\n"
                "tinyui_result_t tinyui_demo_hello_world_build(tinyui_obj_t *screen);\n",
                encoding="utf-8",
            )
            source.write_text(
                "#include \"tinyui_demos.h\"\n"
                "typedef struct tinyui_demo_entry {\n"
                "    const char *name;\n"
                "    tinyui_demo_build_cb_t build;\n"
                "    uint16_t width;\n"
                "    uint16_t height;\n"
                "} tinyui_demo_entry_t;\n"
                "static const tinyui_demo_entry_t demos[] = {\n"
                '    { "hello_world", tinyui_demo_hello_world_build, 480, 320 },\n'
                '    { "scroll_selector_basic", tinyui_demo_scroll_selector_basic_build, 480, 320 },\n'
                "};\n"
                "tinyui_result_t tinyui_demos_build(const char *name, tinyui_obj_t *screen)\n"
                "{ (void)name; (void)screen; return TINYUI_OK; }\n"
                "bool tinyui_demos_get_display_size(const char *name, uint16_t *width, uint16_t *height)\n"
                "{ (void)name; (void)width; (void)height; return true; }\n"
                "void tinyui_demos_show_help(void) {}\n",
                encoding="utf-8",
            )
            main_c.write_text(
                "#include \"tinyui_demos.h\"\n"
                "#include \"tinyui.h\"\n"
                "int main(int argc, char **argv) {\n"
                "    uint16_t w = 0, h = 0;\n"
                "    uint32_t next_ms = 0;\n"
                "    const char *name = (argc > 1) ? argv[1] : 0;\n"
                "    tinyui_obj_t *screen;\n"
                "    if (!tinyui_demos_get_display_size(name, &w, &h)) return 1;\n"
                "    if (tinyui_init() != TINYUI_OK) return 1;\n"
                "    screen = tinyui_screen_create();\n"
                "    if (tinyui_demos_build(name, screen) != TINYUI_OK) return 1;\n"
                "    if (tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) != TINYUI_OK) return 1;\n"
                "    while (tinyui_process(&next_ms) == TINYUI_OK) {}\n"
                "    tinyui_deinit();\n"
                "    return 0;\n"
                "}\n",
                encoding="utf-8",
            )
            errors = checker.check_registry_contract(
                root=root,
                demos_h=header,
                demos_c=source,
                runner_main=main_c,
            )
            self.assertEqual(errors, [], msg=errors)


if __name__ == "__main__":
    unittest.main()
