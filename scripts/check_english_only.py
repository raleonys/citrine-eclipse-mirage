"""Read-only first-party language-signal audit; not automatic translation."""
import argparse
import hashlib
import json
import re
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
# Known non-English ASCII diagnostics are kept as fingerprints, not words.
# The final digest belongs to an English-only synthetic regression marker.
KNOWN_NON_ENGLISH_ASCII_FINGERPRINTS = frozenset({
    '2d52db38d807d5128c78a98a66a43b92b7a8ec4bf1fd50a2322108fb2c592c82',
    '8d0576636843ddc790f79738603e3f1264fe03418f8fbe51682c2e7b7d1011e9',
    '13925dbac99ceb99baa8be3812d150b9097982affedbc2476737a7f127a9871c',
    '27a50528cf95af48dcca17ff7d27c8f6a25703b2e5c32caa99bd5c938a47b279',
    '28b6703a721a90862e9dd6e75ebf760ba4bc8d4dae30b3a4f6d08d2cf735bbdc',
    '87d682e01e80ac31f46a49cb8f74351a19dd7961f645efc5e8803cd42f3c73d0',
    'ae556b1b86e54ea1d6ba981b80a052517767d7951ef450e9481e96993e6d057c',
    '75075457ad2afbe58edf4146d7943a752f0ffc88037f8b3b11c060f6d6a3d275',
    '6323e17e3c2958797a4d722d6db3c1410409dd06455a6cf474f6670ac4eb4454',
    'd94903d18ec80ade29fd50335e67bfeef977f143e733d7d4bf6f73051a3fc80c',
    'a66d22d8de1519d215a0836eaf5abad555fbeab29386a34658037c77fa27f957',
    'dca818b9e9c9c02d982f17c6d3b3bed57527141a3ebb1d11e878e80dee09e270',
})
ASCII_TOKEN = re.compile(r'(?<![A-Za-z])[A-Za-z]+(?![A-Za-z])')


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


def blocking_language_signal(name):
    """Unicode test data and structured metadata require separate review."""
    return not name.startswith(('tests/', 'metadata/'))


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
                             'severity': 'block' if blocking_language_signal(name) else 'review',
                             'codepoints': []})
            continue
        scanned += 1
        for number, line in enumerate(text.splitlines(), 1):
            letters = sorted({f'U+{ord(ch):04X}' for ch in line if is_letter_signal(ch)})
            if letters:
                findings.append({'path': name, 'line': number, 'kind': 'non_ascii_letters',
                                 'severity': 'block' if blocking_language_signal(name) else 'review',
                                 'codepoints': letters})
            for match in ASCII_TOKEN.finditer(line):
                fingerprint = hashlib.sha256(match.group(0).lower().encode('ascii')).hexdigest()
                if fingerprint in KNOWN_NON_ENGLISH_ASCII_FINGERPRINTS:
                    findings.append({'path': name, 'line': number,
                                     'kind': 'non_english_ascii_diagnostic',
                                     'severity': 'block', 'codepoints': []})
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
                        help='Fail on prohibited language signals; review Unicode data fixtures separately')
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
