#!/usr/bin/env python3
"""Stage04E real XML/Grant/AES/FIFO owner source regression; not Windows IOCP."""
import os, pathlib, shutil, subprocess, tempfile, sys, contextlib
r=pathlib.Path(__file__).resolve().parents[1]
# Exact closed source linkage follows the established Wave7 coherence inventory,
# augmented with the same production Stage04A/B owner and async driver.
xml=['src/xml/request_parser.cpp','third_party/tinyxml2/tinyxml2.cpp','src/xml/request_children.cpp',
'src/xml/lsx_framing.cpp','src/xml/response_builder.cpp','src/xml/value_codec.cpp',
'src/xml/value_codec_utf8.cpp','src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp',
'src/xml/attribute_parser.cpp']
config=['src/config/config2_pegtl.cpp','src/config/settings_values.cpp','src/config/settings_provider.cpp',
'src/config/runtime_provider.cpp','src/config/runtime_configuration.cpp',
'src/config/runtime_configuration_parser.cpp','src/config/runtime_configuration_storage.cpp']
state=['src/runtime/service_state.cpp','src/runtime/service_state_storage.cpp',
'src/runtime/service_state_parser.cpp','src/runtime/service_state_grant.cpp',
'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp','src/runtime/utc_time.cpp',
'src/runtime/auth_code_service.cpp','src/runtime/achievement_watcher.cpp',
'src/runtime/local_social_state.cpp','src/runtime/local_social_host.cpp']
handlers=['src/handlers/progressive_installation_route.cpp','src/handlers/permission_route.cpp',
'src/handlers/friends_route.cpp','src/handlers/integration_routes.cpp',
'src/handlers/group_info_route.cpp','src/handlers/utc_time_route.cpp','src/handlers/query_presence_route.cpp',
'src/handlers/social_mutation_routes.cpp','src/handlers/social_mutation_socket.cpp',
'src/handlers/local_social_list_route.cpp','src/handlers/config_route.cpp',
'src/handlers/setting_value_route.cpp','src/handlers/settings_snapshot_route.cpp',
'src/handlers/handler_registry.cpp','src/handlers/connection_state_route.cpp',
'src/handlers/connection_state_binding.cpp','src/handlers/runtime_settings_routes.cpp',
'src/handlers/selector_pipeline.cpp','src/handlers/game_info_routes.cpp',
'src/handlers/profile_routes.cpp','src/handlers/auth_routes.cpp','src/handlers/store_routes.cpp',
'src/handlers/achievement_routes.cpp','src/handlers/achievement_grant_route.cpp',
'src/handlers/achievement_query_route.cpp','src/handlers/image_route.cpp']
core=['src/protocol/socket_exchange_transaction.cpp','src/protocol/socket_exchange_transaction_delivery.cpp',
'src/protocol/socket_exchange_async_transaction.cpp','src/protocol/socket_exchange_write_completion.cpp',
'src/protocol/socket_exchange.cpp','src/protocol/socket_exchange_wire.cpp',
'src/protocol/socket_exchange_frame_step.cpp','src/protocol/challenge_handshake.cpp',
'src/net/socket_transport.cpp','src/codec/session_crypto.cpp','src/codec/message_codec.cpp',
'src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp',
'src/router/route_lookup.cpp','src/router/route_inventory.cpp',
'src/core/request_dispatch.cpp','src/hash/fnv_hash.cpp','src/runtime/event_queue.cpp']
sources=['tests/unit/test_asio_full_grant_dispatch.cpp']+core+xml+config+state+handlers
assert len(sources)==len(set(sources)),"duplicate TU"
for rel in sources:
 assert (r/rel).is_file(),rel
assert 'ASIO_DISABLE_IOCP' not in (r/'CMakeLists.txt').read_text()
native=(r/'tests/native/test_asio_iocp_full_grant_dispatch.cpp').read_text()
for marker in ('prepare_response_transaction(','decode_payload(',
 'asio::async_write(', 'real XML dispatcher+GrantAchievement+AES prepare',
 'defined(ASIO_DISABLE_IOCP)', 'progress()==0'):
 assert marker in native,marker
runner=(r/'scripts/test_asio_iocp_native.py').read_text()
assert "'dispatch'" in runner
for compiler in ('g++','clang++'):
 if not shutil.which(compiler):sys.exit('Missing compiler '+compiler)
 with contextlib.nullcontext(str(pathlib.Path('/mnt/data/hf12_stage04e_compile_cache') / compiler.replace('+','p'))) as td:
  d=pathlib.Path(td);d.mkdir(parents=True,exist_ok=True)
  common=['-std=c++20','-O0','-pthread','-Wno-unused-function',
   '-I'+str(r/'include'),'-I'+str(r/'src/protocol'),
   '-I'+str(r/'third_party/tinyxml2'),
   '-I'+str(r/'third_party/pegtl/upstream/include')]
  objects=[]
  for idx,rel in enumerate(sources):
   obj=d/f'{idx}.o'
   as_win=rel in ('src/protocol/socket_exchange_transaction.cpp',
                  'src/protocol/socket_exchange_transaction_delivery.cpp',
                  'src/runtime/event_queue.cpp','src/runtime/memory_allocator.cpp',
                  'src/runtime/ptd_runtime.cpp','src/handlers/achievement_grant_route.cpp',
                  'tests/unit/test_asio_full_grant_dispatch.cpp')
   flags=['-D_WIN32','-D__declspec(x)=','-D__stdcall='] if as_win else []
   args=[compiler,*common,*flags,'-c',str(r/rel),'-o',str(obj)]
   if obj.exists() and obj.stat().st_mtime_ns >= (r/rel).stat().st_mtime_ns:
    objects.append(str(obj));continue
   result=subprocess.run(args,capture_output=True,text=True,timeout=75)
   if result.returncode:
    print('FAIL',compiler,rel,'\n',result.stderr[-5000:]);sys.exit(1)
   objects.append(str(obj))
  exe=d/'stage04e'
  result=subprocess.run([compiler,'-pthread',*objects,'-o',str(exe)],capture_output=True,text=True,timeout=60)
  if result.returncode:
   print('FAIL LINK',compiler,result.stderr[-6000:]);sys.exit(1)
  run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=40)
  print(compiler,run.stdout.strip(),run.stderr.strip(),flush=True)
  if run.returncode or 'STAGE04E_REAL_DISPATCH_GRANT_CODEC_CALLBACK PASS' not in run.stdout:sys.exit(1)
print('PASS Stage04E GCC/Clang real dispatcher/Grant/AES/FIFO callback; Windows IOCP NOT_RUN')
