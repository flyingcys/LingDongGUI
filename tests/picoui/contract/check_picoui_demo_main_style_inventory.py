import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DEMO_DIR = ROOT / "picoui/demo"
INVENTORY = ROOT / "tests/picoui/contract/picoui_demo_main_style_inventory.json"


def main() -> int:
    assert INVENTORY.is_file(), f"missing {INVENTORY.relative_to(ROOT)}"
    data = json.loads(INVENTORY.read_text(encoding="utf-8"))
    rows = data.get("demos")
    assert isinstance(rows, list), "demos must be a list"
    by_name = {row.get("name"): row for row in rows}
    actual = sorted(path.parent.name for path in DEMO_DIR.glob("*/main.c"))
    missing = [name for name in actual if name not in by_name]
    assert not missing, f"missing demo inventory rows: {missing}"
    for name in actual:
        row = by_name[name]
        style = row.get("main_style")
        assert style in {"pre_v1_app_run", "v1_lvgl_like"}, f"{name} invalid main_style={style!r}"
        uses = row.get("uses_picoui_app")
        assert isinstance(uses, bool), f"{name} uses_picoui_app must be bool"
        migration = row.get("migration_phase")
        assert isinstance(migration, str) and migration.startswith("P"), f"{name} invalid migration_phase"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
