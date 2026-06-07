import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CONTRACT = ROOT / "tests/picoui/contract/picoui_v1_1_boundary_contract.json"
TEST_SOURCE = ROOT / "tests/picoui/unit/test_picoui_v1_1_public_headers.c"
PORT_TEST_SOURCE = ROOT / "tests/picoui/unit/test_picoui_v1_1_port_sdl.c"
PORT_SOURCE = ROOT / "picoui/port/sdl/sdl_v1_1.c"
STAGE_DOC = ROOT / "docs/picoui-serial/v1.1/01-C1-阶段记录.md"
INDEX_DOC = ROOT / "docs/picoui-serial/v1.1/线计划索引.md"
REQUIRED_DELETE_APIS = {
    "picoui_app_create",
    "picoui_app_run",
    "picoui_app_run_background",
    "picoui_app_set_window",
    "picoui_app_set_background",
    "picoui_app_switch_window",
    "picoui_app_switch_background",
    "picoui_app_timer_create",
    "picoui_app_timer_start",
    "picoui_app_timer_stop",
    "picoui_app_timer_is_running",
    "picoui_app_timer_destroy",
    "picoui_app_destroy",
}


def main() -> int:
    assert CONTRACT.is_file(), f"missing {CONTRACT.relative_to(ROOT)}"
    data = json.loads(CONTRACT.read_text(encoding="utf-8"))
    test_text = TEST_SOURCE.read_text(encoding="utf-8")
    port_test_text = PORT_TEST_SOURCE.read_text(encoding="utf-8")
    port_source_text = PORT_SOURCE.read_text(encoding="utf-8")
    stage_doc = STAGE_DOC.read_text(encoding="utf-8")
    index_doc = INDEX_DOC.read_text(encoding="utf-8")

    assert data.get("version") == "v1.1"
    assert set(data.get("delete_public_apis", [])) == REQUIRED_DELETE_APIS
    assert data.get("remove_directories") == [
        "picoui/src/native",
        "picoui/src/backend",
    ]
    assert data.get("forbid_runtime_bones") == [
        "ldBase_t",
        "ld_scene_t",
        "ldMsg",
        "SIGNAL_",
        "arm_2d_control_node_t",
    ]
    for header in data.get("minimal_public_headers", []):
        assert f'#include "{header}"' in test_text
    for header in data.get("minimal_port_headers", []):
        assert f'#include "{header}"' in port_test_text
    for header in data.get("forbid_v1_1_test_headers", []):
        assert f'#include "{header}"' not in test_text
        assert f'#include "{header}"' not in port_test_text
    for claim in data.get("minimal_port_claims", []):
        assert claim in port_source_text or claim in port_test_text
    assert "- `C1` 已完成" not in stage_doc
    assert "- `C1 gate`：PASS" not in stage_doc
    assert "| `C1` | `docs/picoui-serial/v1.1/02-C1-core最小骨架计划.md` | core 最小骨架、删除边界和 build 红线 | 已完成 |" not in index_doc
    assert "当前入口仍保持默认 `picoui = v1.0-native`" in stage_doc
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
