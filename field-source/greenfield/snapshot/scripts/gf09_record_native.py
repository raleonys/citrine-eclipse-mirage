#!/usr/bin/env python3
"""Hash local binaries after Windows CMD has validated compilation and CTest."""
import pathlib,hashlib,sys,json,datetime
if len(sys.argv)!=3 or sys.argv[2] not in ('Release','Debug'):sys.exit(2)
root=pathlib.Path(sys.argv[1]).resolve();config=sys.argv[2]
dll=list(root.rglob('preloader_l.dll'));exe=list(root.rglob('gf_export_host.exe'))
if len(dll)!=1 or len(exe)!=1:print('BLOCKED ambiguous/missing binaries',dll,exe);sys.exit(3)
h=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
rec={'schema':'greenfield.gf09.windows.local.v1','classification':'LOCAL_BUILD_NOT_RELEASE',
'config':config,'compile_and_ctest':'WINDOWS_CMD_PRECONDITION_NOT_INDEPENDENTLY_VERIFIED',
'dll_sha256':h(dll[0]),'export_host_sha256':h(exe[0]),'dll_path':str(dll[0]),
'original_owner_objects':'BUILD_TARGET_PRESENT_NOT_INDEPENDENTLY_VERIFIED',
'lsx':'NOT_RUN','iocp_session':'NOT_RUN','consumer_game':'NOT_RUN',
'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
out=root/('GF09_WINDOWS_'+config+'_LOCAL_RECEIPT.json')
out.write_text(json.dumps(rec,indent=2)+'\n');print('GF09_LOCAL_RECEIPT',out)
