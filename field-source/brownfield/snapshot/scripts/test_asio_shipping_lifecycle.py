#!/usr/bin/env python3
"""Real shipping DLL loopback + IOCP timeout/reconnect acceptance on Windows.

Runs from x64 Developer Command Prompt after building Stage06 Production.
Unlike source-only tests, this spawns a fresh process that LoadLibrary's the
actual Production DLL. Covers true 30s receive idle, non-extensible 10s
fragmented frame, 24 repeated abrupt disconnect/reconnect cycles, then
DllShutdown -> handler drain -> FreeLibrary. Not an Event send-idle test.
Fail closed on missing DLL, dirty ports, host failure or timeout. Test-only.
"""
import argparse
import os
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
PORTS=tuple(range(3226,3237))
CHALLENGE=b'<Challenge key="'

def fail(msg):
    raise RuntimeError(msg)

def greet(port, deadline=3.0):
    with socket.create_connection(('127.0.0.1',port),timeout=deadline) as peer:
        peer.settimeout(deadline)
        payload=bytearray()
        while len(payload)<8192:
            item=peer.recv(1024)
            if not item: fail('EOF before greeting')
            payload.extend(item)
            if b'\0' in payload:break
        if CHALLENGE not in payload:fail('not an LSX Challenge greeting')
        return bytes(payload)

def connect_greeting(port,timeout=4.0):
    peer=socket.create_connection(('127.0.0.1',port),timeout=timeout)
    peer.settimeout(timeout)
    payload=bytearray()
    while len(payload)<8192 and b'\0' not in payload:
        part=peer.recv(1024)
        if not part:peer.close();fail('server closed before greeting')
        payload.extend(part)
    if CHALLENGE not in payload:
        peer.close();fail('unexpected greeting payload')
    return peer

def closed_within(peer, wait):
    peer.settimeout(wait)
    try:return peer.recv(1)==b''
    except socket.timeout:return False
    except ConnectionResetError:return True

def run_one(root,dll,profile):
    out=root/'dist'/'ids06_windows';out.mkdir(parents=True,exist_ok=True)
    host=out/('ids06_host_'+profile+'.exe')
    cl=shutil.which('cl.exe')
    if not cl:fail('x64 MSVC Developer Command Prompt required (cl.exe)')
    cmd=[cl,'/nologo','/std:c++20','/EHsc','/W4','/MT' if profile=='mt' else '/MTd',
         str(root/'tests/native/test_asio_service_lifetime_host.cpp'),'/Fe:'+str(host),
         '/link','kernel32.lib']
    log=out/('ids06_host_'+profile+'_build.log')
    with log.open('wb') as f:
        cp=subprocess.run(cmd,cwd=out,stdout=f,stderr=subprocess.STDOUT)
    if cp.returncode: fail('host BUILD FAIL: '+str(log))
    sandbox=out/('run_'+profile);sandbox.mkdir(exist_ok=True)
    shutil.copyfile(dll,sandbox/'preloader_l.dll')
    # Do not attach to another server. The test must own the loopback target.
    for p in PORTS:
        with socket.socket() as probe:
            probe.settimeout(0.25)
            if probe.connect_ex(('127.0.0.1',p))==0:
                fail('Port already occupied before host launch: '+str(p))
    proc=subprocess.Popen([str(host)],cwd=sandbox,stdin=subprocess.PIPE,
                          stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    try:
        port=None
        end=time.monotonic()+15
        while time.monotonic()<end and proc.poll() is None:
            for candidate in PORTS:
                try:
                    greet(candidate,0.4);port=candidate;break
                except (OSError,RuntimeError):pass
            if port is not None:break
            time.sleep(0.1)
        if port is None:fail('shipping listener not ready within 15 seconds')
        print('IDS06_BOUND_PORT',port,flush=True)
        # Reconnect test exercises real accept/session ownership and peer EOF.
        for i in range(24):
            s=connect_greeting(port)
            s.close()
        print('IDS06_RECONNECT_24 PASS',flush=True)
        # Frame fixed 10-second deadline; extra fragments must not extend it.
        peer=connect_greeting(port)
        # Keep a genuinely incomplete LSX frame buffered.  The earlier
        # b'<' + b'x' sequence is NOT an LSX prefix: the production scanner
        # discards it as malformed, so a fixed-frame deadline cannot be tested.
        peer.sendall(b'<LSX>')
        begun=time.monotonic()
        for fragment in (b'<', b'Request', b' id="'):
            time.sleep(2.0)
            if closed_within(peer,0.01):fail('frame closed too early')
            peer.sendall(fragment)
        remaining=15-(time.monotonic()-begun)
        if not closed_within(peer,max(0.1,remaining)):
            fail('incomplete frame remained open beyond 15s')
        elapsed=time.monotonic()-begun
        peer.close()
        if not 8.5<=elapsed<=15.0:fail(f'frame deadline unexpected: {elapsed:.2f}s')
        print('IDS06_FRAME_10S PASS elapsed='+str(round(elapsed,2)),flush=True)
        # Receive idle: no input after the complete server greeting.
        peer=connect_greeting(port)
        begun=time.monotonic()
        if not closed_within(peer,38):fail('idle receive remained open beyond 38s')
        elapsed=time.monotonic()-begun
        peer.close()
        if not 27.0<=elapsed<=38.0:fail(f'receive idle unexpected: {elapsed:.2f}s')
        print('IDS06_RECEIVE_IDLE_30S PASS elapsed='+str(round(elapsed,2)),flush=True)
        # Listener must still accept a new session after both timeouts.
        greet(port)
        print('IDS06_POST_TIMEOUT_RECONNECT PASS',flush=True)
        if proc.poll() is not None:fail('host exited before shutdown')
        stdout,_=proc.communicate(input=b'\n',timeout=40)
        if proc.returncode or b'IDS06_SHUTDOWN_PASS' not in stdout:
            fail('quiescent shutdown/FreeLibrary failed: '+repr(stdout[-1500:]))
        print('IDS06_SHUTDOWN_DRAIN PASS',flush=True)
    finally:
        if proc.poll() is None:
            proc.kill()
            proc.communicate(timeout=5)

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--dll',type=Path,default=ROOT/'dist/shipping/preloader_l.dll')
    p.add_argument('--profile',choices=('mt','mtd','both'),default='both')
    args=p.parse_args()
    if os.name!='nt':fail('Windows-only acceptance; no portable PASS substitution')
    arch=os.environ.get('VSCMD_ARG_TGT_ARCH','').lower()
    if arch and arch not in ('amd64','x64'):fail('x64 MSVC required')
    dll=args.dll.resolve()
    if not dll.is_file():fail('exact shipping DLL not found: '+str(dll))
    for profile in (('mt','mtd') if args.profile=='both' else (args.profile,)):
        run_one(ROOT,dll,profile)
    print('IDS06_WINDOWS_IOCP_RECEIVE_FRAME_RECONNECT: PASS',flush=True)
    print('IDS06_SEND_IDLE_REAL_IOCP: PENDING (requires explicit stalled-write native injection)',flush=True)

if __name__=='__main__':
    try:main()
    except Exception as e:
        print('IDS06_WINDOWS_IOCP_ACCEPTANCE: FAIL',str(e),file=sys.stderr,flush=True)
        sys.exit(1)
