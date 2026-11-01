#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
abi=(r/'include/preloader_l/runtime/service_abi.hpp').read_text()
auth='\n'.join((r/p).read_text() for p in [
    'src/runtime/service_policy.cpp',
    'src/runtime/service_values.cpp',
    'src/runtime/service_settings.cpp',
    'src/runtime/service_xml.cpp',
])
win=(r/'src/config/process_configuration.cpp').read_text()
config=(r/'src/config/runtime_configuration.cpp').read_text()
import json
qa_catalog=json.loads((r/'metadata/RELEASE_QA_CATALOG.json').read_text())
checks={
 'wide path type':'struct ModelWidePathString32' in abi and 'uint16_t storage[8];' in abi,
 'wide path size':'sizeof(ModelWidePathString32) == 0x20' in abi,
 'xml document type':'struct ModelXmlDocument370' in abi and 'struct ModelXmlMemPoolOpaque98' in abi and 'offsetof(ModelXmlDocument370, element_pool) == 0x110' in abi,
 'xml node core':'struct ModelXmlNodeBase68' in abi and 'sizeof(ModelXmlNodeBase68) == 0x68' in abi,
 'xml char buffer offset':'offsetof(ModelXmlDocument370, char_buffer) == 0x98' in abi,
 'xml line offset':'offsetof(ModelXmlDocument370, parse_cur_line_num) == 0xA0' in abi,
 'xml depth offset':'offsetof(ModelXmlDocument370, parsing_depth) == 0xA4' in abi,
 'old opaque doc removed':'ModelDocumentStorage370' not in abi,
 'old narrow path removed':'ModelPathString32' not in abi,
 'no synthetic document tag':'PLRDOC1' not in auth and 'DocumentPolicyHeader' not in auth,
 'document ctor state':'initialize_model_xml_document' in auth and 'doc->node.document = reinterpret_cast<uintptr_t>(doc);' in auth and 'doc->process_entities = 1u;' in auth,
 'document owns parse buffer':'replacement.char_buffer = reinterpret_cast<uintptr_t>(replacement_buffer);' in auth and 'ModelXmlAllocationRecord{replacement_buffer, allocation_bytes, false}' in auth,
 'document exact release bytes':'release_recorded_model_xml_buffer' in auth and 'release_bytes(pointer, bytes)' in auth,
 'document transactional replacement':'if (!release_model_xml_buffer_only(doc))' in auth and '(void)release_recorded_model_xml_buffer(replacement_buffer);' in auth and '*doc = replacement;' in auth,
 'document swap self repair':'repair_model_xml_document_self(&live->config.document);' in auth and 'repair_model_xml_document_self(&candidate->config.document);' in auth,
 'wide sso seven':'kWidePathSsoCapacity = 7u' in auth and 'size <= kWidePathSsoCapacity' in auth,
 'wide heap element size':'(capacity + 1u) * sizeof(uint16_t)' in auth,
 'wide cwd producer':'GetCurrentDirectoryW' in win and 'set_runtime_current_directory_wide_policy' in win,
 'wide policy commit':'commit_model_service_current_directory_wide' in auth and 'set_runtime_current_directory_wide_policy' in config,
 'runtime semantic gate wired':qa_catalog.get('schema')=='universal-release-qa-catalog.v1' and any(step.get('script')=='test_model_config_semantic_runtime.py' and step.get('migration')=='GENERAL_CHECK' and step.get('retain_in_authoritative_universe') is True for step in qa_catalog.get('steps',[])),
}
failed=[k for k,v in checks.items() if not v]
if failed:
    print('MODEL_CONFIG_SEMANTIC_ABI: FAIL')
    for x in failed: print(' -',x)
    sys.exit(1)
print(f'MODEL_CONFIG_SEMANTIC_ABI: PASS ({len(checks)}/{len(checks)})')
