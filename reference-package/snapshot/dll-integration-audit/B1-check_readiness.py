"""B1 diagnostic gate. Never loads a DLL, runs target code, or substitutes an entry."""
from pathlib import Path
import hashlib,json,re,subprocess,sys

HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1];OUT=ROOT/'outputs';WORK=ROOT/'work'
def read(path):return json.loads(path.read_text(encoding='utf-8-sig'))
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def require(value,message):
    if not value:raise RuntimeError(message)
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

build_path=OUT/'source-workspace/build-record.json';build=read(build_path)
require(build['result']=='PASS','Last authoritative build must pass')
snapshot=read(OUT/'source-workspace/source-snapshot.json')
require(snapshot['build_record_sha256']==sha(build_path),'Snapshot/build provenance')
for item in snapshot['sources']:require(sha(OUT/item['path'])==item['sha256'],'Changed prior source:'+item['path'])
for item in build['archives']:require(sha(OUT/'source-workspace/build'/item['file'])==item['sha256'],'Archive integrity:'+item['file'])
symbol_path=OUT/'reconstruction-inventory/object-symbols.txt';probe=read(OUT/'reconstruction-inventory/current-object-probe.json')
require(probe['immutable_reference_hashes']['common_build_record']==sha(build_path) and probe['immutable_reference_hashes']['compiled_symbol_listing']==sha(symbol_path),'Object symbol provenance')
symbols=symbol_path.read_text(encoding='utf-8-sig')
roots=['anadius::crt_entry_recovered::Entry::entry_04bcfc(',
       'anadius::crt_entry_recovered::Entry::dispatch_04bbc8(',
       'anadius::recovered::Bootstrap::application_dll_main(',
       'anadius::recovered::Bootstrap::initialize(',
       'anadius::bootstrap_native_recovered::application_entry(',
       'anadius::config_startup::BootstrapConfiguration::load_configuration_01c870(']
for symbol in roots:require(symbol in symbols,'Missing compiled source root:'+symbol)
entries=read(HERE/'entry-roots.json');providers=read(HERE/'provider-blockers.json')
require(entries['result']=='PASS' and providers['result']=='PASS_BOUNDED_AUDIT','Independent audit status')
require(entries['entry_address']=='0x18004bcfc' and entries['export_count']==3,'Anadius PE roots')
require(providers['closed_source_providers_count']==52,'Prior provider count')

headers=['anadius-config-startup-source/startup.hpp','anadius-bootstrap-native-source/native_entries.hpp',
         'anadius-crt-startup-bridge-source/startup_bridge.hpp','anadius-crt-glue-source/crt_glue.hpp',
         'anadius-socket-source/node_string.hpp','anadius-socket-source/native_message.hpp']
source='\n'.join('#include "'+(OUT/h).as_posix()+'"' for h in headers)+r'''
#include <type_traits>
using Base=anadius::recovered::BootstrapDependencies;
struct Mixed:anadius::config_startup::BootstrapConfiguration,
             anadius::bootstrap_native_recovered::ThreadBridge {};
extern "C" {
extern const unsigned int dll_readiness_flags[]={
    std::is_abstract_v<anadius::config_startup::BootstrapConfiguration>,
    std::is_abstract_v<anadius::bootstrap_native_recovered::ThreadBridge>,
    std::is_abstract_v<anadius::crt_startup_bridge::StartupBridge>,
    std::is_abstract_v<anadius::socket_recovered::RecoveredNodeStringEffects>,
    std::is_abstract_v<anadius::socket_recovered::RecoveredNativeMessageApi>,
    std::is_abstract_v<anadius::crt_glue::ThreadEffects>,
    std::is_convertible_v<Mixed*,Base*>,
    std::is_convertible_v<decltype(&anadius::bootstrap_native_recovered::application_entry),
                          anadius::bootstrap_native_recovered::ApplicationEntry>
};
}
'''
compiler=ROOT/'work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin/clang++.exe'
require(sha(compiler)==build['compiler_sha256'],'Use the already-validated compiler')
command=[str(compiler),'-std=c++20','-Wall','-Wextra','-Werror','-S','-emit-llvm','-x','c++','-','-o','-']
positive=subprocess.run(command,input=source,capture_output=True,text=True,encoding='utf-8')
(WORK/'dll-entry-type-probe.ll').write_text(positive.stdout,encoding='utf-8')
require(positive.returncode==0,'Type observation compile failed:'+positive.stderr)
match=re.search(r'@dll_readiness_flags\s*=.*?\[8 x i32\]\s*\[([^\]]+)\]',positive.stdout)
require(match,'Compiler emitted explicit constant trait observations')
values=[int(v) for v in re.findall(r'i32 (\d+)',match[1])]
names=['bootstrap_configuration_abstract','thread_bridge_abstract','crt_startup_bridge_abstract',
       'node_string_effects_abstract','message_dispatch_bridge_abstract','thread_effects_abstract',
       'mixed_bridge_convertible_to_single_base','native_application_entry_signature_convertible']
