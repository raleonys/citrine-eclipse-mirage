# SPDX-License-Identifier: GPL-3.0-only

"""Check source scope, native CMake source ownership and pinned SURGE resource."""
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess

from project_files import safe_member


def check_public_timezones(root):
    """Reject regional zone names in current first-party authoring inputs.

    This is a narrow source guard, not a general location/PII detector or a
    claim about retained history, third-party material or hosted GitHub text.
    """
    from project_files import load_unique_json, safe_member
    root = Path(root).resolve()
    manifest = load_unique_json(root / 'metadata/ACTIVE_SOURCE_MANIFEST.json')
    regional = re.compile(
        r'\b(?:Africa|America|Antarctica|Arctic|Asia|Atlantic|Australia|Europe|Indian|Pacific)'
        r'/[A-Za-z_+-]+(?:/[A-Za-z_+-]+)?\b', re.I)
    extensions = {'.md', '.txt', '.json', '.yml', '.yaml', '.py', '.ps1', '.cmd',
                  '.cmake', '.c', '.cpp', '.h', '.hpp', '.rc'}
    errors = []
    for name in sorted(manifest['files']):
        safe_member(name)
        if name.startswith(('third_party/', 'metadata/evidence/', 'docs/evidence/',
                            'metadata/vendor_files/', 'tests/')):
            continue
        path = root / name
        if path.suffix.lower() not in extensions and path.name != 'CMakeLists.txt':
            continue
        if path.is_symlink() or not path.resolve().is_relative_to(root) or not path.is_file():
            raise ValueError('Nonregular public time-metadata input: ' + name)
        for number, line in enumerate(path.read_text(encoding='utf-8-sig').splitlines(), 1):
            if regional.search(line):
                # Do not repeat the location-bearing token into public CI logs.
                errors.append(f'Regional time-zone name in active source: {name}:{number}; use UTC')
    return errors


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
        if (rel.parts[0] in {'build','dist','.git','third_party'} or
                rel.parts[:2] == ('metadata', 'evidence') or not path.is_file()):
            continue
        name=rel.as_posix()
        if pattern.search(name):
            errors.append('Reserved reference label in active path: '+name)
        legal_document=len(rel.parts)==1 and path.name in {'LICENSE','COPYING','NOTICE','AUTHORS'}
        if rel.parts[0] == 'docs' or path.suffix.lower()=='.md' or legal_document:
            continue
        try:
            text=path.read_text(encoding='utf-8-sig')
        except UnicodeError:
            continue
        if pattern.search(text):
            errors.append('Reserved reference label outside documents: '+name)
    return errors



def is_git_worktree_file(path):
    """Trust only a linked worktree registered in Git's own common directory."""
    if path.name != '.git' or path.is_symlink() or not path.is_file():
        return False
    try:
        if path.stat().st_size > 4096:
            return False
        match = re.fullmatch(r'gitdir: ([^\r\n]+)\r?\n?', path.read_text(encoding='utf-8'))
        if not match or '\x00' in match.group(1):
            return False
        root = path.parent.resolve()
        gitdir = (path.parent / match.group(1)).resolve(strict=True)
        if not gitdir.is_dir() or not (gitdir / 'HEAD').is_file():
            return False
        # Worktree-local metadata must point back to this precise gitfile.
        reverse = gitdir / 'gitdir'
        common_ref = gitdir / 'commondir'
        if not reverse.is_file() or not common_ref.is_file():
            return False
        backlink = reverse.read_text(encoding='utf-8').strip()
        if not backlink or (gitdir / backlink).resolve(strict=True) != path.resolve(strict=True):
            return False
        common = common_ref.read_text(encoding='utf-8').strip()
        if not common or not (gitdir / common).resolve(strict=True).is_dir():
            return False
        # A fabricated gitdir/HEAD and backlink alone are not a Git registration.
        # Compare Git's actual selected root, gitdir and linked worktree registry.
        env = {k: v for k, v in os.environ.items()
               if not k.startswith('GIT_')}
        def git(*args):
            done = subprocess.run(['git', '-C', str(root), *args],
                                  capture_output=True, text=True, timeout=8, env=env)
            return done.stdout.strip() if done.returncode == 0 else None
        if git('rev-parse', '--is-inside-work-tree') != 'true':
            return False
        top = git('rev-parse', '--show-toplevel')
        selected_dir = git('rev-parse', '--absolute-git-dir')
        if not top or not selected_dir:
            return False
        if Path(top).resolve(strict=True) != root or Path(selected_dir).resolve(strict=True) != gitdir:
            return False
        worktrees = git('worktree', 'list', '--porcelain')
        if worktrees is None:
            return False
        return any(line.startswith('worktree ')
                   and Path(line[len('worktree '):]).resolve(strict=True) == root
                   for line in worktrees.splitlines())
    except (OSError, UnicodeError, ValueError, subprocess.SubprocessError):
        return False


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
            if (path.is_file() or path.is_symlink()) and path.name not in allowed
            and not is_git_worktree_file(path)]

