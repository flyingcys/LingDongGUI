from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
REPO_ROOT = SDL_ROOT.parents[1]
DEFAULT_CONFIG = SDL_ROOT / "user" / "ldConfig.h"
CONFIG = Path(os.environ.get("LDGUI_TEST_LDCONFIG", DEFAULT_CONFIG))
LEGACY_WIDGET_SOURCE = REPO_ROOT / "examples" / "common" / "demo" / "widget" / "uiWidgetLegacy.c"
SWITCH_HEADER = REPO_ROOT / "src" / "gui" / "ldSwitch.h"

REQUIRED_NAMED_INTS = {
    "UI_WIDGET_LEGACY_SWITCH_ID": 30,
    "UI_WIDGET_LEGACY_SWITCH_LABEL_ID": 31,
    "UI_WIDGET_LEGACY_SWITCH_X": 300,
    "UI_WIDGET_LEGACY_SWITCH_Y": 226,
    "UI_WIDGET_LEGACY_SWITCH_WIDTH": 48,
    "UI_WIDGET_LEGACY_SWITCH_HEIGHT": 24,
    "UI_WIDGET_LEGACY_SWITCH_LABEL_X": 356,
    "UI_WIDGET_LEGACY_SWITCH_LABEL_Y": 218,
    "UI_WIDGET_LEGACY_SWITCH_LABEL_WIDTH": 60,
    "UI_WIDGET_LEGACY_SWITCH_LABEL_HEIGHT": 40,
}

EXPECTED_EXPANSIONS = {
    "LD_CFG_SCREEN_WIDTH": "(1024)",
    "LD_CFG_SCREEN_HEIGHT": "(600)",
    "LD_CFG_PFB_WIDTH": "((1024))",
    "LD_CFG_PFB_HEIGHT": "((600)/10)",
    "LD_DEMO_GUI_FUNC": "uiWidgetLegacyFunc",
}

PREPROCESS_TEMPLATE = """
#include "{header_name}"
#define LDGUI_STR2(x) #x
#define LDGUI_STR(x) LDGUI_STR2(x)
const char *ldgui_screen_width = LDGUI_STR(LD_CFG_SCREEN_WIDTH);
const char *ldgui_screen_height = LDGUI_STR(LD_CFG_SCREEN_HEIGHT);
const char *ldgui_pfb_width = LDGUI_STR(LD_CFG_PFB_WIDTH);
const char *ldgui_pfb_height = LDGUI_STR(LD_CFG_PFB_HEIGHT);
const char *ldgui_entry = LDGUI_STR(LD_DEMO_GUI_FUNC);
""".strip()

OUTPUT_PATTERNS = {
    "LD_CFG_SCREEN_WIDTH": re.compile(r'^const char \*ldgui_screen_width = "(.*)";$'),
    "LD_CFG_SCREEN_HEIGHT": re.compile(r'^const char \*ldgui_screen_height = "(.*)";$'),
    "LD_CFG_PFB_WIDTH": re.compile(r'^const char \*ldgui_pfb_width = "(.*)";$'),
    "LD_CFG_PFB_HEIGHT": re.compile(r'^const char \*ldgui_pfb_height = "(.*)";$'),
    "LD_DEMO_GUI_FUNC": re.compile(r'^const char \*ldgui_entry = "(.*)";$'),
}

IDENTIFIER_OR_INT = r"[A-Z0-9_]+|\d+"


def _preprocess_config(config: Path) -> str:
    preprocess_input = PREPROCESS_TEMPLATE.format(header_name=config.name)
    result = subprocess.run(
        [
            "cc",
            "-E",
            "-P",
            "-x",
            "c",
            "-DUSE_DEMO=0",
            f"-I{config.parent}",
            '-D__ARM_2D_USER_APP_CFG_H__="ldConfig.h"',
            "-",
        ],
        input=preprocess_input,
        text=True,
        capture_output=True,
        check=True,
    )
    return result.stdout


def _extract_expansions(preprocessed: str) -> dict[str, str]:
    expansions: dict[str, str] = {}
    for line in preprocessed.splitlines():
        stripped = line.strip()
        for macro_name, pattern in OUTPUT_PATTERNS.items():
            match = pattern.match(stripped)
            if match:
                expansions[macro_name] = match.group(1)
                break
    return expansions


def _extract_named_int(source: str, name: str) -> int | None:
    match = re.search(rf"\b{name}\s*=\s*(\d+)\b", source)
    if match:
        return int(match.group(1))
    return None


def _resolve_value(token: str, named_ints: dict[str, int]) -> int:
    if token.isdigit():
        return int(token)
    return named_ints[token]


def _require_call(source: str, pattern: str, error_message: str) -> re.Match[str]:
    match = re.search(pattern, source)
    assert match, error_message
    return match


def test_use_demo_0_legacy_widget_preprocessed_expansions() -> None:
    preprocessed = _preprocess_config(CONFIG)
    expansions = _extract_expansions(preprocessed)

    missing = sorted(set(EXPECTED_EXPANSIONS) - set(expansions))
    assert not missing, f"预处理输出缺少宏结果: {missing}\n{preprocessed}"

    for macro_name, expected in EXPECTED_EXPANSIONS.items():
        actual = expansions[macro_name]
        assert (
            actual == expected
        ), f"{macro_name} 实际展开为 {actual}，预期 {expected}"


