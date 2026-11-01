# B1 — Somut sağlayıcı ve sahiplik engelleri

27 Ekim 2026. Mevcut N29 kaynakları üzerinde sınırlı, salt okunur kompozisyon denetimi. Yeni native gövde, bridge veya closure: **0**.

**Ana sonuç:** Derlenen modüller var; bunları sahip olunan tek bir DLL nesne grafiğinde kuran somut üretim kökü henüz gösterilmiş değil. Abstract arayüzler arşivlenebilir; bu yüzden linker hatası olmaması tek başına yeterli değildir.

Kullanıcının yeni sırası: **1 DLL kritik yol → 2 mesaj zinciri → 3 XML işlevsel paketleri → 5 seçici CRT → 4 version preloader**.

| Sıra | Engel | Bitirme ölçütü |
|---:|---|---|
|1|One DLL-owned composition root and admission lifetime|Rooted application path can construct a complete concrete owner without test fixtures; shutdown covers pending workers and callback admission.|
|2|Concrete BootstrapDependencies facade|Concrete facade type is instantiable and process-attach order reaches configured workers in a controlled owned-source test.|
|3|Bind application worker bodies and their shared state|Each Bootstrap worker pointer resolves to rebuilt code with a live owned backend; no test-only static active fixture is used.|
|4|Complete host/callback policies around already concrete SDK bindings|The same owned state survives hook installation, admitted callbacks, disable and quiescent close; test branches cannot supply mock originals in production.|
|5|Own IO operation storage and executor/handler factories|One owned session can receive a frame, run completion/message scheduling and write a response without fixture-only project_* methods or escaped operation lifetimes.|
|6|Compose existing message, document, cipher and dispatcher providers|A representative source-only request reaches a registered handler and serialized response in one production composition; fixture-only adapters are absent from that build.|
|7|Bind mapped document storage to actual DOM view and child parser targets|Representative valid/error XML flows expose the same live document and error cell to parse, dispatch and cleanup; parser effects are not test-only.|
|8|Select runtime integration without restarting the whole CRT leaf backlog|Runtime root and application root are wired once with explicit owned tables and exact remaining phase5 blockers; no original preferred-image addresses are called.|
|9|End-to-end teardown and exception/ABI contract|Normal detach/rollback and late callback/FLS cleanup operate on live state; failure cannot be translated into fabricated API success.|

## B1-runtime-owner — One DLL-owned composition root and admission lifetime

**Mevcut kaynak:** Bootstrap normal application/process-attach orchestration; bootstrap_native Binding/Resolver and native callback Binding/Resolver have compiled implementations.

**Açık bağlantı:** No production owning object is identified that constructs one BootstrapDependencies implementation, Bootstrap, native bindings, configuration/state and late thread/FLS owners together; Bootstrap entry invokes an already-published binding; application_entry is not a self-initializing DLL entry; Closing active_calls==0 alone does not cover suspended/not-yet-admitted workers.

**Önerilen paket:** Define one explicit per-image owner and construction/rollback/shutdown sequence, before adding more leaf bodies. Choose host CRT entry or recovered CRT entry deliberately; do not initialize both accidentally.

**Kanıt:**
- `outputs/anadius-bootstrap-native-source/native_entries.hpp:21` — `// The binding does not register callbacks`
- `outputs/anadius-bootstrap-native-source/native_entries.cpp:105` — `BOOL WINAPI application_entry`
- `outputs/anadius-native-source/native_callbacks.hpp:29` — `struct Context {`

## B2-bootstrap-facade — Concrete BootstrapDependencies facade

**Mevcut kaynak:** BootstrapConfiguration binds setup_environment/setup_console/load_configuration; native ThreadBridge binds enum-to-rebuilt-code for existing begin-thread core; crt_glue::ThreadEffects has recovered thread/PTD/heap source implementations.

**Açık bağlantı:** BootstrapConfiguration and native ThreadBridge remain separate abstract subclasses of BootstrapDependencies; Their BootstrapDependencies bases are non-virtual; naive multiple inheritance creates two base subobjects; register_exit/install_terminate, log, allocation, worker factories, cleanup, guards and error policies require one coherent production facade.

**Önerilen paket:** Implement one concrete facade with explicit delegation to existing configuration/thread cores and typed native callback resolver; reuse 05e8e0 rather than rewriting it.

**Kanıt:**
- `outputs/anadius-source/bootstrap.hpp:25` — `class BootstrapDependencies`
- `outputs/anadius-config-startup-source/startup.hpp:20` — `class BootstrapConfiguration`
- `outputs/anadius-bootstrap-native-source/native_entries.hpp:62` — `class ThreadBridge`
- `outputs/anadius-bootstrap-native-source/native_entries.cpp:133` — `static_assert(std::is_abstract_v<ThreadBridge>)`
- `outputs/anadius-crt-glue-source/crt_glue.hpp:92` — `class ThreadEffects final`

## B3-application-workers — Bind application worker bodies and their shared state

