# SPDX-License-Identifier: GPL-3.0-only
"""One version selection for Windows resources, Golden checks and build receipts."""
import argparse
import copy
import json
import os
from pathlib import Path
import re

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
    selected = selected_version(root)
    major, minor, patch, revision = components(selected)
    info = policy['required']['versioninfo']
    for kind in ('File', 'Product'):
        info['fixed']['dw' + kind + 'VersionMS'] = f'0x{(major << 16) | minor:08X}'
        info['fixed']['dw' + kind + 'VersionLS'] = f'0x{(patch << 16) | revision:08X}'
        info['strings'][kind + 'Version'] = selected
    return policy


def write_header(path, version):
    values = ','.join(map(str, components(version)))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('// SPDX-License-Identifier: GPL-3.0-only\n'
                    '// Generated from the selected project version; do not edit.\n'
                    f'#define CEM_VERSION_NUMERIC {values}\n'
                    f'#define CEM_VERSION_STRING "{version}\\0"\n', encoding='utf-8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--header', type=Path)
    args = parser.parse_args()
    version = selected_version()
    if args.header:
        write_header(args.header, version)
    print(version)
