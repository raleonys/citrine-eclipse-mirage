#!/usr/bin/env python3
from pathlib import Path, PurePosixPath
import hashlib,json,re,sys,csv
r=Path(__file__).resolve().parents[1]
errors=[]
EXPECTED_BASELINE='2436b2dd97f3bb65d6be7b53f680746e71dac915a2f771c60aa4175f549101e9'
EXPECTED_REVIEW='4279bcbaf91ee463a6fdaca5bc0de0632acd0cb8df06340cb76c4eaa81aa745f'
EXPECTED_MAP='5573627b9402daac818ec5909f3a32b8db12a93d325e30aa4ebf8b8e598c4000'
EXPECTED_CSV='052e3f37150c935d3cb414e1887ce4c6b2c978cc6f3c25866f615b9c2559e488'
EXPECTED_LEDGER='cec2d5e0f2b4c71e76f0eaea4cd2112d625467f55896a4a13b8a7fca82da7c20'
paths={
 'review':r/'metadata/FILENAME_MIGRATION_REVIEW.xlsx',
 'map':r/'metadata/FILENAME_MIGRATION_MAP.json',
 'csv':r/'metadata/FILENAME_MIGRATION_MAP.csv',
 'ledger':r/'metadata/FILENAME_MIGRATION_HASH_LEDGER.csv',
 'provenance':r/'metadata/FILENAME_MIGRATION_PROVENANCE.json',
}
for k,p in paths.items():
    if not p.is_file(): errors.append('missing '+k+': '+p.relative_to(r).as_posix())
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
if paths['review'].is_file() and sha(paths['review'])!=EXPECTED_REVIEW: errors.append('review hash drift')
if paths['map'].is_file() and sha(paths['map'])!=EXPECTED_MAP: errors.append('map hash drift')
if paths['csv'].is_file() and sha(paths['csv'])!=EXPECTED_CSV: errors.append('csv hash drift')
if paths['ledger'].is_file() and sha(paths['ledger'])!=EXPECTED_LEDGER: errors.append('W10 hash-ledger artifact drift')
items=[]
if paths['map'].is_file():
    m=json.loads(paths['map'].read_text(encoding='utf-8'))
    if m.get('baseline_sha256')!=EXPECTED_BASELINE: errors.append('baseline hash drift in map')
    if m.get('source_review_sha256')!=EXPECTED_REVIEW: errors.append('review binding drift in map')
    items=m.get('renames',[])
if len(items)!=288: errors.append(f'rename count {len(items)} != 288')
old_paths=[x.get('old_path','') for x in items]; new_paths=[x.get('new_path','') for x in items]
if len(old_paths)!=len(set(old_paths)): errors.append('duplicate old paths')
if len(new_paths)!=len(set(new_paths)): errors.append('duplicate new paths')
post_w10_retired={'tests/support/architecture_hook_allocator_model.cpp','docs/GS_HARDENING_RESEARCH.md','docs/research/ROUTE_ERROR_CONTRACT_RESEARCH.md','docs/research/QUERY_CHUNK_STATUS_CORRECTION.md','docs/research/SOCIAL_ROUTE_ERROR_CONTRACT_RESEARCH.md','docs/research/MELOTRIX_FILENAME_MIGRATION.md','docs/research/MELOTRIX_NATIVE_ACCEPTANCE_PROTOCOL_SURFACE_REALIGNMENT.md','scripts/build/build_crt_acceptance.ps1','scripts/build/build_debug.ps1','scripts/build/build_native_acceptance.ps1','scripts/build/build_production.ps1','scripts/build/build_minhook_vendor.ps1','src/runtime/service_source.cpp','src/config/store_xml.cpp','src/config/store_entitlements.cpp'}
# HF13-approved full removal of the two obsolete native hooks. Keep immutable
# 288-entry W10 migration evidence unchanged; retired current paths are not
# required to exist in new source packages.
post_w10_retired.update({
    'include/preloader_l/platform/surge_hooks.hpp',
    'include/preloader_l/platform/surge_policy.hpp',
    'src/platform/surge_hooks.cpp',
    'src/platform/surge_hooks_internal.hpp',
    'src/platform/surge_inline_hooks.cpp',
    'src/platform/surge_policy.cpp',
    'src/platform/surge_wrapper_hooks.cpp',
    'tests/unit/test_native_hook_status.cpp',
})
for x in items:
    op=r/x['old_path']; np=r/x['new_path']
    if op.exists(): errors.append('old active path remains: '+x['old_path'])
    if x['new_path'] in post_w10_retired:
        if np.exists(): errors.append('post-W10 retired path returned: '+x['new_path'])
        continue
    if not np.is_file(): errors.append('new path missing: '+x['new_path'])

