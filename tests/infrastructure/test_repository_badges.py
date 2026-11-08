# SPDX-License-Identifier: GPL-3.0-only
"""Badge publication boundaries and initial-PR workload regressions."""
import copy
import json
from pathlib import Path
import re
import sys
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

    def request(self, method, path, payload=None, missing_ok=False):
        self.calls.append((method, path, copy.deepcopy(payload)))
        if method == 'GET':
            if path == 'repos/' + REPOSITORY:
                return {'default_branch': 'main'}
            if path.endswith('/releases/latest'):
                return self.release
            if '/actions/workflows/' in path:
                return {'workflow_runs': self.runs}
            if '/git/ref/' in path:
                return {'ref': 'refs/heads/badges', 'object': {'type': 'commit', 'sha': 'a'*40}} if self.existing else None
            if '/git/commits/' in path:
                return {'tree': {'sha': 'c'*40}}
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
        self.assertIn('CI: not run', result['ci.svg'])
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

    def test_initial_publish_writes_only_nine_svg_paths_to_badges_ref(self):
        api = API();rendered = badges.images('3.2.7-ERINTHAR', 'success')
        result = badges.publish(api, REPOSITORY, rendered)
        self.assertTrue(result['changed'])
        writes = api.writes()
        self.assertEqual(3, len(writes))
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
        self.assertIn("github.event.workflow_run.event == 'push'", text)
        self.assertIn('ref: ${{ github.event.repository.default_branch }}', text)
        self.assertNotIn('github.event.workflow_run.head_sha', text)
        self.assertIn('github.event.release.draft == false', text)
        self.assertIn('cancel-in-progress: false', text)
        self.assertNotIn('download-artifact', text)

    def test_readme_badges_are_repository_owned_and_source_seeds_are_valid(self):
        text = (ROOT / 'README.md').read_text()
        images = re.findall(r'\[!\[[^\]]+\]\(([^)]+)\)\]', text)
        self.assertEqual(9, len(images))
        for path in images:
            self.assertTrue(path.startswith(('resources/badges/',
                'https://raw.githubusercontent.com/raleonys/citrine-eclipse-mirage/badges/resources/badges/')), path)
        self.assertNotIn('shields.io', text)
        self.assertNotIn('/badge.svg', text)
        for name in badges.NAMES:
            source = (ROOT / badges.DIRECTORY / name).read_text()
            self.assertEqual('{http://www.w3.org/2000/svg}svg', ET.fromstring(source).tag)


if __name__ == '__main__':
    unittest.main()
