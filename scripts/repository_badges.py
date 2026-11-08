# SPDX-License-Identifier: GPL-3.0-only
"""Render repository-owned SVG badges and publish only their generated branch."""
import argparse
import hashlib
from html import escape
import json
from pathlib import Path
import re
import subprocess

from weekly_naming import require_tag

BRANCH = 'badges'
DIRECTORY = 'resources/badges'
STATIC = {
    'windows.svg': ('Windows', 'x64', '#0078d4'),
    'cpp.svg': ('C++', '20', '#00599c'),
    'python.svg': ('Python', '3.9+', '#3776ab'),
    'cmake.svg': ('CMake', '3.20+', '#064f8c'),
    'license.svg': ('License', 'GPL v3', '#007ec6'),
    'changelog.svg': ('Changelog', 'view', '#007ec6'),
}
NAMES = frozenset(STATIC) | {'ci.svg', 'latest-release.svg', 'release-integrity.svg'}
CONCLUSIONS = {
    'success': ('passing', '#4c1'),
    'failure': ('failing', '#e05d44'),
    'cancelled': ('cancelled', '#9f9f9f'),
    'timed_out': ('timed out', '#e05d44'),
    'action_required': ('action required', '#dfb317'),
    'neutral': ('neutral', '#9f9f9f'),
    'skipped': ('skipped', '#9f9f9f'),
    'startup_failure': ('startup failure', '#e05d44'),
    'stale': ('stale', '#9f9f9f'),
}


def render(label, value, color):
    if not re.fullmatch(r'#[0-9a-f]{3}(?:[0-9a-f]{3})?', color):
        raise ValueError('Invalid badge color')
    if any(len(text) > 100 or any(ord(c) < 32 for c in text) for text in (label, value)):
        raise ValueError('Invalid badge text')
    left, right = max(24, len(label) * 7 + 16), max(24, len(value) * 7 + 16)
    title = escape(label + ': ' + value, quote=True)
    return ('<!-- SPDX-License-Identifier: GPL-3.0-only -->\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{left+right}" height="20" '
            f'role="img" aria-label="{title}">\n<title>{title}</title>\n'
            f'<rect width="{left+right}" height="20" rx="3" fill="{color}"/>\n'
            f'<path d="M3 0h{left-3}v20H3a3 3 0 0 1-3-3V3a3 3 0 0 1 3-3z" fill="#555"/>\n'
            '<g fill="#fff" text-anchor="middle" font-family="Verdana,DejaVu Sans,sans-serif" font-size="11">\n'
            f'<text x="{left/2:g}" y="14">{escape(label)}</text>\n'
            f'<text x="{left+right/2:g}" y="14">{escape(value)}</text>\n'
            '</g>\n</svg>\n')


def images(tag, conclusion, immutable=None):
    if tag is not None:
        require_tag(tag)
    if conclusion is not None and conclusion not in CONCLUSIONS:
        raise ValueError('Unknown CI conclusion')
    if immutable is not None and not isinstance(immutable, bool):
        raise ValueError('Invalid release immutability value')
    result = {name: render(*fields) for name, fields in STATIC.items()}
    result['ci.svg'] = render('CI', *CONCLUSIONS.get(conclusion, ('not run', '#9f9f9f')))
    result['latest-release.svg'] = render('Latest release', 'v' + tag if tag else 'none', '#007ec6')
    integrity = ('no release', '#9f9f9f') if tag is None else (
        ('immutable', '#4c1') if immutable is True else
        ('mutable', '#dfb317') if immutable is False else ('unknown', '#9f9f9f'))
    result['release-integrity.svg'] = render('Release integrity', *integrity)
    return result


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
                  release.get('immutable') if release else None)


def blob_sha(content):
    data = content.encode('utf-8')
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def publish(api, repository, rendered):
    if set(rendered) != NAMES:
        raise ValueError('Only the nine badge SVGs may be published')
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
        if (tree.get('truncated') or set(actual) != set(expected)
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
    args = parser.parse_args()
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
