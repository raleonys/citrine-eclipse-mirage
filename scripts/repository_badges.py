# SPDX-License-Identifier: GPL-3.0-only
"""Render repository-owned SVG badges and publish only their generated branch."""
import argparse
import hashlib
from html import escape
import json
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

from weekly_naming import require_tag

BRANCH = 'badges'
DIRECTORY = 'resources/badges'
ROOT = Path(__file__).resolve().parents[1]
STATIC = frozenset({'windows.svg', 'cpp.svg', 'python.svg', 'cmake.svg', 'license.svg', 'changelog.svg'})
NAMES = STATIC | {'ci.svg', 'latest-release.svg', 'release-integrity.svg'}
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
            f'<rect width="{left}" height="20" fill="#555"/>'
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
    # Retain the original GitHub workflow badge, including its embedded
    # Octocat path, label typography, rounded ends and two gradients.
    namespace = 'http://www.w3.org/2000/svg'
    ET.register_namespace('', namespace)
    root = ET.fromstring((ROOT / DIRECTORY / 'ci.svg').read_text(encoding='utf-8'))
    value, _ = CONCLUSIONS.get(conclusion, ('not run', '#9f9f9f'))
    state = root.find(".//{%s}path[@id='state-bg']/.." % namespace)
    if state is None:
        raise ValueError('Original GitHub workflow badge template is missing')
    width = max(50, text_width(value) + 10)
    left = int(state.attrib['transform'].removeprefix('translate(').removesuffix(')'))
    root.set('width', str(left + width))
    root.set('role', 'img')
    root.set('aria-label', 'Source integrity: ' + value)
    root.find('{%s}title' % namespace).text = 'Source integrity - ' + value
    state.find("{%s}path[@id='state-bg']" % namespace).set(
        'd', f'M0 0h{width-3}a3 3 0 0 1 3 3v14a3 3 0 0 1-3 3H0V0z')
    for node in state.iter('{%s}tspan' % namespace):
        node.text = value
    colors = ('#34D058', '#28A745') if conclusion == 'success' else (
        ('#D73A49', '#CB2431') if conclusion in {'failure', 'timed_out', 'startup_failure'}
        else ('#959DA5', '#6A737D'))
    for node, color in zip(root.findall(".//{%s}linearGradient[@id='state-fill']/{%s}stop" %
                                        (namespace, namespace)), colors):
        node.set('stop-color', color)
    return ET.tostring(root, encoding='unicode') + '\n'


def images(tag, conclusion, immutable=None):
    if tag is not None:
        require_tag(tag)
    if conclusion is not None and conclusion not in CONCLUSIONS:
        raise ValueError('Unknown CI conclusion')
    if immutable is not None and not isinstance(immutable, bool):
        raise ValueError('Invalid release immutability value')
    result = {name: (ROOT / DIRECTORY / name).read_text(encoding='utf-8') for name in STATIC}
    result['ci.svg'] = render_ci(conclusion)
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
