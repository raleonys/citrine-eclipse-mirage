#!/usr/bin/env python3
"""Deterministic file-byte source ZIP packer; no external file dependencies."""
import pathlib,zipfile,json,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
def seal(dst):
 m=json.loads((ROOT/'metadata/manifest.json').read_text())['files']
 names=sorted(list(m)+['metadata/manifest.json'])
 with zipfile.ZipFile(dst,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for name in names:
   info=zipfile.ZipInfo(name,date_time=(1980,1,1,0,0,0))
   info.compress_type=zipfile.ZIP_DEFLATED
   info.external_attr=(0o100644)<<16
   z.writestr(info,(ROOT/name).read_bytes(),compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)
 return len(names)
if __name__=='__main__':
 from verify_gf12 import verify
 assert len(sys.argv)==2, 'usage repack_gf12.py <absolute-output.zip>'
 print('GF12_REPACK',seal(pathlib.Path(sys.argv[1])),'MEMBERS')
