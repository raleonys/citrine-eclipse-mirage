"""Offline inventory of retained evidence and source presence, not equivalence."""
from pathlib import Path
import csv
import hashlib
import json
import re
from collections import Counter, defaultdict

OUT = Path(__file__).resolve().parents[1]
DEST = Path(__file__).resolve().parent
INPUTS = {}
SHA = {
    'anadius64': '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c',
    'version_outer': '07f4da9e7cd87421cabcf6b3bbb5083760e5eae33b357ef2b019216677b3c543',
    'embedded_preloader': '36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846',
}

# Address suffixes in these new composition adapters label a delegated contract;
# they are not additional recovered native function bodies or mapping evidence.
COMPOSITION_ONLY_SOURCES = {'anadius-bootstrap-owner-source/owner.cpp': 'B2 explicit composition/forwarding; lower native policies remain mandatory', 'anadius-bootstrap-owner-source/owner.hpp': 'B2 owner/facade type and lifetime composition; no native body claim', 'anadius-bootstrap-backend-source/backend.cpp': 'B3 existing application source routes and owned backend composition; no additional native body claim', 'anadius-bootstrap-backend-source/backend.hpp': 'B3 concrete backend ownership contract; lower provider implementations remain mandatory', 'anadius-bootstrap-backend-source/tracked_threads.cpp': 'B3 thread Effects forwarding and new join bookkeeping; not additional recovered thread bodies', 'anadius-bootstrap-backend-source/tracked_threads.hpp': 'B3 tracked thread type and typed SDK join boundary; no native body claim', 'anadius-bootstrap-runtime-source/callbacks.hpp': 'B4 callback lifetime contract; no native body claim', 'anadius-bootstrap-runtime-source/callbacks.cpp': 'B4 compiler CRT registration and Services forwarding; new composition only', 'anadius-bootstrap-runtime-source/factory.hpp': 'B4 resource assembly and SDK execution boundary; no native body claim', 'anadius-bootstrap-runtime-source/factory.cpp': 'B4 source-owned runtime assembly, native execution and controlled shutdown; no native body claim', 'anadius-bootstrap-providers-source/providers.hpp': 'B5 configuration and compiler-console composition contract; no native body claim', 'anadius-bootstrap-providers-source/providers.cpp': 'B5 existing path/encoding source and new compiler CRT/SDK policies; no additional native body', 'anadius-bootstrap-providers-source/native_threads.hpp': 'B5 per-launch CRT thread callback lifetime contract; no native body claim', 'anadius-bootstrap-providers-source/native_threads.cpp': 'B5 thread wrapper and recovered ThreadEffects selection; no additional native thread body', 'anadius-thread-domain-source/domain.hpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-thread-domain-source/domain.cpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-thread-domain-source/initial_locale.hpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-thread-domain-source/initial_locale.cpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-thread-domain-source/runtime.hpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-thread-domain-source/runtime.cpp': 'B6 owned initial-locale domain and explicit runtime/heap/FLS policy composition; no native body claim', 'anadius-bootstrap-policies-source/services.hpp': 'B7 selected Services and configuration SDK/source policies; no native body claim', 'anadius-bootstrap-policies-source/services.cpp': 'B7 selected Services and configuration SDK/source policies; no native body claim', 'anadius-bootstrap-policies-source/config_platform.hpp': 'B7 selected Services and configuration SDK/source policies; no native body claim', 'anadius-bootstrap-policies-source/config_platform.cpp': 'B7 selected Services and configuration SDK/source policies; no native body claim', 'anadius-config-storage-source/filesystem.hpp': 'B8 selected filesystem/persistence/retained PNG composition; no native body claim', 'anadius-config-storage-source/filesystem.cpp': 'B8 selected filesystem/persistence/retained PNG composition; no native body claim', 'anadius-config-storage-source/persistence.hpp': 'B8 selected filesystem/persistence/retained PNG composition; no native body claim', 'anadius-config-storage-source/persistence.cpp': 'B8 selected filesystem/persistence/retained PNG composition; no native body claim', 'anadius-config-storage-source/embedded_png_data.hpp': 'B8 selected filesystem/persistence/retained PNG composition; no native body claim', 'anadius-config-iteration-source/thread_error.hpp': 'B9 exact thread error table/compiler exception boundary composition; no native body claim', 'anadius-config-iteration-source/thread_error.cpp': 'B9 exact thread error table/compiler exception boundary composition; no native body claim', 'anadius-services-completion-source/exit_wait.hpp': 'B10 selected owned exit-wait/logger/Services completion policy; no native body claim', 'anadius-services-completion-source/exit_wait.cpp': 'B10 selected owned exit-wait/logger/Services completion policy; no native body claim', 'anadius-services-completion-source/completion.hpp': 'B10 selected owned exit-wait/logger/Services completion policy; no native body claim', 'anadius-services-completion-source/completion.cpp': 'B10 selected owned exit-wait/logger/Services completion policy; no native body claim', 'anadius-config-logging-source/logging.hpp': 'B11 configuration filename logging composition; no native body claim', 'anadius-config-logging-source/logging.cpp': 'B11 configuration filename logging composition; no native body claim', 'anadius-library-effects-source/dialog.hpp': 'B12 selected library log/dialog/error policy composition; no native body claim', 'anadius-library-effects-source/dialog.cpp': 'B12 selected library log/dialog/error policy composition; no native body claim', 'anadius-library-effects-source/effects.hpp': 'B12 selected library log/dialog/error policy composition; no native body claim', 'anadius-library-effects-source/effects.cpp': 'B12 selected library log/dialog/error policy composition; no native body claim', 'anadius-online-platform-source/platform.hpp': 'B13 owned online platform/OS/file/output composition; no native body claim', 'anadius-online-platform-source/platform.cpp': 'B13 owned online platform/OS/file/output composition; no native body claim', 'anadius-host-platform-source/platform.hpp': 'B14 owned host/cleanup SDK/log and existing-engine composition; no native body claim', 'anadius-host-platform-source/platform.cpp': 'B14 owned host/cleanup SDK/log and existing-engine composition; no native body claim', 'anadius-native-context-source/context.hpp': 'B15 exact moved-state native Context attachment composition; no native body claim', 'anadius-native-context-source/context.cpp': 'B15 exact moved-state native Context attachment composition; no native body claim', 'anadius-callback-policy-source/policy.hpp': 'B16 bounded concrete callback memory/CRT/log/wait policy composition; no native body claim', 'anadius-callback-policy-source/policy.cpp': 'B16 bounded concrete callback memory/CRT/log/wait policy composition; no native body claim', 'anadius-callback-admission-source/admission.hpp': 'B17 bounded controller admission enforcement and owner composition; no native body claim', 'anadius-callback-admission-source/admission.cpp': 'B17 bounded controller admission enforcement and owner composition; no native body claim', 'anadius-foreign-callback-source/foreign.hpp': 'B18 bounded sequential foreign callback PTD/publication/drain composition; no native body claim', 'anadius-foreign-callback-source/foreign.cpp': 'B18 bounded sequential foreign callback PTD/publication/drain composition; no native body claim', 'anadius-startup-publication-source/publication.hpp': 'B19 source startup milestone and matched callback composition; no native body claim', 'anadius-startup-publication-source/publication.cpp': 'B19 source startup milestone and matched callback composition; no native body claim', 'anadius-shared-log-source/stream.hpp': 'B20 selected shared compiler-ostream ownership and operation gate; no native body claim', 'anadius-shared-log-source/stream.cpp': 'B20 selected shared compiler-ostream ownership and operation gate; no native body claim', 'anadius-callback-identity-source/identity.hpp': 'B21 actual callback PTD checkpoints and checked encoding source composition; no native body claim', 'anadius-callback-identity-source/identity.cpp': 'B21 actual callback PTD checkpoints and checked encoding source composition; no native body claim', 'anadius-callback-provider-source/provider.hpp': 'B22 owned compiler locale and numerical encoding source composition; no native body claim', 'anadius-callback-provider-source/provider.cpp': 'B22 owned compiler locale and numerical encoding source composition; no native body claim', 'anadius-original-provider-source/originals.hpp': 'B23 original slot ownership and publication source composition; no native body claim', 'anadius-original-provider-source/originals.cpp': 'B23 original slot ownership and publication source composition; no native body claim', 'anadius-original-checkpoint-source/checkpoint.hpp': 'B24 actual PTD checks around selected original SDK source calls; no native body claim', 'anadius-original-checkpoint-source/checkpoint.cpp': 'B24 actual PTD checks around selected original SDK source calls; no native body claim', 'anadius-online-version-source/version.hpp': 'B25 selected owned online version export binding source composition; no native body claim', 'anadius-online-version-source/version.cpp': 'B25 selected owned online version export binding source composition; no native body claim', 'anadius-online-object-source/object.hpp': 'B26 selected typed online factory and object lifetime source composition; no native body claim', 'anadius-online-object-source/object.cpp': 'B26 selected typed online factory and object lifetime source composition; no native body claim', 'anadius-online-query-source/query.hpp': 'B27 selected typed status and getter source composition with owned copies; no native body claim', 'anadius-online-query-source/query.cpp': 'B27 selected typed status and getter source composition with owned copies; no native body claim', 'anadius-config-defaults-source/defaults.hpp': 'B28 evidence-backed post-construction projection for41modeled configuration fields; whole native036d20aggregate not reconstructed; no native body claim', 'anadius-config-defaults-source/defaults.cpp': 'B28 evidence-backed post-construction projection for41modeled configuration fields; whole native036d20aggregate not reconstructed; no native body claim', 'anadius-session-memory-source/memory.hpp': 'B33 owned session allocation/provenance/context composition contract; no native body claim', 'anadius-session-memory-source/memory.cpp': 'B33 actual owned heap/PTD session memory and lifecycle composition, native suffixes label delegated contracts; no native body claim', 'anadius-io-memory-source/io_memory.hpp': 'B34 concrete owned IO storage/projection and explicit SDK loan contract; no native body claim', 'anadius-io-memory-source/io_memory.cpp': 'B34 concrete session-domain IO allocation/sidecar/completion composition with explicit compatibility pins; delegated address suffixes add no native body claim', 'anadius-owned-server-source/server.hpp': 'B35 owned selected host server/context and admitted dispatcher composition contract; no native body claim', 'anadius-owned-server-source/server.cpp': 'B35 concrete owning server/service/session/IO composition, mandatory accept/runtime/quiescence lowers; native0241b0 not recovered and delegated suffixes add no body claim', 'anadius-accept-source/owned.hpp': 'B36 bounded accepted-operation owning memory, SDK loan and composite dispatcher contract; composition only', 'anadius-accept-source/owned.cpp': 'B36 same-domain accept allocation/projection, pending kernel endpoint pins and B35 quiescence composition; delegated helpers do not add native bodies', 'anadius-session-flow-source/coherent_io.hpp': 'B37 coherent accept/IO/response cache and completion composition contract; no native body claim', 'anadius-session-flow-source/coherent_io.cpp': 'B37 DomainFlowApi and coherent projection/completion routing using existing native source; new methods delegate lower algorithms and do not claim native bodies', 'anadius-accept-helpers-source/bindings.hpp': 'B38 selected socket helper and bounded retry resource composition contract; no native body claim', 'anadius-accept-helpers-source/bindings.cpp': 'B38 actual helper source bindings with primary/null-context owning retry profile; no additional native body claim', 'anadius-server-runtime-source/runtime.hpp': 'B39 reusable owned Runtime/Resources server composition; delegated native labels add no recovered body/ABI closure', 'anadius-server-runtime-source/runtime.cpp': 'B39 reusable owned Runtime/Resources server composition; delegated native labels add no recovered body/ABI closure', 'anadius-server-runtime-source/resources.hpp': 'B39 reusable owned Runtime/Resources server composition; delegated native labels add no recovered body/ABI closure', 'anadius-server-runtime-source/resources.cpp': 'B39 reusable owned Runtime/Resources server composition; delegated native labels add no recovered body/ABI closure', 'anadius-session-profile-source/profile.hpp': 'B40 reusable bounded session Profile/Application and owned string/header composition; no new recovered native body/ABI closure', 'anadius-session-profile-source/profile.cpp': 'B40 reusable bounded session Profile/Application and owned string/header composition; no new recovered native body/ABI closure', 'anadius-session-profile-source/strings.hpp': 'B40 reusable bounded session Profile/Application and owned string/header composition; no new recovered native body/ABI closure', 'anadius-session-profile-source/strings.cpp': 'B40 reusable bounded session Profile/Application and owned string/header composition; no new recovered native body/ABI closure', 'anadius-native-profile-source/native.hpp': 'B41 typed native-message Profile composition with bounded parent/inline memory, prefix string/cipher helpers and guarded selected XML/event providers; no new recovered body or native ABI closure', 'anadius-native-profile-source/native.cpp': 'B41 typed native-message Profile composition with bounded parent/inline memory, prefix string/cipher helpers and guarded selected XML/event providers; no new recovered body or native ABI closure', 'anadius-owned-deque-source/deque.hpp': 'B42 owned deque growth composition using existing026670 and authoritative Parent LIVE typed-map metadata; no new recovered body or native ABI closure', 'anadius-owned-deque-source/deque.cpp': 'B42 owned deque growth composition using existing026670 and authoritative Parent LIVE typed-map metadata; no new recovered body or native ABI closure', 'anadius-owned-environment-source/environment.hpp': 'B43 owned same-worker environment/type/executor/scheduling composition with validated capability resolution; no new native body or ABI closure', 'anadius-owned-environment-source/environment.cpp': 'B43 owned same-worker environment/type/executor/scheduling composition with validated capability resolution; no new native body or ABI closure', 'anadius-owned-jobs-source/jobs.hpp': 'B44 owned inline source-ticket job execution/lifetime and default existing native SDK adapter composition; no recovered CRT thread body or native ABI closure', 'anadius-owned-jobs-source/jobs.cpp': 'B44 owned inline source-ticket job execution/lifetime and default existing native SDK adapter composition; no recovered CRT thread body or native ABI closure', 'anadius-session-policy-source/policy.hpp': 'B45 concrete source session policy for error/category/logging/worker/guard operations with required external lifecycle evidence; no native CRT/EH or ABI closure', 'anadius-session-policy-source/policy.cpp': 'B45 concrete source session policy for error/category/logging/worker/guard operations with required external lifecycle evidence; no native CRT/EH or ABI closure', 'anadius-application-graph-source/graph.hpp': 'B46 concrete same-thread application graph assembling Policy/Profile/NativeLogic/XML/Router/crypto and native-prefix event ownership; not full bootstrap Resources or two-phase worker/loader equivalence', 'anadius-application-graph-source/graph.cpp': 'B46 concrete same-thread application graph assembling Policy/Profile/NativeLogic/XML/Router/crypto and native-prefix event ownership; not full bootstrap Resources or two-phase worker/loader equivalence', 'anadius-worker-handoff-source/handoff.hpp': 'B47 controller-owned dedicated memory capability with verified distinct same-domain worker borrowing and native thread/FLS join barrier; not full Resources/loader or ABI equivalence', 'anadius-worker-handoff-source/handoff.cpp': 'B47 controller-owned dedicated memory capability with verified distinct same-domain worker borrowing and native thread/FLS join barrier; not full Resources/loader or ABI equivalence', 'anadius-application-worker-source/worker.hpp': 'B48 concrete SourceBackend application-worker bridge using exact state, prepared memory and actual startup milestone barrier; not native scheduling/full Resources/loader equivalence', 'anadius-application-worker-source/worker.cpp': 'B48 concrete SourceBackend application-worker bridge using exact state, prepared memory and actual startup milestone barrier; not native scheduling/full Resources/loader equivalence', 'anadius-accept-stop-source/stop.hpp': 'B49 accept-only worker-owned CancelIoEx, observed packet/source-retirement receipts and controller NULL-OV wake admission; not whole-server drain or AcceptEx/native ABI equivalence', 'anadius-accept-stop-source/stop.cpp': 'B49 accept-only worker-owned CancelIoEx, observed packet/source-retirement receipts and controller NULL-OV wake admission; not whole-server drain or AcceptEx/native ABI equivalence', 'anadius-configuration-factory-source/factory.hpp': 'B52 owning concrete configuration Dependencies factory over existing lower-free path/platform/filesystem/persistence/logging source; mandatory narrow license/legacy-reader effects and caller-seeded achievement paths; no original ABI/complete Resources closure', 'anadius-configuration-factory-source/factory.cpp': 'B52 owning concrete configuration Dependencies factory over existing lower-free path/platform/filesystem/persistence/logging source; mandatory narrow license/legacy-reader effects and caller-seeded achievement paths; no original ABI/complete Resources closure', 'anadius-services-factory-source/factory.hpp': 'B53 owning narrow remaining Services policies compose actual memory/logger/exit-wait/thread-error/RET source; Runtime registry owns registration, SourceBackend owns6 worker/host/cleanup routes; selected lifecycle not nativeABI/loader/fullResources proof', 'anadius-services-factory-source/factory.cpp': 'B53 owning narrow remaining Services policies compose actual memory/logger/exit-wait/thread-error/RET source; Runtime registry owns registration, SourceBackend owns6 worker/host/cleanup routes; selected lifecycle not nativeABI/loader/fullResources proof', 'anadius-online-factory-source/factory.hpp': 'B54 lower-free owning online component over existing10core+5boundary+7typedforeign+2Executionphase+3ownedcopies; exactview, explicitCodeLease/precleanupforeignusers/lateattestation, no nativeABI/loader/fullResources proof', 'anadius-online-factory-source/factory.cpp': 'B54 lower-free owning online component over existing10core+5boundary+7typedforeign+2Executionphase+3ownedcopies; exactview, explicitCodeLease/precleanupforeignusers/lateattestation, no nativeABI/loader/fullResources proof'}

