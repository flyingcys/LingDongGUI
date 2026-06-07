from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
SDL_API_RE = re.compile(r"(?<![A-Za-z0-9_])SDL_[A-Za-z0-9_]+")


def main() -> int:
    violations: list[str] = []
    for path in ROOT.glob("picoui/**/*.c"):
        rel = path.relative_to(ROOT).as_posix()
        if rel.startswith("picoui/port/sdl/"):
            continue
        text = path.read_text(encoding="utf-8")
        if "<SDL.h>" in text or "SDL.h" in text or SDL_API_RE.search(text):
            violations.append(rel)

    assert not violations, f"SDL leakage outside picoui/port/sdl: {violations}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
