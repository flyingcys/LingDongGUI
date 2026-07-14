#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

moves = {
    ROOT / "tinyui/include/core/app.h": ROOT / "tinyui/include/internal/app_legacy.h",
    ROOT / "tinyui/include/core/widget.h": ROOT / "tinyui/include/internal/widget_legacy.h",
    ROOT / "tinyui/include/core/native.h": ROOT / "tinyui/include/extensions/ldgui_native.h",
}

for src, dst in moves.items():
    dst.parent.mkdir(parents=True, exist_ok=True)
    if not src.exists():
        print(f"skip missing {src}")
        continue
    text = src.read_text(encoding="utf-8")
    if src.name == "app.h":
        text = text.replace("TINYUI_APP_H", "TINYUI_INTERNAL_APP_LEGACY_H")
    elif src.name == "widget.h":
        text = text.replace("TINYUI_WIDGET_H", "TINYUI_INTERNAL_WIDGET_LEGACY_H")
        text = text.replace('#include "core/native.h"', '#include "extensions/ldgui_native.h"')
    elif src.name == "native.h":
        text = text.replace("TINYUI_NATIVE_H", "TINYUI_EXTENSIONS_LDGUI_NATIVE_H")
    dst.write_text(text, encoding="utf-8")
    src.unlink()
    print(f"moved {src.relative_to(ROOT)} -> {dst.relative_to(ROOT)}")

old_c = ROOT / "tinyui/src/widgets/scroll_selecter.c"
new_c = ROOT / "tinyui/src/widgets/scroll_selector.c"
if old_c.exists() and not new_c.exists():
    old_c.rename(new_c)
    print("renamed scroll_selecter.c -> scroll_selector.c")
elif new_c.exists():
    print("scroll_selector.c already present")
else:
    print("scroll_selecter.c missing")
