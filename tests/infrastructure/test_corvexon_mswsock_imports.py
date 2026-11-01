#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Fail-closed HF13 CORVEXON MSWSOCK module/name regression; optional real PE replay."""
from pathlib import Path
import argparse, json, sys
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'scripts'))
from verify_corvexon_golden_trail import PE, validate_restricted_named_imports

root=Path(__file__).resolve().parents[2]
policy=json.loads((root/'tests/support/golden/metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())['import_policy']
assert policy['restricted_named_imports']=={'MSWSOCK.DLL':['AcceptEx','GetAcceptExSockaddrs']}
assert 'MSWSOCK.DLL' in policy['allowed_modules']
assert 'MSWSOCK.DLL' not in policy['required_modules']
checks=3

class Fake:
    def __init__(self,names): self.names=names
    def import_names(self,module):
        assert module=='MSWSOCK.DLL'
        return self.names

def trial(names,present=True):
    modules={'MSWSOCK.DLL'} if present else set()
    return validate_restricted_named_imports(Fake(names),policy,modules)

for names in (['AcceptEx','GetAcceptExSockaddrs'],['GetAcceptExSockaddrs','AcceptEx']):
    assert not trial(names); checks+=1
assert not trial([],False); checks+=1
for names in (['AcceptEx'], ['GetAcceptExSockaddrs'], ['AcceptEx','GetAcceptExSockaddrs','Other'],
              ['AcceptEx','AcceptEx','GetAcceptExSockaddrs'], ['#123','GetAcceptExSockaddrs'],
              ['acceptex','GetAcceptExSockaddrs']):
    assert trial(names), f'wrongly accepted {names}'
    checks+=1

ap=argparse.ArgumentParser()
ap.add_argument('--dll',type=Path,help='Check the MSWSOCK imported function names in an actual linked Windows PE.')
args=ap.parse_args()
if args.dll:
    pe=PE(args.dll)
    modules=set(pe.imports())
    assert 'MSWSOCK.DLL' in modules, 'real PE fixture must contain MSWSOCK.DLL'
    names=pe.import_names('MSWSOCK.DLL')
    assert names==['AcceptEx','GetAcceptExSockaddrs'], names
    assert not validate_restricted_named_imports(pe,policy,modules)
    checks+=2
    # Prove the actual PE import-name parser rejects a tampered named import.
    original=pe.b
    assert original.count(b'AcceptEx\0')==1, 'unexpected fixture import name multiplicity'
    pe.b=original.replace(b'AcceptEx\0',b'RejectEx\0',1)  # in-memory only
    assert 'RejectEx' in pe.import_names('MSWSOCK.DLL')
    assert validate_restricted_named_imports(pe,policy,modules)
    checks+=2
print(f'CORVEXON_MSWSOCK_IMPORTS: PASS ({checks}/{checks})')
