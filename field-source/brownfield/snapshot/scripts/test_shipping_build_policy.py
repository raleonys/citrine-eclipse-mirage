#!/usr/bin/env python3
from pathlib import Path
import os,re,shutil,subprocess,sys
r=Path(__file__).resolve().parents[1]
def read(rel): return (r/rel).read_text(encoding='utf-8',errors='replace')
build=read('scripts/build_orchestration/build_production.ps1')
common=read('scripts/build_orchestration/build_common.ps1')
selector_fn=re.search(r'function\s+Clear-BuildSelectorEnvironment\s*\{(?P<body>.*?)\n\}',common,re.S)
selector_body=selector_fn.group('body') if selector_fn else ''
selector_names=['CL','_CL_','LINK','_LINK_','LINK_REPRO','CC','CXX','CMAKE_GENERATOR','CMAKE_GENERATOR_INSTANCE','CMAKE_TOOLCHAIN_FILE']
selector_reset_operation="[Environment]::SetEnvironmentVariable($name,$null,'Process')" in selector_body
exit_fn=re.search(r'function\s+Invoke-ExternalChecked\s*\{(?P<body>.*?)\n\}',common,re.S)
exit_body=exit_fn.group('body') if exit_fn else ''
def has_exit_contract(body):
    return ('$LASTEXITCODE' in body and re.search(r'if\s*\(\$rc\s*-ne\s*0\s*\)',body,re.I) is not None and 'Fail-Build' in body)
