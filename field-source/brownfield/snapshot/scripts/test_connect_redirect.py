#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, sys, tempfile
r=Path(__file__).resolve().parents[1]
errors=[]
def t(rel):
    p=r/rel
    if not p.is_file(): errors.append('missing '+rel); return ''
    return p.read_text(encoding='utf-8',errors='replace')

cm=t('CMakeLists.txt')
core=t('src/platform/connect_redirect.cpp')
win=t('src/platform/connect_redirect_hook.cpp')
boot=t('src/runtime/bootstrap_thread.cpp')
winsock=t('src/net/winsock_native.cpp')
host=t('tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp')
checks={
 'old source removed':not (r/'src/platform/htons_redirect.cpp').exists() and not (r/'src/platform/htons_redirect_windows.cpp').exists() and not (r/'include/preloader_l/platform/htons_redirect.hpp').exists(),
 'production graph connect only':'src/platform/connect_redirect.cpp' in cm and 'src/platform/connect_redirect_hook.cpp' in cm and 'htons_redirect.cpp' not in cm,
 'connect export target':'GetProcAddress(ws2, "connect")' in win and 'GetProcAddress(ws2, "htons")' not in win,
 'exact sockaddr length':'name_len != static_cast<int>(sizeof(SockAddrIn))' in win,
 'caller address copied':'SockAddrIn local{};' in win and 'target[i] = source[i]' in win,
 'loopback predicate':all(x in core for x in ['address_bytes[0] != 127u','address_bytes[1] != 0u','address_bytes[2] != 0u','address_bytes[3] != 1u']),
 'legacy port predicate':'kLegacyLsxPort = 3216u' in core,
 'selected port publication':'set_connect_selected_port(port);' in boot,
 'loopback listener':'address.address = 0x0100007fu;' in winsock,
 'reuse fail closed':'setsockopt(' in winsock and ') == kSocketError' in winsock,
 'native mapped proof':'open_prehandshake_client(3216u)' in host and 'connect_metric(2u)<=mapped0' in host,
 'native direct passthrough proof':'open_prehandshake_client(static_cast<uint16_t>(selected))' in host and 'connect_metric(2u)!=mapped1' in host,
 'native htons ordinary':'htons(3216u)!=net16(3216u)' in host,
}
for k,v in checks.items():
    if not v: errors.append(k)
if errors:
    print('WAVE3_CONNECT_REDIRECT_CONTRACT: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)

cxx=shutil.which('clang++') or shutil.which('g++')
if not cxx:
    print('WAVE3_CONNECT_REDIRECT_CONTRACT: FAIL - no C++ compiler')
    sys.exit(1)
with tempfile.TemporaryDirectory(prefix='wave3_connect_redirect_') as td:
    exe=Path(td)/'connect_redirect'
    proc=subprocess.run([cxx,'-std=c++17','-Wall','-Wextra','-Werror','-I',str(r/'include'),
                         str(r/'tests/unit/test_connect_redirect.cpp'),str(r/'src/platform/connect_redirect.cpp'),
                         '-o',str(exe)],cwd=r,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if proc.returncode:
        print('WAVE3_CONNECT_REDIRECT_CONTRACT: FAIL - compile')
        print(proc.stdout); print(proc.stderr); sys.exit(1)
    proc=subprocess.run([str(exe)],cwd=r)
    if proc.returncode:
        print('WAVE3_CONNECT_REDIRECT_CONTRACT: FAIL - runtime '+str(proc.returncode)); sys.exit(1)
print(f'WAVE3_CONNECT_REDIRECT_CONTRACT: PASS ({len(checks)}/{len(checks)} static + executable)')
