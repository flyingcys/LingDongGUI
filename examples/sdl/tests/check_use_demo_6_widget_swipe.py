from __future__ import annotations

import re
import subprocess
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
CONFIG = SDL_ROOT / "user" / "ldConfig.h"

EXPECTED_EXPANSIONS = {
    "LD_DEMO_GUI_INCLUDE": '"uiWidgetSwipe.h"',
    "LD_DEMO_GUI_FUNC": "uiWidgetSwipeFunc",
    "LD_CFG_SCREEN_WIDTH": "(480)",
    "LD_CFG_SCREEN_HEIGHT": "(272)",
}

PREPROCESS_SOURCE = """
#include "ldConfig.h"
#define LDGUI_STR2(x) #x
#define LDGUI_STR(x) LDGUI_STR2(x)
const char *ldgui_include = LD_DEMO_GUI_INCLUDE;
const char *ldgui_entry = LDGUI_STR(LD_DEMO_GUI_FUNC);
const char *ldgui_width = LDGUI_STR(LD_CFG_SCREEN_WIDTH);
const char *ldgui_height = LDGUI_STR(LD_CFG_SCREEN_HEIGHT);
""".strip()

OUTPUT_PATTERNS = {
    "LD_DEMO_GUI_INCLUDE": re.compile(r'^const char \*ldgui_include = (".*");$'),
    "LD_DEMO_GUI_FUNC": re.compile(r'^const char \*ldgui_entry = "(.*)";$'),
    "LD_CFG_SCREEN_WIDTH": re.compile(r'^const char \*ldgui_width = "(.*)";$'),
    "LD_CFG_SCREEN_HEIGHT": re.compile(r'^const char \*ldgui_height = "(.*)";$'),
}


def _preprocess() -> str:
    result = subprocess.run(
        [
            "cc",
            "-E",
            "-P",
            "-x",
            "c",
            "-DUSE_DEMO=6",
            f"-I{CONFIG.parent}",
            '-D__ARM_2D_USER_APP_CFG_H__="ldConfig.h"',
            "-",
        ],
        input=PREPROCESS_SOURCE,
        text=True,
        capture_output=True,
        check=True,
    )
    return result.stdout


def _extract(preprocessed: str) -> dict[str, str]:
    expansions: dict[str, str] = {}
    for line in preprocessed.splitlines():
        stripped = line.strip()
        for macro_name, pattern in OUTPUT_PATTERNS.items():
            match = pattern.match(stripped)
            if match:
                expansions[macro_name] = match.group(1)
                break
    return expansions


def test_use_demo_6_widget_swipe_preprocessed_expansions() -> None:
    preprocessed = _preprocess()
    expansions = _extract(preprocessed)

    missing = sorted(set(EXPECTED_EXPANSIONS) - set(expansions))
    assert not missing, f"预处理输出缺少宏结果: {missing}\n{preprocessed}"

    for macro_name, expected in EXPECTED_EXPANSIONS.items():
        actual = expansions[macro_name]
        assert actual == expected, f"{macro_name} 实际展开为 {actual}，预期 {expected}"
