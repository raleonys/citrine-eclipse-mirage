import hashlib,json,subprocess,sys,zipfile
from pathlib import Path
from run_profile import W,S,H,Q
sha=lambda b:hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest,source_files
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
archive=W/'deliverables/BF02_FIX28/preloader_l_BF02_FIX28_APPLICATION_SOURCE_NOT_RELEASE.zip';assert sha(archive.read_bytes())=='d06c37af8766452a6f9d422fa7db81e33cdc0c67a568ae34db07afa78772ef1d'
original=W/'fix28_inputs/dll-reverse-engineering(6).zip';assert sha(original.read_bytes())=='0734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9'
with zipfile.ZipFile(archive) as z:old={n.removeprefix('preloader_l/'):z.read(n) for n in z.namelist()}
current={p.relative_to(S).as_posix():p.read_bytes() for p in source_files()};runtime=[n for n in current if n.startswith('src/')];changed=[n for n in runtime if old.get(n)!=current[n]]
assert len(runtime)==123 and set(changed)=={'src/xml/reader.cpp','src/xml/upstream_parser.cpp'}
vendor=[n for n in current if n.startswith('third_party/')];assert len(vendor)==1060 and all(current[n]==old[n] for n in vendor)
fixtures=[n for n in old if n.startswith('tests/') and n.endswith(('.cpp','.hpp'))];assert len(fixtures)==56;changed_fixtures=[n for n in fixtures if old[n]!=current[n]];assert changed_fixtures==['tests/unit/xml/test_reader.cpp']
tools=[n for n in old if n.startswith('tools/golden/') or n.endswith(('.cmd','.ps1'))];assert all(current[n]==old[n] for n in tools)
with zipfile.ZipFile(original) as z:
 for r in json.loads(current['metadata/PACKET_XML_ORACLE_FIX29.json'])['files']:assert current[r['destination']]==z.read(r['origin']) and sha(current[r['destination']])==r['original_sha256']
 source=z.read('anadius-xml-reader-source/reader.cpp').decode();adapted=source.replace('"reader.hpp"','"src/xml/reader.hpp"').replace('"vendor/tinyxml2/tinyxml2.h"','"third_party/tinyxml2/tinyxml2.h"').replace('namespace anadius::xml_reader_bridge {','namespace preloader_l::xml_reader {').replace('anadius::xml_reader_bridge','preloader_l::xml_reader_bridge').replace('messages_recovered','messages').replace('xml_recovered','xml')
 restored=current['src/xml/reader.cpp'].decode().replace('    std::string_view error_name()const override{\n        return document_.ErrorStr();\n    }','    std::string_view error_name()const override{return document_.ErrorStr();}')
 assert restored==adapted
roles=json.loads(current['metadata/TEST_ORACLE_ROLES_FIX29.json'])['files'];assert len(roles)==61 and all(sha(current[r['file']])==r['sha256'] for r in roles)
graphs={}
for profile in ['production','debug']:
 commands=json.loads((Q/profile/'compile_commands.json').read_text());names=[r['file'] for r in commands]
 assert sum(Path(n).name=='upstream_parser.cpp' for n in names)==1 and not any(Path(n).name=='tinyxml2.cpp' for n in names)
 symbols=subprocess.check_output(['nm','-C',str(Q/profile/'libpreloader_l_core.a')],stderr=subprocess.STDOUT).decode()
 assert symbols.count(' T tinyxml2::XMLDocument::PushDepth()')==1 and 'preloader_l_packet_tinyxml2' not in symbols
 graphs[profile]={'sole_upstream_implementation_TU':True,'raw_vendor_cpp_direct_compilations':0,'packet_parser_symbols_in_core':False,'PushDepth_definitions':1}
receipt={'status':'PASS','source_digest':frozen,'parent_archive_sha256':sha(archive.read_bytes()),'original_archive_sha256':sha(original.read_bytes()),'runtime_files':123,'unchanged_existing_runtime':121,'changed_existing_runtime':['src/xml/reader.cpp'],'new_runtime':['src/xml/upstream_parser.cpp'],'vendor_files_unchanged':1060,'prior_Cpp_fixture_files':56,'unchanged_prior_fixtures':55,'changed_fixture_expectation':'XML adapter-only diagnostic replaced with full source ErrorStr ID18 and line1; boundary and code assertions retained','test_roles':61,'golden_and_launcher_files_unchanged':len(tools),'restored_reader_algorithm_equals_packet_after_documented_include_namespace_alias_and_accessor_formatting':True,'actual_host_build_graph':graphs,'files_changed_or_added':sorted(n for n in current if old.get(n)!=current[n]),'native_execution':'NOT_RUN'}
(H/'INTEGRITY.json').write_text(json.dumps(receipt,indent=2)+'\n');print('PASS vendor1060/other runtime121/prior fixtures55; source reader restored; sole upstream TU in each core')
