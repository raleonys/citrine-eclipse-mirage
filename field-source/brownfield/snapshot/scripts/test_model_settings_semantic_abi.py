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
server=(r/'include/preloader_l/runtime/release_server.hpp').read_text()
config=(r/'src/config/runtime_configuration.cpp').read_text()
errors=[]
checks={
 '0x50 node layout':'struct ModelSettingsNode50' in abi and 'MsvcString32 key;' in abi and 'MsvcString32 value;' in abi,
 '0x40 unordered-map layout':'struct ModelSettingsUnorderedMap40' in abi and 'float max_load_factor;' in abi and 'uint64_t bucket_count;' in abi,
 'curio initial topology':'kModelInitialBuckets = 8u' in auth and 'kModelInitialMask = 7u' in auth and 'write_model_float_one(&map->max_load_factor)' in auth,
 'model FNV1a':'0xcbf29ce484222325' in auth and '0x100000001b3' in auth,
 'model node max-size guard':'0x0333333333333333' in auth,
 'model fast growth':'kModelFastGrowthThreshold = 0x200u' in auth and 'map->bucket_count * 8u' in auth,
 'duplicate update semantics':'find_settings_node(map, input.key, input.key_size)' in auth and 'assign_string(&existing->value' in auth,
 'transactional candidate map':'SettingsMap candidate{};' in auth and 'service->config.settings = candidate;' in auth and 'destroy_settings_map(&candidate);' in auth,
 'clear/destruct ownership':'clear_settings_map' in auth and 'release_string(&node->value)' in auth and 'release_bytes(node, sizeof(SettingsNode))' in auth,
 'hashed lookup':'model_fnv1a(key, key_size) & map->mask' in auth,
 'synthetic map tag removed':'PLRMAP1' not in auth and 'SettingsPolicyHeader' not in auth and 'SettingsPair' not in auth,
 'synthetic document tag removed':'PLRDOC1' not in auth and 'DocumentPolicyHeader' not in auth,
 'clean readiness outside curio bytes':'configuration_loaded' in server and 'configuration_present_sections' in server and 'set_model_service_configuration_state' in auth,
 'document private-pool boundary':'ModelXmlMemPoolOpaque98' in abi and 'offsetof(ModelXmlDocument370, comment_pool) == 0x2D8' in abi and 'PLRDOC1' not in auth,
 'shipping commit marks semantic state':'set_model_service_configuration_state' in config,
}
for k,v in checks.items():
    if not v: errors.append(k)
if errors:
    print('MODEL_SETTINGS_SEMANTIC_ABI: FAIL')
    for e in errors: print(' - '+e)
    sys.exit(1)
print(f'MODEL_SETTINGS_SEMANTIC_ABI: PASS ({len(checks)}/{len(checks)})')
