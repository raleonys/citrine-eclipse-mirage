#!/usr/bin/env python3
"""Internal authoritative QA: execute current DOM grammar/depth negative and positive corpus."""
from pathlib import Path
import shutil,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
old=(r/'src/xml/request_scanner.cpp')
if old.exists() or (r/'src/xml/tinyxml2_compat_bridge.hpp').exists():
    raise SystemExit('XML_QA: FAIL retired scanner/PI bridge present')
headers=(r/'third_party/tinyxml2/tinyxml2.h').read_text()
if 'TINYXML2_MAX_ELEMENT_DEPTH = 500' not in headers:
    raise SystemExit('XML_QA: FAIL vendor depth contract drift')
request=(r/'src/xml/request_parser.cpp').read_text()
if 'dom::parse_document(' not in request or 'dom::child_index(command)' not in request:
    raise SystemExit('XML_QA: FAIL production TinyXML2 binding missing')
compilers=[c for c in ('g++','clang++') if shutil.which(c)]
if not compilers: raise SystemExit('XML_QA: FAIL no C++ compiler')
sources=['src/xml/request_parser.cpp', 'src/xml/request_children.cpp', 'src/xml/lsx_framing.cpp', 'src/xml/attribute_parser.cpp', 'src/xml/value_codec.cpp', 'src/xml/value_codec_utf8.cpp', 'src/xml/value_codec_normalize.cpp', 'src/xml/value_codec_semantic.cpp', 'third_party/tinyxml2/tinyxml2.cpp']
with tempfile.TemporaryDirectory(prefix='hf11_xml_qa_') as td:
    for cxx in compilers:
        exe=Path(td)/(cxx.replace('+','p')+'_request')
        argv=[cxx,'-std=c++17','-O1','-Wall','-Wextra','-Werror','-I'+str(r/'include'),
              str(r/'tests/unit/test_progressive_installation_contract.cpp'),*[str(r/q) for q in sources],'-o',str(exe)]
        p=subprocess.run(argv,capture_output=True,text=True)
        if p.returncode: print(p.stderr[-7000:]); raise SystemExit('XML_QA: FAIL compile '+cxx)
        p=subprocess.run([str(exe)],capture_output=True,text=True)
        if p.returncode: raise SystemExit('XML_QA: FAIL fixture '+cxx+' rc='+str(p.returncode))
print('F1425_ATTRIBUTE_GRAMMAR_DOM: PASS ('+str(len(compilers))+'/'+str(len(compilers))+' executable; depth/grammar/PI fixture)')
