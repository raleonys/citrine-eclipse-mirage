"""Regression tests for the independent repository root ownership gate."""
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from check_repository_contract import check_root_file_ownership


class RepositoryRootOwnership(unittest.TestCase):
    def test_expected_root_files_and_directories_are_allowed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'README.md').write_text('expected\n')
            (root / '.gitignore').write_text('build/\n')
            (root / 'src').mkdir()
            self.assertEqual([], check_root_file_ownership(root, ['README.md', '.gitignore']))

    def test_extra_root_script_and_hidden_file_are_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name in ['README.md', 'UNOWNED_ROOT_SCRIPT.py', '.unexpected']:
                (root / name).write_text('fixture\n')
            self.assertEqual(
                ['Unowned root file: .unexpected', 'Unowned root file: UNOWNED_ROOT_SCRIPT.py'],
                check_root_file_ownership(root, ['README.md']))

    def test_invalid_or_duplicate_policy_fails_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            for policy in [[], ['README.md', 'README.md'], ['../escape'], ['a/b'],
                           ['a\\b'], ['.'], [123], 'README.md']:
                with self.subTest(policy=policy), self.assertRaises(ValueError):
                    check_root_file_ownership(tmp, policy)


if __name__ == '__main__':
    unittest.main()
