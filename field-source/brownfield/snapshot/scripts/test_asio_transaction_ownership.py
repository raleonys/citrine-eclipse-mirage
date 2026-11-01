#!/usr/bin/env python3
"""Stage04A production-owned transaction lifetime and callback regression.

Tests actual production completion/owner code with narrow external side-effect
recorders; this is NOT native store+dispatcher+IOCP acceptance.
"""
from pathlib import Path
import subprocess,shutil,tempfile
r=Path(__file__).resolve().parents[1]
src=(r/'src/protocol/socket_exchange_transaction.cpp').read_text()
header=(r/'src/protocol/socket_exchange_internal.hpp').read_text()
cm=(r/'CMakeLists.txt').read_text()
assert 'struct PreparedResponseTransaction {' in (r/'src/protocol/socket_exchange_transaction_owner_private.hpp').read_text()
for marker in ('allocate_transient_bytes(\n        sizeof(PreparedResponseTransaction),ops)',
               'release_transient_bytes(transaction)',
               'transaction->grant_delivery.commit_response();',
               'transaction->generic_event_delivery.retry();',
               'transaction->social_delivery.commit()',
               'const auto send_status=send_all(ep,wire.bytes,wire.length,&sent)',
               'if (action==PreparedWriteDisposition::send_event) {wire=next;continue;}'):
    assert marker in src+(r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text(), marker
assert 'PreparedResponseTransaction* prepare_response_transaction(' in header
assert 'PreparedWriteDisposition complete_prepared_transaction_write(' in header
assert 'ASIO_DISABLE_IOCP' not in cm
assert 'src/protocol/socket_exchange_transaction.cpp' in cm
assert '.release(' not in src
assert 'socket_exchange_transaction.cpp' not in (r/'tests/native/test_asio_iocp_transaction_driver.cpp').read_text()
assert '#include "socket_exchange_transaction_owner_private.hpp"' in (r/'tests/native/test_asio_iocp_transaction_driver.cpp').read_text()
assert cm.count('src/protocol/socket_exchange_transaction_delivery.cpp')==2
assert "'src/protocol/socket_exchange_transaction_delivery.cpp'" in (r/'scripts/test_asio_iocp_native.py').read_text()
assert 'parse_request_view(' not in (r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text()
assert 'dispatch_plaintext_request' not in (r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text()
with tempfile.TemporaryDirectory(prefix='hf12_owner04_') as td:
    for cc in ('g++','clang++'):
        assert shutil.which(cc),cc+' required'
        exe=Path(td)/('owner_'+cc.replace('+','p'))
        cmd=[cc,'-std=c++17','-O2','-Wall','-ffunction-sections','-fdata-sections',
             '-fno-rtti','-fno-exceptions','-Wl,--gc-sections','-I'+str(r/'include'),
             '-I'+str(r/'src/protocol'),str(r/'tests/unit/test_asio_prepared_transaction_completion.cpp'),
             str(r/'src/protocol/socket_exchange_transaction_delivery.cpp'),
             str(r/'src/protocol/socket_exchange_write_completion.cpp'),'-o',str(exe)]
        p=subprocess.run(cmd,capture_output=True,text=True)
        if p.returncode: raise RuntimeError(cc+' BUILD: '+p.stderr[-3500:])
        p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)
        if p.returncode or 'HF12_STAGE04_PREPARED_TRANSACTION_MODEL PASS assertions=65' not in p.stdout:
            raise RuntimeError(cc+' RUNTIME: '+p.stdout+p.stderr)
        print(cc,p.stdout.strip())
        p=subprocess.run([cc,'-std=c++17','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-rtti','-fno-exceptions',
                          '-fsyntax-only','-I'+str(r/'include'),'-I'+str(r/'src/protocol'),
                          str(r/'src/protocol/socket_exchange_transaction.cpp')],capture_output=True,text=True)
        if p.returncode: raise RuntimeError(cc+' SHIPPING SYNTAX: '+p.stderr[-3500:])
# Stage04B uses the exact production PreparedResponseTransaction in a real
# callback scheduler, and bounded external store/event *recorders*. This is
# not the genuine Windows IOCP/native dispatcher acceptance.
with tempfile.TemporaryDirectory(prefix='hf12_owner04b_') as td:
    for cc in ('g++','clang++'):
        exe=Path(td)/('async_'+cc.replace('+','p'))
        cmd=[cc,'-std=c++17','-O2','-Wall','-Wno-unused-function',
             '-ffunction-sections','-fdata-sections','-fno-rtti','-fno-exceptions',
             '-Wl,--gc-sections','-I'+str(r/'include'),'-I'+str(r/'src/protocol'),
             str(r/'tests/unit/test_asio_async_transaction_driver.cpp'),
             str(r/'src/protocol/socket_exchange_async_transaction.cpp'),
             str(r/'src/protocol/socket_exchange_transaction_delivery.cpp'),
             str(r/'src/protocol/socket_exchange_write_completion.cpp'),
             '-o',str(exe)]
        cp=subprocess.run(cmd,capture_output=True,text=True,timeout=35)
        if cp.returncode:raise RuntimeError(cc+' async driver BUILD: '+cp.stderr[-3500:])
        cp=subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)
        if cp.returncode or 'PASS assertions=103' not in cp.stdout:
            raise RuntimeError(cc+' async driver RUN: '+cp.stdout+cp.stderr)
        print(cc,cp.stdout.strip())
print('HF12_ASIO_ASYNC_TRANSACTION_DRIVER_STAGE04B PASS 2 compilers 103 assertions each; external effects mocked')
print('HF12_ASIO_TRANSACTION_OWNER_STAGE04A PASS 2 compilers 65 assertions each; real dispatcher/Windows pending')

# Stage04C: use the ACTUAL shipping FIFO implementation (not the historical
# queued-event test recorders), actual transaction owner and async callback
# driver. Grant store remains explicitly a narrow recorder in this fixture.
with tempfile.TemporaryDirectory(prefix='hf12_real_fifo04c_') as td:
    for cc in ('g++','clang++'):
        exe=Path(td)/('fifo_'+cc.replace('+','p'))
        cmd=[cc,'-std=c++20','-O1','-Wall','-Wextra','-pthread',
             '-D_WIN32','-D__declspec(x)=','-D__stdcall=',
             '-I'+str(r/'include'),'-I'+str(r/'src/protocol'),
             *[str(r/s) for s in (
                 'tests/unit/test_asio_real_fifo_owner.cpp',
                 'src/runtime/event_queue.cpp',
                 'src/protocol/socket_exchange_async_transaction.cpp',
                 'src/protocol/socket_exchange_transaction_delivery.cpp',
                 'src/protocol/socket_exchange_write_completion.cpp',
                 'src/runtime/memory_allocator.cpp',
                 'src/runtime/ptd_runtime.cpp')],'-o',str(exe)]
        cp=subprocess.run(cmd,capture_output=True,text=True,timeout=90)
        if cp.returncode:raise RuntimeError(cc+' real FIFO BUILD: '+cp.stderr[-3000:])
        cp=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
        if cp.returncode or 'HF12_STAGE04C_REAL_FIFO_OWNER PASS checks=1440' not in cp.stdout:
            raise RuntimeError(cc+' real FIFO RUN: '+cp.stdout+cp.stderr)
        print(cc,cp.stdout.strip())
print('HF12_ASIO_STAGE04C_REAL_FIFO_SOURCE PASS GCC+Clang; Grant store still recorded')
# Stage04C native FIX01: ensure the length actually scheduled on the wire
# matches the length expected by asio::async_read. The Windows /MT fixture
# previously sent seven bytes of the eight-byte literal "RESPONSE", so the
# very first round could never finish its 8-byte async_read.
for rel in ('tests/native/test_asio_iocp_real_fifo.cpp',
            'tests/unit/test_asio_real_fifo_owner.cpp'):
    fixture=(r/rel).read_text()
    for requirement in (
        'constexpr char kResponse[]="RESPONSE";',
        'constexpr size_t kResponseN=sizeof(kResponse)-1;',
        'static_assert(kResponseN==8,',
        'std::memcpy(tx->wire.data,kResponse,kResponseN);',
        'tx->response_required=kResponseN;',
        'begin_delivery_writes(&tx->cursor,kResponseN,',
    ):
        assert requirement in fixture,(rel,requirement)
    assert 'std::memcpy(tx->wire.data,"RESPONSE",7)' not in fixture,rel
native=(r/'tests/native/test_asio_iocp_real_fifo.cpp').read_text()
assert 'std::string(kResponse,kResponseN)+' in native
assert 'FIFO_WIRE_DIAGNOSTIC accepted=' in native
print('HF12_STAGE04C_FIFO_FIX01_WIRE_LENGTH_CONTRACT PASS both fixtures')

