# REDFIELD RF36 — CRT ile inert worker kurulumu

RF35'in kullanıcı production logunda 74 kaynak derlenmiş, önceki net C2386 aşılmış;
75.dosya DLL girişindeki constinit worker bildirimi C2127 ile durmuş. Link veya
DLL başarısı gözlenmemiştir. Log, constexpr kurucudaki belirli alt ifadeyi
göstermediği için tek bir üye/derleyici bug'ı kanıtlanmış gibi sunulmuyor.

WindowsWorker, üretim ve native composition'da standart CRT'nin global nesne
kurulumu ile başlatılır. Composite constructor artık constexpr değildir; iki
entrypoint'in global worker değişkenleri constinit zorlaması kullanmaz. Üyeler
açık değer başlatmasıyla kurulur; constructor yalnız başlangıç durumlarını,
hook referansını ve callback/context adreslerini yazar. Heap/Win32 çağrısı,thread
açma,hook/service kurulumu veya shutdown yoktur. Nesne global ömürlüdür ve iki
composition'ın da destructor'ı trivialdir;detach sırasında cleanup eklenmemiştir.

Mevcut desteklenen _DllMainCRTStartup ve/MT (/MTd Debug) korunur. CRT'nin user
DllMain öncesi global C++ kurucuları çalıştırması Microsoft'un belgelenmiş
davranışıdır. Kurucular da loader-lock sınırlarına tabidir;bu nedenle inert
kurulum zorunludur. /ENTRY:DllMain,/NODEFAULTLIB,/NOENTRY beş profilin tümünde
mevcut Python politikasınca reddedilir. Özel CRT/PTD veya function-local singleton
eklenmedi;session/hook işleri worker thread'de kalır. Explicit shutdown'ın
join/retirement/finalize/close/unpin vehost-held reference görevleri değişmedi.
Resmî kaynaklar metadata/CRT_STARTUP_REVIEW.json içindeki Microsoft URL'leridir.

GCC hostta gerçek production/native entry kaynakları veparent RF35 kaynakları
ayrı derlenip çalıştırıldı.22senaryo karşılaştırması toplam44execution geçti:
attach öncesi idle/config/transport durumu,inertSURGE/nonattach,idle shutdown,
pin/spawn başarısızlığı,normal/eksik/bozuk config,geçersiz yol,cleanup/close/unpin
retry,erken shutdown vegerçek yerel TCP frame/native observation akışları.
OS/WindowsHooks kontrollü modeldir;Win32 loader/MSVC testi değildir.58portable
core object ancak kaynak/header/vendor veRF35 receipt/object hash eşitliği
doğrulandıktan sonra modelde yeniden kullanıldı;entry/host ve7M02/M11 test main'i
taze derlendi. Her iki current entryobject init_array kurucu çağrısı içerir.

Başlangıçta model Win32 çağrısı veintercepted scalar-new sayısı sıfırdı;bu sayaç
her olası allocator API'sinin izlendiği iddiası değildir. Constructor'ın kendisi
kaynakta ayrıca incelendi.15CRT-entry policy negatif kontrol,2trivial-destructor
assertion veconstructor'a yanlışlıkla heap/Win32 yanetkisi ekleyen2mutasyon
beklenen şekilde reddedildi. İlk geçici test-harness derlemesinde eksik bir
WorkerPhase include'u düzeltildi;bu uygulama kaynağı hatası değildi.

M02/M11'in14subject kaynağı aynı olsa da worker_runtime.hpp dependency hash'i
değiştiği için eski proof'lar otomatik güncel sayılmadı. Inert CRT kurulum sınırı
yeniden incelendi;7güncel module programı ve22startup karşılaştırması ile yeni
source/dependency proof'ları bağlandı. Toplam16qualified aynı kaldı;M14 bütün
incelemesi kapanmadı.6modül kaynak review kapalı,14açık;284code+4vendor pending.

Normal aggregate temiz izole kaynaktan98regresyon PASS;RF35'in98stdout/exit
sonucu aynı.159object/99program üretildi. LegacyGetConfig14 ayrıFAIL_OBSERVED
olarak korunur. Build graph/profile,3export,7hook,35binding,resources/vendor,
surge.cfg veönceki RF35/RF34 düzeltmeleri korunur. Dörtoperator CMD veprofile
receipt başlığındaki sabitRF32 etiketi deRF36 kimliğine güncellendi;operasyonlar
aynıdır. Bu tur Golden/sanitizer yeniden çalıştırılmadı.

Gerçek güncel Windows/MSVC/native/CRT/Debug/oyun NOT_RUN. Kodda C2127'yi doğuran
zorunlu constant-init kaldırılmıştır;MSVC zincirinin kalanına başarı atfedilmez.
RFCL-005 oyun profili nedeni veM12/M20 işleri açık;HF19/CURRENT değişmedi.
