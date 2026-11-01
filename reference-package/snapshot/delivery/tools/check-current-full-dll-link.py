from pathlib import Path
from datetime import datetime,timezone
import hashlib,json,re,subprocess
R=Path.cwd();O=R/'outputs';S=O/'source-workspace';B=S/'build';D=O/'dll-link-gate-source';T=D/'build-current-full';assert not T.exists();T.mkdir()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
record=read(S/'build-record.json');snapshot=read(S/'source-snapshot.json');assert record['result']=='PASS' and snapshot['build_record_sha256']==sha(S/'build-record.json')
for row in snapshot['sources']:assert sha(O/row['path'])==row['sha256']
compiler=R/'work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin/clang++.exe';assert sha(compiler)==record['compiler_sha256']
archives=[B/'anadius_core_partial.a',B/'hook_engine_compat.a'];commands=[]
for p in archives:assert sha(p)==next(x['sha256'] for x in record['archives'] if x['file']==p.name)
def run(args,name):
 p=subprocess.run([str(compiler),*map(str,args)],capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=120)
 log=T/name;log.write_text(p.stdout+p.stderr,encoding='utf-8');commands.append({'program':str(compiler),'arguments':list(map(str,args)),'exit_code':p.returncode,'log':log.relative_to(O).as_posix(),'log_sha256':sha(log)});return p
entry=T/'entry.o';leaf=T/'export_leaf.o'
assert run(['-std=c++20','-O2','-Wall','-Wextra','-Werror','-c',D/'entry.cpp','-o',entry],'entry-compile.log').returncode==0
assert run(['-c',D/'export_leaf.S','-o',leaf],'leaf-compile.log').returncode==0
dll=T/'anadius64.reconstruction.dll';link_map=T/'anadius64.reconstruction.map'
args=['-shared','-static',entry,leaf,D/'anadius64.def','-Wl,--whole-archive',*archives,'-Wl,--no-whole-archive','-lkernel32','-luser32','-lshell32','-lws2_32','-lmswsock','-Wl,--exclude-all-symbols,--no-insert-timestamp,--verbose','-Wl,-Map,'+str(link_map),'-o',dll]
p=run(args,'link.log');output=p.stdout+p.stderr;undefined=sorted(set(re.findall(r'(?m)^.*undefined symbol: (.+)$',output)))
expected=['require_loader_attach_runtime','require_loader_detach_quiescence'];known=p.returncode!=0 and len(undefined)==2 and all(any(name in u for u in undefined) for name in expected) and not dll.exists()
result={'schema':'fresh-full-archive-dll-link.v1','checked_at':datetime.now(timezone.utc).isoformat(),'diagnostic_validation':'PASS_EXPECTED_BLOCKER' if known else 'FAIL_UNEXPECTED_LINK_RESULT','production_dll_readiness':'BLOCKED' if p.returncode else 'LINKED_NOT_RUNTIME_VALIDATED','dll_link_attempted':True,'dll_link_succeeded':p.returncode==0,'link_exit_code':p.returncode,'dll_created':dll.exists(),'dll_loaded_or_executed':False,'originals_accessed':False,'whole_archive':True,'selected_archives':[{'path':x.relative_to(O).as_posix(),'sha256':sha(x)} for x in archives],'selection_scope':'All288 anadius core objects and all5 selected hook compatibility objects; separate3preloader objects are outside anadius target','frozen_source_files_verified':len(snapshot['sources']),'common_build_record_sha256':sha(S/'build-record.json'),'source_snapshot_sha256':sha(S/'source-snapshot.json'),'compiler_sha256':sha(compiler),'undefined_symbols':undefined,'expected_missing_production_hooks':expected,'entry_source_sha256':sha(D/'entry.cpp'),'header_sha256':sha(D/'production_image.hpp'),'commands':commands,'run_directory':T.relative_to(O).as_posix(),'runtime_or_loader_equivalence_established':False}
(T/'validation.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(D/'current-full-link.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:result[k] for k in ['diagnostic_validation','production_dll_readiness','link_exit_code','dll_created','undefined_symbols']},indent=2))
if not known:raise SystemExit(1)
