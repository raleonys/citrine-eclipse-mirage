#!/usr/bin/env python3
"""Universal G0 Stage02 QA-only authority preflight. NEVER authorizes a release.

Inputs must be externally supplied, with independently trusted hashes. Even a
verified master is not a release-identity approval. No in-tree release metadata
can substitute for a separately pinned explicit publication decision.
"""
import argparse, hashlib, json, re, sys
from pathlib import Path
SHA=re.compile(r"[0-9a-f]{64}\Z")

def h(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p): return json.loads(p.read_bytes())

def evaluate(args):
    checks={}
    def put(k,v): checks[k]=bool(v)
    root=args.candidate_root.resolve()
    put('candidate_root_present',root.is_dir() and (root/'RELEASE_STATE.json').is_file())
    put('external_master_supplied',bool(args.master_json and args.master_md and args.master_json.is_file() and args.master_md.is_file()))
    put('trusted_master_sha_supplied',bool(args.trusted_master_json_sha and args.trusted_master_md_sha and SHA.fullmatch(args.trusted_master_json_sha) and SHA.fullmatch(args.trusted_master_md_sha)))
    if not (checks['candidate_root_present'] and checks['external_master_supplied'] and checks['trusted_master_sha_supplied']):
        return finish(checks)
    master_json=args.master_json.resolve();master_md=args.master_md.resolve()
    put('authority_outside_candidate',root not in master_json.parents and root not in master_md.parents)
    put('master_hashes_match_pins',h(master_json)==args.trusted_master_json_sha and h(master_md)==args.trusted_master_md_sha)
    if not (checks['authority_outside_candidate'] and checks['master_hashes_match_pins']):return finish(checks)
    try: master=read(master_json);state=read(root/'RELEASE_STATE.json')
    except Exception: put('master_or_candidate_parse',False);return finish(checks)
    put('master_valid_contract',master.get('schema')=='dll-rd-football.project-rules-master.v1' and master.get('scope')=='release-independent project governance' and master.get('status')=='ACTIVE_FAIL_CLOSED' and master.get('governance_first_protocol',{}).get('status')=='ACTIVE_FAIL_CLOSED')
    current=master.get('current_release_line',{})
    put('published_parent_grounded',bool(current.get('display')) and bool(SHA.fullmatch(current.get('artifact_sha256',''))) and current.get('release_type') in master.get('release_types',{}))
    put('no_in_tree_pointer_commit',state.get('canonical_artifact_pointer_commit') is False)
    # Publication intent and current pointer must be independently supplied.
    put('independent_release_intent_supplied',bool(args.intent and args.intent.is_file()))
    put('independent_intent_sha_supplied',bool(args.trusted_intent_sha and SHA.fullmatch(args.trusted_intent_sha)))
    put('independent_current_pointer_supplied',bool(args.current_artifact and args.current_artifact.is_file()))
    put('independent_current_pointer_sha_supplied',bool(args.trusted_current_artifact_sha and SHA.fullmatch(args.trusted_current_artifact_sha)))
    if not all(checks.values()):return finish(checks)
    intent=args.intent.resolve();pointer=args.current_artifact.resolve()
    put('decisions_not_packaged',root not in intent.parents and root not in pointer.parents)
    put('external_decision_hashes',h(intent)==args.trusted_intent_sha and h(pointer)==args.trusted_current_artifact_sha)
    if not (checks['decisions_not_packaged'] and checks['external_decision_hashes']):return finish(checks)
    try: decision=read(intent);canonical=read(pointer)
    except Exception: put('decision_or_pointer_parse',False);return finish(checks)
    # This is a *conservative diagnostic only*, not a publication gate.
    put('intent_separate_target',decision.get('release_identity') != current.get('display') and bool(decision.get('release_identity')))
    put('intent_class_allowed',decision.get('release_type') in master.get('release_types',{}))
    put('intent_parent_bound',decision.get('parent_sha256')==current.get('artifact_sha256'))
    put('intent_governance_revision',decision.get('governance_revision')==master.get('revision'))
    put('intent_explicit_pub_authorized',decision.get('publication_authorized') is True and decision.get('source_development_authorized') is True)
    put('candidate_identity_bound',decision.get('release_identity')==state.get('official_release_identity'))
    put('candidate_parent_bound',state.get('immediate_hotfix_parent',{}).get('sha256')==current.get('artifact_sha256'))
    put('current_artifact_pointer_bound',canonical.get('sha256')==current.get('artifact_sha256') and canonical.get('identity')==current.get('display'))
    return finish(checks)

def finish(checks):
    blockers=[k for k,v in checks.items() if not v]
    # Intentionally no PASS_G0, regardless of synthetic fixtures or inputs.
    return {'schema':'universal-g0-authority-boundary-diagnostic.v1','status':'BLOCKED_PENDING_AUTHORIZED_G0_MIGRATION','checks':checks,'blockers':blockers,'authority_documents_consistent':not blockers,'g0_authorized':False,'build_authorized':False,'publication_authorized':False}

def main():
    p=argparse.ArgumentParser();p.add_argument('--candidate-root',type=Path,required=True)
    p.add_argument('--master-json',type=Path);p.add_argument('--master-md',type=Path)
    p.add_argument('--trusted-master-json-sha');p.add_argument('--trusted-master-md-sha')
    p.add_argument('--intent',type=Path);p.add_argument('--trusted-intent-sha')
    p.add_argument('--current-artifact',type=Path);p.add_argument('--trusted-current-artifact-sha')
    p.add_argument('--report',type=Path);a=p.parse_args();r=evaluate(a)
    if a.report: a.report.write_text(json.dumps(r,indent=2,sort_keys=True)+'\n')
    print('UNIVERSAL_G0_AUTHORITY_BOUNDARY:',r['status'],'checks',sum(r['checks'].values()),'/',len(r['checks']))
    if r['blockers']:print('BLOCKERS:',', '.join(r['blockers']))
    return 1
if __name__=='__main__':sys.exit(main())
