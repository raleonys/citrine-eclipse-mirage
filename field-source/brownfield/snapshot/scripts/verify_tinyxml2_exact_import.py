#!/usr/bin/env python3
"""Fail-closed pinned TinyXML2 dependency verification and optional exact-Git import.

The default action is read-only. --import-from-git requires a local upstream Git
repository containing the exact release commit, never a moving branch or master.
This is an internal automation tool, not a human operator launcher.
"""
from __future__ import annotations
import argparse
import zipfile
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PIN = '8224e427b655b83dae5e2298f1e6919523a78737'
OLD_TAG_PIN = '9148bdf719e997d1f474be6bcc7943881046dba1'
VERSION = '11.0.0'
TARGETS = ('tinyxml2.cpp', 'tinyxml2.h', 'LICENSE.txt')
UPSTREAM_LICENSE_SHA256 = '9332252e9b9e46db8285d4a3f0bf25f139bf1dca6781b956d57f2302efca6432'
TAG_ARCHIVE_ROOT = 'tinyxml2-master/'
PINNED_FILES = {'tinyxml2.cpp': '07b4e7a1d021256255b91a0844640f26508201b40cfc576b9499b068d981c39b', 'tinyxml2.h': 'dc6903d27f5f8ff18f7de98179d979ee3986d12273c495b54f48ac385fcdd7c5', 'LICENSE.txt': '9332252e9b9e46db8285d4a3f0bf25f139bf1dca6781b956d57f2302efca6432'}
VEND = ROOT / 'third_party' / 'tinyxml2'
META = ROOT / 'metadata' / 'TINYXML2_REFERENCE_PROVENANCE.json'


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def git(repo: Path, *args: str) -> bytes:
    p = subprocess.run(['git', '-C', str(repo), *args], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.returncode:
        raise ValueError(f'upstream Git operation failed: {" ".join(args)} ({p.stderr.decode(errors="replace")[:200]})')
    return p.stdout


def validate() -> None:
    d = json.loads(META.read_text(encoding='utf-8'))
    assert d['dependency'] == 'TinyXML2' and d['scope'] == 'production_parser_migration_candidate' and d['shipping_linked'] is True
    assert d['qualified_upstream_fix']['tag_commit'] == OLD_TAG_PIN
    assert d['vendored_base']['version'] == VERSION, ('TinyXML2 source major version mismatch '+str(d['vendored_base'].get('version')))
    assert d['decision_review']['upgrade_status'] == 'EXACT_UPSTREAM_MASTER_COMMIT_FILES_IMPORTED_VERIFIED', 'exact source import pending'
    assert d['vendored_base']['upstream_commit'] == PIN and d['vendored_base']['reference'] == 'master_snapshot_untagged' and d['vendored_base']['local_patches'] == []
    assert d['local_disposition'] == 'EXACT_UPSTREAM_MASTER_PRODUCTION_TARGET_LINKED_CANDIDATE_UNPUBLISHED'
    for name in TARGETS:
        key=f'third_party/tinyxml2/{name}'
        expected=d['files'].get(key)
        assert isinstance(expected,str) and re.fullmatch('[0-9a-f]{64}',expected), key
        actual=sha((VEND/name).read_bytes())
        assert expected == PINNED_FILES.get(name) and actual == expected, f'{key}: pinned upstream bytes mismatch: {actual}'
    header = (VEND/'tinyxml2.h').read_text(encoding='utf-8')
    for k in ('#define TINYXML2_MAJOR_VERSION 11','#define TINYXML2_MINOR_VERSION 0','#define TINYXML2_PATCH_VERSION 0'):
        assert k in header, k
    assert '#include "tinyxml2.h"' in (VEND/'tinyxml2.cpp').read_text(encoding='utf-8')
    assert d['files']['third_party/tinyxml2/LICENSE.txt'] == UPSTREAM_LICENSE_SHA256
    assert d['decision_review']['source_acquisition'] in ('EXACT_GIT_COMMIT_OBJECT', 'GITHUB_MASTER_ARCHIVE_COMMIT_COMMENT_AND_FILE_HASHES')
    if d['decision_review']['source_acquisition'] == 'GITHUB_MASTER_ARCHIVE_COMMIT_COMMENT_AND_FILE_HASHES':
        assert d['decision_review']['source_archive_sha256'] == '8b7508a943e0a06d4c44d40fcc8c6e71f51dd43b342c4c29f768c53a370399cd'
        assert d['decision_review']['archive_comment_commit'] == PIN
        assert d['decision_review']['upstream_file_independent_check'] == 'USER_MASTER_SNAPSHOT_SHA_PIN_AND_PUBLIC_COMMIT_LOOKUP'
    assert d['decision_review']['source_file_hashes_locked'] is True
    assert '10.0.0' not in d['vendored_base']['version']
    print('TINYXML2_EXACT_PIN: PASS (untagged master snapshot, source macro 11.0.0; commit '+PIN+'; 3 pinned source files; provenance='+d['decision_review']['source_acquisition']+'; PRODUCTION_CANDIDATE_ONLY)')


def git_blob_hash(data: bytes) -> str:
    return hashlib.sha1(b'blob '+str(len(data)).encode('ascii')+b'\0'+data).hexdigest()


def install_files(extracted: dict[str,bytes], acquisition: str, extra: dict[str,str]) -> None:
    assert set(extracted) == set(TARGETS)
    for name,data in extracted.items():
        assert len(data)>200, f'upstream {name} unexpectedly short'
    header=extracted['tinyxml2.h'].decode('utf-8')
    assert '#define TINYXML2_MAJOR_VERSION 11' in header
    assert '#define TINYXML2_MINOR_VERSION 0' in header
    assert '#define TINYXML2_PATCH_VERSION 0' in header
    assert b'Lee Thomason' in extracted['tinyxml2.cpp']
    assert b'Lee Thomason' in extracted['tinyxml2.h']
    assert sha(extracted['LICENSE.txt'])==UPSTREAM_LICENSE_SHA256, 'independent upstream license hash mismatch'
    assert all(sha(extracted[n])==PINNED_FILES[n] for n in TARGETS), 'unpinned upstream source bytes'
    d=json.loads(META.read_text(encoding='utf-8'))
    assert d['dependency'] == 'TinyXML2' and d['scope'] == 'production_parser_migration_candidate' and d['shipping_linked'] is True
    assert d['qualified_upstream_fix']['tag_commit'] == OLD_TAG_PIN
    assert d['decision_review']['upgrade_status']=='PENDING_EXACT_UPSTREAM_MASTER_FILE_IMPORT', 'refusing double import'
    # Verify all source bytes, identity, version and disposition before a single write.
    for name,data in extracted.items():
        (VEND/name).write_bytes(data)
        d['files'][f'third_party/tinyxml2/{name}']=sha(data)
    d['vendored_base']={'version':VERSION, 'upstream_commit':PIN, 'reference':'master_snapshot_untagged', 'upstream_project':'leethomason/tinyxml2', 'local_patches':[]}
    d['local_disposition']='EXACT_UPSTREAM_MASTER_PRODUCTION_TARGET_LINKED_CANDIDATE_UNPUBLISHED'
    d['rationale']='Pinned upstream master commit source files; production parser experimental only; full release acceptance pending.'
    d['decision_review']['upgrade_status']='EXACT_UPSTREAM_MASTER_COMMIT_FILES_IMPORTED_VERIFIED'
    d['decision_review']['source_acquisition']=acquisition
    d['decision_review']['source_file_hashes_locked']=True
    d['decision_review']['source_git_blob_sha1']={name:git_blob_hash(data) for name,data in extracted.items()}
    d['decision_review'].update(extra)
    readme=VEND/'REFERENCE_PROVENANCE.md'
    readme.write_text('# TinyXML2 test/reference provenance\n\n'
      f'Pinned upstream version: **{VERSION}**, tag commit `{PIN}` (`leethomason/tinyxml2`).\n'
      'Three upstream source files imported: `tinyxml2.cpp`, `tinyxml2.h`, `LICENSE.txt`.\n'
      f'Acquisition class: `{acquisition}`; per-file SHA-256 and Git blob SHA-1 retained in metadata.\n'
      'ZIP archive comment commits are provenance assertions and NOT equivalent to independently verifying the Git commit object.\n'
      'LICENSE.txt SHA-256 independently corroborated by Buildroot v11.0.0 packaging.\n'
      'Source bytes are not locally modified and are hash-bound in `metadata/TINYXML2_REFERENCE_PROVENANCE.json`.\n'
      'Scope: experimental shipping parser; no public ABI change.\n'
      'Predecessor: 10.0.0 with bounded character-reference security backport, retained as historical provenance only.\n',encoding='utf-8')
    d['files']['third_party/tinyxml2/REFERENCE_PROVENANCE.md']=sha(readme.read_bytes())
    META.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    validate()
    manifest=ROOT/'metadata/ACTIVE_SOURCE_MANIFEST.json'
    m=json.loads(manifest.read_text(encoding='utf-8'))
    m['files']={q.relative_to(ROOT).as_posix():sha(q.read_bytes()) for q in sorted(ROOT.rglob('*'))
                if q.is_file() and q!=manifest and not q.relative_to(ROOT).as_posix().startswith(('build/','dist/'))}
    manifest.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print('TINYXML2_EXACT_IMPORT: SUCCESS, manifest refreshed; run full 8-shard QA and release acceptance')


def import_exact_git(repo: Path) -> None:
    repo = repo.resolve(strict=True)
    assert (repo/'.git').exists() or (repo/'HEAD').exists(), 'not a Git checkout or bare repo'
    got=git(repo,'rev-parse','--verify',PIN+'^{commit}').decode().strip()
    assert got == PIN, f'upstream commit identity mismatch: {got}'
    assert git(repo,'cat-file','-t',PIN).decode().strip() == 'commit'
    extracted = {name:git(repo,'show',f'{PIN}:{name}') for name in TARGETS}
    install_files(extracted,'EXACT_GIT_COMMIT_OBJECT',{'source_git_commit_object_checked': True})


def import_tag_archive(path: Path) -> None:
    path=path.resolve(strict=True)
    archive_data=path.read_bytes()
    archive_sha=sha(archive_data)
    with zipfile.ZipFile(path) as z:
        assert z.testzip() is None, 'ZIP CRC verification failed'
        assert z.comment.strip().decode('ascii') == PIN, 'archive Git commit comment mismatch'
        names=z.namelist()
        assert len(names)==len(set(names)), 'duplicate archive paths'
        assert all(n.startswith(TAG_ARCHIVE_ROOT) and '..' not in Path(n).parts and not n.startswith('/') for n in names), 'unsafe archive path'
        assert all((i.flag_bits & 1)==0 for i in z.infolist()), 'encrypted archive disallowed'
        extracted = {name:z.read(TAG_ARCHIVE_ROOT+name) for name in TARGETS}
    assert archive_sha=='8b7508a943e0a06d4c44d40fcc8c6e71f51dd43b342c4c29f768c53a370399cd', 'unexpected archive SHA256'
    install_files(extracted,'GITHUB_MASTER_ARCHIVE_COMMIT_COMMENT_AND_FILE_HASHES',{
      'source_archive_sha256': archive_sha,
      'source_archive_member_count': len(names),
      'archive_comment_commit': PIN,
      'source_git_commit_object_checked': False,
      'upstream_file_independent_check': 'USER_MASTER_SNAPSHOT_SHA_PIN_AND_PUBLIC_COMMIT_LOOKUP',
      'external_references': [
        'https://github.com/leethomason/tinyxml2/commit/'+PIN,
        'https://github.com/leethomason/tinyxml2/issues/1082'
      ]})

def main() -> int:
    p=argparse.ArgumentParser()
    g=p.add_mutually_exclusive_group()
    g.add_argument('--import-from-git', type=Path, help='local Git repository holding pinned exact upstream commit')
    g.add_argument('--import-from-master-archive', type=Path, help='GitHub master snapshot archive with exact pinned commit comment; distinct from Git object verification')
    args=p.parse_args()
    try:
        if args.import_from_git is not None: import_exact_git(args.import_from_git)
        elif args.import_from_master_archive is not None: import_tag_archive(args.import_from_master_archive)
        else: validate()
    except (AssertionError, KeyError, OSError, ValueError, json.JSONDecodeError) as e:
        print(f'TINYXML2_EXACT_PIN: BLOCKED: {e}',file=sys.stderr)
        return 2
    return 0

if __name__=='__main__': raise SystemExit(main())
