# SPDX-License-Identifier: GPL-3.0-only
"""Render repository-owned SVG badges and publish only their generated branch."""
import argparse
import hashlib
from html import escape
import json
import os
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

from weekly_naming import require_tag

BRANCH = 'badges'
DIRECTORY = 'resources/badges'
ROOT = Path(__file__).resolve().parents[1]
STATIC = frozenset({'windows.svg', 'cpp.svg', 'python.svg', 'cmake.svg', 'license.svg', 'changelog.svg'})
PREVIOUS_NAMES = STATIC | {'ci.svg', 'latest-release.svg', 'release-integrity.svg', 'tests.svg', 'checksums.svg'}
LEGACY_NAMES = PREVIOUS_NAMES - {'tests.svg', 'checksums.svg'}
CODEQL_NAMES = PREVIOUS_NAMES | {'codeql.svg'}
NAMES = CODEQL_NAMES | {'qa.svg'}
# Shared README palette: License blue, QA green and CodeQL state colors.
LABEL_COLOR = '#555'
INFORMATION_COLOR = '#007ec6'
SUCCESS_COLOR = '#4c1'
WARNING_COLOR = '#dfb317'
FAILURE_COLOR = '#e05d44'
NEUTRAL_COLOR = '#9f9f9f'
CONCLUSIONS = {
    'success': ('passing', SUCCESS_COLOR),
    'failure': ('failing', FAILURE_COLOR),
    'cancelled': ('cancelled', NEUTRAL_COLOR),
    'timed_out': ('timed out', FAILURE_COLOR),
    'action_required': ('action required', WARNING_COLOR),
    'neutral': ('neutral', NEUTRAL_COLOR),
    'skipped': ('skipped', NEUTRAL_COLOR),
    'startup_failure': ('startup failure', FAILURE_COLOR),
    'stale': ('stale', NEUTRAL_COLOR),
}

# Observed ASCII advances in pixels at 11px for the installed DejaVu Sans
# fallback. These are numeric layout measurements, not an embedded font.
# Fixed label widths match the original Shields responses exactly.
ADVANCES = dict(zip(
    ''.join(chr(i) for i in range(32, 127)),
    (3.50,4.41,5.06,9.22,7.00,10.45,8.58,3.03,4.30,4.30,5.50,9.22,3.50,3.97,3.50,3.70,
     7.00,7.00,7.00,7.00,7.00,7.00,7.00,7.00,7.00,7.00,3.70,3.70,9.22,9.22,9.22,5.84,
     11.00,7.53,7.55,7.69,8.47,6.95,6.33,8.53,8.27,3.25,3.25,7.22,6.12,9.48,8.23,8.66,
     6.64,8.66,7.64,6.98,6.72,8.05,7.53,10.88,7.53,6.72,7.53,4.30,3.70,4.30,9.22,5.50,
     5.50,6.73,6.98,6.05,6.98,6.77,3.88,6.98,6.97,3.06,3.06,6.38,3.06,10.72,6.97,6.73,
     6.98,6.98,4.52,5.73,4.31,6.97,6.52,9.00,6.52,6.52,5.78,7.00,3.70,7.00,9.22)))


def text_width(text):
    return max(1, round(sum(ADVANCES.get(c, 11) for c in text)))


def render(label, value, color):
    if not re.fullmatch(r'#[0-9a-f]{3}(?:[0-9a-f]{3})?', color):
        raise ValueError('Invalid badge color')
    if any(len(text) > 100 or any(ord(c) < 32 for c in text) for text in (label, value)):
        raise ValueError('Invalid badge text')
    label_width = {'Latest release': 77}.get(label, text_width(label))
    value_width = {'v3.2.7-ERINTHAR': 99}.get(value, text_width(value))
    left, right = label_width + 10, value_width + 10
    title = escape(label + ': ' + value, quote=True)
    return ('<!-- SPDX-License-Identifier: GPL-3.0-only -->\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{left+right}" height="20" '
            f'role="img" aria-label="{title}">\n<title>{title}</title>\n'
            '<filter id="blur"><feGaussianBlur stdDeviation="16"/></filter>\n'
            '<linearGradient id="s" x2="0" y2="100%"><stop offset="0" stop-color="#bbb" '
            'stop-opacity=".1"/><stop offset="1" stop-opacity=".1"/></linearGradient>\n'
            f'<clipPath id="r"><rect width="{left+right}" height="20" rx="3"/></clipPath>\n'
            '<g clip-path="url(#r)">'
            f'<rect width="{left}" height="20" fill="{LABEL_COLOR}"/>'
            f'<rect x="{left}" width="{right}" height="20" fill="{color}"/>'
            f'<rect width="{left+right}" height="20" fill="url(#s)"/></g>\n'
            '<g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" '
            'text-rendering="geometricPrecision" font-size="110">\n'
            + ''.join(
                '<g transform="scale(.1)"><g aria-hidden="true" fill="#010101">'
                f'<text x="{x:g}" y="150" fill-opacity=".8" filter="url(#blur)" '
                f'textLength="{width*10}">{escape(text)}</text>'
                f'<text x="{x:g}" y="150" fill-opacity=".3" '
                f'textLength="{width*10}">{escape(text)}</text></g>'
                f'<text x="{x:g}" y="140" textLength="{width*10}">{escape(text)}</text></g>\n'
                for text, x, width in ((label, left*5+10, label_width),
                                       (value, left*10+right*5-10, value_width))) +
            '</g>\n</svg>\n')


