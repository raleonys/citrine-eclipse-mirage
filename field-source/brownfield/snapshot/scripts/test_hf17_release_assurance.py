#!/usr/bin/env python3
from pathlib import Path
import hashlib,importlib.util,json,os,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True
errors=[]

# HF17 F323-S0037-001: preserve exact acceptance-document role/status oracle
# under new canonical HOTFIX 17 identity. Original HF16 oracle remains immutable.
acceptance_meta=json.loads((r/'metadata/RELEASE_ACCEPTANCE.json').read_text())
acceptance_doc=(r/'docs/RELEASE_ACCEPTANCE.md').read_text(encoding='utf-8')
ordinal=json.loads((r/'RELEASE_STATE.json').read_text()).get('hotfix_ordinal')
assert isinstance(ordinal,int) and ordinal>=17
official=f'preloader_l 3.2.5 SQUALERIS HOTFIX {ordinal}'
def validate_hf17_document(document,metadata,identity):
    issues=[]
    if not document.startswith('# '+identity+' — Acceptance freshness'):issues.append('identity/heading')
    if metadata.get('official_release_identity')!=identity or metadata.get('release_type')!='HOTFIX_RELEASE':issues.append('metadata identity')
    parent=metadata.get('source_parent_sha256','')
    if len(parent)!=64 or document.count('Source parent SHA-256: `'+parent+'`.')!=1:issues.append('parent sha')
    if ordinal==17:
        required={'Windows/MSVC x64 build/link':'NOT RUN','Windows native Win32 11-hook test':'NOT RUN','CollegeFB27 game test':'NOT RUN','Madden27 game test':'NOT RUN','Static CRT, Golden pipeline, release QA':'NOT RUN'}
        if metadata.get('windows_native',{}).get('status')!='NOT_RUN_HF17_CHANGED_BUILD_INPUTS':issues.append('native freshness')
        if metadata.get('source_qa',{}).get('status')!='PENDING_HF17_G2_FULL_8_OF_8':issues.append('source QA truthfulness')
        if any(metadata.get(k,{}).get('status')!='NOT_RUN_HF17' for k in ('collegefb27_game_network','madden27_game_network')):issues.append('game truthfulness')
        if 'HF16 prior Windows/Golden evidence does not automatically transfer' not in document:issues.append('no historical acceptance inheritance')
        if 'G3 — FINAL_RELEASE only: NOT_APPLICABLE for HF17 HOTFIX_RELEASE.' not in document:issues.append('G3 applicability disclaimer')
    elif ordinal==18:
        required={'Windows/MSVC x64 build/link':'PASS BOUNDED FIX01 18/18','Windows native Win32 11-hook test':'PASS BOUNDED FIX01 176/176','CollegeFB27 game test':'NOT RUN','Madden27 game test':'NOT RUN','Static CRT, Golden pipeline, release QA':'G4 8/8 NOT RUN'}
        if metadata.get('windows_native',{}).get('status')!='PASS_FIXED_SCOPE_HF18_FIX01_RETENTION_M0_M3_176_OF_176_SOURCE_LINKED':issues.append('native bounded receipt binding')
        if metadata.get('source_qa',{}).get('status')!='PENDING_G4_AUTHORITATIVE_8_OF_8_AND_FRESH_8_OF_8':issues.append('G4 source QA pending')
        if any(metadata.get(k,{}).get('status')!='NOT_RUN_HF18' for k in ('collegefb27_game_network','madden27_game_network')):issues.append('HF18 game pending')
        if 'They do not demonstrate a new Windows run' not in document:issues.append('bounded evidence disclaimer')
        if 'G3 — FINAL_RELEASE only: NOT_APPLICABLE for HF18 HOTFIX_RELEASE.' not in document:issues.append('G3 class')
    else:
        # Current hotfix acceptance derives its truth from current metadata.
        # Native/game tests remain explicitly optional for HOTFIX_RELEASE.
        required={
            'Windows/MSVC x64 build/link':'NOT RUN (optional)',
            'Windows native Win32 11-hook test':'NOT RUN (optional)',
            'CollegeFB27 game test':'NOT RUN (not required)',
            'Madden27 game test':'NOT RUN (not required)',
            'Static CRT, Golden pipeline, release QA':'G4 RECORDED EXTERNALLY',
        }
        if metadata.get('windows_native',{}).get('status')!=f'NOT_RUN_HF{ordinal}_OPTIONAL_NONBLOCKING':issues.append('optional native status')
        if metadata.get('source_qa',{}).get('status')!='PENDING_G4_AUTHORITATIVE_8_OF_8_AND_FRESH_8_OF_8':issues.append('source snapshot boundary')
        if f'G3 — FINAL_RELEASE only: NOT_APPLICABLE for HF{ordinal} HOTFIX_RELEASE.' not in document:issues.append('HOTFIX G3 applicability')
        if 'Current publication authority is the external Library CURRENT_ARTIFACT pointer.' not in document:issues.append('external current authority')
        if 'Historical HF18 and HF17 acceptance is preserved in their immutable official release archives.' not in document:issues.append('historical release preserved')
    for name,status in required.items():
        if document.count('| '+name+' | '+status+' |')!=1:issues.append('row:'+name)
    if metadata.get('release_class_g3',{}).get('status')!='NOT_APPLICABLE':issues.append('release class G3')
    return issues
