#!/usr/bin/env python3
from pathlib import Path
import json,sys
r=Path(__file__).resolve().parents[1]
errors=[]
p=r/'metadata/METADATA_OWNERSHIP.json'
if not p.is_file():
    print('METADATA_OWNERSHIP: FAIL - registry missing'); sys.exit(1)
d=json.loads(p.read_text(encoding='utf-8'))
entries=d.get('files',{})
actual={x.name for x in (r/'metadata').glob('*.json')}
listed=set(entries)
if actual!=listed:
    for x in sorted(actual-listed): errors.append('unowned current metadata: metadata/'+x)
    for x in sorted(listed-actual): errors.append('owned metadata missing: metadata/'+x)
for rel in d.get('retired_from_current_source',{}):
    if (r/rel).exists(): errors.append('retired historical/experimental metadata returned: '+rel)
allowed={'CURRENT_AUTHORITY','CURRENT_LEDGER','CURRENT_CONTRACT','HISTORICAL_LEDGER_CURRENTLY_REQUIRED','CURRENT_CONTRACT_WITH_INTRODUCTION_HISTORY','CURRENT_RELEASE_SNAPSHOT','CURRENT_CONTRACT_WITH_LINEAGE','CURRENT_LEDGER_WITH_LINEAGE','CURRENT_CONTRACT_WITH_RESEARCH_PROVENANCE','DEPENDENCY_PROVENANCE'}
for name,rec in entries.items():
    if rec.get('role') not in allowed: errors.append('unknown metadata role: '+name)
    if not str(rec.get('purpose','')).strip(): errors.append('metadata purpose missing: '+name)
# Current contracts may retain old releases only as explicitly named introduction/lineage fields.
for name in ['ORIGIN_ERROR_ROUTE_CONTRACTS.json','QUERY_CHUNK_STATUS_PRECEDENCE.json']:
    q=json.loads((r/'metadata'/name).read_text(encoding='utf-8'))
    if q.get('contract_scope')!='CURRENT_RELEASE_INDEPENDENT': errors.append('current contract scope drift: '+name)
    if q.get('introduced_in_release')!='3.2.1 CERVRAX': errors.append('introduction lineage drift: '+name)
    if 'release' in q: errors.append('stale current-contract release field returned: '+name)
# Current source must not point at removed in-tree history/research surfaces.
state=json.loads((r/'RELEASE_STATE.json').read_text(encoding='utf-8'))
h=state.get('historical_lineage',{})
if h.get('ownership')!='IMMUTABLE_LIBRARY_RELEASE_EVIDENCE' or h.get('in_source_archive_required') is not False:
    errors.append('historical lineage ownership is not Library/current-source-clean')
# Third-party project-owned wrappers/provenance must be release-independent.
minhook=(r/'third_party/minhook/README.md').read_text(encoding='utf-8',errors='replace')
tiny=(r/'third_party/tinyxml2/REFERENCE_PROVENANCE.md').read_text(encoding='utf-8',errors='replace')
for token in ['3.2.3 MELOTRIX','CERVRAX_MH_SAFE_FREEZE']:
    if token in minhook: errors.append('stale project release identity in MinHook wrapper: '+token)
for token in ['MELOTRIX pre-audit','Wave 6']:
    if token in tiny: errors.append('stale project release identity in TinyXML provenance: '+token)
if errors:
    print('METADATA_OWNERSHIP: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print(f'METADATA_OWNERSHIP: PASS ({len(actual)}/{len(actual)} current JSON files classified; 3 retired snapshots/experiment excluded)')
