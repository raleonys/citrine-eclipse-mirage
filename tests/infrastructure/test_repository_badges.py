# SPDX-License-Identifier: GPL-3.0-only
"""Badge publication boundaries and initial-PR workload regressions."""
import copy
import base64
import json
import os
from pathlib import Path
import re
import sys
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import repository_badges as badges

REPOSITORY = 'owner/project'


class API:
    def __init__(self, existing=False, tree=None):
        self.calls = []
        self.existing = existing
        self.tree = tree
        self.release = {'tag_name': '3.2.7-ERINTHAR', 'draft': False, 'prerelease': False, 'immutable': True}
        self.runs = [{'head_branch': 'main', 'event': 'push', 'status': 'completed', 'conclusion': 'success'}]
        self.full_runs = []
        self.codeql_runs = []
        self.jobs = {}
        self.trees = {}

    def request(self, method, path, payload=None, missing_ok=False):
        self.calls.append((method, path, copy.deepcopy(payload)))
        if method == 'GET':
            if path == 'repos/' + REPOSITORY:
                return {'default_branch': 'main'}
            if path.endswith('/releases/latest'):
                return self.release
            if path.endswith('/commits/main'):
                return {'sha': 'e'*40, 'commit': {'tree': {'sha': 'f'*40}}}
            if '/actions/workflows/msvc-reference-graph.yml/' in path:
                return {'workflow_runs': self.codeql_runs if 'event=push' in path else self.full_runs}
            if '/actions/workflows/ci.yml/' in path:
                return {'workflow_runs': self.runs}
            if '/actions/runs/' in path and '/jobs?' in path:
                return {'jobs': self.jobs[int(path.split('/actions/runs/')[1].split('/')[0])]}
            if '/git/ref/' in path:
                return {'ref': 'refs/heads/badges', 'object': {'type': 'commit', 'sha': 'a'*40}} if self.existing else None
            if '/git/commits/' in path:
                return {'tree': {'sha': self.trees.get(path.rsplit('/', 1)[1], 'c'*40)}}
            if '/git/trees/' in path:
                return self.tree
        if method == 'POST' and path.endswith('/git/trees'):
            return {'sha': 'd'*40}
        if method == 'POST' and path.endswith('/git/commits'):
            return {'sha': 'b'*40}
        if method in {'POST', 'PATCH'} and '/git/refs' in path:
            return {'ref': 'refs/heads/badges'}
        raise AssertionError((method, path))

    def writes(self):
        return [call for call in self.calls if call[0] != 'GET']


def tree_for(rendered):
    return {'truncated': False, 'tree': [
        {'path': badges.DIRECTORY+'/'+name, 'type': 'blob', 'mode': '100644', 'sha': badges.blob_sha(content)}
        for name, content in rendered.items()]}


