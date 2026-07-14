#!/usr/bin/env python3
"""Compatibility entry point for the canonical TinyUI inventory checker."""

import sys
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parents[2] / "tinyui" / "contract"
sys.path.insert(0, str(CONTRACT_DIR))

from check_ldgui_public_api_inventory import main


if __name__ == "__main__":
    raise SystemExit(main())
