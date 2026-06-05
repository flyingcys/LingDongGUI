import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
INVENTORY = ROOT / "tests/picoui/contract/picoui_ldgui_dependency_inventory.json"
REQUIRED_GROUPS = {"picoui_core", "picoui_backend_ldgui", "picoui_backend_ldgui_runtime", "picoui_demo_targets"}


def main() -> int:
    assert INVENTORY.is_file(), f"missing {INVENTORY.relative_to(ROOT)}"
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    groups = {row.get("name") for row in data.get("targets", [])}
    missing = sorted(REQUIRED_GROUPS - groups)
    assert not missing, f"missing dependency target groups: {missing}"
    for row in data["targets"]:
        assert isinstance(row.get("currently_links_ldgui"), bool), f"{row.get('name')} missing bool"
        assert row.get("v1_target") in {"remove_dependency", "legacy_opt_in", "native_only"}, (
            f"{row.get('name')} invalid v1_target"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
