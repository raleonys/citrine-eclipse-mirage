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

    def test_late_only_spdx_marker_cannot_satisfy_header_contract(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'src' / 'late.cpp'
            path.parent.mkdir(parents=True)
            for source in (
                'int answer = 42;\n// SPDX-License-Identifier: GPL-3.0-only\n',
                '// preceding ordinary comment\n// SPDX-License-Identifier: GPL-3.0-only\n',
            ):
                with self.subTest(prefix=source.splitlines()[0]):
                    path.write_text(source, encoding='utf-8')
                    count, errors = checker.inspect(tmp)
                    self.assertEqual(count, 1)
                    self.assertTrue(any('SPDX header must precede source content'
                                        in error for error in errors), errors)
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\nint answer = 42;\n',
                            encoding='utf-8')
            self.assertEqual((1, []), checker.inspect(tmp))

    def test_launcher_preamble_can_precede_spdx_header(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name, source in (
                ('scripts/runner.py', '#!/usr/bin/env python3\n# SPDX-License-Identifier: GPL-3.0-only\nprint(1)\n'),
                ('scripts/runner.cmd', '@echo off\nREM SPDX-License-Identifier: GPL-3.0-only\necho ok\n'),
            ):
                with self.subTest(name=name):
                    path = root / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text(source, encoding='utf-8')
                    self.assertEqual([], checker.inspect(root)[1])

    def test_late_duplicate_spdx_marker_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'src/late.cpp'
            path.parent.mkdir(parents=True)
            header = '// SPDX-License-Identifier: GPL-3.0-only\n'
            padding = '// ordinary source comment\n' * 150
            self.assertGreater(len(header + padding), 3000)
            path.write_text(header + padding, encoding='utf-8')
            self.assertEqual((1, []), checker.inspect(tmp))
            for second_license in ('GPL-3.0-only', 'MIT'):
                with self.subTest(second_license=second_license):
                    path.write_text(header + padding +
                                    '// SPDX-License-Identifier: ' + second_license + '\n',
                                    encoding='utf-8')
                    checked, errors = checker.inspect(tmp)
                    self.assertEqual(1, checked)
                    self.assertTrue(any('Expected exactly one GPL-3.0-only SPDX header'
                                        in error for error in errors), errors)

    def test_legacy_namespace_and_type_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'include/laureondo/a.hpp'
            path.parent.mkdir(parents=True)
            path.write_text('// SPDX-License-Identifier: GPL-3.0-only\nnamespace laureondo::crypto_recovered {}\n')
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
                path.write_text('namespace laureondo::crypto_recovered {}\n')
            count, errors = checker.inspect(tmp)
            self.assertEqual((count, errors), (0, []))

    def test_retired_namespace_and_runtime_filename_are_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'src/example.cpp'
            path.parent.mkdir(parents=True)
            for source in ('namespace ' + 'preloader' + '_l {}',
                           'const char* dll = "preloader' + '_l.dll";'):
                path.write_text('// SPDX-License-Identifier: GPL-3.0-only\n' + source + '\n')
                self.assertTrue(any('Retired project identity' in error for error in checker.inspect(tmp)[1]))

    def test_cryptquery_compatibility_literal_is_the_only_source_exception(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'include/laureondo/platform/crypt_query.hpp'
            path.parent.mkdir(parents=True)
            source = '// SPDX-License-Identifier: GPL-3.0-only\n' + 'inline constexpr auto target_suffix=u"preloader' + '_l.dll";\n'
            path.write_text(source)
            self.assertEqual((1, []), checker.inspect(tmp))
            path.write_text(source + 'namespace ' + 'preloader' + '_l {}\n')
            self.assertTrue(checker.inspect(tmp)[1])


if __name__ == '__main__':
    unittest.main()
