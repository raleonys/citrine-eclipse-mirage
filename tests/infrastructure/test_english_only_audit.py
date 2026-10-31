"""Deterministic first-party language audit and exclusion boundary tests."""
import json
import re
from pathlib import Path, PurePosixPath
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
import check_english_only as checker


class LanguageSignalAuditTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'metadata').mkdir()

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8')

    def manifest(self, names):
        self.write('metadata/ACTIVE_SOURCE_MANIFEST.json',
                   json.dumps({'files': {name: 'test-only' for name in names}}))

    def test_source_and_prose_block_non_english_letters_but_fixture_data_is_reviewed(self):
        paths = ['docs/prose.md', 'src/module.cpp', 'tests/unit/fixture.cpp',
                 'third_party/vendor/readme.md', 'metadata/evidence/record.md']
        self.write('docs/prose.md', 'English heading\nUnicode scalar: \u03b1\n')
        self.write('src/module.cpp', '// Synthetic scalar: \u03b1\n')
        self.write('tests/unit/fixture.cpp', '// Synthetic scalar: \u03b1\n')
        self.write('third_party/vendor/readme.md', 'Unicode scalar: \u03b1\n')
        self.write('metadata/evidence/record.md', 'Unicode scalar: \u03b1\n')
        self.manifest(paths)
        report = checker.audit(self.root)
        self.assertEqual(3, report['scanned_text_files'])
        self.assertEqual(2, report['blocking'])
        self.assertEqual(1, report['review_required'])
        self.assertEqual(['docs/prose.md', 'src/module.cpp', 'tests/unit/fixture.cpp'],
                         [x['path'] for x in report['findings']])
        self.assertEqual(1, checker.main(['--root', str(self.root), '--check']))
        self.write('docs/prose.md', 'English text only\n')
        self.write('src/module.cpp', '// English diagnostic only\n')
        self.assertEqual(0, checker.main(['--root', str(self.root), '--check']))

    def test_ascii_only_content_is_not_certified(self):
        self.write('docs/prose.md', 'An ASCII-only sentence.\n')
        self.manifest(['docs/prose.md'])
        report = checker.audit(self.root)
        self.assertEqual('surge.english-only-audit.v1', report['schema'])
        self.assertEqual('LANGUAGE_SIGNAL_AUDIT_NOT_SEMANTIC_CERTIFICATE',
                         report['classification'])
        self.assertEqual([], report['findings'])

    def test_nonregular_or_missing_files_fail_closed(self):
        self.manifest(['docs/missing.md'])
        with self.assertRaisesRegex(ValueError, 'Missing or nonregular'):
            checker.audit(self.root)
        self.assertEqual(2, checker.main(['--root', str(self.root), '--check']))

    def test_manifest_paths_cannot_escape_root(self):
        for path in ['../outside.md', '/tmp/absolute.md', 'docs/../prose.md']:
            with self.subTest(path=path):
                self.manifest([path])
                with self.assertRaisesRegex(ValueError, 'Unsafe source path'):
                    checker.audit(self.root)

    def test_invalid_utf8_and_deterministic_order(self):
        self.write('docs/z.md', 'Plain English\n')
        self.write('docs/a.md', 'Plain English\n')
        self.manifest(['docs/z.md', 'docs/a.md'])
        (self.root / 'docs/a.md').write_bytes(b'\xff\xfe')
        report = checker.audit(self.root)
        self.assertEqual(1, report['blocking'])
        self.assertEqual('docs/a.md', report['findings'][0]['path'])
        self.assertEqual(report, checker.audit(self.root))

    def test_intentional_nonletter_unicode_not_mislabeled(self):
        self.write('docs/symbols.md', 'A -> B, \u2192, \u2014 and 12 \u00d7 3\n')
        self.manifest(['docs/symbols.md'])
        self.assertEqual([], checker.audit(self.root)['findings'])


    def test_active_build_diagnostics_use_english(self):
        """Guard user-facing build errors and their existing exit codes."""
        root = Path(__file__).resolve().parents[2]
        expected = {
            "scripts/build_orchestration/build_common.ps1": (
                "if (-not $cmd) { Fail-Build \"$Name was not found.\" 127 }",
                "if ([string]::IsNullOrWhiteSpace($RelativePath)) { Fail-Build \"$Label cannot be empty.\" 64 }",
                "if ([System.IO.Path]::IsPathRooted($RelativePath)) { Fail-Build \"$Label must not be an absolute path: $RelativePath\" 64 }",
                "Fail-Build \"$Label is outside the repository build/ boundary: $RelativePath\" 64",
                "Fail-Build \"$Label cannot be the build root itself.\" 64",
                "if (-not $vswhere) { Fail-Build 'vswhere.exe was not found.' 101 }",
                "if (-not $vsPath) { Fail-Build 'MSVC C++ x64 toolset was not found.' 102 }",
                "if (-not (Test-Path -LiteralPath $vcvars)) { Fail-Build \"vcvars64.bat was not found: $vcvars\" 103 }",
                "if ($rc -ne 0) { Fail-Build \"vcvars64.bat failed (exit $rc).\" $rc }",
                "Fail-Build \"Another production build is using this checkout: $Path\" 73",
                "Fail-Build \"$Label failed (exit $rc).\" $rc",
            ),
            "scripts/build_orchestration/build_profile.ps1": (
                "Fail-Build \"Local Golden chain failed (exit $GoldenCode).\" $GoldenCode",
            ),
        }
        for relative_path, statements in expected.items():
            content = (root / relative_path).read_text(encoding='utf-8')
            for statement in statements:
                with self.subTest(path=relative_path, statement=statement):
                    self.assertEqual(1, content.count(statement))


    def test_all_windows_build_profiles_request_english_tool_diagnostics(self):
        """The shared operator profile opts into en-US without changing caller settings."""
        root = Path(__file__).resolve().parents[2]
        profile = (root / 'scripts/build_orchestration/build_profile.ps1').read_text(
            encoding='utf-8')
        self.assertIn("ValidateSet('Compile','Debug','Native','Crt','Golden')", profile)
        selection = "$env:VSLANG = '1033'"
        self.assertEqual(1, profile.count(selection))
        self.assertLess(profile.index('Import-VcVarsEnvironment $CmdExe $vs[1]'),
                        profile.index(selection))
        self.assertLess(profile.index(selection),
                        profile.index("Invoke-ExternalChecked $CMakeExe @('-S'"))
        self.assertIn('MSVC diagnostic locale requested: English (VSLANG=1033)',
                      profile)
        self.assertIn("Join-Path (Split-Path -Parent $ClExe) '1033\clui.dll'", profile)
        self.assertIn('Fail-Build "English MSVC diagnostic resources are required:', profile)
        self.assertLess(profile.index("1033\clui.dll"),
                        profile.index("Invoke-ExternalChecked $CMakeExe @('-S'"))
        self.assertIn('Get-ChildItem Env: | ForEach-Object { $EnvironmentBefore', profile)
        self.assertIn('foreach ($Name in $EnvironmentBefore.Keys)', profile)
        self.assertIn("[Environment]::SetEnvironmentVariable($Name,$EnvironmentBefore[$Name],'Process')",
                      profile)

    def test_known_ascii_diagnostic_rejected_without_foreign_language_examples(self):
        # This marker is synthetic English. No foreign-language words are used.
        marker = 'Language' + 'Violation' + 'Probe'
        self.write('src/fixture.cpp', '// ' + marker + '\n')
        self.manifest(['src/fixture.cpp'])
        report = checker.audit(self.root)
        self.assertEqual(1, report['blocking'])
        self.assertEqual('non_english_ascii_diagnostic', report['findings'][0]['kind'])
        self.assertEqual(1, checker.main(['--root', str(self.root), '--check']))
        self.write('src/fixture.cpp', '// English diagnostic only\n')
        self.assertEqual(0, checker.main(['--root', str(self.root), '--check']))

    def test_current_code_inventory_has_no_blocking_language_signals(self):
        root = Path(__file__).resolve().parents[2]
        manifest = json.loads((root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
                              .read_text(encoding='utf-8'))
        counts = {'src': 0, 'include': 0, 'tests': 0}
        extensions = {'.c', '.cpp', '.h', '.hpp', '.py', '.ps1'}
        for name in checker.source_paths(manifest):
            family = name.split('/', 1)[0]
            if family in counts and PurePosixPath(name).suffix.lower() in extensions:
                counts[family] += 1
        self.assertGreaterEqual(counts['src'], 100)
        self.assertGreaterEqual(counts['include'], 100)
        self.assertGreaterEqual(counts['tests'], 200)
        report = checker.audit(root)
        self.assertEqual(0, report['blocking'], report['findings'][:10])


if __name__ == '__main__':
    unittest.main()
