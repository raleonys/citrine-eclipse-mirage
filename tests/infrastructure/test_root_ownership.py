# SPDX-License-Identifier: GPL-3.0-only

"""Regression tests for the independent repository root ownership gate."""
import shutil
import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from unittest import mock
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from check_repository_contract import (
    check_root_file_ownership, check_archived_evidence, check_active_identifiers,
    check_layout_migration_reference, declared_application_sources,
)


def fixture_git_environment():
    # Hooks export repository selectors which override git -C. Only child Git
    # processes for disposable fixtures receive this environment copy.
    environment = os.environ.copy()
    local_names = subprocess.run(['git', 'rev-parse', '--local-env-vars'],
                                 capture_output=True, text=True, check=True,
                                 timeout=30).stdout.splitlines()
    for name in local_names:
        environment.pop(name, None)
    return environment


class RepositoryRootOwnership(unittest.TestCase):
    def test_application_declarations_keep_complete_source_path_boundaries(self):
        text = '''add_library(app src/xml/reader.cpp "src/platform/entry.cpp")
        add_library(vendor third_party/pugixml/src/pugixml.cpp)
        add_library(other external/src/adapter.cpp ../src/foreign.cpp)
        add_library(unlisted src/runtime/unowned.cpp)'''
        self.assertEqual({'src/xml/reader.cpp', 'src/platform/entry.cpp',
                          'src/runtime/unowned.cpp'}, declared_application_sources(text))

    def test_nested_fixtures_preserve_inherited_worktree_repository(self):
        if not shutil.which('git'):
            self.skipTest('Git executable not available')
        for all_selectors in (False, True):
            with self.subTest(all_selectors=all_selectors), tempfile.TemporaryDirectory() as tmp:
                parent, child = Path(tmp) / 'parent', Path(tmp) / 'linked'
                parent.mkdir()
                environment = fixture_git_environment()

                def git(root, *args):
                    return subprocess.run(['git', '-C', str(root), *args], env=environment,
                                          check=True, capture_output=True, text=True,
                                          timeout=30).stdout.strip()

                git(parent, 'init')
                (parent / 'README.md').write_text('owned outer fixture\n')
                git(parent, 'add', 'README.md')
                git(parent, '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                    '-c', 'commit.gpgsign=false', 'commit', '-m', 'Outer fixture')
                git(parent, 'worktree', 'add', '--detach', str(child), 'HEAD')
                gitdir = Path(git(child, 'rev-parse', '--absolute-git-dir'))
                head = git(child, 'rev-parse', 'HEAD')
                registry = git(parent, 'worktree', 'list', '--porcelain')
                index = (gitdir / 'index').read_bytes()
                config = (parent / '.git/config').read_bytes()
                selectors = {'GIT_DIR': str(gitdir)}
                if all_selectors:
                    selectors.update(GIT_WORK_TREE=str(child),
                                     GIT_COMMON_DIR=str(parent / '.git'),
                                     GIT_INDEX_FILE=str(gitdir / 'index'))
                with mock.patch.dict(os.environ, selectors):
                    self.test_real_git_worktree_gitfile_is_allowed()
                    self.test_borrowed_valid_worktree_redirect_does_not_claim_another_root()
                    self.assertEqual(selectors, {key: os.environ[key] for key in selectors})
                self.assertEqual(head, git(child, 'rev-parse', 'HEAD'))
                self.assertEqual(registry, git(parent, 'worktree', 'list', '--porcelain'))
                self.assertEqual(index, (gitdir / 'index').read_bytes())
                self.assertEqual(config, (parent / '.git/config').read_bytes())
                self.assertEqual('owned outer fixture\n', (child / 'README.md').read_text())

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

    def test_real_git_worktree_gitfile_is_allowed(self):
        if not shutil.which('git'):
            self.skipTest('Git executable not available')
        with tempfile.TemporaryDirectory() as tmp:
            parent = Path(tmp) / 'parent'
            child = Path(tmp) / 'worktree'
            parent.mkdir()
            environment = fixture_git_environment()
            def git(*args):
                subprocess.run(['git', '-C', str(parent), *args],
                               check=True, capture_output=True, text=True,
                               timeout=30, env=environment)
            git('init')
            git('-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                'commit', '--allow-empty', '-m', 'fixture')
            git('worktree', 'add', '--detach', str(child), 'HEAD')
            self.assertTrue((child / '.git').is_file())
            self.assertEqual([], check_root_file_ownership(child, ['README.md']))
            (child / 'UNOWNED_ROOT_SCRIPT.py').write_text('fixture\n')
            self.assertEqual(['Unowned root file: UNOWNED_ROOT_SCRIPT.py'],
                             check_root_file_ownership(child, ['README.md']))

    def test_arbitrary_gitfile_and_git_symlink_are_not_exempt(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / '.git').write_text('not a gitdir redirect\n')
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(root, ['README.md']))
            (root / '.git').write_text('gitdir: nonexistent\n')
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(root, ['README.md']))
            (root / '.git').unlink()
            try:
                (root / '.git').symlink_to(root / 'nowhere')
            except (OSError, NotImplementedError):
                return
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(root, ['README.md']))

    def test_forged_gitdir_head_and_reverse_pointer_are_not_a_worktree(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'fake-checkout'
            gitdir = Path(tmp) / 'fake-gitdir'
            root.mkdir()
            gitdir.mkdir()
            (root / '.git').write_text('gitdir: ' + str(gitdir) + '\n')
            (gitdir / 'HEAD').write_text('ref: refs/heads/main\n')
            # This would pass the former helper (gitdir directory + HEAD).
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(root, ['README.md']))
            (gitdir / 'gitdir').write_text(str(root / '.git') + '\n')
            (gitdir / 'commondir').write_text('.\n')
            # Even a fabricated backlink and common directory are insufficient:
            # Git must recognize and list the working tree itself.
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(root, ['README.md']))

    def test_borrowed_valid_worktree_redirect_does_not_claim_another_root(self):
        if not shutil.which('git'):
            self.skipTest('Git executable not available')
        with tempfile.TemporaryDirectory() as tmp:
            parent = Path(tmp) / 'parent'
            child = Path(tmp) / 'real-worktree'
            impostor = Path(tmp) / 'impostor'
            parent.mkdir()
            impostor.mkdir()
            environment = fixture_git_environment()
            def git(*args):
                subprocess.run(['git', '-C', str(parent), *args],
                               check=True, capture_output=True, text=True,
                               timeout=30, env=environment)
            git('init')
            git('-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                'commit', '--allow-empty', '-m', 'fixture')
            git('worktree', 'add', '--detach', str(child), 'HEAD')
            self.assertEqual([], check_root_file_ownership(child, ['README.md']))
            (impostor / '.git').write_bytes((child / '.git').read_bytes())
            self.assertEqual(['Unowned root file: .git'],
                             check_root_file_ownership(impostor, ['README.md']))

    def test_invalid_or_duplicate_policy_fails_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            for policy in [[], ['README.md', 'README.md'], ['../escape'], ['a/b'],
                           ['a\\b'], ['.'], [123], 'README.md']:
                with self.subTest(policy=policy), self.assertRaises(ValueError):
                    check_root_file_ownership(tmp, policy)

    def test_layout_migration_contract_pointer_is_safe_and_live(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            target = root / 'metadata/evidence/LAYOUT_MIGRATION.json'
            target.parent.mkdir(parents=True)
            contract = {'layout_migration': 'metadata/evidence/LAYOUT_MIGRATION.json'}
            target.write_text('{"schema":"surge.layout-migration.v1"}', encoding='utf-8')
            self.assertEqual([], check_layout_migration_reference(root, contract))
            self.assertTrue(check_layout_migration_reference(
                root, {'layout_migration': 'docs/evidence/LAYOUT_MIGRATION.json'}))
            for unsafe in ('../escape.json', '/tmp/absolute.json',
                           'metadata//evidence/LAYOUT_MIGRATION.json',
                           'metadata/./evidence/LAYOUT_MIGRATION.json',
                           'metadata/../evidence/LAYOUT_MIGRATION.json',
                           'metadata\\\\evidence\\\\LAYOUT_MIGRATION.json', '', None, 1):
                with self.subTest(unsafe=unsafe):
                    self.assertTrue(check_layout_migration_reference(
                        root, {'layout_migration': unsafe}))
            target.write_text('not JSON', encoding='utf-8')
            self.assertTrue(check_layout_migration_reference(root, contract))
            target.write_text('{"schema":"wrong.document"}', encoding='utf-8')
            self.assertTrue(check_layout_migration_reference(root, contract))
            target.unlink()
            outside = root / 'elsewhere.json'
            outside.write_text('{"schema":"surge.layout-migration.v1"}')
            try:
                target.symlink_to(outside)
            except (OSError, NotImplementedError):
                pass
            else:
                self.assertTrue(check_layout_migration_reference(root, contract))

    def test_historical_evidence_retained_bytes_and_file_set(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / 'metadata/evidence'
            archive.mkdir(parents=True)
            data = b'original owner evidence'
            (archive / 'original.json').write_bytes(data)
            (archive / 'README.md').write_text('immutable historical records')
            policy = {'schema': 'surge.evidence-retention.v1', 'retained_count': 1,
                      'retained_files': {'original.json': hashlib.sha256(data).hexdigest()}}
            (root / 'metadata/EVIDENCE_RETENTION_POLICY.json').write_text(json.dumps(policy))
            self.assertEqual([], check_archived_evidence(root))
            (archive / 'original.json').write_bytes(b'mutated evidence')
            self.assertIn('Historical evidence bytes differ: original.json',
                          check_archived_evidence(root))
            (archive / 'original.json').write_bytes(data)
            (archive / 'unexpected.log').write_text('new output')
            self.assertTrue(any('file-set mismatch' in error
                                for error in check_archived_evidence(root)))
            (archive / 'unexpected.log').unlink()
            (root / 'docs/evidence').mkdir(parents=True)
            self.assertIn('Forbidden legacy evidence directory: docs/evidence',
                          check_archived_evidence(root))

    def test_historical_evidence_links_fail_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / 'metadata/evidence'
            archive.mkdir(parents=True)
            data = b'original owner evidence'
            pinned = archive / 'original.json'
            pinned.write_bytes(data)
            readme = archive / 'README.md'
            readme.write_text('immutable historical records')
            (root / 'metadata/EVIDENCE_RETENTION_POLICY.json').write_text(json.dumps({
                'schema': 'surge.evidence-retention.v1',
                'retained_count': 1,
                'retained_files': {'original.json': hashlib.sha256(data).hexdigest()},
            }))
            unpinned = root / 'outside.json'
            unpinned.write_bytes(b'changed and unpinned bytes')
            self.assertEqual([], check_archived_evidence(root))
            try:
                pinned.unlink()
                pinned.symlink_to(unpinned)
            except (OSError, NotImplementedError) as error:
                self.skipTest(f'Symlinks unavailable: {error}')
            self.assertIn('Historical evidence link is forbidden: original.json',
                          check_archived_evidence(root))
            pinned.unlink()
            pinned.write_bytes(data)
            self.assertEqual([], check_archived_evidence(root))
            readme.unlink()
            readme.symlink_to(unpinned)
            self.assertIn('Historical evidence link is forbidden: README.md',
                          check_archived_evidence(root))
            readme.unlink()
            readme.write_text('immutable historical records')
            archive.rename(root / 'relocated-evidence')
            try:
                archive.symlink_to(root / 'relocated-evidence', target_is_directory=True)
            except (OSError, NotImplementedError) as error:
                self.skipTest(f'Directory symlinks unavailable: {error}')
            self.assertIn('Historical evidence archive is a link',
                          check_archived_evidence(root))

    def test_archived_provenance_labels_do_not_exempt_active_metadata(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'docs').mkdir()
            (root / 'docs/IDENTIFIER_POLICY.md').write_text(
                '<!-- active-name-policy -->\n```json\n'
                '{"reserved_reference_labels": ["obsolete-label"]}\n```\n')
            (root / 'metadata/evidence').mkdir(parents=True)
            (root / 'metadata/evidence/old.json').write_text('{"schema":"obsolete-label.old"}')
            (root / 'metadata/ACTIVE.json').write_text('{"schema":"obsolete-label.active"}')
            errors = check_active_identifiers(root)
            self.assertTrue(any('metadata/ACTIVE.json' in error for error in errors))
            self.assertFalse(any('metadata/evidence/' in error for error in errors))



if __name__ == '__main__':
    unittest.main()
