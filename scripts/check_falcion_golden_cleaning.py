# SPDX-License-Identifier: GPL-3.0-only

"""Infrastructure FALCION boundaries adapted to the SURGE source product."""
import argparse
import json
from pathlib import Path
import stat
import zipfile
from project_files import MANIFEST, ALIAS_STORAGE, digest, safe_member, source_files, verify_manifest, vendor_symlinks


def check(root, package=None):
    root = Path(root)
    files = source_files(root)
    errors = verify_manifest(root)
    links=vendor_symlinks(root)
    portable = json.loads((root / MANIFEST).read_text(encoding='utf-8')).get('vendor_alias_storage') == ALIAS_STORAGE
    contract = json.loads((root / 'metadata/REPOSITORY_CONTRACT.json').read_text(encoding='utf-8'))
    for name in contract['required_files']:
        if name not in files:
            errors.append('Required source file missing: ' + name)
    actual_dirs = {n.split('/')[0] for n in files if '/' in n}
    if actual_dirs != set(contract['source_directories']):
        errors.append('Product root directory set differs from REPOSITORY_CONTRACT')
    if package is None:
        return errors
    # The source tree is verified before ZIP inspection. Its exact byte sizes
    # bound any member we are willing to decompress from an external archive.
    expected = dict(files)
    expected[MANIFEST] = (root / MANIFEST).read_bytes()
    with zipfile.ZipFile(package) as archive:
        infos = archive.infolist()
        seen, folded, preflight_errors = set(), set(), []
        for info in infos:
            name = safe_member(info.filename)
            if name in seen or name.casefold() in folded:
                preflight_errors.append('Duplicate/case-fold package member: ' + name)
            seen.add(name)
            folded.add(name.casefold())
            if info.is_dir():
                preflight_errors.append('Directory entry outside flat source contract: ' + name)
                continue
            is_link = stat.S_ISLNK(info.external_attr >> 16)
            if is_link != (name in links and not portable):
                preflight_errors.append('Package source/link type mismatch: ' + name)
            mode = info.external_attr >> 16
            if portable and mode and not stat.S_ISREG(mode):
                preflight_errors.append('Portable source package requires regular files: ' + name)
            if name not in expected:
                preflight_errors.append('Unexpected package member: ' + name)
            elif info.file_size != len(expected[name]):
                preflight_errors.append('Package member size differs from verified source: ' + name)
                if name == MANIFEST:
                    preflight_errors.append('Package/source manifest identity mismatch')
        if preflight_errors:
            # Do not decompress even the earlier valid members when a later
            # entry is foreign or larger than its manifest-verified source.
            return errors + preflight_errors
        package_files = {info.filename: archive.read(info) for info in infos}
        if archive.testzip():
            errors.append('ZIP CRC failure')
    if set(package_files) != set(expected):
        errors.append('Flat package/source file-set mismatch')
    for name in package_files.keys() & expected.keys():
        if digest(package_files[name]) != digest(expected[name]):
            errors.append('Package differs from verified source: ' + name)
    if package_files.get(MANIFEST) != expected[MANIFEST]:
        errors.append('Package/source manifest identity mismatch')
    return errors


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    ap.add_argument('--package', type=Path)
    args = ap.parse_args()
    try:
        errors = check(args.root, args.package)
    except (OSError, ValueError, KeyError, zipfile.BadZipFile) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('FALCION_GOLDEN_CLEANING:', 'FAIL' if errors else 'PASS_SOURCE_PACKAGE_ONLY')
    return 1 if errors else 0


if __name__ == '__main__':
    raise SystemExit(main())
