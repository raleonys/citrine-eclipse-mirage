"""Read-only first-party language-signal audit; not automatic translation."""
import argparse
import json
from pathlib import Path, PurePosixPath
import sys
import unicodedata

ROOT = Path(__file__).resolve().parents[1]
TEXT_EXTENSIONS = frozenset({
    '.c', '.cmd', '.cmake', '.cpp', '.def', '.h', '.hpp', '.json',
    '.md', '.ps1', '.py', '.rc', '.txt', '.yml', '.yaml',
})
TEXT_NAMES = frozenset({'CMakeLists.txt', 'LICENSE', '.gitattributes', '.gitignore', '.github/hooks/pre-push'})
EXCLUDED_PREFIXES = ('third_party/', 'metadata/evidence/', 'metadata/vendor_files/')


def source_paths(manifest):
    """Use the pinned active-source inventory rather than untracked paths."""
    files = manifest.get('files')
    if not isinstance(files, dict):
        raise ValueError('Missing active-source file inventory')
    for name in sorted(files):
        if not isinstance(name, str):
            raise ValueError('Non-string file path')
        path = PurePosixPath(name)
        if path.is_absolute() or any(part in {'.', '..'} for part in name.split('/')):
            raise ValueError('Unsafe source path: ' + name)
        if name.startswith(EXCLUDED_PREFIXES):
            continue
        if path.suffix.lower() not in TEXT_EXTENSIONS and name not in TEXT_NAMES:
            continue
        yield name


def is_letter_signal(char):
    """Broad review signal; Unicode alone does not determine language."""
    return ord(char) > 127 and unicodedata.category(char).startswith('L')


def is_prose_document(name):
    return PurePosixPath(name).suffix.lower() == '.md'


def audit(root):
    root = Path(root)
    manifest_path = root / 'metadata/ACTIVE_SOURCE_MANIFEST.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    findings = []
    scanned = 0
    for name in source_paths(manifest):
        path = root.joinpath(*PurePosixPath(name).parts)
        if not path.is_file() or path.is_symlink():
            raise ValueError('Missing or nonregular active source: ' + name)
        try:
            text = path.read_text(encoding='utf-8')
        except UnicodeDecodeError:
            findings.append({'path': name, 'line': None, 'kind': 'invalid_utf8',
                             'severity': 'block' if is_prose_document(name) else 'review',
                             'codepoints': []})
            continue
        scanned += 1
        for number, line in enumerate(text.splitlines(), 1):
            letters = sorted({f'U+{ord(ch):04X}' for ch in line if is_letter_signal(ch)})
            if letters:
                findings.append({'path': name, 'line': number, 'kind': 'non_ascii_letters',
                                 'severity': 'block' if is_prose_document(name) else 'review',
                                 'codepoints': letters})
    return {'schema': 'surge.english-only-audit.v1',
            'classification': 'LANGUAGE_SIGNAL_AUDIT_NOT_SEMANTIC_CERTIFICATE',
            'scanned_text_files': scanned,
            'findings': findings,
            'blocking': sum(row['severity'] == 'block' for row in findings),
            'review_required': sum(row['severity'] == 'review' for row in findings)}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--check', action='store_true',
                        help='Fail on prose-document signals, not review-only code/fixture data')
    parser.add_argument('--json', action='store_true', help='Print deterministic full JSON report')
    args = parser.parse_args(argv)
    try:
        report = audit(args.root)
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as error:
        print('ENGLISH_ONLY_AUDIT: ERROR:', error, file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, sort_keys=True, indent=2, ensure_ascii=True))
    else:
        print('ENGLISH_ONLY_AUDIT: files={scanned_text_files} blocking={blocking} '
              'review={review_required}'.format(**report))
        for finding in report['findings'][:15]:
            print(' - {path}:{line}: {severity} ({kind}: {codepoints})'.format(**finding))
        if len(report['findings']) > 15:
            print(' - additional findings omitted; run --json for the complete report')
    return 1 if args.check and report['blocking'] else 0


if __name__ == '__main__':
    raise SystemExit(main())
