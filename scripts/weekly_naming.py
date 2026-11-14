# SPDX-License-Identifier: GPL-3.0-only
"""Owner-approved weekly names, version arithmetic, and LUTRAVEX root safety."""
import re
from pathlib import Path

from project_files import load_unique_json
FILE = Path(__file__).resolve().parents[1] / 'metadata/WEEKLY_NAMING_POLICY.json'
FIRST = '3.2.6'
V = re.compile(r'([1-9][0-9]*)\.([0-9])\.([0-9])\Z')
T = re.compile(r'([1-9][0-9]*\.[0-9]\.[0-9])-([A-Z][A-Z0-9]{2,31})\Z')
N = re.compile(r'[A-Z][A-Z0-9]{2,31}\Z')


def semver(v):
    match = V.fullmatch(v)
    if not match:
        raise ValueError('Minor and patch must each be 0..9')
    return tuple(map(int, match.groups()))


def successor(v):
    major, minor, patch = semver(v)
    return (f'{major}.{minor}.{patch+1}' if patch < 9 else
            f'{major}.{minor+1}.0' if minor < 9 else f'{major+1}.0.0')


def tag_parts(tag):
    match = T.fullmatch(tag)
    if not match:
        raise ValueError('Expected x.y.z-CODENAME without RNN suffix')
    return match.groups()


def asset_names(tag):
    version, codename = tag_parts(tag)
    base = f'Laureondo_{version}_{codename}'
    return base + '_source.zip', base + '.zip'


def load_registry():
    return load_unique_json(FILE)


def validate_registry(data):
    if (not isinstance(data, dict) or data.get('schema') != 'laureondo.weekly.naming.v1'
            or data.get('first_version') != FIRST or data.get('project') != 'Laureondo'):
        raise ValueError('Wrong LUTRAVEX naming authority')
    reserved, names = data.get('historical_reserved_roots'), data.get('approved_codenames')
    if not isinstance(reserved, list) or reserved != sorted(set(reserved)) or not isinstance(names, dict) or not names:
        raise ValueError('Invalid historical roots or approved codename registry')
    used, last = set(reserved), None
    for version, name in names.items():
        semver(version)
        if (last is None and version != FIRST) or (last is not None and version != successor(last)):
            raise ValueError('Gap in approved semantic versions')
        if not isinstance(name, str) or not N.fullmatch(name) or name in used:
            raise ValueError('Codename collision with global LUTRAVEX historical root')
        used.add(name)
        last = version
    return names


def approved_tag(version):
    names = validate_registry(load_registry())
    if version not in names:
        raise ValueError('No owner-approved codename for this semantic version')
    return version + '-' + names[version]


def require_tag(tag):
    version, _ = tag_parts(tag)
    if approved_tag(version) != tag:
        raise ValueError('Unapproved version/codename binding')
    return version


if __name__ == '__main__':
    print('LUTRAVEX_WEEKLY: PASS', len(validate_registry(load_registry())))
