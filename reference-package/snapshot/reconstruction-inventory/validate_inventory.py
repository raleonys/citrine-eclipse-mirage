"""Validate inventory invariants against independent retained counts and files."""
from pathlib import Path
import ast
import csv
import hashlib
import json
import re

HERE = Path(__file__).resolve().parent
OUT = HERE.parent
checks = 0

def check(value, message):
    global checks
    checks += 1
    if not value:
        raise AssertionError(message)

def load(name):
    return json.loads((HERE / name).read_text(encoding='utf-8-sig'))

summary, rows = load('summary.json'), load('functions.json')
probe = load('independent-probe.json')
current_probe = load('current-object-probe.json')
check(summary['priority_order'] == [1, 2, 3, 5, 4], 'Human priority order')
check(len(rows) == len({r['key'] for r in rows}) == 2381, 'Artifact/address keys unique')
expected = {'anadius64': 2076, 'version_outer': 198, 'embedded_preloader': 107}
for artifact, count in expected.items():
    selected = [r for r in rows if r['artifact'] == artifact]
    check(len(selected) == count, 'Canonical count ' + artifact)
    check(sum(summary['artifacts'][artifact]['source_presence'].values()) == count, 'Exclusive source-state totals')
check(summary['anadius_local_thunks'] == probe['local_thunks'] == 45, 'Independent thunk count')
check(summary['external_imports_excluded'] == probe['external_not_in_denominator'] == 170, 'External count excluded')
check(probe['main_production_objects'] == 177 and probe['object_tags_at_canonical_entry'] == 435, 'Historical independent baseline preserved')
check(summary['build']['native_objects'] == current_probe['main_production_objects'] == 274, '274 current main objects')
check(summary['artifacts']['anadius64']['compiled_symbol_correlated_entries'] == current_probe['object_tags_at_canonical_entry'] == 505, 'Current raw object tag correlation includes three explicit projection boundaries')
check(len([r for r in rows if r['address'] == '0x180003e80']) == 2, 'Overlapping VA kept in separate artifacts')
native = {r['address']: r for r in rows if r['artifact'] == 'anadius64'}
check(sum(r['source_presence'] == 'source_definition_mapped' for r in native.values()) == 501, 'B38 exact mapped canonical entries')
check(sum(r['normal_body_stage_claim'] for r in native.values()) == 139, 'B38 exact explicit normal-body claims')
for address in ('0x180029df0', '0x180029590', '0x180029660'):
    check(any(d['file'] == 'anadius-accept-helpers-source/helpers.cpp' for d in native[address]['definitions']), 'B38 actual native helper source mapped')
    check(native[address]['normal_body_stage_claim'], 'B38 full normal-body stage claim')
for address in ('0x180021a10', '0x180025720', '0x1800270e0'):
    check(any(d['file'] == 'anadius-session-flow-source/flow.cpp' for d in native[address]['definitions']), 'B37 exact native flow source mapped')
    check(native[address]['normal_body_stage_claim'], 'B37 full normal-body stage claim')
check(native['0x180025720']['definition_variants'] == 3, 'B37 adds full025720 as third definition without hiding coarse variants')
check({(d['file'], d['symbol']) for d in native['0x180025720']['definitions'] if d['file'] == 'anadius-server-source/server.cpp'} == {('anadius-server-source/server.cpp', 'Session::start_025720'), ('anadius-server-source/server.cpp', 'accept_completion_025720')}, 'B37 keeps both historical coarse025720 variants')
for address in ('0x180021b00', '0x1800271a0'):
    check(any(d['file'] == 'anadius-accept-source/accept.cpp' for d in native[address]['definitions']), 'B36 native algorithm mapped to accept.cpp')
    check(native[address]['normal_body_stage_claim'], 'B36 explicit new native normal-body claim')
