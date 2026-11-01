# SPDX-License-Identifier: GPL-3.0-only

"""Stage, compare and optionally import an owner-supplied ZIP without executing it."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import stat
import tempfile
import zipfile

from project_files import safe_member
from reference_package import POLICY, REFERENCE_ROOT, file_digest, verify_reference_package

INDEX = 'docs/RE_PACKAGE_FILES.json'
REVIEWS = 'docs/RE_PACKAGE_FILE_REVIEWS.json'
UPDATES = 'docs/RE_PACKAGE_UPDATES.json'


def encoded(data):
    return (json.dumps(data, indent=2, ensure_ascii=True) + '\n').encode('utf-8')


def load_optional(root, name, default):
    path = root / name
    if path.is_symlink():
        raise ValueError('Unsafe research record: ' + name)
    return json.loads(path.read_text(encoding='utf-8')) if path.exists() else default


def review_rows(files, previous):
    old = {row['path']: row for row in previous}
    rows = []
    for entry in files:
        prior = old.get(entry['path'])
        if prior and prior['source_sha256'] == entry['sha256']:
            rows.append(prior)
            continue
        row = {'path': entry['path'], 'source_sha256': entry['sha256'],
               'status': 'pending', 'compared_product_paths': [],
               'finding_ids': [], 'review_record': None, 'prior_reviews': []}
        if prior:
            row['prior_reviews'] = prior.get('prior_reviews', [])
            if prior['status'] != 'pending' or prior.get('finding_ids'):
                row['prior_reviews'] = row['prior_reviews'] + [
                    {key: value for key, value in prior.items() if key != 'prior_reviews'}]
        rows.append(row)
    return rows


def import_package(root, archive, label, apply=False):
    root, archive = Path(root).resolve(), Path(archive).resolve()
    # Keep uploaded content outside the folder replaced by this operation.
    if archive.is_relative_to(root / REFERENCE_ROOT):
        raise ValueError('Input ZIP must be outside the research payload directory')
    verify_reference_package(root, safe_member)
    old_index = load_optional(root, INDEX, {'files': [], 'archive_sha256': None})
    old_reviews = load_optional(root, REVIEWS, {'files': []})
    journal = load_optional(root, UPDATES, {'schema': 'cem.reference-updates.v1', 'updates': []})
    staging_parent = root / 'build'
    staging_parent.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='reference-import-', dir=staging_parent) as temp:
        stage = Path(temp)
        payload = stage / REFERENCE_ROOT
        (payload / 'snapshot').mkdir(parents=True)
        archive_sha = file_digest(archive)
        files, names = [], set()
        with zipfile.ZipFile(archive) as bundle:
            entries = bundle.infolist()
            # Validate every member before extracting anything from the ZIP.
            for entry in entries:
                name = safe_member(entry.filename[:-1] if entry.is_dir() else entry.filename)
                mode = entry.external_attr >> 16
                if name.casefold() in names or stat.S_ISLNK(mode):
                    raise ValueError('Duplicate or symlink ZIP member: ' + name)
                if stat.S_IFMT(mode) not in (0, stat.S_IFREG, stat.S_IFDIR):
                    raise ValueError('Special ZIP member: ' + name)
                names.add(name.casefold())
            for entry in entries:
                target = payload / 'snapshot' / entry.filename
                if entry.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                    continue
                target.parent.mkdir(parents=True, exist_ok=True)
                digest = hashlib.sha256()
                size = 0
                with bundle.open(entry) as source, target.open('wb') as destination:
                    for block in iter(lambda: source.read(1024 * 1024), b''):
                        destination.write(block)
                        digest.update(block)
                        size += len(block)
                if size != entry.file_size:
                    raise ValueError('ZIP member extent differs: ' + entry.filename)
                files.append({'path': entry.filename, 'bytes': size, 'sha256': digest.hexdigest()})
        # A concurrently replaced input must not mix extraction and provenance.
        if file_digest(archive) != archive_sha:
            raise ValueError('Input archive changed during import')
        files.sort(key=lambda row: row['path'])
        old = {row['path']: row for row in old_index['files']}
        new = {row['path']: row for row in files}
        delta = {'added': sorted(new.keys() - old.keys()),
                 'removed': sorted(old.keys() - new.keys()),
                 'changed': sorted(name for name in new.keys() & old.keys()
                                   if new[name]['sha256'] != old[name]['sha256'])}
        delta['unchanged'] = len(new.keys() & old.keys()) - len(delta['changed'])
        index = {'schema': 'cem.reference-file-index.v1', 'archive_sha256': archive_sha,
                 'purpose': 'Preserved research bodies; not active product source inputs.', 'files': files}
        policy = {'schema': 'cem.reference-package.v1', 'project_branch': 'research/reconstruction-package',
                  'delivery_label': label, 'archive_sha256': archive_sha,
                  'archive_bytes': archive.stat().st_size,
                  'inventory': INDEX, 'inventory_sha256': hashlib.sha256(encoded(index)).hexdigest(),
                  'snapshot_files': len(files),
                  'product_adoption': 'Each confirmed finding is handled in a separate focused PR.'}
        reviews = {'schema': 'cem.reference-file-reviews.v1', 'archive_sha256': archive_sha,
                   'completion_rule': 'A matching feature or filename never completes a detailed review.',
                   'files': review_rows(files, old_reviews['files'])}
        journal['updates'].append({'delivery_label': label,
                                   'recorded_at': datetime.now(timezone.utc).isoformat(),
                                   'archive_sha256': archive_sha,
                                   'previous_archive_sha256': old_index.get('archive_sha256'),
                                   'snapshot_files': len(files), 'delta': delta})
        records = {INDEX: encoded(index), POLICY: encoded(policy),
                   REVIEWS: encoded(reviews), UPDATES: encoded(journal)}
        for name, data in records.items():
            target = stage / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        verify_reference_package(stage, safe_member)
        result = {'archive_sha256': archive_sha, 'files': len(files),
                  'applied': apply, 'delta': delta}
        if not apply:
            return result
        # Preserve the previous tree and records until replacement verifies.
        backup = stage / 'previous-payload'
        previous_records = {name: (root / name).read_bytes() if (root / name).exists() else None
                            for name in records}
        destination = root / REFERENCE_ROOT
        existed = destination.exists()
        if existed:
            destination.rename(backup)
        try:
            payload.rename(destination)
            for name, data in records.items():
                target = root / name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
            verify_reference_package(root, safe_member)
        except BaseException:
            if destination.exists():
                shutil.rmtree(destination)
            if existed:
                backup.rename(destination)
            for name, data in previous_records.items():
                if data is None:
                    (root / name).unlink(missing_ok=True)
                else:
                    (root / name).write_bytes(data)
            raise
        return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--label', required=True)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--apply', action='store_true', help='Apply verified payload and review-index changes')
    args = parser.parse_args()
    result = import_package(args.root, args.archive, args.label, args.apply)
    # Full per-file deltas stay in the tracked update journal on apply.
    print(json.dumps({key: value for key, value in result.items() if key != 'delta'}, indent=2))
    print(json.dumps({key: len(value) if isinstance(value, list) else value
                      for key, value in result['delta'].items()}, indent=2))


if __name__ == '__main__':
    main()
