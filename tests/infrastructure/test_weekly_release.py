# SPDX-License-Identifier: GPL-3.0-only

"""Release-candidate regressions: source binding, revision ownership and draft-only retry."""
from copy import deepcopy
import json
from pathlib import Path
import shutil
import sys
import tempfile
import subprocess
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import weekly_release as release
from run_source_qa import CHECKS

SOURCE = 'a' * 40
OTHER = 'b' * 40


def candidate(tag='3.2.6-TALVOR', source=SOURCE, status='complete', draft=True, identity=1):
    return {'id': identity, 'tag_name': tag, 'draft': draft,
            'body': release.marker(source, status), 'target_commitish': source,
            'assets': [], 'html_url': 'https://example.invalid/draft'}


class FakeGitHub:
    def __init__(self):
        self.items = []
        self.writes = []
        self.ref_reserved = False
        self.fail_upload = False
        self.publish_before_upload = False
        self.missing_asset = False
        self.corrupt_digest = False

    def releases(self):
        return deepcopy(self.items)

    def api(self, endpoint, data=None):
        if endpoint.startswith('git/matching-refs/'):
            return [{'ref': 'refs/tags/' + endpoint.rsplit('/', 1)[-1]}] if self.ref_reserved else []
        if endpoint == 'releases/generate-notes':
            return {'body': '## Changes\n\nOwner-reviewed changes.'}
        if endpoint == 'releases':
            self.writes.append(deepcopy(data))
            self.items.append({**deepcopy(data), 'id': 10, 'assets': [],
                               'html_url': 'https://example.invalid/draft'})
            return deepcopy(self.items[-1])
        if endpoint.startswith('releases/'):
            if self.publish_before_upload:
                self.items[-1]['draft'] = False
            return deepcopy(self.items[-1])
        raise AssertionError(endpoint)

    def update(self, identity, body):
        self.writes.append({'body': body})
        self.items[-1]['body'] = body
        return deepcopy(self.items[-1])

    def upload(self, tag, paths):
        if self.fail_upload:
            raise OSError('injected upload interruption')
        names = {p.name for p in paths}
        retained = [a for a in self.items[-1]['assets'] if a['name'] not in names]
        self.items[-1]['assets'] = retained + [
            {'name': p.name, 'size': p.stat().st_size,
             'digest': 'sha256:' + ('0' * 64 if self.corrupt_digest else release.sha(p))}
            for p in paths]
        if self.missing_asset:
            self.items[-1]['assets'].pop()


