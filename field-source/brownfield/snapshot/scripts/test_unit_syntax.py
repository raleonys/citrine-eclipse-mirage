#!/usr/bin/env python3
from pathlib import Path
import subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
files=sorted((r/'tests/unit').glob('*.cpp'))
failed=[]
for p in files:
    q=subprocess.run(['c++','-std=c++17','-fsyntax-only','-I'+str(r/'include'),'-I'+str(r/'tests'/'support'),'-I'+str(r/'tests'/'support'/'optional_hook_api_shim'),'-I'+str(r/'src'/'debug'),'-I'+str(r/'src'/'protocol'),'-I'+str(r/'src'/'runtime'),'-I'+str(r/'src'/'config'),'-I'+str(r/'third_party'/'asio'/'upstream'/'include'),'-I'+str(r/'third_party'/'filesystem'/'upstream'/'include'),'-I'+str(r/'third_party'/'pegtl'/'upstream'/'include'),str(p)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    if q.returncode: failed.append((p.name,q.stderr[-1500:]))
if failed:
    print('UNIT_CPP_SYNTAX: FAIL')
    for n,e in failed[:20]: print('---',n,'---\n',e)
    sys.exit(1)
print(f'UNIT_CPP_SYNTAX: PASS ({len(files)}/{len(files)})')

# HOTFIX07: this model used to be syntax-visible through a public header while
# its implementation was never linked. Keep it as explicit research/test
# support and prove the implementation by compiling, linking and executing it.
surge_test=r/'tests/unit/test_surge_vm.cpp'
surge_model=r/'tests/support/research/surge_vm.cpp'
with tempfile.TemporaryDirectory(prefix='preloader_l_surge_vm_') as td:
    exe=Path(td)/'test_surge_vm_model'
    q=subprocess.run([
        'c++','-std=c++17','-O0','-Wall','-Wextra','-pedantic',
        '-I'+str(r/'tests'/'support'),'-I'+str(r/'tests'/'support'/'optional_hook_api_shim'),str(surge_test),str(surge_model),'-o',str(exe)
    ],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    if q.returncode:
        print('SURGE_VM_MODEL_RUNTIME: FAIL - compile/link')
        print(q.stdout[-2000:]); print(q.stderr[-4000:]); sys.exit(1)
    q=subprocess.run([str(exe)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    if q.returncode:
        print(f'SURGE_VM_MODEL_RUNTIME: FAIL - runtime exit {q.returncode}')
        print(q.stdout[-2000:]); print(q.stderr[-4000:]); sys.exit(1)
print('SURGE_VM_MODEL_RUNTIME: PASS (compiled, linked, executed)')
