"""Check source scope, native CMake source ownership and pinned SURGE resource."""
import hashlib
import json
from pathlib import Path
import re


def load_active_name_policy(root):
    text=(Path(root)/'docs/IDENTIFIER_POLICY.md').read_text(encoding='utf-8')
    match=re.search(r'<!-- active-name-policy -->\s*```json\s*(.*?)\s*```',text,re.S)
    if not match:
        raise ValueError('Active identifier policy is missing')
    labels=json.loads(match[1])['reserved_reference_labels']
    if not labels or any(not isinstance(value,str) or not value for value in labels):
        raise ValueError('Invalid active identifier policy')
    return labels


def check_active_identifiers(root):
    root=Path(root)
    pattern=re.compile('|'.join(re.escape(value) for value in load_active_name_policy(root)),re.I)
    errors=[]
    for path in root.rglob('*'):
        rel=path.relative_to(root)
        if rel.parts[0] in {'build','dist','.git','third_party'} or not path.is_file():
            continue
        name=rel.as_posix()
        if pattern.search(name):
            errors.append('Reserved reference label in active path: '+name)
        legal_document=len(rel.parts)==1 and path.name in {'LICENSE','COPYING','NOTICE','AUTHORS'}
        if rel.parts[0] in {'docs','governance'} or path.suffix.lower()=='.md' or legal_document:
            continue
        try:
            text=path.read_text(encoding='utf-8-sig')
        except UnicodeError:
            continue
        if pattern.search(text):
            errors.append('Reserved reference label outside documents: '+name)
    return errors



def check_root_file_ownership(root, allowed_root_files):
    """Reject unowned files at repository root independently of the source manifest."""
    if (not isinstance(allowed_root_files, list) or not allowed_root_files
            or any(not isinstance(name, str) or not name
                   or name in {'.', '..'} or '/' in name or '\\' in name
                   for name in allowed_root_files)
            or len(set(allowed_root_files)) != len(allowed_root_files)):
        raise ValueError('Invalid allowed_root_files repository policy')
    allowed = set(allowed_root_files)
    return ['Unowned root file: ' + path.name
            for path in sorted(Path(root).iterdir())
            if (path.is_file() or path.is_symlink()) and path.name not in allowed]

def check(root):
    root = Path(root)
    errors = check_active_identifiers(root)
    contract = json.loads((root / 'metadata/REPOSITORY_CONTRACT.json').read_text())
    errors.extend(check_root_file_ownership(root, contract['allowed_root_files']))
    for name in contract['required_files']:
        if not (root / name).is_file():
            errors.append('Required project file missing: ' + name)
    for name in contract['source_directories']:
        if not (root / name).is_dir():
            errors.append('Required source directory missing: ' + name)
    # Infrastructure-style module ownership: one module folder, then files.
    for base in [root / 'src', root / 'include/preloader_l']:
        for path in base.rglob('*'):
            if path.is_file() and len(path.relative_to(base).parts) > 2:
                errors.append('Nested first-party module directory: ' + path.relative_to(root).as_posix())
    sources = (root / 'cmake/Sources.cmake').read_text() + (root / 'CMakeLists.txt').read_text()
    declared = set(re.findall(r'\bsrc/[A-Za-z0-9_/.-]+\.cpp\b', sources))
    actual = {p.relative_to(root).as_posix() for p in (root / 'src').rglob('*.cpp')}
    if declared != actual:
        errors.append('Application CMake source ownership differs: missing=' + str(sorted(actual-declared)) + ', stale=' + str(sorted(declared-actual)))
    if re.search(r'file\s*\(\s*GLOB', sources, re.I):
        errors.append('Application target must not glob reference sources')
    identity = json.loads((root / 'metadata/BRANDING_AND_IDENTITY.json').read_text())
    asset = root / identity['branding_asset']
    if hashlib.sha256(asset.read_bytes()).hexdigest() != identity['asset_sha256']:
        errors.append('SURGE avatar bytes differ from owner asset')
    resource_contract = root / 'metadata/RESOURCE_METADATA_CONTRACT.json'
    if resource_contract.is_file():
        profile = json.loads(resource_contract.read_text())
        resource_source = root / profile['source']['member']
        if hashlib.sha256(resource_source.read_bytes()).hexdigest() != profile['source']['member_sha256']:
            errors.append('Infrastructure VERSIONINFO source differs from explicitly adopted bytes')
        if '#include "preloader_version.rc"' not in (root / 'resources/surge.rc').read_text():
            errors.append('Adopted VERSIONINFO is not included in the compiled resource')
        mustavex = json.loads((root / 'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())
        if mustavex.get('required', {}).get('versioninfo') != profile['required']['versioninfo']:
            errors.append('Golden resource target differs from the adopted VERSIONINFO profile')
    for document in (root / 'docs').glob('*'):
        if document.suffix not in {'.md', '.txt'}:
            continue
        for link in re.findall(r'\]\(([^)#]+)(?:#[^)]*)?\)', document.read_text()):
            if ':' not in link and not (document.parent / link).is_file():
                errors.append('Broken document link: ' + document.name + ' -> ' + link)
    if (root / 'metadata/PUBLIC_DLL_ABI.json').is_file():
        from check_dll_exports import check_source
        errors.extend(check_source(root))
    return errors


if __name__ == '__main__':
    try:
        errors = check(Path(__file__).resolve().parents[1])
    except (OSError, ValueError, KeyError) as error:
        errors = [str(error)]
    for error in errors:
        print(' -', error)
    print('REPOSITORY_CONTRACT:', 'FAIL' if errors else 'PASS')
    raise SystemExit(1 if errors else 0)
