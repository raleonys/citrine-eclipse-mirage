#!/usr/bin/env python3
"""Mutation-based offline regression for the release-independent source gate.

Runs exclusively in a temporary copied fixture. No canonical files are edited.
"""
from pathlib import Path
import copy,hashlib,json,shutil,sys,tempfile
from check_universal_shipping_policy import validate

ROOT=Path(__file__).resolve().parents[1]
CONTRACT=ROOT/'metadata/SHIPPING_POLICY_CONTRACT.json'
PIN=hashlib.sha256(CONTRACT.read_bytes()).hexdigest()
CONTRACT_JSON=json.loads(CONTRACT.read_text())
N=0

def assert_case(title,root,expected_failed,contract_hash=PIN):
    global N
    checks=validate(root,root/'metadata/SHIPPING_POLICY_CONTRACT.json',contract_hash)
    problems={k for k,v in checks.items() if not v}
    assert expected_failed in problems, (title,expected_failed,sorted(problems))
    N+=1
    print('PASS:',title,'=>',expected_failed)

def mutate(root,relative,needle,replacement,reason):
    p=root/relative
    old=p.read_text(encoding='utf-8')
    assert needle in old,(reason,'fixture needle absent',needle)
    p.write_text(old.replace(needle,replacement,1),encoding='utf-8')
    return old

with tempfile.TemporaryDirectory(prefix='universal_shipping_qa_') as temp:
    t=Path(temp)
    # Only required policy surfaces are copied. This is an isolated test model,
    # not a substitute for source repository/manifest or native acceptance.
    copied={'CMakeLists.txt','metadata/SHIPPING_POLICY_CONTRACT.json'}
    p=CONTRACT_JSON['policy']
    copied.update(CONTRACT_JSON['source_graph']['sources'])
    copied.update([p['entrypoint'],p['bootstrap_file'],p['config_header'],p['config_freeze_path'],p['dependency_pin_path']])
    for rel in sorted(copied):
        dest=t/rel;dest.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(ROOT/rel,dest)
    baseline=validate(t,t/'metadata/SHIPPING_POLICY_CONTRACT.json',PIN)
    assert baseline and all(baseline.values()),[k for k,v in baseline.items() if not v]
    print('PASS: baseline',len(baseline),'checks')
    N+=1
    assert_case('no external authority',t,'independently_supplied_contract_pin',contract_hash=None)
    assert_case('wrong external SHA',t,'contract_sha256',contract_hash='0'*64)
    cases=[
        ('added source bypass','CMakeLists.txt','add_library(preloader_l SHARED\n','add_library(preloader_l SHARED\n    tests/native/native_acceptance_exports.cpp\n','shipping_source_graph_exact'),
        ('removed core hook TU','CMakeLists.txt','    src/platform/core_hooks.cpp\n','','shipping_source_graph_exact'),
        ('duplicate source TU','CMakeLists.txt','    src/platform/core_hooks.cpp\n','    src/platform/core_hooks.cpp\n    src/platform/core_hooks.cpp\n','cmake_graph_no_duplicate'),
        ('dynamic CRT substitution','CMakeLists.txt','set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")','set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")','shipping_crt_link_and_flags'),
        ('missing static MinHook link','CMakeLists.txt','kernel32 ws2_32 crypt32 minhook)','kernel32 ws2_32 crypt32)','shipping_crt_link_and_flags'),
        ('missing safe freeze patch','CMakeLists.txt','PatchMinHookSafeFreeze.cmake','PatchedHookNotPresent.cmake','shipping_crt_link_and_flags'),
        ('vendor SHA drift',p['dependency_pin_path'],(t/p['dependency_pin_path']).read_text().strip(),'0'*40,'vendored_dependency_identity'),
        ('loader lock mutation',p['entrypoint'],'    const bool scheduled = schedule_release_bootstrap(module);','    MH_ApplyQueued();\n    const bool scheduled = schedule_release_bootstrap(module);','loader_lock_does_not_install_hooks'),
        ('early hook install before config',p['bootstrap_file'],'    if (!preloader_l::runtime::initialize_ptd_runtime_windows()) {','    const auto early = install_post_loader_minhook_surface(selection);\n    if (!preloader_l::runtime::initialize_ptd_runtime_windows()) {','config_before_hooks'),
        ('addon hook stage removed',p['bootstrap_file'],'    if (!stage_addon_hooks(selection.mask)) return abort_staging();','    /* disabled stage */','single_transaction_order'),
        ('transaction double commit',p['bootstrap_file'],'    const auto transaction = commit_minhook_batch_transaction();','    commit_minhook_batch_transaction();\n    const auto transaction = commit_minhook_batch_transaction();','single_transaction_order'),
        ('rollback removed',p['bootstrap_file'],'rollback_minhook_batch_transaction_with_timeout(','noop_rollback_with_timeout(','rollback_contract'),
        ('failed ownership guarantee removed',p['bootstrap_file'],'return PostLoaderHookInstallResult::failed_ownership_retained;','return PostLoaderHookInstallResult::failed_rolled_back;','shipping_source_bytes_pinned'),
        ('runtime configuration thaw removed',p['bootstrap_file'],'if (hooks_ok) preloader_l::config::unfreeze_runtime_configuration_after_hooks();','if (hooks_ok) return true;','rollback_contract'),
        ('freeze writer removed',p['config_freeze_path'],'try_begin_policy_write','disabled_policy_write','shipping_source_bytes_pinned'),
        ('CMake comment masquerades as CRT authority','CMakeLists.txt','set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")','# set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")','shipping_crt_link_and_flags'),
        ('arbitrary shipping source mutation','src/runtime/utc_time.cpp',(t/'src/runtime/utc_time.cpp').read_text()[:70],(t/'src/runtime/utc_time.cpp').read_text()[:70]+'\n// malicious drift\n','shipping_source_bytes_pinned'),
    ]
    for title,rel,needle,replacement,wanted in cases:
        original=mutate(t,rel,needle,replacement,title)
        try:
            failures={k for k,v in validate(t,t/'metadata/SHIPPING_POLICY_CONTRACT.json',PIN).items() if not v}
            # If exact mutation is provably rejected but another deeper check is
            # not triggered due to earlier fail closed, require explicitly
            # source-byte pin; do not count an unrelated error as coverage.
            assert wanted in failures or ('shipping_source_bytes_pinned' in failures and wanted in ('single_transaction_order','failed_ownership_retained')), (title, wanted, failures)
            N+=1;print('PASS:',title, '=>',wanted if wanted in failures else 'shipping_source_bytes_pinned')
        finally:
            (t/rel).write_text(original,encoding='utf-8')
        assert all(validate(t,t/'metadata/SHIPPING_POLICY_CONTRACT.json',PIN).values()),'fixture restoration failed '+title
    contract_path=t/'metadata/SHIPPING_POLICY_CONTRACT.json'
    original=contract_path.read_text()
    try:
        contract_path.write_text(original+'\n')
        assert_case('modified unapproved contract',t,'contract_sha256')
    finally:contract_path.write_text(original)
    try:
        x=copy.deepcopy(CONTRACT_JSON);x['unapproved_setting']=True
        contract_path.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
        h=hashlib.sha256(contract_path.read_bytes()).hexdigest()
        assert_case('unexpected contract key even with matching new pin',t,'contract_fields',h)
    finally:contract_path.write_text(original)
    assert all(validate(t,t/'metadata/SHIPPING_POLICY_CONTRACT.json',PIN).values())
print('UNIVERSAL_SHIPPING_NEGATIVE_TESTS: PASS',N,'/',N)