def render_ci(conclusion):
    value, color = CONCLUSIONS.get(conclusion, ('not run', NEUTRAL_COLOR))
    return render('Source integrity', value, color)


def images(tag, conclusion, immutable=None, tests=None, checksum=None, codeql=None, qa=None):
    if tag is not None:
        require_tag(tag)
    if conclusion is not None and conclusion not in CONCLUSIONS:
        raise ValueError('Unknown CI conclusion')
    if immutable is not None and not isinstance(immutable, bool):
        raise ValueError('Invalid release immutability value')
    result = {name: (ROOT / DIRECTORY / name).read_text(encoding='utf-8') for name in STATIC}
    result['ci.svg'] = render_ci(conclusion)
    result['latest-release.svg'] = render('Latest release', 'v' + tag if tag else 'none', INFORMATION_COLOR)
    integrity = ('no release', NEUTRAL_COLOR) if tag is None else (
        ('immutable', SUCCESS_COLOR) if immutable is True else
        ('mutable', WARNING_COLOR) if immutable is False else ('unknown', NEUTRAL_COLOR))
    result['release-integrity.svg'] = render('Release integrity', *integrity)
    if tests is None:
        result['tests.svg'] = render('Tests (native)', 'not available', NEUTRAL_COLOR)
    else:
        passed, total, status = tests
        if (type(passed) is not int or type(total) is not int or not 0 <= passed <= total <= 100000
                or total == 0 or status not in {'success', 'failure'}):
            raise ValueError('Invalid native test totals')
        result['tests.svg'] = render('Tests (native)', f'{passed}/{total}',
                                     SUCCESS_COLOR if passed == total and status == 'success' else FAILURE_COLOR)
    if checksum is not None and type(checksum) is not bool:
        raise ValueError('Invalid checksum availability')
    checksum_state = ('no release', NEUTRAL_COLOR) if tag is None else (
        ('SHA-256', SUCCESS_COLOR) if checksum is True else
        ('missing', FAILURE_COLOR) if checksum is False else ('unknown', NEUTRAL_COLOR))
    result['checksums.svg'] = render('Checksums', *checksum_state)
    states = {None: ('not available', NEUTRAL_COLOR),
              'success': ('passing', SUCCESS_COLOR), 'failure': ('failing', FAILURE_COLOR),
              'pending': ('running', WARNING_COLOR), 'cancelled': ('cancelled', NEUTRAL_COLOR)}
    if codeql is not None and (not isinstance(codeql, str) or codeql not in states):
        raise ValueError('Invalid CodeQL status')
    result['codeql.svg'] = render('CodeQL', *states[codeql])
    if qa is not None and (not isinstance(qa, str) or qa not in states):
        raise ValueError('Invalid QA status')
    qa_states = {**states, None: ('n/a', NEUTRAL_COLOR)}
    result['qa.svg'] = render('QA', *qa_states[qa])
    return result


