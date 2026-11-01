#!/usr/bin/env python3
"""Stage02 portable source gate; does not claim Windows IOCP or async dispatcher acceptance."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
r=Path(__file__).resolve().parents[1]
checks=0
def must(condition,name):
    global checks
    if not condition: raise RuntimeError('Stage02: '+name)
    checks+=1

src=(r/'src/protocol/socket_exchange.cpp').read_text()
step=(r/'src/protocol/socket_exchange_frame_step.cpp').read_text()
h=(r/'src/protocol/socket_exchange_internal.hpp').read_text()
cmake=(r/'CMakeLists.txt').read_text()
native=(r/'tests/native/test_asio_ingress.cpp').read_text()
must(src.count('detail::classify_received_wire_frame(s, frame, &malformed_streak)')==1,'one sync step consumer')
must('if (!detail::commit_sent_handshake(s, frame, step.handshake, &malformed_streak))' in src,'post send handshake commit')
must(src.index('status = detail::send_all(ep, reinterpret_cast<const uint8_t*>(s->plain_response),\n                                      step.handshake.response_length + 1u')<src.index('if (!detail::commit_sent_handshake(s, frame, step.handshake, &malformed_streak))'),'send first')
must('process_challenge_request(' in step and 'decode_payload(' in step, 'original handshake and codec')
must('WireStepKind::xml_request' in step and 'WireStepKind::handshake_reply' in step,'typed step')
must('tolerate_malformed_frame' in step and '32u' in step,'streak cap retained')
must('consume_wire_frame' in step and 'bind_host_social_profile' in step,'consume and bind')
must('classify_received_wire_frame' in h and 'commit_sent_handshake' in h,'single internal API')
must(cmake.count('src/protocol/socket_exchange_frame_step.cpp')==2,'both MSVC CMake graphs')
native02=(r/'tests/native/test_asio_iocp_handshake.cpp').read_text()
runner02=(r/'scripts/test_asio_iocp_native.py').read_text()
must('#error HF12 Stage02 native requires real Windows IOCP' in native02,'native no-reactor gate')
must('asio::async_write' in native02 and '.release(' not in native02,'native async full reply and no release')
must('commit_sent_handshake' in native02 and 'classify_received_wire_frame' in native02,'source-linked native continuation')
must("('mt','/MT')" in runner02 and "('mtd','/MTd')" in runner02,'native CRT profiles')
must("'src/runtime/memory_allocator.cpp'" in runner02 and "'src/runtime/ptd_runtime.cpp'" in runner02,
     'link actual semantic transient allocator and PTD dependency')
must('VSCMD_ARG_TGT_ARCH' in runner02 and 'target_arch not in' in runner02,
     'reject Windows x86 Developer Command Prompt target')
must("'/D_WIN32_WINNT=0x0601'" in runner02,'explicit Windows API target for native Asio')
must('#if !defined(_WIN64)' in native02 and '#if !defined(_WIN64)' in native,
     'both native tests reject x86 builds')
must('release_transient_bytes(' in (r/'src/runtime/memory_allocator.cpp').read_text() and
     'allocate_transient_bytes(' in (r/'src/runtime/memory_allocator.cpp').read_text(),
     'real shipping transient allocation owner retained')
must('ASIO_DISABLE_IOCP' not in cmake,'production IOCP not disabled')
must('ASIO_DISABLE_IOCP' in native and '.release(' not in native,'Stage01 native IOCP proof retained')
must('src/protocol/socket_exchange_transaction.cpp' in cmake and 'execute_response_transaction(' in src,'response-event legacy transaction unchanged')
parent_hash='f3ace4a4ef3fdbe4c1235c53d07d495a18fabb96fd01d1528afc693a8f866c2d'
must(parent_hash in (r/'docs/ASIO_PRODUCTION.md').read_text(),'exact parent stated')
third=json.loads((r/'third_party/VENDOR_SHA256_MANIFEST.json').read_text()) if (r/'third_party/VENDOR_SHA256_MANIFEST.json').exists() else None
must((r/'third_party/asio/upstream/include/asio.hpp').exists() and (r/'third_party/tinyxml2/tinyxml2.cpp').exists(),'vendors retained')
base=['src/protocol/socket_exchange_frame_step.cpp','src/protocol/socket_exchange_wire.cpp','src/codec/message_codec.cpp','src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/xml/lsx_framing.cpp']
actual=['src/protocol/challenge_handshake.cpp','src/codec/session_crypto.cpp',
 'src/xml/request_parser.cpp','third_party/tinyxml2/tinyxml2.cpp','src/xml/request_children.cpp',
 'src/xml/response_builder.cpp','src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp',
 'src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp','src/xml/attribute_parser.cpp']
with tempfile.TemporaryDirectory(prefix='hf12_iocp_stage02_') as td:
    for compiler in ('g++','clang++'):
        if not shutil.which(compiler):continue
        common=[compiler,'-std=c++17','-O0','-Wall','-Wextra','-Wno-unused-function','-I'+str(r/'include'),'-I'+str(r/'src/protocol'),'-I'+str(r/'third_party/tinyxml2'),'-I'+str(r/'third_party/pegtl/upstream/include')]
        for test,extras,label in [
            ('tests/unit/test_asio_wire_step.cpp',[], 'SIMULATED_HANDSHAKE'),
            ('tests/unit/test_asio_wire_step_handshake.cpp',actual,'REAL_HANDSHAKE')]:
            exe=Path(td)/(compiler.replace('+','p')+'_'+label)
            cp=subprocess.run(common+[str(r/test)]+[str(r/x) for x in base+extras]+['-o',str(exe)],capture_output=True,text=True)
            if cp.returncode: raise RuntimeError(label+' '+compiler+' BUILD FAIL\n'+cp.stderr[-2400:])
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            if run.returncode: raise RuntimeError(label+' '+compiler+' RUN FAIL\n'+run.stdout+'\n'+run.stderr)
            print(label,compiler,run.stdout.strip())
            must('PASS' in run.stdout,label+' '+compiler)
        cp=subprocess.run(common+['-fsyntax-only',str(r/'src/protocol/socket_exchange.cpp')],capture_output=True,text=True)
        if cp.returncode: raise RuntimeError('shipping exchange '+compiler+' syntax FAIL\n'+cp.stderr[-1500:])
        must(True,'shipping sync exchange '+compiler+' syntax')
print('HF12_IOCP_STAGE02_PORTABLE PASS',checks,'checks')
