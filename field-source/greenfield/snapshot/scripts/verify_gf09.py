#!/usr/bin/env python3
"""GF09 source status, provenance, manifest and ABI verifier (not native acceptance).
Optional --parent-zip and --hf19-zip independently replay exact adopted bytes.
"""
import argparse,hashlib,json,pathlib,re,sys,zipfile
P=pathlib.Path(__file__).resolve().parent.parent
SHA=lambda b:hashlib.sha256(b).hexdigest()
H=lambda p:SHA(p.read_bytes())
PARENT='e3481aab953a1c940ab18b25873808b51909b66d7eae3027a41097b1bc1d6bcd'
HF19='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'
NEW=10

def check(parent=None,hf19=None):
    manifest=json.loads((P/'metadata/manifest.json').read_text())['files']
    actual={x.relative_to(P).as_posix() for x in P.rglob('*') if x.is_file()}
    assert actual==set(manifest)|{'metadata/manifest.json'},'SOURCE_FILE_SET_CHANGED'
    for n,d in manifest.items():
        assert H(P/n)==d,'SOURCE_HASH_MISMATCH:'+n
    s=json.loads((P/'metadata/GF09_STATUS.json').read_text())
    l=json.loads((P/'metadata/GF09_REUSE_LEDGER.json').read_text())
    assert s['release']=='NOT_RELEASE' and s['classification']=='NON_RELEASE_WORK','FALSE_RELEASE'
    assert s['parent_gf08_source_sha256']==PARENT and l['parent_gf08_sha256']==PARENT,'BAD_PARENT'
    assert s['official_hf19_source_sha256']==HF19 and l['official_hf19_sha256']==HF19,'BAD_ORIGINAL'
    assert s['cumulative_exact_hf19_files']==l['cumulative_exact_hf19_files']==216,'BAD_REUSE_COUNT'
    assert s['gf09_new_exact_hf19_files']==len(l['gf09_new_files'])==NEW,'NEW_FILE_COUNT'
    for n,d in l['gf09_new_files'].items():
        assert H(P/'adopted_hf19'/n)==d,'ADOPTED_HASH:'+n
    assert s['gf_preloader_dll_main_surge_noop'] and s['dllshutdown_zero_owner'] and not s['gf_preloader_links_dispatch'],'EARLY_RUNTIME_CLAIM'
    for k in ('windows_msvc_x64_release','windows_msvc_x64_debug','windows_abi_export_host','windows_iocp','game_consumer'):
        assert s[k]=='NOT_RUN','FORGED_NATIVE_RESULT:'+k
    assert s['lsx']=='NOT_IMPLEMENTED' and not s['canonical_hf19_mutated'] and not s['current_artifact_pointer_mutated'],'FALSE_RUNTIME_OR_RELEASE'
    exports=(P/'exports/preloader_l.def').read_text()
    assert set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',exports,re.M))=={('DllMain','1'),('SURGE','2'),('DllShutdown','3')},'ABI_EXPORT_DRIFT'
    cm=(P/'CMakeLists.txt').read_text()
    link=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
    assert link and 'gf_core' in link.group(1) and 'gf_adopted' in link.group(1) and not any(n in link.group(1) for n in ('gf_hf19_dispatch_units','gf07_real_dependencies','gf09_original_session_state','gf08_original_win32_owner_units','gf06_config_units')),'EARLY_SHIPPING_LINK'
    assert 'if(WIN32 AND MSVC AND GF07_ENABLE_WINDOWS_DEEP_LINK_PROBE)' in cm and 'add_executable(gf09_handler_registry' in cm,'HANDLER_REGISTRY_NOT_GATED'
    assert 'option(GF09_ENABLE_GET_CONFIG_KNOWN_OPEN_PROBE' in cm and 'OFF)' in cm.split('option(GF09_ENABLE_GET_CONFIG_KNOWN_OPEN_PROBE',1)[1].splitlines()[0], 'FALSE_GET_CONFIG_PASS'
    assert 'if(GF09_ENABLE_GET_CONFIG_KNOWN_OPEN_PROBE)' in cm and 'WILL_FAIL' not in cm,'HIDDEN_FAILURE'
    assert 'set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")' in cm, 'CRT_POLICY_DRIFT'
    assert 'RUN_WINDOWS_GF09.cmd' in ''.join(x.name for x in (P/'scripts').iterdir()), 'MISSING_NATIVE_OPERATOR'
    if parent:
        assert H(parent)==PARENT,'WRONG_GF08_ARCHIVE'
        with zipfile.ZipFile(parent) as z:
            bad=z.testzip();assert bad is None,'PARENT_BAD_CRC'
            diffs={'CMakeLists.txt','metadata/manifest.json'}
            for n in z.namelist():
                if n in diffs:continue
                assert (P/n).read_bytes()==z.read(n),'PARENT_MODIFIED:'+n
    if hf19:
        assert H(hf19)==HF19,'WRONG_HF19_ARCHIVE'
        with zipfile.ZipFile(hf19) as z:
            bad=z.testzip();assert bad is None,'HF19_BAD_CRC'
            for n in l['gf09_new_files']:
                assert (P/'adopted_hf19'/n).read_bytes()==z.read(n),'NOT_DIRECT_HF19:'+n
    return len(manifest),len(l['gf09_new_files'])

if __name__=='__main__':
    a=argparse.ArgumentParser();a.add_argument('--parent-zip',type=pathlib.Path);a.add_argument('--hf19-zip',type=pathlib.Path);v=a.parse_args()
    try:
        n,k=check(v.parent_zip,v.hf19_zip)
        print('GF09_SOURCE_VERIFY_PASS',f'manifest={n}/{n}',f'exact_new_HF19={k}/{k}','NATIVE_NOT_RUN','NOT_RELEASE')
    except Exception as e:
        print('GF09_SOURCE_VERIFY_FAIL',str(e));sys.exit(1)
