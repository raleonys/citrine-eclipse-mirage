#!/usr/bin/env python3
from pathlib import Path
import json,sys
r=Path(__file__).resolve().parents[1]; errors=[]
cm=(r/'CMakeLists.txt').read_text(); bs=(r/'scripts/build_orchestration/build_production.ps1').read_text(); rc=(r/'resources/preloader_version.rc').read_text(); de=(r/'exports/preloader_l.def').read_text()
required_rc=[
 'FILEVERSION 16354,19835,39666,21509','PRODUCTVERSION 46411,29569,417,3778','FILEFLAGSMASK 0x0000003fL','FILEOS 0x00040000L','FILETYPE 0x00000002L','FILESUBTYPE 0x6a1aae62L',
 'BLOCK "040904b0"','VALUE "Translation", 0x0409, 1200',
 'VALUE "FileDescription", "EA Javelin Anticheat\\0"',
 'VALUE "ProductVersion", "3.9.5\\0"',
]
for x in required_rc:
    if x not in rc: errors.append('missing VERSIONINFO token '+x)
# Strict resource allowlist: no manifest/icon/string-table resource declarations.
for forbidden in ('RT_MANIFEST','ICON','RT_ICON','RT_GROUP_ICON','STRINGTABLE'):
    if forbidden in rc: errors.append('non-allowlisted resource '+forbidden)
for token in ('/SUBSYSTEM:WINDOWS,6.00','/OSVERSION:6.0','/DEBUG:NONE','/LTCG:OFF','/NOCOFFGRPINFO'):
    if token not in cm: errors.append('missing linker policy '+token)
if 'resources/preloader_version.rc' not in cm: errors.append('VERSIONINFO resource not in normalized target')
if 'LIBRARY "preloader.unsigned.dll"' not in de: errors.append('internal export identity source mismatch')
for token in ('ursynex_golden_finalizer.py','verify_aryra_golden_layout.py','verify_mustavex_golden_metadata.py','verify_corvexon_golden_trail.py','preloader.unsigned.dll'):
    if token not in bs: errors.append('build pipeline missing '+token)
# Project release identity must not be a compile definition / binary build identity.
for token in ('PRELOADER_L_VERSION_CODE','PRELOADER_L_BUILD_TAG'):
    if token in cm or token in bs: errors.append('embedded project identity mechanism remains: '+token)
for rel in ('src/runtime/build_identity.cpp','include/preloader_l/runtime/build_identity.hpp'):
    if (r/rel).exists(): errors.append('embedded project build identity file remains: '+rel)
# Golden policy files and permanent read-only validators.
for rel in ('metadata/URSYNEX_GOLDEN_FINALIZER.json','docs/URSYNEX_GOLDEN_FINALIZER.md','scripts/ursynex_golden_finalizer.py','metadata/ARYRA_GOLDEN_LAYOUT.json','metadata/MUSTAVEX_GOLDEN_METADATA.json','scripts/verify_aryra_golden_layout.py','scripts/verify_mustavex_golden_metadata.py','metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json','docs/LUTRAVEX_GOLDEN_NAMING_SCHEME.md','scripts/check_lutravex_golden_naming.py','metadata/CORVEXON_GOLDEN_TRAIL.json','docs/CORVEXON_GOLDEN_TRAIL.md','scripts/verify_corvexon_golden_trail.py'):
    if not (r/rel).is_file(): errors.append('missing golden protocol component: '+rel)
