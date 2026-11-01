# FIX24 — TCP yazımı sonraki isteğin işlenmesini engellemez

NON_RELEASE tam kaynak devamı; exact FIX23 ebeveyninden türetildi. Karşılaştırma tam kullanıcı RE kaynak paketidir. Özgün DLL binary/kernel/allocator/ABI eşdeğerliği iddia edilmez.

## Somut sapma ve eski kodu reddeden test

FIX23 Session, bir isteği işledikten sonra yanıtın tamamını TCPye yazmayı bekliyor; sonraki complete frame ancak write completion içinden consume çağrılınca işleniyordu. Bu yalnız thread sayısı veya upstream Asio sürümü farkı değildir. Yanıtı okumayan istemci, daha önce gönderdiği ikinci isteğin executionını da durduruyordu.

Paket `anadius-server-source/server.cpp::frame_completed_025480` workerı başlatır, input prefixini tüketir ve okumayı tekrar kurar. Worker responseu socket executora dispatch eder. `do_write_028270` ayrı output dequeına ekler; yalnız ilk öğe writerı başlatır. `write_completed_025a70` frontu çıkarıp sonraki responseu gönderir. Frame kabulü ile önceki responseun tamamlanması arasında o gate yoktur.

Yeni testin önce bağımsız literal packet Session oracleı çalıştırıldı: ilk write completion tutulurken ikinci frame işlenir, read yeniden kurulur ve iki yanıt output kuyruğunda tutulur. Sonra aynı hedef davranış gerçek FIX23 Asio/TCP coreunda sınandı. Test executableına özel accept interpositionı accepted socket send bufferını küçültür; ilk4MiB response istemci tarafından okunmaz. Sonraki QueryImage isteği gerçek lazy-avatar dosyasını üretmelidir. Eski runtime bu assertionda reddedildi; source oracleı geçiyordu. Düzeltmeden sonra aynı test41 kontrolle geçti. Test beklentisi değiştirilmedi.

## Uygulama değişikliği

Yalnız src/transport/service.cpp runtimeı değişir:

- IO threadi NUL framing ve read admissiona devam eder; tam istekler sahipli request dequeına girer.
- Bir session için mevcut ordered compute/cipher sözleşmesi korunur; önceki compute tamamlandığında sıradaki iş başlar. Bunun için response TCP completionı beklenmez.
- Responses ayrı sahipli output dequeındadır. Tek async_write yalnız frontu kullanır; sonraki enqueue frontun bufferını değiştirmez. Write completion frontu pop edip yeni frontu başlatır.
- Greeting hâlâ asynchronous/cancellabledır; ilk greeting write tamamlanmadan normal read başlatılmaz. Wire greeting-before-request sırası korunur.
- Worker exceptionında failed atomic işaretlenir ve socket close IO executora post edilir. Read/write artık işlemle overlap yapabildiği için eski worker-thread close artık kullanılmaz. Cleanup postu OOMla başarısızsa false-ready bırakmadan IO/ready durur; mevcut joined host cleanup ownerı socketleri kapatır.

Pinned Asio `basic_stream_socket.hpp:46–55`, shared objectler ve özellikle close için thread-safety sınırını açıklar. Bunun için vendor değiştirilmedi. Soket IO-only/host-after-joins ownershipi yeni overlap altında korunur. Service stop/join/provider cleanup sırası değişmez; yeni cleanup timeout/retry değeri yoktur.

Admission ve output kuyrukları için yeni kullanıcı kotası veya CFG selectorı eklenmez. Slow reader varken birden çok response bellekte bekleyebilir; bu, source output queue modelinin somut sonucudur. Yeni keyfî memory/client/frame sınırı konmadı. Filesystem/path/profile/avatar/CFG/default/persistence/hook/provider ve upstream pinleri değiştirilmez. Testteki avatar dosyası yalnız executionın gözlenebilir kanıtıdır; avatar algoritması değiştirilmedi.

## Bilerek korunan farklar

Source per-frame detached workerlarının handler/dispatch tamamlanma sırası arrival sırasından farklı olabilir. Mevcut BLUEFIELD ordered Connection/cipher computationı korunur. Yeni request queue bu owned sıralamayı response completion gateinden ayırır; yeni concurrency veya source detached yarışları dayatılmaz. D02 pool/thread mimarisi ve D06 uygulama kilitleri bu düzeltmeyle kapanmış sayılmaz.

Source greeting sendi blocking write-all iken BLUEFIELD async greetingi, host stopun blocked greetingde IO thread joinine takılmaması için korunur. Bu reviewed integration tercihidir; kaynakla aynı native scheduling veya blocking/error/zero-progress davranışı denmez. Yeni socket close ownershipi dışında mevcut owned error/EOF/no-response/empty-frame politikaları korunur. Packet allocator/detach failure timing, send chunk/zero-progress, exception cleanup ve constructor/ABI ayrıntılarında tam eşdeğerlik iddiası yoktur. Literal oracleın source reset/scheduler çağrılarını göstermesi production constructor/native sayımlarının birebir doğrulandığı anlamına gelmez; production Connection initialization değiştirilmedi.

D03ün receive/write coupling sorunu düzeltildi; retained owned ordering/greeting farkları review ile görünür tutuldu. D03 statüsü bu nitelikli kapanışı söyler. Açık kaynak incelemeleri11→10: DEP05,D02,D06,D07,D13,D14,D15,D16,Q02,D21. Native validation yine ayrı NOT_RUNdır. Sadece sayı azaltmak için diğer açık kararlar doğru kabul edilmedi.

## Test/provenance

`tests/support/transport/packet_session.hpp`, orijinal server.hppdeki Error/Handler/Transport/Scheduling/Session declaration dilimlerini ve server.cppdeki Session bodylerini literal taşır. Yalnız standart prerequisite includelar, standalone receive_request_size declarationı, test namespacei ve SPDX eklenir. Header/body dilimleri ve origin SHAları PACKET_SESSION_ORACLE_FIX24.json içinde bağlıdır. Projection sadece test hedefine girer; ikinci production implementation veya araştırma dumpı değildir.

Yeni actual TCP fixture iki oturum sınar: response frontunun4MiB boyunca bütünlüğü + sonraki iki responseun FIFO sırası ve istemci responseu okumazken queued IO/work ile stop. Kontrol edilen küçük send buffer ve gerçek side effect, write gate counterexampleını ayırt eder. Linux interpositionı test executableına özeldir; production hook/selector eklenmez. Windows controlled-backpressure/IOCP yürütmesi NOT_RUNdır.

Önceki bütün C++ testleri/beklentileri byte-identical korunur. Mevcut Asio fragmentation/FIFO/300k-write/allocation-fault/accept-error/stop/repeated-lifecycle fixtureı ön koşuda59 kontrolle geçti. Yeni integration hedefiyle toplam45 grup vardır. Production/Debug ve değişen Service TU+actual core sanitizer sonuçları, source/package/PE ve targeted write-gate/drop-response negatifleri final digest/ZIP hashine bağlanır. Sanitizer kapsamı full core değilse receipt açıkça söyler; tüm core ASan kabulü uydurulmaz.

121 diğer src ve1.060 vendor dosyası aynıdır. Yeni production TU yoktur; tek test hedefi ve source oracleı eklenir. Golden araçları byte-identical kalır; synthetic Golden/PE native DLL kabulü sayılmaz. Windows/MSVC/DLL/IOCP/provider/hook/CRT/game/actual DLL Golden ve LSan NOT_RUN; iki mevcut GCC noreturn uyarısı sürer. Canonical pointer veya başka FIELD projesi değiştirilmez.
