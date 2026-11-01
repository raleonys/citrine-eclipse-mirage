# FIX16 — Kaynak accept hata akışı düzeltmesi

Tek runtime dosyası src/transport/service.cpp değişti. Paket kaynak atası FIX15 SHA256 `02326c3d3d7899a64014c12ee74111de33a8bcd884e5dd96b252bb425c7d2e34`. Bağımlılıklar ve diğer119 runtime dosyası değişmedi. Ürün adı/ABI/profil/config/avatar/persistence/hook politikaları korunur.

## D04: Neden farklıydı ve ne düzeltildi?

RE kaynağındaki accept_completion_025720 hata durumunu loglar, sonra yeni async_accept kurar. Bizim owned callback aynı hatada listenerı kapatıyor, stopping=true/ready=false yazıp dönüyordu. Kaynakta bu terminal dal yok; Asio upstream bunu gerektirmiyor. İlk kararın tarihsel gerekçesi kayıtlı değil. Teknik neden, completion errorının dispatcher/accept-rearm exceptionlarıyla aynı terminal politikaya sokulmuş olmasıdır; bu geriye dönük yetki değildir.

FIX16 yalnız bu close/stopping/return dalını kaldırır. Gerçek stop, accept rearm sırasında allocation/dispatch exceptionı ve genel dispatcher failureın mevcut terminal davranışı değişmez. Yeni retry sayısı, backoff, config switchi veya production test hooku eklenmez.

## Doğrulama yaklaşımı

Mevcut Asio test executableına yalnız Linux altında native accept sınırı hata enjeksiyonu eklenir. Upstream Asio gerçek TCP yolunu çalıştırır; accept syscallı üç kez EMFILE döndürür. Aynı listener/port üzerinden greeting+GetProfile, daha sonraki istemci ve explicit stop kontrol edilir. Shim yalnız test executableında; production veya vendor kaynakta test seçicisi yok. Mevcut test beklentileri değiştirilmedi. Windows native fault boundary bu yöntemle test edilmiş sayılmaz.

Eski FIX15 servis nesnesi yeni testle ayrıca bağlanır: yeni assertionın gerçekten eski terminal dalı yakalaması gerekir. Ham negatif kontrol current source hashine bağlanır. Diğer testler ve temiz ZIP rebuild QA kaydındadır.

## Başlangıç/ek DLL incelemesi

D01/D12 için PHASE_ORDER_REVIEW_FIX16.md somut kaynak/caller sırası ve sonraki tasarım işini verir. `from_main` adından proses ana-thread garantisi çıkarılmaz. Provider/LSX hazır olma düzenini çözmeden yalnız bir load çağrısını öne taşımak kaynak akışını kurmaz. Bu inceleme sonunda startup/thread/provider akışı değiştirilmedi; bu iki kayıt açık.

Kalan inceleme/karar sayısı18:17 inceleme +1 kısmi status. Bunlar18 kesin bug veya toplu geri alma kararı değildir. D04 kaynak kapsamı içinde kapandı. FIX15 düzeltmeleri korunur; eski kayıtlar current PASS diye kullanılmaz.

## Yetki ve sınır

Kullanıcının devam talimatı ve tam RE kaynakları esas alındı. CHANGE_AUTHORITY_FIX16.json tek runtime dosyasını sınırlar; dosya/hash denetimi anlamsal onay üretmez. Library master MD/JSON version9 yeniden okundu, revision2026-10-25.1 ve hashler eşleşti. NON_RELEASE; canonical/release pointer veya BROWNFIELD/diğer FIELD projeleri değişmez.