if validate_hf17_document(acceptance_doc,acceptance_meta,official):
    errors.append('F323-S0037-001 current acceptance semantic binding failed')
wrong_parent=acceptance_doc.replace(acceptance_meta['source_parent_sha256'],'0'*64,1)
if not validate_hf17_document(wrong_parent,acceptance_meta,official):
    errors.append('F323-S0037-001 parent role swap false-PASS')
native_status=('PASS BOUNDED FIX01 176/176' if ordinal==18 else ('NOT RUN' if ordinal==17 else 'NOT RUN (optional)'))
wrong_status=acceptance_doc.replace('| Windows native Win32 11-hook test | '+native_status+' |','| Windows native Win32 11-hook test | PASS_FRESH |',1)
if not validate_hf17_document(wrong_status,acceptance_meta,official):
    errors.append('F323-S0037-001 synthetic native PASS false-PASS')

def read(rel):
    return (r/rel).read_text(encoding='utf-8',errors='replace')

def sha(rel):
    return hashlib.sha256((r/rel).read_bytes()).hexdigest()

# F11 — QA truthfulness and exact evidence-level separation.
levels=json.loads(read('metadata/QA_EXECUTION_LEVELS.json'))
matrix=json.loads(read('metadata/ACCEPTANCE_MATRIX.json'))
# Historical internal remediation state is Library-owned, not a current release authority.
ledger=json.loads(read('metadata/ACCEPTANCE_FRESHNESS_LEDGER.json'))
if levels.get('schema_version')!=1: errors.append('F11 execution-level schema missing')
for token in ['source_static=>windows_compile_link','portable_compile_or_runtime=>windows_native_acceptance','windows_native_acceptance=>game_network_acceptance']:
    if token not in levels.get('forbidden_conflation',[]): errors.append('F11 forbidden conflation missing: '+token)
gates={g.get('id'):g for g in matrix.get('gates',[])}
for gate in ['wave6_focused','release_source_qa','windows_native_acceptance','crt_transition','collegefb27_game_network','madden27_game_network']:
    if gate not in gates: errors.append('F11 acceptance-matrix gate missing: '+gate)
higher=['windows_native_acceptance','crt_transition','collegefb27_game_network','madden27_game_network']
state=json.loads(read('RELEASE_STATE.json'))
ledger_map={e.get('acceptance_class'):e for e in ledger.get('entries',[])}
if state.get('release_type')=='HOTFIX_RELEASE' and state.get('hotfix_ordinal')==10:
    for gate in ['windows_native_acceptance','crt_transition','collegefb27_game_network','madden27_game_network']:
        if gates.get(gate,{}).get('status')!='NOT_RUN_HF10':
            errors.append('F11 HF10 runtime mutation cannot inherit prior Windows/game acceptance: '+gate)
    for cls in ['WINDOWS_NATIVE','CRT_TRANSITION','COLLEGEFB27_GAME_NETWORK','MADDEN27_GAME_NETWORK']:
        e=ledger_map.get(cls,{})
        if e.get('status')!='PENDING' or e.get('carry_forward') is not None or 'HF10_PRODUCTION_CPP_CHANGED' not in e.get('freshness',''):
            errors.append('F11 HF10 source invalidated external acceptance: '+cls)
