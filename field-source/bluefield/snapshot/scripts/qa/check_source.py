# SPDX-License-Identifier: GPL-3.0-only
"""preloader_l source invariants, split into eight explicitly scoped areas."""
import argparse,hashlib,json,re,sys,zlib
from pathlib import Path
from _project import ROOT,source_files,sha,source_digest,save
def check(root=ROOT,shard=None):
 issues=[]
 def fail(area,reason):
  if shard is None or area==shard:issues.append(reason)
 graph=json.loads((root/'metadata/BUILD_GRAPH.json').read_text())
 # 0: owned repository/production dependency graph
 expected=set(graph['core']+graph['windows']+graph['windows_core']);actual={p.relative_to(root).as_posix() for p in (root/'src').rglob('*.cpp')}
 if actual!=expected:fail(0,'production graph differs from owned source inventory')
 for name in expected:
  if not (root/name).is_file():fail(0,'missing production source: '+name)
 if any('tests/' in n for n in expected):fail(0,'test source in production graph')
 for name in ['reference','recovered','unintegrated']:
  if any(p.is_dir() and p.name==name for p in root.rglob('*')):fail(0,'research dump directory in application: '+name)
 # FIX31 scope is bound to the retained exact parent, not inferred from PASS.
 # This checks file scope/hashes, not the semantic validity of authorization.
 try:
  scope=json.loads((root/'metadata/CHANGE_AUTHORITY_FIX31.json').read_text())
  before=scope['parent_runtime_files'];allowed=scope['runtime_changes']
  current={p.relative_to(root).as_posix():sha(p) for p in (root/'src').rglob('*') if p.is_file()}
  changed={n for n in set(before)|set(current) if before.get(n)!=current.get(n)}
  if changed!=set(allowed):fail(0,'runtime changes exceed recorded FIX31 scope')
  for name,row in allowed.items():
   if current.get(name)!=row['current_sha256']:fail(0,'FIX31 scope hash mismatch: '+name)
  if sha(root/'metadata/THIRD_PARTY.json')!=scope['dependency_manifest_sha256']:fail(0,'unrequested dependency metadata change in FIX31')
  binding=json.loads((root/'metadata/GOVERNANCE_BINDING_FIX31.json').read_text())
  projection=binding['packaged_projection']
  for kind in ['json','md']:
   if sha(root/projection[kind])!=projection[kind+'_sha256']:fail(0,'governance projection hash mismatch')
  rules=json.loads((root/projection['json']).read_text())
  if rules['master_json_sha256']!=binding['master_json']['sha256'] or rules['master_md_sha256']!=binding['master_md']['sha256']:fail(0,'governance authority binding differs')
  if rules.get('master_revision')!=binding['revision'] or 'library_governance_debt' in rules:fail(0,'governance projection scope/revision differs')
  docs=json.loads((root/'metadata/DOCUMENTATION_OWNERSHIP.json').read_text())
  for row in docs['documents']:
   if sha(root/row['path'])!=row['sha256']:fail(0,'current documentation hash mismatch: '+row['path'])
  dependency_doc=(root/'docs/THIRD_PARTY.md').read_text()
  for dependency in json.loads((root/'metadata/THIRD_PARTY.json').read_text())['upstream']:
   if dependency['commit'] not in dependency_doc:fail(0,'stale dependency documentation: '+dependency['name'])
 except Exception as error:fail(0,'FIX31 scope/governance binding unavailable: '+type(error).__name__)
 # 1: current imported/adapted file identities
 imports=json.loads((root/'metadata/IMPORTS.json').read_text())
 for row in imports['files']:
  p=root/row['destination']
  if not p.is_file() or sha(p)!=row['current_sha256']:fail(1,'adoption hash mismatch: '+row['destination'])
 adopted=json.loads((root/'metadata/BROWNFIELD_QA_ADOPTION.json').read_text())
 for row in adopted['files']:
  p=root/row['destination']
  if not p.is_file() or sha(p)!=row['current_sha256']:fail(1,'HF19 QA adoption hash mismatch: '+row['destination'])
  if row['destination'].startswith('tools/golden/') and row['current_sha256']!=row['original_sha256']:fail(1,'HF19 protocol tool bytes drifted')
 for p in (root/'src').rglob('*'):
  if p.suffix not in ['.cpp','.hpp','.h']:continue
  code=p.read_text()
  if re.search(r'\b(?:namespace\s+(?:bluefield|anadius)\b|(?:bluefield|anadius)::)',code):fail(1,'wrong owned C++ namespace: '+p.relative_to(root).as_posix())
  for name in re.findall(r'^\s*#include\s*"([^\"]+)"',p.read_text(),re.M):
   if not (root/name).is_file() and not (p.parent/name).is_file():fail(1,'broken include: '+name)
 # 2: public ABI and build/CRT policy
 definition=(root/'exports/preloader_l.def').read_text();exports=dict(re.findall(r'^\s*(\w+)\s+@(\d+)\s*$',definition,re.M))
 if exports!={'DllMain':'1','SURGE':'2','DllShutdown':'3'}:fail(2,'public export contract')
 production=(root/'exports/production.def').read_text()
 if dict(re.findall(r'^\s*(\w+)\s+@(\d+)\s*$',production,re.M))!=exports or 'LIBRARY preloader.unsigned.dll' not in production:fail(2,'production Golden export contract')
 cmake=(root/'CMakeLists.txt').read_text()
 for term in ['MultiThreadedDebug','MultiThreaded','/GS','/guard:cf','preloader_l SHARED']:
  if term not in cmake:fail(2,'missing build/CRT policy: '+term)
 for term in ['resources/golden_version.rc exports/production.def','/DEBUG:NONE','/LTCG:OFF','/NOCOFFGRPINFO','/MANIFEST:NO','/SUBSYSTEM:WINDOWS,6.00']:
  if term not in cmake:fail(2,'missing Golden production policy: '+term)
 if 'third_party/asio/include' not in cmake or 'ASIO_STANDALONE=1' not in cmake:fail(2,'missing pinned Asio build dependency')
 if 'third_party/pegtl/include' not in cmake:fail(2,'missing pinned PEGTL build dependency')
 parser=(root/'src/configuration/parser.cpp').read_text()
 if '<tao/pegtl.hpp>' not in parser or 'peg::parse<vdf::Grammar,Action,Control>' not in parser:fail(2,'missing active PEGTL Config2 parser')
 corpus=json.loads((root/'metadata/CONFIG2_CORPUS_FIX10.json').read_text())
 if sha(root/'tests/fixtures/config2_fix09_corpus.tsv')!=corpus['corpus_sha256'] or sha(root/'tests/support/configuration/parser_snapshot.hpp')!=corpus['snapshot_format_sha256']:fail(2,'frozen Config2 differential corpus drift')
 if 'third_party/filesystem/include' not in cmake:fail(2,'missing pinned gulrak build dependency')
 sibling=(root/'src/paths/module_relative.cpp').read_text()
 for term in ['<ghc/filesystem.hpp>','GHC_WIN_DISABLE_AUTO_PREFIXES','ghc::filesystem::path','candidate.native()!=expected.native()']:
  if term not in sibling:fail(2,'missing module sibling contract: '+term)
 if 'RuntimePaths::from_system(' not in (root/'src/windows/dll_entry.cpp').read_text():fail(2,'missing native sibling binding')
 if 'env.paths.override_config()' not in (root/'src/application/application.cpp').read_text():fail(2,'missing override sibling binding')
 transport=(root/'src/transport/service.cpp').read_text()
 if 'defined(ASIO_HAS_IOCP)' not in transport or 'defined(ASIO_DISABLE_IOCP)' not in transport or '#error Windows production transport requires' not in transport:fail(2,'missing Windows IOCP-only transport contract')
 dispatch_boundary=transport.split('io_thread=std::thread',1)[-1].split('started=true;',1)[0]
 for term in ['LogEvent::dispatch_failed','listener->close(ignored);io->stop();','stopping.store(true);ready.store(false);']:
  if term not in dispatch_boundary:fail(2,'missing terminal Asio dispatch contract: '+term)
 # The required crypt hook must reach a real imported target, resolver and SDK
 # trampoline; declaring a name or classification helper alone is insufficient.
 if 'preloader_l_hook_engine crypt32' not in cmake:fail(2,'missing CRYPT32 link dependency')
 native=(root/'src/windows/hooks.cpp').read_text()
 if re.search(r'ExitProcess\([^;]+;\s*std::abort\(',native):fail(2,'unreachable abort after nonreturning ExitProcess')
 if '/we4702' not in cmake:fail(2,'MSVC unreachable-code warning regression is not enforced')
 callbacks=(root/'src/windows/hook_callbacks.cpp').read_text()
 if '&::CryptQueryObject' not in native or 'host::Callback::crypt_query_001264' not in native:fail(2,'missing live crypt hook target/install')
 if native.count('MH_ApplyQueued()')!=1 or 'MH_QueueEnableHook(' not in native or 'MH_EnableHook(' in native:fail(2,'required single queued hook activation contract')
 if 'case Callback::crypt_query_001264:' not in callbacks or 'decltype(&::CryptQueryObject)' not in callbacks:fail(2,'missing crypt resolver/SDK ABI contract')
 # Source-selected registry waits must reach the shared phase predicate while
 # callback-owned locks are paused. Native lease remains outside that scope.
 if 'callback_lock::Guard lock(lease.context->serialization' not in callbacks:fail(2,'missing tracked callback serialization frame')
 registry_wait=native.split('void wait_036880(',1)[-1].split('void wait_steps_036880',1)[0]
 for term in ['callback_lock::Pause','service_.wait_steps(count)','SetLastError(incoming)']:
  if term not in registry_wait:fail(2,'missing source registry wait binding: '+term)
 if 'service_.port()' in registry_wait:fail(2,'port-only registry wait regression')
 # The active Application uses the shared configuration owner, retaining its
 # profile/router construction boundary before environment publication.
 application=(root/'src/application/application.cpp').read_text()
 startup=application.split(' void initialize(){',1)[-1].split(' void initialize_online(){',1)[0]
 for term in ['configuration::apply_configuration_layers(state,configuration_effects,passes)','configuration::publish_configuration_environment(state,configuration_effects)']:
  if term not in startup:fail(2,'missing shared configuration owner: '+term)
 for term in ['state.user_directory=','storage.update_01eb00(state)','publish_environment("ContentId"']:
  if term in startup:fail(2,'duplicated configuration orchestration: '+term)
 if startup.find('configuration::publish_configuration_environment(')<startup.find('router=std::make_unique<dispatch::Router>'):fail(2,'environment publication moved before Application router')
 # D10: one explicit source Windows owner, with separate POSIX projection.
 resolver=(root/'src/application/providers.cpp').read_text().split('Path ConfigurationEffects::resolve_path_0192d0',1)[-1].split('configuration::Node ConfigurationEffects::load_document',1)[0]
 if 'return PathEffects::resolve(utf8_path(value),executable,relative);' not in resolver or 'lexically_normal' in resolver:fail(2,'configuration resolver bypasses source owner/normalizes')
 env_header=(root/'src/application/environment.hpp').read_text()
 if '#ifdef _WIN32\nusing PathEffects=paths::WindowsEffects;' not in env_header or 'using PathEffects=HostPathEffects;' not in env_header:fail(2,'Windows/host lexical owner selection differs')
 for name,counts in [('src/application/providers.hpp',{'parent':2,'filename':2,'join':3}),('src/profiles/online_session.hpp',{'parent':1,'join':1})]:
  body=(root/name).read_text()
  for method,count in counts.items():
   if body.count('return PathEffects::'+method+'(')!=count:fail(2,'lexical caller bypasses common owner: '+name+'/'+method)
 lexical=(root/'src/paths/paths.hpp').read_text().split('struct WindowsEffects',1)[-1]
 for term in ['return parent_path_019820(value);','return filename_00e120(value);','return join_016b00(left, right);','relative && !is_absolute_0195b0(value)','return join_016b00(parent_path_019820(executable), value);','return value;']:
  if term not in lexical:fail(2,'source lexical owner body missing: '+term)
 host=(root/'src/application/environment.cpp').read_text().split('Path HostPathEffects::parent',1)[-1].split('Path utf8_path',1)[0]
 if 'lexically_normal' in host:fail(2,'host resolver normalization reintroduced')
 for name in ['test_path_resolution.cpp','test_path_effects.cpp']:
  if 'tests/integration/'+name not in graph['integration_tests'] or 'tests/integration/'+name not in cmake:fail(2,'path fixture missing from build graph: '+name)
 # D11 retains the packet's null-factory guard and independent source oracle.
 online=(root/'src/profiles/online.cpp').read_text()
 if 'if (!state.object_0a3a88)\n        throw OutOfEvidenceInput("035bd0 dereferences a null create_online result");' not in online or 'provider factory rejected' in online:fail(2,'null provider guard replaced by silent completion')
 fixture=(root/'tests/unit/profiles/test_online.cpp').read_text()
 if 'CHECK(state.object==0);CHECK(state.module==41);CHECK(state.online==0);CHECK(!m.called("auth-status"));CHECK(!m.called("finish"));' not in fixture:fail(2,'original null-factory oracle weakened')
 # D03 source receive/write queue separation; worker socket operations stay on IO.
 for term in ['std::deque<std::string> requests,output;','asio::buffer(output.front())','self->output.pop_front();','void queue_response(','void compute_next()','self->fail_compute();']:
  if term not in transport:fail(2,'missing independent transport queue owner: '+term)
 completed=transport.split('if(greeting){',1)[-1].split('}catch(const PersistenceRecoveryRequired&)',1)[0]
 if 'if(self->output.empty())self->compute_next();' in completed or 'self->compute_next();' not in completed:fail(2,'compute completion waits for socket write')
 worker=transport.split('void compute(std::string message,bool greeting)',1)[-1].split('  void read(){',1)[0]
 if 'socket.close(' in worker:fail(2,'worker closes socket with overlapping async IO')
 if 'tests/integration/test_transport_pipeline.cpp' not in graph['integration_tests'] or 'tests/integration/test_transport_pipeline.cpp' not in cmake:fail(2,'pipeline test not integrated')
 oracle=json.loads((root/'metadata/PACKET_SESSION_ORACLE_FIX24.json').read_text())
 if sha(root/oracle['destination'])!=oracle['destination_sha256']:fail(2,'literal source session oracle drift')
 # D16: visible own-avatar preference does not suppress source cache effects.
 avatar=(root/'src/handlers/content.cpp').read_text().split('bool query_image_0314f0',1)[-1]
 for term in ['if(s.online_0a3c3b!=0) {','if(!own)result=cache;','if(own&&s.saved_avatar_size_0a1130<size&&effects.copy_overwrite_048d20(cache,s.last_avatar_0a3a60))','s.saved_avatar_size_0a1130=size;','effects.log_saved_size_085530(size);']:
  if term not in avatar:fail(2,'avatar cache/source effect contract missing: '+term)
 avatar_oracle=json.loads((root/'metadata/PACKET_AVATAR_ORACLE_FIX25.json').read_text())
 if sha(root/avatar_oracle['destination'])!=avatar_oracle['destination_sha256']:fail(2,'literal source avatar oracle drift')
 if 'tests/integration/test_avatar_effects.cpp' not in graph['integration_tests'] or 'tests/integration/test_avatar_effects.cpp' not in cmake:fail(2,'avatar effects test not integrated')
 # GCC/Clang nonreturn hint restates the existing virtual Effect contract.
 license=(root/'src/licensing/license.cpp').read_text()
 for term in ['[[noreturn]] void invoke_exit_effect(Effects& effects,std::int32_t code)','effects.exit_05cdc0(code);','__builtin_unreachable();','invoke_exit_effect(effects_,code);','invoke_exit_effect(effects_,-1);']:
  if term not in license:fail(2,'license compiler contract missing: '+term)
 # D14 LIC11: only initial stage setup maps to failbit, not later failures.
 native_output=(root/'src/files/native_io.cpp').read_text()
 for term in ['writer_=std::make_unique<AtomicFileWriter>(file,owner);','catch(const std::system_error&){initial_=2;}','initial_state()const override{return initial_;}','writer_->write(value);','writer_->commit();writer_.reset();']:
  if term not in native_output:fail(2,'native output open/error contract missing: '+term)
 if 'tests/integration/test_ticket_output.cpp' not in graph['integration_tests'] or 'tests/integration/test_ticket_output.cpp' not in cmake:fail(2,'ticket output test not integrated')
 # Q02 literal source reference and truthful fixed lifecycle failure labels.
 oracle=json.loads((root/'metadata/PACKET_PARSER_ORACLE_FIX28.json').read_text())
 for row in oracle['files']:
  if sha(root/row['destination'])!=row['current_sha256']:fail(2,'live source parser oracle drift')
 if graph['test_support_sources'].get('tests/integration/test_config2_source_oracle.cpp')!=['tests/support/configuration/packet_parser.cpp'] or 'target_sources(${name} PRIVATE tests/support/configuration/packet_parser.cpp)' not in cmake:fail(2,'source parser oracle test-only target binding lost')
 entry=(root/'src/windows/dll_entry.cpp').read_text()
 if entry.count('LogEvent::startup_failed')!=2 or entry.count('LogEvent::shutdown_failed')!=1:fail(2,'wrong native lifecycle failure event')
 for row in json.loads((root/'metadata/TEST_ORACLE_ROLES_FIX31.json').read_text())['files']:
  if sha(root/row['file'])!=row['sha256']:fail(2,'test role binding drift')
 # D15: parse-time source depth via the sole pinned upstream implementation TU.
 wrapper=(root/'src/xml/upstream_parser.cpp').read_text()
 for term in ['namespace tinyxml2 {','constexpr int TINYXML2_MAX_ELEMENT_DEPTH=100;','#include "third_party/tinyxml2/tinyxml2.cpp"','static_assert(::TINYXML2_MAX_ELEMENT_DEPTH==500']:
  if term not in wrapper:fail(2,'source parse depth compile adapter lost: '+term)
 if 'third_party/tinyxml2/tinyxml2.cpp' in graph['core'] or 'src/xml/upstream_parser.cpp' not in graph['core']:fail(2,'upstream implementation must compile exactly once through adapter')
 if '"src/xml/upstream_parser.cpp"' not in (root/'cmake/sources.cmake').read_text() or '"third_party/tinyxml2/tinyxml2.cpp"' in (root/'cmake/sources.cmake').read_text():fail(2,'CMake source depth adapter bypass')
 reader=(root/'src/xml/reader.cpp').read_text()
 if any(term in reader for term in ['SourceDepthPolicy','source_depth_error_','adapter diagnostic']):fail(2,'post-parse depth approximation restored')
 if 'if (_parsingDepth == TINYXML2_MAX_ELEMENT_DEPTH)' not in (root/'third_party/tinyxml2/tinyxml2.cpp').read_text():fail(2,'review namespace lookup adapter for new upstream depth body')
 xml_oracle=json.loads((root/'metadata/PACKET_XML_ORACLE_FIX29.json').read_text())
 for row in xml_oracle['files']:
  if sha(root/row['destination'])!=row['current_sha256']:fail(2,'packet XML oracle bytes drift')
 if graph['test_support_sources'].get('tests/integration/test_xml_depth_source_oracle.cpp')!=['tests/support/xml/packet_oracle.cpp'] or 'target_sources(${name} PRIVATE tests/support/xml/packet_oracle.cpp)' not in cmake:fail(2,'XML reference test-only target binding lost')
 # D14/D21 initial stage collision classification; no catch broadening.
 atomic=(root/'src/files/atomic_io.cpp').read_text()
 if 'throw std::system_error(std::make_error_code(std::errc::file_exists),"atomic temporary names exhausted");' not in atomic or 'attempt<100' not in atomic:fail(2,'initial stage collision class/budget differs')
 if 'catch(const std::exception&)' in (root/'src/files/native_io.cpp').read_text():fail(2,'initial output masks non-system failure')
 binary_oracle=json.loads((root/'metadata/PACKET_BINARY_IO_ORACLE_FIX30.json').read_text())
 for row in binary_oracle['files']:
  if sha(root/row['destination'])!=row['current_sha256']:fail(2,'packet binary caller oracle bytes drift')
 if graph['test_support_sources'].get('tests/integration/test_atomic_stage_collision.cpp')!=['tests/support/files/packet_binary_io.cpp'] or 'target_sources(${name} PRIVATE tests/support/files/packet_binary_io.cpp)' not in cmake:fail(2,'binary caller oracle test-only target lost')
 # Actual production unload body is included only by its explicit test target.
 scan=(root/'src/windows/unload_guard.cpp').read_text()
 if 'if(limit-cursor>64ULL*1024*1024-scanned)return false;' not in scan:fail(2,'whole-region scan budget refusal lost')
 if scan.find('if(limit-cursor>64ULL*1024*1024-scanned)')>scan.find('std::memcpy(&value,'):fail(2,'scan budget checked after read')
 test='tests/integration/test_unload_scan.cpp'
 if graph['included_implementation_sources'].get(test)!=['src/windows/unload_guard.cpp'] or test not in cmake:fail(2,'actual unload body test binding lost')
 for term in ['if(name STREQUAL "test_unload_scan" AND NOT WIN32)','-fno-builtin-memcpy -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0','-Wl,--wrap=memcpy']:
  if term not in cmake:fail(2,'test-only unload read observation binding lost: '+term)
 # 3: loader entry stays bounded; heavy work lives in actual native worker
 entry=(root/'src/windows/dll_entry.cpp').read_text();dllmain=entry[entry.index('extern "C" BOOL WINAPI DllMain'):]
 for term in ['WaitForSingleObject','Service(','hooks->install','read_file(']:
  if term in dllmain:fail(3,'blocking/heavy work in DllMain: '+term)
 for term in ['_beginthreadex','SetEvent','FreeLibrary','environment_lease->restore']:
  if term not in entry:fail(3,'missing native ownership path: '+term)
 # 4: concrete protocol coverage is bound to the router
 messages=(root/'src/protocol/messages.cpp').read_text();router=(root/'src/protocol/router.cpp').read_text()
 registrations=re.findall(r'\{"([A-Za-z]+)",0x([0-9a-f]+)\}',messages)
 if len(registrations)!=39 or len({n for n,_ in registrations})!=39:fail(4,'39-route registration contract')
 cases=set(re.findall(r'case 0x([0-9a-f]+):',router))
 if not {v for _,v in registrations}<=cases:fail(4,'registered route has no concrete dispatch branch')
 # 5: authorized operator entries and exact child result retention
 registry=json.loads((root/'metadata/HUMAN_ENTRYPOINTS.json').read_text());required=set()
 driver=(root/'scripts/build_orchestration/build_driver.ps1').read_text()
 if 'Clear-CompilerSelectors' not in driver or 'Open-BuildLock' not in driver or '$buildLock.Dispose()' not in driver:fail(5,'local-build selector/lock contracts missing')
 for term in ['NMake Makefiles','scripts\\qa\\run_golden.py',"$goldenReceipt.status -ne 'PASS'",'$goldenReceipt.published']:
  if term not in driver:fail(5,'missing production Golden binding: '+term)
 pipeline=(root/'scripts/qa/run_golden.py').read_text()
 if "STAGES=('URSYNEX','ARYRA','MUSTAVEX','CORVEXON','FALCION')" not in pipeline:fail(5,'Golden stage order differs')
 for name in registry['entries']:
  target=root/name;launcher=target.with_suffix('.cmd');required.add(launcher.relative_to(root).as_posix())
  if not target.is_file() or not launcher.is_file():fail(5,'missing entrypoint/launcher: '+name);continue
  text=launcher.read_text();lower=text.lower()
  if '%*' not in text or '%errorlevel%' not in lower or 'pause' not in lower or 'exit /b %rc%' not in lower:fail(5,'launcher forwarding/exit/hold: '+name)
  if target.name not in text or '%~dp0' not in text:fail(5,'launcher target path: '+name)
 actual_cmd={p.relative_to(root).as_posix() for p in root.rglob('*.cmd') if not {'build','dist'}&set(p.parts)}
 if actual_cmd!=required:fail(5,'unauthorized/missing CMD launcher')
 if any(root.rglob('*.bat')):fail(5,'BAT launcher forbidden')
 # 6: complete project license and exact vendor content
 if (root/'LICENSE').stat().st_size<30000 or 'END OF TERMS AND CONDITIONS' not in (root/'LICENSE').read_text():fail(6,'complete GPL text missing')
 vendor=json.loads((root/'metadata/THIRD_PARTY.json').read_text())
 for name,digest in vendor['file_sha256'].items():
  if not (root/name).is_file() or sha(root/name)!=digest:fail(6,'vendor pin mismatch: '+name)
 # 7: source/package hygiene, sensitive-material patterns, default PNG bytes
 for p in root.rglob('*'):
  if p.is_symlink():fail(7,'repository symlink: '+p.name)
 for p in source_files(root):
  if p.is_symlink():fail(7,'source symlink: '+p.name)
  if p.suffix.lower() in ['.dll','.exe','.o','.obj','.a','.lib','.pdb']:fail(7,'binary in source snapshot: '+p.name)
  if p.suffix in ['.cpp','.hpp','.h','.py','.json','.md','.ps1','.cmd']:
   text=p.read_text(errors='replace')
   if re.search(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----|\bsk-(?:proj-)?[A-Za-z0-9_-]{32,}',text):fail(7,'sensitive material pattern: '+p.name)
 # The migration seal binds exact user-provided PNG bytes in both locations.
 try:
  proof=json.loads((root/'metadata/AVATAR_FIX13.json').read_text())['official']
  arrays=dict(re.findall(r'unsigned char (\w+)\[\]=\{([^}]+)',(root/'src/application/avatar_assets.hpp').read_text()))
  if set(arrays)!={'surge_png'}:raise ValueError('asset inventory')
  raw=bytes(map(int,arrays['surge_png'].split(',')));offset=8
  if len(raw)!=proof['bytes'] or hashlib.sha256(raw).hexdigest()!=proof['sha256'] or raw!=(root/'resources/surge.png').read_bytes():raise ValueError('official asset pin')
  if raw[:8]!=b'\x89PNG\r\n\x1a\n' or [int.from_bytes(raw[16:20],'big'),int.from_bytes(raw[20:24],'big')]!=[208,208]:raise ValueError('PNG header/dimensions')
  while offset<len(raw):
   n=int.from_bytes(raw[offset:offset+4],'big');chunk=raw[offset+4:offset+8+n];checksum=int.from_bytes(raw[offset+8+n:offset+12+n],'big')
   if zlib.crc32(chunk)&0xffffffff!=checksum:raise ValueError('PNG CRC')
   offset+=12+n
  if offset!=len(raw):raise ValueError('PNG end')
 except Exception:fail(7,'invalid official SURGE PNG asset')
 # Active runtime names, trusted provider loading and compile-time logging policy.
 runtime=(root/'src/runtime/runtime_paths.cpp').read_text()
 for term in ['data_(local_/"SURGE")','return data_/"surge.png";','RuntimePaths::profile_cache()const{return data_;}','reparse(c)','unowned runtime write']:
  if term not in runtime:fail(7,'runtime ownership policy missing: '+term)
 provider=(root/'src/profiles/online_session.cpp').read_text()
 for term in ['filename()!=provider_filename','env_.paths.provider()','LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32']:
  if term not in provider:fail(7,'trusted provider policy missing: '+term)
 if 'provider_filename="preloader_l_online.dll"' not in (root/'src/runtime/path_kind.hpp').read_text():fail(7,'wrong active provider basename')
 log=(root/'src/application/logging.cpp').read_text()
 for term in ['#if PRELOADER_L_DISK_DIAGNOSTICS','(void)event;']:
  if term not in log:fail(7,'compile-time diagnostics missing: '+term)
 if 'void log(LogEvent)' not in (root/'src/application/environment.hpp').read_text():fail(7,'typed event-only logger missing')
 for term in ['PRELOADER_L_DISK_DIAGNOSTICS=0','PRELOADER_L_DISK_DIAGNOSTICS=1','^(DEBUG|DIAGNOSTIC)$']:
  if term not in cmake:fail(7,'profile disk diagnostics contract missing: '+term)
 if 'PRELOADER_L_TESTING_PROVIDER_ROOT' in cmake:fail(7,'provider search override reintroduced')
 for p in (root/'src').rglob('*'):
  if p.suffix not in ['.hpp','.cpp','.h']:continue
  text=p.read_text()
  if re.search(r'"(?:BLUEFIELD|anadius|LSX emu|anadius64online\.dll|bluefield\.png|default\.png|bluefield\.log)"',text):fail(7,'retired active runtime literal: '+p.relative_to(root).as_posix())
  # The imported status adapter opens metadata with FILE_READ_ATTRIBUTES;
  # its exact source hash is checked above alongside the other imports.
  if re.search(r'\b(?:ofstream|fstream)\s+\w|\b(?:fopen|CreateFile[AW]?)\s*\(',re.sub(r'//[^\n]*|/\*.*?\*/','',text,flags=re.S)) and p.relative_to(root).as_posix() not in ['src/files/atomic_io.cpp','src/application/logging.cpp','src/windows/status_calls.cpp']:
   fail(7,'unowned direct disk writer: '+p.relative_to(root).as_posix())
 # User-rejected additions must stay out of active runtime code.
 for text,label,terms in [(runtime,'runtime layout/naming',['provider-v2','session-','std::from_chars','size()>128']), (provider,'profile cache',['4096','from_chars','cp<min']), (log,'diagnostics cutoff',['limit=256','size.QuadPart>','status.st_size>']), ((root/'src/application/providers.cpp').read_text(),'avatar quotas',['downloaded_bytes_','downloads_.size()','be32('])]:
  if any(term in text for term in terms):fail(7,'rejected FIX12 rule reintroduced: '+label)
 app=(root/'src/application/application.cpp').read_text()
 if 'validate_configuration_input' in app or 'pending_license_key' in app or 'message.size()>1024*1024' in app:fail(7,'unsupported config precheck or deferred license effect')
 if 'pending.max_size()' not in transport or 'clients.size()<32' in transport:fail(7,'invented transport input/client limit')
 if 'n>64*1024*1024' in (root/'src/application/environment.cpp').read_text():fail(7,'invented general file-size limit')
 # All known direct and indirect I/O calls are enumerated; a new surface requires review.
 try:
  from filesystem_inventory import scan_calls
  recorded=json.loads((root/'metadata/FILESYSTEM_CALLS_FIX31.json').read_text())['calls']
  actual=scan_calls(root)
  if [(x['source'],x['line'],x['code_sha256']) for x in actual]!=[(x['source'],x['line'],x['code_sha256']) for x in recorded]:fail(7,'filesystem call inventory differs from source')
 except Exception:fail(7,'filesystem call inventory unavailable')

 return issues
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--shard',type=int,choices=range(8));a=ap.parse_args()
 errors=check(shard=a.shard);save(ROOT/'build/reports'/('source_'+str(a.shard)+'.json'),{'status':'PASS' if not errors else 'FAIL','scope':'source invariants only; not native acceptance','source_digest':source_digest(),'shard':a.shard,'errors':errors});print(('PASS' if not errors else 'FAIL')+' source checks'+(' '+str(a.shard) if a.shard is not None else ''))
 for e in errors:print(e)
 return int(bool(errors))
if __name__=='__main__':sys.exit(main())
