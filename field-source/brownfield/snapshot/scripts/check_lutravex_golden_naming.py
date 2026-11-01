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
for v in set(release_h2):
    if release_h2.count(v)!=1: errors.append('duplicate semantic release '+v)
if errors:
    print('LUTRAVEX_GOLDEN_NAMING: FAIL')
    [print(' -',e) for e in errors]
    sys.exit(1)
print(f'LUTRAVEX_GOLDEN_NAMING: PASS ({len(hist)} historical reserved roots + SQUALERIS current assignment)')
