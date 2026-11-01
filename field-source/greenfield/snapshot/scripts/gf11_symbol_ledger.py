#!/usr/bin/env python3
"""GF11 bounded real ELF static-object symbol ownership audit (never executable-link acceptance).

This is specifically an ELF/nm research tool, not a Win32 PE/MSVC substitute.
Archives can be compiled in two independently pinned trees, since GF10's inherited
source bytes are immutable in GF11.  Does NOT claim link or runtime success.
"""
import argparse,collections,hashlib,json,pathlib,subprocess,sys
TARGET='libgf_hf19_dispatch_units.a'
GF11=('libgf11_socket_runtime.a','libgf11_runtime_state.a','libgf11_exchange_units.a')
PREFIX='_ZN11preloader_l'

def sh(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def scan(paths):
    defs=collections.defaultdict(set);und=collections.defaultdict(set)
    for path in paths:
        name=path.name
        process=subprocess.run(['nm','-P','-g',str(path)],capture_output=True,text=True,check=True)
        for line in process.stdout.splitlines():
            parts=line.split()
            if len(parts)<2 or not parts[0].startswith(PREFIX):continue
            symbol,typ=parts[:2]
            if typ=='U':und[name].add(symbol)
            elif typ in 'TWDBVR':defs[symbol].add(name)
    return defs,und

def demangle(s):
    return subprocess.run(['c++filt',s],capture_output=True,text=True,check=True).stdout.strip()

def run(old_dir,new_dir):
    old=sorted(old_dir.glob('lib*.a'))
    new=[new_dir/n for n in GF11]
    if not old or any(not p.is_file() for p in new) or not (old_dir/TARGET).is_file():raise ValueError('ARCHIVES_NOT_BUILT')
    pre,un=scan(old)
    baseline=un[TARGET]
    missing=baseline-set(pre)
    added,more=scan(new)
    merged=collections.defaultdict(set,{k:set(v) for k,v in pre.items()})
    for sym,owners in added.items():merged[sym]|=owners
    after=baseline-set(merged)
    expected={
       'preloader_l::net::stop_socket_runtime(preloader_l::net::SocketRuntime*)',
       'preloader_l::net::reset_socket_runtime(preloader_l::net::SocketRuntime*)',
       'preloader_l::runtime::try_shutdown()',
       'preloader_l::runtime::try_initialize()',
       'preloader_l::protocol::reset_exchange(preloader_l::protocol::ExchangeState*)',
    }
    if len(baseline)!=58 or len(missing)!=5 or {demangle(x) for x in missing}!=expected:
        raise ValueError(f'GF10_OWNER_BASELINE_DRIFT:{len(baseline)},{len(missing)}')
    if after:raise ValueError('GF11_FIVE_DEFINITIONS_ABSENT')
    for x in missing:
        if len(added.get(x,()))!=1:raise ValueError('GF11_OWNER_NOT_UNIQUE:'+demangle(x))
    transitive=sorted(x for x in more['libgf11_exchange_units.a'] if x not in merged)
    return {'schema':'greenfield.gf11.elf.static_symbol_observation.v1',
      'evidence_type':'ELF_STATIC_ARCHIVE_DEFINITIONS_ONLY_NOT_LINK_NOT_PE',
      'gf10_dispatch_undefined_project_symbols':len(baseline),
      'gf10_defined_in_other_archives':len(baseline)-len(missing),
      'gf10_without_owner':len(missing),
      'gf11_five_owner_definitions_present':len(missing)-len(after),
      'gf11_dispatch_symbols_without_definition':len(after),
      'owner_map':[{ 'symbol':demangle(k),'owner':next(iter(added[k])) } for k in sorted(missing)],
      'new_socket_exchange_unowned_transitive_symbols':list(map(demangle,transitive)),
      'new_socket_exchange_unowned_transitive_count':len(transitive),
      'native_msvc_x64':'NOT_RUN','complete_executable_link':'NOT_RUN',
      'old_dispatch_archive_sha256':sh(old_dir/TARGET),
      'new_owner_archive_sha256':{p.name:sh(p) for p in new}}

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--base-build',type=pathlib.Path,required=True);p.add_argument('--new-build',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path);a=p.parse_args()
    try:
        result=run(a.base_build,a.new_build)
        s=json.dumps(result,indent=2,sort_keys=True)+'\n'
        if a.output:a.output.write_text(s)
        print('GF11_SYMBOL_DEFINITIONS_PASS',result['gf10_dispatch_undefined_project_symbols'],'ORIGINAL',result['gf11_five_owner_definitions_present'],'NEW_OWNERS','TRANSITIVE_UNOWNED',result['new_socket_exchange_unowned_transitive_count'],'NOT_FULL_LINK')
    except Exception as e:
        print('GF11_SYMBOL_DEFINITIONS_FAIL',str(e));sys.exit(1)
