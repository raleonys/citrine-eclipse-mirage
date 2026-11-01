#!/usr/bin/env python3
from pathlib import Path
import subprocess, tempfile, os, sys
r=Path(__file__).resolve().parents[1]
errors=[]
session=(r/'src/codec/session_crypto.cpp').read_text(encoding='utf-8',errors='replace')
children=(r/'src/xml/request_children.cpp').read_text(encoding='utf-8',errors='replace')
test_session=(r/'tests/unit/test_codec_session.cpp').read_text(encoding='utf-8',errors='replace')
test_request=(r/'tests/unit/test_request.cpp').read_text(encoding='utf-8',errors='replace')
route_test=(r/'tests/unit/test_runtime_configuration_routes.cpp').read_text(encoding='utf-8',errors='replace')
for token in ['const uint32_t limit = negative ? UINT32_C(0x80000000) : UINT32_C(0x7fffffff);',
              'if (overflow) return negative ? (-INT32_C(2147483647) - 1) : INT32_C(2147483647);',
              'if (value == UINT32_C(0x80000000)) return -INT32_C(2147483647) - 1;']:
    if token not in session: errors.append('F09 source marker missing: '+token)
for forbidden in ['return negative ? -signed_value : signed_value;']:
    if forbidden in session: errors.append('F09 UB form reintroduced: '+forbidden)
for token in ['"2147483648"','"-2147483648"','"-2147483649"','"4294967296"']:
    if token not in test_session: errors.append('F09 boundary regression missing: '+token)
for token in ['dom::child_index(element)', 'valid_trivia', 'ToText()', 'FirstChildElement()']:
    if token not in children and token!='valid_trivia': errors.append('F10 TinyXML2 DOM child contract marker missing: '+token)
if 'valid_trivia' not in (r/'include/preloader_l/xml/request_parser.hpp').read_text(): errors.append('F10 cached trivia validation missing')
for token in ['child_comment','child_cdata','child_pi','nested_child_text','direct_sequence']:
    if token not in test_request: errors.append('F10 request regression missing: '+token)
if 'q5c=' not in route_test: errors.append('F10 QueryImage handler regression missing')
if errors:
    print('PREAUDIT_WAVE5_SEMANTIC_EDGE: FAIL',len(errors))
    for e in errors: print(' -',e)
    sys.exit(1)

cxx=os.environ.get('CXX','c++')
common=[cxx,'-std=c++17','-Wall','-Wextra','-Werror','-Wno-unused-function','-I',str(r/'include')]
with tempfile.TemporaryDirectory(prefix='melotrix_wave5_') as td:
    td=Path(td)
    # F09 executable regression under UBSan: any signed-overflow recurrence must fail.
    exe=td/'f09_ubsan'
    cmd=common+['-fsanitize=undefined','-fno-sanitize-recover=undefined',
        str(r/'tests/unit/test_codec_session.cpp'),str(r/'src/codec/session_crypto.cpp'),
        str(r/'src/codec/message_codec.cpp'),str(r/'src/codec/aes128_cipher.cpp'),str(r/'src/codec/hex_codec.cpp'),'-o',str(exe)]
    subprocess.run(cmd,cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)

    # F10 direct parser/helper executable regression.
    exe=td/'f10_request'
    request_sources=[r/'src/xml/request_parser.cpp',r/'third_party/tinyxml2/tinyxml2.cpp',r/'src/xml/request_children.cpp']
    value_sources=[r/'src/xml/value_codec.cpp',r/'src/xml/value_codec_utf8.cpp',r/'src/xml/value_codec_normalize.cpp',r/'src/xml/value_codec_semantic.cpp']
    subprocess.run(common+[str(r/'tests/unit/test_progressive_installation_contract.cpp'),*[str(x) for x in request_sources],
        str(r/'src/xml/lsx_framing.cpp'),*[str(x) for x in value_sources],str(r/'src/xml/attribute_parser.cpp'),'-o',str(exe)],cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)
print('PREAUDIT_WAVE5_SEMANTIC_EDGE: PASS 18/18')