def is_composition_only(rel):
    return rel in COMPOSITION_ONLY_SOURCES

def read(rel):
    p = OUT / rel
    b = p.read_bytes()
    INPUTS[rel] = {'sha256': hashlib.sha256(b).hexdigest(), 'bytes': len(b)}
    return b.decode('utf-8-sig')

def load(rel):
    return json.loads(read(rel))

def emit(name, data):
    (DEST / name).write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

def addr(value):
    if not isinstance(value, str):
        return None
    m = re.fullmatch(r'(?:0x)?(180[0-9a-fA-F]{6})', value.strip())
    return '0x' + m.group(1).lower() if m else None

def target(rel):
    top = rel.split('/')[0]
    if top == 'hook-engine-source' and Path(rel).name in ('adapters.cpp', 'adapters.hpp'):
        return 'anadius64'
    if top.startswith('anadius-'):
        return 'anadius64'
    if top.startswith('preloader-'):
        return 'embedded_preloader'
    if top == 'version-rebuild':
        return 'version_outer'
    return None

def mask_cpp(text):
    # Preserve offsets/newlines while excluding comments and quoted literals.
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|R"([^ ()\\\t\r\n]{0,16})\([\s\S]*?\)\1"|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\\n])\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)

def definitions(text):
    masked = mask_cpp(text)
    pattern = r'(?P<name>(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*_(?P<tag>[0-9a-fA-F]{6}))\s*\('
    for m in re.finditer(pattern, masked):
        p, depth = m.end(), 1
        while p < len(masked) and depth:
            depth += (masked[p] == '(') - (masked[p] == ')')
            p += 1
        tail = re.match(r'\s*(?:(?:const|noexcept|override|final)\b\s*(?:\([^)]*\))?\s*|[&\s])*(?:->[^;={}]*?)?\{', masked[p:])
        if not tail:
            continue
        start = p + tail.end() - 1
        end, depth = start + 1, 1
        while end < len(masked) and depth:
            depth += (masked[end] == '{') - (masked[end] == '}')
            end += 1
        if depth:
            raise ValueError('Unclosed candidate body: ' + m['name'])
        yield {'address': '0x180' + m['tag'].lower(), 'symbol': m['name'],
               'line': text.count('\n', 0, m.start()) + 1,
               'body': masked[start:end]}

