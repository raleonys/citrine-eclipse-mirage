#!/usr/bin/env python3
"""Release-independent, fail-closed source-level shipping policy.

A contract is DATA, not trusted authority. Production callers MUST supply an
independently approved SHA-256 for the contract. A PASS only establishes static
policy conformance; it never authorizes governance, publication or runtime.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys

SCHEMA='universal-shipping-source-contract.v1'
SHA_RE=re.compile(r'^[a-f0-9]{64}$')
SUFFIXES=('.cpp','.c','.rc','.def','.asm')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def clean_cpp(raw):
    # Blank comments and strings, retaining punctuation and line breaks.
    pat=re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
    return pat.sub(lambda m: ''.join('\n' if x=='\n' else ' ' for x in m.group()),raw)


def function_body(source,name):
    # Match definition, not an incidental call/declaration; require a body.
    identifier=re.escape(name)
    for m in re.finditer(r'\b'+identifier+r'\s*\([^;{}]*\)\s*(?:noexcept\s*)?\{',source):
        start=source.find('{',m.start())
        depth=1
        for i in range(start+1,len(source)):
            if source[i]=='{':depth+=1
            elif source[i]=='}':
                depth-=1
                if depth==0:return source[start+1:i]
    raise ValueError('function definition missing: '+name)


def ordered_calls(code,names):
    last=-1
    for symbol in names:
        hits=[m.start() for m in re.finditer(r'\b'+re.escape(symbol)+r'\s*\(',code)]
        next_hit=next((x for x in hits if x>last),None)
        if next_hit is None:
            return False,symbol
        last=next_hit
    return True,None


def valid_path(value):
    if not isinstance(value,str) or not value or '\\' in value or value.startswith('/') or '\x00' in value:
        return False
    return all(t not in ('','.','..') for t in value.split('/'))


def validate(root,contract_path,trusted_hash):
    checks={}
    def add(key,condition):checks[key]=bool(condition)
    add('independently_supplied_contract_pin',isinstance(trusted_hash,str) and SHA_RE.fullmatch(trusted_hash) is not None)
    if not checks['independently_supplied_contract_pin']:
        return checks
    add('contract_exists',contract_path.is_file())
    if not checks['contract_exists']:return checks
    raw=contract_path.read_bytes()
    add('contract_sha256',digest(raw)==trusted_hash)
    if not checks['contract_sha256']:return checks
    try: doc=json.loads(raw)
    except (ValueError,UnicodeError):add('contract_parse',False);return checks
    add('contract_schema',isinstance(doc,dict) and doc.get('schema')==SCHEMA and doc.get('release_identity') is None)
    add('contract_fields',set(doc)=={'schema','release_identity','shipping_target','target_type','source_graph','policy'})
    if not checks['contract_schema'] or not checks['contract_fields']:return checks
    target=doc.get('shipping_target');p=doc.get('policy');g=doc.get('source_graph')
    add('contract_shape',isinstance(target,str) and bool(re.fullmatch(r'[A-Za-z][A-Za-z_0-9]*',target)) and doc.get('target_type')=='SHARED' and isinstance(p,dict) and isinstance(g,dict))
    if not checks['contract_shape']:return checks
    required_keys={'allowed_roots','forbidden_source_roots','required_cmake_markers','forbidden_cmake_markers','dependency_pin_path','dependency_pin_marker','entrypoint','bootstrap_file','bootstrap_function','init_order','installer_function','transaction_order','rollback_function','rollback_requirements','config_header','config_freeze_path','config_freeze_tokens','forbidden_prod_tokens','minhook_non_loader_markers','required_hook_sources','forbidden_hook_sources'}
    add('policy_contract_keys',set(p)==required_keys)
    if not checks['policy_contract_keys']:return checks
    source_list=g.get('sources');graph_hash=g.get('sha256');source_hashes=g.get('source_hashes')
    add('source_contract_shape',isinstance(source_list,list) and len(source_list)>0 and all(valid_path(s) and s.endswith(SUFFIXES) for s in source_list) and source_list==sorted(set(source_list)) and isinstance(graph_hash,str) and SHA_RE.fullmatch(graph_hash) is not None)
    if not checks['source_contract_shape']:return checks
    add('contract_graph_digest',digest(('\n'.join(source_list)+'\n').encode())==graph_hash)
    if not checks['contract_graph_digest']:return checks
    add('per_source_hash_contract_shape',isinstance(source_hashes,dict) and set(source_hashes)==set(source_list) and all(isinstance(v,str) and SHA_RE.fullmatch(v) is not None for v in source_hashes.values()))
    if not checks['per_source_hash_contract_shape']:return checks
    add('shipping_source_bytes_pinned',all((root/x).is_file() and digest((root/x).read_bytes())==source_hashes[x] for x in source_list))
    cmake_path=root/'CMakeLists.txt'
    if not cmake_path.is_file():add('cmake_exists',False);return checks
    cmake=cmake_path.read_text(encoding='utf-8')
    cmake_active='\n'.join(line.split('#',1)[0] for line in cmake.splitlines())
    target_block=re.search(r'add_library\(\s*'+re.escape(target)+r'\s+SHARED\b(.*?)\n\)',cmake,re.S)
    add('shipping_target_declared',target_block is not None)
    if not target_block:return checks
    found=re.findall(r'(?m)^\s*([A-Za-z0-9_./-]+\.(?:cpp|c|rc|def|asm))\s*$',target_block.group(1))
    add('cmake_graph_no_duplicate',len(found)==len(set(found)))
    actual=sorted(set(found))
    add('shipping_source_graph_exact',actual==source_list)
    add('shipping_graph_digest',digest(('\n'.join(actual)+'\n').encode())==graph_hash)
    add('shipping_path_roots',all(any(x.startswith(prefix) for prefix in p['allowed_roots']) and not any(x.startswith(prefix) for prefix in p['forbidden_source_roots']) for x in actual))
    add('shipping_sources_exist',all((root/x).is_file() for x in actual))
    add('required_hook_sources',all(x in actual for x in p['required_hook_sources']))
    add('forbidden_hook_sources',not any(x in actual for x in p['forbidden_hook_sources']))
    add('shipping_crt_link_and_flags',all(isinstance(token,str) and token in cmake_active for token in p['required_cmake_markers']))
    add('forbidden_cmake_features',not any(isinstance(token,str) and token in cmake_active for token in p['forbidden_cmake_markers']))
    pin_path=p['dependency_pin_path'];pin_marker=p['dependency_pin_marker']
    add('dependency_pin_declared',valid_path(pin_path) and isinstance(pin_marker,str) and bool(pin_marker) and re.search(r'\b'+re.escape(pin_marker)+r'\s+"([a-f0-9]{40})"',cmake) is not None)
    if checks['dependency_pin_declared']:
        expected=re.search(r'\b'+re.escape(pin_marker)+r'\s+"([a-f0-9]{40})"',cmake).group(1)
        add('vendored_dependency_identity',(root/pin_path).is_file() and (root/pin_path).read_text().strip()==expected)
    else:add('vendored_dependency_identity',False)
    for pathkey in ('entrypoint','bootstrap_file','config_header','config_freeze_path'):
        add(pathkey+'_path',valid_path(p[pathkey]) and (root/p[pathkey]).is_file())
    if not all(checks[x+'_path'] for x in ('entrypoint','bootstrap_file','config_header','config_freeze_path')):return checks
    loader=clean_cpp((root/p['entrypoint']).read_text(encoding='utf-8'))
    add('loader_lock_does_not_install_hooks',not any(re.search(r'\b'+re.escape(t)+r'\s*\(',loader) for t in p['minhook_non_loader_markers']+[p['installer_function']]))
    boot=clean_cpp((root/p['bootstrap_file']).read_text(encoding='utf-8'))
    try:
        startup=function_body(boot,p['bootstrap_function'])
        installer=function_body(boot,p['installer_function'])
        rollback=function_body(boot,p['rollback_function'])
    except ValueError:
        add('bootstrap_functions_available',False)
        return checks
    add('bootstrap_functions_available',True)
    add('config_before_hooks',ordered_calls(startup,p['init_order'])[0] and all(len(re.findall(r'\b'+re.escape(fn)+r'\s*\(',startup))==1 for fn in p['init_order']))
    add('single_transaction_order',ordered_calls(installer,p['transaction_order'])[0] and sum(len(re.findall(r'\b'+re.escape(name)+r'\s*\(',installer)) for name in [p['transaction_order'][0],p['transaction_order'][-1]])==2)
    add('staging_failure_aborts','abort_minhook_batch_transaction' in installer and re.search(r'if\s*\(\s*!stage_core_hooks\(\)\s*\)\s*return\s+abort_staging\(\)',installer) is not None and re.search(r'if\s*\(\s*!stage_addon_hooks\(',installer) is not None)
    add('failed_ownership_retained','failed_ownership_retained' in installer and 'rolled_back' in installer)
    add('rollback_contract',all(re.search(r'\b'+re.escape(symbol)+r'\b',rollback) for symbol in p['rollback_requirements']) and rollback.find('rollback_minhook_batch_transaction_with_timeout') < rollback.find('unfreeze_runtime_configuration_after_hooks'))
    cfg=(root/p['config_freeze_path']).read_text(encoding='utf-8')
    add('cfg_write_freeze_contract',all(s in cfg for s in p['config_freeze_tokens']))
    add('forbidden_transaction_in_entry',not any(s in loader for s in p['forbidden_prod_tokens']))
    return checks


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1]);ap.add_argument('--contract',type=Path)
    ap.add_argument('--trusted-contract-sha',default=os.environ.get('DLL_RD_SHIPPING_CONTRACT_SHA256'))
    ap.add_argument('--report',type=Path)
    a=ap.parse_args()
    root=a.root.resolve();con=(a.contract or root/'metadata/SHIPPING_POLICY_CONTRACT.json').resolve()
    try:checks=validate(root,con,a.trusted_contract_sha)
    except Exception as exc:
        checks={'policy_engine_internal_error':False};print('POLICY_ENGINE_ERROR:',type(exc).__name__,str(exc))
    problems=[k for k,v in checks.items() if not v]
    result={'schema':'universal-shipping-policy-result.v1','execution_level':'source_static','status':'PASS' if not problems else 'FAIL_CLOSED','checks':checks,'failures':problems,'release_or_publication_authorized':False}
    if a.report:
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(result,sort_keys=True,indent=2)+'\n')
    print('UNIVERSAL_SHIPPING_POLICY:',result['status'],f'{len(checks)-len(problems)}/{len(checks)}','publication_authorized=false')
    for name in problems:print(' -',name)
    return 0 if not problems else 1
if __name__=='__main__':sys.exit(main())
