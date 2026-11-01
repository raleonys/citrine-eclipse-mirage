#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
policy=(r/'src/platform/environment_policy.cpp').read_text()
hdr=(r/'include/preloader_l/platform/process_environment.hpp').read_text()
win=(r/'src/platform/process_environment.cpp').read_text()
boot=(r/'src/runtime/bootstrap_thread.cpp').read_text()
checks={
 'process restore declared':'restore_process_environment(uint32_t written_mask' in hdr,
 'generic restore exists':'EnvironmentStatus restore_environment(' in policy,
 'restore considers only written bits':'if ((written_mask & bit) == 0u) continue;' in policy,
 'restore deletes rather than overwrites':'backend->set(names[i], nullptr, backend->context)' in policy,
 'restore reports exact successful mask':'*restored_mask |= bit;' in policy,
 'windows wrapper binds same backend':'return restore_environment(&backend, written_mask, restored_mask);' in win,
 'restore occurs before release teardown':boot.index('restore_process_environment(') < boot.index('release_release_service(&service)'),
 'restore helper verifies status+remaining mask':'status == preloader_l::platform::EnvironmentStatus::ok && remaining == 0' in boot and 'const uint32_t remaining = written_mask & ~restored_mask;' in boot and 'g_environment_restore_mask' in boot,
 'restore failure becomes shutdown_failed':'if (!environment_ok || !unwind_ok)' in boot and 'result = preloader_l::runtime::BootstrapRunResult::shutdown_failed;' in boot,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('F1420_ENVIRONMENT_RESTORE: FAIL'); [print(' -',x) for x in failed]; sys.exit(1)
print(f'F1420_ENVIRONMENT_RESTORE: PASS ({len(checks)}/{len(checks)})')
