import hashlib,json,os,subprocess,sys,time
from pathlib import Path
from run_profile import W,S,H,Q
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
out=Q/'supplemental';out.mkdir(parents=True,exist_ok=True)
def run(cmd,label,reject=False,env=None):
 start=time.monotonic();r=subprocess.run(list(map(str,cmd)),cwd=S,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
 (H/(label+'.log')).write_bytes(r.stdout)
 assert (r.returncode==1 and b'FAIL ' in r.stdout) if reject else r.returncode==0,(label,r.returncode,r.stdout[:2000])
 return {'label':label,'command':list(map(str,cmd)),'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3),'output':r.stdout.decode(errors='replace')[:16000],'log':label+'.log'}
base=['g++','-std=c++20','-O1','-g','-pthread','-I'+str(S)];core=Q/'production/libpreloader_l_core.a';parent=W/'fix29_qa/production/libpreloader_l_core.a';assert core.exists() and parent.exists()
atomic=S/'src/files/atomic_io.cpp';native=S/'src/files/native_io.cpp';oracle=S/'tests/support/files/packet_binary_io.cpp';driver=S/'tests/integration/test_atomic_stage_collision.cpp';ticket=S/'tests/integration/test_ticket_output.cpp'
result={'status':'PASS','source_digest':frozen,'negative_controls':[],'sanitizers':[],'parent_build_pin':{'source_digest':source_digest(W/'fix29_work/preloader_l'),'host_core_sha256':hashlib.sha256(parent.read_bytes()).hexdigest(),'scope':'Portable exact FIX29 source model; not original DLL'}}
assert result['parent_build_pin']['source_digest']=='2f7a8c5d75326c6f3e03bd0069ba669168ae21520777b467cf6ff4584c86465b'
exe=out/'parent_collision';run(base+[oracle,driver,parent,'-o',exe],'parent_compile');exe.chmod(0o755);r=run([exe],'parent_rejection',True);assert 'atomic temporary quota' in r['output'];result['negative_controls'].append(r)
p=out/'native_broad_catch.cpp';p.write_text(native.read_text().replace('catch(const std::system_error&)','catch(const std::exception&)'));exe=out/'broad_catch'
run(base+[atomic,p,ticket,core,'-o',exe],'broad_catch_compile');exe.chmod(0o755);r=run([exe],'broad_catch_rejection',True);assert 'non-system open error masked' in r['output'];result['negative_controls'].append(r)
p=out/'short_budget.cpp';p.write_text(atomic.read_text().replace('attempt<100','attempt<99'));exe=out/'short_budget'
run(base+[p,oracle,driver,core,'-o',exe],'short_budget_compile');exe.chmod(0o755);result['negative_controls'].append(run([exe],'short_budget_rejection',True))
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
for label,inputs in [('collision_focused_sanitizer',[atomic,oracle,driver]),('ticket_focused_sanitizer',[atomic,native,ticket])]:
 exe=out/label;run(base+['-fsanitize=address,undefined','-fno-omit-frame-pointer']+inputs+[core,'-o',exe],label+'_compile');exe.chmod(0o755)
 r=run([exe],label,env=env);r['instrumented_translation_units']=[str(p.relative_to(S)) for p in inputs];r['uninstrumented_archive']=str(core.relative_to(W));r['full_core']='NOT_RUN';r['LSan']='NOT_RUN';result['sanitizers'].append(r)
assert source_digest()==frozen
(H/'SUPPLEMENTAL.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS parent+2 mutants rejected; focused collision/ticket ASan/UBSan PASS',flush=True)
