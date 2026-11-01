#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,sys
r=Path(__file__).resolve().parents[1]
I='preloader_l 3.2.5 SQUALERIS HOTFIX 16';F='preloader_l_3.2.5_SQUALERIS_HOTFIX_16.zip';P='d396109c90b7054fa592f4143a9aa511c687eaff879fc1c418051236e26f6235'
j=lambda p:json.loads((r/p).read_text())
s=j('RELEASE_STATE.json');a=j('metadata/RELEASE_ACCEPTANCE.json');p=j('metadata/PUBLICATION_TRANSACTION.json');g=j('metadata/GOVERNANCE_BINDING.json');l=j('metadata/ACCEPTANCE_FRESHNESS_LEDGER.json');v=j('metadata/PROJECT_GOVERNANCE_INVARIANTS.json')
e=[]
def ok(name,cond):
 if not cond:e.append(name)
def validate_acceptance_document(document,metadata,identity):
 issues=[]
 if not document.startswith('# '+identity+' — Acceptance freshness'):issues.append('identity/heading')
 if metadata.get('official_release_identity')!=identity or metadata.get('release_type')!='HOTFIX_RELEASE':issues.append('metadata identity')
 parent=metadata.get('source_parent_sha256','')
 if len(parent)!=64 or document.count('Source parent SHA-256: `'+parent+'`.')!=1:issues.append('parent sha')
 rows={'Windows Production Golden':'PASS_CARRIED_FORWARD','Windows Debug':'PASS_CARRIED_FORWARD','Windows Native':'PASS_CARRIED_FORWARD','Static CRT /MT':'PASS_CARRIED_FORWARD','Windows path/locale native':'PASS_SOURCE_LINKED','HF16 exact source QA 8/8 and fresh-extract 8/8':'PENDING','CollegeFB27 / Madden27':'USER_REPORTED_SMOKE'}
 for name,status in rows.items():
  if document.count('| '+name+' | '+status+' |')!=1:issues.append('row:'+name)
 for k in ('windows_production_golden','windows_debug','windows_native','static_crt'):
  if metadata.get(k,{}).get('status')!='PASS_CARRIED_FORWARD_IDENTICAL_BUILD_INPUTS':issues.append('native freshness '+k)
 for k in ('collegefb27_game_network','madden27_game_network'):
  if metadata.get(k,{}).get('status')!='USER_REPORTED_NORMAL_GAMEPLAY_SMOKE_ONLY':issues.append('game truthfulness '+k)
 if metadata.get('source_qa',{}).get('status')!='PENDING_FINAL_EXACT_HF16_PACKAGE_8_OF_8':issues.append('no fake sealed QA')
 if 'No new HF16 Windows binary build or instrumented Consumer scenario completion is claimed.' not in document:issues.append('limitations disclaimer')
 return issues
ok('release identity',s.get('release_type')=='HOTFIX_RELEASE' and s.get('hotfix_ordinal')==16 and s.get('official_release_identity')==I)
ok('exact official parent',s.get('immediate_hotfix_parent',{}).get('sha256')==P)
ok('package filename',s.get('official_package_filename')==F and p.get('official_package_filename')==F)
ok('authorized',p.get('production_authorized') is True and 'hotfix' in p.get('publication_authorization',{}).get('text','').lower())
ok('no fake pointer',p.get('canonical_pointer_committed') is False and p.get('post_commit_verified') is False)
ok('acceptance binding',not validate_acceptance_document((r/'docs/RELEASE_ACCEPTANCE.md').read_text(),a,I))
ok('governance revision',g.get('master_revision')=='2026-10-15.1' and s.get('governance_master_revision')=='2026-10-15.1')
ok('governance normative mirror',hashlib.sha256((r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_bytes()).hexdigest()==g.get('normative_governance_mirror_sha256'))
ok('governance invariants mirror',hashlib.sha256((r/'metadata/PROJECT_GOVERNANCE_INVARIANTS.json').read_bytes()).hexdigest()==g.get('packaged_mirror',{}).get('json_sha256'))
ok('normative debt excluded','library_governance_debt' not in v and '## 20. Library-only governance debt' not in (r/'docs/PROJECT_GOVERNANCE_CONTRACT.md').read_text())
ok('lineage',l.get('release_identity')==I and bool(s.get('lineage_gap_disclosure')))
ok('seven-hook production',j('metadata/RELEASE_ARCHITECTURE.json').get('minhook',{}).get('detours')==7)
ok('canonical docs',all((r/n).read_text().startswith(x) for n,x in {'docs/BUILD_ID.md':'# BUILD ID — '+I,'docs/RELEASE_NOTES.md':'# '+I,'docs/RELEASE_PROVENANCE.md':'# Release provenance — '+I,'docs/RELEASE_ACCEPTANCE.md':'# '+I,'SOURCE_REVISION.md':'# SOURCE REVISION — '+I}.items()))
ok('resource binary compatibility',not any(x in (r/'resources/preloader_version.rc').read_text() for x in ('SQUALERIS','HOTFIX 16')))
if e:
 print('HOTFIX16_RELEASE_CONTRACT: FAIL',','.join(e));sys.exit(1)
print('HOTFIX16_RELEASE_CONTRACT: PASS')