build = load('source-workspace/build-record.json')
snapshot = load('source-workspace/source-snapshot.json')
assert build['result'] == 'PASS'
units, objects = {}, {}
for command in build['commands']:
    args = command['arguments']
    if '-c' not in args or '-o' not in args:
        continue
    obj = Path(args[args.index('-o') + 1])
    if obj.suffix != '.o':
        continue  # Separate compatibility archive recompiles use .obj.
    source = Path(args[args.index('-c') + 1]).relative_to(OUT).as_posix()
    units[source] = obj
    objects[str(obj).replace('\\', '/').lower()] = source
assert len(units) == build['native_objects']
assert sum(p.endswith('.cpp') for p in units) == build['cpp_objects']
assert sum(p.endswith('.c') for p in units) == build['c_objects']

index = load('anadius-recovery-index.json')
graph = load('anadius-callgraph.json')
procedures = {p['address']: p for p in index['procedures']['normalized_result']}
native = {p['address']: p for p in index['pseudocodeIndex']}
assert len(native) == 2076
pseudo = read('anadius-all-functions.pseudo.c')
assembly = read('anadius-all-functions.asm.txt')
ph = dict(re.findall(r'^/\* (0x[0-9a-f]+) .*?Evidence (ev_[a-f0-9]+)', pseudo, re.M))
ah = dict(re.findall(r'^; (0x[0-9a-f]+) .*?Evidence (ev_[a-f0-9]+)', assembly, re.M))
assert set(ph) == set(native) == set(ah)
assert sum(procedures[a]['procedure']['thunk'] for a in native) == 45

