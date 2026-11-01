# SPDX-License-Identifier: GPL-3.0-only
"""Real five-stage orchestration on synthetic, never-executed PE fixtures."""
import contextlib,hashlib,importlib.util,io,json,shutil,struct,subprocess,sys,tempfile
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts/qa'))
from _project import source_files,sha,save,source_digest
from run_golden import run_pipeline

def load(path):
 spec=importlib.util.spec_from_file_location('helper',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
def node(key,value=b'',children=(),text=False):
 if text:value=(value+'\0').encode('utf-16le');count=len(value)//2
 else:count=len(value)
 body=bytearray(struct.pack('<HHH',0,count,1 if text else 0)+(key+'\0').encode('utf-16le'))
 while len(body)%4:body.append(0)
 body+=value
 while len(body)%4:body.append(0)
 for child in children:
  body+=child
  while len(body)%4:body.append(0)
 struct.pack_into('<H',body,0,len(body));return body
def fixture(root):
 # Reuse the unchanged HF19 export/import/cookie fixture as a base. Its test
 # module only operates on temporary byte arrays; no PE is executed.
 with contextlib.redirect_stdout(io.StringIO()):base=load(root/'tools/golden/scripts/test_pe_metadata_tools.py')
 raw,_,opt=base.build_corvexon_fixture();raw.extend(bytes(0x3200-len(raw)))
 # The old ABI-only fixture's name overlapped its function table. Give the
 # complete metadata+ABI fixture its own non-overlapping export-name storage.
 struct.pack_into('<I',raw,0x400+0x100+12,0x1800)
 name=b'preloader.unsigned.dll\0';raw[0xc00:0xc00+len(name)]=name
 coff=0x104;section=opt+0xf0;struct.pack_into('<H',raw,coff+2,6)
 struct.pack_into('<II',raw,opt+32,0x1000,0x200);struct.pack_into('<I',raw,opt+60,0x400)
 struct.pack_into('<HH',raw,opt+40,6,0);struct.pack_into('<HH',raw,opt+48,6,0)
 struct.pack_into('<H',raw,opt+70,0x4160);struct.pack_into('<I',raw,opt+56,0x9000)
 struct.pack_into('<H',raw,coff+18,0x2022)
 names=['.text','.rdata','.data','.pdata','.rsrc','.reloc']
 geometry=[(0x2000,0x1000,0x2000,0x400),(0x200,0x4000,0x200,0x2400),(0x200,0x5000,0x200,0x2600),(0x200,0x6000,0x200,0x2800),(0x600,0x7000,0x600,0x2a00),(0x200,0x8000,0x200,0x3000)]
 for i,(name,shape) in enumerate(zip(names,geometry)):
  off=section+i*40;raw[off:off+8]=name.encode().ljust(8,b'\0');struct.pack_into('<IIII',raw,off+8,*shape);struct.pack_into('<I',raw,off+36,0xe0000020 if i==0 else 0x40000040)
 policy=json.loads((root/'tools/golden/metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())['required']['versioninfo']
 fixed=struct.pack('<13I',*[int(v,16) if isinstance(v,str) else v for v in policy['fixed'].values()])
 strings=[node(k,v,text=True) for k,v in policy['strings'].items()]
 version=node('VS_VERSION_INFO',fixed,[node('StringFileInfo',children=[node(policy['string_table'],children=strings)]),node('VarFileInfo',children=[node('Translation',struct.pack('<HH',int(policy['translation_language'],16),policy['translation_codepage']))])])
 rb=0x2a00
 for offset,ident,target in [(0,16,0x80000018),(24,1,0x80000030),(48,0x409,72)]:
  struct.pack_into('<IIHHHH',raw,rb+offset,0,0,0,0,0,1);struct.pack_into('<II',raw,rb+offset+16,ident,target)
 struct.pack_into('<IIII',raw,rb+72,0x7060,len(version),1200,0);raw[rb+96:rb+96+len(version)]=version
 struct.pack_into('<II',raw,opt+112+16,0x7000,0x600)
 struct.pack_into('<I',raw,opt+64,0);return raw
def copy_source(target):
 for p in source_files(ROOT):
  q=target/p.relative_to(ROOT);q.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,q)
 save(target/'metadata/SOURCE_MANIFEST.json',{'source_digest':source_digest(target),'files':{p.relative_to(target).as_posix():{'sha256':sha(p),'bytes':p.stat().st_size} for p in source_files(target)}})

def main():
 cases=0
 with tempfile.TemporaryDirectory(prefix='preloader_l_golden_') as temp:
  root=Path(temp)/'source with spaces';copy_source(root);build=root/'build/production/msvc';build.mkdir(parents=True)
  rule=build/'CMakeFiles/preloader_l.dir/link.txt';rule.parent.mkdir(parents=True)
  rule.write_text('link.exe /DLL /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO /OUT:preloader_l.dll fixture.obj\n')
  dll=build/'preloader_l.dll';output=root/'dist/production/preloader_l.dll';report=root/'build/reports/golden.json';good=fixture(root)
  def run(raw=good,runner=None):
   dll.write_bytes(raw)
   with contextlib.redirect_stdout(io.StringIO()):rc=run_pipeline(root,dll,build,output,report,runner)
   return rc,json.loads(report.read_text())
  rc,data=run();assert rc==0 and data['published'] and [s['name'] for s in data['stages']]==['URSYNEX','ARYRA','MUSTAVEX','CORVEXON','FALCION'],data;cases+=1
  assert dll.read_bytes()==good and sha(output)==data['output_sha256'] and sha(output)!=sha(dll);cases+=1
  assert all(s['sha256_before']==s['sha256_after'] for s in data['stages'][1:]);cases+=1
  accepted=output.read_bytes()
  # Every stage is reached through real subprocesses; these are failures of
  # the actual validators rather than fake PASS/FAIL executables.
  bad=bytearray(good);bad[0]=0;rc,data=run(bad);assert rc and data['stages'][-1]['name']=='URSYNEX' and output.read_bytes()==accepted;cases+=1
  bad=bytearray(good);bad[0x208:0x210]=b'.wrong\0\0';rc,data=run(bad);assert rc and data['stages'][-1]['name']=='ARYRA' and not data['published'];cases+=1
  bad=bytearray(good);needle='EA Javelin Anticheat'.encode('utf-16le');at=bad.find(needle);assert at>=0;bad[at]=ord('X');rc,data=run(bad);assert rc and data['stages'][-1]['name']=='MUSTAVEX';cases+=1
  bad=bytearray(good);struct.pack_into('<H',bad,0x400+0x1b0+2,2);rc,data=run(bad);assert rc and data['stages'][-1]['name']=='CORVEXON';cases+=1
  rule.write_text('link.exe /DEBUG:FULL fixture.obj');rc,data=run();assert rc and data['stages'][-1]['name']=='CORVEXON';cases+=1
  rule.write_text('link.exe /DLL /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO fixture.obj')
  stray=root/'src/unlisted.txt';stray.write_text('unlisted');rc,data=run();stray.unlink();assert rc and data['stages'][-1]['name']=='FALCION';cases+=1
  def mutate(name,command,staged):
   proc=subprocess.run(command,cwd=root,capture_output=True,text=True)
   if name=='ARYRA' and proc.returncode==0:staged.write_bytes(staged.read_bytes()+b'x')
   return proc
  rc,data=run(runner=mutate);assert rc and data['stages'][-1]['error']=='read-only Golden gate changed DLL bytes' and output.read_bytes()==accepted;cases+=1
  def pdb(name,command,staged):
   proc=subprocess.run(command,cwd=root,capture_output=True,text=True)
   if name=='CORVEXON' and proc.returncode==0:(staged.parent/'debug.pdb').write_bytes(b'not allowed')
   return proc
  rc,data=run(runner=pdb);assert rc and data['stages'][-1]['name']=='FALCION';cases+=1
  assert output.read_bytes()==accepted;cases+=1
  with contextlib.redirect_stdout(io.StringIO()):rc=run_pipeline(root,dll,build,root.parent/'outside.dll',report)
  assert rc and not (root.parent/'outside.dll').exists();cases+=1
  # Prove the hygiene scan itself, with a correctly resealed synthetic source
  # fixture, rather than relying only on an earlier hash-mismatch failure.
  secret=root/'src/fake_secret.txt';secret.write_text('-----BEGIN '+'PRIVATE KEY-----\nSYNTHETIC\n')
  save(root/'metadata/SOURCE_MANIFEST.json',{'files':{p.relative_to(root).as_posix():{'sha256':sha(p),'bytes':p.stat().st_size} for p in source_files(root)}})
  rc,data=run();assert rc and data['stages'][-1]['name']=='FALCION' and 'sensitive material private-key' in data['error'];cases+=1
  secret.unlink()
 print('PASS real five-stage Golden orchestration SYNTHETIC cases='+str(cases)+'; Windows DLL execution NOT_RUN');return 0
if __name__=='__main__':sys.exit(main())
