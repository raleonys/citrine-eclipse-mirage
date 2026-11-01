#!/usr/bin/env python3
"""GF10 independent source integrity / adopted file / native status fail-closed gate."""
import argparse,hashlib,json,pathlib,re,sys,zipfile
R=pathlib.Path(__file__).resolve().parents[1]
PARENT='0d5218505ed279bf73c4d4278621957f274769d683619041f6303d83d32d253d'
HF19='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'
def sh(b):return hashlib.sha256(b).hexdigest()
def hh(p):return sh(p.read_bytes())
def verify(parent=None,hf19=None):
    files=json.loads((R/'metadata/manifest.json').read_text())['files']
    actual={p.relative_to(R).as_posix() for p in R.rglob('*') if p.is_file()}
    assert actual==set(files)|{'metadata/manifest.json'},'FILE_SET_CHANGED'
    for n,h in files.items():assert hh(R/n)==h,'MISMATCH:'+n
    status=json.loads((R/'metadata/GF10_STATUS.json').read_text())
    ledger=json.loads((R/'metadata/GF10_REUSE_LEDGER.json').read_text())
    assert status['classification']=='NON_RELEASE_WORK' and status['release']=='NOT_RELEASE','FALSE_RELEASE'
    assert status['gf09_parent_sha256']==ledger['gf09_parent_sha256']==PARENT,'BAD_PARENT'
    assert status['official_hf19_sha256']==ledger['official_hf19_sha256']==HF19,'BAD_HF19'
    assert status['cumulative_exact_hf19_files']==221 and len(ledger['new_hf19_files'])==5,'REUSE_COUNT'
    for n,h in ledger['new_hf19_files'].items(): assert hh(R/'adopted_hf19'/n)==h,'NEW_SOURCE_CHANGED:'+n
    for k in ('msvc_x64_release','msvc_x64_debug','windows_loadlibrary_export','native_win32_owner','iocp','lsx','consumer_game'):
        assert status[k]=='NOT_RUN','FORGED_NATIVE:'+k
    assert not status['canonical_hf19_modified'] and not status['current_artifact_modified'],'CANONICAL_MUTATION_CLAIM'
    assert status['gf_preloader_surge']=='NOOP' and status['gf_preloader_dllshutdown']=='ZERO_OWNER','EARLY_RUNTIME_CLAIM'
    cm=(R/'CMakeLists.txt').read_text()
    link=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
    assert link and 'gf_core' in link.group(1) and 'gf_adopted' in link.group(1) and not any(x in link.group(1) for x in ('dispatch_units','gf07_real_dependencies','gf06_full_handler','gf_hf19_asio_owner')),'PREMATURE_SHIPPING_LINK'
    assert 'GF10_ENABLE_RESPONSE_SINKS_WINDOWS_PROBE' in cm and 'OFF)' in cm.split('option(GF10_ENABLE_RESPONSE_SINKS_WINDOWS_PROBE',1)[1].splitlines()[0], 'FALSE_LINK_PROBE_PASS'
    assert 'WILL_FAIL' not in cm, 'HIDDEN_TEST_FAILURE'
    defs=(R/'exports/preloader_l.def').read_text()
    assert set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',defs,re.M))=={('DllMain','1'),('SURGE','2'),('DllShutdown','3')},'EXPORT_DRIFT'
    assert 'check(data)' in (R/'scripts/gf10_pe_export_gate.py').read_text(), 'MISSING_PE_CHECK'
    if parent:
        assert hh(parent)==PARENT,'PARENT_SHA'
        with zipfile.ZipFile(parent) as z:
            assert z.testzip() is None,'PARENT_ZIP_BAD'
            for n in z.namelist():
                if n in ('CMakeLists.txt', 'metadata/manifest.json'):continue
                assert (R/n).read_bytes()==z.read(n),'PARENT_DRIFT:'+n
    if hf19:
        assert hh(hf19)==HF19,'HF19_SHA'
        with zipfile.ZipFile(hf19) as z:
            assert z.testzip() is None,'HF19_ZIP_BAD'
            for n in ledger['new_hf19_files']:
                assert (R/'adopted_hf19'/n).read_bytes()==z.read(n),'NOT_EXACT_HF19:'+n
    return len(files)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--parent-zip',type=pathlib.Path);p.add_argument('--hf19-zip',type=pathlib.Path);a=p.parse_args()
    try:print('GF10_SOURCE_VERIFY_PASS',verify(a.parent_zip,a.hf19_zip),'MANIFEST_FILES','NOT_RELEASE','WINDOWS_NOT_RUN')
    except Exception as e:print('GF10_SOURCE_VERIFY_FAIL',str(e));sys.exit(1)
