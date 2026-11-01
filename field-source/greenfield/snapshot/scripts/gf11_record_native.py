#!/usr/bin/env python3
"""Operator-only native receipt; must run after *actual* GF11 Windows build/CTest."""
import sys,pathlib,json,hashlib,datetime
from gf10_pe_export_gate import check,PEError
if __name__=='__main__':
 if len(sys.argv)!=3 or sys.argv[2] not in ('Release','Debug'):sys.exit(2)
 build=pathlib.Path(sys.argv[1]).resolve();cfg=sys.argv[2]
 dll=list(build.rglob('preloader_l.dll'));hosts=list(build.rglob('gf_export_host.exe'))
 if len(dll)!=1 or len(hosts)!=1:print('GF11_NATIVE_BLOCKED_PE_SET');sys.exit(3)
 try:shape=check(dll[0].read_bytes())
 except (PEError,ValueError) as e:print('GF11_NATIVE_BLOCKED_PE_EXPORT',str(e));sys.exit(4)
 h=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
 result={'schema':'gf11.windows.local.receipt.v1','classification':'LOCAL_BUILD_NOT_RELEASE','source_class':'GF11_NOT_RELEASE','configuration':cfg,'platform_precondition':'MSVC_X64_PROMPT_VERIFIED_BY_BATCH','dll_sha256':h(dll[0]),'export_host_sha256':h(hosts[0]),'dll_export_shape':shape['dll_exports'],'actual_native_compile_ctest':'EXECUTED_BY_BATCH_PRECONDITION','lsX_runtime':'NOT_RUN','ioCP_dispatch':'NOT_RUN','game_consumer':'NOT_RUN','record_created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 f=build/('GF11_WINDOWS_'+cfg+'_LOCAL_RECEIPT.json'); tmp=f.with_suffix('.partial');tmp.write_text(json.dumps(result,indent=2,sort_keys=True)+'\n');tmp.replace(f)
 print('GF11_NATIVE_RECEIPT_CREATED',f)
