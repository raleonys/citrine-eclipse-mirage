#!/usr/bin/env python3
"""HF13: fail-closed Asio bridge retirement before service policy destruction."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
src = (ROOT / 'src/runtime/release_service_lifetime.cpp').read_text(encoding='utf-8')
bridge = (ROOT / 'src/runtime/asio_release_bridge.cpp').read_text(encoding='utf-8')
bootstrap = (ROOT / 'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8')
service = (ROOT / 'src/runtime/release_server.cpp').read_text(encoding='utf-8')
checks = []
def check(name, condition):
    checks.append((name, bool(condition)))

begin = src.index('shutdown_published_owner_if_idle()')
end = src.index('}\n}', begin)
body = src[begin:end]

gate = re.compile(
    r'auto\* adapter\s*=\s*preloader_l::runtime::model_service_adapter\(service\);'
    r'\s*if\s*\(adapter\s*&&\s*adapter->asio_bridge\s*&&'
    r'\s*!preloader_l::runtime::shutdown_release_asio_windows\(adapter\)\)'
    r'\s*return ReleaseServiceShutdownResult::resource_cleanup_pending;', re.S)

check('native bridge prototype available', '"asio_release_bridge.hpp"' in src)
check('complete native adapter type included', '"preloader_l/runtime/release_server.hpp"' in src)
check('gate uses true adapter and checks actual bridge', bool(gate.search(body)))
check('gate after strong-count precondition', body.index('owner.strong_count != 1') < body.index('adapter->asio_bridge'))
check('gate before events/config/store shutdown', body.index('adapter->asio_bridge') < body.index('clear_queued_events()'))
check('gate before derived state/policy retirement', body.index('adapter->asio_bridge') < body.index('prepare_release_worker_policy_retirement_windows'))
check('gate before reference release', body.index('adapter->asio_bridge') < body.index('reference_owner_release_alias(&g_published_owner)'))
check('bridge failure returns false preserving pointer', 'if (!bridge->request_stop()||!bridge->drain()) return false;' in bridge)
check('bridge nullptr only on completed drain', bridge.index('if (!bridge->request_stop()||!bridge->drain()) return false;') < bridge.index('context->asio_bridge=nullptr;'))
check('bootstrap failure uses common permanent-owner shutdown', 'shutdown_release_service_after_bootstrap_failure' in bootstrap)
check('external shutdown uses common permanent-owner shutdown', 'shutdown_release_service_after_quiescence' in bootstrap)
check('both owner-disposal callers share gate', src.count('return shutdown_published_owner_if_idle();') == 2)
check('original Asio backend shutdown still propagates failure', 'context && context->asio_bridge && !shutdown_release_asio_windows(context)' in service)
check('no ignore of Asio shutdown result inside owner gate', '(!preloader_l::runtime::shutdown_release_asio_windows(adapter))' not in body and bool(gate.search(body)))
# Negative tests: known unsound mutations must be rejected by the gate contract.
for name, bad in [
    ('missing bridge test', body.replace('adapter->asio_bridge &&', 'true &&', 1)),
    ('ignored failure', body.replace('!preloader_l::runtime::shutdown_release_asio_windows(adapter)', 'preloader_l::runtime::shutdown_release_asio_windows(adapter)', 1)),
    ('silent success on failure', body.replace(gate.search(body).group(0),gate.search(body).group(0).replace('return ReleaseServiceShutdownResult::resource_cleanup_pending;', 'return ReleaseServiceShutdownResult::shutdown;'), 1)),
    ('entire gate removed', gate.sub('', body, 1))]:
    check('reject ' + name, gate.search(bad) is None)

bad = [name for name, passed in checks if not passed]
if bad:
    print('ASIO_SERVICE_RETIREMENT: FAIL', len(checks)-len(bad), '/', len(checks))
    for name in bad: print('  -', name)
    sys.exit(1)
print('ASIO_SERVICE_RETIREMENT: PASS', len(checks), '/', len(checks))
