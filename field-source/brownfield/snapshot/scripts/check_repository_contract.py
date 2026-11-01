#!/usr/bin/env python3
from pathlib import Path
import re,sys,json,hashlib,ast,subprocess
r=Path(__file__).resolve().parents[1]; errors=[]
cm=(r/'CMakeLists.txt').read_text(); prod=(r/'exports/preloader_l.def').read_text()
# HF12 three-vendor contract is a mandatory fail-closed source/repository gate.
# Reuse the existing vendor test/checker; no new independent QA script.
try:
    _hf12_contract=subprocess.run([sys.executable,str(r/'scripts/test_vendor_sources.py'),'--contract-only'],
        cwd=r,timeout=40,capture_output=True,text=True)
    if _hf12_contract.returncode != 0:
        errors.append('HF12 pinned third-party governance failed: '+(_hf12_contract.stdout+_hf12_contract.stderr)[-1200:])
except Exception as exc:
    errors.append('HF12 pinned third-party governance could not run: '+repr(exc))

# Repository Structure Contract: physical ownership, not documentary intent only.
required_root={'README.md','LICENSE','CMakeLists.txt','CHANGELOG.md','SOURCE_REVISION.md','RELEASE_STATE.json'}
actual_root={p.name for p in r.iterdir() if p.is_file()}
if actual_root!=required_root:
    errors.append('root file contract delta missing='+repr(sorted(required_root-actual_root))+' extra='+repr(sorted(actual_root-required_root)))
# Project license contract: exact complete GPL-3.0-only text.
LICENSE_SHA='3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986'
license_path=r/'LICENSE'
if not license_path.is_file() or hashlib.sha256(license_path.read_bytes()).hexdigest()!=LICENSE_SHA:
    errors.append('root LICENSE is not the complete approved GPL-3.0-only text')
third_party_license=r/'third_party/minhook/upstream/LICENSE.txt'
if not third_party_license.is_file(): errors.append('third-party MinHook license missing')
elif hashlib.sha256(third_party_license.read_bytes()).hexdigest()==LICENSE_SHA: errors.append('project GPL overwrote third-party MinHook license')
required_dirs={'docs','exports','include','metadata','resources','scripts','src','tests','third_party','tools'}
actual_dirs={p.name for p in r.iterdir() if p.is_dir() and p.name not in ('build','dist')}
if actual_dirs!=required_dirs:
    errors.append('root directory ownership delta missing='+repr(sorted(required_dirs-actual_dirs))+' extra='+repr(sorted(actual_dirs-required_dirs)))
for p in r.iterdir():
    if p.is_file() and p.suffix.lower() in {'.cmd','.ps1','.py'}:
        errors.append('root-level tooling leaked: '+p.name)

