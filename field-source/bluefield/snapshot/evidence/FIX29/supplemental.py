import hashlib,json,os,subprocess,sys,time,zipfile
from pathlib import Path
from run_profile import W,S,H,Q
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
out=Q/'supplemental';out.mkdir(parents=True,exist_ok=True)
def run(cmd,label,reject=False,env=None):
 start=time.monotonic();r=subprocess.run(list(map(str,cmd)),cwd=S,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
 (H/(label+'.log')).write_bytes(r.stdout)
 assert (r.returncode==1 and b'XML source mismatch' in r.stdout) if reject else r.returncode==0,(label,r.returncode,r.stdout[:1000])
 return {'label':label,'exit_code':r.returncode,'seconds':round(time.monotonic()-start,3),'output':r.stdout.decode(errors='replace')[:16000],'log':label+'.log'}
base=['g++','-std=c++20','-O1','-g','-pthread','-I'+str(S)];core=Q/'production/libpreloader_l_core.a';parent=W/'fix28_qa/production/libpreloader_l_core.a';assert core.exists() and parent.exists()
reader=S/'src/xml/reader.cpp';wrapper=S/'src/xml/upstream_parser.cpp';oracle=S/'tests/support/xml/packet_oracle.cpp';driver=S/'tests/integration/test_xml_depth_source_oracle.cpp'
results={'status':'PASS','source_digest':frozen,'negative_controls':[],'sanitizers':[]}
exe=out/'parent_xml';run(base+[oracle,driver,parent,'-o',exe],'parent_compile');exe.chmod(0o755);results['negative_controls'].append(run([exe],'parent_rejection',True))
probe=out/'malformed_precedence.cpp';probe.write_text('#include "src/xml/reader.hpp"\n#include <iostream>\n#include <string>\nint main(){preloader_l::xml_reader::Backend b;std::string s;for(int i=0;i<99;++i)s+="<R>";s+="<bad a=>";for(int i=0;i<99;++i)s+="</R>";auto r=b.parse_c_string(s);std::cout<<r.returned_code<<" "<<r.document->error_name()<<"\\n";return r.returned_code==7?0:1;}\n');exe=out/'parent_malformed_precedence';run(base+[probe,parent,'-o',exe],'parent_precedence_compile');exe.chmod(0o755);results['parent_malformed_precedence']=run([exe],'parent_malformed_precedence')
p=out/'depth500.cpp';p.write_text(wrapper.read_text().replace('TINYXML2_MAX_ELEMENT_DEPTH=100;','TINYXML2_MAX_ELEMENT_DEPTH=500;'))
for label,implementation in [('depth500',p),('adapter_bypass',S/'third_party/tinyxml2/tinyxml2.cpp')]:
 exe=out/label;run(base+[reader,implementation,oracle,driver,core,'-o',exe],label+'_compile');exe.chmod(0o755);results['negative_controls'].append(run([exe],label+'_rejection',True))
exe=out/'xml_focused_sanitizer';inputs=[reader,wrapper,oracle,driver]
run(base+['-fsanitize=address,undefined','-fno-omit-frame-pointer']+inputs+[core,'-o',exe],'xml_focused_sanitizer_compile');exe.chmod(0o755)
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
r=run([exe],'xml_focused_sanitizer',env=env);r['instrumented_translation_units']=[str(p.relative_to(S)) for p in inputs];r['included_implementations_instrumented']=['third_party/tinyxml2/tinyxml2.cpp','tests/support/xml/packet/tinyxml2.cpp'];r['uninstrumented_archive']=str(core.relative_to(W));r['full_core']='NOT_RUN';r['LSan']='NOT_RUN';results['sanitizers'].append(r)
assert source_digest()==frozen
(H/'SUPPLEMENTAL.json').write_text(json.dumps(results,indent=2)+'\n');print('PASS parent+2 mutants rejected; parse precedence7 captured; focused ASan/UBSan PASS',flush=True)