version = load('version-reconstruction-input.json')
preloader = load('preloader-analysis/ghidra-analysis.json')
exports = load('version-rebuild/export-evidence.json')
assert len(version['allPseudo']) == 198 and len(preloader['functions']) == 107
canonical = {'anadius64': native,
             'version_outer': {p['address']: p for p in version['allPseudo']},
             'embedded_preloader': {addr(p['address']): p for p in preloader['functions']}}

nm = read('reconstruction-inventory/object-symbols.txt')
nm_rows, current_object = [], None
for line in nm.splitlines():
    if line.endswith('.o:'):
        current_object = line[:-1].replace('\\', '/').lower()
    m = re.match(r'^[0-9a-fA-F]+\s+([TtWw])\s+(.+)$', line)
    if m and current_object in objects:
        nm_rows.append({'file': objects[current_object], 'symbol': m[2], 'kind': m[1]})

defs, providers, composition_defs, composition_sources = defaultdict(list), [], [], []
for row in snapshot['sources']:
    rel = row['path']
    artifact = target(rel)
    if not artifact or Path(rel).suffix not in ('.cpp', '.hpp') or re.search(r'(?:test|fixture)', Path(rel).name, re.I):
        continue
    text = read(rel)
    assert INPUTS[rel]['sha256'] == row['sha256'], 'Source snapshot mismatch: ' + rel
    if is_composition_only(rel):
        composition_sources.append({'file': rel, 'sha256': row['sha256'],
                                    'reason': COMPOSITION_ONLY_SOURCES[rel]})
    masked = mask_cpp(text)
    for d in definitions(text):
        symbol_base = d['symbol'].split('::')[-1]
        boundary_only = rel == 'anadius-socket-source/cipher_session.cpp' and symbol_base in {
            'allocate_04b0bc', 'free_sized_04b0f8', 'length_error_00ce80',
            'allocation_overflow_00cd30', 'invalid_parameter_05c974'}
        symbols = [x for x in nm_rows if re.search(r'(?<![A-Za-z0-9_])' + re.escape(symbol_base) + r'\(', x['symbol'])
                   and x['file'].split('/')[0] == rel.split('/')[0]]
        d.update(file=rel, artifact=artifact, sha256=row['sha256'],
                 compiled_unit=rel in units,
                 compiled_symbol_candidates=symbols,
                 mapping_basis='bounded_projection_boundary_definition' if boundary_only else 'selected_upstream_adapter_definition' if rel.startswith('hook-engine-source/') else 'address_suffix_on_function_definition')
        d['tagged_calls'] = sorted(set('0x180' + a.lower() for a in re.findall(r'\b[A-Za-z_]\w*_([0-9a-fA-F]{6})\s*\(', d.pop('body'))))
        if is_composition_only(rel):
            d['mapping_basis'] = 'composition_forwarding_not_native_body'
            d['native_body_claim'] = False
            composition_defs.append(d)
            continue
        defs[(artifact, d['address'])].append(d)
    if rel.endswith('.hpp'):
        for m in re.finditer(r'\bvirtual\b([^;{}]*?)=\s*0\s*;', masked, re.S):
            names = re.findall(r'([A-Za-z_]\w*)\s*\(', m[1])
            if not names:
                continue
            name = names[0]
            suffix = re.search(r'_([0-9a-fA-F]{6})$', name)
            providers.append({'artifact': artifact, 'header': rel,
                              'line': text.count('\n', 0, m.start()) + 1,
                              'method': name, 'address': '0x180' + suffix[1].lower() if suffix else None,
                              'declaration': ' '.join(text[m.start():m.end()].split()),
                              'status': 'interface_boundary_not_globally_unresolved'})

