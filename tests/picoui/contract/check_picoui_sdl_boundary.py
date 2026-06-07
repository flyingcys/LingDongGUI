from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PICOUI_ROOT = ROOT / "picoui"
ALLOWLIST = {
    PICOUI_ROOT / "port" / "sdl" / "sdl.c",
    PICOUI_ROOT / "include" / "picoui" / "port" / "sdl.h",
}
SDL_MARKERS = (
    "#include <SDL",
    "#include \"SDL",
    "SDL_",
)


def main() -> None:
    violations: list[str] = []
    for path in sorted(PICOUI_ROOT.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix not in {".c", ".h"}:
            continue
        if path in ALLOWLIST:
            continue
        text = path.read_text(encoding="utf-8")
        if any(marker in text for marker in SDL_MARKERS):
            violations.append(str(path.relative_to(ROOT)))

    assert not violations, "SDL boundary leaked outside picoui/port/sdl:\n" + "\n".join(violations)


if __name__ == "__main__":
    main()