def test_counts(path):
    data = path.read_bytes()
    if len(data) > 8 * 1024 * 1024 or b'<!DOCTYPE' in data or b'<!ENTITY' in data:
        raise ValueError('Unexpected native test report')
    root = ET.fromstring(data)
    if root.tag not in {'testsuite', 'testsuites'}:
        raise ValueError('Expected CTest JUnit report')
    cases = list(root.iter('testcase'))
    if not 0 < len(cases) <= 100000:
        raise ValueError('Native test report must contain testcases')
    parents = {child: parent for parent in root.iter() for child in parent}
    suites = list(root.iter('testsuite'))
    if not suites or any(node is not root and node.tag == 'testsuites' for node in root.iter()):
        raise ValueError('Native test report has invalid suite structure')
    if any(suite is not root and parents.get(suite) is not root and
           parents.get(suite) not in suites for suite in suites):
        raise ValueError('Native test suite has an invalid parent')
    if any(parents.get(case) not in suites for case in cases):
        raise ValueError('Native testcase is not directly owned by a suite')
    for suite in root.iter():
        if suite.tag in {'testsuite', 'testsuites'}:
            suite_cases = list(suite.iter('testcase'))
            count = suite.get('tests')
            if suite.tag == 'testsuite' and count is None:
                raise ValueError('Native test suite must declare its test count')
            if count is not None:
                if not re.fullmatch(r'[0-9]+', count) or int(count) != len(suite_cases):
                    raise ValueError('Native test count does not match testcases')
            # Optional JUnit summary totals must not contradict case outcomes.
            # A malformed declared failure count must never render a green badge.
            for field, tag in (('failures', 'failure'), ('errors', 'error'), ('skipped', 'skipped')):
                declared = suite.get(field)
                if declared is not None:
                    observed = sum(case.find(tag) is not None for case in suite_cases)
                    if not re.fullmatch(r'[0-9]+', declared) or int(declared) != observed:
                        raise ValueError('Native test ' + field + ' count does not match testcases')
    passed = sum(not any(case.find(tag) is not None for tag in ('failure', 'error', 'skipped')) for case in cases)
    return passed, len(cases)


def native_result(api, base, default, repository):
    # Main's merge commit may differ from the tested PR SHA. Require an exact
    # Git tree match, never merely an ancestor or an unrelated latest PR.
    main = api.request('GET', base + '/commits/' + default)
    target = main['commit']['tree']['sha']
    trees = {main['sha']: target}
    runs = api.request('GET', base + '/actions/workflows/msvc-reference-graph.yml/runs?event=pull_request&per_page=30')['workflow_runs']
    for run in runs:
        owner = run.get('head_repository')
        if (run.get('event') != 'pull_request' or not isinstance(owner, dict)
                or owner.get('full_name') != repository or not isinstance(run.get('head_sha'), str)
                or not re.fullmatch(r'[0-9a-f]{40}', run['head_sha'])
                or type(run.get('id')) is not int or run['id'] <= 0):
            continue
        sha = run['head_sha']
        if sha not in trees:
            commit = api.request('GET', base + '/git/commits/' + sha, missing_ok=True)
            if commit is None:
                continue
            trees[sha] = commit['tree']['sha']
        if trees[sha] != target:
            continue
        jobs = api.request('GET', base + '/actions/runs/' + str(run['id']) + '/jobs?per_page=100')['jobs']
        for job in jobs:
            if job['name'] != 'Windows native':
                continue
            # Do not replace a newer pending/failed native run with an older PASS.
            if job['status'] != 'completed' or job['conclusion'] not in {'success', 'failure'}:
                return None
            for step in reversed(job.get('steps', [])):
                match = re.fullmatch(r'Native tests: ([0-9]+)/([0-9]+)', step['name'])
                if match and step['conclusion'] == 'success':
                    return int(match[1]), int(match[2]), job['conclusion']
            return None
    return None