**Mevcut kaynak:** server_start_029ea0 and socket partial backends exist; host_init initialize_host_00e310 plus callback-to-source resolver and hook engine adapters exist.

**Açık bağlantı:** Bootstrap worker()/boolean_worker_035850() still return externally supplied rebuilt function pointers; ServerBackend still owns initialize_context, pending socket, async accept, run and destruction boundaries; Host init callback Context, registry/network StateRefs and ten original trampoline slots must be one shared owner.

**Önerilen paket:** Provide rebuilt worker adapters with explicit owned contexts; connect actual normal algorithms to one context/registry and define thread admission/termination.

**Kanıt:**
- `outputs/anadius-source/bootstrap.hpp:37` — `virtual NoArgumentFunction worker`
- `outputs/anadius-source/bootstrap.cpp:43` — `deps_.boolean_worker_035850()(true)`
- `outputs/anadius-server-source/server.hpp:31` — `struct ServerBackend`
- `outputs/anadius-host-init-source/host_init.hpp:21` — `struct StateRefs`

## B4-native-host — Complete host/callback policies around already concrete SDK bindings

**Mevcut kaynak:** native OriginalEffects implements ten original SDK trampoline calls; native Resolver returns compiled rebuilt callback addresses; HostInitEffects/CleanupEffects/PreloaderEngine hook adapters are concrete.

**Açık bağlantı:** OriginalEffects intentionally remains abstract for allocation/log/wait/narrow/copy/UI policies and WideOperations; Hook HostInitEffects wraps a remaining Effects reference rather than creating a complete Win32/log/encoding backend; Errors policies and callback/state lifetime owners are mandatory.

**Önerilen paket:** Supply the remaining concrete host policy and UTF-16 byte-safe operations, sharing the original-slot array with hook creation and callback forwarding.

**Kanıt:**
- `outputs/anadius-native-source/native_callbacks.hpp:70` — `class OriginalEffects`
- `outputs/anadius-native-source/native_callbacks.hpp:61` — `class WideOperations`
- `outputs/hook-engine-source/adapters.hpp:15` — `class HostInitEffects final`

## B5-io-storage — Own IO operation storage and executor/handler factories

**Mevcut kaynak:** Receive R1/R2/R3/R4 normal bodies are source-bound; Send/composed-write/queued-write/dispatcher bodies and selected SDK adapters exist.

**Açık bağlantı:** PostedMemoryEffects and HandlerDispatchMemory require real typed allocation/projection/free ownership; Operation registry, IoHandlerVtable destroy/move/query, environment factory/type query and scheduling callback tables need one coherent backend; AddressBackend/AcceptorBackend/SynchronousTransport are partial bridges, not a full ServerBackend/Transport/Scheduling implementation.

**Önerilen paket:** Create one bounded typed operation owner/registry, then provide executor/handler factory bindings; connect source receive and send in that same owner.

**Kanıt:**
- `outputs/anadius-socket-source/posted_operation.hpp:35` — `class PostedMemoryEffects`
- `outputs/anadius-socket-source/handler_dispatch.hpp:38` — `class HandlerDispatchMemory`
- `outputs/anadius-socket-source/io_completion.hpp:6` — `class IoHandlerVtable`
- `outputs/anadius-socket-source/operations.hpp:54` — `class SynchronousTransport`
- `outputs/anadius-socket-source/acceptor.hpp:48` — `class AcceptorBackend`

## B6-message-composition — Compose existing message, document, cipher and dispatcher providers

**Mevcut kaynak:** RecoveredNativeMessageApi closes process_message_033d70 in the frame/response hierarchy; RecoveredMessageDocumentApi extends cipher/append/native dispatch hierarchy with three document calls; SessionCipher and factory provide selected owned cipher state.

**Açık bağlantı:** NativeMessageContext still takes a NativeMessageApi&; RecoveredNativeMessageApi itself belongs to MessageDispatchApi hierarchy and is not the complete NativeMessageApi implementation; NativeDispatchEffects registry/DOM/printer/string/handler/exception hooks currently have controlled direct implementations in native_dispatch_test_fixture.hpp; Event queue/mutex/logging/error providers need one production adapter sharing live handler/cipher/document state.

**Önerilen paket:** Build one concrete NativeMessageApi facade using the existing source bridges, plus a concrete NativeDispatchEffects backend; connect it to the same frame owner and event queue.

**Kanıt:**
- `outputs/anadius-socket-source/native_message.hpp:17` — `class NativeMessageApi`
- `outputs/anadius-socket-source/native_message.hpp:40` — `struct NativeMessageContext`
- `outputs/anadius-socket-source/native_message.hpp:49` — `class RecoveredNativeMessageApi`
- `outputs/anadius-socket-source/native_document.hpp:43` — `class RecoveredMessageDocumentApi`
- `outputs/anadius-socket-source/native_dispatch.hpp:9` — `class NativeDispatchEffects`
- `outputs/anadius-socket-source/native_dispatch_test_fixture.hpp:28` — `// Controlled lower providers.`

