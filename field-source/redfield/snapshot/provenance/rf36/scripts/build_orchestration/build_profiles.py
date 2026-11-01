# SPDX-License-Identifier: GPL-3.0-only
"""Internal profile composition over the one shared compiler/session engine."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
from build_common import (BuildLock, BuildSession, atomic_copy, clear_dist,
                          digest, load_profiles, prepare_output, publish_production, audit_production_link,
                          source_preflight, write_receipt)
from pe_acceptance import inspect
from golden import execute as execute_golden

ROOT = Path(__file__).resolve().parents[2]


def run_profile(root, profile):
    """Only fixed profile destinations; no caller-controlled build/dist override."""
    if profile not in {'production','debug','native','crt'}:
        raise ValueError('UNKNOWN_OPERATOR_PROFILE')
    if os.name != 'nt':
        raise RuntimeError('WINDOWS_MSVC_X64_REQUIRED;use build_verify.py for portable regressions')
    with BuildLock(root, profile):
        build = prepare_output(root, 'build/' + profile)
        receipt = {'checkpoint':'RF36', 'classification':'LOCAL_BUILD/NOT_RELEASE',
                   'profile':profile, 'status':'STARTED', 'golden':'NOT_RUN',
                   'game':'NOT_RUN', 'published':[]}
        session = None
        try:
            # No stale success survives a new attempt, including failed preflight.
            clear_dist(root, profile)
            receipt['import_refactor'] = source_preflight(root)
            policy = load_profiles(root)
            graph = json.loads((root / 'metadata/BUILD_GRAPH.json').read_text(encoding='utf-8'))
            print('[START] RF36 CRT STARTUP FIX 01 - ' + profile, flush=True)
            print('[OUTPUT]', build, flush=True)
            session = BuildSession(root, build, graph, windows=True, profile=profile)
            objects = [session.compile(p) for p in graph['core_sources']+
                       graph['windows_core_sources']+graph['windows_vendor_sources']]
            outputs = []
            receipt['acceptance'] = []
            if profile in {'production','debug','crt'}:
                names = {'production':'preloader.unsigned.dll',
                         'debug':'preloader_l_debug.dll', 'crt':'preloader_l_crt_test.dll'}
                entry = [session.compile(p) for p in graph['windows_dll_sources']]
                dll = build / names[profile]
                session.link(dll, [*entry,*objects], dll=True, exports='exports/preloader_l.def')
                pe = inspect(dll.read_bytes())
                smoke_obj = session.compile(graph['windows_smoke_source'])
                host = build / 'abi_smoke.exe'
                session.link(host, [smoke_obj])
                before = digest(dll)
                output = session.run([host, dll], timeout=120)
                if digest(dll) != before:
                    raise RuntimeError('ACCEPTANCE_MUTATED_DLL')
                if 'ABI_SMOKE_PASS:3_LOAD_SHUTDOWN_UNLOAD_CYCLES' not in output:
                    raise RuntimeError('ABI_SMOKE_COMPLETION_MARKER_MISSING')
                receipt['acceptance'].append({'kind':'public_abi_lifecycle', 'status':'PASS',
                    'scope':'Current ABI host only;not complete legacy CRT acceptance',
                    'dll_sha256':before, 'pe':pe, 'stdout':output.strip()})
                outputs += [dll, host]
            if profile in {'native','crt'}:
                entry = [session.compile(p) for p in graph['windows_native_dll_sources']]
                dll = build / 'preloader_l_native_test.dll'
                session.link(dll, [*entry,*objects], dll=True, exports='exports/preloader_l_native_test.def')
                pe = inspect(dll.read_bytes(), native=True)
                host_obj = session.compile(graph['windows_native_test_source'])
                host = build / 'native_acceptance.exe'
                session.link(host, [host_obj])
                before = digest(dll)
                output = session.run([host, dll], timeout=120)
                if digest(dll) != before:
                    raise RuntimeError('ACCEPTANCE_MUTATED_DLL')
                if 'PRELOADER_L_NATIVE_ACCEPTANCE_PASS' not in output:
                    raise RuntimeError('NATIVE_COMPLETION_MARKER_MISSING')
                receipt['acceptance'].append({'kind':'current_native_scenarios', 'status':'PASS',
                    'scope':'Current host coverage only;HF19 scenario parity remains under review',
                    'dll_sha256':before, 'pe':pe, 'stdout':output.strip()})
                outputs += [dll, host]
            session.finish(receipt)
            if profile == 'production':
                receipt['link_policy'] = audit_production_link(session,dll)
                receipt['golden'] = {'status':'RUNNING','stages':[]}
                outcome = execute_golden(root,dll,stage_sink=receipt['golden']['stages'])
                target = publish_production(session,outcome,receipt)
                receipt['published'] = [{'path':target.relative_to(root).as_posix(),'sha256':digest(target)}]
                receipt['golden']['status'] = 'PASS_LOCAL_BOUND_CHAIN'
                receipt['status'] = 'PASS_LOCAL_PRODUCTION_GOLDEN_NOT_RELEASE'
                receipt['scope'] = 'Current local production build andordered Golden chain;not official release/full native/CRT/game acceptance'
                write_receipt(build/'receipt.json',receipt)
                print('PRELOADER_L_LOCAL_PRODUCTION_GOLDEN_PASS',target,flush=True)
                return 0
            dist_root = {'debug':'debug','native':'native_acceptance','crt':'crt_acceptance'}[profile]
            # Include symbols only with the Debug profile, never shipping paths.
            if profile == 'debug':
                outputs += list(build.glob('*.pdb')) + list(build.glob('*.map'))
            try:
                for output in outputs:
                    relative = 'dist/' + dist_root + '/' + output.name
                    atomic_copy(root, output, relative, digest(output))
                    receipt['published'].append({'path':relative, 'sha256':digest(output)})
                # Revalidate compiler/input/object/binary identity after copying.
                session.finish(receipt)
            except BaseException:
                clear_dist(root, profile)
                raise
            receipt['status'] = 'PASS_SCOPED_' + profile.upper()
            receipt['scope'] = 'Isolated current build andhost checks;not full HF19 coverage,Golden or game acceptance'
            write_receipt(build / 'receipt.json', receipt)
            print('PRELOADER_L_PROFILE_SCOPED_PASS', profile, build / 'receipt.json', flush=True)
            return 0
        except BaseException as error:
            try:
                clear_dist(root, profile)
            except OSError as cleanup_error:
                receipt['cleanup_error'] = str(cleanup_error)
            receipt['status'] = 'FAIL'
            if isinstance(receipt['golden'],dict) and receipt['golden'].get('status') == 'RUNNING':
                receipt['golden']['status'] = 'FAIL'
            receipt['error'] = str(error)
            # Failure evidence is deliberately a different name from success.
            write_receipt(build / 'failure.json', receipt)
            raise


def operator(profile):
    parser = argparse.ArgumentParser(description='Isolated ' + profile + ' local build;not official release')
    parser.parse_args()  # Unknown arguments/override attempts are real failures.
    try:
        return run_profile(ROOT, profile)
    except (RuntimeError, ValueError, OSError, KeyError, TypeError, subprocess.SubprocessError) as error:
        print('PRELOADER_L_PROFILE_FAIL:', error, file=sys.stderr)
        return 1
