#!/usr/bin/env python3
"""Source-linked genuine Config2 paired startup test, portable Windows-I/O shim.
Test-only: actual MSVC/native/Consumer execution remains a separate gate.
"""
from pathlib import Path
import os,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
f=r/'src/config/runtime_configuration_file.cpp'
p=r/'src/config/config2_pegtl.cpp'
t=r/'tests/unit/test_config_overlay_pair.cpp'
base=[f'-I{r / "include"}',f'-I{r / "src/config"}']
peg=[f'-I{r / "third_party/pegtl/upstream/include"}',f'-I{r / "src/config"}']
with tempfile.TemporaryDirectory(prefix='config_overlay_pair_') as tmp:
    d=Path(tmp)
    total=0
    for cc in ('g++','clang++'):
        steps=[([cc,'-std=c++20','-O1','-D_WIN32','-D__declspec(x)=','-D__stdcall=',*base,'-c',str(f),'-o',str(d/'file.o')]),
               ([cc,'-std=c++20','-O1',*peg,'-c',str(p),'-o',str(d/'parser.o')]),
               ([cc,'-std=c++20','-O1',*base,'-c',str(t),'-o',str(d/'test.o')]),
               ([cc,str(d/'file.o'),str(d/'parser.o'),str(d/'test.o'),'-o',str(d/'run')])]
        for cmd in steps:
            x=subprocess.run(cmd,cwd=r,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            if x.returncode:
                print(cc,'FAIL',x.stdout[-4000:]);sys.exit(1)
        x=subprocess.run([str(d/'run')],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        print(cc,x.stdout.strip())
        if x.returncode:sys.exit(x.returncode)
        total+=13
    print(f'CONFIG_OVERLAY_PAIR: {total}/{total} PASS (GCC+CLANG; SOURCE-LINKED; NOT_NATIVE)')
