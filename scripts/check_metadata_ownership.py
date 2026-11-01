# SPDX-License-Identifier: GPL-3.0-only

"""Infrastructure ownership-ledger checks, without Infrastructure release assumptions."""
import json
from pathlib import Path


def check(root):
    root = Path(root)
    registry = json.loads((root / 'metadata/METADATA_OWNERSHIP.json').read_text())['files']
    actual = {p.name for p in (root / 'metadata').glob('*.json')}
    errors = ['Unowned metadata: ' + n for n in actual - registry.keys()]
    errors += ['Missing owned metadata: ' + n for n in registry.keys() - actual]
    for name, record in registry.items():
        if not record.get('owner') or not record.get('purpose') or not record.get('role'):
            errors.append('Incomplete ownership: ' + name)
    script_registry = json.loads((root / 'metadata/SCRIPT_OWNERSHIP.json').read_text())['files']
    actual_scripts = {p.relative_to(root).as_posix() for directory in ['scripts', 'tools']
                      for p in (root / directory).rglob('*') if p.suffix in {'.py', '.ps1', '.cmd'}}
    if actual_scripts != set(script_registry):
        errors.append('Script ownership file-set mismatch')
    for name, record in script_registry.items():
        if not record.get('owner') or not record.get('purpose'):
            errors.append('Incomplete script ownership: ' + name)
    return sorted(errors)


if __name__ == '__main__':
    try:
        errors = check(Path(__file__).resolve().parents[1])
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('METADATA_OWNERSHIP:', 'FAIL' if errors else 'PASS')
    raise SystemExit(1 if errors else 0)
