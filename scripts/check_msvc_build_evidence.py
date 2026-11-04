# SPDX-License-Identifier: GPL-3.0-only

"""Bind the MSVC link audit to its exact DLL, cache and source receipt."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from check_msvc_shipping_link_command import collect_text, audit


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def check(root, build_dir, dll):
    root, build_dir, dll = map(Path, (root, build_dir, dll))
    receipt = json.loads((build_dir / 'BUILD_RECEIPT.json').read_text(encoding='utf-8-sig'))
    errors = []
    if receipt.get('build_succeeded') is not True or receipt.get('mode') != 'Golden' or receipt.get('configuration') != 'Release':
        errors.append('Missing successful Golden/Release MSVC build receipt')
    for field, path in [('dll_sha256', dll), ('cmake_cache_sha256', build_dir / 'CMakeCache.txt'),
                        ('source_manifest_sha256', root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')]:
        if receipt.get(field) != sha(path):
            errors.append('Build identity mismatch: ' + field)
    compiler = Path(receipt['compiler'])
    if compiler.name.lower() != 'cl.exe' or sha(compiler) != receipt.get('compiler_sha256'):
        errors.append('MSVC compiler tool identity mismatch')
    compiler_records = list((build_dir / 'CMakeFiles').glob('*/CMakeCXXCompiler.cmake'))
    if len(compiler_records) != 1 or not re.search(r'set\(CMAKE_CXX_COMPILER_ID\s+"MSVC"\)', compiler_records[0].read_text()):
        errors.append('Exact build compiler ID is not MSVC')
    command, kind, link_errors = collect_text(build_dir)
    errors.extend(link_errors)
    if not link_errors:
        errors.extend(audit(command))
    return errors


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('build_dir', type=Path)
    ap.add_argument('dll', type=Path)
    args = ap.parse_args()
    try:
        errors = check(Path(__file__).resolve().parents[1], args.build_dir, args.dll)
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('MSVC_BUILD_EVIDENCE:', 'FAIL' if errors else 'PASS_MSVC_BUILD')
    raise SystemExit(1 if errors else 0)
