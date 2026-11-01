#!/usr/bin/env python3
"""Focused shipping-source integration + deterministic policy acceptance.

Compilation/link of existing real TCP session tests is separate. Windows IOCP,
transport cancellation and real wall-clock stall tests are NOT claimed here.
"""
from pathlib import Path
import concurrent.futures
import shutil
import subprocess
import tempfile
root=Path(__file__).resolve().parents[1]
cpp=(root/'src/runtime/asio_session_owner.cpp').read_text()
hpp=(root/'src/runtime/asio_session_owner.hpp').read_text()
policy=root/'src/runtime/asio_session_timeout_policy.hpp'
assert 'timeout_policy_.begin_receive' in cpp
assert 'timeout_policy_.receive_progress' in cpp
assert 'timeout_policy_.begin_send' in cpp
assert 'timeout_policy_.send_progress' in cpp
assert cpp.count('asio::transfer_all()(ec,n)') == 3
assert 'deadline_callbacks_!=0u' in cpp and 'cancel_deadline();' in cpp
assert 'Status::receive_timeout' in cpp and 'Status::send_timeout' in cpp
assert 'ASIO_DISABLE_IOCP' in cpp and '#error HF13 shipping Asio session requires the Windows IOCP backend.' in cpp
assert 'asio::steady_timer deadline_timer_' in hpp
assert policy.is_file()
cm=(root/'CMakeLists.txt').read_text()
assert cm.count('src/runtime/asio_session_owner.cpp')==2
flags=['-std=c++20','-O0','-g0','-pthread','-DASIO_STANDALONE=1','-DASIO_NO_TYPEID=1',
       '-I'+str(root/'src/runtime'),'-I'+str(root/'include'),'-I'+str(root/'src'),
       '-I'+str(root/'third_party/asio/upstream/include')]
with tempfile.TemporaryDirectory(prefix='asio_timeout_policy_') as tmp:
    def test(compiler):
        cc=shutil.which(compiler)
        if not cc: raise RuntimeError('missing '+compiler)
        exe=str(Path(tmp)/compiler.replace('+','p'))
        src=root/'tests/unit/test_asio_session_timeout_policy.cpp'
        c=subprocess.run([cc,*flags,str(src),'-o',exe],capture_output=True,text=True,timeout=90)
        if c.returncode: raise RuntimeError(compiler+' compile: '+c.stderr[-3000:])
        run=subprocess.run([exe],capture_output=True,text=True,timeout=10)
        if run.returncode or 'ASIO_SESSION_TIMEOUT_POLICY: PASS' not in run.stdout:
            raise RuntimeError(compiler+' run: '+run.stdout+run.stderr)
        s=subprocess.run([cc,*flags,'-fsyntax-only',str(root/'src/runtime/asio_session_owner.cpp')],
                         capture_output=True,text=True,timeout=90)
        if s.returncode: raise RuntimeError(compiler+' session syntax: '+s.stderr[-3000:])
        return (compiler,run.stdout.strip())
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        for name,out in pool.map(test,['g++','clang++']):
            print(name,out,'REAL_SHIPPING_SESSION_SYNTAX: PASS',flush=True)
print('ASIO_SESSION_TIMEOUT_STAGE01: PASS (source + deterministic policy; Windows/native NOT_RUN)')
