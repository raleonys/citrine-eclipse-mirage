#!/usr/bin/env python3
"""Invoked ONLY after Windows cmd build+ctest; independent PE static check and hashes."""
import sys,pathlib,json,hashlib,datetime
from gf10_pe_export_gate import check,PEError
if __name__=='__main__':
    if len(sys.argv)!=3 or sys.argv[2] not in ('Release','Debug'):sys.exit(2)
    build=pathlib.Path(sys.argv[1]).resolve(); cfg=sys.argv[2]
    dll=list(build.rglob('preloader_l.dll'));hosts=list(build.rglob('gf_export_host.exe'))
    if len(dll)!=1 or len(hosts)!=1:
        print('GF10_NATIVE_RECEIPT_BLOCKED: ambiguous or absent PE',dll,hosts);sys.exit(3)
    try: pe=check(dll[0].read_bytes())
    except (PEError,ValueError) as e:print('GF10_NATIVE_RECEIPT_BLOCKED:',e);sys.exit(4)
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    record={'schema':'gf10.windows.local.receipt.v1','status':'LOCAL_BUILD_NOT_RELEASE',
      'configuration':cfg,'platform':'WIN32_MSVC_X64_OPERATOR_PRECONDITION',
      'source_class':'GF10_NOT_RELEASE','dll_sha256':sha(dll[0]),
      'dll_path':str(dll[0]),'export_host_sha256':sha(hosts[0]),
      'dll_export_shape':pe['dll_exports'],
      'native_build_and_ctest':'EXECUTED_BY_OPERATOR_BEFORE_RECEIPT',
      'origin_abi_behavioral_parity':'NOT_ESTABLISHED',
      'release_server_iocp':'NOT_RUN','real_lsx':'NOT_RUN','consumer_game':'NOT_RUN',
      'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
    p=build/('GF10_WINDOWS_'+cfg+'_LOCAL_RECEIPT.json')
    tmp=p.with_suffix('.json.partial');tmp.write_text(json.dumps(record,sort_keys=True,indent=2)+'\n',encoding='utf8');tmp.replace(p)
    print('GF10_NATIVE_RECEIPT_CREATED',p);sys.exit(0)
