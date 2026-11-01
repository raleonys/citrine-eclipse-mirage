#!/usr/bin/env python3
"""Exact HF17 HOTFIX_RELEASE G1 contract. Does NOT certify G2/G3 or publication.
The historical HF16 release contract remains byte-identical and is never weakened.
External Governance First authority is required to run the preflight.
"""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
ID='preloader_l 3.2.5 SQUALERIS HOTFIX 17'
FN='preloader_l_3.2.5_SQUALERIS_HOTFIX_17.zip'
PARENT='9b380e4d953dffd03ace8b762e1199f11ca8ca4f373d86a0c9b3c074d647082c'
MD='81ce3b20e0151cb80e55642172bf2deebd7f70f560689787560d7bcffed6d69d'
JS='217399a51464b2bd7e72d39920845d4f1cc69d003201bc95337aacc737c66504'
HISTORICAL_HF16_CONTRACT_SHA='2aa3cb5c1977ad21c9863f381bf0e6fcf2083200c2f158430bab05f15b33ad23'
HISTORICAL_ACHIEVEMENT_SHA='40f817b5c9ff07d91186e0ab5b742fbaf710dcb7eeebf2c14763b875ead045ea'
sha=lambda b:hashlib.sha256(b).hexdigest()
j=lambda name:json.loads((ROOT/name).read_text(encoding='utf-8'))
errors=[]
def check(k,v):
 if not v:errors.append(k)
# Check G0 from separate, externally pinned Library authority before all G1 work.
g0=subprocess.run([sys.executable,'-B',str(ROOT/'scripts/check_governance_first.py')],cwd=ROOT,capture_output=True,text=True)
check('G0 external library authority',g0.returncode==0)
if g0.returncode:
 print('HF17_G1_RELEASE_CONTRACT: BLOCKED G0', (g0.stdout+g0.stderr)[-1800:]);sys.exit(2)
