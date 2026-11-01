#!/usr/bin/env python3
"""GF02 optional physical parent check: python scripts/verify_imports_against_hf19.py HF19.zip GF01.zip"""
from pathlib import Path
from zipfile import ZipFile
import hashlib,json,sys
root=Path(__file__).resolve().parents[1]
h=lambda b:hashlib.sha256(b).hexdigest()
if len(sys.argv)!=3:sys.exit('USAGE: verify_imports_against_hf19.py exact_HF19.zip exact_GF01.zip')
hf,prior=map(Path,sys.argv[1:]); d=json.loads((root/'metadata/GF02_PARENT_DELTA.json').read_text())
if h(hf.read_bytes())!=d['donor_hf19_source_zip_sha256'] or h(prior.read_bytes())!=d['parent_source_zip_sha256']:sys.exit('WRONG_SOURCE_PARENT_HASH')
led=json.loads((root/'metadata/CODE_REUSE_LEDGER.json').read_text())
with ZipFile(hf) as z:
 if z.testzip():sys.exit('HF19_CRC_FAIL')
 for e in led['copied_implementation_sources']:
  if z.read(e['original_path'])!=(root/e['import_path']).read_bytes():sys.exit('HF19_BYTE_DRIFT:'+e['original_path'])
  if h(z.read(e['original_path']))!=e['sha256']:sys.exit('HF19_HASH_DRIFT:'+e['original_path'])
 if z.read('LICENSE')!=(root/'licenses/HF19_LICENSE').read_bytes():sys.exit('LICENSE_CHANGED')
with ZipFile(prior) as z:
 if z.testzip():sys.exit('GF01_CRC_FAIL')
 changed={'CMakeLists.txt','metadata/manifest.json','metadata/CODE_REUSE_LEDGER.json','metadata/STATUS.json','metadata/QA_STATUS.json','docs/README_TR.md','scripts/run_portable_qa.sh','scripts/verify_package.py','scripts/verify_gf01.py','scripts/verify_imports_against_hf19.py'}
 for f in z.namelist():
  if f in changed:continue
  if not (root/f).exists() or z.read(f)!=(root/f).read_bytes():sys.exit('GF01_PARENT_DRIFT:'+f)
print('GF02_EXACT_HF19_30_AND_GF01_PARENT_PASS')