minhook=read('scripts/test_minhook_lifetime_contract.py')
for token in ['g_remove_status=MH_ERROR_MEMORY_PROTECT','assert(!remove_minhook(&t)); assert(minhook_ticket_has_ownership(&t));','g_remove_status=MH_OK']:
    if token not in minhook: errors.append('F11 MinHook remove-failure regression missing: '+token)
active=read('scripts/check_production_syntax.py')
if 'source-only CMake-derived Windows /Zs inventory; Windows compiler not executed by this gate' not in active:
    errors.append('F11 production syntax gate still overclaims Windows compiler execution')


# Third-party known-vulnerability coverage must be source-linked and never waive release acceptance.
third_party_register = json.loads(read('metadata/THIRD_PARTY_KNOWN_VULNERABILITIES.json'))
deps = {x.get('name'): x for x in third_party_register.get('dependency_records', [])}
if third_party_register.get('release_authority') != 'CURRENT_TRACKING_ONLY_NOT_A_WAIVER_NOT_PUBLICATION_AUTHORIZATION':
    errors.append('third-party vulnerabilities registry silently grants release authority')
required_deps = {'TinyXML2','MinHook'}
if not required_deps.issubset(deps): errors.append('third-party vulnerabilities registry missing deployed dependencies')
# Vendored directory keys need not equal their upstream brand/display names.
# Distinguish shipping-linked and explicitly test-only HF12 dependencies; do not
# mistake vendoring Asio for production socket-backend integration.
_vendor_directory_name = {'tinyxml2':'TinyXML2', 'minhook':'MinHook'}
try:
    _hf12_contract = json.loads(read('metadata/DEPENDENCY_SOURCE_CONTRACT.json'))
    for _key,_rec in _hf12_contract['dependencies'].items():
        _vendor_directory_name[_key] = _rec['name']
        _risk = deps.get(_rec['name'],{})
        if _risk.get('pinned_commit') != _rec['pinned_git_commit'] or _risk.get('usage') != _rec['usage']:
            errors.append('HF12 registered vendor provenance/usage drift: '+_key)
except Exception as exc:
    errors.append('HF12 vendor contract/known-vulnerabilities binding invalid: '+repr(exc))
active_vendor_dirs = {p.name.lower() for p in (r/'third_party').iterdir() if p.is_dir()}
if active_vendor_dirs != set(_vendor_directory_name):
    errors.append('active vendor folder set differs from named registry mapping')
if not set(_vendor_directory_name.values()).issubset(deps):
    errors.append('new vendored dependency lacks known-vulnerabilities coverage')
if deps.get('TinyXML2', {}).get('pinned_commit') != json.loads(read('metadata/TINYXML2_REFERENCE_PROVENANCE.json')).get('vendored_base',{}).get('upstream_commit'):
    errors.append('third-party registry TinyXML2 commit out of sync')
if deps.get('MinHook', {}).get('pinned_commit') != json.loads(read('third_party/minhook/UPSTREAM_PROVENANCE.json')).get('commit'):
    errors.append('third-party registry MinHook commit out of sync')
issues = {x.get('finding_id'): x for x in deps.get('TinyXML2',{}).get('issues',[])}
if set(issues) != {'TINYXML2-A02-A','TINYXML2-A02-B'}: errors.append('third-party registry missing tracked open A02')
for code in ('TINYXML2-A02-A','TINYXML2-A02-B'):
    if issues.get(code, {}).get('status') != 'OPEN' or issues.get(code, {}).get('fix_ready') is not False:
        errors.append('third-party registry closes unremediated risk: '+code)
if not (r/'docs/THIRD_PARTY_KNOWN_VULNERABILITIES.md').is_file():
    errors.append('third-party registry policy view missing')

