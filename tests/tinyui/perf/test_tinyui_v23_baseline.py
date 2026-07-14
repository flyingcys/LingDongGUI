#!/usr/bin/env python3
"""M0 任务8基线采集与检查器的 TDD 测试。"""
from __future__ import annotations

import importlib.util
import json
import shutil
import struct
import tempfile
import sys
import unittest
from pathlib import Path


PERF_DIR = Path(__file__).resolve().parent
if str(PERF_DIR) not in sys.path:
    sys.path.insert(0, str(PERF_DIR))


def _load_module(name: str):
    path = PERF_DIR / f"{name}.py"
    if not path.is_file():
        return None
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load {name} from {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


COLLECTOR = _load_module("collect_tinyui_v23_baseline")
CHECKER = _load_module("check_tinyui_v23_baseline")
PERF_CHECKER = _load_module("check_tinyui_perf")


class TinyUIV23BaselineTests(unittest.TestCase):
    def test_baseline_modules_exist(self):
        self.assertIsNotNone(COLLECTOR, "collector module is not implemented")
        self.assertIsNotNone(CHECKER, "checker module is not implemented")
        self.assertIsNotNone(PERF_CHECKER, "perf checker module is not implemented")

    def test_host_benchmark_build_enables_real_observer(self):
        self.assertIn("-DENABLE_TEST=ON", getattr(PERF_CHECKER, "CMAKE_BENCHMARK_OPTIONS", ()))

    def test_common_target_config_maps_build_paths_for_reproducible_artifacts(self):
        cmake_path = COLLECTOR.ROOT / "cmake" / "LingDongGUI.cmake"
        cmake_text = cmake_path.read_text(encoding="utf-8")
        self.assertIn('"-ffile-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"', cmake_text)
        self.assertIn('"-fdebug-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"', cmake_text)
        self.assertIn('"-fmacro-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"', cmake_text)

    def test_tinyui_demo_uses_a_build_local_lto_object_path(self):
        cmake_path = COLLECTOR.ROOT / "examples" / "sdl" / "CMakeLists.txt"
        cmake_text = cmake_path.read_text(encoding="utf-8")
        self.assertIn("object_path_lto", cmake_text)

    def test_artifacts_are_checked_against_passed_build_dir(self):
        baseline_path = PERF_DIR / "v2.3-baselines" / "7d78276bbf838ddc.json"
        if not baseline_path.is_file():
            self.skipTest("当前机器 fingerprint JSON 不存在")
        payload = json.loads(baseline_path.read_text(encoding="utf-8"))
        with tempfile.TemporaryDirectory() as temp_dir:
            build_dir = Path(temp_dir) / "replacement-build"
            for entry in payload["artifacts"].values():
                relative = Path(entry["path"])
                target = build_dir.joinpath(*relative.parts[2:])
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(COLLECTOR.ROOT / relative, target)
            CHECKER._check_artifacts(payload, build_dir)
            corrupted = build_dir / "examples/sdl/tinyui_demo"
            corrupted.write_bytes(corrupted.read_bytes() + b"changed")
            with self.assertRaisesRegex(ValueError, "artifact SHA-256 mismatch: full_binary"):
                CHECKER._check_artifacts(payload, build_dir)

    def test_checker_resolves_a_unique_baseline_from_directory(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            directory = Path(temp_dir)
            (directory / "fingerprint.json").write_text("{}", encoding="utf-8")
            self.assertEqual(
                CHECKER.resolve_baseline_path(None, directory), directory / "fingerprint.json"
            )

    def test_collector_output_option_is_an_exact_file(self):
        output = Path("/tmp/tinyui-baseline.json")
        self.assertEqual(COLLECTOR.resolve_output_path(output, None, "abcdef"), output)

    def test_task8_schema_rejects_missing_field(self):
        baseline_path = PERF_DIR / "v2.3-baselines" / "7d78276bbf838ddc.json"
        if not baseline_path.is_file():
            self.skipTest("当前机器 fingerprint JSON 不存在")
        payload = json.loads(baseline_path.read_text(encoding="utf-8"))
        del payload["fingerprint_sha256"]
        with self.assertRaisesRegex(ValueError, "fingerprint_sha256"):
            CHECKER.validate_schema(payload)

    def test_task8_schema_rejects_extra_field(self):
        baseline_path = PERF_DIR / "v2.3-baselines" / "7d78276bbf838ddc.json"
        if not baseline_path.is_file():
            self.skipTest("当前机器 fingerprint JSON 不存在")
        payload = json.loads(baseline_path.read_text(encoding="utf-8"))
        payload["unexpected"] = True
        with self.assertRaisesRegex(ValueError, "unexpected"):
            CHECKER.validate_schema(payload)

    @unittest.skipUnless(COLLECTOR is not None and CHECKER is not None, "待实现 collector/checker")
    def test_p95_uses_nearest_rank_29th_value_for_30_samples(self):
        samples = [float(value) for value in range(1, 31)]

        self.assertEqual(COLLECTOR.percentile_nearest_rank(samples, 0.95), 29.0)

    @unittest.skipUnless(COLLECTOR is not None and CHECKER is not None, "待实现 collector/checker")
    def test_samples_require_exactly_30_values(self):
        with self.assertRaisesRegex(ValueError, "exactly 30"):
            CHECKER.validate_metric_samples([1.0] * 29, warmup_count=5)
        with self.assertRaisesRegex(ValueError, "exactly 30"):
            CHECKER.validate_metric_samples([1.0] * 31, warmup_count=5)

    @unittest.skipUnless(COLLECTOR is not None and CHECKER is not None, "待实现 collector/checker")
    def test_warmup_requires_exactly_5_runs(self):
        with self.assertRaisesRegex(ValueError, "exactly 5"):
            CHECKER.validate_metric_samples([1.0] * 30, warmup_count=4)
        with self.assertRaisesRegex(ValueError, "exactly 5"):
            CHECKER.validate_metric_samples([1.0] * 30, warmup_count=6)

    @unittest.skipUnless(COLLECTOR is not None and CHECKER is not None, "待实现 collector/checker")
    def test_macho_uuid_is_ignored_by_collector_and_checker_digest(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            build_dir = Path(temp_dir) / "build"
            artifact = build_dir / "tests/tinyui/tinyui_minimal_consumer"
            artifact.parent.mkdir(parents=True)
            artifact.write_bytes(_macho_with_uuid(bytes(range(16))))
            baseline_digest = COLLECTOR.artifact_sha256(artifact)

            artifact.write_bytes(_macho_with_uuid(bytes(reversed(range(16)))))
            self.assertEqual(baseline_digest, COLLECTOR.artifact_sha256(artifact))

            payload = {
                "artifacts": {
                    "minimal_binary": {
                        "path": "build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer",
                        "sha256": baseline_digest,
                    }
                }
            }
            CHECKER._check_artifacts(payload, build_dir)

    def test_macho_digest_keeps_non_signature_linkedit_bytes_strict(self):
        source = COLLECTOR.ROOT / "build" / "v2.3-m0-full" / "examples" / "sdl" / "tinyui_demo"
        if not source.is_file():
            self.skipTest("当前构建目录没有 full demo")
        data = bytearray(source.read_bytes())
        offset = 32
        ncmds = struct.unpack_from("<I", data, 16)[0]
        symoff = None
        for _ in range(ncmds):
            command, command_size = struct.unpack_from("<II", data, offset)
            if command == 2:
                symoff = struct.unpack_from("<I", data, offset + 8)[0]
                break
            offset += command_size
        if symoff is None:
            self.skipTest("Mach-O 没有 LC_SYMTAB")
        with tempfile.TemporaryDirectory() as temp_dir:
            original = Path(temp_dir) / "original"
            mutated = Path(temp_dir) / "mutated"
            original.write_bytes(data)
            data[symoff] ^= 1
            mutated.write_bytes(data)
            self.assertNotEqual(
                COLLECTOR.artifact_sha256(original),
                COLLECTOR.artifact_sha256(mutated),
            )

    def test_full_and_closeout_macho_artifacts_share_digest(self):
        relative_paths = (
            Path("examples/sdl/tinyui_demo"),
            Path("tests/tinyui/tinyui_minimal_consumer"),
        )
        for relative_path in relative_paths:
            full = COLLECTOR.ROOT / "build" / "v2.3-m0-full" / relative_path
            closeout = COLLECTOR.ROOT / "build" / "v2.3-m0-closeout" / relative_path
            if not full.is_file() or not closeout.is_file():
                self.skipTest("当前构建目录缺少 full/closeout artifact")
            self.assertEqual(
                COLLECTOR.artifact_sha256(full),
                COLLECTOR.artifact_sha256(closeout),
                str(relative_path),
            )

    @unittest.skipUnless(COLLECTOR is not None, "待实现 collector")
    def test_non_macho_digest_remains_strict(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            first = Path(temp_dir) / "first.bin"
            second = Path(temp_dir) / "second.bin"
            first.write_bytes(b"same-size-a")
            second.write_bytes(b"same-size-b")
            self.assertNotEqual(
                COLLECTOR.artifact_sha256(first),
                COLLECTOR.artifact_sha256(second),
            )

    @unittest.skipUnless(COLLECTOR is not None and CHECKER is not None, "待实现 collector/checker")
    def test_every_fingerprint_field_changes_fingerprint_digest(self):
        fingerprint = {
            "os": {"name": "Darwin", "version": "25.0.0"},
            "cpu": {"model": "test-cpu", "architecture": "arm64"},
            "compiler": {
                "name": "AppleClang",
                "version": "17.0.0",
                "target_triple": "arm64-apple-darwin25.0.0",
            },
            "cmake": {"version": "4.0.0", "build_type": "Release"},
            "optimization_flags": ["-O2", "-DNDEBUG"],
            "tinyui_options": {"LD_TINYUI_PORT": "sdl"},
            "ld_options": {"LD_BUILD_SDL_DEMO": "ON"},
            "git_commit": "0123456789abcdef",
        }
        original_digest = COLLECTOR.fingerprint_sha256(fingerprint)

        for field_path in (
            ("os", "version"),
            ("cpu", "architecture"),
            ("compiler", "version"),
            ("compiler", "target_triple"),
            ("cmake", "build_type"),
            ("optimization_flags", 0),
            ("tinyui_options", "LD_TINYUI_PORT"),
            ("ld_options", "LD_BUILD_SDL_DEMO"),
            ("git_commit",),
        ):
            changed = _copy_fingerprint(fingerprint)
            target = changed
            for key in field_path[:-1]:
                target = target[key]
            key = field_path[-1]
            target[key] = _changed_value(target[key])
            self.assertNotEqual(
                original_digest,
                COLLECTOR.fingerprint_sha256(changed),
                field_path,
            )


def _copy_fingerprint(value):
    if isinstance(value, dict):
        return {key: _copy_fingerprint(item) for key, item in value.items()}
    if isinstance(value, list):
        return [_copy_fingerprint(item) for item in value]
    return value


def _changed_value(value):
    if isinstance(value, bool):
        return not value
    if isinstance(value, int):
        return value + 1
    if isinstance(value, float):
        return value + 1.0
    if isinstance(value, str):
        return value + ".changed"
    raise TypeError(f"unsupported test value: {value!r}")


def _macho_with_uuid(uuid: bytes) -> bytes:
    header = struct.pack("<IiiIIIII", 0xFEEDFACF, 0x0100000C, 0, 2, 1, 24, 0, 0)
    load_command = struct.pack("<II16s", 0x1B, 24, uuid)
    return header + load_command + b"payload"


if __name__ == "__main__":
    unittest.main()
