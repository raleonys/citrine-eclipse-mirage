#!/usr/bin/env python3
from pathlib import Path
import re,sys,hashlib
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text()
m=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',cm,re.S)
errors=[]
if not m:
    errors.append('production target block not found')
    sources=set()
else:
    sources=set(re.findall(r'(?m)^\s*([A-Za-z0-9_./-]+\.(?:cpp|c|rc|def|asm))\s*$',m.group(1)))
    # The exact approved source set is externally hash-bound by the universal
    # policy contract, not hard-coded to a historical source count or digest.
    from check_universal_shipping_policy import validate
    import os
    pin = os.environ.get('DLL_RD_SHIPPING_CONTRACT_SHA256')
    policy_checks = validate(r, r/'metadata/SHIPPING_POLICY_CONTRACT.json', pin)
    for gate, passed in policy_checks.items():
        if not passed:
            errors.append('universal shipping contract: '+gate)
    allowed_prefixes=('src/','resources/','exports/')
    for item in sorted(sources):
        if not item.startswith(allowed_prefixes):
            errors.append('non-production root leaked into production target '+item)
# Asio-only production gate: native jthread accept/worker units must remain
# outside both shipping and Debug targets. The native acceptance target is
# allowed to retain them solely as bounded fault-injection test machinery.
# Stage03 production excludes legacy mock listener/endpoint translation units.
# Debug probes and explicit native acceptance still require these real units.
# Shipping may use only the lifecycle provider; native fault injection must
# retain the original complete WinSock adapter and the optional debug probe.
if 'src/net/winsock_native.cpp' in sources or 'src/net/winsock_lifecycle_native.cpp' not in sources:
    errors.append('shipping Winsock lifecycle/full adapter ownership regression')
if 'src/net/winsock_native.cpp' not in cm.split('if(PRELOADER_L_NATIVE_ACCEPTANCE)',1)[1]:
    errors.append('native fault injection lost complete WinSock adapter')
for legacy_net in ('src/net/socket_listener.cpp', 'src/net/socket_endpoint.cpp', 'src/net/socket_transport.cpp'):
    if legacy_net in sources:
        errors.append('legacy mock socket returned to Production: '+legacy_net)
    debug_m = re.search(r'add_library\(preloader_l_debug SHARED(.*?)\n    \)',cm,re.S)
    if not debug_m or not re.search(r'(?m)^\s*'+re.escape(legacy_net)+r'\s*$',debug_m.group(1)):
        errors.append('debug diagnostic socket dependency missing: '+legacy_net)
    native_section = re.search(r'if\(PRELOADER_L_NATIVE_ACCEPTANCE\)(.*?)\nelse\(\)',cm,re.S)
    if not native_section or legacy_net not in native_section.group(1):
        errors.append('native fault-injection socket dependency missing: '+legacy_net)
for legacy in ('src/runtime/release_server_worker.cpp',
               'src/runtime/release_server_acceptor.cpp'):
    if legacy in sources:
        errors.append('legacy worker returned to Production: '+legacy)
    debug_m = re.search(r'add_library\(preloader_l_debug SHARED(.*?)\n    \)',cm,re.S)
    if debug_m and re.search(r'(?m)^\s*'+re.escape(legacy)+r'\s*$',debug_m.group(1)):
        errors.append('legacy worker returned to Debug: '+legacy)
    conditional = re.search(r'if\(PRELOADER_L_NATIVE_ACCEPTANCE\)(.*?)\nelse\(\)',cm,re.S)
    if not conditional or legacy not in conditional.group(1):
        errors.append('native-only historical worker fault-injection owner missing: '+legacy)

required={
    'src/runtime/exception_runtime.cpp',
    'src/runtime/fatal_policy.cpp',
    'src/runtime/fatal_termination.cpp',
    'src/runtime/memory_allocator.cpp',
    'src/runtime/heap_allocator.cpp',
    'src/runtime/release_service_lifetime.cpp',
    'src/runtime/asio_listener_owner.cpp',
    'src/runtime/asio_session_owner.cpp',
    'src/runtime/asio_service_owner.cpp',
    'src/runtime/asio_release_bridge.cpp',
    'src/config/byte_vector_binding.cpp',
    'src/protocol/socket_exchange_frame_step.cpp',
    'src/protocol/socket_exchange_write_completion.cpp',
    'src/protocol/socket_exchange_async_transaction.cpp',
    'src/protocol/socket_exchange_transaction_delivery.cpp',
    'src/platform/minhook_runtime.cpp',
}
# A removed parser is forbidden BOTH as a production TU and as an active file.
for retired_xml in [
    'src/xml/request_scanner.cpp', 'src/xml/request_internal.hpp',
    'src/xml/tinyxml2_compat_bridge.hpp',
    'src/config/store_xml.cpp', 'src/config/store_entitlements.cpp',
]:
    if retired_xml in sources or (r/retired_xml).exists():
        errors.append('retired custom XML parser returned '+retired_xml)
