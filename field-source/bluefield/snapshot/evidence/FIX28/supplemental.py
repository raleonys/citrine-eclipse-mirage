import hashlib,json,os,subprocess,sys,time,zipfile
from pathlib import Path
W=Path(__file__).resolve().parents[1];S=W/'fix28_work/preloader_l';H=W/'fix28_helpers';Q=W/'fix28_qa'
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
O=Q/'supplemental';O.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def execute(argv,label,expect=0,env=None):
 start=time.monotonic();r=subprocess.run(list(map(str,argv)),cwd=S,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
 (H/f'{label}.log').write_bytes(r.stdout)
 if expect=='rejected':assert r.returncode==1 and b'FAIL ' in r.stdout,(label,r.returncode,r.stdout[:500])
 else:assert r.returncode==expect,(label,r.returncode,r.stdout[:500])
 return {'label':label,'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3),'log':f'{label}.log','output':r.stdout.decode(errors='replace')[:12000]}
base=['g++','-std=c++20','-O1','-g','-pthread','-I'+str(S),'-I'+str(S/'third_party/pegtl/include')]
parser=S/'src/configuration/parser.cpp';reference=S/'tests/support/configuration/packet_parser.cpp';driver=S/'tests/integration/test_config2_source_oracle.cpp'
core=Q/'production/libpreloader_l_core.a';debugcore=Q/'debug/libpreloader_l_core.a'
assert core.is_file() and debugcore.is_file()
results={'source_digest':frozen,'status':'PASS','scope':'temporary mutants and explicitly focused host sanitizer; no original DLL/Windows execution','mutants':[],'sanitizers':[]}
body=parser.read_text()
mutants=[('allow_adjacent_key_value','struct KeyValue : peg::seq<Key,Seps,Value> {};','struct KeyValue : peg::seq<Key,peg::star<Sep>,Value> {};'),('retain_BOM','if(state.removed_utf8_bom)raw.remove_prefix(3);','/* mutation: do not remove BOM */')]
for label,old,new in mutants:
 assert body.count(old)==1
 p=O/f'{label}.cpp';p.write_text(body.replace(old,new));exe=O/label
 execute(base+[p,reference,driver,core,'-o',exe],label+'_compile');exe.chmod(0o755)
 result=execute([exe],label+'_rejection','rejected');assert 'source mismatch' in result['output'];result['mutated_candidate_sha256']=sha(p);result['reference_sha256']=sha(reference);results['mutants'].append(result)
# Compile the FIX27 formatter body against CURRENT headers. This is a projected
# counterexample, not an original complete FIX27 binary or a cross-TU enum ODR mix.
with zipfile.ZipFile(W/'fix28_inputs/preloader_l_BF02_FIX27_APPLICATION_SOURCE_NOT_RELEASE.zip') as z:oldlogger=z.read('preloader_l/src/application/logging.cpp')
p=O/'parent_formatter_current_interface.cpp';p.write_bytes(oldlogger);exe=O/'parent_formatter_current_interface'
execute(base+['-DPRELOADER_L_DISK_DIAGNOSTICS=1',p,S/'tests/integration/test_failure_diagnostics.cpp',debugcore,'-o',exe],'parent_formatter_projection_compile');exe.chmod(0o755)
result=execute([exe],'parent_formatter_projection_rejection','rejected');assert 'startup failure event lost' in result['output'];result['scope']='Sealed FIX27 logging.cpp body compiled against current appended-event enum headers; not complete parent/native execution';results['mutants'].append(result)
san=['-fsanitize=address,undefined','-fno-omit-frame-pointer'];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
for label,inputs,archive,defines in [('oracle_focused_sanitizer',[parser,reference,driver],core,[]),('diagnostics_focused_sanitizer',[S/'src/application/logging.cpp',S/'tests/integration/test_failure_diagnostics.cpp'],debugcore,['-DPRELOADER_L_DISK_DIAGNOSTICS=1'])]:
 exe=O/label;execute(base+san+defines+inputs+[archive,'-o',exe],label+'_compile');exe.chmod(0o755)
 result=execute([exe],label,env=env);result['instrumented_translation_units']=[str(p.relative_to(S)) for p in inputs];result['uninstrumented_archive']=str(archive.relative_to(W));result['leak_sanitizer']='NOT_RUN';results['sanitizers'].append(result)
assert source_digest()==frozen
(H/'SUPPLEMENTAL.json').write_text(json.dumps(results,indent=2,ensure_ascii=False)+'\n');print('PASS 3 real negative controls and 2 focused ASan/UBSan executions',flush=True)
