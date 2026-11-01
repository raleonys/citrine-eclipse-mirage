#!/usr/bin/env python3
"""Fail-closed Stage05 source-linked WinSock lifecycle parity and ownership."""
from pathlib import Path
import subprocess, shutil, sys, tempfile, re
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text()
prod=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',cm,re.S)
native=re.search(r'if\(PRELOADER_L_NATIVE_ACCEPTANCE\)(.*?)\nelse\(\)',cm,re.S)
debug=re.search(r'add_library\(preloader_l_debug SHARED(.*?)\n    \)',cm,re.S)
source=(r/'src/net/winsock_lifecycle_native.cpp').read_text()
checks=[]
def check(name,ok):checks.append((name,bool(ok)))
check('Production lifecycle module',prod and 'src/net/winsock_lifecycle_native.cpp' in prod.group(1))
check('Production excludes legacy full adapter',prod and 'src/net/winsock_native.cpp' not in prod.group(1))
check('Native full adapter retained',native and 'src/net/winsock_native.cpp' in native.group(1))
check('Debug full adapter retained',debug and 'src/net/winsock_native.cpp' in debug.group(1))
check('Full adapter parent is unchanged', 'const SocketApi* native_winsock_api() noexcept' in (r/'src/net/winsock_native.cpp').read_text())
check('Conditional avoids duplicate exported provider', '#if !defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in source)
check('Version negotiation preserved', 'winsock_negotiated_version_supported(negotiated)' in source)
check('Compensating cleanup retry token', 'winsock_version_cleanup_retry_required' in source)
check('No legacy raw Winsock imports',not re.search(r'__declspec\(dllimport\).*\b(?:socket|bind|listen|accept|send|recv|closesocket)\s*\(',source))
check('SocketRuntime kept byte-domain', 'src/net/socket_runtime.cpp' in prod.group(1))
check('No fake raw-socket callbacks in production',
      'reject_open' not in source and 'reject_send' not in source and
      'nullptr, startup, cleanup, nullptr' in source)
check('Source-level lifecycle-only validation is explicit',
      'valid_socket_lifecycle_api' in (r/'src/net/socket_runtime.cpp').read_text() and
      'full_socket_api && !valid_socket_api(*api)' in (r/'src/net/socket_runtime.cpp').read_text())
check('Asio release bridge calls lifecycle-only entrypoint',
      'start_socket_runtime_lifecycle(&context->runtime,context->socket_api)' in
      (r/'src/runtime/asio_release_bridge.cpp').read_text())
check('Strict legacy entrypoint preserved for native/debug',
      'return start_socket_runtime_impl(runtime, api, true);' in
      (r/'src/net/socket_runtime.cpp').read_text())

clang=shutil.which('clang++')
check('Clang C++ compiler available',clang is not None)
if clang:
 with tempfile.TemporaryDirectory(prefix='ids05_lifecycle_') as td:
  base=[clang,'-std=c++20','-fms-extensions','-Wno-ignored-attributes','-D_WIN32',
        '-I'+str(r/'include')]
  new=str(r/'src/net/winsock_lifecycle_native.cpp')
  old=str(r/'src/net/winsock_native.cpp')
  runtime=str(r/'src/net/socket_runtime.cpp')
  test=str(r/'tests/unit/test_winsock_lifecycle_native.cpp')
  # Native x86-64 functions use the same source code; this offline compile is
  # NOT a Windows MSVC /MT or true IOCP execution.
  for mode in ('shipping','full_native'):
   cmd=base[:]
   if mode=='full_native':cmd+=['-DIDS_TEST_FULL_NATIVE=1']
   cmd += [test,runtime,new if mode=='shipping' else old,'-o',td+'/'+mode]
   p=subprocess.run(cmd,capture_output=True,text=True)
   check(mode+' source-linked compile',p.returncode==0)
   if p.returncode!=0:
    print(p.stderr[-2400:]);continue
   p=subprocess.run([td+'/'+mode],capture_output=True,text=True)
   check(mode+' source-linked runtime',p.returncode==0 and 'PASS' in p.stdout)
   if p.returncode!=0:print(p.stderr[-2200:])
  obj=td+'/shipping.o'
  p=subprocess.run(base+['-c',new,'-o',obj],capture_output=True,text=True)
  check('shipping provider object compile',p.returncode==0)
  if p.returncode==0:
   syms=subprocess.run(['nm','-u',obj],capture_output=True,text=True).stdout
   check('three WSA imports and no legacy transport imports',all(x in syms for x in ['WSAStartup','WSACleanup','WSAGetLastError']) and not any(re.search(r'\b'+x+r'\b',syms) for x in ('socket','bind','listen','accept','send','recv','closesocket')))
# Mutation tests: a production restore of legacy implementation must fail.
if prod:
 changed=cm.replace('    src/net/winsock_lifecycle_native.cpp','    src/net/winsock_native.cpp',1)
 updated=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',changed,re.S)
 check('reject native raw provider in shipping',updated and 'src/net/winsock_native.cpp' in updated.group(1))
failed=[name for name,ok in checks if not ok]
print('ASIO_WINSOCK_LIFECYCLE_STAGE05:', 'FAIL' if failed else 'PASS',len(checks)-len(failed),'/',len(checks))
for x in failed:print(' -',x)
sys.exit(1 if failed else 0)
