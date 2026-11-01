#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
boot=(r/'src/runtime/bootstrap_thread.cpp').read_text()
hdr=(r/'include/preloader_l/runtime/bootstrap_thread.hpp').read_text()
mh=(r/'src/platform/minhook_runtime.cpp').read_text()
mhh=(r/'include/preloader_l/platform/minhook_runtime.hpp').read_text()
checks={
 'public timeout result':'hook_quiescence_timeout = 11' in hdr,
 'end to end contract':'end-to-end budget' in hdr and '0xffffffff means infinite' in hdr,
 'single deadline':'make_shutdown_deadline(timeout_ms)' in boot and 'remaining_shutdown_timeout(shutdown_deadline)' in boot,
 'bootstrap uses remaining budget':'wait_release_bootstrap_stopped(\n        remaining_shutdown_timeout(shutdown_deadline))' in boot,
 'hook teardown uses remaining budget':'teardown_bootstrap_failure_hooks(\n        remaining_shutdown_timeout(shutdown_deadline))' in boot,
 'bounded rollback API':'rollback_minhook_batch_transaction_with_timeout' in mhh and 'quiescence_timeout = 5' in mhh,
 'draining state':'BatchTransactionMode::draining' in mh,
 'timeout retains transaction':'return MinHookBatchTransactionStatus::quiescence_timeout;' in mh and 'reset_batch_transaction();' in mh,
 'retry drains without reapply':'if (g_batch_transaction.mode == BatchTransactionMode::draining)' in mh,
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('PREAUDIT_SHUTDOWN_TIMEOUT_CONTRACT: FAIL');[print(' -',x) for x in failed];sys.exit(1)
print(f'PREAUDIT_SHUTDOWN_TIMEOUT_CONTRACT: PASS ({len(checks)}/{len(checks)})')
