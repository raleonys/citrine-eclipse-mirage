#!/usr/bin/env python3
"""Mutation probes: detect /MDd TinyXML2 mismatch and softened native evidence."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[1]
cm=(root/'CMakeLists.txt').read_text()
host=(root/'tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp').read_text()
check=(root/'scripts/check_tinyxml2_target_contract.py').read_text()
mutations=[
 ('native_missing_vendor_link',lambda c,h:(c.replace('kernel32 ws2_32 preloader_l_tinyxml2)','kernel32 ws2_32)'),h)),
 ('debug_missing_link',lambda c,h:(c.replace('minhook_debug preloader_l_tinyxml2_debug)','minhook_debug)'),h)),
 ('cross_crt_debug_link',lambda c,h:(c.replace('minhook_debug preloader_l_tinyxml2_debug)','minhook_debug preloader_l_tinyxml2)'),h)),
 ('debug_wrong_crt',lambda c,h:(c.replace('TARGET preloader_l_tinyxml2_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug"','TARGET preloader_l_tinyxml2_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"'),h)),
 ('event_padding_gate_missing',lambda c,h:(c,h.replace('strict_pkcs7_xml_length(event_xml,event_n,&event_xml_n)','false /* removed event padding validation */'))),
 ('response_padding_gate_missing',lambda c,h:(c,h.replace('strict_pkcs7_xml_length(response_xml,response_n,&response_xml_n)','false /* removed response padding validation */'))),
 ('softened_native',lambda c,h:(c,h.replace('return event_ok;','return true;'))),
 ('missing_response_gate',lambda c,h:(c,h.replace('if(!achievement_frame_contract::response(', 'if(!false /* weakened */ && !achievement_frame_contract::response('))),
]
with tempfile.TemporaryDirectory(prefix='tinyxml2-windows-target-check-') as d:
    q=Path(d);(q/'scripts').mkdir();(q/'tools/preloader_l_native_acceptance_host').mkdir(parents=True)
    (q/'scripts/check_tinyxml2_target_contract.py').write_text(check)
    (q/'tools/preloader_l_native_acceptance_host/achievement_frame_contract.hpp').write_text((root/'tools/preloader_l_native_acceptance_host/achievement_frame_contract.hpp').read_text())
    total=0
    for label,mutate in [('positive',lambda c,h:(c,h))]+mutations:
        c,h=mutate(cm,host)
        (q/'CMakeLists.txt').write_text(c)
        (q/'tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp').write_text(h)
        p=subprocess.run([sys.executable,str(q/'scripts/check_tinyxml2_target_contract.py')],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        expect=0 if label=='positive' else 1
        if (p.returncode==0)!=(expect==0):
            print(f'TINYXML2_WINDOWS_TARGET_NEGATIVES: FAIL ({label}):\n{p.stdout}\n{p.stderr}')
            sys.exit(1)
        total+=1
print(f'TINYXML2_WINDOWS_TARGET_NEGATIVES: PASS ({total}/{total})')