# Untagged definitions: bounded, explicit mappings from retained export/source docs.
manual = [
    ('embedded_preloader', '0x180001000', 'preloader-source/preloader_core.cpp', 'Core::dll_main'),
    ('embedded_preloader', '0x180001229', 'preloader-source/preloader_core.cpp', 'Core::process_next'),
    ('embedded_preloader', '0x180001139', 'preloader-source/preloader_core.cpp', 'Core::mutex_trylock'),
    ('embedded_preloader', '0x180001264', 'preloader-source/preloader_core.cpp', 'Core::crypt_query'),
    ('embedded_preloader', '0x180003f8f', 'preloader-source/preloader_core.cpp', 'Once::call'),
    ('version_outer', '0x180005469', 'version-rebuild/loader.cpp', 'ReconstructedDllMain'),
]
for artifact, address, rel, symbol in manual:
    text = read(rel)
    m = re.search(re.escape(symbol) + r'\s*\(', mask_cpp(text))
    assert m and address in canonical[artifact], (artifact, address, symbol)
    symbols = [x for x in nm_rows if x['file'] == rel and re.search(re.escape(symbol) + r'\(', x['symbol'])]
    defs[(artifact, address)].append({'address': address, 'symbol': symbol, 'line': text.count('\n', 0, m.start()) + 1,
        'file': rel, 'artifact': artifact, 'sha256': INPUTS[rel]['sha256'], 'compiled_unit': rel in units,
        'compiled_symbol_candidates': symbols, 'tagged_calls': [], 'mapping_basis': 'explicit_retained_document_mapping'})

