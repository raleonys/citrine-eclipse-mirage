# SPDX-License-Identifier: GPL-3.0-only
"""Fail-closed source namespace and SPDX checks for owned first-party code.

Vendor payloads, vendor-derived tests and binary fixtures are deliberately excluded.
Historical audit documents and strings are not part of executable namespace identity.
The project VERSIONINFO source follows the same first-party license policy.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
OWNED_TOP = ('src', 'include', 'tests', 'tools', 'scripts', 'cmake', '.github')
EXCLUDED = ('third_party/', 'tests/vendors/', 'tests/fixtures/', 'docs/evidence/', 'metadata/evidence/')
C_FAMILY = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hpp', '.hxx', '.rc'}
SCRIPT = {'.py', '.sh', '.bash', '.ps1', '.cmake', '.yml', '.yaml', '.cmd'}
LEGACY = re.compile(r'\b(?:[A-Za-z_]\w*_recovered|Recovered[A-Z]\w*)\b|\b(?:namespace\s+laureondo::recovered|recovered::)')
IDENTIFIER = re.compile(r'(?mi)^\s*(?:#|//|/\*+|\*|REM|;)\s*SPDX-License-Identifier:\s*([A-Za-z0-9.+-]+)\s*(?:\*/)?\s*$')

RECOVERY_WORDING = re.compile(r'recovered', re.I)


def has_initial_spdx_header(data, path):
    """Require the owned license marker before source, after only a launcher prefix."""
    lines = data.splitlines()
    if lines and (lines[0].startswith('#!') or
                  (path.suffix.lower() == '.cmd' and lines[0].strip().lower() == '@echo off')):
        lines = lines[1:]
    return bool(lines and IDENTIFIER.fullmatch(lines[0]) and
                IDENTIFIER.fullmatch(lines[0]).group(1) == 'GPL-3.0-only')


def candidates(root):
    for prefix in OWNED_TOP:
        base = root / prefix
        if not base.is_dir():
            continue
        for path in sorted(base.rglob('*')):
            if not path.is_file() or path.is_symlink():
                continue
            rel = path.relative_to(root).as_posix()
            if rel.startswith(EXCLUDED):
                continue
            if path.suffix.lower() in C_FAMILY | SCRIPT or path.name in ('CMakeLists.txt', 'pre-push'):
                yield path
    for name in ('CMakeLists.txt', 'resources/surge.rc', 'resources/version.rc', 'exports/laureondo.def'):
        path = root / name
        if path.is_file() and not path.is_symlink():
            yield path


def inspect(root=ROOT):
    root = Path(root)
    errors = []
    checked = 0
    for path in candidates(root):
        rel = path.relative_to(root).as_posix()
        try:
            data = path.read_text(encoding='utf-8')
        except UnicodeError:
            errors.append('Source is not UTF-8: ' + rel)
            continue
        checked += 1
        ids = IDENTIFIER.findall(data)
        if ids != ['GPL-3.0-only']:
            errors.append(f'Expected exactly one GPL-3.0-only SPDX header: {rel} {ids}')
        elif not has_initial_spdx_header(data, path):
            errors.append('SPDX header must precede source content: ' + rel)
        if path.suffix.lower() in C_FAMILY and LEGACY.search(data):
            errors.append('Legacy recovery-derived C++ identity: ' + rel)
        if rel.startswith(('src/', 'include/')) and RECOVERY_WORDING.search(data):
            errors.append('Legacy recovery wording in active source: ' + rel)
        if rel.startswith(('src/', 'include/')):
            identity = data
            if rel == 'include/laureondo/platform/crypt_query.hpp':
                identity = identity.replace('target_suffix=u"preloader' + '_l.dll"', 'target_suffix=u"compatibility.dll"')
            if re.search(r'preloader' + r'_l', identity, re.I):
                errors.append('Retired project identity outside CryptQuery target: ' + rel)
    return checked, errors


if __name__ == '__main__':
    count, problems = inspect()
    for problem in problems:
        print(' -', problem)
    print(f'SOURCE_IDENTITY_SPDX: {"PASS" if not problems else "FAIL"} ({count} checked)')
    raise SystemExit(bool(problems))
