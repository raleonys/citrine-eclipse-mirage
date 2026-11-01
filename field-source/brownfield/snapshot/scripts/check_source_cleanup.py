#!/usr/bin/env python3
from pathlib import Path
import json,sys
r=Path(__file__).resolve().parents[1]
s=json.loads((r/'RELEASE_STATE.json').read_text())
m=json.loads((r/'metadata/METADATA_OWNERSHIP.json').read_text())
e=[]
if s.get('hotfix_ordinal')!=16:e.append('release identity')
for p in ('metadata/INTERNAL_REMEDIATION_STATE.json','metadata/REMEDIATION_RECORD.json'):
 if (r/p).exists():e.append('obsolete historical metadata returned: '+p)
if not (r/'docs/ASIO_PRODUCTION.md').is_file():e.append('current Asio documentation missing')
for p in m.get('retired_from_current_source',{}):
 if (r/p).exists():e.append('retired metadata returned: '+p)
for p in list((r/'docs').glob('HF11_*'))+list((r/'docs').glob('ASIO_*')):
 if p.name!='ASIO_PRODUCTION.md':e.append('unretired development checkpoint: '+p.name)
a=json.loads((r/'metadata/ASIO_ADOPTION_CONTRACT.json').read_text())
if a.get('scope')!='CURRENT_SHIPPING_ASIO_IOCP_PRODUCTION':e.append('stale adoption scope')
if a.get('candidate_usage')!='SHIPPING_NATIVE_RELEASERVER_ASIO_ACTIVE':e.append('stale Asio active routing')
if not s.get('documentation_cleanup',{}).get('archived_source_records')==21:e.append('archive record count')
for folder in ('src','include','tests','scripts','tools','metadata','docs'):
 for q in (r/folder).rglob('*'):
  if q.is_file() and (q.name.startswith(('HF11_','HF12_','HF13_','hf10_','test_hf10_','test_hf11_','test_hf12_','test_request_hf11_'))):
   e.append('transient release bookkeeping in active basename: '+str(q.relative_to(r)))
if e:
 print('SOURCE_CLEANUP: FAIL');[print(' -',x) for x in e];sys.exit(1)
print('SOURCE_CLEANUP: PASS (21 archived stage files absent, current Asio/Hotfix authority)')
