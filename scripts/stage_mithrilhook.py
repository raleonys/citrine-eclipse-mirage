# SPDX-License-Identifier: GPL-3.0-only

"""Materialize unchanged engine sources in an owned, hash-verified build root.

The supplied native test project requires control outputs below its own build/
directory. No vendor file or guard is patched to accommodate the parent build.
"""
import argparse
import hashlib
from pathlib import Path
import shutil
import tempfile

from vendor_sync import load_lock, expected_files, validate_installed


def owned_output(root, output):
    root, output = Path(root).resolve(), Path(output).absolute()
    if root / 'build' not in output.parents or output.is_symlink() or output.resolve() != output:
        raise ValueError('Engine staging must remain in the owned build directory')
    return root, output


def source_inventory(root):
    record = load_lock(root)['vendors']['mithrilhook']
    expected = expected_files(root, record)
    validate_installed(root, 'mithrilhook', record, expected, {})
    return record, expected


def verify(root, output):
    root, output = owned_output(root, output)
    _, expected = source_inventory(root)
    if not output.is_dir():
        raise ValueError('Staged engine source is missing')
    actual = {}
    # Only the engine's own generated build/ subtree is excluded. Its source,
    # scripts, CMake, licenses and complete member set remain exact byte inputs.
    for path in output.rglob('*'):
        rel = path.relative_to(output)
        if rel.parts[0] == 'build':
            continue
        if path.is_symlink() or path.resolve() != path:
            raise ValueError('Staged engine source cannot contain aliases')
        if path.is_file():
            actual[rel.as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
        elif not path.is_dir():
            raise ValueError('Unexpected staged source type')
    if actual != expected:
        changed = sorted(name for name in actual.keys() | expected.keys()
                         if actual.get(name) != expected.get(name))
        raise ValueError('Staged engine member/hash mismatch: ' + ', '.join(changed))
    return expected


def prepare(root, output):
    root, output = owned_output(root, output)
    record, _ = source_inventory(root)
    if output.exists():
        verify(root, output)  # Never silently repair or overwrite unknown bytes.
        return
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.mithril-stage-', dir=output.parent) as temporary:
        stage = Path(temporary) / 'source'
        shutil.copytree(root / record['source_install_path'], stage)
        verify(root, stage)
        stage.rename(output)
    verify(root, output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prepare', action='store_true')
    args = parser.parse_args()
    if args.prepare:
        prepare(args.root, args.output)
    members = verify(args.root, args.output)
    print('MITHRILHOOK_STAGE: PASS', len(members), 'unchanged members; no source patch')


if __name__ == '__main__':
    main()
