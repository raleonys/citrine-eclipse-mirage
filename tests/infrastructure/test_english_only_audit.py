"""Deterministic first-party language audit and exclusion boundary tests."""
import json
from pathlib import Path
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

    def test_document_blocks_code_requires_review_and_exclusions_are_preserved(self):
        paths = ['docs/prose.md', 'src/module.cpp',
                 'third_party/vendor/readme.md', 'metadata/evidence/record.md']
        self.write('docs/prose.md', 'Headline\nCaf\u00e9\n')
        self.write('src/module.cpp', '// \u03b1: intentional fixture\n')
        self.write('third_party/vendor/readme.md', 'Caf\u00e9\n')
        self.write('metadata/evidence/record.md', 'Caf\u00e9\n')
        self.manifest(paths)
        report = checker.audit(self.root)
        self.assertEqual(2, report['scanned_text_files'])
        self.assertEqual(1, report['blocking'])
        self.assertEqual(1, report['review_required'])
        self.assertEqual(['docs/prose.md', 'src/module.cpp'],
                         [x['path'] for x in report['findings']])
        self.assertEqual(1, checker.main(['--root', str(self.root), '--check']))
        self.write('docs/prose.md', 'English text only\n')
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

if __name__ == '__main__':
    unittest.main()
