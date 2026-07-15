#!/usr/bin/env python3
"""Unit tests for TinyUI lightweight / anti-heavyweight architecture checker."""

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


CHECKER_PATH = Path(__file__).with_name("check_tinyui_lightweight_architecture.py")


def _load_checker():
    if not CHECKER_PATH.is_file():
        raise FileNotFoundError(f"missing checker: {CHECKER_PATH}")
    spec = importlib.util.spec_from_file_location(
        "check_tinyui_lightweight_architecture", CHECKER_PATH
    )
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class LightweightArchitectureCheckerTests(unittest.TestCase):
    def test_checker_module_exists(self):
        self.assertTrue(CHECKER_PATH.is_file(), f"missing checker: {CHECKER_PATH}")

    def test_forbidden_public_identifiers_are_rejected(self):
        checker = _load_checker()
        fixtures = {
            "style_class_registry.h": "typedef struct tinyui_style_class_registry { int n; } t;",
            "event_bubble.h": "void tinyui_event_bubble(void);",
            "resource_cache.h": "struct tinyui_resource_cache;",
            "renderer_plugin.h": "void tinyui_renderer_plugin_init(void);",
            "property_database.h": "int tinyui_property_database_lookup(void);",
        }
        for name, body in fixtures.items():
            with self.subTest(name=name):
                errors = checker.scan_public_identifiers_text(name, body)
                self.assertTrue(errors, f"expected rejection for {name}")

    def test_comments_and_strings_do_not_count_as_identifier_hits(self):
        checker = _load_checker()
        text = """
        /* mentions tinyui_style_class_registry only in a comment */
        // tinyui_event_bubble
        const char *doc = "tinyui_resource_cache and tinyui_renderer_plugin";
        int tinyui_process(void);
        """
        errors = checker.scan_public_identifiers_text("core/runtime.h", text)
        self.assertEqual(errors, [])

    def test_scroll_selector_is_not_false_positive_for_selector(self):
        checker = _load_checker()
        text = """
        tinyui_obj_t *tinyui_scroll_selector_create(tinyui_obj_t *parent);
        #define TINYUI_ENABLE_SCROLL_SELECTOR 1
        """
        errors = checker.scan_public_identifiers_text("widgets/scroll_selector.h", text)
        self.assertEqual(errors, [])

    def test_malloc_inside_tinyui_process_is_rejected(self):
        checker = _load_checker()
        text = """
        tinyui_result_t tinyui_process(uint32_t *next_ms)
        {
            void *p = malloc(16);
            (void)p;
            (void)next_ms;
            return 0;
        }
        """
        errors = checker.scan_forbidden_allocations(text, ["tinyui_process"])
        self.assertTrue(any("malloc" in item for item in errors))

    def test_allowed_process_body_without_heap_is_clean(self):
        checker = _load_checker()
        text = """
        tinyui_result_t tinyui_process(uint32_t *next_ms)
        {
            *next_ms = 0;
            return 0;
        }
        """
        errors = checker.scan_forbidden_allocations(text, ["tinyui_process"])
        self.assertEqual(errors, [])

    def test_repo_public_and_runtime_scan_helpers_exist(self):
        checker = _load_checker()
        self.assertTrue(callable(checker.scan_public_identifiers))
        self.assertTrue(callable(checker.scan_forbidden_allocations))


if __name__ == "__main__":
    unittest.main()