exit_negative_fixture=has_exit_contract(exit_body.replace('Fail-Build','Write-Host',1)) if exit_body else True
checker=read('scripts/check_msvc_shipping_link_command.py')
finalizer=read('scripts/ursynex_golden_finalizer.py')
cmake=read('CMakeLists.txt')
native=read('scripts/build_orchestration/build_native_acceptance.ps1')
crt=read('scripts/build_orchestration/build_crt_acceptance.ps1')
env_test=read('scripts/test_build_environment.ps1')
release_host=read('tools/preloader_l_release_host/release_host_main.cpp')
native_host=read('tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp')
minhook_pin='8af6b4acae5a9388fd742b56fa79ece89d96f823'
from check_universal_shipping_policy import validate as validate_shipping_contract
source_policy = validate_shipping_contract(r, r/'metadata/SHIPPING_POLICY_CONTRACT.json', os.environ.get('DLL_RD_SHIPPING_CONTRACT_SHA256'))
checks={
 'script relative root':'$PSScriptRoot' in build and 'Get-Location' not in build,
 'shared build helper':". (Join-Path $PSScriptRoot 'build_common.ps1')" in build and 'function Fail-Build' in common,
 'development build entrypoints independent of G0': all(
     re.search(r"(?m)^\. \(Join-Path \$PSScriptRoot 'build_common\.ps1'\)\s*\nAssert-DevelopmentBuildPreflight\s*$",read('scripts/build_orchestration/'+entry))
     for entry in ('build_production.ps1','build_debug.ps1','build_native_acceptance.ps1','build_crt_acceptance.ps1','build_unstable.ps1')
 ),
 'release authority remains fail-closed': 'function Assert-GovernanceFirstGate {' in common and 'check_governance_first.py' in common and 'if ($LASTEXITCODE -ne 0)' in common,
 'local compile cannot publish': all(x in read('scripts/build_orchestration/build_unstable.ps1') for x in ['COMPILE_ONLY','NOT_RELEASE','preloader.unsigned.dll']) and "Join-Path $Root 'dist'" not in read('scripts/build_orchestration/build_unstable.ps1'),
 'local compile retains CMake and exit checks': all(x in read('scripts/build_orchestration/build_unstable.ps1') for x in ['Resolve-ConfinedBuildDirectory','Invoke-ExternalChecked','Find-VcVars64','Clear-BuildSelectorEnvironment']),

 'selector environment sanitized':selector_fn is not None and all(("'"+x+"'") in selector_body for x in selector_names) and selector_reset_operation,
 'selector environment executable regression packaged':'Clear-BuildSelectorEnvironment' in env_test and 'BUILD_SELECTOR_SENTINEL' in env_test,
 'selector environment regression complete':all(("'"+x+"'") in env_test for x in selector_names),
 'external exit propagation source contract':exit_fn is not None and has_exit_contract(exit_body) and not exit_negative_fixture,
 'external exit propagation executable regression packaged':all(x in env_test for x in ['controlled-exit','exit /b 37','unexpected_continuation.txt','BUILD_EXIT_PROPAGATION: PASS']),
 'link checker rejects conflicting incremental':all(x in checker for x in ['incremental_opts','forbidden incremental linker option(s)','incremental_yes_before_no','incremental_yes_after_no']),
 'link checker uses referenced response closure':all(x in checker for x in ['_response_refs','referenced response file not found','unreferenced_rsp_cannot_complete_policy','missing_referenced_rsp_fails_closed']),
 'bound tools':all(x in build for x in ["Resolve-BoundExecutable 'cmake.exe'","Resolve-BoundExecutable 'python.exe'","Resolve-BoundExecutable 'cl.exe'","Resolve-BoundExecutable 'link.exe'"]),
 'build directory confined':'Resolve-ConfinedBuildDirectory' in build and 'Resolve-ConfinedBuildDirectory' in native and 'Resolve-ConfinedBuildDirectory' in crt,
 'production build lock':'Acquire-ExclusiveBuildLock' in build and '.preloader_l_production.lock' in build,
 'toolchain provenance':'Write-ToolchainProvenance' in build and 'BUILD_TOOLCHAIN_PROVENANCE.log' in build,
 'release shared flags normalized':'-DCMAKE_SHARED_LINKER_FLAGS_RELEASE:STRING=/INCREMENTAL:NO /OPT:REF /OPT:ICF' in build,
 'compile before link audit':build.index("Invoke-ExternalChecked $CMakeExe @('--build',$Out,'--target','preloader_l')") < build.index('check_msvc_shipping_link_command.py'),
 'authoritative output invalidated':"$GameDll = Join-Path $Dist 'preloader_l.dll'" in build and 'Remove-Item -LiteralPath $GameDll -Force' in build,
 'atomic publication':'[System.IO.File]::Move($PublishTemp,$GameDll)' in build,
 'publication hash bound':'Staged publication hash mismatch' in build and 'Published DLL hash mismatch after atomic rename' in build,
 'golden candidate identity continuity':all(x in build for x in ['function Assert-CandidateIdentity','$ValidatedCandidateHash = $UrsynexAfter','ARYRA precondition','ARYRA postcondition','MUSTAVEX precondition','MUSTAVEX postcondition','CORVEXON precondition','CORVEXON postcondition','FALCION precondition','FALCION postcondition','Published DLL is not the validated Golden candidate identity.']),
 'candidate identity PowerShell interpolation safe':'Validated candidate missing at ${Label}: $Path' in build and 'Validated candidate missing at $Label: $Path' not in build,
 'finalizer atomic same-directory replacement':all(x in finalizer for x in ['atomic_replace_bytes','tempfile.mkstemp','os.replace','_write_file_fully']),
 'golden cli IO fail closed':all(x in finalizer for x in ['load_policies','policy load failed','URSYNEX_GOLDEN_FINALIZER: FAIL:']),
 'prior golden logs invalidated':all(x in build for x in ['01_URSYNEX_GOLDEN_FINALIZER.log','02_ARYRA_GOLDEN_LAYOUT.log','03_MUSTAVEX_GOLDEN_METADATA.log','04_CORVEXON_GOLDEN_TRAIL.log','05_FALCION_GOLDEN_CLEANING.log']),
 'target debug none':'/DEBUG:NONE' in cmake,
 'target incremental off':'/INCREMENTAL:NO' in cmake,
 'target pogo metadata off':'/LTCG:OFF' in cmake and '/NOCOFFGRPINFO' in cmake,
 'target manifest suppressed':'/MANIFEST:NO' in cmake,
 'shipping GS enabled':'/EHsc;/GR-;/GS>' in cmake,
 'shipping static CRT':'MSVC_RUNTIME_LIBRARY "MultiThreaded"' in cmake,
 'compat custom entry permanently disabled':'PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY' in cmake and re.search(r'message\(FATAL_ERROR\s+"[^"\n]*static CRT does not support the historical custom-entry comparison',cmake) is not None,
 'MinHook vendored static dependency':'add_library(minhook STATIC' in cmake and 'third_party/minhook/upstream' in cmake,
 'MinHook exact pin':minhook_pin in cmake and (r/'third_party/minhook/UPSTREAM_COMMIT.txt').read_text().strip()==minhook_pin,
 'MinHook safe-freeze patch staged':'PatchMinHookSafeFreeze.cmake' in cmake and '_preloader_l_minhook_patched' in cmake,
 'MinHook network fallback absent':all(x not in cmake for x in ['FetchContent','GIT_REPOSITORY','PRELOADER_L_MINHOOK_SOURCE_DIR']),
 'MinHook upstream CMake bypassed':'add_subdirectory("${_preloader_l_minhook_patched}"' not in cmake,
 'loader-lock MinHook mutation absent':'install_early_minhook_surface()' not in read('src/runtime/secondary_integration_entry.cpp') and 'MH_ApplyQueued' not in read('src/runtime/secondary_integration_entry.cpp'),
 'post-loader safe-freeze contract':all(source_policy.get(n,False) for n in ('loader_lock_does_not_install_hooks','config_before_hooks','single_transaction_order','rollback_contract')) and 'NtGetNextThread' in read('scripts/build_orchestration/PatchMinHookSafeFreeze.cmake'),
 'shipping MinHook link':'target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)' in cmake,
 'shipping MinHook runtime matched':'set_property(TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")' in cmake,
 'debug MinHook target':'add_library(minhook_debug STATIC' in cmake and 'set_property(TARGET minhook_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")' in cmake,
 'debug MinHook and CRT-matched TinyXML2 link':'target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook_debug preloader_l_tinyxml2_debug)' in cmake and 'set_property(TARGET preloader_l_tinyxml2_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")' in cmake,
 'debug does not reuse shipping MinHook':'target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook)' not in cmake,
 'custom MASM hook path absent':'enable_language(ASM_MASM)' not in cmake and 'crypt_query_tail_wrapper_x64.asm' not in cmake,
 'custom inline engine absent':all(x not in cmake for x in ['inline_hook_backend_windows.cpp','inline_hook_allocator_windows.cpp','inline_hook_page_trap_windows.cpp','inline_hook_patch_windows.cpp','x64_trampoline_relocator.cpp']),
 'IAT hook engine absent':'iat_hook_backend_windows.cpp' not in cmake and 'surge_native_hooks_iat_windows.cpp' not in cmake,
 'release host optimized':'target_compile_options(preloader_l_release_host PRIVATE /W4 /permissive- /O2 /GS /GR-)' in cmake,
 'release host debug override absent':'target_link_options(preloader_l_release_host PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /DEBUG:NONE)' in cmake,
 'release host codec linked':all(x in cmake for x in ['src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/codec/message_codec.cpp','src/codec/session_crypto.cpp','target_include_directories(preloader_l_release_host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)']),
 'native host codec linked':all(x in cmake for x in ['add_executable(preloader_l_native_acceptance_host','src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/codec/message_codec.cpp','src/codec/session_crypto.cpp']),
 'native host no IAT-only advapi retention':'target_link_libraries(preloader_l_native_acceptance_host PRIVATE kernel32 ws2_32 preloader_l_tinyxml2)' in cmake and 'RegOpenKeyExA(nullptr' not in native_host,
 'native build shared helper':". (Join-Path $PSScriptRoot 'build_common.ps1')" in native and 'Clear-BuildSelectorEnvironment' in native,
 'CRT build shared helper':". (Join-Path $PSScriptRoot 'build_common.ps1')" in crt and 'Clear-BuildSelectorEnvironment' in crt,
 'CRT host variable safe':'$AcceptanceHost=Join-Path' in crt and '$Host=Join-Path' not in crt,
 'release host built by CRT acceptance':"@('preloader_l','preloader_l_release_host')" in crt,
 'shipping blackbox executed':'SHIPPING_BLACKBOX' in crt and 'preloader_l_release_host.exe' in crt,
 'native custom hook campaigns retired':all(x not in native for x in ['iat_fallback_restore','inline_hook_quiescence','cleanup_fault_injection','hook_install_uninstall']),
 'CRT custom hook campaigns retired':all(x not in crt for x in ['iat_fallback_restore','inline_hook_quiescence','cleanup_fault_injection','hook_install_uninstall']),
 'PTD campaign retained':'ptd_fls_lifecycle' in native and 'ptd_fls_lifecycle' in crt,
 'protocol campaigns retained':all(x in native and x in crt for x in ['frame_fragmentation','frame_coalescing','frame_malformed','protocol_route_surface','store_content_entitlement','achievement_event_delivery','f02_delivery_stall_parallel']),
 'release host shutdown before unload':'DllShutdown' in release_host and release_host.index('shutdown_runtime(30000u)') < release_host.rindex('FreeLibrary(module)'),
 'link checker default CRT policy':'custom /ENTRY is forbidden' in checker and '/NODEFAULTLIB is forbidden' in checker,
 'link checker manifest policy':'missing /MANIFEST:NO' in checker,
 'link checker pogo policy':'missing /LTCG:OFF' in checker and 'missing /NOCOFFGRPINFO' in checker,
 'finalizer fail closed':'normalized shipping build must emit no Debug Directory' in finalizer,
 'FALCION data footprint integrated':'[DATA_VIRTUAL_FOOTPRINT]' in build and 'Tee-Object -FilePath $FalcionLog -Append' in build,
}
failed=[k for k,v in checks.items() if not v]
for k,v in checks.items(): print(('PASS' if v else 'FAIL')+': '+k)
if failed:
    print('SHIPPING_BUILD_POLICY: FAIL')
    for x in failed: print(' -',x)
    sys.exit(1)

