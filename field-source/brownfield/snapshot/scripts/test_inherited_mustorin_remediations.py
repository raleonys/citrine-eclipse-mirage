#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, sys, tempfile
r=Path(__file__).resolve().parents[1]
errors=[]
def txt(rel):
    p=r/rel
    if not p.is_file():
        errors.append('missing '+rel); return ''
    return p.read_text(encoding='utf-8',errors='replace')

# Protocol/runtime semantic wave retained from MUSTORIN.
env=txt('src/platform/environment_policy.cpp')
cmd=txt('src/config/runtime_configuration_command_line.hpp')
framing=txt('src/xml/lsx_framing.cpp')
children=txt('src/xml/request_children.cpp')
parser=txt('src/xml/request_parser.cpp')
index_contract=txt('include/preloader_l/xml/request_parser.hpp')
exchange=txt('src/protocol/socket_exchange.cpp')
exchange_step=txt('src/protocol/socket_exchange_frame_step.cpp')
debug_sock=txt('src/debug/net/socket_session.cpp')
store_routes=txt('src/handlers/store_routes.cpp')
store_runtime=txt('src/config/store_runtime.cpp')
config=txt('src/config/runtime_configuration.cpp')
bootstrap=txt('src/runtime/bootstrap_thread.cpp')
ptd=txt('src/runtime/ptd_runtime.cpp')
events=txt('src/runtime/event_queue.cpp')
listener=txt('src/net/socket_listener.cpp')
endpoint=txt('src/net/socket_endpoint.cpp')
service=txt('src/runtime/release_service_lifetime.cpp')
cmake=txt('CMakeLists.txt')

checks={
 'environment distinguishes length-query error':'environment_length_query_error' in env and 'restored_mask' in env,
 'command line malformed quote/suffix':'CommandTokenStatus::malformed' in cmd and 'if(*p!=quote)' in cmd and '!command_ws(*p)' in cmd,
 'XML comment lexical mode':'LexMode::comment' in framing and '"-->"' in framing,
 'XML CDATA lexical mode':'LexMode::cdata' in framing and '"<![CDATA["' in framing,
 'XML processing instruction mode':'LexMode::processing_instruction' in framing and '"?>"' in framing,
 'DOM owns decoded CDATA/first-text and cached child-trivia':all(k in children for k in ['ToText()', 'FirstChildElement()', 'valid_trivia']) and 'dom::child_index(command)' in parser and 'DirectChildIndex' in index_contract,
 'malformed frame budget retained':'kMalformedFrameLimit = 32u' in exchange_step and 'tolerate_malformed_frame' in exchange_step and 'classify_received_wire_frame' in exchange,
 'invalid hex is peer malformed not terminal':'MessageCodecStatus::invalid_hex' in exchange_step and 'malformed_streak' in exchange,
 'debug reaccept ownership guard':'already_connected' in debug_sock or 'already-connected' in debug_sock,
 'entitlement filters decode before match':'source_equals_service' in store_routes and 'malformed' in store_routes and 'FilterProbe' in store_routes,
 'store generation reader leases':'generation' in store_runtime and 'reader' in store_runtime and 'retire' in store_runtime,
 'configuration generation transaction':'g_policy_readers' in config and 'g_policy_writer' in config and 'g_policy_generation' in config,
 'configuration active readers fail closed':'g_policy_readers!=0u' in config and 'resource_error' in config,
 # Ownership/lifecycle wave that remains application-level under MinHook.
 'bootstrap environment cleanup retained':'g_environment_restore_mask' in bootstrap and 'retry_bootstrap_pin_release()' in bootstrap,
 'bootstrap handle close retry':'ReleaseBootstrapWaitResult::handle_close_failed' in bootstrap,
 'PTD pending ownership':'retry_pending_ptds' in ptd and 'ptd_cleanup_pending()' in ptd and 'retain_pending_ptd' in ptd,
 'event free retry ownership':'g_pending_free_head' in events and 'retry_pending_frees()' in events and 'g_pending_free_count==0' in events,
 'socket close retry ownership':'SocketStatus close_socket_listener' in listener and 'api->close(api->context, listener->socket) != 0' in listener,
 'Audit3 S0445 accept preserves failed-close owner':'if (endpoint->open) return SocketStatus::already_started;' in endpoint and endpoint.find('if (endpoint->open) return SocketStatus::already_started;') < endpoint.find('reset_socket_endpoint(endpoint);', endpoint.find('SocketStatus accept_socket_endpoint(')),
 'service resource cleanup pending':'ReleaseServiceShutdownResult::resource_cleanup_pending' in service and 'clear_queued_events()' in service,
 'release host optimized':'target_compile_options(preloader_l_release_host PRIVATE /W4 /permissive- /O2 /GS /GR-)' in cmake,
 'release host debug override removed':'target_link_options(preloader_l_release_host PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /DEBUG:NONE)' in cmake,
 # MinHook fork supersedes custom hook-engine remediations rather than reintroducing them.
 'custom IAT backend absent':not (r/'src/platform/iat_hook_backend_windows.cpp').exists(),
 'custom inline engine absent':not (r/'src/platform/inline_hook_backend_windows.cpp').exists(),
 'Surge IAT backend absent':not (r/'src/platform/surge_native_hooks_iat_windows.cpp').exists(),
 'MinHook authority retained':(r/'src/platform/minhook_runtime.cpp').is_file() and 'minhook' in cmake.lower(),
}
for k,v in checks.items():
    print(('PASS' if v else 'FAIL')+': '+k)
    if not v: errors.append(k)