for address, file in (('0x1800261d0', 'composed_write.cpp'), ('0x180023dd0', 'environment_constructor.cpp'),
                      ('0x180025a70', 'write_finish.cpp'), ('0x18002a500', 'queued_write.cpp'), ('0x180021d10','async_receive.cpp'),
                      ('0x180010040','receive_buffer.cpp'),('0x180008350','receive_buffer.cpp'),('0x180025c40','composed_read.cpp'),
                      ('0x180025480','frame_read.cpp'),('0x1800280d0','frame_read.cpp'),
                      ('0x180021770','message_dispatch.cpp'),('0x180028d30','message_dispatch.cpp'),
                      ('0x180021420','response_dispatch.cpp'),('0x180028270','response_dispatch.cpp'),('0x180022240','response_dispatch.cpp'),
                      ('0x180020cb0','owned_response.cpp'),('0x180022280','owned_response.cpp'),('0x1800219d0','owned_response.cpp'),
                      ('0x180009eb0','response_containers.cpp'),('0x180026670','response_containers.cpp'),
                      ('0x18003ce30','request_mutation.cpp'),('0x180034730','request_mutation.cpp'),
                      ('0x18002b940','request_growth.cpp'),('0x18003d190','response_hex.cpp'),('0x180012350','append_growth.cpp'),('0x180007e30','append_growth.cpp'),('0x180042480','prefix_cipher.cpp'),('0x180042610','prefix_cipher.cpp'),('0x180045620','prefix_cipher.cpp'),('0x180045630','prefix_cipher.cpp'),('0x1800454e0','prefix_cipher.cpp'),('0x180045580','prefix_cipher.cpp'),('0x1800423d0','cipher_session.cpp'),('0x180042760','cipher_session.cpp'),('0x180042780','cipher_session.cpp'),('0x18002bc70','cipher_session.cpp'),('0x180032d00','native_dispatch.cpp'),('0x180001830','native_document.cpp'),('0x180004280','native_document.cpp'),('0x180001e00','native_document.cpp'),('0x180002e60','document_clear.cpp'),('0x180003110','document_clear.cpp'),('0x180003030','node_delete.cpp'),('0x180003240','node_delete.cpp'),('0x1800033a0','pool_release.cpp'),('0x180005a20','pool_release.cpp'),('0x180002b10','pool_allocate.cpp'),('0x180002600','pool_allocate.cpp'),('0x180002960','pool_allocate.cpp'),('0x1800027b0','pool_allocate.cpp'),('0x180005440','pool_append.cpp'),('0x180002d70','pool_clear.cpp'),('0x180002dc0','pool_clear.cpp'),('0x180002e10','pool_clear.cpp'),('0x180001bb0','pool_destruct.cpp'),('0x180001c30','pool_destruct.cpp'),('0x180001cb0','pool_destruct.cpp'),('0x180001d30','pool_destruct.cpp'),('0x180001ea0','node_base.cpp'),('0x1800041b0','parse_entry.cpp'),('0x180005820','document_error.cpp'),('0x180004970','node_parse.cpp'),('0x1800038b0','node_identify.cpp'),('0x180001590','node_constructors.cpp'),('0x180001610','node_constructors.cpp'),('0x180001690','node_constructors.cpp'),('0x180001720','node_constructors.cpp'),('0x1800017b0','node_constructors.cpp'),('0x180003aa0','node_insertion.cpp'),('0x1800035e0','node_string.cpp'),('0x180023d20','move.cpp'),('0x180028950','move.cpp'),('0x180023000','owner.cpp'),('0x180026880','dispose.cpp'),('0x180026570','dispose.cpp'),('0x180017420','dispose.cpp'),('0x180026290','dispose.cpp'),('0x180021b00','accept.cpp'),('0x1800271a0','accept.cpp'),('0x180021a10','flow.cpp'),('0x180025720','flow.cpp'),('0x1800270e0','flow.cpp')):
    check(any(d['file'].endswith('/' + file) for d in native[address]['definitions']), 'Latest source closure mapped')
    check(native[address]['normal_body_stage_claim'], 'Explicit normal-body claim preserved')
