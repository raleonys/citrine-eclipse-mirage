import hashlib,json,sys,zipfile
from pathlib import Path
from run_profile import W,S,H,Q
sha=lambda b:hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest,source_files
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
archive=W/'deliverables/BF02_FIX30/preloader_l_BF02_FIX30_APPLICATION_SOURCE_NOT_RELEASE.zip';assert sha(archive.read_bytes())=='106d67c12e7a2fa82e907f4520afaa1bf28ccd92d60d33cdf9d35381cabc6095'
with zipfile.ZipFile(archive) as z:old={n.removeprefix('preloader_l/'):z.read(n) for n in z.namelist() if not n.endswith('/')}
current={p.relative_to(S).as_posix():p.read_bytes() for p in source_files()};src=[n for n in current if n.startswith('src/')];assert len(src)==123
changed=[n for n in src if old[n]!=current[n]];assert changed==['src/windows/unload_guard.cpp']
guard=b'  // Refuse a region exceeding the remaining scan budget; never infer it safe.\n  if(limit-cursor>64ULL*1024*1024-scanned)return false;\n'
assert current[changed[0]].replace(guard,b'')==old[changed[0]]
vendor=[n for n in current if n.startswith('third_party/')];assert len(vendor)==1060 and all(current[n]==old[n] for n in vendor)
fixtures=[n for n in old if n.startswith('tests/') and n.endswith(('.cpp','.hpp','.h'))];assert len(fixtures)==64 and all(old[n]==current[n] for n in fixtures)
tools=[n for n in old if n.startswith('tools/golden/') or n.endswith(('.cmd','.ps1'))];assert all(current[n]==old[n] for n in tools)
graph=json.loads(current['metadata/BUILD_GRAPH.json']);prior=json.loads(old['metadata/BUILD_GRAPH.json']);assert all(graph[k]==prior[k] for k in ['core','windows','windows_core','vendor_c','unit_tests','tool_tests','test_support_sources'])
assert current['cmake/sources.cmake']==old['cmake/sources.cmake']
roles=json.loads(current['metadata/TEST_ORACLE_ROLES_FIX31.json'])['files'];assert len(roles)==67 and all(sha(current[r['file']])==r['sha256'] for r in roles)
for profile in ['production','debug']:
 commands=json.loads((Q/profile/'compile_commands.json').read_text());uses=[r for r in commands if 'tests/support/unload/winshim' in r['command']]
 assert len(uses)==1 and uses[0]['file'].endswith('/tests/integration/test_unload_scan.cpp')
 for r in commands:
  observed='-D_FORTIFY_SOURCE=0' in r['command'] or '-fno-builtin-memcpy' in r['command']
  assert observed==r['file'].endswith('/tests/integration/test_unload_scan.cpp'),r['file']
report={'status':'PASS','source_digest':frozen,'parent_archive_sha256':sha(archive.read_bytes()),'runtime_files':123,'changed_runtime':changed,'exact_REVIEW31_guard_only':True,'other_runtime_unchanged':122,'vendor_files_unchanged':1060,'all64_prior_Cpp_headers_unchanged':True,'test_roles':67,'production_source_graph_unchanged':True,'golden_and_launchers_unchanged':len(tools),'test_shim_and_observation_flags_confined_to_one_test_TU':True,'source_main_open':7,'carried_REVIEW31_DEP05_closure':True,'main_reviews_before':6,'main_reviews_after':6,'resolved_subfinding':'D13/D21_OVER_BUDGET_REGION_READ_BEFORE_REFUSAL','native_execution':'NOT_RUN','files_changed_or_added':sorted(n for n in current if old.get(n)!=current[n])}
(H/'INTEGRITY.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS one exact guard;122 runtime/1060vendor/64fixtures/Golden/production graph unchanged; test-only shim confined',flush=True)
