from pathlib import Path
import collections,csv,difflib,hashlib,json,sys,zipfile
W=Path(__file__).resolve().parents[1];R=W/'fix31_work/preloader_l';H=W/'fix31_helpers'
archive=W/'deliverables/BF02_FIX30/preloader_l_BF02_FIX30_APPLICATION_SOURCE_NOT_RELEASE.zip'
with zipfile.ZipFile(archive) as z:parent={n.removeprefix('preloader_l/'):z.read(n) for n in z.namelist() if not n.endswith('/')}
sha=lambda b:hashlib.sha256(b).hexdigest()
def save(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n')
def load(n):return json.loads(parent[n])
g0=json.loads((H/'G0.json').read_text());assert sha(archive.read_bytes())==g0['parent_zip_sha256']
changed={}
for p in sorted((R/'src').rglob('*')):
 if p.is_file():
  n=p.relative_to(R).as_posix()
  if p.read_bytes()!=parent.get(n):changed[n]={'before_sha256':sha(parent[n]),'current_sha256':sha(p.read_bytes()),'authority_ids':['B31-R1']}
assert list(changed)==['src/windows/unload_guard.cpp']
guard=b'  // Refuse a region exceeding the remaining scan budget; never infer it safe.\n  if(limit-cursor>64ULL*1024*1024-scanned)return false;\n'
assert (R/'src/windows/unload_guard.cpp').read_bytes().replace(guard,b'')==parent['src/windows/unload_guard.cpp']
assert sha((R/'src/windows/unload_guard.cpp').read_bytes())=='563ed1c1b3a140128422e627ba1f5c0a02dd701c73b9bf36cc4e6508b0408ece'
scope={**g0,'accepted_scope':g0['scope'],'parent_archive':archive.name,'parent_archive_sha256':g0['parent_zip_sha256'],
 'parent_runtime_files':{n:sha(b) for n,b in parent.items() if n.startswith('src/')},'runtime_changes':changed,
 'dependency_manifest_sha256':sha(parent['metadata/THIRD_PARTY.json']),
 'authority_records':{'B31-R1':{'classification':'REVIEW31_SOURCE_BOUND_REGION_SCAN_BUDGET_FIX',
 'prior_own_decision':'64MiB scan is own unload integration policy, not packet/upstream/user-specific number. Parent outer loop did not bound final region.',
 'repair':'Exact unapplied REVIEW31 proposal now applied; reject over-budget region before any memory read. Same value; can conservatively refuse unload earlier.',
 'preserved':'Pin, leases, freeze/hook/engine, exception/CRT, thread snapshot and retry/deadline policies. No new user input/cache quotas.',
 'native_execution':'NOT_RUN','historical_reason_limit':'No retrospective user authorization claimed.'}},
 'test_only_adapters':'Linux SDK declarations and SEH syntax adapter, actual production cpp include, linker memcpy observation. Not Windows ABI/SEH/API execution. Native MSVC branch supplied, NOT_RUN.',
 'test_instrumentation':'-fno-builtin-memcpy and test-only -U_FORTIFY_SOURCE/-D_FORTIFY_SOURCE=0 prevent glibc fortified inline from hiding observed memcpy; production flags unchanged.',
 'new_policies':[],'vendor_changed':False}
save(R/'metadata/CHANGE_AUTHORITY_FIX31.json',scope)
g=load('metadata/BUILD_GRAPH.json');test='tests/integration/test_unload_scan.cpp';g['integration_tests'].append(test);g['included_implementation_sources'][test]=['src/windows/unload_guard.cpp'];save(R/'metadata/BUILD_GRAPH.json',g)
p=R/'CMakeLists.txt';s=parent['CMakeLists.txt'].decode().replace('foreach(source IN ITEMS tests/integration/test_atomic_stage_collision.cpp','foreach(source IN ITEMS '+test+' tests/integration/test_atomic_stage_collision.cpp')
anchor='    target_link_libraries(${name} PRIVATE preloader_l_core)';assert s.count(anchor)==1
extra='''    if(name STREQUAL "test_unload_scan" AND NOT WIN32)
      target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/support/unload/winshim")
      # Test-only linker observation of production memcpy; no shipping flags change.
      target_compile_options(${name} PRIVATE -fno-builtin-memcpy -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0)
      target_link_options(${name} PRIVATE "-Wl,--wrap=memcpy")
    endif()
''';p.write_text(s.replace(anchor,extra+anchor))
imports=load('metadata/IMPORTS.json')
for row in imports['files']:
 p=R/row['destination'];d=sha(p.read_bytes())
 if row['destination'] in changed:row.update(current_sha256=d,FIX31_authority_ids=['B31-R1'])
 else:assert d==row['current_sha256'],row['destination']
save(R/'metadata/IMPORTS.json',imports)
roles=load('metadata/TEST_ORACLE_ROLES_FIX30.json');byfile={r['file']:r for r in roles['files']};rows=[]
for p in sorted((R/'tests').rglob('*')):
 if not p.is_file() or p.suffix not in ['.cpp','.hpp','.h']:continue
 n=p.relative_to(R).as_posix();row=dict(byfile.get(n,{'file':n,'role':'OWNED_TEST_ONLY_OS_DECLARATIONS','origin':'','original_sha256':'','limit':'Synthetic SDK declaration subset, not native ABI/OS/SEH oracle.'}))
 row.update(sha256=sha(p.read_bytes()),byte_identical_to_FIX30=n in parent and p.read_bytes()==parent[n])
 if n==test:row.update(role='OWNED_ACTUAL_PRODUCTION_BODY_SCAN_AND_THREAD_EFFECTS',limit='Actual current production cpp included unchanged; synthetic OS and read observation on Linux. Real Windows memory-query fixture present but NOT_RUN. Does not establish race-free unload or original binary equivalence.')
 rows.append(row)
assert len(rows)==67
roles.update(files=rows,scope='Preserve all64 parent C++/header fixtures; one new test and2 declaration headers. Distinguish actual body/model effects from native ABI/OS acceptance.');save(R/'metadata/TEST_ORACLE_ROLES_FIX31.json',roles)
with (R/'docs/TEST_ORACLE_ROLES_FIX31.csv').open('w',newline='') as f:
 fields=sorted({k for row in rows for k in row});w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
a=load('metadata/RE_SOURCE_DEVIATIONS_FIX30.json');a['schema']='preloader_l.RE_source_deviations_FIX31.v1';a['scope']=scope['accepted_scope'];a['FIX31_parent_zip_sha256']=scope['parent_archive_sha256'];a['FIX31_runtime_changes']=changed
for f in a['findings']:
 if f['id']=='DEP05':
  f.update(status='CLOSED_REVIEWED_MIXED_FILESYSTEM_OWNERS',difference='gulrak constructs module-sibling paths with native-spelling preservation; packet Windows lexical/encoding/known-status owners remain active; std host paths/streams and owned atomic OS IO retain their responsibilities.',recommended_next_action='Retain documented mixed ownership. No universal gulrak migration or native IO equivalence claim. Unknown status domain is D07.')
  f['FIX31_review_authority']='REVIEW31 report libfile_a2543782e8c88191b017f55542b54f85; source ownership review only, not restored source equivalence.'
 if f['id']=='D07':
  f['difference']='Known Windows packet error/permission/sharing/reparse-target branches restored. Only packet-explicit unknown flags3 tag domain falls back to std status; non-Windows remains host projection.'
  f['recommended_next_action']='Review unknown-domain policy across StorageEffects, LicenseEffects AND OnlineSession cookie/profile-cache caller; three active bridges, kind/error used. Do not invent native equivalence or silent rejection.'
 if f['id'] in ['D13','D21']:
  f['FIX31_resolved_subfinding']='OVER_BUDGET_REGION_READ_BEFORE_REFUSAL'
  f['FIX31_resolution']='Same64MiB budget: reject whole over-budget region before reading any word. Original pin/lease/thread handling/retry/CRT policies retained. quiesce timeout bounds lease polling only, not entire native scan/uninitialize or whole DllShutdown.'
  f['difference']='Own unload integration retains self-pin/lease/snapshot/register-stack scan and retry/failure policies. FIX31 enforces retained64MiB region-read budget; original selected number and total deadline/native quiescence policy remain open.' if f['id']=='D13' else 'Own100 temp-name attempts,512→32768 module buffer,5s lease poll,100ms/10ms retry and1000ms failure sleep remain. Same64MiB scan budget now refuses whole over-budget regions before read. Numeric provenance and total-operation deadline remain open.'
 for x in f['current']:
  raw=(R/x['file']).read_bytes();hits=[i+1 for i,l in enumerate(raw.decode().splitlines()) if x['locator'] in l] if x['locator']!='<file>' else [1];assert hits,(f['id'],x);x.update(line=hits[0],sha256=sha(raw))
for row in a['sources']:row.update(current_sha256=sha((R/row['current_file']).read_bytes()),application_changed_FIX31=row['current_file'] in changed,FIX31_authority_ids=['B31-R1'] if row['current_file'] in changed else [])
a['status_counts']=dict(collections.Counter(f['status'] for f in a['findings']));opens=[f['id'] for f in a['findings'] if f['status'].startswith(('REVIEW','PARTIAL'))];assert opens==['D02','D06','D07','D13','D14','D21']
disposition={'source_FIX30_open':7,'review31_open':6,'before_open':6,'after_open':6,'closed_bounded_records':[],'carried_review_closure':['DEP05'],
 'resolved_subfindings':['D13/D21_OVER_BUDGET_REGION_READ_BEFORE_REFUSAL'],'remaining_open':opens,
 'not_claimed':['D13/D21 main closure','All6 reviews solved','Native Windows/SEH/thread/DLL/game acceptance','Original binary equality'],'native_validation':'NOT_RUN_separate_dimension'}
a['FIX31_closure_accounting']=disposition;save(R/'metadata/RE_SOURCE_DEVIATIONS_FIX31.json',a);save(R/'metadata/AUDIT_DISPOSITIONS_FIX31.json',disposition)
native=load('metadata/NATIVE_VALIDATION_FIX30.json');native['changed_surface']='unload_guard.cpp whole-region budget refusal. Actual production body/model effects executed on Linux; native SDK/SEH/thread/DLL Windows tests NOT_RUN. MSVC real memory scanner fixture included for later execution.';save(R/'metadata/NATIVE_VALIDATION_FIX31.json',native)
for name,fields,records in [('RE_FINDINGS_FIX31.csv',['id','status','priority','title','FIX31_resolution','recommended_next_action'],a['findings']),('RE_SOURCE_CROSSWALK_FIX31.csv',['current_file','current_sha256','comparison','original','finding_ids','FIX31_authority_ids'],a['sources'])]:
 with (R/'docs'/name).open('w',newline='') as f:
  w=csv.DictWriter(f,fieldnames=fields);w.writeheader()
  for row in records:w.writerow({k:json.dumps(row.get(k,''),ensure_ascii=False) if isinstance(row.get(k),list) else row.get(k,'') for k in fields})
p=R/'scripts/qa/check_source.py';s=parent['scripts/qa/check_source.py'].decode()
for old,new in [('CHANGE_AUTHORITY_FIX30','CHANGE_AUTHORITY_FIX31'),('GOVERNANCE_BINDING_FIX30','GOVERNANCE_BINDING_FIX31'),('FILESYSTEM_CALLS_FIX30','FILESYSTEM_CALLS_FIX31'),('TEST_ORACLE_ROLES_FIX30','TEST_ORACLE_ROLES_FIX31'),('recorded FIX30','recorded FIX31'),('change in FIX30','change in FIX31'),('FIX30 scope','FIX31 scope')]:s=s.replace(old,new)
at=s.index(' # 3: loader entry');extra=''' # Actual production unload body is included only by its explicit test target.
 scan=(root/'src/windows/unload_guard.cpp').read_text()
 if 'if(limit-cursor>64ULL*1024*1024-scanned)return false;' not in scan:fail(2,'whole-region scan budget refusal lost')
 if scan.find('if(limit-cursor>64ULL*1024*1024-scanned)')>scan.find('std::memcpy(&value,'):fail(2,'scan budget checked after read')
 test='tests/integration/test_unload_scan.cpp'
 if graph['included_implementation_sources'].get(test)!=['src/windows/unload_guard.cpp'] or test not in cmake:fail(2,'actual unload body test binding lost')
 for term in ['if(name STREQUAL "test_unload_scan" AND NOT WIN32)','-fno-builtin-memcpy -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0','-Wl,--wrap=memcpy']:
  if term not in cmake:fail(2,'test-only unload read observation binding lost: '+term)
''';p.write_text(s[:at]+extra+s[at:])
p=R/'tests/tools/test_source_policy.py';s=parent['tests/tools/test_source_policy.py'].decode();anchor="  assert not check(root),'restored baseline'"
extra="""  modify('src/windows/unload_guard.cpp',lambda b:b.replace(b'if(limit-cursor>64ULL*1024*1024-scanned)return false;',b'/* guard removed */'),'scan region budget removed')
  modify('src/windows/unload_guard.cpp',lambda b:b.replace(b'if(limit-cursor>64ULL*1024*1024-scanned)return false;',b'if(limit-cursor>64ULL*1024*1024-scanned)return true;'),'over-budget scan falsely reports safe')
  modify('CMakeLists.txt',lambda b:b.replace(b'if(name STREQUAL "test_unload_scan" AND NOT WIN32)',b'if(name STREQUAL "test_unload_scan")'),'test shim target leaks to native build')
  modify('CMakeLists.txt',lambda b:b.replace(b'-Wl,--wrap=memcpy',b'-Wl,--no-wrap'),'scan read observation removed')
""";p.write_text(s.replace(anchor,extra+anchor))
p=R/'scripts/qa/package_source.py';p.write_text(parent['scripts/qa/package_source.py'].decode().replace('BF02_FIX30_APPLICATION_SOURCE','BF02_FIX31_APPLICATION_SOURCE'))
d=load('metadata/PROJECT.json');d.update(code_state='BF02_FIX31_UNLOAD_REGION_SCAN_BUDGET',snapshot_parent={'archive':archive.name,'sha256':scope['parent_archive_sha256'],'classification':'FULL_SOURCE_SNAPSHOT_PARENT'},change_reason=scope['accepted_scope']);d['governance']['binding']='metadata/GOVERNANCE_BINDING_FIX31.json';save(R/'metadata/PROJECT.json',d)
sys.path.insert(0,str(R/'scripts/qa'));from filesystem_inventory import scan_calls
save(R/'metadata/FILESYSTEM_CALLS_FIX31.json',{'scope':'FIX31 runtime change only pre-read whole-region budget refusal; no filesystem call changes.','calls':scan_calls(R)})
save(R/'metadata/ACCEPTANCE.json',{'classification':'NON_RELEASE_WORK','status':'PENDING_FIX31_FINAL_QA','official_publication':False})
governance='''# BLUEFIELD için geçerli governance — FIX31

En yeni kullanıcı talimatı üstündür. Güncel master version9 revision2026-10-25.1 raw hashleri fresh read ile doğrulandı; metadata/GOVERNANCE_BINDING_FIX31.json bağlar. Normatif alt küme; Library-only debt/resmî tarihçe mirrorlanmaz.

BLUEFIELD NON_RELEASE kod atası tam RE paketidir. preloader_l/SURGE/export/optional CFG/profil/avatar, Production disk log kapatma, failure-atomic ve owned shutdown korunur. Diğer FIELD/canonical release/pointer/vendor pinleri değişmez. Yerel build/test yayın veya oyun kabulüne bağlanmaz.

FIX31 yalnız REVIEW31 whole-region scan guardını uygular; aynı64MiB, aşan bölge okunmadan false döner. Kullanıcı girdisine/cache'e yeni kota yok. Module pin/lease/thread freeze/hook/CRT/persistence/timeout politikası değiştirilmez. quiesce timeout yalnız lease pollingdir; total süre garantisi değildir.

Yeni test gerçek production cpp'yi tek test TU içinde derler. Linux test-only SDK/SEH syntax shim ve linker memcpy gözlemi native ABI/SEH/OS kanıtı sayılmaz. Test gözlemi için yalnız bu TU'da fortify inline devre dışıdır; shipping flagleri aynı. MSVC actual VirtualQuery/VirtualAlloc fixtureı sağlanır; burada Windows/MSVC/DLL/game/actual Golden NOT_RUN.64 önceki C++/header fixture korunur. REVIEW31 DEP05 inceleme kapanışı taşınır;6 ana inceleme açık, yalnız D13/D21 alt guard hatası düzeltilir.
''';(R/'docs/GOVERNANCE_APPLICABLE.md').write_text(governance)
binding=load('metadata/GOVERNANCE_BINDING_FIX30.json');binding.update(status='CURRENT_LIBRARY_VERSION9_FRESH_HASH_READ_FIX31',scope=scope['accepted_scope']);binding['packaged_projection']['md_sha256']=sha((R/'docs/GOVERNANCE_APPLICABLE.md').read_bytes());save(R/'metadata/GOVERNANCE_BINDING_FIX31.json',binding)
p=R/'AGENTS.md';p.write_text(p.read_text().replace('GOVERNANCE_BINDING_FIX30','GOVERNANCE_BINDING_FIX31'))
p=R/'README.md';s=parent['README.md'].decode();i=s.index('FIX30:');j=s.index('\n\n',i);s=s[:i]+'FIX31: Unload stack scan son bellek bölgesiyle64MiB bütçeyi aşamıyor; aşan bölge okunmadan unload reddediliyor. Diğer runtime/vendor politikaları aynı. REVIEW31 filesystem kapsam kapanışı taşındı;6 ana inceleme açık. Ayrıntı docs/RE_SOURCE_DEVIATIONS_FIX31.md.'+s[j:];p.write_text(s.replace('BLUEFIELD FIX30','BLUEFIELD FIX31').replace('GOVERNANCE_BINDING_FIX30','GOVERNANCE_BINDING_FIX31'))
p=R/'docs/ARCHITECTURE.md';p.write_text(parent['docs/ARCHITECTURE.md'].decode().replace('# Güncel mimari — FIX30','# Güncel mimari — FIX31').replace('RE_FINDINGS_FIX30.csv','RE_FINDINGS_FIX31.csv')+'\nFIX31: scan_stack her region için kalan64MiB bütçeye sığma denetimini read öncesinde yapar. Sığmazsa false; pin/lease/disable/drain/uninitialize politikası aynı. Quiesce timeout yalnız aktif lease pollingine uygulanır. Native scan/uninitialize süre üst sınırı veya bütün DllShutdown deadlineı iddia edilmez. Test TU üretim cppyi dahil eder; test shimi production graphına girmez.\n')
(R/'docs/DEVELOPMENT.md').write_text('''# Geliştirme — FIX31

Tek runtime dosyasında iki ek satır:64MiB kalan scan bütçesine sığmayan region read öncesi false. Bu REVIEW31 exact-base proposalıdır; sayıyı değiştirmez ve taranmayan bölgeyi güvenli saymaz. Diğer122 runtime ve bütün vendor bytesı korunur. Önceki64 C++/header fixture unchanged.

test_unload_scan tek TUda actual production cppyi dahil eder. Linux testinde declaration subset + sentetik API etkileri, SEH syntax adapterı ve GNU linker memcpy gözlemi kullanılır. Compiler fortify inline memcpyyi gizlediği için yalnız bu targetta -U_FORTIFY_SOURCE/-D_FORTIFY_SOURCE=0 ve -fno-builtin-memcpy gerekir; üretim hardeningi değişmez. MSVC kolu actual VirtualAlloc/VirtualQuery fixtureını derler, shimi kullanmaz.

Parent body, false-safe ve off-by-one mutantları aynı unchanged test driverla reddedilmelidir. Source algorithm kopyası oracle değildir; test actual body okuma izini gözler. SDK layout/SEH/thread suspension ve bütün unload güvenliği bu modelin kanıtı değildir. Native NOT_RUN ayrı tutulur.
''')
(R/'docs/VALIDATION.md').write_text('''# Doğrulama — FIX31

52 portable CTest grubu:49 C++ executable +3 Python Golden/tool grubu. Actual source-body scanner testi150 model/effect kontrolü içerir. Kalan bütçeyi aşan ilk/ikinci bölge sıfır okuma, tam bütçede conservative false, son8 byteın kabulü,16 register, module pointer ve guard/query/error dalları sınanır. Suspend/context/resume/close sequence sentetik API etkileridir; gerçek Windows thread suspension değildir.

80 source policy,6 ZIP,8 synthetic PE negatifleri. Parent/false-safe/off-by-one semantic controls; focused ASan/UBSan yalnız driver+included production body TU, core archive uninstrumented. Exact source ZIP fresh source/object Release/Production build ve52/52 replaye bağlanır.

Windows/MSVC SDK/SEH/thread/DLL/IOCP/provider/CRT/game/actual DLL Golden NOT_RUN. Native real memory scanner fixtureı sağlanır fakat çalışmış diye sunulmaz. D13/D21 ana başlıkları açık kalır; guard alt hatası çözülür. DEP05 REVIEW31 scope review kapanışı korunur. Kodda hiçbir yeni ürün kotası veya toplam timeout politikası eklenmez.
''')
p=R/'docs/RELEASE_PROCESS.md';p.write_text(parent['docs/RELEASE_PROCESS.md'].decode().replace('FIX30','FIX31'))
report='''# FIX31 — unload stack taramasında kalan region bütçesi

NON_RELEASE tam kaynak devam paketi. Exact kaynak parent FIX30; REVIEW31 ayrı rapor/evidence devamıdır, yeni source parent değildir. Karşılaştırma tam RE paketidir; özgün DLL binary eşdeğerliği iddiası yoktur.

REVIEW31de görülen hata: while(scanned<64MiB) yalnız önceki bölgelerin toplamını sınırlıyordu; son region bütçeyi aşsa da tümü okunabiliyordu. Bu kendi unload entegrasyonumuzun kontrol akışı sorunudur.64MiB seçimi kaynak/upstream zorunluluğu veya ayrı kullanıcı sayı onayı değildir. Kayıtsız eski motivasyon uydurulmaz.

Exact REVIEW31 patchi uygulandı: limit<=cursor denetiminden sonra, herhangi bir read öncesinde limit-cursor>64MiB-scanned ise false. Başka runtime satırı değişmedi. Aynı budget, whole-region refusal; sığmayan bölge kısmen taranıp safe sayılmaz. Öncekine göre daha erken unload reddi mümkündür. Modül pin/lease ve trampoline sahibi korunur. Freeze/MinHook, driver/thread enumeration, callback/CRT, config/profil/avatar, persistence, vendor ve tüm retry değerleri aynı.

Yeni testte scanner yeniden yazılmadı: actual unload_guard.cpp tek test TUya include edilir. Linux declaration/API/SEH-syntax shimleri gerçek Windows değildir. Linker-wrapped memcpy actual body read sayısını gözler; glibc fortified inlineın sayacı gizlememesi için yalnız test TUda fortify inline ve memcpy builtin devre dışıdır. Shipping hardening flagleri değişmez. MSVC kolu gerçek VirtualAlloc/VirtualQuery scanner fixtureı sağlar, burada NOT_RUN.

150 kontrol: küçük allocation success;16 register/module word rejection; query/guard/noaccess/uncommitted rejection; ilk region ve kalan8 byte üstündeki regionun read öncesi reddi; tam bütçede conservative false; son izinli8 byte; thread exclusion; open/suspend/context/resume/enumeration failure ve handle cleanup sırası. Parent body/mutants aynı test driverını düşürmelidir. SDK layout/SEH/access violation ve gerçek native suspension bu modele dahil değildir.

Unload/timeout incelemesi: NativeHooks::quiesce(timeout) yalnız active_calls pollingini deadline ile kontrol eder; ardından external thread scan ve MH_Uninitialize deadline almaz. Destructor5s, bütün destructor5s garantisi değildir. Worker100ms quiesce denemeleri +10ms retry sınırsız tekrar edebilir; failureda1000ms sleep modu pinli tutar. DllShutdown timeout tüm export süresinin ölçülmüş boundu sayılmaz. Bu tur yeni total deadline/cancellation politikası eklenmedi. Thread snapshot/yeniden giriş/SEH ve shutdown callerının module reference sözleşmesi native test gerektirir. D13 ve D21 bu yüzden bütünüyle kapanmaz.

Güncel review: DEP05 karma filesystem sahiplik incelemesi REVIEW31de kapandı; evrensel gulrak göçü/IO eşdeğerliği değildir.6 açık: D02 worker/order; D06 broad lock/reentry; D07 unknown-tag fallback; D13 native quiescence; D14 exit/CRT; D21 numeric policy/whole deadline. D07 güncel metni tarihsel FIX14 özetinden ayrıldı; storage/license yanında OnlineSession cookie/profile callerı da vardır. Bu rapor düzeltmesi runtime status politikasını değiştirmez.

Build/QA final sonuçları exact ZIPye bağlı dış FINAL_QA ile verilir. Windows/MSVC/DLL/native SDK/SEH/IOCP/provider/CRT/game/actual DLL Golden ve full-core/LSan NOT_RUN. Bu alt hatanın portable PASSı tüm unload güvenliği veya altı ana inceleme kapanışı değildir.
''';(R/'docs/RE_SOURCE_DEVIATIONS_FIX31.md').write_text(report)
own=load('metadata/DOCUMENTATION_OWNERSHIP.json');own['master_binding']='metadata/GOVERNANCE_BINDING_FIX31.json'
for row in own['documents']:row['sha256']=sha((R/row['path']).read_bytes())
save(R/'metadata/DOCUMENTATION_OWNERSHIP.json',own)
names=[*changed,test,'tests/support/unload/winshim/windows.h','tests/support/unload/winshim/tlhelp32.h','CMakeLists.txt','metadata/BUILD_GRAPH.json','scripts/qa/check_source.py','tests/tools/test_source_policy.py'];patch=[]
for n in names:patch.extend(difflib.unified_diff(parent.get(n,b'').decode().splitlines(True),(R/n).read_text().splitlines(True),fromfile='FIX30/'+n,tofile='FIX31/'+n))
(R/'docs/SOURCE_CHANGES_FIX31.patch').write_text(''.join(patch));print('PASS prepared FIX31; one exact REVIEW31 runtime guard;67 test roles;6 main reviews remain')