check(any(d['file'].endswith('/message_core_bridge.cpp') for d in native['0x180033d70']['definitions']), 'Selected message adapter mapped')
check(native['0x180033d70']['normal_body_stage_claim'], 'New N9 typed normal body claim independent of historical adapter')
check(any(d['file'].endswith('/native_message.cpp') for d in native['0x180033d70']['definitions']), 'N9 typed source and caller bridge mapped')
check(native['0x18002a500']['definition_variants'] >= 3, 'Coarse model/body/API bridge not collapsed into a false one-to-one mapping')
for address in ('0x18004b0f8','0x18000ce80','0x18000cd30'):
    check(native[address]['source_presence'] == 'bounded_projection_boundary_only' and not native[address]['normal_body_stage_claim'], 'Unsupported local-buffer boundary is not a recovered native body')
    check(all(d['mapping_basis'] == 'bounded_projection_boundary_definition' for d in native[address]['definitions']), 'Explicit boundary classification')
for address in ('0x180046a00', '0x1800466e0', '0x18000af90'):
    check(native[address]['source_presence'] == 'selected_upstream_adapter_only', 'Adapter is not a recovered body')
for r in rows:
    check(r['abi_equivalence'] == 'not_established', 'No fabricated native equivalence')
    check(r['binding_status'] == 'requires_explicit_dependency_review', 'No automatic provider closure claim')
    for d in r['definitions']:
        check((OUT / d['file']).is_file(), 'Mapped source file exists')
        check(hashlib.sha256((OUT / d['file']).read_bytes()).hexdigest() == d['sha256'], 'Mapped source bytes')

inputs = load('inputs.json')['files']
for rel, record in inputs.items():
    data = (OUT / rel).read_bytes()
    check(len(data) == record['bytes'] and hashlib.sha256(data).hexdigest() == record['sha256'], 'Generator input unchanged ' + rel)
with (HERE / 'functions.csv').open(encoding='utf-8-sig', newline='') as stream:
    csv_rows = list(csv.DictReader(stream))
check(len(csv_rows) == len(rows), 'CSV/JSON count')
check([(r['artifact'], r['address'], r['source_presence']) for r in csv_rows] == [(r['artifact'], r['address'], r['source_presence']) for r in rows], 'CSV exact row identities/statuses')
extra = load('outside-canonical.json')
check(len(extra) == summary['outside_canonical_tagged_addresses'] == 24, 'Noncanonical tags isolated')
check(all(not any(r['artifact'] == x['artifact'] and r['address'] == x['address'] for r in rows) for x in extra), 'No gap/data tag denominator inflation')

