#!/usr/bin/env python3
from pathlib import Path
import json,re,sys
r=Path(__file__).resolve().parents[1]
p=json.loads((r/'metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json').read_text())
errors=[]
hist=p.get('historical_reserved_roots',[])
if 'SQUALERIS' in hist: errors.append('current root collides with historical set')
if 'RANQUEL' not in hist: errors.append('RANQUEL missing from historical set')
if p.get('current_assigned_roots')!=['SQUALERIS']: errors.append('current assignment')
if p.get('current_release')!={'version':'3.2.5','codename':'SQUALERIS','status':'ASSIGNED_CURRENT_RELEASE'}: errors.append('current release')
text=(r/'CHANGELOG.md').read_text()
blocks=re.findall(r'## Machine-readable reserved namespace\n.*?```text\n(.*?)\n```',text,re.S)
if len(blocks)!=1: errors.append('machine-readable namespace block count != 1')
else:
    roots=[x.strip() for x in blocks[0].splitlines() if x.strip()]
    expected=sorted(hist+['SQUALERIS'])
    if roots!=expected: errors.append('CHANGELOG/JSON reserved namespace drift')
for token in ('## 3.2.5 SQUALERIS','## 3.2.4 RANQUEL','## 3.2.3 MELOTRIX','## 3.2.2 MUSTORIN','## 3.2.1 CERVRAX','## 3.2.0 SCIURONYX'):
    if token not in text: errors.append('CHANGELOG missing '+token)
release_h2=re.findall(r'^## ([0-9]+\.[0-9]+\.[0-9]+)\b.*$',text,re.M)
# 3.2.5 is intentionally a single approved semantic root with numbered hotfix
# headings. Reject duplicate base entries or duplicate hotfix ordinals.
for v in set(release_h2):
    hs=re.findall(r'^## '+re.escape(v)+r'\b([^\n]*)$',text,re.M)
    if v=='3.2.5':
        base=[h for h in hs if h.strip()=='SQUALERIS']
        hotfix=[h.strip() for h in hs if re.fullmatch(r'\s*SQUALERIS\s+HOTFIX\s+\d+(?:\s+—\s+SOURCE PREPARATION, NOT_RELEASE(?:\s+\(\d{4}-\d{2}-\d{2}\))?)?\s*',h)]
        nums=[int(re.search(r'HOTFIX\s+(\d+)',h).group(1)) for h in hotfix]
        if len(base)!=1 or len(nums)!=len(set(nums)) or not {17,18}.issubset(set(nums)) or len(base)+len(hotfix)!=len(hs):
            errors.append('3.2.5 semantic root / hotfix ordinal duplicate or malformed')
    elif len(hs)!=1:
        errors.append('duplicate semantic release '+v)
if errors:
    print('HF17_LUTRAVEX_GOLDEN_NAMING: FAIL')
    [print(' -',e) for e in errors]
    sys.exit(1)
print(f'HF17_LUTRAVEX_GOLDEN_NAMING: PASS ({len(hist)} historical reserved roots + SQUALERIS current assignment)')