def qa_result(api, base, default, repository):
    # The accepted merge and tested PR can have different SHAs but must have
    # exactly the same Git tree, including their workflow and test sources.
    main = api.request('GET', base + '/commits/' + default)
    target = main['commit']['tree']['sha']
    if not re.fullmatch(r'[0-9a-f]{40}', target):
        raise ValueError('Invalid default-branch tree')
    trees = {main['sha']: target}
    runs = api.request('GET', base + '/actions/workflows/msvc-reference-graph.yml/runs?event=pull_request&per_page=30')['workflow_runs']
    for run in runs:
        owner = run.get('head_repository')
        if (run.get('event') != 'pull_request' or not isinstance(owner, dict)
                or owner.get('full_name') != repository
                or not isinstance(run.get('head_sha'), str)
                or not re.fullmatch(r'[0-9a-f]{40}', run['head_sha'])
                or type(run.get('id')) is not int or run['id'] <= 0):
            continue
        sha = run['head_sha']
        if sha not in trees:
            commit = api.request('GET', base + '/git/commits/' + sha, missing_ok=True)
            if commit is None:
                continue
            trees[sha] = commit['tree']['sha']
        if trees[sha] != target:
            continue
        response = api.request('GET', base + '/actions/runs/' + str(run['id'])
                               + '/jobs?filter=latest&per_page=100')
        jobs = response['jobs']
        if not isinstance(jobs, list):
            return None
        total = response.get('total_count', len(jobs))
        if type(total) is not int or total != len(jobs):
            return None
        selected = [job for job in jobs if isinstance(job, dict)
                    and job.get('name') == 'Source QA and portable tests']
        # Never fall back to an older green result for this tested tree.
        if len(selected) != 1:
            return 'pending' if not selected and run.get('status') in {
                'queued', 'in_progress', 'waiting', 'pending', 'requested'} else None
        job = selected[0]
        if job.get('status') != 'completed':
            return 'pending' if job.get('status') in {
                'queued', 'in_progress', 'waiting', 'pending', 'requested'} else None
        conclusion = job.get('conclusion')
        if conclusion == 'success':
            return 'success'
        if conclusion in {'failure', 'timed_out', 'action_required', 'startup_failure'}:
            return 'failure'
        return 'cancelled' if conclusion == 'cancelled' else None
    return None


def codeql_result(api, base, default, repository):
    # Only the current accepted commit and all three actual analyses can be green.
    main = api.request('GET', base + '/commits/' + default)
    sha = main['sha']
    if not re.fullmatch(r'[0-9a-f]{40}', sha):
        raise ValueError('Invalid default-branch commit')
    runs = api.request('GET', base + '/actions/workflows/msvc-reference-graph.yml/runs?event=push&branch='
                       + default + '&per_page=30')['workflow_runs']
    for run in runs:
        owner = run.get('head_repository')
        if (run.get('event') != 'push' or run.get('head_branch') != default
                or run.get('head_sha') != sha or type(run.get('id')) is not int
                or run['id'] <= 0 or not isinstance(owner, dict)
                or owner.get('full_name') != repository):
            continue
        if run.get('status') != 'completed':
            return 'pending' if run.get('status') in {'queued', 'in_progress', 'waiting', 'pending', 'requested'} else None
        conclusion = run.get('conclusion')
        if conclusion != 'success':
            return ('failure' if conclusion in {'failure', 'timed_out', 'action_required'}
                    else 'cancelled' if conclusion == 'cancelled' else None)
        jobs = api.request('GET', base + '/actions/runs/' + str(run['id'])
                           + '/jobs?filter=latest&per_page=100')['jobs']
        expected = {'CodeQL (actions)', 'CodeQL (c-cpp)', 'CodeQL (python)'}
        analyses = [job for job in jobs if job.get('name') in expected]
        if (len(analyses) == 3 and {job['name'] for job in analyses} == expected
                and all(job.get('status') == 'completed' and job.get('conclusion') == 'success'
                        for job in analyses)):
            return 'success'
        return None
    return None


def checksum_available(release):
    if release is None:
        return None
    tag = release['tag_name']
    require_tag(tag)
    name = 'Citrine_Eclipse_Mirage_' + tag.replace('-', '_') + '.zip'
    assets = release.get('assets')
    if not isinstance(assets, list):
        return None
    return all(any(asset.get('name') == expected and asset.get('state') == 'uploaded'
                   and type(asset.get('size')) is int and asset['size'] > 0 for asset in assets)
               for expected in (name, name + '.sha256'))


class GitHub:
    def request(self, method, path, payload=None, missing_ok=False):
        command = ['gh', 'api', '--method', method, path]
        if payload is not None:
            command += ['--input', '-']
        result = subprocess.run(command, input=json.dumps(payload) if payload is not None else None,
                                text=True, capture_output=True, timeout=60)
        if result.returncode:
            if missing_ok and re.search(r'HTTP 404\b', result.stderr):
                return None
            raise RuntimeError('GitHub API failed: ' + result.stderr.strip())
        return json.loads(result.stdout) if result.stdout.strip() else None


def repository_path(repository):
    if (not re.fullmatch(r'[A-Za-z0-9-]+/[A-Za-z0-9_.-]+', repository)
            or repository.split('/')[-1] in {'.', '..'}):
        raise ValueError('Expected owner/repository')
    return 'repos/' + repository


