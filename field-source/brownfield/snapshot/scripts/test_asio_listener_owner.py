#!/usr/bin/env python3
"""Source-linked Asio listener owner regression (portable, not Windows IOCP)."""
from pathlib import Path
import subprocess, shutil, tempfile, sys
r=Path(__file__).resolve().parents[1]
source=r/'src/runtime/asio_listener_owner.cpp'
header=r/'src/runtime/asio_listener_owner.hpp'
test=r/'tests/unit/test_asio_listener_owner.cpp'
cm=(r/'CMakeLists.txt').read_text()
assert all(p.is_file() for p in (source,header,test))
assert cm.count('src/runtime/asio_listener_owner.cpp')==2
assert cm.count('src/runtime/asio_session_owner.cpp')==2
assert 'third_party/asio/upstream/include' in cm
assert 'ASIO_STANDALONE=1 ASIO_NO_TYPEID=1' in cm
body=source.read_text()
assert '#error HF13 Windows listener must use the real Asio IOCP backend' in body
for requirement in ('async_accept(', 'asio::bind_executor(strand_',
    'asio::post(strand_', 'acceptor_.cancel(', 'acceptor_.close(',
    'sessions_ >= kSessionCap', 'notify_quiesced()', 'session_finished()'):
    if requirement not in body and requirement not in header.read_text():
        raise AssertionError('missing acceptance contract '+requirement)
assert '.release(' not in body and 'closesocket(' not in body
for compiler in ('g++','clang++'):
    cc=shutil.which(compiler)
    if not cc:
        raise RuntimeError('required compiler missing: '+compiler)
    with tempfile.TemporaryDirectory(prefix='asio_owner_') as temporary:
        exe=Path(temporary)/'owner'
        command=[cc,'-std=c++20','-O0','-g0','-pthread',
          '-DASIO_STANDALONE=1','-DASIO_NO_TYPEID=1',
          '-I'+str(r/'third_party/asio/upstream/include'),
          '-I'+str(r/'src/runtime'),str(source),str(test),'-o',str(exe)]
        cp=subprocess.run(command,capture_output=True,text=True,timeout=150)
        if cp.returncode:
            raise RuntimeError(compiler+' compile: '+cp.stderr[-2000:])
        try:
            run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)
        except subprocess.TimeoutExpired:
            raise RuntimeError(compiler+' actual loopback TIMEOUT')
        if run.returncode or 'HF13_ASIO_LISTENER_OWNER_PORTABLE: PASS' not in run.stdout:
            raise RuntimeError(compiler+' runtime: '+run.stdout+run.stderr)
        print(f'{compiler}: SOURCE_LINKED_REAL_TCP_LOOPBACK PASS',flush=True)
print('ASIO_LISTENER_OWNER: PASS (portable GCC+Clang; Windows native NOT_RUN)')
