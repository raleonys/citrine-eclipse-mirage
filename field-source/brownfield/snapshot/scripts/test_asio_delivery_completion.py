#!/usr/bin/env python3
"""Source-linked Stage03 response/event completion gate. Native IOCP is separate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
r=Path(__file__).resolve().parents[1]
checks=0
def must(b,name):
    global checks
    if not b: raise RuntimeError('HF12 Stage03: '+name)
    checks+=1
s=(r/'src/protocol/socket_exchange_transaction.cpp').read_text()
d=(r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text()
o=(r/'src/protocol/socket_exchange_transaction_owner_private.hpp').read_text()
h=(r/'src/protocol/socket_exchange_write_completion.hpp').read_text()
c=(r/'src/protocol/socket_exchange_write_completion.cpp').read_text()
cm=(r/'CMakeLists.txt').read_text()
runner=(r/'scripts/test_asio_iocp_native.py').read_text()
native=(r/'tests/native/test_asio_iocp_delivery_completion.cpp').read_text()
must('DeliveryWriteCursor cursor{};' in o,'shipping prepared owner retains cursor')
must('complete_delivery_write(' in d and 'begin_delivery_writes(' in s,
     'shipping completion decision still source-owned')
must(d.count('complete_delivery_write(')==1 and
     'complete_prepared_transaction_write(' in s and 'complete_prepared_transaction_write(' in d,
     'single completion boundary shared by sync and future IOCP')
must('const auto send_status=send_all(ep,wire.bytes,wire.length,&sent)' in s and
     'if (action==PreparedWriteDisposition::send_event) {wire=next;continue;}' in s,
     'sync path dispatches Response then optional Event through owner')
must(d.index('const auto decision=complete_delivery_write(') <
     d.index('transaction->grant_delivery.commit_response();') <
     d.index('transaction->grant_delivery.commit_event();') <
     d.index('transaction->show_igo_delivery.commit();'),
     'commit authority follows full Response/Event completion')
must('transaction->grant_delivery.retry_event();' in d and
     'transaction->show_igo_delivery.retry();' in d and
     'transaction->generic_event_delivery.retry();' in d,
     'event FIFO preserved on failed/cancelled write')
must(d.index('transaction->grant_delivery.commit_event();') <
     d.index('transaction->social_delivery.commit()'),
     'social commit remains at full transaction boundary')
must('successful, size_t transferred' in h,'async-compatible completion contract')
must('!successful || !expected || transferred != expected' in c,'reject error, partial, zero')
must('state->phase != DeliveryWritePhase::response && state->phase != DeliveryWritePhase::event' in c,
     'reject late or duplicate completion')
must('ASIO_DISABLE_IOCP' not in cm,'production IOCP not disabled')
must('src/protocol/socket_exchange_write_completion.cpp' in cm and
     cm.count('src/protocol/socket_exchange_write_completion.cpp')==2,'production and native shipping graph')
must('#error HF12 Stage03 native requires Windows Asio IOCP' in native and
     '#if !defined(_WIN64)' in native,'real Windows x64 and no-reactor')
must('asio::async_write' in native and '.release(' not in native,'Asio IOCP write only')
must("'transactions'" in runner and 'src/protocol/socket_exchange_write_completion.cpp' in runner,
     'native runner links actual shipping completion source')
must('No async dispatcher' not in (r/'docs/ASIO_PRODUCTION.md').read_text(),
     'stage03 documentation distinguish coverage')
with tempfile.TemporaryDirectory(prefix='hf12_delivery_') as td:
    for compiler in ('g++','clang++'):
        if not shutil.which(compiler):
            raise RuntimeError('required portable compiler unavailable: '+compiler)
        exe=Path(td)/(compiler.replace('+','p')+'_completion')
        cmd=[compiler,'-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(r/'src/protocol'),
            str(r/'tests/unit/test_asio_delivery_completion.cpp'),
            str(r/'src/protocol/socket_exchange_write_completion.cpp'),'-o',str(exe)]
        cp=subprocess.run(cmd,capture_output=True,text=True)
        if cp.returncode: raise RuntimeError(compiler+' BUILD FAIL\n'+cp.stderr[-3000:])
        cp=subprocess.run([str(exe)],capture_output=True,text=True)
        if cp.returncode or 'PASS assertions=43335' not in cp.stdout:
            raise RuntimeError(compiler+' RUN FAIL\n'+cp.stdout+cp.stderr)
        print(cp.stdout.strip(),compiler)
        must(True,compiler+' source-linked completion model')
        cp=subprocess.run([compiler,'-std=c++17','-fsyntax-only',
                           '-I'+str(r/'include'),'-I'+str(r/'src/protocol'),
                           str(r/'src/protocol/socket_exchange_transaction.cpp'),
                           str(r/'src/protocol/socket_exchange_transaction_delivery.cpp')],capture_output=True,text=True)
        if cp.returncode: raise RuntimeError(compiler+' shipping transaction syntax\n'+cp.stderr[-3000:])
        must(True,compiler+' shipping transaction syntax')
print('HF12_ASIO_DELIVERY_COMPLETION PASS',checks,'checks')