def test_use_demo_0_legacy_widget_contains_switch_demo() -> None:
    source = LEGACY_WIDGET_SOURCE.read_text(encoding="utf-8")
    switch_header = SWITCH_HEADER.read_text(encoding="utf-8")
    named_ints = {
        name: _extract_named_int(source, name) for name in REQUIRED_NAMED_INTS
    }
    missing_named_ints = sorted(
        name for name, value in named_ints.items() if value is None
    )
    assert not missing_named_ints, (
        "demo0 legacy 页面仍缺少 switch/label 具名常量: "
        f"{missing_named_ints}"
    )
    named_ints = {name: value for name, value in named_ints.items() if value is not None}

    assert "ldSwitchInit(" in source, "demo0 legacy 页面还没有接入 switch"
    assert "ldSwitchNavigate(" in switch_header, "ldSwitch 头文件缺少键盘/导航 API `ldSwitchNavigate`"
    assert (
        "ldSwitchSetChecked(obj, false);" in source
    ), "demo0 legacy 页面缺少显式默认关闭状态"
    assert (
        "connect(UI_WIDGET_LEGACY_SWITCH_ID, SIGNAL_VALUE_CHANGED, uiWidgetLegacySwitchValueChanged)"
        in source
    ), "demo0 legacy 页面缺少 switch 值变化回调连接"
    assert (
        "ldBaseGetWidget(ptScene->ptNodeRoot, UI_WIDGET_LEGACY_SWITCH_ID)" in source
    ), "demo0 legacy 页面缺少 switch 具名 widget id 查询"
    assert (
        "ldBaseGetWidget(ptScene->ptNodeRoot, UI_WIDGET_LEGACY_SWITCH_LABEL_ID)" in source
    ), "demo0 legacy 页面缺少状态 label 具名 widget id 查询"
    assert "KEY_NUM_ENTER" in source, "demo0 legacy 页面缺少 KEY_NUM_ENTER 键盘入口"
    assert "ldSwitchNavigate(" in source, "demo0 legacy 页面缺少 switch 键盘/导航调用"
    assert "uiWidgetLegacyNavigateSelectedSwitch" in source, "demo0 legacy 页面缺少 switch 键盘导航 helper"
    assert "uiWidgetLegacyNavigateSelectedSwitch(ptScene, ptSwitch, NAV_UP)" in source, "demo0 legacy 页面缺少 switch NAV_UP 键盘接线"
    assert "uiWidgetLegacyNavigateSelectedSwitch(ptScene, ptSwitch, NAV_DOWN)" in source, "demo0 legacy 页面缺少 switch NAV_DOWN 键盘接线"
    assert "uiWidgetLegacyNavigateSelectedSwitch(ptScene, ptSwitch, NAV_LEFT)" in source, "demo0 legacy 页面缺少 switch NAV_LEFT 键盘接线"
    assert "uiWidgetLegacyNavigateSelectedSwitch(ptScene, ptSwitch, NAV_RIGHT)" in source, "demo0 legacy 页面缺少 switch NAV_RIGHT 键盘接线"
    assert '"ON"' in source, 'demo0 legacy 页面缺少 "ON" 状态文案'
    assert '"OFF"' in source, 'demo0 legacy 页面缺少 "OFF" 状态文案'

    for name, expected in REQUIRED_NAMED_INTS.items():
        actual = named_ints[name]
        assert actual == expected, f"{name} 实际值为 {actual}，预期 {expected}"

    switch_match = _require_call(
        source,
        rf"ldSwitchInit\(\s*UI_WIDGET_LEGACY_SWITCH_ID,\s*0,\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT})\s*\)",
        "demo0 legacy 页面缺少可解析的 switch 坐标",
    )
    label_match = _require_call(
        source,
        rf"ldLabelInit\(\s*UI_WIDGET_LEGACY_SWITCH_LABEL_ID,\s*0,\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT}),\s*({IDENTIFIER_OR_INT}),\s*FONT_ARIAL_16_A8\s*\)",
        "demo0 legacy 页面缺少可解析的状态 label 坐标",
    )

    switch_x, switch_y, switch_w, switch_h = (
        _resolve_value(token, named_ints) for token in switch_match.groups()
    )
    label_x, label_y, label_w, label_h = (
        _resolve_value(token, named_ints) for token in label_match.groups()
    )

    text_x, text_y, text_w, text_h = 300, 10, 150, 200

    def overlaps(box: tuple[int, int, int, int], text_box: tuple[int, int, int, int]) -> bool:
        box_x, box_y, box_w, box_h = box
        txt_x, txt_y, txt_w, txt_h = text_box
        return not (
            box_x + box_w <= txt_x
            or txt_x + txt_w <= box_x
            or box_y + box_h <= txt_y
            or txt_y + txt_h <= box_y
        )

    text_box = (text_x, text_y, text_w, text_h)
    assert not overlaps((switch_x, switch_y, switch_w, switch_h), text_box), (
        "demo0 switch 仍与 ldTextInit(9, 0, 300, 10, 150, 200, ...) 区域重叠"
    )
    assert not overlaps((label_x, label_y, label_w, label_h), text_box), (
        "demo0 switch 状态 label 仍与 ldTextInit(9, 0, 300, 10, 150, 200, ...) 区域重叠"
    )


def main() -> int:
    tests = [
        test_use_demo_0_legacy_widget_preprocessed_expansions,
        test_use_demo_0_legacy_widget_contains_switch_demo,
    ]
    failures = 0

    for test in tests:
        try:
            test()
        except AssertionError as exc:
            failures += 1
            print(f"[FAIL] {test.__name__}: {exc}", file=sys.stderr)
        except Exception as exc:  # pragma: no cover - direct execution path
            failures += 1
            print(f"[ERROR] {test.__name__}: {exc}", file=sys.stderr)
        else:
            print(f"[PASS] {test.__name__}")

    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
