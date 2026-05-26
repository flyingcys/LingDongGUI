from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "picoui" / "include" / "picoui"
FORBIDDEN = ["ld", "arm_2d_", "arm_2d_tile_t", "arm_2d_font_t", "SIGNAL_"]


def main() -> int:
    headers = sorted(PUBLIC_DIR.glob("*.h"))
    assert headers, "expected PicoUI public headers to exist"
    for header in headers:
        text = header.read_text(encoding="utf-8")
        for needle in FORBIDDEN:
            assert needle not in text, f"{header.name} leaks forbidden token: {needle}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
