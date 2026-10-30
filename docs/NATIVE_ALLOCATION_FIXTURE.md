# Native hata enjeksiyonu: Asio tahsis yolu

Sahibin gönderdiği Windows/MSVC loglarında Compile Only, Production/LOCAL_GOLDEN,
Debug ve Static CRT profilleri başarılıdır. Native Acceptance204/205; yalnız
net_failure_contracts, `accept initiation allocation failure was not injected`
ile ilk senaryoda durmuştur. Static CRT6/6 geçmiştir. Oyun kabulü bu loglarda
bildirilmemiştir. Üretim profillerinde warning yok; native test derlemesindeki12
uyarı kaydı test fixture'larından gelir, bu dar düzeltmeyle giderilmiş sayılmaz.

Özgün girdiler ve özet [OWNER_LOG_REVIEW.json](../metadata/evidence/native-allocation/OWNER_LOG_REVIEW.json)
içindedir. Production logunun bildirdiği son DLL SHA256:
`5d75523d82a575277e1c5e91395637d301f0f981913c8358deee02c3c3cc24fc`.
Bu DLL bu yüklemede verilmedi; hash logdan okunmuştur. Bunlar önceki kaynak
snapshot'ının kullanıcı ortamı kanıtlarıdır; yeni testin native kabulü değildir.

## Neden ve düzeltme

Eski test yalnız global operator new/new[] çağrısını bir thread-local flag ile
başarısız yapıyordu. Pin'li Asio'nun detail/memory.hpp dosyası MinGW yolunda
operator new kullanırken MSVC yolunda `_aligned_malloc` çağırır; null sonucu
Asio tarafından bad_alloc'a çevrilir. Testin flag'i o yolda okunmadığından accept
işlemi normal başlatılıyor ve test, enjeksiyon oluşmadığını bildiriyordu. Bu
mesaj gerçek bir accept başarısızlığı veya stale-pending bayrağı kanıtı değildir.

Test, `_aligned_malloc` çağrısını yalnız kendi executable'ında ve yalnız Asio'nun
memory helper header'ı işlenirken kapsar. Arızada null döndürür; bad_alloc'u
gerçek Asio helper'ı üretir. Başarılı çağrı orijinal CRT allocator'a gider ve
normal aligned free eşleşmesi korunur. Global scalar new arızası da tutulur.
Helper'ın standart/vendor bağımlı header'ları macro'dan önce işlenir; mapping
STL/CRT header gövdelerine sızmaz. Helper önceden işlenmişse veya başka bir CRT
macro mapping'i varsa fixture sessizce devam etmez, compile-time hata verir.
Vendor dosyaları, ürün src/include, production derleyici seçenekleri, listener,
handler, rollback ve shutdown kodları değiştirilmez.

Hangi allocator yolunun gerçekten tüketildiği ve tam bir enjeksiyon gerçekleştiği
assert edilir. Post-failure senaryosu da enjeksiyon sayısını doğrular; stop-oom
senaryosu çağıran thread'deki iki izlenen tahsis yolunda arıza tüketilmediğini
denetler. Bu, bütün malloc/HeapAlloc çağrıları veya bütün thread'ler için global
"hiç tahsis yok" ispatı değildir. Yeni ve desteklenmeyen std/Boost aligned branch
görülürse fixture compile-time hata verir; sessizce yeşil sonuç üretmez.

## Platformlar arası regresyon

Normal net_failure_contracts hedefi MSVC'de doğal CRT aligned yolunu, MinGW'de
normal compiler yolunu kullanır. MinGW'ye ayrıca
net_failure_msvc_memory_replay_contracts eklenmiştir. Bu test yalnız memory.hpp
işlenirken MSVC allocation branch'ini seçer, sonra geçici macro'yu geri alır.
Geri kalan Asio platform/compiler algısı ve gerçek Windows IOCP/socket/timer
işlemleri aynı kalır. Bu bir MSVC derleyicisi emülasyonu veya native MSVC kabulü
değildir; pin'li helper'ın tahsis yolunu gerçek Windows CRT üstünde canlandırır.

İzole kontrolde eski test bu yolla aynı hatayı verdi; düzeltilmiş testin normal ve
aligned yollarının her birinde accept, post, stop-oom, io-exception ve drain-retry
başarılı oldu. Ürünün queue_accept catch/reset davranışı değiştirilmedi. Assert
kaldırılmadı, test skip edilmedi, deadline/bütçe büyütülmedi ve production Asio
bayrakları değiştirilmedi.

MSVC Native Acceptance205 test olarak kalır; MinGW çapraz suite ek replay ile206
testtir. Her ikisinin sayısı ve yürütme sınıfı [VALIDATION.json](../metadata/evidence/native-allocation/VALIDATION.json)
içinde ayrı tutulur. Önceki204/205 Windows sonucu düzeltilmiş test205/205 geçti
diye yeniden etiketlenmez; native Windows tekrarına hâlâ ihtiyaç vardır.

Güncel sözleşme [NATIVE_FIXTURE_REVIEW.json](../metadata/NATIVE_FIXTURE_REVIEW.json).
Kaynak paketinin temiz açılımından derleme/QA/üç DLL döngüsü için paket dışı
`surge-native-fixture-delivery-verification.json` son kaydı kullanılmalıdır.
Oyun ve VirusTotal etiketi hakkında yeni kabul veya neden iddiası yoktur.

Yerel doğrulama: Release206/206, ilgili Debug9/9 ve kaynak QA17/17 geçti.
Vendor regresyonları19/19, atlanan test yok. Pending bayrağı resetini yalnız
paket dışındaki negatif kontrol kopyasında kaldırmak, her iki tahsis yolunda
beklenen stale-pending hatasını üretti. Ürün/vendor/resource/export3096 dosyası
üst paketle byte-byte aynı; geliştirme DLL API adı3346 olarak korundu.
