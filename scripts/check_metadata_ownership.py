"""Validate metadata and script registry schema, ownership and role contracts."""
import json
from pathlib import Path

METADATA_SCHEMA = 'surge.metadata-ownership.v1'
SCRIPT_SCHEMA = 'surge.script-ownership.v1'
METADATA_ROLES = frozenset({
    'CURRENT_CONTRACT', 'CURRENT_CONTRACT_OR_PROVENANCE',
    'CURRENT_CONTRACT_AND_EVIDENCE', 'CURRENT_VENDOR_CONTRACT_OR_VALIDATION',
})
SCRIPT_ROLES = frozenset({'AUTOMATION', 'HUMAN_LAUNCHER', 'PYTHON_TOOL_OR_MODULE'})


def check(root):
    root = Path(root)
    metadata = json.loads((root / 'metadata/METADATA_OWNERSHIP.json').read_text())
    scripts = json.loads((root / 'metadata/SCRIPT_OWNERSHIP.json').read_text())
    errors = []
    if metadata.get('schema') != METADATA_SCHEMA:
        errors.append('Unrecognized metadata ownership schema')
    if scripts.get('schema') != SCRIPT_SCHEMA:
        errors.append('Unrecognized script ownership schema')
    registry = metadata['files']
    script_registry = scripts['files']
    if not isinstance(registry, dict) or not isinstance(script_registry, dict):
        return sorted(errors + ['Ownership registry files must be a mapping'])

    actual = {p.name for p in (root / 'metadata').glob('*.json')}
    errors += ['Unowned metadata: ' + n for n in actual - registry.keys()]
    errors += ['Missing owned metadata: ' + n for n in registry.keys() - actual]
    for name, record in registry.items():
        if (not isinstance(record, dict) or not record.get('owner')
                or not record.get('purpose') or record.get('role') not in METADATA_ROLES):
            errors.append('Incomplete or invalid metadata ownership: ' + name)
    actual_scripts = {p.relative_to(root).as_posix() for directory in ['scripts', 'tools']
                      for p in (root / directory).rglob('*') if p.suffix in {'.py', '.ps1', '.cmd'}}
    if actual_scripts != set(script_registry):
        errors.append('Script ownership file-set mismatch')
    for name, record in script_registry.items():
        if (not isinstance(record, dict) or not record.get('owner')
                or not record.get('purpose') or record.get('role') not in SCRIPT_ROLES):
            errors.append('Incomplete or invalid script ownership: ' + name)
    return sorted(errors)


if __name__ == '__main__':
    try:
        errors = check(Path(__file__).resolve().parents[1])
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('METADATA_OWNERSHIP:', 'FAIL' if errors else 'PASS')
    raise SystemExit(1 if errors else 0)
