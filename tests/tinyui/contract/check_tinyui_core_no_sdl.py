#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[3]
CORE_DIR = ROOT / "tinyui" / "src" / "core"

SDL_PATTERN = re.compile(
    r"#\s*include\s*[<\"]SDL(?:\.h|/)|\bSDL_[A-Za-z0-9_]*\b|\bSDL[A-Z][A-Za-z0-9_]*\b"
)


def main() -> int:
    violations: list[str] = []
    legacy_runtime_host = CORE_DIR / "runtime_host.c"
    if legacy_runtime_host.exists():
        rel = legacy_runtime_host.relative_to(ROOT)
        violations.append(f"{rel}: SDL host runtime must live under tinyui/port/sdl")

    for source in sorted(CORE_DIR.glob("*.[ch]")):
        text = source.read_text(encoding="utf-8")
        for lineno, line in enumerate(text.splitlines(), start=1):
            if SDL_PATTERN.search(line):
                rel = source.relative_to(ROOT)
                violations.append(f"{rel}:{lineno}: {line.strip()}")

    if violations:
        print("SDL symbols must stay out of tinyui/src/core:", file=sys.stderr)
        print("\n".join(violations), file=sys.stderr)
        return 1

    print("tinyui core SDL boundary guard OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