# B2/B3 forwarding definitions remain visible in a separate ledger, not native
# address coverage. Validate the actual output against explicit source identity.
composition = load('composition-only.json')
expected_composition = {'anadius-accept-helpers-source/bindings.cpp', 'anadius-accept-helpers-source/bindings.hpp', 'anadius-accept-source/owned.cpp', 'anadius-accept-source/owned.hpp', 'anadius-accept-stop-source/stop.cpp', 'anadius-accept-stop-source/stop.hpp', 'anadius-application-graph-source/graph.cpp', 'anadius-application-graph-source/graph.hpp', 'anadius-application-worker-source/worker.cpp', 'anadius-application-worker-source/worker.hpp', 'anadius-bootstrap-backend-source/backend.cpp', 'anadius-bootstrap-backend-source/backend.hpp', 'anadius-bootstrap-backend-source/tracked_threads.cpp', 'anadius-bootstrap-backend-source/tracked_threads.hpp', 'anadius-bootstrap-owner-source/owner.cpp', 'anadius-bootstrap-owner-source/owner.hpp', 'anadius-bootstrap-policies-source/config_platform.cpp', 'anadius-bootstrap-policies-source/config_platform.hpp', 'anadius-bootstrap-policies-source/services.cpp', 'anadius-bootstrap-policies-source/services.hpp', 'anadius-bootstrap-providers-source/native_threads.cpp', 'anadius-bootstrap-providers-source/native_threads.hpp', 'anadius-bootstrap-providers-source/providers.cpp', 'anadius-bootstrap-providers-source/providers.hpp', 'anadius-bootstrap-runtime-source/callbacks.cpp', 'anadius-bootstrap-runtime-source/callbacks.hpp', 'anadius-bootstrap-runtime-source/factory.cpp', 'anadius-bootstrap-runtime-source/factory.hpp', 'anadius-callback-admission-source/admission.cpp', 'anadius-callback-admission-source/admission.hpp', 'anadius-callback-identity-source/identity.cpp', 'anadius-callback-identity-source/identity.hpp', 'anadius-callback-policy-source/policy.cpp', 'anadius-callback-policy-source/policy.hpp', 'anadius-callback-provider-source/provider.cpp', 'anadius-callback-provider-source/provider.hpp', 'anadius-config-defaults-source/defaults.cpp', 'anadius-config-defaults-source/defaults.hpp', 'anadius-config-iteration-source/thread_error.cpp', 'anadius-config-iteration-source/thread_error.hpp', 'anadius-config-logging-source/logging.cpp', 'anadius-config-logging-source/logging.hpp', 'anadius-config-storage-source/embedded_png_data.hpp', 'anadius-config-storage-source/filesystem.cpp', 'anadius-config-storage-source/filesystem.hpp', 'anadius-config-storage-source/persistence.cpp', 'anadius-config-storage-source/persistence.hpp', 'anadius-configuration-factory-source/factory.cpp', 'anadius-configuration-factory-source/factory.hpp', 'anadius-foreign-callback-source/foreign.cpp', 'anadius-foreign-callback-source/foreign.hpp', 'anadius-host-platform-source/platform.cpp', 'anadius-host-platform-source/platform.hpp', 'anadius-io-memory-source/io_memory.cpp', 'anadius-io-memory-source/io_memory.hpp', 'anadius-library-effects-source/dialog.cpp', 'anadius-library-effects-source/dialog.hpp', 'anadius-library-effects-source/effects.cpp', 'anadius-library-effects-source/effects.hpp', 'anadius-native-context-source/context.cpp', 'anadius-native-context-source/context.hpp', 'anadius-native-profile-source/native.cpp', 'anadius-native-profile-source/native.hpp', 'anadius-online-factory-source/factory.cpp', 'anadius-online-factory-source/factory.hpp', 'anadius-online-object-source/object.cpp', 'anadius-online-object-source/object.hpp', 'anadius-online-platform-source/platform.cpp', 'anadius-online-platform-source/platform.hpp', 'anadius-online-query-source/query.cpp', 'anadius-online-query-source/query.hpp', 'anadius-online-version-source/version.cpp', 'anadius-online-version-source/version.hpp', 'anadius-original-checkpoint-source/checkpoint.cpp', 'anadius-original-checkpoint-source/checkpoint.hpp', 'anadius-original-provider-source/originals.cpp', 'anadius-original-provider-source/originals.hpp', 'anadius-owned-deque-source/deque.cpp', 'anadius-owned-deque-source/deque.hpp', 'anadius-owned-environment-source/environment.cpp', 'anadius-owned-environment-source/environment.hpp', 'anadius-owned-jobs-source/jobs.cpp', 'anadius-owned-jobs-source/jobs.hpp', 'anadius-owned-server-source/server.cpp', 'anadius-owned-server-source/server.hpp', 'anadius-server-runtime-source/resources.cpp', 'anadius-server-runtime-source/resources.hpp', 'anadius-server-runtime-source/runtime.cpp', 'anadius-server-runtime-source/runtime.hpp', 'anadius-services-completion-source/completion.cpp', 'anadius-services-completion-source/completion.hpp', 'anadius-services-completion-source/exit_wait.cpp', 'anadius-services-completion-source/exit_wait.hpp', 'anadius-services-factory-source/factory.cpp', 'anadius-services-factory-source/factory.hpp', 'anadius-session-flow-source/coherent_io.cpp', 'anadius-session-flow-source/coherent_io.hpp', 'anadius-session-memory-source/memory.cpp', 'anadius-session-memory-source/memory.hpp', 'anadius-session-policy-source/policy.cpp', 'anadius-session-policy-source/policy.hpp', 'anadius-session-profile-source/profile.cpp', 'anadius-session-profile-source/profile.hpp', 'anadius-session-profile-source/strings.cpp', 'anadius-session-profile-source/strings.hpp', 'anadius-shared-log-source/stream.cpp', 'anadius-shared-log-source/stream.hpp', 'anadius-startup-publication-source/publication.cpp', 'anadius-startup-publication-source/publication.hpp', 'anadius-thread-domain-source/domain.cpp', 'anadius-thread-domain-source/domain.hpp', 'anadius-thread-domain-source/initial_locale.cpp', 'anadius-thread-domain-source/initial_locale.hpp', 'anadius-thread-domain-source/runtime.cpp', 'anadius-thread-domain-source/runtime.hpp', 'anadius-worker-handoff-source/handoff.cpp', 'anadius-worker-handoff-source/handoff.hpp'}
check({x['file'] for x in composition['excluded_sources']} == expected_composition, 'Explicit one-hundred-seventeen-file B2/B3/B4/B5/B6/B7/B8/B9/B10/B11/B12/B13/B14/B15/B16/B17/B18/B19/B20/B21/B22/B23/B24/B25/B26/B27/B28 composition policy')
check(summary['composition_only_source_files'] == 117 and summary['composition_only_address_definition_variants'] == len(composition['definitions']) > 0, 'Composition observations retained')
check(composition['new_native_bodies'] == composition['new_native_closures'] == 0, 'Forwarding adds no native recovery')
check(all(d['file'] not in expected_composition for row in rows for d in row['definitions']), 'Forwarders excluded from canonical mappings')
for d in composition['definitions']:
    check(d['mapping_basis'] == 'composition_forwarding_not_native_body' and not d['native_body_claim'], 'Forwarder label is explicit')
    check(hashlib.sha256((OUT / d['file']).read_bytes()).hexdigest() == d['sha256'], 'Forwarder source bytes')
