"""Release-candidate regressions: source binding, revision ownership and draft-only retry."""
from copy import deepcopy
import json
from pathlib import Path
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


def candidate(tag='TEST-R01', source=SOURCE, status='complete', draft=True, identity=1):
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
        self.items[-1]['assets'] = [
            {'name': p.name, 'size': p.stat().st_size,
             'digest': 'sha256:' + ('0' * 64 if self.corrupt_digest else release.sha(p))}
            for p in paths]
        if self.missing_asset:
            self.items[-1]['assets'].pop()


class PlannerTests(unittest.TestCase):
    def test_first_schedule_requires_external_bootstrap_not_an_invented_version(self):
        self.assertFalse(release.plan([], SOURCE)['ready'])
        self.assertIn('BOOTSTRAP_REQUIRED', release.plan([], SOURCE)['reason'])
        self.assertEqual('TEST-R07', release.plan([], SOURCE, 'TEST-R07')['tag'])

    def test_unchanged_source_does_not_create_another_candidate(self):
        self.assertFalse(release.plan([candidate()], SOURCE)['ready'])

    def test_new_source_increments_only_the_current_external_revision(self):
        self.assertEqual('TEST-R02', release.plan([candidate()], OTHER)['tag'])

    def test_reserved_revisions_are_not_reused(self):
        reserved = {'id': 0, 'tag_name': 'TEST-R03', 'draft': False, 'body': ''}
        self.assertEqual('TEST-R04', release.plan([candidate(), reserved], OTHER)['tag'])

    def test_published_complete_candidate_still_prevents_duplicate_source(self):
        self.assertFalse(release.plan([candidate(draft=False)], SOURCE)['ready'])

    def test_partial_upload_is_retried_with_the_same_tag(self):
        p = release.plan([candidate(status='preparing')], SOURCE)
        self.assertTrue(p['ready'])
        self.assertEqual(('TEST-R01', 1), (p['tag'], p['existing_id']))

    def test_manually_published_incomplete_candidate_is_not_rewritten(self):
        with self.assertRaisesRegex(ValueError, 'published'):
            release.plan([candidate(status='preparing', draft=False)], SOURCE)

    def test_new_series_is_explicit_and_can_repackage_the_same_commit(self):
        p = release.plan([candidate()], SOURCE, 'NEXT-R01')
        self.assertTrue(p['ready'])
        self.assertEqual('NEXT-R01', p['tag'])

    def test_next_schedule_follows_the_most_recent_selected_series(self):
        items = [candidate(), candidate('NEXT-R01', identity=2)]
        self.assertEqual('NEXT-R02', release.plan(items, OTHER)['tag'])

    def test_revision_exhaustion_cannot_silently_change_the_release_line(self):
        with self.assertRaisesRegex(ValueError, 'R99'):
            release.plan([candidate('TEST-R99')], OTHER)

    def test_tag_collision_cannot_overwrite_another_source(self):
        with self.assertRaisesRegex(ValueError, 'reserved'):
            release.plan([candidate()], OTHER, 'TEST-R01')

    def test_invalid_tags_and_injected_output_lines_are_rejected(self):
        for tag in ('../TEST-R01', 'TEST-R00', 'TEST-R100', 'TEST-R01\nready=true', 'TEST;cmd-R01'):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                release.plan([], SOURCE, tag)

    def test_ambiguous_markers_are_not_silently_accepted(self):
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
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name).resolve()
        self.payload = self.base / 'payload'
        self.payload.mkdir()
        self.manifest = release.sha(ROOT / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
        self.p = {'ready': True, 'tag': 'TEST-R01', 'source_sha': SOURCE, 'existing_id': None}
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
                   'compiler': 'C:/fixture/cl.exe'}
        for file in ('BUILD_TOOLCHAIN_PROVENANCE.log', 'PUBLIC_DLL_ABI_CHECK.json', 'DLL_RESOURCE_CHECK.json'):
            (folder / file).write_text('fixture evidence')
        if name == 'native':
            self.xml(folder / 'CTEST_RESULTS.xml')
        else:
            dist = folder / 'dist'
            dist.mkdir()
            dll = dist / 'preloader_l.dll'
            dll.write_bytes(b'MZ fixture; not real MSVC or game acceptance')
            release.sidecar(dll)
            digest = release.sha(dll)
            golden = {'result': 'LOCAL_GOLDEN_PASS', 'sha256': digest,
                      'steps': [{'stage': stage, 'exit_code': 0} for stage in release.ORDER],
                      'runtime_check': {'exit_code': 0, 'sha256_before': digest, 'sha256_after': digest}}
            release.write(folder / 'GOLDEN_REPORT.json', golden)
            release.write(dist / 'LOCAL_DIST_RECEIPT.json', {
                **golden, 'source_manifest_sha256': self.manifest, 'input_sha256': 'c' * 64})
            receipt['dist_sha256'] = digest
        release.write(folder / 'LOCAL_BUILD_RECEIPT.json', receipt)

    def validate(self, name):
        return release.validate_profile(self.payload / name, name, self.manifest, SOURCE)

    def test_profile_receipt_must_match_the_exact_source(self):
        path = self.payload / 'native/EVIDENCE_IDENTITY.json'
        data = release.load(path)
        data['source_sha'] = OTHER
        release.write(path, data)
        with self.assertRaisesRegex(ValueError, 'different source'):
            self.validate('native')

    def test_manifest_drift_is_not_native_acceptance(self):
        path = self.payload / 'native/LOCAL_BUILD_RECEIPT.json'
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

    def test_empty_junit_is_not_a_successful_native_run(self):
        (self.payload / 'native/CTEST_RESULTS.xml').write_text('<testsuite tests="0"/>')
        with self.assertRaises(ValueError):
            self.validate('native')

    def test_wrong_profile_and_non_msvc_receipts_are_rejected(self):
        path = self.payload / 'native/LOCAL_BUILD_RECEIPT.json'
        original = release.load(path)
        for change in ({'mode': 'Compile'}, {'compiler': '/fixture/clang++'}, {'validation_exit_code': 1}):
            release.write(path, {**original, **change})
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate('native')

    def test_changed_dll_is_rejected_even_if_it_has_a_new_sidecar(self):
        dll = self.payload / 'production/dist/preloader_l.dll'
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

    def test_bundle_contains_verified_runtime_evidence_and_all_checksums(self):
        release.package(ROOT, self.payload / 'TEST-R01_source.zip')
        output = self.base / 'assets'
        with patch.object(release, 'head', return_value=SOURCE):
            receipt = release.bundle(ROOT, self.payload, output)
        self.assertFalse(receipt['publication'])
        self.assertEqual('NOT_RUN', receipt['game_acceptance'])
        self.assertEqual(6, len(list(output.iterdir())))
        for path in output.iterdir():
            if not path.name.endswith('.sha256'):
                release.verify_sidecar(path)
        with zipfile.ZipFile(output / 'TEST-R01_windows_x64.zip') as archive:
            self.assertIn('preloader_l.dll', archive.namelist())
            self.assertIn('evidence/native/CTEST_RESULTS.xml', archive.namelist())
            self.assertEqual((self.payload / 'production/dist/preloader_l.dll').read_bytes(),
                             archive.read('preloader_l.dll'))

    def test_failed_portable_run_stops_before_creating_assets(self):
        self.xml(self.payload / 'PORTABLE_CTEST.xml', '<failure/>')
        with patch.object(release, 'head', return_value=SOURCE), self.assertRaises(ValueError):
            release.bundle(ROOT, self.payload, self.base / 'assets')
        self.assertFalse((self.base / 'assets').exists())


class DraftTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.assets = Path(self.temporary.name).resolve()
        self.p = {'ready': True, 'tag': 'TEST-R01', 'source_sha': SOURCE, 'existing_id': None}
        release.write(self.assets / 'RELEASE_RECEIPT.json', {
            'tag': 'TEST-R01', 'source_sha': SOURCE, 'classification': 'DRAFT_NOT_RELEASED',
            'publication': False, 'game_acceptance': 'NOT_RUN'})
        for name in ('TEST-R01_source.zip', 'TEST-R01_windows_x64.zip'):
            (self.assets / name).write_bytes(b'fixture package')
        for name in ('TEST-R01_source.zip', 'TEST-R01_windows_x64.zip', 'RELEASE_RECEIPT.json'):
            release.sidecar(self.assets / name)
        self.github = FakeGitHub()

    def publish(self):
        return release.publish_draft(self.github, self.p, self.assets)

    def test_all_uploaded_assets_finish_a_draft_without_publishing(self):
        result = self.publish()
        self.assertTrue(result['draft'])
        self.assertEqual('complete', release.state(result)['status'])
        self.assertTrue(self.github.writes[0]['draft'])
        self.assertNotIn('draft', self.github.writes[-1])

    def test_upload_interruption_retains_incomplete_draft_for_retry(self):
        self.github.fail_upload = True
        with self.assertRaises(OSError):
            self.publish()
        self.assertEqual('preparing', release.state(self.github.items[0])['status'])
        self.github.fail_upload = False
        self.assertEqual('complete', release.state(self.publish())['status'])
        self.assertEqual(1, len(self.github.items))

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
        (self.assets / 'TEST-R01_source.zip').write_bytes(b'changed package')
        with self.assertRaisesRegex(ValueError, 'Checksum'):
            self.publish()
        self.assertEqual([], self.github.writes)


if __name__ == '__main__':
    unittest.main(verbosity=2)
