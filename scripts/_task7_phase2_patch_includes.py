#!/usr/bin/env python3
"""Rewrite includes and strip public typo declarations for Task 7."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

INCLUDE_REWRITES = {
    '#include "core/app.h"': '#include "internal/app_legacy.h"',
    '#include "core/widget.h"': '#include "internal/widget_legacy.h"',
    '#include "core/native.h"': '#include "extensions/ldgui_native.h"',
    '#include <core/app.h>': '#include "internal/app_legacy.h"',
    '#include <core/widget.h>': '#include "internal/widget_legacy.h"',
    '#include <core/native.h>': '#include "extensions/ldgui_native.h"',
}

# Paths that must keep using private includes (source + tests that already include them).
# Public widget headers currently including native must stop doing so.
PUBLIC_STRIP_NATIVE = {
    ROOT / "tinyui/include/widgets/table.h",
    ROOT / "tinyui/include/widgets/keyboard.h",
}


def rewrite_includes(path: Path, text: str) -> str:
    for old, new in INCLUDE_REWRITES.items():
        text = text.replace(old, new)
    return text


def strip_typo_decls(path: Path, text: str) -> str:
    if path.name == "button.h":
        text = re.sub(
            r"\nint tinyui_button_set_press\(tinyui_obj_t \*button, int pressed\);\n",
            "\n",
            text,
        )
        text = re.sub(
            r"\nint tinyui_button_get_press\(tinyui_obj_t \*button, int \*pressed\);\n",
            "\n",
            text,
        )
    if path.name == "table.h":
        text = re.sub(
            r"\nint tinyui_tabel_show_keyboard\(tinyui_obj_t \*table\);\n",
            "\n",
            text,
        )
    if path.name == "qrcode.h":
        text = re.sub(
            r"\nint tinyui_q_r_code_set_text\(tinyui_obj_t \*qrcode, const char \*text\);\n",
            "\n",
            text,
        )
    return text


def main() -> None:
    changed = 0
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in {"build", ".git", "__pycache__", "third_party", "artifacts"} for part in path.parts):
            continue
        if path.suffix not in {".c", ".h", ".cpp", ".hpp", ".cc"}:
            continue
        # Skip the checker itself and plan docs (not code).
        if "scripts/_task7" in str(path):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        original = text
        text = rewrite_includes(path, text)
        if path in PUBLIC_STRIP_NATIVE or path.parent.name == "widgets" and path.suffix == ".h":
            # Public widget headers must not pull native/extensions.
            text = text.replace('#include "extensions/ldgui_native.h"\n', "")
            text = text.replace('#include "core/native.h"\n', "")
        text = strip_typo_decls(path, text)
        if text != original:
            path.write_text(text, encoding="utf-8")
            changed += 1
            print(f"patched {path.relative_to(ROOT)}")
    print(f"changed_files={changed}")


if __name__ == "__main__":
    main()
