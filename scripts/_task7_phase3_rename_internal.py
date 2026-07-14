#!/usr/bin/env python3
"""Rename production-side forbidden symbols to tinyui_runtime_internal_*.

Only touches tinyui/src/** production sources and private headers that
implement/call those symbols. Public/demo/test call sites keep the old
names and resolve them via v22_demo_bridge.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "tinyui" / "src"

# Exact symbols that must leave production TUs (definitions + call sites in src).
# Types like struct tinyui_app are intentionally not renamed.
EXACT_RENAMES: dict[str, str] = {}

# Collect function-like identifiers from current archive forbidden set by scanning src.
IDENT_RE = re.compile(r"\btinyui_(?:app_|widget_)[A-Za-z0-9_]*\b")
INIT_RE = re.compile(r"\btinyui_[A-Za-z0-9_]+_init\b")
TYPO_EXACT = {
    "tinyui_timer_handler",
    "tinyui_button_set_press",
    "tinyui_button_get_press",
    "tinyui_tabel_show_keyboard",
    "tinyui_q_r_code_init",
    "tinyui_q_r_code_set_text",
    "tinyui_widget_set_style_class",
}

SKIP_NAMES = {
    "tinyui_init",
    # type/callback tags that are not function symbols; still match prefix.
    "tinyui_app",
    "tinyui_app_timer",
    "tinyui_app_timer_cb_t",
    "tinyui_widget",
    "tinyui_widget_type",
}


def internal_name(name: str) -> str:
    if name.startswith("tinyui_app_"):
        return "tinyui_runtime_internal_" + name[len("tinyui_") :]
    if name.startswith("tinyui_widget_"):
        return "tinyui_runtime_internal_" + name[len("tinyui_") :]
    if name == "tinyui_timer_handler":
        return "tinyui_runtime_internal_timer_handler"
    if name == "tinyui_widget_set_style_class":
        return "tinyui_runtime_internal_widget_set_style_class"
    if name.endswith("_init") and name != "tinyui_init":
        # widget init helpers become static-only later; rename if left non-static
        return "tinyui_runtime_internal_" + name[len("tinyui_") :]
    if name in TYPO_EXACT:
        return "tinyui_runtime_internal_" + name[len("tinyui_") :]
    return name


def should_rename(name: str) -> bool:
    if name in SKIP_NAMES or name == "tinyui_init":
        return False
    if name in TYPO_EXACT:
        return True
    if name.startswith("tinyui_app_") or name.startswith("tinyui_widget_"):
        return True
    if INIT_RE.fullmatch(name) and name != "tinyui_init":
        # only rename non-static exported widget inits in production
        return True
    return False


def rewrite_text(text: str) -> str:
    def repl(match: re.Match[str]) -> str:
        name = match.group(0)
        if not should_rename(name):
            return name
        return internal_name(name)

    # broader identifier pass for app_/widget_/known typos/inits
    pattern = re.compile(
        r"\b(?:tinyui_app_[A-Za-z0-9_]*|tinyui_widget_[A-Za-z0-9_]*|"
        r"tinyui_timer_handler|tinyui_button_set_press|tinyui_button_get_press|"
        r"tinyui_tabel_show_keyboard|tinyui_q_r_code_init|tinyui_q_r_code_set_text|"
        r"tinyui_[A-Za-z0-9_]+_init)\b"
    )
    return pattern.sub(repl, text)


def main() -> None:
    paths = list(SRC.rglob("*.c")) + list(SRC.rglob("*.h"))
    # also private include headers that declare production APIs
    paths += list((ROOT / "tinyui/include/internal").rglob("*.h"))
    changed = 0
    for path in paths:
        if path.name in {"v22_demo_bridge.h"}:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        new = rewrite_text(text)
        if new != text:
            path.write_text(new, encoding="utf-8")
            changed += 1
            print(f"renamed symbols in {path.relative_to(ROOT)}")
    print(f"changed_files={changed}")


if __name__ == "__main__":
    main()
