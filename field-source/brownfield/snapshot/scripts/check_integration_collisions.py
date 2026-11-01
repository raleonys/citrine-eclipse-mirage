from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]

def text(p): return (r/p).read_text(encoding='utf-8',errors='replace')

native=text('scripts/build_orchestration/build_native_acceptance.ps1')
crt=text('scripts/build_orchestration/build_crt_acceptance.ps1')
host=text('tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp')
exports=text('exports/preloader_l_native_acceptance.def')
cm=text('CMakeLists.txt')
checks={
 'native runner keeps f02': "'f02_delivery_stall_parallel'" in native,
 'native runner keeps connect': "'connect_redirect_lifecycle'" in native,
 'crt runner keeps f02': "'f02_delivery_stall_parallel'" in crt,
 'crt runner keeps connect': "'connect_redirect_lifecycle'" in crt,
 'retired scenario absent': "'htons_lifecycle'" not in native and "'htons_lifecycle'" not in crt,
 'host handles f02': 'equal(scenario,"f02_delivery_stall_parallel")' in host,
 'host handles connect': 'equal(scenario,"connect_redirect_lifecycle")' in host,
 'export ordinal connect 205': 'preloader_l_native_acceptance_connect_metric @205' in exports,
 'export ordinal f02 206': 'preloader_l_native_acceptance_f02_control @206' in exports,
 'old htons export absent': 'preloader_l_native_acceptance_htons_metric' not in exports,
 'production connect tus present': 'src/platform/connect_redirect.cpp' in cm and 'src/platform/connect_redirect_hook.cpp' in cm,
 'production htons tus absent': 'src/platform/htons_redirect.cpp' not in cm and 'src/platform/htons_redirect_windows.cpp' not in cm,
 'old htons source removed': not (r/'src/platform/htons_redirect.cpp').exists() and not (r/'src/platform/htons_redirect_windows.cpp').exists(),
 'wave1 bounded teardown retained': 'rollback_minhook_batch_transaction_with_timeout' in text('src/runtime/bootstrap_thread.cpp'),
 'wave2 send timeout retained': 'kSendIdleTimeoutMs = 5000u' in text('src/runtime/release_server_worker.cpp'),
}
failed=[k for k,v in checks.items() if not v]
for k,v in checks.items(): print(f"{'PASS' if v else 'FAIL'}: {k}")
if failed:
 print(f'CUMULATIVE_WAVE_COLLISION_CONTRACT: FAIL ({len(checks)-len(failed)}/{len(checks)})')
 sys.exit(1)
print(f'CUMULATIVE_WAVE_COLLISION_CONTRACT: PASS ({len(checks)}/{len(checks)})')
