#!/usr/bin/env python3
"""Fail-closed gate: current-facing TinyUI docs use only canonical API, and
fenced examples marked ``c compile`` / ``cpp compile`` compile and link against
the installed TinyUI package only.

CLI::

    python3 tests/tinyui/contract/check_tinyui_docs_examples.py \\
        --build-dir build/v2.3-m4-docs \\
        --prefix build/v2.3-m4-demo/_tinyui_install

Optional ``--host-build`` is used to run ``cmake --install`` when the prefix
is missing. Canonical ``tinyui_window_*`` is allowed; legacy app/widget/native
and backend leak tokens are not.
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[3]

DOC_REL_PATHS = (
    "tinyui/docs/quick_start.md",
    "tinyui/docs/api_overview.md",
    "tinyui/docs/demo_guide.md",
    "tinyui/docs/resource_lifetime.md",
)

# Do NOT blanket-ban tinyui_window_ — window is a required canonical capability.
FORBIDDEN_PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    ("tinyui_app_", re.compile(r"\btinyui_app_[A-Za-z0-9_]*\b")),
    ("tinyui_widget_", re.compile(r"\btinyui_widget_[A-Za-z0-9_]*\b")),
    ("tinyui_native_", re.compile(r"\btinyui_native_[A-Za-z0-9_]*\b")),
    # Backend leak: ld followed by uppercase / known native-style tokens.
    ("ld[A-Z]", re.compile(r"\bld[A-Z][A-Za-z0-9_]*\b")),
    ("arm_2d_", re.compile(r"\barm_2d_[A-Za-z0-9_]*\b")),
    ("SIGNAL_", re.compile(r"\bSIGNAL_[A-Za-z0-9_]*\b")),
]

# ```lang extras...  — any fence language tag containing "compile" is buildable.
FENCE_RE = re.compile(
    r"^```([^\n`]*)\n(.*?)(?:^```[ \t]*\n?)",
    re.MULTILINE | re.DOTALL,
)


@dataclass
class CompileExample:
    doc: Path
    index: int
    lang: str  # "c" or "cpp"
    tag: str
    body: str
    line: int


def error(code: str, **details: Any) -> dict[str, Any]:
    return {"code": code, **details}


def format_error(item: dict[str, Any]) -> str:
    code = item.get("code", "error")
    parts = [f"{code}"]
    for key, value in item.items():
        if key == "code":
            continue
        parts.append(f"{key}={value}")
    return " | ".join(parts)


def rel_to_root(path: Path, root: Path = ROOT) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return str(path)


def scan_forbidden(path: Path, text: str) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    for line_no, line in enumerate(text.splitlines(), start=1):
        for name, pattern in FORBIDDEN_PATTERNS:
            for match in pattern.finditer(line):
                errors.append(
                    error(
                        "forbidden_token",
                        doc=rel_to_root(path),
                        line=line_no,
                        pattern=name,
                        match=match.group(0),
                    )
                )
    return errors


def extract_compile_examples(path: Path, text: str) -> list[CompileExample]:
    examples: list[CompileExample] = []
    for index, match in enumerate(FENCE_RE.finditer(text)):
        tag = match.group(1).strip()
        body = match.group(2)
        if "compile" not in tag.split():
            # Also accept tags like "c compile" / "cpp compile" with other tokens.
            if "compile" not in tag:
                continue
        tokens = tag.split()
        lang = tokens[0].lower() if tokens else ""
        if lang in ("c++", "cxx"):
            lang = "cpp"
        if lang not in ("c", "cpp"):
            # Require a C or C++ language tag when compile is requested.
            continue
        line = text.count("\n", 0, match.start()) + 1
        examples.append(
            CompileExample(
                doc=path,
                index=index,
                lang=lang,
                tag=tag,
                body=body,
                line=line,
            )
        )
    return examples


def ensure_prefix(prefix: Path, host_build: Path | None) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    config = prefix / "lib" / "cmake" / "TinyUI" / "TinyUIConfig.cmake"
    if config.is_file():
        return errors

    if host_build is None:
        # Heuristic: prefix ends with _tinyui_install under a cmake build tree.
        candidate = prefix.parent
        if (candidate / "CMakeCache.txt").is_file():
            host_build = candidate

    if host_build is None or not (host_build / "CMakeCache.txt").is_file():
        errors.append(
            error(
                "missing_install_prefix",
                prefix=str(prefix),
                detail=(
                    "TinyUIConfig.cmake not found; provide --prefix to an installed "
                    "tree or --host-build so the checker can cmake --install"
                ),
            )
        )
        return errors

    prefix.mkdir(parents=True, exist_ok=True)
    cmd = ["cmake", "--install", str(host_build), "--prefix", str(prefix)]
    print(f"[docs-examples] installing TinyUI: {' '.join(cmd)}", flush=True)
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        errors.append(
            error(
                "install_failed",
                host_build=str(host_build),
                prefix=str(prefix),
                returncode=proc.returncode,
                stdout=proc.stdout[-2000:],
                stderr=proc.stderr[-2000:],
            )
        )
        return errors

    if not config.is_file():
        errors.append(
            error(
                "install_missing_config",
                prefix=str(prefix),
                expected=str(config),
            )
        )
    return errors


def write_examples_project(
    stage: Path, examples: list[CompileExample]
) -> tuple[list[str], list[dict[str, Any]]]:
    """Materialize a temporary find_package consumer for all compile examples."""
    errors: list[dict[str, Any]] = []
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True, exist_ok=True)

    targets: list[str] = []
    cmake_lines = [
        "cmake_minimum_required(VERSION 3.16)",
        "project(tinyui_docs_examples LANGUAGES C CXX)",
        "find_package(TinyUI 2.3 CONFIG REQUIRED)",
        "",
    ]

    for i, ex in enumerate(examples):
        if ex.lang == "c":
            src_name = f"example_{i:02d}.c"
            target = f"docs_example_{i:02d}_c"
            ext_feature = "c_std_11"
        else:
            src_name = f"example_{i:02d}.cpp"
            target = f"docs_example_{i:02d}_cpp"
            ext_feature = "cxx_std_17"

        src_path = stage / src_name
        body = ex.body
        # Ensure a self-contained translation unit: if the block has no main,
        # wrap it is not our job — docs must provide complete compile units.
        if "main" not in body:
            errors.append(
                error(
                    "compile_example_missing_main",
                    doc=rel_to_root(ex.doc),
                    line=ex.line,
                    tag=ex.tag,
                    detail="compile examples must define main() as a complete unit",
                )
            )
            continue

        src_path.write_text(body if body.endswith("\n") else body + "\n", encoding="utf-8")
        targets.append(target)
        cmake_lines.extend(
            [
                f"add_executable({target} {src_name})",
                f"target_compile_features({target} PRIVATE {ext_feature})",
                f"target_link_libraries({target} PRIVATE TinyUI::tinyui)",
                "",
            ]
        )

    (stage / "CMakeLists.txt").write_text("\n".join(cmake_lines) + "\n", encoding="utf-8")
    return targets, errors


def _host_sanitizer_cmake_args(host_build: Path | None) -> list[str]:
    """Propagate host -fsanitize flags so consumers can link sanitizer archives."""
    if host_build is None:
        return []
    cache = host_build / "CMakeCache.txt"
    if not cache.is_file():
        return []
    wanted = {
        "CMAKE_C_FLAGS",
        "CMAKE_CXX_FLAGS",
        "CMAKE_EXE_LINKER_FLAGS",
        "CMAKE_SHARED_LINKER_FLAGS",
    }
    args: list[str] = []
    for line in cache.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line or line.startswith("//") or line.startswith("#") or "=" not in line:
            continue
        key_type, value = line.split("=", 1)
        key = key_type.split(":", 1)[0]
        if key in wanted and "-fsanitize=" in value:
            args.append(f"-D{key}={value}")
    return args


def build_examples(
    stage: Path,
    build_dir: Path,
    prefix: Path,
    host_build: Path | None = None,
) -> list[dict[str, Any]]:
    errors: list[dict[str, Any]] = []
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    configure = [
        "cmake",
        "-S",
        str(stage),
        "-B",
        str(build_dir),
        f"-DCMAKE_PREFIX_PATH={prefix}",
        *_host_sanitizer_cmake_args(host_build),
    ]
    print(f"[docs-examples] configure: {' '.join(configure)}", flush=True)
    proc = subprocess.run(configure, capture_output=True, text=True)
    if proc.returncode != 0:
        errors.append(
            error(
                "cmake_configure_failed",
                returncode=proc.returncode,
                stdout=proc.stdout[-3000:],
                stderr=proc.stderr[-3000:],
            )
        )
        return errors

    build = ["cmake", "--build", str(build_dir), "--parallel"]
    print(f"[docs-examples] build: {' '.join(build)}", flush=True)
    proc = subprocess.run(build, capture_output=True, text=True)
    if proc.returncode != 0:
        errors.append(
            error(
                "cmake_build_failed",
                returncode=proc.returncode,
                stdout=proc.stdout[-4000:],
                stderr=proc.stderr[-4000:],
            )
        )
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=ROOT,
        help="repository root (default: inferred from script path)",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        required=True,
        help="temporary cmake build directory for doc examples",
    )
    parser.add_argument(
        "--prefix",
        type=Path,
        required=True,
        help="TinyUI install prefix (must provide find_package(TinyUI))",
    )
    parser.add_argument(
        "--host-build",
        type=Path,
        default=None,
        help="optional host cmake build dir used to cmake --install when prefix is missing",
    )
    args = parser.parse_args(argv)

    root: Path = args.root.resolve()
    build_dir: Path = args.build_dir.resolve()
    prefix: Path = args.prefix.resolve()
    host_build: Path | None = (
        args.host_build.resolve() if args.host_build is not None else None
    )

    errors: list[dict[str, Any]] = []
    examples: list[CompileExample] = []

    for rel in DOC_REL_PATHS:
        path = root / rel
        if not path.is_file():
            errors.append(error("missing_doc", doc=rel))
            continue
        text = path.read_text(encoding="utf-8")
        errors.extend(scan_forbidden(path, text))
        examples.extend(extract_compile_examples(path, text))

    c_count = sum(1 for ex in examples if ex.lang == "c")
    cpp_count = sum(1 for ex in examples if ex.lang == "cpp")
    print(
        f"[docs-examples] docs={len(DOC_REL_PATHS)} "
        f"compile_examples={len(examples)} (c={c_count}, cpp={cpp_count})",
        flush=True,
    )

    if c_count < 1:
        errors.append(
            error(
                "missing_c_compile_example",
                detail="at least one ```c compile``` example is required",
            )
        )

    # Install package before attempting to compile examples.
    errors.extend(ensure_prefix(prefix, host_build))

    # Only attempt compile if forbidden scan and prefix are otherwise usable.
    has_fatal_pre = any(
        e.get("code")
        in {
            "missing_doc",
            "missing_install_prefix",
            "install_failed",
            "install_missing_config",
            "compile_example_missing_main",
            "missing_c_compile_example",
        }
        for e in errors
    )

    if examples and not any(
        e.get("code")
        in {
            "missing_install_prefix",
            "install_failed",
            "install_missing_config",
        }
        for e in errors
    ):
        stage = build_dir / "_src"
        targets, write_errors = write_examples_project(stage, examples)
        errors.extend(write_errors)
        if targets and not write_errors:
            errors.extend(build_examples(stage, build_dir / "_build", prefix, host_build=host_build))

    if errors:
        print("[docs-examples] FAIL", flush=True)
        for item in errors:
            print(f"  - {format_error(item)}", flush=True)
        print(f"[docs-examples] {len(errors)} error(s)", flush=True)
        return 1

    print("[docs-examples] PASS", flush=True)
    print(
        f"[docs-examples] compiled {len(examples)} example(s) against {prefix}",
        flush=True,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
