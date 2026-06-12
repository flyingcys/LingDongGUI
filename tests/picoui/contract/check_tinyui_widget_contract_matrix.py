#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
from pathlib import Path
import types


LEGACY_SCRIPT = Path(__file__).with_name("check_picoui_widget_contract_matrix.py")


def _load_legacy_module() -> types.ModuleType:
    spec = importlib.util.spec_from_file_location(
        "check_picoui_widget_contract_matrix_legacy",
        LEGACY_SCRIPT,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"unable to load legacy widget-contract checker: {LEGACY_SCRIPT}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    result = _load_legacy_module().main()
    return 0 if result is None else int(result)


if __name__ == "__main__":
    raise SystemExit(main())
