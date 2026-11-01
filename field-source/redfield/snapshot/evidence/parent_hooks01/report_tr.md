# REDFIELD RF21 HOOKS01 — Windows hook entegrasyonu

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

Bu paket hookları Windows DLL'e fiilen bağlar. RF20 FIX04'ün yalnız listener/service başlatan worker'ına, HF19 kaynaklı MinHook kurulumu ve retry edilebilir söküm eklendi. Windows'ta bu yeni hooklu DLL henüz derlenip çalıştırılmadı; paket kaynak teslimidir ve oyun açılış garantisi değildir.

## Dahil edilen hooklar

Beş registry API'si: RegOpenKeyExA/W, RegQueryValueExA/W, RegCloseKey. Ayrıca CryptQueryObject ve destination-aware ws2_32!connect. Toplam7. Connect yalnız sockaddr_in uzunluğu16, IPv4 127.0.0.1:3216 hedefini seçilmiş listener portuna yönlendirir; diğer IP/port/aileler ve çağıranın adres belleği korunur. Crypt wrapper'ın mevcut HF19 companion-argument davranışı korunur; Windows API sonucu veya güven sonucu üretilmez. Registry policy yalnız Game bölümündeki ayarları, son duplicate kazanacak şekilde immutable ConfigSnapshot üzerinden okur. Yoksa orijinal HF19 registry varsayılanları geçerlidir.

Master'ın eski9-hook envanterine karşı güncel gerçek kaynak disposition'ı: aynı HF19 arşivinin CHANGELOG'u HF16/HF17'de Process32NextW ve _Mtx_trylock uygulamalarının açıkça kaldırıldığını ve7 çekirdek hook kaldığını kaydeder. evidence/hf19_changelog_original.md bu dosyanın birebir kopyasıdır. Kullanıcının bu RF21 hook talebinde güncel7 çekirdek kaynak korunur; iki kaldırılmış Surge hooku veya isteğe bağlı CreateProcess/SHGetFolderPath addon hookları geri getirilmez. Process-wide htons detour yoktur. SURGE passive single-return kalır.

## Başlangıç ve kapanış

DllMain yalnız worker scheduling yapar. Worker config/listener portunu elde eder, immutable hook configuration authority'sini alır, gerekli sistem modüllerini referansla tutar, yedi hooku tek MinHook batch transaction içinde oluşturup kuyruğa alır. Tek SAFE-FREEZE apply başarılı olduktan sonra registry policy ve ready fazı yayımlanır. Patching/freeze/wait DllMain içinde çalışmaz. Listener seçimi hook kurulumu öncesindedir; Consumer'ın worker-ready beklemesi veya erken bağlantı zamanlaması bu aşamada kanıtlanmış değildir.

Kapanış önce açık pseudo HKEY olup olmadığını kontrol eder. Böyle bir owner varsa interception korunur ve DllShutdown retry ister; RegCloseKey hâlâ çalışır. Ardından global restore/gate drain, subsystem remove, MinHook uninitialize, config/module release ve service stop yapılır. Orijinal registry teardown ayrıca idempotent disable uygulayabilir. Restore/remove/quiescence/module-release başarısızsa sahiplik, gerekli config ve DLL pin'i korunur; başarısız cleanup temizmiş gibi sunulmaz. İkinci WindowsHooks sahibinin ilk sahibin hooklarına dokunması config authority acquisition ile engellenir.

Orijinal registry recovery hatası bounded olarak düzeltilir: recovery_pending kontrolü mode kısa devresinden ve policy activation'dan önce yapılır. İki orijinal TU provenance altında birebir saklanır ve değişiklikler derived recipe ile bağlanır. Diğer HF19 hook gövdeleri ve MinHook upstream dosyaları birebirdir. Kümülatif1227 adopted ve27 derived dosya vardır; yeni patch engine yoktur.

## Build ve doğrulama

