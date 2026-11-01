#!/usr/bin/env python3
"""HF15 IDS Stage04: fail-closed shipping vs native/mock socket ownership."""
import hashlib
from pathlib import Path
import re, subprocess, shutil, sys, tempfile

r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text(encoding='utf-8')
server=(r/'src/runtime/release_server.cpp').read_text(encoding='utf-8')
checks=[]
def check(label, condition): checks.append((label, bool(condition)))
prod=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',cm,re.S)
debug=re.search(r'add_library\(preloader_l_debug SHARED(.*?)\n    \)',cm,re.S)
native=re.search(r'if\(PRELOADER_L_NATIVE_ACCEPTANCE\)(.*?)\nelse\(\)',cm,re.S)
check('production target exists',prod is not None)
check('native conditional exists',native is not None)
check('debug target exists',debug is not None)
for unit in ('src/net/socket_listener.cpp','src/net/socket_endpoint.cpp','src/net/socket_transport.cpp'):
    check('no Production legacy: '+unit, bool(prod) and unit not in prod.group(1))
    check('native compatibility: '+unit, bool(native) and unit in native.group(1))
    check('Debug diagnostic compatibility: '+unit, bool(debug) and unit in debug.group(1))
check('explicit shipping Asio-only branch', '#if !defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in server and
      'context->socket_api != preloader_l::net::native_winsock_api()' in server and
      'return start_release_asio_windows(context, bound_port);' in server)
check('native-only mock server path', '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)\n    if (preloader_l::net::start_socket_runtime' in server)
check('native-only mock serve path', '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)\n    if (!context->listener.listening)' in server)
check('no native startup fallback in shipping', '// Native Asio bridge not present: never fall back to raw accept/worker.' in server)
check('native-only socket cleanup', '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)\n    if (context->client.socket' in server)
check('shipping empty legacy slots', 'context->listener.socket = preloader_l::net::invalid_socket_handle;' in server and
      'context->client.socket = preloader_l::net::invalid_socket_handle;' in server)
exchange=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8')
check('compat transport predicate preserved', 'ep.receive != nullptr && ep.send != nullptr && ep.close != nullptr' in exchange and
      exchange.count('valid_exchange_endpoint(ep)')==2 and
      'preloader_l::net::valid_endpoint(ep)' not in exchange)
check('transport source excluded only from shipping',
      bool(prod) and 'src/net/socket_transport.cpp' not in prod.group(1) and
      bool(debug) and 'src/net/socket_transport.cpp' in debug.group(1) and
      bool(native) and 'src/net/socket_transport.cpp' in native.group(1))
check('WSA explicit lifecycle retained', 'preloader_l::net::stop_socket_runtime(&context->runtime)' in server and
      'src/net/socket_runtime.cpp' in prod.group(1) and 'src/net/winsock_lifecycle_native.cpp' in prod.group(1) and 'src/net/winsock_native.cpp' not in prod.group(1))
# Negative construction deliberately alters a private textual copy, never live source.
def prod_safe(text):
    block=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',text,re.S)
    return bool(block) and 'src/net/socket_listener.cpp' not in block.group(1) and 'src/net/socket_endpoint.cpp' not in block.group(1) and 'src/net/socket_transport.cpp' not in block.group(1)
check('reject listener regression', not prod_safe(cm.replace('    src/net/socket_runtime.cpp',
       '    src/net/socket_listener.cpp\n    src/net/socket_runtime.cpp',1)))
check('reject endpoint regression', not prod_safe(cm.replace('    src/net/socket_runtime.cpp',
       '    src/net/socket_endpoint.cpp\n    src/net/socket_runtime.cpp',1)))
check('reject transport regression', not prod_safe(cm.replace('    src/net/socket_runtime.cpp',
       '    src/net/socket_transport.cpp\n    src/net/socket_runtime.cpp',1)))

# Physical production objects must not depend on retired mock listener/endpoint or generic transport helpers.
compilers=[shutil.which(c) for c in ('g++','clang++')]
if all(compilers):
    with tempfile.TemporaryDirectory(prefix='hf15_asio_stage03_') as tmp:
        for c in compilers:
            name=Path(c).name
            for mode in ('shipping','native'):
                path=Path(tmp)/(name+'_'+mode+'.o')
                cmd=[c,'-std=c++20','-D_WIN32','-DPRELOADER_L_TRACE_DIAGNOSTICS=0',
                    '-I'+str(r/'include'),'-I'+str(r/'src/runtime')]
                if mode=='native': cmd+=['-DPRELOADER_L_NATIVE_ACCEPTANCE=1']
                cmd+=['-c',str(r/'src/runtime/release_server.cpp'),'-o',str(path)]
                p=subprocess.run(cmd,capture_output=True,text=True)
                check(name+' '+mode+' source compile',p.returncode==0)
                if p.returncode != 0:
                    print(p.stderr[:1500]); continue
                symbols=subprocess.run(['nm','-u',str(path)],capture_output=True,text=True)
                targets=['close_socket_listener','create_loopback_listener','reset_socket_listener',
                         'close_socket_endpoint','accept_socket_endpoint','reset_socket_endpoint']
                has=all(t in symbols.stdout for t in targets)
                check(name+' '+mode+' expected symbol boundary',has if mode=='native' else
                      not any(t in symbols.stdout for t in targets))

        # Physical protocol object must not import the obsolete global transport
        # predicate. Native/debug still compile and exercise the original TU.
        for c in compilers:
            name=Path(c).name
            out=Path(tmp)/(name+'_exchange.o')
            cmd=[c,'-std=c++20','-I'+str(r/'include'),
                 '-c',str(r/'src/protocol/socket_exchange.cpp'),'-o',str(out)]
            p=subprocess.run(cmd,capture_output=True,text=True)
            check(name+' exchange object compile',p.returncode==0)
            if p.returncode!=0:
                print(p.stderr[:1500]); continue
            symbols=subprocess.run(['nm','-u',str(out)],capture_output=True,text=True)
            check(name+' exchange no global valid_endpoint',
                  'valid_endpoint' not in symbols.stdout)
else:
    print('STAGE04 SOURCE COMPILER GATE NOT_RUN: g++/clang++ missing')
    checks.append(('GCC/Clang physical TU verification',False))
fail=[n for n,v in checks if not v]
print('ASIO_LEGACY_STAGE04:', 'PASS' if not fail else 'FAIL',len(checks)-len(fail),'/',len(checks))
for x in fail: print(' -',x)
sys.exit(1 if fail else 0)
