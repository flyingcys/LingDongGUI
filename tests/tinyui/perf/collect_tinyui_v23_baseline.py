#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import re
import shutil
import struct
import subprocess
import statistics
from datetime import datetime, timezone
from pathlib import Path

import check_tinyui_perf as perf


ROOT = Path(__file__).resolve().parents[3]
WARMUP_COUNT = 5
SAMPLE_COUNT = 30


def percentile_nearest_rank(samples: list[float], percentile: float) -> float:
    if not samples or not 0 < percentile <= 1:
        raise ValueError("percentile requires non-empty samples and 0 < percentile <= 1")
    rank = max(1, int((len(samples) * percentile) + 0.999999999))
    return sorted(samples)[rank - 1]


def fingerprint_sha256(fingerprint: dict[str, object]) -> str:
    encoded = json.dumps(fingerprint, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(encoded).hexdigest()


def _command_output(command: list[str]) -> str:
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    return result.stdout.strip()


def _compiler_fingerprint() -> dict[str, str]:
    compiler = os.environ.get("CC", "cc")
    name_version = _command_output([compiler, "--version"]).splitlines()[0]
    try:
        triple = _command_output([compiler, "-dumpmachine"])
    except (OSError, subprocess.CalledProcessError):
        triple = "unknown"
    return {"name": Path(compiler).name, "version": name_version, "target_triple": triple}


def _cmake_cache(build_dir: Path) -> dict[str, str]:
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        raise ValueError(f"missing CMake cache: {cache}")
    values: dict[str, str] = {}
    for line in cache.read_text(encoding="utf-8").splitlines():
        match = re.match(r"^([^#:=]+):[^=]*=(.*)$", line)
        if match:
            values[match.group(1)] = match.group(2)
    return values


def collect_environment(build_dir: Path) -> dict[str, object]:
    cache = _cmake_cache(build_dir)
    options = {key: cache[key] for key in sorted(cache) if key.startswith(("TINYUI_", "LD_"))}
    environment_options = {
        key: value for key, value in sorted(os.environ.items()) if key.startswith(("TINYUI_", "LD_"))
    }
    return {
        "os": {"name": platform.system(), "version": platform.version()},
        "cpu": {"model": platform.processor() or platform.machine(), "architecture": platform.machine()},
        "compiler": _compiler_fingerprint(),
        "cmake": {
            "version": _command_output(["cmake", "--version"]).splitlines()[0],
            "build_type": cache.get("CMAKE_BUILD_TYPE", ""),
        },
        "optimization_flags": {
            "c": cache.get("CMAKE_C_FLAGS", ""),
            "c_release": cache.get("CMAKE_C_FLAGS_RELEASE", ""),
        },
        "tinyui_options": {
            "cmake": {key: value for key, value in options.items() if key.startswith("TINYUI_")},
            "environment": {key: value for key, value in environment_options.items() if key.startswith("TINYUI_")},
        },
        "ld_options": {
            "cmake": {key: value for key, value in options.items() if key.startswith("LD_")},
            "environment": {key: value for key, value in environment_options.items() if key.startswith("LD_")},
        },
        "git_commit": _command_output(["git", "rev-parse", "HEAD"]),
    }


def _artifact_paths(build_dir: Path) -> dict[str, Path]:
    candidates = {
        "full_binary": [build_dir / "examples/sdl/tinyui_demo", build_dir / "tinyui_demo"],
        "minimal_binary": [build_dir / "tests/tinyui/tinyui_minimal_consumer", build_dir / "tinyui_minimal_consumer"],
        "minimal_map": [build_dir / "tests/tinyui/tinyui_minimal_consumer.map", build_dir / "tinyui_minimal_consumer.map"],
        "wrapper_probe": [build_dir / "tests/tinyui/test_tinyui_wrapper_struct_overhead", build_dir / "test_tinyui_wrapper_struct_overhead"],
        "allocation_probe": [build_dir / "tests/tinyui/test_tinyui_steady_state_allocation", build_dir / "test_tinyui_steady_state_allocation"],
    }
    paths: dict[str, Path] = {}
    for name, options in candidates.items():
        path = next((candidate for candidate in options if candidate.is_file()), None)
        if path is None:
            raise FileNotFoundError(f"missing task 5/6/7 artifact: {name}")
        paths[name] = path
    return paths


def _normalize_macho_uuid(data: bytes) -> bytes:
    magic = data[:4]
    if magic in (b"\xce\xfa\xed\xfe", b"\xcf\xfa\xed\xfe"):
        endian = "<"
    elif magic in (b"\xfe\xed\xfa\xce", b"\xfe\xed\xfa\xcf"):
        endian = ">"
    else:
        return data

    is_64_bit = magic in (b"\xcf\xfa\xed\xfe", b"\xfe\xed\xfa\xcf")
    header_size = 32 if is_64_bit else 28
    if len(data) < header_size:
        return data
    ncmds = struct.unpack_from(f"{endian}I", data, 16)[0]
    offset = header_size
    normalized = bytearray(data)
    found_uuid = False
    code_signature_ranges: list[tuple[int, int]] = []
    symbol_table_range: tuple[int, int] | None = None
    canonical_symbols = bytearray()
    for _ in range(ncmds):
        if offset + 8 > len(data):
            return data
        command, command_size = struct.unpack_from(f"{endian}II", data, offset)
        if command_size < 8 or offset + command_size > len(data):
            return data
        if command == 0x1B and command_size >= 24:
            normalized[offset + 8 : offset + 24] = b"\0" * 16
            found_uuid = True
        if command == 0x1D and command_size >= 16:
            data_offset, data_size = struct.unpack_from(f"{endian}II", data, offset + 8)
            if data_offset + data_size > len(data):
                return data
            code_signature_ranges.append((data_offset, data_offset + data_size))
            normalized[offset + 8 : offset + 16] = b"\0" * 8
        if command == 0x2 and command_size >= 24:
            symoff, nsyms, stroff, strsize = struct.unpack_from(f"{endian}IIII", data, offset + 8)
            entry_size = 16 if is_64_bit else 12
            symbol_end = symoff + nsyms * entry_size
            string_end = stroff + strsize
            if symbol_end > len(data) or string_end > len(data) or symoff > stroff:
                return data
            symbol_table_range = (symoff, string_end)
            for index in range(nsyms):
                record_offset = symoff + index * entry_size
                if is_64_bit:
                    string_index, symbol_type, section, description, value = struct.unpack_from(
                        f"{endian}IBBHQ", data, record_offset
                    )
                else:
                    string_index, symbol_type, section, description, value = struct.unpack_from(
                        f"{endian}IBBHI", data, record_offset
                    )
                if string_index >= strsize:
                    return data
                name_start = stroff + string_index
                name_end = data.find(b"\0", name_start, string_end)
                if name_end < 0:
                    return data
                name = re.sub(rb"/build/[^/\x00]+/", b"/build/<tinyui-build>/", data[name_start:name_end])
                if symbol_type == 0x66:
                    value = 0
                if is_64_bit:
                    canonical_symbols.extend(
                        struct.pack(f"{endian}IBBHQ", 0, symbol_type, section, description, value)
                    )
                else:
                    canonical_symbols.extend(
                        struct.pack(f"{endian}IBBHI", 0, symbol_type, section, description, value)
                    )
                canonical_symbols.extend(struct.pack(f"{endian}I", len(name)))
                canonical_symbols.extend(name)
            normalized[offset + 16 : offset + 24] = b"\0" * 8
        if command in (0x1, 0x19):
            is_segment_64 = command == 0x19
            segment_name = data[offset + 8 : offset + 24].split(b"\0", 1)[0]
            if segment_name == b"__LINKEDIT":
                file_size_offset = offset + (48 if is_segment_64 else 36)
                file_size_width = 8 if is_segment_64 else 4
                normalized[file_size_offset : file_size_offset + file_size_width] = b"\0" * file_size_width
        offset += command_size
    if not found_uuid and not code_signature_ranges and symbol_table_range is None:
        return data
    for start, end in code_signature_ranges:
        normalized[start:end] = b"\0" * (end - start)
    excluded_ranges = list(code_signature_ranges)
    if symbol_table_range is not None:
        excluded_ranges.append(symbol_table_range)
        symbol_end = symbol_table_range[1]
        for signature_start, _ in code_signature_ranges:
            if symbol_end <= signature_start <= symbol_end + 16:
                excluded_ranges.append((symbol_end, signature_start))
    result = bytearray()
    cursor = 0
    for start, end in sorted(excluded_ranges):
        if start < cursor:
            continue
        result.extend(normalized[cursor:start])
        cursor = end
    result.extend(normalized[cursor:])
    if canonical_symbols:
        result.extend(b"\0TINYUI_CANONICAL_MACHO_SYMBOLS\0")
        result.extend(canonical_symbols)
    return bytes(result)


def artifact_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    digest.update(_normalize_macho_uuid(path.read_bytes()))
    return digest.hexdigest()


def _sha256(path: Path) -> str:
    return artifact_sha256(path)


def _run_probes(build_dir: Path, artifacts: dict[str, Path]) -> dict[str, str]:
    commands = {
        "task5_wrapper_overhead": [str(artifacts["wrapper_probe"])],
        "task7_binary_size": ["python3", str(perf.ROOT / "tests/tinyui/perf/check_tinyui_binary_size.py"), "--binary", str(artifacts["full_binary"])],
        "task7_minimal_symbols": [shutil.which("nm") or "nm", "-g", str(artifacts["minimal_binary"])],
        "task6_allocation": [str(artifacts["allocation_probe"])],
    }
    outputs = {}
    for name, command in commands.items():
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        outputs[name] = (result.stdout + result.stderr).strip()
    return outputs


def _collect_metric(executable: Path, marker: str) -> dict[str, object]:
    for _ in range(WARMUP_COUNT):
        perf.run_demo_once(executable)
    samples = [perf._parse_marker(perf.run_demo_once(executable).stdout, marker) for _ in range(SAMPLE_COUNT)]
    return {
        "warmup_count": WARMUP_COUNT,
        "samples": samples,
        "median": statistics.median(samples),
        "p95": percentile_nearest_rank(samples, 0.95),
    }


def resolve_output_path(output: Path | None, output_dir: Path | None, fingerprint: str) -> Path:
    if (output is None) == (output_dir is None):
        raise ValueError("exactly one of --output or --output-dir is required")
    if output is not None:
        return output
    assert output_dir is not None
    return output_dir / f"{fingerprint[:16]}.json"


def collect(build_dir: Path, output_dir: Path | None = None, output: Path | None = None) -> Path:
    build_dir = build_dir.resolve()
    if output_dir is not None:
        output_dir = output_dir.resolve()
    if output is not None:
        output = output.resolve()
    executable = perf.prepare_build(
        build_dir,
        targets=(
            perf.TARGET,
            "tinyui_minimal_consumer",
            "test_tinyui_wrapper_struct_overhead",
            "test_tinyui_steady_state_allocation",
        ),
    )
    environment = collect_environment(build_dir)
    artifacts = _artifact_paths(build_dir)
    probes = _run_probes(build_dir, artifacts)
    payload = {
        "$schema": "v2.3-performance-baseline.schema.json",
        "schema_version": 1,
        "fingerprint": environment,
        "fingerprint_sha256": fingerprint_sha256(environment),
        "measured_at": datetime.now(timezone.utc).isoformat(),
        "scenarios": {
            "screen_object_create_ms": _collect_metric(executable, "TINYUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS"),
            "capture_ready_ms": _collect_metric(executable, "TINYUI_BENCHMARK_CAPTURE_READY_MS"),
        },
        "probes": probes,
        "artifacts": {name: {"path": str(path.relative_to(ROOT)), "sha256": artifact_sha256(path)} for name, path in artifacts.items()},
    }
    output = resolve_output_path(output, output_dir, str(payload["fingerprint_sha256"]))
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(".json.tmp")
    temporary.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    temporary.replace(output)
    print(f"TINYUI_V23_BASELINE_COLLECTED {output}")
    return output


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    collect_parser = subparsers.add_parser("collect")
    collect_parser.add_argument("--build-dir", type=Path, required=True)
    output_group = collect_parser.add_mutually_exclusive_group(required=True)
    output_group.add_argument("--output", type=Path)
    output_group.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    if args.command == "collect":
        collect(args.build_dir, args.output_dir, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