if errors:
    print('INHERITED_MUSTORIN_REMEDIATIONS: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)

# Execute the new platform-independent failure/ownership regressions where the
# current environment can model them exactly.
cxx=shutil.which('c++') or shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('INHERITED_MUSTORIN_REMEDIATIONS: FAIL - no C++ compiler')
    sys.exit(1)
with tempfile.TemporaryDirectory(prefix='inherited_mustorin_regress_') as td_s:
    td=Path(td_s)
    def run(name,sources,defs=()):
        exe=td/name
        cmdline=[cxx,'-std=c++17',*defs,'-I'+str(r/'include'),*map(str,sources),'-o',str(exe)]
        p=subprocess.run(cmdline,cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        if p.returncode:
            print('INHERITED_MUSTORIN_REMEDIATIONS: FAIL - compile '+name)
            print(p.stdout[-2000:]); print(p.stderr[-4000:]); sys.exit(1)
        p=subprocess.run([str(exe)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        if p.returncode:
            print(f'INHERITED_MUSTORIN_REMEDIATIONS: FAIL - runtime {name} exit {p.returncode}')
            print(p.stdout[-2000:]); print(p.stderr[-4000:]); sys.exit(1)
        print('PASS: executable '+name)
    run('runtime_command_line',[r/'tests/unit/test_config_command_line.cpp'])
    run('ptd_retry',[r/'tests/unit/test_ptd_retry_ownership.cpp',r/'src/runtime/ptd_runtime.cpp'])
    stub=td/'errno_stub.cpp'
    stub.write_text('namespace preloader_l { namespace runtime { int* current_errno_slot() noexcept { static int v=0; return &v; } } }\n')
    run('allocator_release_retry',[r/'tests/unit/test_allocator_release_failure.cpp',r/'src/runtime/memory_allocator.cpp',stub])
    run('socket_close_retry',[r/'tests/unit/test_socket_close_retry.cpp',r/'src/net/socket_listener.cpp',r/'src/net/socket_endpoint.cpp',r/'src/net/socket_runtime.cpp'])
    run('event_free_retry',[r/'tests/unit/test_event_queue_free_retry.cpp',r/'src/runtime/event_queue.cpp'],['-D_WIN32','-D__stdcall=','-D__declspec(x)='])
print(f'INHERITED_MUSTORIN_REMEDIATIONS: PASS ({len(checks)}/{len(checks)} source + 5/5 executable)')
