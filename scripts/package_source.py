# SPDX-License-Identifier: GPL-3.0-only

"""Create a flat source ZIP from an already verified active source manifest."""
import argparse
import json
from pathlib import Path
import zipfile
from check_falcion_golden_cleaning import check
from project_files import MANIFEST, ALIAS_STORAGE, digest, source_files


def package(root, output):
    root = root.resolve()
    output = output.resolve()
    if output == root or root in output.parents:
        raise ValueError('Write the source ZIP outside the source tree.')
    errors = check(root)
    if errors:
        raise ValueError('; '.join(errors))
    files = source_files(root)
    files[MANIFEST] = (root / MANIFEST).read_bytes()
    if json.loads(files[MANIFEST]).get('vendor_alias_storage') != ALIAS_STORAGE:
        raise ValueError('Record the portable vendor alias storage policy before packaging')
    output.parent.mkdir(parents=True, exist_ok=True)
    partial = output.with_suffix(output.suffix + '.partial')
    try:
        with zipfile.ZipFile(partial, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
            for name, data in sorted(files.items()):
                info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.create_system = 3
                # The two locked alias payloads remain byte-exact but use normal
                # files. Windows ZIP tools do not need to create Unix symlinks.
                info.external_attr = 0o100644 << 16
                archive.writestr(info, data)
        errors = check(root, partial)
        if errors:
            raise ValueError('; '.join(errors))
        partial.replace(output)
    finally:
        if partial.exists():
            partial.unlink()
    checksum = digest(output.read_bytes())
    sidecar = Path(str(output) + '.sha256')
    sidecar_partial = Path(str(sidecar) + '.partial')
    try:
        sidecar_partial.write_text(checksum + '  ' + output.name + '\n', encoding='utf-8')
        sidecar_partial.replace(sidecar)
    finally:
        sidecar_partial.unlink(missing_ok=True)
    return {'path': str(output), 'files': len(files), 'bytes': output.stat().st_size,
            'sha256': checksum, 'sha256_file': str(sidecar), 'root': 'FLAT'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(package(args.root, args.output), indent=2))
