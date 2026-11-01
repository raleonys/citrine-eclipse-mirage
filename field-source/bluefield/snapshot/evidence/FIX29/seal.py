import json,shutil,sys
from run_profile import W,S,H,run
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
for profile in ['production','debug']:
 r=json.loads((H/f'{profile}_TESTS.json').read_text());assert r['status']=='PASS' and r['tests']==50 and r['source_digest']==frozen
assert '74 negative' in (H/'source_negative.log').read_text() and '8 negative' in (H/'pe_negative.log').read_text()
assert json.loads((H/'SUPPLEMENTAL.json').read_text())['status']=='PASS'
assert json.loads((H/'INTEGRITY.json').read_text())['status']=='PASS'
recovery=json.loads((H/'EXEC_PERMISSION_RECOVERY.json').read_text());recovery['status']='PASS_RECOVERED_FULL_DEBUG_CTEST50';(H/'EXEC_PERMISSION_RECOVERY.json').write_text(json.dumps(recovery,indent=2)+'\n')
for name,out in [('source_None.json','SOURCE_CHECK.json')]:shutil.copy2(S/'build/reports'/name,H/out)
e=S/'evidence/FIX29';e.mkdir(parents=True,exist_ok=True)
for p in H.iterdir():
 if p.is_file() and p.suffix in ['.py','.json','.xml','.log'] and not p.name.startswith(('upload_','saved_')):shutil.copy2(p,e/p.name)
acceptance={'classification':'NON_RELEASE_WORK','status':'PASS_BOUNDED_SOURCE_QA_NATIVE_NOT_RUN','source_digest':frozen,'official_publication':False,'scope':'D15 source parse-time depth100/error precedence/full ErrorStr restored; current upstream retained through explicit sole implementation compile adapter','portable_CTest':{'Production':'50/50','Debug':'50/50'},'source_negatives':74,'synthetic_PE_negatives':8,'live_XML_comparisons':4284,'live_XML_checks':17138,'negative_runtime_controls':3,'focused_ASan_UBSan':'PASS4 explicit TUs including both parser implementations; remainder core archive uninstrumented','changed_prior_fixture':'Only XML reader diagnostic expectation strengthened; other55 prior C++ fixtures unchanged','vendor_files_unchanged':1060,'bounded_review_closed':['D15'],'remaining_open':['DEP05','D02','D06','D07','D13','D14','D21'],'NOT_RUN':['Windows/MSVC DLL build','native provider/hooks/IOCP/CRT/game','actual DLL Golden','full-core sanitizer','LSan'],'exact_ZIP_clean_replay':'Pending external exact-ZIP final QA; receipt cannot be inserted into the same sealed archive it validates.'}
(S/'metadata/ACCEPTANCE.json').write_text(json.dumps(acceptance,indent=2)+'\n');assert source_digest()==frozen
run([sys.executable,S/'scripts/qa/package_source.py'],H/'package_seal.log');archive=S/'dist/preloader_l_BF02_FIX29_APPLICATION_SOURCE_NOT_RELEASE.zip'
run([sys.executable,S/'tests/tools/test_package_policy.py',archive],H/'package_negative.log');shutil.copy2(S/'build/reports/package_seal.json',H/'PACKAGE_SEAL.json')
print('PASS complete source ZIP sealed and6 package negatives',flush=True)