class PlannerTests(unittest.TestCase):
    def test_first_approved_release(self):
        self.assertEqual('3.2.6-TALVOR', release.plan([], SOURCE)['tag'])

    def test_no_duplicate_unchanged_source(self):
        self.assertFalse(release.plan([candidate()], SOURCE)['ready'])

    def test_new_commit_advances_to_327(self):
        self.assertEqual('3.2.7-ERINTHAR', release.plan([candidate()], OTHER)['tag'])

    def test_329_rollover(self):
        self.assertEqual('3.3.0-VESPERKAI', release.plan([candidate('3.2.9-CETRION')], OTHER)['tag'])

    def test_399_rollover(self):
        self.assertEqual('4.0.0', release.successor('3.9.9'))

    def test_no_unapproved_next_version(self):
        with self.assertRaisesRegex(ValueError, 'owner-approved'):
            release.plan([candidate('3.5.9-COLOZHEM')], OTHER)

    def test_no_unrelated_tag_collision(self):
        other = {'tag_name': '3.2.7-ERINTHAR', 'body': '', 'draft': False}
        with self.assertRaisesRegex(ValueError, 'reserved'):
            release.plan([candidate(), other], OTHER)

    def test_published_source_does_not_duplicate(self):
        self.assertFalse(release.plan([candidate(draft=False)], SOURCE)['ready'])

    def test_pending_draft_resumes(self):
        x = release.plan([candidate(status='preparing')], SOURCE)
        self.assertEqual(('3.2.6-TALVOR', 1), (x['tag'], x['existing_id']))

    def test_published_incomplete_draft_blocks(self):
        with self.assertRaisesRegex(ValueError, 'published'):
            release.plan([candidate(status='preparing', draft=False)], SOURCE)

    def test_manual_override_must_be_exact_next(self):
        self.assertEqual('3.2.6-TALVOR', release.plan([], SOURCE, '3.2.6-TALVOR')['tag'])
        with self.assertRaisesRegex(ValueError, 'exact next'):
            release.plan([], SOURCE, '3.2.7-ERINTHAR')

    def test_registry_and_archive_contract(self):
        names = release.validate_registry(release.load_registry())
        self.assertEqual(34, len(names))
        self.assertEqual(('TALVOR', 'VESPERKAI', 'COLOZHEM'),
                         (names['3.2.6'], names['3.3.0'], names['3.5.9']))
        self.assertEqual(('Citrine_Eclipse_Mirage_3.2.6_TALVOR_source.zip',
                          'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip'),
                         release.asset_names('3.2.6-TALVOR'))

    def test_historical_root_reuse_rejected(self):
        p = deepcopy(release.load_registry())
        p['approved_codenames']['3.2.6'] = 'SQUALERIS'
        with self.assertRaisesRegex(ValueError, 'collision'):
            release.validate_registry(p)

    def test_old_revision_tags_and_bad_semver_rejected(self):
        for tag in ('3.2.10-TALVOR', '3.10.0-TALVOR', '3.2.6-TALVOR-R25', '../3.2.6-TALVOR'):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                release.plan([], SOURCE, tag)

    def test_ambiguous_markers_fail_closed(self):
        r = candidate()
        r['body'] += '\n' + r['body']
        with self.assertRaisesRegex(ValueError, 'Ambiguous'):
            release.plan([r], SOURCE)


class TransportTests(unittest.TestCase):
    def test_streamed_pages_work_without_newer_gh_cli_flags(self):
        cp = subprocess.CompletedProcess([], 0, '[{"id":1}]\n[{"id":2}]\n[]\n', '')
        with patch.object(release.subprocess, 'run', return_value=cp) as run:
            self.assertEqual([{'id': 1}, {'id': 2}], release.GitHub('owner/repository').releases())
        self.assertIn('--paginate', run.call_args.args[0])
        self.assertNotIn('--slurp', run.call_args.args[0])

    def test_api_errors_remain_visible_and_do_not_become_an_empty_history(self):
        cp = subprocess.CompletedProcess([], 1, '', 'HTTP 403: Resource not accessible')
        with patch.object(release.subprocess, 'run', return_value=cp), self.assertRaisesRegex(RuntimeError, '403'):
            release.GitHub('owner/repository').releases()

    def test_invalid_page_shape_cannot_hide_a_revision_collision(self):
        cp = subprocess.CompletedProcess([], 0, '{"error":"unexpected object"}', '')
        with patch.object(release.subprocess, 'run', return_value=cp), self.assertRaises(ValueError):
            release.GitHub('owner/repository').releases()

    def test_draft_body_update_does_not_change_its_publication_state(self):
        cp = subprocess.CompletedProcess([], 0, '{"draft":true}', '')
        with patch.object(release.subprocess, 'run', return_value=cp) as run:
            release.GitHub('owner/repository').update(1, 'complete draft')
        self.assertEqual({'body': 'complete draft'}, json.loads(run.call_args.kwargs['input']))


