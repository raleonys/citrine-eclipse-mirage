import json,shutil,subprocess,sys
from pathlib import Path
from run_profile import W,S,H,run
sys.path.insert(0,str(S/'scripts/qa'));from _project import source_digest
frozen=json.loads((H/'FROZEN_SOURCE.json').read_text())['source_digest'];assert source_digest()==frozen
for profile in ['production','debug']:
 r=json.loads((H/f'{profile}_TESTS.json').read_text());assert r['status']=='PASS' and r['tests']==49 and r['source_digest']==frozen
assert '71 negative' in (H/'source_negative.log').read_text()
assert '8 negative' in (H/'pe_negative.log').read_text()
assert json.loads((H/'SUPPLEMENTAL.json').read_text())['status']=='PASS'
e=S/'evidence/FIX28';e.mkdir(parents=True,exist_ok=True)
for p in H.iterdir():
 if p.is_file() and p.suffix in ['.log','.json','.xml'] and not p.name.startswith(('saved_','upload_')):shutil.copy2(p,e/p.name)
for p in H.glob('*.py'):shutil.copy2(p,e/p.name)
acceptance={'classification':'NON_RELEASE_WORK','status':'PASS_BOUNDED_SOURCE_QA_NATIVE_NOT_RUN','source_digest':frozen,'official_publication':False,'scope':'Q02 live source parser oracle and accurate fixed lifecycle failure events; privacy retained. No production parser or vendor change.','portable_CTest':{'Production':'49/49','Debug':'49/49'},'source_negatives':71,'synthetic_PE_negatives':8,'live_Config2_comparisons':77205,'live_Config2_checks':85973,'failure_diagnostics_checks':{'Production':5,'Debug':13},'negative_runtime_controls':3,'focused_ASan_UBSan':'PASS; explicitly enumerated 5 TUs across 2 executables, unchanged remaining core archive uninstrumented','generated_artifact_recovery':'Observed empty/invalid build outputs rebuilt through generated rules/link commands; logs retained. Cause undetermined; no source/expectation edit.','bounded_review_closed':['Q02'],'remaining_open':['DEP05','D02','D06','D07','D13','D14','D15','D21'],'NOT_RUN':['Windows/MSVC DLL build','Windows bootstrap catches and provider','native IOCP/CRT/game','actual DLL Golden','full-core sanitizer','LSan'],'exact_ZIP_clean_replay':'Pending external final QA; replay receipt intentionally cannot be embedded in the exact ZIP it validates.'}
(S/'metadata/ACCEPTANCE.json').write_text(json.dumps(acceptance,indent=2,ensure_ascii=False)+'\n')
assert source_digest()==frozen
run([sys.executable,S/'scripts/qa/package_source.py'],H/'package_seal.log')
archive=S/'dist/preloader_l_BF02_FIX28_APPLICATION_SOURCE_NOT_RELEASE.zip'
run([sys.executable,S/'tests/tools/test_package_policy.py',archive],H/'package_negative.log')
print('PASS complete source sealed and 6 ZIP negative mutations',flush=True)
