# REDFIELD RF23 OWNERSHIP02 — bellek/kapanış ve bağımlılık refaktörü

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

RF22 üzerinde belirlenen üç somut iş tamamlandı: bekleyen fiziksel bellek serbest bırakmalarını DLL kapanışına bağlamak, geçici XML belleğini genel allocator/PTD bağımlılığından ayırmak ve kayıt yanıtlarındaki ortak writer kodunu toplamak. Yeni route/hook/protokol özelliği eklenmedi.

## Kapanış boşluğu ve düzeltme

RF22'nin gerçek WorkerOwner ve allocator kaynakları, kontrollü heap-free başarısızlığıyla çalıştırıldı: `OLD_CLEAN_WORKER shutdown=0 unpins=1 pending=1`. Worker temiz işaretlendiğinde shutdown başarılı sayılabiliyor, fakat fiziksel release kuyruğu beklemede kalıyordu. Bu Linux kaynak yeniden üretimidir; kullanıcı makinesinde gerçekleşmiş bir Windows HeapFree hatası olarak sunulmaz.

WorkerOperations artık bağımsız, tekrar denenebilir finalize callback'i taşır. Worker beklemesi ve session cleanup sonrasında, thread handle ve DLL referansı bırakılmadan önce çağrılır. Worker clean işareti bu kontrolü atlatamaz. DLL callback'i drain_transient_releases kullanır; başarısızlık mevcut10/service_cleanup_pending sonucunu üretir, thread handle ve modül referansı korunur. Sonraki çağrı temizliği tamamlayabilir. Sıfır süre beklemesiz tek denemedir; pozitif süre kalan shutdown bütçesini kullanır. Fiziksel heap callback'leri senkron çağrılardır; timeout çalışan bir OS çağrısını zorla kesmez.

Retry işleminin kuyruğu listeden geçici olarak ayırdığı sırada ikinci bir çağrı listeyi boş görebiliyordu. Tek retry yürütücüsü ve in-progress durumu eklendi. Devam eden heap-free callback'i varken ikinci drain başarılı kabul edilmez. Latch kontrollü gerçek iki-thread testi bu sınırı denetler. Shutdown fence'i üreticiler durduktan sonra kullanılır; keyfi dış çağrı yapan thread'ler için bağımsız DLL lifetime garantisi değildir. Pending count/byte istatistikleri kuyruktaki blokları sayar; in-progress bool ayrılmış batch'i de kapsar.

## Bağımlılıklar ve ortak yanıt kodu

TransientPlatformOps yalnız context/heap_alloc/heap_free taşır. XML SemanticValue, ayrı transient_allocator.hpp/cpp üzerinden çalışır; genel allocation retry/new-handler/errno/PTD arayüzlerine bağlı değildir. Üretim core58 TU'dur. Eski genel memory_allocator.cpp ve ptd_runtime.cpp davranışları korunarak yalnız regresyon desteği için derlenir; Python üretim DLL link listesine ve CMake redfield_core hedefine girmezler. Bu ayrım kodun kökenine değil gerçek üretim kullanımına dayanır. Kaynaklar hâlâ aynı bakım ve provenance kurallarındadır.

Gerçek _WIN32 SemanticValue dalı, Heap API shim'iyle yalnız transient_allocator.cpp bağlanarak normal/ASan/UBSan altında çalıştı. Üretilen ELF sembollerinde current_errno_slot, initialize_ptd_runtime ve allocate_block yoktur. __declspec/__stdcall bu modelde kaldırılmıştır; Windows native/ABI testi değildir.

Content/Entitlement/Achievement yanıtlarında aynı ServiceTextView uyarlaması ve measure/render finish sözleşmesi record_response_support.hpp içinde tek uygulamaya taşındı. Değişmez snapshot okuyan record_count/record_at metotlarının yanıltıcı _locked son eki kaldırıldı. Gerçek registry kilidi altında çalışan yöntemlerin adları korunur. Mevcut wire format ve validation davranışı değişmedi.

Ek olarak RF22 CMake legacy diagnostic hedefinde kalmış tests/adopted yolu bulundu; kaynak artık doğrudan BUILD_GRAPH diagnostics kaydından okunur. CMake bu ortamda mevcut olmadığı için configure/build çalıştırılmadı; Python tam derlemesi ve CMake kaynak/bağımlılık kaydı kontrol edildi.

## Doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz GCC derlemesi |58 production +2 test-support TU;155 toplam obje,95 linked çıktı PASS|
| Regresyonlar |94/94 PASS; önceki92 stdout/exit RF22 ile birebir|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14; pozitif test sayısından ayrı|
| Worker,pending session,retirement,shutdown fence,concurrency,semantic allocator |ASan leak kapalı ve UBSan altında ayrı ayrı6/6 PASS|
| _WIN32 SemanticValue heap modeli |3/3 PASS,genel allocator/PTD olmadan link ve çalışma|
| Eski clean-worker kapanışı |Beklenen kusur yeniden üretildi; yeni iki test fence/retry ve concurrency'yi denetler|
| Mimari geriye dönüş/production PTD coupling |9 beklenen ret; metadata ve hash yeniden bağlandığında da guard devrede|
| Paket bozma kontrolleri |2 beklenen ret|
| Encoding/environment/POSIX runner |16/13/7 case PASS veya beklenen ret|
| Manifest/538 ancestry recipe/ZIP CRC/deterministik repack/taze extract |PASS|
| RF23 Windows MSVC/native/game/CMake |NOT_RUN|

258 uygulama dosyası ortak REDFIELD düzenindedir. Windows tam build planı175 compile eylemidir. Üretimde58 core +12 hook C++ +4 MinHook C +1 DLL entry vardır.7 hook backend,35 route binding,3 production export ve bütün vendor/pin/SAFE-FREEZE byte'ları RF22 ile korunur. RF22'deki81 hook-model sonucu historical kanıttır; hook backend'leri bu kapsamda yeniden test edilmiş gibi sunulmaz. Bellek/kapanışta yukarıdaki taze odaklı testler çalışmıştır. LeakSanitizer ortam kısıtı sürer; leak tespiti PASS iddiası yoktur.

## Çalıştırma ve kapsam

Yeni klasöre çıkarıp `REDFIELD_RF23\scripts\build_orchestration\build_verify.cmd` çalıştırın. Başlangıç58 core,94 regresyon ve RF23 OWNERSHIP REFACTOR 02 göstermelidir. Production DLL `build\msvc_x64\preloader_l.dll`,log aynı klasörde `build.log` olur. Tam native kabul completion marker'ı ayrıca gereklidir; DLL'in üretilmesi tek başına tüm testlerin geçtiği anlamına gelmez.

Bu paket belirtilen üç refaktörü ve ilgili build düzeltmesini tamamlar. Tüm kod tabanının ayrıntılı anlamsal denetiminin bittiği veya yeni Windows/game sonucunun doğrulandığı iddia edilmez. Canonical HF19/CURRENT,lisanslar ve immutable parent kayıtları korunur. Parent RF22 ZIP SHA-256: bacc81fd6bbcf8170e1a2b651f1639a56541bc2152ad6ace913cb668530a5d20.
