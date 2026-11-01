# SPDX-License-Identifier: GPL-3.0-only

"""Verify the owner-preserved research package before excluding it from product inputs."""
import hashlib
import json
from pathlib import Path
import re

REFERENCE_ROOT = 'reference-package'
POLICY = 'docs/RE_PACKAGE_IMPORT.json'


def file_digest(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def verify_reference_package(root, safe_name):
    root = Path(root)
    base = root / REFERENCE_ROOT
    policy_path = root / POLICY
    if not base.exists() and not base.is_symlink():
        # Flat product packages intentionally omit research bodies. A Git
        # checkout carrying the registry must retain its registered payload.
        if policy_path.exists() and (root / '.git').exists():
            raise ValueError('Registered research package is missing from checkout')
        return
    if base.is_symlink() or not base.is_dir() or policy_path.is_symlink():
        raise ValueError('Unsafe research package root or registry')
    policy = json.loads(policy_path.read_text(encoding='utf-8'))
    if policy.get('schema') != 'cem.reference-package.v1':
        raise ValueError('Unsupported research package registry')
    index_name = safe_name(policy['inventory'])
    if index_name != 'docs/RE_PACKAGE_FILES.json':
        raise ValueError('Unowned research package inventory')
    index_path = root / index_name
    if index_path.is_symlink() or file_digest(index_path) != policy['inventory_sha256']:
        raise ValueError('Research package inventory digest differs')
    index = json.loads(index_path.read_text(encoding='utf-8'))
    if index['archive_sha256'] != policy['archive_sha256']:
        raise ValueError('Research package archive identity differs')
    records = {}
    folded = set()
    def register(name, row):
        safe_name(name)
        if name.casefold() in folded:
            raise ValueError('Duplicate research package path: ' + name)
        if (type(row['bytes']) is not int or row['bytes'] < 0 or
                not re.fullmatch('[a-f0-9]{64}', row['sha256'])):
            raise ValueError('Invalid research package byte identity')
        folded.add(name.casefold())
        records[name] = row
    for row in index['files']:
        register('snapshot/' + safe_name(row['path']), row)
    if len(index['files']) != policy['snapshot_files']:
        raise ValueError('Research package snapshot count differs')
    actual = {}
    actual_folded = set()
    for path in base.rglob('*'):
        name = safe_name(path.relative_to(base).as_posix())
        if path.is_symlink() or (not path.is_file() and not path.is_dir()):
            raise ValueError('Unsafe research package member: ' + name)
        if name.casefold() in actual_folded:
            raise ValueError('Case-colliding research package member: ' + name)
        actual_folded.add(name.casefold())
        if path.is_file():
            actual[name] = path
    if set(actual) != set(records):
        raise ValueError('Research package file set differs: missing=' +
                         str(sorted(set(records) - set(actual))) + ', extra=' +
                         str(sorted(set(actual) - set(records))))
    for name, row in records.items():
        path = actual[name]
        if path.stat().st_size != row['bytes'] or file_digest(path) != row['sha256']:
            raise ValueError('Research package file differs: ' + name)


if __name__ == '__main__':
    from project_files import safe_member
    verify_reference_package(Path(__file__).resolve().parents[1], safe_member)
    print('REFERENCE_PACKAGE: PASS_BYTES_NOT_REVIEW_OR_EXECUTION')
