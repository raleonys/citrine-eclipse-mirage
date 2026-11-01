#!/usr/bin/env python3
"""Fail-closed contract: exact approved neutral-build / colored-final CMD operator UI."""
from pathlib import Path
import json,re,sys
root=Path(__file__).resolve().parents[1]
reg=json.loads((root/'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())
own=json.loads((root/'metadata/SCRIPT_OWNERSHIP.json').read_text())
errs=[]
if reg.get('mode')!='POSITIVE_DEFAULT_DENY':errs.append('registry not default-deny')
expected={p['launcher']:p['target'] for p in reg.get('registered_entrypoints',[])}
actual={p.relative_to(root).as_posix() for p in root.rglob('*.cmd')}
if actual!=set(expected):errs.append('launcher set mismatch')
human={x['path'] for x in own['scripts'] if x['human_invocable']}
if human!=set(expected.values()):errs.append('human entries mismatch')
DESCRIPTIONS={
 'build_production':'Build the shipping DLL and release artifacts',
 'build_unstable':'a local unsigned DLL without release gates',
 'build_native_acceptance':'Run native Windows acceptance scenarios',
 'build_crt_acceptance':'HF12 /MT shipping, Golden, native CRT acceptance',
 'build_debug':'Build debug-oriented helper and host artifacts',
}
for launch,target in expected.items():
 p=root/launch
 if not p.is_file():continue
 s=p.read_text(encoding='ascii');low=s.lower();stem=Path(launch).stem
 if Path(launch).parent!=Path(target).parent or stem!=Path(target).stem:errs.append('stem/path mismatch '+launch)
 if Path(target).name.lower() not in low:errs.append('incorrect target '+launch)
 for tok in ['%~dp0','%*','set "RC=%ERRORLEVEL%"','pause >nul','exit /b %RC%',
             'title preloader_l - %LABEL%',
             'Write-Host $line -ForegroundColor Green','Write-Host $line -ForegroundColor Red',
             'Exit code: %RC%', '%LABEL%: PASS', '%LABEL%: FAIL',
             'echo Press any key to close this operator window.',
             'echo ============================================================',
             DESCRIPTIONS[stem], 'echo Running: '+Path(target).name, 'powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%TARGET%" %*']:
  if tok.lower() not in low:errs.append('missing approved UI token '+tok+' in '+launch)
 for bad in ['color 0A','color 0C','color 0B','WARN','PARTIAL','echo  Output directory','echo  Expected artifacts / logs','launcher_common.cmd','build_minhook_vendor.cmd','explorer.exe']:
  if bad.lower() in low:errs.append('forbidden UI/launcher token '+bad+' in '+launch)
 if not (s.index('powershell.exe') < s.index('set "RC=%ERRORLEVEL%"') < s.index('Write-Host $line -ForegroundColor Green') < s.index('pause >nul') < s.index('exit /b %RC%')):
  errs.append('build-neutral/result ordering invalid '+launch)
 if re.search(r'(?im)^\s*color\s+[0-9a-f]{2}\s*$',s):errs.append('whole-console color mutation '+launch)
 if s.count('Write-Host $line -ForegroundColor Green')!=1 or s.count('Write-Host $line -ForegroundColor Red')!=1:errs.append('result-only color count mismatch '+launch)
 if not ('if errorlevel 1 (' in s and s.count('if errorlevel 1 (')==2):errs.append('monochrome fallback missing '+launch)
 if len(re.findall(r'^echo ={25,}$',s,re.M)) < 2:errs.append('visible separators missing '+launch)
 if s.count('echo Press any key to close this operator window.')!=1:errs.append('dismissal notice missing '+launch)
 if not s.rstrip().endswith('exit /b %RC%'):errs.append('nonterminal exact child RC propagation '+launch)
 if 'cls' in [a.strip().lower() for a in s.splitlines()]:errs.append('unexpected build screen clear '+launch)
for target in ('build_production.ps1','build_native_acceptance.ps1','build_crt_acceptance.ps1','build_debug.ps1','build_common.ps1'):
 p=root/'scripts/build_orchestration'/target
 if re.search(r'(?i)-ForegroundColor|-BackgroundColor',p.read_text(encoding='utf8')):errs.append('non-neutral intermediate build output '+target)
if any((root/'scripts').rglob('*.bat')):errs.append('parallel .bat found')
if errs:
 print('HUMAN_LAUNCHER_CONTRACT: FAIL');[print(' -',e) for e in errs];sys.exit(1)
print(f'HUMAN_LAUNCHER_CONTRACT: PASS ({len(expected)} launchers; PASS green, FAIL red only; original exit codes and pause)')
