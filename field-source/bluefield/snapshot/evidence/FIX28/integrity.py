import hashlib,json,sys,zipfile
from pathlib import Path
W=Path(__file__).resolve().parents[1];S=W/'fix28_work/preloader_l';H=W/'fix28_helpers'
def digest(b):return hashlib.sha256(b).hexdigest()
def file_sha(p):return digest(p.read_bytes())
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest,source_files
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
parent=W/'fix28_inputs/preloader_l_BF02_FIX27_APPLICATION_SOURCE_NOT_RELEASE.zip'
original=W/'fix28_inputs/dll-reverse-engineering(6).zip'
assert file_sha(parent)=='ce59c8b61a052aa12f2ed248339c864114a91a84b3fdbb1db96e46e02b295b4e'
assert file_sha(original)=='0734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9'
with zipfile.ZipFile(parent) as z:
 old={n.removeprefix('preloader_l/'):z.read(n) for n in z.namelist() if n.startswith('preloader_l/') and not n.endswith('/')}
current={p.relative_to(S).as_posix():p.read_bytes() for p in source_files()}
auth=json.loads((S/'metadata/CHANGE_AUTHORITY_FIX28.json').read_text())
src=sorted(n for n in current if n.startswith('src/'));changed=[n for n in src if old.get(n)!=current[n]]
assert len(src)==122 and changed==sorted(auth['runtime_changes']) and len(changed)==3
assert {n:digest(old[n]) for n in src}==auth['parent_runtime_files']
vendor=[n for n in current if n.startswith('third_party/')];assert len(vendor)==1060 and all(current[n]==old[n] for n in vendor)
fixtures=[n for n in old if n.startswith('tests/') and n.endswith(('.cpp','.hpp'))];assert len(fixtures)==52 and all(old[n]==current[n] for n in fixtures)
tools=[n for n in old if n.startswith('tools/golden/') or n.endswith(('.cmd','.ps1'))];assert all(current[n]==old[n] for n in tools)
oldgraph=json.loads(old['metadata/BUILD_GRAPH.json']);graph=json.loads(current['metadata/BUILD_GRAPH.json'])
assert all(oldgraph[k]==graph[k] for k in ['core','windows','windows_core','vendor_c','unit_tests','tool_tests'])
binding=json.loads(current['metadata/PACKET_PARSER_ORACLE_FIX28.json'])
with zipfile.ZipFile(original) as z:
 for f in binding['files']:
  raw=z.read(f['origin']);new=current[f['destination']]
  assert digest(raw)==f['original_sha256'] and digest(new)==f['current_sha256']
  reversed=new.replace(b'preloader_l::test_packet_parser',b'anadius::parser')
  reversed=reversed.replace(b'"src/configuration/state.hpp"',b'"../anadius-config-source/configuration.hpp"').replace(b'"tests/support/configuration/packet_parser.hpp"',b'"parser.hpp"')
  assert reversed==raw
 roles=json.loads(current['metadata/TEST_ORACLE_ROLES_FIX28.json'])['files'];assert len(roles)==56
 for r in roles:
  assert digest(current[r['file']])==r['sha256']
  if r['origin']:assert digest(z.read(r['origin']))==r['original_sha256']
report={'status':'PASS','source_digest':frozen,'parent_archive_sha256':file_sha(parent),'original_archive_sha256':file_sha(original),'runtime_files':122,'changed_runtime':changed,'unchanged_runtime':119,'vendor_files_unchanged':1060,'prior_Cpp_test_files_unchanged':52,'current_test_roles':56,'golden_and_launcher_files_unchanged':len(tools),'production_graph_unchanged':True,'packet_parser_exact_reversible_source_binding':True,'files_changed_or_added':sorted(n for n in current if old.get(n)!=current[n]),'limit':'Byte/provenance/graph verification, not Windows execution or original DLL equivalence'}
(H/'INTEGRITY.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS runtime119/vendor1060/fixtures52/Golden unchanged; literal RE oracle exact reversal')