require(len(values)==len(names),'Eight compiler observations')
require(values[:4]==[1,1,1,1] and values[5:]==[0,0,1],'Source/ABI observations match bounded audit')
negative=source+'\nvoid consume_one_base(Base*);\nvoid invalid_combination(Mixed* p){consume_one_base(p);}\n'
negative_command=[str(compiler),'-std=c++20','-Wall','-Wextra','-Werror','-fsyntax-only','-x','c++','-']
failure=subprocess.run(negative_command,input=negative,capture_output=True,text=True,encoding='utf-8')
(WORK/'dll-entry-ambiguous-base-diagnostic.txt').write_text(failure.stderr,encoding='utf-8')
require(failure.returncode!=0 and 'ambiguous' in failure.stderr.lower() and 'BootstrapDependencies' in failure.stderr,'Expected two-base ambiguity, not an unrelated compile failure')
record={
 'schema':'dll-integration.readiness.v1','diagnostic_validation':'PASS','production_dll_readiness':'BLOCKED',
 'source_stage': 'N29','source_files_checked':len(snapshot['sources']),'changed_prior_sources':[],
 'native_objects':build['native_objects'],'normal_test_groups':build['mock_test_suites'],
 'full_common_build_repeated':False,'test_executable_run':False,'dll_link_performed':False,'dll_created':False,
 'reason':'No production per-image runtime owner and complete concrete BootstrapDependencies/CRT/application binding graph are identified. Partial bridge symbols being archived is insufficient.',
 'compiled_source_roots':roots,'compiler_observations':dict(zip(names,[bool(x) for x in values])),
 'positive_probe':{'command':command,'stdin_source':source,'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'exit_code':positive.returncode,'llvm_ir_file':'work/dll-entry-type-probe.ll','llvm_ir_sha256':sha(WORK/'dll-entry-type-probe.ll')},
 'negative_probe':{'command':negative_command,'exit_code':failure.returncode,'expected_failure':'Ambiguous BootstrapDependencies base conversion','diagnostic_file':'work/dll-entry-ambiguous-base-diagnostic.txt','diagnostic_sha256':sha(WORK/'dll-entry-ambiguous-base-diagnostic.txt')},
 'references':{'build_record_sha256':sha(build_path),'source_snapshot_sha256':sha(OUT/'source-workspace/source-snapshot.json'),'object_symbols_sha256':sha(symbol_path),'compiler_sha256':sha(compiler),'entry_audit_sha256':sha(HERE/'entry-roots.json'),'provider_audit_sha256':sha(HERE/'provider-blockers.json'),'checker_sha256':sha(Path(__file__))},
 'new_native_bodies':0,'new_source_bridges':0,'new_typed_closures':0,
 'original_read_performed':False,'original_execution':False,'original_modification':False,
 'next_work':'B2: one runtime owner and explicit concrete BootstrapDependencies facade; reuse configuration and existing CRT thread source, then establish a production entry strategy.',
 'limits':['Compiler traits and source/object presence do not prove native ABI/EH or DLL runtime equivalence.','No empty DllMain, unreferenced archive or success stub is used as a completed DLL.','This checker records the last authoritative source build; new audit/plan documents are a separate work package.','Blocked version input and historical generated DLL are never read or retried.']}
write(HERE/'readiness.json',record)
print(json.dumps({k:record[k] for k in ('diagnostic_validation','production_dll_readiness','source_files_checked','native_objects','compiler_observations','new_native_bodies','next_work')},ensure_ascii=False))
if '--require-ready' in sys.argv:raise SystemExit(2)
