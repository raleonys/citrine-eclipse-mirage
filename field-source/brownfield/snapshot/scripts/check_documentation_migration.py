#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,sys,re
r=Path(__file__).resolve().parents[1]; errors=[]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else None
for d in ['docs/history','docs/historical','docs/research','docs/governance']:
    if (r/d).exists(): errors.append('forbidden live-doc archive directory: '+d)
_gb=json.loads((r/'metadata/GOVERNANCE_BINDING.json').read_text())
if sha(r/'docs/PROJECT_GOVERNANCE_CONTRACT.md')!=_gb.get('packaged_mirror',{}).get('md_sha256'): errors.append('governance MD mirror drift')
if sha(r/'metadata/PROJECT_GOVERNANCE_INVARIANTS.json')!=_gb.get('packaged_mirror',{}).get('json_sha256'): errors.append('governance JSON mirror drift')
try: m=json.loads((r/'metadata/DOCUMENTATION_HISTORY_MIGRATION.json').read_text())
except Exception as e: m={}; errors.append('migration ledger unreadable: '+repr(e))
if m.get('coverage_percent')!=100.0: errors.append('history migration coverage != 100%')
if m.get('file_count')!=len(m.get('files',[])): errors.append('ledger file_count mismatch')
if m.get('record_count')!=sum(x.get('record_count',0) for x in m.get('files',[])): errors.append('ledger record_count mismatch')
if m.get('mandatory_melotrix_backfill')!='COMPLETE_IN_CHANGELOG_3_2_3_SECTION': errors.append('mandatory MELOTRIX backfill status missing')
a=m.get('migration_source_archive',{})
if a.get('sha256')!='d7d6d841391bb4733c84513331bce9ea953dd7346dd260b298deb955a2f70169': errors.append('migration archive SHA drift')
if a.get('library_id')!='libfile_3a0d977ed0b08191bdeeee796fa98b4b': errors.append('migration archive Library ID drift')
allowed={'MIGRATED_TO_CHANGELOG','DUPLICATE_ALREADY_PRESENT','PROVENANCE_ONLY_WITH_IMMUTABLE_LIBRARY_BINDING'}
for f in m.get('files',[]):
    if f.get('file_disposition') not in allowed: errors.append('invalid file disposition '+str(f.get('source_path')))
    if f.get('record_count')!=len(f.get('records',[])): errors.append('record count mismatch '+str(f.get('source_path')))
    for rec in f.get('records',[]):
        if rec.get('disposition') not in allowed: errors.append('invalid record disposition '+str(f.get('source_path')))
        if not rec.get('sha256'): errors.append('record SHA missing '+str(f.get('source_path')))
for x in m.get('externalized_nonhistory_docs',[]):
    if x.get('disposition')!='PROVENANCE_ONLY_WITH_IMMUTABLE_LIBRARY_BINDING': errors.append('externalized doc disposition drift '+str(x.get('path')))
ch=(r/'CHANGELOG.md').read_text(encoding='utf-8',errors='replace')
required=[
 '## 3.2.5 SQUALERIS','Audit2 remediation convergence carried into SQUALERIS',
 '## 3.2.3 MELOTRIX','MinHook-only SAFE-FREEZE architecture',
 'Pre-audit remediation and ownership/lifecycle convergence','Repository, naming and QA architecture',
 'Acceptance-oracle convergence and Windows closure','Historical limitations and non-goals',
 'NtGetNextThread','9/9 hooks','rollback-incomplete','PTD initialization failure',
 '288 approved project-owned path decisions','Native Acceptance','CRT Transition Acceptance'
]
for token in required:
    if token not in ch: errors.append('CHANGELOG non-loss token missing: '+token)
# Current docs must not delegate history to removed source-tree archives.
for rel in ['README.md','SOURCE_REVISION.md','docs/RELEASE_NOTES.md','docs/RELEASE_PROVENANCE.md','docs/RELEASE_ACCEPTANCE.md']:
    txt=(r/rel).read_text(encoding='utf-8',errors='replace')
    if re.search(r'preserved under `?docs/(?:history|historical|research)/',txt,re.I): errors.append('current doc delegates to forbidden archive: '+rel)
if errors:
    print('DOCUMENTATION_HISTORY_MIGRATION: FAIL'); [print(' -',x) for x in errors]; sys.exit(1)
print(f"DOCUMENTATION_HISTORY_MIGRATION: PASS (files={m.get('file_count')}, records={m.get('record_count')}, externalized={m.get('externalized_nonhistory_count')}, coverage=100%)")
