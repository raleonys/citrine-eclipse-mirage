#!/usr/bin/env python3
"""Approved HF06 human UI negative mutation suite; no Windows runtime claimed."""
from pathlib import Path
import tempfile,shutil,subprocess,sys
root=Path(__file__).resolve().parents[1]; checker=root/'scripts/check_human_launcher_contract.py'
assert subprocess.run([sys.executable,str(checker)],capture_output=True).returncode==0
with tempfile.TemporaryDirectory() as tmp:
 t=Path(tmp)
 for rel in ['metadata/HUMAN_ENTRYPOINT_REGISTRY.json','metadata/SCRIPT_OWNERSHIP.json','scripts/check_human_launcher_contract.py']:
  f=t/rel;f.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(root/rel,f)
 for p in (root/'scripts/build_orchestration').glob('*.cmd'):
  f=t/'scripts/build_orchestration'/p.name;f.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,f)
 p=t/'scripts/build_orchestration/build_production.cmd';src=p.read_bytes()
 newline=b'\r\n' if b'\r\n' in src else b'\n'
 mutations=[
  lambda b:b.replace(b'-ForegroundColor Green',b'-ForegroundColor Yellow',1),
  lambda b:b.replace(b'-ForegroundColor Red',b'-ForegroundColor Yellow',1),
  lambda b:b.replace(b'echo  Exit code: %RC%',b'echo  Exit code missing'),
  lambda b:b.replace(b'pause >nul',b'echo missing pause',1),
  lambda b:b.replace(b'%*',b'MISSING_ARG_FORWARD',1),
  lambda b:b.replace(b'Build the shipping DLL and release artifacts',b'BAD DESCRIPTION',1),
  lambda b:b.replace(b'exit /b %RC%',b'exit /b 0',1),
  lambda b:b.replace(b'Running: build_production.ps1',b'Running: broken.ps1',1),
  lambda b:b.replace(b'echo  %LABEL%: PASS',b'echo  PASS HIDDEN',1),
  lambda b:b.replace(b'echo  %LABEL%: FAIL',b'echo  FAIL HIDDEN',1),
  lambda b:b.replace(b'echo.'+newline,b'color 0A'+newline+b'echo.'+newline,1),
  lambda b:b.replace(b'echo.'+newline,b'echo.'+newline+b'echo  Output directory (if produced): guessed'+newline,1),
 ]
 for i,mutation in enumerate(mutations):
  m=mutation(src);assert m!=src,i;p.write_bytes(m)
  run=subprocess.run([sys.executable,str(t/'scripts/check_human_launcher_contract.py')],capture_output=True)
  assert run.returncode!=0,(i,run.stdout,run.stderr)
  p.write_bytes(src)
print('HUMAN_LAUNCHER_UI_NEGATIVE: PASS (12/12 mutations rejected)')