# historical single-document architecture: no reparse of synthetic child / tag snippets,
# no printer-mediated input reconstruction and no route-private XML scanner.
for rel in ['src/xml/request_parser.cpp','src/xml/request_children.cpp',
            'src/xml/attribute_parser.cpp','src/xml/tinyxml2_dom.hpp']:
    body=(r/rel).read_text()
    if re.search(r'\b[A-Za-z]+[0-9]+_CHILDREN\b', body):
        errors.append('retired secondary XML projection token: '+rel)
    for forbidden in ('XMLPrinter','dom::child_markup(',
                      'dom::start_tag(', 'self_closed.insert('):
        if forbidden in body:
            errors.append('nonminimal secondary XML projection returned: '+rel+' '+forbidden)
for rel in ['src/xml/attribute_parser.cpp','src/xml/request_children.cpp']:
    body=(r/rel).read_text()
    if 'if (!tag.element || !tag.document)' not in body and rel.endswith('attribute_parser.cpp'):
        errors.append('attribute DOM ownership requirement missing')
    if ('if (!content.element || !content.document)' not in body and
        rel.endswith('request_children.cpp')):
        errors.append('child DOM ownership requirement missing')
# CORVEXON regression: shipping request-child indexing must not introduce
# MSVCP140!std::_Xlength_error through a growing std::vector.
for rel in ['src/xml/request_parser.cpp', 'src/xml/request_children.cpp',
            'include/preloader_l/xml/request_parser.hpp']:
    body=(r/rel).read_text()
    if 'std::vector<' in body or '_Xlength_error' in body:
        errors.append('forbidden STL growth/import path in TinyXML2 child index: '+rel)
if 'std::unique_ptr<const tinyxml2::XMLElement*[]>' not in \
        (r/'include/preloader_l/xml/request_parser.hpp').read_text():
    errors.append('bounded owned TinyXML2 child-index representation missing')
if 'target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)' not in cm or \
   'add_library(preloader_l_tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp)' not in cm:
    errors.append('TinyXML2 production link or compiled source missing')
for required_xml in ['src/xml/request_parser.cpp','src/xml/request_children.cpp',
                     'src/xml/attribute_parser.cpp','src/config/store_parser.cpp']:
    if required_xml not in sources:
        errors.append('missing TinyXML2 DOM-backed production TU '+required_xml)
required.add('src/config/config2_pegtl.cpp')
# historical filesystem path ownership: original DLL-relative Win32 authority and
# read/open error mappings remain in place; only lexical composition changes.
if not (r/'src/config/module_relative_filesystem.hpp').is_file():
    errors.append('historical shared native filesystem path helper missing')
if '#include "../config/module_relative_filesystem.hpp"' not in (r/'src/runtime/avatar_path.cpp').read_text():
    errors.append('avatar path bypasses historical filesystem helper')
if '#include "module_relative_filesystem.hpp"' not in (r/'src/config/process_configuration.cpp').read_text():
    errors.append('configuration path bypasses historical filesystem helper')
if 'third_party/filesystem/upstream/include' not in cm or re.search(r'\b_[A-Za-z0-9_]*filesystem_pin\b', cm) is None:
    errors.append('historical filesystem include or pinned upstream contract missing')
if 'src/config/config2_pegtl.cpp' not in sources:
    errors.append('historical shared Config2 grammar TU missing from shipping graph')
if not (r/'src/config/config2_pegtl.hpp').is_file():
    errors.append('historical shared Config2 grammar header missing')
if '#include "config2_pegtl.hpp"' not in (r/'src/config/runtime_configuration_parser.cpp').read_text():
    errors.append('runtime configuration projection bypasses historical PEGTL grammar')
if '#include "../config/config2_pegtl.hpp"' not in (r/'src/runtime/service_state_parser.cpp').read_text():
    errors.append('service state projection bypasses historical PEGTL grammar')
if 'third_party/pegtl/upstream/include' not in cm or 'cxx_std_20' not in cm:
    errors.append('historical PEGTL include root or C++20 contract missing')
