#!/usr/bin/env python3
"""Actual ELF archive symbol definition comparison. NOT Windows or final-link acceptance."""
import argparse,subprocess,pathlib,json,hashlib,sys,collections
ROOT=pathlib.Path(__file__).resolve().parents[1]
PREV='libgf11_exchange_units.a'; NEW='libgf12_exchange_transitive_units.a'
def dem(x):return subprocess.run(['c++filt',x],text=True,capture_output=True,check=True).stdout.strip()
def scan(p):
 d=set();u=set();out=subprocess.run(['nm','-P','-g',str(p)],text=True,capture_output=True,check=True).stdout
 for line in out.splitlines():
  x=line.split()
  if len(x)<2 or not x[0].startswith('_ZN11preloader_l'):continue
  if x[1]=='U':u.add(x[0])
  elif x[1] in 'TWDBVR':d.add(x[0])
 return d,u
def audit(build):
 files=list(build.glob('lib*.a'))
 if not all((build/n).is_file() for n in (PREV,NEW)):raise ValueError('ARCHIVE_ABSENT')
 before,unused=scan(build/PREV); added,more=scan(build/NEW)
 expected=json.loads((ROOT/'metadata/GF11_SYMBOL_CLOSURE.json').read_text())['new_socket_exchange_unowned_transitive_symbols']
 result={dem(x) for x in added}
 found=sorted(x for x in expected if x in result)
 notfound=sorted(x for x in expected if x not in result)
 if len(expected)!=12 or len(found)!=12 or notfound:raise ValueError('GF11_12_SYMBOLS_NOT_CLOSED')
 defs=set();us=set()
 for p in files:
  d,u=scan(p);defs|=d;us|=u
 missing=sorted(dem(x) for x in us-defs)
 return {'schema':'greenfield.gf12.actual-elf-symbol-audit.v1','archived_libraries':len(files),'gf11_transitive_original_count':12,'gf12_defined_from_actual_nm':found,'gf11_original_missing_after_gf12':notfound,'all_present_archive_project_symbol_gaps':missing,'gf12_elf_archive_sha256':hashlib.sha256((build/NEW).read_bytes()).hexdigest(),'full_executable_link':'NOT_RUN','windows_native':'NOT_RUN','meaning':'STATIC_DEFINITIONS_ONLY'}
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--build',required=True,type=pathlib.Path);ap.add_argument('--output',type=pathlib.Path);args=ap.parse_args()
 try:
  v=audit(args.build)
  if args.output:args.output.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
  print('GF12_ELF_SOURCE_OWNER_PASS 12/12; OTHER_GAPS',len(v['all_present_archive_project_symbol_gaps']),'NOT_FULL_LINK')
 except Exception as e:print('GF12_ELF_AUDIT_FAIL',e);sys.exit(1)
