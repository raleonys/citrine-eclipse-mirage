#!/usr/bin/env python3
"""Bounded HF11 migration candidate tests; does not certify Windows/game acceptance."""
from pathlib import Path
import hashlib
import shutil
import subprocess
import tempfile
import sys

r = Path(__file__).resolve().parents[1]
pin = {
    'third_party/tinyxml2/tinyxml2.cpp': '07b4e7a1d021256255b91a0844640f26508201b40cfc576b9499b068d981c39b',
    'third_party/tinyxml2/tinyxml2.h': 'dc6903d27f5f8ff18f7de98179d979ee3986d12273c495b54f48ac385fcdd7c5',
}
for rel, expected in pin.items():
    if hashlib.sha256((r/rel).read_bytes()).hexdigest() != expected:
        raise SystemExit('FAIL pinned TinyXML2 source: '+rel)
cmake = (r/'CMakeLists.txt').read_text()
if 'add_library(preloader_l_tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp)' not in cmake or \
   'target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)' not in cmake:
    raise SystemExit('FAIL missing static production linkage')
cases = {
    'request': ['tests/unit/test_progressive_installation_contract.cpp', 'src/xml/request_parser.cpp',
                'src/xml/request_children.cpp',
                'src/xml/lsx_framing.cpp','src/xml/attribute_parser.cpp',
                'src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp',
                'src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp'],
    'attribute': ['tests/unit/test_attribute.cpp', 'src/xml/request_parser.cpp',
                  'src/xml/request_children.cpp',
                  'src/xml/lsx_framing.cpp','src/xml/attribute_parser.cpp'],
    'store_boundary': ['tests/unit/test_store_document_boundary.cpp',
                       'src/config/store_runtime.cpp','src/config/store_parser.cpp',
                       'src/config/store_materialization.cpp',
                       'src/config/store_values.cpp',
                       'src/config/store_payload.cpp']
}
compilers = [name for name in ['g++','clang++'] if shutil.which(name)]
if not compilers:
    raise SystemExit('FAIL no supported portable C++ compiler')
with tempfile.TemporaryDirectory(prefix='hf11_tinyxml2_portable_') as td:
    for cxx in compilers:
        for name, sources in cases.items():
            exe=Path(td)/(cxx.replace('+','p')+'_'+name)
            cmd = [cxx,'-std=c++17','-O1','-Wall','-Wextra','-Werror',
                   '-I'+str(r/'include'), *[str(r/p) for p in sources],
                   str(r/'third_party/tinyxml2/tinyxml2.cpp'),'-o',str(exe)]
            subprocess.run(cmd,cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)
            print('PASS',cxx,name)
print('HF11_TINYXML2_BOUNDED_PORTABLE: PASS (%d/%d; NOT A RELEASE GATE)' % (len(compilers)*len(cases),len(compilers)*len(cases)))
