import importlib.util
import pathlib
import unittest


MODULE_PATH = pathlib.Path(__file__).with_name("check_picoui_runtime.py")
SPEC = importlib.util.spec_from_file_location("check_picoui_runtime", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class CheckPicouiRuntimeTests(unittest.TestCase):
    def test_all_demos_alias_maps_to_all_targets(self) -> None:
        selected = MODULE._targets_for_demo("all", all_demos=True)
        self.assertEqual(selected, MODULE.TARGETS)

    def test_runtime_only_demo_does_not_require_capture(self) -> None:
        failures = MODULE._validate_runtime_result(
            target="picoui_hello_world_demo",
            runtime_policy="runtime_only",
            result={
                "returncode": 1,
                "stdout": "PICOUI_RUNTIME_READY\n",
                "stderr": "",
                "runtime_ready": True,
                "capture_ready": False,
                "capture_size": 0,
            },
        )
        self.assertEqual(failures, [])

    def test_visible_demo_still_requires_capture(self) -> None:
        failures = MODULE._validate_runtime_result(
            target="picoui_basic_widgets_demo",
            runtime_policy="visible",
            result={
                "returncode": 0,
                "stdout": "PICOUI_RUNTIME_READY\n",
                "stderr": "",
                "runtime_ready": True,
                "capture_ready": False,
                "capture_size": 0,
            },
        )
        self.assertIn("expected capture artifact", failures)


if __name__ == "__main__":
    unittest.main()
