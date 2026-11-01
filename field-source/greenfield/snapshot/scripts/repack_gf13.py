#!/usr/bin/env python3
"""Deterministic GF13 source ZIP builder: no generated files or directory entries."""
from pathlib import Path
import zipfile,sys
ROOT=Path(__file__).resolve().parents[1]
if len(sys.argv)!=2:raise SystemExit('USAGE: repack_gf13.py output.zip')
out=Path(sys.argv[1]);out.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=9,allowZip64=True) as z:
 for p in sorted(ROOT.rglob('*')):
  if not p.is_file():continue
  n=p.relative_to(ROOT).as_posix()
  if '__pycache__' in n or n.endswith('.pyc'):raise ValueError('UNTRACKED_CACHE_FILE')
  info=zipfile.ZipInfo(n,date_time=(2026,10,20,0,0,0))
  info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o100644<<16
  z.writestr(info,p.read_bytes(),compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)
print('GF13_REPACK_CREATED',out)
