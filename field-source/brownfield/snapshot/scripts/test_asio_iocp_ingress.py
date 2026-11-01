#!/usr/bin/env python3
"""HF12 Stage01 narrow portable source-linked gate (not Windows IOCP execution)."""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
WIRE = ROOT / 'src/protocol/socket_exchange_wire.cpp'
HEADER = ROOT / 'src/protocol/socket_exchange_internal.hpp'
NATIVE = ROOT / 'tests/native/test_asio_ingress.cpp'
SCRIPT = ROOT / 'scripts/test_asio_iocp_native.py'
checks = 0

def check(v, name):
    global checks
    if not v:
        raise RuntimeError(name)
    checks += 1

s = WIRE.read_text(encoding='utf-8')
h = HEADER.read_text(encoding='utf-8')
n = NATIVE.read_text(encoding='utf-8')
bat = SCRIPT.read_text(encoding='utf-8')
check('bool scan_available_wire_frame(ExchangeState* s, WireFrameView* view) noexcept' in s, 'scanner definition')
check('if (scan_available_wire_frame(s, view)) return ExchangeStatus::ok;' in s, 'sync parity authority')
check('bool scan_available_wire_frame(ExchangeState* state, WireFrameView* view) noexcept;' in h, 'internal api')
check('w::scan_available_wire_frame(&self->state, &view)' in n, 'native source-linked framing')
check('socket.async_read_some' in n and 'listener.async_accept' in n, 'real async operations')
check('socket.cancel' in n and 'socket.close' in n, 'Asio-owned cancellation close')
check('.release(' not in n and 'ASIO_DISABLE_IOCP)' in n, 'no release and reactor fail-closed')
check("('/mt','/MT')" not in bat and "('mtd','/MTd')" in bat and "('mt','/MT')" in bat, 'both static CRT profiles')
check('Mswsock.lib' in bat and 'Ws2_32.lib' in bat, 'native Winsock extension linkage')
check('ASIO_DISABLE_IOCP' not in (ROOT/'CMakeLists.txt').read_text(), 'shipping macro forbidden')
for compiler in ('g++','clang++'):
    cc = shutil.which(compiler)
    if cc is None:
        print(f'{compiler} unavailable: cannot claim portable compile PASS', file=sys.stderr)
        sys.exit(3)
    with tempfile.TemporaryDirectory(prefix='hf12_iocp_') as td:
        exe=Path(td)/'scanner'
        command=[cc,'-std=c++20','-O1',f'-I{ROOT / "include"}',f'-I{ROOT / "src/protocol"}',
                 str(ROOT/'tests/unit/test_async_wire_scanner.cpp'),str(WIRE),
                 str(ROOT/'src/xml/lsx_framing.cpp'),str(ROOT/'src/codec/message_codec.cpp'),
                 str(ROOT/'src/codec/aes128_cipher.cpp'),str(ROOT/'src/codec/hex_codec.cpp'),
                 '-o',str(exe)]
        cp=subprocess.run(command,text=True,capture_output=True,timeout=80)
        check(cp.returncode==0,compiler+' compile: '+cp.stderr[:1000])
        rp=subprocess.run([str(exe)],text=True,capture_output=True,timeout=15)
        check(rp.returncode==0 and 'PASS checks=26' in rp.stdout,compiler+' execute: '+rp.stderr[:1000])
print(f'HF12_IOCP_STAGE01_FOCUSED: PASS {checks}/{checks} (real Windows native remains NOT_RUN)')
