#!/usr/bin/env python3
"""Byte-reproducible GF14 source ZIP, builds only from manifest-verified tree."""
import pathlib,sys,zipfile
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"qa"))
from check_source_provenance import verify
ROOT=pathlib.Path(__file__).resolve().parents[2]
if len(sys.argv)!=2:raise SystemExit('USAGE: repack_gf14.py DESTINATION_ZIP')
verify(ROOT)
dst=pathlib.Path(sys.argv[1]);dst.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(dst,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for p in sorted(ROOT.rglob('*')):
  if not p.is_file():continue
  n=p.relative_to(ROOT).as_posix()
  info=zipfile.ZipInfo(n,date_time=(2026,10,20,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o100644<<16
  z.writestr(info,p.read_bytes(),compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)
print('GF14_DETERMINISTIC_SOURCE_ZIP',dst)
