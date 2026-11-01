from pathlib import Path
import collections,csv,difflib,hashlib,json,sys,zipfile
B=Path('/workspace/scratch/2072698c927d');R=B/'fix28_work/preloader_l';H=B/'fix28_helpers';P=zipfile.ZipFile(B/'fix28_inputs/preloader_l_BF02_FIX27_APPLICATION_SOURCE_NOT_RELEASE.zip');O=zipfile.ZipFile(B/'fix28_inputs/dll-reverse-engineering(6).zip');parent={n.removeprefix('preloader_l/'):P.read(n) for n in P.namelist()};sha=lambda b:hashlib.sha256(b).hexdigest()
def save(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n')
changes={}
for p in (R/'src').rglob('*'):
 if p.is_file():
  n=p.relative_to(R).as_posix()
  if p.read_bytes()!=parent[n]:changes[n]={'before_sha256':sha(parent[n]),'current_sha256':sha(p.read_bytes()),'authority_ids':['B28-R2']}
assert set(changes)=={'src/application/logging.hpp','src/application/logging.cpp','src/windows/dll_entry.cpp'}
oracle=json.loads((H/'PACKET_PARSER_ORACLE_BINDING.json').read_text());save(R/'metadata/PACKET_PARSER_ORACLE_FIX28.json',oracle)
scope={'classification':'NON_RELEASE_WORK','user_instruction':'devam edelim','parent_archive':Path(P.filename).name,'parent_archive_sha256':sha(Path(P.filename).read_bytes()),'original_RE_archive_sha256':sha(Path(O.filename).read_bytes()),'accepted_scope':'Q02 direct live RE Config2 parser oracle and conservative classification of all current test files; correct lifecycle failure logging to fixed startup_failed/shutdown_failed events. Preserve production disk-off, payload/what/path privacy and all runtime parser/config/lifecycle/exit/vendor policies.','parent_runtime_files':{n:sha(b) for n,b in parent.items() if n.startswith('src/')},'runtime_changes':changes,'dependency_manifest_sha256':sha(parent['metadata/THIRD_PARTY.json']),'authority_records':{'B28-R1':{'classification':'QA_SOURCE_REFERENCE','repair':'Compile literal user RE parser algorithm under test-only namespace with include relocation only; compare actual source to active PEGTL including partial state and errors. Historic FIX09 corpus contributes inputs only.','limit':'Original DLL/native ABI is not the comparison target. Conservative roles do not assert that adapted fixtures are independent source oracles.'},'B28-R2':{'classification':'DIAGNOSTIC_CORRECTION_WITH_USER_PRIVACY_RETAINED','repair':'Two Windows bootstrap catches emitted startup on failure; cleanup catch emitted phase. Replace those labels with constant startup_failed/shutdown_failed. Append enum values to preserve existing ordinals.','preserved':'Failure/stop/cleanup/control flow, suppression of error.what and raw XML/tokens/paths, production diagnostics disabled.','historic_reason':'Previous mislabel cause not independently recorded; not required by any dependency.','retained_review':'Existing network_parse/network_crypto/network_failure fixed categories keep privacy without claiming original diagnostic text parity. Detailed exception payload is deliberately not restored.','native_execution':'NOT_RUN'}} ,'all_prior_Cpp_fixture_bytes_unchanged':True,'new_policies':[],'vendor_changed':False,'official_publication':False};save(R/'metadata/CHANGE_AUTHORITY_FIX28.json',scope)
imports=json.loads(parent['metadata/IMPORTS.json'])
for row in imports['files']:
 raw=(R/row['destination']).read_bytes()
 if sha(raw)!=row['current_sha256']:row['current_sha256']=sha(raw);row['adaptation']+=' FIX28 fixed lifecycle failure event labels only; production disk-off/privacy/control flow preserved.'
for row in oracle['files']:imports['files'].append({'origin':row['origin'],'original_sha256':row['original_sha256'],'destination':row['destination'],'role':'TEST_ONLY_LITERAL_RE_SOURCE_ORACLE','adaptation':row['adaptation'],'current_sha256':row['current_sha256']})
save(R/'metadata/IMPORTS.json',imports)
new_tests=['tests/integration/test_config2_source_oracle.cpp','tests/integration/test_failure_diagnostics.cpp'];g=json.loads(parent['metadata/BUILD_GRAPH.json']);g['integration_tests']+=new_tests;g['test_support_sources'][new_tests[0]]=['tests/support/configuration/packet_parser.cpp'];save(R/'metadata/BUILD_GRAPH.json',g)
p=R/'CMakeLists.txt';s=parent['CMakeLists.txt'].decode();s=s.replace('foreach(source IN ITEMS tests/integration/test_ticket_output.cpp','foreach(source IN ITEMS '+' '.join(new_tests)+' tests/integration/test_ticket_output.cpp');needle='    add_executable(${name} "${source}")\n    target_link_libraries(${name} PRIVATE preloader_l_core)';assert s.count(needle)==1;s=s.replace(needle,'    add_executable(${name} "${source}")\n    if(name STREQUAL "test_config2_source_oracle")\n      target_sources(${name} PRIVATE tests/support/configuration/packet_parser.cpp)\n    endif()\n    target_link_libraries(${name} PRIVATE preloader_l_core)');p.write_text(s)
# All roles are intentionally conservative: ancestry is not unchanged semantics.
bydest={r['destination']:r for r in imports['files']};roles=[]
for p in sorted((R/'tests').rglob('*')):
 if not p.is_file() or p.suffix not in ['.cpp','.hpp']:continue
 n=p.relative_to(R).as_posix();imp=bydest.get(n);role='OWNED_APPLICATION_TEST_OR_SUPPORT';limit='Current application/synthetic fixture; not independent source/native acceptance.'
 if imp:
  assert sha(O.read(imp['origin']))==imp['original_sha256']
  role='PACKET_ANCESTRY_WITH_ADAPTED_EXPECTATIONS';limit='Original provenance bound; app/upstream adaptations may alter expectations. Not automatically a literal oracle.'
  if imp['role']=='TEST_ONLY_LITERAL_RE_SOURCE_ORACLE':role='LITERAL_SOURCE_ORACLE_NAMESPACE_INCLUDE_ONLY';limit='Exact reversible transform of user RE C++; only test target links it; not original DLL.'
  elif imp['role']=='TEST_ONLY_SOURCE_ORACLE_EXTRACT':role='BOUND_SOURCE_FRAGMENT_WITH_DISCLOSED_ADAPTER';limit='Partial source extract; see packet oracle binding metadata and earlier adaptation record.'
 if n=='tests/unit/configuration/test_pegtl_differential.cpp':role='FROZEN_SELF_REGRESSION';limit='8766 historical expected outputs are regression evidence, not independent source proof.'
 if n=='tests/integration/test_config2_source_oracle.cpp':role='LIVE_SOURCE_DIFFERENTIAL_DRIVER';limit='Uses live literal packet parse outputs; ignores historical output column; generic state serializer shared only by test.'
 if n=='tests/integration/test_failure_diagnostics.cpp':role='OWNED_FIXED_EVENT_PRIVACY_INTEGRATION';limit='Actual host logger; Windows bootstrap catch dispatch is statically checked, not executed.'
 if n.startswith('tests/native/'):role='NATIVE_FIXTURE_NOT_RUN';limit='Presence/compilation plan does not imply native execution.'
 roles.append({'file':n,'sha256':sha(p.read_bytes()),'role':role,'origin':imp['origin'] if imp else '', 'original_sha256':imp['original_sha256'] if imp else '', 'byte_identical_to_FIX27':n in parent and p.read_bytes()==parent[n],'limit':limit})
assert len([r for r in roles if r['byte_identical_to_FIX27']])==52
save(R/'metadata/TEST_ORACLE_ROLES_FIX28.json',{'scope':'Conservative current C++ test/support roles, not semantic approval by metadata.','files':roles,'Golden':'Inherited tools unchanged; protocol/tool/synthetic link-policy tests are not actual Windows DLL acceptance.'})
with (R/'docs/TEST_ORACLE_ROLES_FIX28.csv').open('w',newline='') as f:w=csv.DictWriter(f,fieldnames=list(roles[0]));w.writeheader();w.writerows(roles)
a=json.loads(parent['metadata/RE_SOURCE_DEVIATIONS_FIX27.json']);a['schema']='preloader_l.RE_source_deviations_FIX28.v1';a['scope']='Q02 bounded source-QA/diagnostic debt disposed: live literal RE parser oracle, test-role classification, accurate fixed lifecycle failure events; privacy categories retained after explicit review.';a['FIX28_parent_zip_sha256']=scope['parent_archive_sha256'];a['FIX28_runtime_changes']=changes
for f in a['findings']:
 if f['id']=='Q02':
  f['historical_FIX27_finding']={k:f[k] for k in ['status','difference','impact','authority','recommended_next_action']};f['status']='CORRECTED_LIVE_SOURCE_ORACLE_AND_FAILURE_EVENTS_PRIVACY_RETAINED';f['FIX28_resolution']='Live independent user RE parser runs against PEGTL, including malformed/partial state; frozen outputs are not oracle authority. All test roles conservatively classified. Startup/cleanup errors use distinct nonsecret failure labels. Existing fixed network categories and payload/what suppression retained as disclosed privacy choice.';f['difference']=f['FIX28_resolution'];f['impact']='QA no longer relies solely on historical parser snapshots; lifecycle failures are not labeled startup/phase. This does not assert every fixture is independent or native/Windows diagnostic execution has passed.';f['authority']='Requested full RE-source deviation review plus existing sensitive-data logging restriction; no upstream mandates the prior frozen-only test or wrong event labels.';f['recommended_next_action']='Retain literal source hashes/target isolation, live corpus and generated/reused-state comparisons, fixed-event privacy tests and native NOT_RUN separation. New diagnostic detail requests require their own data/privacy review.'
  f['current']=[{'file':n,'locator':loc,'line':0,'sha256':''} for n,loc in [('tests/integration/test_config2_source_oracle.cpp','historical expected-output column'),('src/application/application.cpp','void log_bad_xml'),('src/windows/dll_entry.cpp','catch(const std::exception&)'),('src/application/logging.cpp','EVENT(startup_failed)')]]
 for x in f['current']:
  raw=(R/x['file']).read_bytes();hits=[i+1 for i,l in enumerate(raw.decode().splitlines()) if x['locator'] in l] if x['locator']!='<file>' else [1];assert hits,(f['id'],x);x.update(line=hits[0],sha256=sha(raw))
for row in a['sources']:row['current_sha256']=sha((R/row['current_file']).read_bytes());row['application_changed_FIX28']=row['current_file'] in changes;row['FIX28_authority_ids']=['B28-R2'] if row['application_changed_FIX28'] else [];row['assessment']='Current FIX28 source binding; live RE-source oracle and retained app/privacy decisions are distinct from native acceptance.'
a['status_counts']=dict(collections.Counter(f['status'] for f in a['findings']));opens=[f['id'] for f in a['findings'] if f['status'].startswith(('REVIEW','PARTIAL'))];assert len(opens)==8;a['FIX28_remaining_review_or_partial_records']=8;a['FIX28_closure_accounting']={'before_open':9,'after_open':8,'closed_bounded_records':['Q02'],'corrections':['Live RE parser reference instead of frozen-only authority','Wrong lifecycle failure event labels'],'reviewed_retained':['Fixed nonsecret network event categories and no raw what/payload/path restoration'],'remaining_open':opens,'not_claimed':['All fixtures are independent','Original DLL equivalence','Native Windows catch execution'],'native_validation':'NOT_RUN_separate_dimension'};save(R/'metadata/RE_SOURCE_DEVIATIONS_FIX28.json',a);save(R/'metadata/AUDIT_DISPOSITIONS_FIX28.json',a['FIX28_closure_accounting'])
native=json.loads(parent['metadata/NATIVE_VALIDATION_FIX27.json']);native['changed_surface']='Windows bootstrap startup/cleanup failure event selection. Source statically bound; actual Windows/MSVC/DLL catch execution NOT_RUN.';save(R/'metadata/NATIVE_VALIDATION_FIX28.json',native)
for filename,fields,rows in [('RE_FINDINGS_FIX28.csv',['id','status','priority','title','FIX28_resolution','recommended_next_action'],a['findings']),('RE_SOURCE_CROSSWALK_FIX28.csv',['current_file','current_sha256','comparison','original','finding_ids','FIX28_authority_ids'],a['sources'])]:
 with (R/'docs'/filename).open('w',newline='') as f:
  w=csv.DictWriter(f,fieldnames=fields);w.writeheader()
  for row in rows:w.writerow({k:json.dumps(row.get(k,''),ensure_ascii=False) if isinstance(row.get(k),list) else row.get(k,'') for k in fields})
p=R/'scripts/qa/check_source.py';s=parent['scripts/qa/check_source.py'].decode()
for old,new in [('CHANGE_AUTHORITY_FIX27','CHANGE_AUTHORITY_FIX28'),('GOVERNANCE_BINDING_FIX27','GOVERNANCE_BINDING_FIX28'),('FILESYSTEM_CALLS_FIX27','FILESYSTEM_CALLS_FIX28'),('recorded FIX27','recorded FIX28'),('change in FIX27','change in FIX28'),('FIX27 scope','FIX28 scope')]:s=s.replace(old,new)
i=s.index(' # 3: loader entry');extra=''' # Q02 literal source reference and truthful fixed lifecycle failure labels.
 oracle=json.loads((root/'metadata/PACKET_PARSER_ORACLE_FIX28.json').read_text())
 for row in oracle['files']:
  if sha(root/row['destination'])!=row['current_sha256']:fail(2,'live source parser oracle drift')
 if graph['test_support_sources'].get('tests/integration/test_config2_source_oracle.cpp')!=['tests/support/configuration/packet_parser.cpp'] or 'target_sources(${name} PRIVATE tests/support/configuration/packet_parser.cpp)' not in cmake:fail(2,'source parser oracle test-only target binding lost')
 entry=(root/'src/windows/dll_entry.cpp').read_text()
 if entry.count('LogEvent::startup_failed')!=2 or entry.count('LogEvent::shutdown_failed')!=1:fail(2,'wrong native lifecycle failure event')
 for row in json.loads((root/'metadata/TEST_ORACLE_ROLES_FIX28.json').read_text())['files']:
  if sha(root/row['file'])!=row['sha256']:fail(2,'test role binding drift')
''';p.write_text(s[:i]+extra+s[i:])
p=R/'tests/tools/test_source_policy.py';s=parent['tests/tools/test_source_policy.py'].decode();anchor="  assert not check(root),'restored baseline'";s=s.replace(anchor,"  modify('src/windows/dll_entry.cpp',lambda b:b.replace(b'LogEvent::startup_failed',b'LogEvent::startup'),'failure relabeled as startup')\n  modify('tests/support/configuration/packet_parser.cpp',lambda b:b+b'\\n// tampered oracle\\n','literal source oracle tamper')\n"+anchor);p.write_text(s)
p=R/'scripts/qa/package_source.py';p.write_text(parent['scripts/qa/package_source.py'].decode().replace('BF02_FIX27_APPLICATION_SOURCE','BF02_FIX28_APPLICATION_SOURCE'))
d=json.loads(parent['metadata/PROJECT.json']);d.update(code_state='BF02_FIX28_LIVE_SOURCE_ORACLE_AND_FAILURE_EVENTS',snapshot_parent={'archive':scope['parent_archive'],'sha256':scope['parent_archive_sha256'],'classification':'FULL_SOURCE_SNAPSHOT_PARENT'},change_reason=scope['accepted_scope']);d['governance']['binding']='metadata/GOVERNANCE_BINDING_FIX28.json';save(R/'metadata/PROJECT.json',d)
sys.path.insert(0,str(R/'scripts/qa'));from filesystem_inventory import scan_calls
save(R/'metadata/FILESYSTEM_CALLS_FIX28.json',{'scope':'FIX28 constant failure event labels only; logger filesystem behavior unchanged; test-only oracle target added.','calls':scan_calls(R)});save(R/'metadata/ACCEPTANCE.json',{'classification':'NON_RELEASE_WORK','status':'PENDING_FIX28_FINAL_QA','official_publication':False})
p=R/'docs/GOVERNANCE_APPLICABLE.md';s=parent['docs/GOVERNANCE_APPLICABLE.md'].decode().replace('FIX27','FIX28');i=s.index('Kapsam D14');j=s.index('\n\n',i);s=s[:i]+'Kapsam Q02 canlı RE parser oracleı, dürüst test rol ayrımı ve sabit lifecycle failure event düzeltmesidir. Üretim parser/vendor/default/CFG/lifecycle/exit davranışı değişmez. Startup_failed/shutdown_failed adları mevcut failure dallarını doğru etiketler; what/payload/path loglanmaz ve Production disk log kapalıdır. Q02 bounded QA incelemesi kapanır;8 ana inceleme sürer. Tüm testlerin bağımsızlığı veya native Windows PASS iddia edilmez.'+s[j:];p.write_text(s);binding=json.loads(parent['metadata/GOVERNANCE_BINDING_FIX27.json']);binding.update(status='CURRENT_LIBRARY_VERSION9_RECHECKED_FOR_FIX28',scope=scope['accepted_scope']);binding['packaged_projection']['md_sha256']=sha(p.read_bytes());save(R/'metadata/GOVERNANCE_BINDING_FIX28.json',binding)
p=R/'README.md';s=parent['README.md'].decode().replace('FIX27','FIX28');i=s.index('FIX28:');j=s.index('\n\n',i);s=s[:i]+'FIX28: RE paketindeki parser gövdesi test-only canlı referans olarak PEGTL ile karşılaştırılır; eski corpusun expected sütunu kullanılmaz. Başlangıç/cleanup hataları startup_failed/shutdown_failed sabit olaylarıyla doğru etiketlenir. Production log kapalı ve veri gizliliği korunur. Q02 bounded incelemesi kapanır;8 kayıt açıktır. Ayrıntı docs/RE_SOURCE_DEVIATIONS_FIX28.md içindedir.'+s[j:];p.write_text(s)
p=R/'docs/ARCHITECTURE.md';s=parent['docs/ARCHITECTURE.md'].decode().replace('# Güncel mimari — FIX27','# Güncel mimari — FIX28').replace('docs/RE_FINDINGS_FIX27.csv','docs/RE_FINDINGS_FIX28.csv').replace('diğer9 açık','diğer8 açık').replace('Toplam9 inceleme/kısmi kayıt açık.','Toplam8 inceleme/kısmi kayıt açık.').replace('9 kaynak incelemesi açıktır.','8 kaynak incelemesi açıktır.');s+='\nFIX28: Packet parser hpp/cpp yalnız test_config2_source_oracle hedefinde derlenir; production graphına girmez. Body/State/Node ayrı ve literal RE tanımlarıdır; include ve namespace dönüşümü byte düzeyinde geri çevrilebilir. Config parse acceptance algoritması değişmez. Native startup catches iki startup_failed ve cleanup catch bir shutdown_failed sabit event üretir; control flow ve cleanup waitleri aynıdır. Production disk log kapalıdır; Debug fixed-event API raw exception/payload kabul etmez.\n';p.write_text(s)
(R/'docs/DEVELOPMENT.md').write_text('''# Geliştirme — FIX28

NON_RELEASE Q02 düzeltmesi. Runtime scope logging.hpp/cpp ve windows/dll_entry.cpp içindeki sabit failure label mappingidir. Mevcut enum değerleri kaymaz; yeni iki değer sona eklenir. Production log kapalı kalır; exception what/raw payload/path bilgisi geri eklenmez. Parser/CFG/default/avatar/persistence/lifecycle/exit ve vendor algoritmaları değişmez.

RE parser header/bodyları test-only namespace/include relocationla alınır. Literal byte reverse-check, archive/origin/destination SHA bağları ve test-only CMake targetı zorunludur. 8766 corpus sadece input sağlar; expected sütunu yeni oracle tarafından kullanılmaz. Source/current state ayrı nesnelerdir. Ortak serializer yalnız gözlenen bütün state alanlarını karşılaştırır, parse kuralı veya expected result üretmez.

Önceki52 C++ fixture/header bytesı korunur. Test rol listesi ancestryyi bağımsız oracle diye göstermez; adapted app expectations, frozen self regression, bound source fragments, native fixtures ve owned integration ayrı etiketlenir. Mevcut fixed network_parse/network_crypto/failure kategorileri privacy tradeoff olarak açıkça korunur; tam orijinal diagnostic text veya native çalıştırma iddiası yoktur.
''')
(R/'docs/VALIDATION.md').write_text('''# Doğrulama — FIX28

49 CMake/CTest grubu (46 C++ +3 Python). Yeni canlı Config2 oracle 77205 karşılaştırmada tree/assignment chronology/open frames/accumulator/key/value/BOM ve ParseError rule/byte/line/columnu içerir. 8766 eski input, kısa byte-string/context üretimi ve reused-State ardışık parse örnekleri vardır. Production parser body aynı kalır. Test-only oracle aynı algoritmanın yeniden yazımı değildir; literal user RE sourceudur. Original DLL/exception ABI oracleı değildir.

Yeni logger testi gerçek host Environment fixed-event logunu Debugda okur, Productionda dosya oluşmadığını doğrular. Windows catch event seçimleri source guard/negatiflerle bağlanır; Windows bootstrap burada çalıştırılmaz. Parent formatter body projectionı ve wrong parse/control mutations scoped counterexamplesdır; native equivalence değildir. Test rol metadata hashleri ve exact oracle reversal doğrulanır.

71 source policy,6 package,8 synthetic PE negatifleri ve Golden selftests ayrı kayıtlıdır. Debug/Production, focused instrumented surface ve exact ZIP fresh-object replay final hashlerle bağlanır. Full-core sanitizer/LSan, Windows/MSVC/DLL/native provider/CRT/game ve actual DLL Golden NOT_RUN. Q02 kapanışı tüm fixtureların bağımsızlığı/gelecekteki QA kusursuzluğu anlamına gelmez;8 ana inceleme açıktır.
''')
p=R/'docs/RELEASE_PROCESS.md';p.write_text(parent['docs/RELEASE_PROCESS.md'].decode().replace('FIX27','FIX28'))
report='''# FIX28 — Config2 için canlı kaynak referansı ve doğru failure olayları

NON_RELEASE tam kaynak/build/QA devam paketi, exact FIX27 ebeveyninden türetildi. Karşılaştırma kullanıcının RE C++ paketidir; özgün DLL binary eşdeğerliği değildir.

## Giderilen QA borcu

Eski Config2 differential testi8766 dondurulmuş FIX09 expected çıktısına bakıyordu. Bu değerler regresyon kanıtıdır; kendi başlarına bağımsız kaynak doğrulaması değildir. FIX28 paket anadius-parser-source/parser.hpp/cpp dosyalarını ayrı test namespaceinde gerçekten derler. Include relocation ve namespace dışında bütün body bytesı aynıdır; ters dönüşüm orijinal dosyayla birebir eşleşir. Test referansı production DLL graphına girmez. Referansın parser State/Node/algoritması adaydan ayrıdır; dormant configuration conversion API için mevcut config tipi kullanılır ve bunun sınırı binding kaydında açıklanır.

Yeni test eski corpusun sadece input sütununu alır, expected sütununu okumaz. Canlı source parse sonucu PEGTL sonucu ile77205 örnekte karşılaştırılır. Logical tree, assignment sırası, incomplete/open frames, accumulator/key/value, BOM, hata kuralı ve üç koordinat dahildir. Kısa byte stringler, iki grammar contexti ve art arda reused-State örnekleri eski corpusun ötesine geçer. Bu sonucun anlamı source model ile aktif parserın incelenen sözleşmesidir; orijinal binary exception ABI veya bütün sonsuz input uzayı değildir. Parser üretim kodu değiştirilmedi.

Önceki52 C++ test/header bytesı korunur. Tüm current C++ test/support dosyaları conservative rol tablosuna bağlanır. Packet ancestryli uyarlanmış fixturelar literal independent oracle sayılmaz; app expectations, frozen self regression, source fragments, native fixtures ve owned integration açıkça ayrılır. Rol metadataı kendi başına expectation onayı üretmez.

## Somut log hatası ve korunan privacy tercihi

Windows bootstrapın iki exception catchi hata olduğunda startup olayını yazıyordu; cleanup catchi phase yazıyordu. Artık sabit startup_failed ve shutdown_failed etiketleri var. Enum değerleri sona eklendi; önceki ordinal değerleri değişmedi. Exception/stop/cleanup kontrol akışı, timeout/looplar ve mevcut exit davranışı aynıdır.

Debug loguna what/raw XML/token/config/path dökülmez. Mevcut network_parse/network_crypto/network_failure sabit kategorileri privacy tercihi olarak korunur ve tam orijinal diagnostic-text eşdeğerliği diye sunulmaz. Production disk log kapalıdır. Gerçek host logger testinde fixed event formatı ve Production dosyasızlık doğrulanır. Windows catchlerin actual native yürütmesi NOT_RUNdır; source binding ve negatif kontroller onun yerine native PASS yazmaz.

## Durum ve sınır

Q02 bounded source-QA/diagnostic incelemesi kapandı: frozen-only referans eksikliği ve yanlış failure etiketleri giderildi, kalan fixed-category privacy tercihi açıkça review edilip korundu. Açık9→8: DEP05, D02, D06, D07, D13, D14, D15, D21. Bu kapanış bütün testlerin bağımsız, bütün runtime farklarının çözülmüş veya Windows/game kabulünün verilmiş olduğu iddiası değildir.

119 diğer runtime dosyası,1060 vendor, parser/CFG/default/avatar/persistence algoritmaları ve bütün önceki Golden/CMD/PS1 araçları değişmez. Final QA sonuçları ayrı rapordadır; Windows/MSVC/provider/CRT/IOCP/game, actual DLL Golden ve LSan NOT_RUN.
''';(R/'docs/RE_SOURCE_DEVIATIONS_FIX28.md').write_text(report)
own=json.loads(parent['metadata/DOCUMENTATION_OWNERSHIP.json']);own['master_binding']='metadata/GOVERNANCE_BINDING_FIX28.json'
for row in own['documents']:row['sha256']=sha((R/row['path']).read_bytes())
save(R/'metadata/DOCUMENTATION_OWNERSHIP.json',own)
patch=[]
for n in [*changes,*new_tests,*[x['destination'] for x in oracle['files']],'CMakeLists.txt','metadata/BUILD_GRAPH.json','scripts/qa/check_source.py','tests/tools/test_source_policy.py']:patch.extend(difflib.unified_diff(parent.get(n,b'').decode().splitlines(True),(R/n).read_text().splitlines(True),fromfile='FIX27/'+n,tofile='FIX28/'+n,n=3))
(R/'docs/SOURCE_CHANGES_FIX28.patch').write_text(''.join(patch));print('Prepared FIX28; test roles',len(roles),'open',len(opens))
