#!/usr/bin/env python3
"""Bounded portable executable evidence only; NEVER substitutes release QA."""
from pathlib import Path
import hashlib
import shutil
import subprocess
import tempfile

r=Path(__file__).resolve().parents[1]
pins={
 'third_party/tinyxml2/tinyxml2.cpp':'07b4e7a1d021256255b91a0844640f26508201b40cfc576b9499b068d981c39b',
 'third_party/tinyxml2/tinyxml2.h':'dc6903d27f5f8ff18f7de98179d979ee3986d12273c495b54f48ac385fcdd7c5',
}
for path,expected in pins.items():
    if hashlib.sha256((r/path).read_bytes()).hexdigest()!=expected:
        raise SystemExit('PINNED_TINYXML2_MISMATCH: '+path)
retired=['src/xml/request_scanner.cpp','src/xml/request_internal.hpp',
         'src/xml/tinyxml2_compat_bridge.hpp','src/config/store_xml.cpp',
         'src/config/store_entitlements.cpp']
for rel in retired:
    if (r/rel).exists(): raise SystemExit('LEGACY_XML_RETURNED: '+rel)
xml=['src/xml/request_parser.cpp','src/xml/request_children.cpp','src/xml/lsx_framing.cpp',
     'src/xml/attribute_parser.cpp','src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp',
     'src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp']
store=['src/config/store_runtime.cpp','src/config/store_parser.cpp','src/config/store_materialization.cpp',
       'src/config/store_values.cpp','src/config/store_payload.cpp','src/config/settings_values.cpp']
tests={
 'semantic_dom':('tests/unit/test_xml_semantics.cpp',xml),
 'single_dom_contract':('tests/unit/test_xml_single_dom_contract.cpp',xml),
 'original_request':('tests/unit/test_request.cpp',xml),
 'owned_dom':('tests/unit/test_tinyxml2_dom_migration.cpp',xml),
 'attribute':('tests/unit/test_attribute.cpp',xml),
 'request_pi_review_variant':('tests/unit/test_progressive_installation_contract.cpp',xml),
 'store_payload':('tests/unit/test_store_payload.cpp',store),
 'store_boundary_revised_pi':('tests/unit/test_store_document_boundary.cpp',store),
 'store_auxiliary':('tests/unit/test_store_auxiliary.cpp',store),
 'store_feature_interleaving':('tests/unit/test_store_feature_interleaving.cpp',store),
 'store_transactional':('tests/unit/test_store_transactional.cpp',store),
 'store_decode_reallocation':('tests/unit/test_store_decode_reallocation.cpp',store),
 'store_snapshot':('tests/unit/test_store_snapshot.cpp',store),
}
compilers=[name for name in ('g++','clang++') if shutil.which(name)]
if len(compilers)!=2: raise SystemExit('REQUIRED_COMPILERS_MISSING')
with tempfile.TemporaryDirectory(prefix='hf11_candidate03_') as td:
    for cc in compilers:
        unique_sources=sorted({src for _,source_set in tests.values() for src in source_set}
                              | {'third_party/tinyxml2/tinyxml2.cpp'})
        objects={}
        # Compile dependency sources only once per compiler; repeated full TinyXML2
        # builds can hit the test harness timeout and are not additional evidence.
        for index,src in enumerate(unique_sources):
            obj=Path(td)/(cc.replace('+','p')+'_src'+str(index)+'.o')
            cmd=[cc,'-std=c++17','-O0','-Wall','-Wextra','-Werror','-I'+str(r/'include'),
                 '-c',str(r/src),'-o',str(obj)]
            subprocess.run(cmd,cwd=r,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            objects[src]=obj
        for name,(test,src) in tests.items():
            exe=Path(td)/(cc.replace('+','p')+'_'+name)
            obj_list=[objects[path] for path in src]+[objects['third_party/tinyxml2/tinyxml2.cpp']]
            cmd=[cc,'-std=c++17','-O0','-Wall','-Wextra','-Werror',
                 '-I'+str(r/'include'),str(r/test)] + [str(x) for x in obj_list]+['-o',str(exe)]
            subprocess.run(cmd,cwd=r,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            subprocess.run([str(exe)],cwd=r,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            print('PASS',cc,name,flush=True)
print('HF11_CANDIDATE04_BOUNDED_PORTABLE: PASS',len(tests)*len(compilers),'/',len(tests)*len(compilers))
print('PI: explicit TinyXML2 fail-closed regression; donor/Consumer native authority still unresolved')
print('NOT_RUN: Windows/MSVC/Native/CRT/CollegeFB27/Madden27/full 8-shard QA')
