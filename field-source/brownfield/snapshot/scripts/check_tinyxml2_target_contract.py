#!/usr/bin/env python3
"""HF11 architecture gate: one pinned XML engine, CRT-matched Windows targets.

No runtime/native PASS is implied by static source checks. This checker is
intentionally separate from the old CRT comparison-source policy.
"""
from pathlib import Path
import re
import sys
root=Path(__file__).resolve().parents[1]
cm=(root/'CMakeLists.txt').read_text()
host=(root/'tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp').read_text()
errors=[]
need={
    'shipping TinyXML2 built from pinned vendor': r'add_library\(preloader_l_tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp\)',
    'debug TinyXML2 built from identical vendor': r'add_library\(preloader_l_tinyxml2_debug STATIC third_party/tinyxml2/tinyxml2.cpp\)',
    'shipping TinyXML2 /MD': r'set_property\(TARGET preloader_l_tinyxml2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"\)',
    'debug TinyXML2 /MDd': r'set_property\(TARGET preloader_l_tinyxml2_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug"\)',
    'shipping links TinyXML2': r'target_link_libraries\(preloader_l PRIVATE preloader_l_tinyxml2\)',
    'native acceptance host links same TinyXML2': r'target_link_libraries\(preloader_l_native_acceptance_host PRIVATE kernel32 ws2_32 preloader_l_tinyxml2\)',
    'debug links Debug MinHook and TinyXML2': r'target_link_libraries\(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook_debug preloader_l_tinyxml2_debug\)',
}
for label,pattern in need.items():
    if len(re.findall(pattern,cm))!=1: errors.append(label)
block=cm.split('if(PRELOADER_L_ENABLE_DEBUG_PROTOTYPE)',1)
if len(block)!=2:errors.append('missing optional debug target boundary')
else:
    if 'add_library(preloader_l_tinyxml2_debug STATIC' not in block[1]: errors.append('debug vendor target outside Debug option')
    if 'preloader_l_tinyxml2_debug' in block[0]: errors.append('Debug-only vendor library leaked into shipping portion')
if 'target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook_debug preloader_l_tinyxml2)' in cm:
    errors.append('cross CRT /MD archive leaked into /MDd Debug target')
# Native scenario failure remains fail-closed, but must yield privacy-safe evidence.
if not all(x in host for x in ['NATIVE_ACH_TWO_FRAME_DIAG: stage=response', 'NATIVE_ACH_TWO_FRAME_DIAG: stage=event', 'recv_achievement_frames(c,achievement_id)', 'achievement_frame_contract::response(', 'achievement_frame_contract::event(', 'return event_ok;', 'if(!achievement_frame_contract::response(']):
    errors.append('native achievement independent-frame validation missing or failure softened')
if not all(x in host for x in [
        'strict_pkcs7_xml_length(response_xml,response_n,&response_xml_n)',
        'strict_pkcs7_xml_length(event_xml,event_n,&event_xml_n)',
        'response_n=response_xml_n;', 'event_n=event_xml_n;',
        'NATIVE_ACH_TWO_FRAME_DIAG: stage=response padding_failed',
        'NATIVE_ACH_TWO_FRAME_DIAG: stage=event padding_failed']):
    errors.append('native test host must validate PKCS#7 before TinyXML2 on both independent frames')
if 'finish(200u+(pr&0x1fu))' not in host: errors.append('native failure exit preservation missing')
contract=(root/'tools/preloader_l_native_acceptance_host/achievement_frame_contract.hpp').read_text()
if not all(x in contract for x in ['tinyxml2::XMLDocument doc;', 'doc.Parse(data,n)', 'only_element(root,e)', 'only_element(e,a)', 'id_matches(a,token,token_n)', 'memchr(data,0,n)', 'strict_pkcs7_xml_length(', 'padded_n&15u', 'pad>16u', 'decoded[i])!=pad']): errors.append('native XML shape verification is not DOM-based and fail-closed')
if errors:
    print('TINYXML2_WINDOWS_TARGETS: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print('TINYXML2_WINDOWS_TARGETS: PASS (static contract only, real MSVC/native still pending)')