class PayloadTests(unittest.TestCase):
    def setUp(self):
        version_selection = patch.dict('os.environ', {'CITRINE_ECLIPSE_MIRAGE_VERSION': '3.2.6'})
        version_selection.start()
        self.addCleanup(version_selection.stop)
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name).resolve()
        self.payload = self.base / 'payload'
        self.payload.mkdir()
        self.manifest = release.sha(ROOT / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
        self.p = {'ready': True, 'tag': '3.2.6-TALVOR', 'source_sha': SOURCE, 'existing_id': None}
        release.write(self.payload / 'plan.json', self.p)
        release.write(self.payload / 'SOURCE_QA.json', {
            'result': 'PASS_SOURCE_QA',
            'checks': [{'check': name, 'exit_code': 0} for name, _ in CHECKS]})
        self.xml(self.payload / 'PORTABLE_CTEST.xml')
        self.profile('native')
        self.profile('production')

    def xml(self, path, body=''):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text('<testsuite tests="1" failures="0"><testcase name="fixture">'
                        + body + '</testcase></testsuite>')

    def profile(self, name):
        folder = self.payload / name
        folder.mkdir()
        release.write(folder / 'EVIDENCE_IDENTITY.json', {
            'source_sha': SOURCE, 'source_manifest_sha256': self.manifest, 'profile': name})
        receipt = {'mode': 'Native' if name == 'native' else 'Golden',
                   'configuration': 'Release', 'build_succeeded': True,
                   'validation_result': 'PASS_SELECTED_PROFILE', 'validation_exit_code': 0,
                   'source_manifest_sha256': self.manifest, 'dll_sha256': 'c' * 64,
                   'compiler': 'C:/fixture/cl.exe', 'project_version': '3.2.6'}
        for file in ('BUILD_TOOLCHAIN_PROVENANCE.log', 'PUBLIC_DLL_ABI_CHECK.json', 'DLL_RESOURCE_CHECK.json'):
            (folder / file).write_text('fixture evidence')
        if name == 'native':
            self.xml(folder / 'CTEST_RESULTS.xml')
        else:
            dist = folder / 'dist'
            dist.mkdir()
            dll = dist / 'citrine_eclipse_mirage.dll'
            dll.write_bytes(b'MZ fixture; not real MSVC or game acceptance')
            release.sidecar(dll)
            digest = release.sha(dll)
            golden = {'result': 'GOLDEN_PASS', 'sha256': digest,
                      'steps': [{'stage': stage, 'exit_code': 0} for stage in release.ORDER],
                      'runtime_check': {'exit_code': 0, 'sha256_before': digest, 'sha256_after': digest}}
            release.write(folder / 'GOLDEN_REPORT.json', golden)
            release.write(dist / 'DIST_RECEIPT.json', {
                **golden, 'source_manifest_sha256': self.manifest, 'input_sha256': 'c' * 64})
            receipt['dist_sha256'] = digest
        release.write(folder / 'BUILD_RECEIPT.json', receipt)

    def validate(self, name):
        return release.validate_profile(self.payload / name, name, self.manifest, SOURCE)

    def test_stale_or_missing_release_version_is_rejected(self):
        path = self.payload / 'native/BUILD_RECEIPT.json'
        receipt = release.load(path)
        for value in (None, '3.2.5', '3.2.7', '3.2.6-TALVOR'):
            release.write(path, {**receipt, 'project_version': value})
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, 'version'):
                self.validate('native')

    def test_next_release_requires_next_version_in_both_builds(self):
        for name in ('native', 'production'):
            with self.assertRaisesRegex(ValueError, 'version'):
                release.validate_profile(self.payload / name, name, self.manifest, SOURCE, '3.2.7')
            path = self.payload / name / 'BUILD_RECEIPT.json'
            release.write(path, {**release.load(path), 'project_version': '3.2.7'})
            release.validate_profile(self.payload / name, name, self.manifest, SOURCE, '3.2.7')

    def test_capture_uses_current_receipts_and_only_declared_dist_members(self):
        root=self.base/'capture-source'
        (root/'metadata').mkdir(parents=True)
        shutil.copyfile(ROOT/'metadata/ACTIVE_SOURCE_MANIFEST.json',
                        root/'metadata/ACTIVE_SOURCE_MANIFEST.json')
        build=root/'build/production'
        (build/'golden').mkdir(parents=True)
        fixture=self.payload/'production'
        for name in ('BUILD_RECEIPT.json','BUILD_TOOLCHAIN_PROVENANCE.log',
                     'PUBLIC_DLL_ABI_CHECK.json','DLL_RESOURCE_CHECK.json'):
            shutil.copyfile(fixture/name,build/name)
        shutil.copyfile(fixture/'GOLDEN_REPORT.json',build/'golden/GOLDEN_REPORT.json')
        shutil.copytree(fixture/'dist',root/'dist')
        historical=root/'dist/LOCAL_DIST_RECEIPT.json'
        historical.write_bytes(b'previous receipt retained outside the new package')
        output=self.base/'captured'
        with patch.object(release,'head',return_value=SOURCE):
            release.capture(root,'production',build,output,SOURCE)
        self.assertEqual({'citrine_eclipse_mirage.dll','citrine_eclipse_mirage.dll.sha256','DIST_RECEIPT.json'},
                         {path.name for path in (output/'production/dist').iterdir()})
        self.assertEqual(b'previous receipt retained outside the new package',historical.read_bytes())
        self.assertEqual(self.validate('production'),
                         release.validate_profile(output/'production','production',self.manifest,SOURCE))

    def test_renamed_build_receipt_is_required(self):
        folder=self.payload/'native'
        (folder/'BUILD_RECEIPT.json').rename(folder/'LOCAL_BUILD_RECEIPT.json')
        with self.assertRaises(FileNotFoundError):
            self.validate('native')

    def test_profile_receipt_must_match_the_exact_source(self):
        path = self.payload / 'native/EVIDENCE_IDENTITY.json'
        data = release.load(path)
        data['source_sha'] = OTHER
        release.write(path, data)
        with self.assertRaisesRegex(ValueError, 'different source'):
            self.validate('native')

    def test_manifest_drift_is_not_native_acceptance(self):
        path = self.payload / 'native/BUILD_RECEIPT.json'
        data = release.load(path)
        data['source_manifest_sha256'] = '0' * 64
        release.write(path, data)
        with self.assertRaises(ValueError):
            self.validate('native')

    def test_failed_or_skipped_native_tests_cannot_create_a_candidate(self):
        for body in ('<failure/>', '<error/>', '<skipped/>'):
            self.xml(self.payload / 'native/CTEST_RESULTS.xml', body)
            with self.subTest(body=body), self.assertRaises(ValueError):
                self.validate('native')

    def assert_junit_rejected_by_candidate_paths(self, xml):
        native = self.payload / 'native/CTEST_RESULTS.xml'
        native.write_text(xml, encoding='utf-8')
        with self.assertRaises(ValueError):
            release.junit(native)
        with self.assertRaises(ValueError):
            self.validate('native')
        self.xml(native)
        (self.payload / 'PORTABLE_CTEST.xml').write_text(xml, encoding='utf-8')
        output = self.base / 'assets'
        with patch.object(release, 'head', return_value=SOURCE), self.assertRaises(ValueError):
            release.bundle(ROOT, self.payload, output)
        self.assertFalse(output.exists())

    def test_junit_valid_flat_and_nested_suites_count_each_case_once(self):
        leaf = '<testsuite tests="1"><testcase name="a"/></testsuite>'
        nested = ('<testsuite tests="3"><testcase name="direct"/>'
                  '<testsuite tests="2"><testcase name="a"/><testcase name="b"/>'
                  '</testsuite></testsuite>')
        valid = [
            ('single', leaf, 1),
            ('aggregate', '<testsuites tests="2">' + leaf * 2 + '</testsuites>', 2),
            ('optional-aggregate', '<testsuites>' + leaf * 2 + '</testsuites>', 2),
            ('nested-single', nested, 3),
            ('nested-aggregate', '<testsuites tests="4">' + nested + leaf + '</testsuites>', 4),
            ('empty-sibling', '<testsuites>' + leaf + '<testsuite tests="0"/></testsuites>', 1),
            ('metadata', '<testsuite tests="01"><properties><property name="x" value="y"/>'
             '</properties><testcase/><system-out>&lt;testcase/&gt;</system-out></testsuite>', 1),
        ]
        path = self.payload / 'native/CTEST_RESULTS.xml'
        for name, xml, expected in valid:
            with self.subTest(valid=name):
                path.write_text(xml, encoding='utf-8')
                self.assertEqual(expected, release.junit(path))
                self.assertEqual({'tests': expected}, self.validate('native'))

    def test_junit_rejects_inconsistent_counts_in_native_and_portable_candidates(self):
        leaf = '<testsuite tests="1"><testcase/></testsuite>'
        invalid_counts = ('', '-1', '+1', ' 1', '1 ', '1.0', '0x1', 'one', '\u0661', '\u00b2', '0', '2')
        for count in invalid_counts:
            for scope, xml in (
                    ('suite', '<testsuite tests="' + count + '"><testcase/></testsuite>'),
                    ('aggregate', '<testsuites tests="' + count + '">' + leaf + '</testsuites>')):
                with self.subTest(count=count, scope=scope):
                    self.assert_junit_rejected_by_candidate_paths(xml)
        for xml in (
                '<testsuite><testcase/></testsuite>',
                '<testsuite tests="1"><testcase/><testcase/></testsuite>',
                '<testsuite tests="1">' + leaf + '<testcase/></testsuite>',
                '<testsuite tests="1"><testsuite tests="2"><testcase/></testsuite></testsuite>',
                '<testsuite tests="1">' + leaf + '<testsuite tests="99"/></testsuite>',
                '<testsuite tests="1">' + leaf + '<testsuite/></testsuite>'):
            with self.subTest(xml=xml):
                self.assert_junit_rejected_by_candidate_paths(xml)

    def test_junit_rejects_wrong_roots_and_hidden_or_orphan_nodes(self):
        leaf = '<testsuite tests="1"><testcase/></testsuite>'
        for xml in (
                '<report>' + leaf + '</report>', '<testcase/>', '<testsuites/>',
                '<testsuite tests="0"/>',
                '<testsuites tests="2">' + leaf + '<testcase/></testsuites>',
                '<testsuites><wrapper>' + leaf + '</wrapper></testsuites>',
                '<testsuite tests="1"><wrapper><testcase/></wrapper></testsuite>',
                '<testsuite tests="2"><testcase><testcase/></testcase></testsuite>',
                '<testsuite tests="1"><wrapper>' + leaf + '</wrapper></testsuite>',
                '<testsuite tests="1"><testcase><testsuite tests="0"/></testcase></testsuite>',
                '<testsuite tests="1"><testcase/><wrapper><testsuite tests="0"/></wrapper></testsuite>',
                '<testsuite tests="1"><testsuites>' + leaf + '</testsuites></testsuite>'):
            with self.subTest(xml=xml):
                self.assert_junit_rejected_by_candidate_paths(xml)

    def test_junit_nested_failure_statuses_still_reject_candidates(self):
        for marker in ('failure', 'error', 'skipped'):
            xml = ('<testsuites tests="1"><testsuite tests="1"><testsuite tests="1">'
                   '<testcase><' + marker + '/></testcase></testsuite></testsuite></testsuites>')
            with self.subTest(marker=marker):
                self.assert_junit_rejected_by_candidate_paths(xml)
        for status in ('failures', 'errors', 'skipped', 'disabled'):
            xml = ('<testsuites tests="1"><testsuite tests="1"><testsuite tests="1" '
                   + status + '="1"><testcase/></testsuite></testsuite></testsuites>')
            with self.subTest(status=status):
                self.assert_junit_rejected_by_candidate_paths(xml)

    def test_junit_deep_suites_do_not_recurse_or_double_count(self):
        depth = 1200
        xml = '<testsuite tests="1">' * depth + '<testcase/>' + '</testsuite>' * depth
        path = self.payload / 'native/CTEST_RESULTS.xml'
        path.write_text(xml, encoding='utf-8')
        self.assertEqual(1, release.junit(path))
        self.assertEqual({'tests': 1}, self.validate('native'))

    def test_empty_junit_is_not_a_successful_native_run(self):
        (self.payload / 'native/CTEST_RESULTS.xml').write_text('<testsuite tests="0"/>')
        with self.assertRaises(ValueError):
            self.validate('native')

    def test_wrong_profile_and_non_msvc_receipts_are_rejected(self):
        path = self.payload / 'native/BUILD_RECEIPT.json'
        original = release.load(path)
        for change in ({'mode': 'Compile'}, {'compiler': '/fixture/clang++'}, {'validation_exit_code': 1}):
            release.write(path, {**original, **change})
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate('native')

    def test_changed_dll_is_rejected_even_if_it_has_a_new_sidecar(self):
        dll = self.payload / 'production/dist/citrine_eclipse_mirage.dll'
        dll.write_bytes(b'changed DLL')
        release.sidecar(dll)
        with self.assertRaises(ValueError):
            self.validate('production')

    def test_missing_golden_stage_and_missing_runtime_check_are_rejected(self):
        path = self.payload / 'production/GOLDEN_REPORT.json'
        original = release.load(path)
        for change in ({'steps': original['steps'][:-1]}, {'runtime_check': {}}):
            release.write(path, {**original, **change})
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate('production')

    def test_bundle_ships_exactly_dll_dll_checksum_and_project_license(self):
        release.package(ROOT, self.payload / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR_source.zip')
        output = self.base / 'assets'
        with patch.object(release, 'head', return_value=SOURCE):
            receipt = release.bundle(ROOT, self.payload, output)
        self.assertFalse(receipt['publication'])
        self.assertEqual('DRAFT',receipt['publication_status'])
        self.assertNotIn('classification',receipt)
        self.assertEqual('NOT_RUN', receipt['game_acceptance'])
        binary = output / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip'
        self.assertEqual({binary.name, binary.name + '.sha256'},
                         {path.name for path in output.iterdir()})
        release.verify_sidecar(binary)
        self.assertEqual(release.sha(binary), receipt['binary_zip_sha256'])
        self.assertEqual(receipt, release.load(self.payload / 'RELEASE_RECEIPT.json'))
        release.verify_sidecar(self.payload / 'RELEASE_RECEIPT.json')
        self.assertTrue((self.payload / 'native/CTEST_RESULTS.xml').is_file())
        self.assertTrue((self.payload / 'production/GOLDEN_REPORT.json').is_file())
        with zipfile.ZipFile(output / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip') as archive:
            self.assertEqual({'citrine_eclipse_mirage.dll', 'citrine_eclipse_mirage.dll.sha256', 'LICENSE'},
                             set(archive.namelist()))
            self.assertEqual((self.payload / 'production/dist/citrine_eclipse_mirage.dll').read_bytes(),
                             archive.read('citrine_eclipse_mirage.dll'))
            self.assertEqual((ROOT / 'LICENSE').read_bytes(), archive.read('LICENSE'))
            self.assertEqual(release.sha(self.payload / 'production/dist/citrine_eclipse_mirage.dll')
                             + '  citrine_eclipse_mirage.dll',
                             archive.read('citrine_eclipse_mirage.dll.sha256').decode('utf-8').strip())

    def test_invalid_runtime_member_set_does_not_claim_bundle_output(self):
        # Limit this fixture to output preflight; the successful real ZIP path
        # and source manifest are covered by the regular bundle test above.
        source_name, _ = release.asset_names(self.p['tag'])
        source = self.payload / source_name
        source.write_bytes(b'synthetic source package fixture')
        release.sidecar(source)
        extra = self.payload / 'production/dist/unexpected.bin'
        extra.write_bytes(b'extra runtime payload')
        output = self.base / 'assets'
        with patch.object(release, 'head', return_value=SOURCE), \
                patch.object(release, 'check', return_value=[]):
            with self.assertRaisesRegex(ValueError, 'Unexpected runtime payload'):
                release.bundle(ROOT, self.payload, output)
            self.assertFalse(output.exists())
            extra.unlink()
            receipt = release.bundle(ROOT, self.payload, output)
        self.assertFalse(receipt['publication'])
        self.assertTrue((output / release.asset_names(self.p['tag'])[1]).is_file())

    def test_symlinked_runtime_input_does_not_claim_bundle_output(self):
        source_name, _ = release.asset_names(self.p['tag'])
        source = self.payload / source_name
        source.write_bytes(b'synthetic source package fixture')
        release.sidecar(source)
        original = self.payload / 'production/dist/citrine_eclipse_mirage.dll'
        target = self.base / 'borrowed-dll'
        target.write_bytes(original.read_bytes())
        original.unlink()
        try:
            original.symlink_to(target)
        except (OSError, NotImplementedError):
            self.skipTest('Host does not permit symlinks')
        output = self.base / 'assets'
        with patch.object(release, 'head', return_value=SOURCE), \
                patch.object(release, 'check', return_value=[]), \
                self.assertRaisesRegex(ValueError, 'cannot contain symlinks'):
            release.bundle(ROOT, self.payload, output)
        self.assertFalse(output.exists())
        self.assertEqual(b'MZ fixture; not real MSVC or game acceptance', target.read_bytes())

    def test_failed_portable_run_stops_before_creating_assets(self):
        self.xml(self.payload / 'PORTABLE_CTEST.xml', '<failure/>')
        with patch.object(release, 'head', return_value=SOURCE), self.assertRaises(ValueError):
            release.bundle(ROOT, self.payload, self.base / 'assets')
        self.assertFalse((self.base / 'assets').exists())


class DraftTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.assets = Path(self.temporary.name).resolve() / 'assets'
        self.assets.mkdir()
        self.receipt = self.assets.parent / 'evidence/RELEASE_RECEIPT.json'
        self.p = {'ready': True, 'tag': '3.2.6-TALVOR', 'source_sha': SOURCE, 'existing_id': None}
        binary = self.assets / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip'
        binary.write_bytes(b'fixture package')
        release.sidecar(binary)
        release.write(self.receipt, {
            'tag': '3.2.6-TALVOR', 'source_sha': SOURCE, 'publication_status': 'DRAFT',
            'publication': False, 'game_acceptance': 'NOT_RUN',
            'binary_zip_sha256': release.sha(binary)})
        release.sidecar(self.receipt)
        self.github = FakeGitHub()

    def publish(self):
        return release.publish_draft(self.github, self.p, self.assets, self.receipt)

    def test_all_uploaded_assets_finish_a_draft_without_publishing(self):
        result = self.publish()
        self.assertTrue(result['draft'])
        self.assertEqual('complete', release.state(result)['status'])
        self.assertTrue(self.github.writes[0]['draft'])
        self.assertNotIn('draft', self.github.writes[-1])
        self.assertEqual({'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip',
                          'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip.sha256'},
                         {asset['name'] for asset in result['assets']})

    def test_invalid_publication_status_cannot_prepare_a_draft(self):
        path=self.receipt
        original=release.load(path)
        for status in (None,'PUBLISHED','COMPLETE'):
            release.write(path,{**original,'publication_status':status})
            release.sidecar(path)
            with self.subTest(status=status),self.assertRaisesRegex(ValueError,'Draft receipt/source mismatch'):
                self.publish()
            self.assertEqual([],self.github.writes)

    def test_completed_release_notes_are_ready_for_owner_publication(self):
        result = self.publish()
        body = result['body']
        self.assertTrue(result['draft'])
        self.assertEqual('complete', release.state(result)['status'])
        for leftover in ('# Draft candidate', 'publish this draft',
                         'Game acceptance is NOT RUN', 'Asset preparation is incomplete'):
            self.assertNotIn(leftover, body)
        self.assertIn('Source: `' + SOURCE + '`', body)
        self.assertIn('Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip', body)
        self.assertIn('## Changes\n\nOwner-reviewed changes.', body)
        self.assertIn('Source code (zip/tar.gz)', body)
        # Publishing changes GitHub's draft flag; the already prepared notes
        # must remain suitable for readers without another metadata rewrite.
        self.github.items[-1]['draft'] = False
        self.assertEqual(body, self.github.items[-1]['body'])
        self.assertEqual('NOT_RUN', release.load(self.receipt)['game_acceptance'])

    def test_upload_interruption_retains_incomplete_draft_for_retry(self):
        self.github.fail_upload = True
        with self.assertRaises(OSError):
            self.publish()
        self.assertEqual('preparing', release.state(self.github.items[0])['status'])
        self.assertIn('Asset preparation is incomplete', self.github.items[0]['body'])
        self.github.fail_upload = False
        self.assertEqual('complete', release.state(self.publish())['status'])
        self.assertEqual(1, len(self.github.items))
        self.assertNotIn('Asset preparation is incomplete', self.github.items[0]['body'])
        self.assertNotIn('# Draft candidate', self.github.items[0]['body'])

    def test_published_release_is_never_modified(self):
        self.github.items = [candidate(draft=False)]
        with self.assertRaisesRegex(ValueError, 'published'):
            self.publish()
        self.assertEqual([], self.github.writes)

    def test_repeated_complete_candidate_does_not_replace_its_assets(self):
        self.publish()
        writes = len(self.github.writes)
        self.publish()
        self.assertEqual(writes, len(self.github.writes))

    def test_existing_git_tag_cannot_be_retargeted(self):
        self.github.ref_reserved = True
        with self.assertRaisesRegex(ValueError, 'Git tag'):
            self.publish()
        self.assertEqual([], self.github.writes)

    def test_changed_draft_target_cannot_receive_another_commits_assets(self):
        existing = candidate(status='preparing')
        existing['target_commitish'] = OTHER
        self.github.items = [existing]
        with self.assertRaisesRegex(ValueError, 'unrelated'):
            self.publish()
        self.assertEqual([], self.github.writes)

    def test_concurrent_owner_publication_stops_further_preparation(self):
        self.github.publish_before_upload = True
        with self.assertRaisesRegex(ValueError, 'published during'):
            self.publish()
        self.assertEqual([], self.github.items[0]['assets'])

    def test_missing_or_changed_remote_assets_cannot_mark_a_draft_complete(self):
        for key in ('missing_asset', 'corrupt_digest'):
            self.github = FakeGitHub()
            setattr(self.github, key, True)
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.publish()
            self.assertEqual('preparing', release.state(self.github.items[0])['status'])

    def test_local_checksum_failure_happens_before_any_external_write(self):
        (self.assets / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip').write_bytes(b'changed package')
        with self.assertRaisesRegex(ValueError, 'Checksum'):
            self.publish()
        self.assertEqual([], self.github.writes)

    def test_different_binary_with_valid_sidecar_cannot_reuse_receipt(self):
        binary = self.assets / 'Citrine_Eclipse_Mirage_3.2.6_TALVOR.zip'
        binary.write_bytes(b'different package with its own checksum')
        release.sidecar(binary)
        with self.assertRaisesRegex(ValueError, 'receipt/binary mismatch'):
            self.publish()
        self.assertEqual([], self.github.writes)

    def test_extra_source_receipt_and_log_assets_are_rejected_before_writes(self):
        for name in ('Citrine_Eclipse_Mirage_3.2.6_TALVOR_source.zip',
                     'RELEASE_RECEIPT.json', 'RELEASE_RECEIPT.json.sha256', 'build.log'):
            path = self.assets / name
            path.write_bytes(b'internal evidence')
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'Unexpected release assets'):
                self.publish()
            path.unlink()
            self.assertEqual([], self.github.writes)

    def test_receipt_inside_public_assets_is_rejected(self):
        path = self.assets / 'RELEASE_RECEIPT.json'
        shutil.copyfile(self.receipt, path)
        release.sidecar(path)
        with self.assertRaisesRegex(ValueError, 'outside release assets'):
            release.publish_draft(self.github, self.p, self.assets, path)
        self.assertEqual([], self.github.writes)

    def test_old_extra_remote_assets_keep_an_incomplete_draft_blocked(self):
        existing = candidate(status='preparing')
        existing['assets'] = [{'name': 'RELEASE_RECEIPT.json', 'size': 1}]
        self.github.items = [existing]
        with self.assertRaisesRegex(ValueError, 'unexpected remote asset set'):
            self.publish()
        self.assertEqual('preparing', release.state(self.github.items[0])['status'])
        self.assertIn('RELEASE_RECEIPT.json', {a['name'] for a in self.github.items[0]['assets']})


if __name__ == '__main__':
    unittest.main(verbosity=2)
