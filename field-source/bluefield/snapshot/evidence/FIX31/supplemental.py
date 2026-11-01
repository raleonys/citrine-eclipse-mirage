import hashlib,json,os,subprocess,sys,time,zipfile
from pathlib import Path
from run_profile import W,S,H,Q
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
out=Q/'supplemental';out.mkdir(parents=True,exist_ok=True)
def run(cmd,label,reject=False,env=None):
 start=time.monotonic();r=subprocess.run(list(map(str,cmd)),cwd=S,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
 (H/(label+'.log')).write_bytes(r.stdout)
 assert (r.returncode==1 and b'FAIL unload scan:' in r.stdout) if reject else r.returncode==0,(label,r.returncode,r.stdout[:2000])
 return {'label':label,'command':list(map(str,cmd)),'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3),'output':r.stdout.decode(errors='replace')[:6000]}
driver=S/'tests/integration/test_unload_scan.cpp';body=S/'src/windows/unload_guard.cpp'
base=['g++','-std=c++20','-O1','-g','-fno-builtin-memcpy','-U_FORTIFY_SOURCE','-D_FORTIFY_SOURCE=0','-I'+str(S),'-I'+str(S/'tests/support/unload/winshim')]
link=['-Wl,--wrap=memcpy']
result={'status':'PASS','source_digest':frozen,'negative_controls':[],'sanitizers':[]}
with zipfile.ZipFile(W/'deliverables/BF02_FIX30/preloader_l_BF02_FIX30_APPLICATION_SOURCE_NOT_RELEASE.zip') as z:old=z.read('preloader_l/src/windows/unload_guard.cpp')
for label,contents,message in [('parent',old,'over-budget region was read before refusal'),
 ('false_safe',body.read_bytes().replace(b'if(limit-cursor>64ULL*1024*1024-scanned)return false;',b'if(limit-cursor>64ULL*1024*1024-scanned)return true;'),'oversize region refused'),
 ('off_by_one',body.read_bytes().replace(b'if(limit-cursor>64ULL*1024*1024-scanned)return false;',b'if(limit-cursor>=64ULL*1024*1024-scanned)return false;'),'exact budget read count')]:
 source=out/(label+'.cpp');source.write_bytes(contents);exe=out/label
 cmd=base+['-DPRELOADER_L_UNLOAD_SOURCE="'+str(source)+'"',driver]+link+['-o',exe]
 run(cmd,label+'_compile');exe.chmod(0o755);r=run([exe],label+'_rejected',True);assert message in r['output'];result['negative_controls'].append({**r,'actual_body_sha256':hashlib.sha256(contents).hexdigest(),'same_driver_sha256':hashlib.sha256(driver.read_bytes()).hexdigest()})
exe=out/'unload_focused_sanitizer';run(base+['-fsanitize=address,undefined','-fno-omit-frame-pointer',driver]+link+['-o',exe],'sanitizer_compile');exe.chmod(0o755)
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
r=run([exe],'unload_focused_sanitizer',env=env);r.update(instrumented_translation_units=['tests/integration/test_unload_scan.cpp (includes actual src/windows/unload_guard.cpp)'],production_core_archive_linked=False,full_core='NOT_RUN',LSan='NOT_RUN',native_SDK_SEH_OS='NOT_RUN');result['sanitizers'].append(r)
assert source_digest()==frozen
(H/'SUPPLEMENTAL.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS actual parent+false-safe+off-by-one controls rejected; actual-body TU ASan/UBSan PASS',flush=True)
