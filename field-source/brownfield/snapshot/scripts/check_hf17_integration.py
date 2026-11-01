#!/usr/bin/env python3
"""Scoped HF17 source-graph gate; not a substitute for MSVC/native/game acceptance."""
from pathlib import Path
import re,json,subprocess,hashlib,sys
r=Path(__file__).resolve().parents[1]
f=lambda n:(r/n).read_text()
errors=[]
def ok(name,condition):
 if not condition:errors.append(name)
s=f('CMakeLists.txt');b=f('src/runtime/bootstrap_thread.cpp');h=f('src/platform/addon_hooks.cpp')
cm=re.search(r'add_library\(preloader_l SHARED(.*?)\n\)',s,re.S)
prod=cm.group(1) if cm else ''
ok('one production TU',prod.count('src/platform/addon_hooks.cpp')==1)
ok('not a separate hook engine',all(x in h for x in ('create_minhook(', 'enable_minhook_batch(', 'disable_minhook_batch(', 'remove_minhook(')))
ok('single staged transaction',b.find('begin_minhook_batch_transaction()')>=0 and
   b.rfind('stage_addon_hooks(')>b.rfind('stage_core_hooks(') and
   b.find('commit_minhook_batch_transaction()')>b.rfind('stage_addon_hooks('))
ok('teardown before runtime release',b.find('uninstall_addon_hooks()')<b.find('shutdown_minhook_runtime()'))
ok('exact 4 function targets',all(h.count('"'+n+'"')==1 for n in
   ('CreateProcessA','CreateProcessW','SHGetFolderPathA','SHGetFolderPathW')))
ok('config gates',all(n in h for n in ('runtime_configuration_loaded()', 'BlockReruns','ClientPath','Documents','begin_runtime_configuration_read','end_runtime_configuration_read')))
ok('no B prototype production integration','b_opaque_registry' not in prod and 'b_opaque_registry' not in h)
ok('HF17 architecture pins',json.loads(f('metadata/RELEASE_ARCHITECTURE.json'))['minhook']['detours']==11)
ok('HF18 Candidate01 has not been canonically published',json.loads(f('metadata/PUBLICATION_TRANSACTION.json')).get('canonical_pointer_committed') is False and (r/'docs/HF18_CANDIDATE01_NOT_RELEASE.md').is_file())
ok('HF16 original Registry stable',all((r/p).exists() for p in ('src/platform/registry_hooks.cpp','src/platform/registry_policy_state.cpp')))
print('HF18_CANDIDATE01_SOURCE_IMPLEMENTATION: '+('PASS' if not errors else 'FAIL')+' (checks='+str(10-len(errors))+'/10)')
for x in errors: print(' -',x)
sys.exit(bool(errors))
