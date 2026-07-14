#!/usr/bin/env python3
"""Fail closed when generated TinyUI public-header probes drift from headers."""

import argparse
import json
from pathlib import Path

from generate_tinyui_public_contract_probes import (
    MANIFEST_SCHEMA_VERSION,
    _manifest,
    scan_public_functions,
    scan_public_headers,
)


def _load_manifest(path: Path) -> tuple[dict | None, list[str]]:
    if not path.is_file():
        return None, [f"missing manifest: {path}"]
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        return None, [f"invalid manifest JSON: {path}: {error}"]
    if not isinstance(value, dict):
        return None, ["manifest root must be an object"]
    required_fields = {"schema_version", "headers", "functions", "generated_files"}
    if set(value) != required_fields:
        return None, ["manifest fields are invalid"]
    if value["schema_version"] != MANIFEST_SCHEMA_VERSION:
        return None, ["manifest schema_version is invalid"]
    if not all(isinstance(value[field], list) for field in ("headers", "functions", "generated_files")):
        return None, ["manifest collection fields are invalid"]
    return value, []


def check_manifest(root: Path, output_dir: Path) -> list[str]:
    """Return all manifest and generated-file contract violations."""
    manifest, errors = _load_manifest(output_dir / "manifest.json")
    if manifest is None:
        return errors
    try:
        headers = scan_public_headers(root)
        expected = _manifest(headers, scan_public_functions(root, headers))
    except ValueError as error:
        return [str(error)]
    for field in ("headers", "functions", "generated_files"):
        if manifest[field] != expected[field]:
            errors.append(f"manifest {field} does not match the public header scan")
    expected_generated = set(expected["generated_files"])
    for relative_path in manifest["generated_files"]:
        if not isinstance(relative_path, str) or not relative_path:
            errors.append("manifest generated_files contains an invalid path")
            continue
        if Path(relative_path).is_absolute() or ".." in Path(relative_path).parts:
            errors.append("manifest generated_files contains a path outside the output directory")
            continue
        generated_path = output_dir / relative_path
        if not generated_path.is_file():
            errors.append(f"missing generated file: {relative_path}")
    actual_generated = {
        path.relative_to(output_dir).as_posix()
        for directory, pattern in (
            (output_dir / "c", "header_*.c"),
            (output_dir / "cpp", "header_*.cpp"),
            (output_dir / "link", "function_*.c"),
        )
        if directory.is_dir()
        for path in directory.rglob(pattern)
    }
    if (output_dir / "probes.cmake").is_file():
        actual_generated.add("probes.cmake")
    unexpected = sorted(actual_generated - expected_generated)
    if unexpected:
        errors.append(f"unexpected generated files: {', '.join(unexpected)}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    arguments = parser.parse_args()
    errors = check_manifest(arguments.root, arguments.output_dir)
    if errors:
        print(json.dumps({"errors": errors}, indent=2))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
