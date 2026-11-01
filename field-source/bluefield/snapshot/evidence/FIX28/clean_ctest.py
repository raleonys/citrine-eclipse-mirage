import hashlib,json,subprocess,time,xml.etree.ElementTree as ET
from run_profile import S,H,Q,T
build=Q/'clean_build';result=json.loads((H/'CLEAN_BUILD.json').read_text());assert json.loads((H/'clean_EXEC_PREFLIGHT.json').read_text())['status']=='PASS'
start=time.monotonic()
with (H/'clean_ctest.log').open('w') as f:r=subprocess.run([str(T),'--test-dir',str(build),'--output-on-failure','--parallel','2','--output-junit',str(H/'clean_CTEST.xml')],stdout=f,stderr=subprocess.STDOUT)
assert r.returncode==0
tree=ET.parse(H/'clean_CTEST.xml').getroot();cases=tree.findall('testcase');assert len(cases)==49 and not tree.findall('.//failure') and not tree.findall('.//skipped')
result['CTest']={'status':'PASS','tests':49,'failed':0,'skipped':0,'seconds':round(time.monotonic()-start,3),'tests_detail':[{'name':c.attrib['name'],'output':c.findtext('system-out','')} for c in cases]}
archive=S/'dist/preloader_l_BF02_FIX28_APPLICATION_SOURCE_NOT_RELEASE.zip';assert hashlib.sha256(archive.read_bytes()).hexdigest()==result['archive_sha256']
result['status']='PASS';result['archive_unchanged_after_replay']=True
(H/'CLEAN_REPLAY.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS exact sealed ZIP clean replay49/49; archive unchanged',flush=True)
