#!/usr/bin/env python3
"""Generate standalone C and C++ probes for every TinyUI public header."""

import argparse
import hashlib
import json
import re
from pathlib import Path


MANIFEST_SCHEMA_VERSION = "tinyui-v2.3-public-contract-probes-v1"


def scan_public_headers(root: Path) -> list[str]:
    """Return sorted header paths relative to tinyui/include, excluding internal."""
    include_dir = root / "tinyui" / "include"
    if not include_dir.is_dir():
        raise ValueError(f"missing TinyUI include directory: {include_dir}")
    return [
        header.relative_to(include_dir).as_posix()
        for header in sorted(include_dir.rglob("*.h"))
        if not any(
            part in {"internal", "extensions"}
            for part in header.relative_to(include_dir).parts
        )
    ]


def _header_id(header: str) -> str:
    return hashlib.sha256(header.encode("utf-8")).hexdigest()[:12]


def _function_id(header: str, symbol: str) -> str:
    return hashlib.sha256(f"{header}\0{symbol}".encode("utf-8")).hexdigest()[:12]


def _strip_comments_and_preprocessor(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", "", text)
    lines: list[str] = []
    skip_continuation = False
    for line in text.splitlines():
        stripped = line.lstrip()
        if skip_continuation or stripped.startswith("#"):
            skip_continuation = line.rstrip().endswith("\\")
            continue
        lines.append(line)
    return "\n".join(lines)


def _find_closing_paren(text: str, opening_index: int) -> int | None:
    depth = 0
    for index in range(opening_index, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    return None


def scan_public_functions(root: Path, headers: list[str] | None = None) -> list[dict]:
    """Return external tinyui_* declarations from the public header surface."""
    include_dir = root / "tinyui" / "include"
    rows: list[dict] = []
    for header in headers or scan_public_headers(root):
        text = _strip_comments_and_preprocessor((include_dir / header).read_text(encoding="utf-8", errors="replace"))
        text = re.sub(r"\bstatic\s+inline\b[^{};]*\{.*?\}", "", text, flags=re.DOTALL)
        for statement in text.split(";"):
            normalized = " ".join(statement.split())
            if not normalized or re.search(r"\b(?:typedef|static|inline)\b", normalized):
                continue
            for match in re.finditer(r"\b(?P<symbol>tinyui_[A-Za-z0-9_]+)\s*\(", normalized):
                closing_index = _find_closing_paren(normalized, normalized.find("(", match.start()))
                if closing_index is None or normalized[closing_index + 1 :].strip():
                    continue
                rows.append(
                    {
                        "header": header,
                        "symbol": match.group("symbol"),
                        "signature": normalized + ";",
                    }
                )
                break
    seen_by_symbol: dict[str, dict] = {}
    for row in rows:
        previous = seen_by_symbol.get(row["symbol"])
        if previous is not None and previous["signature"] != row["signature"]:
            raise ValueError(
                f"public TinyUI function signature mismatch for {row['symbol']}:\n"
                f"{previous['header']}: {previous['signature']}\n"
                f"{row['header']}: {row['signature']}"
            )
        seen_by_symbol[row["symbol"]] = row
    unique = {(row["header"], row["symbol"]): row for row in rows}
    return [unique[key] for key in sorted(unique)]


def _probe_source(header: str, language: str) -> str:
    if language == "c":
        return f'#include "{header}"\n\nint main(void) {{ return 0; }}\n'
    if language == "cpp":
        return f'extern "C" {{\n#include "{header}"\n}}\n\nint main() {{ return 0; }}\n'
    raise ValueError(f"unsupported probe language: {language}")


def _link_probe_source(header: str, symbol: str) -> str:
    return (
        f'#include "{header}"\n\n'
        f"static void *volatile symbol_ref = (void *)&{symbol};\n\n"
        "int main(void) { return symbol_ref == 0; }\n"
    )


def _manifest(headers: list[str], functions: list[dict]) -> dict:
    header_rows = [{"path": header, "id": _header_id(header)} for header in headers]
    function_rows = [
        {**row, "id": _function_id(row["header"], row["symbol"])}
        for row in functions
    ]
    generated_files = [
        path
        for row in header_rows
        for path in (f"c/header_{row['id']}.c", f"cpp/header_{row['id']}.cpp")
    ]
    generated_files.extend(f"link/function_{row['id']}.c" for row in function_rows)
    generated_files.append("probes.cmake")
    return {
        "schema_version": MANIFEST_SCHEMA_VERSION,
        "headers": header_rows,
        "functions": function_rows,
        "generated_files": generated_files,
    }


def _probes_cmake(headers: list[str], functions: list[dict]) -> str:
    lines = [
        "set(TINYUI_PUBLIC_HEADER_C_PROBES)",
        "set(TINYUI_PUBLIC_HEADER_CPP_PROBES)",
        "set(TINYUI_PUBLIC_SYMBOL_LINK_PROBES)",
    ]
    for header in headers:
        header_id = _header_id(header)
        c_target = f"tinyui_public_header_c_{header_id}"
        cpp_target = f"tinyui_public_header_cpp_{header_id}"
        lines.extend(
            [
                f'add_executable({c_target} "${{CMAKE_CURRENT_LIST_DIR}}/c/header_{header_id}.c")',
                f'target_include_directories({c_target} PRIVATE "${{CMAKE_SOURCE_DIR}}/tinyui/include")',
                f"set_target_properties({c_target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)",
                f"target_compile_options({c_target} PRIVATE -Wall -Wextra -Werror)",
                f"list(APPEND TINYUI_PUBLIC_HEADER_C_PROBES {c_target})",
                f'add_executable({cpp_target} "${{CMAKE_CURRENT_LIST_DIR}}/cpp/header_{header_id}.cpp")',
                f'target_include_directories({cpp_target} PRIVATE "${{CMAKE_SOURCE_DIR}}/tinyui/include")',
                f"set_target_properties({cpp_target} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)",
                f"target_compile_options({cpp_target} PRIVATE -Wall -Wextra -Werror)",
                f"list(APPEND TINYUI_PUBLIC_HEADER_CPP_PROBES {cpp_target})",
            ]
        )
    for function in functions:
        target = f"tinyui_public_symbol_link_{function['id']}"
        lines.extend(
            [
                f'add_executable({target} "${{CMAKE_CURRENT_LIST_DIR}}/link/function_{function["id"]}.c")',
                f'target_include_directories({target} PRIVATE "${{CMAKE_SOURCE_DIR}}/tinyui/include")',
                f"set_target_properties({target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)",
                f"target_compile_options({target} PRIVATE -Wall -Wextra -Werror)",
                f"target_link_libraries({target} PRIVATE tinyui_backend_ldgui tinyui_port_mcu)",
                f"list(APPEND TINYUI_PUBLIC_SYMBOL_LINK_PROBES {target})",
            ]
        )
    lines.extend(
        [
            "add_custom_target(tinyui_public_header_probes_c DEPENDS ${TINYUI_PUBLIC_HEADER_C_PROBES})",
            "add_custom_target(tinyui_public_header_probes_cpp DEPENDS ${TINYUI_PUBLIC_HEADER_CPP_PROBES})",
            "add_custom_target(tinyui_public_symbol_link_probes DEPENDS ${TINYUI_PUBLIC_SYMBOL_LINK_PROBES})",
            "add_custom_target(tinyui_public_header_probes DEPENDS",
            "    tinyui_public_header_probes_c",
            "    tinyui_public_header_probes_cpp",
            "    tinyui_public_symbol_link_probes",
            ")",
            "",
        ]
    )
    return "\n".join(lines)


def _prune_managed_files(output_dir: Path) -> None:
    for directory, pattern in (("c", "header_*.c"), ("cpp", "header_*.cpp"), ("link", "function_*.c")):
        for path in (output_dir / directory).glob(pattern):
            path.unlink()


def generate_probes(root: Path, output_dir: Path) -> dict:
    """Write standalone probes, their CMake declarations, and a checked manifest."""
    headers = scan_public_headers(root)
    functions = scan_public_functions(root, headers)
    manifest = _manifest(headers, functions)
    _prune_managed_files(output_dir)
    (output_dir / "c").mkdir(parents=True, exist_ok=True)
    (output_dir / "cpp").mkdir(parents=True, exist_ok=True)
    (output_dir / "link").mkdir(parents=True, exist_ok=True)
    for row in manifest["headers"]:
        (output_dir / "c" / f"header_{row['id']}.c").write_text(
            _probe_source(row["path"], "c"), encoding="utf-8"
        )
        (output_dir / "cpp" / f"header_{row['id']}.cpp").write_text(
            _probe_source(row["path"], "cpp"), encoding="utf-8"
        )
    for row in manifest["functions"]:
        (output_dir / "link" / f"function_{row['id']}.c").write_text(
            _link_probe_source(row["header"], row["symbol"]), encoding="utf-8"
        )
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "probes.cmake").write_text(
        _probes_cmake(headers, manifest["functions"]), encoding="utf-8"
    )
    (output_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    arguments = parser.parse_args()
    try:
        generate_probes(arguments.root, arguments.output_dir)
    except ValueError as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
