"""Create a flat source ZIP from an already verified active source manifest."""
import argparse
import json
import os
import shutil
from pathlib import Path
import tempfile
import zipfile
from check_falcion_golden_cleaning import check
from project_files import MANIFEST, ALIAS_STORAGE, digest, source_files


def package(root, output):
    root = root.resolve()
    # abspath normalizes caller input without following a final symlink.
    output = Path(os.path.abspath(output))
    if output.is_symlink():
        raise ValueError('Refusing a symlinked source ZIP destination')
    if output == root or root in output.parents or root in output.parent.resolve().parents:
        raise ValueError('Write the source ZIP outside the source tree.')
    errors = check(root)
    if errors:
        raise ValueError('; '.join(errors))
    files = source_files(root)
    files[MANIFEST] = (root / MANIFEST).read_bytes()
    if json.loads(files[MANIFEST]).get('vendor_alias_storage') != ALIAS_STORAGE:
        raise ValueError('Record the portable vendor alias storage policy before packaging')
    output.parent.mkdir(parents=True, exist_ok=True)
    sidecar = Path(str(output) + '.sha256')
    if sidecar.is_symlink() or sidecar.is_dir():
        raise ValueError('Unsafe existing source ZIP checksum destination')
    # Do not adopt the former fixed .partial paths, even if they are dangling links.
    legacy_partial = output.with_suffix(output.suffix + '.partial')
    legacy_checksum_partial = Path(str(sidecar) + '.partial')
    for path in (legacy_partial, legacy_checksum_partial):
        if path.is_symlink() or path.exists():
            raise ValueError('Unsafe existing source package partial path: ' + str(path))

    existing_zip = output.exists()
    existing_sidecar = sidecar.exists()
    if existing_zip != existing_sidecar:
        raise ValueError('Refusing to replace an incomplete source ZIP/checksum pair')
    if existing_zip:
        expected_sidecar = digest(output.read_bytes()) + '  ' + output.name + '\n'
        if sidecar.read_text(encoding='utf-8') != expected_sidecar:
            raise ValueError('Existing source ZIP/checksum pair is inconsistent')

    fd, name = tempfile.mkstemp(prefix='.' + output.name + '.',
                                suffix='.partial', dir=output.parent)
    partial = Path(name)
    checksum_partial = None
    rollback_zip = None
    try:
        # The ZIP is written through an exclusively created descriptor and
        # validated before either published artifact is changed.
        with os.fdopen(fd, 'w+b') as stream:
            with zipfile.ZipFile(stream, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
                for member, data in sorted(files.items()):
                    info = zipfile.ZipInfo(member, date_time=(2026, 1, 1, 0, 0, 0))
                    info.compress_type = zipfile.ZIP_DEFLATED
                    info.create_system = 3
                    info.external_attr = 0o100644 << 16
                    archive.writestr(info, data)
        errors = check(root, partial)
        if errors:
            raise ValueError('; '.join(errors))
        checksum = digest(partial.read_bytes())
        checksum_line = checksum + '  ' + output.name + '\n'

        # A failed checksum write/creation must not publish the new ZIP.
        checksum_fd, checksum_name = tempfile.mkstemp(
            prefix='.' + sidecar.name + '.', suffix='.partial', dir=output.parent)
        checksum_partial = Path(checksum_name)
        with os.fdopen(checksum_fd, 'w', encoding='utf-8') as stream:
            stream.write(checksum_line)

        if existing_zip:
            # Keep the previous matching ZIP available until both new files
            # have been committed. A sidecar commit error restores the old ZIP.
            rollback_fd, rollback_name = tempfile.mkstemp(
                prefix='.' + output.name + '.', suffix='.rollback', dir=output.parent)
            rollback_zip = Path(rollback_name)
            with output.open('rb') as previous, os.fdopen(rollback_fd, 'wb') as backup:
                shutil.copyfileobj(previous, backup)

        zip_published = False
        try:
            os.replace(partial, output)
            zip_published = True
            os.replace(checksum_partial, sidecar)
        except OSError:
            if zip_published:
                # The prior sidecar has not been replaced: restore its ZIP.
                # This is exception rollback, not power-loss multi-file atomicity.
                if rollback_zip is not None:
                    try:
                        os.replace(rollback_zip, output)
                    except OSError as rollback_error:
                        raise RuntimeError(
                            'Source ZIP/checksum rollback failed; output may be incomplete'
                        ) from rollback_error
                else:
                    try:
                        output.unlink()
                    except OSError as rollback_error:
                        raise RuntimeError(
                            'Cannot remove incomplete source ZIP publication'
                        ) from rollback_error
            raise

        return {'path': str(output), 'files': len(files), 'bytes': output.stat().st_size,
                'sha256': checksum, 'sha256_file': str(sidecar), 'root': 'FLAT'}
    finally:
        partial.unlink(missing_ok=True)
        if checksum_partial is not None:
            checksum_partial.unlink(missing_ok=True)
        if rollback_zip is not None:
            rollback_zip.unlink(missing_ok=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(package(args.root, args.output), indent=2))