check(current_probe['composition_only_source_files'] == ['anadius-bootstrap-owner-source/owner.cpp', 'anadius-bootstrap-backend-source/backend.cpp', 'anadius-bootstrap-backend-source/tracked_threads.cpp', 'anadius-bootstrap-runtime-source/callbacks.cpp', 'anadius-bootstrap-runtime-source/factory.cpp', 'anadius-bootstrap-providers-source/providers.cpp', 'anadius-bootstrap-providers-source/native_threads.cpp', 'anadius-thread-domain-source/domain.cpp', 'anadius-thread-domain-source/initial_locale.cpp', 'anadius-thread-domain-source/runtime.cpp', 'anadius-bootstrap-policies-source/services.cpp', 'anadius-bootstrap-policies-source/config_platform.cpp', 'anadius-config-storage-source/filesystem.cpp', 'anadius-config-storage-source/persistence.cpp', 'anadius-config-iteration-source/thread_error.cpp', 'anadius-services-completion-source/exit_wait.cpp', 'anadius-services-completion-source/completion.cpp', 'anadius-config-logging-source/logging.cpp', 'anadius-library-effects-source/dialog.cpp', 'anadius-library-effects-source/effects.cpp', 'anadius-online-platform-source/platform.cpp', 'anadius-host-platform-source/platform.cpp', 'anadius-native-context-source/context.cpp', 'anadius-callback-policy-source/policy.cpp', 'anadius-callback-admission-source/admission.cpp', 'anadius-foreign-callback-source/foreign.cpp', 'anadius-startup-publication-source/publication.cpp', 'anadius-shared-log-source/stream.cpp', 'anadius-callback-identity-source/identity.cpp', 'anadius-callback-provider-source/provider.cpp', 'anadius-original-provider-source/originals.cpp', 'anadius-original-checkpoint-source/checkpoint.cpp', 'anadius-online-version-source/version.cpp', 'anadius-online-object-source/object.cpp', 'anadius-online-query-source/query.cpp', 'anadius-config-defaults-source/defaults.cpp', 'anadius-session-memory-source/memory.cpp', 'anadius-io-memory-source/io_memory.cpp', 'anadius-owned-server-source/server.cpp', 'anadius-accept-source/owned.cpp', 'anadius-session-flow-source/coherent_io.cpp', 'anadius-accept-helpers-source/bindings.cpp', 'anadius-server-runtime-source/runtime.cpp', 'anadius-server-runtime-source/resources.cpp', 'anadius-session-profile-source/profile.cpp', 'anadius-session-profile-source/strings.cpp', 'anadius-native-profile-source/native.cpp', 'anadius-owned-deque-source/deque.cpp', 'anadius-owned-environment-source/environment.cpp', 'anadius-owned-jobs-source/jobs.cpp', 'anadius-session-policy-source/policy.cpp', 'anadius-application-graph-source/graph.cpp', 'anadius-worker-handoff-source/handoff.cpp', 'anadius-application-worker-source/worker.cpp', 'anadius-accept-stop-source/stop.cpp', 'anadius-configuration-factory-source/factory.cpp', 'anadius-services-factory-source/factory.cpp', 'anadius-online-factory-source/factory.cpp'] and current_probe['composition_only_objects'] == ['bootstrap_owner.o', 'bootstrap_backend.o', 'bootstrap_tracking.o', 'bootstrap_callbacks.o', 'bootstrap_factory.o', 'bootstrap_providers.o', 'bootstrap_thread_start.o', 'thread_domain.o', 'thread_domain_locale.o', 'thread_domain_runtime.o', 'bootstrap_services.o', 'configuration_platform.o', 'configuration_filesystem.o', 'configuration_persistence.o', 'thread_error_policy.o', 'exit_wait_completion.o', 'services_completion.o', 'config_logging.o', 'library_dialog.o', 'library_effects.o', 'online_platform.o', 'host_platform.o', 'native_context.o', 'callback_policy.o', 'callback_admission.o', 'foreign_callback.o', 'startup_publication.o', 'shared_log.o', 'callback_identity.o', 'callback_provider.o', 'original_provider.o', 'original_checkpoint.o', 'online_version.o', 'online_object.o', 'online_query.o', 'config_defaults.o', 'session_memory.o', 'io_memory.o', 'owned_server.o', 'owned_accept.o', 'coherent_io.o', 'accept_bindings.o', 'server_runtime.o', 'server_runtime_resources.o', 'session_profile.o', 'session_profile_strings.o', 'native_profile.o', 'owned_deque.o', 'owned_environment.o', 'owned_jobs.o', 'owned_session_policy.o', 'application_graph.o', 'worker_handoff.o', 'application_worker.o', 'accept_stop.o', 'configuration_factory.o', 'services_factory.o', 'online_factory.o'], 'Explicit object identity exclusions')
check(current_probe['raw_all_object_tags_at_canonical_entry'] >= current_probe['object_tags_at_canonical_entry'] == 505 and current_probe['composition_only_tagged_symbol_lines'] > 0, 'Raw-all tags retained separately from comparable505')

