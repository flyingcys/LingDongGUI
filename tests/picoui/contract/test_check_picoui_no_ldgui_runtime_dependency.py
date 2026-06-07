import importlib.util
import json
import pathlib
import tempfile
import threading
import time
import unittest
from unittest import mock


MODULE_PATH = pathlib.Path(__file__).with_name("check_picoui_no_ldgui_runtime_dependency.py")
SPEC = importlib.util.spec_from_file_location("check_picoui_no_ldgui_runtime_dependency", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class CheckPicouiNoLdguiRuntimeDependencyTests(unittest.TestCase):
    def test_load_codemodel_falls_back_to_latest_reply_file(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            reply_dir = pathlib.Path(tmp)
            index_path = reply_dir / "index-1.json"
            stale_name = "codemodel-v2-stale.json"
            fresh_name = "codemodel-v2-fresh.json"
            index_path.write_text(
                json.dumps(
                    {
                        "reply": {
                            "codemodel-v2": {
                                "jsonFile": stale_name,
                                "kind": "codemodel",
                                "version": {"major": 2, "minor": 9},
                            }
                        }
                    }
                ),
                encoding="utf-8",
            )
            (reply_dir / fresh_name).write_text(
                json.dumps(
                    {
                        "configurations": [
                            {
                                "targets": [
                                    {
                                        "id": "target-id",
                                        "name": "picoui_demo",
                                        "jsonFile": "target.json",
                                    }
                                ]
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            (reply_dir / "target.json").write_text(
                json.dumps({"dependencies": [], "sources": []}),
                encoding="utf-8",
            )

            with mock.patch.object(MODULE, "REPLY_DIR", reply_dir):
                targets, by_id = MODULE._load_codemodel()

            self.assertEqual([target["name"] for target in targets], ["picoui_demo"])
            self.assertIn("target-id", by_id)

    def test_load_codemodel_retries_when_reply_is_not_ready(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            reply_dir = pathlib.Path(tmp)
            stale_index = reply_dir / "index-1.json"
            stale_index.write_text(
                json.dumps(
                    {
                        "reply": {
                            "codemodel-v2": {
                                "jsonFile": "codemodel-v2-missing.json",
                                "kind": "codemodel",
                                "version": {"major": 2, "minor": 9},
                            }
                        }
                    }
                ),
                encoding="utf-8",
            )

            def fake_configure() -> None:
                (reply_dir / "index-2.json").write_text(
                    json.dumps(
                        {
                            "reply": {
                                "codemodel-v2": {
                                    "jsonFile": "codemodel-v2-fresh.json",
                                    "kind": "codemodel",
                                    "version": {"major": 2, "minor": 9},
                                }
                            }
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "codemodel-v2-fresh.json").write_text(
                    json.dumps(
                        {
                            "configurations": [
                                {
                                    "targets": [
                                        {
                                            "id": "target-id",
                                            "name": "picoui_demo",
                                            "jsonFile": "target.json",
                                        }
                                    ]
                                }
                            ]
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "target.json").write_text(
                    json.dumps({"dependencies": [], "sources": []}),
                    encoding="utf-8",
                )

            with mock.patch.object(MODULE, "REPLY_DIR", reply_dir):
                with mock.patch.object(MODULE, "_configure_file_api_build", side_effect=fake_configure):
                    targets, by_id = MODULE._load_codemodel()

            self.assertEqual([target["name"] for target in targets], ["picoui_demo"])
            self.assertIn("target-id", by_id)

    def test_load_codemodel_retries_when_index_has_no_reply(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            reply_dir = pathlib.Path(tmp)
            (reply_dir / "index-1.json").write_text(
                json.dumps({"objects": []}),
                encoding="utf-8",
            )

            def fake_configure() -> None:
                (reply_dir / "index-2.json").write_text(
                    json.dumps(
                        {
                            "reply": {
                                "codemodel-v2": {
                                    "jsonFile": "codemodel-v2-fresh.json",
                                    "kind": "codemodel",
                                    "version": {"major": 2, "minor": 9},
                                }
                            }
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "codemodel-v2-fresh.json").write_text(
                    json.dumps(
                        {
                            "configurations": [
                                {
                                    "targets": [
                                        {
                                            "id": "target-id",
                                            "name": "picoui_demo",
                                            "jsonFile": "target.json",
                                        }
                                    ]
                                }
                            ]
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "target.json").write_text(
                    json.dumps({"dependencies": [], "sources": []}),
                    encoding="utf-8",
                )

            with mock.patch.object(MODULE, "REPLY_DIR", reply_dir):
                with mock.patch.object(MODULE, "_configure_file_api_build", side_effect=fake_configure):
                    targets, by_id = MODULE._load_codemodel()

            self.assertEqual([target["name"] for target in targets], ["picoui_demo"])
            self.assertIn("target-id", by_id)

    def test_load_codemodel_retries_when_target_reply_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            reply_dir = pathlib.Path(tmp)
            (reply_dir / "index-1.json").write_text(
                json.dumps(
                    {
                        "reply": {
                            "codemodel-v2": {
                                "jsonFile": "codemodel-v2-stale.json",
                                "kind": "codemodel",
                                "version": {"major": 2, "minor": 9},
                            }
                        }
                    }
                ),
                encoding="utf-8",
            )
            (reply_dir / "codemodel-v2-stale.json").write_text(
                json.dumps(
                    {
                        "configurations": [
                            {
                                "targets": [
                                    {
                                        "id": "target-id",
                                        "name": "picoui_demo",
                                        "jsonFile": "target-missing.json",
                                    }
                                ]
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            def fake_configure() -> None:
                (reply_dir / "index-2.json").write_text(
                    json.dumps(
                        {
                            "reply": {
                                "codemodel-v2": {
                                    "jsonFile": "codemodel-v2-fresh.json",
                                    "kind": "codemodel",
                                    "version": {"major": 2, "minor": 9},
                                }
                            }
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "codemodel-v2-fresh.json").write_text(
                    json.dumps(
                        {
                            "configurations": [
                                {
                                    "targets": [
                                        {
                                            "id": "target-id",
                                            "name": "picoui_demo",
                                            "jsonFile": "target-fresh.json",
                                        }
                                    ]
                                }
                            ]
                        }
                    ),
                    encoding="utf-8",
                )
                (reply_dir / "target-fresh.json").write_text(
                    json.dumps({"dependencies": [], "sources": []}),
                    encoding="utf-8",
                )

            with mock.patch.object(MODULE, "REPLY_DIR", reply_dir):
                with mock.patch.object(MODULE, "_configure_file_api_build", side_effect=fake_configure):
                    targets, by_id = MODULE._load_codemodel()

            self.assertEqual([target["jsonFile"] for target in targets], ["target-fresh.json"])
            self.assertIn("target-id", by_id)

    def test_load_codemodel_waits_for_target_reply_to_appear(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            reply_dir = pathlib.Path(tmp)
            (reply_dir / "index-1.json").write_text(
                json.dumps(
                    {
                        "reply": {
                            "codemodel-v2": {
                                "jsonFile": "codemodel-v2-fresh.json",
                                "kind": "codemodel",
                                "version": {"major": 2, "minor": 9},
                            }
                        }
                    }
                ),
                encoding="utf-8",
            )
            (reply_dir / "codemodel-v2-fresh.json").write_text(
                json.dumps(
                    {
                        "configurations": [
                            {
                                "targets": [
                                    {
                                        "id": "target-id",
                                        "name": "picoui_demo",
                                        "jsonFile": "target-delayed.json",
                                    }
                                ]
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            def write_target_later() -> None:
                time.sleep(0.05)
                (reply_dir / "target-delayed.json").write_text(
                    json.dumps({"dependencies": [], "sources": []}),
                    encoding="utf-8",
                )

            writer = threading.Thread(target=write_target_later)
            writer.start()
            try:
                with mock.patch.object(MODULE, "REPLY_DIR", reply_dir):
                    targets, by_id = MODULE._load_codemodel(retry=False)
            finally:
                writer.join()

            self.assertEqual([target["jsonFile"] for target in targets], ["target-delayed.json"])
            self.assertIn("target-id", by_id)


if __name__ == "__main__":
    unittest.main()
