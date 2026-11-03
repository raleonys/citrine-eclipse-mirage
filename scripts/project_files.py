# SPDX-License-Identifier: GPL-3.0-only

"""Canonical source inventory shared by source QA and flat-package verification."""
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re

MANIFEST = 'metadata/ACTIVE_SOURCE_MANIFEST.json'
MANIFEST_SCHEMA = 'surge.active-source.v1'
ALIAS_STORAGE = 'locked-relative-target-descriptors-v1'
TRANSIENT = {'build', 'dist', '.git'}
ARCHIVES = ('.zip', '.7z', '.rar', '.tar', '.tgz', '.tar.gz', '.gz')
BINARIES = {'.dll', '.exe', '.obj', '.o', '.a', '.lib', '.pyc', '.pyo', '.pdb'}

# A source ZIP verified on another host must retain its exact path identity on Windows.
_WINDOWS_FORBIDDEN = frozenset('<>:"|?*')
_WINDOWS_DEVICES = frozenset(('CON', 'PRN', 'AUX', 'NUL', 'CONIN$', 'CONOUT$'))
_WINDOWS_PORT = re.compile(r'(?:COM|LPT)[1-9\u00b9\u00b2\u00b3]', re.IGNORECASE)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def safe_member(name):
    parts = name.split('/')
    if (not name or '\\' in name or name.startswith('/') or re.match(r'^[A-Za-z]:', name)
            or any(p in ('', '.', '..') for p in parts)
            or any(ord(c) < 32 or ord(c) == 127 for c in name)):
        raise ValueError('Unsafe package path: ' + repr(name))
    for part in parts:
        stem = part.split('.', 1)[0].rstrip(' ').upper()
        if (part.endswith((' ', '.')) or any(c in _WINDOWS_FORBIDDEN for c in part)
                or stem in _WINDOWS_DEVICES or _WINDOWS_PORT.fullmatch(stem)):
            raise ValueError('Unsafe Windows package path: ' + repr(name))
    if str(PurePosixPath(name)) != name:
        raise ValueError('Noncanonical package path: ' + repr(name))
    return name


def vendor_symlinks(root):
    """Only archive-locked, vendor-local aliases are eligible source links."""
    path=Path(root)/'metadata/VENDOR_LOCK.json'
    if not path.is_file():
        return {}
    lock=json.loads(path.read_text(encoding='utf-8'))
    links={}
    for name,record in lock['vendors'].items():
        base=record['source_install_path']
        if base!='third_party/'+name:
            raise ValueError('Vendor root is not owned by its declared name')
        for rel,target in record.get('symlinks',{}).items():
            safe_member(rel)
            if (not target or target.startswith('/') or
                    any(c in target for c in ['\\', ':']) or
                    any(ord(c) < 32 or ord(c) == 127 for c in target)):
                raise ValueError('Unsafe vendor symlink target')
            stack=list(PurePosixPath(rel).parent.parts)
            for part in target.split('/'):
                if part=='..':
                    if not stack:
                        raise ValueError('Vendor symlink escapes its archive root')
                    stack.pop()
                elif part not in ('','.'):stack.append(part)
            if not stack:
                raise ValueError('Vendor symlink cannot alias the entire root')
            links[base+'/'+rel]=target
    return links


def source_payload(path):
    return os.readlink(path).encode('utf-8') if path.is_symlink() else path.read_bytes()


def alias_payload(path, target):
    """Only the exact locked target, represented as a link or a regular file."""
    if not path.is_symlink() and not path.is_file():
        raise ValueError('Missing or invalid locked vendor alias: ' + str(path))
    data = source_payload(path)
    if data != target.encode('utf-8'):
        raise ValueError('Unregistered or altered source symlink/descriptor: ' + str(path))
    return data


def source_files(root):
    root = Path(root).resolve()
    files, folded = {}, {}
    allowed_links=vendor_symlinks(root)
    seen_links=set()
    for path in sorted(root.rglob('*')):
        rel = path.relative_to(root)
        if rel.parts[0] in TRANSIENT:
            continue
        if rel.as_posix() in allowed_links:
            alias_payload(path, allowed_links[rel.as_posix()])
            seen_links.add(rel.as_posix())
        elif path.is_symlink():
            raise ValueError('Unregistered or altered source symlink: ' + rel.as_posix())
        elif not path.is_file():
            continue
        name = safe_member(rel.as_posix())
        if name == MANIFEST:
            continue
        if '__pycache__' in rel.parts or path.suffix.lower() in BINARIES or name.lower().endswith(ARCHIVES):
            raise ValueError('Generated binary/cache or nested archive in source: ' + name)
        if name.casefold() in folded:
            raise ValueError('Case-insensitive source path collision: ' + name)
        folded[name.casefold()] = name
        files[name] = source_payload(path)
    if seen_links!=set(allowed_links):
        raise ValueError('Missing locked vendor aliases: '+str(sorted(set(allowed_links)-seen_links)))
    return files


def write_manifest(root):
    root = Path(root)
    files = source_files(root)
    manifest = {'schema': MANIFEST_SCHEMA,
                'vendor_alias_storage': ALIAS_STORAGE,
                'symlinks': vendor_symlinks(root),
                'files': {name: digest(data) for name, data in files.items()}}
    (root / MANIFEST).write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    return manifest


def verify_manifest(root):
    root = Path(root)
    manifest = json.loads((root / MANIFEST).read_text(encoding='utf-8'))
    if not isinstance(manifest, dict) or manifest.get('schema') != MANIFEST_SCHEMA:
        return ['Unrecognized active source manifest schema']
    if 'classification' in manifest:
        return ['Release classification does not belong in source identity manifests']
    actual = {name: digest(data) for name, data in source_files(root).items()}
    expected = manifest['files']
    if manifest.get('symlinks',{})!=vendor_symlinks(root):
        return ['Active manifest/vendor symlink type identity mismatch']
    storage = manifest.get('vendor_alias_storage')
    if storage not in (None, ALIAS_STORAGE):
        return ['Unsupported vendor alias storage policy']
    if storage is None and any(not (root / name).is_symlink() for name in vendor_symlinks(root)):
        return ['Legacy source manifest requires real vendor symlinks']
    errors = ['Missing source: ' + n for n in expected.keys() - actual.keys()]
    errors += ['Unrecorded source: ' + n for n in actual.keys() - expected.keys()]
    errors += ['Source hash differs: ' + n for n in expected.keys() & actual.keys() if expected[n] != actual[n]]
    return sorted(errors)