## B7-xml-bridge — Bind mapped document storage to actual DOM view and child parser targets

**Mevcut kaynak:** N29 RecoveredNodeStringEffects includes prior constructor/Identify/insertion/GetStr source chain; Known 004970 target dispatch is selected and source-bound; Existing pool allocation/free/clear/destructor normal bodies are ready.

**Açık bağlantı:** NativeDispatchEffects::document requires a messages_recovered::XmlDocument view over the parsed document; no automatic raw mapped-byte to DOM conversion is supplied by NativeMessageDocument; NativeMessageDocument only projects error_6c; invocation storage and binding must exist; Other child parse virtuals, node probes/destructors, numeric decode, copy/compare/classifier and error argument/scratch transport remain mandatory.

**Önerilen paket:** First choose the XML representation/provider used by one real message composition; implement only child/parser/DOM adapters on its critical path, with numeric0033c0 as one dependency rather than the global next task.

**Kanıt:**
- `outputs/anadius-socket-source/native_message.hpp:10` — `struct NativeMessageDocument`
- `outputs/anadius-socket-source/native_dispatch.hpp:16` — `virtual const messages_recovered::XmlDocument& document`
- `outputs/anadius-socket-source/native_document.hpp:40` — `struct NativeDocumentBinding`
- `outputs/anadius-socket-source/node_parse.hpp:55` — `virtual std::uintptr_t parse_other_virtual`
- `outputs/anadius-socket-source/node_string.hpp:8` — `class NativeNodeStringEffects`

## B8-selected-crt — Select runtime integration without restarting the whole CRT leaf backlog

**Mevcut kaynak:** crt_entry::Entry normal entry/dispatch and AcrtTablesBridge/LifecycleBridge/StartupBridge exist; Pair bindings dispatch16 rebuilt ACRT initialization pairs; Native FLS/PTD/heap/lock components have separate concrete source adapters.

**Açık bağlantı:** StartupBridge still requires concrete security cookie, argv, application callback, TLS image validation/invocation and fastfail policies; crt_pair_bindings::Effects leaves invoke_void_initializer/invoke_int_initializer/common_flush_all pure; Canonical shared state/cookie/cache/heap/PTD/locale/MB/onexit aliases and loader/runtime entry policy are not created by linking a static archive.

**Önerilen paket:** Record exact startup-link runtime dependencies now, defer generic CRT body reconstruction to selected phase5; choose a single justified runtime entry policy.

**Kanıt:**
- `outputs/anadius-crt-entry-source/entry.hpp:20` — `struct Effects {`
- `outputs/anadius-crt-startup-bridge-source/startup_bridge.hpp:34` — `class StartupBridge`
- `outputs/anadius-crt-pair-bindings-source/bindings.hpp:70` — `// invoke_void_initializer, invoke_int_initializer and common_flush_all`
- `outputs/anadius-crt-entry-source/entry.hpp:54` — `virtual std::uint32_t application_dll_main_038870`

## B9-shutdown-errors — End-to-end teardown and exception/ABI contract

**Mevcut kaynak:** Bootstrap on_exit and callback lease/error admission machinery exist; Source close/cancel/cleanup bodies are implemented.

**Açık bağlantı:** Detached/raw captured session pointers and pending/suspended callbacks must quiesce before owner teardown; Native bootstrap/native callback Errors policies are abstract and require real no-return implementations; Current LLVM-MinGW host STL/types are source projections, not original MSVC STL/EH/SEH/native image layout proof.

**Önerilen paket:** Add owner-level stop/admission/join/cleanup ordering and an explicit ABI boundary policy, tested as one lifecycle rather than isolated leaf cleanup.

**Kanıt:**
- `outputs/anadius-bootstrap-native-source/native_entries.hpp:26` — `// active_calls()==0 does not cover suspended`
- `outputs/anadius-source/bootstrap.cpp:69` — `void Bootstrap::on_exit()`
- `outputs/anadius-native-source/native_callbacks.hpp:27` — `[[noreturn]] virtual void callback_exception`

## Yorum sınırı

680 CPP/HPP dosyasında build klasörleri dışarıda bırakılarak adlandırılmış direct-derived sınıflar tarandı. Eşleşmeler JSON içinde test/fixture ve diğer kaynaklar olarak ayrılmıştır. Bu tarama alias/makro üzerinden bütün C++ tür grafiğinin ispatı değildir. Açık yöntemler, arayüz ilişkileri ve native_entries.cpp abstract static_assert ayrıca dayanak oluşturur.

52 historical typed source closure korunur; bunların tekrar yazılması önerilmez. Numeric0033c0 tek başına tüm DLL için sıradaki iş sayılmaz. Önce ortak owner/somut backend ve bir mesaj senaryosunun ihtiyaçları seçilmelidir.

Orijinaller okunmadı/çalıştırılmadı/değiştirilmedi; gerçek CFG/token/achievement içeriği okunmadı; engelli version.dll girdisi/üretilmiş DLL tekrar denenmedi. Ortak build ve paketleme çalıştırılmadı.
