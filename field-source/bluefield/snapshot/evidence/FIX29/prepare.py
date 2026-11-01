from pathlib import Path
import collections,csv,difflib,hashlib,json,sys,zipfile
W=Path(__file__).resolve().parents[1];R=W/'fix29_work/preloader_l';H=W/'fix29_helpers';P=zipfile.ZipFile(W/'deliverables/BF02_FIX28/preloader_l_BF02_FIX28_APPLICATION_SOURCE_NOT_RELEASE.zip');O=zipfile.ZipFile(W/'fix28_inputs/dll-reverse-engineering(6).zip')
parent={n.removeprefix('preloader_l/'):P.read(n) for n in P.namelist()};sha=lambda b:hashlib.sha256(b).hexdigest()
def save(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n')
def load(n):return json.loads(parent[n])
changes={}
for p in sorted((R/'src').rglob('*')):
 if p.is_file():
  n=p.relative_to(R).as_posix()
  if p.read_bytes()!=parent.get(n):changes[n]={'before_sha256':sha(parent[n]) if n in parent else None,'current_sha256':sha(p.read_bytes()),'authority_ids':['B29-R1']}
assert set(changes)=={'src/xml/reader.cpp','src/xml/upstream_parser.cpp'}
scope={'classification':'NON_RELEASE_WORK','user_instruction':'devam edelim','parent_archive':Path(P.filename).name,'parent_archive_sha256':sha(Path(P.filename).read_bytes()),'original_RE_archive_sha256':sha(Path(O.filename).read_bytes()),'accepted_scope':'D15 restore parse-time XML depth=100 and original full diagnostic/error precedence while preserving current pinned upstream fixes and byte-identical vendor. Compile current upstream implementation once through an explicitly recorded namespace-lookup adapter; remove post-success visitor. Test-only literal packet compatibility parser supplies live reference.','parent_runtime_files':{n:sha(b) for n,b in parent.items() if n.startswith('src/')},'runtime_changes':changes,'dependency_manifest_sha256':sha(parent['metadata/THIRD_PARTY.json']),'authority_records':{'B29-R1':{'classification':'RE_SOURCE_BEHAVIOR_REPAIR_WITH_UPSTREAM_RETAINED','prior_difference':'FIX08 kept pristine upstream but implemented source depth=100 only after successful parsing. This was an integration choice, not required by upstream or a user request to change error precedence. The earlier broad upstream-update request does not authorize that behavior drift.','repair':'Use standard namespace lookup inside the sole upstream implementation TU to select the source depth=100 in XMLDocument::PushDepth. Public header constant500 and all vendor files remain unchanged. No private member access, preprocessor private/public trick, generated fork or alternate production parser.','preserved':'All other upstream fixes/behavior, app lifetime, serializers, CFG/profile/defaults, logging/exit/atomic persistence, Golden/export policy.','limit':'Pinned source adapter must be reviewed when PushDepth lookup changes. Not original DLL/native ABI equivalence.','native_execution':'NOT_RUN'}},'changed_prior_fixture':{'file':'tests/unit/xml/test_reader.cpp','reason':'Replace invented adapter-diagnostic assertion with stronger source ErrorStr ID/line assertion; boundary/code assertions retained. Actual source oracle establishes expected behavior, not current app output.'},'new_policies':[],'vendor_changed':False,'official_publication':False}
save(R/'metadata/CHANGE_AUTHORITY_FIX29.json',scope)
oracle={'archive_sha256':scope['original_RE_archive_sha256'],'role':'TEST_ONLY_PACKET_SELECTED_COMPATIBILITY_SOURCE_NOT_RECOVERED_ORIGINAL_DLL','files':[],'adaptation':'Archived tinyxml2.cpp/h/license bytes copied exactly. Only a separate owned test TU privately renames namespace and global depth symbol; archived files themselves unchanged. Source header already has documented depth=100 alteration.','limit':'Source packet selected tinyxml2 11.0.0 compatibility parser, not proof of original binary release. Current upstream security/entity fixes are retained; differential domain is ordinary structural XML/depth/error precedence.'}
for name in ['tinyxml2.cpp','tinyxml2.h','LICENSE.txt']:
 origin='anadius-xml-reader-source/vendor/tinyxml2/'+name;dest='tests/support/xml/packet/'+name;raw=(R/dest).read_bytes();assert raw==O.read(origin)
 oracle['files'].append({'origin':origin,'destination':dest,'original_sha256':sha(raw),'current_sha256':sha(raw),'production_linked':False})
save(R/'metadata/PACKET_XML_ORACLE_FIX29.json',oracle)
imports=load('metadata/IMPORTS.json')
for row in imports['files']:
 current=sha((R/row['destination']).read_bytes())
 if current!=row['current_sha256']:row['current_sha256']=current;row['adaptation']+=' FIX29 removes post-parse depth visitor, restores parse-time source depth/ErrorStr via separate upstream compile adapter; source-based XML test strengthens error diagnostic expectation.'
for row in oracle['files']:imports['files'].append({**row,'role':'TEST_ONLY_PACKET_COMPATIBILITY_ORACLE','adaptation':oracle['adaptation']})
save(R/'metadata/IMPORTS.json',imports)
g=load('metadata/BUILD_GRAPH.json');g['core']=[('src/xml/upstream_parser.cpp' if n=='third_party/tinyxml2/tinyxml2.cpp' else n) for n in g['core']];test='tests/integration/test_xml_depth_source_oracle.cpp';g['integration_tests'].append(test);g['test_support_sources'][test]=['tests/support/xml/packet_oracle.cpp'];g['included_implementation_sources']={'src/xml/upstream_parser.cpp':['third_party/tinyxml2/tinyxml2.cpp'],'tests/support/xml/packet_oracle.cpp':['tests/support/xml/packet/tinyxml2.cpp']};save(R/'metadata/BUILD_GRAPH.json',g)
p=R/'cmake/sources.cmake';p.write_text(parent['cmake/sources.cmake'].decode().replace('"third_party/tinyxml2/tinyxml2.cpp"','"src/xml/upstream_parser.cpp"'))
p=R/'CMakeLists.txt';s=parent['CMakeLists.txt'].decode().replace('foreach(source IN ITEMS tests/integration/test_config2_source_oracle.cpp','foreach(source IN ITEMS '+test+' tests/integration/test_config2_source_oracle.cpp');s=s.replace('    target_link_libraries(${name} PRIVATE preloader_l_core)','    if(name STREQUAL "test_xml_depth_source_oracle")\n      target_sources(${name} PRIVATE tests/support/xml/packet_oracle.cpp)\n    endif()\n    target_link_libraries(${name} PRIVATE preloader_l_core)');assert s.count('target_sources(${name} PRIVATE tests/support/xml/packet_oracle.cpp)')==1;p.write_text(s)
roles=load('metadata/TEST_ORACLE_ROLES_FIX28.json');byfile={r['file']:r for r in roles['files']};byimport={r['destination']:r for r in imports['files']};rows=[]
for p in sorted((R/'tests').rglob('*')):
 if not p.is_file() or p.suffix not in ['.cpp','.hpp','.h']:continue
 n=p.relative_to(R).as_posix();row=dict(byfile.get(n,{'file':n,'role':'OWNED_LIVE_XML_DIFFERENTIAL_DRIVER_OR_ADAPTER','origin':'','original_sha256':'','limit':'Test-only XML comparison support, not native/DLL proof.'}));row['sha256']=sha(p.read_bytes());row['byte_identical_to_FIX28']=n in parent and p.read_bytes()==parent[n]
 if n.startswith('tests/support/xml/packet/'):
  imp=byimport[n];row.update(role='EXACT_PACKET_SELECTED_UPSTREAM_COMPATIBILITY_SOURCE',origin=imp['origin'],original_sha256=imp['original_sha256'],limit=oracle['limit'])
 if n=='tests/unit/xml/test_reader.cpp':row['limit']='Source-based expectations strengthened for parse-time ErrorStr; previous adapter-only string expectation corrected with live source reference.'
 rows.append(row)
roles['files']=rows;roles['scope']='Conservative C++/header test roles including literal packet XML compatibility dependency; not semantic approval by metadata.';save(R/'metadata/TEST_ORACLE_ROLES_FIX29.json',roles)
with (R/'docs/TEST_ORACLE_ROLES_FIX29.csv').open('w',newline='') as f:
 fields=sorted({k for row in rows for k in row});w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
a=load('metadata/RE_SOURCE_DEVIATIONS_FIX28.json');a['schema']='preloader_l.RE_source_deviations_FIX29.v1';a['scope']=scope['accepted_scope'];a['FIX29_parent_zip_sha256']=scope['parent_archive_sha256'];a['FIX29_runtime_changes']=changes
for f in a['findings']:
 if f['id']=='D15':
  f['historical_FIX28_finding']={k:f[k] for k in ['status','difference','impact','authority','recommended_next_action']};f.update(status='CORRECTED_SOURCE_PARSE_TIME_DEPTH_UPSTREAM_RETAINED',difference='Source depth=100 is enforced by active XMLDocument::PushDepth during parsing. Post-success visitor and invented depth diagnostic removed. Pinned upstream bytes/API and other upstream fixes remain unchanged; source compile adapter is explicitly owned and bound.',impact='Malformed/deep XML stops at source depth branch before later syntax failure; full native TinyXML2 ErrorStr and parse cleanup restored. Current dependency version is not rolled back.',authority='User requested repair of unsupported RE-source drift while retaining deliberate upstream updates. Prior post-success approximation was own integration choice, not required by dependency.',recommended_next_action='Keep sole implementation-TU binding, depth lookup source/hash guards, exact packet compatibility oracle and malformed/boundary differential tests. Review adapter at future explicit upstream update. Native Windows/ABI remains NOT_RUN.')
  f['FIX29_resolution']=f['difference'];f['causal_accountability']={'status':'OWN_INTEGRATION_APPROXIMATION_CORRECTED','evidence':'Previous reader.cpp explicitly moved source depth validation to post-success visitor to keep vendor pristine. This motive is source-documented but is not user authorization for changed error precedence.','required_by_upstream':False}
  f['current']=[{'file':'src/xml/upstream_parser.cpp','locator':'constexpr int TINYXML2_MAX_ELEMENT_DEPTH=100;'}, {'file':'src/xml/reader.cpp','locator':'int parse('}, {'file':test,'locator':'Concrete earlier-parse-error'}]
 for x in f['current']:
  raw=(R/x['file']).read_bytes();hits=[i+1 for i,l in enumerate(raw.decode().splitlines()) if x['locator'] in l] if x['locator']!='<file>' else [1];assert hits,(f['id'],x);x.update(line=hits[0],sha256=sha(raw))
for row in a['sources']:
 row['current_sha256']=sha((R/row['current_file']).read_bytes());row['application_changed_FIX29']=row['current_file'] in changes;row['FIX29_authority_ids']=['B29-R1'] if row['application_changed_FIX29'] else []
a['sources'].append({'current_file':'src/xml/upstream_parser.cpp','current_sha256':sha((R/'src/xml/upstream_parser.cpp').read_bytes()),'comparison':'OWNED_UPSTREAM_BUILD_ADAPTER_SOURCE_DEPTH_RESTORED','original':'anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h','original_sha256':sha(O.read('anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h')),'finding_ids':['DEP03','D15'],'assessment':'New explicit compile owner; sole pristine upstream implementation TU; namespace lookup selects source parse depth100. Not a recovered source file.','application_changed_FIX29':True,'FIX29_authority_ids':['B29-R1']})
a['source_file_count']=len(a['sources']);assert a['source_file_count']==123;a['status_counts']=dict(collections.Counter(f['status'] for f in a['findings']));opens=[f['id'] for f in a['findings'] if f['status'].startswith(('REVIEW','PARTIAL'))];assert len(opens)==7
disposition={'before_open':8,'after_open':7,'closed_bounded_records':['D15'],'corrections':['Parse-time source XML depth100 instead of post-success visitor','Source error precedence/full ErrorStr instead of invented adapter diagnostic'],'reviewed_retained':['Pinned current upstream fixes and public header constant500; depth configured inside implementation TU only'],'remaining_open':opens,'not_claimed':['Full XML binary equivalence','Native Windows/MSVC execution','Other upstream behavior rolled back'],'native_validation':'NOT_RUN_separate_dimension'};a['FIX29_closure_accounting']=disposition;save(R/'metadata/RE_SOURCE_DEVIATIONS_FIX29.json',a);save(R/'metadata/AUDIT_DISPOSITIONS_FIX29.json',disposition)
native=load('metadata/NATIVE_VALIDATION_FIX28.json');native['changed_surface']='XML parse-time depth/error precedence and sole upstream implementation compile TU. Actual Windows/MSVC/DLL build execution NOT_RUN.';save(R/'metadata/NATIVE_VALIDATION_FIX29.json',native)
for name,fields,records in [('RE_FINDINGS_FIX29.csv',['id','status','priority','title','FIX29_resolution','recommended_next_action'],a['findings']),('RE_SOURCE_CROSSWALK_FIX29.csv',['current_file','current_sha256','comparison','original','finding_ids','FIX29_authority_ids'],a['sources'])]:
 with (R/'docs'/name).open('w',newline='') as f:
  w=csv.DictWriter(f,fieldnames=fields);w.writeheader()
  for row in records:w.writerow({k:json.dumps(row.get(k,''),ensure_ascii=False) if isinstance(row.get(k),list) else row.get(k,'') for k in fields})
p=R/'scripts/qa/check_source.py';s=parent['scripts/qa/check_source.py'].decode()
for old,new in [('CHANGE_AUTHORITY_FIX28','CHANGE_AUTHORITY_FIX29'),('GOVERNANCE_BINDING_FIX28','GOVERNANCE_BINDING_FIX29'),('FILESYSTEM_CALLS_FIX28','FILESYSTEM_CALLS_FIX29'),('TEST_ORACLE_ROLES_FIX28','TEST_ORACLE_ROLES_FIX29'),('recorded FIX28','recorded FIX29'),('change in FIX28','change in FIX29'),('FIX28 scope','FIX29 scope')]:s=s.replace(old,new)
s=s.replace("|{'third_party/tinyxml2/tinyxml2.cpp'}",'')
i=s.index(' # 3: loader entry');extra=''' # D15: parse-time source depth via the sole pinned upstream implementation TU.
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
''';p.write_text(s[:i]+extra+s[i:])
p=R/'tests/tools/test_source_policy.py';s=parent['tests/tools/test_source_policy.py'].decode();anchor="  assert not check(root),'restored baseline'";s=s.replace(anchor,"  modify('src/xml/upstream_parser.cpp',lambda b:b.replace(b'TINYXML2_MAX_ELEMENT_DEPTH=100;',b'TINYXML2_MAX_ELEMENT_DEPTH=500;'),'source parse depth removed')\n  modify('cmake/sources.cmake',lambda b:b.replace(b'src/xml/upstream_parser.cpp',b'third_party/tinyxml2/tinyxml2.cpp'),'source depth compile adapter bypassed')\n  modify('tests/support/xml/packet/tinyxml2.h',lambda b:b+b'\\n// mutation\\n','packet XML reference tamper')\n"+anchor);p.write_text(s)
p=R/'scripts/qa/package_source.py';p.write_text(parent['scripts/qa/package_source.py'].decode().replace('BF02_FIX28_APPLICATION_SOURCE','BF02_FIX29_APPLICATION_SOURCE'))
d=load('metadata/PROJECT.json');d.update(code_state='BF02_FIX29_SOURCE_XML_PARSE_DEPTH_RESTORED',snapshot_parent={'archive':scope['parent_archive'],'sha256':scope['parent_archive_sha256'],'classification':'FULL_SOURCE_SNAPSHOT_PARENT'},change_reason=scope['accepted_scope']);d['governance']['binding']='metadata/GOVERNANCE_BINDING_FIX29.json';save(R/'metadata/PROJECT.json',d)
sys.path.insert(0,str(R/'scripts/qa'));from filesystem_inventory import scan_calls
save(R/'metadata/FILESYSTEM_CALLS_FIX29.json',{'scope':'FIX29 removes reader post-success policy; no new application filesystem calls; upstream implementation is now included by the owned compile TU.','calls':scan_calls(R)})
save(R/'metadata/ACCEPTANCE.json',{'classification':'NON_RELEASE_WORK','status':'PENDING_FIX29_FINAL_QA','official_publication':False})
governance='''# BLUEFIELD için geçerli governance — FIX29

En yeni kullanıcı talimatı üstündür. Library master MD/JSON version9 revision2026-10-25.1 bu tur yeniden okundu; metadata/GOVERNANCE_BINDING_FIX29.json hashleri bağlar. Bu belge normatif alt kümedir, tam master mirror veya tarihsel release/debt kaydı değildir.

NON_RELEASE BLUEFIELD kod atası kullanıcının tam RE paketidir. preloader_l/SURGE, export ordinal, optional CFG, profil/avatar, production log kapatma, failure-atomic ve owned shutdown talepleri korunur. Diğer FIELD uygulamaları/canonical pointer değiştirilmez. Yerel build/test yayın izni istemez; resmî yayın ayrıca yetkilendirilir.

FIX29 kapsamı D15 parse-time XML depth100/error precedence düzeltmesidir. Vendor pinleri ve bytesı aynı kalır; current TinyXML2 implementation yalnız src/xml/upstream_parser.cpp içinden bir kez derlenir. Standart namespace lookup, public header500 sabitini değiştirmeden implementation PushDepth içinde source100 değerini seçer. Post-parse visitor/uydurulmuş diagnostic kaldırılır. Yeni parser/fork/private-field hack eklenmez. RE compatibility XML kaynağı yalnız test referansıdır; current upstream security/entity değişimleri geri alınmaz.

Metadata/test kendi kendine davranış yetkisi oluşturmaz. Eski XML testindeki invented adapter diagnostic beklentisi canlı source karşılaştırmasıyla daha güçlü full ErrorStr beklentisine çevrilir; diğer önceki fixturelar korunur. Source/Q02/G0/Golden/CMD sözleşmeleri ve dürüst Windows/MSVC/game NOT_RUN ayrımı korunur.
''';(R/'docs/GOVERNANCE_APPLICABLE.md').write_text(governance)
binding=load('metadata/GOVERNANCE_BINDING_FIX28.json');binding.update(status='CURRENT_LIBRARY_VERSION9_RECHECKED_FOR_FIX29',scope=scope['accepted_scope']);binding['packaged_projection']['md_sha256']=sha((R/'docs/GOVERNANCE_APPLICABLE.md').read_bytes());save(R/'metadata/GOVERNANCE_BINDING_FIX29.json',binding)
(R/'AGENTS.md').write_text('''# preloader_l — çalışma talimatı

Bu rehber davranış yetkisi üretmez. En yeni kullanıcı talimatı ve güncel Library master üstündür; docs/GOVERNANCE_APPLICABLE.md / metadata/GOVERNANCE_BINDING_FIX29.json uygulanabilir alt kümeyi bağlar.

- BLUEFIELD NON_RELEASE, kod atası tam kullanıcı RE paketidir. BROWNFIELD QA/tool disiplini korunur; uygulama C++ otomatik alınmaz. Diğer FIELD projeleri/resmî release/canonical pointer değiştirilmez.
- preloader_l/SURGE adı, exports DllMain@1 / SURGE@2 / DllShutdown@3, optional surge.cfg, profil/avatar, production disk log kapatma, failure-atomic ve owned shutdown korunur. Reddedilmiş quota/cache/sidecar/INI-delay kararları eklenmez.
- FIX29 D15 source parse-time XML depth100 ve error precedence düzeltmesidir. Runtime scope reader.cpp + yeni upstream_parser.cpp. Pinned vendor1060 dosyası değişmez. Tek upstream implementation TU derleme bağlayıcısından geçer; test-only packet XML kaynağı productiona girmez. Public upstream header500 sabiti ve upstream security/entity fixleri korunur.
- Source farkı otomatik revert yetkisi değildir. Kullanıcı talebi/upstream/own integration ayrı yazılır. Kayıtsız tarihsel gerekçeler uydurulmaz. Dependencies yalnız açık kullanıcı tetiklemesiyle güncellenir.
- XML testindeki adapter-only diagnostic yerine source ErrorStr ID/line beklentisi güçlendirilir. Diğer mevcut C++ fixture bytesı korunur. Native Windows/MSVC/DLL/game ve actual DLL Golden burada NOT_RUN; portable PASS onların yerine geçmez.
- Yeni source/test graph ve import/current hashleri, test roles ve includes bindingine kaydedilir. Standard include-TU adapterının namespace lookupi her explicit dependency updateinde incelenir. Vendor fork/generated copy/private member hack eklenmez.
- Yerel build/test için yayın onayı gerekmez. Golden zinciri, launcher sözleşmesi, /we4702, lisanslar ve exact ZIP/hash replay korunur. Beklentiler PASS almak için gevşetilmez.
''')
(R/'README.md').write_text('''# preloader_l — BLUEFIELD FIX29

NON_RELEASE tam C++ kaynak/build/QA paketi. Exact FIX28 ebeveyni; karşılaştırma kullanıcının RE kaynak paketidir, original DLL binary eşdeğerliği değildir.

FIX29: XML derinlik100 denetimi successful parse sonrasındaki visitor yerine gerçek parse aşamasına taşındı. Derin/hatalı XML artık source error precedence ve full ErrorStr üretir. Yeni ayar/limit icat edilmedi:100 kaynak paketindeki değerdir. Vendor dosyaları ve upstream güncellemeleri korunur; explicit compile adapter tek implementation TUyu bağlar.

Windows operator entrypoints scripts/build_orchestration/ altında, generated output build/ altındadır. Production/Debug/Native/CRT/Diagnostic profilleri ve Golden zinciri korunur. preloader_l.dll için MSVC x64 kullanılır. Bu ortamda Windows binary/game kabulü NOT_RUN; detay docs/RE_SOURCE_DEVIATIONS_FIX29.md ve final QA raporundadır.
''')
(R/'docs/ARCHITECTURE.md').write_text('''# Güncel mimari — FIX29

Kod atası tam RE paketi; bağımlılıklar exact pinned upstreamdir. Application/Service owned lifetime, Asio IOCP Windows backend, iki handler workerı, per-session queue/cipher düzeni ve atomik persistence korunur. Kaynak farklarının tam123 dosyalık eşlemesi metadata/RE_SOURCE_DEVIATIONS_FIX29.json içindedir;38 kaydın7 ana incelemesi açıktır.

XML reader artık source gibi doğrudan XMLDocument::Parse/Error/ErrorStr kullanır. src/xml/upstream_parser.cpp public TinyXML2 headerını include eder; tinyxml2 namespaceindeki TU-local depth100 sabiti, aynı TUya include edilen upstream cpp içindeki unqualified PushDepth lookupini seçer. Global public header500 sabiti başka TUlarda farklı tanımlanmaz; ODR çatışması veya private layout erişimi yoktur. Upstream cpp doğrudan ikinci kez derlenmez. Bu compile-time entegrasyon bilinçli bir uygulama adapterıdır; untouched upstreamin varsayılan500 davranışı diye sunulmaz.

Current upstream security/entity/destructor/printer fixleri aynen korunur. Packet selected compatibility h/cpp/license bytesı ayrı test-only TU içinde private namespace/depth-symbol renaming ile karşılaştırılır. İki production parser yolu yoktur. D15 source davranış kaydı kapanır; Windows/MSVC/ABI/game kabulü farklı ve NOT_RUNdır.
''')
(R/'docs/DEVELOPMENT.md').write_text('''# Geliştirme — FIX29

Kapsam D15 XML parse-time depth100/error precedence repair. Current upstream bytesı/pinleri değişmez; source limitinin compile adapterındaki namespace lookupi hash/source/build graph guards ile bağlanır. Bağımlılık açık kullanıcı kararıyla güncellendiğinde PushDepthin unqualified name lookup sözleşmesi yeniden incelenir. Yeni vendor fork veya private-member access kullanılmaz.

Production source değişimi reader.cpp ve yeni upstream_parser.cpp; önceki122 dosyanın121i aynıdır. Eski56 C++ test/support dosyasından yalnız XML reader testinin adapter diagnostic beklentisi kaynak ErrorStr ID/line kontrolüne güçlendirilir. Diğer55 bytesı aynıdır. Test-only packet compatibility parser özgün DLLden kurtarılmış tam parser diye etiketlenmez. Profil/CFG/Asio/thread/exit/logging/persistence davranışı değişmez.
''')
(R/'docs/VALIDATION.md').write_text('''# Doğrulama — FIX29

50 CTest grubu:47 C++ executable ve3 Python Golden/tool grubu. Yeni canlı XML oracle4284 örnekte17138 kontrol yapar; returned code/error flag/full ErrorStr/visible element-attribute-order-nullable-text tree birlikte karşılaştırılır. Açık/selfclosing depth sınırı, CRLF line coordinates, malformed attributes/comment/CDATA/declarations/mismatch, multiple roots ve C-string NUL domaini vardır. Numeric Unicode upstream-security farklarının geri alındığı veya bütün parser input uzayının aynı olduğu iddia edilmez.

Parent visitor counterexample ve depth100→500 / compile adapter bypass negatifleri davranış testlerini gerçekten düşürmelidir.74 source policy,6 ZIP,8 synthetic PE negatifleri kayıtlıdır. Exact ZIP ayrı source/object build ve full CTest replay ile bağlanır. Focused ASan/UBSan instrument edilen TUlarda raporlanır; full-core/LSan değildir.

Windows/MSVC DLL build, native hooks/CRT/IOCP/provider/game ve actual DLL Golden NOT_RUN. Bu durum build/test/publication için kullanıcıdan oyun kabulü isteme gerekçesi değildir; source closure ile ayrı boyuttur.
''')
p=R/'docs/RELEASE_PROCESS.md';p.write_text(parent['docs/RELEASE_PROCESS.md'].decode().replace('FIX28','FIX29'))
p=R/'docs/THIRD_PARTY.md';p.write_text(parent['docs/THIRD_PARTY.md'].decode()+'\nFIX29 dependency update değildir; tüm vendor bytes/pinleri aynı. TinyXML2 implementation yalnız src/xml/upstream_parser.cpp içinden bir kez derlenir. Public global depth500 sabiti korunur; implementation namespace lookup source100 limitini parse sırasında seçer. Bu adapter kendi entegrasyon kodumuzdur ve gelecekteki explicit upstream updateinde review zorunludur. Packet XML h/cpp/license kopyası yalnız test referansıdır, zlib notice korunur.\n')
# Preserve previous operational architecture and invocation instructions.
p=R/'README.md';new=p.read_text();old=parent['README.md'].decode();i=old.index('FIX28:');j=old.index('\n\n',i);intro=new.split('\n\n')[2];p.write_text((old[:i]+intro+old[j:]).replace('BLUEFIELD FIX28','BLUEFIELD FIX29').replace('GOVERNANCE_BINDING_FIX28','GOVERNANCE_BINDING_FIX29'))
p=R/'docs/ARCHITECTURE.md';addition=p.read_text().split('\n\n',2)[-1];old=parent['docs/ARCHITECTURE.md'].decode().replace('# Güncel mimari — FIX28','# Güncel mimari — FIX29').replace('RE_FINDINGS_FIX28.csv','RE_FINDINGS_FIX29.csv').replace('diğer8 açık','diğer7 açık').replace('Toplam8 inceleme','Toplam7 inceleme').replace('8 kaynak incelemesi','7 kaynak incelemesi');p.write_text(old+'\n## FIX29 XML derleme ve parse sahipliği\n\n'+addition)
p=R/'docs/DEVELOPMENT.md';new=p.read_text();old=parent['docs/DEVELOPMENT.md'].decode();p.write_text(new+'\n## Korunan FIX28 Config2 ve fixed-event test sözleşmesi\n\n'+old.split('\n\n',2)[-1])
report='''# FIX29 — XML parse-time derinlik ve hata önceliği düzeltmesi

Karşılaştırma hedefi tam kullanıcı RE kaynak paketidir; özgün DLL runtime eşdeğerliği değildir. NON_RELEASE, exact FIX28 parent. D15 düzeltildi; açık ana inceleme8→7.

## Neden sapmıştı?

RE paketinin seçtiği compatibility TinyXML2 headerında depth100, parser PushDepth içinde uygulanır. FIX08de pristine güncel upstreami korumak için depth500 ile parse edilip yalnız başarılı document sonrasında visitor ile100 denetlenmişti. Bu, önceki entegrasyon seçimimizdi; upstream güncellemesinin zorunlu kıldığı bir davranış değildi. Kaynakta bu teknik amaç açık olsa da kullanıcı100 kontrolünün zamanını/hata davranışını değiştirmemizi istemedi. Sonradan bir kullanıcı onayı veya bağımlılık zorunluluğu varmış gibi sunulmuyor.

Geçerli belgelerin limit testleri bu farkı saklıyordu.99 açık R elementi içinde <bad a=>, source parserda18 depth error üretirken parent7 attribute error üretebiliyor. Parent ayrıca source ErrorStr yerine invented adapter diagnostic dönüyordu. Daha ilerideki syntax/allocation çalışması da gereksizce yapılabiliyordu.

## Ne değişti?

Readerın post-success visitorı, source_depth_error flagi ve özel diagnostic stringi kaldırıldı. Reader yeniden doğrudan Parse/Error/ErrorStr kullanır. Current pinned upstream cpp, src/xml/upstream_parser.cpp içinden tam bir kez derlenir. Bu TUda tinyxml2 namespaceindeki constexpr100, PushDepthin standart unqualified name lookupiyle source limitini seçer. Public global header500 sabiti değişmez. Hiçbir vendor dosyası değiştirilmedi; generated vendor fork/private-field hack yoktur. Bu adapter açıkça kendi build entegrasyonumuz olarak kaydedildi; upstreamin varsayılan davranışı diye gösterilmiyor. Gelecek explicit dependency updateinde lookup/body guard yeniden incelenmelidir.

Dependency pinleri ve current upstreamin Unicode/entity/destructor/printer/security fixleri geri alınmadı. Header/cpp referansı, paket içindeki seçilmiş compatibility dependencydir; recovered original DLL sourceu veya binary sürüm kanıtı değildir.

## Kanıt

Test-only packet h/cpp/license arşivle byte-identicaldır. Ayrı TUda namespace/depth symbol private rename yapılır; production graphına girmez. Canlı karşılaştırma4284 örnekte17138 kontrol: return code/error flag/full diagnostic/tree; derinlik98/99 sınırı, selfclose, bozuk attribute/comment/CDATA/mismatch, CRLF line, multi-root ve NUL davranışı. Input alanı sonludur; upstreamin farklı numeric entity güvenlik davranışlarını kaynak adına geri almaz.

Eski56 C++ fixture/headerın55i aynı; XML testinin invented adapter string assertionı, source ErrorStr ID18/line expectationına güçlendirildi. PASS almak için kaynak beklenen sonucu değiştirilmedi; yeni bağımsız dependency reference bekleneni üretir. Runtime122 parent dosyasının121i aynı; yalnız reader.cpp değişti ve tek yeni compile TU eklendi. Vendor1060, tüm CMD/PS1/Golden araçları ve diğer runtime politikaları aynı.

Full Debug/Production/clean ZIP CTest, negatifler ve focused sanitizer sonuçları teslimdeki final QAya bağlanır. Windows/MSVC/DLL/native/game/actual DLL Golden, full-core sanitizer ve LSan NOT_RUN. Diğer açık kayıtlar DEP05, D02, D06, D07, D13, D14, D21; D15 kapanışı bunları kapatmaz.
''';(R/'docs/RE_SOURCE_DEVIATIONS_FIX29.md').write_text(report)
own=load('metadata/DOCUMENTATION_OWNERSHIP.json');own['master_binding']='metadata/GOVERNANCE_BINDING_FIX29.json'
for row in own['documents']:row['sha256']=sha((R/row['path']).read_bytes())
save(R/'metadata/DOCUMENTATION_OWNERSHIP.json',own)
names=[*changes,'tests/unit/xml/test_reader.cpp',test,'tests/support/xml/packet_oracle.cpp','tests/support/xml/packet_oracle.hpp','CMakeLists.txt','cmake/sources.cmake','metadata/BUILD_GRAPH.json','scripts/qa/check_source.py','tests/tools/test_source_policy.py'];patch=[]
for n in names:patch.extend(difflib.unified_diff(parent.get(n,b'').decode().splitlines(True),(R/n).read_text().splitlines(True),fromfile='FIX28/'+n,tofile='FIX29/'+n))
(R/'docs/SOURCE_CHANGES_FIX29.patch').write_text(''.join(patch));print('Prepared FIX29; runtime files123, test roles',len(rows),'open',len(opens))
