import json,subprocess,sys,time,xml.etree.ElementTree as ET
from pathlib import Path
from run_profile import W,S,H,Q,T,preflight
profile=sys.argv[1];build=Q/profile
preflight_receipt=json.loads((H/f'{profile}_EXEC_PREFLIGHT.json').read_text());assert preflight_receipt['status']=='PASS'
preflight(build,profile+"_runtime")
start=time.monotonic()
with (H/f'{profile}_ctest.log').open('w') as log:
 r=subprocess.run([str(T),'--test-dir',str(build),'--output-on-failure','--parallel','2','--output-junit',str(H/f'{profile}_CTEST.xml')],stdout=log,stderr=subprocess.STDOUT)
assert r.returncode==0,(profile,r.returncode)
tree=ET.parse(H/f'{profile}_CTEST.xml').getroot();cases=tree.findall('testcase')
assert len(cases)==52 and not tree.findall('.//failure') and not tree.findall('.//skipped')
result={'status':'PASS','tests':52,'failed':0,'skipped':0,'profile':profile,'seconds':round(time.monotonic()-start,3),'source_digest':json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'],'tests_detail':[{'name':c.attrib['name'],'time':c.attrib.get('time'),'output':c.findtext('system-out','')} for c in cases]}
(H/f'{profile}_TESTS.json').write_text(json.dumps(result,indent=2)+'\n');print(f'PASS {profile} CTest52/52',flush=True)
