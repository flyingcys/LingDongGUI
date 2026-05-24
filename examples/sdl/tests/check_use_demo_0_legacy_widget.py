from __future__ import annotations

import os
import re
import subprocess
from pathlib import Path


TEST_FILE = Path(__file__).resolve()
SDL_ROOT = TEST_FILE.parents[1]
DEFAULT_CONFIG = SDL_ROOT / "user" / "ldConfig.h"
CONFIG = Path(os.environ.get("LDGUI_TEST_LDCONFIG", DEFAULT_CONFIG))

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
