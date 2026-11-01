#!/usr/bin/env python3
"""Synthetic, self-contained G0 -> QA receipt positive/negative protocol tests.

A generated operator fixture is NOT a real independent publication authority.
No release or official 8-shard PASS can result from this test.
"""
import copy
import hashlib
import json
import sys
import tempfile
import zipfile
from pathlib import Path

sys.dont_write_bytecode = True
from check_external_g0 import evaluate, INPUTS
import check_release_qa_catalog
from check_release_qa_authority import verify as verify_receipt


def digest(data): return hashlib.sha256(data).hexdigest()
def canon(value): return (json.dumps(value, sort_keys=True, indent=2)+'\n').encode()

def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(canon(value) if not isinstance(value, bytes) else value)

def make_zip(root, dest):
    with zipfile.ZipFile(dest, 'w', compression=zipfile.ZIP_DEFLATED) as z:
        for path in sorted(root.rglob('*')):
            if path.is_file(): z.write(path, path.relative_to(root).as_posix())

checks = {}
def put(key, good): checks[key] = bool(good)

with tempfile.TemporaryDirectory(prefix='universal_release_protocol_TEST_ONLY_') as td:
    base=Path(td);root=base/'source';root.mkdir();ext=base/'external';ext.mkdir()
    version, codename, ordinal = '3.2.5', 'SQUALERIS', 57
    parent_display=f'preloader_l {version} {codename} HOTFIX {ordinal}'
    target=f'preloader_l {version} {codename} HOTFIX {ordinal+1}'
    parent=ext/'parent.zip';parent.write_bytes(b'fixture parent only, not authentic')
    parent_sha=digest(parent.read_bytes())
    master_md=ext/'master.md';master_md.write_bytes(b'test master; not real governance')
    master={'schema':'dll-rd-football.project-rules-master.v1','revision':'TEST_ONLY', 'status':'ACTIVE_FAIL_CLOSED','governance_first_protocol':{'status':'ACTIVE_FAIL_CLOSED'},'current_release_line':{'artifact_sha256':parent_sha,'display':parent_display,'release_type':'HOTFIX_RELEASE','hotfix_ordinal':ordinal,'semantic_version':version,'codename':codename}}
    master_json=ext/'master.json';write(master_json,master)
    parent_identity={'display':parent_display,'hotfix_ordinal':ordinal,'release_type':'HOTFIX_RELEASE'}
    pointer=ext/'pointer.json';write(pointer,{'schema':'dll-rd-football.current-artifact.v1','status':'CURRENT_ACCEPTED_OFFICIAL_RELEASE','release_identity':parent_identity,'current_artifact':{'sha256':parent_sha}})
    state={'official_release_identity':target,'hotfix_ordinal':ordinal+1,'release_type':'HOTFIX_RELEASE','immediate_hotfix_parent':{'sha256':parent_sha},'governance_master_revision':master['revision'],'governance_master_json_sha256':digest(master_json.read_bytes()),'governance_master_md_sha256':digest(master_md.read_bytes()),'canonical_artifact_pointer_commit':False,'publication_date':None}
    write(root/'RELEASE_STATE.json',state)
    write(root/'metadata/RELEASE_AUTHORIZATION.json',{'official_release_identity':target,'publication_authorized':True})
    write(root/'metadata/PUBLICATION_TRANSACTION.json',{'official_release_identity':target,'publication_authorized':True,'canonical_before':{'identity':parent_display,'sha256':parent_sha},'canonical_pointer_committed':False,'post_commit_verified':False})
    script_names=['test_regressions.py','test_unit_syntax.py','check_active_release_contract.py']+[f'test_fixture_{n}.py' for n in range(61)]
    for name in script_names: write(root/'scripts'/name, b'# SYNTHETIC TEST ONLY\n')
    catalog={'schema':'universal-release-qa-catalog.v1','scope':'IDS_QA_ONLY_NOT_AUTHORIZED','release_identity':None,'shard_count':8,'steps':[{'script':x,'migration':'GENERAL_CHECK','retain_in_authoritative_universe':True} for x in script_names], 'inherited_step_count':len(script_names),'inherited_step_order_sha256':digest(('\n'.join(script_names)+'\n').encode()),'inherited_lineage_role':'SOURCE_ONLY_UNREVIEWED_HISTORICAL_AUTHORITATIVE_SET','behavioral_step':'test_regressions.py','review_blockers':[]}
    write(root/'metadata/RELEASE_QA_CATALOG.json',catalog)
    for n in range(110): write(root/f'fixtures/sample_{n:03d}.txt', b'fixture')
    manifest={p.relative_to(root).as_posix():digest(p.read_bytes()) for p in root.rglob('*') if p.is_file()}
    write(root/'metadata/ACTIVE_SOURCE_MANIFEST.json',{'files':manifest})
    candidate=ext/'candidate.zip';make_zip(root,candidate)
    intent=ext/'intent.json';write(intent,{'release_identity':target,'release_type':'HOTFIX_RELEASE','publication_authorized':True,'source_development_authorized':True,'publication_requires_gate_pass':True,'publication_committed':False,'user_instruction':'explicit synthetic test-only example','immediate_official_parent':{'sha256':parent_sha},'master':{'revision':'TEST_ONLY','json_sha256':digest(master_json.read_bytes()),'md_sha256':digest(master_md.read_bytes())},'candidate_source_zip_sha256':digest(candidate.read_bytes())})
    paths={'master_md':master_md,'master_json':master_json,'intent':intent,'pointer':pointer,'parent_zip':parent,'source_zip':candidate}
    env={}
    for name,(path_key,pin_key) in INPUTS.items():
        env[path_key]=str(paths[name]);env[pin_key]=digest(paths[name].read_bytes())
    positive=evaluate(root,env)
    put('positive_general_hotfix58',positive['g0_authorized'] and positive['status']=='PASS_G0_PREPUBLICATION')
    receipt=ext/'G0_RECEIPT.json';write(receipt,positive)
    receipt_env={'DLL_RD_G0_RECEIPT_PATH':str(receipt),'DLL_RD_TRUSTED_G0_RECEIPT_SHA256':digest(receipt.read_bytes()),'DLL_RD_G0_CANDIDATE_ZIP':str(candidate),'DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256':digest(candidate.read_bytes()),'DLL_RD_TRUSTED_RELEASE_QA_CATALOG_SHA256':digest((root/'metadata/RELEASE_QA_CATALOG.json').read_bytes())}
    previous_root=check_release_qa_catalog.ROOT;check_release_qa_catalog.ROOT=root
    try:
        put('receipt_authorizes_preflight_only',verify_receipt(root,receipt_env)['status']=='PASS_PREPUBLICATION_AUTH_ONLY')
        for key in ['DLL_RD_TRUSTED_G0_RECEIPT_SHA256','DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256','DLL_RD_TRUSTED_RELEASE_QA_CATALOG_SHA256']:
            altered=dict(receipt_env);altered[key]='0'*64
            put('bad_pin_'+key,verify_receipt(root,altered)['status']=='BLOCKED')
        altered=dict(receipt_env);altered.pop('DLL_RD_G0_RECEIPT_PATH')
        put('missing_receipt_blocked',verify_receipt(root,altered)['status']=='BLOCKED')
        record=copy.deepcopy(positive);record['g0_authorized']=False
        write(receipt,record);altered=dict(receipt_env);altered['DLL_RD_TRUSTED_G0_RECEIPT_SHA256']=digest(receipt.read_bytes())
        put('forged_blocked_receipt_rejected',verify_receipt(root,altered)['status']=='BLOCKED')
        write(receipt,positive)
        # Mutating a source byte after the G0 snapshot cannot use the old receipt.
        source=(root/'scripts/test_unit_syntax.py');original=source.read_bytes();source.write_bytes(original+b'changed\n')
        put('source_mutation_blocked',verify_receipt(root,receipt_env)['status']=='BLOCKED');source.write_bytes(original)
    finally: check_release_qa_catalog.ROOT=previous_root
    for key, mutate in [
        ('bad_parent',lambda e:e.__setitem__('DLL_RD_TRUSTED_G0_PARENT_ZIP_SHA256','0'*64)),
        ('missing_master',lambda e:e.pop('DLL_RD_GOVERNANCE_MASTER_JSON')),
        ('wrong_candidate',lambda e:e.__setitem__('DLL_RD_TRUSTED_G0_CANDIDATE_ZIP_SHA256','0'*64))]:
        altered=dict(env);mutate(altered)
        put(key+'_blocked',evaluate(root,altered)['status']=='BLOCKED')
    bad=copy.deepcopy(state);bad['hotfix_ordinal']+=2;write(root/'RELEASE_STATE.json',bad)
    put('wrong_ordinal_blocked',evaluate(root,env)['status']=='BLOCKED');write(root/'RELEASE_STATE.json',state)
    bad=copy.deepcopy(intent.read_bytes());intent.write_bytes(b'{"publication_authorized":false}')
    altered=dict(env);altered[INPUTS['intent'][1]]=digest(intent.read_bytes())
    put('invalid_independently_pinned_intent_blocked',evaluate(root,altered)['status']=='BLOCKED')

print('UNIVERSAL_RELEASE_AUTHORITY_SYNTHETIC:',sum(checks.values()),'/',len(checks),'PASS' if all(checks.values()) else 'FAIL','; NOT_A_REAL_G0_RECEIPT_OR_G4')
for key,passed in checks.items():
    if not passed:print('FAIL',key)
raise SystemExit(0 if all(checks.values()) else 1)
