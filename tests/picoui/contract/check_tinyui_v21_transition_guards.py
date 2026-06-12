#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
from pathlib import Path
import types


ROOT = Path(__file__).resolve().parents[3]
CANONICAL_SCRIPT = ROOT / "tests" / "tinyui" / "contract" / "check_tinyui_v21_transition_guards.py"


def _load_canonical_module() -> types.ModuleType:
    spec = importlib.util.spec_from_file_location(
        "check_tinyui_v21_transition_guards_canonical",
        CANONICAL_SCRIPT,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"unable to load canonical v2.1 transition-guard checker: {CANONICAL_SCRIPT}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    result = _load_canonical_module().main(__import__("sys").argv)
    return 0 if result is None else int(result)


if __name__ == "__main__":
    raise SystemExit(main())
