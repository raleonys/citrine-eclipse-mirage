# SPDX-License-Identifier: GPL-3.0-only
"""Negative regression checks for the bounded C++ type-identity guard."""
import importlib.util
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location(
    "cpp_types", ROOT / "scripts/check_cpp_type_identities.py")
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


class CppTypeIdentityTests(unittest.TestCase):
    def test_current_headers(self):
        count, errors = checker.inspect(ROOT)
        self.assertGreater(count, 100)
        self.assertEqual(errors, [])

    def test_direct_self_inheritance_is_rejected(self):
        _, errors = checker.inspect_text(
            "class WorkerEffects:public WorkerEffects {};\n", "fixture.hpp")
        self.assertTrue(any("self-inheritance" in error for error in errors))

    def test_duplicate_definitions_are_rejected(self):
        _, errors = checker.inspect_text(
            "class Effects {};\nclass Effects final {};\n", "fixture.hpp")
        self.assertTrue(any("duplicate" in error for error in errors))

    def test_forward_declaration_is_allowed(self):
        definitions, errors = checker.inspect_text(
            "class Effects;\nclass Effects final:public Dependency {};\n")
        self.assertEqual(definitions, [("Effects", 2)])
        self.assertEqual(errors, [])

    def test_duplicate_types_across_socket_headers_are_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp) / "include" / "preloader_l" / "net"
            folder.mkdir(parents=True)
            for name in ("a.hpp", "b.hpp"):
                (folder / name).write_text(
                    "namespace preloader_l::socket {\nclass ShadowApi {};\n}\n",
                    encoding="utf-8")
            _, errors = checker.inspect(tmp)
            self.assertTrue(any("across socket headers for ShadowApi" in e for e in errors))

    def test_qualified_unrelated_base_is_not_self_inheritance(self):
        _, errors = checker.inspect_text(
            "class Cipher final:public messages::Cipher {};\n")
        self.assertEqual(errors, [])


if __name__ == "__main__":
    unittest.main()
