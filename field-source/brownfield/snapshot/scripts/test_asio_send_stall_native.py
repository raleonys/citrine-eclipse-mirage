#!/usr/bin/env python3
"""Test-only Windows x64 IOCP send-stall acceptance; fail closed.

Compiles a dedicated genuinely pending kernel async_write fixture against pinned
standalone Asio, then runs the source-linked native acceptance host scenario
which stalls a real AsioSessionOwner transaction *before* its async_write.
These two probes are independent: neither is a claim of an actual kernel stall
on a bounded LSX Response/Event frame in the production binary.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[1]

def fail(message):
    raise RuntimeError(message)

def run(cmd,cwd,log,env=None,timeout=180):
    with log.open('wb') as output:
        try:r=subprocess.run(cmd,cwd=cwd,env=env,stdout=output,stderr=subprocess.STDOUT,timeout=timeout)
        except subprocess.TimeoutExpired:fail('TIMEOUT: '+str(log))
    if r.returncode:fail(f'exit={r.returncode}; see {log}')
    return r.returncode

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--profile',choices=('mt','mtd','both'),default='both')
    p.add_argument('--native-dir',type=Path,default=ROOT/'dist/native_acceptance')
    args=p.parse_args()
    if os.name!='nt':fail('Windows-only native IOCP acceptance')
    arch=os.environ.get('VSCMD_ARG_TGT_ARCH','').lower()
    if arch not in ('x64','amd64'):fail('vcvars64 x64 must be initialized')
    cl=shutil.which('cl.exe')
    if not cl:fail('cl.exe not found')
    out=ROOT/'dist/ids07_send_stall';out.mkdir(parents=True,exist_ok=True)
    native=args.native_dir.resolve()
    host=native/'preloader_l_native_acceptance_host.exe'
    dll=native/'preloader_l_native_test.dll'
    if not host.is_file() or not dll.is_file():fail('build Stage07 Native Acceptance first (host and DLL required)')
    expected_host_sha=sha(host);expected_dll_sha=sha(dll)
    for profile in (('mt','mtd') if args.profile=='both' else (args.profile,)):
        exe=out/('ids07_write_backpressure_'+profile+'.exe')
        cmd=[cl,'/nologo','/std:c++20','/EHsc','/W4','/DASIO_STANDALONE=1',
             '/DASIO_NO_TYPEID=1','/MT' if profile=='mt' else '/MTd',
             '/I'+str(ROOT/'third_party/asio/upstream/include'),
             '/I'+str(ROOT/'src/runtime'),
             str(ROOT/'tests/native/test_asio_iocp_write_backpressure.cpp'),
             '/Fe:'+str(exe),'/link','ws2_32.lib','mswsock.lib']
        run(cmd,out,out/('compile_'+profile+'.log'),timeout=150)
        run([str(exe)],out,out/('iocp_backpressure_'+profile+'.log'),timeout=22)
        if b'IDS07_REAL_IOCP_WRITE_BACKPRESSURE: PASS' not in (out/('iocp_backpressure_'+profile+'.log')).read_bytes():
            fail('IOCP fixture marker missing: '+profile)
        print('IDS07_REAL_IOCP_WRITE_BACKPRESSURE',profile.upper(),'PASS',flush=True)
    env=os.environ.copy();env['PRELOADER_L_ACCEPTANCE_SCENARIO']='ids07_send_idle_timeout'
    run([str(host)],native,out/'native_transaction_timeout.log',env=env,timeout=45)
    native_log=(out/'native_transaction_timeout.log').read_bytes()
    if b'IDS07_NATIVE_SEND_TIMEOUT PASS' not in native_log or b'IDS07_PARALLEL_AND_NO_EVENT_LEAK PASS' not in native_log:
        fail('native transaction acceptance markers missing')
    if sha(host)!=expected_host_sha or sha(dll)!=expected_dll_sha:fail('native artifacts modified during test')
    result={'status':'PASS_FRESH_NATIVE_TEST_ONLY','source':'IDS07 source tree',
            'native_dll_sha256':expected_dll_sha,'host_sha256':expected_host_sha,
            'profiles':args.profile,
            'classes':['real IOCP large-write kernel-backpressure fixture',
                       'real native AsioSessionOwner 5s prewrite timeout and transaction cancellation'],
            'not_proven':['real kernel-pending stalled LSX production Response/Event frame',
                          'actual persistent store replay/ACK exactly-once','games','release acceptance']}
    (out/'IDS07_NATIVE_RESULT.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('IDS07_NATIVE_SEND_IDLE_ACCEPTANCE: PASS',flush=True)
    print('RESULTS:',out,flush=True)
if __name__=='__main__':
    try:main()
    except Exception as e:
        print('IDS07_NATIVE_SEND_IDLE_ACCEPTANCE: FAIL',str(e),file=sys.stderr,flush=True)
        sys.exit(1)
