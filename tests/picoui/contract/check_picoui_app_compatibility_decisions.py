import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
DECISIONS = ROOT / "tests/picoui/contract/picoui_app_compatibility_decisions.json"
REQUIRED = {
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
VALID_DECISIONS = {"delete", "compat_wrapper", "replace"}


def main() -> int:
    assert DECISIONS.is_file(), f"missing {DECISIONS.relative_to(ROOT)}"
    data = json.loads(DECISIONS.read_text(encoding="utf-8"))
    rows = data.get("picoui_app_apis")
    assert isinstance(rows, list), "picoui_app_apis must be a list"

    by_name = {row.get("name"): row for row in rows}
    missing = sorted(REQUIRED - set(by_name))
    assert not missing, f"missing app API decisions: {missing}"

    for name in sorted(REQUIRED):
        row = by_name[name]
        decision = row.get("decision")
        replacement = row.get("replacement")
        rationale = row.get("rationale")
        assert decision in VALID_DECISIONS, f"{name} has invalid decision {decision!r}"
        assert isinstance(rationale, str) and rationale.strip(), f"{name} missing rationale"
        if decision == "replace":
            assert isinstance(replacement, str) and replacement.startswith("picoui_"), (
                f"{name} replacement must name a picoui_* API"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