def check_archived_evidence(root):
    """Fail closed on new evidence dumps and modifications to locked historical files."""
    root = Path(root)
    errors = []
    if (root / 'docs/evidence').exists():
        errors.append('Forbidden legacy evidence directory: docs/evidence')
    metadata = root / 'metadata'
    archive = metadata / 'evidence'
    policy_path = metadata / 'EVIDENCE_RETENTION_POLICY.json'

    def kind(path):
        try:
            info = path.lstat()
        except FileNotFoundError:
            return 'missing'
        except OSError:
            return 'nonregular'
        # Windows reparse attributes are available on Python 3.9; unlike
        # Path.is_junction this does not depend on a newer pathlib API.
        if (stat.S_ISLNK(info.st_mode) or
                getattr(info, 'st_file_attributes', 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT):
            return 'link'
        if stat.S_ISDIR(info.st_mode):
            return 'directory'
        return 'file' if stat.S_ISREG(info.st_mode) else 'nonregular'

    # Check the parent and trust-anchor file before reading policy contents.
    for path, label, allowed in ((metadata, 'metadata directory', {'directory'}),
                                 (archive, 'archive', {'directory', 'missing'}),
                                 (policy_path, 'retention policy', {'file'})):
        value = kind(path)
        if value == 'link':
            return errors + ['Historical evidence ' + label + ' is a link']
        if value not in allowed:
            return errors + ['Invalid historical evidence ' + label]
    policy = json.loads(policy_path.read_text(encoding='utf-8'))
    retained = policy.get('retained_files')
    if (policy.get('schema') != 'surge.evidence-retention.v1' or
            not isinstance(retained, dict) or not retained or
            policy.get('retained_count') != len(retained)):
        return errors + ['Invalid historical evidence retention policy']
    for name, digest in retained.items():
        if (not isinstance(name, str) or not name or
                not isinstance(digest, str) or not re.fullmatch(r'[a-f0-9]{64}', digest)):
            return errors + ['Unsafe historical evidence entry']
        try:
            safe_member(name)
        except ValueError:
            return errors + ['Unsafe historical evidence entry']

    # Enumerate without following redirected directories. Classify the whole
    # tree before reading any evidence payload, including nested pinned files.
    actual = set()
    pending = [archive] if kind(archive) == 'directory' else []
    unsafe = False
    while pending:
        directory = pending.pop()
        for path in sorted(directory.iterdir()):
            name = path.relative_to(archive).as_posix()
            value = kind(path)
            if value == 'directory':
                pending.append(path)
            elif value == 'file':
                actual.add(name)
            else:
                unsafe = True
                if value == 'link':
                    actual.add(name)
                    errors.append('Historical evidence link is forbidden: ' + name)
                else:
                    errors.append('Historical evidence nonregular entry: ' + name)
    expected = set(retained) | {'README.md'}
    if actual != expected:
        errors.append('Historical evidence file-set mismatch: missing='
                      + str(sorted(expected - actual)) + ', unrecorded='
                      + str(sorted(actual - expected)))
    if unsafe:
        return errors
    for name, digest in retained.items():
        path = archive / name
        if name not in actual:
            continue
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            errors.append('Historical evidence bytes differ: ' + name)
    return errors


def check_layout_migration_reference(root, contract):
    """Reject stale, malformed or escaping layout_migration contract targets."""
    root = Path(root)
    name = contract.get('layout_migration')
    if (not isinstance(name, str) or not name or
            '\\' in name or not name.endswith('.json') or
            any(part in ('', '.', '..') for part in name.split('/'))):
        return ['Unsafe layout_migration repository contract reference']
    target = root / name
    try:
        resolved = target.resolve(strict=True)
        if (not resolved.is_relative_to(root.resolve(strict=True)) or
                target.is_symlink() or not target.is_file()):
            return ['Invalid layout_migration file target: ' + name]
        payload = json.loads(target.read_text(encoding='utf-8'))
    except (OSError, UnicodeError, ValueError):
        return ['Invalid or missing layout_migration file target: ' + name]
    schema = payload.get('schema') if isinstance(payload, dict) else None
    # The historical schema name is pinned by SHA-256 to avoid reproducing
    # a reserved reference identifier in active source or metadata.
    if (not isinstance(schema, str) or not schema.isascii() or
            hashlib.sha256(schema.encode('ascii')).hexdigest() !=
            '4f01afb8fc1e048dd02b9331c729506bc911ce880df79bb51b5008ba87d0d829'):
        return ['Unexpected layout_migration schema: ' + name]
    return []


def declared_application_sources(text):
    # A vendor path such as third_party/pugixml/src/pugixml.cpp is not
    # an application-relative src/ path. Keep complete path boundaries.
    return set(re.findall(r'(?<![A-Za-z0-9_./\\-])src/[A-Za-z0-9_/.-]+\.cpp\b', text))


def check(root):
    root = Path(root)
    errors = check_active_identifiers(root)
    errors.extend(check_public_timezones(root))
    errors.extend(check_archived_evidence(root))
    contract = json.loads((root / 'metadata/REPOSITORY_CONTRACT.json').read_text())
    errors.extend(check_layout_migration_reference(root, contract))
    errors.extend(check_root_file_ownership(root, contract['allowed_root_files']))
    for name in contract['required_files']:
        if not (root / name).is_file():
            errors.append('Required project file missing: ' + name)
    for name in contract['source_directories']:
        if not (root / name).is_dir():
            errors.append('Required source directory missing: ' + name)
    # Infrastructure-style module ownership: one module folder, then files.
    for base in [root / 'src', root / 'include/citrine_eclipse_mirage']:
        for path in base.rglob('*'):
            if path.is_file() and len(path.relative_to(base).parts) > 2:
                errors.append('Nested first-party module directory: ' + path.relative_to(root).as_posix())
    sources = (root / 'cmake/Sources.cmake').read_text() + (root / 'CMakeLists.txt').read_text()
    declared = declared_application_sources(sources)
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
        if '#include "version.rc"' not in (root / 'resources/surge.rc').read_text():
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
