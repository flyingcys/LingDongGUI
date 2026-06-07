import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CONTRACT = ROOT / "tests" / "picoui" / "contract" / "picoui_v1_1_boundary_contract.json"
APP_HEADER = ROOT / "picoui" / "include" / "picoui" / "app.h"
UMBRELLA_HEADER = ROOT / "picoui" / "include" / "picoui" / "picoui.h"
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
APP_SYMBOL_RE = re.compile(r"\b(picoui_app_[A-Za-z0-9_]+)\b(?=\s*\()")


def main() -> int:
    assert CONTRACT.is_file(), f"missing {CONTRACT.relative_to(ROOT)}"
    assert APP_HEADER.is_file(), f"missing {APP_HEADER.relative_to(ROOT)}"
    assert UMBRELLA_HEADER.is_file(), f"missing {UMBRELLA_HEADER.relative_to(ROOT)}"
    data = json.loads(CONTRACT.read_text(encoding="utf-8"))
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
    exported = set(APP_SYMBOL_RE.findall(APP_HEADER.read_text(encoding="utf-8")))
    assert exported == REQUIRED_DELETE_APIS, (
        f"app.h exports drifted from v1.1 delete inventory: {sorted(exported ^ REQUIRED_DELETE_APIS)}"
    )
    umbrella = UMBRELLA_HEADER.read_text(encoding="utf-8")
    assert '#include "picoui/app.h"' in umbrella, "picoui.h no longer exposes app.h"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
