#!/usr/bin/env python3
"""GF11 independent source integrity and forbidden shipping-link audit.
Does not assert Windows/native acceptance or completion of executable link.
"""
import argparse,hashlib,json,pathlib,re,sys,zipfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
PARENT='d7f608e7290ba7eb6bbf0ba68f528061dd687702dca11ed35a32f1e9c6d389e0'
HF19='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'

def sha(b):return hashlib.sha256(b).hexdigest()
def verify(parent=None,hf19=None):
    mf=json.loads((ROOT/'metadata/manifest.json').read_text())
    files=mf['files']
    actual={p.relative_to(ROOT).as_posix() for p in ROOT.rglob('*') if p.is_file()}
    if any('__pycache__' in pathlib.PurePosixPath(n).parts or n.endswith('.pyc') for n in actual):raise ValueError('CACHE_ARTIFACT_FORBIDDEN')
    if actual!=set(files)|{'metadata/manifest.json'}:raise ValueError('FILE_SET_DRIFT')
    for name,expected in files.items():
        if '..' in pathlib.PurePosixPath(name).parts or name.startswith('/'):raise ValueError('BAD_MEMBER_PATH')
        if sha((ROOT/name).read_bytes())!=expected:raise ValueError('FILE_SHA_DRIFT:'+name)
    g=json.loads((ROOT/'metadata/GF11_STATUS.json').read_text())
    l=json.loads((ROOT/'metadata/GF11_REUSE_LEDGER.json').read_text())
    if g['release']!='NOT_RELEASE' or g['classification']!='NON_RELEASE_WORK':raise ValueError('FALSE_RELEASE')
    if g['gf10_parent_sha256']!=PARENT or g['official_hf19_sha256']!=HF19:raise ValueError('FALSE_PARENT')
    if l['gf10_parent_sha256']!=PARENT or l['official_hf19_sha256']!=HF19:raise ValueError('FALSE_LEDGER_PARENT')
    if g['new_exact_hf19_files']!=5 or g['cumulative_exact_hf19_files']!=226 or len(l['new_exact_hf19_files'])!=5:raise ValueError('SOURCE_COUNT_DRIFT')
    for k,h in l['new_exact_hf19_files'].items():
        if sha((ROOT/'adopted_hf19'/k).read_bytes())!=h:raise ValueError('ADOPTED_FILE_MUTATION:'+k)
    for k in ('msvc_release','msvc_debug','iocp','lsx','consumer_game','complete_dispatch_link'):
        if g[k]!='NOT_RUN':raise ValueError('FALSE_NATIVE_OR_LINK_PASS:'+k)
    if g['gf_preloader_surge']!='NOOP' or g['gf_preloader_dllshutdown']!='ZERO_OWNER':raise ValueError('FALSE_SHIPPING_STATUS')
    if g['original_get_config_probe']!='OPEN_EXIT_14_NOT_COUNTED_PASS':raise ValueError('HIDDEN_ORIGINAL_FAILURE')
    if g['canonical_hf19_modified'] or g['current_artifact_modified']:raise ValueError('FALSE_CANONICAL_MUTATION')
    cm=(ROOT/'CMakeLists.txt').read_text()
    m=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
    if not m or m.group(1).strip()!='gf_core gf_adopted':raise ValueError('PREMATURE_SHIPPING_LINK')
    if 'WILL_FAIL' in cm:raise ValueError('MASKED_TEST_FAILURE')
    if not all(('adopted_hf19/'+n in cm) for n in ('src/net/socket_runtime.cpp','src/runtime/runtime_state.cpp','src/protocol/socket_exchange.cpp','tests/unit/test_socket_runtime.cpp','tests/unit/test_runtime_state.cpp')):raise ValueError('NEW_SOURCE_NOT_COMPILED')
    defs=(ROOT/'exports/preloader_l.def').read_text()
    if set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',defs,re.M))!={('DllMain','1'),('SURGE','2'),('DllShutdown','3')}:raise ValueError('EXPORT_DRIFT')
    if parent:
        if sha(parent.read_bytes())!=PARENT:raise ValueError('PARENT_SHA')
        with zipfile.ZipFile(parent) as z:
            if z.testzip() is not None:raise ValueError('PARENT_CRC')
            for n in z.namelist():
                if n in ('CMakeLists.txt','metadata/manifest.json','scripts/__pycache__/gf10_pe_export_gate.cpython-313.pyc'):continue
                if (ROOT/n).read_bytes()!=z.read(n):raise ValueError('INHERITED_FILE_CHANGED:'+n)
    if hf19:
        if sha(hf19.read_bytes())!=HF19:raise ValueError('OFFICIAL_HF19_SHA')
        with zipfile.ZipFile(hf19) as z:
            if z.testzip() is not None:raise ValueError('OFFICIAL_HF19_CRC')
            for n in l['new_exact_hf19_files']:
                if (ROOT/'adopted_hf19'/n).read_bytes()!=z.read(n):raise ValueError('NOT_BYTE_IDENTICAL:'+n)
    return len(files)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--parent-zip',type=pathlib.Path);p.add_argument('--hf19-zip',type=pathlib.Path);a=p.parse_args()
    try:print('GF11_SOURCE_VERIFY_PASS',verify(a.parent_zip,a.hf19_zip),'FILES','NOT_RELEASE')
    except Exception as e:print('GF11_SOURCE_VERIFY_FAIL',str(e));sys.exit(1)