# F14 — TinyXML2 must be a pinned production parser; verify source pin and link graph.
prov=json.loads(read('metadata/TINYXML2_REFERENCE_PROVENANCE.json'))
if prov.get('scope')!='production_parser_migration_candidate' or prov.get('shipping_linked') is not True:
    errors.append('F14 TinyXML2 role drift')
if prov.get('vendored_base',{}).get('version')!='11.0.0': errors.append('F14 v11 pinned version drift')
if prov.get('vendored_base',{}).get('upstream_commit')!='8224e427b655b83dae5e2298f1e6919523a78737': errors.append('F14 vendored commit drift')
if prov.get('decision_review',{}).get('upgrade_status')!='EXACT_UPSTREAM_MASTER_COMMIT_FILES_IMPORTED_VERIFIED': errors.append('F14 source import not verified')
if prov.get('qualified_upstream_fix',{}).get('tag_commit')!='9148bdf719e997d1f474be6bcc7943881046dba1':
    errors.append('F14 qualified upstream commit missing')
for rel,digest in prov.get('files',{}).items():
    if not (r/rel).is_file() or sha(rel)!=digest: errors.append('F14 provenance hash mismatch: '+rel)
cmake=read('CMakeLists.txt')
if 'add_library(preloader_l_tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp)' not in cmake or 'target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)' not in cmake: errors.append('F14 production TinyXML2 graph binding missing')
if (r/'src/xml/request_scanner.cpp').exists() or (r/'src/xml/tinyxml2_compat_bridge.hpp').exists(): errors.append('F14 forbidden legacy XML reader or bridge returned')
header=read('third_party/tinyxml2/tinyxml2.h')
for token in ['TIXML2_MAJOR_VERSION = 11','TIXML2_MINOR_VERSION = 0','TIXML2_PATCH_VERSION = 0']:
    if token not in header: errors.append('F14 v11 header version marker missing: '+token)
tiny=read('third_party/tinyxml2/tinyxml2.cpp')
for token in ['static const uint32_t MAX_CODE_POINT = 0x10FFFF;', 'if (mult > MAX_CODE_POINT)', 'if (ucs > MAX_CODE_POINT)', 'ucs > MAX_CODE_POINT - digitScaled']:
    if token not in tiny: errors.append('F14 upstream v11 security guard missing: '+token)

# D01 — arena-backed decoder input is stabilized before borrowing a pointer.
decode=read('src/config/store_values.cpp')
for fn in ['decode_quartets','decode_hex']:
    pos=decode.find('ParseCode '+fn)
    if pos<0: errors.append('D01 function missing: '+fn); continue
    body=decode[pos:decode.find('\n}',pos)+2]
    reserve=body.find('reserve_arena(state, begin + text.len)')
    borrow=body.find('const uint8_t* input = state.arena + text.off;')
    if reserve<0 or borrow<0 or reserve>borrow: errors.append('D01 reserve-before-borrow ordering missing: '+fn)
if not (r/'tests/unit/test_store_decode_reallocation.cpp').is_file(): errors.append('D01 forced-reallocation regression missing')

# D02 — exact allocation bytes live in private side metadata; strlen recovery is forbidden.
doc=read('src/runtime/service_xml.cpp')
for token in ['struct ModelXmlAllocationRecord','kModelXmlAllocationRecordCapacity','release_recorded_model_xml_buffer','retry_unpublished_model_xml_allocations','ModelXmlAllocationRecord{replacement_buffer, allocation_bytes, false}']:
    if token not in doc: errors.append('D02 side-ledger marker missing: '+token)
if 'strlen(' in doc or 'ModelXmlBufferHeader' in doc: errors.append('D02 length-recovery/header ownership form reintroduced')
model_test=read('tests/unit/test_model_config_semantic_runtime.cpp')
for token in ['embedded_nul','embedded_allocation==sizeof(embedded_nul)+1u','g_fail_release=true','g_last_release_size==embedded_allocation']:
    if token not in model_test: errors.append('D02 embedded-NUL/retry regression missing: '+token)

