import json,shutil,sys
from run_profile import W,S,H,run
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
for profile in ['production','debug']:
 r=json.loads((H/f'{profile}_TESTS.json').read_text());assert r['status']=='PASS' and r['tests']==51 and r['source_digest']==frozen
assert '76 negative' in (H/'source_negative.log').read_text() and '8 negative' in (H/'pe_negative.log').read_text()
assert json.loads((H/'SUPPLEMENTAL.json').read_text())['status']=='PASS' and json.loads((H/'INTEGRITY.json').read_text())['status']=='PASS'
shutil.copy2(S/'build/reports/source_None.json',H/'SOURCE_CHECK.json')
e=S/'evidence/FIX30';e.mkdir(parents=True,exist_ok=True)
for p in H.iterdir():
 if p.is_file() and p.suffix in ['.py','.json','.xml','.log'] and not p.name.startswith(('upload_','saved_','MASTER_CURRENT.')):shutil.copy2(p,e/p.name)
acceptance={'classification':'NON_RELEASE_WORK','status':'PASS_BOUNDED_SOURCE_QA_NATIVE_NOT_RUN','source_digest':frozen,'official_publication':False,'scope':'Single exhausted-stage-name throw classification corrected; source initial false/dialog branch restored;100 attempts and other exception/persistence policies retained','portable_CTest':{'Production':'51/51','Debug':'51/51'},'source_negatives':76,'synthetic_PE_negatives':8,'real_collision_checks':811,'preexisting_collision_files_preserved':399,'ticket_checks':120,'negative_runtime_controls':3,'focused_ASan_UBSan':'PASS2 executables,3 instrumented TUs each (5 unique); remaining core archive uninstrumented','changed_prior_fixture':'Ticket collision moved to source initial-failure fallback matrix; other60 prior C++/header fixtures unchanged','vendor_files_unchanged':1060,'resolved_subfinding':'D14/D21_INITIAL_STAGE_NAME_EXHAUSTION_ERROR_CLASS','main_reviews_before':7,'main_reviews_after':7,'remaining_open':['DEP05','D02','D06','D07','D13','D14','D21'],'NOT_RUN':['Windows/MSVC DLL build','native collision/hooks/provider/IOCP/CRT/game','actual DLL Golden','full-core sanitizer','LSan'],'exact_ZIP_clean_replay':'Pending external exact-ZIP final QA; receipt cannot be inserted into the same sealed archive it validates.'}
(S/'metadata/ACCEPTANCE.json').write_text(json.dumps(acceptance,indent=2)+'\n');assert source_digest()==frozen
run([sys.executable,S/'scripts/qa/package_source.py'],H/'package_seal.log');archive=S/'dist/preloader_l_BF02_FIX30_APPLICATION_SOURCE_NOT_RELEASE.zip'
run([sys.executable,S/'tests/tools/test_package_policy.py',archive],H/'package_negative.log');shutil.copy2(S/'build/reports/package_seal.json',H/'PACKAGE_SEAL.json')
print('PASS complete source ZIP sealed;6 ZIP negatives',flush=True)
