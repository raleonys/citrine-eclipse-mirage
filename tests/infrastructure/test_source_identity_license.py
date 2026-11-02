# SPDX-License-Identifier: GPL-3.0-only
"""Negative regression checks for first-party source/license identity."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('source_identity', ROOT / 'scripts/check_source_identity_license.py')
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


class SourceIdentityTests(unittest.TestCase):
    def test_current_sources_pass(self):
        count, errors = checker.inspect(ROOT)
        self.assertGreater(count, 500)
        self.assertEqual(errors, [])

    def test_missing_and_wrong_spdx_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'src/a.cpp'
            path.parent.mkdir(parents=True)
            path.write_text('int main() { return 0; }\n')
            self.assertTrue(checker.inspect(tmp)[1])
            path.write_text('// SPDX-License-Identifier: MIT\nint main() { return 0; }\n')
            self.assertTrue(checker.inspect(tmp)[1])

    def test_legacy_namespace_and_type_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'include/preloader_l/a.hpp'
            path.parent.mkdir(parents=True)
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\nnamespace preloader_l::crypto_recovered {}\n')
            self.assertTrue(any('Legacy recovery-derived' in e for e in checker.inspect(tmp)[1]))
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\nstruct RecoveredCipher {};\n')
            self.assertTrue(any('Legacy recovery-derived' in e for e in checker.inspect(tmp)[1]))

    def test_recovery_wording_in_comments_and_messages_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'src/example.cpp'
            path.parent.mkdir(parents=True)
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\n// recovered implementation note\n')
            self.assertTrue(any('Legacy recovery wording' in e for e in checker.inspect(tmp)[1]))
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\n'
                            'const char* error = "Recovered input required";\n')
            self.assertTrue(any('Legacy recovery wording' in e for e in checker.inspect(tmp)[1]))

    def test_vendor_and_fixture_excluded(self):
        with tempfile.TemporaryDirectory() as tmp:
            for name in ('third_party/lib/a.cpp', 'tests/vendors/lib.cpp', 'tests/fixtures/a.cpp'):
                path = Path(tmp) / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('namespace preloader_l::crypto_recovered {}\n')
            count, errors = checker.inspect(tmp)
            self.assertEqual((count, errors), (0, []))


if __name__ == '__main__':
    unittest.main()
