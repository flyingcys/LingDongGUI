from __future__ import annotations

import sys
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
REPO_ROOT = SDL_ROOT.parents[1]
BASE_SOURCE = REPO_ROOT / "src" / "gui" / "ldBase.c"


def test_ldbase_routes_selected_switch_navigation() -> None:
    source = BASE_SOURCE.read_text(encoding="utf-8")

    assert "widgetTypeSwitch" in source, "ldBaseFocusNavigate 还没有识别 switch 焦点类型"
    assert "ldSwitchCanNavigate(" in source, "ldBaseFocusNavigate 还没有先判断 switch 是否应该消费导航"
    assert "ldSwitchNavigate(" in source, "ldBaseFocusNavigate 还没有把导航委托给 ldSwitchNavigate"
    assert "case NAV_ENTER:" in source, "ldBaseFocusNavigate 缺少 NAV_ENTER 分支"
    assert "case NAV_UP:" in source, "ldBaseFocusNavigate 缺少 NAV_UP 分支"
    assert "case NAV_DOWN:" in source, "ldBaseFocusNavigate 缺少 NAV_DOWN 分支"
    assert "case NAV_LEFT:" in source, "ldBaseFocusNavigate 缺少 NAV_LEFT 分支"
    assert "case NAV_RIGHT:" in source, "ldBaseFocusNavigate 缺少 NAV_RIGHT 分支"


def main() -> int:
    tests = [test_ldbase_routes_selected_switch_navigation]
    failures = 0

    for test in tests:
        try:
            test()
        except AssertionError as exc:
            failures += 1
            print(f"[FAIL] {test.__name__}: {exc}", file=sys.stderr)
        except Exception as exc:
            failures += 1
            print(f"[ERROR] {test.__name__}: {exc}", file=sys.stderr)
        else:
            print(f"[PASS] {test.__name__}")

    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