try:
 s=j('RELEASE_STATE.json');a=j('metadata/RELEASE_AUTHORIZATION.json');p=j('metadata/PUBLICATION_TRANSACTION.json');g=j('metadata/GOVERNANCE_BINDING.json')
 art=j('metadata/ARTIFACT_PROVENANCE.json');v=j('metadata/PROJECT_GOVERNANCE_INVARIANTS.json');acc=j('metadata/RELEASE_ACCEPTANCE.json');led=j('metadata/ACCEPTANCE_FRESHNESS_LEDGER.json');arch=j('metadata/RELEASE_ARCHITECTURE.json')
 check('class identity cross surface',all(t.get('release_type')=='HOTFIX_RELEASE' and t.get('official_release_identity')==ID for t in (s,a,g,art,acc,v)))
 check('exact parent',s.get('hotfix_ordinal')==17 and a.get('requested_ordinal')==17 and s.get('immediate_hotfix_parent',{}).get('sha256')==PARENT and art.get('immediate_parent',{}).get('sha256')==PARENT)
 check('canonical filename',all(x.get('official_package_filename',x.get('official_filename'))==FN for x in (s,p,art)))
 check('HF17 active authorization',a.get('source_development_authorized') is True and p.get('production_authorized') is True and a.get('publication_authorized') is True and p.get('publication_authorized') is True and g.get('publication_authorized') is True)
 check('HF16 pointer preserved',p.get('canonical_before',{}).get('sha256')==PARENT and not s.get('canonical_artifact_pointer_commit') and p.get('canonical_pointer_committed') is False and p.get('post_commit_verified') is False)
 check('publication date not inherited',s.get('publication_date') in (None,'NOT_PUBLISHED','PENDING'))
 check('master exact revision SHA',g.get('master_revision')=='2026-10-17.2' and g.get('library_master_md_sha256')==MD and g.get('library_master_json_sha256')==JS and s.get('governance_master_revision')=='2026-10-17.2')
 check('governance normative mirror integrity',sha((ROOT/'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_bytes())==g.get('normative_governance_mirror_sha256') and sha((ROOT/'metadata/PROJECT_GOVERNANCE_INVARIANTS.json').read_bytes())==g.get('packaged_mirror',{}).get('json_sha256'))
 check('historical HF16 contract protected',sha((ROOT/'scripts/check_release_contract.py').read_bytes())==HISTORICAL_HF16_CONTRACT_SHA)
 check('historical achievement test protected',sha((ROOT/'scripts/test_achievement_coherence.py').read_bytes())==HISTORICAL_ACHIEVEMENT_SHA)
 check('11 physical MinHook targets',arch.get('minhook',{}).get('detours')==11)
 check('HF17 acceptance all pending',len(led.get('entries',[]))>=8 and all(t.get('status')=='PENDING' and t.get('carry_forward') is None and t.get('binary_sha256') is None for t in led.get('entries',[])))
 check('no inherited native/consumer acceptance',acc.get('source_qa',{}).get('status')=='PENDING_HF17_G2_FULL_8_OF_8' and all(acc.get(k,{}).get('status')=='NOT_RUN_HF17' for k in ('collegefb27_game_network','madden27_game_network')) and all(acc.get(k,{}).get('status')=='NOT_RUN_HF17_CHANGED_BUILD_INPUTS' for k in ('windows_native','windows_production_golden','static_crt')))
 check('HF17 architectural acceptance pending',arch.get('source_qa')=='PENDING_HF17_G2_FULL_8_OF_8' and all('HF13' not in str(t) and 'HF10' not in str(t) and 'HF14' not in str(t) for t in arch.get('validation',{}).values()))
 check('historical architecture retained explicitly',arch.get('historical_architecture_validation_pre_hf17',{}).get('status')=='HISTORICAL_NONTRANSFERABLE')
 check('docs build target identity',FN in (ROOT/'docs/BUILD_ID.md').read_text(encoding='utf-8'))
 readme=(ROOT/'README.md').read_text(encoding='utf-8');check('README current release unambiguous',readme.startswith('# '+ID+'\n') and '# preloader_l 3.2.5 SQUALERIS HOTFIX 16\n' not in readme)
 check('release acceptance document fresh-not-published',ID in (ROOT/'docs/RELEASE_ACCEPTANCE.md').read_text(encoding='utf-8').splitlines()[0] and 'HF16 prior Windows/Golden evidence does not automatically transfer' in (ROOT/'docs/RELEASE_ACCEPTANCE.md').read_text(encoding='utf-8'))
 check('HOTFIX G3 applicability',j('metadata/RELEASE_ACCEPTANCE.json').get('release_class_g3',{}).get('status')=='NOT_APPLICABLE' and j('metadata/PROJECT_GOVERNANCE_INVARIANTS.json').get('governance_first_protocol',{}).get('gate_applicability',{}).get('G3',{}).get('not_applicable_status')=='NOT_APPLICABLE')
 check('source-to-historical-limits',s.get('hf17_implementation',{}).get('b_ownership_prototype')=='EXCLUDED' and p.get('publication_authorized') is True)
 qa_script=(ROOT/'scripts/test_release_qa.py').read_text(encoding='utf-8')
 check('authoritative QA routes G1'," 'check_hf17_release_contract.py'," in qa_script and " 'check_release_contract.py'," not in qa_script)
 catalog=j('metadata/SCRIPT_OWNERSHIP.json')
 check('G1 script owned',sum(t.get('path')=='scripts/check_hf17_release_contract.py' for t in catalog.get('scripts',[]))==1)
except Exception as exc:errors.append('mandatory document parse: '+str(exc))
if errors:
 print('HF17_G1_RELEASE_CONTRACT: BLOCKED',len(errors));[print(' -',e) for e in errors];sys.exit(1)
print('HF17_G1_RELEASE_CONTRACT: PASS - repository/metadata only; G2/G3/G4 NOT CLAIMED')