# Independent syntactic fixtures exercise false-positive classes rather than
# repeating the generator algorithm. Only the two parser helpers are loaded.
tree = ast.parse((HERE / 'generate_inventory.py').read_text(encoding='utf-8'))
nodes = [n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name in ('mask_cpp', 'definitions', 'is_composition_only')]
policy = next(n for n in tree.body if isinstance(n, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'COMPOSITION_ONLY_SOURCES' for t in n.targets))
namespace = {'re': re, 'COMPOSITION_ONLY_SOURCES': ast.literal_eval(policy.value)}
exec(compile(ast.Module(body=nodes, type_ignores=[]), 'inventory-parser-fixtures', 'exec'), namespace)
parse = namespace['definitions']
for text in ('void f_025c40() = 0;', 'x.f_025c40();', '// void f_025c40() {}',
             '/* void f_025c40() {} */', 'const char* s="void f_025c40() {}";',
             'if(f_025c40()) { x(); }'):
    check(not list(parse(text)), 'Declaration/reference/comment/string excluded')
found = list(parse('void A::real_025c40(int x) noexcept { auto f=[](){x_021d10();}; f(); }'))
check(len(found) == 1 and found[0]['symbol'] == 'A::real_025c40', 'Definition with nested lambda accepted exactly once')

forwarding_fixture = 'void BootstrapFacade::guard_return_017a30() { services.guard_return(); }'
check(len(list(parse(forwarding_fixture))) == 1, 'Suffix parser still sees forwarded identity')
check(namespace['is_composition_only']('anadius-bootstrap-owner-source/owner.cpp'), 'Explicit forwarding file is separated')
check(namespace['is_composition_only']('anadius-bootstrap-backend-source/backend.cpp'), 'B3 application source routing remains composition')
check(namespace['is_composition_only']('anadius-bootstrap-backend-source/tracked_threads.cpp'), 'B3 SDK join bookkeeping remains composition')
check(not namespace['is_composition_only']('anadius-source/bootstrap.cpp'), 'Unrelated reconstructed source stays eligible')
check(not namespace['is_composition_only']('anadius-socket-source/node_string.cpp'), 'Latest original body remains eligible')
check(not namespace['is_composition_only']('anadius-accept-source/accept.hpp') and not namespace['is_composition_only']('anadius-accept-source/accept.cpp'), 'B36 actual native accept bodies stay eligible')
check(namespace['is_composition_only']('anadius-accept-source/owned.hpp') and namespace['is_composition_only']('anadius-accept-source/owned.cpp'), 'B36 owned adapter is composition only')
check(not namespace['is_composition_only']('anadius-session-flow-source/flow.hpp') and not namespace['is_composition_only']('anadius-session-flow-source/flow.cpp'), 'B37 actual native flow body sources stay eligible')
check(namespace['is_composition_only']('anadius-session-flow-source/coherent_io.hpp') and namespace['is_composition_only']('anadius-session-flow-source/coherent_io.cpp'), 'B37 coherent adapter remains composition only')
check(not namespace['is_composition_only']('anadius-accept-helpers-source/helpers.hpp') and not namespace['is_composition_only']('anadius-accept-helpers-source/helpers.cpp'), 'B38 native helper sources remain eligible')
check(namespace['is_composition_only']('anadius-accept-helpers-source/bindings.hpp') and namespace['is_composition_only']('anadius-accept-helpers-source/bindings.cpp'), 'B38 selected bindings are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-server-runtime-source/runtime.hpp', 'anadius-server-runtime-source/runtime.cpp', 'anadius-server-runtime-source/resources.hpp', 'anadius-server-runtime-source/resources.cpp']), 'B39 all new runtime/resources production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-session-profile-source/profile.hpp', 'anadius-session-profile-source/profile.cpp', 'anadius-session-profile-source/strings.hpp', 'anadius-session-profile-source/strings.cpp']), 'B40 all new session profile/string production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-native-profile-source/native.hpp', 'anadius-native-profile-source/native.cpp']), 'B41 new native-profile production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-owned-deque-source/deque.hpp', 'anadius-owned-deque-source/deque.cpp']), 'B42 owned-deque production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-owned-environment-source/environment.hpp', 'anadius-owned-environment-source/environment.cpp']), 'B43 owned-environment production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-owned-jobs-source/jobs.hpp', 'anadius-owned-jobs-source/jobs.cpp']), 'B44 owned-jobs production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-session-policy-source/policy.hpp', 'anadius-session-policy-source/policy.cpp']), 'B45 session-policy production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-application-graph-source/graph.hpp', 'anadius-application-graph-source/graph.cpp']), 'B46 application-graph production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-worker-handoff-source/handoff.hpp', 'anadius-worker-handoff-source/handoff.cpp']), 'B47 worker-handoff production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-application-worker-source/worker.hpp', 'anadius-application-worker-source/worker.cpp']), 'B48 application-worker production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-accept-stop-source/stop.hpp', 'anadius-accept-stop-source/stop.cpp']), 'B49 accept-stop production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-configuration-factory-source/factory.hpp', 'anadius-configuration-factory-source/factory.cpp']), 'B52 configuration factory production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-services-factory-source/factory.hpp', 'anadius-services-factory-source/factory.cpp']), 'B53 remaining Services factory production files are composition only')
check(all(namespace['is_composition_only'](p) for p in ['anadius-online-factory-source/factory.hpp', 'anadius-online-factory-source/factory.cpp']), 'B54 online factory production files are composition only')

report = {'result': 'PASS', 'scope': 'Offline generated presence inventory, input integrity, independent canonical/object-count cross-check and parser fixtures',
          'checks': checks, 'canonical_rows': len(rows), 'priority_order': summary['priority_order'],
          'production_sources_changed': True, 'new_native_bodies': 0, 'original_execution': 'none',
          'independent_scope': 'Historical independent canonical/object baseline; current object probe rerun by same agent; independent final review pending',
          'limitations': summary['limitations']}
(HERE / 'validation.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(json.dumps({k: report[k] for k in ('result', 'checks', 'canonical_rows', 'priority_order')}, ensure_ascii=False))
