#!/usr/bin/env python3
"""Internal release-QA gate. Executes actual TinyXML2 request + owned child graph."""
from pathlib import Path
import shutil,subprocess,tempfile,sys
r=Path(__file__).resolve().parents[1]
scanner=r/'src/xml/request_scanner.cpp'
bridge=r/'src/xml/tinyxml2_compat_bridge.hpp'
if scanner.exists() or bridge.exists() or (r/'src/config/store_xml.cpp').exists():
    raise SystemExit('XML_SINGLE_ENGINE: FAIL retired parser/bridge reintroduced')
source=(r/'src/xml/request_children.cpp').read_text()
parser=(r/'src/xml/request_parser.cpp').read_text()
header=(r/'include/preloader_l/xml/request_parser.hpp').read_text()
if 'dom::child_index(command)' not in parser or 'dom::child_index(element)' not in source or 'valid_trivia' not in header:
    raise SystemExit('XML_SINGLE_ENGINE: FAIL cached owned direct-child validation missing')
parts=['src/xml/request_parser.cpp','src/xml/request_children.cpp','src/xml/lsx_framing.cpp',
       'src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp','src/xml/value_codec_normalize.cpp',
       'src/xml/value_codec_semantic.cpp','src/xml/attribute_parser.cpp','third_party/tinyxml2/tinyxml2.cpp']
cases=['tests/unit/test_xml_child_index_semantics.cpp','tests/unit/test_progressive_installation_contract.cpp',
       'tests/unit/test_xml_single_dom_contract.cpp']
compilers=[c for c in ('g++','clang++') if shutil.which(c)]
if not compilers: raise SystemExit('XML_SINGLE_ENGINE: FAIL no C++ compiler')
with tempfile.TemporaryDirectory(prefix='hf11_single_engine_') as td:
    total=0
    for cxx in compilers:
        for case in cases:
            exe=Path(td)/(cxx.replace('+','p')+'_'+Path(case).stem)
            argv=[cxx,'-std=c++17','-O1','-Wall','-Wextra','-Werror','-I'+str(r/'include'),
                  str(r/case),*[str(r/x) for x in parts],'-o',str(exe)]
            q=subprocess.run(argv,capture_output=True,text=True)
            if q.returncode: print(q.stderr[-6000:]); raise SystemExit('XML_SINGLE_ENGINE: FAIL compile '+cxx+' '+case)
            q=subprocess.run([str(exe)],capture_output=True,text=True)
            if q.returncode: print(q.stdout, q.stderr); raise SystemExit(f'XML_SINGLE_ENGINE: FAIL runtime {cxx} {case} rc={q.returncode}')
            total+=1; print('XML_SINGLE_ENGINE: PASS',cxx,case)
print(f'XML_SINGLE_ENGINE: PASS ({total}/{total}; independent owned DOM/negative fixtures)')