# Governance revision 2026-10-07.8 supersedes the historical scripts/build location and normalizes active launcher names.
for rel in ['scripts/build_orchestration/build_crt_acceptance.ps1','scripts/build_orchestration/build_debug.ps1','scripts/build_orchestration/build_native_acceptance.ps1','scripts/build_orchestration/build_production.ps1','scripts/build_orchestration/build_minhook_vendor.ps1']:
    if not (r/rel).is_file(): errors.append('build orchestration migration target missing: '+rel)
if (r/'scripts/build').exists(): errors.append('superseded scripts/build directory returned')
for p in (r/'scripts/build_orchestration').iterdir():
    if p.is_file() and p.name.startswith('BUILD_') and p.suffix.lower() == '.cmd':
        errors.append('uppercase active build launcher remains: '+p.relative_to(r).as_posix())

# Hard naming rules on active project-owned physical basenames.
codenames=('ARYRA','MUSTAVEX','CORVEXON','LUTRAVEX','FALCION','URSYNEX','MELOTRIX','CASTORADRON','SCIURONYX','VULPHEXON','MARTAVEX','CERVRAX','MUSTORIN','CAPRYRA','RANQUEL')
for p in r.rglob('*'):
    if not p.is_file(): continue
    rel=p.relative_to(r).as_posix()
    if rel.startswith('third_party/') or '__pycache__' in rel: continue
    low=p.name.lower()
    # Exact Win32 SDK-compat header required by the source-linked mock test; never a production header.
    is_pinned_mock_sdk=(rel=='tests/support/optional_hook_api_shim/windows.h' and sha(p)=='840ac97a8be0c6519bc8d49d3e6b91686c5bc2b9854ec058b4e90475b6b7b77e' and json.loads((r/'RELEASE_STATE.json').read_text()).get('release_type')=='HOTFIX_RELEASE')
    if 'windows' in low and not is_pinned_mock_sdk: errors.append('windows basename remains: '+rel)
    if 'x64' in low: errors.append('x64 basename remains: '+rel)
    if re.search(r'(^|_)platform(_|\.|$)',low): errors.append('platform placeholder basename remains: '+rel)
    if 'internet_state' in low or 'internet_route' in low: errors.append('internet_state vocabulary remains: '+rel)
    if re.fullmatch(r'settings?_route\.(?:cpp|hpp)',low): errors.append('weak setting/settings route pair remains: '+rel)
# Protected technical/codename identity may not be removed by mapped renames.
for x in items:
    o=x['old_name'].lower(); n=x['new_name'].lower()
    if 'tinyxml' in o and 'tinyxml' not in n: errors.append('TinyXML identity removed: '+x['old_path'])
    if 'minhook' in o and 'minhook' not in n: errors.append('MinHook identity removed: '+x['old_path'])
    if 'golden' in o: errors.append('Golden filename unexpectedly mapped: '+x['old_path'])
    if any(c.lower() in o for c in codenames): errors.append('codename filename unexpectedly mapped: '+x['old_path'])
# Operational stale references: full repo paths and known compiler include-root relative paths.
variants=[]
for x in items:
    o=x['old_path']; n=x['new_path']
    variants.append(o)
    if o.startswith('include/'): variants.append(o[len('include/'):])
    if o.startswith('tests/support/'): variants.append(o[len('tests/support/'):])
    if o.startswith('src/debug/'): variants.append(o[len('src/debug/'):])
# De-duplicate and prefer long tokens.
variants=sorted(set(v for v in variants if '/' in v),key=len,reverse=True)
scan_roots=['CMakeLists.txt','src','include','tests','tools','scripts','metadata']
exclude={
 'scripts/check_filename_migration.py','scripts/check_repository_contract.py',
 'metadata/FILENAME_MIGRATION_MAP.json','metadata/FILENAME_MIGRATION_MAP.csv',
 'metadata/FILENAME_MIGRATION_HASH_LEDGER.csv','metadata/FILENAME_MIGRATION_PROVENANCE.json',
 'metadata/DOCUMENTATION_PRESERVATION_BASELINE.json','metadata/DOCUMENTATION_HISTORY_MIGRATION.json','metadata/ACTIVE_SOURCE_MANIFEST.json',
}
scan=[]
for root in scan_roots:
    p=r/root
    if p.is_file(): scan.append(p)
    elif p.is_dir(): scan.extend(q for q in p.rglob('*') if q.is_file())
for p in scan:
    rel=p.relative_to(r).as_posix()
    if rel in exclude or rel.startswith('third_party/'): continue
    try: data=p.read_bytes()
    except OSError: continue
    for v in variants:
        if v.encode() in data:
            errors.append('stale operational reference '+v+' in '+rel); break
