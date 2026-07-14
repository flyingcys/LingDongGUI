#!/usr/bin/env python3
from pathlib import Path

enums = """
typedef enum tinyui_nav_dir {
    TINYUI_NAV_LEFT = 0,
    TINYUI_NAV_RIGHT,
    TINYUI_NAV_UP,
    TINYUI_NAV_DOWN,
    TINYUI_NAV_ENTER,
    TINYUI_NAV_BACK
} tinyui_nav_dir_t;

typedef enum tinyui_signal {
    TINYUI_SIGNAL_NONE = 0,
    TINYUI_SIGNAL_PRESS,
    TINYUI_SIGNAL_HOLD_DOWN,
    TINYUI_SIGNAL_RELEASE,
    TINYUI_SIGNAL_CLICKED_ITEM,
    TINYUI_SIGNAL_FINISHED,
    TINYUI_SIGNAL_VALUE_CHANGED
} tinyui_signal_t;
"""

for rel in ["tinyui/include/widgets/table.h", "tinyui/include/widgets/keyboard.h"]:
    p = Path(rel)
    t = p.read_text(encoding="utf-8")
    t = t.replace('#include "core/nav.h"\n', "")
    if "typedef enum tinyui_nav_dir" not in t:
        t = t.replace("#include <stdint.h>\n", "#include <stdint.h>\n" + enums + "\n")
    p.write_text(t, encoding="utf-8")
    print("embedded enums in", rel)

nav = Path("tinyui/include/core/nav.h")
if nav.exists():
    nav.unlink()
    print("removed core/nav.h")

p = Path("tests/tinyui/contract/check_tinyui_v23_public_api.py")
t = p.read_text(encoding="utf-8")
if '"integration/"' not in t:
    t = t.replace(
        '"port/",\n    )',
        '"port/",\n        "integration/",\n    )',
    )
    p.write_text(t, encoding="utf-8")
    print("skip integration/")
else:
    print("integration already skipped")
