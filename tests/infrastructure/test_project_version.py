# SPDX-License-Identifier: GPL-3.0-only
"""Release stamping regressions; synthetic resources are not native acceptance."""
import copy
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import project_version as version
from golden_policy import load_target
from verify_mustavex_golden_metadata import validate_versioninfo_root
from project_files import write_manifest, verify_manifest


def resource_node(required):
    info = required['versioninfo']
    return {'key': 'VS_VERSION_INFO', 'value_bytes': struct.pack(
        '<13I', *(int(value, 16) for value in info['fixed'].values())), 'children': [
        {'key': 'StringFileInfo', 'children': [
            {'key': info['string_table'], 'children': [
                {'key': key, 'value': value} for key, value in info['strings'].items()]}]},
        {'key': 'VarFileInfo', 'children': [{'key': 'Translation', 'value_bytes': struct.pack(
            '<HH', int(info['translation_language'], 16), info['translation_codepage'])}]}]}


class ProjectVersionTests(unittest.TestCase):
    def test_default_and_release_override_share_all_four_version_fields(self):
        with tempfile.TemporaryDirectory() as tmp, patch.dict('os.environ', {}, clear=True):
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'metadata/BRANDING_AND_IDENTITY.json').write_text(
                json.dumps({'default_project_version': '5.4.3'}))
            self.assertEqual('5.4.3', version.selected_version(root))
        with patch.dict('os.environ', {version.ENVIRONMENT: '3.2.8'}):
            policy = load_target(ROOT, 'MUSTAVEX_GOLDEN_METADATA.json')['required']
            self.assertEqual('0x00030002', policy['versioninfo']['fixed']['dwFileVersionMS'])
            self.assertEqual('0x00080000', policy['versioninfo']['fixed']['dwFileVersionLS'])
            self.assertEqual('0x00080000', policy['versioninfo']['fixed']['dwProductVersionLS'])
            self.assertEqual('3.2.8', policy['versioninfo']['strings']['FileVersion'])
            self.assertEqual('3.2.8', policy['versioninfo']['strings']['ProductVersion'])
            self.assertEqual([], validate_versioninfo_root(resource_node(policy), policy))


    def test_versioninfo_uses_one_consistent_selected_version(self):
        policy = {
            'version_source': 'metadata/BRANDING_AND_IDENTITY.json',
            'required': {'versioninfo': {'fixed': {}, 'strings': {}}},
        }
        original = copy.deepcopy(policy)
        # A mutable environment or metadata file must not mix versions within
        # one VERSIONINFO policy. Later selections are a counterfactual.
        with patch.object(version, 'selected_version',
                          side_effect=['3.2.7', '3.2.8', '3.3.0']) as select:
            expanded = version.resolve_policy(policy, ROOT)
        select.assert_called_once_with(ROOT)
        fixed = expanded['required']['versioninfo']['fixed']
        strings = expanded['required']['versioninfo']['strings']
        for kind in ('File', 'Product'):
            self.assertEqual('0x00030002', fixed['dw' + kind + 'VersionMS'])
            self.assertEqual('0x00070000', fixed['dw' + kind + 'VersionLS'])
            self.assertEqual('3.2.7', strings[kind + 'Version'])
        self.assertEqual(original, policy)

    def test_invalid_versions_fail_closed(self):
        for value in ('', '3.2', '3.2.6.0', '03.2.6', '3.2.6-TALVOR', '-1.2.3',
                      '65536.0.0', '3.2.6\n', '3.2.6;anything', '3.2.x'):
            with self.subTest(value=value), patch.dict('os.environ', {version.ENVIRONMENT: value}):
                with self.assertRaises(ValueError):
                    version.selected_version()
        self.assertEqual((65535, 65535, 65535, 0), version.components('65535.65535.65535'))

    def test_generated_header_updates_on_version_change(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'generated/project_version.h'
            version.write_header(path, '3.2.6')
            version.write_header(path, '3.3.0')
            self.assertIn('#define LAUREONDO_VERSION_NUMERIC 3,3,0,0', path.read_text())
            self.assertIn('"3.3.0\\0"', path.read_text())
            self.assertNotIn('3.2.6', path.read_text())

    def test_source_archive_generates_its_version_without_git_or_external_tools(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'scripts').mkdir()
            (root / 'metadata').mkdir()
            script = root / 'scripts/project_version.py'
            script.write_bytes((ROOT / 'scripts/project_version.py').read_bytes())
            (root / 'metadata/BRANDING_AND_IDENTITY.json').write_text(
                json.dumps({'default_project_version': '5.4.3'}))
            header = root / 'generated/project_version.h'
            environment = {**os.environ, 'PATH': tmp}
            environment.pop(version.ENVIRONMENT, None)
            run = subprocess.run([sys.executable, '-B', str(script), '--header', str(header)],
                                 env=environment, capture_output=True, text=True, timeout=30)
            self.assertEqual(0, run.returncode, run.stderr)
            self.assertEqual('5.4.3', run.stdout.strip())
            self.assertIn('#define LAUREONDO_VERSION_NUMERIC 5,4,3,0', header.read_text())
            self.assertFalse((root / '.git').exists())
    def test_generated_header_rejects_symlink_output_without_mutating_target(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            victim = root / 'victim.txt'
            victim.write_bytes(b'unchanged target')
            output = root / 'version.h'
            try:
                output.symlink_to(victim)
            except (OSError, NotImplementedError):
                self.skipTest('Host does not permit file symlinks')
            with self.assertRaisesRegex(ValueError, 'symlinks'):
                version.write_header(output, '3.2.6')
            self.assertTrue(output.is_symlink())
            self.assertEqual(b'unchanged target', victim.read_bytes())

    def test_generated_header_rejects_hardlink_output_without_mutating_target(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            victim = root / 'victim.txt'
            victim.write_bytes(b'unchanged target')
            output = root / 'version.h'
            try:
                os.link(victim, output)
            except (OSError, NotImplementedError):
                self.skipTest('Host does not permit file hardlinks')
            with self.assertRaisesRegex(ValueError, 'exclusive regular file'):
                version.write_header(output, '3.2.6')
            self.assertEqual(b'unchanged target', victim.read_bytes())
            self.assertEqual(b'unchanged target', output.read_bytes())

    def test_generated_header_rejects_symlinked_parent_before_directory_creation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            outside = root / 'outside'
            outside.mkdir()
            alias = root / 'redirect'
            try:
                alias.symlink_to(outside, target_is_directory=True)
            except (OSError, NotImplementedError):
                self.skipTest('Host does not permit directory symlinks')
            with self.assertRaisesRegex(ValueError, 'symlinks'):
                version.write_header(alias / 'nested/version.h', '3.2.6')
            self.assertFalse((outside / 'nested').exists())

    def test_generated_header_rejects_directory_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'version.h'
            path.mkdir()
            with self.assertRaisesRegex(ValueError, 'exclusive regular file'):
                version.write_header(path, '3.2.6')
            self.assertTrue(path.is_dir())

    def test_fixture_policy_is_independent_and_input_policy_is_not_mutated(self):
        fixture = json.loads((ROOT / 'tests/support/golden/metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())
        original = copy.deepcopy(fixture)
        product = json.loads((ROOT / 'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())
        before = copy.deepcopy(product)
        with patch.dict('os.environ', {version.ENVIRONMENT: '3.3.0'}):
            self.assertEqual(original, version.resolve_policy(fixture))
            version.resolve_policy(product)
        self.assertEqual(before, product)

    def test_stale_version_and_branding_are_rejected(self):
        with patch.dict('os.environ', {version.ENVIRONMENT: '3.2.7'}):
            required = load_target(ROOT, 'MUSTAVEX_GOLDEN_METADATA.json')['required']
        for field in ('FileVersion', 'ProductVersion', 'FileDescription', 'ProductName', 'LegalCopyright'):
            with self.subTest(field=field):
                stale = copy.deepcopy(required)
                stale['versioninfo']['strings'][field] = 'stale identity'
                self.assertTrue(validate_versioninfo_root(resource_node(stale), required))
        for field in ('dwFileVersionMS', 'dwFileVersionLS', 'dwProductVersionMS', 'dwProductVersionLS'):
            with self.subTest(field=field):
                stale = copy.deepcopy(required)
                stale['versioninfo']['fixed'][field] = '0x00000000'
                self.assertTrue(validate_versioninfo_root(resource_node(stale), required))

    def release_tree(self, root):
        (root / 'metadata').mkdir()
        (root / 'metadata/BRANDING_AND_IDENTITY.json').write_text(json.dumps({
            'default_project_version': '3.2.7', 'retained_identity': 'unchanged'}))
        (root / 'metadata/SOURCE_FILES.json').write_text('{"retained_source_pins":true}\n')
        (root / 'CHANGELOG.md').write_text('# Changelog\n\n### This PR\n\n- Existing entry.\n')
        write_manifest(root)
        return {'tag_name': '3.2.8-FORMADRIL', 'draft': False, 'prerelease': False,
                'published_at': '2026-11-13T14:00:00Z'}

    def test_stable_publication_prepares_only_version_changelog_and_manifest(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            release = self.release_tree(root)
            pins = (root / 'metadata/SOURCE_FILES.json').read_bytes()
            result = version.refresh_release_default(root, release, release['tag_name'])
            self.assertTrue(result['changed'])
            self.assertEqual('release/default-version-3-2-8', result['branch'])
            identity = json.loads((root / 'metadata/BRANDING_AND_IDENTITY.json').read_text())
            self.assertEqual('unchanged', identity['retained_identity'])
            self.assertEqual('3.2.8-FORMADRIL', identity['default_project_version_release_tag'])
            with patch.dict('os.environ', {}, clear=True):
                self.assertEqual('3.2.8', version.selected_version(root))
            with patch.dict('os.environ', {version.ENVIRONMENT: '3.3.0'}):
                self.assertEqual('3.3.0', version.selected_version(root))
            self.assertIn('- Existing entry.', (root / 'CHANGELOG.md').read_text())
            self.assertEqual(pins, (root / 'metadata/SOURCE_FILES.json').read_bytes())
            self.assertEqual([], verify_manifest(root))
            before = {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()}
            self.assertFalse(version.refresh_release_default(root, release)['changed'])
            self.assertEqual(before, {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()})

    def test_unpublished_prerelease_and_invalid_tags_cannot_change_the_default(self):
        for field, value in [('draft', True), ('prerelease', True), ('draft', 0),
                             ('published_at', None), ('published_at', 'invalid'),
                             ('published_at', '2026-11-13T14:00:00'),
                             ('tag_name', '3.2.8-WRONG'), ('tag_name', '../main')]:
            with self.subTest(field=field, value=value), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                release = self.release_tree(root)
                before = {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()}
                release[field] = value
                with self.assertRaises(ValueError):
                    version.refresh_release_default(root, release)
                self.assertEqual(before, {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()})

    def test_old_event_and_older_release_do_not_roll_back_the_default(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            release = self.release_tree(root)
            before = {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()}
            self.assertFalse(version.refresh_release_default(root, release, '3.2.7-ERINTHAR')['changed'])
            release['tag_name'] = '3.2.6-TALVOR'
            self.assertFalse(version.refresh_release_default(root, release)['changed'])
            self.assertEqual(before, {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()})

    def test_damaged_source_manifest_blocks_the_update_before_any_write(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            release = self.release_tree(root)
            (root / 'CHANGELOG.md').write_text('unexpected source change\n')
            before = {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()}
            with self.assertRaisesRegex(ValueError, 'unverified source tree'):
                version.refresh_release_default(root, release)
            self.assertEqual(before, {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()})


if __name__ == '__main__':
    unittest.main()
