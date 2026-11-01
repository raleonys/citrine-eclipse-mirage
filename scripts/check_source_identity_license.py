# SPDX-License-Identifier: GPL-3.0-only
"""Fail-closed source namespace and SPDX checks for owned first-party code.

Vendor payloads, vendor-derived tests and binary fixtures are deliberately excluded.
Historical audit documents and strings are not part of executable namespace identity.
The byte-locked VERSIONINFO source is excluded to preserve exact adopted PE bytes.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
OWNED_TOP = ('src', 'include', 'tests', 'tools', 'scripts', 'cmake', '.github')
EXCLUDED = ('third_party/', 'tests/vendors/', 'tests/fixtures/', 'docs/evidence/', 'metadata/evidence/')
C_FAMILY = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hpp', '.hxx', '.rc'}
SCRIPT = {'.py', '.sh', '.bash', '.ps1', '.cmake', '.yml', '.yaml', '.cmd'}
LEGACY = re.compile(r'\b(?:[A-Za-z_]\w*_recovered|Recovered[A-Z]\w*)\b|\b(?:namespace\s+preloader_l::recovered|recovered::)')
IDENTIFIER = re.compile(r'(?mi)^\s*(?:#|//|/\*+|\*|REM|;)\s*SPDX-License-Identifier:\s*([A-Za-z0-9.+-]+)\s*(?:\*/)?\s*$')


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
    for name in ('CMakeLists.txt', 'resources/surge.rc', 'exports/preloader_l.def'):
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
        ids = IDENTIFIER.findall(data[:3000])
        if ids != ['GPL-3.0-only']:
            errors.append(f'Expected exactly one GPL-3.0-only SPDX header: {rel} {ids}')
        if path.suffix.lower() in C_FAMILY and LEGACY.search(data):
            errors.append('Legacy recovery-derived C++ identity: ' + rel)
    return checked, errors


if __name__ == '__main__':
    count, problems = inspect()
    for problem in problems:
        print(' -', problem)
    print(f'SOURCE_IDENTITY_SPDX: {"PASS" if not problems else "FAIL"} ({count} checked)')
    raise SystemExit(bool(problems))
