#!/usr/bin/env python3
"""HF13 real TCP Asio session-owner smoke; protocol/dispatcher stubs (not native IOCP parity)."""
from pathlib import Path
import shutil, subprocess, tempfile, sys, concurrent.futures
r=Path(__file__).resolve().parents[1]
service_mode = '--service' in sys.argv
paths=[r/'src/runtime/asio_session_owner.cpp',r/'src/runtime/asio_listener_owner.cpp',
       r/'tests/unit/test_asio_session_owner.cpp',r/'src/protocol/socket_exchange_async_transaction.cpp']
if service_mode:
    paths.insert(2,r/'src/runtime/asio_service_owner.cpp')
    paths[3]=r/'tests/unit/test_asio_service_owner.cpp'
for p in paths:
    if not p.is_file(): raise RuntimeError('missing '+str(p))
cm=(r/'CMakeLists.txt').read_text(encoding='utf-8')
assert cm.count('src/runtime/asio_session_owner.cpp')==2
assert cm.count('src/runtime/asio_service_owner.cpp')==2
assert cm.count('src/runtime/asio_release_bridge.cpp')==2
assert 'AsioServiceOwner' not in (r/'src/runtime/release_server.cpp').read_text()
assert '#error HF13 shipping Asio session requires the Windows IOCP backend.' in paths[0].read_text()
flags=['-std=c++20','-O0','-g0','-pthread','-DASIO_STANDALONE=1','-DASIO_NO_TYPEID=1',
       '-I'+str(r/'include'),'-I'+str(r/'src'),'-I'+str(r/'src/runtime'),
       '-I'+str(r/'third_party/asio/upstream/include')]
compilers = [sys.argv[sys.argv.index('--compiler')+1]] if '--compiler' in sys.argv else ('clang++','g++')
for compiler in compilers:
    cc=shutil.which(compiler)
    if not cc: raise RuntimeError('compiler unavailable: '+compiler)
    with tempfile.TemporaryDirectory(prefix='hf13_session_') as tmp:
        obj=[]
        def compile_one(pair):
            i,src=pair
            o=str(Path(tmp)/f's{i}.o')
            cp=subprocess.run([cc,*flags,'-c',str(src),'-o',o],capture_output=True,text=True,timeout=180)
            if cp.returncode:
                raise RuntimeError(f'{compiler} {src.name} compile failed: '+cp.stderr[-1600:])
            return o
        if service_mode:
            # A deterministic input/output order while independent heavy Asio
            # compilation runs concurrently; avoid cumulative runner timeouts.
            with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
                obj=list(pool.map(compile_one, enumerate(paths)))
        else:
            obj=[compile_one(pair) for pair in enumerate(paths)]
        exe=str(Path(tmp)/'session')
        cp=subprocess.run([cc,'-pthread',*obj,'-o',exe],capture_output=True,text=True,timeout=45)
        if cp.returncode: raise RuntimeError(f'{compiler} link failed: '+cp.stderr[-1600:])
        cp=subprocess.run([exe],capture_output=True,text=True,timeout=12)
        if cp.returncode or ('HF13_ASIO_SERVICE_OWNER_REAL_TCP: PASS' if service_mode else 'HF13_ASIO_SESSION_OWNER_TRANSPORT: PASS') not in cp.stdout:
            raise RuntimeError(f'{compiler} socket lifecycle failure: '+cp.stdout+cp.stderr)
        print(f'{compiler} REAL_TCP_TRANSPORT: PASS (stub protocol; service={service_mode})',flush=True)
print('HF13_ASIO_SERVICE_OWNER' if service_mode else 'HF13_ASIO_SESSION_OWNER',': PASS (portable transport only; Windows IOCP/Production NOT_RUN)')