aryra=(r/'metadata/ARYRA_GOLDEN_LAYOUT.json').read_text() if (r/'metadata/ARYRA_GOLDEN_LAYOUT.json').is_file() else ''
mustavex=(r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text() if (r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').is_file() else ''
for token in ('\".text\"','\".rdata\"','\".data\"','\".pdata\"','\".rsrc\"','\".reloc\"'):
    if token not in aryra: errors.append('ARYRA policy missing '+token)
for token in ('0xCD268EB1','preloader.unsigned.dll','MUSTAVEX Golden Metadata'):
    if token not in mustavex: errors.append('MUSTAVEX policy missing '+token)

# Golden policy semantics must remain narrow and explicit.
if (r/'metadata/ARYRA_GOLDEN_LAYOUT.json').is_file():
    a=json.loads((r/'metadata/ARYRA_GOLDEN_LAYOUT.json').read_text())
    if a.get('sections_in_order')!=['.text','.rdata','.data','.pdata','.rsrc','.reloc']:
        errors.append('ARYRA section policy drift')
    if a.get('section_count')!=6: errors.append('ARYRA section count drift')
    # HF12/MT: a single fail-closed optional MSVC UCRT .fptable profile.
    required_static={
        'profile':'HF12_MSVC_STATIC_UCRT_FPTABLE_EXACT',
        'requires_shipping_runtime':'/MT (MultiThreaded)',
        'section_count':7,
        'sections_in_order':['.text','.rdata','.data','.pdata','.fptable','.rsrc','.reloc'],
        'fptable':{
            'name':'.fptable', 'index':4, 'virtual_size':'0x100', 'raw_size':'0x200',
            'characteristics':'0xC0000040', 'require_zero_initialized_raw_bytes':True,
            'require_nonexecutability':True, 'require_read_and_write':True
        }
    }
    if a.get('static_crt_fptable_profile')!=required_static:
        errors.append('ARYRA static UCRT .fptable exact profile drift')
    static_crt=json.loads((r/'metadata/CRT_STATIC_CONTRACT.json').read_text())
    if static_crt.get('target_modes',{}).get('shipping')!=required_static['requires_shipping_runtime']:
        errors.append('ARYRA .fptable is not bound to /MT shipping runtime')
    validator=(r/'scripts/verify_aryra_golden_layout.py').read_text()
    for token in ('static_crt_fptable_profile','fptable_seen',"'.fptable raw bytes not zero initialized'", "'.fptable Characteristics=", "'.fptable VirtualSize=", "'.fptable SizeOfRawData=", 'shipping /MT contract'):
        if token not in validator: errors.append('ARYRA .fptable runtime guard missing '+token)
    if a.get('exact_rva_size_offset_parity') is not False: errors.append('ARYRA exact RVA/size/offset parity must remain disabled')
if (r/'metadata/CORVEXON_GOLDEN_TRAIL.json').is_file():
    c=json.loads((r/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())
    if c.get('policy')!='CORVEXON Golden Trail': errors.append('CORVEXON policy drift')
    if c.get('mode')!='READ_ONLY_FAIL_CLOSED_POST_FINALIZATION_ACCEPTANCE': errors.append('CORVEXON mode drift')
    wanted=[('DllMain',1),('SURGE',2),('DllShutdown',3)]
    exports=c.get('shipping_exports',[])
    got=[(x.get('name'),x.get('ordinal')) for x in exports]
    if got!=wanted: errors.append('CORVEXON export ABI drift')
    dllmain=next((x for x in exports if x.get('name')=='DllMain'),{})
    if dllmain.get('must_not_equal_entrypoint') is not True: errors.append('CORVEXON CRT entrypoint/DllMain separation drift')
    imports=c.get('import_policy',{})
    if imports.get('restricted_named_imports')!={'MSWSOCK.DLL':['AcceptEx','GetAcceptExSockaddrs']}:
        errors.append('CORVEXON Asio MSWSOCK named import policy drift')
    if 'MSWSOCK.DLL' not in imports.get('allowed_modules',[]):
        errors.append('CORVEXON Asio MSWSOCK module missing')
    if 'MSWSOCK.DLL' in imports.get('required_modules',[]):
        errors.append('CORVEXON MSWSOCK cannot be mandatory for all binary profiles')
    cv_import=(r/'scripts/verify_corvexon_golden_trail.py').read_text()
    if 'restricted_named_imports' not in cv_import or 'def import_names(' not in cv_import:
        errors.append('CORVEXON MSWSOCK import-name enforcement missing')
    crt=c.get('import_policy',{}).get('crt_runtime_policy',{})
    if crt.get('shipping_runtime')!='/MT (MultiThreaded)': errors.append('CORVEXON CRT runtime policy drift')
    if c.get('require_entrypoint_executable_section') is not True: errors.append('CORVEXON executable-entrypoint section requirement drift')
    gs=c.get('import_policy',{}).get('gs_runtime_policy',{})
    if gs.get('require_pe_load_config_security_cookie') is not True: errors.append('CORVEXON /GS cookie requirement drift')
    if gs.get('require_security_cookie_writable_section') is not True: errors.append('CORVEXON writable /GS cookie section requirement drift')
    integ=c.get('integrity_policy',{})
    if integ.get('require_pe_checksum_valid') is not True or integ.get('checksum_owner')!='URSYNEX Golden Finalizer':
        errors.append('CORVEXON PE checksum verification policy drift')
    cv=(r/'scripts/verify_corvexon_golden_trail.py').read_text()
    if 'compute_pe_checksum' not in cv or 'PE CheckSum=' not in cv:
        errors.append('CORVEXON PE checksum read-only verification missing')

if (r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').is_file():
    m=json.loads((r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())
    if m.get('strict_recovery_allowlist')!=['#1','#2','#3','#5','#6','#7','#10','#11','#18','#36']:
        errors.append('MUSTAVEX recovery allowlist drift')
    if m.get('intentionally_stripped_allowlist_items')!=['#13 RSDS','#14 REPRO']:
        errors.append('MUSTAVEX stripped-item policy drift')
    if m.get('forbidden_project_identity_tokens')!=['3.2.3','MELOTRIX','melotrix','preloader_l-3.2.3']:
        errors.append('MUSTAVEX current-release identity exclusion drift')
    mv=(r/'scripts/verify_mustavex_golden_metadata.py').read_text()
    if 'forbidden_project_identity_tokens' not in mv:
        errors.append('MUSTAVEX current-release identity exclusion is not enforced')
    if 'string_tables' not in mv or "expected_table=req['versioninfo']['string_table']" not in mv:
        errors.append('MUSTAVEX VersionInfo StringTable key policy is not enforced')
# URSYNEX is the sole bounded mutating Golden step; all downstream validators are read-only.
urs=(r/'scripts/ursynex_golden_finalizer.py').read_text() if (r/'scripts/ursynex_golden_finalizer.py').is_file() else ''
if 'atomic_replace_bytes(' not in urs or 'struct.pack_into(' not in urs or 'os.replace(' not in urs: errors.append('URSYNEX atomic mutating policy missing')
u=json.loads((r/'metadata/URSYNEX_GOLDEN_FINALIZER.json').read_text())
if u.get('mutation_values')!={'linker_version':[14,0],'timestamp':'0xCD268EB1','rich_header':'ABSENT'}:
    errors.append('URSYNEX structured mutation policy drift')
ui=u.get('integrity_finalization',{})
if ui.get('pe_checksum')!='RECOMPUTE_AFTER_APPROVED_MUTATIONS' or ui.get('downstream_validation')!='CORVEXON_READ_ONLY_REQUIRED':
    errors.append('URSYNEX PE checksum policy drift')
if 'validate_policy_bindings' not in urs or 'compute_pe_checksum' not in urs:
    errors.append('URSYNEX policy binding/checksum implementation missing')
if 'PE OptionalHeader.CheckSum recomputed after approved mutations' not in u.get('approved_mutations',[]):
    errors.append('URSYNEX checksum mutation is not explicitly allowlisted')
# Read-only golden validators must never write/patch the binary.
for rel in ('scripts/verify_aryra_golden_layout.py','scripts/verify_mustavex_golden_metadata.py','scripts/verify_corvexon_golden_trail.py'):
    if (r/rel).is_file():
        t=(r/rel).read_text()
        for forbidden in ('write_bytes(', 'struct.pack_into(', "open(path,'wb'", 'open(path,"wb"'):
            if forbidden in t: errors.append(rel+' is not read-only: '+forbidden)
# Production publication is invalidated before compile and is republished only by
# same-directory staged hash verification + atomic rename after all gates.
if "$GameDll = Join-Path $Dist 'preloader_l.dll'" not in bs or 'Remove-Item -LiteralPath $GameDll -Force' not in bs:
    errors.append('transactional production publication invalidation missing')
else:
    if bs.index('Remove-Item -LiteralPath $GameDll -Force') > bs.index('[1/6] MSVC Build'):
        errors.append('previous published DLL is not invalidated before compile')
for token in ("$PublishTemp = Join-Path $Dist ('.preloader_l.publish.'",'Staged publication hash mismatch','[System.IO.File]::Move($PublishTemp,$GameDll)','Published DLL hash mismatch after atomic rename'):
    if token not in bs: errors.append('atomic publication contract missing '+token)
if '[System.IO.File]::Move($PublishTemp,$GameDll)' in bs and bs.index('[System.IO.File]::Move($PublishTemp,$GameDll)') < bs.index('check_data_footprint.py'):
    errors.append('production DLL is published before all validation gates complete')
if 'Acquire-ExclusiveBuildLock' not in bs:
    errors.append('production build checkout lock missing')

# Enforce complete canonical order and bind visible labels to the invoked tools.
order=['[1/6] MSVC Build','ursynex_golden_finalizer.py','verify_aryra_golden_layout.py','verify_mustavex_golden_metadata.py','verify_corvexon_golden_trail.py','check_falcion_golden_cleaning.py','check_data_footprint.py','[System.IO.File]::Move($PublishTemp,$GameDll)']
for log_name in ('01_URSYNEX_GOLDEN_FINALIZER.log','02_ARYRA_GOLDEN_LAYOUT.log','03_MUSTAVEX_GOLDEN_METADATA.log','04_CORVEXON_GOLDEN_TRAIL.log','05_FALCION_GOLDEN_CLEANING.log'):
    if log_name not in bs: errors.append('Golden log standard missing '+log_name)
for label in ('[2/6] URSYNEX Golden Finalizer','[3/6] ARYRA Golden Layout','[4/6] MUSTAVEX Golden Metadata','[5/6] CORVEXON Golden Trail','[6/6] FALCION Golden Cleaning'):
    if label not in bs: errors.append('canonical Golden stage label missing '+label)
try:
    idx=[bs.index(x) for x in order]
    if idx!=sorted(idx): errors.append('golden protocol build order drift')
except ValueError:
    errors.append('golden protocol build order token missing')
canonical='MSVC Build -> URSYNEX Golden Finalizer -> ARYRA Golden Layout -> MUSTAVEX Golden Metadata -> CORVEXON Golden Trail -> FALCION Golden Cleaning'
if canonical not in bs: errors.append('canonical six-stage success vocabulary missing')
if 'MSVC BUILD + GOLDEN PROTOCOLS' in bs: errors.append('superseded aggregate success vocabulary remains')
if (r/'scripts/verify_pe_allowlist.py').exists(): errors.append('superseded PE allowlist verifier remains')
if (r/'docs/PE_METADATA_POLICY.md').exists(): errors.append('superseded PE metadata policy doc remains')
# Normalized three-word Golden policy names; compatibility protocol/controller labels are forbidden in active policy files.
normalized_names=('URSYNEX Golden Finalizer','ARYRA Golden Layout','MUSTAVEX Golden Metadata','CORVEXON Golden Trail','FALCION Golden Cleaning')
active_text='\n'.join((r/x).read_text(errors='replace') for x in ('scripts/build_orchestration/build_production.ps1','docs/GOLDEN_PROTOCOLS.md','docs/URSYNEX_GOLDEN_FINALIZER.md','docs/CORVEXON_GOLDEN_TRAIL.md','docs/FALCION_GOLDEN_CLEANING.md','metadata/MUSTAVEX_GOLDEN_METADATA.json','metadata/CORVEXON_GOLDEN_TRAIL.json'))
for name in normalized_names:
    if name not in active_text: errors.append('normalized Golden name missing: '+name)
compatibility_names=(
    'MUSTAVEX Golden Metadata'+' Protocol',
    'Corvexon Golden Trail'+' Controller',
    'CORVEXON Golden Trail'+' Controller',
    'FALCION Release Hygiene'+' Protocol',
)
for compatibility in compatibility_names:
    if compatibility in active_text: errors.append('compatibility Golden name leaked: '+compatibility)
# Assets intentionally not distributed.
for rel in ('surge.cfg','surge.png'):
    if (r/rel).exists(): errors.append('excluded release asset present: '+rel)
if errors:
    print('GOLDEN_PROTOCOL_SOURCE_POLICY: FAIL'); [print(' -',x) for x in errors]; sys.exit(1)
print('GOLDEN_PROTOCOL_SOURCE_POLICY: PASS')
