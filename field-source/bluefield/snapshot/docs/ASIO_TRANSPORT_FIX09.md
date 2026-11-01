> Tarihsel aşama kaydı. Güncel FIX13 davranışı ve geri alınan sınırlar için ORIGINAL_DEVIATION_AUDIT_FIX13.md geçerlidir.

# FIX09 — gerçek Asio taşıma katmanı

FIX08'in ardından kullanıcı devam edilmesini istedi. Bu adımda gerçek runtime
accept/read/write backend'i Asio'ya taşınır. Config2/PEGTL ve filesystem seçimleri
ayrı aşamada kalır. Var olan profile/config, public C ABI ve handler gövdeleri korunur.

## Bağımlılık ve köken

Asio 1.38.2, exact commit 8806a6803cde7054c3049d3666d3ec36786568c5. Include ağacı ve
BSL lisansı HF19'un pristine upstream ağacından byte-identical alınır. BROWNFIELD
application C++'ı kopyalanmaz. Yerel vendor yaması, network fetch veya alternatif
transport seçicisi eklenmez. Windows source guard ASIO_HAS_IOCP ister ve
ASIO_DISABLE_IOCP'yi reddeder. Portable testler platformun Asio backend'ini çalıştırır;
Windows IOCP çalıştırılmış veya Anadius'un exact Asio sürümü bulunmuş sayılmaz.

## Üretim yolu

Service::prepare, Application/config'i kurar, Asio TCP acceptor'ını loopback'e
bind/listen eder ve EALsxPort'u yayımlar. Host hook kurulumundan sonra activate,
online initialization'ı tamamlar; iki handler worker'ı ve tek IO thread başlatır.
Readiness bu kaynaklar kurulduktan sonra yayımlanır. İstemci başına thread yoktur.

Her session kendi socket, Connection/cipher, receive buffer ve outgoing string'ini
tutar. Aynı session'da bir yanıt bitmeden sonraki request handler'a kabul edilmez:
FIX08'in connection FIFO/cipher sırası korunur. Kernel read/write asenkrondur;
greeting/handler/persistence işi IO thread'ini bloke etmemek için sınırlı worker pool'dadır.
Uygulamanın global state mutex'i hâlâ handler'ları serialize edebilir; throughput
artışı veya tüm lock sorunlarının çözüldüğü iddia edilmez.

Receive window mevcut server::receive_request_size algoritmasını kullanır.
NUL frame, boş frame atlama ve 1 MiB frame/config sınırı sürer. Teslim edilen byte'lar
completion error kontrolünden önce pending'e commit edilir, fakat error completion'dan
request dispatch edilmez. async_write tüm outgoing string'ini sahipli tutar;
error veya beklenenden kısa completion session'ı kapatır. Aynı anda yalnız bir write
vardır. Yeni timeout veya config anahtarı eklenmez; mevcut 32 istemci limiti korunur.

## Ömür ve stop

Session ve işlem closures'ı shared ownership taşır; socket ile Application'a
başvuran işler aynı lifetime'a bağlıdır. Stop önce ready=false/stopping=true yapar,
IO context'i durdurur ve IO thread'ini join eder. Handler pool stop/join olduktan
sonra socket'leri kapatır, pending ownership'i ve io_context'i yok eder. Application
ve online provider bu işlemlerden sonra kapanır. Worker'ın io'ya dönüş post'u
başarısız olduğunda, o session için socket operation bulunmadığından allocation
istemeyen close yapılır; failed flag'i yeni accept veya stop sırasında owner'ı
emekliye ayırır. Host socket'lere pool join'den önce dokunmaz.

Accepted socket, Session allocation/start exception'ında otomatik sahiplik altında
kalır. IO thread girişinde exception sınırı vardır; allocating constructor'lar
noexcept değildir. Tek seferlik accepted-client allocation failure sonrasında yeni
istemciye başarıyla yanıt dönmesi regresyondadır (CMP05'in source yolu). Sürekli
accept rearm allocation/completion hatası listener'ı kapatır ve service failure'ı
loglar; sonsuz retry veya sahte ready yoktur. Her OOM/interleaving'in eşdeğer
olduğu ya da Windows kernel cleanup'ın test edildiği iddia edilmez.

IOCP cancellation/drain ve gerçek hook/unload senaryoları Windows Native/CRT
fixture'larında ayrıca çalıştırılmalıdır. Host DllShutdown nonzero sonucunda modülü
boşaltamaz; mevcut module pin/quiescence düzeni korunur.

## Doğrulama

Var olan gerçek TCP/crypto/achievement/persistence ve 8-client integration testleri
aynı handler/core üzerinde çalışır. Yeni test: parçalı/coalesced/boş frame, 30 yanıt
FIFO'su, 300 KB yanıtı 137-byte recv parçalarıyla okuma, malformed session sonrası
başka client, tek seferlik background allocation hatası ve accept recovery, eksik
frame varken stop, repeated lifecycle, 32-client admission sınırı ve toplu stop.

Portable object cache Asio .ipp/.inl implementation başlıklarını da hashler;
header-only implementation değişikliği eski object'i tekrar kullanamaz.
Source policy, aktif Asio include/standalone bağımlılığını ve Windows IOCP-only
koşulunu negatif mutasyonlarla bağlar. Gerçek Windows sonucu burada NOT_RUN.

## Özgün eşdeğerlik sınırı

Özgün Asio/IOCP tanımları ve receive-window sırası input kanıtında vardır. Buradaki
yeni owned Asio backend, opaque original scheduler/private-ABI nesnelerini veya
özgün detached-worker tamamlama yarışlarını taklit etmez. FIX08 request sırasını
koruyan bounded sahiplik seçilmiştir. Orijinal async sıralama, EOF/error/zero-progress
ve provider lifecycle için Consumer/original karşılaştırmaları hâlâ gerekir.
Bu adım “orijinal DLL bütünüyle tamamlandı” sonucu değildir.
