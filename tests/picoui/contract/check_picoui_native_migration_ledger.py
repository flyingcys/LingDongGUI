import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
LEDGER = ROOT / "tests/picoui/contract/picoui_native_migration_ledger.json"
REQUIRED_PHASES = {"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"}
VALID_STATUS = {"not_started", "in_progress", "covered", "non_user_capability"}


def main() -> int:
    assert LEDGER.is_file(), f"missing {LEDGER.relative_to(ROOT)}"
    data = json.loads(LEDGER.read_text(encoding="utf-8"))
    entries = data.get("entries")
    assert isinstance(entries, list) and entries, "entries must be a non-empty list"
    phases = {entry.get("phase") for entry in entries}
    missing = sorted(REQUIRED_PHASES - phases)
    assert not missing, f"ledger missing phase coverage: {missing}"
    for entry in entries:
        assert entry.get("status") in VALID_STATUS, f"{entry.get('id')} invalid status"
        assert isinstance(entry.get("capability"), str) and entry["capability"], "missing capability"
        assert isinstance(entry.get("evidence"), list), f"{entry.get('id')} evidence must be list"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
