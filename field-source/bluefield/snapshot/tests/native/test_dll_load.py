# SPDX-License-Identifier: GPL-3.0-only
"""Internal Windows-only child-process DLL load/redirect/shutdown fixture."""
import argparse,ctypes,json,os,shutil,socket,struct,subprocess,sys,tempfile,time
import xml.etree.ElementTree as ET
from pathlib import Path
def child(dll,online=False,scenario='configured'):
 k=ctypes.WinDLL('kernel32',use_last_error=True)
 k.GetEnvironmentVariableW.argtypes=[ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_uint32];k.GetEnvironmentVariableW.restype=ctypes.c_uint32
 k.SetEnvironmentVariableW.argtypes=[ctypes.c_wchar_p,ctypes.c_wchar_p];k.SetEnvironmentVariableW.restype=ctypes.c_int
 k.FreeLibrary.argtypes=[ctypes.c_void_p];k.FreeLibrary.restype=ctypes.c_int
 k.GetModuleHandleW.argtypes=[ctypes.c_wchar_p];k.GetModuleHandleW.restype=ctypes.c_void_p
 def variable(name):
  b=ctypes.create_unicode_buffer(4096);n=k.GetEnvironmentVariableW(name,b,len(b));return b.value if n else None
 crypt=ctypes.WinDLL('crypt32',use_last_error=True)
 dword=ctypes.c_uint32;pointer=ctypes.c_void_p
 crypt.CryptQueryObject.argtypes=[dword,pointer,dword,dword,dword,ctypes.POINTER(dword),ctypes.POINTER(dword),ctypes.POINTER(dword),ctypes.POINTER(pointer),ctypes.POINTER(pointer),ctypes.POINTER(pointer)]
 crypt.CryptQueryObject.restype=ctypes.c_int
 def query(object_type,object):
  encoding=dword();content=dword();format=dword();store=pointer();message=pointer();context=pointer()
  ctypes.set_last_error(0x1234)
  result=crypt.CryptQueryObject(object_type,object,0x3ffe,14,0,ctypes.byref(encoding),ctypes.byref(content),ctypes.byref(format),ctypes.byref(store),ctypes.byref(message),ctypes.byref(context))
  error=ctypes.get_last_error()
  if result:raise RuntimeError('synthetic invalid crypt fixture unexpectedly parsed')
  return result,error
 def query_path(path):return query(1,ctypes.cast(ctypes.c_wchar_p(str(path)),pointer))
 missing=dll.parent/'crypt_fixture_missing.dll'
 companion='EAAntiCheat.GameServiceLauncher.dll'
 class Blob(ctypes.Structure):_fields_=[('size',dword),('data',ctypes.POINTER(ctypes.c_ubyte))]
 invalid_bytes=(ctypes.c_ubyte*4)(1,2,3,4);blob=Blob(4,invalid_bytes)
 before_missing=query_path(missing);before_companion=query_path(companion);before_blob=query(2,ctypes.byref(blob))
 before={'EALsxPort':'fixture-before','ContentId':'fixture-content','EAGameLocale':'fixture-locale','OriginEmuCmdArgs':'fixture-argument'}
 for name,value in before.items():assert k.SetEnvironmentVariableW(name,value)
 library=ctypes.WinDLL(str(dll));shutdown=library.DllShutdown;shutdown.argtypes=[ctypes.c_uint32];shutdown.restype=ctypes.c_uint32
 until=time.monotonic()+10;port=None
 while time.monotonic()<until:
  value=variable('EALsxPort')
  if value and value.isdigit():port=int(value);break
  time.sleep(.02)
 if not port:raise RuntimeError('runtime did not publish its bound port')
 ws=ctypes.WinDLL('ws2_32',use_last_error=True);ws.connect.argtypes=[ctypes.c_size_t,ctypes.c_void_p,ctypes.c_int];ws.connect.restype=ctypes.c_int
 raw=struct.pack('=H',2)+struct.pack('!H',3216)+socket.inet_aton('127.0.0.1')+bytes(8)
 client=None
 for _ in range(6):
  candidate=socket.socket();candidate.settimeout(4);address=ctypes.create_string_buffer(raw)
  if ws.connect(candidate.fileno(),address,len(raw))==0:client=candidate;break
  candidate.close();time.sleep(.3)
 if client is None:raise RuntimeError('native connect hook did not redirect port 3216')
 if query_path(missing)!=before_missing:raise RuntimeError('crypt file forwarding result/LastError changed')
 if query(2,ctypes.byref(blob))!=before_blob:raise RuntimeError('crypt blob forwarding result/LastError changed')
 if query(1,None)!=before_companion:raise RuntimeError('crypt null-file companion selection mismatch')
 target=dll.parent/'preloader_l.dll'
 if not target.exists():target.write_bytes(b'synthetic invalid certificate fixture')
 if query_path(target)!=before_companion:raise RuntimeError('crypt suffix companion selection mismatch')
 log=Path(os.environ['LOCALAPPDATA'])/'SURGE/surge.log'
 if log.exists():raise RuntimeError('Native/CRT must not create disk diagnostics')
 def frame():
  value=bytearray()
  while not value.endswith(b'\0'):
   block=client.recv(1)
   if not block:raise RuntimeError('unexpected EOF')
   value+=block
   if len(value)>1024*1024:raise RuntimeError('fixture frame quota')
  return bytes(value[:-1])
 assert b'Challenge' in frame()
 authenticated=online and scenario not in ('cached','cached_invalid','old_only','bad_version','null_factory')
 expected={'Country':'US','GeoCountry':'US','PersonaId':'1144668899','IsTrialSubscriber':'false','SubscriberLevel':'2','Persona':'SURGE','UserId':'1000200030000','CommerceCountry':'US','CommerceCurrency':'USD','AvatarId':'','IsSubscriber':'true','UserIndex':'0','IsUnderAge':'false','IsSteamSubscriber':'false'}
 if scenario=='configured':expected.update(Persona='FixturePlayer',PersonaId='1',UserId='2')
 if scenario=='partial':expected.update(Persona='Partial & User',CommerceCurrency='EUR')
 if scenario=='override_only':expected.update(Persona='OverrideUser')
 if authenticated:expected.update(Persona='FixtureOnlineUser',PersonaId='31001',UserId='32001')
 if scenario=='cached':expected.update(Persona='CachedUser',PersonaId='41001',UserId='42001')
 commands=[(b'<GetConfig/>',b'GetConfigResponse'),(b'<GetProfile index="0"/>',b'GetProfileResponse')]
 if authenticated:commands+=[(b'<GetAuthCode ClientId="fixture"/>',b'SYNTHETIC_ONLINE_AUTH'),(b'<QueryImage Width="416" Height="416" ImageId="avatar:32001"/>',b'ResourcePath'),(b'<QueryImage Width="416" Height="416" ImageId="avatar:33001"/>',b'ResourcePath')]
 for command,wanted in commands:
  client.sendall(b'<LSX><Request id="fixture" recipient="EbisuSDK">'+command+b'</Request></LSX>\0');reply=frame();assert wanted in reply
  if command.startswith(b'<GetProfile '):
   profile=ET.fromstring(reply).find('./Response/GetProfileResponse')
   if profile is None or profile.attrib!=expected:raise RuntimeError('complete native profile differs: '+repr(None if profile is None else profile.attrib))
 client.close()
 if authenticated:
  record=Path(os.environ['PRELOADER_L_FIXTURE_AVATAR_RECORD']).read_text()
  if record!='33001:2\n':raise RuntimeError('avatar owner/size ABI or own-profile avatar isolation mismatch: '+record)
 if log.exists():raise RuntimeError('Native/CRT protocol activity created diagnostics')
 code=shutdown(15000)
 if code:raise RuntimeError('DllShutdown did not authorize unload: '+str(code))
 if {name:variable(name) for name in before}!=before:raise RuntimeError('environment restoration mismatch')
 handle=library._handle
 if not k.FreeLibrary(handle):raise RuntimeError('host FreeLibrary failed')
 library._handle=0
 if k.GetModuleHandleW(str(dll)):raise RuntimeError('module remained loaded after clean shutdown')
 if query_path(missing)!=before_missing:raise RuntimeError('crypt original forwarding after DLL unload changed')
 print('PASS Windows DLL scenario='+scenario+'; complete profile, loader/hooks/LSX/environment/unload')
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('dll',type=Path);ap.add_argument('--child',action='store_true');ap.add_argument('--online-fixture',type=Path);ap.add_argument('--scenario');a=ap.parse_args()
 if os.name!='nt':print('Windows DLL fixture requires Windows; NOT_RUN');return 2
 if a.child:child(a.dll.resolve(),a.online_fixture is not None,a.scenario or 'configured');return 0
 results=[]
 scenarios=[a.scenario] if a.scenario else ['configured','absent','partial','override_only']+(['cached','cached_invalid','old_only','null_factory'] if a.online_fixture else [])
 for scenario in scenarios:
  with tempfile.TemporaryDirectory(prefix='preloader_l_native_') as temp:
   root=Path(temp);dll=root/a.dll.name;shutil.copy2(a.dll,dll)
   if a.online_fixture:shutil.copy2(a.online_fixture,root/('anadius64online.dll' if scenario=='old_only' else 'preloader_l_online.dll'))
   if scenario=='configured':(root/'surge.cfg').write_text('"Config2"{"Game"{"Name" "Fixture" "ContentId" "424242" "Language" "en_US"}"Emulator"{"ServerPort" "random" "SDK" "9.9.0.0"}"User"{"Username" "FixturePlayer" "PersonaId" "1" "UserId" "2"}}')
   elif scenario=='partial':(root/'surge.cfg').write_text('"Config2"{"User"{"Username" "Partial & User" "Currency" "EUR"}}')
   elif scenario=='override_only':(root/'surge_override.cfg').write_text('"Config2"{"User"{"Username" "OverrideUser"}}')
   elif scenario.startswith('cached'):
    (root/'surge.cfg').write_text('"Config2"{"User"{"UseLastOnlineProfile" "true"}}')
    cache=root/'localdata/SURGE';cache.mkdir(parents=True)
    (cache/'last_profile').write_text('41001;42001;CachedUser' if scenario=='cached' else 'invalid;fields')
    (cache/'last_avatar').write_bytes(b'synthetic avatar')
   env=dict(os.environ,LOCALAPPDATA=str(root/'localdata'),PROGRAMDATA=str(root/'programdata'),PRELOADER_L_FIXTURE_AVATAR_RECORD=str(root/'provider_avatar_calls.txt'),PRELOADER_L_FIXTURE_BAD_VERSION='1' if scenario=='bad_version' else '0',PRELOADER_L_FIXTURE_NULL_FACTORY='1' if scenario=='null_factory' else '0',PRELOADER_L_FIXTURE_AUTH_FAIL='1' if scenario.startswith('cached') else '0')
   command=[sys.executable,__file__,str(dll),'--child','--scenario',scenario]
   if a.online_fixture:command+=['--online-fixture',str(root/('anadius64online.dll' if scenario=='old_only' else 'preloader_l_online.dll'))]
   p=subprocess.run(command,env=env,cwd=root,capture_output=True,text=True,timeout=None if scenario=='bad_version' else 35)
   row={'scenario':scenario,'exit_code':p.returncode,'output':(p.stdout+p.stderr).replace(str(root),'${FIXTURE_ROOT}')};results.append(row);print(row['output'])
 result={'status':'PASS' if all(r['exit_code']==0 for r in results) else 'FAIL','scenarios':results,'scope':'real Windows loader/hooks/protocol/unload; synthetic files, no game process','manual_bad_version':'Required: original version-mismatch modal dialog must be observed and acknowledged; not silently auto-passed'}
 report=a.dll.parent/('NATIVE_DLL_ONLINE_FIXTURE_RESULT.json' if a.online_fixture else 'NATIVE_DLL_RESULT.json');report.write_text(json.dumps(result,indent=2)+'\n');return 0 if result['status']=='PASS' else 1
if __name__=='__main__':sys.exit(main())