claims, gap_records = defaultdict(list), []
for p in sorted(OUT.glob('*/*evidence*.json')):
    rel = p.relative_to(OUT).as_posix()
    artifact = target(rel)
    if not artifact or p.stat().st_size > 5_000_000:
        continue
    data = load(rel)
    if not isinstance(data, dict):
        continue
    fs = data.get('functions', [])
    if isinstance(fs, dict):
        fs = [dict(v, address=k) if isinstance(v, dict) else {'address': k} for k, v in fs.items()]
    for f in fs if isinstance(fs, list) else []:
        if not isinstance(f, dict):
            continue
        address = next((addr(f.get(k)) for k in ('address', 'va', 'original_va', 'native_address') if addr(f.get(k))), None)
        if address:
            claims[(artifact, address)].append({'file': rel, 'kind': 'function_record_only_not_implementation_proof',
                'context_only': f.get('newly_recovered') is False or 'no new original' in str(f.get('mapping', '')).lower()})
    for a in data.get('normal_source_procedures_added', []):
        if addr(a) and data.get('new_original_procedures', 0) > 0:
            claims[(artifact, addr(a))].append({'file': rel, 'kind': 'explicit_normal_body_stage_claim', 'context_only': False})
    for f in data.get('raw_gap_leaves', []):
        gap_records.append({'artifact': artifact, 'address': f.get('va'), 'evidence': rel,
                            'size': f.get('size'), 'bytes_sha256': f.get('sha256'),
                            'status': 'explicit_gap_body_claim_outside_canonical_if_not_listed'})

