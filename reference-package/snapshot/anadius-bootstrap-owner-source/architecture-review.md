# B2 — Bağımsız mimari ve yaşam süresi incelemesi

27 Ekim2026. **PASS — bounded statik kaynak incelemesi.** Üretim yazarı dışındaki `/root/b2_architecture_audit` tarafından exact CPP/HPP ve önceki Bootstrap/config/native-entry/thread sözleşmeleri karşılaştırıldı. Yeni native gövde/closure **0**.

## Sabit kaynaklar

- `owner.hpp` SHA-256`f12f284f5f23e27ebc320e9c5ac24fa016b5116dcbd615a566790568ca78e003`
- `owner.cpp` SHA-256`6cb82edd31ac5617f6679f97a0d1eb04fce10644b802ed314950fca6cc16d9c3`

## Doğrulanan bileşim

1. **single_base** — BootstrapFacade derives only BootstrapConfiguration; no second ThreadBridge/BootstrapDependencies subobject. (`outputs/anadius-bootstrap-owner-source/owner.hpp:56`).
2. **config_reuse** — Three inherited final configuration methods use prior startup/config source with the owned state and flags. (`outputs/anadius-bootstrap-owner-source/owner.cpp:23`).
3. **thread_reuse** — begin_thread routes through the existing05e8e0 source and rebuilt native Resolver, preserving security0/stack0/flags0 and outputID. (`outputs/anadius-bootstrap-owner-source/owner.cpp:40`).
4. **handle_identity** — Detached-handle closure uses the same cached threads Effects provider. (`outputs/anadius-bootstrap-owner-source/owner.cpp:43`).
5. **callback_identity** — Exit/terminate enums map to compiled rebuilt callback functions; unsupported selector values fail explicitly. (`outputs/anadius-bootstrap-owner-source/owner.cpp:24`).
6. **mandatory_services** — All remaining Services methods are mandatory forwarding policies; their native implementation/lifetime contracts remain open. (`outputs/anadius-bootstrap-owner-source/owner.hpp:12`).
7. **state_ownership** — RuntimeOwner owns the configuration seed and three console flag bytes; it does not manufacture native initial globals. (`outputs/anadius-bootstrap-owner-source/owner.hpp:103`).
8. **backend_lifetime** — Backend is uniquely owned and declared after state/flags so reverse destruction keeps referenced state alive during backend destruction. (`outputs/anadius-bootstrap-owner-source/owner.hpp:104`).
9. **cached_providers** — providers() is called once during construction and the references are reused by one facade. (`outputs/anadius-bootstrap-owner-source/owner.cpp:57`).
10. **null_backend** — A missing provider owner fails before publishing or constructing a live binding. (`outputs/anadius-bootstrap-owner-source/owner.cpp:8`).
11. **state_binding** — bind_state receives the final owned state after all facade/bootstrap fields have initialized; its no-publication/no-workers contract is explicit. (`outputs/anadius-bootstrap-owner-source/owner.cpp:58`).
12. **delayed_publication** — publish is allowed only in Constructed; global binding publication happens after the RuntimeOwner constructor has completed. (`outputs/anadius-bootstrap-owner-source/owner.cpp:67`).
13. **permanent_errors** — Errors is an external borrowed process-lifetime policy and explicitly outlives post-close/post-destruction entry attempts, matching retained native last_errors. (`outputs/anadius-bootstrap-owner-source/owner.hpp:84`).
14. **stable_address** — Copy and move are both disabled; published Bootstrap/dependency addresses remain stable. (`outputs/anadius-bootstrap-owner-source/owner.hpp:94`).
15. **controlled_entry_access** — Owner exposes application entry only in Bound phase and exposes no mutable facade/bootstrap getter. (`outputs/anadius-bootstrap-owner-source/owner.cpp:83`).
16. **quiescence_contract** — stop_and_join must stop all admission and future callbacks, including pending/suspended workers; active_calls0 alone is insufficient. (`outputs/anadius-bootstrap-owner-source/owner.hpp:47`).
17. **failed_close** — False/exception from stop_and_join leaves the backend, bootstrap and binding alive in Stopping for explicit retry; no implicit rollback/free. (`outputs/anadius-bootstrap-owner-source/owner.cpp:76`).
18. **busy_close** — Busy native Binding::close rejection likewise occurs before reset/Closed and retains the published owner. (`outputs/anadius-bootstrap-owner-source/owner.cpp:77`).
19. **successful_close** — After quiescence and successful native close, the optional binding resets before phase becomes Closed; repeated close is idempotent. (`outputs/anadius-bootstrap-owner-source/owner.cpp:73`).
20. **destructor_contract** — Busy control or Bound/Stopping destruction terminates rather than silently unpublishing or freeing resources whose pending callbacks may still exist. (`outputs/anadius-bootstrap-owner-source/owner.cpp:63`).
21. **reentrancy_guard** — A control scope rejects reentrant publish/close before changing the phase, preserves the outer busy flag on rejected nested construction, and clears the flag after false returns or exceptions. (`outputs/anadius-bootstrap-owner-source/owner.cpp:15`).
22. **entry_strategy** — This source supplies application-layer composition only: no PE entry, CRT initialization, double-CRT route,export body or native ABI completion is claimed. (`outputs/anadius-bootstrap-owner-source/owner.hpp:81`).
23. **concrete_types** — Compile-time checks require a nonabstract facade/owner, one unambiguous BootstrapDependencies conversion, and stable nonmovable owner. (`outputs/anadius-bootstrap-owner-source/owner.cpp:86`).

## Açık sınırlar

- No concrete OS/application ProviderOwner is supplied. providers/bind_state/quiescence contracts remain required from B3 and later source integration.
- Services forwarding does not close its17 native policy implementations; worker pointers still need rebuilt worker/backend/global state owners.
- stop_and_join must exclude late exit/terminate registration callbacks as well as worker callbacks. Without that concrete guarantee successful close is not established.
- Control lifecycle calls belong to one serialized control thread; reentry is explicitly rejected. The control flag is not a cross-thread lock or synchronization mechanism.
- Compiler/recovered CRT entry strategy, TLS/global startup, physical MSVC object ABI, exceptions/SEH and full DLL lifecycle remain open.
- No native function body, existing typed native provider closure, original-image execution or DLL entry installation is added by this composition.

Önceki691 kaynak dosyası hash karşılaştırmasında aynı kaldı. Bu inceleme test çalıştırmadı veya kaynak değiştirmedi; orijinaller okunmadı, değiştirilmedi veya çalıştırılmadı. Tam üretim DLL hazırlığı **BLOCKED**; facade/owner tiplerinin somut olması native backend/global/CRT/ABI/EH tamamlanması anlamına gelmez.
