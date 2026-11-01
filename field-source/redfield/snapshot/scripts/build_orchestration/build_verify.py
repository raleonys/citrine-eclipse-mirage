# SPDX-License-Identifier: GPL-3.0-only
"""Developer aggregate regression/DLL runner; production dist uses its own profile."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
from build_common import (BuildLock, BuildSession, prepare_output, source_preflight,
                          write_receipt, digest)

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sanitize', choices=['none','address','undefined','thread'], default='none')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--leak-check', choices=['on','off'], default='on')
    args = parser.parse_args()
    windows = os.name == 'nt'
    if windows and args.sanitize != 'none':
        raise RuntimeError('Sanitizers are supported by portable GCC validation only')
    mode = 'msvc_x64' if windows else 'portable_' + args.sanitize + ('_no_leaks' if args.leak_check == 'off' else '')
    with BuildLock(ROOT, 'verify'):
        build = prepare_output(ROOT, 'build/verify/' + mode)
        qualification = source_preflight(ROOT)
        print('[START] RF37 GOLDEN BINDING FIX 01 - developer aggregate', flush=True)
        print('[OUTPUT]', build, flush=True)
        graph = json.loads((ROOT / 'metadata/BUILD_GRAPH.json').read_text(encoding='utf-8'))
        session = BuildSession(ROOT, build, graph, windows=windows, cxx=args.cxx,
                               sanitize=args.sanitize, leak_check=args.leak_check)
        objects = [session.compile(p) for p in graph['core_sources']]
        if windows:
            objects += [session.compile(p) for p in graph['windows_core_sources']+graph['windows_vendor_sources']]
        support = [session.compile(p) for p in graph['test_support_sources']]
        receipt = {'project':'REDFIELD', 'checkpoint':'RF37', 'classification':'LOCAL_BUILD/NOT_RELEASE',
                   'platform':sys.platform, 'tests':[], 'diagnostics':[], 'windows_dll':'NOT_RUN',
                   'production_objects':[p.name for p in objects], 'test_support_objects':[p.name for p in support],
                   'sanitizer':args.sanitize, 'leak_detection_requested':args.leak_check,
                   'import_refactor':qualification, 'dist':'NOT_TOUCHED_DEVELOPER_AGGREGATE',
                   'consumer':'HOOKS_INTEGRATED; SDK_HANDSHAKE_NOT_IMPLEMENTED'}
        for name, relative in graph['tests'].items():
            main_obj = session.compile(relative)
            exe = build / (name + ('.exe' if windows else ''))
            session.link(exe, [main_obj, *objects, *support])
            arguments = [ROOT / p for p in graph.get('test_arguments', {}).get(name, [])]
            output = session.run([exe, *arguments], timeout=45)
            receipt['tests'].append({'name':name, 'status':'PASS', 'exit_code':0,
                                     'binary_sha256':digest(exe), 'stdout':output.strip()})
            print('PASS', name, flush=True)
        for name, diagnostic in graph['diagnostics'].items():
            main_obj = session.compile(diagnostic['source'])
            exe = build / (name + ('.exe' if windows else ''))
            session.link(exe, [main_obj, *objects, *support])
            output = session.run([exe], timeout=45, allowed_exit=diagnostic['observed_exit'])
            receipt['diagnostics'].append({'name':name, 'status':'FAIL_OBSERVED',
                'exit_code':diagnostic['observed_exit'], 'reason':diagnostic['reason'],
                'stdout':output.strip(), 'binary_sha256':digest(exe)})
            print('DIAGNOSTIC_FAIL_OBSERVED', name, flush=True)
        if windows:
            dll_objects = [session.compile(p) for p in graph['windows_dll_sources']]
            dll = build / 'preloader_l.dll'
            session.link(dll, [*dll_objects, *objects], dll=True, exports='exports/preloader_l.def')
            smoke_obj = session.compile(graph['windows_smoke_source'])
            smoke = build / 'abi_smoke.exe'
            session.link(smoke, [smoke_obj])
            output = session.run([smoke, dll])
            receipt['windows_dll'] = {'status':'PASS','binary_sha256':digest(dll),
                                       'scope':'BOOTSTRAP_LIFECYCLE_ONLY','smoke':output.strip()}
            native_objects = [session.compile(p) for p in graph['windows_native_dll_sources']]
            native_dll = build / 'preloader_l_native_test.dll'
            session.link(native_dll, [*native_objects,*objects], dll=True, exports='exports/preloader_l_native_test.def')
            host_obj = session.compile(graph['windows_native_test_source'])
            host = build / 'native_acceptance.exe'
            session.link(host, [host_obj])
            native_output = session.run([host, native_dll], timeout=120)
            if 'PRELOADER_L_NATIVE_ACCEPTANCE_PASS' not in native_output:
                raise RuntimeError('Native acceptance completion marker missing')
            receipt['windows_native_acceptance'] = {'status':'PASS', 'test_only':True,
                'binary_sha256':digest(native_dll), 'stdout':native_output.strip()}
        session.finish(receipt)
        write_receipt(build / 'receipt.json', receipt)
        print('PRELOADER_L_RF37_SCOPED_PASS_WITH_RECORDED_DIAGNOSTIC', build / 'receipt.json')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (RuntimeError, ValueError, OSError, KeyError, TypeError, subprocess.SubprocessError) as error:
        print('PRELOADER_L_RF37_FAIL:', error, file=sys.stderr)
        sys.exit(1)
