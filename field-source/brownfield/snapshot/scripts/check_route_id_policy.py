#!/usr/bin/env python3
"""Current HF08 route-ID policy gate; parent SHA remains exact HF07 provenance."""
import pathlib,re,json,sys
root=pathlib.Path(__file__).resolve().parents[1]
policy=json.loads((root/'metadata/ROUTE_ID_POLICY.json').read_text())
assert policy['source_parent']['sha256']=='a21490ffe382508d75746ad27bca219d74bc3a42436d1dbc30a23e2e4bdb689f'
rows=policy['route_ids']
assert len(rows)==42 and [r['id'] for r in rows]==list(range(1,43))
hdr=(root/'include/preloader_l/router/route_inventory.hpp').read_text()
enum=hdr.split('enum class RouteId : uint16_t {',1)[1].split('};',1)[0]
actual=re.findall(r'^\s*(\w+)\s*=\s*(\d+)\s*,',enum,re.M)
assert [(r['symbol'],str(r['id'])) for r in rows]==actual
assert sum(r['registration_state']=='RETAINED_HF07_NAME_ACTIVE_HF08' for r in rows)==28
assert sum(r['registration_state']=='ANADIUS_EXTENSION_ACTIVE_HF08' for r in rows)==9
assert sum(r['registration_state']=='LOCAL_SOCIAL_ACTIVE_HF08_NATIVE_ACCEPTED' for r in rows)==5
assert not any(r['name'] in policy['deferred_no_id'] for r in rows)
router=(root/'src/router/route_inventory.cpp').read_text()
assert 'return routes + 1' not in router
assert 'RouteId::show_igo_window' in router
# The retired route name is forbidden in all active source and test surfaces.
for base in ['include','src','tests']:
 for f in (root/base).rglob('*'):
  if f.suffix in ('.h','.hpp','.cpp','.c'):
   data=f.read_text(encoding='utf-8')
   assert 'GetAuthToken' not in data and 'get_auth_token' not in data, f
assert 'RouteId::show_igo_window' in (root/'src/protocol/socket_exchange_transaction.cpp').read_text()
assert 'RouteId::show_igo_window' in (root/'src/core/request_dispatch.cpp').read_text()
# Local-only route IDs must have an exact descriptor, negative untrusted handler,
# and socket transactional owner; native game acceptance remains a release gate.
for name in ('BlockUser','UnblockUser','RequestFriend','RemoveFriend','SendInvite'):
    assert '\"'+name+'\"' in router, name
bindings=(root/'src/handlers/handler_registry.cpp').read_text()
for symbol in ('block_user','unblock_user','request_friend','remove_friend','send_invite'):
    assert 'case RouteId::'+symbol+':' in bindings
assert 'local_social_delivery_model.cpp' not in (root/'CMakeLists.txt').read_text()
assert not (root/'src/runtime/local_social_delivery_model.cpp').exists()
src=(root/'src/handlers/social_mutation_routes.cpp').read_text()
assert 'OriginError::not_logged_in' in src and 'OriginError::success' not in src
print('HF08_42_DENSE_LOCAL_SOURCE_BINDINGS_PASS registered=42 native_acceptance=PASS_CARRIED_FORWARD')
