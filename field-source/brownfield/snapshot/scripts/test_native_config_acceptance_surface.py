#!/usr/bin/env python3
from pathlib import Path
import sys,json
r=Path(__file__).resolve().parents[1]
host=(r/'tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp').read_text(); native=(r/'tests/native/native_acceptance_exports.cpp').read_text(); exports=(r/'exports/preloader_l_native_acceptance.def').read_text(); build=(r/'scripts/build_orchestration/build_native_acceptance.ps1').read_text(); status=json.loads((r/'RELEASE_STATE.json').read_text())
checks={
 'config scenario export':'preloader_l_native_acceptance_config_scenario' in native and 'preloader_l_native_acceptance_config_scenario' in exports,
 'UTF16 direct policy':'set_runtime_current_directory_wide_policy' in native,
 'ascii SSO7':'path_ascii_sso7' in host and 'path_ascii_sso7' in build,
 'unicode heap':'path_unicode_heap' in host and 'path_unicode_heap' in build,
 'long replace':'path_long_replace' in host and 'path_long_replace' in build,
 'XML no BOM':'xml_no_bom' in host and 'xml_no_bom' in build,
 'XML BOM':'xml_bom' in host and 'xml_bom' in build,
 'XML large':'xml_large' in host and 'xml_large' in build,
 'XML invalid retain':'xml_invalid_retain' in host and 'xml_invalid_retain' in build,
 'XML replace':'xml_replace' in host and 'xml_replace' in build,
 'XML clear':'xml_clear' in host and 'xml_clear' in build,
 'document inspection':'service->config.document.char_buffer' in native and 'service->config.document.node.document' in native,
 'path inspection':'service->config.path.capacity' in native and 'wide_path_equals' in native,
 'Windows execution truth classified':status.get('release_type')=='HOTFIX_RELEASE' and status.get('hotfix_ordinal')==16 and status.get('windows_tested_checkpoint',{}).get('ids07','').startswith('PASS_USER_WINDOWS_LOGS_SHIPPING_DLL_SHA256_') and all(status.get('human_confirmation',{}).get(g)=='USER_REPORTED_NORMAL_GAMEPLAY_SMOKE_NOT_INSTRUMENTED_EXACT_DLL' for g in ('collegefb27','madden27')),
}
fail=[k for k,v in checks.items() if not v]
if fail:
 print('NATIVE_CONFIG_ACCEPTANCE_SURFACE: FAIL'); [print(' -',x) for x in fail]; sys.exit(1)
print(f'NATIVE_CONFIG_ACCEPTANCE_SURFACE: PASS ({len(checks)}/{len(checks)})')
