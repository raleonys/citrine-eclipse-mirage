"""Remove only bytecode caches belonging to manifest-owned first-party Python tools."""
import argparse
import json
from pathlib import Path
import re


def clear(root):
    root = Path(root).resolve()
    manifest = json.loads((root / 'metadata/ACTIVE_SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
    removed = []
    for name in manifest['files']:
        source = root / name
        if source.suffix != '.py' or source.is_symlink() or not source.is_file():
            continue
        if Path(name).parts[0] not in ('scripts', 'tools', 'tests'):
            continue
        if root not in source.resolve().parents:
            continue
        cache = source.parent / '__pycache__'
        if cache.is_symlink() or not cache.is_dir() or root not in cache.resolve().parents:
            continue
        pattern = re.compile(re.escape(source.stem) + r'\.(?:cpython|pypy)-[A-Za-z0-9_-]+(?:\.opt-[0-9]+)?\.pyc')
        for path in cache.iterdir():
            if path.is_file() and not path.is_symlink() and pattern.fullmatch(path.name):
                path.unlink()
                removed.append(path.relative_to(root).as_posix())
        if not any(cache.iterdir()):
            cache.rmdir()
    return sorted(removed)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--quiet', action='store_true', help='Keep successful cleanup off the console')
    parser.add_argument('--report', type=Path, help='Write the detailed cleanup result as JSON')
    args = parser.parse_args()
    try:
        report = {'result':'CLEARED_KNOWN_GENERATED_CACHE_ONLY','removed':clear(args.root)}
        output = json.dumps(report, indent=2) + '\n'
        if args.report:
            args.report.write_text(output, encoding='utf-8')
        if not args.quiet:
            print(output, end='')
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, 'TOOL_CACHE_CLEANUP: FAIL: ' + str(error) + '\n')