class BadgeTests(unittest.TestCase):
    def test_svg_text_is_escaped_and_cannot_add_scripts_or_external_assets(self):
        source = badges.render('CI<&"', '<script href="https://example.invalid">', '#4c1')
        node = ET.fromstring(source)
        self.assertEqual('{http://www.w3.org/2000/svg}svg', node.tag)
        self.assertEqual('CI<&": <script href="https://example.invalid">', node.attrib['aria-label'])
        self.assertFalse(any(n.tag.endswith(('script', 'image', 'use')) for n in node.iter()))
        self.assertFalse(any(key.endswith('href') for n in node.iter() for key in n.attrib))
        with self.assertRaises(ValueError):
            badges.render('CI', 'passing', 'red" onload="bad')

    def test_published_stable_release_and_completed_main_run_only(self):
        api = API()
        result = badges.live_images(api, REPOSITORY)
        self.assertIn('v3.2.7-ERINTHAR', result['latest-release.svg'])
        self.assertIn('passing', result['ci.svg'])
        self.assertIn('Release integrity: immutable', result['release-integrity.svg'])
        self.assertEqual([], api.writes())
        for field, value in (('head_branch', 'feature'), ('event', 'pull_request'), ('status', 'in_progress')):
            api = API();api.runs[0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                badges.live_images(api, REPOSITORY)
        for field in ('draft', 'prerelease'):
            api = API();api.release[field] = True
            with self.subTest(field=field), self.assertRaises(ValueError):
                badges.live_images(api, REPOSITORY)

    def test_absent_release_or_ci_is_explicit_and_invalid_values_fail(self):
        api = API();api.release = None;api.runs = []
        result = badges.live_images(api, REPOSITORY)
        self.assertIn('Latest release: none', result['latest-release.svg'])
        self.assertIn('Source integrity: not run', result['ci.svg'])
        self.assertIn('Release integrity: no release', result['release-integrity.svg'])
        for tag in ('3.2.7-WRONG', '../main', '<svg/>'):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                badges.images(tag, 'success')
        with self.assertRaises(ValueError):
            badges.images(None, 'invented-success')
        for repo in ('../project', 'owner/..', 'owner/project/extra'):
            with self.assertRaises(ValueError):
                badges.repository_path(repo)

    def test_immutability_badge_uses_actual_release_flag_without_claiming_unknown_success(self):
        for flag, expected in ((True, 'immutable'), (False, 'mutable'), (None, 'unknown')):
            with self.subTest(flag=flag):
                rendered = badges.images('3.2.7-ERINTHAR', 'success', flag)
                title = ET.fromstring(rendered['release-integrity.svg']).attrib['aria-label']
                self.assertEqual('Release integrity: '+expected, title)
        with self.assertRaises(ValueError):
            badges.images('3.2.7-ERINTHAR', 'success', 'true')

    def test_initial_publish_writes_only_twelve_svg_paths_to_badges_ref(self):
        api = API();rendered = badges.images('3.2.7-ERINTHAR', 'success')
        result = badges.publish(api, REPOSITORY, rendered)
        self.assertTrue(result['changed'])
        writes = api.writes()
        self.assertEqual(3, len(writes))
        self.assertEqual(12, len(rendered))
        self.assertEqual({badges.DIRECTORY+'/'+name for name in badges.NAMES},
                         {entry['path'] for entry in writes[0][2]['tree']})
        self.assertEqual([], writes[1][2]['parents'])
        self.assertEqual('refs/heads/badges', writes[2][2]['ref'])
        self.assertFalse(any('/heads/main' in path for _, path, _ in writes))

    def test_noop_does_not_create_commits_and_update_never_forces(self):
        rendered = badges.images('3.2.7-ERINTHAR', 'success')
        api = API(existing=True, tree=tree_for(rendered))
        self.assertFalse(badges.publish(api, REPOSITORY, rendered)['changed'])
        self.assertEqual([], api.writes())
        api.tree['tree'][0]['sha'] = 'e'*40
        self.assertTrue(badges.publish(api, REPOSITORY, rendered)['changed'])
        self.assertEqual(['a'*40], api.writes()[1][2]['parents'])
        self.assertEqual({'sha': 'b'*40, 'force': False}, api.writes()[2][2])

    def test_unrecognized_branch_tree_is_never_overwritten(self):
        rendered = badges.images('3.2.7-ERINTHAR', 'success')
        for problem in ('extra_file', 'executable', 'truncated'):
            tree = tree_for(rendered)
            if problem == 'extra_file':
                tree['tree'].append({'path': 'README.md', 'type': 'blob', 'mode': '100644', 'sha': 'f'*40})
            elif problem == 'executable':
                tree['tree'][0]['mode'] = '100755'
            else:
                tree['truncated'] = True
            api = API(existing=True, tree=tree)
            with self.subTest(problem=problem), self.assertRaises(ValueError):
                badges.publish(api, REPOSITORY, rendered)
            self.assertEqual([], api.writes())
        with self.assertRaises(ValueError):
            badges.publish(API(), REPOSITORY, {**rendered, 'README.md': 'unrelated'})

    def test_api_failure_does_not_turn_into_a_missing_branch_or_empty_badge(self):
        class Broken(API):
            def request(self, method, path, payload=None, missing_ok=False):
                if '/git/ref/' in path or path.endswith('/releases/latest'):
                    self.calls.append((method, path, payload))
                    raise RuntimeError('HTTP 403: fixture permission failure')
                return super().request(method, path, payload, missing_ok)
        for operation in ('read', 'publish'):
            api = Broken()
            with self.subTest(operation=operation), self.assertRaises(RuntimeError):
                if operation == 'read':
                    badges.live_images(api, REPOSITORY)
                else:
                    badges.publish(api, REPOSITORY, badges.images(None, None))
            self.assertEqual([], api.writes())

    def test_workflow_cannot_publish_pr_code_or_allocate_on_initial_pr(self):
        text = (ROOT / '.github/workflows/repository-badges.yml').read_text()
        trigger = text.split('\non:\n', 1)[1].split('\npermissions:', 1)[0]
        self.assertNotIn('pull_request', trigger)
        self.assertNotIn('  push:', trigger)
        self.assertIn('branches: [main]', trigger)
        self.assertIn('workflows: [Source integrity, Full validation after main synchronization]', trigger)
        self.assertIn("github.event.workflow_run.event == 'push'", text)
        self.assertIn('ref: ${{ github.event.repository.default_branch }}', text)
        self.assertNotIn('github.event.workflow_run.head_sha', text)
        self.assertIn('github.event.release.draft == false', text)
        self.assertIn('cancel-in-progress: false', text)
        self.assertNotIn('download-artifact', text)

    def test_readme_badges_are_repository_owned_and_source_seeds_are_valid(self):
        text = (ROOT / 'README.md').read_text()
        images = re.findall(r'\[!\[[^\]]+\]\(([^)]+)\)\]', text)
        self.assertEqual(12, len(images))
        for path in images:
            self.assertTrue(path.startswith(('resources/badges/',
                'https://raw.githubusercontent.com/raleonys/citrine-eclipse-mirage/badges/resources/badges/')), path)
        self.assertNotIn('shields.io', text)
        self.assertNotIn('/badge.svg', text)
        for name in badges.NAMES:
            source = (ROOT / badges.DIRECTORY / name).read_text()
            self.assertEqual('{http://www.w3.org/2000/svg}svg', ET.fromstring(source).tag)

    def test_original_logos_are_embedded_and_all_badges_render_without_network_assets(self):
        svg = '{http://www.w3.org/2000/svg}'
        rendered = badges.images('3.2.7-ERINTHAR', 'success', True)
        for name, source in rendered.items():
            with self.subTest(name=name):
                root = ET.fromstring(source)
                self.assertEqual('20', root.attrib['height'])
                for node in root.iter():
                    self.assertNotIn(node.tag, {svg+'script', svg+'foreignObject'})
                    self.assertFalse(any(key.lower().startswith('on') for key in node.attrib))
                    for key, value in node.attrib.items():
                        if key.endswith('href'):
                            self.assertTrue(value.startswith('data:image/svg+xml;base64,'))
                            logo = ET.fromstring(base64.b64decode(value.split(',', 1)[1], validate=True))
                            self.assertTrue(any(n.tag == svg+'path' for n in logo.iter()))
                if name in {'cpp.svg', 'python.svg', 'cmake.svg'}:
                    self.assertTrue(root.findall('.//'+svg+'image'), 'Keep original embedded logo')
        for conclusion in [None, *badges.CONCLUSIONS]:
            with self.subTest(conclusion=conclusion):
                root = ET.fromstring(badges.images(None, conclusion)['ci.svg'])
                self.assertTrue(root.findall('.//'+svg+'linearGradient'))
                self.assertTrue(root.find(".//"+svg+"path[@id='workflow-bg']") is not None)
                self.assertIn(badges.CONCLUSIONS.get(conclusion, ('not run',))[0], root.attrib['aria-label'])

    def test_native_report_counts_failures_and_skips_without_claiming_them_as_passes(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'report.xml'
            path.write_text('<testsuite tests="4"><testcase/><testcase><failure/></testcase>'
                            '<testcase><error/></testcase><testcase><skipped/></testcase></testsuite>')
            self.assertEqual((1, 4), badges.test_counts(path))
            output = Path(tmp) / 'outputs'
            run = subprocess.run([sys.executable, '-B', str(ROOT/'scripts/repository_badges.py'),
                                  '--record-tests', str(path)], capture_output=True, text=True,
                                 env={**os.environ, 'GITHUB_OUTPUT': str(output), 'PATH': tmp})
            self.assertEqual(0, run.returncode, run.stderr)
            self.assertEqual('passed=1\ntotal=4\n', output.read_text())
            for data in ('<testsuite tests="2"><testcase/></testsuite>', '<testsuite tests="0"/>',
                         '<html/>', '<!DOCTYPE testsuite><testsuite><testcase/></testsuite>'):
                path.write_text(data)
                with self.subTest(data=data), self.assertRaises(ValueError):
                    badges.test_counts(path)

    def test_native_badge_rejects_unowned_or_misnested_junit_cases(self):
        invalid = (
            '<testsuites><testcase name="orphan"/></testsuites>',
            '<testsuite><testcase name="missing-count"/></testsuite>',
            '<testsuite tests="1"><wrapper><testcase/></wrapper></testsuite>',
            '<testsuite tests="1"><testsuites><testcase/></testsuites></testsuite>',
            '<testsuites><wrapper><testsuite tests="1"><testcase/></testsuite></wrapper></testsuites>',
        )
        valid = (
            ('<testsuite tests="1"><testcase name="ok"/></testsuite>', (1, 1)),
            ('<testsuites tests="2"><testsuite tests="1"><testcase name="ok"/></testsuite>'
             '<testsuite tests="1"><testcase name="failed"><failure/></testcase></testsuite>'
             '</testsuites>', (1, 2)),
        )
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'report.xml'
            for data in invalid:
                with self.subTest(invalid=data):
                    path.write_text(data, encoding='utf-8')
                    with self.assertRaises(ValueError):
                        badges.test_counts(path)
            for data, expected in valid:
                with self.subTest(valid=data):
                    path.write_text(data, encoding='utf-8')
                    self.assertEqual(expected, badges.test_counts(path))

    def test_native_report_rejects_inconsistent_declared_outcome_totals(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'report.xml'
            valid = (
                ('<testsuite tests="3" failures="1" errors="1" skipped="1">'
                 '<testcase><failure/></testcase><testcase><error/></testcase>'
                 '<testcase><skipped/></testcase></testsuite>', (0, 3)),
                ('<testsuites tests="2" failures="1"><testsuite tests="2" failures="1">'
                 '<testcase/><testcase><failure/></testcase></testsuite></testsuites>', (1, 2)),
                ('<testsuite tests="1"><testcase/></testsuite>', (1, 1)),
            )
            for source, expected in valid:
                path.write_text(source, encoding='utf-8')
                with self.subTest(valid=source):
                    self.assertEqual(expected, badges.test_counts(path))
            invalid = (
                '<testsuite tests="2" failures="1"><testcase/><testcase/></testsuite>',
                '<testsuite tests="1" errors="1"><testcase/></testsuite>',
                '<testsuite tests="1" skipped="1"><testcase/></testsuite>',
                '<testsuite tests="1" failures="NaN"><testcase/></testsuite>',
                '<testsuites tests="2" failures="0"><testsuite tests="2" failures="1">'
                '<testcase/><testcase><failure/></testcase></testsuite></testsuites>',
            )
            for source in invalid:
                path.write_text(source, encoding='utf-8')
                with self.subTest(invalid=source), self.assertRaises(ValueError):
                    badges.test_counts(path)

    def test_native_badge_requires_the_same_git_tree_as_main_and_does_not_hide_failure(self):
        api = API()
        api.full_runs = [dict(id=12, event='pull_request', head_sha='1'*40,
                             head_repository={'full_name': REPOSITORY}),
                         dict(id=11, event='pull_request', head_sha='2'*40,
                             head_repository={'full_name': REPOSITORY})]
        api.trees = {'1'*40: 'a'*40, '2'*40: 'f'*40}
        api.jobs[11] = [dict(name='Windows native', status='completed', conclusion='success',
                            steps=[dict(name='Native tests: 222/222', conclusion='success')])]
        result = badges.live_images(api, REPOSITORY)
        self.assertIn('Tests (native): 222/222', result['tests.svg'])
        self.assertFalse(any('/runs/12/jobs?' in path for _, path, _ in api.calls))
        api.jobs[11][0].update(status='in_progress', conclusion=None)
        self.assertIn('not available', badges.live_images(api, REPOSITORY)['tests.svg'])
        api.jobs[11][0].update(status='completed', conclusion='failure')
        api.jobs[11][0]['steps'][0]['name'] = 'Native tests: 220/222'
        result = badges.live_images(api, REPOSITORY)['tests.svg']
        self.assertIn('220/222', result)
        self.assertIn('#e05d44', result)
        for totals in ((2, 1, 'success'), (True, 1, 'success'), (0, 0, 'success')):
            with self.assertRaises(ValueError):
                badges.images(None, None, tests=totals)

    def test_checksum_badge_requires_the_matching_uploaded_zip_and_sidecar(self):
        api = API()
        name = 'Citrine_Eclipse_Mirage_3.2.7_ERINTHAR.zip'
        api.release['assets'] = [dict(name=name, state='uploaded', size=100),
                                 dict(name=name+'.sha256', state='uploaded', size=108)]
        self.assertIn('Checksums: SHA-256', badges.live_images(api, REPOSITORY)['checksums.svg'])
        for problem in ('missing_zip', 'wrong_release', 'empty', 'uploading'):
            bad = copy.deepcopy(api.release)
            if problem == 'missing_zip': bad['assets'].pop(0)
            elif problem == 'wrong_release': bad['assets'][1]['name'] = name.replace('3.2.7', '3.2.6')+'.sha256'
            elif problem == 'empty': bad['assets'][1]['size'] = 0
            else: bad['assets'][1]['state'] = 'starter'
            with self.subTest(problem=problem):
                self.assertFalse(badges.checksum_available(bad))
        self.assertIsNone(badges.checksum_available(None))

    def test_only_exact_previous_nine_and_eleven_file_trees_can_upgrade(self):
        rendered = badges.images(None, None)
        for names in (badges.LEGACY_NAMES, badges.PREVIOUS_NAMES):
            with self.subTest(count=len(names)):
                tree = tree_for({name: value for name, value in rendered.items() if name in names})
                api = API(existing=True, tree=tree)
                self.assertTrue(badges.publish(api, REPOSITORY, rendered)['changed'])
                tree['tree'].pop()
                api = API(existing=True, tree=tree)
                with self.assertRaises(ValueError):
                    badges.publish(api, REPOSITORY, rendered)
                self.assertEqual([], api.writes())

    def test_readme_groups_blue_badges_before_status_badges(self):
        text = (ROOT / 'README.md').read_text()
        upper, lower = text.split('<br>', 1)
        paths = lambda part: re.findall(r'resources/badges/([a-z-]+\.svg)', part)
        self.assertEqual(['windows.svg', 'cpp.svg', 'python.svg', 'cmake.svg', 'license.svg',
                          'latest-release.svg', 'changelog.svg'], paths(upper))
        self.assertEqual(['ci.svg', 'tests.svg', 'release-integrity.svg', 'checksums.svg', 'codeql.svg'], paths(lower))

    @staticmethod
    def codeql_api():
        api = API()
        api.codeql_runs = [dict(id=90, event='push', head_branch='main', head_sha='e'*40,
                                head_repository={'full_name': REPOSITORY},
                                status='completed', conclusion='success')]
        api.jobs[90] = [dict(name='CodeQL ('+language+')', status='completed', conclusion='success')
                        for language in ('actions', 'c-cpp', 'python')]
        return api

    def test_codeql_green_requires_current_main_and_all_three_actual_analyses(self):
        api = self.codeql_api()
        self.assertIn('CodeQL: passing', badges.live_images(api, REPOSITORY)['codeql.svg'])
        self.assertEqual([], api.writes())
        for field, value in [('head_sha', 'd'*40), ('event', 'pull_request'),
                             ('head_branch', 'feature'), ('head_repository', {'full_name': 'other/project'}),
                             ('head_repository', None), ('id', '90'), ('id', -1)]:
            with self.subTest(field=field, value=value):
                api = self.codeql_api();api.codeql_runs[0][field] = value
                self.assertIn('CodeQL: not available', badges.live_images(api, REPOSITORY)['codeql.svg'])
        for problem in ('missing', 'duplicate', 'skipped', 'pending', 'failed'):
            with self.subTest(problem=problem):
                api = self.codeql_api()
                if problem == 'missing':api.jobs[90].pop()
                elif problem == 'duplicate':api.jobs[90][2] = copy.deepcopy(api.jobs[90][0])
                elif problem == 'skipped':api.jobs[90][1]['conclusion'] = 'skipped'
                elif problem == 'pending':api.jobs[90][1].update(status='in_progress', conclusion=None)
                else:api.jobs[90][1]['conclusion'] = 'failure'
                self.assertIn('CodeQL: not available', badges.live_images(api, REPOSITORY)['codeql.svg'])

    def test_codeql_does_not_reuse_an_older_green_run_for_a_newer_pending_or_failed_run(self):
        for status, conclusion, expected in [('in_progress', None, 'running'),
                                              ('completed', 'failure', 'failing'),
                                              ('completed', 'cancelled', 'cancelled'),
                                              ('completed', 'skipped', 'not available')]:
            with self.subTest(status=status, conclusion=conclusion):
                api = self.codeql_api();old = copy.deepcopy(api.codeql_runs[0])
                api.codeql_runs[0].update(id=91, status=status, conclusion=conclusion)
                api.codeql_runs.append(old)
                source = badges.live_images(api, REPOSITORY)['codeql.svg']
                self.assertIn('CodeQL: '+expected, source)
                self.assertNotIn('#4c1', source)

    def test_codeql_pending_unknown_and_invalid_statuses_are_not_green(self):
        for status in (None, 'pending', 'cancelled', 'failure'):
            self.assertNotIn('#4c1', badges.images(None, None, codeql=status)['codeql.svg'])
        for status in ('invented-success', True, []):
            with self.assertRaises(ValueError):badges.images(None, None, codeql=status)


if __name__ == '__main__':
    unittest.main()