Python build ve CMake grafiği59 portable core TU,12 Windows hook TU ve4 MinHook C birimini kullanır.90 regresyon +ayrı GetConfig tanısı vardır. Windows toplam170 compile eylemi; aynı hook/vendor objectleri test ve production/native DLL linklerinde kullanılır. MinHook commit'i8af6b4acae5a9388fd742b56fa79ece89d96f823;61 upstream dosyanın hash'i ve BSD lisansı korunur. Mevcut SAFE-FREEZE patch'i kaynakla bağlı olarak build-tree hook.c'ye uygulanır. Python yolu CMake kurulumuna ihtiyaç duymaz; CMake yolu orijinal patch'i bağımsız çalıştırıp aynı hash'i arar. Burada CMake mevcut olmadığından o yol NOT_RUN.

| Kontrol | Gerçek sonuç |
|---|---|
| İzole temiz GCC C++20 normal koşusu |90/90 PASS;150 object/91 linked output|
| Sonraki Windows-only değişiklik |windows_hooks.cpp singleton acquisition guard; Linux derlenen tüm girdiler aynı; receipt değiştirilmedi|
| Güncel actual-source hook harness |27 senaryo ×normal/ASan/UBSan =81 PASS|
| Environment mock / gerçek POSIX runner |13 /7 PASS|
| GetConfig legacy tanısı |exit14, ayrı FAIL_OBSERVED|
|61 vendor hash /adoption/recipe/graph/export/launcher |PASS|
| Bağımsız bozulma testleri / ZIP CRC / temiz çıkarım integrity |beklenen ret /PASS /PASS|
| Yeni gerçek Windows/MSVC/MinHook/SAFE-FREEZE/oyun |NOT_RUN|

Hook harness gerçek WindowsHooks, HF19 MinHook runtime/gate, wrapper ve registry-policy gövdelerini Linux ELF'te derler; Win32 API ve MH backend kontrollü sahte uygulamalardır. Gerçek executable patch, Windows ABI veya NtGetNextThread çalıştırılmaz. Create/queue failure noktaları, apply+rollback failure, pseudo-handle preflight, remove retry, aktif çağrı drain timeout, module release retry, ikinci owner reddi,3 tekrar lifecycle, connect passthrough/adres değişmezliği ve crypt forwarding/LastError sınırı denenir. ASan leak detection kapalıdır; leak-free iddiası yoktur.

İlk workspace koşusunda bütün testler çalıştıktan sonra dört ELF dosyasının execute izni değişmiş bulundu; neden belirlenmedi ve koşu PASS sayılmadı. Guard bypass edilmeden /tmp altındaki temiz izole kopyada tam90 test tekrar derlenip çalıştı. Bu receipt sonrası yalnız Windows-only singleton guard değişti. evidence/rf21_portable_conservation.json bütün diğer build girdilerinin aynı olduğunu ve bu Windows dosyasının portable executable'lara derlenmediğini doğrular. Son Windows kaynak hash'i81 senaryoya bağlanır; eski geniş source-tree receipt'i yeniymiş gibi yeniden yazılmaz.

## Kullanım ve kalan iş

Yeni ZIP'i ayrı klasöre çıkarın. REDFIELD_RF21 içinde scripts\build_orchestration\build_verify.cmd çalıştırın. Testlerin ardından build\msvc_x64\preloader_l.dll, production lifecycle smoke ve genişletilmiş native acceptance çalışır. Native host artık published porta doğrudan bağlanmak yerine3216 üzerinden gerçek connect hookunu dener; registry pseudo handle açıkken shutdown'ın bekleyip close sonrası retry ile bitmesini de kontrol eder. Bu yeni native senaryolar hazırlanmıştır; henüz çalıştırılmış PASS değildir.

SDK greeting/challenge/session crypto ve Consumer launch/handshake entegrasyonu bu pakete dahil değildir. Dolayısıyla hookların eklenmesi oyunun tek engelinin kalktığını kanıtlamaz.35 mevcut plaintext route korunur. Parent FIX04 kullanıcı Windows86/86/ABI/native PASS kanıtı ayrı tutulur; RF21 kabulüne miras alınmaz. Üç production export, GPL-3.0-only ve resmî HF19/CURRENT korunur. Parent kaynak ZIP SHA-256:06e048125b99333c51374d78958b0994e27a97594d4fec185b1aea3e7dcea199.
