#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline, pinned, fail-closed materialization of the active owner-provided vendor ZIPs.

Preserves every member payload, with portable exact descriptors for locked aliases.
Never downloads, patches, or overwrites vendor files.
"""
import argparse
import hashlib
import json
import os
import pathlib
import shutil
import stat
import sys
import re
import tempfile
import zipfile

VENDORS = ('asio','minhook','pegtl','pugixml')
MAX_ARCHIVE_BYTES = 100 * 1024 * 1024
MAX_MEMBER_BYTES = 32 * 1024 * 1024


def hash_file(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for part in iter(lambda:f.read(1024*1024),b''):
            h.update(part)
    return h.hexdigest()


def safe_rel(path):
    if not isinstance(path, str) or not path or '\\' in path or ':' in path or '\x00' in path:
        raise ValueError('unsafe member name')
    p=pathlib.PurePosixPath(path)
    if p.is_absolute() or any(x in ('..','.','') for x in p.parts) or p.as_posix()!=path.strip('/'):
        raise ValueError('unsafe member path')
    return p


def validate_symlink(rel, target):
    if not isinstance(target,str) or not target or '\\' in target or ':' in target or any(ord(c)<32 for c in target):
        raise ValueError('unsafe symlink')
    dest=pathlib.PurePosixPath(rel).parent
    stack=list(dest.parts)
    for part in pathlib.PurePosixPath(target).parts:
        if part=='.':continue
        if part=='..':
            if not stack:raise ValueError('symlink escapes vendor root')
            stack.pop()
        elif part!='':stack.append(part)
        else:raise ValueError('bad symlink')
    if pathlib.PurePosixPath(target).is_absolute() or not stack:
        raise ValueError('symlink target invalid')
    return True


def load_lock(root):
    lock=json.loads((root/'metadata/VENDOR_LOCK.json').read_text('utf8'))
    if lock.get('schema')!='surge.vendor-lock.v2' or set(lock.get('vendors',{}))!=set(VENDORS):
        raise ValueError('vendor lock schema or mandatory inventory mismatch')
    for record in lock['vendors'].values():
        if not re.fullmatch('[0-9a-f]{40}',record.get('upstream_commit','')):
            raise ValueError('vendor archive commit identifier missing or invalid')
    return lock


def expected_files(root, record):
    f=root/record['member_manifest']
    if hash_file(f)!=record['member_manifest_sha256']:raise ValueError('member manifest hash mismatch')
    records={}
    for line in f.read_text('utf8').splitlines():
        h,sep,rel=line.partition('  ')
        if sep!='  ' or len(h)!=64 or any(c not in '0123456789abcdef' for c in h):raise ValueError('invalid member manifest line')
        safe_rel(rel)
        if rel in records:raise ValueError('duplicate manifest member')
        records[rel]=h
    if len(records)!=record['member_count']:raise ValueError('member count mismatch')
    return records


def archive_file(root, record, archive_dir=None):
    return (pathlib.Path(archive_dir) if archive_dir is not None else root/'vendor_sources')/pathlib.Path(record['archive']).name


def inspect_archive(root, name, record, archive_dir=None):
    archive=archive_file(root,record,archive_dir)
    if not archive.is_file() or hash_file(archive)!=record['sha256'] or archive.stat().st_size!=record['bytes']:
        raise ValueError(name+': archive SHA/size mismatch')
    expected=expected_files(root,record)
    raw_symlinks={}
    actual={}
    seenfold=set();total=0
    with zipfile.ZipFile(archive) as z:
        if z.comment.decode('ascii')!=record['upstream_commit']:
            raise ValueError(name+': archive commit comment mismatch')
        if z.testzip():raise ValueError(name+': archive CRC error')
        for info in z.infolist():
            if info.filename=='/' or not info.filename.startswith(record['archive_folder']):
                raise ValueError(name+': incorrect archive root')
            rel=info.filename[len(record['archive_folder']):]
            if not rel:continue
            rel=rel.rstrip('/')
            safe_rel(rel)
            key=rel.casefold()
            if key in seenfold:raise ValueError(name+': Windows-case duplicate member')
            seenfold.add(key)
            if info.is_dir():continue
            if info.file_size>MAX_MEMBER_BYTES:raise ValueError(name+': too large member')
            total+=info.file_size
            if total>MAX_ARCHIVE_BYTES:raise ValueError(name+': excessive expanded size')
            mode=info.external_attr >> 16
            if not(stat.S_ISREG(mode) or stat.S_ISLNK(mode) or mode==0):raise ValueError(name+': unsupported file type')
            payload=z.read(info)
            actual[rel]=hashlib.sha256(payload).hexdigest()
            if stat.S_ISLNK(mode):
                target=payload.decode('utf8')
                validate_symlink(rel,target)
                raw_symlinks[rel]=target
        if actual!=expected:raise ValueError(name+': per-member mismatch')
        if raw_symlinks!=record.get('symlinks',{}):raise ValueError(name+': symlink metadata mismatch')
    return expected,raw_symlinks


def validate_installed(root,name,record,expected,links):
    vendor_dir=(root/record['source_install_path'])
    if vendor_dir.is_symlink() or not vendor_dir.is_dir():raise ValueError(name+': not installed')
    actual={};invalid_types=[]
    for p in vendor_dir.rglob('*'):
        rel=p.relative_to(vendor_dir).as_posix()
        if p.is_symlink():
            if rel not in links or os.readlink(p)!=links[rel]:
                invalid_types.append(rel+' (unregistered or altered symlink)')
            actual[rel]=hashlib.sha256(os.readlink(p).encode('utf8')).hexdigest()
        elif p.is_file():actual[rel]=hash_file(p)
        elif not p.is_dir():raise ValueError(name+': unexpected file type '+rel)
        if rel in links and not p.is_symlink():
            if not p.is_file() or p.read_bytes()!=links[rel].encode('utf8'):
                invalid_types.append(rel+' (expected exact locked alias target descriptor)')
    if actual!=expected or invalid_types:
        missing=sorted(set(expected)-set(actual));extra=sorted(set(actual)-set(expected))
        changed=sorted(rel for rel in set(actual)&set(expected) if actual[rel]!=expected[rel])
        details=[name+': installed member set/hash mismatch '+str(len(actual))+'/'+str(len(expected)),
                 'missing='+str(len(missing))+' unexpected='+str(len(extra))+' changed='+str(len(changed))]
        for label,paths in [('missing',missing),('unexpected',extra)]:
            details.extend('  '+label+': '+rel for rel in paths[:12])
        for rel in changed[:12]:
            details.append('  changed: '+rel+' expected_sha256='+expected[rel]+' actual_sha256='+actual[rel])
        details.extend('  type: '+rel for rel in invalid_types[:12])
        if any(rel in links for rel in missing+changed) or invalid_types:
            details.append('  Locked Asio aliases may be symlinks or byte-exact target descriptors; other content is rejected. Use the portable source ZIP; do not refresh vendor hashes.')
        raise ValueError('\n'.join(details))
    return True


def install(root,name,record,expected,links,archive_dir=None):
    path=root/record['source_install_path']
    if path.exists() or path.is_symlink():raise ValueError(name+': installation destination exists; refusing overwrite')
    parent=path.parent
    parent.mkdir(parents=True,exist_ok=True)
    if parent.is_symlink():raise ValueError(name+': destination parent symlink')
    tmp=pathlib.Path(tempfile.mkdtemp(prefix='.vendor-tmp-',dir=parent))
    try:
        with zipfile.ZipFile(archive_file(root,record,archive_dir)) as z:
            for info in z.infolist():
                rel=info.filename[len(record['archive_folder']):].rstrip('/')
                if not rel:continue
                safe_rel(rel)
                out=tmp/pathlib.Path(rel)
                if info.is_dir():out.mkdir(parents=True,exist_ok=True);continue
                out.parent.mkdir(parents=True,exist_ok=True)
                if rel in links:
                    out.write_bytes(links[rel].encode('utf8'))
                else:
                    out.write_bytes(z.read(info))
        # Can validate while temp path carries the original destination metadata
        actual={}
        for p in tmp.rglob('*'):
            rel=p.relative_to(tmp).as_posix()
            if p.is_symlink():actual[rel]=hashlib.sha256(os.readlink(p).encode('utf8')).hexdigest()
            elif p.is_file():actual[rel]=hash_file(p)
        if actual!=expected:raise ValueError(name+': staged bytes mismatch')
        tmp.rename(path)
        validate_installed(root,name,record,expected,links)
    finally:
        if tmp.exists():shutil.rmtree(tmp)


def run(root,mode,archive_dir=None):
    root=root.resolve();lock=load_lock(root)
    for name in VENDORS:
        record=lock['vendors'][name]
        if mode=='installed':
            files=expected_files(root,record);links=record.get('symlinks',{})
        else:
            files,links=inspect_archive(root,name,record,archive_dir)
        if mode=='install':install(root,name,record,files,links,archive_dir)
        if mode in ('installed','install'):validate_installed(root,name,record,files,links)
        print(name+': '+('INSTALLED_MEMBER_HASHES_PASS' if mode=='installed' else 'ARCHIVE_EXACT_PASS')+(' INSTALLED_PASS' if mode=='install' else ''))
    print('VENDOR_SYNC='+('PASS_ARCHIVES_ONLY_NOT_ACTIVE_PRODUCTION' if mode=='archives' else 'PASS_BYTES_NOT_ACTIVE_PRODUCTION'))


if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--root',type=pathlib.Path,default=pathlib.Path('.'))
    p.add_argument('--archive-dir',type=pathlib.Path,help='Directory containing the active original owner ZIPs; never downloaded')
    group=p.add_mutually_exclusive_group(required=True)
    group.add_argument('--verify-archives',dest='mode',action='store_const',const='archives')
    group.add_argument('--verify-installed',dest='mode',action='store_const',const='installed')
    group.add_argument('--install',dest='mode',action='store_const',const='install')
    a=p.parse_args()
    try:run(a.root,a.mode,a.archive_dir)
    except (OSError,ValueError,KeyError,zipfile.BadZipFile,UnicodeError) as exc:
        print('VENDOR_SYNC=BLOCKED:',str(exc),file=sys.stderr);sys.exit(1)
