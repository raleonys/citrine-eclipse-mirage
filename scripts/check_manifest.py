"""Explicitly refresh or verify the SURGE source identity."""
import argparse
from pathlib import Path
from project_files import write_manifest, verify_manifest


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--refresh', action='store_true', help='Record current bytes; not an acceptance claim')
    ap.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    args = ap.parse_args()
    try:
        if args.refresh:
            data = write_manifest(args.root)
            print('ACTIVE_SOURCE_MANIFEST: RECORDED', len(data['files']), 'files; NOT_RELEASE')
            return 0
        errors = verify_manifest(args.root)
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('ACTIVE_SOURCE_MANIFEST:', 'FAIL' if errors else 'PASS')
    return 1 if errors else 0


if __name__ == '__main__':
    raise SystemExit(main())
