#!/usr/bin/env python3
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
SCAN_ROOTS = (
    ROOT / "tinyui" / "demo",
    ROOT / "tinyui_demo",
    ROOT / "tinyui" / "docs",
    ROOT / "docs" / "ability",
)
SCAN_SUFFIXES = {".c", ".h", ".md"}

DEPRECATED_APIS = {
    "tinyui_q_r_code_init": "tinyui_qrcode_create",
    "tinyui_q_r_code_set_text": "tinyui_qrcode_set_text",
    "tinyui_tabel_show_keyboard": "tinyui_table_show_keyboard",
}


def iter_scan_files():
    for root in SCAN_ROOTS:
        if not root.exists():
            continue
        for path in sorted(root.rglob("*")):
            if path.is_file() and path.suffix in SCAN_SUFFIXES:
                yield path


def main() -> None:
    failures: list[str] = []
    for path in iter_scan_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        for old, new in DEPRECATED_APIS.items():
            if old in text:
                rel = path.relative_to(ROOT)
                failures.append(f"{rel}: uses deprecated {old}; use {new}")

    assert not failures, "\n".join(failures)


if __name__ == "__main__":
    main()
