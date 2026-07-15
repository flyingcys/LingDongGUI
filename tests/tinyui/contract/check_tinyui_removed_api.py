#!/usr/bin/env python3
"""Fail-closed gate: legacy / misspelled / duplicate ABI must leave the canonical surface.

Canonical surfaces (must be zero hits):
  - tinyui_v23_public_api.json manifest
  - default-preprocessed public headers
  - install projection tree (optional --install-tree)
  - minimal archive (--archive-profile minimal)
  - full archive (--archive-profile full)

Both full and minimal archives require forbidden defined symbols to have zero
hits. Private helpers under tinyui_runtime_internal_* remain allowed.
The v22_demo_bridge has been removed; no bridge TU exception remains.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_INCLUDE = ROOT / "tinyui" / "include"
DEFAULT_MANIFEST = ROOT / "tests" / "tinyui" / "contract" / "tinyui_v23_public_api.json"

BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//.*?$", re.MULTILINE)
TINYUI_IDENTIFIER_RE = re.compile(r"\btinyui_[A-Za-z0-9_]*\b")
FORBIDDEN_INIT_RE = re.compile(r"^tinyui_[A-Za-z0-9_]+_init$")

# Prefix families (symbol.startswith)
FORBIDDEN_PREFIXES = (
    "tinyui_app_",
    "tinyui_widget_",
    "tinyui_tabel_",
    "tinyui_scroll_selecter_",
    "tinyui_q_r_code_",
)

# Exact names that are not fully covered by prefixes or need isolation
FORBIDDEN_EXACT = frozenset(
    {
        "tinyui_timer_handler",
        "tinyui_button_set_press",
        "tinyui_button_get_press",
        "tinyui_theme_create",
        "tinyui_theme_destroy",
        "tinyui_theme_apply_to_widget",
        "tinyui_app_set_theme",
        "tinyui_widget_set_style_class",
    }
)

# Headers that remain private / non-canonical and are not scanned as public surface.
# internal/ is never part of the install projection; extensions/ is opt-in native.
PRIVATE_HEADER_PREFIXES = (
    "internal/",
    "extensions/",
    "display/",
    "indev/",
    "tick/",
    "osal/",
    "port/",
    "integration/",
)

NM_DEFINED_TYPES = frozenset("ABCDRSTVW")


def strip_c_comments(text: str) -> str:
    text = BLOCK_COMMENT_RE.sub("", text)
    return LINE_COMMENT_RE.sub("", text)


def is_forbidden_symbol(name: str) -> bool:
    if name == "tinyui_init":
        return False
    # Private runtime helpers remain non-canonical but are not the removed public ABI.
    if name.startswith("tinyui_runtime_internal_"):
        return False
    if name in FORBIDDEN_EXACT:
        return True
    if any(name.startswith(prefix) for prefix in FORBIDDEN_PREFIXES):
        return True
    if FORBIDDEN_INIT_RE.fullmatch(name) is not None:
        return True
    return False


def error(code: str, **details: object) -> dict:
    return {"code": code, **details}


def iter_public_headers(include_dir: Path) -> list[Path]:
    headers: list[Path] = []
    for header in sorted(include_dir.rglob("*.h")):
        relative = header.relative_to(include_dir).as_posix()
        if any(relative.startswith(prefix) for prefix in PRIVATE_HEADER_PREFIXES):
            continue
        headers.append(header)
    return headers


def scan_text_for_forbidden(source: str, text: str) -> list[dict]:
    errors: list[dict] = []
    cleaned = strip_c_comments(text)
    for name in sorted(set(TINYUI_IDENTIFIER_RE.findall(cleaned))):
        if is_forbidden_symbol(name):
            errors.append(error("forbidden_symbol_in_header", source=source, name=name))
    return errors


def check_public_headers(include_dir: Path) -> list[dict]:
    if not include_dir.is_dir():
        return [error("missing_public_include", path=str(include_dir))]
    errors: list[dict] = []
    for header in iter_public_headers(include_dir):
        relative = header.relative_to(include_dir).as_posix()
        text = header.read_text(encoding="utf-8", errors="replace")
        errors.extend(scan_text_for_forbidden(relative, text))
    return errors


def check_manifest(manifest_path: Path) -> list[dict]:
    if not manifest_path.is_file():
        return [error("missing_manifest", path=str(manifest_path))]
    try:
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return [error("invalid_manifest_json", path=str(manifest_path), message=str(exc))]
    errors: list[dict] = []
    entries = payload.get("entries", [])
    if not isinstance(entries, list):
        return [error("invalid_manifest_entries", path=str(manifest_path))]
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        name = entry.get("name")
        if isinstance(name, str) and is_forbidden_symbol(name):
            errors.append(
                error(
                    "forbidden_symbol_in_manifest",
                    name=name,
                    header=entry.get("header"),
                )
            )
    return errors


def _config_template_path(root: Path = ROOT) -> Path:
    return root / "tinyui" / "include" / "tinyui_config.h.in"


def _discover_generated_config_dirs(root: Path = ROOT) -> list[Path]:
    """Locate build trees that already configured tinyui_config.h."""
    build_root = root / "build"
    if not build_root.is_dir():
        return []
    found: list[Path] = []
    for path in sorted(build_root.glob("*/generated/tinyui/tinyui_config.h")):
        found.append(path.parent)
    return found


def _materialize_default_config(tmp_dir: Path, root: Path = ROOT) -> Path:
    """Create a full-on tinyui_config.h for standalone preprocess checks."""
    template = _config_template_path(root)
    if not template.is_file():
        raise FileNotFoundError(f"missing tinyui_config template: {template}")
    # Default profile enables widgets/modules; preprocess only needs macros defined.
    text = template.read_text(encoding="utf-8")
    text = re.sub(r"#cmakedefine01\s+(\w+)", r"#define \1 1", text)
    out_dir = tmp_dir / "generated_tinyui"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "tinyui_config.h"
    out_path.write_text(text, encoding="utf-8")
    return out_dir


def check_preprocessed_aggregate(include_dir: Path) -> list[dict]:
    """Default preprocess of tinyui.h must not expose forbidden identifiers."""
    if shutil.which("cc") is None:
        return [error("missing_cc_for_preprocess")]
    aggregate = include_dir / "tinyui.h"
    if not aggregate.is_file():
        return [error("missing_aggregate_header", path=str(aggregate))]

    with tempfile.TemporaryDirectory(prefix="tinyui-removed-api-") as tmp:
        tmp_dir = Path(tmp)
        probe = tmp_dir / "probe.c"
        out = tmp_dir / "probe.i"
        probe.write_text('#include "tinyui.h"\n', encoding="utf-8")
        config_dirs = _discover_generated_config_dirs(ROOT)
        if not config_dirs:
            config_dirs = [_materialize_default_config(tmp_dir, ROOT)]
        include_flags: list[str] = [f"-I{include_dir}"]
        for config_dir in config_dirs[:1]:
            include_flags.append(f"-I{config_dir}")
        result = subprocess.run(
            [
                "cc",
                "-E",
                "-P",
                *include_flags,
                str(probe),
                "-o",
                str(out),
            ],
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode != 0:
            return [
                error(
                    "preprocess_failed",
                    stderr=result.stderr.strip(),
                    stdout=result.stdout.strip(),
                )
            ]
        text = out.read_text(encoding="utf-8", errors="replace")
        return [
            error("forbidden_symbol_in_preprocessed_aggregate", name=item["name"])
            for item in scan_text_for_forbidden("tinyui.h(preprocessed)", text)
        ]


def check_install_tree(install_tree: Path) -> list[dict]:
    if not install_tree.is_dir():
        return [error("missing_install_tree", path=str(install_tree))]
    # Prefer include/tinyui layout, fall back to tree root.
    candidates = [
        install_tree / "include" / "tinyui",
        install_tree / "include",
        install_tree,
    ]
    include_dir = next((path for path in candidates if path.is_dir()), install_tree)
    errors = check_public_headers(include_dir)
    # Internal headers / removed bridge must never ship.
    for pattern in ("**/internal/**", "**/v22_demo_bridge.h"):
        for path in install_tree.glob(pattern):
            if path.is_file() or path.is_dir():
                errors.append(
                    error(
                        "forbidden_internal_in_install_tree",
                        path=str(path.relative_to(install_tree)),
                    )
                )
    return errors


def _normalize_symbol(raw: str) -> str:
    return raw.lstrip("_")


def _run_nm(archive: Path) -> str:
    candidates = []
    for tool in ("nm", "llvm-nm", "gnm"):
        path = shutil.which(tool)
        if path:
            candidates.append(path)
    if not candidates:
        raise RuntimeError("no usable nm tool found in PATH")

    errors: list[str] = []
    for tool in candidates:
        for args in (
            [tool, "-g", "-A", str(archive)],
            [tool, "-g", "-o", str(archive)],
            [tool, "-g", str(archive)],
        ):
            result = subprocess.run(
                args,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            if result.returncode == 0 and result.stdout.strip():
                return result.stdout
            errors.append(
                f"{' '.join(args)} -> rc={result.returncode} stderr={result.stderr.strip()}"
            )
    raise RuntimeError("all nm commands failed:\n" + "\n".join(errors))


def parse_nm_defined_symbols(stdout: str) -> list[tuple[str, str, str]]:
    """Return list of (object, type, symbol) for defined symbols."""
    rows: list[tuple[str, str, str]] = []
    for line in stdout.splitlines():
        raw = line.strip()
        if not raw:
            continue
        # Formats:
        #   archive.a:obj.o: 00000000 T _sym
        #   archive.a:obj.o: T _sym
        #   00000000 T _sym
        object_name = ""
        body = raw
        if ".a:" in raw:
            archive_part, _, rest = raw.partition(".a:")
            object_name, sep, body = rest.partition(":")
            if not sep:
                body = rest
                object_name = ""
            else:
                object_name = object_name.strip()
                body = body.strip()
            _ = archive_part
        fields = body.split()
        if len(fields) < 2:
            continue
        if len(fields) >= 3 and re.fullmatch(r"[0-9A-Fa-f]+", fields[0]):
            symbol_type = fields[1]
            symbol_name = fields[2]
        else:
            symbol_type = fields[0]
            symbol_name = fields[1]
        symbol_type = symbol_type.upper()
        if symbol_type == "U":
            continue
        if symbol_type not in NM_DEFINED_TYPES:
            continue
        rows.append((object_name, symbol_type, _normalize_symbol(symbol_name)))
    return rows


def check_archive(archive: Path, *, profile: str) -> list[dict]:
    """Both full and minimal profiles require zero forbidden defined symbols."""
    if not archive.is_file():
        return [error("missing_archive", path=str(archive))]
    try:
        stdout = _run_nm(archive)
    except RuntimeError as exc:
        return [error("nm_failed", path=str(archive), message=str(exc))]

    errors: list[dict] = []
    defined = parse_nm_defined_symbols(stdout)
    forbidden_hits: dict[str, set[str]] = {}
    for object_name, _symbol_type, symbol in defined:
        if not is_forbidden_symbol(symbol):
            continue
        forbidden_hits.setdefault(symbol, set()).add(object_name or "<unknown>")

    code = (
        "forbidden_symbol_in_minimal_archive"
        if profile == "minimal"
        else "forbidden_symbol_in_full_archive"
    )
    for symbol, objects in sorted(forbidden_hits.items()):
        errors.append(
            error(
                code,
                name=symbol,
                objects=sorted(objects),
            )
        )
    return errors


def run_checks(
    *,
    root: Path,
    manifest: Path,
    archive: Path | None,
    archive_profile: str,
    install_tree: Path | None,
) -> list[dict]:
    include_dir = root / "tinyui" / "include"
    errors: list[dict] = []
    errors.extend(check_manifest(manifest))
    errors.extend(check_public_headers(include_dir))
    errors.extend(check_preprocessed_aggregate(include_dir))
    if install_tree is not None:
        errors.extend(check_install_tree(install_tree))
    if archive is not None:
        errors.extend(check_archive(archive, profile=archive_profile))
    return sorted(errors, key=lambda item: (item.get("code", ""), str(item)))


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument(
        "--archive",
        type=Path,
        help="Path to libtinyui_core.a (or other tinyui archive) for symbol checks",
    )
    parser.add_argument(
        "--archive-profile",
        choices=("full", "minimal"),
        default="full",
        help="full/minimal: both require zero forbidden defined symbols",
    )
    parser.add_argument(
        "--install-tree",
        type=Path,
        help="Optional installed include projection to scan",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    errors = run_checks(
        root=args.root.resolve(),
        manifest=args.manifest.resolve(),
        archive=args.archive.resolve() if args.archive else None,
        archive_profile=args.archive_profile,
        install_tree=args.install_tree.resolve() if args.install_tree else None,
    )
    if errors:
        for item in errors:
            print(json.dumps(item, sort_keys=True, ensure_ascii=True))
        print(f"TINYUI_REMOVED_API_FAIL count={len(errors)}", file=sys.stderr)
        return 1
    archive_note = f" archive={args.archive}" if args.archive else ""
    print(f"TINYUI_REMOVED_API_OK{archive_note}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
