# SPDX-License-Identifier: GPL-3.0-only
"""FALCION source integrity and flat DLL payload gate for LOCAL_BUILD."""
import argparse,hashlib,json,re,sys
from pathlib import Path
sys.dont_write_bytecode=True

SUFFIXES={'.md','.txt','.json','.csv','.py','.ps1','.cmd','.cmake','.cpp','.c','.hpp','.h','.rc','.def','.xml','.ini','.cfg','.log'}
RULES=[
 ('private-key',re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----')),
 ('aws-access-key',re.compile(r'(?<![A-Z0-9])AKIA[0-9A-Z]{16}(?![A-Z0-9])')),
 ('github-token',re.compile(r'(?<![A-Za-z0-9_])gh[pousr]_[A-Za-z0-9_]{24,}(?![A-Za-z0-9_])')),
 ('bearer-token',re.compile(r'(?i)\bBearer\s+[A-Za-z0-9._~+/-]{24,}={0,2}')),
 ('jwt',re.compile(r'\beyJ[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\.[A-Za-z0-9_-]{12,}\b')),
 ('windows-user-path',re.compile(r'(?i)\b[A-Z]:\\Users\\(?!USER(?:NAME)?\\|Public\\|Default\\|example\\|test\\|dummy\\)[^\\\r\n\t ]+\\')),
 ('unix-user-path',re.compile(r'(?<![A-Za-z0-9_])/(?:home|Users)/(?!USER(?:NAME)?/|Public/|Default/|example/|test/|dummy/)[^/\r\n\t ]+/')),
]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def inspect(root,payload):
 root=Path(root).resolve();payload=Path(payload).resolve();errors=[]
 policy=json.loads((root/'metadata/FALCION_GOLDEN_CLEANING.json').read_text())
 project=json.loads((root/'metadata/PROJECT.json').read_text())
 if project.get('classification')!='NON_RELEASE_WORK' or project.get('release_status')!='NOT_RELEASE':errors.append('this source profile is LOCAL_BUILD/NON_RELEASE_WORK only')
 files={p.name for p in root.iterdir() if p.is_file()};dirs={p.name for p in root.iterdir() if p.is_dir()}
 if files!=set(policy['source_root_files']):errors.append('source root file allowlist mismatch')
 allowed=set(policy['source_root_directories'])|set(policy['transient_root_directories'])
 if dirs-allowed:errors.append('unexpected source root directory')
 if not set(policy['source_root_directories'])<=dirs:errors.append('missing source root directory')
 manifest=json.loads((root/'metadata/SOURCE_MANIFEST.json').read_text())['files']
 derived=set(policy['derived_metadata_files']);transient=set(policy['transient_root_directories'])
 actual={}
 for p in root.rglob('*'):
  rel=p.relative_to(root)
  if rel.parts[0] in transient or '__pycache__' in rel.parts:continue
  if p.is_symlink():errors.append('source symlink: '+rel.as_posix());continue
  if not p.is_file():continue
  if rel.parts[0]=='evidence':continue # NON_RELEASE audit evidence is never copied into DLL payload.
  if p.name in derived:continue
  actual[rel.as_posix()]=p
 if {n.casefold() for n in actual}.__len__()!=len(actual):errors.append('case-fold source collision')
 if set(actual)!=set(manifest):errors.append('source manifest membership mismatch')
 for name,p in actual.items():
  expected=manifest.get(name,{})
  if p.stat().st_size!=expected.get('bytes') or digest(p)!=expected.get('sha256'):errors.append('source manifest hash mismatch: '+name)
  if p.suffix.lower() in policy['forbidden_source_suffixes']:errors.append('forbidden generated/source payload: '+name)
  if p.suffix.lower() in SUFFIXES and not name.startswith('third_party/'):
   text=p.read_text(encoding='utf-8',errors='replace')
   for label,rx in RULES:
    if rx.search(text):errors.append('sensitive material '+label+': '+name)
 if not payload.is_dir():errors.append('payload directory missing');return errors
 entries=list(payload.iterdir())
 if {p.name for p in entries}!=set(policy['payload_files']):errors.append('flat payload allowlist mismatch')
 for p in entries:
  if p.is_symlink() or not p.is_file():errors.append('payload is not a regular flat file: '+p.name)
 return errors
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--root',type=Path,required=True);ap.add_argument('--payload',type=Path,required=True);a=ap.parse_args()
 try:errors=inspect(a.root,a.payload)
 except (OSError,ValueError,KeyError,TypeError) as error:errors=[str(error)]
 print('FALCION_GOLDEN_CLEANING: '+('FAIL' if errors else 'PASS')+' (LOCAL_BUILD source + flat DLL payload)')
 for error in errors:print(' - '+error)
 return 1 if errors else 0
if __name__=='__main__':sys.exit(main())
