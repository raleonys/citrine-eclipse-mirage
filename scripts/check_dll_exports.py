# SPDX-License-Identifier: GPL-3.0-only

"""Read-only validation of the application DLL's explicit C export contract."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from verify_corvexon_golden_trail import PE, PEError, u16


def check_source(root):
    root = Path(root)
    policy = json.loads((root / 'metadata/PUBLIC_DLL_ABI.json').read_text())
    lines = (root / 'exports/citrine_eclipse_mirage.def').read_text().splitlines()
    expected = {name: record['ordinal'] for name, record in policy['exports'].items()}
    actual = {}
    library_seen = False
    exports_seen = False
    for line in lines:
        line = line.strip()
        if not line or line.startswith(';'):
            continue
        if line == 'LIBRARY "citrine_eclipse_mirage.unsigned.dll"':
            if library_seen or exports_seen:
                return ['Missing, duplicate or out-of-order application DEF header: LIBRARY']
            library_seen = True
            continue
        if line == 'EXPORTS':
            if not library_seen or exports_seen:
                return ['Missing, duplicate or out-of-order application DEF header: EXPORTS']
            exports_seen = True
            continue
        if not exports_seen:
            return ['Application DEF export before required header: ' + line]
        match = re.fullmatch(r'(\w+)\s+@(\d+)', line)
        if not match or match[1] in actual:
            return ['Invalid or duplicate application DEF declaration: '+line]
        actual[match[1]] = int(match[2])
    if not library_seen or not exports_seen:
        return ['Missing application DEF LIBRARY or EXPORTS header']
    return [] if actual == expected else ['Application DEF differs from public C ABI contract']


def inspect(path, root):
    path, root = Path(path), Path(root)
    before = hashlib.sha256(path.read_bytes()).hexdigest()
    policy = json.loads((root / 'metadata/PUBLIC_DLL_ABI.json').read_text())
    expected = policy['exports']
    pe = PE(path)
    actual = pe.exports()
    errors = check_source(root)
    if u16(pe.b, pe.coff) != 0x8664:
        errors.append('Application C ABI requires AMD64')
    if set(actual) != set(expected):
        errors.append('Export names differ: missing=' + str(sorted(set(expected)-set(actual)))
                      + ', unexpected=' + str(sorted(set(actual)-set(expected))))
    if pe.export_counts != (len(expected), len(expected)):
        errors.append('Unexpected unnamed or additional export table entries')
    for name in set(actual) & set(expected):
        ordinal, rva, forwarded = actual[name]
        if ordinal != expected[name]['ordinal'] or not rva or forwarded:
            errors.append('Wrong ordinal, missing body or forwarded export: ' + name)
    if hashlib.sha256(path.read_bytes()).hexdigest() != before:
        errors.append('Read-only export validation changed the DLL')
    return {'schema': 'surge.public-abi-check.v1',
            'result': 'FAIL' if errors else 'PASS_PUBLIC_C_EXPORT_CONTRACT',
            'dll_sha256': before, 'export_count': len(actual),
            'exports': {name: {'ordinal': v[0], 'rva': hex(v[1]), 'forwarded': v[2]}
                        for name, v in sorted(actual.items())},
            'errors': errors, 'native_consumer_acceptance': 'NOT_RUN'}


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('dll', type=Path)
    ap.add_argument('--report', type=Path)
    args = ap.parse_args()
    try:
        report = inspect(args.dll, Path(__file__).resolve().parents[1])
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps(report, indent=2))
        raise SystemExit(bool(report['errors']))
    except (OSError, ValueError, KeyError, PEError) as error:
        print('PUBLIC_DLL_ABI: FAIL:', error)
        raise SystemExit(1)
