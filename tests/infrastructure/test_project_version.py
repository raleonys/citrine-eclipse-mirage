# SPDX-License-Identifier: GPL-3.0-only
"""Release stamping regressions; synthetic resources are not native acceptance."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import project_version as version
from golden_policy import load_target
from verify_mustavex_golden_metadata import validate_versioninfo_root


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
        with patch.dict('os.environ', {}, clear=True):
            self.assertEqual('3.2.6', version.selected_version())
        with patch.dict('os.environ', {version.ENVIRONMENT: '3.2.7'}):
            policy = load_target(ROOT, 'MUSTAVEX_GOLDEN_METADATA.json')['required']
            self.assertEqual('0x00030002', policy['versioninfo']['fixed']['dwFileVersionMS'])
            self.assertEqual('0x00070000', policy['versioninfo']['fixed']['dwFileVersionLS'])
            self.assertEqual('0x00070000', policy['versioninfo']['fixed']['dwProductVersionLS'])
            self.assertEqual('3.2.7', policy['versioninfo']['strings']['FileVersion'])
            self.assertEqual('3.2.7', policy['versioninfo']['strings']['ProductVersion'])
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
            self.assertIn('#define CEM_VERSION_NUMERIC 3,3,0,0', path.read_text())
            self.assertIn('"3.3.0\\0"', path.read_text())
            self.assertNotIn('3.2.6', path.read_text())

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


if __name__ == '__main__':
    unittest.main()
