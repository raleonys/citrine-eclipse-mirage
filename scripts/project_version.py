# SPDX-License-Identifier: GPL-3.0-only
"""One version selection for Windows resources, Golden checks and build receipts."""
import argparse
import copy
import json
import os
from pathlib import Path
import re
from datetime import datetime

ROOT = Path(__file__).resolve().parents[1]
ENVIRONMENT = 'CITRINE_ECLIPSE_MIRAGE_VERSION'


def components(version):
    if not isinstance(version, str) or not re.fullmatch(r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)', version):
        raise ValueError('Project version must be canonical major.minor.patch')
    values = tuple(map(int, version.split('.')))
    if any(value > 65535 for value in values):
        raise ValueError('Windows version components must fit in 16 bits')
    return (*values, 0)


def selected_version(root=ROOT):
    identity = json.loads((Path(root) / 'metadata/BRANDING_AND_IDENTITY.json').read_text(encoding='utf-8'))
    version = os.environ.get(ENVIRONMENT, identity['default_project_version'])
    components(version)
    return version


def resolve_policy(policy, root=ROOT):
    """Expand only explicitly versioned product policies; fixtures stay independent."""
    if policy.get('version_source') != 'metadata/BRANDING_AND_IDENTITY.json':
        return policy
    policy = copy.deepcopy(policy)
    major, minor, patch, revision = components(selected_version(root))
    info = policy['required']['versioninfo']
    for kind in ('File', 'Product'):
        info['fixed']['dw' + kind + 'VersionMS'] = f'0x{(major << 16) | minor:08X}'
        info['fixed']['dw' + kind + 'VersionLS'] = f'0x{(patch << 16) | revision:08X}'
        info['strings'][kind + 'Version'] = selected_version(root)
    return policy


def write_header(path, version):
    values = ','.join(map(str, components(version)))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('// SPDX-License-Identifier: GPL-3.0-only\n'
                    '// Generated from the selected project version; do not edit.\n'
                    f'#define CEM_VERSION_NUMERIC {values}\n'
                    f'#define CEM_VERSION_STRING "{version}\\0"\n', encoding='utf-8')


def refresh_release_default(root, release, expected_tag=''):
    """Prepare only a stable-version metadata/changelog update for owner review."""
    from project_files import digest, source_files, verify_manifest, write_manifest
    from weekly_naming import require_tag

    root = Path(root)
    if (not isinstance(release, dict) or release.get('draft') is not False
            or release.get('prerelease') is not False):
        raise ValueError('Default version requires a published stable release')
    published = release.get('published_at')
    if not isinstance(published, str) or not published:
        raise ValueError('Stable release must have a publication time')
    if datetime.fromisoformat(published.replace('Z', '+00:00')).tzinfo is None:
        raise ValueError('Stable release publication time must include its timezone')
    tag = release.get('tag_name')
    if not isinstance(tag, str):
        raise ValueError('Stable release tag must be a string')
    next_version = require_tag(tag)
    if expected_tag and expected_tag != tag:
        return {'changed': False, 'reason': 'EVENT_IS_NOT_LATEST_STABLE'}
    identity_path = root / 'metadata/BRANDING_AND_IDENTITY.json'
    identity = json.loads(identity_path.read_text(encoding='utf-8'))
    current = identity['default_project_version']
    if components(next_version) <= components(current):
        return {'changed': False, 'reason': 'DEFAULT_IS_CURRENT_OR_NEWER'}
    errors = verify_manifest(root)
    if errors:
        raise ValueError('Cannot update an unverified source tree: ' + '; '.join(errors[:3]))
    before = {name: digest(data) for name, data in source_files(root).items()}
    changelog_path = root / 'CHANGELOG.md'
    changelog = changelog_path.read_text(encoding='utf-8')
    heading = '### This PR\n\n'
    if changelog.count(heading) != 1:
        raise ValueError('Expected one current PR changelog heading')
    entry = (f'- Align the offline build default version with published stable '
             f'`{tag}`; retain explicit version overrides for release candidates.\n\n')
    identity['default_project_version'] = next_version
    identity['default_project_version_release_tag'] = tag
    identity_path.write_text(json.dumps(identity, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    changelog_path.write_text(changelog.replace(heading, heading + entry), encoding='utf-8')
    after = {name: digest(data) for name, data in source_files(root).items()}
    changed = {name for name in before.keys() | after.keys() if before.get(name) != after.get(name)}
    if changed != {'metadata/BRANDING_AND_IDENTITY.json', 'CHANGELOG.md'}:
        raise ValueError('Release-default update escaped its two-file payload scope')
    write_manifest(root)
    return {'changed': True, 'version': next_version, 'tag': tag,
            'branch': 'release/default-version-' + next_version.replace('.', '-')}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--header', type=Path)
    parser.add_argument('--refresh-default-from-release', type=Path)
    parser.add_argument('--expected-tag', default='')
    args = parser.parse_args()
    if args.refresh_default_from_release:
        if args.header:
            parser.error('Header generation and release-default preparation are separate operations')
        result = refresh_release_default(ROOT, json.loads(
            args.refresh_default_from_release.read_text(encoding='utf-8')), args.expected_tag)
        print(json.dumps(result))
        if os.environ.get('GITHUB_OUTPUT'):
            with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8') as output:
                output.write('changed=' + str(result['changed']).lower() + '\n')
                if result['changed']:
                    output.write('branch=' + result['branch'] + '\nversion=' + result['version'] + '\n')
        raise SystemExit(0)
    version = selected_version()
    if args.header:
        write_header(args.header, version)
    print(version)