checks={
 'include root':(r/'include/preloader_l').is_dir(),
 'production def':(r/'exports/preloader_l.def').is_file(),
 'native acceptance def':(r/'exports/preloader_l_native_acceptance.def').is_file(),
 'debug def':(r/'exports/preloader_l_debug.def').is_file(),
 'production ABI DllMain':'DllMain' in prod,
 'production ABI SURGE':'SURGE' in prod,
 'production ABI shutdown':'DllShutdown' in prod,
 'normalized target':'add_library(preloader_l SHARED' in cm,
 'native acceptance option':'PRELOADER_L_NATIVE_ACCEPTANCE' in cm,
 'debug prototype optional':'option(PRELOADER_L_ENABLE_DEBUG_PROTOTYPE' in cm,
 'build script':(r/'scripts/build_orchestration/build_production.ps1').is_file(),
 'native build script':(r/'scripts/build_orchestration/build_native_acceptance.ps1').is_file(),
 'debug build script':(r/'scripts/build_orchestration/build_debug.ps1').is_file(),
 'CRT acceptance build script':(r/'scripts/build_orchestration/build_crt_acceptance.ps1').is_file(),
 'production launcher':(r/'scripts/build_orchestration/build_production.cmd').is_file(),
 'native acceptance launcher':(r/'scripts/build_orchestration/build_native_acceptance.cmd').is_file(),
 'debug launcher':(r/'scripts/build_orchestration/build_debug.cmd').is_file(),
 'CRT acceptance launcher':(r/'scripts/build_orchestration/build_crt_acceptance.cmd').is_file(),
 'CRT transition checker':(r/'scripts/check_crt_transition.py').is_file(),
 'CRT runtime policy checker':(r/'scripts/check_crt_runtime_policy.py').is_file(),
 'Golden protocols doc':(r/'docs/GOLDEN_PROTOCOLS.md').is_file(),
 'FALCION policy doc':(r/'docs/FALCION_GOLDEN_CLEANING.md').is_file(),
 'repository structure contract':(r/'docs/RELEASE_HYGIENE.md').is_file(),
 'direct governance mirror':(r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').is_file(),
 'documentation preservation baseline':(r/'metadata/DOCUMENTATION_PRESERVATION_BASELINE.json').is_file(),
 'URSYNEX policy':(r/'metadata/URSYNEX_GOLDEN_FINALIZER.json').is_file(),
 'URSYNEX policy doc':(r/'docs/URSYNEX_GOLDEN_FINALIZER.md').is_file(),
 'URSYNEX finalizer':(r/'scripts/ursynex_golden_finalizer.py').is_file(),
 'FALCION active checker':(r/'scripts/check_falcion_golden_cleaning.py').is_file(),
 'obsolete generic launcher absent':not (r/'BUILD.cmd').exists(),
 'build guide':(r/'docs/BUILD_ME_FIRST.txt').is_file(),
 'build identity doc':(r/'docs/BUILD_ID.md').is_file(),
 'active manifest':(r/'metadata/ACTIVE_SOURCE_MANIFEST.json').is_file(),
 'changelog':(r/'CHANGELOG.md').is_file(),
 'release state':(r/'RELEASE_STATE.json').is_file(),
 'compatibility comparison shim retained':(r/'src/runtime/freestanding_memory.cpp').is_file(),
 'version resource':(r/'resources/preloader_version.rc').is_file(),
 'ARYRA policy':(r/'metadata/ARYRA_GOLDEN_LAYOUT.json').is_file(),
 'MUSTAVEX policy':(r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').is_file(),
 'ARYRA verifier':(r/'scripts/verify_aryra_golden_layout.py').is_file(),
 'MUSTAVEX verifier':(r/'scripts/verify_mustavex_golden_metadata.py').is_file(),
 'CORVEXON policy':(r/'metadata/CORVEXON_GOLDEN_TRAIL.json').is_file(),
 'CORVEXON verifier':(r/'scripts/verify_corvexon_golden_trail.py').is_file(),
 'CORVEXON policy doc':(r/'docs/CORVEXON_GOLDEN_TRAIL.md').is_file(),
 'LUTRAVEX naming registry':(r/'metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json').is_file(),
 'LUTRAVEX naming policy':(r/'docs/LUTRAVEX_GOLDEN_NAMING_SCHEME.md').is_file(),
 'LUTRAVEX naming verifier':(r/'scripts/check_lutravex_golden_naming.py').is_file(),
 'no top-level evidence':not (r/'evidence').exists(),
 'no full history':not (r/'FULL_HISTORY').exists(),
 'retired hook allocator support TU absent from active tests':not (r/'tests/support/architecture_hook_allocator_model.cpp').exists(),
 'F1451 exception policy support TU':(r/'tests/support/exception_policy.cpp').is_file(),
 'F1451 old hook allocator src path absent':not (r/'src/platform/x64_hook_allocator_model.cpp').exists(),
 'F1451 old exception policy src path absent':not (r/'src/runtime/exception_policy.cpp').exists(),
 'route handler umbrella':(r/'include/preloader_l/handlers/route_handlers.hpp').is_file(),
 'compatibility curio routes header absent':not (r/'include/preloader_l/handlers/curio_routes.hpp').exists(),
 'compatibility curio routes TU absent':not (r/'src/handlers/curio_routes.cpp').exists(),
 'game-info route module':(r/'src/handlers/game_info_routes.cpp').is_file(),
 'profile route module':(r/'src/handlers/profile_routes.cpp').is_file(),
 'auth route module':(r/'src/handlers/auth_routes.cpp').is_file(),
 'store-query route module':(r/'src/handlers/store_routes.cpp').is_file(),
 'achievement route facade':(r/'src/handlers/achievement_routes.cpp').is_file(),
 'achievement grant route module':(r/'src/handlers/achievement_grant_route.cpp').is_file(),
 'achievement query route module':(r/'src/handlers/achievement_query_route.cpp').is_file(),
 'achievement route private contract':(r/'src/handlers/achievement_routes_internal.hpp').is_file(),
 'image route module':(r/'src/handlers/image_route.cpp').is_file(),
 'compatibility store entitlement monolith absent':not (r/'src/config/store_entitlement.cpp').exists(),
 'compatibility store entitlement parser monolith absent':not (r/'src/config/store_entitlement_parser.cpp').exists(),
 'TinyXML2 vendored XML engine':(r/'third_party/tinyxml2/tinyxml2.cpp').is_file(),
 'store value decode module':(r/'src/config/store_values.cpp').is_file(),
 'legacy Store scanners absent':not (r/'src/config/store_entitlements.cpp').exists() and not (r/'src/config/store_xml.cpp').exists(),
 'store document parser module':(r/'src/config/store_parser.cpp').is_file(),
 'store entitlement materialization module':(r/'src/config/store_materialization.cpp').is_file(),
 'store entitlement runtime module':(r/'src/config/store_runtime.cpp').is_file(),
 'store entitlement model bridge module':(r/'src/config/store_service_bridge.cpp').is_file(),
 'store entitlement private internal contract':(r/'src/config/store_internal.hpp').is_file(),
 'compatibility release server monolith absent':not (r/'src/runtime/release_server_windows.cpp').exists(),
 'release worker module':(r/'src/runtime/release_server_worker.cpp').is_file(),
 'release acceptor module':(r/'src/runtime/release_server_acceptor.cpp').is_file(),
 'release server lifecycle module':(r/'src/runtime/release_server_lifecycle.cpp').is_file(),
 'release server private internal contract':(r/'src/runtime/release_server_io_internal.hpp').is_file(),
 'MinHook runtime module':(r/'src/platform/minhook_runtime.cpp').is_file(),
 'MinHook runtime public contract':(r/'include/preloader_l/platform/minhook_runtime.hpp').is_file(),
 'MinHook SAFE_FREEZE patch':(r/'scripts/build_orchestration/PatchMinHookSafeFreeze.cmake').is_file(),
 'legacy inline hook quiescence absent':not (r/'src/platform/inline_hook_quiescence.cpp').exists(),
 'legacy inline hook allocator absent':not (r/'src/platform/inline_hook_allocator_windows.cpp').exists(),
 'legacy inline hook page-trap absent':not (r/'src/platform/inline_hook_page_trap_windows.cpp').exists(),
 'legacy inline hook patch absent':not (r/'src/platform/inline_hook_patch_windows.cpp').exists(),
 'legacy inline hook backend absent':not (r/'src/platform/inline_hook_backend_windows.cpp').exists(),
 'legacy inline hook internal contract absent':not (r/'src/platform/inline_hook_backend_windows_internal.hpp').exists(),
 'model service policy façade':(r/'src/runtime/service_policy.cpp').is_file(),
 'model service value-storage module':(r/'src/runtime/service_values.cpp').is_file(),
 'model service settings module':(r/'src/runtime/service_settings.cpp').is_file(),
 'model service document module':(r/'src/runtime/service_xml.cpp').is_file(),
 'model service private internal contract':(r/'src/runtime/service_policy_internal.hpp').is_file(),
 'socket exchange session façade':(r/'src/protocol/socket_exchange.cpp').is_file(),
 'socket exchange wire module':(r/'src/protocol/socket_exchange_wire.cpp').is_file(),
 'socket exchange frame-step module':(r/'src/protocol/socket_exchange_frame_step.cpp').is_file(),
 'socket exchange transaction module':(r/'src/protocol/socket_exchange_transaction.cpp').is_file(),
 'socket exchange private internal contract':(r/'src/protocol/socket_exchange_internal.hpp').is_file(),
 'runtime configuration façade':(r/'src/config/runtime_configuration.cpp').is_file(),
 'runtime configuration parser module':(r/'src/config/runtime_configuration_parser.cpp').is_file(),
 'runtime configuration storage module':(r/'src/config/runtime_configuration_storage.cpp').is_file(),
 'runtime configuration private internal contract':(r/'src/config/runtime_configuration_internal.hpp').is_file(),
}
for k,v in checks.items():
    if not v: errors.append(k)
# Active launcher naming is semantic lowercase build_<target>.cmd; historical uppercase BUILD_*.cmd is not active.
for p in (r/'scripts/build_orchestration').iterdir():
    if p.is_file() and p.name.startswith('BUILD_') and p.suffix.lower() == '.cmd':
        errors.append('uppercase active build launcher remains: '+p.relative_to(r).as_posix())
st=json.loads((r/'RELEASE_STATE.json').read_text())
HF17_PREPUB=(st.get('release_type')=='HOTFIX_RELEASE' and st.get('official_release_identity')=='preloader_l 3.2.5 SQUALERIS HOTFIX 17' and st.get('hotfix_ordinal')==17)
HF18_PREPUB=(st.get('release_type')=='HOTFIX_RELEASE' and st.get('official_release_identity')=='preloader_l 3.2.5 SQUALERIS HOTFIX 18' and st.get('hotfix_ordinal')==18)
if st.get('release_type')!='HOTFIX_RELEASE' or not isinstance(st.get('hotfix_ordinal'),int) or st['hotfix_ordinal']<1 or st.get('official_release_identity')!=f"preloader_l 3.2.5 SQUALERIS HOTFIX {st['hotfix_ordinal']}": errors.append('invalid active HOTFIX release identity')
rh=st.get('release_hygiene',{})
if rh.get('build_ownership')!='scripts/build_orchestration': errors.append('release_state stale build ownership')
if rh.get('governance_docs')!='docs/PROJECT_GOVERNANCE_CONTRACT.md': errors.append('release_state stale governance docs ownership')
if 'hotfix_release' in st: errors.append('historical hotfix block leaked into current final release state')

# Human-invocable launcher contract is catalog-driven and default-deny.
# Shared dispatcher layers are prohibited, including former launcher_common.cmd.
for retired in ('scripts/build_orchestration/launcher_common.cmd', 'scripts/build_orchestration/build_minhook_vendor.cmd'):
    if (r/retired).exists(): errors.append('retired launcher still present: '+retired)
for bat in (r/'scripts').rglob('*.bat'):
    errors.append('parallel .bat launcher forbidden: '+bat.relative_to(r).as_posix())

# Build scripts must resolve the repository root from scripts/build_orchestration.
for rel in ('scripts/build_orchestration/build_production.ps1','scripts/build_orchestration/build_debug.ps1','scripts/build_orchestration/build_native_acceptance.ps1','scripts/build_orchestration/build_crt_acceptance.ps1'):
    text=(r/rel).read_text(encoding='utf-8',errors='replace')
    if 'Join-Path $PSScriptRoot "..\\.."' not in text and "Join-Path $PSScriptRoot '..\\..'" not in text:
        errors.append('build script repository-root ownership drift: '+rel)

# Script ownership/discoverability: every active script has an exact catalog entry.
if not (r/'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').is_file(): errors.append('human entrypoint registry missing')
active_scripts=[p for p in (r/'scripts').rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.suffix.lower() in {'.py','.ps1','.cmd','.cmake'}]
script_catalog_path=r/'metadata/SCRIPT_OWNERSHIP.json'
if not script_catalog_path.is_file():
    errors.append('active script ownership catalog missing')
    script_catalog={}
else:
    try:
        script_catalog=json.loads(script_catalog_path.read_text(encoding='utf-8'))
    except Exception as exc:
        errors.append('active script ownership catalog unreadable: '+repr(exc)); script_catalog={}
registered={}
for rec in script_catalog.get('scripts',[]):
    rel=rec.get('path',''); owner=rec.get('owner',''); purpose=rec.get('purpose','')
    if not rel or not owner.strip() or not purpose.strip():
        errors.append('invalid active script ownership record: '+repr(rec)); continue
    if rel in registered: errors.append('duplicate active script ownership record: '+rel)
    registered[rel]=rec
active_rel={p.relative_to(r).as_posix() for p in active_scripts}
# Every explicitly human/operator entrypoint has one same-directory,
# same-stem .cmd launcher. Internal automation/helpers are explicitly exempt.
contract=script_catalog.get('human_invocation_contract',{})
if contract.get('launcher_extension')!='.cmd' or contract.get('parallel_bat_forbidden') is not True:
    errors.append('human invocation registry contract drift')
for rel,rec in registered.items():
    if not isinstance(rec.get('human_invocable'),bool):
        errors.append('script invocation classification missing: '+rel); continue
    if rec.get('human_invocable'):
        target=r/rel; launcher=rec.get('launcher')
        expected=target.with_suffix('.cmd').relative_to(r).as_posix()
        if launcher!=expected: errors.append('human entrypoint launcher path/stem drift: '+rel)
        lp=r/launcher if launcher else None
        if not lp or not lp.is_file(): errors.append('human entrypoint launcher missing: '+rel); continue
        txt=lp.read_text(encoding='utf-8',errors='replace').lower()
        if target.name.lower() not in txt: errors.append('human launcher target drift: '+launcher)
        if any(token not in txt for token in ('%~dp0','%*','set "rc=%errorlevel%"','exit /b %rc%','pause >nul')):
            errors.append('human direct-launcher/args/exit/hold contract drift: '+launcher)
        if 'launcher_common.cmd' in txt: errors.append('generic dispatcher in human entrypoint: '+launcher)

for rel in sorted(active_rel-set(registered)):
    errors.append('active script has no owner/purpose: '+rel)
for rel in sorted(set(registered)-active_rel):
    errors.append('script ownership catalog stale entry: '+rel)
for p in active_scripts:
    rel=p.relative_to(r).as_posix(); name=p.name.lower()
    if re.search(r'(?:hotfix|audit[-_]?hotfix|stage[0-9_\-]*|one[-_]?off|scratch|tmp|temp|probe[0-9_\-]*)',name):
        errors.append('checkpoint/one-off active script name: '+rel)
seen={}
for p in active_scripts:
    if p.name in seen: errors.append('duplicate active script basename: '+p.name+' in '+seen[p.name]+' and '+p.relative_to(r).as_posix())
    else: seen[p.name]=p.relative_to(r).as_posix()

# Active filename contract: hard rules are enforced repository-wide.
protected_script_names={'ursynex_golden_finalizer.py','write_acceptance_record.py'}
for p in r.rglob('*'):
    if not p.is_file(): continue
    rel=p.relative_to(r).as_posix()
    if rel.startswith('third_party/'): continue
    base=p.name.lower(); stem=p.stem.lower()
    if re.search(r'(?:hotfix|audit[-_]?hotfix|(?:^|[_-])audit[0-9]+(?:[_-]|$)|preaudit|phase[0-9]|stage[0-9]|wave[0-9]|(?:^|_)w[0-9]+(?:_|$)|(?:^|[_-])rc[0-9]+(?:[_-]|$))',stem):
        errors.append('transient working identifier in active filename: '+rel)
    if re.search(r'(?<![0-9])[0-9]+_[0-9]+_[0-9]+(?![0-9])',stem) or re.search(r'(?<![0-9])[0-9]+\.[0-9]+\.[0-9]+(?![0-9])',stem):
        errors.append('semantic version encoded in active filename: '+rel)
    is_pinned_mock_sdk=(st.get('release_type')=='HOTFIX_RELEASE' and rel=='tests/support/optional_hook_api_shim/windows.h' and hashlib.sha256(p.read_bytes()).hexdigest()=='840ac97a8be0c6519bc8d49d3e6b91686c5bc2b9854ec058b4e90475b6b7b77e')
    if ('windows' in stem or 'x64' in stem) and rel.startswith(('src/','include/','tests/','scripts/','tools/')) and not is_pinned_mock_sdk:
        errors.append('decorative platform/architecture qualifier in active filename: '+rel)
    if rel.startswith('tests/unit/') and p.suffix.lower()=='.cpp' and not base.startswith('test_'):
        errors.append('unit-test filename grammar drift: '+rel)
    if rel.startswith('scripts/') and not rel.startswith('scripts/build_orchestration/') and p.suffix.lower()=='.py':
        if base not in protected_script_names and not (base.startswith('check_') or base.startswith('test_') or base.startswith('verify_')):
            errors.append('QA/tool filename grammar drift: '+rel)

# Current documentation/history contract: live docs only, canonical CHANGELOG, immutable Library provenance.
for forbidden_dir in ['docs/history','docs/historical','docs/research','docs/governance']:
    if (r/forbidden_dir).exists(): errors.append('forbidden archival/research docs directory present: '+forbidden_dir)
if not (r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').is_file(): errors.append('direct governance mirror missing')
try:
    mig=json.loads((r/'metadata/DOCUMENTATION_HISTORY_MIGRATION.json').read_text(encoding='utf-8'))
    if mig.get('coverage_percent')!=100.0: errors.append('documentation migration coverage not 100%')
    if mig.get('mandatory_melotrix_backfill')!='COMPLETE_IN_CHANGELOG_3_2_3_SECTION': errors.append('MELOTRIX mandatory backfill not complete')
    if mig.get('file_count')!=len(mig.get('files',[])): errors.append('documentation migration file-count mismatch')
    if mig.get('record_count')!=sum(x.get('record_count',0) for x in mig.get('files',[])): errors.append('documentation migration record-count mismatch')
    if mig.get('migration_source_archive',{}).get('sha256')!='d7d6d841391bb4733c84513331bce9ea953dd7346dd260b298deb955a2f70169': errors.append('documentation migration source archive hash drift')
    ch=(r/'CHANGELOG.md').read_text(encoding='utf-8',errors='replace')
    required_melotrix=['MinHook-only SAFE-FREEZE architecture','Pre-audit remediation and ownership/lifecycle convergence','Repository, naming and QA architecture','Acceptance-oracle convergence and Windows closure','Historical limitations and non-goals']
    for token in required_melotrix:
        if token not in ch: errors.append('MELOTRIX backfill token missing: '+token)
except Exception as exc:
    errors.append('documentation migration contract unreadable: '+repr(exc))

def _sha_bytes(b): return hashlib.sha256(b).hexdigest()

# Library governance master binding uses full-master hashes; packaged mirror contains normative rules only and excludes Library-only debt/backlog.
project_governance_path=r/'docs/PROJECT_GOVERNANCE_CONTRACT.md'
governance_invariants_path=r/'metadata/PROJECT_GOVERNANCE_INVARIANTS.json'
governance_binding_path=r/'metadata/GOVERNANCE_BINDING.json'
# HF18 normative-only source snapshot bound to the independently pinned 2026-10-19.1 Library master.
# G0 separately requires caller-supplied external master/pointer/parent/source pins;
# this G1 gate tests local mirror integrity and identity without accepting package-self PASS.
_expected_mirror_md='d7c56a57cc496b72f040b9c55fb76126c964d473260b6a76e77c47305b5bb0f2'
_expected_master_md='1be0e614f98dde1657aca9482be03a121c4f801ea724159592ec8f8853c1147a'
_expected_mirror_json='e42afb2dc2bd181c3788ea74e70c2e1442152f0b952c5ac2c5d65e172e09df4c'
_expected_master_json='952af5e29f332f129287b44a7d7ed73cdc9ec35289d8e711e5870243d716085d'
if not project_governance_path.is_file(): errors.append('packaged Library governance MD mirror missing')
elif _sha_bytes(project_governance_path.read_bytes())!=_expected_mirror_md: errors.append('packaged Library governance MD mirror hash drift')
if not governance_invariants_path.is_file(): errors.append('packaged Library governance JSON mirror missing')
elif _sha_bytes(governance_invariants_path.read_bytes())!=_expected_mirror_json: errors.append('packaged Library governance JSON mirror hash drift')
if not governance_binding_path.is_file(): errors.append('release governance binding missing')
else:
    try:
        gb=json.loads(governance_binding_path.read_text(encoding='utf-8'))
        lm=gb.get('library_master',{})
        if lm.get('revision')!='2026-10-19.1': errors.append('release governance master revision drift')
        if lm.get('md_sha256')!=_expected_master_md or lm.get('json_sha256')!=_expected_master_json: errors.append('release governance binding hash drift')
        if gb.get('release_type')!='HOTFIX_RELEASE' or gb.get('release')!=st.get('official_release_identity'): errors.append('release governance binding hotfix identity drift')
        pm=gb.get('packaged_mirror',{})
        if pm.get('policy')!='NORMATIVE_RULES_ONLY_LIBRARY_DEBT_EXCLUDED_FAIL_CLOSED': errors.append('release governance packaged-mirror policy drift')
        if pm.get('md_sha256')!=_expected_mirror_md or pm.get('json_sha256')!=_expected_mirror_json: errors.append('release governance packaged-mirror hash drift')
    except Exception as exc:
        errors.append('release governance binding unreadable: '+repr(exc))
project_governance=project_governance_path.read_text(encoding='utf-8',errors='replace') if project_governance_path.is_file() else ''
for token in [
    'exactly **four official release types**',
    'There is **no** current `FULL SOURCE`, `FULL HISTORY`, `SOURCE CANDIDATE`',
    '`CLEAN_MINIMAL_FIX_STANDARD`',
    'Automatic pre-release REMEDIATION_READY retention',
    'The named Wave model is abolished.',
    'RELEASE_VALIDATION_TIERING',
    'PROACTIVE_HARDENING_STANDARD',
    'The Library governance master is the **authoritative release-independent governance source**',
    'HUMAN_INVOCABLE_SCRIPT_LAUNCHER_CONTRACT',
    'SENSITIVE_MATERIAL_AND_SECRET_EXCLUSION_CONTRACT',
    'ACCEPTANCE_FRESHNESS_AND_OWNERSHIP_LEDGER',
    'CANONICAL_ARTIFACT_PUBLICATION_TRANSACTION_CONTRACT',
    'EVIDENCE_DERIVATION_AND_PROVENANCE_CONTRACT',
]:
    if token not in project_governance: errors.append('packaged Library governance invariant missing: '+token)
# Governance-debt implementation readiness. Library debt itself remains open
# until a later authorized official publication and post-release exact-hash reconciliation.
try:
    _ledger=json.loads((r/'metadata/ACCEPTANCE_FRESHNESS_LEDGER.json').read_text(encoding='utf-8'))
    _pub=json.loads((r/'metadata/PUBLICATION_TRANSACTION.json').read_text(encoding='utf-8'))
    _active_release=json.loads((r/'RELEASE_STATE.json').read_text(encoding='utf-8'))
    if _ledger.get('schema')!='dll-rd-football.acceptance-freshness-ledger.v1': errors.append('acceptance freshness ledger schema drift')
    if set(_ledger.get('status_vocabulary',[]))!={'PENDING','PASS_FRESH','PASS_CARRIED_FORWARD','FAIL','NOT_APPLICABLE','BLOCKED'}: errors.append('acceptance freshness status vocabulary drift')
    ext={'WINDOWS_PRODUCTION_GOLDEN','WINDOWS_NATIVE','CRT_TRANSITION','DEBUG_BUILD','COLLEGEFB27_GAME_NETWORK','MADDEN27_GAME_NETWORK'}
    em={e.get('acceptance_class'):e for e in _ledger.get('entries',[])}
    # Release-independent current HOTFIX contract; no ordinal-specific owner.
    # Preserve historical acceptance receipts, but require current active gates
    # to be truthful and uncommitted until the external pointer transaction.
    ordinal = _active_release.get('hotfix_ordinal')
    if not isinstance(ordinal,int) or ordinal < 1 or _active_release.get('release_type')!='HOTFIX_RELEASE':
        errors.append('active HOTFIX release ordinal invalid')
    if _pub.get('candidate_release_identity')!=_active_release.get('official_release_identity'):
        errors.append('current HOTFIX candidate identity drift')
    if _pub.get('canonical_before',{}).get('sha256')!=_active_release.get('immediate_hotfix_parent',{}).get('sha256'):
        errors.append('current HOTFIX canonical parent drift')
    if _pub.get('canonical_pointer_committed') is not False or _pub.get('post_commit_verified') is not False:
        errors.append('prepublication canonical pointer must remain uncommitted')
    if any(em.get(c,{}).get('status')!='PENDING' or em.get(c,{}).get('carry_forward') is not None for c in ('SOURCE_QA_8_OF_8','FRESH_EXTRACT_SOURCE_QA_8_OF_8')):
        errors.append('current HOTFIX source QA falsely promoted')
    _ca=json.loads((r/'metadata/RELEASE_ACCEPTANCE.json').read_text(encoding='utf-8'))
    if _ca.get('release_class_g3',{}).get('status')!='NOT_APPLICABLE':
        errors.append('current HOTFIX G3 applicability drift')
    # Games are never HOTFIX publication gates (master 2026-10-19.1).
    # Optional run statuses may remain NOT_RUN and must not be upgraded to PASS.
except Exception as exc:
    errors.append('governance-debt machine-state unreadable: '+repr(exc))

# HF08 W01: standalone ShowIGOWindow route identity with existing event semantics.
inventory_alias=(r/'src/router/route_inventory.cpp').read_text(encoding='utf-8',errors='replace')
integration_alias=(r/'src/handlers/integration_routes.cpp').read_text(encoding='utf-8',errors='replace')
inventory_header=(r/'include/preloader_l/router/route_inventory.hpp').read_text(encoding='utf-8',errors='replace')
alias_test=(r/'tests/unit/test_inventory.cpp').read_text(encoding='utf-8',errors='replace')
if 'constexpr size_t route_count = 21;' not in inventory_header:
    errors.append('HF08 expected 21 retained historical descriptors')
for marker in ['"ShowIGOWindow"','UINT64_C(0xc96101f996a3e86d)','RouteId::show_igo_window']:
    if marker not in inventory_alias:
        errors.append('ShowIGOWindow compatibility alias marker missing: '+marker)
if 'show_igo_window = 15' not in inventory_header:
    errors.append('HF08 ShowIGOWindow route ID 15 missing')
if 'span_equals(request.command,"ShowIGOWindow")' not in integration_alias:
    errors.append('ShowIGOWindow handler-name normalization missing')

# SCIURONYX HOTFIX03: ShowIGO/ShowIGOWindow follows Consumer + Donor1 authority.
# The route is connectivity-independent, always renders Success(0), and publishes
# exactly one DOWN event only after a real successful render. Global Online and
# unrelated route policies remain untouched.
render_start=integration_alias.find('HandlerStatus render_show_igo_response')
render_end=integration_alias.find('HandlerStatus handle_show_igo',render_start)
render_body=integration_alias[render_start:render_end]
show_start=integration_alias.find('HandlerStatus handle_show_igo')
show_end=integration_alias.find('HandlerStatus prepare_show_igo_delivery',show_start)
show_body=integration_alias[show_start:show_end]
prepare_start=integration_alias.find('HandlerStatus prepare_show_igo_delivery')
prepare_end=integration_alias.find('HandlerStatus handle_start_download',prepare_start)
prepare_body=integration_alias[prepare_start:prepare_end]
for forbidden in ['integration_offline()', 'show_igo_offline', 'proxy_bad_request']:
    if forbidden in render_body+show_body+prepare_body:
        errors.append('HOTFIX03 ShowIGO regained route-local offline substitution: '+forbidden)
for required in ['route_error_contracts::show_igo_success','span_equals(request.command,"ShowIGOWindow")']:
    if required not in render_body:
        errors.append('HOTFIX03 ShowIGO render contract missing: '+required)
for required in ['render_show_igo_response','publish_show_igo_down_event()','status==HandlerStatus::ok&&output']:
    if required not in show_body:
        errors.append('HOTFIX03 ShowIGO direct success/event contract missing: '+required)
for required in ['render_show_igo_response','prepare_show_igo_down_event(candidate)']:
    if required not in prepare_body:
        errors.append('CERVRAX ShowIGO prepared success/event contract missing: '+required)
show_test=(r/'tests/unit/test_show_igo.cpp').read_text(encoding='utf-8',errors='replace')
for required in ['literal_value("false")','literal_value("0")',r'Code=\"0\"','queued_event_count_for_test() != 1u','handle_show_igo(req, nullptr, 0, &written)','queued_event_count_for_test() != 0u']:
    if required not in show_test:
        errors.append('HOTFIX03 ShowIGO regression contract missing: '+required)
if r'Code=\"-1576468080\"' in show_test:
    errors.append('HOTFIX03 stale ShowIGO offline-error fixture remains')
# CERVRAX send-atomicity: direct rendering stays unchanged, while the socket
# path stages DOWN and commits only after send_all succeeds.
event_queue_h=(r/'include/preloader_l/runtime/event_queue.hpp').read_text(encoding='utf-8',errors='replace')
socket_transaction=(r/'src/protocol/socket_exchange_transaction.cpp').read_text(encoding='utf-8',errors='replace')
socket_transaction_owner=(r/'src/protocol/socket_exchange_transaction_owner_private.hpp').read_text(encoding='utf-8',errors='replace')
socket_transaction_delivery=(r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text(encoding='utf-8',errors='replace')
for required in ['render_show_igo_response','prepare_show_igo_delivery','prepare_show_igo_down_event']:
    if required not in integration_alias and required not in event_queue_h:
        errors.append('CERVRAX ShowIGO prepared-delivery contract missing: '+required)
for required in ['struct QueuedEventSocketDelivery {','measured.route==preloader_l::router::RouteId::show_igo','show_igo_delivery.begin(','show_igo_delivery.commit();']:
    if required not in socket_transaction+socket_transaction_owner+socket_transaction_delivery:
        errors.append('CERVRAX ShowIGO socket transaction missing: '+required)
# HF10 B: both encodes must complete before the first socket byte; ShowIGO
# event consumption is authorized only after its own encrypted frame sends.
# Stage04A moved the actual queued delivery owner across the async callback
# boundary. Keep this gate tied to the real Response->Event state progression:
# sync sender uses the same production prepare/complete API as future IOCP.
response_send=socket_transaction.find('const auto send_status=send_all(ep,wire.bytes,wire.length,&sent)')
write_complete=socket_transaction.find('complete_prepared_transaction_write(',response_send)
response_commit=socket_transaction_delivery.find('transaction->grant_delivery.commit_response();')
event_authorization=socket_transaction.find('PreparedWriteDisposition::send_event')
event_commit=socket_transaction_delivery.find('transaction->grant_delivery.commit_event();')
show_commit=socket_transaction_delivery.find('transaction->show_igo_delivery.commit();',event_commit)
completion_call=socket_transaction_delivery.find('const auto decision=complete_delivery_write(')
if min(response_send,write_complete,response_commit,event_authorization,event_commit,show_commit,completion_call)<0 \
   or not completion_call<response_commit<event_commit<show_commit \
   or not response_send<write_complete \
   or 'if (action==PreparedWriteDisposition::send_event) {wire=next;continue;}' not in socket_transaction \
   or 'transaction->grant_delivery.retry_event();' not in socket_transaction_delivery \
   or 'transaction->show_igo_delivery.retry();' not in socket_transaction_delivery \
   or 'transaction->generic_event_delivery.retry();' not in socket_transaction_delivery:
    errors.append('HF10 B Stage04A Response/Event async owner/queue commit ordering drift')
if 'resolve_queued_event_delivery_for_retry(&queue)' not in socket_transaction_owner:
    errors.append('HF10 B queue retry resolution missing')
if not (r/'tests/unit/test_show_igo_socket_atomicity.cpp').is_file():
    errors.append('CERVRAX ShowIGO socket atomicity regression missing')

# SCIURONYX current route convergence: SetPresence and QueryFriends are
# connectivity-independent under Consumer + Donor1. QueryFriends remains an
# exact success-only response with zero Friend children; no synthetic payload.
set_start=integration_alias.find('HandlerStatus handle_set_presence')
set_end=integration_alias.find('HandlerStatus handle_get_presence_visibility',set_start)
set_body=integration_alias[set_start:set_end]
for forbidden in ['integration_offline()', 'set_presence_offline', 'proxy_bad_request']:
    if forbidden in set_body:
        errors.append('SetPresence regained route-local offline substitution: '+forbidden)
if 'route_error_contracts::set_presence_success' not in set_body:
    errors.append('SetPresence success contract missing')
qf_src=(r/'src/handlers/friends_route.cpp').read_text(encoding='utf-8',errors='replace')
for forbidden in ['offline()', 'query_friends_offline', 'ErrorSuccess']:
    if forbidden in qf_src:
        errors.append('QueryFriends regained route-local offline/error substitution: '+forbidden)
for required in [r'<QueryFriendsResponse Return=\"Success\"/>','zero Friend children']:
    if required not in qf_src:
        errors.append('QueryFriends Donor1 success-only contract missing: '+required)
set_test=(r/'tests/unit/test_set_presence.cpp').read_text(encoding='utf-8',errors='replace')
qf_test=(r/'tests/unit/test_query_friends.cpp').read_text(encoding='utf-8',errors='replace')
for required in ['literal_value("false")',r'Code=\"0\"','-1576468080']:
    if required not in set_test:
        errors.append('SetPresence offline-convergence regression missing: '+required)
for required in ['literal_value("false")',r'<QueryFriendsResponse Return=\"Success\"/>','<Friend ','ErrorSuccess']:
    if required not in qf_test:
        errors.append('QueryFriends zero-Friend/offline convergence regression missing: '+required)
route_contract_text=(r/'src/handlers/route_error_contracts.hpp').read_text(encoding='utf-8',errors='replace')
if 'query_friends_offline' in route_contract_text or 'set_presence_offline' in route_contract_text:
    errors.append('stale route-local offline error contract remains for SetPresence/QueryFriends')
for marker in ['find_route(alias, routes, router::route_count) != nullptr','alias_route == routes + 1']:
    if marker not in alias_test:
        errors.append('ShowIGOWindow focused inventory regression marker missing: '+marker)

# SCIURONYX HOTFIX02: evidence-backed compatibility routes remain separate from
# the recovered 22-entry Primary inventory. Error semantics are typed, route
# descriptions are explicit evidence-bound strings, and QueryChunkStatus has a
# first-class recognized/no-response result distinct from unknown_route.
origin_error=(r/'include/preloader_l/protocol/origin_error.hpp').read_text(encoding='utf-8',errors='replace')
status_header=(r/'include/preloader_l/handlers/handler_status.hpp').read_text(encoding='utf-8',errors='replace')
dispatch_header=(r/'include/preloader_l/core/request_dispatch.hpp').read_text(encoding='utf-8',errors='replace')
socket_exchange=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace')
hf02_test=(r/'tests/unit/test_compatibility_routes.cpp').read_text(encoding='utf-8',errors='replace')
hf02_contracts=(r/'src/handlers/route_error_contracts.hpp').read_text(encoding='utf-8',errors='replace')
hf02_error_mirror=json.loads((r/'metadata/ORIGIN_ERROR_ROUTE_CONTRACTS.json').read_text(encoding='utf-8'))
hf02_error_metadata=(r/'metadata/ORIGIN_ERROR_ROUTE_CONTRACTS.json').read_text(encoding='utf-8',errors='replace')
for marker in [
    'constexpr size_t compatibility_route_count = 6;',
    'RouteId::get_presence_visibility', 'RouteId::are_chunks_installed', 'RouteId::restart_game',
    'RouteId::get_voip_status', 'RouteId::query_chunk_status', 'RouteId::get_block_list',
    'UINT64_C(0xf21c42469f19cfa6)', 'UINT64_C(0xdfae2d60bb09340b)',
    'UINT64_C(0xfe55c0b14d39b446)', 'UINT64_C(0x627425d75cc9b05f)', 'UINT64_C(0x7edda9d82a5429b2)', 'UINT64_C(0xc1644021c24b1ff0)',
]:
    if marker not in inventory_header and marker not in inventory_alias:
        errors.append('HOTFIX02 compatibility-route contract missing: '+marker)
for marker in [
    'not_implemented) == -1593835518',
    'proxy_bad_request) == -1576468080',
    'no_service) == -1577058293',
    'origin_error_is_success',
    'origin_error_name',
    'origin_error_area',
    'origin_error_level',
]:
    if marker not in origin_error:
        errors.append('HOTFIX02 OriginError contract missing: '+marker)
for marker in [
    'struct ErrorSuccessContract',
    'start_download_success', 'restart_game_pending', 'get_voip_status_not_implemented',
    'get_auth_code_offline', 'check_permission_offline',
    'Requested a download of a DLC offer.', 'Origin is waiting for the game to shutdown.',
    'The function you are calling is not implemented.', 'You need to be online to get an AuthCode.',
]:
    if marker not in hf02_contracts:
        errors.append('HOTFIX02 route ErrorSuccess profile missing: '+marker)
for marker in [
    '"global_description_inference": false',
    '"same_eventual_outcome_is_semantic_equivalence": false',
    '"RestartGame"', '"GetVoipStatus"', '"StartDownload"', '"QueryChunkStatus"',
    '"serialize_rust_corollary": false',
]:
    if marker not in hf02_error_metadata:
        errors.append('HOTFIX02 machine error-contract mirror missing: '+marker)
if hf02_error_mirror.get('typed_codes',{}).get('Pending') != 1 or hf02_error_mirror.get('typed_codes',{}).get('NotImplemented') != -1593835518:
    errors.append('HOTFIX02 machine error-contract mirror drift')
if hf02_error_mirror.get('routes',{}).get('QueryChunkStatus',{}).get('outcome') != 'NO_RESPONSE_REQUIRED':
    errors.append('HOTFIX02 machine no-response route contract drift')
if hf02_error_mirror.get('authority_policy',{}).get('universal_description_from_code') is not False:
    errors.append('HOTFIX02 machine route-description policy drift')
if 'no_response_required' not in status_header or 'no_response_required' not in dispatch_header:
    errors.append('HOTFIX02 explicit no-response status missing')
if 'DispatchStatus::no_response_required' not in socket_exchange:
    errors.append('HOTFIX02 persistent-session no-response continuation missing')
for marker in [
    'Origin is waiting for the game to shutdown.',
    'The function you are calling is not implemented.',
    'Requested a download of a DLC offer.',
    'DispatchStatus::no_response_required',
]:
    if marker not in integration_alias and marker not in hf02_test:
        errors.append('HOTFIX02 route-specific response contract missing: '+marker)
for marker in ['"GetBlockList"','get_block_list_body',r'<GetBlockListResponse Return=\"Success\"/>']:
    if marker not in inventory_alias and marker not in integration_alias and marker not in hf02_test:
        errors.append('CERVRAX GetBlockList contract missing: '+marker)
if '<User' not in hf02_test or 'has(out,"<User")' not in hf02_test:
    errors.append('CERVRAX GetBlockList zero-User regression missing')

if 'integration_offline()?preloader_l::protocol::OriginError::proxy_bad_request' in integration_alias and 'handle_start_download' in integration_alias:
    # Guard the route specifically below instead of rejecting other integration routes' offline behavior.
    start_pos=integration_alias.find('HandlerStatus handle_start_download')
    next_pos=integration_alias.find('HandlerStatus handle_set_presence',start_pos)
    start_body=integration_alias[start_pos:next_pos]
    if 'integration_offline' in start_body or 'proxy_bad_request' in start_body:
        errors.append('HOTFIX02 StartDownload regained generic offline substitution')

# RC07 closure guard: stale tests must not reassert RC06-removed offline route errors.
original_consistency=(r/'tests/unit/test_route_defaults.cpp').read_text(encoding='utf-8',errors='replace')
query_whitespace=(r/'tests/unit/test_query_achievements_all_whitespace.cpp').read_text(encoding='utf-8',errors='replace')
stale_fixture_tokens=[
    'if(!has(out,"Unknown Error") || !has(out,"Code=\\"-1\\"")) return 23;',
    'if(!has(out,"Code=\\"-1576468080\\"")) return 25;',
    'HandlerStatus::ok || !has(out,"-1576468080")) return 26;',
]
for token in stale_fixture_tokens:
    if token in original_consistency:
        errors.append('stale RC06 offline-route fixture reintroduced: '+token)
if '!has(out,"Code=\\"-1576468080\\"") || has(out,"Id=\\"ZERO\\"")' in query_whitespace:
    errors.append('stale QueryAchievements offline-error fixture reintroduced')

# Source-health architecture heuristic: line count alone is never an error.
# Known files with 3+ independently reviewed responsibilities are surfaced as
# architecture-review targets while semantic remediation remains separate.
architecture_registry_path=r/'metadata/ARCHITECTURE_REVIEW_REGISTRY.json'
architecture_review_registry={}
if not architecture_registry_path.is_file():
    errors.append('architecture review registry missing')
else:
    try:
        architecture_review_registry=json.loads(architecture_registry_path.read_text(encoding='utf-8')).get('entries',{})
    except Exception as exc:
        errors.append('architecture review registry unreadable: '+repr(exc))
if not architecture_review_registry:
    errors.append('architecture review registry empty')
for rel,responsibilities in architecture_review_registry.items():
    path=r/rel
    if not path.is_file():
        errors.append('architecture review target missing: '+rel); continue
    if len(responsibilities)<3:
        errors.append('architecture review target lacks responsibility inventory: '+rel); continue
    line_count=sum(1 for _ in path.open(encoding='utf-8',errors='replace'))
    if line_count>=500:
        print(f'ARCHITECTURE_REVIEW: {rel} ({line_count} lines; {len(responsibilities)} responsibilities)')

# 3.2.3 MELOTRIX RC03: MinHook owns low-level executable patching. The project
# runtime owns lifecycle, gate publication, one-transaction commit/rollback and
# retained ownership only; legacy allocator/VEH/page-protection patch engines stay absent.
minhook_runtime=(r/'src/platform/minhook_runtime.cpp').read_text(encoding='utf-8',errors='replace')
minhook_contract=(r/'include/preloader_l/platform/minhook_runtime.hpp').read_text(encoding='utf-8',errors='replace')
for forbidden_token in ['VirtualAlloc(', 'AddVectoredExceptionHandler(', 'VirtualProtect(', 'FlushInstructionCache(',
                        'protect_range_segments(', 'plan_x64_relocation(']:
    if forbidden_token in minhook_runtime or forbidden_token in minhook_contract:
        errors.append('MinHook runtime regained low-level patch-engine responsibility: '+forbidden_token)
for required in ['begin_minhook_batch_transaction()', 'commit_minhook_batch_transaction()',
                 'rollback_minhook_batch_transaction()', 'MinHookBatchTransactionStatus::rollback_incomplete']:
    if required not in minhook_runtime:
        errors.append('MinHook transaction marker missing: '+required)

# CASTORADRON HOTFIX02: keep the model-service public policy TU as a thin
# orchestration/façade layer. Ownership-heavy implementations belong to the
# dedicated private modules above.
model_service_policy=(r/'src/runtime/service_policy.cpp').read_text(encoding='utf-8',errors='replace')
for forbidden_token in ['bool rehash_settings_map(', 'bool assign_model_xml_document(',
                        'uintptr_t load_wide_path_pointer_bits(', 'bool assign_string(']:
    if forbidden_token in model_service_policy:
        errors.append('model-service policy façade regained implementation responsibility: '+forbidden_token)

# CASTORADRON HOTFIX03: the public socket-exchange TU owns session policy only.
# Bounded stream scanning and the measure/render/encode/send transaction remain
# in their dedicated private modules so later protocol fixes land at one stable boundary.
socket_exchange_facade=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace')
for forbidden_token in ['WireScanStatus scan_wire_frame(', 'struct GrantSocketDelivery {',
                        'struct HeapBytes {', 'dispatch_plaintext_request(']:
    if forbidden_token in socket_exchange_facade:
        errors.append('socket-exchange façade regained implementation responsibility: '+forbidden_token)

# CASTORADRON HOTFIX06: runtime configuration remains split into public
# lifecycle/model-service orchestration, Config2 parsing, and transient storage.
runtime_configuration_facade=(r/'src/config/runtime_configuration.cpp').read_text(encoding='utf-8',errors='replace')
runtime_configuration_parser=(r/'src/config/runtime_configuration_parser.cpp').read_text(encoding='utf-8',errors='replace')
runtime_configuration_storage=(r/'src/config/runtime_configuration_storage.cpp').read_text(encoding='utf-8',errors='replace')
for forbidden_token in ['enum class TokenKind', 'ParseCode parse_object(', 'bool reserve_arena(', 'bool put_setting(']:
    if forbidden_token in runtime_configuration_facade:
        errors.append('runtime-configuration façade regained parser/storage responsibility: '+forbidden_token)
for forbidden_token in ['void* transient_alloc(', 'bool reserve_arena(', 'bool put_setting(']:
    if forbidden_token in runtime_configuration_parser:
        errors.append('runtime-configuration parser regained storage responsibility: '+forbidden_token)
for forbidden_token in ['enum class TokenKind', 'SectionPolicy reserved_key_policy(', 'ParseCode parse_config2(']:
    if forbidden_token in runtime_configuration_storage:
        errors.append('runtime-configuration storage regained parser responsibility: '+forbidden_token)

# HF11 supersedes HOTFIX04's retired XML scanner and entitlement TU: one pinned
# TinyXML2 parser + explicit Store value/runtime/materialization ownership.
store_value_decode=(r/'src/config/store_values.cpp').read_text(encoding='utf-8',errors='replace')
store_document_parser=(r/'src/config/store_parser.cpp').read_text(encoding='utf-8',errors='replace')
store_runtime=(r/'src/config/store_runtime.cpp').read_text(encoding='utf-8',errors='replace')
store_internal=(r/'src/config/store_internal.hpp').read_text(encoding='utf-8',errors='replace')
store_model_bridge=(r/'src/config/store_service_bridge.cpp').read_text(encoding='utf-8',errors='replace')
store_materialization=(r/'src/config/store_materialization.cpp').read_text(encoding='utf-8',errors='replace')
for rel in ['src/config/store_xml.cpp','src/config/store_entitlements.cpp','src/xml/request_scanner.cpp','src/xml/request_internal.hpp']:
    if (r/rel).exists(): errors.append('HF11 retired parser/bridge returned: '+rel)
for required in ['tinyxml2::XMLDocument','FirstChildElement(', 'ParseCode parse_entitlement(', 'ParseCode parse_document(']:
    if required not in store_document_parser: errors.append('HF11 Store DOM transaction/parser surface missing: '+required)
for required in ['ParseCode decode_quartets(', 'ParseCode decode_hex(']:
    if required not in store_value_decode: errors.append('HF11 Store value decoder absent: '+required)
if 'ParseCode decode_quartets(' in store_document_parser or 'ParseCode decode_hex(' in store_document_parser:
    errors.append('HF11 Store duplicated primitive decoder in DOM projector')
if 'tinyxml2::XMLDocument' in store_value_decode:
    errors.append('HF11 Store value decoder owns extra XML parser')
# HF17 parser state is still explicit; its names changed during TinyXML2 migration.
store_ownership_surface='\n'.join([store_internal,store_runtime,store_model_bridge,store_materialization,store_value_decode,store_document_parser])
for forbidden_token in ['g_parse_store','parse_store()']:
    if forbidden_token in store_ownership_surface: errors.append('HF17 implicit Store parse ownership returned: '+forbidden_token)
for required_token in ['void reset_store(StoreState& state)', 'ParseCode parse_document(StoreState& state', 'ParseCode parse_store(StoreState& state', 'ParseCode parse_entitlement(StoreState& state']:
    if required_token not in store_ownership_surface: errors.append('HF17 explicit StoreState ownership marker missing: '+required_token)

# CASTORADRON HOTFIX07: the surge VM model is research/test support, not
# shipping or debug runtime source. Its implementation must be linked and
# executed by release QA rather than merely syntax-checked through a public header.
old_surge_paths = [
    r/'src/research/surge_vm_model.cpp',
    r/'include/preloader_l/research/surge_vm_model.hpp',
]
for old_path in old_surge_paths:
    if old_path.exists():
        errors.append('surge VM model regained production-like source topology: '+old_path.relative_to(r).as_posix())
surge_support_cpp = r/'tests/support/research/surge_vm.cpp'
surge_support_hpp = r/'tests/support/research/surge_vm.hpp'
surge_test = r/'tests/unit/test_surge_vm.cpp'
surge_runtime_gate = r/'scripts/test_unit_syntax.py'
for required_path in [surge_support_cpp, surge_support_hpp, surge_test, surge_runtime_gate]:
    if not required_path.is_file():
        errors.append('missing surge VM research test component '+required_path.relative_to(r).as_posix())
release_qa=(r/'scripts/test_release_qa.py').read_text(encoding='utf-8',errors='replace')
if "'test_unit_syntax.py'" not in release_qa:
    errors.append('unit/link runtime gate missing from release QA')
# S0885: match a live top-level subprocess invocation, never inert prose.
try:
    qa_tree = ast.parse(release_qa)
    def active_behavior_call(node):
        if not isinstance(node, ast.Expr) or not isinstance(node.value, ast.Call):
            return False
        call = node.value
        if not (isinstance(call.func, ast.Attribute) and call.func.attr == 'run'
                and isinstance(call.func.value, ast.Name) and call.func.value.id == 'subprocess'):
            return False
        words = ast.dump(call, include_attributes=False)
        return all(repr(token) in words for token in ('behavior','--shard-index','--shard-count','AUTHORITATIVE_SHARD_COUNT'))
    if not any(active_behavior_call(node) for node in qa_tree.body):
        errors.append('release QA behavioral regression invocation absent/inert')
except SyntaxError:
    errors.append('release QA runner syntax invalid')
# MUSTORIN current policy: authoritative release/source QA is exactly eight
# deterministic shards. The release runner must fail closed rather than fall
# back to a monolithic aggregate invocation.
for token in [
    'AUTHORITATIVE_SHARD_COUNT=8',
    'authoritative release QA requires exactly 8 deterministic shards',
    'qa_shard_count != AUTHORITATIVE_SHARD_COUNT',
    "'--shard-count',str(AUTHORITATIVE_SHARD_COUNT)",
]:
    if token not in release_qa:
        errors.append('8-shard release-QA contract marker missing: '+token)
if 'behavioral_shards=4' in release_qa or 'qa_shard_count is None' in release_qa:
    errors.append('monolithic/4-shard release-QA fallback remains active')
behavioral_entry=(r/'scripts/test_regressions.py').read_text(encoding='utf-8',errors='replace')
behavioral_qa=(r/'scripts/test_minhook_regressions.py').read_text(encoding='utf-8',errors='replace')
if "test_minhook_regressions.py" not in behavioral_entry:
    errors.append('canonical behavioral entrypoint no longer forwards to MinHook behavioral suite')
for token in [
    '_AUTHORITATIVE_SHARD_COUNT = 8',
    'full behavioral QA requires exactly 8 deterministic shards',
    '_shard_count != _AUTHORITATIVE_SHARD_COUNT',
]:
    if token not in behavioral_qa:
        errors.append('8-shard behavioral-QA contract marker missing: '+token)
if "BEHAVIORAL_REGRESSIONS: PASS (91/91)" in behavioral_qa:
    errors.append('unsharded behavioral aggregate PASS path remains active')
build_me_first=(r/'docs/BUILD_ME_FIRST.txt').read_text(encoding='utf-8',errors='replace')
if build_me_first.count('--shard-count 8 --shard-index') != 8 or '--shard-count 4' in build_me_first:
    errors.append('BUILD_ME_FIRST does not expose exactly eight authoritative release-QA invocations')
release_state_text=(r/'RELEASE_STATE.json').read_text(encoding='utf-8',errors='replace')
release_state_requirements=[
    ('source QA', ['PENDING_8_OF_8_DETERMINISTIC_SHARDS','PASS_8_OF_8_DETERMINISTIC_SHARDS']),
    ('behavioral QA', ['PENDING_92_CASE_INVENTORY_VIA_8_OF_8_SHARDS','PASS_92_CASE_INVENTORY_VIA_8_OF_8_SHARDS']),
    ('fresh-extract QA', ['PENDING_8_OF_8_DETERMINISTIC_SHARDS_FINAL_RELEASE_PACKAGE','PASS_8_OF_8_DETERMINISTIC_SHARDS_FINAL_RELEASE_PACKAGE']),
]
for label,alternatives in release_state_requirements:
    if not any(token in release_state_text for token in alternatives):
        errors.append('current release state 8-shard status wording missing for '+label)
for rel in ['README.md','SOURCE_REVISION.md','docs/RELEASE_NOTES.md']:
    txt=(r/rel).read_text(encoding='utf-8',errors='replace')
    if '8/8' not in txt:
        errors.append('current document 8-shard policy marker missing: '+rel)
    if rel == 'README.md' and '4/4' not in txt:
        errors.append('current document historical 4/4 preservation marker missing: '+rel)
governance_text=(r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_text(encoding='utf-8',errors='replace')
for token in [
    'Authoritative release/source QA is one logical QA universe partitioned across exactly **8 deterministic shards**;',
    'ZIP/package structure, member set, path safety, embedded manifest/hash consistency, package integrity and final package SHA-256 are validated **once per exact package artifact**',
    'Therefore `fresh-extract 8/8` means **one clean extraction + eight distinct QA partitions against that one extracted tree**.',
]:
    if token not in governance_text:
        errors.append('project governance 8-shard invariant missing: '+token)
# HF03 bounded research is closed but does not authorize social mutation routes.
social_disposition=r/'metadata/SOCIAL_ROUTE_ERROR_CONTRACT_DISPOSITIONS.json'
if not social_disposition.is_file():
    errors.append('HF03 social error-contract machine disposition missing: '+social_disposition.relative_to(r).as_posix())
if social_disposition.is_file():
    social_text=social_disposition.read_text(encoding='utf-8',errors='replace')
    for route in ['RequestFriend','RemoveFriend','BlockUser','UnblockUser','SendInvite']:
        if route not in social_text or 'EVIDENCE_BLOCKED' not in social_text:
            errors.append('HF03 social blocked disposition missing: '+route)
route_inventory=(r/'src/router/route_inventory.cpp').read_text(encoding='utf-8',errors='replace')
# HF03 remote-donor-equivalence remains EVIDENCE_BLOCKED. A separately
# user-approved LOCAL_ONLY source activation requires explicit nonrelease
# provenance, host Config2 authority and the socket send/commit boundary.
local_activation=r/'metadata/LOCAL_SOCIAL_ACTIVATION_CANDIDATE.json'
try:
    activation=json.loads(local_activation.read_text(encoding='utf-8'))
    if activation.get('native_game_acceptance')!='WINDOWS_NATIVE_PASS; GAMES_USER_REPORTED_PASS_UNBOUND':
        errors.append('local social native acceptance falsely promoted')
    if activation.get('remote_social_equivalence')!='OUT_OF_SCOPE':
        errors.append('local social remote equivalence improperly asserted')
    if activation.get('registered_ids')!=[38,39,40,41,42]:
        errors.append('local social registered ID set mismatch')
    if 'LocalSocialRecipients' not in (r/'src/runtime/local_social_host.cpp').read_text():
        errors.append('missing Config2-controlled local social registry')
    if 'commit_local_social_mutation' not in socket_transaction_owner or 'transaction->social_delivery.commit()' not in socket_transaction_delivery:
        errors.append('missing socket social transaction commit owner')
    if 'render_local_social_list' not in (r/'src/core/request_dispatch.cpp').read_text():
        errors.append('missing populated committed reader')
except Exception as exc:
    errors.append('social local activation contract unreadable: '+repr(exc))
for route in ['RequestFriend','RemoveFriend','BlockUser','UnblockUser','SendInvite']:
    if ('"'+route+'"') not in route_inventory:
        errors.append('local social route missing inventory: '+route)
if surge_runtime_gate.is_file():
    surge_gate_text=surge_runtime_gate.read_text(encoding='utf-8',errors='replace')
    for token in ["test_surge_vm.cpp", "surge_vm.cpp", "subprocess.run([str(exe)]"]:
        if token not in surge_gate_text:
            errors.append('surge VM runtime gate no longer compiles/links/executes model: '+token)
if surge_test.is_file() and 'research/surge_vm.hpp' not in surge_test.read_text(encoding='utf-8',errors='replace'):
    errors.append('surge VM unit test no longer consumes test-support research header')

# CASTORADRON HOTFIX08: debug/evidence implementation topology must be honest.
# These TUs are not part of normalized shipping and must not return to
# production-looking runtime/net/platform/handlers locations.
hf08_moves = {
    'src/runtime/active_runtime.cpp': 'src/debug/runtime/runtime_status.cpp',
    'src/runtime/consistency_closure.cpp': 'src/debug/runtime/consistency_closure.cpp',
    'src/runtime/secondary_integration.cpp': 'src/debug/runtime/secondary_integration.cpp',
    'src/runtime/signals.cpp': 'src/debug/runtime/loader_signals.cpp',
    'src/runtime/trace.cpp': 'src/debug/runtime/event_trace.cpp',
    'src/runtime/trace_format.cpp': 'src/debug/runtime/trace_format.cpp',
    'src/runtime/trace_windows.cpp': 'src/debug/runtime/debug_trace.cpp',
    'src/net/ingress.cpp': 'src/debug/net/frame_ingress.cpp',
    'src/net/receive_buffer.cpp': 'src/debug/net/receive_buffer.cpp',
    'src/net/session.cpp': 'src/debug/net/session_state.cpp',
    'src/net/socket_session.cpp': 'src/debug/net/socket_session.cpp',
    'src/handlers/settings_query.cpp': 'src/debug/handlers/settings_query.cpp',
    'src/platform/crypt_observer_diagnostics_windows.cpp': 'src/debug/platform/crypt_observer_diagnostics.cpp',
    'src/platform/optional_host_compat.cpp': 'src/debug/platform/optional_host_compat.cpp',
    'src/platform/optional_host_compat_windows.cpp': 'src/debug/platform/optional_host_scan.cpp',
    'src/platform/crypt_guard.cpp': 'src/debug/evidence/crypt_guard.cpp',
    'src/platform/crypt_path_restore.cpp': 'src/debug/evidence/crypt_path_restore.cpp',
    'src/platform/reconstructed_identity_state.cpp': 'src/debug/evidence/identity_state.cpp',
    'src/platform/x64_provenance.cpp': 'src/debug/evidence/architecture_provenance.cpp',
}
hf08_prod_block=cm.split('add_library(preloader_l SHARED',1)[1].split(')\n',1)[0]
for old_path,new_path in hf08_moves.items():
    if (r/old_path).exists():
        errors.append('HF08 debug/evidence source regained production-like topology: '+old_path)
    if not (r/new_path).is_file():
        errors.append('HF08 relocated debug/evidence source missing: '+new_path)
    if new_path in hf08_prod_block:
        errors.append('HF08 debug/evidence source leaked into production target: '+new_path)
    if cm.count(new_path) != 1:
        errors.append('HF08 debug/evidence source must appear once in CMake debug graph: '+new_path)
old_diag_header=r/'src/platform/crypt_observer_diagnostics_internal.hpp'
new_diag_header=r/'src/debug/platform/crypt_observer_diagnostics_internal.hpp'
if old_diag_header.exists():
    errors.append('HF08 diagnostic private header regained production-like platform topology')
if not new_diag_header.is_file():
    errors.append('HF08 relocated diagnostic private header missing')

# CASTORADRON HOTFIX09: acceptance/evidence topology is test/debug owned.
# Native acceptance exports are test-only additions to the DLL, evidence headers
# must not masquerade as shipping public API, and the weaker generic lifecycle
# host remains retired in favor of the wired shipping black-box acceptance host.
hf09_old_paths = [
    'src/runtime/native_acceptance_windows.cpp',
    'include/preloader_l/platform/safetyhook_inline_abi.hpp',
    'include/preloader_l/runtime/optional_host_callback_contract.hpp',
    'include/preloader_l/runtime/strict_consistency_ledger.hpp',
    'include/preloader_l/runtime/strict_consistency_status.hpp',
    'include/preloader_l/platform/integration_identity.hpp',
    'tools/preloader_l_host/main.cpp',
]
for old_path in hf09_old_paths:
    if (r/old_path).exists():
        errors.append('HF09 retired production-like acceptance/evidence path returned: '+old_path)
hf09_required = [
    'tests/native/native_acceptance_exports.cpp',
    'tests/support/platform/safetyhook_inline_abi.hpp',
    'tests/support/runtime/optional_host_callback_contract.hpp',
    'src/debug/runtime/strict_consistency_ledger.hpp',
    'src/debug/runtime/strict_consistency_status.hpp',
    'src/debug/evidence/identity_constants.hpp',
    'tools/preloader_l_release_host/release_host_main.cpp',
]
for required_path in hf09_required:
    if not (r/required_path).is_file():
        errors.append('HF09 acceptance/evidence component missing: '+required_path)
if 'target_sources(preloader_l PRIVATE\n        tests/native/native_acceptance_exports.cpp\n        exports/preloader_l_native_acceptance.def' not in cm:
    errors.append('HF09 native acceptance exports are not scoped to PRELOADER_L_NATIVE_ACCEPTANCE')
if 'tests/native/native_acceptance_exports.cpp' in hf08_prod_block:
    errors.append('HF09 native acceptance exports leaked into normalized shipping source block')
if '"${CMAKE_CURRENT_SOURCE_DIR}/src/debug"' not in cm:
    errors.append('HF09 debug-private header include root missing')
if 'add_executable(preloader_l_host ' in cm:
    errors.append('HF09 retired generic lifecycle host target returned')
crt_acceptance=(r/'scripts/build_orchestration/build_crt_acceptance.ps1').read_text(encoding='utf-8',errors='replace')
if "@('preloader_l','preloader_l_release_host')" not in crt_acceptance:
    errors.append('HF09 release host is not built by CRT shipping acceptance')
if 'SHIPPING_BLACKBOX' not in crt_acceptance or "& (Join-Path $BlackboxDir 'preloader_l_release_host.exe')" not in crt_acceptance:
    errors.append('HF09 release host is not executed as shipping black-box acceptance')
release_host_text=(r/'tools/preloader_l_release_host/release_host_main.cpp').read_text(encoding='utf-8',errors='strict')
for token in ['recv_nul_frame','challenge_marker','ChallengeResponse','make_challenge_reply','encode_response','decode_payload','<GetConfigResponse>']:
    if token not in release_host_text:
        errors.append('release-host corrected protocol sequence marker missing: '+token)
if 'send_all_socket(s,request' in release_host_text or 'send(s,request' in release_host_text:
    errors.append('release-host plaintext GetConfig send path returned')
try:
    if not (release_host_text.index('recv_nul_frame') < release_host_text.index('ChallengeResponse') <
            release_host_text.index('make_challenge_reply') < release_host_text.index('encode_response') <
            release_host_text.index('decode_payload')):
        errors.append('release-host protocol sequencing order is not fail-closed')
except ValueError:
    errors.append('release-host protocol sequencing markers unavailable')
for token in ['src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/codec/message_codec.cpp','src/codec/session_crypto.cpp',
              'target_include_directories(preloader_l_release_host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)']:
    if token not in cm:
        errors.append('release-host codec build dependency missing: '+token)

# CASTORADRON HOTFIX10: behavioral service state remains decomposed into
# public lifecycle/façade, owned store storage, Config2 projection and
# GrantAchievement transaction responsibilities.
hf10_behavioral = [
    'src/runtime/service_state.cpp',
    'src/runtime/service_state_storage.cpp',
    'src/runtime/service_state_parser.cpp',
    'src/runtime/service_state_grant.cpp',
    'src/runtime/service_state_internal.hpp',
]
for rel in hf10_behavioral:
    if not (r/rel).is_file():
        errors.append('HF10 behavioral service component missing: '+rel)
behavioral_facade=(r/'src/runtime/service_state.cpp').read_text(encoding='utf-8',errors='replace')
behavioral_parser=(r/'src/runtime/service_state_parser.cpp').read_text(encoding='utf-8',errors='replace')
behavioral_storage=(r/'src/runtime/service_state_storage.cpp').read_text(encoding='utf-8',errors='replace')
behavioral_grant=(r/'src/runtime/service_state_grant.cpp').read_text(encoding='utf-8',errors='replace')
for token in ['enum class Tk', 'bool leaf(', 'struct GrantTxn', 'bool set_text(']:
    if token in behavioral_facade:
        errors.append('HF10 behavioral façade regained implementation responsibility: '+token)
for token in ['struct GrantTxn', 'prepare_grant_achievement_locked(']:
    if token in behavioral_parser:
        errors.append('HF10 behavioral parser regained grant transaction responsibility: '+token)
for token in ['enum class Tk', 'bool leaf(', 'struct GrantTxn']:
    if token in behavioral_storage:
        errors.append('HF10 behavioral storage regained parser/grant responsibility: '+token)
for token in ['enum class Tk', 'bool build_cfg_store(']:
    if token in behavioral_grant:
        errors.append('HF10 behavioral grant module regained parser responsibility: '+token)
for required in ['if(teq(c,key,"Name"))return v.size?replace_text(r->display_name,v):true;',
                 'if(depth!=2u)return true;']:
    if required not in behavioral_parser:
        errors.append('HF10 behavioral parser policy marker missing: '+required)
for required in ['struct GrantTxn {', 'prepare_grant_achievement_locked(', 'commit_grant_achievement_locked(']:
    if required not in behavioral_grant:
        errors.append('HF10 GrantAchievement transaction marker missing: '+required)

# Model release derived state stays decomposed into public façade, low-level ABI
# storage and store materialization. Audit3 F323-S0027-001 retired the private
# runtime-source adapter because it had no live caller; the config/store-owned
# materialization sources remain authoritative.
hf11_derived = [
    'src/runtime/service_projection.cpp',
    'src/runtime/service_abi.cpp',
    'src/runtime/service_store.cpp',
    'src/runtime/service_projection_internal.hpp',
]
for rel in hf11_derived:
    if not (r/rel).is_file():
        errors.append('model derived-state component missing: '+rel)
if (r/'src/runtime/service_source.cpp').exists():
    errors.append('retired dead runtime-source adapter remains: src/runtime/service_source.cpp')
derived_facade=(r/'src/runtime/service_projection.cpp').read_text(encoding='utf-8',errors='replace')
derived_abi=(r/'src/runtime/service_abi.cpp').read_text(encoding='utf-8',errors='replace')
derived_store=(r/'src/runtime/service_store.cpp').read_text(encoding='utf-8',errors='replace')
derived_internal=(r/'src/runtime/service_projection_internal.hpp').read_text(encoding='utf-8',errors='replace')
for token in ['bool assign_string(', 'bool materialize_record(', 'runtime_loaded_cb(', 'allocate_raw_vector(']:
    if token in derived_facade:
        errors.append('derived-state façade regained implementation responsibility: '+token)
for token in ['runtime_loaded_cb(', 'runtime_source() noexcept']:
    if token in derived_abi or token in derived_store or token in derived_internal:
        errors.append('retired runtime-source adapter leaked into active runtime projection: '+token)
for token in ['bool assign_string(', 'void initialize_empty_string(']:
    if token in derived_store:
        errors.append('ABI string ownership leaked out of ABI-storage module: '+token)
for required in ['materialize_entitlement_vector(', 'materialize_store_vectors(', 'release_store_vectors(']:
    if required not in derived_store:
        errors.append('store materialization marker missing: '+required)
for live_source in ['src/config/store_payload.cpp','src/config/store_vectors.cpp']:
    live=(r/live_source).read_text(encoding='utf-8',errors='replace')
    if 'StoreMaterializationSource runtime_source() noexcept' not in live:
        errors.append('live config/store materialization source missing: '+live_source)

# HF13: legacy Process32NextW and _Mtx_trylock hooks are physically retired.
for rel in ('src/platform/surge_hooks.cpp','src/platform/surge_wrapper_hooks.cpp',
            'src/platform/surge_inline_hooks.cpp','src/platform/surge_policy.cpp',
            'src/platform/surge_hooks_internal.hpp','include/preloader_l/platform/surge_hooks.hpp',
            'include/preloader_l/platform/surge_policy.hpp'):
    if (r/rel).exists(): errors.append('HF13 obsolete hook source returned: '+rel)

# CASTORADRON HOTFIX13: registry host policy/bridge remain decomposed into
# configuration matching, handle ownership, value serialization, Windows
# wrappers/hook ownership, and thin public lifecycle facades.
hf13_host = [
    'src/platform/registry_policy.cpp',
    'src/platform/registry_policy_config.cpp',
    'src/platform/registry_policy_state.cpp',
    'src/platform/registry_policy_values.cpp',
    'src/platform/registry_policy_internal.hpp',
    'src/platform/registry_hook_runtime.cpp',
    'src/platform/registry_wrappers.cpp',
    'src/platform/registry_hooks.cpp',
    'src/platform/registry_hook_runtime_internal.hpp',
]
for rel in hf13_host:
    if not (r/rel).is_file():
        errors.append('HF13 host component missing: '+rel)
host_policy_facade=(r/'src/platform/registry_policy.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_config=(r/'src/platform/registry_policy_config.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_state=(r/'src/platform/registry_policy_state.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_value=(r/'src/platform/registry_policy_values.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_facade=(r/'src/platform/registry_hook_runtime.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_wrappers=(r/'src/platform/registry_wrappers.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_hooks=(r/'src/platform/registry_hooks.cpp').read_text(encoding='utf-8',errors='replace')
for token in ['g_pseudo_allocation_fenced', 'MultiByteToWideChar(', 'HandleNode {']:
    if token in host_policy_facade:
        errors.append('HF13 host-policy facade regained implementation responsibility: '+token)
for required in ['classify_a(', 'classify_w(', 'language_policy()']:
    if required not in host_policy_config:
        errors.append('HF13 host-policy config marker missing: '+required)
for required in ['g_pseudo_allocation_fenced', 'allocate_pseudo(', 'begin_teardown_impl()']:
    if required not in host_policy_state:
        errors.append('HF13 host-policy state marker missing: '+required)
for required in ['write_a(', 'write_w(', 'value_name_w(']:
    if required not in host_policy_value:
        errors.append('HF13 host-policy value marker missing: '+required)
for token in ['preloader_l_RegOpenKeyExA', 'create_minhook(', 'enable_minhook_batch(']:
    if token in host_bridge_facade:
        errors.append('HF13 host-bridge facade regained wrapper/hook responsibility: '+token)
for required in ['preloader_l_RegOpenKeyExA', 'MinHookCallGuard quiescence', 'registry_replacement_addresses']:
    if required not in host_bridge_wrappers:
        errors.append('HF13 host-bridge wrapper marker missing: '+required)
for required in ['install_registry_inline_global()', 'RegistryHookState', 'enable_minhook_batch(batch, gates, registry_hook_count)']:
    if required not in host_bridge_hooks:
        errors.append('HF13 host-bridge hook marker missing: '+required)
if 'install_registry_iat_fallback' in host_bridge_hooks or 'make_iat_hook_backend' in host_bridge_hooks:
    errors.append('MELOTRIX retired registry IAT fallback returned')

# CASTORADRON HOTFIX13: registry host policy/bridge remains decomposed into
# policy configuration, handle ownership, value serialization and Windows
# wrapper/hook ownership modules behind thin public facades.
hf13_host = [
    'src/platform/registry_policy.cpp',
    'src/platform/registry_policy_config.cpp',
    'src/platform/registry_policy_state.cpp',
    'src/platform/registry_policy_values.cpp',
    'src/platform/registry_policy_internal.hpp',
    'src/platform/registry_hook_runtime.cpp',
    'src/platform/registry_wrappers.cpp',
    'src/platform/registry_hooks.cpp',
    'src/platform/registry_hook_runtime_internal.hpp',
]
for rel in hf13_host:
    if not (r/rel).is_file():
        errors.append('HF13 host-policy/bridge component missing: '+rel)
host_policy_facade=(r/'src/platform/registry_policy.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_config=(r/'src/platform/registry_policy_config.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_state=(r/'src/platform/registry_policy_state.cpp').read_text(encoding='utf-8',errors='replace')
host_policy_value=(r/'src/platform/registry_policy_values.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_facade=(r/'src/platform/registry_hook_runtime.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_wrappers=(r/'src/platform/registry_wrappers.cpp').read_text(encoding='utf-8',errors='replace')
host_bridge_hooks=(r/'src/platform/registry_hooks.cpp').read_text(encoding='utf-8',errors='replace')
for token in ['LanguageRegistryKey', 'struct HandleNode', 'MultiByteToWideChar(']:
    if token in host_policy_facade:
        errors.append('HF13 host-policy facade regained split responsibility: '+token)
for required in ['LanguagePolicy language_policy()', 'uint32_t classify_a(', 'uint32_t classify_w(']:
    if required not in host_policy_config:
        errors.append('HF13 host-policy config marker missing: '+required)
for required in ['bool track_handle(', 'uintptr_t allocate_pseudo(', 'bool begin_teardown_impl()']:
    if required not in host_policy_state:
        errors.append('HF13 host-policy state marker missing: '+required)
for required in ['int32_t write_a(', 'int32_t write_w(', 'bool value_name_a(']:
    if required not in host_policy_value:
        errors.append('HF13 host-policy value marker missing: '+required)
for token in ['create_minhook(', 'enable_minhook_batch(', 'preloader_l_RegOpenKeyExA']:
    if token in host_bridge_facade:
        errors.append('HF13 host-bridge facade regained wrapper/hook responsibility: '+token)
for required in ['preloader_l_RegOpenKeyExA', 'preloader_l_RegQueryValueExW', 'registry_replacement_addresses']:
    if required not in host_bridge_wrappers:
        errors.append('HF13 host-bridge wrapper marker missing: '+required)
for required in ['install_registry_inline_global()', 'RegistryHookState', 'enable_minhook_batch(batch, gates, registry_hook_count)']:
    if required not in host_bridge_hooks:
        errors.append('HF13 host-bridge hook marker missing: '+required)
if 'install_registry_iat_fallback' in host_bridge_hooks or 'make_iat_hook_backend' in host_bridge_hooks:
    errors.append('MELOTRIX retired registry IAT fallback returned')

# CASTORADRON HOTFIX18: Windows host-bridge mutable ownership stays encapsulated.
host_bridge_internal=(r/'src/platform/registry_hook_runtime_internal.hpp').read_text(encoding='utf-8',errors='replace')
retired_host_globals=['g_path_a','g_path_w','g_bridge','g_registry_iat','g_registry_tickets','g_registry_originals','g_registry_inline','g_registry_quiescence','g_registry_mode','g_registry_inline_recovery_pending','g_registry_iat_recovery_pending','g_replacements']
for token in retired_host_globals:
    if token in host_bridge_internal:
        errors.append('HF18 raw cross-TU host-bridge state escaped through internal header: '+token)
if 'extern ' in host_bridge_internal:
    errors.append('HF18 host-bridge internal header regained extern storage exposure')
for required in ['struct RegistryHookState', 'RegistryHookState g_registry_hooks{}', 'registry_hook_install_blocked()', 'registry_quiescence_gate(size_t index)']:
    if required not in host_bridge_hooks:
        errors.append('HF18 registry hook owner marker missing: '+required)
for required in ['struct HostBridgeRuntimeState', 'HostBridgeRuntimeState g_host_runtime{}', 'dispatch_host_reg_open_a(', 'dispatch_host_reg_close(']:
    if required not in host_bridge_facade:
        errors.append('HF18 host runtime owner marker missing: '+required)
for required in ['registry_quiescence_gate(0u)', 'registry_quiescence_gate(1u)', 'registry_quiescence_gate(2u)', 'registry_quiescence_gate(3u)', 'registry_quiescence_gate(4u)']:
    if required not in host_bridge_wrappers:
        errors.append('HF18 registry wrapper lease accessor missing: '+required)
if 'minhook_gate_forwarding_target' not in host_bridge_wrappers:
    errors.append('HF18/MELOTRIX registry wrapper forwarding target accessor missing')
for token in retired_host_globals:
    if token in host_bridge_wrappers or token in host_bridge_facade:
        errors.append('HF18 non-owner TU regained raw hook storage access: '+token)

# CASTORADRON HOTFIX14: XML value codec remains decomposed into public
# decode/escape facade, UTF-8/entity core, source normalization/comparison and
# owning SemanticValue storage.
hf14_value_codec = [
    'src/xml/value_codec.cpp',
    'src/xml/value_codec_utf8.cpp',
    'src/xml/value_codec_normalize.cpp',
    'src/xml/value_codec_semantic.cpp',
    'src/xml/value_codec_internal.hpp',
]
for rel in hf14_value_codec:
    if not (r/rel).is_file():
        errors.append('HF14 value-codec component missing: '+rel)
vc_facade=(r/'src/xml/value_codec.cpp').read_text(encoding='utf-8',errors='replace')
vc_utf8=(r/'src/xml/value_codec_utf8.cpp').read_text(encoding='utf-8',errors='replace')
vc_norm=(r/'src/xml/value_codec_normalize.cpp').read_text(encoding='utf-8',errors='replace')
vc_sem=(r/'src/xml/value_codec_semantic.cpp').read_text(encoding='utf-8',errors='replace')
for token in ['SemanticValue::SemanticValue', 'normalize_source_attribute_value(', 'DecodedUnit decode_unit(']:
    if token in vc_facade:
        errors.append('HF14 value-codec facade regained split responsibility: '+token)
for required in ['Utf8Unit validate_utf8_unit(', 'DecodedUnit decode_unit(', 'size_t literal_length(']:
    if required not in vc_utf8:
        errors.append('HF14 UTF-8/entity marker missing: '+required)
for required in ['normalize_source_attribute_value(', 'source_value_equals(', 'source_value_equals_literal(']:
    if required not in vc_norm:
        errors.append('HF14 normalization/comparison marker missing: '+required)
for required in ['SemanticValue::SemanticValue', 'SemanticValue::assign_source(', 'SemanticValue::clear()']:
    if required not in vc_sem:
        errors.append('HF14 SemanticValue ownership marker missing: '+required)

# HF11: one parser engine; split by envelope policy / DOM child traversal only.
for rel in ['src/xml/request_parser.cpp','src/xml/request_children.cpp','src/xml/tinyxml2_dom.hpp']:
    if not (r/rel).is_file(): errors.append('HF11 DOM request component missing: '+rel)
request_facade=(r/'src/xml/request_parser.cpp').read_text(encoding='utf-8',errors='replace')
request_children=(r/'src/xml/request_children.cpp').read_text(encoding='utf-8',errors='replace')
if 'dom::parse_document(' not in request_facade or 'dom::child_index(command)' not in request_facade:
    errors.append('HF11 owned TinyXML2 request/document graph missing')
for token in ['bool parse_start_tag(', 'bool find_element_close(']:
    if token in request_facade or token in request_children: errors.append('HF11 second source-range parser present: '+token)
for required in ['ChildTextStatus find_direct_child_text(', 'DirectChildStatus next_direct_child(', 'dom::child_index(element)']:
    if required not in request_children: errors.append('HF11 DOM child traversal marker missing: '+required)
if 'src/xml/request_children.cpp' not in cm or 'preloader_l_tinyxml2' not in cm:
    errors.append('HF11 production DOM graph missing from CMake')
if (r/'src/xml/request_scanner.cpp').exists() or (r/'src/xml/tinyxml2_compat_bridge.hpp').exists():
    errors.append('HF11 legacy request scanner/PI compatibility bridge returned')

# Project release identity is package/docs-only and must not appear in compile definitions.
# F1532/F1559: debug-only build concerns must not leak into shipping graph.
if '$<$<COMPILE_LANGUAGE:CXX>:/W4;/permissive-;/Od;/Zi;/GS;/GR->' not in cm:
    errors.append('debug C++ flags are not language-scoped from MASM')
if 'target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 vcruntime)' in cm:
    errors.append('debug target manually mixes release vcruntime with /MDd policy')
prod_block=cm.split('add_library(preloader_l SHARED',1)[1].split(')\n',1)[0]
if 'src/debug/evidence/architecture_provenance.cpp' in prod_block:
    errors.append('dormant x64 provenance diagnostic TU leaked into production graph')
if cm.count('src/debug/evidence/architecture_provenance.cpp') != 1:
    errors.append('x64 provenance diagnostic TU must remain debug-only exactly once')

if 'PRELOADER_L_VERSION_CODE' in cm or 'PRELOADER_L_BUILD_TAG' in cm:
    errors.append('embedded project release identity mechanism present')
# Every literal source/export input named by CMake must exist.
for token in re.findall(r'(?<![A-Za-z0-9_$\{])((?:src|exports|resources)/[A-Za-z0-9_./-]+\.(?:cpp|asm|def|rc))',cm):
    if not (r/token).is_file(): errors.append('missing CMake input '+token)
# HOTFIX22 vocabulary collateral closure.  The historical HOTFIX15 migration is
# project-owned naming policy, not a license to rewrite arbitrary language,
# platform, ABI, ISA, algorithmic or mathematical terminology.  Enforcement is
# therefore scoped to concrete retired project-owned paths/stems.  Generic root
# substring policing is intentionally forbidden (VOC-HF22-019 / F319-P006).
retired_project_owned_paths = (

    'scripts/test_reference_config_semantic_runtime.py',
    'scripts/test_reference_worker_semantic_abi.py',
    'scripts/test_reference_settings_semantic_runtime.py',
    'scripts/test_reference_settings_semantic_abi.py',
    'scripts/test_reference_config_semantic_abi.py',
    'src/runtime/donor_time.cpp',
    'src/runtime/reference_release_service_derived_state.cpp',
    'src/runtime/reference_release_service_authority.cpp',
    'src/runtime/donor_service_state.cpp',
    'src/config/reference_store_vector_model.cpp',
    'src/config/reference_byte_vector_binding.cpp',
    'src/platform/recovered_crypt_wrapper.cpp',
    'src/platform/reference_hook_allocator_exact_windows.cpp',
    'include/preloader_l/runtime/parity_closure.hpp',
    'include/preloader_l/runtime/reference_release_service_abi.hpp',
    'include/preloader_l/runtime/donor_service_state.hpp',
    'include/preloader_l/runtime/reference_release_service_derived_state.hpp',
    'include/preloader_l/runtime/primary_runtime.hpp',
    'include/preloader_l/runtime/reference_release_service_authority.hpp',
    'include/preloader_l/runtime/donor_time.hpp',
    'include/preloader_l/config/reference_store_vector_model.hpp',
    'include/preloader_l/config/reference_byte_vector_binding.hpp',
    'include/preloader_l/platform/recovered_crypt_wrapper.hpp',
    'include/preloader_l/platform/reference_hook_allocator_exact.hpp',
    'tests/unit/test_reference_settings_semantic_runtime.cpp',
    'tests/unit/test_donor_parity_service_state.cpp',
    'tests/unit/test_exact_wrapper_oracle.cpp',
    'tests/unit/test_exact_parity_status_source_migration_baseline.cpp',
    'tests/unit/test_tinyxml_reference_geometry.cpp',
    'tests/unit/test_session_parity.cpp',
    'tests/unit/test_exact_parity_status_initial_accounting.cpp',
    'tests/unit/test_connectivity_auth_authority.cpp',
    'tests/unit/test_primary_state_recovered_integrated_masks.cpp',
    'tests/unit/test_primary_state_binding_aliases.cpp',
    'tests/unit/test_exact_parity_status_live_binding.cpp',
    'tests/unit/test_primary_state_allocator_closure.cpp',
    'tests/unit/test_selector_builder_exact.cpp',
    'tests/unit/test_parity_closure.cpp',
    'tests/unit/test_primary_state_native_hook_lifecycle.cpp',
    'tests/unit/test_primary_recovered_mask.cpp',
    'tests/unit/test_primary_state_thread_trap_allocator.cpp',
    'tests/unit/test_primary_state_7000_service.cpp',
    'tests/unit/test_fatal_policy_parity.cpp',
    'tests/unit/test_exact_parity_status_callback_closure.cpp',
    'tests/unit/test_exact_parity_status_vector_binding.cpp',
    'tests/unit/test_primary_state_relocation_allocator.cpp',
    'tests/unit/test_surge_vm_reference.cpp',
    'tests/unit/test_primary_state_hook_service_parity.cpp',
    'tests/unit/test_primary_state_ae00_aux.cpp',
    'tests/unit/test_primary_state_optional_scanner.cpp',
    'tests/unit/test_exact_parity_status_d5f0_6b31_closure.cpp',
    'tests/unit/test_original_parity.cpp',
    'tests/unit/test_primary_state_optional_callback_parity.cpp',
    'tests/unit/test_recovered_crypt_wrapper.cpp',
    'tests/unit/test_primary_state_d5f0_payload.cpp',
    'tests/unit/test_primary_state_optional_host_hooks.cpp',
    'tests/unit/test_reference_config_semantic_runtime.cpp',
)
retired_project_owned_stems = (
    'reference_config_semantic_runtime',
    'reference_worker_semantic_abi',
    'reference_settings_semantic_runtime',
    'reference_settings_semantic_abi',
    'reference_config_semantic_abi',
    'donor_time',
    'reference_release_service_derived_state',
    'reference_release_service_authority',
    'donor_service_state',
    'reference_store_vector_model',
    'reference_byte_vector_binding',
    'recovered_crypt_wrapper',
    'reference_hook_allocator_exact_windows',
    'parity_closure',
    'reference_release_service_abi',
    'primary_runtime',
    'reference_hook_allocator_exact',
    'donor_parity_service_state',
    'exact_wrapper_oracle',
    'exact_parity_status_source_migration_baseline',
    'tinyxml_reference_geometry',
    'session_parity',
    'exact_parity_status_initial_accounting',
    'connectivity_auth_authority',
    'primary_state_recovered_integrated_masks',
    'primary_state_binding_aliases',
    'exact_parity_status_live_binding',
    'primary_state_allocator_closure',
    'selector_builder_exact',
    'primary_state_native_hook_lifecycle',
    'primary_recovered_mask',
    'primary_state_thread_trap_allocator',
    'primary_state_7000_service',
    'fatal_policy_parity',
    'exact_parity_status_callback_closure',
    'exact_parity_status_vector_binding',
    'primary_state_relocation_allocator',
    'surge_vm_reference',
    'primary_state_hook_service_parity',
    'primary_state_ae00_aux',
    'primary_state_optional_scanner',
    'exact_parity_status_d5f0_6b31_closure',
    'original_parity',
    'primary_state_optional_callback_parity',
    'primary_state_d5f0_payload',
    'primary_state_optional_host_hooks',
)

def project_owned_terminology_violation(text):
    lower=text.lower()
    for stem in retired_project_owned_stems:
        # Match a retired project stem as an identifier/path component, while
        # allowing it to be embedded in a larger project symbol by underscore.
        if re.search(r'(?<![a-z0-9])'+re.escape(stem)+r'(?![a-z0-9])', lower):
            return stem
    return None

# Fail closed only on the concrete project-owned migration targets.
for rel in retired_project_owned_paths:
    if (r/rel).exists():
        errors.append('retired project-owned terminology path returned: '+rel)
for scan_root in [r/'src',r/'include',r/'tests',r/'scripts',r/'tools',r/'metadata',r/'CMakeLists.txt',r/'RELEASE_STATE.json']:
    if scan_root.is_file(): items=(scan_root,)
    elif scan_root.exists(): items=(x for x in scan_root.rglob('*') if x.is_file() and 'third_party' not in x.parts)
    else: items=()
    for item in items:
        if item.name!='CMakeLists.txt' and item.suffix.lower() not in {'.cpp','.cc','.c','.cxx','.hpp','.hh','.h','.py','.ps1','.cmd','.json','.txt','.rc','.def','.cmake'}:
            continue
        try: source=item.read_text(encoding='utf-8',errors='strict')
        except (UnicodeDecodeError,OSError): continue
        rel=item.relative_to(r).as_posix()
        if rel in {'scripts/check_repository_contract.py','metadata/DOCUMENTATION_HISTORY_MIGRATION.json'}:
            continue
        for line_no,line in enumerate(source.splitlines(),1):
            stem=project_owned_terminology_violation(line)
            if stem:
                errors.append(f'retired project-owned terminology {rel}:{line_no}: {stem}')
                break

# Adversarial safety regression: these are genuine technical/platform terms,
# not project naming labels.  A future terminology guard must leave them legal.
technical_terminology_safety_probes = (
    '$ErrorActionPreference', 'dereferenced', 'ReferenceOwnerControl',
    'reference_owner_snapshot', 'restored_mask', 'recovered_delta',
    'even_low_byte_parity', 'is_legacy_prefix', 'source_exact_offset',
    'canonicalization', 'exact_rva_size_offset_parity',
    'std::reference_wrapper<int>', 'CPU parity flag', 'exact byte count',
)
for probe in technical_terminology_safety_probes:
    if project_owned_terminology_violation(probe):
        errors.append('technical terminology protection regression: '+probe)

# HF16: keep GrantAchievement mutation/delivery and QueryAchievements read/serialization
# in separate implementation TUs behind the public achievement route facade.
achievement_facade=(r/'src/handlers/achievement_routes.cpp').read_text(encoding='utf-8',errors='replace')
achievement_grant=(r/'src/handlers/achievement_grant_route.cpp').read_text(encoding='utf-8',errors='replace') if (r/'src/handlers/achievement_grant_route.cpp').is_file() else ''
achievement_query=(r/'src/handlers/achievement_query_route.cpp').read_text(encoding='utf-8',errors='replace') if (r/'src/handlers/achievement_query_route.cpp').is_file() else ''
if 'prepare_grant_achievement_locked(' in achievement_facade:
    errors.append('HF16 achievement facade regained grant transaction responsibility')
if 'next_direct_child(r.command_content' in achievement_facade:
    errors.append('HF16 achievement facade regained query traversal responsibility')
if 'prepare_grant_achievement_locked(' not in achievement_grant or 'publish_achievement_watcher_snapshot(' not in achievement_grant:
    errors.append('HF16 GrantAchievement transaction implementation missing from grant TU')
if 'next_direct_child(r.command_content' not in achievement_query or 'source_value_equals_literal(all_text,"true"' not in achievement_query:
    errors.append('HF16 QueryAchievements traversal implementation missing from query TU')

# CASTORADRON HOTFIX20: residual cross-TU mutable ownership closure.
# These private contracts may expose operations/borrows, but not writable storage.
for rel in [
    'src/config/runtime_configuration_internal.hpp',
    'src/config/store_internal.hpp',
    'src/runtime/service_state_internal.hpp',
]:
    text=(r/rel).read_text(encoding='utf-8',errors='replace')
    if __import__('re').search(r'(?m)^\s*extern\s+(?!"C")',text):
        errors.append(f'HOTFIX20 raw mutable extern returned to {rel}')
for forbidden in ['g_store_slots','g_active_store','g_last_store_status']:
    if forbidden in (r/'src/config/store_internal.hpp').read_text(encoding='utf-8',errors='replace'):
        errors.append(f'HOTFIX20 store runtime ownership leaked through internal header: {forbidden}')
if re.search(r'\bg_store\b',(r/'src/runtime/service_state_internal.hpp').read_text(encoding='utf-8',errors='replace')):
    errors.append('HOTFIX20 behavioral store storage leaked through internal header')
for forbidden in ['g_states','g_active','g_cwd']:
    if forbidden in (r/'src/config/runtime_configuration_internal.hpp').read_text(encoding='utf-8',errors='replace'):
        errors.append(f'HOTFIX20 runtime configuration storage leaked through internal header: {forbidden}')


# CASTORADRON HOTFIX21 / MELOTRIX RC03: process-wide synchronization/storage stays owner-TU local.
minhook_public_hf21=(r/'include/preloader_l/platform/minhook_runtime.hpp').read_text(encoding='utf-8',errors='replace')
release_internal_hf21=(r/'src/runtime/release_server_io_internal.hpp').read_text(encoding='utf-8',errors='replace')
behavioral_internal_hf21=(r/'src/runtime/service_state_internal.hpp').read_text(encoding='utf-8',errors='replace')
for forbidden in ['g_batch_transaction','g_runtime_state','g_operation_lock','g_traps','g_trap_lock','g_virtual_protect_lock']:
    if forbidden in minhook_public_hf21:
        errors.append('HF21/MELOTRIX raw MinHook synchronization/storage leaked through public contract: '+forbidden)
for forbidden in ['g_stop_requested','g_acceptance_accepted','g_acceptance_started','g_acceptance_completed',
                  'g_acceptance_peer_nonempty','g_acceptance_reaped']:
    if forbidden in release_internal_hf21:
        errors.append('HF21 raw synchronization/storage leaked through release-server internal contract: '+forbidden)
if 'host_bridge_state()' in host_bridge_internal or 'host_bridge_state()' in host_bridge_wrappers:
    errors.append('HF21 broad mutable HostBridgeState accessor returned')
if re.search(r'\bactive_store\s*\(', behavioral_internal_hf21):
    errors.append('HF21 broad mutable behavioral Store accessor returned')
for required in ['release_server_stop_requested()', 'signal_release_server_stop()',
                 'note_release_acceptance_completed()', 'release_acceptance_counter(uint32_t selector)']:
    if required not in release_internal_hf21:
        errors.append('HF21 release-server operation contract missing: '+required)
for required in ['replace_active_store(', 'find_active_achievement(', 'active_achievement_at(']:
    if required not in behavioral_internal_hf21:
        errors.append('HF21 behavioral owner operation missing: '+required)

if errors:
    print('REPOSITORY_CONTRACT: FAIL'); [print(' -',x) for x in errors]; sys.exit(1)


# Current repository root is reserved for canonical entry/state files; launchers belong under scripts/build_orchestration.
for _p in r.iterdir():
    if _p.is_file() and _p.name.startswith('BUILD_') and _p.suffix.lower() == '.cmd':
        errors.append('root BUILD launcher returned: '+_p.name)

# HF13 native ReleaseServer source cutover; native/Golden/game acceptance remains pending.
_service=(r/'src/runtime/asio_service_owner.cpp').read_text(encoding='utf-8')
_bridge_header=(r/'src/runtime/asio_service_owner.hpp').read_text(encoding='utf-8')
for needle in ('listener_->start(', 'listener_->session_finished()', 'session->request_stop()', 'listener_drained_', 'self_hold_.reset()'):
    if needle not in _service: errors.append('HF13 Asio service bridge missing lifecycle '+needle)
if cm.count('src/runtime/asio_service_owner.cpp') != 2:
    errors.append('HF13 Asio service bridge missing from both production/debug targets')
if 'socket.release(' in _service or 'closesocket(' in _service:
    errors.append('HF13 Asio service bridge bypasses owned Asio socket semantics')
_bridge=(r/'src/runtime/asio_release_bridge.cpp').read_text(encoding='utf-8')
_release=(r/'src/runtime/release_server.cpp').read_text(encoding='utf-8')
if cm.count('src/runtime/asio_release_bridge.cpp') != 2:
    errors.append('HF13 native Asio ReleaseServer bridge not in both build targets')
for needle in ('start_release_asio_windows(context, bound_port)',
               'drive_release_asio_windows(context)',
               'stop_release_asio_windows(context)',
               'shutdown_release_asio_windows(context)'):
    if needle not in _release:
        errors.append('HF13 native ReleaseServer cutover branch missing '+needle)
for needle in ('io.run_for(', 'AsioServiceOwner',
               'abort_before_start(ec)', 'return mark_bootstrap_client_served()',
               'context->asio_bridge=nullptr'):
    if needle not in _bridge:
        errors.append('HF13 native Asio release ownership contract missing '+needle)
if 'socket.release(' in _bridge or 'closesocket(' in _bridge:
    errors.append('HF13 Asio release bridge bypasses Asio socket ownership')


if errors:
    print('REPOSITORY_CONTRACT: FAIL'); [print(' -',x) for x in errors]; sys.exit(1)

print('REPOSITORY_CONTRACT: PASS')
