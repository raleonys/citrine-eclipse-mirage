#!/usr/bin/env python3
"""A non-authoritative SHA receipt after native compile+CTest success.
No binary is executed here; the Windows batch must pass CTest first.
"""
import json, hashlib, sys, pathlib, datetime
if len(sys.argv)!=3: sys.exit(2)
root=pathlib.Path(sys.argv[1]).resolve()
config=sys.argv[2]
if config not in ('Release','Debug'): sys.exit(2)
dll=list(root.rglob('preloader_l.dll'))
exe=list(root.rglob('gf_export_host.exe'))
if len(dll)!=1 or len(exe)!=1:
 print('BLOCKED missing/ambiguous native artifacts',dll,exe);sys.exit(3)
H=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
receipt={'schema':'greenfield.gf08.native_handoff.local.v1',
 'classification':'LOCAL_BUILD_NOT_RELEASE', 'config':config,
 'native_ctest':'PRECONDITION_CTEST_EXIT_ZERO_VERIFIED_BY_WINDOWS_BATCH_NOT_INDEPENDENT',
 'lsx_acceptance':'NOT_RUN','consumer_game':'NOT_RUN',
 'dll_path':str(dll[0]),'dll_sha256':H(dll[0]),
 'host_path':str(exe[0]),'host_sha256':H(exe[0]),
 'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
out=root/('GF08_WINDOWS_'+config+'_LOCAL_RECEIPT.json')
out.write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
print('LOCAL_NATIVE_ARTIFACT_HASH_RECEIPT',out)
