#!/usr/bin/env python3
"""Stage04D bounded real persistent Grant store + FIFO portable source gate.

This does NOT compile Windows Asio IOCP, real XML dispatcher, or shipping DLL.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
root=Path(__file__).resolve().parents[1]
assert "'store'" in (root/'scripts/test_asio_iocp_native.py').read_text()
assert 'src/runtime/service_state_grant.cpp' in (root/'scripts/test_asio_iocp_native.py').read_text()
assert 'src/config/config2_pegtl.cpp' in (root/'scripts/test_asio_iocp_native.py').read_text()
assert 'ASIO_DISABLE_IOCP' not in (root/'CMakeLists.txt').read_text()
# The adapter delegates to the genuine persistent store. The production
# handler must retain the same mutation and reservation behavior.
handler=(root/'src/handlers/achievement_grant_route.cpp').read_text()
for marker in ('commit_grant_achievement_locked(delivery->transaction)',
               'release_grant_achievement_transaction(delivery->transaction)',
               'release_grant_achievement_delivery()'):
    assert marker in handler,marker
native=(root/'tests/native/test_asio_iocp_grant_store.cpp').read_text()
for marker in ('#include <asio.hpp>', 'defined(ASIO_DISABLE_IOCP)',
               'progress()==1', 'destroy_prepared_response_transaction(tx)'):
    assert marker in native,marker
assert 'store_mt' not in native  # profiles are supplied by native driver

state=['src/runtime/service_state.cpp','src/runtime/service_state_storage.cpp',
       'src/runtime/service_state_parser.cpp','src/runtime/service_state_grant.cpp',
       'src/config/config2_pegtl.cpp']
owner=['tests/unit/test_asio_grant_store_fifo.cpp','src/runtime/event_queue.cpp',
       'src/protocol/socket_exchange_async_transaction.cpp',
       'src/protocol/socket_exchange_transaction_delivery.cpp',
       'src/protocol/socket_exchange_write_completion.cpp',
       'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp']
for compiler in ('g++','clang++'):
    if not shutil.which(compiler):raise RuntimeError('Missing required compiler: '+compiler)
    with tempfile.TemporaryDirectory(prefix='hf12_stage04d_') as tmp:
        tmp=Path(tmp)
        base=['-std=c++20','-O1','-pthread','-Wall','-Wextra',
              '-I'+str(root/'include'),'-I'+str(root/'src/protocol'),
              '-I'+str(root/'third_party/pegtl/upstream/include')]
        objects=[]
        for i,rel in enumerate(state+owner):
            target=tmp/('unit_'+str(i)+'.o')
            flags=[] if rel in state else ['-D_WIN32','-D__declspec(x)=','-D__stdcall=']
            args=[compiler,*base,*flags,'-c',str(root/rel),'-o',str(target)]
            built=subprocess.run(args,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,timeout=95)
            if built.returncode:raise RuntimeError(compiler+' '+rel+' compile: '+built.stderr[-2200:])
            objects.append(str(target))
        exe=tmp/'grant_store_fifo_test'
        link=subprocess.run([compiler,'-pthread',*objects,'-o',str(exe)],capture_output=True,text=True,timeout=45)
        if link.returncode:raise RuntimeError(compiler+' link: '+link.stderr[-2200:])
        run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=35)
        if run.returncode or 'HF12_STAGE04D_REAL_STORE_FIFO_OWNER PASS checks=' not in run.stdout:
            raise RuntimeError(compiler+' run: '+run.stdout+run.stderr)
        print(compiler,run.stdout.strip())
print('STAGE04D_BOUNDED_REAL_STORE_FIFO_PORTABLE PASS GCC/Clang; native IOCP NOT_RUN')
