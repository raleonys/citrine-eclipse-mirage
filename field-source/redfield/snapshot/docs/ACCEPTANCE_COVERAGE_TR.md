# Debug/native/CRT kabul kapsamı — RF32

LOCAL_BUILD / NOT_RELEASE.29 eski native/CRT senaryosu iki canonical betikten
aynı sırayla çıkarıldı;9 ek host/CMake/export referansı HF19 ZIP'inden exact alındı.
Bu dosyalar immutable provenance'tır;uygulamaya eski runtime yeniden eklenmedi.
Tablo mevcut testlerin ilişkisini gösterir;hiçbir satır tam native parity PASS değildir.
Gerçek Windows/MSVC ve oyun bu tur NOT_RUN.

| Eski senaryo | Mevcut portable testler | Açık kalan kapsam |
|---|---|---|
|reload_generation|test_config_snapshot_view,test_service_session|Actual DLL reload generations/config mutation isolation pending|
|shipping_shutdown|test_worker_owner,test_session_retirement|Current public ABI3 cycles;old shipping GetConfig black-box path pending|
|zero|test_tcp_ingress|Exact zero-client lifecycle scenario pending|
|one|test_tcp_ingress|Current one-client native path;oldscenario assertions not fully matched|
|growth|test_tcp_ingress|Multi-client native growth scenario pending|
|parallel|test_tcp_delivery|Parallel live native client ownership andshutdown scenario pending|
|active_shutdown|test_pending_session_cleanup,test_session_retirement|Current connected client closure;old active-shutdown stress rounds pending|
|blocked_shutdown|test_worker_owner,test_transient_shutdown|Native blocked completion/timeout ownership andfour stress rounds pending|
|f02_delivery_stall_parallel|test_tcp_delivery,test_tcp_grant_failure|Native delayed completion/fault controls andparallel scenario pending|
|ids07_send_idle_timeout|test_asio_session_timeout_policy|Native real idle timeout/send-progress scenario pending|
|connect_redirect_lifecycle|test_connect_redirect|Current targeted connect/registry retry;generation/direct-target bypass/native restoration assertions pending|
|wave4_resource_retry|test_worker_owner,test_transient_retry_concurrency|Current close/unpin andregistry pending owner retries;full historical resource fault matrix pending|
|path_ascii_sso7|test_service_session|Module-relative config implemented;old exact ASCII path selection pending|
|path_unicode_heap|test_service_session|Current native workspace Unicode path exists;old long/replace scenario assertions pending|
|path_long_replace|test_service_session|Native long-path/config replacement pending|
|xml_no_bom|test_config,test_xml|Old native Config XML scenario uses changed parser/config boundaries;explicit destination equivalence review pending|
|xml_bom|test_config,test_xml|Native BOM/config boundary equivalence pending|
|xml_large|test_config,test_xml|Native large XML/config ownership andfailure equivalence pending|
|xml_invalid_retain|test_config,test_config_snapshot_view|Current invalid Config2 startup tested;old live-retain semantics need separate mapping|
|xml_replace|test_config_snapshot_view|Native live source replacement parity pending|
|xml_clear|test_config|Native clear-versus-empty semantics pending|
|prestart_stop|test_worker_owner|Current native10 early-stop cycles;oldscenario whole lifecycle equivalence pending|
|ptd_fls_lifecycle|test_ptd_lifecycle,test_ptd_fls_free|Old custom PTD/FLS model is tests/support only;production supported /MT CRT;actual platform runtime lifetime acceptance required,not model-test substitution|
|frame_fragmentation|test_framing,test_tcp_ingress|Current native fragment+coalesce3 replies;fullold malformed/connection boundary scenario pending|
|frame_coalescing|test_framing,test_tcp_ingress|Current native coalescing covered inonecase;fulloldnative assertions pending|
|frame_malformed|test_framing,test_ingress|Native malformed frame isolation scenario pending|
|protocol_route_surface|test_router,test_tcp_profile_time,test_tcp_settings|35bindings/7deferred;SDK handshake/sessioncrypto intentionallydeferred;fulloldroute/native parity not implied|
|store_content_entitlement|test_tcp_store_replies,test_tcp_content,test_tcp_entitlements|Portable wire tests exist;native end-to-end scenario pending|
|achievement_event_delivery|test_tcp_achievements,test_achievement_delivery_wire|Portable event delivery exists;native state/timeout/event delivery scenario pending|

active_shutdown ve blocked_shutdown için her native/CRT akışındaki ilave4 stress
round aktarılmadı.Debug/runtime/session/socket adlı4 eski Debug host ve ek tanı
API'leri henüz taşınmadı.Mevcut Debug /MTd+symbols+public ABI host'tur;eski debug
prototype parity iddiası yok.CRT shipping black-box GetConfig veCRT profilinin
Golden kabulü de ayrı açık kapsam.Production Golden zinciri RF31'de mevcut.
PTD/FLS generic model testleri production UCRT/Windows lifetime kabulü yerine geçmez.
SDK handshake/sessioncrypto ve7route bilinçli deferred olarak takip edilir.
