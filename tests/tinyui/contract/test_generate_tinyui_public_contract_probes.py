#!/usr/bin/env python3
"""TinyUI public-header probe generator contract tests."""

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


CONTRACT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(CONTRACT_DIR))

from check_tinyui_public_contract_manifest import check_manifest
from generate_tinyui_public_contract_probes import generate_probes, scan_public_headers


class TinyuiPublicContractProbeTests(unittest.TestCase):
    def _write_headers(self, root: Path) -> None:
        include = root / "tinyui" / "include"
        (include / "core").mkdir(parents=True)
        (include / "widgets").mkdir()
        (include / "internal").mkdir()
        (include / "tinyui.h").write_text("#ifndef TINYUI_H\n#define TINYUI_H\n#endif\n", encoding="utf-8")
        (include / "core" / "runtime.h").write_text(
            "#ifndef TINYUI_RUNTIME_H\n#define TINYUI_RUNTIME_H\n"
            "int tinyui_runtime_run(void);\n"
            "#endif\n",
            encoding="utf-8",
        )
        (include / "widgets" / "label.h").write_text(
            "#ifndef TINYUI_LABEL_H\n#define TINYUI_LABEL_H\n"
            "int tinyui_label_set_text(struct tinyui_label *label, const char *text);\n"
            "#endif\n",
            encoding="utf-8",
        )
        (include / "internal" / "private.h").write_text(
            "#ifndef TINYUI_PRIVATE_H\n#define TINYUI_PRIVATE_H\n#endif\n", encoding="utf-8"
        )

    def test_scans_recursive_public_headers_and_generates_c_cpp_wrappers(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            self._write_headers(root)
            output_dir = root / "generated" / "tinyui-public-contract"

            self.assertEqual(
                ["core/runtime.h", "tinyui.h", "widgets/label.h"],
                scan_public_headers(root),
            )

            manifest = generate_probes(root, output_dir)

            self.assertEqual(
                ["core/runtime.h", "tinyui.h", "widgets/label.h"],
                [entry["path"] for entry in manifest["headers"]],
            )
            self.assertEqual(
                ["tinyui_runtime_run", "tinyui_label_set_text"],
                [entry["symbol"] for entry in manifest["functions"]],
            )
            label_id = hashlib.sha256(b"widgets/label.h").hexdigest()[:12]
            self.assertEqual(
                label_id,
                next(entry["id"] for entry in manifest["headers"] if entry["path"] == "widgets/label.h"),
            )
            self.assertEqual(
                '#include "widgets/label.h"\n\nint main(void) { return 0; }\n',
                (output_dir / "c" / f"header_{label_id}.c").read_text(encoding="utf-8"),
            )
            self.assertEqual(
                'extern "C" {\n#include "widgets/label.h"\n}\n\nint main() { return 0; }\n',
                (output_dir / "cpp" / f"header_{label_id}.cpp").read_text(encoding="utf-8"),
            )
            label_function = next(
                entry for entry in manifest["functions"] if entry["symbol"] == "tinyui_label_set_text"
            )
            self.assertEqual(
                '#include "widgets/label.h"\n\n'
                'static void *volatile symbol_ref = (void *)&tinyui_label_set_text;\n\n'
                'int main(void) { return symbol_ref == 0; }\n',
                (output_dir / "link" / f"function_{label_function['id']}.c").read_text(encoding="utf-8"),
            )
            self.assertEqual(
                len(manifest["functions"]),
                sum(path.startswith("link/function_") for path in manifest["generated_files"]),
            )
            for function in manifest["functions"]:
                source = (output_dir / "link" / f"function_{function['id']}.c").read_text(encoding="utf-8")
                self.assertIn(f'#include "{function["header"]}"', source)
                self.assertEqual(1, source.count(f"&{function['symbol']}"))
            probes_cmake = (output_dir / "probes.cmake").read_text(encoding="utf-8")
            self.assertIn(
                f"tinyui_public_symbol_link_{label_function['id']}",
                probes_cmake,
            )
            self.assertIn("tinyui_backend_ldgui", probes_cmake)
            self.assertIn("tinyui_port_mcu", probes_cmake)
            self.assertIn("-Wl,--start-group", probes_cmake)
            self.assertIn("longdonggui_arm2d", probes_cmake)
            self.assertEqual([], check_manifest(root, output_dir))

    def test_scans_only_external_tinyui_function_declarations(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            include = root / "tinyui" / "include" / "widgets"
            include.mkdir(parents=True)
            (include / "sample.h").write_text(
                "typedef void (*tinyui_sample_callback)(int value);\n"
                "static inline int tinyui_sample_inline(int value) { return value; }\n"
                "#define tinyui_sample_macro(value) ((value) + 1)\n"
                "int tinyui_sample_set(\n"
                "    int value,\n"
                "    void (*callback)(int));\n"
                "const char *tinyui_sample_get(void);\n",
                encoding="utf-8",
            )

            manifest = generate_probes(root, root / "generated" / "tinyui-public-contract")

            self.assertEqual(
                ["tinyui_sample_get", "tinyui_sample_set"],
                [entry["symbol"] for entry in manifest["functions"]],
            )
            self.assertTrue(all("static inline" not in entry["signature"] for entry in manifest["functions"]))

    def test_rejects_same_public_symbol_with_different_signatures(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            include = root / "tinyui" / "include"
            (include / "alpha").mkdir(parents=True)
            (include / "beta").mkdir()
            (include / "alpha" / "one.h").write_text(
                "int tinyui_conflict(int value);\n", encoding="utf-8"
            )
            (include / "beta" / "two.h").write_text(
                "int tinyui_conflict(const char *value);\n", encoding="utf-8"
            )

            result = subprocess.run(
                [
                    sys.executable,
                    str(CONTRACT_DIR / "generate_tinyui_public_contract_probes.py"),
                    "--root",
                    str(root),
                    "--output-dir",
                    str(root / "generated" / "tinyui-public-contract"),
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertNotEqual(0, result.returncode)
            self.assertIn("tinyui_conflict", result.stderr)
            self.assertIn("alpha/one.h: int tinyui_conflict(int value);", result.stderr)
            self.assertIn("beta/two.h: int tinyui_conflict(const char *value);", result.stderr)

    def test_project_cmake_registers_public_symbol_link_ctest(self):
        cmake = (CONTRACT_DIR.parent / "CMakeLists.txt").read_text(encoding="utf-8")

        self.assertIn("NAME check_tinyui_public_symbols_link", cmake)
        self.assertIn("--target tinyui_public_symbol_link_probes", cmake)
        self.assertIn('LABELS "tinyui;contract;public-link"', cmake)

    def test_real_public_headers_include_window_layout_getters(self):
        project_root = CONTRACT_DIR.parents[2]

        with tempfile.TemporaryDirectory() as temporary_directory:
            manifest = generate_probes(project_root, Path(temporary_directory) / "probes")

        symbols = {entry["symbol"] for entry in manifest["functions"]}
        self.assertIn("tinyui_window_get_gap", symbols)
        self.assertIn("tinyui_window_get_layout_type", symbols)

    def test_regeneration_prunes_only_managed_stale_probe_files(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            self._write_headers(root)
            output_dir = root / "generated" / "tinyui-public-contract"
            first_manifest = generate_probes(root, output_dir)
            label_header = next(entry for entry in first_manifest["headers"] if entry["path"] == "widgets/label.h")
            label_function = next(
                entry for entry in first_manifest["functions"] if entry["symbol"] == "tinyui_label_set_text"
            )
            preserved = output_dir / "c" / "preserve.txt"
            preserved.write_text("outside generator ownership\n", encoding="utf-8")
            (root / "tinyui" / "include" / "widgets" / "label.h").unlink()

            manifest = generate_probes(root, output_dir)

            self.assertNotIn("widgets/label.h", [entry["path"] for entry in manifest["headers"]])
            self.assertFalse((output_dir / "c" / f"header_{label_header['id']}.c").exists())
            self.assertFalse((output_dir / "cpp" / f"header_{label_header['id']}.cpp").exists())
            self.assertFalse((output_dir / "link" / f"function_{label_function['id']}.c").exists())
            self.assertEqual("outside generator ownership\n", preserved.read_text(encoding="utf-8"))
            self.assertEqual([], check_manifest(root, output_dir))

    def test_uses_relative_path_hashes_to_avoid_basename_collisions(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            include = root / "tinyui" / "include"
            for directory in (include / "alpha", include / "beta"):
                directory.mkdir(parents=True)
                (directory / "common.h").write_text("#pragma once\n", encoding="utf-8")
            output_dir = root / "generated" / "tinyui-public-contract"

            manifest = generate_probes(root, output_dir)

            headers = {entry["path"]: entry["id"] for entry in manifest["headers"]}
            self.assertNotEqual(headers["alpha/common.h"], headers["beta/common.h"])
            self.assertTrue((output_dir / "c" / f"header_{headers['alpha/common.h']}.c").is_file())
            self.assertTrue((output_dir / "c" / f"header_{headers['beta/common.h']}.c").is_file())

    def test_manifest_checker_rejects_missing_or_drifted_generated_files(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            self._write_headers(root)
            output_dir = root / "generated" / "tinyui-public-contract"
            manifest = generate_probes(root, output_dir)
            manifest_path = output_dir / "manifest.json"
            value = json.loads(manifest_path.read_text(encoding="utf-8"))
            value["headers"].pop()
            manifest_path.write_text(json.dumps(value), encoding="utf-8")

            errors = check_manifest(root, output_dir)

            self.assertTrue(any("headers" in error for error in errors), errors)
            self.assertEqual(manifest, generate_probes(root, output_dir))
            missing = output_dir / manifest["generated_files"][0]
            missing.unlink()
            errors = check_manifest(root, output_dir)
            self.assertTrue(any("missing generated file" in error for error in errors), errors)


if __name__ == "__main__":
    unittest.main()