for item in sorted(required-sources):
    errors.append('missing production source '+item)
for forbidden in [
    'tests/native/native_acceptance_exports.cpp',
    'src/runtime/build_identity.cpp',
    'src/debug/runtime_memory.cpp',
    'src/platform/x64_hook_allocator_model.cpp',
    'src/runtime/exception_policy.cpp',
    'src/debug/evidence/x64_inline_hook_plan.cpp',
    'src/debug/evidence/crypt_guard.cpp',
    'src/debug/evidence/crypt_path_restore.cpp',
    'src/debug/evidence/architecture_provenance.cpp',
]:
    if forbidden in sources:
        errors.append('non-production source leaked into normalized graph '+forbidden)
if 'target_sources(preloader_l PRIVATE\n        tests/native/native_acceptance_exports.cpp\n        exports/preloader_l_native_acceptance.def' not in cm:
    errors.append('native acceptance source is not scoped to PRELOADER_L_NATIVE_ACCEPTANCE')
if (r/'tests/support/architecture_hook_allocator_model.cpp').exists():
    errors.append('retired custom-hook allocator support TU returned to active tests')
try:
    import json
    mig=json.loads((r/'metadata/DOCUMENTATION_HISTORY_MIGRATION.json').read_text())
    rec=next((x for x in mig.get('files',[]) if x.get('source_path')=='docs/historical/retired_custom_hook_tests/architecture_hook_allocator_model.cpp'),None)
    if not rec or rec.get('file_disposition')!='PROVENANCE_ONLY_WITH_IMMUTABLE_LIBRARY_BINDING':
        errors.append('retired custom-hook allocator provenance binding missing')
except Exception as exc:
    errors.append('retired custom-hook allocator provenance unreadable: '+repr(exc))
if (r/'src/runtime/native_acceptance_windows.cpp').exists():
    errors.append('native acceptance implementation regained production runtime topology')
if not (r/'tests/native/native_acceptance_exports.cpp').is_file():
    errors.append('native acceptance test export implementation missing')
if '$<$<COMPILE_LANGUAGE:CXX>:' not in cm:
    errors.append('production C++ flags are not language-scoped')
if cm.count('src/debug/evidence/architecture_provenance.cpp') != 1 or 'PRELOADER_L_ENABLE_DEBUG_PROTOTYPE' not in cm:
    errors.append('x64 provenance diagnostic TU must be debug-only and absent from normalized production graph')
if '/DEBUG:NONE' not in cm:
    errors.append('shipping debug directory is not explicitly stripped')
if 'resources/preloader_version.rc' not in cm:
    errors.append('allowlisted VERSIONINFO resource missing from normalized graph')
if 'set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")' not in cm:
    errors.append('normalized production /MD CRT runtime property missing')
if 'target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)' not in cm:
    errors.append('normalized production Win32 + MinHook link surface drift')
if 'kernelbase' in cm.lower() or 'synchronization' in cm.lower():
    errors.append('forbidden historical linker workaround returned')

for retired in [
    'src/platform/inline_hook_backend_windows.cpp',
    'src/platform/inline_hook_quiescence.cpp',
    'src/platform/inline_hook_allocator_windows.cpp',
    'src/platform/inline_hook_page_trap_windows.cpp',
    'src/platform/inline_hook_patch_windows.cpp',
    'src/platform/x64_trampoline_relocator.cpp',
    'src/platform/model_hook_allocator_strict_windows.cpp',
    'src/platform/iat_hook_backend_windows.cpp',
    'src/platform/surge_native_hooks_iat_windows.cpp',
    'src/platform/crypt_query_tail_wrapper_x64.asm',
    'src/platform/x64_hook_allocator_model.cpp',
    'src/runtime/exception_policy.cpp',
]:
    if (r/retired).exists():
        errors.append('production-looking non-shipping implementation path returned '+retired)
for support in [
    'tests/support/exception_policy.cpp',
]:
    if not (r/support).is_file():
        errors.append('missing explicit test-support implementation '+support)

aw=(r/'src/runtime/heap_allocator.cpp').read_text()
if '#if defined(PRELOADER_L_ENABLE_ALLOCATOR_THROW_BRIDGE)' not in aw:
    errors.append('non-shipping allocator throw bridge is not compile-gated')
if errors:
    print('PRODUCTION_SOURCE_GRAPH: FAIL')
    [print(' -',e) for e in errors]
    sys.exit(1)
print('PRODUCTION_SOURCE_GRAPH: PASS')
