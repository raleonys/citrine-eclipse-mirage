# B41 bağımsız kaynak incelemesi

**PASS — kapsamı sınırlı kaynak bileşimi. Üretim DLL teslimi BLOCKED.**

Sekiz dondurulmuş kaynak/test dosyasını bağımsız olarak inceledim. Üretim ve iki test yazarından bağımsızım; hiçbir EXE/DLL veya test çalıştırmadım. Kayıtlar ve dosya hashleri salt okunur doğrulandı. Tam ortak derleme bu incelemede **PENDING**.

Typed Profile yolu request/output prefixlerini gerçek `process_native_message_033d70` gövdesine verir. Aynı session cipher/flag kullanılır; exact owner + slot/generation ve Strings/Parent eşleşmesi doğrulanır. Diğer SessionLogic türlerinin semantic normal-return davranışı korunur.

Memory görünümü Parent'ın canlı heap bölgeleriyle invocation/event SSO alanlarını birleştirir. Callback ve allocation/free sonrasında yenilenir; çakışmalar reddedilir. Bilinen canlı büyük string header kayıtları korunur, free öncesi silinir. Native request mutasyonları ve event front'un callback sonrası canlı yeniden okunması korunur.

Nested XML/node/printer/cipher/factory/random/effects çağrıları, greeting ve event sınırları admission kontrolü yapar. Yakalanıp yutulan proxy exception ve node metadata allocation failure sticky poison bırakır. Raw opaque nesne pool kapasitesi callback'ten önce ayrılır; proxy allocation başarısız olsa bile dönen nesne tutulur. Normal sonuçta açıkça emekliye ayrılır. Logic dependency retirement, üyeler canlıyken Retiring mührü altında gerçekleşir.

Router'ın yeni GuardedPrinterAccess/GuardedNodeAccess yolları compact/text/CDATA ile attribute-order/GetText işlemlerini guarded raw backing üzerinde uygular. Eski MemoryPrinter/Element yolu korunur. Integration gerçek Router'ı kullanır; handler bypass yoktur.

Doğrulanan güncel kanıtlar:

- `/root/acceptor_tests`: **333** ana kontrol; ayrıca parent tarafından doğrulanan dokuz ayrı retained-prefix child çıkışı **231–239**. V3 EXE/log/record hashleri doğrulandı. Core doğrudan native.cpp + router.cpp derler; modified Profile.cpp veya integration testini çalıştırmış sayılmaz. Kontrollü metadata failure hem child235 hem child239'da kullanılır; kayıt source_scope içindeki eski “only child235” ifadesi final testin kapsamını dar tarif eder.
- `/root/b22_integration`: **99** kontrol, V4; yedi actual-use kaynak hash'i ve sekiz watched hash. Fresh modified Profile.cpp/router.cpp derlenmiştir. İki overlap plain/AES session, gerçek Router QueryEntitlements/GetConfig, canlı mutasyona uğramış request log'u, AES/hex event append, deferred EOF job ve source disposal akışları doğrulanmıştır. Exact wrong-generation retained child237 ayrıdır.
- Toplam **432** ana kontrol. Root yalnız immutable author kayıtlarını normalize etti; reviewer/root tekrar çalıştırması iddia edilmiyor.
- B40 ZIP `517848795b79a7f0bc0897fdd53b67516256d24ba09f246e2e62dba17b1099ac`: 894 eski kaynağın 890'ı aynı, dört prior değişikliği ve dört yeni dosyayla **898** kaynak. Kullanılan immutable archive `75db97cc59ce79eb1e0d75a0f6f7953cc390da47ecc4aac3a8674eb7de444568`, ZIP üyesiyle eşleşir.
- Planlanan ortak kapsam: **263 object / 259 CPP + 4 C / 311 ordinary group + 19 terminal**; composition **95 dosya / 47 object**, compiler observations **82**. Bunlar burada common-PASS iddiası değildir.

Native delta **0**, ABI/provider closure delta **0**. Mevcut **501 mapped / 505 eligible / 139 normal body / 52 historical closure** korunur; son yeni native body aşaması B38'dir.

Raw XML/printer/cipher destruction normal invocation sonuna ertelenen seçilmiş HOST politikasıdır. Unknown invocation temizlenmez/yeniden denenmez; unwind sonrası request/output stack referanslarını pinlemiş sayılmaz ve bunlar tekrar okunmaz. XML node/string_view/printer proxy referansları callback dışına kaçmamalıdır. Rich printer için gerçek recovered MemoryPrinter backing gerekir. Router'ın kendi information/content/licensing/IGO lower iç callbackleri genel olarak instrument edilmiş sayılmaz.

Aynı worker/fiber ve bütün Parent Operation zorunludur. XML/semantic032d00 ve event/SDK/formatter/scheduler/drain providers açık sözleşmelerdir; native DOM/printer/registry/stream ABI, native EH veya detached concurrency kanıtı yoktur. Full Resources, gerçek kernel/rawjob drain ve B31 loader attach/detach hooks açık kalır.

`work/b41-register.py` salt okunur staged preview'sunu ben hazırladım ve strict kontrolünü yaptım; üretim/test kodunu değiştirmedim veya pipeline apply çalıştırmadım. Bu rol açıklaması bağımsız production/test incelemesini korur; pipeline yazarlığı için bağımsızlık iddiası yoktur.

Kesin sekiz kaynak hash'i, current quick/actual record/log/EXE/archive hashleri ve sınırlamalar eşlik eden JSON'dadır.
