#!/usr/bin/env python3
"""QA-only runner. Not an authoritative release or G0 publication gate."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    ap=argparse.ArgumentParser(description='Offline source-level, NOT_RELEASE policy checks')
    ap.add_argument('--trusted-contract-sha',required=True,help='Explicit SHA pin from test evidence; NOT release authority')
    ap.add_argument('--report',type=Path)
    a=ap.parse_args()
    env={**os.environ,'PYTHONDONTWRITEBYTECODE':'1','DLL_RD_SHIPPING_CONTRACT_SHA256':a.trusted_contract_sha}
    steps=[
        ('active_manifest',['scripts/check_manifest.py']),
        ('universal_policy',['scripts/check_universal_shipping_policy.py','--trusted-contract-sha',a.trusted_contract_sha]),
        ('negative_regressions',['scripts/test_universal_shipping_policy.py']),
        ('shipping_build_policy',['scripts/test_shipping_build_policy.py']),
        ('production_source_graph',['scripts/check_production_source_graph.py']),
        ('repository_contract',['scripts/check_repository_contract.py']),
    ]
    results=[]
    for label,argv in steps:
        res=subprocess.run([sys.executable,'-B',str(ROOT/argv[0]),*argv[1:]],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        output=res.stdout[-8000:]
        print(label, 'PASS' if res.returncode==0 else 'FAIL',res.returncode)
        if res.returncode:print(output)
        results.append({'name':label,'exit_code':res.returncode,'last_output':output})
    checks={x['name']:x['exit_code']==0 for x in results}
    summary={'schema':'shipping-policy-qa-only.v1','status':'PASS_SOURCE_STATIC_QA_ONLY' if all(checks.values()) else 'FAIL_SOURCE_STATIC','checks':checks,'results':results,'execution_level':'source_static','governance_G0':'BLOCKED_PENDING_INDEPENDENT_AUTHORITY','authoritative_8x8':'NOT_RUN','native_or_game_acceptance':'NOT_RUN','release_authorized':False}
    if a.report:
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(summary,indent=2,sort_keys=True)+'\n')
    print('UNIVERSAL_SHIPPING_QA_ONLY:',summary['status'],'NO_RELEASE_CLAIM')
    return 0 if all(checks.values()) else 1
if __name__=='__main__':sys.exit(main())