def live_images(api, repository):
    base = repository_path(repository)
    default = api.request('GET', base)['default_branch']
    if not re.fullmatch(r'[A-Za-z0-9_.-]+', default):
        raise ValueError('Unexpected default branch')
    release = api.request('GET', base + '/releases/latest', missing_ok=True)
    if release is not None and (release.get('draft') or release.get('prerelease')):
        raise ValueError('Latest release must be a published stable release')
    runs = api.request('GET', base + '/actions/workflows/ci.yml/runs?event=push&status=completed&branch='
                       + default + '&per_page=1')['workflow_runs']
    if runs and (runs[0]['head_branch'] != default or runs[0]['event'] != 'push'
                 or runs[0]['status'] != 'completed'):
        raise ValueError('CI badge requires a completed default-branch push run')
    return images(release['tag_name'] if release else None, runs[0]['conclusion'] if runs else None,
                  release.get('immutable') if release else None,
                  native_result(api, base, default, repository), checksum_available(release),
                  codeql_result(api, base, default, repository),
                  qa_result(api, base, default, repository))


def blob_sha(content):
    data = content.encode('utf-8')
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def publish(api, repository, rendered):
    if set(rendered) != NAMES:
        raise ValueError('Only the thirteen badge SVGs may be published')
    base = repository_path(repository)
    expected = {DIRECTORY + '/' + name: blob_sha(value) for name, value in rendered.items()}
    ref = api.request('GET', base + '/git/ref/heads/' + BRANCH, missing_ok=True)
    parent = None
    if ref:
        if ref['ref'] != 'refs/heads/' + BRANCH or ref['object']['type'] != 'commit':
            raise ValueError('Unexpected generated-branch reference')
        parent = ref['object']['sha']
        commit = api.request('GET', base + '/git/commits/' + parent)
        tree = api.request('GET', base + '/git/trees/' + commit['tree']['sha'] + '?recursive=1')
        actual = {entry['path']: entry['sha'] for entry in tree['tree'] if entry['type'] == 'blob'}
        legacy = {DIRECTORY + '/' + name for name in LEGACY_NAMES}
        previous = {DIRECTORY + '/' + name for name in PREVIOUS_NAMES}
        codeql = {DIRECTORY + '/' + name for name in CODEQL_NAMES}
        if (tree.get('truncated') or set(actual) not in (set(expected), codeql, previous, legacy)
                or any(entry['type'] not in {'tree', 'blob'} or
                       (entry['type'] == 'blob' and entry['mode'] != '100644') or
                       (entry['type'] == 'tree' and entry['path'] not in {'resources', DIRECTORY})
                       for entry in tree['tree'])):
            raise ValueError('Refusing to replace an unrecognized generated badge tree')
        if actual == expected:
            return {'changed': False, 'branch': BRANCH, 'sha': parent}
    tree = api.request('POST', base + '/git/trees', {'tree': [
        {'path': DIRECTORY + '/' + name, 'mode': '100644', 'type': 'blob', 'content': content}
        for name, content in sorted(rendered.items())]})
    commit = api.request('POST', base + '/git/commits', {
        'message': 'Update repository badge images', 'tree': tree['sha'],
        'parents': [parent] if parent else []})
    if parent:
        api.request('PATCH', base + '/git/refs/heads/' + BRANCH, {'sha': commit['sha'], 'force': False})
    else:
        api.request('POST', base + '/git/refs', {'ref': 'refs/heads/' + BRANCH, 'sha': commit['sha']})
    return {'changed': True, 'branch': BRANCH, 'sha': commit['sha']}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', default='raleonys/citrine-eclipse-mirage')
    parser.add_argument('--output', type=Path)
    parser.add_argument('--publish', action='store_true')
    parser.add_argument('--record-tests', type=Path)
    args = parser.parse_args()
    if args.record_tests:
        if args.output or args.publish:
            parser.error('Test recording cannot publish images')
        passed, total = test_counts(args.record_tests)
        with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8') as output:
            output.write(f'passed={passed}\ntotal={total}\n')
        raise SystemExit(0)
    if not args.output and not args.publish:
        parser.error('Select --output and/or --publish')
    api = GitHub()
    rendered = live_images(api, args.repository)
    if args.output:
        args.output.mkdir(parents=True, exist_ok=True)
        for name, content in rendered.items():
            (args.output / name).write_text(content, encoding='utf-8')
    if args.publish:
        print(json.dumps(publish(api, args.repository, rendered)))
