import hashlib,json,sys,zipfile
from pathlib import Path
from run_profile import W,S,H
sha=lambda b:hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest,source_files
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
archive=W/'deliverables/BF02_FIX29/preloader_l_BF02_FIX29_APPLICATION_SOURCE_NOT_RELEASE.zip';assert sha(archive.read_bytes())=='15e628a403337c8b55c9c57481f993187316878c58d34c99e9f5c982736fbef4'
original=W/'fix28_inputs/dll-reverse-engineering(6).zip';assert sha(original.read_bytes())=='0734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9'
with zipfile.ZipFile(archive) as z:old={n.removeprefix('preloader_l/'):z.read(n) for n in z.namelist()}
current={p.relative_to(S).as_posix():p.read_bytes() for p in source_files()};src=[n for n in current if n.startswith('src/')];assert len(src)==123
changed=[n for n in src if old[n]!=current[n]];assert changed==['src/files/atomic_io.cpp']
assert current[changed[0]].replace(b'throw std::system_error(std::make_error_code(std::errc::file_exists),"atomic temporary names exhausted");',b'throw std::runtime_error("atomic temporary quota");')==old[changed[0]]
vendor=[n for n in current if n.startswith('third_party/')];assert len(vendor)==1060 and all(current[n]==old[n] for n in vendor)
fixtures=[n for n in old if n.startswith('tests/') and n.endswith(('.cpp','.hpp','.h'))];assert len(fixtures)==61 and [n for n in fixtures if old[n]!=current[n]]==['tests/integration/test_ticket_output.cpp']
tools=[n for n in old if n.startswith('tools/golden/') or n.endswith(('.cmd','.ps1'))];assert all(current[n]==old[n] for n in tools)
graph=json.loads(current['metadata/BUILD_GRAPH.json']);prior=json.loads(old['metadata/BUILD_GRAPH.json']);assert all(graph[k]==prior[k] for k in ['core','windows','windows_core','vendor_c','unit_tests','tool_tests','included_implementation_sources']) and current['cmake/sources.cmake']==old['cmake/sources.cmake']
with zipfile.ZipFile(original) as z:
 for r in json.loads(current['metadata/PACKET_BINARY_IO_ORACLE_FIX30.json'])['files']:
  raw=current[r['destination']];assert sha(raw)==r['current_sha256']
  rev=raw.replace(b'preloader_l::test_packet_binary_io',b'anadius::file_recovered').replace(b'"src/configuration/state.hpp"',b'"../anadius-config-source/configuration.hpp"').replace(b'"tests/support/files/packet_binary_io.hpp"',b'"binary_io.hpp"');assert rev==z.read(r['origin']) and sha(rev)==r['original_sha256']
roles=json.loads(current['metadata/TEST_ORACLE_ROLES_FIX30.json'])['files'];assert len(roles)==64 and all(sha(current[r['file']])==r['sha256'] for r in roles)
report={'status':'PASS','source_digest':frozen,'parent_archive_sha256':sha(archive.read_bytes()),'original_archive_sha256':sha(original.read_bytes()),'runtime_files':123,'changed_runtime':changed,'single_throw_change_exact_reversal':True,'other_runtime_unchanged':122,'vendor_files_unchanged':1060,'prior_Cpp_header_fixtures':61,'unchanged_prior_fixtures':60,'changed_fixture':'ticket collision branch now source false/dialog matrix; other failure assertions preserved','test_roles':64,'production_graph_unchanged':True,'golden_and_launchers_unchanged':len(tools),'literal_packet_binary_caller_exact_reversal':True,'main_reviews_before':7,'main_reviews_after':7,'resolved_subfinding':'D14/D21_INITIAL_STAGE_NAME_EXHAUSTION_ERROR_CLASS','files_changed_or_added':sorted(n for n in current if old.get(n)!=current[n]),'native_execution':'NOT_RUN'}
(H/'INTEGRITY.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS exactly1 throw; runtime122/vendor1060/fixtures60/production graph/Golden unchanged')
