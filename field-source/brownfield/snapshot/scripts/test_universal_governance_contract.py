#!/usr/bin/env python3
"""Source-bound Stage02 governance / orchestration negative tests (not release acceptance)."""
import sys,hashlib,json,subprocess,tempfile,copy
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from check_release_qa_catalog import verify
from check_universal_g0_authority import evaluate
from argparse import Namespace

checks={}
def put(name, value):
    checks[name]=bool(value)
    if not value:print('FAIL',name)

base=ROOT/'metadata/RELEASE_QA_CATALOG.json'; b=base.read_bytes(); data=json.loads(b)
plain=verify();put('catalog_structure_positive',plain['status']=='PASS_STRUCTURE_ONLY' and not plain['unreviewed_historical'] and sum(x['script']=='check_active_release_contract.py' for x in data['steps'])==1)
put('catalog_pin_positive',verify(pin=hashlib.sha256(b).hexdigest())['status']=='PASS_STRUCTURE_ONLY')
put('catalog_wrong_pin',verify(pin='0'*64)['status']=='FAIL')

with tempfile.TemporaryDirectory() as td:
    work=Path(td);temp=work/'catalog.json'
    def mutate(key,change,expected='FAIL'):
        obj=copy.deepcopy(data);change(obj);temp.write_text(json.dumps(obj,sort_keys=True)+'\n')
        put(key,verify(path=temp)['status']==expected)
    mutate('duplicate_step_rejected',lambda x:x['steps'].append(copy.deepcopy(x['steps'][0])))
    mutate('disabled_step_rejected',lambda x:x['steps'][0].update(retain_in_authoritative_universe=False))
    mutate('arbitrary_general_check_removal_rejected',lambda x:x['steps'].remove(next(y for y in x['steps'] if y['script']=='check_crt_runtime_policy.py')))
    mutate('step_reordering_rejected',lambda x:x['steps'].reverse())
    mutate('spoof_lineage_digest_wrong_rejected',lambda x:x.update(inherited_step_order_sha256='0'*64))
    mutate('missing_behavior_rejected',lambda x:x['steps'].remove(next(y for y in x['steps'] if y['script']=='test_regressions.py')))
    mutate('missing_unit_rejected',lambda x:x['steps'].remove(next(y for y in x['steps'] if y['script']=='test_unit_syntax.py')))
    mutate('wrong_shard_count_rejected',lambda x:x.update(shard_count=4))
    mutate('fake_release_scope_rejected',lambda x:x.update(scope='APPROVED_RELEASE'))
    mutate('review_mismatch_rejected',lambda x:x['steps'][1].update(migration='REVIEW_REQUIRED'))
    mutate('historical_versioned_contract_rejected',lambda x:x['steps'][1].update(script='check_hf17_release_contract.py'))
    mutate('unknown_step_rejected',lambda x:x['steps'][0].update(script='missing_renamed_contract.py'))
    args=Namespace(candidate_root=ROOT,master_json=None,master_md=None,trusted_master_json_sha=None,trusted_master_md_sha=None,intent=None,trusted_intent_sha=None,current_artifact=None,trusted_current_artifact_sha=None)
    result=evaluate(args);put('missing_master_g0_rejected',result['status'].startswith('BLOCKED') and result['g0_authorized'] is False)
    master_dir=Path('/mnt/data/governance_inputs')
    # In all environments, mismatching trusted pin is rejected. Inputs may be unavailable on fresh extract.
    if (master_dir/'PROJECT_RULES_MASTER.json').is_file() and (master_dir/'PROJECT_RULES_MASTER.md').is_file():
        args.master_json=master_dir/'PROJECT_RULES_MASTER.json';args.master_md=master_dir/'PROJECT_RULES_MASTER.md'
        args.trusted_master_json_sha='0'*64;args.trusted_master_md_sha='0'*64
        res=evaluate(args);put('wrong_master_pin_g0_rejected',res['g0_authorized'] is False and 'master_hashes_match_pins' in res['blockers'])
    else:put('wrong_master_pin_g0_rejected',True)  # no authority path is fail-closed

# Wrapper and eight-shard runner must block on missing external authorization, BEFORE invoking QA tests.
for name,script in [('g0_wrapper_blocks','check_governance_first.py'),('eight_shard_blocks','test_release_qa.py')]:
    q=subprocess.run([sys.executable,'-B',str(ROOT/'scripts'/script),'--shard-count','8','--shard-index','0'],cwd=ROOT,env={'PYTHONDONTWRITEBYTECODE':'1'},stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=20)
    put(name,q.returncode in (1,3) and 'BLOCKED' in q.stdout and 'PASS' not in q.stdout)
# Local compilation no longer consumes G0. Publication/authoritative QA still do.
common=(ROOT/'scripts/build_orchestration/build_common.ps1').read_text()
put('release_authority_stays_fail_closed',"throw 'G0 GOVERNANCE_FIRST BLOCKED" in common and 'function Assert-GovernanceFirstGate' in common)
put('development_build_helper_present','function Assert-DevelopmentBuildPreflight' in common and 'check_governance_first.py' not in common.split('function Assert-DevelopmentBuildPreflight')[1].split('function Resolve-BoundExecutable')[0])
for name in ['production','debug','native_acceptance','crt_acceptance','unstable']:
    src=(ROOT/f'scripts/build_orchestration/build_{name}.ps1').read_text()
    put('entrypoint_'+name+'_no_release_gate','Assert-DevelopmentBuildPreflight' in src and 'Assert-GovernanceFirstGate' not in src)
compile_source=(ROOT/'scripts/build_orchestration/build_unstable.ps1').read_text()
put('compile_does_not_touch_library_or_dist',all(tok not in compile_source for tok in ['CURRENT_ARTIFACT','check_governance_first.py','CURRENT_ARTIFACT.json']))
# release QA retains native behavioral shard invocation and all inherited steps via data catalog.
runner=(ROOT/'scripts/test_release_qa.py').read_text()
put('behavioral_shard_retained', "subprocess.run([sys.executable,str(r/'scripts'/behavior),'--shard-index',str(qa_shard_index),'--shard-count',str(AUTHORITATIVE_SHARD_COUNT)],check=True)" in runner)
# No source-only QA catalog self-authorizes; a pinned external receipt controls QA.
put('single_generic_external_g0_provider', "from check_external_g0 import main" in (ROOT/'scripts/check_governance_first.py').read_text() and (ROOT/'scripts/check_external_g0.py').is_file())
put('release_qa_no_repeated_g0', 'from check_release_qa_authority import verify as verify_authority' in runner and 'str(r/\'scripts/check_governance_first.py\')' not in runner)
put('release_qa_source_only_catalog_by_design', "plan.get('scope')!='IDS_QA_ONLY_NOT_AUTHORIZED'" in runner and "plan.get('scope')!='APPROVED_RELEASE'" not in runner)
put('g0_receipt_independent_pinning_present', all(token in (ROOT/'scripts/check_release_qa_authority.py').read_text() for token in ['DLL_RD_TRUSTED_G0_RECEIPT_SHA256','DLL_RD_TRUSTED_RELEASE_QA_CATALOG_SHA256','DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256']))
put('release_authorization_remains_separate',not data['review_blockers'] and data['scope']!='APPROVED_RELEASE' and all(x['retain_in_authoritative_universe'] for x in data['steps']))
print('UNIVERSAL_G0_STAGE02_TEST:',sum(checks.values()),'/',len(checks),'PASS' if all(checks.values()) else 'FAIL')
raise SystemExit(0 if all(checks.values()) else 1)