# Operational basename/dynamic reference closure missed by the original W10 checker.
# Build launchers live under scripts/build and must resolve their same-directory PowerShell target.
launcher_targets={
 'scripts/build_orchestration/build_production.cmd':'build_production.ps1',
 'scripts/build_orchestration/build_native_acceptance.cmd':'build_native_acceptance.ps1',
 'scripts/build_orchestration/build_debug.cmd':'build_debug.ps1',
 'scripts/build_orchestration/build_crt_acceptance.cmd':'build_crt_acceptance.ps1',
}
for launcher,target in launcher_targets.items():
    lp=r/launcher
    if not lp.is_file(): errors.append('missing build launcher: '+launcher); continue
    text=lp.read_text(encoding='utf-8',errors='replace')
    if target not in text: errors.append('stale launcher basename: '+launcher+' -> '+target)
    if not (lp.parent/target).is_file(): errors.append('launcher target missing: '+launcher+' -> '+target)
# Optional debug-host graph uses generated host names; resolve the template against all declared hosts.
cm=(r/'CMakeLists.txt').read_text(encoding='utf-8',errors='replace')
if 'tools/preloader_l_${host}_host/${host}_host_main.cpp' not in cm:
    errors.append('dynamic debug-host source template not rebound to semantic host main filenames')
for host in ('debug','runtime','session','socket'):
    rel=f'tools/preloader_l_{host}_host/{host}_host_main.cpp'
    if not (r/rel).is_file(): errors.append('dynamic debug-host source missing: '+rel)
# Current top-level documentation must not point at migrated-away full paths.
for dp in (r/'docs').glob('*'):
    if not dp.is_file() or dp.name in ('RELEASE_PROVENANCE.md',): continue
    try: doc=dp.read_text(encoding='utf-8',errors='replace')
    except OSError: continue
    for x in items:
        if x['old_path'] in doc:
            errors.append('stale current-doc path '+x['old_path']+' in docs/'+dp.name)
            break
# Post-W10 active filename contract: provenance identifiers live in history/content, not active basenames.
for fp in r.rglob('*'):
    if not fp.is_file(): continue
    rel=fp.relative_to(r).as_posix()
    if rel.startswith(('third_party/','docs/history/','docs/historical/')): continue
    stem=fp.stem.lower()
    if re.search(r'(?:hotfix|audit[-_]?hotfix|(?:^|[_-])audit[0-9]+(?:[_-]|$)|preaudit|phase[0-9]|stage[0-9]|wave[0-9]|(?:^|_)w[0-9]+(?:_|$)|(?:^|[_-])rc[0-9]+(?:[_-]|$))',stem):
        errors.append('active filename carries transient lineage identifier: '+rel)
    if re.search(r'(?<![0-9])[0-9]+_[0-9]+_[0-9]+(?![0-9])',stem) or re.search(r'(?<![0-9])[0-9]+\.[0-9]+\.[0-9]+(?![0-9])',stem):
        errors.append('active filename carries semantic version: '+rel)
correction=r/'metadata/NAMING_REFERENCE_CORRECTIONS.json'
if not correction.is_file(): errors.append('post-W10 naming/reference correction record missing')


# Historical release snapshots / experiments belong in Library, never current source metadata.
for rel in ['metadata/BASE_CERVRAX_ACTIVE_SOURCE_MANIFEST.json','metadata/BASE_CERVRAX_RELEASE_STATE.json','metadata/CONNECT_REDIRECT_EXPERIMENT.json']:
    if (r/rel).exists(): errors.append('retired historical metadata returned to current source: '+rel)

# Hash ledger/provenance internal closure.
if paths['ledger'].is_file():
    with paths['ledger'].open(newline='',encoding='utf-8') as f: ledger=list(csv.DictReader(f))
    if len(ledger)!=288: errors.append(f'hash ledger count {len(ledger)} != 288')
    lmap={row.get('new_path'):row for row in ledger}
    for x in items:
        row=lmap.get(x['new_path'])
        if not row: errors.append('hash ledger missing '+x['new_path']); continue
        if not re.fullmatch(r'[0-9a-f]{64}',row.get('w10_sha256','')): errors.append('invalid W10 hash ledger digest '+x['new_path'])
if paths['provenance'].is_file():
    pv=json.loads(paths['provenance'].read_text(encoding='utf-8'))
    if pv.get('baseline_zip_sha256')!=EXPECTED_BASELINE: errors.append('provenance baseline drift')
    if pv.get('rename_count')!=288: errors.append('provenance rename count drift')
    ah=pv.get('artifact_hashes',{})
    expected={'review':EXPECTED_REVIEW,'map_json':EXPECTED_MAP,'map_csv':EXPECTED_CSV,'hash_ledger':EXPECTED_LEDGER}
    for k,v in expected.items():
        if ah.get(k)!=v: errors.append('provenance artifact hash drift: '+k)
if errors:
    print('FILENAME_MIGRATION: FAIL')
    for e in errors[:100]: print(' -',e)
    sys.exit(1)
print('FILENAME_MIGRATION: PASS (288/288 paths; naming rules; references; immutable W10 hash-ledger artifact)')
