#!/usr/bin/env python3
from pathlib import Path
import argparse,sys,re,zipfile,json,hashlib
errors=[]
ap=argparse.ArgumentParser(description='FALCION Golden Cleaning: release-source and optional flat-package gate.')
ap.add_argument('--package',type=Path,help='Optional official release ZIP to validate for flat root/package exclusions.')
ap.add_argument('--root',type=Path,help='Optional source root used by deterministic self-tests.')
args=ap.parse_args()
r=(args.root.resolve() if args.root is not None else Path(__file__).resolve().parents[1])
required_root={'README.md','LICENSE','CMakeLists.txt','CHANGELOG.md','SOURCE_REVISION.md','RELEASE_STATE.json'}
allowed_source_dirs={'docs','exports','include','metadata','resources','scripts','src','tests','third_party','tools'}
transient_source_dirs={'build','dist'}
archive_suffixes=('.zip','.7z','.rar','.tar','.tgz','.tar.gz','.gz')
def is_embedded_archive(rel):
    low=rel.as_posix().casefold() if isinstance(rel,Path) else str(rel).casefold()
    return any(low.endswith(sfx) for sfx in archive_suffixes)

# Governance 11.3: fail-closed project-owned sensitive-material scan. Keep
# signatures high-confidence so synthetic source text/policy examples do not
# become permanent allowlisted secrets merely to satisfy the gate.
_sensitive_text_suffixes={'.md','.txt','.json','.csv','.py','.ps1','.cmd','.cmake','.cpp','.c','.hpp','.h','.rc','.def','.xml','.ini','.cfg','.log'}
_sensitive_rules=[
    ('private-key',re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----')),
    ('aws-access-key',re.compile(r'(?<![A-Z0-9])AKIA[0-9A-Z]{16}(?![A-Z0-9])')),
    ('github-token',re.compile(r'(?<![A-Za-z0-9_])gh[pousr]_[A-Za-z0-9_]{24,}(?![A-Za-z0-9_])')),
    ('bearer-token',re.compile(r'(?i)\bBearer\s+[A-Za-z0-9._~+/-]{24,}={0,2}')),
    ('jwt',re.compile(r'\beyJ[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\b')),
    ('windows-user-path',re.compile(r'(?i)\b[A-Z]:\\Users\\(?!USER(?:NAME)?\\|Public\\|Default\\|example\\|test\\|dummy\\)[^\\\r\n\t ]+\\')),
    ('unix-user-path',re.compile(r'(?<![A-Za-z0-9_])/(?:home|Users)/(?!USER(?:NAME)?/|Public/|Default/|example/|test/|dummy/)[^/\r\n\t ]+/')),
]
def sensitive_hits(rel,data):
    if rel.parts and rel.parts[0]=='third_party': return []
    if Path(rel).suffix.lower() not in _sensitive_text_suffixes: return []
    try: text=data.decode('utf-8')
    except UnicodeDecodeError: return []
    hits=[]
    for label,rx in _sensitive_rules:
        if rx.search(text): hits.append(label)
    return hits

def scan_sensitive_source():
    out=[]
    for q in r.rglob('*'):
        if not q.is_file(): continue
        rel=q.relative_to(r)
        if rel.parts and rel.parts[0] in transient_source_dirs: continue
        for label in sensitive_hits(rel,q.read_bytes()):
            out.append('sensitive material '+label+' in '+rel.as_posix())
    return out
root_files={p.name for p in r.iterdir() if p.is_file()}
extra=sorted(root_files-required_root); missing=sorted(required_root-root_files)
if extra: errors.append('unexpected root files: '+', '.join(extra))
if missing: errors.append('missing root files: '+', '.join(missing))
root_dirs={p.name for p in r.iterdir() if p.is_dir()}
unexpected_dirs=sorted(root_dirs-allowed_source_dirs-transient_source_dirs)
missing_dirs=sorted(allowed_source_dirs-root_dirs)
if unexpected_dirs: errors.append('unexpected top-level directories: '+', '.join(unexpected_dirs))
if missing_dirs: errors.append('missing top-level directories: '+', '.join(missing_dirs))
for d in ('evidence','FULL_HISTORY'):
    if (r/d).exists(): errors.append('historical top-level directory leaked: '+d)
for p in r.rglob('*'):
    rel=p.relative_to(r)
    if rel.parts and rel.parts[0] in transient_source_dirs: continue
    if '__pycache__' in rel.parts or (p.is_file() and p.suffix.lower() in ('.pyc','.pyo')):
        errors.append('generated Python bytecode cache leaked: '+rel.as_posix())
    if p.is_file() and is_embedded_archive(rel):
        errors.append('embedded archive payload forbidden: '+rel.as_posix())
errors.extend(scan_sensitive_source())
if (r/'scripts/history').exists(): errors.append('historical script directory leaked')
for stale in ('ACTIVE_ROUTE_POLICY.json','CURRENT_FEATURE_STATE.json'):
    if (r/stale).exists() or (r/'metadata'/stale).exists(): errors.append('stale dual policy leaked: '+stale)
if not (r/'docs/RELEASE_HYGIENE.md').is_file(): errors.append('Repository Structure Contract missing')
if not (r/'docs/FALCION_GOLDEN_CLEANING.md').is_file(): errors.append('FALCION policy document missing')
state=json.loads((r/'RELEASE_STATE.json').read_text())
falcion=state.get('falcion',{})
if falcion.get('policy')!='FALCION Golden Cleaning': errors.append('FALCION state policy drift')
if falcion.get('active_checker')!='scripts/check_falcion_golden_cleaning.py': errors.append('FALCION checker state drift')
if falcion.get('active_document')!='docs/FALCION_GOLDEN_CLEANING.md': errors.append('FALCION document state drift')
if falcion.get('flat_package_root') is not True: errors.append('FALCION flat-package contract drift')
if falcion.get('historical_bundle_release_class')!='NOT_A_CURRENT_RELEASE_CLASS': errors.append('obsolete historical-bundle release-class policy drift')
if falcion.get('provenance_history_in_source') is not False: errors.append('live-doc provenance policy drift')
for d in ('docs/history','docs/historical','docs/research','docs/governance'):
    if (r/d).exists(): errors.append('forbidden archival/research docs directory leaked: '+d)
if not (r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').is_file(): errors.append('direct governance mirror missing')
if (r/'scripts/build').exists(): errors.append('superseded active scripts/build directory leaked')
if not (r/'scripts/build_orchestration').is_dir(): errors.append('build orchestration directory missing')
for asset in ('surge.cfg','surge.png'):
    if (r/asset).exists(): errors.append('excluded release asset leaked: '+asset)

def source_manifest_errors():
    out=[]; mp=r/'metadata/ACTIVE_SOURCE_MANIFEST.json'
    if not mp.is_file(): return ['ACTIVE_SOURCE_MANIFEST missing'],None
    try: manifest=json.loads(mp.read_text())
    except (OSError,json.JSONDecodeError) as e: return ['ACTIVE_SOURCE_MANIFEST unreadable: '+str(e)],None
    files=manifest.get('files')
    if not isinstance(files,dict): return ['ACTIVE_SOURCE_MANIFEST files mapping missing'],None
    expected=set(files)
    actual={p.relative_to(r).as_posix() for p in r.rglob('*') if p.is_file() and p.relative_to(r).as_posix()!='metadata/ACTIVE_SOURCE_MANIFEST.json' and not p.relative_to(r).as_posix().startswith(('dist/','build/'))}
    if actual!=expected: out.append(f'active manifest file-set delta missing={len(expected-actual)} extra={len(actual-expected)}')
    for rel,expected_hash in files.items():
        p=r/rel
        if not p.is_file(): out.append('active manifest missing '+rel); continue
        got=hashlib.sha256(p.read_bytes()).hexdigest()
        if got!=expected_hash: out.append('active manifest hash mismatch '+rel)
    return out,hashlib.sha256(mp.read_bytes()).hexdigest()

manifest_sha=None
source_errors,manifest_sha=source_manifest_errors()
errors.extend(source_errors)

if args.package is not None:
    try:
        with zipfile.ZipFile(args.package,'r') as z:
            infos=z.infolist()
            raw_names=[i.filename for i in infos]
            seen=set(); seen_fold={}; safe_infos=[]
            for info in infos:
                name=info.filename
                if name in seen: errors.append('duplicate package member name: '+repr(name))
                seen.add(name)
                if not name: errors.append('empty package member name'); continue
                if '\\' in name: errors.append('backslash-ambiguous package member name: '+repr(name)); continue
                if name.startswith('/') or re.match(r'^[A-Za-z]:',name): errors.append('absolute/drive-qualified package member name: '+repr(name)); continue
                if any(ord(ch)<32 or ord(ch)==127 for ch in name): errors.append('control character in package member name: '+repr(name)); continue
                logical=name[:-1] if name.endswith('/') else name
                parts=logical.split('/')
                if not logical or any(part in ('','.','..') for part in parts): errors.append('unsafe package member path: '+repr(name)); continue
                folded=logical.casefold()
                previous=seen_fold.get(folded)
                if previous is not None and previous!=logical: errors.append(f'case-fold package collision: {previous!r} vs {logical!r}')
                elif previous is None: seen_fold[folded]=logical
                safe_infos.append((info,logical,parts))
            file_names=[logical for info,logical,_parts in safe_infos if not info.is_dir()]
            file_set=set(file_names)
            for name in required_root:
                if name not in file_set: errors.append('flat package missing root file: '+name)
            top_dirs={parts[0] for info,_logical,parts in safe_infos if len(parts)>1}
            unexpected=sorted(top_dirs-allowed_source_dirs)
            missing_package_dirs=sorted(allowed_source_dirs-top_dirs)
            if unexpected: errors.append('flat package unexpected top-level directory: '+', '.join(unexpected))
            if missing_package_dirs: errors.append('flat package missing root directory: '+', '.join(missing_package_dirs))
            for info,logical,parts in safe_infos:
                low=logical.lower()
                if parts[-1].lower() in ('surge.cfg','surge.png'): errors.append('excluded release asset packaged: '+logical)
                if parts[0].lower() in ('full_history','evidence','build','dist'): errors.append('forbidden top-level material packaged: '+logical)
                if any(x.lower()=='__pycache__' for x in parts) or low.endswith(('.pyc','.pyo')): errors.append('generated Python bytecode packaged: '+logical)
                if not info.is_dir() and is_embedded_archive(logical): errors.append('embedded archive payload packaged: '+logical)
                if not info.is_dir():
                    for label in sensitive_hits(Path(logical),z.read(info)):
                        errors.append('package sensitive material '+label+' in '+logical)
            manifest_name='metadata/ACTIVE_SOURCE_MANIFEST.json'
            if manifest_name not in file_set:
                errors.append('flat package missing embedded ACTIVE_SOURCE_MANIFEST.json')
            else:
                try:
                    embedded_manifest_bytes=z.read(manifest_name)
                    pm=json.loads(embedded_manifest_bytes.decode('utf-8'))
                except (KeyError,UnicodeError,json.JSONDecodeError) as e: errors.append('embedded ACTIVE_SOURCE_MANIFEST unreadable: '+str(e)); pm=None; embedded_manifest_bytes=None
                if embedded_manifest_bytes is not None and manifest_sha is not None:
                    embedded_manifest_sha=hashlib.sha256(embedded_manifest_bytes).hexdigest()
                    if embedded_manifest_sha!=manifest_sha:
                        errors.append('package/source manifest identity mismatch')
                if isinstance(pm,dict) and isinstance(pm.get('files'),dict):
                    expected_files=set(pm['files'])|{manifest_name}
                    if file_set!=expected_files:
                        errors.append(f'package/manifest membership mismatch missing={len(expected_files-file_set)} extra={len(file_set-expected_files)}')
                    info_by_name={logical:info for info,logical,_parts in safe_infos if not info.is_dir()}
                    for rel,expected_hash in pm['files'].items():
                        info=info_by_name.get(rel)
                        if info is None: continue
                        got=hashlib.sha256(z.read(info)).hexdigest()
                        if got!=expected_hash: errors.append('package manifest hash mismatch '+rel)
                elif pm is not None: errors.append('embedded ACTIVE_SOURCE_MANIFEST files mapping missing')
    except (OSError,zipfile.BadZipFile) as e:
        errors.append('package inspection failed: '+str(e))
script_count=sum(1 for p in (r/'scripts').iterdir() if p.is_file())
readme=(r/'README.md').read_text(); build=(r/'docs/BUILD_ME_FIRST.txt').read_text()
for token in ('%LOCALAPPDATA%\\surge.png','configured AuthCode returned','GetAuthCode uses Config2 AuthCode only'):
    if token in readme or token in build: errors.append('stale documentation token: '+token)
entitlement_test=(r/'tests/unit/test_entitlement_grammar.cpp').read_text(encoding='utf-8',errors='replace')
stale_consistency=(r/'tests/unit/test_service_state.cpp').read_text(encoding='utf-8',errors='replace')
if '<FilterOffers><OfferId>OFFER_B</OfferId></FilterOffers>' not in entitlement_test or 'return 13;' not in entitlement_test: errors.append('active direct-child entitlement grammar regression missing')
if 'F1585 current policy' not in stale_consistency or r'if(!has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\"")) return 103;' not in stale_consistency: errors.append('stale entitlement consistency fixture still conflicts with current direct-child policy')
if errors:
    print('FALCION_GOLDEN_CLEANING: FAIL'); [print(' -',x) for x in errors]; sys.exit(1)
suffix=(' package=FLAT' if args.package is not None else '')
print(f'FALCION_GOLDEN_CLEANING: PASS (root={len(required_root)+len(allowed_source_dirs)}, scripts={script_count} informational{suffix})')
if manifest_sha: print(' active_manifest_sha256='+manifest_sha)
