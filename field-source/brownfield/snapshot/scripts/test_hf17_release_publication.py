#!/usr/bin/env python3
from pathlib import Path
import hashlib, importlib.util, json, shutil, subprocess, sys, tempfile, zipfile
sys.dont_write_bytecode = True
ROOT=Path(__file__).resolve().parents[1]

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    mod=importlib.util.module_from_spec(spec); spec.loader.exec_module(mod); return mod

def run(cmd):
    return subprocess.run(cmd,text=True,capture_output=True)

def require_controlled(cp,prefix):
    text=(cp.stdout or '')+(cp.stderr or '')
    assert cp.returncode==1,(cp.returncode,text)
    assert prefix in text,text
    assert 'Traceback (most recent call last)' not in text,text

fin=load('hf03_fin',ROOT/'scripts/ursynex_golden_finalizer.py')
build=(ROOT/'scripts/build_orchestration/build_production.ps1').read_text(encoding='utf-8',errors='replace')

# S0104: same-directory staged replacement must preserve the original on a
# synthetic partial-write failure and must remove temporary staging material.
with tempfile.TemporaryDirectory(prefix='hf03_atomic_') as td_s:
    td=Path(td_s); target=td/'candidate.dll'; original=b'original-candidate-bytes'; target.write_bytes(original)
    real_write=fin._write_file_fully
    def fail_after_partial(handle,data):
        handle.write(data[:max(1,len(data)//2)])
        handle.flush()
        raise OSError('synthetic write failure')
    fin._write_file_fully=fail_after_partial
    try:
        try:
            fin.atomic_replace_bytes(target,b'replacement-candidate-bytes')
            raise AssertionError('atomic_replace_bytes unexpectedly succeeded')
        except OSError as e:
            assert 'synthetic write failure' in str(e)
    finally:
        fin._write_file_fully=real_write
    assert target.read_bytes()==original
    assert not list(td.glob('.candidate.dll.*.tmp')),list(td.iterdir())
    fin.atomic_replace_bytes(target,b'replacement-candidate-bytes')
    assert target.read_bytes()==b'replacement-candidate-bytes'
    assert not list(td.glob('.candidate.dll.*.tmp'))

# S0116: the post-finalizer identity is the single authority used before/after
# every read-only Golden gate and again for staged/final publication.
required=[
    'function Assert-CandidateIdentity',
    '$ValidatedCandidateHash = $UrsynexAfter',
    "'post-URSYNEX validated candidate'",
    "'ARYRA precondition'", "'ARYRA postcondition'",
    "'MUSTAVEX precondition'", "'MUSTAVEX postcondition'",
    "'CORVEXON precondition'", "'CORVEXON postcondition'",
    "'FALCION precondition'", "'FALCION postcondition'",
    "'publication source'",
    'Published DLL is not the validated Golden candidate identity.',
]
for token in required: assert token in build,token
assert build.count('Assert-CandidateIdentity $Dll $ValidatedCandidateHash') >= 10
assert build.index('$ValidatedCandidateHash = $UrsynexAfter') < build.index("'ARYRA precondition'")
assert build.index("'ARYRA postcondition'") < build.index("'MUSTAVEX precondition'")
assert build.index("'MUSTAVEX postcondition'") < build.index("'CORVEXON precondition'")
assert build.index("'CORVEXON postcondition'") < build.index("'FALCION precondition'")
assert build.index("'FALCION postcondition'") < build.index("'publication source'")

# Ordinary-data identity continuity model: mutation between gates must be
# distinguishable from the validated identity rather than silently accepted.
with tempfile.TemporaryDirectory(prefix='hf03_identity_') as td_s:
    p=Path(td_s)/'candidate.bin'; p.write_bytes(b'validated')
    expected=hashlib.sha256(p.read_bytes()).hexdigest()
    assert hashlib.sha256(p.read_bytes()).hexdigest()==expected
    p.write_bytes(b'drifted')
    assert hashlib.sha256(p.read_bytes()).hexdigest()!=expected

# S0120: missing input must be a controlled diagnostic, never a Python traceback.
with tempfile.TemporaryDirectory(prefix='hf03_cli_') as td_s:
    td=Path(td_s); missing=td/'missing.dll'; build_dir=td/'build'; build_dir.mkdir()
    require_controlled(run([sys.executable,str(ROOT/'scripts/ursynex_golden_finalizer.py'),str(missing)]),'URSYNEX_GOLDEN_FINALIZER: FAIL:')
    require_controlled(run([sys.executable,str(ROOT/'scripts/verify_aryra_golden_layout.py'),str(missing)]),'ARYRA_GOLDEN_LAYOUT: FAIL:')
    require_controlled(run([sys.executable,str(ROOT/'scripts/verify_corvexon_golden_trail.py'),str(missing),'--build-dir',str(build_dir)]),'CORVEXON_GOLDEN_TRAIL: FAIL')

# Missing/invalid policy files are likewise controlled. Run isolated copies so
# the authoritative repository metadata is never mutated by the negative test.
with tempfile.TemporaryDirectory(prefix='hf03_policy_') as td_s:
    td=Path(td_s); scripts=td/'scripts'; metadata=td/'metadata'; scripts.mkdir(); metadata.mkdir()
    dummy=td/'dummy.dll'; dummy.write_bytes(b'unchanged')

    shutil.copy2(ROOT/'scripts/ursynex_golden_finalizer.py',scripts/'ursynex_golden_finalizer.py')
    shutil.copy2(ROOT/'metadata/MUSTAVEX_GOLDEN_METADATA.json',metadata/'MUSTAVEX_GOLDEN_METADATA.json')
    (metadata/'URSYNEX_GOLDEN_FINALIZER.json').write_text('{ invalid json',encoding='utf-8')
    cp=run([sys.executable,str(scripts/'ursynex_golden_finalizer.py'),str(dummy)])
    require_controlled(cp,'URSYNEX_GOLDEN_FINALIZER: FAIL:')
    assert dummy.read_bytes()==b'unchanged'

with tempfile.TemporaryDirectory(prefix='hf03_policy_aryra_') as td_s:
    td=Path(td_s); scripts=td/'scripts'; metadata=td/'metadata'; scripts.mkdir(); metadata.mkdir()
    dummy=td/'dummy.dll'; dummy.write_bytes(b'unchanged')
    shutil.copy2(ROOT/'scripts/verify_aryra_golden_layout.py',scripts/'verify_aryra_golden_layout.py')
    cp=run([sys.executable,str(scripts/'verify_aryra_golden_layout.py'),str(dummy)])
    require_controlled(cp,'ARYRA_GOLDEN_LAYOUT: FAIL:')
    assert dummy.read_bytes()==b'unchanged'

with tempfile.TemporaryDirectory(prefix='hf03_policy_corvexon_') as td_s:
    td=Path(td_s); scripts=td/'scripts'; metadata=td/'metadata'; scripts.mkdir(); metadata.mkdir()
    dummy=td/'dummy.dll'; dummy.write_bytes(b'unchanged'); build_dir=td/'build'; build_dir.mkdir()
    shutil.copy2(ROOT/'scripts/verify_corvexon_golden_trail.py',scripts/'verify_corvexon_golden_trail.py')
    (metadata/'CORVEXON_GOLDEN_TRAIL.json').write_text('{ invalid json',encoding='utf-8')
    cp=run([sys.executable,str(scripts/'verify_corvexon_golden_trail.py'),str(dummy),'--build-dir',str(build_dir)])
    require_controlled(cp,'CORVEXON_GOLDEN_TRAIL: FAIL')
    assert dummy.read_bytes()==b'unchanged'

def refresh_manifest(root):
    mp=root/'metadata/ACTIVE_SOURCE_MANIFEST.json'
    data=json.loads(mp.read_text())
    files={}
    for q in sorted(root.rglob('*')):
        if not q.is_file(): continue
        rel=q.relative_to(root).as_posix()
        if rel=='metadata/ACTIVE_SOURCE_MANIFEST.json' or rel.startswith(('build/','dist/')): continue
        files[rel]=hashlib.sha256(q.read_bytes()).hexdigest()
    data['files']=files
    mp.write_text(json.dumps(data,indent=2,sort_keys=True)+'\n',encoding='utf-8')

def make_flat_zip(root,out):
    with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
        for q in sorted(root.rglob('*')):
            if q.is_file() and not q.relative_to(root).as_posix().startswith(('build/','dist/')):
                z.write(q,q.relative_to(root).as_posix())

# S0127: a nested archive cannot be made acceptable merely by rebinding the active manifest.
with tempfile.TemporaryDirectory(prefix='hf04_falcion_archive_') as td_s:
    td=Path(td_s); source=td/'source'; shutil.copytree(ROOT,source,ignore=shutil.ignore_patterns('build','dist','__pycache__','*.pyc','*.pyo'))
    nested=source/'docs/research_evidence.zip'
    with zipfile.ZipFile(nested,'w') as z: z.writestr('note.txt','harmless evidence')
    refresh_manifest(source)
    pkg=td/'source.zip'; make_flat_zip(source,pkg)
    cp=run([sys.executable,str(source/'scripts/check_falcion_golden_cleaning.py'),'--root',str(source),'--package',str(pkg)])
    text=(cp.stdout or '')+(cp.stderr or '')
    assert cp.returncode==1,(cp.returncode,text)
    assert 'embedded archive payload' in text,text

# S0135: a drifted package with its own self-consistent rebound manifest must
# still fail because package identity is bound to the verified source manifest.
with tempfile.TemporaryDirectory(prefix='hf04_falcion_identity_') as td_s:
    td=Path(td_s); canonical=td/'canonical'; shutil.copytree(ROOT,canonical,ignore=shutil.ignore_patterns('build','dist','__pycache__','*.pyc','*.pyo'))
    drifted=td/'drifted'; shutil.copytree(canonical,drifted)
    (drifted/'README.md').write_text((drifted/'README.md').read_text()+"\npackage-only drift\n",encoding='utf-8')
    refresh_manifest(drifted)
    pkg=td/'drifted.zip'; make_flat_zip(drifted,pkg)
    cp=run([sys.executable,str(canonical/'scripts/check_falcion_golden_cleaning.py'),'--root',str(canonical),'--package',str(pkg)])
    text=(cp.stdout or '')+(cp.stderr or '')
    assert cp.returncode==1,(cp.returncode,text)
    assert 'package/source manifest identity mismatch' in text,text

print('RELEASE_PUBLICATION_REMEDIATION: PASS (S0104/S0116/S0120/S0127/S0135)')

# Governance sensitive-material contract: a project-owned secret marker must
# fail closed even after the fixture rebinds its own manifest.
with tempfile.TemporaryDirectory(prefix='hf02_sensitive_') as td_s:
    td=Path(td_s); source=td/'source'; shutil.copytree(ROOT,source,ignore=shutil.ignore_patterns('build','dist','__pycache__','*.pyc','*.pyo'))
    marker='-----BEGIN '+'PRIVATE KEY-----\nsynthetic-only\n-----END '+'PRIVATE KEY-----\n'
    (source/'docs'/'sensitive_fixture.txt').write_text(marker,encoding='utf-8')
    refresh_manifest(source)
    cp=run([sys.executable,str(source/'scripts/check_falcion_golden_cleaning.py'),'--root',str(source)])
    text=(cp.stdout or '')+(cp.stderr or '')
    assert cp.returncode==1,(cp.returncode,text)
    assert 'sensitive material private-key' in text,text

# HOTFIX14 authorization and transactional pointer gate. Keep corruption/sensitive-material tests above unchanged.
pub=json.loads((ROOT/'metadata/PUBLICATION_TRANSACTION.json').read_text(encoding='utf-8'))
state=json.loads((ROOT/'RELEASE_STATE.json').read_text(encoding='utf-8'))
ordinal=state.get('hotfix_ordinal')
assert isinstance(ordinal,int) and ordinal >= 17
identity=f'preloader_l 3.2.5 SQUALERIS HOTFIX {ordinal}'
if ordinal in (17,18):
    expected_parent={17:'9b380e4d953dffd03ace8b762e1199f11ca8ca4f373d86a0c9b3c074d647082c',18:'1e629c07eeb21edaefd6f7f6fd9481ddb550116f8e77078cd02b1d11baa090b8'}[ordinal]
    expected_status={17:'GRANTED_HF17_2026_10_17_SUBJECT_TO_G2_AND_G4',18:'GRANTED_HF18_2026_10_18_SUBJECT_TO_G0_G1_G2_G4'}[ordinal]
else:
    # New hotfix identity/parent comes from explicit user-authored release
    # authorization record, not a newly hard-coded test per hotfix.
    authorization=json.loads((ROOT/'metadata/RELEASE_AUTHORIZATION.json').read_text())
    granted=authorization.get(f'hf{ordinal}_authorization',{})
    assert granted.get('exact_identity')==identity
    assert granted.get('publication_authorized') is True
    assert granted.get('production_source_change_authorized') is True
    assert str(granted.get('user_text','')).strip()
    expected_parent=granted.get('immediate_official_parent_sha256')
    assert isinstance(expected_parent,str) and len(expected_parent)==64 and all(c in '0123456789abcdef' for c in expected_parent)
    assert pub['canonical_before']['identity']==f'preloader_l 3.2.5 SQUALERIS HOTFIX {ordinal-1}'
    expected_status=f"GRANTED_HF{ordinal}_{granted['decision_date'].replace('-','_')}_SUBJECT_TO_G0_G1_G2_G4"
assert pub['official_release_identity']==identity and pub['candidate_release_identity']==identity
assert state['official_release_identity']==identity and state['release_type']=='HOTFIX_RELEASE'
assert pub['production_authorized'] is True
assert pub['publication_authorized'] is True and pub['publication_authorization']['status']==expected_status
assert pub['canonical_before']['sha256']==expected_parent
assert pub['official_package_filename']==f'preloader_l_3.2.5_SQUALERIS_HOTFIX_{ordinal:02d}.zip'
assert pub['canonical_pointer_committed'] is False
assert pub['post_commit_verified'] is False
print(f'HOTFIX{ordinal}_GOVERNANCE_AUTHORIZED_UNCOMMITTED_AND_SENSITIVE_MATERIAL: PASS')