asm_text = read('version-rebuild/wrappers.asm')
for e in exports:
    symbol = e['symbol']
    m = re.search(r'^' + re.escape(symbol) + r'\s+PROC\b', asm_text, re.M | re.I)
    assert m, symbol
    address = e['original_va']
    defs[('version_outer', address)].append({'address': address, 'symbol': symbol, 'line': asm_text.count('\n', 0, m.start()) + 1,
        'file': 'version-rebuild/wrappers.asm', 'artifact': 'version_outer',
        'sha256': INPUTS['version-rebuild/wrappers.asm']['sha256'], 'compiled_unit': False,
        'compiled_symbol_candidates': [], 'tagged_calls': [], 'mapping_basis': 'export_wrapper_assembly_not_cpp'})
assert len(exports) == 197

rows = []
outgoing, incoming = defaultdict(list), Counter()
for edge in graph['edges']:
    outgoing[edge['source']].append(edge)
    if edge.get('target') in native:
        incoming[edge['target']] += 1
unresolved = Counter(x['source'] for x in graph['unresolved'])
for artifact, functions in canonical.items():
    for address, f in sorted(functions.items()):
        ds, cs = defs[(artifact, address)], claims[(artifact, address)]
        cpp = [d for d in ds if d['mapping_basis'] not in ('export_wrapper_assembly_not_cpp', 'selected_upstream_adapter_definition', 'bounded_projection_boundary_definition')]
        adapters = [d for d in ds if d['mapping_basis'] == 'selected_upstream_adapter_definition']
        boundaries = [d for d in ds if d['mapping_basis'] == 'bounded_projection_boundary_definition']
        observed = [d for d in ds if d['compiled_symbol_candidates']]
        state = 'source_definition_mapped' if cpp else 'selected_upstream_adapter_only' if adapters else 'bounded_projection_boundary_only' if boundaries else 'assembly_wrapper_mapped' if ds else 'module_record_only' if any(not c['context_only'] for c in cs) else 'no_definition_mapping'
        locals_out = sorted(set(e['target'] for e in outgoing[address] if e.get('target') in native)) if artifact == 'anadius64' else []
        provider_hints = sorted(set(c for d in ds for c in d['tagged_calls'] if c in canonical[artifact] and not defs[(artifact, c)]))
        rows.append({'artifact': artifact, 'artifact_sha256': SHA[artifact], 'address': address,
            'key': artifact + '@' + SHA[artifact] + ':' + address,
            'name': f.get('name', 'unknown'), 'thunk': procedures[address]['procedure']['thunk'] if artifact == 'anadius64' else f.get('thunk'),
            'pseudo_evidence': ph[address] if artifact == 'anadius64' else f.get('evidence_id'),
            'assembly_evidence': ah[address] if artifact == 'anadius64' else None,
            'source_presence': state, 'definitions': ds, 'evidence_records': cs,
            'definition_variants': len(ds), 'compiled_symbol_correlated': bool(observed),
            'direct_native_targets': locals_out, 'native_incoming_edge_count': incoming[address] if artifact == 'anadius64' else None,
            'retained_unresolved_callsites': unresolved[address] if artifact == 'anadius64' else None,
            'tagged_source_calls_without_definition_mapping': provider_hints,
            'binding_status': 'requires_explicit_dependency_review', 'abi_equivalence': 'not_established',
            'normal_body_stage_claim': any(c['kind'] == 'explicit_normal_body_stage_claim' for c in cs)})
