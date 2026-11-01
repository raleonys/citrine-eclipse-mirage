#!/usr/bin/env python3
from pathlib import Path
import argparse,hashlib,json,sys

def sha256(p: Path):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(1024*1024),b''): h.update(b)
    return h.hexdigest()

ap=argparse.ArgumentParser()
ap.add_argument('--gate',required=True)
ap.add_argument('--execution-level',required=True,choices=['source_static','portable_compile_or_runtime','windows_compile_link','windows_native_acceptance','crt_transition','game_network_acceptance'])
ap.add_argument('--platform',required=True)
ap.add_argument('--command',required=True)
ap.add_argument('--status',required=True,choices=['PASS','FAIL','PENDING'])
ap.add_argument('--source-zip',required=True)
ap.add_argument('--artifact',action='append',default=[])
ap.add_argument('--scenario',action='append',default=[])
ap.add_argument('--output',required=True)
a=ap.parse_args()
source=Path(a.source_zip)
if not source.is_file(): raise SystemExit('ACCEPTANCE_RECORD: FAIL - source zip missing')
arts=[]
for item in a.artifact:
    p=Path(item)
    if not p.is_file(): raise SystemExit('ACCEPTANCE_RECORD: FAIL - artifact missing '+item)
    arts.append({'path':p.name,'sha256':sha256(p),'bytes':p.stat().st_size})
record={'schema_version':1,'gate':a.gate,'execution_level':a.execution_level,'platform':a.platform,'command':a.command,'status':a.status,'source_zip':{'path':source.name,'sha256':sha256(source),'bytes':source.stat().st_size},'artifacts':arts,'scenarios':a.scenario}
Path(a.output).write_text(json.dumps(record,indent=2,sort_keys=True)+'\n',encoding='utf-8')
print('ACCEPTANCE_RECORD: PASS')