# D03 — generic drain must retain FIFO head on output-capacity failure.
event=read('src/runtime/event_queue.cpp')
start=event.find('bool drain_one_queued_event(')
end=event.find('\n}',start)
body=event[start:end+2] if start>=0 and end>start else ''
cap=body.find('capacity<n+1u')
unlink=body.find('unlink_node_locked(node)')
if cap<0 or unlink<0 or cap>unlink: errors.append('D03 capacity check must precede unlink')
prepared=read('tests/unit/test_prepared_event_queue.cpp')
for token in ['char tiny[4]','queued_event_count_for_test()!=1u','drain_one_queued_event(out,sizeof(out),&n)']:
    if token not in prepared: errors.append('D03 non-destructive regression missing: '+token)

if errors:
    print('PREAUDIT_WAVE6_ASSURANCE: FAIL',len(errors))
    for e in errors: print(' -',e)
    sys.exit(1)

cxx=os.environ.get('CXX','c++')
common=[cxx,'-std=c++17','-O0','-Wall','-Wextra','-Werror']
with tempfile.TemporaryDirectory(prefix='melotrix_wave6_') as td_s:
    td=Path(td_s)
    # D01 forced arena relocation.
    exe=td/'d01_store_decode'
    subprocess.run(common+['-I',str(r/'include'),'-I',str(r/'src/config'),
        str(r/'tests/unit/test_store_decode_reallocation.cpp'),str(r/'src/config/store_values.cpp'),
        str(r/'src/config/store_materialization.cpp'),str(r/'third_party/tinyxml2/tinyxml2.cpp'),'-o',str(exe)],cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)

    # D03 generic queue copy transaction.
    exe=td/'d03_event_drain'
    subprocess.run(common+['-Wno-misleading-indentation','-D_WIN32','-D__stdcall=','-D__declspec(x)=','-I',str(r/'include'),
        str(r/'tests/unit/test_prepared_event_queue.cpp'),str(r/'src/runtime/event_queue.cpp'),'-o',str(exe)],cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)

    # F14 debug build: preserve oversize/overflow character-reference execution regressions on pinned v11.
    exe=td/'f14_tinyxml2'
    # Upstream v11 DynArray::Size retains the benign signedness assertion on size_t.
    # Suppress this warning ONLY for the external pinned F14 unit, never globally.
    subprocess.run(common+['-Wno-type-limits','-DTINYXML2_DEBUG','-I',str(r/'third_party/tinyxml2'),
        str(r/'tests/unit/test_tinyxml2_references.cpp'),str(r/'third_party/tinyxml2/tinyxml2.cpp'),'-o',str(exe)],cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)

    # F11 hash-bound acceptance-record utility.
    fake_source=td/'source.zip'; fake_source.write_bytes(b'wave6-source-binding')
    fake_artifact=td/'native.log'; fake_artifact.write_bytes(b'pending-test-record')
    record=td/'record.json'
    subprocess.run([sys.executable,str(r/'scripts/write_acceptance_record.py'),
        '--gate','test-record','--execution-level','windows_native_acceptance','--platform','Windows-x64',
        '--command','TEST_ONLY','--status','PENDING','--source-zip',str(fake_source),
        '--artifact',str(fake_artifact),'--scenario','binding_probe','--output',str(record)],cwd=r,check=True)
    data=json.loads(record.read_text(encoding='utf-8'))
    if data['source_zip']['sha256']!=hashlib.sha256(fake_source.read_bytes()).hexdigest() or data['artifacts'][0]['sha256']!=hashlib.sha256(fake_artifact.read_bytes()).hexdigest():
        raise SystemExit('PREAUDIT_WAVE6_ASSURANCE: FAIL - acceptance record hash binding')

# D02 and F11 MinHook execute through their dedicated current source gates.
subprocess.run([sys.executable,str(r/'scripts/test_model_config_semantic_runtime.py')],cwd=r,check=True)
subprocess.run([sys.executable,str(r/'scripts/test_minhook_lifetime_contract.py')],cwd=r,check=True)
print('PREAUDIT_WAVE6_ASSURANCE: PASS 26/26')
