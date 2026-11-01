#!/usr/bin/env python3
"""Compile the native-only F02 timer path using pinned Asio headers on two C++20 compilers.

Only for this non-shipping portable syntax probe, we replace the _WIN32 part
of the native-acceptance conditional in temporary source/header copies. Thus,
no fake Win32 SDK declarations are introduced and original sources remain
unchanged. Windows/MSVC Native/CRT acceptance is still separately required.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import sys
import concurrent.futures

root = Path(__file__).resolve().parents[1]
cpp = (root/'src/runtime/asio_session_owner.cpp').read_text()
hpp = (root/'src/runtime/asio_session_owner.hpp').read_text()
acceptance = (root/'src/runtime/asio_acceptance_delivery.hpp').read_text()
resource = (root/'src/runtime/asio_acceptance_resource_fault.hpp').read_text()
assert 'f02_stall_timer_->cancel(ec)' not in cpp
assert '(void)f02_stall_timer_->cancel();' in cpp
assert 'catch (...) { terminal_status_=Status::resource_error; }' in cpp
assert 'socket_.cancel(ec);' in cpp and 'socket_.close(ec);' in cpp
assert cpp.count('#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)') >= 3
assert hpp.count('#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)') >= 2

guard = '#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)'
portable_guard = '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)'

with tempfile.TemporaryDirectory(prefix='hf13_f02_timer_syntax_') as td:
    t = Path(td)
    (t/'src/runtime').mkdir(parents=True)
    (t/'src/protocol').symlink_to(root/'src/protocol', target_is_directory=True)
    for basename, src in [('asio_session_owner.cpp',cpp),('asio_session_owner.hpp',hpp),
                          ('asio_acceptance_delivery.hpp',acceptance),
                          ('asio_acceptance_resource_fault.hpp',resource),
                          ('asio_session_timeout_policy.hpp', (root/'src/runtime/asio_session_timeout_policy.hpp').read_text())]:
        if basename != 'asio_session_timeout_policy.hpp': assert guard in src
        (t/'src/runtime'/basename).write_text(src.replace(guard,portable_guard))
    flags = ['-std=c++20','-fsyntax-only','-pthread','-DASIO_STANDALONE=1',
             '-DASIO_NO_TYPEID=1','-DPRELOADER_L_NATIVE_ACCEPTANCE=1',
             '-I'+str(root/'include'),'-I'+str(root/'src'),
             '-I'+str(root/'third_party/asio/upstream/include')]
    def run(compiler):
        exe=shutil.which(compiler)
        if exe is None:raise RuntimeError('missing '+compiler)
        result=subprocess.run([exe,*flags,str(t/'src/runtime/asio_session_owner.cpp')],
                              capture_output=True,text=True,timeout=85)
        if result.returncode:
            raise RuntimeError(compiler+' F02 native branch syntax failed:\n'+result.stderr[-4000:])
        return compiler
    # Negative replay: prove the native F02 branch is actually compiled, with
    # a deliberately invalid arity. The vendor may support cancel(ec) on some
    # platforms; that historical overload assumption is not a valid gate.
    old_source=(t/'src/runtime/asio_session_owner.cpp').read_text()
    (t/'src/runtime/asio_session_owner.cpp').write_text(old_source.replace(
        'try { (void)f02_stall_timer_->cancel(); }',
        'try { (void)f02_stall_timer_->cancel(ec,ec); }'))
    rejected=subprocess.run([shutil.which('clang++'),*flags,
        str(t/'src/runtime/asio_session_owner.cpp')],
        capture_output=True,text=True,timeout=85)
    if rejected.returncode == 0 or 'cancel' not in rejected.stderr:
        raise RuntimeError('invalid cancel(error_code,error_code) unexpectedly accepted; test invalid')
    print('Asio 1.38.2 invalid two-argument timer cancel rejected: PASS',flush=True)
    (t/'src/runtime/asio_session_owner.cpp').write_text(old_source)
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        for name in pool.map(run,['clang++','g++']):
            print(f'{name} F02_NATIVE_TIMER_SYNTAX: PASS',flush=True)
print('HF13_F02_TIMER_API: PASS (portable branch compiler replay; Windows/MSVC NOT_RUN)')
