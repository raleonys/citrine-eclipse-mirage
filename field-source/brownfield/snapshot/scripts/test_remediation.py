#!/usr/bin/env python3
from pathlib import Path
import json,re,shutil,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
errors=[]

def run(cmd,cwd=r):
    p=subprocess.run(cmd,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    return p.returncode,p.stdout

def check(name,ok,detail=''):
    print(('PASS' if ok else 'FAIL')+': '+name+((' - '+detail) if detail and not ok else ''))
    if not ok: errors.append(name+((': '+detail) if detail else ''))

# F323-S0016-001: injected non-production CPP must fail all three shipped graph gates.
with tempfile.TemporaryDirectory(prefix='remediation_s0016_') as td:
    fx=Path(td)/'src'; shutil.copytree(r,fx)
    p=fx/'CMakeLists.txt'; s=p.read_text(); p.write_text(s.replace('add_library(preloader_l SHARED\n','add_library(preloader_l SHARED\n    tests/native/native_acceptance_exports.cpp\n',1))
    rejected=True
    for rel in ['scripts/check_production_source_graph.py','scripts/check_minhook_architecture.py','scripts/check_production_syntax.py']:
        rc,_=run([sys.executable,rel],fx); rejected &= rc!=0
    check('F323-S0016-001 production graph rejects test source',rejected)

# F323-S0021-001: exact 24 permutations of 2 desc + 2 flag values.
with tempfile.TemporaryDirectory(prefix='remediation_s0021_') as td:
    exe=Path(td)/'store_feature_interleaving'
    srcs=['tests/unit/test_store_feature_interleaving.cpp','third_party/tinyxml2/tinyxml2.cpp','src/config/store_values.cpp','src/config/store_parser.cpp','src/config/store_materialization.cpp','src/config/store_runtime.cpp','src/config/settings_values.cpp']
    rc1,out1=run(['c++','-std=c++17','-O0','-Wall','-Wextra','-Werror','-I'+str(r/'include')]+[str(r/x) for x in srcs]+['-o',str(exe)])
    rc2,out2=(99,'compile failed') if rc1 else run([str(exe)])
    check('F323-S0021-001 feature interleaving 24/24',rc1==0 and rc2==0,(out1+out2)[-500:])

# F323-S0031-001: authoritative acceptance-freshness clause deletion must fail the exact Library-master mirror check.
with tempfile.TemporaryDirectory(prefix='remediation_s0031_') as td:
    fx=Path(td)/'src'; shutil.copytree(r,fx)
    p=fx/'docs/PROJECT_GOVERNANCE_CONTRACT.md'; s=p.read_text()
    line='- Shipping-impacting changes invalidate inherited acceptance only for the relevant changed surfaces; required freshness must be re-established before release claims.\n'
    mutated=line in s
    if mutated: p.write_text(s.replace(line,'',1))
    rc,out=run([sys.executable,'scripts/check_repository_contract.py'],fx)
    check('F323-S0031-001 governance freshness fail-closed',mutated and rc!=0 and 'packaged Library governance MD mirror hash drift' in out)

# F323-S0041-001: unregistered active script must fail.
with tempfile.TemporaryDirectory(prefix='remediation_s0041_') as td:
    fx=Path(td)/'src'; shutil.copytree(r,fx); (fx/'scripts/maintenance.py').write_text('')
    rc,out=run([sys.executable,'scripts/check_repository_contract.py'],fx)
    check('F323-S0041-001 script owner/purpose fail-closed',rc!=0 and 'active script has no owner/purpose' in out)

# F323-S0042-001: maintained architecture registry must be active.
rc,out=run([sys.executable,'scripts/check_repository_contract.py'])
check('F323-S0042-001 architecture review registry active',rc==0 and 'ARCHITECTURE_REVIEW:' in out)

# F323-S0056-001 / F323-S0070-001: vendoring uses single separator normalization and exact tracked tree export.
vendor=(r/'scripts/build_orchestration/build_minhook_vendor.ps1').read_text(encoding='utf-8',errors='replace')
check('F323-S0056-001 MinHook manifest path normalization',".Replace('\\','/')" in vendor and ".Replace('\\\\','/')" not in vendor)
check('F323-S0070-001 MinHook tracked-tree membership','ls-tree -r --name-only $Pin' in vendor and 'Get-ChildItem -LiteralPath $src -Force' not in vendor and '--untracked-files=all' in vendor)

# F323-S0062-001: standalone Windows syntax gate carries the pinned MinHook include root.
syntax=(r/'scripts/test_production_cpp_syntax.ps1').read_text(encoding='utf-8',errors='replace')
check('F323-S0062-001 standalone syntax MinHook include',"third_party\\minhook\\upstream\\include" in syntax)

# F323-S0066-001: deleting the actual environment reset operation must fail the shipping policy.
with tempfile.TemporaryDirectory(prefix='remediation_s0066_') as td:
    fx=Path(td)/'src'; shutil.copytree(r,fx)
    p=fx/'scripts/build_orchestration/build_common.ps1'; s=p.read_text(); needle="        [Environment]::SetEnvironmentVariable($name,$null,'Process')\n"
    mutated=needle in s
    if mutated: p.write_text(s.replace(needle,'        # removed by negative fixture\n',1))
    rc,out=run([sys.executable,'scripts/test_shipping_build_policy.py'],fx)
    check('F323-S0066-001 build selector reset operation protected',mutated and rc!=0 and 'selector environment sanitized' in out)

# Filename-policy hard rules pulled from the current Library governance mirror.
gov=(r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_text(encoding='utf-8',errors='replace')
for token in [
    'Production source/header pairs representing the same component use the same semantic stem.',
    'Single route handler: `<concept>_route.cpp`.',
    'Active filenames do not contain transient HF/F/W/version/campaign/RVA bookkeeping; lineage belongs in provenance/evidence/history.',
]:
    check('filename governance '+token[:36],token in gov)

if errors:
    print('REMEDIATION_REGRESSIONS: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print('REMEDIATION_REGRESSIONS: PASS (9/9 retained roots + filename governance)')