# Preserve the fail-closed PowerShell entry-surface contract.
p006_paths=(
 'scripts/build_orchestration/build_production.ps1','scripts/build_orchestration/build_debug.ps1',
 'scripts/build_orchestration/build_native_acceptance.ps1','scripts/build_orchestration/build_crt_acceptance.ps1',
 'scripts/test_production_cpp_syntax.ps1',
)
name='ErrorAction'+'Pre'+'ference'
assignment=re.compile(r'(?m)^\s*\$'+re.escape(name)+r'\s*=\s*(["\'])Stop\1\s*$')
errors=[]
for rel in p006_paths:
    body=read(rel)
    if len(list(assignment.finditer(body)))!=1: errors.append(rel+': fail-closed setting count')
    for hit in re.finditer(r'\$ErrorAction[A-Za-z0-9_]*',body,flags=re.I):
        if hit.group(0).lower()!=('$'+name).lower(): errors.append(rel+': '+hit.group(0))
if errors:
    print('P006_POWERSHELL_FAIL_CLOSED: FAIL')
    for x in errors: print(' -',x)
    sys.exit(1)
print(f'P006_POWERSHELL_FAIL_CLOSED: PASS ({len(p006_paths)}/{len(p006_paths)} static)')
subprocess.run([sys.executable,str(r/'scripts/check_msvc_shipping_link_command.py'),'--self-test'],check=True)
if os.name=='nt':
    shell=shutil.which('powershell.exe') or shutil.which('pwsh.exe')
    if not shell:
        print('BUILD_EXIT_PROPAGATION_WINDOWS: FAIL - PowerShell executable not found')
        sys.exit(1)
    subprocess.run([shell,'-NoProfile','-ExecutionPolicy','Bypass','-File',str(r/'scripts/test_build_environment.ps1'),'-Root',str(r)],check=True)
else:
    print('BUILD_EXIT_PROPAGATION_WINDOWS: PENDING (non-Windows source QA; exact Windows execution required)')
print(f'SHIPPING_BUILD_POLICY: PASS ({len(checks)}/{len(checks)})')
