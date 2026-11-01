#!/usr/bin/env python3
import pathlib,hashlib,json,sys
root=pathlib.Path(__file__).resolve().parents[1]
m=json.loads((root/'metadata/manifest.json').read_text())
actual={str(p.relative_to(root)).replace('\\','/') for p in root.rglob('*') if p.is_file() and not any(x.startswith('_local_') for x in p.relative_to(root).parts) and '.git' not in p.parts}
expected=set(m['files'])|{'metadata/manifest.json'}
if actual!=expected:sys.exit(f'FILE_SET_INVALID:{sorted(actual^expected)}')
for name,digest in m['files'].items():
 if hashlib.sha256((root/name).read_bytes()).hexdigest()!=digest:sys.exit('HASH_MISMATCH:'+name)
reuse=json.loads((root/'metadata/CODE_REUSE_LEDGER.json').read_text())
if len(reuse['copied_implementation_sources'])!=30:sys.exit('REUSE_COUNT_MISMATCH')
for entry in reuse['copied_implementation_sources']:
 if hashlib.sha256((root/entry['import_path']).read_bytes()).hexdigest()!=entry['sha256']:sys.exit('ADOPTED_SOURCE_DRIFT')
if hashlib.sha256((root/reuse['original_notice_path']).read_bytes()).hexdigest()!=reuse['original_notice_sha256']:sys.exit('NOTICE_DRIFT')
if (root/'metadata/STATUS.json').is_file():
 status=json.loads((root/'metadata/STATUS.json').read_text())
 if status['release']!='NOT_RELEASE' or status['canonical_source_mutation'] or status['publication_authorized'] or status['state_id']!='GF02':sys.exit('WRONG_RELEASE_STATUS')
code=(root/'src/core/resource_owners.cpp').read_text()
if 'if (closed) entries_[i-1].cleanup = nullptr;' not in code:sys.exit('RETRY_OWNER_INVARIANT_DRIFT')
# Fresh identities and source contract, no Windows/native claims.
import re
exports=(root/'exports/preloader_l.def').read_text()
pairs=re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',exports,re.M)
if sorted(pairs)!=sorted([('DllMain','1'),('SURGE','2'),('DllShutdown','3')]):sys.exit('EXPORT_CONTRACT_INVALID')
print('GF02_MANIFEST_REUSE_AND_STATUS_PASS:'+str(len(m['files'])))
