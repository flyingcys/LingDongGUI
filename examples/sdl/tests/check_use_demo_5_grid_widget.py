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
UI_LAYOUT_SOURCE = REPO_ROOT / "examples" / "common" / "demo" / "layout" / "uiLayout.c"

EXPECTED_EXPANSIONS = {
    "LD_DEMO_GUI_FUNC": "uiGridFunc",
}

PREPROCESS_TEMPLATE = """
#include "{header_name}"
#define LDGUI_STR2(x) #x
#define LDGUI_STR(x) LDGUI_STR2(x)
const char *ldgui_entry = LDGUI_STR(LD_DEMO_GUI_FUNC);
""".strip()

OUTPUT_PATTERNS = {
    "LD_DEMO_GUI_FUNC": re.compile(r'^const char \*ldgui_entry = "(.*)";$'),
}

EXPECTED_ROOT_CALL = (
    "ldWindowInit(ID_GRID_BG, ID_GRID_BG, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT)"
)


def _preprocess_config(config: Path) -> str:
    preprocess_input = PREPROCESS_TEMPLATE.format(header_name=config.name)
    result = subprocess.run(
        [
            "cc",
            "-E",
            "-P",
            "-x",
            "c",
            "-DUSE_DEMO=5",
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


def _extract_function_body(source: str, function_name: str) -> str:
    match = re.search(
        rf"void\s+{function_name}\s*\([^)]*\)\s*\{{(?P<body>.*?)\n\}}",
        source,
        re.DOTALL,
    )
    assert match, f"找不到函数 {function_name}"
    return match.group("body")


def _first_call(body: str, function_name: str) -> str | None:
    match = re.search(rf"{function_name}\s*\((.*?)\);", body, re.DOTALL)
    if not match:
        return None
    args = " ".join(match.group(1).split())
    return f"{function_name}({args})"


def test_use_demo_5_routes_to_grid_func() -> None:
    preprocessed = _preprocess_config(CONFIG)
    expansions = _extract_expansions(preprocessed)

    missing = sorted(set(EXPECTED_EXPANSIONS) - set(expansions))
    assert not missing, f"预处理输出缺少宏结果: {missing}\n{preprocessed}"

    for macro_name, expected in EXPECTED_EXPANSIONS.items():
        actual = expansions[macro_name]
        assert (
            actual == expected
        ), f"{macro_name} 实际展开为 {actual}，预期 {expected}"


def test_use_demo_5_grid_root_contract() -> None:
    source = UI_LAYOUT_SOURCE.read_text(encoding="utf-8")

    id_match = re.search(r"\bID_GRID_BG\s*=\s*(\d+)\b", source)
    assert id_match, "uiLayout.c 缺少 ID_GRID_BG 定义"
    assert id_match.group(1) == "0", (
        f"ID_GRID_BG 实际值为 {id_match.group(1)}，预期 0"
    )

    body = _extract_function_body(source, "uiGridInit")
    first_window_init = _first_call(body, "ldWindowInit")
    assert first_window_init is not None, "uiGridInit 缺少 ldWindowInit root 初始化"
    assert first_window_init == EXPECTED_ROOT_CALL, (
        "uiGridInit 第一处 ldWindowInit 不是 root contract："
        f" 实际 `{first_window_init}`，预期 `{EXPECTED_ROOT_CALL}`"
    )


def main() -> int:
    tests = [
        test_use_demo_5_routes_to_grid_func,
        test_use_demo_5_grid_root_contract,
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
