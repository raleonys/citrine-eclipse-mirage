import json,shutil,sys
from run_profile import S,H,run
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
for profile in ['production','debug']:
 r=json.loads((H/f'{profile}_TESTS.json').read_text());assert r['status']=='PASS' and r['tests']==52 and r['source_digest']==frozen
assert '80 negative' in (H/'source_negative.log').read_text() and '8 negative' in (H/'pe_negative.log').read_text()
assert json.loads((H/'SUPPLEMENTAL.json').read_text())['status']=='PASS' and json.loads((H/'INTEGRITY.json').read_text())['status']=='PASS'
run([sys.executable,S/'scripts/qa/check_source.py'],H/'source_final.log')
shutil.copy2(S/'build/reports/source_None.json',H/'SOURCE_CHECK.json')
e=S/'evidence/FIX31';e.mkdir(parents=True,exist_ok=True)
for p in H.iterdir():
 if p.is_file() and p.suffix in ['.py','.json','.xml','.log'] and not p.name.startswith(('upload_','saved_','PROJECT_RULES_MASTER.')):shutil.copy2(p,e/p.name)
acceptance={'classification':'NON_RELEASE_WORK','status':'PASS_BOUNDED_SOURCE_QA_NATIVE_NOT_RUN','source_digest':frozen,'official_publication':False,
 'scope':'Exact REVIEW31 whole-region64MiB remaining scan guard; other122 runtime unchanged. All native lifecycle/timeout/numeric policies retained.',
 'portable_CTest':{'Production':'52/52','Debug':'52/52'},'source_negatives':80,'synthetic_PE_negatives':8,
 'actual_body_model_checks':150,'negative_runtime_controls':3,'focused_ASan_UBSan':'PASS1 test TU including actual production unload body; not native SDK/SEH/OS or full-core',
 'all64_prior_Cpp_headers_unchanged':True,'vendor_files_unchanged':1060,
 'resolved_subfinding':'D13/D21_OVER_BUDGET_REGION_READ_BEFORE_REFUSAL','source_FIX30_open':7,'carried_REVIEW31_DEP05_review_closure':True,'main_reviews_before':6,'main_reviews_after':6,
 'remaining_open':['D02','D06','D07','D13','D14','D21'],
 'NOT_RUN':['Windows/MSVC DLL or native fixture build','native SDK/SEH/thread suspension/unload/provider/IOCP/CRT/game','actual DLL Golden','full-core sanitizer','LSan'],
 'exact_ZIP_clean_replay':'Pending external exact-ZIP final QA; receipt cannot be inserted into the same sealed archive it validates.'}
(S/'metadata/ACCEPTANCE.json').write_text(json.dumps(acceptance,indent=2)+'\n');assert source_digest()==frozen
run([sys.executable,S/'scripts/qa/package_source.py'],H/'package_seal.log')
archive=S/'dist/preloader_l_BF02_FIX31_APPLICATION_SOURCE_NOT_RELEASE.zip'
run([sys.executable,S/'tests/tools/test_package_policy.py',archive],H/'package_negative.log')
shutil.copy2(S/'build/reports/package_seal.json',H/'PACKAGE_SEAL.json')
print('PASS full source ZIP sealed;6 ZIP negatives',flush=True)
