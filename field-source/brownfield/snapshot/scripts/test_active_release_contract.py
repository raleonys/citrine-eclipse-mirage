#!/usr/bin/env python3
"""Execute source-only contract positives and mutation negatives without a G0 waiver."""
from pathlib import Path
import copy, hashlib, importlib.util, json, re, shutil, sys, tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('active_release_contract', ROOT/'scripts/check_active_release_contract.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

FILES = (
 'RELEASE_STATE.json',
 'metadata/RELEASE_AUTHORIZATION.json',
 'metadata/PUBLICATION_TRANSACTION.json',
 'metadata/GOVERNANCE_BINDING.json',
 'metadata/ARTIFACT_PROVENANCE.json',
 'metadata/PROJECT_GOVERNANCE_INVARIANTS.json',
 'metadata/RELEASE_ACCEPTANCE.json',
 'metadata/ACCEPTANCE_FRESHNESS_LEDGER.json',
 'metadata/RELEASE_ARCHITECTURE.json',
 'docs/PROJECT_GOVERNANCE_CONTRACT.md',
 'README.md', 'docs/BUILD_ID.md', 'docs/RELEASE_NOTES.md', 'docs/RELEASE_ACCEPTANCE.md',
)

def write(obj, root, name):
 (root/name).write_text(json.dumps(obj,indent=2,sort_keys=True,ensure_ascii=False)+'\n',encoding='utf-8')

def load(root, name): return json.loads((root/name).read_text(encoding='utf-8'))

def mutate(root, name, fn):
 j=load(root,name);fn(j);write(j,root,name)

def prepare(root):
 for item in FILES:
  p=root/item;p.parent.mkdir(parents=True, exist_ok=True)
  shutil.copy2(ROOT/item,p)

def to_future(root):
 # Prove release-agnostic logic by converting the fixture's identity/parent
 # coherently. No source-gate code or baked SHA pins are changed.
 src=load(root,'RELEASE_STATE.json')
 current=src['official_release_identity']
 match=module.IDENTITY.fullmatch(current)
 if match is None:raise RuntimeError('invalid source identity')
 ordinal=int(match.group('ordinal'))
 future=current[:match.start('ordinal')]+str(ordinal+1)
 parent=current
 current_file=src['official_package_filename']
 future_file=f"{match.group('product')}_{match.group('version')}_{match.group('codename')}_HOTFIX_{ordinal+1}.zip"
 fake_sha='a'*64
 for name in FILES:
  if not (name.endswith('.json') or name in ('README.md','docs/BUILD_ID.md','docs/RELEASE_NOTES.md','docs/RELEASE_ACCEPTANCE.md')):continue
  txt=(root/name).read_text(encoding='utf-8')
  # Convert visible identity and canonical filename in source fixture.
  txt=txt.replace(current,future).replace(current_file,future_file)
  (root/name).write_text(txt,encoding='utf-8')
 s=load(root,'RELEASE_STATE.json');s['hotfix_ordinal']=ordinal+1;s['immediate_hotfix_parent']={'release':parent,'sha256':fake_sha};write(s,root,'RELEASE_STATE.json')
 a=load(root,'metadata/RELEASE_AUTHORIZATION.json');a['requested_ordinal']=ordinal+1;write(a,root,'metadata/RELEASE_AUTHORIZATION.json')
 p=load(root,'metadata/PUBLICATION_TRANSACTION.json');p['canonical_before']={'identity':parent,'sha256':fake_sha};write(p,root,'metadata/PUBLICATION_TRANSACTION.json')
 art=load(root,'metadata/ARTIFACT_PROVENANCE.json');art['immediate_parent']={'class':'HOTFIX_RELEASE','identity':parent,'sha256':fake_sha};write(art,root,'metadata/ARTIFACT_PROVENANCE.json')
 # A future release has a new exact mirror SHA, derived from that fixture's bytes.
 g=load(root,'metadata/GOVERNANCE_BINDING.json')
 g['packaged_mirror']['json_sha256']=hashlib.sha256((root/'metadata/PROJECT_GOVERNANCE_INVARIANTS.json').read_bytes()).hexdigest()
 write(g,root,'metadata/GOVERNANCE_BINDING.json')
 # Synthetic future metadata is not an independent authenticated G0 fixture.

checks=[]
def check(label, passed):
 checks.append((label, bool(passed)))
 if not passed:print('FAIL',label)

with tempfile.TemporaryDirectory(prefix='active_release_contract_') as td:
 root=Path(td)
 prepare(root)
 check('current-source-structure', not module.validate(root))
 checks_mutations=[
  ('identity-mismatch','metadata/RELEASE_ACCEPTANCE.json',lambda x:x.__setitem__('official_release_identity','INCORRECT')),
  ('ordinal-mismatch','RELEASE_STATE.json',lambda x:x.__setitem__('hotfix_ordinal',999)),
  ('parent-sha-mismatch','metadata/ARTIFACT_PROVENANCE.json',lambda x:x['immediate_parent'].__setitem__('sha256','f'*64)),
  ('pointer-parent-mismatch','metadata/PUBLICATION_TRANSACTION.json',lambda x:x['canonical_before'].__setitem__('sha256','f'*64)),
  ('false-commit','metadata/PUBLICATION_TRANSACTION.json',lambda x:x.__setitem__('canonical_pointer_committed',True)),
  ('false-g4-acceptance','metadata/RELEASE_ACCEPTANCE.json',lambda x:x['source_qa'].__setitem__('status','PASS_FAKE_G4')),
  ('false-game-acceptance','metadata/RELEASE_ACCEPTANCE.json',lambda x:x['madden27_game_network'].__setitem__('status','PASS_FAKE_GAME')),
  ('false-acceptance-ledger','metadata/ACCEPTANCE_FRESHNESS_LEDGER.json',lambda x:x['entries'][0].__setitem__('status','PASS_FRESH')),
  ('bad-governance-pin','metadata/GOVERNANCE_BINDING.json',lambda x:x.__setitem__('library_master_json_sha256','f'*64)),
  ('invalid-g3','metadata/RELEASE_ACCEPTANCE.json',lambda x:x['release_class_g3'].__setitem__('status','PASS_FAKE_G3')),
  ('bad-official-filename','RELEASE_STATE.json',lambda x:x.__setitem__('official_package_filename','BAD.zip')),
 ]
 for label, path, fn in checks_mutations:
  mutate(root,path,fn)
  check('reject-'+label,bool(module.validate(root)))
  shutil.copy2(ROOT/path,root/path)
 to_future(root)
 check('synthetic-future-hotfix-no-code-change',not module.validate(root))
 mutate(root,'RELEASE_STATE.json',lambda x:x.__setitem__('hotfix_ordinal',int(x['hotfix_ordinal'])+1))
 check('reject-future-ordinal-mismatch',bool(module.validate(root)))

failed=[k for k,v in checks if not v]
print(f'ACTIVE_RELEASE_CONTRACT_REGRESSION: {len(checks)-len(failed)}/{len(checks)}', 'PASS' if not failed else 'FAIL')
print('G0/G2/G4: NOT_EXECUTED, NO_PUBLICATION')
sys.exit(bool(failed))