assert len(rows) == 2381

extra = [{'artifact': artifact, 'address': address, 'definitions': ds,
          'explicit_gap_claims': [g for g in gap_records if g['artifact'] == artifact and g['address'] == address],
          'interpretation': 'outside_canonical_not_added_to_function_denominator'}
         for (artifact, address), ds in sorted(defs.items()) if ds and address not in canonical[artifact]]
for p in providers:
    p['definition_candidates'] = [dict(file=d['file'], symbol=d['symbol'], line=d['line'])
        for (artifact, _), ds in defs.items() if artifact == p['artifact'] for d in ds
        if d['symbol'].split('::')[-1] == p['method']]
    p['composition_definition_candidates'] = [dict(file=d['file'], symbol=d['symbol'], line=d['line'], native_body_claim=False)
        for d in composition_defs if d['artifact'] == p['artifact'] and d['symbol'].split('::')[-1] == p['method']]

summary = {'schema': 'reconstruction-inventory.presence.v1', 'priority_order': [1, 2, 3, 5, 4],
    'scope': 'Retained canonical entries; source presence and recorded dependencies, not native equivalence',
    'build': {k: build[k] for k in ('result', 'native_objects', 'cpp_objects', 'c_objects', 'mock_test_suites')},
    'source_snapshot_entries': len(snapshot['sources']), 'canonical_total_separate_artifacts': len(rows),
    'artifacts': {artifact: {'canonical_functions': len(fs), 'source_presence': dict(Counter(r['source_presence'] for r in rows if r['artifact'] == artifact)),
        'compiled_symbol_correlated_entries': sum(r['compiled_symbol_correlated'] for r in rows if r['artifact'] == artifact),
        'multiple_definition_entries': sum(r['definition_variants'] > 1 for r in rows if r['artifact'] == artifact),
        'explicit_normal_body_claim_entries': sum(r['normal_body_stage_claim'] for r in rows if r['artifact'] == artifact)} for artifact, fs in canonical.items()},
    'anadius_local_thunks': 45, 'external_imports_excluded': 170,
    'pure_virtual_declarations': len(providers), 'outside_canonical_tagged_addresses': len(extra),
    'composition_only_source_files': len(composition_sources),
    'composition_only_address_definition_variants': len(composition_defs),
    'explicit_raw_gap_records': gap_records,
    'limitations': ['No completion percentage is derived. Legacy ~80% is a planning guess, not this inventory metric.',
        'Missing exact address mapping does not prove absent source or unanalysed behavior.',
        'Address-tagged definitions include coarse models, adapters and duplicate implementations; not equivalent native bodies.',
        'Explicit B2 composition-only address-suffixed forwarders are retained separately and do not add canonical source mappings or native body claims.',
        'An interface declaration may be satisfied by a derived override. It is not automatically globally unresolved.',
        'Source-call tags and compiler symbols are bounded observations; no indirect/ABI/runtime equivalence proof.',
        'Embedded preloader and outer version have independent VA namespaces; original embedded bytes remain in version build.',
        'Gap/data/helper tags outside canonical entries are separate and are not all extra native functions.',
        'Original target code was not executed or modified; static Ghidra imports/data reads are recorded separately. No real user CFG/token/achievement contents read.']}
emit('functions.json', rows)
emit('providers.json', providers)
emit('outside-canonical.json', extra)
emit('summary.json', summary)
emit('composition-only.json', {'schema': 'inventory.composition-only.v1',
    'scope': 'Compiled new composition forwarding observed separately; no native address coverage increment',
    'excluded_sources': composition_sources, 'definitions': composition_defs,
    'new_native_bodies': 0, 'new_native_closures': 0})
with (DEST / 'functions.csv').open('w', newline='', encoding='utf-8-sig') as stream:
    keys = ['artifact', 'address', 'name', 'source_presence', 'definition_variants', 'compiled_symbol_correlated', 'normal_body_stage_claim', 'source_locations', 'binding_status', 'abi_equivalence', 'retained_unresolved_callsites']
    writer = csv.DictWriter(stream, fieldnames=keys)
    writer.writeheader()
    for r in rows:
        writer.writerow({k: (' | '.join(f"{d['file']}:{d['line']}:{d['symbol']}" for d in r['definitions']) if k == 'source_locations' else r[k]) for k in keys})
emit('inputs.json', {'scope': 'Files actually read by offline generator, relative to outputs; original inputs never read', 'files': INPUTS})
print(json.dumps(summary, ensure_ascii=False, indent=2))
