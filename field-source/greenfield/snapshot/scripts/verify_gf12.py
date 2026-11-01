#!/usr/bin/env python3
"""GF12 fail-closed exact-source, parent lineage, shipping-surface, donor identity audit.
Does not assert DLL link, native Windows, IOCP or live LSX acceptance.
"""
import argparse,hashlib,json,pathlib,re,sys,zipfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
PARENT='962d4d636f8dd666e84486336d3b378346c05136f78e1879c07fa09f78b1659e'
HF19='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'
ORIGINAL=[
'src/protocol/challenge_handshake.cpp','src/protocol/socket_exchange_frame_step.cpp',
'src/protocol/socket_exchange_wire.cpp','src/protocol/socket_exchange_transaction.cpp',
'tests/unit/test_async_wire_scanner.cpp','tests/unit/test_handshake.cpp']
SHA=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def verify(parent=None,hf19=None):
    m=json.loads((ROOT/'metadata/manifest.json').read_text())['files']
    actual={p.relative_to(ROOT).as_posix() for p in ROOT.rglob('*') if p.is_file()}
    if any('__pycache__' in pathlib.PurePosixPath(x).parts or x.endswith('.pyc') for x in actual):raise ValueError('CACHE_FILE_FORBIDDEN')
    if actual!=set(m)|{'metadata/manifest.json'}:raise ValueError('FILE_SET_DRIFT')
    for n,h in m.items():
        if n.startswith('/') or '..' in pathlib.PurePosixPath(n).parts:raise ValueError('PATH_ESCAPE')
        if SHA(ROOT/n)!=h:raise ValueError('HASH_MISMATCH:'+n)
    status=json.loads((ROOT/'metadata/GF12_STATUS.json').read_text())
    ledger=json.loads((ROOT/'metadata/GF12_REUSE_LEDGER.json').read_text())
    if status['release']!='NOT_RELEASE' or status['classification']!='NON_RELEASE_WORK':raise ValueError('FALSE_RELEASE')
    if status['gf11_parent_sha256']!=PARENT or status['official_hf19_sha256']!=HF19:raise ValueError('FALSE_STATUS_PARENT')
    if ledger['parent_gf11_sha256']!=PARENT or ledger['official_hf19_sha256']!=HF19:raise ValueError('FALSE_LEDGER_PARENT')
    if status['new_byte_identical_hf19_files']!=6 or status['cumulative_exact_hf19_files']!=232:raise ValueError('FALSE_COUNT')
    if set(ledger['new_exact_hf19_files'])!=set(ORIGINAL):raise ValueError('FALSE_SOURCE_LIST')
    for n,h in ledger['new_exact_hf19_files'].items():
        if SHA(ROOT/'adopted_hf19'/n)!=h:raise ValueError('ADOPTED_SOURCE_MUTATED:'+n)
    if status['gf11_exchange_12_symbols_closed_by_gf12_sources']!=12:raise ValueError('SYMBOL_STATUS_DRIFT')
    for k in ('msvc_x64','windows_export_load','iocp','live_lsx','consumer_game','real_windows_dispatch_final_link','shipping_final_link'):
        if status[k]!='NOT_RUN':raise ValueError('FORGED_NATIVE_OR_FINAL_LINK_PASS:'+k)
    if status['linux_full_dispatch_link']!='BLOCKED_WIN32_AVATAR_MISSING':raise ValueError('FORGED_ELF_LINK')
    if status['gf_preloader_surge']!='NOOP' or status['gf_preloader_dllshutdown']!='ZERO_OWNER':raise ValueError('FALSE_SHIPPING_LIFECYCLE')
    if status['canonical_hf19_modified'] or status['current_artifact_modified']:raise ValueError('CANONICAL_DRIFT')
    if status['get_config_malformed_entity']!='OPEN_EXIT14_NOT_COUNTED_PASS':raise ValueError('HIDDEN_KNOWN_FAILURE')
    governance=json.loads((ROOT/'metadata/GF12_GOVERNANCE_SNAPSHOT.json').read_text())
    if governance['library_master_revision']!='2026-10-19.3' or governance['publication_authorization']:raise ValueError('GOVERNANCE_FAIL')
    cm=(ROOT/'CMakeLists.txt').read_text()
    # Every library link statement for the shipping target must be accounted for.
    # Checking only the first invocation misses a second, appended link command.
    shiplinks=re.findall(r'\btarget_link_libraries\s*\(\s*gf_preloader\b([^)]*)\)',cm,re.I|re.S)
    if len(shiplinks)!=1 or shiplinks[0].split()!=['PRIVATE','gf_core','gf_adopted']:
        raise ValueError('PREMATURE_SHIPPING_LINK')
    if 'WILL_FAIL' in cm:raise ValueError('MASKED_FAIL')
    for n in ORIGINAL:
        if 'adopted_hf19/'+n not in cm:raise ValueError('NEW_ORIGINAL_NOT_SOURCE_LINKED:'+n)
    if not all(s in cm for s in ('add_library(gf12_exchange_transitive_units STATIC','gf12_original_handshake_real_source','gf12_original_wire_scanner_real_source')):raise ValueError('DORMANT_LINK_SHAPE_DRIFT')
    deff=(ROOT/'exports/preloader_l.def').read_text()
    if set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',deff,re.M))!={('DllMain','1'),('SURGE','2'),('DllShutdown','3')}:raise ValueError('EXPORT_DRIFT')
    # Preserve the clean GF00 shipping entry: no live dispatcher start.
    entry=(ROOT/'src/win/dll_entry.cpp').read_text()
    if 'surge_invoked' not in entry and 'SURGE' not in entry:raise ValueError('ENTRY_SURFACE_DRIFT')
    if parent:
        if SHA(parent)!=PARENT:raise ValueError('GF11_PARENT_HASH')
        with zipfile.ZipFile(parent) as z:
            if z.testzip() is not None:raise ValueError('GF11_PARENT_CRC')
            for n in z.namelist():
                if n in ('CMakeLists.txt','metadata/manifest.json'):continue
                if not (ROOT/n).is_file() or (ROOT/n).read_bytes()!=z.read(n):raise ValueError('GF11_PARENT_FILE_DRIFT:'+n)
    if hf19:
        if SHA(hf19)!=HF19:raise ValueError('HF19_SOURCE_HASH')
        with zipfile.ZipFile(hf19) as z:
            if z.testzip() is not None:raise ValueError('HF19_CRC')
            for n in ORIGINAL:
                if (ROOT/'adopted_hf19'/n).read_bytes()!=z.read(n):raise ValueError('NOT_ORIGINAL_HF19:'+n)
    return len(m)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--parent-zip',type=pathlib.Path);p.add_argument('--hf19-zip',type=pathlib.Path);a=p.parse_args()
    try:print('GF12_SOURCE_VERIFY_PASS',verify(a.parent_zip,a.hf19_zip),'FILES','NOT_RELEASE')
    except Exception as e:print('GF12_SOURCE_VERIFY_FAIL',e);sys.exit(1)
