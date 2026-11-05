Kaynak kararı: 2026-11-03. Kullanıcı özgün davranışı korumayı, ek güvenli
kaldırma şartlarını üretim hedefinden çıkarmayı ve derleme yapmamayı istedi.

Özgün application DllMain038870 yalnız reason1 için038970 çağırır; reason0
için uygulama işi yapmaz. entry.cpp içindeki require_loader_detach_quiescence
bu davranışla tanımlandı. Tarihsel adı quiescence garantisi anlamına gelmez.
DllMain detach'te Runtime.close, join, proof veya source exit callback çağrısı
yapılmaz. Compiler CRT, kaynakta gerçekten register_exit ile kayıtlı exit
callback'ini normal CRT sırasındaki yerinde çalıştırır. İkinci recovered CRT
katmanı ve ikinci cleanup çağrısı eklenmez.

Son turda eklenen zorunlu pre_cleanup_quiescence / CallbackCleanupPrerequisite
katmanı ve ona ait fixture değişiklikleri geri alındı. Backend/host assembly'nin
dört dosyası donmuş tam kaynak/ZIP ile birebir aynı hash'e döndü. Resources test
dosyası bu katmandan önceki hash'e döndü; önceki native IO/executor/lifecycle
kaynak çalışması korunuyor. Kooperatif Runtime.close/pre-unload yardımcıları
ayrı kaynak yolları olarak kalır; özgün DLL detach'inin yerine konulmaz.

require_loader_attach_runtime hâlâ tanımsızdır. Gerçek per-image Domain,
configuration, host/callback, Online, server/Graph ve kalıcı error/log sahipleri
kurulmalı; mevcut initial_cells::make_fresh_runtime gözlenen başlangıç seed'ini
sağlıyor. Gerçek image kökleri publication öncesinde sahiplenilmeli ve gerçek
ApplicationEntry038870 bir kez senkron çağrılmalıdır.038970 worker/host/online/
035850(true)/third-worker sırasına bekleyen controller veya yeni Initialize API
eklenmez. Aktif CRT exit callback'inin sahipleri cleanup boyunca yaşamalıdır.

Yeni kaynak için link beklentisi attach sembolünün eksik kalmasıdır. Bu bir
link sonucu değildir. Son gerçek donmuş link iki eksik sembol bildiriyordu;
yeni detach gövdesi henüz derlenmedi. build-link-gate.ps1'in gelecekteki beklentisi
buna göre güncellendi; verifier raporun kendi expected sembol sayısını kullanır.
Eski build/link/ZIP kanıtları değişmedi. Yeni test yazılmadı/çalıştırılmadı,
derleme/link/DLL execution ve ZIP güncellemesi yapılmadı.

Native kanıt: ../dll-critical-path/original-detach-profile.audit.json ve bağlı
yedi assembly Evidence kaydı. Bu statik dilim arbitrary live FreeLibrary için
güvenlik sertifikası değildir; böyle ek bir sertifika artık üretim hedefinin
zorunlu bitirme koşulu olarak kullanılmaz.
